/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Nick van der Meijs
 *	Simon de Graaf
 * Delft University of Technology
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/scan.h"
#include "src/space/scan/extern.h"
#include "src/space/scan/determ.h"

extern int nrOfResizes;
#ifndef DRIVER
extern resizeData_t * resizes;
#else
#undef  Debug
#define Debug(S) S
#undef  PRINT
#define PRINT(S) printf (">> %s\n", S)
#endif

#define LAYOUT_VIEW 0

struct Proj {
    int xcontrol;
    DM_PROJECT  *pkey;
    struct Proj *next;
};
struct Cell {
    int depth;
    int imported;
    char * name;
    struct Cell * prev;         /* all cells seen so far */
    struct Cell * next;
};

#ifdef __cplusplus
  extern "C" {
#endif

Private IMPCELL *dmGetImpCell (char *name);
Private do_t *makeCandidate (char *name);
Private int resizeUpToDate (DM_CELL *layoutKey);
Private void freeImpStuff (void);

#ifdef __cplusplus
  }
#endif

static DM_PROJECT *dmproject;

/* The head of a list of all cells seen so far. */
static struct Cell * cells;
static struct Proj * projs;

static unsigned long glntime = 0; /* layout last expand time */

#ifdef DRIVER
int cellcount = 0;
DM_PROCDATA *procdata;
#endif

/* Sole entry point of this module.
 *
 * Given a project and root cell name as argument, this function
 * returns a do_t pointer of the root candidate to be extracted.
 * The do_t also contains info describing whether the cell must
 * be preprocessed first.
 */
do_t * findCandidates (DM_PROJECT *project, char *root)
{
    dmproject = project;

    return makeCandidate (root);
}

Private void addCell (char *name, int imported, int depth)
{
    struct Cell *c;

    for (c = cells; c && strcmp (c -> name, name); c = c -> prev);
    if (!c) {
	c = NEW (struct Cell, 1);
	c -> depth = depth + 1;
	c -> imported = imported;
	c -> name = _dmStrSave (name);
	if (cells) cells -> next = c;
	c -> prev = cells;
	c -> next = NULL;
	cells = c;
#ifdef DRIVER
	++cellcount;
#endif
    }
}

Private void disposeCells ()
{
    struct Cell *c;

    while (cells) {
	cells = (c = cells) -> prev;
	_dmStrFree (c -> name);
	DISPOSE (c, sizeof (struct Cell));
    }
}

Private int checkCell (struct Cell *c)
{
    DM_XDATA xdata;
    struct stat buf1;
    DM_PROJECT *project;
    DM_CELL   * lkey;
    int cstatus, rv = 0;
    char *name = c -> name;

    Debug (fprintf (stderr, "visit '%s', depth %d, imported %d\n", name, c -> depth, c -> imported));

    xdata.name = name;
    cstatus = (dmGetCellStatus (dmproject, &xdata) == 0);
    if (cstatus) {
	if (xdata.timestamp > glntime) {
	    PRINT ("cell status timestamp > glntime");
	    return (1);
	}
    }

    if (c -> imported) {
	struct Proj *p;
	IMPCELL *icp = dmGetImpCell (name);
	project = dmOpenProject (icp -> dmpath, PROJ_READ);
	for (p = projs; p && p -> pkey != project; p = p -> next) ;
	if (!p) {
	    p = NEW (struct Proj, 1);
	    p -> xcontrol = (dmStatXData (project, &buf1) == 0);
	    p -> pkey = project;
	    p -> next = projs;
	    projs = p;
	    if (!p -> xcontrol) say ("%s: project has no xcontrol", project -> dmpath);
	}
	name = icp -> cellname;

	/* If the current project has xcontrol, the status of
	   imported cells must be specified locally for old projects,
	   else the remote project must have also xcontrol.
	*/
	/* Must we use remote xcontrol? */
	if (!cstatus || xdata.celltype == DM_CT_IMPORT) {
	    cstatus = 0;
	    if (p -> xcontrol) { /* remote project has xcontrol */
		xdata.name = name;
		if (dmGetCellStatus (project, &xdata) == 0) {
		    if (xdata.timestamp > glntime) {
			PRINT ("imported cell status timestamp > glntime");
			return (1);
		    }
		    cstatus = 1;
		}
	    }
	}
    }
    else {
	project = dmproject;
    }

    if (cstatus) {
	switch (xdata.celltype) {
	case DM_CT_MACRO:
	    PRINT ("is macro");
	    break;
	case DM_CT_DEVICE:
	    PRINT ("is device");
	case DM_CT_LIBRARY:
	    if (xdata.celltype == DM_CT_LIBRARY) PRINT ("is library");
	    if (xdata.interfacetype == DM_IF_STRICT) return (0); /* DEVMOD */
	    PRINT ("is macro");
	}
    }

    lkey = dmCheckOut (project, name, ACTUAL, DONTCARE, LAYOUT, READONLY);

    /* get layout last modified time */
    if (dmStat (lkey, "mc", &buf1)) {
	PRINT ("mc: cannot stat");
    }
    else if (buf1.st_mtime > glntime) {
	PRINT ("mc lmtime > glntime");
	rv = 1;
    }
    else if (!c -> imported) {
	/* In flat mode, visit the sub-cells to see
	 * if the root cell should be pre-processed.
	 */
	DM_STREAM *dsp = dmOpenStream (lkey, "mc", "r");
	while (dmGetDesignData (dsp, GEO_MC) > 0) {
	    addCell (gmc.cell_name, gmc.imported, c -> depth);
        }
        dmCloseStream (dsp, COMPLETE);
    }

    dmCheckIn (lkey, COMPLETE);
    return (rv);
}

Private do_t *makeCandidate (char *name)
{
    char gln_file[DM_MAXLAY + 8];
    struct stat buf1;
    unsigned long lmtime;
    DM_CELL *lkey;
    DM_XDATA xdata;
    do_t *l;
    int ok = 1;

    if (dmStatXData (dmproject, &buf1)) {
	say ("sorry: current project has no xcontrol"); die ();
    }

    l = NEW (do_t, 1);
    l -> name = name;
    l -> status = DEVMOD;

    /* By flat extraction, there is only 1 candidate and
     * this candidate is always extracted (except when it is a device).
     */
    Debug (fprintf (stderr, "candidate '%s', depth 1\n", name));

    xdata.name = name;
    if (dmGetCellStatus (dmproject, &xdata) == 0) {
	if (xdata.celltype == DM_CT_DEVICE) {
	    PRINT ("is device");
	    return (l);
	}
	if (xdata.celltype == DM_CT_MACRO) {
	    PRINT ("is macro");
	    l -> status = ISMACRO;
	    return (l);
	}
    }

    lkey = dmCheckOut (dmproject, name, WORKING, DONTCARE, LAYOUT, ATTACH);

    if (dmStat (lkey, "mc", &buf1)) { /* get layout last modified time */
	say ("%s: cannot stat stream 'mc'", name); die ();
    }
    lmtime = buf1.st_mtime;

    l -> lkey = lkey;
    l -> status = DO_SPACE;

#ifdef DRIVER
    sprintf (gln_file, "%s_gln", procdata -> mask_name[0]);
#else
    sprintf (gln_file, "%s_gln", masktable[0].name);
#endif
    Debug (fprintf (stderr, "gln_file = %s\n", gln_file));

    if (dmStat (lkey, gln_file, &buf1)) {
	PRINT ("old expand: no gln file!");
	ok = 0;
    }
    else {
	glntime = buf1.st_mtime;
	if (!existDmStream (lkey, "spec")) {
	    PRINT ("old expand: not flat!");
	    ok = 0;
	}
    }

    if (ok) {
	if (existDmStream (lkey, "resize.t")) {
	    PRINT ("old resize");
	    if (nrOfResizes > 0) {
		PRINT ("new resize");
		if (!resizeUpToDate (lkey)) {
		    PRINT ("old != new resize!");
		    ok = 0;
		}
		else
		    PRINT ("old == new resize");
	    }
	    else {
		PRINT ("no new resize!");
		ok = 0;
	    }
	}
	else if (nrOfResizes > 0) {
	    PRINT ("no old resize, new resize!");
	    PRINT ("do resize");
	    l -> status |= DO_RESIZE;
	}
    }

    if (ok && !optNoPrepro) { /* preprocessing needed? */
	/* In flat mode, recursively visit the sub-cells to see
	 * if the root cell should be pre-processed.
	 */
	if (lmtime > glntime) {
	    PRINT ("old expand: too old!");
	    ok = 0;
	}
	if (ok) {
	    struct Cell *c;
	    DM_STREAM *dsp = dmOpenStream (lkey, "mc", "r");

	    addCell (name, 0, 0); c = cells; /* root cell */

	    while (dmGetDesignData (dsp, GEO_MC) > 0) {
		addCell (gmc.cell_name, gmc.imported, 1);
	    }
	    dmCloseStream (dsp, COMPLETE);

	    while ((c = c -> next)) {
		if (checkCell (c)) {
		    PRINT ("old expand: subcell newer!");
		    ok = 0; break;
		}
	    }

	    disposeCells ();
	}
    }

    if (!ok) {
	PRINT ("do expand");
	l -> status |= DO_EXP;
	if (nrOfResizes > 0) {
	    PRINT ("do resize");
	    l -> status |= DO_RESIZE;
	}
    }
    else PRINT ("old expand: ok");

    if (dmproject -> impcelllist[LAYOUT_VIEW]) freeImpStuff ();

    return (l);
}

/* Returns TRUE if stream exists, FALSE otherwise.
 */
bool_t existDmStream (DM_CELL *key, char *streamName)
{
    struct stat buf;
    return (dmStat (key, streamName, &buf) == 0 ? TRUE : FALSE);
}

#ifndef DRIVER
Private int resizeUpToDate (DM_CELL *layoutKey)
{
    int ok;
    int i, j;
    int maj, min, nr;
    char buf[1024];
    double val;
    int condcnt, id;
    mask_t p, a;
    resizeCond_t *resCond;
    DM_STREAM * dsp_rs;

    /* read resize information from the stream "resize.t" to
       decide whether or not it is necessary to run makesize again. */

    if ((dsp_rs = dmOpenStream (layoutKey, "resize.t", "r"))) {
        ok = 1;
        if (dmScanf (dsp_rs, "%d %d", &maj, &min) != 2
            || maj > 1 || min > 1) {
            ok = 0; goto end_of_resize_check;
        }
        if (dmScanf (dsp_rs, "%d\n", &nr) != 1
            || nr != nrOfResizes) {
            ok = 0; goto end_of_resize_check;
        }
        for (i = 0; i < nrOfResizes; i++) {
            if (dmScanf (dsp_rs, "%s %d %lg %d", buf, &id, &val, &condcnt) != 4
                || strcmp (resizes[i].newmaskname, buf)
                || resizes[i].id != id
                || resizes[i].val != val
                || resizes[i].condcnt != condcnt) {
                ok = 0; goto end_of_resize_check;
            }
            resCond = resizes[i].cond;
            for (j = 0; j < condcnt; j++) {
		nr = dmScanf (dsp_rs, " %s", buf);
		if (nr == 1) initcolorint (&p, buf);
		nr += dmScanf (dsp_rs, " %s", buf);
		if (nr == 2) initcolorint (&a, buf);
		if (nr != 2
		    || !COLOR_EQ_COLOR (&p, &resCond -> present)
		    || !COLOR_EQ_COLOR (&a, &resCond -> absent)) {
		    ok = 0; goto end_of_resize_check;
		}
                resCond = resCond -> next;
            }
        }
    }
    else
        return (0);

end_of_resize_check:
    dmCloseStream (dsp_rs, COMPLETE);
    return (ok);
}
#else
Private int resizeUpToDate (DM_CELL *layoutKey)
{
    return nrOfResizes > 1;
}
#endif

/* Get imported celllist structure for imported cell (alias) name.
 * Don't read the imported celllist more than once.
 */
Private IMPCELL * dmGetImpCell (char *name)
{
    register IMPCELL **icl;

    if (!(icl = dmproject -> impcelllist[LAYOUT_VIEW])) {
	dmGetMetaDesignData (IMPORTEDCELLLIST, dmproject, LAYOUT);
	icl = dmproject -> impcelllist[LAYOUT_VIEW];
	if (!icl) goto err;
    }
    while (*icl) {
	if (strsame ((*icl) -> alias, name)) return *icl;
	++icl;
    }
err:
    dmerrno = DME_NOCELL;
    dmError (name); /* die */
    return NULL;
}

Private void freeImpStuff ()
{
    register struct Proj *p;

    _dmFreeImportedCelllist (dmproject, LAYOUT_VIEW);

    while ((p = projs)) {
#ifdef DRIVER
	printf ("imp.proj = %s (xctrl=%d)\n", p -> pkey -> dmpath, p -> xcontrol);
#endif
	dmCloseProject (p -> pkey, QUIT);
	projs = p -> next;
	DISPOSE (p, sizeof (struct Proj));
    }
}

#ifdef DRIVER

bool_t optNoPrepro = FALSE;
int nrOfResizes = 0;

int main (int argc, char **argv)
{
    DM_PROJECT  * project;
    do_t *l;
    char *cellname = NULL;
    int i, j;

    for (i = 1; i < argc; ++i) {
	if (argv[i][0] == '-') {
	  for (j = 1; argv[i][j]; ++j)
	    switch (argv[i][j]) {
	    case 'r':
		++nrOfResizes;
		break;
	    case 'u':
		optNoPrepro = TRUE;
		break;
	    }
	}
	else {
	    if (cellname) {
		printf ("specify only one cellname\n");
		return 1;
	    }
	    cellname = argv[i];
	}
    }

    if (!cellname) {
	printf ("no cellname specified\n");
	printf ("usage: determ [-ru] cellname\n");
	return 1;
    }

    dmInit ("test");
    project = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);
    procdata = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, project);

    l = findCandidates (project, cellname);

    printf ("cellcount = %d\n", cellcount);
    printf ("extract flat %s\n", cellname);

    if (l) {
	printf ("%s, status %d\n", l -> name, (int) l -> status);
	if (l -> status & DO_SPACE) {
	    if (l -> status & DO_EXP)    printf ("do expand\n");
	    if (l -> status & DO_RESIZE) printf ("do resize\n");
	    printf ("do extract\n");
	    dmCheckIn (l -> lkey, COMPLETE);
	}
	else {
	    if (l -> status & DEVMOD) printf ("no extract, has device status\n");
	    else
	    if (l -> status & ISMACRO) printf ("no extract, has macro status\n");
	    else
		printf ("error: no extract status!\n");
	}
    }
    else printf ("error: no candidate found!\n");

    dmCloseProject (project, QUIT);
    return 0;
}

void die ()
{
    dmQuit ();
    exit (1);
}

void dmError (char *s)
{
    dmPerror (s);
    die ();
}

#endif /* DRIVER */
