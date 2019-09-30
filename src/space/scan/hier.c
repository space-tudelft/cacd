/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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
#include <stdlib.h>
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/scan.h"
#include "src/space/scan/extern.h"
#include "src/space/lump/export.h"

terminal_t **TERM = NULL;
terminal_t **TERMBYNAME = NULL;
char **LABELNAME = NULL;

int nrOfTerminals = 0;
int nrOfTermNames = 0;
int nrOfLabelNames = 0;
#ifndef CONFIG_SPACE2
int maxTermNumber = 0;
#endif

char * instArray[40];
char * instNames = NULL;
static int instArrayCnt = 0;
static int instNameSize = 0;
static int freeNameSize = 0;

#ifdef __cplusplus
  extern "C" {
#endif

/* local operations */
Private void addInstance (DM_STREAM *stream, char *cellName, char *instName, int nx, int ny);
Private void addTerminal (DM_STREAM *stream, char *termName, int nx, int ny);
Private void newLabelName (char *name);

#ifdef __cplusplus
  }
#endif

/*  Read the 'tid' file.

    The 'tid' files assigns a number to each terminal
    of the current cell and its first-level subcells.
    The t_mask_bxx files then contain the terminal coordinates.

    This routine handles the hierarchy,
    i.e. it writes the 'term' and 'mc' streams in the
    circuit view.
*/
void readTid (DM_CELL *layoutKey, DM_CELL *circuitKey)
{
    static long lower[2], upper[2]; /* initial zero */
    int i_nx, i_ny, t_nx, t_ny;
    int instX, instY, termX, termY;
    int instNumber = 0;
    coor_t xl, yb, xr, yt, dx, dy;
    int len;
    char attr_buf[32];
    char fullInstName[1024];
    char buf[1024];
    char help_name[1024];
    char *labelName, *termName, *cellName, *instName;
    int noRealInstName = 0;
    DM_STREAM *tidStream, *tidnamStream, *tidposStream,
		*termStream, *flagStream;
    extern DM_STREAM *dmsMc; /* from lump/out.c */

    instArrayCnt = instNameSize = freeNameSize = 0;
    nrOfTerminals = 0;
    nrOfTermNames = 0;
    nrOfLabelNames = 0;
#ifndef CONFIG_SPACE2
    maxTermNumber = 0;
#endif

    tidStream = dmOpenStream (layoutKey, "tid", "r");
    if (tidStream == NULL) {
	say ("cell '%s' not expandend, run exp -g first", layoutKey -> cell);
	die ();
    }

    tidnamStream = NULL;
    tidposStream = NULL;
    termStream = NULL;

    if (extrPass) {
	cterm.term_attribute = NULL;
	cterm.term_lower = lower;
	cterm.term_upper = upper;
	cmc.inst_attribute = NULL;
	cmc.inst_lower = lower;
	cmc.inst_upper = upper;

	if (optTorPos && existDmStream (layoutKey, "tidpos")) {
	    tidposStream = dmOpenStream (layoutKey, "tidpos", "r");
	    if (tidposStream) cmc.inst_attribute = attr_buf;
	}
	termStream = dmOpenStream (circuitKey, "term", "w");
    }

    if (existDmStream (layoutKey, "tidnam"))
	tidnamStream = dmOpenStream (layoutKey, "tidnam", "r");

    if (optFlat) {
	flagStream = dmOpenStream (circuitKey, "flat", "w");
	dmCloseStream (flagStream, COMPLETE);
    }
    else if (existDmStream (circuitKey, "flat")) {
	dmUnlink (circuitKey, "flat");
    }

    if (optPseudoHier) {
	flagStream = dmOpenStream (circuitKey, "pseudo_hier", "w");
	dmCloseStream (flagStream, COMPLETE);
    }
    else if (existDmStream (circuitKey, "pseudo_hier")) {
	dmUnlink (circuitKey, "pseudo_hier");
    }

    i_nx = i_ny = 0; /* init for compiler warning */
    cellName = instName = NULL; /* init for compiler warning */

    while (dmGetDesignData (tidStream, GEO_TID) > 0) {
	if (gtid.term_offset == -1) {
	    noRealInstName = 0;
	    cellName = gtid.cell_name;
	    if (strsame (gtid.inst_name, "$"))
		instName = NULL;
	    else if (strsame (gtid.inst_name, ".")) {
                if (tidnamStream
                    && dmScanf (tidnamStream, "%s\n", fullInstName) > 0
                    && strcmp (fullInstName, ".")) {
                    strcpy (buf, fullInstName);
		    convertHierName (buf);
		    instName = truncDmName (buf);
                }
                else {
		    instName = mprintf ("_I%d", instNumber++);
		    strcpy (fullInstName, instName);
                    noRealInstName = 1;
                }
            }
	    else {
		instName = gtid.inst_name;
                strcpy (fullInstName, instName);
            }
	    i_nx = (int) gtid.m_nx;
	    i_ny = (int) gtid.m_ny;

	    if (instName) {
		/* To be able to clean up,
		 * remember the names of the instances
		 */
		len = strlen (instName) + 1;
		if (len > freeNameSize) {
		    if (instArrayCnt == 40) { say ("too many instance names"); die (); }
		    if (!instNameSize)
			instNameSize = 4096 - 8;
		    else
			instNameSize += 4096;
		    instArray[instArrayCnt] = NEW (char, instNameSize);
		    instNames = instArray[instArrayCnt++];
		    freeNameSize = instNameSize;
		}
		instName = strcpy (instNames, instName);
		instNames += len; freeNameSize -= len;

		if (!nrOfTermNames) nrOfTermNames = nrOfTerminals;

		if (tidposStream) {
		    if (fscanf (tidposStream -> dmfp, "%d %d %d %d %d %d", &xl, &yb, &xr, &yt, &dx, &dy) < 6) {
			say ("stream 'tidpos' read error"); die ();
		    }
		    sprintf (attr_buf, "x=%s;y=%s", strCoor (inScale * xl), strCoor (inScale * yb));
#ifndef CONFIG_SPACE2
		    if (optBackInfo > 1)
			backInfoInstance (instName, inScale * dx, i_nx, inScale * dy, i_ny,
				inScale * xl, inScale * yb, inScale * xr, inScale * yt);
#endif
		}

		if (dmsMc) addInstance (dmsMc, cellName, instName, i_nx, i_ny);
	    }
	}
	else {
	    ASSERT (nrOfTerminals == gtid.term_offset);
	    t_nx = (int) gtid.t_nx;
	    t_ny = (int) gtid.t_ny;
	    termName = strsave (gtid.term_name);

	    for (instX = 0; instX <= i_nx; instX++) {
	     for (instY = 0; instY <= i_ny; instY++) {
	      for (termX = 0; termX <= t_nx; termX++) {
	       for (termY = 0; termY <= t_ny; termY++) {
		   /* INF to cover erroneous tech file */
		   newTerminal (tTerminal, -1, INF, INF, termName, instName,
			(i_nx > 0 ? instX : -1), (i_ny > 0 ? instY : -1),
			(t_nx > 0 ? termX : -1), (t_ny > 0 ? termY : -1));
                   /* N.B. Later, in openInput(), we read the 'term' stream
                           and add conductor no. and x,y values.
                           After that, terminals for which no conductor layer
                           has been defined, will then be sorted towards the
			   end of the terminal list.
                   */

                    /* We now already begin to construct the label names
                       since later on we do not have the original instance
                       names and cell names. */
                    if (useLeafTerminals) {
			if (instName) {
			    if (useCellNames || noRealInstName) {
				sprintf (help_name, " %s_%s%s",
                                         cellName, termName, makeIndex (t_nx, t_ny, termX, termY));
				/* This name is not yet ready since x and y are missing. */
			    }
			    else {
				sprintf (help_name, "%s%s", fullInstName,
					makeIndex (i_nx, i_ny, instX, instY));
				convertHierName (help_name);
				sprintf (help_name + strlen (help_name), "%s%s%s",
				    inst_term_sep, termName, makeIndex (t_nx, t_ny, termX, termY));
			    }
			    labelName = strsave (help_name);
			    newLabelName (labelName);
                        }
                        else {
                            /* Later we will access label names based on the
                               terminal number (check type).  So the labels
                               must have a similar index as the terminals
                               and so for each terminal we must have an entry
                               in LABELNAME[].
                            */
			    newLabelName (NULL);
                        }
                    }
	       }
	      }
	     }
	    }

	    if (!instName && termStream)
		addTerminal (termStream, termName, t_nx, t_ny);
	}
    }

    if (nrOfTermNames) /* there are instances */
	nrOfTermNames = nrOfTerminals - nrOfTermNames; /* instance terminals! */

    ASSERT (!useLeafTerminals || nrOfTerminals == nrOfLabelNames);

    dmCloseStream (tidStream, COMPLETE);
    if (tidposStream) dmCloseStream (tidposStream, COMPLETE);
    if (tidnamStream) dmCloseStream (tidnamStream, COMPLETE);
    if (termStream)   dmCloseStream (termStream, COMPLETE);

    /* for all other instances */
    cmc.imported = LOCAL;
    cmc.inst_dim = 0;
}

/* Add a new instance to the circuit
*/
Private void addInstance (DM_STREAM *stream, char *cellName, char *instName, int nx, int ny)
{
    DM_PROJECT * dmProject = stream -> dmkey -> dmproject;

    strcpy (cmc.cell_name, cellName);
    strcpy (cmc.inst_name, instName);

    /* Check if the cell is local or imported and set the
     * 'imported' field in the mc record.
     * If the cell is imported, we don't know wether the cell
     * is also imported in the circuit view.
     * The designer should eventually import the cell himself
     */
    if (_dmExistCell (dmProject, cellName, LAYOUT) == 0)
	cmc.imported = IMPORTED;
    else
	cmc.imported = LOCAL;

    if (paramCapitalize && cmc.imported == LOCAL
        && cmc.cell_name[0] >= 'a' && cmc.cell_name[0] <= 'z')
        cmc.cell_name[0] += 'A' - 'a';

    cmc.inst_dim = 0;
    if (nx > 0) cmc.inst_upper[cmc.inst_dim++] = nx;
    if (ny > 0) cmc.inst_upper[cmc.inst_dim++] = ny;

    dmPutDesignData (stream, CIR_MC);
}

/* Add a new terminal to the circuit
*/
Private void addTerminal (DM_STREAM *stream, char *termName, int nx, int ny)
{
    strcpy (cterm.term_name, termName);

    cterm.term_dim = 0;
    if (nx > 0) cterm.term_upper[cterm.term_dim++] = nx;
    if (ny > 0) cterm.term_upper[cterm.term_dim++] = ny;

    dmPutDesignData (stream, CIR_TERM);
}

void disposeTid ()
{
    terminal_t *t;
    register int i;

    for (i = 0; i < nrOfTerminals; ++i) {
	t = TERM[i];
	if (t -> instX <= 0 && t -> instY <= 0
	 && t -> termX <= 0 && t -> termY <= 0) {
	    DISPOSE (t -> termName, strlen(t -> termName) + 1);
	}
	DISPOSE (t, sizeof(terminal_t));
    }

    while (--instArrayCnt > 0) DISPOSE (instArray[instArrayCnt], 0);
    if (instArrayCnt == 0) DISPOSE (instArray[0], instNameSize);
}

void newTerminal (termtype_t type, int conductor, coor_t x, coor_t y,
	char *tName, char *iName, int ix, int iy, int tx, int ty)
{
    static int TERMSize = 0;
    terminal_t *t;
    int i;

    if (nrOfTerminals >= TERMSize) {
	int old_size = TERMSize;
	TERMSize = !TERMSize ? 1000 : 2 * TERMSize;
	TERM = RESIZE (TERM, terminal_t *, TERMSize, old_size);
    }
    TERM[nrOfTerminals++] = t = NEW (terminal_t, 1);
    t -> next = NULL;

    t -> type = type;
    t -> conductor = conductor;
    t -> x = x;
    t -> y = y;
    t -> termName = tName;
    t -> instName = iName;
    t -> instX = ix;
    t -> instY = iy;
    t -> termX = tx;
    t -> termY = ty;

    if (!iName && tx < 0 && ty < 0) {
        for (i = strlen (tName) - 1; i >= 0; i--) {
            if (tName[i] < '0' || tName[i] > '9') break;
        }
        if (i < 0) {
#ifndef CONFIG_SPACE2
	    int nr = atoi (tName);
            if (nr > maxTermNumber) maxTermNumber = nr;
#else
	    fprintf (stderr, "warning: newTerminal '%s' is nr\n", tName);
#endif
        }
    }
}

Private void newLabelName (char *name)
{
    static int LABELNAMESize = 0;

    if (nrOfLabelNames >= LABELNAMESize) {
	int old_size = LABELNAMESize;
	LABELNAMESize = !LABELNAMESize ? 1000 : 2 * LABELNAMESize;
	LABELNAME = RESIZE (LABELNAME, char *, LABELNAMESize, old_size);
    }
    LABELNAME[nrOfLabelNames++] = name;
}

void addTerminalNames ()
{
    static int TERMBYNAMESize = 0;
    int i;

    if (nrOfTerminals > TERMBYNAMESize) {
	if (TERMBYNAME) DISPOSE (TERMBYNAME, sizeof(terminal_t *) * TERMBYNAMESize);
	TERMBYNAME = NEW (terminal_t *, nrOfTerminals);
	TERMBYNAMESize = nrOfTerminals;
    }

    for (i = 0; i < nrOfTerminals; ++i) TERMBYNAME[i] = TERM[i];
}

int findTerminal (char *termName)
{
    terminal_t **base, **last, **p;
    int v;

    if (!(base = TERMBYNAME)) return 0; /* no terminals */
    last = TERMBYNAME + nrOfTermNames - 1;

    while (last >= base) {

	p = base + ((last - base)/2);

	if (!(v = strcmp (termName, (*p) -> termName))) return 1; /* found */
	if (v < 0) last = p - 1;
	else       base = p + 1;
    }
    return 0; /* not found */
}

int compareByName (const void *e1, const void *e2) /* compare two terminals by name */
{
    terminal_t ** t1 = (terminal_t **) e1;
    terminal_t ** t2 = (terminal_t **) e2;
    char *n1, *n2;
    int v;

    n1 = (*t1) -> instName;
    n2 = (*t2) -> instName;

    if (n1 != n2) {
	if (!n1) return -1;
	if (!n2) return  1;
	if ((v = strcmp (n1, n2)) != 0) return v;
    }
    return strcmp ((*t1) -> termName, (*t2) -> termName);
}
