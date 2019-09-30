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
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/scan.h"
#include "src/space/scan/extern.h"
#include "src/space/extract/export.h"

#ifdef CAP3D
extern coor_t stripRleftX (void);
#endif

static tile_t * freeTile = NULL;

#ifdef __cplusplus
  extern "C" {
#endif

/* local operations */
Private void TL (coor_t y, tile_t *tile, tile_t *t_left);
Private void BR (coor_t y, tile_t *tile, tile_t *t_bot);
Private void TR (coor_t y, tile_t *tile, tile_t *t_right, tile_t *t_top);
Private void trySliceDown (edge_t *startEdge);

#ifdef __cplusplus
  }
#endif

#define HasCon(tile) !COLOR_ABSENT (&tile -> color, &filterBitmask)
#define HasRes(tile) !COLOR_ABSENT (&tile -> color, &resBitmask)
#define HasResC(color) !COLOR_ABSENT (&color, &resBitmask)

extern coor_t thisX, thisY, lastY;
extern mask_t thisColor;
extern double effectDist;

/* tileInsertEdge (edge)
 * pre:  edge is just inserted in scanline
 * post: tiles around this edge are updated
 */
void tileInsertEdge (edge_t *edge)
{
    edge_t *ebwd = edge -> bwd;

    Debug (printEdge ("tileInsertEdge", edge));

    if (!freeTile) {
	coor_t prevY = Y (ebwd, thisX);
	if (optResMesh == 1 && prevY > -INF && HasRes (ebwd -> tile)) trySliceDown (ebwd);
	BR (prevY, ebwd -> tile, ebwd -> bwd -> tile);
	freeTile = ebwd -> tile;
	NEW_TILE (ebwd -> tile, thisX, prevY, ebwd -> xr, ebwd -> yr, freeTile -> color, ebwd -> bwd -> tile, freeTile, ebwd -> cc);
    }

    TL (thisY, ebwd -> tile, freeTile);
    NEW_TILE (edge -> tile, thisX, thisY, edge -> xr, edge -> yr, thisColor, ebwd -> tile, freeTile, edge -> cc);
}

/* tileDeleteEdge (edge)
 * pre:  edge is just to be deleted from scanline (but is not so already).
 *       edge forms the top-right point of the tile below
 *       and the bottom-right point of the tile above the edge.
 * post: if there is a free tile, it is finished
 *       tile connected to this edge becomes free,
 */
void tileDeleteEdge (edge_t *edge)
{
    edge_t *ebwd = edge -> bwd;

    Debug (printEdge ("tileDeleteEdge", edge));

    if (!freeTile) {
	coor_t prevY = Y (ebwd, thisX);
	if (optResMesh == 1 && prevY > -INF && HasRes (ebwd -> tile)) trySliceDown (ebwd);
	BR (prevY, ebwd -> tile, ebwd -> bwd -> tile);
	freeTile = ebwd -> tile;

	/* The tile will have coordinates that assure
	 * that the slope of its edges is correct, in order
	 * not to confuse the lateral coupling cap computations.
	 */
	NEW_TILE (ebwd -> tile, thisX, prevY, ebwd -> xr, ebwd -> yr, freeTile -> color, ebwd -> bwd -> tile, freeTile, ebwd -> cc);
    }

    TR (thisY, freeTile, ebwd -> tile, edge -> tile);
    freeTile = edge -> tile; /* BR: new freeTile */
    freeTile -> xr = thisX;
    freeTile -> br = thisY;
}

/* tileCrossEdge (edge, split)
 * pre:  edge is crossed in scanline
 *       edge -> xl < thisX < edge -> xr
 * post: if there is a free tile, it is finished.
 *
 *       The scanline intersects the edge, there are at least two tiles,
 *       1 above and 1 below the edge, but each of them may be splitted
 *       because the color to the left and right of the scanline may be
 *       different.
 */
void tileCrossEdge (edge_t *edge, int split)
{
    tile_t *tbwd, *tile;
    edge_t *ebwd = edge -> bwd;

    tbwd = ebwd -> tile;
    tile = edge -> tile;

    Debug (printEdge ("tileCrossEdge", edge));

    if (!split) split = !COLOR_EQ_COLOR (&tile -> color, &thisColor);

    if (freeTile) { /* two tiles below edge */
do_tr:
	if (!split && optResMesh == 1 && HasRes (tile) && (HasRes (freeTile) || HasRes (tbwd)) &&
	    (max_tan_slice_y < 0 ||
	    ((double)Y (edge -> fwd, thisX) - thisY) / ((double)thisX - tile -> xl)
			<= max_tan_slice_y)) split = 1;
	tbwd -> tl = thisY; /* TL */
	TR (thisY, freeTile, tbwd, tile);
    }
    else if (!split) {
	if (lastY != INF) { /* lateral cap "oe" */
	    double w = thisY - lastY;
	    if (edge -> yr != edge -> yl) w /= sqrt (2.);
	    if (w > effectDist) lastY = INF;
	    else if ((edge -> xc - thisX) > effectDist && tbwd -> xl < thisX)
		 if (HasCon (tile) && hasLatCap (tbwd, tile)) split = 1;
	}
	if (edge -> yr != edge -> yl) {
	    if (edge -> yr < edge -> yl) {
		if (!split && HasCon (tile) && hasLatCap (tbwd, tile)) split = 1;
	    }
	    else if (HasCon (tbwd) && hasLatCap (tile, tbwd)) { // split tbwd
		coor_t prevY = Y (ebwd, thisX);
		BR (prevY, tbwd, ebwd -> bwd -> tile);
		freeTile = tbwd;
		NEW_TILE (tbwd, thisX, prevY, ebwd -> xr, ebwd -> yr, freeTile -> color, ebwd -> bwd -> tile, freeTile, ebwd -> cc);
		ebwd -> tile = tbwd;
		goto do_tr;
	    }
	}
    }
    else if (optResMesh == 1 && tbwd -> bl > -INF && (HasRes (tile) || HasResC (thisColor))) {
	trySliceDown (edge);
	tbwd = ebwd -> tile;
    }

    if (split) { /* two tiles above edge */
	BR (thisY, tile, tbwd);
	freeTile = tile;
	NEW_TILE (edge -> tile, thisX, thisY, edge -> xr, edge -> yr, thisColor, tbwd, freeTile, edge -> cc);
    }
}

void tileAddTerm (edge_t *ebwd, coor_t termY)
{
    Debug (printEdge ("tileAddTerm", ebwd));

    if (!freeTile) {
	coor_t prevY = Y (ebwd, thisX);
	ASSERT (termY >= prevY);
	if (optResMesh == 1 && prevY > -INF) trySliceDown (ebwd);
	freeTile = ebwd -> tile;
	ASSERT (thisX < freeTile -> xr);
	BR (prevY, freeTile, ebwd -> bwd -> tile);
	NEW_TILE (ebwd -> tile, thisX, prevY, ebwd -> xr, ebwd -> yr, freeTile -> color, ebwd -> bwd -> tile, freeTile, ebwd -> cc);
    }
    else
	ASSERT (freeTile -> xr == thisX);
}

static tile_t *back, *front, *backR;
static tile_t *doit; /* bandWidth2 */

void tileAdvanceScan (edge_t *edge)
{
    ASSERT (edge -> yr == INF);
    lastY = INF;

    if (freeTile) TR (INF, freeTile, edge -> bwd -> tile, edge -> tile);

    /* while (back && back -> xr <= thisX - bandWidth),
     * but watch out for overflow
     */
    if (doit && thisX > -INF + bandWidth2) {
	coor_t posX = thisX - bandWidth2;
	while (doit && doit->xr <= posX) { enumTile (doit); doit = doit -> next; }
    }
    while (back) {
	if (thisX >= 0) { if (back -> xr > thisX - bandWidth) break; }
	else if (back -> xr + bandWidth > thisX) break;

	back -> stt -> stb = back -> str; /* !!! */
	clearTile (back); disposeTile (back);
	back = back -> next;
    }
}

void tileStopScan (edge_t *head)
{
    while (doit) { enumTile (doit); doit = doit -> next; }
    while (back) {
	clearTile (back); disposeTile (back);
	back = back -> next;
    }
    backR = NULL;
    /* dispose the special tiles created in initScan */
    disposeTile (head -> tile);
    disposeTile (head -> fwd -> tile);
}

Private void TL (coor_t y, tile_t *tile, tile_t *t_left)
{
    tile -> tl = y;
    Debug (printTile ("TL tile", tile));
    enumPair (t_left, tile, 'v');
}

Private void BR (coor_t y, tile_t *tile, tile_t *t_bot)
{
 /* ASSERT (!freeTile); */
    tile -> xr = thisX;
    tile -> br = y;
    Debug (printTile ("BR tile", tile));

    if (thisX <= tile -> xl) { say ("error: BR: tile->xr=%d <= tile->xl=%d (br=%d)", thisX, tile->xl, y); die (); }

    if (t_bot -> xl < thisX) enumPair (t_bot, tile, 'h');
}

Private void TR (coor_t y, tile_t *tile, tile_t *t_right, tile_t *t_top)
{
 /* ASSERT (tile == freeTile); */
    ASSERT (tile -> xr == thisX);
    tile -> tr = y;
    tile -> str = t_right;		/* corner stitches */
    tile -> stt = t_top;
    Debug (printTile ("TR tile", tile));

    if (t_top -> xr <  thisX) { say ("error: TR: t_top->xr=%d <  tile->xr=%d (tr=%d)", t_top->xr, thisX, y); die (); }
    if (t_top -> xl >= thisX) { say ("error: TR: t_top->xl=%d >= tile->xr=%d (tr=%d)", t_top->xl, thisX, y); die (); }

    enumPair (tile, t_top,   'h');
    enumPair (tile, t_right, 'v');
    if (!bandWidth2) enumTile (tile);

    if (bandWidth == 0) { clearTile (tile); disposeTile (tile); }
    else { /* inject tile */
	if (back) { front -> next = tile; front = tile; }
	else {
	    back = front = tile;
	    if (bandWidth2) doit = tile;
	}
    }
    freeTile = NULL;
}

Private void trySliceDown (edge_t *startEdge)
{
    edge_t *edge, *ebwd;
    coor_t y;

 /* ASSERT (!freeTile); */
    ebwd = (edge = startEdge) -> bwd;
    while (ebwd -> tile -> xl < thisX && HasRes (ebwd -> tile) &&
	(max_tan_slice_y < 0 ||
	((double)Y (edge, thisX) - Y (ebwd, thisX)) / ((double)thisX - ebwd -> tile -> xl)
			<= max_tan_slice_y)) {
	ebwd = (edge = ebwd) -> bwd;
    }

    while (edge != startEdge) {
	y = Y (edge, thisX);
	BR (y, edge -> tile, ebwd -> tile);
	freeTile = edge -> tile;
	NEW_TILE (edge -> tile, thisX, y, edge -> xr, edge -> yr, freeTile -> color, ebwd -> tile, freeTile, edge -> cc);
	edge = (ebwd = edge) -> fwd;
	y = Y (edge, thisX);
	TR (y, freeTile, ebwd -> tile, edge -> tile);
	ebwd -> tile -> tl = y; /* TL */
    }
}

#ifdef CAP3D
void findCenterSpiderInit ()
{
    coor_t xl = stripRleftX ();

    backR = back;
    while (backR && backR->xr < xl) backR = backR -> next;
}

nodePoint_t * findCenterSpider (int sidewall, double x, double y, int cx, tile_t **tile) // used in spider/refine.c
{
    nodePoint_t *p, *pn;
    double dx, dy, dy2;
    tile_t *t;
#ifdef DEBUG_SPIDER
    char *s = NULL;
#endif

    p = NULL; // surpress uninitialized warning

    if (sidewall) { /* face */
	/* point is always on an edge of the tile */
	for (t = backR; t; t = t -> next) {
	    if (!t -> cons[cx]) continue;
	    if (x == t -> xr) {
		if (y < t -> br || y > t -> tr) continue;
		if (!(p = t -> rbPoints) || !(t -> cons[cx] -> highres & 1)) goto ret;
#ifdef DEBUG_SPIDER
		s = "SW_XR";
#endif
		ASSERT (p -> next);
		pn = p -> next;
		while (pn -> x == t -> xr && y < pn -> y) { p = pn; pn = p -> next; }
		if (pn -> x == t -> xr)
		    if ((y - pn -> y) < (p -> y - y)) p = pn;
		goto ret;
	    }
	    if (x == t -> xl) {
		if (y < t -> bl || y > t -> tl) continue;
		if (!(p = t -> tlPoints) || !(t -> cons[cx] -> highres & 1)) goto ret;
#ifdef DEBUG_SPIDER
		s = "SW_XL";
#endif
		ASSERT (p -> prev);
		while (p -> x != t -> xl) { p = p -> prev; ASSERT (p); }
		pn = p -> prev;
		while (pn -> x == t -> xl && y < pn -> y) { p = pn; pn = p -> prev; }
		if (pn -> x == t -> xl)
		    if ((y - pn -> y) < (p -> y - y)) p = pn;
		goto ret;
	    }
	    if (x > t -> xl && x < t -> xr) {
		if ((dy = t -> bl) != t -> br) {
		    if (dy < t -> br) dy += x - t -> xl; else dy -= x - t -> xl;
		}
		if ((dy -= y) < 0) dy = -dy;
		if (dy < 0.1) { /* bottom edge */
		    if (!(p = t -> rbPoints) || !(t -> cons[cx] -> highres & 1)) goto ret;
#ifdef DEBUG_SPIDER
		    s = "SW_YB";
#endif
		    ASSERT (p -> next);
		    while (p -> y != t -> br) { p = p -> next; ASSERT (p); }
		    pn = p -> next;
		    while (x < pn -> x) { p = pn; pn = p -> next; }
		    if ((x - pn -> x) < (p -> x - x)) p = pn;
		    goto ret;
		}
		if ((dy = t -> tl) != t -> tr) {
		    if (dy < t -> tr) dy += x - t -> xl; else dy -= x - t -> xl;
		}
		if ((dy -= y) < 0) dy = -dy;
		if (dy < 0.1) { /* top edge */
		    if (!(p = t -> tlPoints) || !(t -> cons[cx] -> highres & 1)) goto ret;
#ifdef DEBUG_SPIDER
		    s = "SW_YT";
#endif
		    ASSERT (p -> prev);
		    pn = p -> prev;
		    while (x < pn -> x) { p = pn; pn = p -> prev; }
		    if ((x - pn -> x) < (p -> x - x)) p = pn;
		    goto ret;
		}
	    }
	}
    }
    else { /* non sidewall face */
	for (t = backR; t; t = t -> next) {
	    if (!t -> cons[cx]) continue;
	    if (x <= t -> xr && x > t -> xl) {
		dy = t -> br;
		if (t -> bl > t -> br) dy += t -> xr - x;
		if (t -> bl < t -> br) dy -= t -> xr - x;
		if (y < dy + 0.1) continue;

		dy = t -> tr;
		if (t -> tl > t -> tr) dy += t -> xr - x;
		if (t -> tl < t -> tr) dy -= t -> xr - x;
		if (y > dy + 0.1) continue;

		if (!(p = t -> rbPoints) || !(t -> cons[cx] -> highres & 1)) goto ret;
		ASSERT (p -> next);
#ifdef DEBUG_SPIDER
		s = "TB_IN";
#endif
		dx = p -> x - x; dx *= dx;
		dy = p -> y - y; dy *= dy;
		pn = p -> next;
		while (pn -> x == t -> xr) {
		    dy2 = pn -> y - y; dy2 *= dy2;
		    if (dy2 < dy) { p = pn; dy = dy2; }
		    pn = pn -> next;
		}
		dy += dx; // distance^2
		while (pn -> x != t -> xl) {
		    dx = pn -> x - x; dx *= dx; dy2 = pn -> y - y; dy2 *= dy2; dy2 += dx;
		    if (dy2 < dy) { p = pn; dy = dy2; }
		    pn = pn -> next;
		}
		pn = t -> tlPoints -> prev;
		while (pn -> x != t -> xl) {
		    dx = pn -> x - x; dx *= dx; dy2 = pn -> y - y; dy2 *= dy2; dy2 += dx;
		    if (dy2 < dy || (dy2 == dy && pn->x >= p->x)) { p = pn; dy = dy2; }
		    pn = pn -> prev;
		}
		dx = x - t -> xl; dx *= dx; dy -= dx;
		if (dy > 0)
		while (pn -> x == t -> xl) {
		    dy2 = pn -> y - y; dy2 *= dy2;
		    if (dy2 < dy) { p = pn; dy = dy2; }
		    pn = pn -> prev;
		}
		goto ret;
	    }
	}
    }
ret:
    ASSERT (t);
#ifdef DEBUG_SPIDER
    if (s) fprintf (stderr, "-- tile %s xl=%3d xr=%3d yb=%3d yt=%3d con=%d p(%3d,%3d)\n",
		    s, t->xl/4, t->xr/4, t->bl/4, t->tl/4, cx, p->x/4, p->y/4);
#endif
    *tile = t;
    return p;
}
#endif /* CAP3D */
