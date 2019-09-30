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

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/include/version.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"

#include "src/space/makesize/rscan.h"
#include "src/space/makesize/extern.h"

#define MajorProtNr  1
#define MinorProtNr  1

/* local operations */
Private void docell (char *cellname, char *version);
Private void resize (int resizeIndex);

DM_CELL    * layoutKey;
DM_PROJECT * dmproject;

coor_t Inf = INF-3;
coor_t InfXl, InfXr;
coor_t bxl, byb, bxr, byt;
int    scale = 1;
double meters;    	/* this many meters per internal layout unit */

edge_t *shrinkEdge[2];	/* edges used for skrinking a part of the layout */
int shrinkEdgeIndex;
int newMask;
int resizeIndex;
int secondTime = 0;
mask_t idcolor;
int    growPos;
coor_t growSize;

bool_t optVerbose  = FALSE;
bool_t optScale = FALSE;
bool_t optDebug = FALSE;
bool_t optPS = FALSE;

/* Added because the linker was complaining.
 * These options migth be used in resize as well
 */
bool_t optPrintRecog = FALSE;
/* These options migth have to be removed
 */
bool_t optNoPrepro = FALSE;
bool_t optCompress = TRUE;
bool_t optInfo     = FALSE;
bool_t optSortOnly = FALSE;
bool_t optNoOutput = FALSE;
bool_t optSpecial  = FALSE;
char ** cellNames  = NULL;

size_t optMaxMemory = 1024*1024;	/* Default 1 Mbyte */

static char * currentCell = NULL;

extern char *optarg;
extern int optind;

int main (int argc, char *argv[])
{
    int     c, errflg = 0;
    double     maxMem = 0.0;	/* megabytes of sort buffer memory */
    char  * version = WORKING;	/* default version */
    char  * optstring;
    char  * techFile = NULL;

    argv0 = "makesize";

    catchSignals ();

    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == '%')
	optstring = "%4dpiSNE:v";
    else
	optstring = "E:vcm:";

    while ((c = getopt (argc, argv, optstring)) != EOF) {
	switch (c) {
	    case 'i': optInfo     = TRUE;    /* implies verbose */
	    case 'v': optVerbose  = TRUE;    break;
	    case '4': optScale    = TRUE;    break;
	    case 'd': optDebug    = TRUE;    break;
	    case 'p': optPS       = TRUE;    break;
	    case 'c': optCompress = FALSE;   break;
            case 'E': techFile   = strsave (optarg); break;
	    case 'm': maxMem = atof (optarg);break;
	    case 'S': optSortOnly = TRUE;    break;
	    case 'N': optNoOutput = TRUE;    break;
	    case '%': optSpecial  = TRUE;    break;
	    case '?': errflg++;              break;
	}
    }
    cellNames = argv + optind;

    if (!*cellNames) {
	say ("No cellname(s) specified\n");
	errflg++;
    }

    if (maxMem < 0) {
	say ("Max memory must be > 0");
	errflg++;
    }

    if (errflg) {
	printf ("Usage: %s %s[-vc] [-E file] [-m maxmem] cell [...]\n",
	    argv0, optSpecial ? "[-%4dpiSN] " : "");
	exit (1);
    }

    if (maxMem > 0) { optMaxMemory = (size_t) (1024 * 1024 * maxMem); }

    if (optVerbose) {
	verboseSetMode (TRUE);
	message ("%s ", argv0);
	for (c = 1; c < argc; c++) message ("%s ", argv[c]);
	message ("\n");

	message ("-v: verbose mode\n");
	if (optDebug)    message ("-d: debug output\n");
	if (optScale)    message ("-4: dividing debug output by scale factor\n");
	if (optInfo)     message ("-i: print statistics, implies -v\n");
	if (optSortOnly) message ("-S: sort edge only, no overlap removal\n");
	if (optNoOutput) message ("-N: no database gln file output\n");
	if (!optCompress) message ("-c: do not compress tmp file\n");

	if (maxMem > 0) {
	    message ("-m %g: %ld bytes of sort buffer core memory\n", maxMem, (long) optMaxMemory);
	}
    }

    if (optInfo) clockInit ();

    dmInit (argv0);

    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    getTechnology (dmproject, NULL, techFile);
    if (optVerbose) {
	message ("technology file: %s\n", giveICD (usedTechFile));
    }

    while (*cellNames) docell (*cellNames++, version);

    dmCloseProject (dmproject, COMPLETE);

    dmQuit ();

    if (optInfo) {
	clockPrintAll (stdout);
	clockPrintTime (stdout);
	scanPrintInfo (stdout);
	sortPrintInfo (stdout);
	outputPrintInfo (stdout);
    }
    else /* close the sort module */
	sortPrintInfo  (NULL);

    verbose ("%s --- Finished ---\n", argv0);
    return (0);
}

Private void readBoundingBox ()
{
    DM_STREAM * info3 = dmOpenStream (layoutKey, "info3", "r");
    dmGetDesignData (info3, GEO_INFO3);
    dmCloseStream (info3, COMPLETE);

    scale = SCALE;
    if (ginfo3.nr_samples != 0) scale = 1;

    bxl = (coor_t) (scale * ginfo3.bxl);
    bxr = (coor_t) (scale * ginfo3.bxr);
    byb = (coor_t) (scale * ginfo3.byb);
    byt = (coor_t) (scale * ginfo3.byt);
}

Private void writeBoundingBox ()
{
    DM_STREAM * info3 = dmOpenStream (layoutKey, "info3", "w");

    ginfo3.bxl = (long) floor (((double)bxl / scale));
    ginfo3.bxr = (long) ceil  (((double)bxr / scale));
    ginfo3.byb = (long) floor (((double)byb / scale));
    ginfo3.byt = (long) ceil  (((double)byt / scale));

    dmPutDesignData (info3, GEO_INFO3);
    dmCloseStream (info3, COMPLETE);
}

Private void docell (char *cellname, char *version) /* do a cell */
{
    int i, j;
    resizeCond_t *resCond;
    DM_STREAM *dsp_rs;

    tick (mprintf ("%s %s", argv0, cellname));

    verbose ("cell %s, version %s\n", cellname, version);

    layoutKey = dmCheckOut (dmproject, cellname, version, DONTCARE, LAYOUT, ATTACH);

    meters = layoutKey -> dmproject -> lambda * 1e-6;

    currentCell = cellname;

    for (i = 0; i < nrOfResizes; i++) resize (i);

    /* Write resize information in a stream "resize.t".
       This information will later be used to decide whether or
       not it is necessary to run makesize again. */
    if ((dsp_rs = dmOpenStream (layoutKey, "resize.t", "w"))) {
        dmPrintf (dsp_rs, "%d %d\n", MajorProtNr, MinorProtNr);
        dmPrintf (dsp_rs, "%d\n", nrOfResizes);
        for (i = 0; i < nrOfResizes; i++) {
            dmPrintf (dsp_rs, "%s %d %lg %d ",
                      resizes[i].newmaskname, resizes[i].id,
                      resizes[i].val, resizes[i].condcnt);
            resCond = resizes[i].cond;
            for (j = 0; j < resizes[i].condcnt; j++) {
                dmPrintf (dsp_rs, " %s", colorIntStr (&resCond -> present));
                dmPrintf (dsp_rs, " %s", colorIntStr (&resCond -> absent));
                resCond = resCond -> next;
            }
            dmPrintf (dsp_rs, "\n");
        }
        dmCloseStream (dsp_rs, COMPLETE);
    }

    currentCell = NULL;
    dmCheckIn (layoutKey, COMPLETE);

    tock (mprintf ("%s %s", argv0, cellname));
}

Private void resize (int i)
{
    coor_t Byb, Byt;
    mask_t color;
    resizeCond_t * resizeCond;
    char newname[255];
    int negativeMask;
    double grow_val = resizes[i].val;

    resizeIndex = i;
    verbose ("growing mask '%s' with %le\n", resizes[i].newmaskname, grow_val);

    readBoundingBox ();

    if (grow_val < 0) {
	grow_val = -grow_val;
	growPos = 0;
    }
    else
	growPos = 1;

    growSize = (coor_t) ((grow_val / meters) + 0.5) * 4;

    if (optDebug) initExtract (currentCell, scale); /* create debug files */

    COLORINITINDEX (idcolor, resizes[resizeIndex].id);

    negativeMask = 0;

    /* color determines which gln streams are opened */
    color = cNull;
    resizeCond = resizes[resizeIndex].cond;
    while (resizeCond) {/* for every or-expression ...*/
			/* put on masks that must be opened and checked */
	COLOR_ADD (color, resizeCond -> present);
	COLOR_ADD (color, resizeCond -> absent);
	resizeCond = resizeCond -> next;
    }

    newMask = COLOR_ABSENT (&color, &idcolor);

l1:
    /* create special edges for inverted layout masks */
    if (growSize < 0) {
	InfXl = bxl + 2 * growSize;
	InfXr = bxr - 2 * growSize;
    }
    else if (growSize > 0) {
	InfXl = bxl - 2 * growSize;
	InfXr = bxr + 2 * growSize;
    }
    else {
	InfXl = bxl - 20; /* SdeG */
	InfXr = bxr + 20; /* SdeG */
    }
    NEW_EDGE (shrinkEdge[0], InfXl, -Inf, InfXr, -Inf, cNull);
    NEW_EDGE (shrinkEdge[1], InfXl, +Inf, InfXr, +Inf, cNull);
    shrinkEdgeIndex = 0;

    openInput (layoutKey, &color);
    if (!secondTime) {
	resizeCond = resizes[resizeIndex].cond;
	while (resizeCond) {
	    if (COLOR_PRESENT (&shrinkEdge[0]->color, &resizeCond->present) &&
		COLOR_ABSENT  (&shrinkEdge[0]->color, &resizeCond->absent)) {
		negativeMask = 1;
		break;
	    }
	    resizeCond = resizeCond -> next;
	}
    }
    rscan ();      /* perform grow */
    closeInput (); /* new edges are generated and sorted */

    if (optDebug) endExtract (); /* close debug files */

    /* remove overlap, use last part of makegln,
     * the edges that were generated above, are now used as
     * input for the scan.
     * creates makesize_gln file in layout directory
     */
    Byb = byb;
    Byt = byt;
    openOutput (layoutKey, "makesize");
    byb = -Inf;
    byt = +Inf;
    if (negativeMask) {
	edge_t edge;
	edge.xl = -INF;
	edge.xr =  INF;
	edge.yl = edge.yr = -INF;
	outputEdge (&edge);
	edge.yl = edge.yr =  INF;
	outputEdge (&edge);
    }
    scan ();
    closeOutput ();
    byb = Byb;
    byt = Byt;

    if (neggrow_list) {
	secondTime = 1;
	goto l1;
    }
    secondTime = 0;

    /* renames the makesize_gln file to the desired name */
    strcpy (newname, resizes[resizeIndex].newmaskname);
    strcat (newname, "_gln");
    dmRenameStream (layoutKey, "makesize_gln", newname);

    /* Only write bounding box if layout has been grown.
     * If layout has been shrunk, the bounding box is
     * larger than it was, but that's because of the
     * two auxiliary edges for layout inversion.
     * These edges are not part of the output so the
     * bounding box need not to be updated
     */
    if ((growPos && !negativeMask) || (!growPos && negativeMask)) writeBoundingBox ();
}

void dmError (char *s) /* ddm error handler */
{
    dmPerror (s);
    die ();
}

void die () /* clean-up and stop */
{
    static int recursive = 0;

    if (recursive++ > 0) {
	fprintf (stderr, "Emergency exit\n");
	exit (1);
    }

    outputCleanUp ();

    dmQuit ();

    if (optInfo) {
	clockPrintAll  (stdout);
	clockPrintTime (stdout);
	scanPrintInfo  (stdout);
	sortPrintInfo  (stdout);
	outputPrintInfo (stdout);
    }
    else /* close the sort module */
	sortPrintInfo  (NULL);

    exit (1);
}

void extraSay ()
{
}
