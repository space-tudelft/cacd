/*
 * ISC License
 *
 * Copyright (C) 1993-2018 by
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

#include "src/space/include/config.h"
#include <stddef.h>
#include <stdio.h>
#include "src/space/auxil/auxil.h"
#include "src/space/makegln/makegln.h"
#include "src/space/auxil/proto.h"
#include "src/space/makegln/proto.h"

#define X(y,x1,y1,x2,y2)  (x1 == x2 ? x1 : XX (y, x1, y1, x2, y2))

#define LowerOrOn(sl,e) (sl -> x2 > prevXscan ? sl -> y2 <= Y (e, sl -> x2) : sl -> y2 <= yprevRoofEdge)
#define Above(sl,e)     (sl -> x1 > prevXscan ? sl -> y1 >  Y (e, sl -> x1) : sl -> y1 >  yprevRoofEdge)

struct shadowLine {
    coor_t x1, y1;
    coor_t x2, y2;
    struct shadowLine * next;
    struct shadowLine * prev;
};

static struct shadowLine top;
static struct shadowLine bottom;
static struct shadowLine * currSL;

static edge_t start_e;
static edge_t stop_e;

static coor_t prevX;
static coor_t prevXscan;

#ifdef __cplusplus
  extern "C" {
#endif
edge_t *createEdge (coor_t, coor_t, coor_t, coor_t, slope_t, sign_t);

/* local operations */
Private void newMeshEdge (coor_t x1, coor_t x2, coor_t y);
#ifdef DEBUG
Private void checkSL (struct shadowLine *sl);
#endif

#ifdef __cplusplus
  }
#endif

void glnInit ()
{
    top.prev = &bottom;
    bottom.next = &top;

    top.x1 = -INF;
    top.x2 = -INF;
    top.y1 = INF;
    top.y2 = INF;
    bottom.x1 = -INF;
    bottom.x2 = -INF;
    bottom.y1 = -INF;
    bottom.y2 = -INF;

    top.next = &bottom;    /* make loop through infinity */
    bottom.prev = &top;

    start_e.fwd = &stop_e;
    start_e.bwd = NULL;
    stop_e.bwd = &start_e;
    stop_e.fwd = NULL;

    start_e.xl = -INF;
    stop_e.xl = INF;

    prevX = -INF;
    prevXscan = -INF;
    currSL = &bottom;
}

void glnEnd ()
{
    edge_t *e, *e2;

    ASSERT (bottom.next == &top);

    e = start_e.fwd;
    while (e != &stop_e) {

        e2 = e -> fwd;

	Debug (printEdge ("output mesh_gln:", e));

	selectForOutput (e);
	readyForOutput (e);

	scanAdvance ();

        e = e2;
    }
}

Private coor_t XX (coor_t y, coor_t x1, coor_t y1, coor_t x2, coor_t y2)
{
    coor_t dx, dy;

    dx = x2 - x1;
    dy = y2 - y1;
    if (dx ==  dy) return (x1 + y - y1);
    if (dx == -dy) return (x1 + y1 - y);
    ASSERT (dy != 0);
    return ((coor_t)(x1 + (double)(y - y1) * dx / dy));
}

void glnUpdate (edge_t *edge, coor_t currX)
{
    static sign_t nw;
    static sign_t ne;
    static sign_t sw;
    static sign_t se;

    static edge_t *roofEdge, *shortenedEdge;
    static coor_t yRoofEdge;
    static coor_t xlRoofEdge;
    static coor_t yprevRoofEdge;
    static coor_t changeNwAtY;

    struct shadowLine * newSL;
    struct shadowLine * tmpSL;
    struct shadowLine * helpSL;
    coor_t bdr_x_low;
    coor_t bdr_y_low;
    coor_t bdr_x_up;
    coor_t bdr_y_up;
    coor_t x, yEdge;
    int left, right;

    yEdge = Y (edge, currX);

    if (currX > prevX) {
	prevXscan = prevX;
	ASSERT (nw == 0 && ne == 0 && sw == 0 && se == 0);
	currSL = &bottom;
	nw = ne = sw = se = 0;
	changeNwAtY = INF;
	shortenedEdge = NULL;
	roofEdge = edge;
	yRoofEdge = yEdge;
	xlRoofEdge = roofEdge -> xl;
    }

    while (xlRoofEdge == currX || yRoofEdge <= yEdge) {
	roofEdge = roofEdge -> fwd;
	yRoofEdge = Y (roofEdge, currX);
	xlRoofEdge = roofEdge -> xl;
    }
    ASSERT (xlRoofEdge < currX && yRoofEdge > yEdge);

    /* Note: not necessarily roofEdge -> xl < currX,
       since roofEdge may be shortened. */

    Debug (fprintf (stderr, "glnUpdate: currX=%ld nw=%d ne=%d sw=%d se=%d\n", currX, nw, ne, sw, se));
    Debug (printEdge ("edge", edge));
    Debug (printEdge ("roofEdge", roofEdge));

    if (yEdge == changeNwAtY) {
	nw = nw ? 0 : 1;
	changeNwAtY = INF;
    }

    if (edge -> xl < currX) {
	if (edge -> signLeft) nw = nw ? 0 : 1;
    }
    if (edge -> xr > currX) {
	if (edge -> sign) ne = ne ? 0 : 1;
    }

    if ((edge -> yl != edge -> yr && edge -> xl < currX) || edge -> xl == edge -> xr) {

	/* Consider only edges that are
	   (1) non-horizontal and start before currX
	or (2) are vertical or point-edges */

	if (edge -> xl == edge -> xr) { /* vertical or point-edges */
	    left  = nw ? 1 : 0;
	    right = ne ? 1 : 0;
	    bdr_x_low = bdr_x_up = edge -> xr;
	    bdr_y_low = edge -> yl;
	    bdr_y_up  = edge -> yr;
	    ASSERT (bdr_y_up >= bdr_y_low);
	    if (bdr_y_up == bdr_y_low && edge == shortenedEdge) {
		/* The edge is a point-edge.
		   See below, a normal edge (the roofEdge) can become a point-edge.
		   That edge must be skipped!  (is flagged as shortenedEdge)
		   Note: Don't even update currSL!
		*/
		shortenedEdge = NULL;
		goto ret;
	    }
	}
	else if (edge -> yr > edge -> yl) { /* non-horizontal */
	    left  = nw ? 1 : 0;
	    right = sw ? 1 : 0;
	    bdr_x_low = Max (prevXscan, edge -> xl);
	    bdr_x_up  = currX;
	    bdr_y_low = Y (edge, bdr_x_low);
	    bdr_y_up  = yEdge;
	    ASSERT (bdr_y_up > bdr_y_low);
	}
	else { /* non-horizontal */
	    ASSERT (edge -> yr < edge -> yl);
	    left  = sw ? 1 : 0;
	    right = nw ? 1 : 0;
	    bdr_x_low = currX;
	    bdr_x_up  = Max (prevXscan, edge -> xl);
	    bdr_y_low = yEdge;
	    bdr_y_up  = Y (edge, bdr_x_up);
	    ASSERT (bdr_y_up > bdr_y_low);
	}

	yprevRoofEdge = Y (roofEdge, prevXscan);

	Debug (fprintf (stderr, "Real Update xl=%d yl=%ld xu=%ld yu=%ld\n",
	    bdr_x_low, bdr_y_low, bdr_x_up, bdr_y_up));

	while (currSL -> y2 <= bdr_y_low
	       || (currSL -> x2 > prevXscan && LowerOrOn (currSL, edge))) {
	    currSL = currSL -> next;
	}
	ASSERT (currSL -> y2 > bdr_y_low);

	if (left && bdr_y_up > bdr_y_low
	    && bdr_y_up > yprevRoofEdge && changeNwAtY > bdr_y_up) {

            ASSERT (roofEdge -> slope > 0 && roofEdge -> slope < INF);

	    /* First create an SL element from the roofEdge */

            helpSL = currSL;
	    while (LowerOrOn (helpSL, roofEdge)) {
		helpSL = helpSL -> next;
	    }

	    newSL = NEW (struct shadowLine, 1);
	    newSL -> x1 = prevXscan;
	    newSL -> y1 = yprevRoofEdge;
	    newSL -> x2 = currX;
	    newSL -> y2 = yRoofEdge;
	    newSL -> next = helpSL;
	    newSL -> prev = helpSL -> prev;
	    newSL -> prev -> next = newSL;
	    helpSL -> prev = newSL;

	    ASSERT (newSL -> y2 > newSL -> y1);

	    /* roofEdge is shortened !!!!!!! */

	    roofEdge -> xl = newSL -> x2;
	    roofEdge -> yl = newSL -> y2;
	    shortenedEdge = roofEdge; /* flagged, to fix a bug */

	    if (roofEdge -> signLeft) {
		changeNwAtY = roofEdge -> yl;
	    }

	    if (newSL -> y1 < currSL -> y1) {
		currSL = newSL;
		ASSERT (currSL -> y2 > bdr_y_low);
	    }
	}

	if (bdr_y_up == bdr_y_low) { /* disturbation of interconnection */
	    /* The edge can only be a point-edge, because only then bdr_y_up == bdr_y_low.
	    */
	    ASSERT (left || right); /* point-edge must be inside interconnection */

	    if (currSL -> y1 < bdr_y_low && currSL -> y2 > bdr_y_low) {

		/* 'bdr' breaks shadow line (currSL) into 2 pieces */

		newSL = NEW (struct shadowLine, 1);
		newSL -> x1 = currSL -> x1;
		newSL -> y1 = currSL -> y1;
		newSL -> x2 = X (bdr_y_low, currSL -> x1, currSL -> y1,
					    currSL -> x2, currSL -> y2);
		newSL -> y2 = bdr_y_low;
		newSL -> prev = currSL -> prev;
		newSL -> next = currSL;
		newSL -> prev -> next = newSL;
		currSL -> prev = newSL;
		currSL -> x1 = newSL -> x2;
		currSL -> y1 = newSL -> y2;
		helpSL = newSL;
	    }
	}

	if (left && bdr_y_up > bdr_y_low) { /* stop of interconnection */

	    ASSERT (currSL -> y1 <= bdr_y_low);

            if (!(currSL -> y1 == bdr_y_low && currSL -> x1 == bdr_x_low)) {

		/* new mesh edge due to bottom of 'bdr' */

		if (currSL -> prev -> y2 == bdr_y_low && currSL -> prev -> x2 < currSL -> x1)
		    helpSL = currSL -> prev;
		else
		    helpSL = currSL;
		newMeshEdge (X (bdr_y_low, helpSL -> x1, helpSL -> y1,
					   helpSL -> x2, helpSL -> y2),
			     bdr_x_low, bdr_y_low);
	    }

	    while (currSL -> y2 < bdr_y_up && LowerOrOn (currSL, roofEdge)) {

		/* new mesh edge due to y2 of 'currSL' */

		if (currSL -> next -> y1 == currSL -> y2 && currSL -> next -> x1 < currSL -> x2)
		    x = currSL -> next -> x1;
		else
		    x = currSL -> x2;
		newMeshEdge (x, X (currSL -> y2, bdr_x_low, bdr_y_low, bdr_x_up, bdr_y_up),
			     currSL -> y2);

                if (currSL -> y1 < bdr_y_low) { /* 'bdr' removes top of 'currSL' */
		    currSL -> x2 = X (bdr_y_low, currSL -> x1, currSL -> y1,
					      currSL -> x2, currSL -> y2);
		    currSL -> y2 = bdr_y_low;
		    currSL = currSL -> next;
		}
		else { /* 'bdr' completely removes 'currSL' */
		    tmpSL = currSL;
		    currSL = currSL -> next;
		    currSL -> prev = tmpSL -> prev;
		    tmpSL -> prev -> next = currSL;
		    DISPOSE (tmpSL, sizeof(struct shadowLine));
		}
	    }

	    if (currSL -> y1 < bdr_y_up && !Above (currSL, roofEdge)) {

                if (currSL -> y1 < bdr_y_low) {
                    if (currSL -> y2 > bdr_y_up) { /* 'bdr' breaks 'currSL' into 2 parts */
			newSL = NEW (struct shadowLine, 1);
			newSL -> x1 = X (bdr_y_up, currSL -> x1, currSL -> y1,
					         currSL -> x2, currSL -> y2);
			newSL -> y1 = bdr_y_up;
			newSL -> x2 = currSL -> x2;
			newSL -> y2 = currSL -> y2;
			newSL -> prev = currSL;
			newSL -> next = currSL -> next;
			currSL -> next = newSL;
			newSL -> next -> prev = newSL;
		    }
		    if (currSL -> y2 > bdr_y_low) { /* 'bdr' removes top of 'currSL' */
			currSL -> x2 = X (bdr_y_low, currSL -> x1, currSL -> y1,
						  currSL -> x2, currSL -> y2);
			currSL -> y2 = bdr_y_low;
			currSL = currSL -> next;
		    }
		}
                else if (currSL -> y2 > bdr_y_up) { /* 'bdr' removes bottom of 'currSL' */
		    currSL -> x1 = X (bdr_y_up, currSL -> x1, currSL -> y1,
					      currSL -> x2, currSL -> y2);
		    currSL -> y1 = bdr_y_up;
		}
		else { /* 'bdr' completely removes 'currSL' */
		    /* currSL -> y1 >= bdr_y_low && currSL -> y2 <= bdr_y_up */
		    tmpSL = currSL;
		    currSL = currSL -> next;
		    currSL -> prev = tmpSL -> prev;
		    tmpSL -> prev -> next = currSL;
		    DISPOSE (tmpSL, sizeof(struct shadowLine));
		}
	    }
	    /* ASSERT (currSL -> y2 > bdr_y_low); THIS IS NOT ALWAYS TRUE */
	}

	if (right && bdr_y_up > bdr_y_low) { /* start of interconnection */

	    newSL = NEW (struct shadowLine, 1);
	    newSL -> x1 = bdr_x_low;
	    newSL -> y1 = bdr_y_low;
	    newSL -> x2 = bdr_x_up;
	    newSL -> y2 = bdr_y_up;
	    newSL -> next = currSL;
	    newSL -> prev = currSL -> prev;
	    newSL -> prev -> next = newSL;
	    currSL -> prev = newSL;
	    currSL = newSL;
	}
    }

ret:
    prevX = currX;
    sw = nw; se = ne;
    edge -> signLeft = edge -> sign;

#ifdef DEBUG
    if (currSL != &bottom) checkSL (currSL);
#endif
}

Private void newMeshEdge (coor_t x1, coor_t x2, coor_t y)
{
    register edge_t * e;
    edge_t * new_e;

    ASSERT (x2 > x1);

    Debug (fprintf (stderr, "NEWMESHEDGE %ld %ld %ld\n", x1, x2, y));

    e = stop_e.bwd;

    while (e -> xl > x1) {
	e = e -> bwd;
    }
    while (e -> xl == x1 && e -> yl >  y) {
	e = e -> bwd;
    }

    new_e = createEdge (x1, y, x2, y, 0, 0);

    new_e -> fwd = e -> fwd;
    new_e -> bwd = e;
    new_e -> fwd -> bwd = new_e;
    new_e -> bwd -> fwd = new_e;
}

#ifdef DEBUG
Private void checkSL (struct shadowLine *sl)
{
    if (sl == &top) sl = sl -> next;

    while (sl != &bottom) {
	ASSERT (sl -> y2 > sl -> y1);
	ASSERT (sl -> y1 >= sl -> prev -> y2);
	sl = sl -> prev;
    }
}
#endif
