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
#endif

#ifdef __cplusplus
  extern "C" {
#endif

/* local operations */
Private struct Cell *makeTree (char *name, int depth, int imported);
Private IMPCELL *dmGetImpCell (char *name);
Private DM_CELL *existCircuit (char *name);
Private do_t *makeCandidateArray (void);
Private int resizeUpToDate (DM_CELL *layoutKey);
Private unsigned long traverse (struct Cell *c, unsigned long ct, int flat);

#ifdef __cplusplus
  }
#endif

struct Cell {
    char * name;
    struct Cell * next;         /* all cells seen so far */
    struct Cell * candidates;	/* extraction candidates in dfs order */
    struct Cell * fringe;
    struct adj * children;
    int depth;
    unsigned int status;
    unsigned long smtime;	/* status last modified time */
    unsigned long lmtime;	/* layout last modified time */
};

struct adj {
    struct Cell * cell;
    struct adj  * next;
};

static DM_PROJECT *dmproject;

/* The head of a list of all cells that must be extracted (if not
 * restricted by the optMaxDepth option).
 * The list is in reverse depth-first order, after another reversion
 * it is suitable for extraction.
 */
static struct Cell * candidates;

static struct Cell * root_c;

/* The head of a list of all cells seen so far. */
static struct Cell * cells;

static char * gln_file = NULL;
static int xcontrol = 0;

/* Sole entry point of this module.
 *
 * Given a project and cell name as argument, this function returns
 * an array of do_t's (not pointers to them) describing all cells
 * in the complete hierarchical tree that must be extracted.
 * The depth of the tree can be restricted by optMaxDepth.
 * The do_t also contains info describing whether the cell must
 * be preprocessed first.
 *
 * Primarily meant for hierarchical mode, but for convenience it
 * also works in flat mode.
 *
 * The end of the list is marked by .name == NULL.
 *
 * General strategy:
 * 1. Make a dag with root at the top.
 * 2. Determine depth (distance to the root) of each cell.
 * 3. Make a linked list in correct order for extraction,
 *    and return the list.
 */
do_t * findCandidates (DM_PROJECT *project, char *root)
{
    struct stat buf1;

    xcontrol = (dmStatXData (project, &buf1) == 0);
    dmproject = project;
#ifndef DRIVER
    if (!gln_file)
	gln_file = strsave (mprintf ("%s_gln", masktable[0].name));
#endif

    root_c = makeTree (root, ROOT_DEPTH, 0);

    return makeCandidateArray ();
}

Private struct Cell * makeTree (char *name, int depth, int imported)
{
    DM_XDATA xdata;
    int cstatus;
    struct stat buf1;
    struct adj  * a;
    struct Cell * c;
    DM_PROJECT *project;
    DM_STREAM * dsp;
    DM_CELL   * lkey;
    char * versionstatus;
    int flag, mode;

    /* Check if cell has already been done.
     * If so, return immediately
     */
    for (c = cells; c && strcmp (c -> name, name); c = c -> next);
    if (c) return (c);

    Debug (fprintf (stderr, "pre-order visit '%s', depth %d\n", name, depth));

    c = NEW (struct Cell, 1);
    c -> name = strsave (name);
    c -> children = NULL;
    c -> status = 0;		/* assume circuit present */

    c -> next = cells;
    cells = c;

    c -> smtime = 0;

    if (xcontrol) {
	xdata.name = name;
	cstatus = (dmGetCellStatus (dmproject, &xdata) == 0);
	if (cstatus) c -> smtime = xdata.timestamp;
	else if (!imported) cstatus = 1;
    }
    else cstatus = 0;

    if (imported) {
	IMPCELL *icp = dmGetImpCell (name);
	project = dmOpenProject (icp -> dmpath, PROJ_READ);
	name = icp -> cellname;
	c -> depth = -1; /* never a candidate for extraction */

	/* (a) If the current project has xcontrol, the status of
	   imported cells must be specified locally for old projects,
	   else the remote project must have also xcontrol.
	   (b) If the current project does not has xcontrol, the
	   remote project can has xcontrol. (SdeG4.44)
	*/
	if (xcontrol) /* (a) Must we use remote xcontrol? */
	    mode = (!cstatus || xdata.celltype == DM_CT_IMPORT);
	else /* (b) Has remote project xcontrol? */
	    mode = (dmStatXData (project, &buf1) == 0);
	if (mode) {
	    cstatus = 1;
	    xdata.name = name;
	    (void)dmGetCellStatus (project, &xdata);
	    if (c -> smtime < xdata.timestamp) c -> smtime = xdata.timestamp;
	}
    }
    else {
	project = dmproject;
	c -> depth = 0;
    }

    versionstatus = (depth == ROOT_DEPTH ? WORKING : ACTUAL);

    /*
     * dmCheckOut read-only to increase potential concurrency
     */
    lkey = dmCheckOut (project, name, versionstatus, DONTCARE, LAYOUT, READONLY);

    if (!lkey) return (NULL);

    if (dmStat (lkey, "mc", &buf1) == 0) /* layout last modified time */
	c -> lmtime = buf1.st_mtime;
    else {
	c -> lmtime = 0;
	dmError (name);
    }

    if (cstatus) {
	switch (xdata.celltype) {
	case DM_CT_MACRO:
	    PRINT ("is macro");
	    c -> status |= ISMACRO;
	    break;
	case DM_CT_DEVICE:
	case DM_CT_LIBRARY:
	    if (xdata.interfacetype != DM_IF_STRICT) {
		PRINT ("is macro");
		c -> status |= ISMACRO;
	    }
	    if (xdata.celltype == DM_CT_DEVICE) {
		PRINT ("is device");
	    }
	    else {
		PRINT ("is library");
		c -> status |= ISLIBRARY;
	    }
	    goto set_devmod;
	}
	goto after_devmod;
    }

    if (dmStat (lkey, "is_macro", &buf1) == 0) { /* stream exist */
	if ((time_t)(c -> smtime) < buf1.st_mtime)
	    c -> smtime = buf1.st_mtime;
	dsp = dmOpenStream (lkey, "is_macro", "r");
	if (fscanf (dsp -> dmfp, "%d", &flag) > 0 && flag == 1) {
	    PRINT ("is macro");
	    c -> status |= ISMACRO;
        }
	dmCloseStream (dsp, COMPLETE);
    }

    /* We don't use paramCapitalize: cktName (name) anymore! */

    if (_dmExistCell (project, name, CIRCUIT) == 1) {
	DM_CELL *ckey;

	Debug (fprintf (stderr, "equivalent circuit '%s'\n", name));

	ckey = dmCheckOut (project, name, WORKING, DONTCARE, CIRCUIT, READONLY);
	mode = dmStat (ckey, "devmod", &buf1);
	dmCheckIn (ckey, COMPLETE);

	if (mode == 0) { /* stream exist */
	    if (buf1.st_mtime > (time_t)(c -> smtime)) /* device status changed */
		c -> smtime = buf1.st_mtime;
	    PRINT ("device model present");
set_devmod:
	    c -> status |= DEVMOD;
	    if (!optPseudoHier && !(c -> status & ISMACRO)) goto ret;
	}
    }

after_devmod:
    if (!imported && !(optFlat && optNoPrepro)) {
	/* In hierarchical mode, recursively visit the children
	 * to see whether they must be extracted.
	 * In flat mode, recursively visit the children to see
	 * if the root cell should be pre-processed.
	 */
	if ((dsp = dmOpenStream (lkey, "mc", "r")) == NULL) goto ret;

	while (dmGetDesignData (dsp, GEO_MC) > 0) {

	    name = gmc.cell_name;

	    a = c -> children;
	    while (a && strcmp (a -> cell -> name, name)) a = a -> next;
	    if (!a) { /* add new child */
		a = NEW (struct adj, 1);
		a -> next = c -> children;
		c -> children = a;
		a -> cell = makeTree (name, depth + 1, gmc.imported); /* recursive call */
		if (optFlat || optPseudoHier) {
		    if (a -> cell -> lmtime > c -> lmtime)
			c -> lmtime = a -> cell -> lmtime;
		    if (a -> cell -> smtime > c -> lmtime)
			c -> lmtime = a -> cell -> smtime;
		}
	    }
        }
        dmCloseStream (dsp, COMPLETE);
    }

ret:
    dmCheckIn (lkey, COMPLETE);

    if (!imported) {
	c -> candidates = candidates;
	candidates = c;
    }
    Debug (fprintf (stderr, "post-order visit '%s', depth %d\n", c -> name, depth));
    return (c);
}

/* Set the last modified time correctly for hierarchical mode.
 */
Private unsigned long traverse (struct Cell *c, unsigned long ct, int flat)
{
    struct adj *a;
    for (a = c -> children; a; a = a -> next) {
	c = a -> cell;
	if (c -> lmtime > ct) ct = c -> lmtime;
	if (c -> smtime > ct) ct = c -> smtime;
	if (flat || c -> status & ISMACRO) ct = traverse (c, ct, flat);
    }
    return ct;
}

/* Make an array of do_t's that must be extracted:
 * OptMaxDepth is for debugging only, it can deliver a non-consistent tree.
 */
Private do_t * makeCandidateArray ()
{
    static do_t *list;
    static int size = 0;
    struct adj  *a;
    struct Cell *c, *f;
    struct stat buf1;
    do_t *l;
    unsigned long ct;
    int num;

    if (!(c = root_c)) return (NULL);

    /* Make candidates fringe list and
     * count the number of cells to be extracted.
     */
    c -> depth = ROOT_DEPTH; /* = 1 */
    num = 1;

    if (optFlat) {
	Debug (fprintf (stderr, "candidate '%s', depth %d, status %d\n", c -> name, c -> depth, (int) c -> status));
	if (c -> status & DEVMOD) {
	    PRINT ("is device");
	}
	else
	    c -> status |= DO_SPACE;
    }
    else for (f = c;; c = c -> fringe) {
	Debug (fprintf (stderr, "candidate '%s', depth %d, status %d\n", c -> name, c -> depth, (int) c -> status));

	if (c -> status & DEVMOD) {
	    PRINT ("is device");
	}
	else if (c -> status & ISMACRO && c != root_c) {
	    PRINT ("is macro");
	}
	else if (c -> depth > optMinDepth) { /* Extract only if needed! */
	    DM_CELL *ckey = NULL;
	    int ok = 1;

	    if (c -> status >= DONE_SPACE) goto needed;

	    if (!(ckey = existCircuit (c -> name)) || dmStat (ckey, "mc", &buf1)) {
		PRINT ("no equivalent circuit");
		ok = 0;
		goto needed;
	    }

	    ct = c -> lmtime;

	    if (existDmStream (ckey, "flat")) {
		PRINT ("previous extraction was flat");
		if (!optPseudoHier) ct = traverse (c, ct, 1);
		ok = DONE_FLAT;
	    }
	    else if (existDmStream (ckey, "pseudo_hier")) {
		PRINT ("previous extraction was pseudo-hier");
		if (!optPseudoHier) ct = traverse (c, ct, 1);
		ok = DONE_PSEUDO;
	    }
	    else {
		PRINT ("previous extraction was hier?");
		if (!optPseudoHier) ct = traverse (c, ct, 0);
	    }

	    if ((time_t)ct >= buf1.st_mtime) {
		PRINT ("layout is newer");
		ok = 0;
	    }
needed:
	    if (ckey) dmCheckIn (ckey, COMPLETE);

	    if (!ok) {
		c -> status |= DO_SPACE;
		PRINT ("extraction needed!");
	    }
	    else {
		PRINT ("already correct extracted");
		if (ok >= DONE_FLAT) {
		    c -> status |= ok;
		    if (ok == DONE_FLAT) goto skip; /* children */
		}
	    }
	}
	else {
	    c -> status |= DO_SPACE;
	    PRINT ("extract: depth <= min_depth");
	}

	if (c -> depth >= optMaxDepth) {
	    PRINT ("children not extracted: depth >= max_depth");
	    goto skip;
	}
	for (a = c -> children; a; a = a -> next) {
	    if (a -> cell -> depth == 0) {
		f -> fringe = a -> cell;
		f = a -> cell;
		f -> depth = c -> depth + 1;
		++num;
	    }
	}
skip:
	if (c == f) break;
    }

    /* make sure the list is long enough */
    if (num + 1 > size) {
	if (size > 0) DISPOSE (list, sizeof(do_t) * size);
	size = num + 1;
	list = NEW (do_t, size);
    }

    /* put marker in first position */
    l = list + num;
    l -> name = NULL;

    /* By flat extraction, there is only 1 candidate and
     * this candidate is always extracted (except when it is a device),
     * because c -> depth == 1 and optMinDepth == 1.
     *
     * The candidates are returned in reverse order,
     * the highest level subcells are done first, the topcell last.
     */
    if (num > 1) c = candidates;
    for (; c; c = c -> candidates) {
	if (c -> depth <= 0) continue;

	--num;
	--l;
	l -> status = c -> status;
	l -> name = c -> name;
	Debug (fprintf (stderr, "candidate '%s', depth %d, status %d\n", c -> name, c -> depth, (int) c -> status));
	c -> depth = 0;

	if (!(c -> status & DO_SPACE)) {
	    if (num == 0) break;
	    continue;
	}
	c -> status &= ~DO_SPACE;

	if (l -> status & ISMACRO) l -> status &= ~ISMACRO;

	l -> lkey = dmCheckOut (dmproject, c -> name, WORKING, DONTCARE, LAYOUT, ATTACH);

	if (c -> status & DONE_SPACE) {
	    PRINT ("done before: no preprocessing!");
	}
	else {
	    c -> status |= DONE_SPACE;

	if (!optNoPrepro) { /* preprocessing needed? */
	    DM_CELL *lkey = l -> lkey;
	    int ok = 0;

	    if (dmStat (lkey, gln_file, &buf1)) {
		PRINT ("no gln file, no previous exp");
		goto not_ok;
	    }

	    if (!(optFlat || optPseudoHier))
		ct = traverse (c, c -> lmtime, 0);
	    else
		ct = c -> lmtime;

	    if ((time_t)ct > buf1.st_mtime) {
		PRINT ("previous exp was old");
	    }
	    else if (existDmStream (lkey, "spec")) {
		PRINT ("previous exp was flat");
		ok = optFlat;
	    }
	    else if (existDmStream (lkey, "pseudo_hier")) {
		PRINT ("previous exp was pseudo-hier");
		ok = (!optFlat && optPseudoHier);
	    }
	    else {
		PRINT ("previous exp was hier?");
		ok = (!optFlat && !optPseudoHier);
	    }
not_ok:
	    if (!ok) {
		PRINT ("previous exp not ok, do exp!");
		l -> status |= DO_EXP;
		if (nrOfResizes > 0) l -> status |= DO_RESIZE;
	    }
	    else if (existDmStream (lkey, "resize.t")) { /* exists */
		if (nrOfResizes > 0) {
		    if (!resizeUpToDate (lkey)) {
			PRINT ("previous resize not ok, do it!");
			l -> status |= (DO_EXP | DO_RESIZE);
		    }
		}
		else {
		    PRINT ("previous resize, do exp!");
		    l -> status |= DO_EXP;
		}
	    }
	    else if (nrOfResizes > 0) {
		PRINT ("previous exp ok, but do resize!");
		l -> status |= DO_RESIZE;
	    }
	    else
		PRINT ("previous exp ok!");
	}
	}
	if (num == 0) break;
    }

    ASSERT (l == list);

    /* The list with 'struct Cell's is not destroyed since these structures
       may be used for the extraction of other cells (if more than one cell
       is specified on the argument list or with Xspace).
    */

    return (list);
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
    return 0;
}
#endif

/* Get imported celllist structure for imported cell (alias) name.
 * Don't read the imported celllist more than once.
 */
Private IMPCELL * dmGetImpCell (char *name)
{
    register IMPCELL **icl;
    int i = _dmValidView (LAYOUT);
    if (i < 0 || !(icl = dmproject -> impcelllist[i]))
	icl = (IMPCELL **) dmGetMetaDesignData (IMPORTEDCELLLIST, dmproject, LAYOUT);
    if (icl)
    while (*icl) {
	if (strsame ((*icl) -> alias, name)) return *icl;
	++icl;
    }
    dmerrno = DME_NOCELL;
    dmError (name);
    return 0;
}

/* Look if an equivalent extracted circuit exists for the layout cell name.
 * If the circuit cell exists, check it out for usage.
 * Note that the layout cell name can't be an imported cell name.
 */
Private DM_CELL * existCircuit (char *name)
{
    DM_CELL *ckey = NULL;
    char c = *name;
    if (paramCapitalize) *name = toupper (c);
    if (_dmExistCell (dmproject, name, CIRCUIT) == 1)
	ckey = dmCheckOut (dmproject, name, WORKING, DONTCARE, CIRCUIT, READONLY);
    if (paramCapitalize) *name = c;
    return ckey;
}

#ifdef DRIVER

int optMaxDepth = INF;
int optMinDepth = ROOT_DEPTH; /* = 1 */
bool_t optCap3D = FALSE;
bool_t optPrick = FALSE;
bool_t optNoPrepro = FALSE;
bool_t optPseudoHier = FALSE;
bool_t optResMesh = FALSE;
bool_t optFlat = FALSE;
bool_t paramCapitalize = FALSE;
bool_t substrRes = FALSE;
int nrOfResizes = 0;

#define PR fprintf (stdout,

main (int argc, char **argv)
{
    DM_PROCDATA * procdata;
    do_t *l;
    int c;
    char * cellname;

    while ((c = getopt (argc, argv, "FIL:D:PT")) != EOF) {
	switch (c) {
	    case 'F': optFlat = TRUE; break;
	    case 'P': optPseudoHier = TRUE; break;
	    case 'D': optMinDepth = atoi (optarg); break;
	    case 'L': optMaxDepth = atoi (optarg); break;
	    case 'I': optMinDepth = INF; break;
	    case 'T': optMaxDepth = 1;   break;
	    case '?': break;
	}
    }

    if (optFlat) {
	if (optPseudoHier)
	    PR "Options -P is ignored with -F\n");
	if (optMinDepth != ROOT_DEPTH)
	    PR "Options -D and -I are ignored with -F\n");
	if (optMaxDepth != INF)
	    PR "Options -L and -T are ignored with -F\n");
	optMinDepth = ROOT_DEPTH;
    }

    dmInit ("test");
    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);
    procdata = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, dmproject);
    gln_file = strsave (mprintf ("%s_gln", procdata -> mask_name[0]));

    while (optind < argc) {
	cellname = argv[optind++];
	l = findCandidates (dmproject, cellname);

	if (optFlat)
	    PR "extract flat %s\n", cellname);
	else
	    PR "extract hierarchy of %s\n", cellname);
	if (l) while (l -> name) {
	    PR "%s, status %d\n", l -> name, (int) l -> status);
	    if (l -> status & ISMACRO)
		PR "%s skipped, has macro status\n", l -> name);
	    else if (l -> status & DEVMOD)
		PR "%s skipped, device model present\n", l -> name);
	    else if (l -> status & DO_SPACE) {
		if (l -> status & DO_EXP)
		    PR "%s expand and extract!\n", l -> name);
		else
		    PR "%s extract!\n", l -> name);
		dmCheckIn (l -> lkey, COMPLETE);
	    }
	    else {
		char *t = "";
		if (l -> status & DONE_FLAT) t = " flat";
		else if (l -> status & DONE_PSEUDO) {
		    if (!optPseudoHier) t = " pseudo";
		}
		else {
		    if (optPseudoHier) t = " !pseudo";
		}
		PR "%s already%s extracted\n", l -> name, t);
	    }
	    l++;
	}
    }
    dmCloseProject (dmproject, QUIT);
}

void dmError (char *s)
{
    dmPerror (s);
    dmQuit ();
    exit (1);
}

#endif /* DRIVER */
