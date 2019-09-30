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
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/spider/define.h"

#include "src/space/spider/recog.h"
#include "src/space/spider/extern.h"
#include "src/space/scan/export.h"
#include "src/space/extract/export.h"
#ifdef DISPLAY
#include "src/space/X11/export.h"
#endif

extern DM_STREAM *cbemStream;

int *spidercols;
spider_t **spiderlist;
static int spiderlistsize = 0;

static strip_t  strips[5];
strip_t *stripL, *stripR, *stripA, *stripAA;
static strip_t *stripB;
static FILE * gbuf_fp[2];
static char * fname[2];

static int bucketsize;
static int numBuckets, bucketDist;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void freeSpiders (strip_t *s);
Private void freeEdges (strip_t *s);
Private void setGreenBuf (void);
Private void drawStrip (strip_t *s);
#ifdef DEBUG
Private int checkSpider (spider_t *spider);
Private int findBar (spider_t *spider, spider_t **bar, int y);
#endif
Private void printFace (face_t *f);
#ifdef __cplusplus
  }
#endif

#define D(x) (double)(x)

/* A macro that returns the bucket index of a spider
 */
#define YS(y) ((int) (y - bigbyb) / bucketsize)

/* start a strip with these coordinates,
 * Initialiase some variables
 */
void stripInit (coor_t xl, coor_t xr)
{
    int i, j;
    double d;

    /* The first strip to be used and filled is stripA.
     * This strip has the coordinates of the first layout band.
     * The other ones are to the left of it.
     */
    stripAA = strips + 0;
    stripA = strips + 1;
    stripR = strips + 2;
    stripL = strips + 3;
    stripB = strips + 4;

    d = sqrt (D(spiderControl.maxFeArea));
    if (spiderControl.WindowHeight < d) d = spiderControl.WindowHeight;
    d *= 0.2;

    if (d <= INT_MAX)
        bucketsize = Max (1, (int)d);
    else
        bucketsize = INT_MAX;

    numBuckets = YS (bigbyt) + 1;
    bucketDist = ((int) spiderControl.WindowHeight / bucketsize) + 1;
    if (bucketDist > numBuckets) bucketDist = numBuckets;

    for (i = 0; i < 5; i++) {
	strips[i].xl = xl - (i - 1) * (xr - xl);
	strips[i].xr = xr - (i - 1) * (xr - xl);

	/* SdeG: The last bucket is used for not used strip spiders */
	strips[i].bucket = NEW (spider_t *, numBuckets+1);
	for (j = 0; j <= numBuckets; j++) strips[i].bucket[j] = NULL;

	strips[i].ehead = NULL;
	strips[i].fhead = NULL;
	strips[i].flags = 0;
	strips[i].gbuf  = NULL;
	strips[i].drawn = 0;
	strips[i].numSpiders = 0;
    }

    setGreenBuf ();

    Debug (fprintf (stderr, "numBuckets: %d, bucketDist %d\n", numBuckets, bucketDist));
    Debug (fprintf (stderr, "stripB %d %d, stripL %d stripR %d stripA %d\n",
	stripB -> xl, stripB -> xr, stripL -> xr, stripR -> xr, stripA -> xr));
}

void stripMove (coor_t left, coor_t right)
{
    strip_t *strip;

    /* free the mesh-behind spiders */
    freeSpiders (stripB);

    /* free the edges in stripB */
    freeEdges (stripB); /* was L */

    stripFreeFaces (stripB);

    /* unset the drawing status */
    stripB -> drawn = 0;

    /* rotate the strips */
    strip  = stripB;
    stripB = stripL;
    stripL = stripR;
    stripR = stripA;
    stripA = stripAA;
    stripAA = strip;

    /* set the coordinates of the new rightmost strips */
    stripA -> xl = left;
    stripA -> xr = right;
    stripAA -> xl = right;
    stripAA -> xr = right + (right - left);

    /* file buffers for saving computed green functions */
    setGreenBuf ();

    /* Can eventually free the spiders in stripB that are not on
     * the right boundary of the strip.
     */

    if (spiderControl.debug) {
	fprintf (spiderControl.debug, "stripB %d %d, stripL %d stripR %d stripA %d\n",
	    stripB -> xl, stripB -> xr, stripL -> xr, stripR -> xr, stripA -> xr);
    }
    Debug (fprintf (stderr, "stripB %d %d, stripL %d stripR %d stripA %d\n",
	stripB -> xl, stripB -> xr, stripL -> xr, stripR -> xr, stripA -> xr));
}

/* To be called when a cell is finished,
 * deallocation of memory.
 */
void stripStop ()
{
    int i;

    /* Note: this is not necesarily in the order
     * stripB - stripL - stripR - stripA - stripAA
     */
    for (i = 0; i < 5; i++) {
	freeSpiders (strips + i);
	freeEdges (strips + i);
	stripFreeFaces (strips + i);
    }

    if (gbuf_fp[0]) { fclose (gbuf_fp[0]); gbuf_fp[0] = NULL; }
    if (gbuf_fp[1]) { fclose (gbuf_fp[1]); gbuf_fp[1] = NULL; }
    if (fname[0]) {
	unlink (fname[0]);
	unlink (fname[1]);
    }
    Debug2 (if (spiderControl.debug) fflush (spiderControl.debug));
}

void stripVerbose (int mode)
{
    coor_t l, r;
    char *s;

    if (mode == DOUBLE_STRIP) {
	l = stripL -> xl; s = "add";
    }
    else {
	l = stripR -> xl; s = "subtract";
    }
    r = stripR -> xr;

    if (l < bigbxl) l = bigbxl;
    if (r > bigbxr) r = bigbxr;

    verbose ("strip %s %s (%s)\n", strCoor (l), strCoor (r), s);
}

/* Dispose all spider in the strip.
 * Also dispose faces that are represented by spiders.
 */
Private void freeSpiders (strip_t *strip)
{
    register spider_t *spider;
    register int i;

    Debug (fprintf (stderr, "free spiders %d %d\n", (int) strip -> xl, (int) strip -> xr));

    for (i = 0; i <= numBuckets; i++) {
	while ((spider = strip -> bucket[i])) {
	    strip -> bucket[i] = spider -> next;
	    disposeSpider (spider);
	}
    }
    strip -> numSpiders = 0;
}

Private void freeEdges (strip_t *strip)
{
    spiderEdge_t *edge;

    Debug (fprintf (stderr, "free edges %d %d\n", (int) strip -> xl, (int) strip -> xr));

    while ((edge = strip -> ehead)) {
	strip -> ehead = edge -> next;
	disposeSpiderEdge (edge);
    }
}

Private void setGreenBuf ()
{
    static int f = -1;

    if (f == -1) { /* first time called */
	f = 0;
	fname[0] = tempname ("space1.", NULL, 0);
	fname[1] = tempname ("space2.", NULL, 0);
    }
    else { /* toggle between 0 and 1 */
	f = 1 - f;
    }

    if (gbuf_fp[f]) fclose (gbuf_fp[f]);

    stripR -> gbuf = gbuf_fp[f] = cfopen (fname[f], "wb+");
    stripR -> flags = 0;
    unlink (fname[f]);
}

void stripAddSpider (spider_t *spider)
{
    strip_t *strip;
    spider_t *s, *p;
    int y;

    if (spider -> strip) return; /* already in strip */

#ifdef DEBUG
    Debug2 (ASSERT (checkSpider (spider) == 0));
#endif

    /* nom_x is used here, because act_x is not definite
     */
    if (stripA -> xl >= bigbxr) {
	ASSERT (spider -> nom_x <= bigbxr);
	strip = stripR;
    }
    else {
	strip = (spider -> nom_x < stripA -> xl) ? stripR
	     : ((spider -> nom_x < stripA -> xr || stripA -> xr >= bigbxr) ? stripA : stripAA);
    }

    if ((FeModePwl && !spider -> face) || (!FeModePwl && spider -> face)) {

	y = YS (spider -> nom_y);
	ASSERT (y >= 0 && y < numBuckets);

#if SortAboutZFirst
	for (p = NULL, s = strip -> bucket[y]; s && (s -> nom_z  < spider -> nom_z ||
	    (s -> nom_z == spider -> nom_z && s -> nom_x > spider -> nom_x)); p = s, s = s -> next);
#else /* X first */
	for (p = NULL, s = strip -> bucket[y]; s && (s -> nom_x  > spider -> nom_x ||
	    (s -> nom_x == spider -> nom_x && s -> nom_z > spider -> nom_z)); p = s, s = s -> next);
#endif
	spider -> next = s;
	if (p) p -> next = spider;
	else   strip -> bucket[y] = spider;
	strip -> numSpiders++;
    }
    else { /* SdeG: unused strip spider */
	spider -> next = strip -> bucket[numBuckets];
	strip -> bucket[numBuckets] = spider;
    }

    spider -> strip = strip;
    ASSERT (spider -> nom_x >= strip -> xl);
    ASSERT (spider -> nom_x < strip -> xr || spider -> nom_x == bigbxr);
}

void stripRemoveSpider (spider_t *spider)
{
    strip_t *strip;
    spider_t *sp;
    int y;

    if (!(strip = spider -> strip)) return; /* SdeG: not in strip */

    ASSERT (!spider -> face);

    y = YS (spider -> nom_y);
    ASSERT (y >= 0 && y < numBuckets);

    if ((sp = strip -> bucket[y]) == spider) strip -> bucket[y] = spider -> next;
    else {
	while (sp && sp -> next != spider) sp = sp -> next;
	ASSERT (sp);
	sp -> next = spider -> next;
    }
    strip -> numSpiders--;

    /* don't dispose the spider */
    spider -> next = strip -> bucket[numBuckets];
    strip -> bucket[numBuckets] = spider;
}

/* Spider edges are also maintained in strips
 * in order to be able to deallocate them when the
 * spiders in a strip are deallocated.
 * This list is also used for drawing spiders
 * in display mode.
 */
void stripAddEdge (spiderEdge_t *edge)
{
    strip_t *strip;

    if (stripA -> xl >= bigbxr) {
	ASSERT (edge -> sp -> nom_x <= bigbxr);
	ASSERT (edge -> oh -> sp -> nom_x <= bigbxr);
	strip = stripR;
    }
    else {
	strip = ((edge -> sp -> nom_x < stripA -> xl
	&&  edge -> oh -> sp -> nom_x  < stripA -> xl) ? stripR
	: (((edge -> sp -> nom_x < stripA -> xr
	     &&  edge -> oh -> sp -> nom_x  < stripA -> xr)
	     || stripA -> xr >= bigbxr) ? stripA : stripAA));
    }

    edge -> next = NULL;
    if (strip -> ehead == NULL)
	strip -> ehead = strip -> etail = edge;
    else {
	strip -> etail -> next = edge;
	strip -> etail = edge;
    }
}

bool_t inStripR (face_t *face) /* returns TRUE if 'face' is in stripR */
{
    if (SP(0) && !isKnown (face)) { /* face is finished and not refined */
	meshCoor_t a, b, c, d, x;

	if ((x = stripA -> xl) >= bigbxr) return (TRUE);

	if ((a = SP(0) -> nom_x) > x) return (FALSE);
	if ((b = SP(1) -> nom_x) > x) return (FALSE);
	if ((c = SP(2) -> nom_x) > x) return (FALSE);
	if (!SP(3)) d = c; else
	if ((d = SP(3) -> nom_x) > x) return (FALSE);

	if (a == x && b == x && c == x && d == x) return (FALSE);
	return (TRUE);
    }
    return (FALSE);
}

char * sprintfStripRcoor ()
{
    static char buf[80];

    sprintf (buf, "%d %d", stripR -> xl, stripR -> xr);
    return (buf);
}

coor_t stripRleftX ()
{
    return stripR -> xl;
}

void stripTraverse (int dmode, int *maxRow, int *maxCol)
{
    int row, col, k, l, numSpiders;
    spider_t *sp;
    spider_t **barL = stripL -> bucket;
    spider_t **barR = stripR -> bucket;

    if (!dmode) {
	/* mark the right strip as having its greens functions retained */
	stripR -> flags = SINGLE_STRIP;
	numSpiders = stripR->numSpiders;
    }
    else {
	if (stripL -> flags == SINGLE_STRIP) rewind (stripL -> gbuf);
	if (stripR -> flags == SINGLE_STRIP) rewind (stripR -> gbuf);
	numSpiders = stripL->numSpiders + stripR->numSpiders;
    }

    Debug (fprintf (stderr, "stripTraverse: %s xl=%d xr=%d numSpiders=%d\n",
	dmode ? "DOUBLE_STRIP" : "SINGLE_STRIP",
	dmode ? stripL -> xl : stripR -> xl, stripR -> xr, numSpiders));

    if (numSpiders <= 0) return;

    if (spiderlistsize < numSpiders) {
	if (spiderlistsize > 0) {
	    DISPOSE (spidercols, sizeof(int) * spiderlistsize);
	    DISPOSE (spiderlist, sizeof(spider_t *) * spiderlistsize);
	    spiderlistsize = 2*numSpiders;
	}
	else spiderlistsize = 5*numSpiders;
	spiderlist = NEW (spider_t *, spiderlistsize);
	spidercols = NEW (int, spiderlistsize);
    }

    row = -1; col = -1;

    for (l = 0; l < bucketDist; ++l) {
	for (sp = barR[l]; sp; sp = sp -> next) ++col;
	if (dmode)
	for (sp = barL[l]; sp; sp = sp -> next) ++col;
    }
    *maxCol = col;

    for (k = 0; k < numBuckets; ++k) {
	for (sp = barR[k]; sp; sp = sp -> next) { ++row; spidercols[row] = col--; spiderlist[row] = sp; }
	if (dmode)
	for (sp = barL[k]; sp; sp = sp -> next) { ++row; spidercols[row] = col--; spiderlist[row] = sp; }

	if (l < numBuckets) {
	    for (sp = barR[l]; sp; sp = sp -> next) ++col;
	    if (dmode)
	    for (sp = barL[l]; sp; sp = sp -> next) ++col;
	    ++l;
	    if (*maxCol < col) *maxCol = col;
	}
    }
    *maxRow = row;
    ASSERT (row+1 == numSpiders);
}

void stripDrawAndPrint ()
{
    spiderEdge_t *edge;

    drawStrip (stripL);
    drawStrip (stripR);

    /* Draw edges that are on the boundary between stripR and stripA
       (they are stored with stripA). */

#ifdef OLD_VERSION
    /* SdeG: For the new version, this code gives trouble, because
       Xspace can't show the real mesh after it is modified.
     */
    for (edge = stripA -> ehead; edge; edge = edge -> next) {
	spider_t *sp1 = edge -> sp;
	spider_t *sp2 = edge -> oh -> sp;

	/* draw and print the edges only once.
	 * Implement this by comparing spider pointer values.
	 * Note: sp1 is zero, when the edge is unused (SdeG).
	 */
	if (!sp1 || sp1 > sp2) continue;

	if (sp1 -> nom_x > stripA -> xl || sp2 -> nom_x > stripA -> xl) continue;

#ifdef DISPLAY
	if (goptDrawSpider) {
	    if (greenCase == DIEL)
		drawSpiderEdge (sp1, sp2, edge -> type == CONTACTEDGE);
	    else
		drawSpiderEdge (sp1, sp2, -1);
	}
#endif
    }
#endif /* OLD_VERSION */
}

/* Draw and/or print the mesh in strip, each spider edge is drawn
 * in the order in which they were created.
 */
Private void drawStrip (strip_t *strip)
{
    spiderEdge_t *edge;
    spider_t *sp;
    int i;

    /* make sure a strip is drawn and printed only once */
    if ((strip -> drawn)++ != 0) return;

    Debug (fprintf (stderr, "draw strip %g %g\n", D(strip -> xl), D(strip -> xr)));

    for (i = 0; i <= numBuckets; i++) {
	for (sp = strip -> bucket[i]; sp; sp = sp -> next) {
	    if (spiderControl.printSpider) pspf (spiderControl.debug, sp);
	    if (sp -> face) {
		if (cbemStream) printFace (sp -> face);
	    }
	}
    }

    for (edge = strip -> ehead; edge; edge = edge -> next) {
	spider_t *sp1 = edge -> sp;
	spider_t *sp2 = edge -> oh -> sp;

	/* draw and print the edges only once.
	 * Implement this by comparing spider pointer values.
	 * Note: sp1 is zero, when the edge is unused (SdeG).
	 */
	if (!sp1 || sp1 > sp2) continue;

	if (spiderControl.printSpider)
	   psef (spiderControl.debug, meshFindEdge (sp1, sp2));

#ifdef DISPLAY
	if (goptDrawSpider) {
	    if (greenCase == DIEL)
		drawSpiderEdge (sp1, sp2, edge -> type == CONTACTEDGE);
	    else
		drawSpiderEdge (sp1, sp2, -1);
	}
#endif
    }

    if (spiderControl.printSpider) fflush (spiderControl.debug);
}

Private void printFace (face_t *face)
{
    spider_t *sp1, *sp;
    int i;
    char *cond;
    FILE *fp = cbemStream -> dmfp;

    if (!fp) return;

    sp1 = SP(0);

    /* determine conductor name */
    cond = conNr2Name (sp1 -> conductor);
    for (i = 1; i < 4 && (sp = SP(i)); ++i) {
	if (sp -> conductor != sp1 -> conductor) {
	    if (sp -> conductor < sp1 -> conductor)
		fprintf (fp, "F via_%s_%s", cond, conNr2Name (sp -> conductor));
	    else
		fprintf (fp, "F via_%s_%s", conNr2Name (sp -> conductor), cond);
	    cond = NULL;
	    break;
	}
    }

    if (cond) fprintf (fp, "F %s", cond);
    for (i = 0; i < 4 && (sp = SP(i)); ++i) {
	fprintf (fp, " %.20g %.20g %.20g", sp -> act_x, sp -> act_y, sp -> act_z);
    }
    fprintf (fp, "\n");
}

#ifdef DEBUG
/* Check that there is not already a spider with same coordinates.
 */
Private int checkSpider (spider_t *sp)
{
    int y = YS (sp -> nom_y);

    if (findBar (sp, stripR -> bucket, y)) {
	say ("spider at xyz = %g %g %g already in rbar\n", D(sp -> nom_x), D(sp -> nom_y), D(sp -> nom_z));
	return (1);
    }
    if (findBar (sp, stripL -> bucket, y)) {
	say ("spider at xyz = %g %g %g already in lbar\n", D(sp -> nom_x), D(sp -> nom_y), D(sp -> nom_z));
	return (2);
    }
    return (0);
}

Private bool_t findBar (spider_t *spider, spider_t **bar, int y)
{
    spider_t *s;
    for (s = bar[y]; s; s = s -> next) {
	/* For faces that are part of the side-wall of a slope, the nominal
	   x and y coordinates of the points of the face may all be equal.
	   In that case, if we refine, x, y and z coordinate of a new point
	   may be equal to x, y and z coordinate of an existing point.
	   So, default, we can not check on a nominal coordinates.
	*/
#ifdef CHECK_NOM
	if (s -> nom_x == spider -> nom_x
	&&  s -> nom_y == spider -> nom_y
	&&  s -> nom_z == spider -> nom_z) return (TRUE);
#endif
	if (s -> act_x == spider -> act_x
	&&  s -> act_y == spider -> act_y
	&&  s -> act_z == spider -> act_z) return (TRUE);
    }
    return (FALSE);
}
#endif /* DEBUG */
