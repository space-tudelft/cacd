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
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/makesize/rscan.h"
#include "src/space/makesize/extern.h"

#ifdef __cplusplus
  extern "C" {
#endif

/* local operations */
Private void bundle (edge_t *newEdge, edge_t *edge);
Private void unbundle (edge_t *edge);
#ifdef DEBUG
Private void checkSR (edge_t *head, coor_t x);
#endif

#ifdef __cplusplus
  }
#endif

#define smallerAtX(e1, e2) (e2 -> xl == thisX && (e2 -> yl < thisY || \
	(e2 -> yl == thisY && e1 -> xr > thisX && compareSlope (e2, <,  e1))))

/*
 * Comment added NvdM, 891213
 * the comparison with e1 -> xr is (probably!!!) necessary because
 * the ordering in the stateruler can become wrong
 * in cases where an edge under +45 straddles the scanline
 * in say, (x,y) and an edge under -45 ends in (x,y).
 * This can, however give other problems because horizontal
 * edges ending and starting at (x,y) will not be bundled.
 */
#define equalAtX(e1, e2) (e2 -> xl == thisX && e2 -> yl == thisY && \
	compareSlope (e1, ==,  e2) && e1 -> xr > thisX)

#define TestIntersection(e1, e2) if (compareSlope (e1, !=,  e2)) rtestIntersection(e1, e2);

mask_t thisColor;

void rscan ()
{
    edge_t *edge, *head, *tail, *newEdge;
    coor_t thisX, thisY, nextX, xr;

    /*
     * init head and tail of stateruler
     */
    NEW_EDGE (head, -INF, -INF, INF, -INF, cNull);
    NEW_EDGE (tail, -INF,  INF, INF,  INF, cNull);

    head -> fwd = head -> bwd = tail;
    tail -> fwd = tail -> bwd = head;

    NEW_TILE (head -> tile, -INF, -INF, INF, -INF, cNull);
    NEW_TILE (tail -> tile, -INF,  INF, INF,  INF, cNull);
    tail -> tile -> tl = tail -> tile -> tr = -INF;

    newEdge = rfetchEdge ();
    thisX = newEdge -> xl;

    while (thisX < INF) {
	nextX = INF;

	Debug (fprintf (stderr, "thisX=%d\n", thisX));

	edge = head -> fwd;

	while (edge != tail || newEdge -> xl == thisX) {

	    if (edge != tail) {
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

		if (edge -> xi == thisX) rsplit (edge, thisX, thisY);

		if (edge -> xc == thisX && edge -> bundle) unbundle (edge);

		if (smallerAtX (edge, newEdge)) goto insert_below;
	    }
	    else {
		/* insert newEdge below edge */
insert_below:
		Debug (rprintEdge ("insert", newEdge));
		Debug (rprintEdge ("below", edge));

		newEdge -> bwd = edge -> bwd;
		newEdge -> fwd = edge;
		edge -> bwd -> fwd = newEdge;
		edge -> bwd = newEdge;

		edge = newEdge;
		newEdge = rfetchEdge ();
		thisY = edge -> yl;
	    }

	    if (equalAtX (edge, newEdge)) {
		xr = edge -> xr;
		do {
		    bundle (newEdge, edge);
		    newEdge = rfetchEdge ();
		} while (equalAtX (edge, newEdge));

		if (edge -> xr > xr) {
		    if (edge -> tile) {
			edge -> tile -> xr = edge -> xr;
			edge -> tile -> br = edge -> yr;
		    }
		    if (edge -> fwd != tail) TestIntersection (edge, edge -> fwd);
		    TestIntersection (edge, edge -> bwd);
		}
	    }

	    if (edge -> xr == thisX) {
		edge_t *efwd;
		tileDeleteEdge (edge);
		/*
		 * delete edge from scanline
		 */
		efwd = edge -> fwd;
		Debug (rprintEdge ("delete", edge));
		edge -> bwd -> fwd = efwd;
		efwd -> bwd = edge -> bwd;
		rdisposeEdge (edge);
		edge = efwd;
		if (edge != tail) TestIntersection (edge, edge -> bwd);
	    }
	    else {
		COLOR_XOR (thisColor, edge -> color);
		if (edge -> xl == thisX) {
		    TestIntersection (edge, edge -> bwd);
		    tileInsertEdge (edge);
		}
		else {
		    if (edge -> bwd -> xl == thisX) TestIntersection (edge, edge -> bwd);
                    tileCrossEdge (thisX, edge);
		}

		if (edge -> xi < nextX) nextX = edge -> xi;
		if (edge -> xc < nextX) nextX = edge -> xc;

		edge = edge -> fwd;
	    }
	}

	/* important test: thisColor must be zero */
	if (IS_COLOR (&thisColor)) ASSERT (!IS_COLOR (&thisColor));

	if (!(nextX > thisX)) {
	    fprintf(stderr,"thisX=%d nextX=%d\n", thisX, nextX);
	    ASSERT (nextX > thisX);
	}
	if (!(newEdge -> xl > thisX)) {
	    fprintf(stderr,"thisX=%d newEdge->xl=%d\n", thisX, newEdge -> xl);
	    ASSERT (newEdge -> xl > thisX);
	}

	ASSERT (edge == tail);
	tileAdvanceScan ();

	Debug2 (checkSR (head, thisX));

	if (newEdge -> xl < nextX) nextX = newEdge -> xl;

	ASSERT (nextX > thisX);
	thisX = nextX;
    }

    ASSERT (head -> fwd == tail);

    disposeTile (head -> tile);
    disposeTile (tail -> tile);

    /* Remove head and tail of stateruler.
     * These can not be retained, since the tiles
     * have a cons array depending on resistance extraction.
     * With display mode, resistance extraction can be turned
     * on or off.
     */
    rdisposeEdge (tail);
    rdisposeEdge (head);
}

/*
 * insert overlapping edge
 */
Private void bundle (edge_t *newEdge, edge_t *edge)
{
    edge_t * e = edge, * e1 = edge;
    mask_t m;

    Debug (rprintEdge ("bundle", newEdge));
    Debug (rprintEdge ("with",  edge));

 // ASSERT (compareSlope (edge, ==, newEdge));
    ASSERT (newEdge -> xr == newEdge -> xc);

    if (e -> xr == newEdge -> xl) {
	ASSERT (e -> bundle == NULL);
	e -> xr = newEdge -> xr;
	e -> yr = newEdge -> yr;
	e -> xc = newEdge -> xc;
	e -> color = newEdge -> color;
	rdisposeEdge (newEdge);
	goto post;
    }

    if (newEdge -> xr > edge -> xr)
	edge -> xr = newEdge -> xr, edge -> yr = newEdge -> yr;

    while (e1 && e1 -> xc <= newEdge -> xr) {
	e1 = (e = e1) -> bundle;
	COLOR_ADD (e -> color, newEdge -> color);
    }

    /* e is the last edge in the bundle list with e->xc <= newEdge->xr
     * if such an edge exist, and the first edge otherwise
     */

    ASSERT (e == edge || (e -> xc <= newEdge -> xr &&
	(!e -> bundle || e -> bundle -> xc > newEdge -> xr)));

    if (e -> xc == newEdge -> xr) {
	rdisposeEdge (newEdge);
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
	m = newEdge -> color;
	newEdge -> color = e -> color;
	COLOR_ADD (e -> color, m);
	newEdge -> bundle = e -> bundle;
	e -> bundle = newEdge;
    }

post:

#ifdef DEBUG
    for (e = edge; e -> bundle; e = e -> bundle) {
	ASSERT (e -> xc < e -> bundle -> xc);
    }
    ASSERT (e -> xc == edge -> xr);
#endif

    return;
}

/*
 * delete overlapping edge
 */
Private void unbundle (edge_t *edge)
{
    edge_t * b = edge -> bundle;

    Debug (rprintEdge ("unbundle", edge));

    edge -> color  = b -> color;
    edge -> xc     = b -> xc;
    edge -> bundle = b -> bundle;
    rdisposeEdge (b);
}

#ifdef DEBUG
Private void checkSR (edge_t *head, coor_t x)
{
    edge_t * e;
    for (e = head -> fwd; e != head -> bwd; e = e -> fwd) {
	if (DEBUG) rprintEdge ("edge", e);
	ASSERT (e -> xr > x);
	ASSERT (e -> xl <= x);
	ASSERT (Y (e -> fwd, x) > Y (e, x) ||
	   (Y (e -> fwd, x) == Y (e, x) && compareSlope (e -> fwd, >, e)));
    }
}
#endif /* DEBUG */
