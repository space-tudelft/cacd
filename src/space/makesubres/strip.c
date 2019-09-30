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

extern int maxRow;
extern int maxCol;
extern int outScale;

int *spidercols;
spider_t **spiderlist;
static int spiderlistsize = 0;

static strip_t  strips[5];
static strip_t *stripB;
static strip_t *stripL;
static strip_t *stripR;
static strip_t *stripA;
static strip_t *stripAA;
static FILE * gbuf_fp[2];
static char * fname[2];

static int bucketsize;
static int numBuckets, bucketDist;

#ifdef __cplusplus
  extern "C" {
#endif

/* local operations */
Private void freeSpiders (strip_t *s);
Private void freeEdges   (strip_t *s);
Private void setGreenBuf (void);

#ifdef __cplusplus
  }
#endif

#define Valid(sp) (FeModePwl ? (sp -> face == NULL) : (sp -> face != NULL))

/* A macro that returns the bucket index of a spider
 */
#define YS(y) I(y - bbyb) / bucketsize

/* Start a strip with these coordinates, init some variables.
 */
void stripInit (coor_t xl, coor_t xr)
{
    int i, j;
    double d;

    /* The first strip to be used and filled is stripA.
     * This should have the coordinates of the first band
     * in the layout.  The other ones are to the left of it.
     */
    stripAA = strips + 0;
    stripA = strips + 1;
    stripR = strips + 2;
    stripL = strips + 3;
    stripB = strips + 4;

    d = sqrt (D(spiderControl.maxFeArea));
    if (spiderControl.WindowHeight < d) d = spiderControl.WindowHeight;
    d *= 0.2;

    if ((i = I(d)) < 1) i = 1;
    bucketsize = i < INT_MAX ? i : INT_MAX;
    numBuckets = YS (bbyt) + 1;
    bucketDist = YS (bbyb + spiderControl.WindowHeight) + 1;
    if (bucketDist > numBuckets) bucketDist = numBuckets;

    for (i = 0; i < 5; i++) {
	strips[i].xl = xl - (i - 1) * (xr - xl);
	strips[i].xr = xr - (i - 1) * (xr - xl);

	strips[i].bucket = NEW (spider_t *, numBuckets);
	for (j = 0; j < numBuckets; j++) strips[i].bucket[j] = NULL;

	strips[i].ehead = NULL;
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
    strip_t * tmp;

    /* free the mesh-behind spiders */
    freeSpiders (stripB);

    /* free the edges in stripB */
    freeEdges (stripB);			/* previous stripL */

    /* unset the drawing status */
    stripB -> drawn = 0;

    /* rotate the strips */
    tmp    = stripB;
    stripB = stripL;
    stripL = stripR;
    stripR = stripA;
    stripA = stripAA;
    stripAA = tmp;

    /* set the coordinates of the new rightmost strips */
    stripA -> xl = left;
    stripA -> xr = right;
    stripAA -> xl = right;
    stripAA -> xr = right + (right - left);

    /* tmp file buffers for saving computed green functions. */
    setGreenBuf ();

    /* Can eventually free the spiders in stripB that are not on
     * the right boundary of the strip.
     */
#ifdef DEBUG
    if (spiderControl.debug)
	fprintf (spiderControl.debug, "stripB %d %d, stripL %d stripR %d stripA %d\n",
	    stripB -> xl, stripB -> xr, stripL -> xr, stripR -> xr, stripA -> xr);
#endif
    Debug (fprintf (stderr, "stripB %d %d, stripL %d stripR %d stripA %d\n",
	stripB -> xl, stripB -> xr, stripL -> xr, stripR -> xr, stripA -> xr));
}

/* To be called when a cell is finished, for deallocation of memory.
 */
void stripStop ()
{
    int i;

    /* Note: this is not necessarily in the order
     * stripB - stripL - stripR - stripA - stripAA
     */
    for (i = 0; i < 5; i++) {
	freeSpiders (strips + i);
	freeEdges (strips + i);
    }
    freeFaces ();

    if (gbuf_fp[0]) { fclose (gbuf_fp[0]); gbuf_fp[0] = NULL; }
    if (gbuf_fp[1]) { fclose (gbuf_fp[1]); gbuf_fp[1] = NULL; }
    if (fname[0]) {
	unlink (fname[0]);
	unlink (fname[1]);
    }

#ifdef DEBUG
    if (spiderControl.debug) fflush (spiderControl.debug);
#endif
}

void stripVerbose (int mode)
{
    coor_t l, r;
    char * s;

    if (mode == DOUBLE_STRIP) {
	l = stripL -> xl; s = "add";
    }
    else {
	l = stripR -> xl; s = "subtract";
    }
    r = stripR -> xr;

    if (l < bbxl) l = bbxl;
    if (r > bbxr) r = bbxr;

#define strCoor(x) D(x) / D(outScale)
    verbose ("strip %g %g (%s)\n", strCoor (l), strCoor (r), s);
}

/*
 * Dispose all spider in strip pointed to by s.
 * Also dispose faces that are represented by spiders
 */
Private void freeSpiders (strip_t *s)
{
    register spider_t *spider;
    register int i;

    Debug (fprintf (stderr, "free spiders %d %d\n", I(s -> xl), I(s -> xr)));

    for (i = 0; i < numBuckets; i++) {
	while ((spider = s -> bucket[i])) {
	    s -> bucket[i] = spider -> next;
	    if (spider -> face) disposeFace (spider -> face);
	    disposeSpider (spider);
	}
    }
    s -> numSpiders = 0;
}

Private void freeEdges (strip_t *s)
{
    spiderEdge_t * edge;

    Debug (fprintf (stderr, "free edges %d %d\n", I(s -> xl), I(s -> xr)));

    while ((edge = s -> ehead)) {
	s -> ehead = edge -> next;
	disposeSpiderEdge (edge);
    }
}

Private void setGreenBuf ()
{
    static int f = -1;

    if (f == -1) { /* first time called */
	f = 0;
	fname[0] = tempname ("mksub1.", NULL, 0);
	fname[1] = tempname ("mksub2.", NULL, 0);
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
    strip_t  *strip;
    spider_t *s, *p;
    int i;

    ASSERT (!spider -> strip); /* not already in strip */

    if (stripA -> xl >= bbxr) {
	ASSERT (spider -> act_x <= bbxr);
	strip = stripR;
    }
    else {
	strip = (spider -> act_x < stripA -> xl) ? stripR
	     : ((spider -> act_x < stripA -> xr || stripA -> xr >= bbxr) ? stripA : stripAA);
    }

    i = YS (spider -> act_y);
    ASSERT (i >= 0 && i < numBuckets);

    for (p = 0, s = strip -> bucket[i]; s && s -> act_x > spider -> act_x; s = (p = s) -> next);
    spider -> next = s;
    if (p) p -> next = spider;
    else   strip -> bucket[i] = spider;

    if (Valid (spider)) strip -> numSpiders++;

    spider -> strip = strip;
    ASSERT (spider -> act_x >= strip -> xl);
    ASSERT (spider -> act_x < strip -> xr || spider -> act_x == bbxr);
}

void stripFreeSpider (spider_t *spider)
{
    spider_t **bar, *s, *p;
    int i;

    ASSERT (spider -> strip);
    bar = spider -> strip -> bucket;

    if (Valid (spider)) spider -> strip -> numSpiders--;

    i = YS (spider -> act_y);
    ASSERT (i >= 0 && i < numBuckets);

    for (p = 0, s = bar[i]; s && s != spider; s = (p = s) -> next);
    ASSERT (s);
    if (p) p -> next = spider -> next;
    else   bar[i] = spider -> next;
    disposeSpider (spider);
}

spider_t *stripFindSpider (meshCoor_t x, meshCoor_t y, meshCoor_t z)
{
    spider_t *sp;
    strip_t *strip;
    int i;

    if (stripA -> xl >= bbxr) {
	ASSERT (x <= bbxr);
	strip = stripR;
    }
    else {
	strip = (x < stripA -> xl) ? stripR :
		((x < stripA -> xr || stripA -> xr >= bbxr) ? stripA : stripAA);
    }

    i = YS (y);
    ASSERT (i >= 0 && i < numBuckets);

    for (sp = strip -> bucket[i]; sp; sp = sp -> next) {
	if (Nearby (sp, x, y, z)) break;
    }
    Debug (fprintf (stderr, "stripFindSpider %s spider at (%g %g %g), bucket %d\n",
		sp ? "found" : "did not find", D(x), D(y), D(z), i));
    return sp;
}

/* Spider edges are also maintained in the strips in order to be able
 * to deallocate them, when the spiders in a strip are deallocated.
 */
void stripAddEdge (spiderEdge_t *edge)
{
    strip_t * strip;

    if (stripA -> xl >= bbxr) {
	ASSERT (edge -> sp -> act_x <= bbxr);
	ASSERT (edge -> oh -> sp -> act_x <= bbxr);
	strip = stripR;
    }
    else {
	strip = ((edge -> sp -> act_x < stripA -> xl
	&&  edge -> oh -> sp -> act_x < stripA -> xl) ? stripR
	: (((edge -> sp -> act_x < stripA -> xr
	&&  edge -> oh -> sp -> act_x < stripA -> xr)
	     || stripA -> xr >= bbxr) ? stripA : stripAA));
    }

    if (!strip -> ehead) strip -> ehead = edge;
    else strip -> etail -> next = edge;
    strip -> etail = edge;
    edge -> next = NULL;
}

bool_t inStripA (face_t *face) // Returns TRUE if 'face' is in stripA/AA.
{
    meshCoor_t xl;

    if (!face -> corners[2]) return (TRUE); // Face not yet finished; it must be in stripA/AA.
    if (face -> mark == FACE_KNOWN) return (TRUE); // Face already refined!

    xl = stripA -> xl;
    if (face -> corners[0] -> act_x > xl) return (TRUE);
    if (face -> corners[1] -> act_x > xl) return (TRUE);
    if (face -> corners[2] -> act_x > xl) return (TRUE);
    if (face -> corners[3])
    if (face -> corners[3] -> act_x > xl) return (TRUE);
    return (FALSE);
}

char * sprintfStripRcoor ()
{
    static char buf[80];

    sprintf (buf, "%d %d", stripR -> xl, stripR -> xr);
    return (buf);
}

void stripTraverse (int mode)
{
    int row, col, k, l;
    int numSpiders;
    register spider_t *s1;
    spider_t ** barL = stripL -> bucket;
    spider_t ** barR = stripR -> bucket;

    Debug (fprintf (stderr, "stripTraverse: mode %d %d %d\n",
	mode, (mode == SINGLE_STRIP) ? stripR -> xl : stripL -> xl, stripR -> xr));

    if (mode == SINGLE_STRIP && stripR -> gbuf) {
	/* mark the right strip as having its greens functions retained */
	stripR -> flags = SINGLE_STRIP;
    }
    else {
	if (stripL -> flags == SINGLE_STRIP) rewind (stripL -> gbuf);
	if (stripR -> flags == SINGLE_STRIP) rewind (stripR -> gbuf);
    }

    numSpiders = stripR -> numSpiders;
    if (mode == DOUBLE_STRIP) numSpiders += stripL -> numSpiders;

    if (numSpiders > spiderlistsize) {
	if (spiderlistsize > 0) {
	    DISPOSE (spidercols, sizeof(int) * spiderlistsize);
	    DISPOSE (spiderlist, sizeof(spider_t *) * spiderlistsize);
	    spiderlistsize = 2 * numSpiders;
	}
	else spiderlistsize = 5 * numSpiders;
	spiderlist = NEW (spider_t *, spiderlistsize);
	spidercols = NEW (int, spiderlistsize);
    }

    col = -1;
    for (l = 0; l < bucketDist; ++l) {
	for (s1 = barR[l]; s1; s1 = s1 -> next) if (Valid (s1)) ++col;
	if (mode == DOUBLE_STRIP)
	for (s1 = barL[l]; s1; s1 = s1 -> next) if (Valid (s1)) ++col;
    }
    maxCol = col;

    row = -1;
    for (k = 0; k < numBuckets; ++k) {
	for (s1 = barR[k]; s1; s1 = s1 -> next) if (Valid (s1)) { spiderlist[++row] = s1; spidercols[row] = col--; }
	if (mode == DOUBLE_STRIP)
	for (s1 = barL[k]; s1; s1 = s1 -> next) if (Valid (s1)) { spiderlist[++row] = s1; spidercols[row] = col--; }

	if (l < numBuckets) {
	    for (s1 = barR[l]; s1; s1 = s1 -> next) if (Valid (s1)) ++col;
	    if (mode == DOUBLE_STRIP)
	    for (s1 = barL[l]; s1; s1 = s1 -> next) if (Valid (s1)) ++col;
	    ++l;
	    if (col > maxCol) maxCol = col;
	}
    }
    maxRow = row;

    ASSERT (maxRow+1 == numSpiders);
}

#ifdef DEBUG
/*
 * Print the mesh in the strip.  Each spider edge is printed
 * in the order in which they were created.
 */
Private void printStrip (strip_t *s)
{
    spider_t *sp;
    spiderEdge_t *edge;
    int i;

    if (s -> drawn++) return; // print a strip only once

    Debug (fprintf (stderr, "draw strip %g %g\n", D(s -> xl), D(s -> xr)));
    fprintf (spiderControl.debug, "strip %g %g\n", D(s -> xl), D(s -> xr));

    for (i = 0; i < numBuckets; i++)
	for (sp = s -> bucket[i]; sp; sp = sp -> next)
	    pspf (spiderControl.debug, sp);

    /* Print the edges only once (by comparing spiders).
     * Note: edge -> sp is zero, when the edge is unused.
     */
    for (edge = s -> ehead; edge; edge = edge -> next)
	if (edge -> sp && edge -> sp < edge -> oh -> sp)
	    psef (spiderControl.debug, edge);

    fflush (spiderControl.debug);
}

void printStrips ()
{
    printStrip (stripL);
    printStrip (stripR);
}
#endif // DEBUG
