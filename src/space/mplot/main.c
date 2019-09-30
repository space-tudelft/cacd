/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Nick van der Meijs
 *	Arjan van Genderen
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

#include "src/space/mplot/config.h"

static void docell (char *cellname, char *version);
static void domask (char *maskname, int color);
static void doterm (char *maskname, int maskno, int color);
static void expand (char *cellname);
static void run    (char *command, char *arg);
static bool_t existDmStream (DM_CELL *key, char *streamName);
static int compareGlnDate (DM_CELL *key1, char *stream1, DM_CELL *key2);
static int compareDmStreamDate (DM_CELL *key1, char *stream1, DM_CELL *key2, char *stream2);

extern char * argv0;
extern char * optarg;
extern int    optind;

DM_CELL          * cellKey;
DM_PROCDATA      * process;
DM_PROJECT       * dmproject;

long bxl, bxr, byb, byt;
int order[100];
int restroke[100];

bool_t optVerbose  = FALSE;
bool_t optAlsoSymb = FALSE;
bool_t optTransparent = FALSE;
bool_t optFullTransparent = FALSE;
bool_t optInfo     = FALSE;
bool_t optBbox     = FALSE;
bool_t optPlotTerm = FALSE;
bool_t optNoExpand = FALSE;
char * maskNot     = NULL;
char * maskOnly    = NULL;

int main (int argc, char **argv)
{
    int     c, errflg = 0;
    char  * version = WORKING;	/* default version */
#ifdef EPSPLOT
    char  * procsuffix = NULL, * procfile = NULL;

    argv0 = "getepslay";
#else
    argv0 = "mplot";
#endif

    catchSignals ();

 /* evaluate all flags and filenames */

#ifndef EPSPLOT
    while ((c = getopt (argc, argv, "to:n:vie")) != EOF) {
	switch (c) {
	    case 'n': maskNot     = optarg;  break;
	    case 'o': maskOnly    = optarg;  break;
	    case 'v': optVerbose  = TRUE;    break;
	    case 'i': optInfo     = TRUE;    break;
	    case 't': optPlotTerm = TRUE;    break;
	    case 'e': optNoExpand = TRUE;    break;
	    case '?': errflg++;              break;
	}
    }

    if (errflg || optind != argc - 1) {
	printf ("Usage: %s [-ivt] [-n masklist] [-o masklist] cellname\n", argv0);
	exit (1);
    }
#else /* EPSPLOT */
    while ((c = getopt (argc, argv, "FTbsrtl:w:h:o:n:vip:P:e")) != EOF) {
	switch (c) {
	    case 'n': maskNot     = optarg;  break;
	    case 'o': maskOnly    = optarg;  break;
	    case 'v': optVerbose  = TRUE;    break;
	    case 's': optAlsoSymb = TRUE;    break;
	    case 'i': optInfo     = TRUE;    break;
	    case 'b': optBbox     = TRUE;    break;
	    case 't': optPlotTerm = TRUE;    break;
	    case 'T': optTransparent = TRUE;    break;
	    case 'F': optFullTransparent = TRUE;    break;
	    case 'w': plotSetDrawWidth (optarg); break;
	    case 'h': plotSetDrawHeight (optarg); break;
	    case 'l': plotSetLambda (optarg); break;
	    case 'r': plotSetRotation ();    break;
	    case 'p': procsuffix  = optarg;  break;
	    case 'P': procfile    = optarg;  break;
	    case 'e': optNoExpand = TRUE;    break;
	    case '?': errflg++;              break;
	}
    }

    if (errflg || optind != argc - 1) {
	printf ("Usage: %s [-ivt] (-p procsuffix | -P procfile)\n\
       [-n masklist] [-o masklist] [-w drawwidth] [-h drawheight]\n\
       [-l lambda] [-r] cellname\n", argv0);
	exit (1);
    }
#endif /* EPSPLOT */

    if (optVerbose) verboseSetMode (TRUE);

    dmInit (argv0);

    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, dmproject);

    for (c = 0; c < process -> nomasks; c++) {
	order[c] = c;
	restroke[c] = -1;
    }

#ifdef EPSPLOT
    doEpsTechnology (dmproject, procsuffix, procfile, order, restroke);
#endif /* EPSPLOT */

    while (optind <= argc -1)
	docell (argv[optind++], version);

    dmCloseProject (dmproject, COMPLETE);

    dmQuit ();
    return (0);
}

static void
docell (char *cellname, char *version) /* plot a cell */
{
    int i, j, pass, num_passes = 1;

#ifdef EPSPLOT
    num_passes = 3;
#endif

    verbose ("cell %s\n", cellname);

    if (!optNoExpand) expand (cellname);

    cellKey = dmCheckOut (dmproject, cellname, version, DONTCARE, LAYOUT, READONLY);

    mplotInit (cellKey);

    for (pass = 1; pass <= num_passes; pass++) {
	plotPass (pass);
	verbose ("pass %d\n", pass);

	for (j = 0; j < process -> nomasks; j++) {

	    i = (pass == 1)? order[j] : restroke[j];

	    if (i < 0) continue;

	    if (pass == 1
	    || (pass == 2 && optFullTransparent == TRUE)
	    || (pass == 2 && optTransparent == TRUE
			  && process -> mask_type[i] == DM_INTCON_MASK)) {
		if (!optAlsoSymb && process -> mask_type[i] == DM_SYMB_MASK) {
		    verbose ("skip %s\n", process -> mask_name[i]);
		    continue;	/* skip symbolic layers */
		}

		if (maskNot && strfind (maskNot, process -> mask_name[i])) {
		    verbose ("skip %s\n", process -> mask_name[i]);
		    continue;
		}

		if (maskOnly && !strfind (maskOnly, process -> mask_name[i])) {
		    verbose ("skip %s\n", process -> mask_name[i]);
		    continue;
		}

		domask (process -> mask_name[i], process -> PLOT[i] -> code);
	    }

	    if (pass == 2 || pass == 3 || num_passes == 1) {
		if (optPlotTerm && process -> mask_type[i] == DM_INTCON_MASK) {
		    doterm (process -> mask_name[i], process -> mask_no[i],
			    process -> PLOT[i] -> code);
		}
	    }
	}

	if (pass == 2 || pass == 3) {
	    if (optBbox) plotBbox ();
	    doAnnotations ();
	}
    }

    mplotEnd ();

    dmCheckIn (cellKey, COMPLETE);
}

static void
domask (char *maskname, int color) /* plot mask maskname with color */
{
    verbose ("mask %s\n", maskname);

    plotMask (maskname, color);

    openInput (cellKey, maskname);

    scan ();

    closeInput ();
}

static void
doterm (char *maskname, int maskno, int color) /* plot terminals in mask maskname with color */
{
    DM_STREAM * stream = NULL;

    plotMask (mprintf ("t_%s", maskname), color);

    stream = dmOpenStream (cellKey, "term", "r");

    while (dmGetDesignData (stream, GEO_TERM) > 0) {

	if (maskno != gterm.layer_no) continue; /* wrong layer (oops) */

	if (gterm.nx == 0 && gterm.ny == 0) {
	    plotTerminal (gterm.xl, gterm.xr, gterm.yb, gterm.yt, gterm.term_name);
	}
	else {
	    int i, j;
	    char name[100];
	    for (i = 0; i <= gterm.nx; i++) {
		for (j = 0; j <= gterm.ny; j++) {
		    if (gterm.nx == 0 && gterm.ny != 0)
			 sprintf (name, "%s[%d]", gterm.term_name, j + 1);
		    else if (gterm.nx != 0 && gterm.ny == 0)
			 sprintf (name, "%s[%d]", gterm.term_name, i + 1);
		    else
			 sprintf (name, "%s[%d,%d]", gterm.term_name,
			     i + 1, j + 1);
		    plotTerminal (
			gterm.xl + i * gterm.dx, gterm.xr + i * gterm.dx,
			gterm.yb + j * gterm.dy, gterm.yt + j * gterm.dy,
			name);
		}
	    }
	}
    }

    dmCloseStream (stream, COMPLETE);
}

void dmError (char *s) /* ddm error handler */
{
    fprintf (stderr, "%s: ddm error: ", argv0);
    dmPerror (s);
    die ();
}

void die () /* clean-up and stop */
{
    dmQuit ();
    exit (1);
}

int doAnnotations ()
{
#ifdef EPSPLOT
    DM_STREAM * dmfp;
    struct stat statBuf;

    verbose ("annotations\n");

    if (dmStat (cellKey, "annotations", &statBuf) == -1) return -1;
    if (!(dmfp = dmOpenStream (cellKey, "annotations", "r"))) return -1;

    /* format record first */
    if (dmGetDesignData (dmfp, GEO_ANNOTATE) > 0) {
        ASSERT (ganno.type == GA_FORMAT);
        ASSERT (ganno.o.format.fmajor == 1);
        ASSERT (ganno.o.format.fminor == 1);
    }

    while (dmGetDesignData (dmfp, GEO_ANNOTATE) > 0) {
	switch (ganno.type) {
	    case GA_LINE:
		mplotLine ((long int)ganno.o.line.x1, (long int)ganno.o.line.y1,
                          (long int)ganno.o.line.x2, (long int)ganno.o.line.y2,
                          ganno.o.line.mode);
		break;
	    case GA_TEXT:
		plotText ((long int)ganno.o.text.x, (long int)ganno.o.text.y,
                          (int)ganno.o.text.ax, ganno.o.text.text);
	    case GA_LABEL:
		plotText ((long int)ganno.o.Label.x, (long int)ganno.o.Label.y,
                          (int)ganno.o.Label.ax, ganno.o.Label.name);
		break;
        }
    }
    dmCloseStream (dmfp, COMPLETE);
#endif /* EPSPLOT */
    return (0);
}

static void expand (char *cellname)
{
    bool_t ppNeeded = FALSE;

    DM_CELL * cellKey = dmCheckOut
	(dmproject, cellname, WORKING, DONTCARE, LAYOUT, READONLY);

    if (existDmStream (cellKey, "spec") == FALSE) {
	Debug (fprintf (stderr, "no previous exp or hierarchical\n"));
	ppNeeded = TRUE;
    }
    else if (compareGlnDate (cellKey, "box", cellKey) <= 0) {
	Debug (fprintf (stderr, "no gln file or box file is newer\n"))
	ppNeeded = TRUE;
    }

    if (ppNeeded) {
	verbose ("preprocessing %s...", cellname);

	dmCheckIn (cellKey, COMPLETE);

	run ("makeboxl", cellname);
	run ("makegln", cellname);

	verbose ("done\n");
    }
    else
	dmCheckIn (cellKey, COMPLETE);
}

static void run (char *command, char *arg)
{
    if (_dmRun (command, arg, NULL)) {
	say ("error in '%s %s'", command, arg);
	die ();
    }
}

/* returns TRUE if stream exist.
*/
static bool_t existDmStream (DM_CELL *key, char *streamName)
{
    struct stat buf;
    return (dmStat (key, streamName, &buf) == 0 ? TRUE : FALSE);
}

static int compareGlnDate (DM_CELL *key1, char *stream1, DM_CELL *key2)
{
    static char * stream2 = NULL;

    if (stream2 == NULL) {
	int i;
	for (i = 0; i < process -> nomasks; i++) {
	    if (process -> mask_type[i] == DM_INTCON_MASK) break;
	}
	ASSERT (i < process -> nomasks);

        stream2 = strsave (mprintf ("%s_gln", process -> mask_name[i]));
    }

    return (compareDmStreamDate (key1, stream1, key2, stream2));
}

/* returns < 0 if stream2 older then stream 1, and stream1 exists
 *
 * If stream 2 does not exist: -2
 * If stream 2 exists:         -1
 */
static int compareDmStreamDate (DM_CELL *key1, char *stream1, DM_CELL *key2, char *stream2)
{
    struct stat buf1, buf2;

    if (dmStat (key1, stream1, &buf1) != 0) return ( 2);
    if (dmStat (key2, stream2, &buf2) != 0) return (-2);

    if (buf1.st_mtime < buf2.st_mtime) return ( 1);
    if (buf1.st_mtime > buf2.st_mtime) return (-1);

    return (0);
}
