/*
 * ISC License
 *
 * Copyright (C) 2004-2018 by
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

#include "src/space/makesubres/define.h"
#include "src/space/makesubres/extern.h"

#ifdef __cplusplus
  extern "C" {
#endif

/* local operations */
Private void doStrip            (int mode);
Private void computeConductance (int mode, int row, int col, spider_t *s1, spider_t *s2);
Private void schurInput       (int row, int col, schur_t val, spider_t *s1, spider_t *s2);
Private void schurStartMatrix (int row, int col);
Private void schurStopMatrix  (void);
Private void addCon           (spider_t *s1, spider_t *s2, schur_t val);
Private void updateStatistics (spider_t *s1, spider_t *s2);

#ifdef __cplusplus
  }
#endif

/* Since the window size will be stored in a coor_t,
   we must guarantee that it is not too large.
   Therefore, the following parameter defines the
   maximum (internal) window size.
*/
#define MAX_INTERN_WINDOW    100000000
#define MAX_NORM_SPIDER_LEN  2 	/* 2 times sqrt area */

extern int *spidercols;
extern spider_t **spiderlist;
int maxRow;
int maxCol;

bool_t FeModePwl = FALSE;
bool_t FeModeGalerkin = FALSE;

static bool_t  noSchur; 	/* debugging */
static bool_t  noGreen; 	/* debugging */
static double greenFactor = 1.0;
#ifdef DEBUG
static bool_t  serverMatrix = FALSE;
#endif

static meshCoor_t MinArea;
static meshCoor_t MinDist;
static meshCoor_t MinLength;
static meshCoor_t MaxArea;
static meshCoor_t MaxDist;
static meshCoor_t MaxLength;
static meshCoor_t MaxLengthRatio;
static meshCoor_t TotalArea;

static int brutoNumGreens = 0;
static int MaxDimension = 0;
static int MaxOrder = 0;
static int ww_too_big = 0;
static int wh_too_big = 0;

struct spiderControl spiderControl;
int numFaces    = 0;
int numVertices = 0;
int numGreens   = 0;

int greenCase   = SUBS;
int greenType   = 2; /* 2 = sub3d, G; 3 = sub3d, dG/dn. */
int FeShape = 0;

extern int makesubcap;

extern coor_t thisX;
static coor_t windowWidthFe;
static coor_t nextStrip;
static coor_t stripLeft  = 0;
static coor_t stripRight = 0;
static int    cntStrip;

static struct schurdata {
    int row, col, maxcol;
    schur_t * buf;
    schur_t  sign;
} schurdata;

static bool_t printSub3dInit;
static int numSqrtDomainErrors;
static int said = 0;

static double microns;
static double microns2;

void mathInit ()
{
    numSqrtDomainErrors = 0;
}

void mathStop ()
{
    if (numSqrtDomainErrors > 0)
	say ("%d domain errors in sqrt",  numSqrtDomainErrors);
}

void init_param_sub3d (char *k, char *v)
{
    if (v || (v = paramLookupS (mprintf ("subcap3d.%s", k), v)))
	paramSetOption2 (mprintf ("sub3d.%s", k), v);
}

void init_sub3d_params ()
{
    /* MAKESUBRES */
 // init_param_sub3d ("be_window", NULL);
    init_param_sub3d ("max_be_area", NULL);
    init_param_sub3d ("edge_be_ratio", NULL);
    init_param_sub3d ("edge_be_split", NULL);
    init_param_sub3d ("edge_be_split_lw", NULL);
    init_param_sub3d ("be_shape", NULL);

    /* GREEN module */
    init_param_sub3d ("use_unigreen", "off");
 // init_param_sub3d ("use_unigreen_interpolation", NULL);
 // init_param_sub3d ("test_unigreen", NULL);
    init_param_sub3d ("use_mean_green_values", NULL);
    init_param_sub3d ("merge_images", NULL);
    init_param_sub3d ("mp_min_dist", NULL);
    init_param_sub3d ("mp_max_order", NULL);
    init_param_sub3d ("saw_dist", NULL);
    init_param_sub3d ("edge_dist", NULL);
    init_param_sub3d ("be_mode", NULL);
    init_param_sub3d ("use_lowest_medium", NULL);
    init_param_sub3d ("use_old_images", NULL);
    init_param_sub3d ("neumann_simulation_ratio", NULL);
    init_param_sub3d ("point_green", NULL);
    init_param_sub3d ("collocation_green", NULL);
    init_param_sub3d ("asym_collocation_green", NULL);
    init_param_sub3d ("max_green_terms", NULL);
    init_param_sub3d ("green_eps", NULL);
    init_param_sub3d ("col_rel_eps", NULL);
}

void sub3dInitParam ()
{
    double ratio, split, maxlw, ww, wh;
    char buf1[132], buf2[132], *val, *param;
    int n, no = 0;

    if (makesubcap) init_sub3d_params ();

    printSub3dInit = paramLookupB ("debug.print_cap3d_init", "off");

    /* The parameter dimensions are specified in microns
     * and convert them into internal layout units.
     * meters specifies the number of meters per layout unit.
     * microns is used to convert the dimensions into layout units.
     */
    microns = meters * 1e6;
    microns2 = microns * microns;

    greenCase = SUBS;
    greenType = 2;

    FeModeGalerkin = FeModePwl = FALSE;
    val = paramLookupS ("sub3d.be_mode", "0c");
         if (strsame (val, "0c")) ;
    else if (strsame (val, "0g")) FeModeGalerkin = TRUE;
    else if (strsame (val, "1c")) FeModePwl = TRUE;
    else if (strsame (val, "1g")) FeModePwl = FeModeGalerkin = TRUE;
    else if (strsame (val, "collocation")) ;
    else if (strsame (val, "galerkin")) FeModeGalerkin = TRUE;
    else say ("warning: incorrect sub3d.be_mode '%s' (using '0c')\n", val);

    if (printSub3dInit) {
	fprintf (stderr, "sub3dInit: scalefactor microns=%g\n", microns);
	fprintf (stderr, "sub3dInit: greenCase=SUBS greenType=%d\n", greenType);
    }

    ww = wh = 0;

    param = "sub3d.be_window";
    if ((val = paramLookupS (param, Null(char *)))) {
	buf1[0] = '\0'; buf2[0] = '\0';
	if ((n = sscanf (val, "%s %s", buf1, buf2)) > 0) {
	    if (strsame (buf1, "inf") || strsame (buf1, "infinite")) ww = 9e99;
	    else { sscanf (buf1, "%le", &ww); ww /= microns; }
	    if (n > 1) {
		if (strsame (buf2, "inf") || strsame (buf2, "infinite")) wh = 9e99;
		else { sscanf (buf2, "%le", &wh); wh /= microns; }
	    }
	    else wh = ww;
	}
	if (ww <= 0.0) { paramError (param, "width value must be > 0"); no++; }
	if (wh <= 0.0) { paramError (param, "height value must be > 0"); no++; }
    }
    else { say ("parameter %s must be specified", param); no++; }


    if (ww > MAX_INTERN_WINDOW) {
	spiderControl.WindowWidth = MAX_INTERN_WINDOW;
	ww_too_big = 1;
    }
    else {
	spiderControl.WindowWidth  = ww > 0 ? ceil (ww - 1e-6) : 0;
	ww_too_big = 0;
    }

    if (wh > MAX_INTERN_WINDOW) {
	spiderControl.WindowHeight = MAX_INTERN_WINDOW;
	wh_too_big = 1;
    }
    else {
	spiderControl.WindowHeight = wh > 0 ? ceil (wh - 1e-6) : 0;
	wh_too_big = 0;
    }

    spiderControl.maxFeArea = paramLookupD ("sub3d.max_be_area", "-1");
    ratio = paramLookupD ("sub3d.edge_be_ratio", "1");
    split = paramLookupD ("sub3d.edge_be_split", "0.5");
    maxlw = paramLookupD ("sub3d.edge_be_split_lw", "4");

    if (spiderControl.maxFeArea <= 0) {
	paramError ("max_be_area", "value > 0 needed");
	no++;
    }
    if (ratio <= 0) {
	paramError ("edge_be_ratio", "value > 0 needed");
	no++;
    }
    spiderControl.maxEdgeFeArea = spiderControl.maxFeArea * ratio;

    if (split <= 0.0 || split >= 1.0) {
	paramError ("edge_be_split", "value > 0 and < 1 needed");
	no++;
    }
    spiderControl.edgeSplitRatio = split;

    if (maxlw < 2.0) {
	say ("edge_be_split_lw < 2, set to 2");
	maxlw = 2.0;
    }
    spiderControl.maxEdgeFeRatio = maxlw;

#ifdef DEBUG
    serverMatrix = paramLookupB ("debug.server_matrix", "off");
    spiderControl.printMatrix = paramLookupB ("debug.print_matrix", "off");
    spiderControl.printGreen  = paramLookupB ("debug.print_green",  "off");
    spiderControl.printSpider = paramLookupB ("debug.print_spider", "off");
#endif

    if (optEstimate3D) noSchur = noGreen = 1;
    else {
	noSchur = paramLookupB ("no_schur", "off");
	noGreen = paramLookupB ("no_green", "off");
    }

    spiderControl.maxFeArea     /= microns2;
    spiderControl.maxEdgeFeArea /= microns2;

    /* Also split faces that have bad aspect ratio.
     */
    spiderControl.maxSpiderLength = MAX_NORM_SPIDER_LEN * sqrt (spiderControl.maxFeArea);

    if (printSub3dInit) {
	fprintf (stderr, "sub3dInit: be_window (width,height) = %g,%g\n",
	    D(spiderControl.WindowWidth), D(spiderControl.WindowHeight));
	fprintf (stderr, "sub3dInit: max_be_area=%g max_edge_be_area=%g\n",
	    spiderControl.maxFeArea, spiderControl.maxEdgeFeArea);
	fprintf (stderr, "sub3dInit: edge_be_ratio=%g edge_be_split=%g edge_be_split_lw=%g\n",
	    ratio, split, maxlw);
    }

    if (no) { say ("Can't continue, stop"); die (); }
}

coor_t sub3dInit ()
{
    sub3dInitParam ();

#ifdef DEBUG
    if (!spiderControl.debug
	&& (spiderControl.printMatrix || spiderControl.printGreen
		|| spiderControl.printSpider || serverMatrix)) {
	say ("Debugging output to ``spiderDebug''\n");
	spiderControl.debug = cfopen ("spiderDebug", "w");
    }
#endif

    /* Only when the reduction in window size really has influence on the
       strip size and window height, we give a message.
    */
    if (ww_too_big && spiderControl.WindowWidth * 2 < (bbxr - bbxl))
	say ("warning: BE window width reduced to %g micron due to internal limitations.",
		spiderControl.WindowWidth * microns);

    if (wh_too_big && spiderControl.WindowHeight < (bbyt - bbyb))
	say ("warning: BE window height reduced to %g micron due to internal limitations.",
		spiderControl.WindowHeight * microns);

    windowWidthFe = spiderControl.WindowWidth;

    /* initialize the strip module */
    stripLeft = bbxl;
    stripRight = stripLeft + spiderControl.WindowWidth;
    if (stripRight > bbxr) stripRight = bbxr;

    if (printSub3dInit) {
	fprintf (stderr, "sub3dInit: cellbbox xl=%g xr=%g yb=%g yt=%g\n",
		D(bbxl), D(bbxr), D(bbyb), D(bbyt));
	fprintf (stderr, "sub3dInit: initial stripLeft=%g stripRight=%g\n",
		D(stripLeft), D(stripRight));
    }
    stripInit (stripLeft, stripRight);

    /* initialize the math error handler module */
    mathInit ();

    /* initialize the greens function evaluation module */
    greenFactor = greenInit (meters);

    if (!FeModePwl) { // see meshRefine
	// 3: force triangles, 4: force quadrilaterals
	FeShape = paramLookupI ("sub3d.be_shape", "4");
	if (FeShape != 3 && FeShape != 4) {
	    fprintf (stderr, "sub3dInit: incorrect sub3d.be_shape (set to '4')\n");
	    FeShape = 4;
	}
    }
    else FeShape = 3;

    /* set variables for statistics. */
    MaxDimension   = 0;
    MaxOrder       = 0;
    brutoNumGreens = 0;

    numFaces       = 0;
    numVertices    = 0;
    numGreens      = 0;
    said = 0;

    MaxArea        = 0;
    MaxDist        = 0;
    MaxLength      = 0;
    MaxLengthRatio = 0;
    TotalArea      = 0;
    MinArea        = INF;
    MinDist        = INF;
    MinLength      = INF;

    /* Return the bandwidth for the scanline module, so that
     * it retains the tiles long enough.
     * It is necessary return 4 times the window width,
     * because of the mesh-ahead and mesh-behind.
     */
    return (4 * (coor_t) spiderControl.WindowWidth);
}

coor_t sub3dStart ()
{
    nextStrip = stripLeft + windowWidthFe;
    cntStrip = 0;

    ASSERT (thisX < INF);
    while (nextStrip <= thisX) { nextStrip = sub3dStrip (); cntStrip = 0; }

    if (nextStrip > bbxr) nextStrip = bbxr;
    return (nextStrip);
}

void sub3dStop ()
{
    meshRefine ();
    doStrip (DOUBLE_STRIP);
    stripStop ();
    mathStop ();
}

void sub3dEnd ()
{
    printIfNoconverge ();
}

coor_t sub3dStrip ()
{
    Debug (fprintf (stderr, "scanX=%8d  strip#%3d\n", (int) stripRight, cntStrip+1));

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

    /* this lies one strip behind the scanline (needed for meshahead) */
    if (++cntStrip > 2) {
	doStrip (SINGLE_STRIP);
	doStrip (DOUBLE_STRIP);
    }

    /* Move the strip.
     */
    stripLeft  = stripRight;
    stripRight = (coor_t) Min (bbxr, stripRight + spiderControl.WindowWidth);
    stripMove (stripLeft, stripRight);

    if (stripLeft < bbxr) return (stripRight);
    return (INF);
}

Private void doStrip (int mode)
{
    int row, col;

    Debug (fprintf (stderr, "doStrip %s\n", mode == DOUBLE_STRIP ? "DOUBLE_STRIP" : "SINGLE_STRIP"));

    stripVerbose (mode);

#ifdef DEBUG
    if (spiderControl.printSpider) printStrips ();
#endif

    if (mode == DOUBLE_STRIP)
	schurdata.sign =  1;	/* add */
    else
	schurdata.sign = -1;	/* subtract */

    stripTraverse (mode);

    MaxDimension = Max (MaxDimension, maxRow + 1);
    MaxOrder     = Max (MaxOrder, maxCol);

    if (noGreen) return;

    if (maxRow >= 0) {
	tick ("computeConductance");

	schurStartMatrix (maxRow, maxCol);

	for (row = 0; row <= maxRow; ++row) {
	    for (col = 0; col <= spidercols[row]; ++col) {
		computeConductance (mode, row, col, spiderlist[row], spiderlist[row+col]);
	    }
	}

	schurStopMatrix ();

	tock ("computeConductance");
    }
}

/* Begin a new matrix.
 * First make sure that schurdata.buf is big enough,
 * reset some variables and prepare the schur module for new matrix.
 */
Private void schurStartMatrix (int row, int col)
{
    verbose ("Schur dimension %d, maxorder %d\n", row + 1, col);

    if (noSchur) return;

    ASSERT (row >= 0);
    ASSERT (col >= 0);

    if (col > schurdata.maxcol || !schurdata.buf) {
	if (schurdata.buf) DISPOSE (schurdata.buf, sizeof(schur_t) * (schurdata.maxcol+1));
	schurdata.buf = NEW (schur_t, col + 1);
	schurdata.maxcol = col;
    }

    schurdata.row = 0;
    schurdata.col = 0;

    initSchur (row, col);
#ifdef DEBUG
    if (serverMatrix) fprintf (spiderControl.debug, "%d\n", col);
#endif
}

/* Matrix is ready.  Send the last row to the schur module.
 */
Private void schurStopMatrix ()
{
    if (noSchur) return;

    ASSERT (schurdata.col == 0);

#ifdef DEBUG
    if (serverMatrix) {
	fprintf (spiderControl.debug, "%d ", schurdata.col);
	printUpperMatrix (spiderControl.debug, schurdata.row, schurdata.buf, schurdata.col);
	fprintf (spiderControl.debug, "-1\n");
    }
    if (spiderControl.printMatrix) {
	fprintf (spiderControl.debug, "row_in %d %d ", schurdata.row, schurdata.col);
	printUpperMatrix (spiderControl.debug, schurdata.row, schurdata.buf, schurdata.col);
    }
#endif /* DEBUG */

    tock ("other");
    tick ("schurRowIn");
    schurRowIn (schurdata.row, schurdata.buf, schurdata.col); /* last row */
    tock ("schurRowIn");
    tick ("other");
}

Private schur_t doGreen (int mode, spider_t *s1, spider_t *s2)
{
    float result;
#ifdef DEBUG
    static int check_green = 0;
#endif
    if (s1 != s2 && s1 -> act_x == s2 -> act_x && s1 -> act_y == s2 -> act_y) {
	say ("space: internal error: duplicate spider at xyz = %g %g %g\n",
		D(s1 -> act_x), D(s1 -> act_y), D(s1 -> act_z));
	return ((schur_t) 0);
    }

    if (mode == DOUBLE_STRIP && s1 -> strip == s2 -> strip
			&& s1 -> strip -> flags == SINGLE_STRIP) {
	if (fread ((char *) &result, sizeof (result),
		1, s1 -> strip -> gbuf) != 1) say ("tmp file read error");
#ifdef DEBUG
	if (check_green) ASSERT ((float) green (s1, s2) == result);
#endif
    }
    else {
	result = (float) green (s1, s2);
	if (optVerbose) updateStatistics (s1, s2);
	if (mode == SINGLE_STRIP) {
	    ASSERT (s1 -> strip == s2 -> strip);
	    if (fwrite ((char *) &result, sizeof (result),
		1, s1 -> strip -> gbuf) != 1) say ("tmp file write error");
	}
    }

    return ((schur_t) result);
}

Private void computeConductance (int mode, int row, int col, spider_t *s1, spider_t *s2)
{
    schur_t val = doGreen (mode, s1, s2);

    brutoNumGreens++;

#define Epsilon0 8.855e-12 /* Farad/meter */
    if (makesubcap) val /= Epsilon0;

    if (!noSchur) schurInput (row, col, val, s1, s2);

#ifdef DEBUG
    if (spiderControl.printGreen)
	fprintf (spiderControl.debug, "green %g %g %g %g %g %g %g\n",
		D(s1 -> act_x), D(s1 -> act_y), D(s1 -> act_z),
		D(s2 -> act_x), D(s2 -> act_y), D(s2 -> act_z), D(val));
#endif
}

Private void schurInput (int row, int col, schur_t val, spider_t *s1, spider_t *s2)
{
    ASSERT (col <= schurdata.maxcol);

    if (row != schurdata.row) {
	ASSERT (row == schurdata.row + 1);

#ifdef DEBUG
    if (serverMatrix) {
	fprintf (spiderControl.debug, "%d ", schurdata.col);
	printUpperMatrix (spiderControl.debug, schurdata.row, schurdata.buf, schurdata.col);
    }
    if (spiderControl.printMatrix) {
	fprintf (spiderControl.debug, "row_in %d %d ", schurdata.row, schurdata.col);
	printUpperMatrix (spiderControl.debug, schurdata.row, schurdata.buf, schurdata.col);
    }
#endif /* DEBUG */

	tick ("schurRowIn");
	schurRowIn (schurdata.row, schurdata.buf, schurdata.col);
	tock ("schurRowIn");

	schurdata.row = row;
    }

    schurdata.buf[col] = val;
    schurdata.col = col;

    if (col > 0 && val >= schurdata.buf[0] && said++ == 0) {
	spiderEdge_t *edge;
	say ("Inconsistent value in influence matrix, probably due to too large");
	say ("\tmesh granularity (large differences in element sizes).");
	say ("\tExtraction results will be incorrect.");
	say ("\tIt concerns the following mesh points (coordinates are in microns,");
	say ("\tarea sizes in square microns):");
	say ("\t  point 1: x=%g y=%g z=%g",
	     s1 -> act_x * microns, s1 -> act_y * microns, s1 -> act_z * microns);
	if (s1 -> face)
	    say ("\t  connected to element with size %g", s1 -> face -> area * microns2);
	else {
	    say ("\t  connected to elements with sizes:");
	    for (edge = s1 -> edge; edge; edge = edge -> nb)
		if (edge -> face) say ("\t    %g", edge -> face -> area * microns2);
	}
	say ("\t  point 2: x=%g y=%g z=%g",
	     s2 -> act_x * microns, s2 -> act_y * microns, s2 -> act_z * microns);
	if (s2 -> face)
	    say ("\t  connected to element with size %g", s2 -> face -> area * microns2);
	else {
	    say ("\t  connected to elements with sizes:");
	    for (edge = s2 -> edge; edge; edge = edge -> nb)
		if (edge -> face) say ("\t    %g", edge -> face -> area * microns2);
	}
    }
}

void schurRowOut (int row, schur_t *buf, int ncols)
{
    int col;

#ifdef DEBUG
    if (spiderControl.printMatrix) {
	fprintf (spiderControl.debug, "row_out %d %d ", row, ncols);
	printUpperMatrix (spiderControl.debug, row, buf, ncols);
    }
#endif

    ASSERT (ncols == spidercols[row]);

    for (col = 0; col <= ncols; col++) {
	addCon (spiderlist[row], spiderlist[row+col], buf[col]);
    }
}

/* Add the conductance present between s1 and s2 to the network.
 * Con is an entry from the matrix (inverse of elastance matrix)
 */
Private void addCon (spider_t *s1, spider_t *s2, schur_t val)
{
    node_t *nA, *nB;

    // multiply the value, see spider/init.c
    if (schurdata.sign > 0)
	val *= greenFactor;
    else
	val *= -greenFactor;

    if (s1 == s2) { // diagonal matrix element
	nA = s1 -> subnode -> node;
	nA -> substrCon += val;
    }
    else {
	nA = s1 -> subnode -> node;
	nB = s2 -> subnode -> node;

	if (nA != nB) elemAdd (nA, nB, -val);
	nA -> substrCon += val;
	nB -> substrCon += val;
    }
}

void sub3dStatistics (FILE *fp)
{
    fprintf (fp, "overall sub3d statistics:\n");
    fprintf (fp, "\tnumber of elements  : %d\n",
	FeModePwl ? numVertices - numFaces : numFaces);
    fprintf (fp, "\tnumber of vertices  : %d\n",
	FeModePwl ? numVertices : numVertices - numFaces);
    fprintf (fp, "\tnumber of faces     : %d\n", numFaces);
    fprintf (fp, "\tnumber of greens    : %d\n", numGreens);
    fprintf (fp, "\tbruto num. of greens: %d\n", brutoNumGreens);
    fprintf (fp, "\tmax matrix dimension: %d\n", MaxDimension);
    fprintf (fp, "\tmax matrix order    : %d\n", MaxOrder);

    fprintf (fp, "\tmin element dist    : %le\t(microns)\n",  microns * MinDist);
    fprintf (fp, "\tmax element dist    : %le\t(microns)\n",  microns * MaxDist);
    fprintf (fp, "\tmin element area    : %le\t(microns^2)\n", microns2 * MinArea);
    fprintf (fp, "\tmax element area    : %le\t(microns^2)\n", microns2 * MaxArea);
    if (!FeModePwl)
    fprintf (fp, "\tavg element area    : %le\t(microns^2)\n", microns2 * TotalArea / numFaces);
    fprintf (fp, "\tmin edge length     : %le\t(microns)\n",  microns * MinLength);
    fprintf (fp, "\tmax edge length     : %le\t(microns)\n",  microns * MaxLength);
    fprintf (fp, "\tmax ratio of lengths: %le\n\n", MaxLengthRatio);
}

Private void updateStatistics (spider_t *sp1, spider_t *sp2)
{
    meshCoor_t l, lmin, lmax;
    face_t *face;

    numGreens++;

    if (sp1 != sp2) {
	l = spiderDist (sp1, sp2);
	if (l < MinDist) MinDist = l;
	if (l > MaxDist) MaxDist = l;
    }
    else if (FeModePwl) {
	spiderEdge_t *edge;
	for (edge = sp1 -> edge; edge; edge = edge -> nb)
	if ((face = edge -> face)) {
	    if (face -> area < MinArea) MinArea = face -> area;
	    if (face -> area > MaxArea) MaxArea = face -> area;
	    lmin = face -> len2;
	    lmax = face -> len;
	    if (lmin < MinLength) MinLength = lmin;
	    if (lmax > MaxLength) MaxLength = lmax;
	    lmax /= lmin;
	    if (lmax > MaxLengthRatio) MaxLengthRatio = lmax;
	}
    }
    else {
	ASSERT (sp1 -> face);
	face = sp1 -> face;

	if (face -> area < MinArea) MinArea = face -> area;
	if (face -> area > MaxArea) MaxArea = face -> area;
	TotalArea += face -> area;

	lmin = face -> len2;
	lmax = face -> len;
	if (lmin < MinLength) MinLength = lmin;
	if (lmax > MaxLength) MaxLength = lmax;
	lmax /= lmin;
	if (lmax > MaxLengthRatio) MaxLengthRatio = lmax;
    }
}
