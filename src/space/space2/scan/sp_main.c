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
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h> /* for execv */
#include <sys/stat.h>

#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/include/version.h"
#include "src/space/auxil/auxil.h"
#include "src/space/auxil/monitor.h"
#include "src/space/include/type.h"
#include "src/space/scan/determ.h"
#include "src/space/scan/scan.h"
#include "src/space/scan/extern.h"
#include "src/space/extract/export.h"

#define HTTP_ADDRESS "http://www.space.tudelft.nl"
#define L(v) (long)(v)

static char *prep_bin;
static DM_CELL     * circuitKey;
static char        circuitName[DM_MAXNAME + 1];
static DM_PROJECT  * dmproject = NULL;
double meters; /* this many meters per internal layout unit */

bool_t optFineNtw = FALSE;
bool_t optTime    = FALSE;
bool_t optSimpleSubRes = FALSE;
bool_t optSubRes  = FALSE;
bool_t optSubResMakefem = FALSE;
bool_t optSubResSave = FALSE;
bool_t optTorPos  = TRUE;
bool_t optDsConJoin = TRUE;
bool_t optCap     = FALSE;
bool_t optCap3D   = FALSE;
bool_t optCapSave = FALSE;
bool_t optCoupCap = FALSE;
bool_t optCoupCapSave = FALSE;
bool_t optLatCap  = FALSE;
bool_t optIntRes  = FALSE;
bool_t optRes     = FALSE;
bool_t optAllRes  = FALSE;
bool_t optNoReduc = FALSE;
bool_t optVerbose = FALSE;
bool_t optInfo    = FALSE;
bool_t optMonitor = FALSE;
bool_t optResMesh = FALSE;
bool_t optFlat    = TRUE;
bool_t optPseudoHier = FALSE;
bool_t optNoPrepro = FALSE;
bool_t optEstimate3D = FALSE;
bool_t optPrintRecog = FALSE;
bool_t optOnlyLastPass = FALSE;
bool_t optOnlyPrePass = FALSE;
bool_t optWriteRecogs = FALSE;

int    nrOfMasksExt;
int    optSpecial = 0;
int    verbosityLevel = 0;

int    sub_caps_entry = 0;
extern int subcap_cnt;
extern double sub_rc_const;

int    add_sub_caps = 0;
bool_t alsoPrePass = 0;
bool_t extrPass = 0;
bool_t extrEdgeCaps = 0;
bool_t extrSurfCaps = 0;
bool_t firstPass = 0;
bool_t lastPass = 0;
int    prePass = 0;
bool_t prePass1 = 0;
bool_t skipPrePass0 = 0;

FILE *xout = NULL;
bool_t optDisplay = FALSE;

bool_t substrRes = FALSE; /* TRUE if optSubRes or optSimpleSubRes */

bool_t useAnnotations = TRUE;
bool_t useHierAnnotations = TRUE;
bool_t useHierTerminals = TRUE;
bool_t useLeafTerminals = TRUE;
bool_t useCellNames = TRUE;

static char * techFile = NULL;
static char * techDef = NULL;
static char * paramFile = NULL;
static char * paramDef = NULL;
static char ** cellNames = NULL;
int optAlarmInterval = 0;

coor_t bandWidth;   /* band width used in the 'scan' module */
coor_t bandWidth2;  /* band width used in the 'scan' module */
double effectDist;  /* lateral cap band width used */
coor_t baseWindow;  /* maximum basewidth for lateral bipolar pnp's */

coor_t bigbxl, bigbxr, bigbyb, bigbyt;
coor_t bbxl, bbxr, bbyb, bbyt; /* global bounding box */

#ifdef __cplusplus
  extern "C" {
#endif

Private void optionImplications (void);
Private void extractTree (char *cellName);
Private void prepass (do_t *dObject);
Private void extract (do_t *dObject);
Private void expand (do_t *dObject);
Private void processgln (char *cellname);
Private void run (char *command, char *cellname, char *option);
Private void run2 (char *argv[]);
Private void initHierNames (void);
Private void endHierNames (void);

#ifdef __cplusplus
  }
#endif

extern char *optarg;
extern int optind;

void overallUtilization ()
{
    message ("overall resource utilization:\n");
    message ("\tmemory allocation  : %.3f Mbyte\n", allocatedMbyte ());
    clockPrintTime (stdout);
}

int sp_main (int argc, char **argv)
{
    FILE *fpnew;
    bool_t optPrintUnused;
    int c, i, optErr = 0;
    char optstring[100];
    char optspecial[20], *s, *t;

    argv0 = "space2";

    dmInit (argv0);
    fpnew = fopen ("space2.log", "w");
    verboseSetNewStream (fpnew);

    s = optstring;
    strcpy (s, "cClrRzb"); s += 7;
#ifdef CAP3D
    *s++ = 'B';
#endif
    strcpy (s, "nuhvi"); s += 5;
    t = s; /* remember end */
    strcpy (s, "a:E:e:P:p:S:");

    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == '%') {
	++optSpecial;
	s = optspecial;
	*s++ = '%';
	*s++ = 'f'; /* add fine network option */
#ifdef MONITOR
	*s++ = 'm'; /* add monitor option */
#endif
	strcpy (s, "qwXZ02"); /* add other special options */
	strcat (t, optspecial); /* add to optstring */
    }

    while ((c = getopt (argc, argv, optstring)) != EOF) {
	switch (c) {
	    case 'l': optLatCap  = TRUE;
	    case 'C': optCoupCap = TRUE;
	    case 'c': optCap     = TRUE; break;
	    case 'z': optResMesh = 1;
	    case 'r': optIntRes  = TRUE; break;
	    case 'R': optIntRes  = TRUE; optAllRes = TRUE; break;
	    case 'n': optNoReduc = TRUE; break;
	    case 'u': optNoPrepro= TRUE; break;
	    case 'i': optInfo    = TRUE;
	    case 'v': optVerbose = TRUE; ++verbosityLevel; break;
	    case 'E': if (!techFile) techFile = strsave (optarg);
		      else { say ("Use only ones the -E option."); optErr++; }
		      break;
	    case 'e': if (!techDef) techDef = strsave (optarg);
		      else { say ("Use only ones the -e option."); optErr++; }
		      break;
	    case 'P': if (!paramFile) paramFile = strsave (optarg);
		      else { say ("Use only ones the -P option."); optErr++; }
		      break;
	    case 'p': if (!paramDef) paramDef = strsave (optarg);
		      else { say ("Use only ones the -p option."); optErr++; }
		      break;
	    case 'S': paramSetOption (optarg); break;
	    case 'a': optAlarmInterval = atoi (optarg); break;
	    case 'B': optSubRes = TRUE;  break;
	    case 'b': optSimpleSubRes = TRUE; break;

	    /* special options */
	    case '%': break;
	    case 'f': optFineNtw = TRUE; break;
	    case 'm': optMonitor = TRUE; break;
	    case 'q': optPrintRecog = TRUE; break;
	    case 'w': optWriteRecogs = TRUE; break;
	    case 'X': optDisplay = TRUE; break;
	    case 'Z': optOnlyPrePass = TRUE; break;
	    case '0': skipPrePass0   = optNoPrepro = TRUE; break;
	    case '2': optOnlyLastPass= optNoPrepro = TRUE; break;

	    default: optErr++;
	}
    }

    if (optCap && !optCoupCap) { say ("Option -c not supported!"); optCap = FALSE; }

    cellNames = argv + optind;

    if (optOnlyLastPass && optOnlyPrePass) {
	say ("Use only one of -Z and -2 options."); optErr++;
    }

    if (techDef && techFile) {
	say ("Use only one of -E and -e options."); optErr++;
    }

    if (paramDef && paramFile) {
	say ("Use only one of -P and -p options."); optErr++;
    }

    if (!cellNames[0] || !cellNames[0][0]) {
	say ("You must specify a cell name."); optErr++;
    }
    else if (cellNames[1]) {
	say ("You may only specify one cell name."); optErr++;
    }

    if (optErr) {
	fprintf (stderr, "\nUsage: %s ", argv0);
	if (optSpecial) {
	    fprintf (stderr, "[-%s] ", optspecial);
	}
	*t = 0; /* set end */
	fprintf (stderr, "[-%s] [-a time]", optstring+1);
	fprintf (stderr, "\n       [-E file | -e def] [-P file | -p def] [-S param=value]");
	fprintf (stderr, " cell\n\n");
	compileDate (argv0);
	message ("\nType 'icdman toolname' to obtain the manual page of a tool\n");
	message ("(e.g 'icdman %s' to obtain the manual page of this tool).\n", argv0);
	message ("User manuals can be found in $ICDPATH/share/doc.\n");
	message ("See %s for (more) documentation, background\n", HTTP_ADDRESS);
	message ("information and release notes about the latest version of this program.\n\n");
	die ();
    }

#ifdef CAP3D
    /* Set some flags, depending on the verbosity level. */
    if (verbosityLevel > 1) schurShowProgress = TRUE;
#endif /* CAP3D */

    /************ Real Program Starts Here *******************/

    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    paramFile = getParameters (dmproject, paramDef, paramFile);
    getTechFile (dmproject, techDef, techFile);

    c = 0;
    if (!optVerbose) { optVerbose = paramLookupB ("verbose", "off"); ++c; }
    if (optVerbose || fpnew) {
	if (optInfo || fpnew) {
	    if (!optInfo) swapVerboseStreams ();
	    message ("%s ", argv0);
	    for (i = 0; ++i < argc;) message ("%s ", argv[i]);
	    message ("\n");
	    if (!optInfo && optVerbose) swapVerboseStreams ();
	}
	compileDate (argv0);
	message ("See %s\n", HTTP_ADDRESS);
	message ("parameter file: %s\n", giveICD (paramFile));
	message ("technology file: %s\n", giveICD (usedTechFile));
	if (!optVerbose) swapVerboseStreams ();
    }

    if (paramLookupB ("param_verbose", "off")) {
	paramSetVerbose (1);
	paramLookupB ("param_verbose", "off");
	if(c) paramLookupB ("verbose", "off");
    }
    else if (fpnew) paramSetVerbose (2);

    optPrintUnused = paramLookupB ("param_unused", "off");

    if (!optSimpleSubRes) optSimpleSubRes = paramLookupB ("simple_sub_extraction", "off");
#ifdef CAP3D
    if (!optSubRes) optSubRes = paramLookupB ("sub3d_extraction", "off");
    if (optSubRes) optSubResMakefem  = paramLookupB ("sub3d.makefem", "off");
#endif
    if (!optAlarmInterval) optAlarmInterval = paramLookupI ("progress_timer", "0");
    optTorPos = paramLookupB ("component_coordinates", "on");

    if (!optNoReduc) optNoReduc = !paramLookupB ("heuristics", "on");
    if (!optFineNtw) optFineNtw = !paramLookupB ("network_reduction", "on");

    if (!optTime) optTime = paramLookupB ("print_time", "off");

    if (paramLookupB ("no_parasitics", "off")) {
        optLatCap = optCoupCap = optCap = FALSE;
        optIntRes = optAllRes = optResMesh = FALSE;
        optSubRes = optSimpleSubRes = FALSE;
	if (optVerbose) message ("No extraction of parasitics!");
    }

    optionImplications ();

    if (optMonitor) monitorStart ();
    if (optTime || optVerbose) clockInit ();
    if (optTime) tick ("startup");
    if (optVerbose) verboseSetMode (TRUE);

    /* get some of the other parameters */
    lookupParameters ();
    if (optResMesh == 2 && !skipPrePass0 && !optOnlyLastPass) alsoPrePass = 1;

#ifndef DEBUG
    catchSignals (); /* don't catch, to get core dump */
#endif

    if (optAlarmInterval >= 0) { /* init progress timer using alarm signal */
	if (setAlarmInterval (optAlarmInterval, catchAlarm)) {
	    /* no alarm signal available, use alternative method */
	    optAlarmInterval = -optAlarmInterval;
	}
    }

    getTechnology (dmproject, techDef, techFile);

    if (optSimpleSubRes && (!self_cnt || !mut_cnt)) {
	if (!self_cnt) say ("Must specify selfsubres data in technology file");
	if (!mut_cnt)  say ("Must specify coupsubres data in technology file");
	say ("Option -b ignored!");
	optSimpleSubRes = substrRes = FALSE;
	optRes = optIntRes;
    }

    if (!substrRes) add_sub_caps = 0;
    else if ((add_sub_caps == 1 && sub_rc_const <= 0) || (add_sub_caps > 1 && subcap_cnt <= 0 && !optSubResMakefem)) {
	if (add_sub_caps == 1)
	    say ("warning: not adding substrate capacitances, parameter sub_rc_const must be > 0");
	else
	    say ("warning: not adding substrate capacitances, technology data must contain subcaplayers");
	add_sub_caps = 0;
    }
    sub_caps_entry = add_sub_caps;

    prep_bin = paramLookupS ("prep_bin", NULL);

    if (optTime) tock ("startup");

    extractTree (cellNames[0]);

    if (optWriteRecogs) printRecogCnt ();

    dmCloseProject (dmproject, COMPLETE);
    dmQuit ();

    if (optTime) clockPrintAll (stdout);

    if (optInfo) {
	scanPrintInfo (stdout);
	nodePointStatistics (stdout);
    }

    if (optMonitor) monitorStop ();

    if (optVerbose) overallUtilization ();
    if (optPrintUnused) {
	if (fpnew) verboseSetNewStream (NULL);
	printUnusedParams ();
	if (fpnew) verboseSetNewStream (fpnew);
    }

    if (optVerbose) message ("%s: --- Finished ---\n", argv0);
    return (0);
}

Private void optionImplications ()
{
    if (optSimpleSubRes && optSubRes) {
        say ("Both methods for substrate resistance extraction are specified");
	say ("Option -B ignored!");
	optSubRes = FALSE;
    }
    substrRes = optSimpleSubRes || optSubRes;
    optRes = optIntRes || substrRes;

    if (!optOnlyLastPass) alsoPrePass = substrRes || (optResMesh && !skipPrePass0);

    optCapSave = optCap;
    optCoupCapSave = optCoupCap;
}

Private void extractTree (char *cellName)
{
    do_t *obj;
    char *optDel = NULL;

    if (optResMesh == 1 && paramLookupB ("delete_aln", "on")) optDel = "-D";

    if ((obj = findCandidates (dmproject, cellName))) {

	Debug (fprintf (stderr, "%s, status %#o\n", obj -> name, obj -> status));

	if (obj -> status & DO_SPACE) {

	    if (obj -> status & (DO_EXP | DO_RESIZE)) {
		expand (obj);
	    }

	    firstPass = 1;

	    if (alsoPrePass) {
		if (optResMesh && !skipPrePass0) {
		    lastPass = optOnlyPrePass && !substrRes;
		    substrRes = 0;
#ifdef DEBUG
		    if(DEBUG) fprintf(stderr, "PASS: starting prepass 0\n");
#endif
		    if (optResMesh == 2) {
			optResMesh = 0;
			prePass = 1; prepass (obj); prePass = 0;
			verbose ("running mkmesh for %s\n", obj -> name);
			run ("mkmesh", obj -> name, optDel);
			optResMesh = 2;
#if 1
			prePass = 1; prepass (obj); prePass = 0;
			verbose ("running mkmesh for %s\n", obj -> name);
			run ("mkmesh", obj -> name, optDel);
#endif
		    }
		    else {
			optResMesh = 0;
			prePass = 1; prepass (obj); prePass = 0;
			verbose ("refining resistance mesh for %s\n", obj -> name);
			run ("makemesh", obj -> name, optDel);
			optResMesh = 1;
		    }
#ifdef DEBUG
		    if(DEBUG) fprintf(stderr, "PASS: finished prepass 0\n");
#endif
		    substrRes = optSimpleSubRes || optSubRes;
		}

		if (substrRes) {
		    lastPass = optOnlyPrePass;
#ifdef DEBUG
		    if(DEBUG) fprintf(stderr, "PASS: starting prepass 1\n");
#endif
		    prePass1 = 1; prepass (obj); prePass1 = 0;

		    processgln (obj -> name); /* run makedela / makesubres */
#ifdef DEBUG
		    if(DEBUG) fprintf(stderr, "PASS: finished prepass 1\n");
#endif
		}
	    }

	    if (!optOnlyPrePass) {
		if (hasEdgeCaps) extrEdgeCaps = TRUE;
		if (hasSurfCaps) extrSurfCaps = TRUE;
		if (add_sub_caps) optCap = optCoupCap = TRUE;
#ifdef DEBUG
		if(DEBUG) fprintf(stderr, "PASS: starting extract pass\n");
#endif
		lastPass = extrPass = 1; extract (obj); extrPass = 0;
#ifdef DEBUG
		if(DEBUG) fprintf(stderr, "PASS: finished extract pass\n");
#endif
		if (add_sub_caps) { optCap = optCapSave; optCoupCap = optCoupCapSave; }
		extrEdgeCaps = extrSurfCaps = 0;
	    }
	}
	else {
	    if (obj -> status & DEVMOD) optDel = "device";
	    else
	    if (obj -> status & ISMACRO) optDel = "macro";
	    else optDel = "unknown";
	    say ("%s not extracted, has %s status\n", obj -> name, optDel);
	}
    }
    else {
	say ("%s not extracted, no candidate\n", cellName);
    }
}

Private void prepass (do_t * dObject)
{
    bool_t optResSave = optRes;
    bool_t optLatCapSave = optLatCap;

    /* optAllRes is not turned off
     * since it may be used during mesh refinement
     */
    optRes = FALSE; /* must be off in all prepasses */
    optCap = optCoupCap = optLatCap = FALSE;

    if (prePass1 && optSubRes) {
	optSubResSave = optSimpleSubRes = TRUE; optSubRes = FALSE;
    }

    extract (dObject);

    if (optSubResSave) { /* prepare pass */
	optSubResSave = optSimpleSubRes = FALSE; optSubRes = TRUE;
    }

    optRes = optResSave;
    optCap = optCapSave;
    optCoupCap = optCoupCapSave;
    optLatCap = optLatCapSave;
}

int outScale;
int inScale;	  /* for reading term files */

Private void extract (do_t * dObject)
{
    char *mode, *cellname = dObject -> name;
    DM_CELL *layoutKey = dObject -> lkey;
    coor_t coor_sub_ext;

    if (optTime) tick (mprintf ("extract %s", cellname));

    if (extrPass) {
	mode = "pe";
	verbose ("extracting %s\n", cellname);
	if (optAlarmInterval) message ("progress: extracting %s\n", cellname);
    }
    else {
	char *note;
	if (prePass) {
	    mode = "p0";
	    note = "for resistance mesh refinement";
	}
	else {
	    ASSERT (prePass1);
	    mode = "p1";
	    if (optSubResSave)
		note = "for substrate resistance prepare";
	    else
		note = "for substrate resistance";
	}
	verbose ("prepassing %s %s\n", cellname, note);
	if (optAlarmInterval) message ("progress: prepassing %s %s\n", cellname, note);
    }

    ASSERT (layoutKey);

    if (firstPass) {
	DM_STREAM *info3;
	char *cirname;

	cirname = mprintf ("%s%s", cellname, paramLookupS ("name_extension", ""));
	if (paramCapitalize) *cirname = toupper (*cirname);
	if (strlen (cirname) > DM_MAXNAME) {
	    say ("error: too long circuitName (> %d chars)", DM_MAXNAME);
	    die ();
	}
	strcpy (circuitName, cirname);
        Debug (fprintf (stderr, "dmCheckOut %s CIRCUIT %d\n", circuitName, CREATE));
        circuitKey = dmCheckOut (dmproject, circuitName, DERIVED, DONTCARE, CIRCUIT, CREATE);

	info3 = dmOpenStream (layoutKey, "info3", "r");
	if (dmGetDesignData (info3, GEO_INFO3) <= 0) {
	    say ("internal error: cannot read info3 data for cell '%s'", cellname);
	    die ();
	}
	dmCloseStream (info3, COMPLETE);

	inScale = outScale = SCALE;
	if (ginfo3.nr_samples != 0) {
	    /* OK to assume ginfo3.nr_samples < 2^^15 */
	    outScale = (int) ginfo3.nr_samples;
	    inScale = 1;
	}

	meters = (1e-6 * layoutKey -> dmproject -> lambda) / outScale;

	/* convert baseWindow from param file to coor_t */
	baseWindow = (coor_t) ceil (max_lat_base * 1e-6 / meters);

	if (equiBandWidth > 0) { /* convert bandWidth from param file to coor_t */
	    double w = ceil (equiBandWidth * 1e-6 / meters);
	    coor_t max = INF/2;
	    if (w > max) bandWidth2 = max; else bandWidth2 = (coor_t) w;
	    if (bandWidth2 <= 0) { say ("too small equi_line_width specified!"); die(); }
	}

	/* set global bounding box variables */
	bbxl = (coor_t) (inScale * ginfo3.bxl);
	bbxr = (coor_t) (inScale * ginfo3.bxr);
	bbyb = (coor_t) (inScale * ginfo3.byb);
	bbyt = (coor_t) (inScale * ginfo3.byt);

	if (hasSubmask) {
	    coor_sub_ext = paramLookupI ("substrate_extension", "10");
	    if (coor_sub_ext < 0) coor_sub_ext = 0;
	    else if (coor_sub_ext > INF/1000) paramError ("substrate_extension", "too big value??");
	    coor_sub_ext *= inScale;
	}
	else coor_sub_ext = 0;

	bigbxl = bbxl - coor_sub_ext;
	bigbxr = bbxr + coor_sub_ext;
	bigbyb = bbyb - coor_sub_ext;
	bigbyt = bbyt + coor_sub_ext;

	setContext (cellname, bigbxl, bigbxr);

	useAnnotations = !paramLookupB ("no_labels", "off");
	useHierAnnotations = paramLookupB ("hier_labels", "off");
	useHierTerminals = paramLookupB ("hier_terminals", "off");
	useLeafTerminals = paramLookupB ("leaf_terminals", "off");
	useCellNames = paramLookupB ("cell_pos_name", "off");

	firstPass = 0;
    }

    ASSERT (circuitKey);

    if (optDisplay && extrPass) {
	DM_STREAM * dsp = dmOpenStream (circuitKey, "display.out", "w");
	if ((xout = dsp -> dmfp)) {
	    fprintf (xout, "#c %s\n", cellname);
	    fprintf (xout, "#s %d %d %g\n", inScale, outScale, layoutKey -> dmproject -> lambda);
	    fprintf (xout, "w %ld %ld %ld %ld\n", L(bbxl), L(bbyb), L(bbxr), L(bbyt));
	    fprintf (xout, "#pe\n");
	}
    }

    if (optResMesh) {
        /* we are going to read the 'mesh_gln' stream as well */
        masktable[nrOfMasks].name = "mesh";
        masktable[nrOfMasks].terminal = 0;
        masktable[nrOfMasks].color = cNull;
        masktable[nrOfMasks].conductor = -1;
        masktable[nrOfMasks].gln = 1;
	nrOfMasksExt = nrOfMasks + 1;
    }
    else
	nrOfMasksExt = nrOfMasks;

    if (optLatCap) { /* convert bandWidth from param file to coor_t */
	effectDist = ceil (physBandWidth * 1e-6 / meters);
	bandWidth = effectDist > INF ? INF : (coor_t) effectDist;
	if (bandWidth < 2 * bandWidth2) bandWidth = 2 * bandWidth2;
    }
    else
	bandWidth = 2 * bandWidth2;

    initHierNames ();

    initExtract (circuitKey, layoutKey);

    readTid (layoutKey, circuitKey);

    openInput (layoutKey);

    scan ();

    closeInput ();

    endExtract ();

    endHierNames ();
    disposeTid ();

    if (lastPass) {
	dmCheckIn (layoutKey,  COMPLETE);
	if (circuitKey) {
	    if (xout) { fprintf (xout, "#end\n"); xout = NULL; }
	    dmCheckIn (circuitKey, COMPLETE); circuitKey = NULL;
	}
    }

    if (optTime) tock (mprintf ("extract %s", cellname));
}

Private void expand (do_t *dObject)
{
    char *cellname = dObject -> name;

    if (dObject -> status & DO_EXP) {
	char *optDel = NULL;
	if (!paramLookupB ("compression", "on")) optDel = "-c";

	verbose ("preprocessing %s (phase 1 - flattening layout)\n", cellname);
	run ("makeboxl", cellname, NULL);

	verbose ("preprocessing %s (phase 2 - removing overlap)\n", cellname);
	if (paramLookupB ("delete_bxx", "on")) { /* and nxx files */
	    optDel = optDel ? "-cD" : "-D";
	}
	run ("makegln", cellname, optDel);
    }

    if (dObject -> status & DO_RESIZE) {
	verbose ("resizing masks for %s\n", cellname);
	run ("makesize", cellname, mprintf ("-E%s", usedTechFile));
    }
}

Private void processgln (char *cellname)
{
    if (optSimpleSubRes) { /* prePass1 */
	char ops[8], *s;
	int i;
	s = ops; *s++ = '-';
	if (paramLookupB ("delete_aln", "on")) *s++ = 'D';
	if (!paramLookupB ("decrease_sub_res", "on")) *s++ = 'n';
	i = paramLookupI ("sub_res_interp_method", "-1");
	if (i >= 0 && i < 10) *s++ = '0' + i;
	*s = 0;
	verbose ("computing substrate effects for %s\n", cellname);
	run ("makedela", cellname, mprintf ("%sE%s", ops, usedTechFile));
	if (add_sub_caps > 1) {
	    say ("warning: not adding substrate capacitances, subcap3d not supported for -b");
	    add_sub_caps = 0;
	}
    }
    else { /* prePass1: optSubRes */
	char optbuf[1024], *s;
	char *av[64], *opt;
	int c, i = 0;

	av[i++] = optSubResMakefem ? "makefem" : "makesubres";

        if (optInfo) av[i++] = "-%i";

	opt = optbuf;
	*opt = 0;
	if (techFile) sprintf (opt, "-E%s", techFile);
	else if (techDef) sprintf (opt, "-e%s", techDef);
	if (*opt) { av[i++] = opt; opt += strlen (opt) + 1; }

	*opt = 0;
	if (paramFile) sprintf (opt, "-P%s", paramFile);
	else if (paramDef) sprintf (opt, "-p%s", paramDef);
	if (*opt) { av[i++] = opt; opt += strlen (opt) + 1; }
	ASSERT (opt - optbuf < 1024);

	c = paramGetOptionCount ();
	while (--c >= 0) {
	    s = paramGetOptionValue (c);
	    if (*s) sprintf (opt, "-S%s=%s", paramGetOptionKey (c), s);
	    else    sprintf (opt, "-S%s", paramGetOptionKey (c));
	    av[i++] = opt; opt += strlen (opt) + 1;
	    ASSERT (opt - optbuf < 1024);
	}

	if (optEstimate3D) av[i++] = "-U";
        if (optVerbose) av[i++] = "-v";

	av[i++] = cellname;
	ASSERT (i < 64);
	av[i] = NULL;

	verbose ("computing substrate effects for %s\n", cellname);
	if (optVerbose) {
	    char buffer[1024];

	    buffer[0] = 0;
	    for (i = 1; av[i]; ++i) {
		strcat (buffer, " ");
		strcat (buffer, av[i]);
	    }
	    verbose ("running: %s%s\n", av[0], buffer);
	}
	run2 (av);

	if (!optSubResMakefem && add_sub_caps > 1) {
	    av[0] = "makesubcap";
	    if (optVerbose) verbose ("running: %s ...\n", av[0]);
	    run2 (av);
	}
    }
}

void dmError (char *s)
{
    dmPerror (s);
    die ();
}

void die ()
{
    dmQuit ();
    if (optMonitor) monitorStop ();
    exit (1);
}

void quit ()
{
    dmQuit ();
    if (optMonitor) monitorStop ();
    if (optVerbose || optInfo) overallUtilization ();
    exit (0);
}

Private void run (char *command, char *cellname, char *option)
{
    char path[1000];

    if (prep_bin)
	sprintf (path, "%s/%s", prep_bin, command);
    else
	sprintf (path, "%s", command);

    if (!option) { option = cellname; cellname = NULL; }

 // fprintf (stderr, "%s %s %s\n", path, option, cellname ? cellname : "");
    if (_dmRun (path, option, cellname, (char *) NULL)) {
	say ("error in '%s %s %s'", command, option, cellname ? cellname : "");
	die ();
    }
}

Private void run2 (char *argv[])
{
    char path[1000];

    if (prep_bin)
	sprintf (path, "%s/%s", prep_bin, argv[0]);
    else
	sprintf (path, "%s", argv[0]);
    argv[0] = path;

    if (_dmRun2 (path, argv)) {
	int i;
	fprintf (stderr, "%s: error in '%s'\n", argv0, path);
	for (i = 0; argv[i]; ++i) fprintf (stderr, "\targv[%d]='%s'\n", i, argv[i]);
	die ();
    }
}

/* string of coordinates in lambda (database unit)
*/
char *strCoorBrackets (coor_t x, coor_t y)
{
    static char buf[88];
    sprintf (buf, "(%s, %s)", strCoor (x), strCoor (y));
    return (buf);
}

char *strCoor (coor_t a)
{
    static char buf1[40], buf2[40], *buf;
    double d_a = a;

    d_a /= outScale;
    buf = (buf == buf1) ? buf2 : buf1;
    if (d_a == (double)((long)d_a))
	sprintf (buf, "%ld", (long)d_a);
    else
	sprintf (buf, "%.2f", d_a);
    return (buf);
}

static char fn_nmp[DM_MAXNAME + 8];
static FILE * fp_nmp;
static int nmp_cnt;
static char * hier_name_sep;
char * inst_term_sep;
char * trunc_name_prefix;
int max_name_length = 0;

Private void initHierNames ()
{
    struct stat statBuf;

    sprintf (fn_nmp, "%s.nmp", circuitName);
    if (stat (fn_nmp, &statBuf) == 0) {
        if (unlink (fn_nmp)) say ("cannot remove/overwrite file '%s'", fn_nmp);
    }
    fp_nmp = NULL;
    nmp_cnt = 0;

    if (!max_name_length) {
	int m;
	hier_name_sep = paramLookupS ("hier_name_sep", ".");
	inst_term_sep = paramLookupS ("inst_term_sep", ".");
	trunc_name_prefix = paramLookupS ("trunc_name_prefix", "n");

	max_name_length = DM_MAXNAME;
	m = paramLookupI ("max_name_length", mprintf ("%d", max_name_length));
	if (m > 5 && m < max_name_length) max_name_length = m;
	ASSERT (max_name_length >= 14);
    }
}

Private void endHierNames ()
{
    if (fp_nmp) fclose (fp_nmp);
}

char *truncDmName (char *s)
{
    int i, l, m;
    static char buf[DM_MAXNAME + 1];

    if ((l = strlen (s)) <= max_name_length) return (s);

    sprintf (buf, "%s%d_", trunc_name_prefix, ++nmp_cnt);
    m = strlen (buf);
    i = l - max_name_length + m;
    while (m < max_name_length) buf[m++] = s[i++];
    buf[m] = '\0';

    if (!fp_nmp) {
	if (!(fp_nmp = fopen (fn_nmp, "w"))) {
	    say ("Cannot open file '%s'\n", fn_nmp);
	    die ();
	}
	say ("hierarchical %s %d characters occurred.\n%s\n%s '%s' %s",
	    "instance/label name(s) longer than", max_name_length,
	    "   These names have been converted to shorter names.",
	    "   See file", fn_nmp, "for a conversion list.");
    }
    fprintf (fp_nmp, "%s %s\n", buf, s);

    return (buf);
}

void convertHierName (char *name)
{
    --name;
    while (*++name) if (*name == '/') *name = *hier_name_sep;
}

char *makeIndex (int nx, int ny, int x, int y)
{
     static char buf[80];

     if (nx > 0 && ny > 0) sprintf (buf, "[%d,%d]", x, y);
     else if (nx > 0) sprintf (buf, "[%d]", x);
     else if (ny > 0) sprintf (buf, "[%d]", y);
     else buf[0] = '\0';
     return (buf);
}
