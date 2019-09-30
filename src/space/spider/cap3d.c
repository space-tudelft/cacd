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
#include <math.h>		/* contains ceil, sqrt */
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/spider/define.h"

#include "src/space/spider/recog.h"
#include "src/space/spider/extern.h"
#include "src/space/extract/export.h"
#include "src/space/lump/export.h"
#include "src/space/green/green.h"
#include "src/space/green/export.h"
#include "src/space/schur/export.h"
#include "src/space/scan/export.h"

#ifdef DISPLAY
#include "src/space/X11/export.h"
#endif

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
void catchAlarm (void);
Private void doStrip (int mode);
Private void computeCapacitance (int mode, int row, int col, spider_t *s1, spider_t *s2);
Private void schurStartMatrix (int row, int col);
Private void schurStopMatrix  (void);
Private void schurInput (int row, int col, schur_t val, spider_t *s1, spider_t *s2);
Private void addCap (spider_t *s1, spider_t *s2, schur_t val);
Private int extractWindow (spider_t *s);
Private void cbemdataInit (DM_CELL * key);
Private void cbemdataQuit (void);
#ifdef __cplusplus
  }
#endif

/* Since the window size will be stored in a coor_t,
   we must guarantee that it is not too large.
   Therefore, the following parameter defines the
   maximum (internal) window size.
*/
#define MAX_INTERN_WINDOW    100000000

#define D(x) (double)(x)

double maxFeRatio;
double new_microns;
double new_microns2;

static int maxRow;
static int maxCol;
static bool_t  noSchur; 	/* debugging */
static bool_t  noGreen; 	/* debugging */
static double greenFactor = 1.0;
static double greenFactor2 = 1.0;
static bool_t serverMatrix = FALSE;
static bool_t printSchur = FALSE;
static bool_t printAllPoints = FALSE;

static double MinArea, MaxArea, TotalArea;
static double MinDist, MaxDist;
static double MinLength, MaxLength, MaxLengthRatio;

static int brutoNumGreens = 0;
static int cap_assign_type = 0;
static int MaxDimension = 0;
static int MaxOrder = 0;

DM_STREAM * cbemStream = NULL;

bool_t check_green = FALSE;
bool_t doStripDrawAndPrint = FALSE;
bool_t extractGateDsCap3d = TRUE;
bool_t extractGateGndCap3d = FALSE;
bool_t FeModePwl = FALSE;
bool_t FeModeGalerkin = FALSE;

struct spiderControl spiderControl;
int numJoins    = 0;
int numFaces    = 0;
int numVertices = 0;
int numGreens   = 0;

int FeShape     = 0;
int greenCase   = 0; /* DIEL = 0 = cap3d, SUBS = 1 = sub3d */
int greenType   = 0; /* 0 = cap3d, G; 1 = cap3d, dG/dn; */
                     /* 2 = sub3d, G; 3 = sub3d, dG/dn. */

extern int optAlarmInterval;
static int max_fe_count = 0;
static int sub_fe_count = 0;
static coor_t windowWidthCap;
static coor_t windowWidthSub;
static coor_t windowWidthSubFe;
static coor_t windowWidthSubFe2;
static coor_t windowHeightCap;
static coor_t windowHeightSub;
static coor_t stripLeft  = 0;
static coor_t stripRight = 0;
static coor_t nextStrip;
static int    cntStrip;

static struct schurdata {
    int row, col, maxcol;
    schur_t * buf;
    schur_t   capSign;
} schurdata;

extern int *spidercols;
extern spider_t **spiderlist;

extern double step_slope;

extern int nrOfSpiderLevels;

extern bool_t optIntRes;
static bool_t printCap3dInit;
static int said = 0;
static int messflag = 0;

Private void get3DWindow (int gCase, coor_t *WW, coor_t *WH)
{
    char *par_value, *param;
    double ww, wh;

    param = (gCase == DIEL)? "cap3d.be_window" : "sub3d.be_window";

    ww = wh = 0;
    if ((par_value = paramLookupS (param, (char *) NULL))) {
	char buf1[132], buf2[132];
	int n;

	buf1[0] = buf2[0] = '\0';
	if ((n = sscanf (par_value, "%s %s", buf1, buf2)) > 0) {
	    if (strsame (buf1, "inf") || strsame (buf1, "infinite")) ww = 9e99;
	    else if (sscanf (buf1, "%le", &ww) != 1) ww = 0;
	    if (n > 1) {
		if (strsame (buf2, "inf") || strsame (buf2, "infinite")) wh = 9e99;
		else if (sscanf (buf2, "%le", &wh) != 1) wh = 0;
	    }
	}
	if (ww > 0 && wh <= 0) wh = ww;
    }

    if (ww <= 0 || wh <= 0) {
	if (par_value) paramError (param, "Value(s) must be > 0");
	say ("Parameter %s must be specified!", param);
	say ("Can't continue, stop"); die ();
    }

    ww /= new_microns;
    wh /= new_microns;

    if (ww > MAX_INTERN_WINDOW) {
	ww = MAX_INTERN_WINDOW;
	messflag |= (gCase == DIEL)? 1 : 4;
    }
    if (wh > MAX_INTERN_WINDOW) {
	wh = MAX_INTERN_WINDOW;
	messflag |= (gCase == DIEL)? 2 : 8;
    }
    *WW = ceil (ww - 1e-6);
    *WH = ceil (wh - 1e-6);
}

/* Used by optCap3D.
 * In prePass1 for optSubRes and in extrPass for optCap3D.
 * Is only done once in the first pass.
 */
coor_t cap3dInitParam ()
{
    check_green = paramLookupB ("debug.check_green", "off");
    printCap3dInit = paramLookupB ("debug.print_cap3d_init", "off");

    new_microns = meters * 1e6; /* number of microns per layout unit */
    new_microns2 = new_microns * new_microns;

    messflag = 0;
    windowWidthSub = 0;
    windowWidthCap = 0;
    if (prePass1 || optSubRes) {
	get3DWindow (SUBS, &windowWidthSub, &windowHeightSub);
	ASSERT (windowWidthSub > 0);
    }
    if (optCap3DSave) {
	get3DWindow (DIEL, &windowWidthCap, &windowHeightCap);
	ASSERT (windowWidthCap > 0);
    }
    ASSERT (windowWidthSub > 0 || windowWidthCap > 0);

    serverMatrix = paramLookupB ("debug.server_matrix", "off");
    spiderControl.printMatrix = paramLookupB ("debug.print_matrix", "off");
    spiderControl.printGreen  = paramLookupB ("debug.print_green",  "off");
    spiderControl.printSpider = paramLookupB ("debug.print_spider", "off");
    printSchur = paramLookupB ("debug.print_schur", "off");
    printAllPoints = paramLookupB ("debug.print_problem_points", "off");

    if (optEstimate3D) { /* goptOnly3DMesh */
	noSchur = noGreen = 1;
    }
    else {
	noSchur = paramLookupB ("no_schur", "off") ? TRUE : FALSE;
	noGreen = paramLookupB ("no_green", "off") ? TRUE : FALSE;
    }

    /* Interpret the dimensions in spiderControl as microns
     * and convert them into internal layout units.
     * Don't convert xxxxGreenDist, these are ratio's!
     */
    Debug (fprintf (stderr, "cap3dInitParam: new scale factor: %g\n", new_microns));

    return ((coor_t)(windowWidthSub > 0 ? windowWidthSub : windowWidthCap));
}

Private void testControlWindow (int gCase)
{
    /* Only when the reduction in window size really has influence on the
       strip size and window height, we give a message.
    */
    int mflag = (gCase == DIEL)? messflag : (messflag >> 2);
    if ((mflag & 1) && spiderControl.WindowWidth * 2 < (bigbxr - bigbxl))
	say ("warning: %s.be_window width reduced to %g micron due to internal limitations.",
	    (char *)(gCase == DIEL ? "cap3d" : "sub3d"), Microns (spiderControl.WindowWidth));
    if ((mflag & 2) && spiderControl.WindowHeight < (bigbyt - bigbyb))
	say ("warning: %s.be_window height reduced to %g micron due to internal limitations.",
	    (char *)(gCase == DIEL ? "cap3d" : "sub3d"), Microns (spiderControl.WindowHeight));
}

/* Each cell can have a different scale and size.
 * cap3dInit is called for each cell to be
 * extracted to set the cell bbox size.
 * It returns the width of the band in dimensions of the cell.
 */
coor_t cap3dInit (DM_CELL *key)
{
    double ratio, split, maxlw;
    char buf[DM_MAXNAME+10];
    char *s, *mode;
    int no = 0;

    if (spiderControl.debug == NULL
    && (spiderControl.printMatrix
	|| spiderControl.printGreen
	|| spiderControl.printSpider
	|| serverMatrix
	)) {
	say ("Debugging output to `spiderDebug'\n");
	spiderControl.debug = cfopen ("spiderDebug", "w");
    }

    Debug (fprintf (stderr, "cap3dInit: cell bbox size: %g %g %g %g\n",
		D(bigbxl), D(bigbxr), D(bigbyb), D(bigbyt)));

    greenCase = prePass1 ? SUBS : DIEL;
    greenType = prePass1 ? 2 : 0;
    FeModeGalerkin = FeModePwl = FALSE;
    s = greenCase == DIEL ? "cap3d.be_mode" : "sub3d.be_mode";
    mode = paramLookupS (s, "0c");
	 if (strsame (mode, "0c")) ;
    else if (strsame (mode, "0g")) FeModeGalerkin = TRUE;
    else if (strsame (mode, "1c")) FeModePwl = TRUE;
    else if (strsame (mode, "1g")) FeModePwl = FeModeGalerkin = TRUE;
    else if (strsame (mode, "collocation")) ;
    else if (strsame (mode, "galerkin")) FeModeGalerkin = TRUE;
    else say ("warning: incorrect %s '%s' (using '0c')\n", s, mode);

    if (printCap3dInit)
	fprintf (stderr, "cap3dInit: prePass1=%d optSubRes=%d greenCase=%s greenType=%d be_mode=%s\n",
	    prePass1, optSubRes, greenCase == SUBS ? "SUBS" : "DIEL", greenType, mode);

    if (greenCase == DIEL) {
	spiderControl.WindowWidth  = windowWidthCap;
	spiderControl.WindowHeight = windowHeightCap;
	if (messflag & 3) testControlWindow (greenCase);

	if (optIntRes && !FeModePwl)
	    cap_assign_type = paramLookupI ("cap3d.cap_assign_type", "0");
	else
	    cap_assign_type = 0;
	extractGateDsCap3d = !paramLookupB ("cap3d.omit_gate_ds_cap", "off");
	extractGateGndCap3d = !paramLookupB ("cap3d.omit_gate_gnd_cap", "on");

	spiderControl.maxFeArea = paramLookupD ("cap3d.max_be_area", "-1");
	maxFeRatio = paramLookupD ("cap3d.max_be_ratio", "-1");
	if (maxFeRatio < 2) maxFeRatio = -1;
	spiderControl.minFeArea = paramLookupD ("cap3d.min_be_area", "-1");
	spiderControl.maxCoarseFeArea = paramLookupD ("cap3d.max_coarse_be_area", "-1");
	ratio = paramLookupD ("cap3d.edge_be_ratio", "1.0");
	split = paramLookupD ("cap3d.edge_be_split", "0.5");
	maxlw = paramLookupD ("cap3d.edge_be_split_lw", "4");
	/* old fashioned definition */
	step_slope = paramLookupD ("default_step_slope", "-1");
	if (step_slope == 0) {
	    paramError ("default_step_slope", "value > 0 needed");
	    no++;
	}
	else {
	    if (step_slope > 0)
		say ("warning old fashioned definition for default_step_slope");
	    else
		step_slope = 0.5;
	    sprintf (buf, "%e", 1.0 / step_slope);
	    step_slope = paramLookupD ("cap3d.default_step_slope", buf);
	    if (step_slope <= 0) {
		paramError ("cap3d.default_step_slope", "value > 0 needed");
		no++;
	    }
	}
	FeShape = FeModePwl ? 3 : atoi (paramLookupS ("cap3d.be_shape", "4"));
    }
    else {
	spiderControl.WindowWidth  = windowWidthSub;
	spiderControl.WindowHeight = windowHeightSub;
	if (messflag & 12) testControlWindow (greenCase);

	windowWidthSubFe = windowWidthSub;

	spiderControl.maxFeArea = paramLookupD ("sub3d.max_be_area", "-1");
	spiderControl.maxCoarseFeArea = -1.0;
	ratio = paramLookupD ("sub3d.edge_be_ratio", "1.0");
	split = paramLookupD ("sub3d.edge_be_split", "0.5");
	maxlw = paramLookupD ("sub3d.edge_be_split_lw", "4");
	FeShape = FeModePwl ? 3 : atoi (paramLookupS ("sub3d.be_shape", "4"));
    }

    if (spiderControl.maxFeArea <= 0) {
	paramError (greenCase == DIEL ? "cap3d.max_be_area" : "sub3d.max_be_area", "value > 0 needed");
	no++;
    }
    spiderControl.maxFeArea /= new_microns2;
    spiderControl.maxEdgeFeArea = spiderControl.maxFeArea;

    if (Nearby (ratio, 1)) ratio = 1;
    else if (ratio <= 0 || ratio > 1) {
	paramError (greenCase == DIEL ? "cap3d.edge_be_ratio" : "sub3d.edge_be_ratio", "value > 0 and <= 1.0 needed");
	no++;
    }
    else spiderControl.maxEdgeFeArea *= ratio;

    if (Nearby (split, 0.5)) split = 0.5;
    else if (split <= 0 || split > 0.5) {
	paramError (greenCase == DIEL ? "cap3d.edge_be_split" : "sub3d.edge_be_split", "value > 0 and <= 0.5 needed");
	no++;
    }
    spiderControl.edgeSplitRatio = split;

    if (no) { say ("Can't continue, stop"); die (); }

    if (FeShape != 3 && FeShape != 4) { say ("be_shape != 4 and != 3, set to 4"); FeShape = 4; }

    if (maxlw < 2) { say ("edge_be_split_lw < 2, set to 2"); maxlw = 2; }
    spiderControl.maxEdgeFeRatio = maxlw * split;

    if (spiderControl.minFeArea < 0) spiderControl.minFeArea = 1;
    else {
	spiderControl.minFeArea /= new_microns2;
	if (spiderControl.minFeArea * 16 > spiderControl.maxFeArea) spiderControl.minFeArea = spiderControl.maxFeArea / 16;
    }
    if (spiderControl.maxCoarseFeArea > 0) spiderControl.maxCoarseFeArea /= new_microns2;

    /*
     * Also split faces that have bad aspect ratio.
     */
#define MaxLen(area) MAX_NORM_SPIDER_LEN * sqrt (area)
    spiderControl.maxSpiderLength = MaxLen (spiderControl.maxFeArea);
    if (spiderControl.maxCoarseFeArea > 0)
        spiderControl.maxCoarseSpiderLength = MaxLen (spiderControl.maxCoarseFeArea);
    else
        spiderControl.maxCoarseSpiderLength = spiderControl.maxSpiderLength;

    if (printCap3dInit) {
	fprintf (stderr, "cap3dInit: be_window (width,height) = %g,%g\n",
	    D(spiderControl.WindowWidth), D(spiderControl.WindowHeight));
	fprintf (stderr, "cap3dInit: max_be_area=%g max_edge_be_area=%g max_coarse_be_area=%g\n",
	    spiderControl.maxFeArea, spiderControl.maxEdgeFeArea, spiderControl.maxCoarseFeArea);
	if (greenCase == DIEL) fprintf (stderr, "cap3dInit: min_be_area=%g\n", spiderControl.minFeArea);
	fprintf (stderr, "cap3dInit: edge_be_ratio=%g edge_be_split=%g edge_be_split*lw=%g\n",
	    ratio, split, spiderControl.maxEdgeFeRatio);
    }

    /* FVF: init spider hashing */
    if (spiderHashInitTable () < 0) { fprintf (stderr, "cap3dInit: spiderHashInit problem\n"); die(); }
    spiderDelayedDisplaceInit ();

    /* initialize the strip module
     */
    stripLeft  = bigbxl;
    stripRight = (coor_t) Min (bigbxl + spiderControl.WindowWidth, bigbxr);
    if (printCap3dInit)
	fprintf (stderr, "cap3dInit: stripInit(stripLeft=%g stripRight=%g)\n", D(stripLeft), D(stripRight));
    stripInit (stripLeft, stripRight);

    /* initialize the math error handler module */
    mathInit ();

    /* initialize the greens function evaluation module */
    greenFactor = greenInit (meters);
    greenFactor2 = greenFactor * greenFactor / Epsilon0;

    /* initialize the cbemdata stream */
    cbemdataInit (key);

    /* set variables for statistics. */
    MaxDimension   = 0;
    MaxOrder       = 0;
    brutoNumGreens = 0;

    numJoins       = 0;
    numFaces       = 0;
    numVertices    = 0;
    numGreens      = 0;
    said = 0;

    MaxArea        = 0;
    MaxDist        = 0;
    MaxLength      = 0;
    MaxLengthRatio = 0;
    TotalArea      = 0;
    MinArea        = 1e99; /* INF */
    MinDist        = 1e99; /* INF */
    MinLength      = 1e99; /* INF */

    doStripDrawAndPrint = FALSE;
    if (spiderControl.printSpider ||
#ifdef DISPLAY
	goptDrawSpider ||
#endif
	cbemStream) doStripDrawAndPrint = TRUE;

    /* Return the  bandwidth for the scanline module, so that
     * it retains the tiles long enough.
     * It is necessary return 4 times the window width,
     * because of the mesh-ahead and mesh-behind.
     */
    return (4 * (coor_t) spiderControl.WindowWidth);
}

coor_t cap3dStart ()
{
    cntStrip = 0;
    if (prePass1 && max_fe_count > 1) {
	sub_fe_count = 1;
	nextStrip = stripLeft + windowWidthSubFe;
	if (nextStrip < stripRight) return (nextStrip);
    }
    return (stripRight);
}

void cap3dStop ()
{
    spiderDisplaceNow ();
    meshRefine ();

    doStrip (DOUBLE_STRIP);
    stripStop ();
    mathStop ();
    cbemdataQuit ();
}

void cap3dEnd ()
{
    sub_fe_count = 0;

    spiderHashFreeTable ();

    printIfNoconverge ();
}

/* This one is called from the scanline
 */
coor_t cap3dStrip ()
{
    if (prePass1 && sub_fe_count) {
        if (++sub_fe_count < max_fe_count) {
            nextStrip += windowWidthSubFe;
            if (nextStrip < stripRight) return (nextStrip);
        }
        sub_fe_count = 0;
        return (stripRight);
    }

    Debug (fprintf (stderr, "scanX = %d\n", (int) stripRight));

    spiderDisplaceNow ();
    meshRefine ();

    /* If there is only one strip, do one double-band traversal,
     * with an empty left band.
     * If there are more strips, do one double-band traversal
     * for all but the first strip and a single-band traversal
     * for all but the first and last strip.
     * Note that a strip must first be processed as single strip
     * and then with its left neighbor as a double strip,
     * because the caching of green's function relies on this fact.
     */

    cntStrip++;

    /* deze ligt 1 strip achterop bij scanline v.w. meshahead */
    if (cntStrip > 2) {
	doStrip (SINGLE_STRIP);
	doStrip (DOUBLE_STRIP);
    }

    /* Move the strip.
     */
    stripLeft  = stripRight;
    stripRight = (coor_t) Min (bigbxr, stripRight + spiderControl.WindowWidth);
    stripMove (stripLeft, stripRight);

    if (stripLeft < bigbxr) {
	if (prePass1 && max_fe_count > 1) {
	    sub_fe_count = 1;
	    nextStrip = stripLeft + windowWidthSubFe;
	    if (nextStrip < stripRight) return (nextStrip);
	}
	return (stripRight);
    }
    return (INF);
}

coor_t findStripForCapStart (coor_t scanX)
{
    coor_t x = (coor_t) Min (bigbxl + windowWidthCap, bigbxr);
    while (x <= scanX) x = findStripForCapNext (x);
    return x;
}

coor_t findStripForSubStart (coor_t scanX)
{
    coor_t x = (coor_t) Min (bigbxl + windowWidthSubFe, bigbxr);
    if (max_fe_count > 1) sub_fe_count = 1;
    while (x <= scanX) x = findStripForSubNext (x);
    return x;
}

coor_t findStripForCapNext (coor_t scanX)
{
    if (scanX < bigbxr)
	scanX = (coor_t) Min (bigbxr, scanX + windowWidthCap);
    else
	scanX = INF;
    return (scanX);
}

coor_t findStripForSubNext (coor_t scanX)
{
    if (scanX < bigbxr) {
	scanX += windowWidthSubFe;
	if (max_fe_count > 1 && ++sub_fe_count == max_fe_count) {
	    sub_fe_count = 0;
	    scanX += windowWidthSubFe2;
	}
	if (scanX > bigbxr) scanX = bigbxr;
    }
    else
	scanX = INF;
    return (scanX);
}

coor_t findStripForSubNextY (coor_t y)
{
    static int sub_fe_count_y;
    if (y == bigbyb) sub_fe_count_y = 0;
    y += windowWidthSubFe;
    if (max_fe_count > 1 && ++sub_fe_count_y == max_fe_count) {
	sub_fe_count_y = 0;
	y += windowWidthSubFe2;
    }
    return (y);
}

Private void cbemdataInit (DM_CELL *key)
{
    if (paramLookupB ("cbemdata_output", "off")) {
	cbemStream = dmOpenStream (key, "cbemdata", "w");
    }
}

Private void cbemdataQuit ()
{
    if (cbemStream) { dmCloseStream (cbemStream, COMPLETE); cbemStream = NULL; }
}

Private void doStrip (int mode)
{
    int row, col;

    stripVerbose (mode);

    /* stripDrawAndPrint will make sure a strip is drawn/printed only once,
     * and only if appropriate options are set
     */
    if (doStripDrawAndPrint) stripDrawAndPrint ();

    if (mode == DOUBLE_STRIP)
	schurdata.capSign =  1;	/* add */
    else
	schurdata.capSign = -1;	/* subtract */

if (printSchur) {
    fprintf (stderr, "doStrip: %s\n", mode == DOUBLE_STRIP ? "DOUBLE_STRIP" : "SINGLE_STRIP");
    fprintf (stderr, "doStrip: ===computeMatrixSize=============\n");
}
    maxRow = -1;
    maxCol = -1;
    stripTraverse ((mode == DOUBLE_STRIP) ? 1 : 0, &maxRow, &maxCol);

if (printSchur) {
    for (row = 0; row <= maxRow; ++row) {
	col = spidercols[row];
	fprintf (stderr, "doStrip: -- row %3d   cols %3d  total %d\n", row, col, row + col + 1);
    }
	fprintf (stderr, "doStrip: maxRow %3d maxCol %3d\n", maxRow, maxCol);
}

    MaxDimension = Max (MaxDimension, maxRow + 1);
    MaxOrder     = Max (MaxOrder, maxCol);

    if (noGreen) return;

    if (maxRow >= 0) {
	tick ("computeCapacitance");

	schurStartMatrix (maxRow, maxCol);

	for (row = 0; row <= maxRow; ++row) {
	    for (col = 0; col <= spidercols[row]; ++col) {
		computeCapacitance (mode, row, col, spiderlist[row], spiderlist[row+col]);
	    }
	}

	schurStopMatrix ();

	tock ("computeCapacitance");
    }
}

/* Begin a new matrix.
 * First make sure that buf is big enough,
 * reset some variables and prepare the schur module
 * for a new matrix.
 */
Private void schurStartMatrix (int row, int col)
{
    verbose ("Schur dimension %d, maxorder %d\n", row + 1, col);

    if (noSchur) return;

    ASSERT (row >= 0);
    ASSERT (col >= 0);

    if (col > schurdata.maxcol || !schurdata.buf) {
	int size = (col / 256 + 1) * 256;
	if (schurdata.buf) DISPOSE (schurdata.buf, sizeof(schur_t) * (schurdata.maxcol+1));
	schurdata.buf = NEW (schur_t, size);
	schurdata.maxcol = size - 1;
    }

    schurdata.row = 0;
    schurdata.col = 0;

    initSchur (row, col);
    if (serverMatrix) fprintf (spiderControl.debug, "%d\n", col);
}

/* Matrix is ready.
 * Send the last row to the schur module.
 */
Private void schurStopMatrix ()
{
    if (noSchur) return;

    ASSERT (schurdata.col == 0);

    if (serverMatrix) {
	fprintf (spiderControl.debug, "%d ", schurdata.col);
	printUpperMatrix (spiderControl.debug, schurdata.row, schurdata.buf, schurdata.col);
	fprintf (spiderControl.debug, "-1\n");
    }
    if (spiderControl.printMatrix) {
	fprintf (spiderControl.debug, "row_in %d %d ", schurdata.row, schurdata.col);
	printUpperMatrix (spiderControl.debug, schurdata.row, schurdata.buf, schurdata.col);
    }

if (printSchur) fprintf (stderr, "schurStopMatrix: schurRowIn row=%d col=%d\n", schurdata.row, schurdata.col);
    tock ("other");
    tick ("schurRowIn");
    if (optAlarmInterval < 0) catchAlarm ();
    schurRowIn (schurdata.row, schurdata.buf, schurdata.col);
    tock ("schurRowIn");
    tick ("other");
}

Private void computeCapacitance (int mode, int row, int col, spider_t *s1, spider_t *s2)
{
    schur_t val = doGreen (mode, s1, s2);

    brutoNumGreens++;

    if (greenCase == DIEL) val /= Epsilon0;

    if (!noSchur) schurInput (row, col, val, s1, s2);

#ifdef DISPLAY
    if (goptDrawGreen) {
	if (greenCase == DIEL)
	    drawGreen (D(s1 -> act_x), D(s1 -> act_y), D(s1 -> act_z),
		       D(s2 -> act_x), D(s2 -> act_y), D(s2 -> act_z));
	else
	    drawGreen (D(s1 -> act_x), D(s1 -> act_y), D(0),
		       D(s2 -> act_x), D(s2 -> act_y), D(0));
    }
#endif

    if (spiderControl.printGreen)
	fprintf (spiderControl.debug, "green %g %g %g %g %g %g %g\n",
	    D(s1 -> act_x), D(s1 -> act_y), D(s1 -> act_z),
	    D(s2 -> act_x), D(s2 -> act_y), D(s2 -> act_z), D(val));
}

Private void schurInput (int row, int col, schur_t val, spider_t *s1, spider_t *s2)
{
    ASSERT (col <= schurdata.maxcol);

    if (row != schurdata.row) {
	ASSERT (row == schurdata.row + 1);

	if (serverMatrix) {
	    fprintf (spiderControl.debug, "%d ", schurdata.col);
	    printUpperMatrix (spiderControl.debug, row-1, schurdata.buf, schurdata.col);
	}
	if (spiderControl.printMatrix) {
	    fprintf (spiderControl.debug, "row_in %d %d ", row-1, schurdata.col);
	    printUpperMatrix (spiderControl.debug, row-1, schurdata.buf, schurdata.col);
	}

if (printSchur) fprintf (stderr, "schurInput: schurRowIn row=%d col=%d\n", schurdata.row, schurdata.col);
	tick ("schurRowIn");
	if (optAlarmInterval < 0) catchAlarm ();
	schurRowIn (schurdata.row, schurdata.buf, schurdata.col);
	tock ("schurRowIn");

	schurdata.row = row;
    }

    schurdata.buf[col] = val;
    schurdata.col = col;

    if (col > 0 && val >= schurdata.buf[0]) {
	spiderEdge_t *edge;
      if (++said == 1) {
	say ("Inconsistent value in influence matrix, probably due to too large");
	say ("\tmesh granularity (large differences in element sizes).");
	say ("\tExtraction results will be incorrect.");
	say ("\tIt concerns the following mesh points (coordinates are in microns,");
	say ("\tarea sizes in square microns):");
	say ("\t  point 1: mask=%s x=%g y=%g z=%g", conNr2Name (s1 -> conductor),
		Microns (s1 -> act_x), Microns (s1 -> act_y), Microns (s1 -> act_z));
	if (s1 -> face)
	    say ("\t  connected to element with size %g", Microns2 (s1 -> face -> area));
	else {
	    say ("\t  connected to elements with sizes:");
	    for (edge = s1 -> edge; edge; edge = NEXT_EDGE (s1, edge))
		if (edge -> face) say ("\t    %g", Microns2 (edge -> face -> area));
	}
	say ("\t  point 2: mask=%s x=%g y=%g z=%g", conNr2Name (s2 -> conductor),
		Microns (s2 -> act_x), Microns (s2 -> act_y), Microns (s2 -> act_z));
	if (s2 -> face)
	    say ("\t  connected to element with size %g", Microns2 (s2 -> face -> area));
	else {
	    say ("\t  connected to elements with sizes:");
	    for (edge = s2 -> edge; edge; edge = NEXT_EDGE (s2, edge))
		if (edge -> face) say ("\t    %g", Microns2 (edge -> face -> area));
	}
      }
      else if (printAllPoints) {
	say ("Inconsistent value %d in influence matrix, it concerns points:", said);
	if (s1 -> face)
	    say ("\t  point 1: mask=%s x=%g y=%g z=%g element-size: %g", conNr2Name (s1 -> conductor),
		Microns (s1 -> act_x), Microns (s1 -> act_y), Microns (s1 -> act_z), Microns2 (s1 -> face -> area));
	else
	    say ("\t  point 1: mask=%s x=%g y=%g z=%g", conNr2Name (s1 -> conductor),
		Microns (s1 -> act_x), Microns (s1 -> act_y), Microns (s1 -> act_z));
	if (s2 -> face)
	    say ("\t  point 2: mask=%s x=%g y=%g z=%g element-size: %g", conNr2Name (s2 -> conductor),
		Microns (s2 -> act_x), Microns (s2 -> act_y), Microns (s2 -> act_z), Microns2 (s2 -> face -> area));
	else
	    say ("\t  point 2: mask=%s x=%g y=%g z=%g", conNr2Name (s2 -> conductor),
		Microns (s2 -> act_x), Microns (s2 -> act_y), Microns (s2 -> act_z));
      }
    }
}

void schurRowOut (int row, schur_t *buf, int ncols)
{
    int col;
    spider_t *s1, *s2;

if (printSchur) fprintf (stderr, "schurRowOut row=%d ncols=%d\n", row, ncols);
    if (spiderControl.printMatrix) {
	fprintf (spiderControl.debug, "row_out %d %d ", row, ncols);
	printUpperMatrix (spiderControl.debug, row, buf, ncols);
    }

    s1 = spiderlist[row];
    ASSERT (ncols == spidercols[row]);

    for (col = 0; col <= ncols; col++) {
	s2 = spiderlist[row+col];
	addCap (s1, s2, buf[col]);
    }
}

Private void cap3dAdd (spider_t *s1, subnode_t *sn2, schur_t v)
{
    spider_t *sp1, *sp2, *sp3, *sp4;

    sp1 = s1 -> face -> corners[0];
    sp3 = s1 -> face -> corners[2];
    if (sp1 -> subnode == sp3 -> subnode) {
	capAdd (sp1 -> subnode, sn2, v, 0);
    }
    else {
	sp2 = s1 -> face -> corners[1];
	sp4 = s1 -> face -> corners[3];
	if (sp4) v /= 4;
	else     v /= 3;
	capAdd (sp1 -> subnode, sn2, v, 0);
	capAdd (sp2 -> subnode, sn2, v, 0);
	capAdd (sp3 -> subnode, sn2, v, 0);
	if (sp4) capAdd (sp4 -> subnode, sn2, v, 0);
    }
}

#define hasCap2(sp) (sp -> isGate & (1 << 29))
#define hasDiff(sp) (sp -> isGate & (1 << 30))
#define hasGate(sp) (sp -> isGate & (1 << 31))

Private int extractGnd (spider_t *s1)
{
    if (s1 -> isGate) {
	if (hasDiff (s1)) return extractDiffusionCap3d;
	else if (hasGate (s1)) return extractGateGndCap3d;
    }
    return 1;
}

Private int extractCoup (spider_t *s1, spider_t *s2)
{
    if (s1 -> isGate) {
	if (hasCap2 (s1) || hasCap2 (s2)) return 0;
	if (hasDiff (s1)) {
	    if (hasDiff (s2)) return extractDiffusionCap3d;
	    else if (hasGate (s2)) return extractGateDsCap3d;
	}
	else if (hasGate (s1))
	    if (hasDiff (s2)) return extractGateDsCap3d;
    }
    else if (hasCap2 (s2)) return 0;
    return 1;
}

/* Add the capacitance present between s1 and s2 to the network.
 * Cap is an entry from the capacitance matrix
 * (inverse of elastance matrix)
 *
 * If s1 == s2, this is a diagonal element
 * Take care of diffusion conductors.
 *
 * Note about: capAdd (subnode, subnode2, val, 0)
 *	If subnode2 == NULL, subnGND is used, because subnGND = NULL!
 *
 * Used by optCap3D.
 * In prePass1 for optSubRes and in extrPass for optCap3D.
 */
Private void addCap (spider_t *s1, spider_t *s2, schur_t val)
{
    val *= schurdata.capSign;
    val *= greenFactor; /* multiply the value, see spider/init.c */

    if (s1 == s2) {
	if (!extractWindow (s1)) return;

	ASSERT (s1 -> subnode);

	if (prePass1) {
	    s1 -> subnode -> node -> substrCon[0] += val;
	    return;
	}
	if (extractGnd (s1)) {
	    if (cap_assign_type > 0)
		cap3dAdd (s1, s1 -> subnode2, val);
	    else
		capAdd (s1 -> subnode, s1 -> subnode2, val, 0);
	}
    }
    else {
	bool_t ew1, ew2;
	subnode_t *subn1, *subn2;

	ew1 = extractWindow (s1);
	ew2 = extractWindow (s2);
	if (!ew1 && !ew2) return;

	subn1 = s1 -> subnode;
	subn2 = s2 -> subnode;
	ASSERT (subn1 && subn2);

	if (prePass1) {
	    if (subn1 -> node != subn2 -> node) conAddS (subn1, subn2, -val);
	    if (ew1) subn1 -> node -> substrCon[0] += val;
	    if (ew2) subn2 -> node -> substrCon[0] += val;
	    return;
	}
	if (subn1 -> node != subn2 -> node || cap_assign_type > 0) {
	    if (extractCoup (s1, s2)) {
		if (cap_assign_type > 0) {
		    spider_t *sp1, *sp2, *sp3, *sp4;
		    schur_t v;

		    if (cap_assign_type > 2) {
			sp1 = s1 -> face -> corners[3];
			sp2 = s2 -> face -> corners[3];
			if ((sp1 && !sp2) || (!sp1 && sp2)) goto type_1_2;
			v = sp1 ? -val / 4 : -val / 3;
			if (sp1) capAdd (sp1 -> subnode, sp2 -> subnode, v, 0);
			sp1 = s1 -> face -> corners[0];
			sp2 = s2 -> face -> corners[0];
			capAdd (sp1 -> subnode, sp2 -> subnode, v, 0);
			sp1 = s1 -> face -> corners[1];
			sp2 = s2 -> face -> corners[1];
			capAdd (sp1 -> subnode, sp2 -> subnode, v, 0);
			sp1 = s1 -> face -> corners[2];
			sp2 = s2 -> face -> corners[2];
			capAdd (sp1 -> subnode, sp2 -> subnode, v, 0);
		    }
		    else {
type_1_2:
			sp1 = s2 -> face -> corners[0];
			sp3 = s2 -> face -> corners[2];
			if (sp1 -> subnode == sp3 -> subnode) {
			    cap3dAdd (s1, sp1 -> subnode, -val);
			}
			else {
			    sp2 = s2 -> face -> corners[1];
			    sp4 = s2 -> face -> corners[3];
			    v = sp4 ? -val / 4 : -val / 3;
			    cap3dAdd (s1, sp1 -> subnode, v);
			    cap3dAdd (s1, sp2 -> subnode, v);
			    cap3dAdd (s1, sp3 -> subnode, v);
			    if (sp4) cap3dAdd (s1, sp4 -> subnode, v);
			}
		    }
		}
		else
		    capAdd (subn1, subn2, -val, 0);
	    }
	}
	if (ew1 && extractGnd (s1)) {
	    if (cap_assign_type > 0)
		cap3dAdd (s1, s1 -> subnode2, val);
	    else
		capAdd (subn1, s1 -> subnode2, val, 0);
	}
	if (ew2 && extractGnd (s2)) {
	    if (cap_assign_type > 0)
		cap3dAdd (s2, s2 -> subnode2, val);
	    else
		capAdd (subn2, s2 -> subnode2, val, 0);
	}
    }
}

void cap3dEstimate (FILE *fp)
{
    fprintf (fp, "cap3dEstimate:\n");
    fprintf (fp, "\tnumber of elements  : %d\n",
        (FeModePwl ? numVertices - numFaces : numFaces));
}

void cap3dStatistics (FILE *fp)
{
    fprintf (fp, "overall cap3d statistics:\n");
    fprintf (fp, "\tnumber of elements  : %d\n",
	FeModePwl ? numVertices - numFaces : numFaces);
    fprintf (fp, "\tnumber of vertices  : %d\n",
	FeModePwl ? numVertices : numVertices - numFaces);
    fprintf (fp, "\tnumber of faces     : %d\n", numFaces);
    fprintf (fp, "\tnumber of greens    : %d\n", numGreens);
    fprintf (fp, "\tbruto num. of greens: %d\n", brutoNumGreens);
    fprintf (fp, "\tmax matrix dimension: %d\n", MaxDimension);
    fprintf (fp, "\tmax matrix order    : %d\n", MaxOrder);

    fprintf (fp, "\tmin element dist    : %g\t(microns)\n",  Microns (MinDist));
    fprintf (fp, "\tmax element dist    : %g\t(microns)\n",  Microns (MaxDist));
    fprintf (fp, "\tmin element area    : %g\t(microns^2)\n", Microns2 (MinArea));
    fprintf (fp, "\tmax element area    : %g\t(microns^2)\n", Microns2 (MaxArea));
    fprintf (fp, "\tavg element area    : %g\t(microns^2)\n", Microns2 (TotalArea / numFaces));
    fprintf (fp, "\tmin edge length     : %g\t(microns)\n",  Microns (MinLength));
    fprintf (fp, "\tmax edge length     : %g\t(microns)\n",  Microns (MaxLength));
    fprintf (fp, "\tmax ratio of lengths: %g\n", MaxLengthRatio);
    fprintf (fp, "\tjoined spiders      : %d\n", numJoins);
    hashStatistics (fp);
}

void cap3dUpdateStatistics (spider_t *sp1, spider_t *sp2)
{
    spiderEdge_t *edge;
    face_t *face;
    double area, dist, lmin, lmax;

    numGreens++;

    if (sp1 != sp2) {
	dist = spiderDist (sp1, sp2);
	if (dist < MinDist) MinDist = dist;
	if (dist > MaxDist) MaxDist = dist;
    }
    else if (FeModePwl) {
	for (edge = sp1 -> edge; edge; edge = NEXT_EDGE (sp1, edge))
	if ((face = edge -> face)) {
	    area = face -> area;
	    if (area < MinArea) MinArea = area;
	    if (area > MaxArea) MaxArea = area;

	    lmin = face -> len2;
	    if (lmin < MinLength) MinLength = lmin;
	    lmax = face -> len;
	    if (lmax > MaxLength) MaxLength = lmax;
	    lmax /= lmin;
	    if (lmax > MaxLengthRatio) MaxLengthRatio = lmax;
	}
    }
    else {
	face = sp1 -> face;
	ASSERT (face);

	area = face -> area;
	if (area < MinArea) MinArea = area;
	if (area > MaxArea) MaxArea = area;
	TotalArea += area;

	lmin = face -> len2;
	if (lmin < MinLength) MinLength = lmin;
	lmax = face -> len;
	if (lmax > MaxLength) MaxLength = lmax;
	lmax /= lmin;
	if (lmax > MaxLengthRatio) MaxLengthRatio = lmax;
    }
}

Private bool_t extractWindow (spider_t *s)
{
    static int state = 0;
    static coor_t xl, xr, yb, yt;

    if (state == 1) return TRUE;
    if (state == 0) {
	char * param;
	state = 1;
	if ((param = paramLookupS ("extract_window", (char *) NULL))) {
	    if (sscanf (param, "%d %d %d %d", &xl, &xr, &yb, &yt) == 4) {
		state = 2;
		xl *= outScale;
		xr *= outScale;
		yb *= outScale;
		yt *= outScale;
	    }
	    else say ("array_instance syntax error");
	}
	if (state == 1) return TRUE;
    }

    if (s -> nom_x >= xl && s -> nom_x <= xr
     && s -> nom_y >= yb && s -> nom_y <= yt) return TRUE;
    return FALSE;
}
