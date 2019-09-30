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

static tile_t *freeTile = NULL;
static tile_t *back, *front;

extern coor_t thisX, thisY;
extern mask_t thisColor;
static coor_t XR = -INF;

void setXr (coor_t xr) { XR = xr; }

Private void TL (coor_t tl, tile_t *tile, tile_t *t_left)
{
    ASSERT (tl >= tile -> bl);
    tile -> tl = tl;
    Debug (printTile ("TL tile", tile));
    if (t_left) enumPair (t_left, tile, 'v');
}

Private void BR (coor_t xr, coor_t br, tile_t *tile, tile_t *t_bot)
{
    ASSERT (xr > tile -> xl);
    ASSERT (!freeTile);
    tile -> xr = xr;
    tile -> br = br;
    Debug (printTile ("BR tile", tile));
    if (t_bot && t_bot -> xl < xr) enumPair (t_bot, tile, 'h');
}

Private void TR (coor_t tr, tile_t *tile, tile_t *t_right, tile_t *t_top)
{
    ASSERT (tile == freeTile);
    tile -> tr = tr;
    ASSERT (t_top -> xr >= tile -> xr);
    Debug (printTile ("TR tile", tile));

    if (t_top -> xl < tile -> xr) enumPair (tile, t_top, 'h');
    enumPair (tile, t_right, 'v'); /* placed after tile t_top */
    enumTile (tile);

    if (back) { front -> t_next = tile; front = tile; }
    else back = front = tile;

    freeTile = NULL;
}

/* tileInsertEdge (edge)
 * pre:  edge is just inserted in scanline
 * post: tiles around this edge are updated
 */
void tileInsertEdge (edge_t *edge)
{
    mask_t color2;
    edge_t *ebwd = edge -> bwd;

    Debug (printEdge ("tileInsertEdge", edge));

    if (!freeTile) {
	coor_t prevY = Y(ebwd, thisX);
	BR (thisX, prevY, ebwd -> tile, ebwd -> bwd -> tile);
	freeTile = ebwd -> tile;
	NEW_TILE (ebwd -> tile, thisX, prevY, XR, Y(ebwd, XR), freeTile -> color, ebwd -> cc);
    }

    TL (thisY, ebwd -> tile, freeTile);
    NEW_TILE (edge -> tile, thisX, thisY, XR, Y(edge, XR), thisColor, edge -> cc);
    color2 = ebwd -> tile -> color;
    COLOR_XOR (color2, edge -> color);
    ASSERT (COLOR_EQ_COLOR (&thisColor, &color2));
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
	coor_t prevY = Y(ebwd, thisX);
	BR (thisX, prevY, ebwd -> tile, ebwd -> bwd -> tile);
	freeTile = ebwd -> tile;
	NEW_TILE (ebwd -> tile, thisX, prevY, XR, Y(ebwd, XR), freeTile -> color, ebwd -> cc);
    }

    TR (thisY, freeTile, ebwd -> tile, edge -> tile);
    BR (thisX, thisY, edge -> tile, Null (tile_t *));
    freeTile = edge -> tile;
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
    mask_t color2;
    edge_t *ebwd = edge -> bwd;

    Debug (printEdge ("tileCrossEdge", edge));

    if (!split) split = !COLOR_EQ_COLOR (&edge -> tile -> color, &thisColor);

    if (freeTile) { /* two tiles below edge */
	TR (thisY, freeTile, ebwd -> tile, edge -> tile);
	TL (thisY, ebwd -> tile, Null (tile_t *));
    }

    if (split) { /* two tiles above edge */
	BR (thisX, thisY, edge -> tile, ebwd -> tile);
	freeTile = edge -> tile;
	NEW_TILE (edge -> tile, thisX, thisY, XR, Y(edge, XR), thisColor, edge -> cc);
	color2 = ebwd -> tile -> color;
	COLOR_XOR (color2, edge -> color);
	ASSERT (COLOR_EQ_COLOR (&thisColor, &color2));
    }
}

void tileAdvanceScan (edge_t *edge)
{
    tile_t *old;
    ASSERT (edge -> yr == INF);

    if (freeTile) TR (INF, freeTile, edge -> bwd -> tile, edge -> tile);

    while (back) {
	if (thisX >= 0) { if (back -> xr > thisX - bandWidth) break; }
	else if (back -> xr + bandWidth > thisX) break;
	back = (old = back) -> t_next;
	clearTile (old);
    }
}

void tileStopScan ()
{
    tile_t *old;
    while (back) {
	back = (old = back) -> t_next;
	clearTile (old);
    }
}
