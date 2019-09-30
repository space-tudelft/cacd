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

#include "src/space/makegln/config.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "src/libddm/dmincl.h"
#include "src/space/auxil/auxil.h"
#include "src/space/makegln/makegln.h"
#include "src/space/makegln/proto.h"

#ifdef MAKEDELA
#include "src/space/makedela/dproto.h"
void getTechnology (DM_PROJECT *dmproject, char *techDef, char *techFile);
char *giveICD (char *filepath);
extern char *usedTechFile;
#endif

#ifdef MAKEMESH
/* from gln.c */
void glnInit ();
void glnEnd ();
#endif

/* local operations */
Private void docell (char *cellname, char *version);
Private void makegln (char *maskname, int masktype, int scale);
Private char *strsub (const char *s1, const char *s2);
Private void remove_gln (void);

DM_CELL          * cellKey;
DM_PROCDATA      * process;
DM_PROJECT       * dmproject;

coor_t bxl, byb, bxr, byt;
int    outScale;
double meters;    /* this many meters per internal layout unit */

bool_t optVerbose = FALSE;
bool_t optDelete = FALSE;
bool_t optDebug = FALSE;
bool_t optCompress = TRUE;

#ifdef MAKEDELA
bool_t optDraw = FALSE;
bool_t optDrawP = FALSE;
int decrease_sub_res = 1;
int eliminateSubstrNode = 0;
int sub_res_interp_method = -1;
char * techFile = NULL;
#endif

bool_t optInfo     = FALSE;
bool_t optSortOnly = FALSE;
bool_t optNoOutput = FALSE;
bool_t optSpecial  = FALSE;
char * maskNot     = NULL;
char * maskOnly    = NULL;
char ** cellNames  = NULL;

size_t optMaxMemory = 1024*1024;	/* Default 1 Mbyte */

static char * currentCell = NULL;

extern char *optarg;
extern int optind;

int main (int argc, char **argv)
{
    int     c, errflg = 0;
    double     maxMem = 0.0;	/* megabytes of sort buffer memory */
    char  * version = WORKING;	/* default version */
    char  * optstring;

#ifdef MAKEMESH
    argv0 = "makemesh";
    optstring = "Dcm:v";
#else
#ifdef MAKEDELA
    argv0 = "makedela";
    optstring = "01DE:dem:nvwx";
#else
    argv0 = "makegln";
    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == '%')
	optstring = "Dcm:n:o:v%diSN";
    else
	optstring = "Dcm:n:o:v";
#endif
#endif

    catchSignals ();

 /* evaluate all flags and filenames */

    while ((c = getopt (argc, argv, optstring)) != EOF) {
	switch (c) {
#ifdef MAKEDELA
	    case '0': sub_res_interp_method = 0; break;
	    case '1': sub_res_interp_method = 1; break;
	    case 'n': decrease_sub_res = 0;  break;
	    case 'e': eliminateSubstrNode = 1; break;
            case 'E': techFile = strsave (optarg); break;
#else
	    case 'n': maskNot     = optarg;  break;
#endif
	    case 'o': maskOnly    = optarg;  break;
	    case 'i': optInfo     = TRUE;    /* implies verbose */
	    case 'v': optVerbose  = TRUE;    break;
	    case 'd': optDebug    = TRUE;    break;
	    case 'c': optCompress = FALSE;   break;
	    case 'D': optDelete   = TRUE;    break;
#ifdef MAKEDELA
	    case 'w': optDraw = TRUE;    break;
	    case 'x': optDrawP = TRUE;    break;
#endif
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
	printf (
#ifdef MAKEMESH
	    "Usage: %s %s[-Dcv] [-m maxmem] cell [...]\n",
#else
#ifdef MAKEDELA
	    "Usage: %s %s[-01Ddenvwx] [-E file] [-m maxmem] cell [...]\n",
#else
	    "Usage: %s %s[-Dcv] [-m maxmem] [-n masklist] [-o masklist] cell [...]\n",
#endif
#endif
	    argv0, optSpecial ? "[-%diSN] " : "");
	exit (1);
    }

    if (maxMem > 0) { optMaxMemory = (size_t) (1024 * 1024 * maxMem); }

    if (optVerbose) {
	verboseSetMode (TRUE);
	message ("%s ", argv0);
	for (c = 1; c < argc; c++) message ("%s ", argv[c]);
	message ("\n");

	message ("-v: verbose mode\n");
	if (maskNot)     message ("-n: skip masks %s\n", maskNot);
	if (maskOnly)    message ("-o: only masks %s\n", maskOnly);
	if (optInfo)     message ("-i: print statistics, implies -v\n");
	if (optSortOnly) message ("-S: sort edge only, no overlap removal\n");
	if (optNoOutput) message ("-N: no database gln file output\n");
#ifdef MAKEDELA
	if (eliminateSubstrNode) message ("-e: eliminate substrate node\n");
	if (!decrease_sub_res) message ("-n: no decrease_sub_res\n");
	if (sub_res_interp_method >= 0)
	    message ("-%d: sub_res_interp_method\n", sub_res_interp_method);
#endif
#if !defined(MAKEMESH) && !defined(MAKEDELA)
	if (optDelete) message ("-D: remove _bxx files\n");
#else
	if (optDelete) message ("-D: remove _aln files\n");
#endif
	if (!optCompress) message ("-c: do not compress tmp file\n");

	if (maxMem > 0) {
	    message ("-m %g: %ld bytes of sort buffer core memory\n", maxMem, (long) optMaxMemory);
	}
    }

    if (optInfo) clockInit ();

    dmInit (argv0);

    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

#ifdef MAKEDELA
    getTechnology (dmproject, NULL, techFile);
    if (optVerbose) {
	message ("technology file: %s\n", giveICD (usedTechFile));
    }
#else
#ifndef MAKEMESH
    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, dmproject);
#endif
#endif

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
	sortPrintInfo (NULL);

    verbose ("%s --- Finished ---\n", argv0);
    return (0);
}

Private void remove_gln ()
{
#ifdef MAKEMESH
    dmUnlink (cellKey, "mesh_gln");
#else
#ifdef MAKEDELA
    dmUnlink (cellKey, "cont_gln");
#else
    struct stat st_buf;
    int i;
    for (i = 0; i < process -> nomasks; i++) {
	if (maskNot  &&  strsub (maskNot,  process -> mask_name[i])) continue;
	if (maskOnly && !strsub (maskOnly, process -> mask_name[i])) continue;
	dmUnlink (cellKey, mprintf ("%s_gln", process -> mask_name [i]));
    }

    /* remove info about resizing since it has become invalid. */
    /* makegln must always remove this file, see: scan/determ.c */
    if (dmStat (cellKey, "resize.t", &st_buf) == 0)
	dmUnlink (cellKey, "resize.t");

#endif /* MAKEDELA */
#endif /* MAKEMESH */
}

int scale = 1;

Private void docell (char *cellname, char *version) /* do a cell */
{
    int i;
    DM_STREAM * info3;

    tick (mprintf ("%s %s", argv0, cellname));

    verbose ("cell %s, version %s\n", cellname, version);

    cellKey = dmCheckOut (dmproject, cellname, version, DONTCARE, LAYOUT, ATTACH);

    currentCell = cellname;

    info3 = dmOpenStream (cellKey, "info3", "r");
    dmGetDesignData (info3, GEO_INFO3);

    if (ginfo3.nr_samples != 0) {
	i = scale = 1;
	outScale = (int) ginfo3.nr_samples;
    }
    else {
	scale = SCALE;
	outScale = SCALE;
    }

    meters = (1e-6 * cellKey -> dmproject -> lambda) / outScale;

    bxl = (coor_t) (scale * ginfo3.bxl);
    bxr = (coor_t) (scale * ginfo3.bxr);
    byb = (coor_t) (scale * ginfo3.byb);
    byt = (coor_t) (scale * ginfo3.byt);

    dmCloseStream (info3, COMPLETE);

#ifdef MAKEMESH
    remove_gln ();
    makegln ("mesh", 1, scale);
#else
#ifdef MAKEDELA
    remove_gln ();
    makegln ("cont", 1, scale);

    if (optDraw) {
	DM_STREAM * dmsDelau;

	dmsDelau = dmOpenStream (cellKey, "delgraph", "w");
	drawGraph (dmsDelau -> dmfp, 1);
	dmCloseStream (dmsDelau, COMPLETE);
    }

    if (optDrawP) {
	FILE *fp_graph;

	if ((fp_graph = fopen ("delaunay.pic", "w")) == NULL) {
	    fprintf (stderr, "Cannot open %s\n", "delaunay.pic");
	}
	else {
	    drawGraph (fp_graph, 0);
	    fclose (fp_graph);
	}
    }

#else

    /* first remove old gln files */
    remove_gln ();

    for (i = 0; i < process -> nomasks; i++) {
#if 0
	if (process -> mask_type[i] == DM_SYMB_MASK) /*!!!*/
	    continue;		/* skip symbolic layers */ /*!!!*/
#endif

	if ((maskNot != NULL)
	&&  (strsub (maskNot, process -> mask_name[i]) != NULL)) {
	    verbose ("skip %s\n", process -> mask_name[i]);
	    continue;
	}

	if ((maskOnly != NULL)
	&&  (strsub (maskOnly, process -> mask_name[i]) == NULL)) {
	    verbose ("skip %s\n", process -> mask_name[i]);
	    continue;
	}

	makegln (process -> mask_name [i], process -> mask_type [i], scale);
    }
#endif /* MAKEDELA */
#endif /* MAKEMESH */

    currentCell = NULL;
    dmCheckIn (cellKey, COMPLETE);
    tock (mprintf ("%s %s", argv0, cellname));
}

Private void makegln (char *maskname, int masktype, int scale) /* do mask maskname of type masktype */
{
    verbose ("mask %s\n", maskname);

    tick ("readEdges");
    readEdges (cellKey, maskname, masktype, scale);
    tock ("readEdges");

    openOutput (cellKey, maskname);

#ifdef MAKEMESH
    glnInit ();
#endif

    tick ("scan");
    scan ();
    tock ("scan");

#ifdef MAKEMESH
    glnEnd ();
#endif

#ifdef MAKEDELA
    makeOutput ();
#endif

    closeOutput ();
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
        char * s = "Emergency exit\n";
        write (fileno (stderr), s, strlen (s));
        exit (1);
    }

    outputCleanUp ();

#ifndef DEBUG
    if (currentCell) {
#if !defined(MAKEMESH) && !defined(MAKEDELA)
	say ("cleaning up %s (removing gln files)", currentCell);
#endif
	remove_gln ();
    }
#endif

    dmQuit ();

    if (optInfo) {
        clockPrintAll (stdout);
        clockPrintTime (stdout);
        scanPrintInfo (stdout);
        sortPrintInfo (stdout);
        outputPrintInfo (stdout);
    }
    else /* close the sort module */
	sortPrintInfo (NULL);

    exit (1);
}

#define SEP " \t\n,"

Private char * strsub (const char *s1, const char *s2)
{
    char * p;
    char *buf;

    buf = strsave (s1);

    for (p = strtok (buf, SEP); p; p = strtok ((char *) NULL, SEP)) {
        if (strcmp (p, s2) == 0) break; /* found */
    }

    DISPOSE (buf, strlen(s1)+1);
    return (p);
}

void extraSay ()
{
}
