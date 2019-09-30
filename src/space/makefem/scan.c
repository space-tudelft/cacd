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

#include "src/space/makefem/define.h"
#include "src/space/makefem/extern.h"

#ifdef __cplusplus
  extern "C" {
#endif

/* local operations */
Private void bundle (edge_t *newEdge, edge_t *edge);
Private void unbundle (edge_t *edge);

#ifdef __cplusplus
  }
#endif

#define equalSlope(e1, e2) compareSlope (e1, ==, e2)
#define smallerAtX(e1, e2) (e2 -> xl == thisX && (e2 -> yl < thisY \
 || (e2 -> yl == thisY && e1 -> xr > thisX && compareSlope (e2, <, e1))))
#define equalAtX(e1, e2) (e2 -> xl == thisX && e2 -> yl == thisY && e1 -> xr > thisX && equalSlope (e1, e2))
#define TestIntersection(e1, e2) if (!equalSlope (e1, e2)) testIntersection (e1, e2)

struct _scanInfo {
    int xpos;
    int srl;	/* stateruler length */
    int edges;  /* number of edges */
} scanInfo;

coor_t thisX, thisY;
mask_t thisColor;

void scan (edge_t *newEdge)
{
    edge_t *edge, *head, *tail, *ebwd;
    coor_t nextX;
    int doSplit;

    // initScan
    NEW_EDGE (head, -INF, -INF, INF, -INF, cNull);
    NEW_EDGE (tail, -INF,  INF, INF,  INF, cNull);
    head -> fwd = head -> bwd = tail;
    tail -> fwd = tail -> bwd = head;
    NEW_TILE (head -> tile, -INF, -INF, INF, -INF, cNull, 0);
    NEW_TILE (tail -> tile, -INF,  INF, INF,  INF, cNull, 0);
    tail -> tile -> tl = tail -> tile -> tr = -INF;

    thisX = newEdge -> xl;

    while (thisX < INF) {
	nextX = INF;

	Debug (fprintf (stderr, "thisX=%d\n", thisX));
	scanInfo.xpos++;

	edge = head -> fwd;

	while (edge -> yr < INF || newEdge -> xl == thisX) {

	    thisY = Y (edge, thisX);

	    /* The following tests for a special case:
	       An intersection between edges before and after an edge
	       that ends at thisX (see also split ()) (AvG)
	    */
	    if (edge -> xl < thisX && edge -> xr > thisX
		&& edge -> fwd -> xr == thisX
		&& edge -> fwd -> yr == thisY
		&& Y (edge -> fwd -> fwd, thisX) == thisY) {
		TestIntersection (edge, edge -> fwd -> fwd);
	    }

	    if (edge -> xi == thisX) edge = split (edge);

	    if (edge -> xc == thisX && edge -> bundle) unbundle (edge);

	    if (smallerAtX (edge, newEdge)) {
		/* insert newEdge below edge */
		Debug (printEdge ("insert", newEdge));
		Debug (printEdge ("below", edge));
		newEdge -> bwd = edge -> bwd;
		newEdge -> fwd = edge;
		edge -> bwd -> fwd = newEdge;
		edge -> bwd = newEdge;
		edge = newEdge;
		scanInfo.edges++;
		newEdge = fetchEdge ();
		thisY = edge -> yl;
	    }

	    doSplit = 0;

	    if (equalAtX (edge, newEdge) && edge -> xr > thisX) {
		coor_t old_xr = edge -> xr;
		do {
		    if (!(newEdge -> cc & 0xc00)) doSplit = 1;
		    bundle (newEdge, edge);
		    scanInfo.edges++;
		    newEdge = fetchEdge ();
		} while (equalAtX (edge, newEdge));
		if (edge -> xr > old_xr) {
		    TestIntersection (edge, edge -> fwd);
		    TestIntersection (edge, edge -> bwd);
		}
	    }

	    scanInfo.srl++;

	    if (edge -> xr == thisX) {
		tileDeleteEdge (edge);

		/* delete edge from scanline */
		Debug (printEdge ("delete", edge));
		ebwd = edge -> bwd;
		ebwd -> fwd = edge -> fwd;
		edge -> fwd -> bwd = ebwd;
		disposeEdge (edge);
		edge = ebwd -> fwd;

		TestIntersection (edge, ebwd);
	    }
            else if (edge -> yr < INF) {
		COLOR_XOR (thisColor, edge -> color);
		if (edge -> xl == thisX) {
		    TestIntersection (edge, edge -> bwd);
		    tileInsertEdge (edge);
		}
		else {
		    if (edge -> bwd -> xl == thisX)
			TestIntersection (edge, edge -> bwd);
		    tileCrossEdge (edge, doSplit);
		}

		if (edge -> xi < nextX) nextX = edge -> xi;
		if (edge -> xc < nextX) nextX = edge -> xc;

		edge = edge -> fwd;
	    }
	}

	/* important test: thisColor must be zero */
	if (IS_COLOR (&thisColor)) ASSERT (!IS_COLOR (&thisColor));

	ASSERT (nextX > thisX && newEdge -> xl > thisX);

	tileAdvanceScan (edge);

	if (nextX > newEdge -> xl) nextX = newEdge -> xl;

	ASSERT (nextX > thisX);
	thisX = nextX;
    }

    tileStopScan ();

    /* dispose the special tiles */
    disposeTile (head -> tile);
    disposeTile (head -> fwd -> tile);

    /* dispose the special edges */
    disposeEdge (newEdge); /* EOF edge */
    disposeEdge (head -> fwd);
    disposeEdge (head);
}

void scanPrintInfo (FILE *fp)
{
    fprintf (fp, "overall scan statistics:\n");
    fprintf (fp, "\tnumber of edges    : %d\n", scanInfo.edges);
    fprintf (fp, "\tnumber of x pos.   : %d\n", scanInfo.xpos);
    fprintf (fp, "\taverage sr length  : %d\n",
	scanInfo.xpos != 0 ? scanInfo.srl/scanInfo.xpos : 0);
    edgeStatistics (fp);
    tileStatistics (fp);
}

Private void bundle (edge_t *newEdge, edge_t *edge) /* insert overlapping edge */
{
    edge_t * e = edge, * e1 = edge;
    mask_t m;

    Debug (printEdge ("bundle", newEdge));
    Debug (printEdge ("with",  edge));

    ASSERT (equalSlope (edge, newEdge));
    ASSERT (newEdge -> xr == newEdge -> xc);
    ASSERT (edge -> xr > newEdge -> xl);

    if (newEdge -> xr > edge -> xr)
	edge -> xr = newEdge -> xr, edge -> yr = newEdge -> yr;

    while (e1 && e1 -> xc <= newEdge -> xr) {
	e = e1; e1 = e1 -> bundle;
	COLOR_ADD (e -> color, newEdge -> color);
	if (!(newEdge -> cc & 0x400)) e -> cc = newEdge -> cc;
    }

    /* e is the last edge in the bundle list with e->xc <= newEdge->xr
     * if such an edge exist, and the first edge otherwise
     */
    ASSERT (e == edge || (e -> xc <= newEdge -> xr &&
	(!e -> bundle || e -> bundle -> xc > newEdge -> xr)));

    if (e -> xc == newEdge -> xr) {
	disposeEdge (newEdge);
    }
    else if (e -> xc < newEdge -> xr) {
	/* insert newEdge after e */
	if (e -> bundle) 		/* if it is not last */
	    COLOR_ADD (newEdge -> color, e -> bundle -> color);
	newEdge -> bundle = e -> bundle;
	e -> bundle = newEdge;
    }
    else {
	ASSERT (e == edge);	 /* e is first edge */
	ASSERT (newEdge -> xr < edge -> xc);
	newEdge -> xc = e -> xc;
	e -> xc = newEdge -> xr;
	if (!(newEdge -> cc & 0x400)) e -> cc = newEdge -> cc;
	m = newEdge -> color;
	newEdge -> color = e -> color;
	COLOR_ADD (e -> color, m);
	newEdge -> bundle = e -> bundle;
	e -> bundle = newEdge;
    }

#ifdef DEBUG
    for (e = edge; e -> bundle; e = e -> bundle) {
	ASSERT (e -> xc < e -> bundle -> xc);
    }
    ASSERT (e -> xc == edge -> xr);
#endif
}

Private void unbundle (edge_t *edge) /* delete overlapping edge */
{
    edge_t * b = edge -> bundle;

    Debug (printEdge ("unbundle", edge));

    edge -> color  = b -> color;
    edge -> xc     = b -> xc;
    edge -> bundle = b -> bundle;
    disposeEdge (b);
}
