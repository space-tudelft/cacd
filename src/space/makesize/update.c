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
#include "src/space/makesize/rscan.h"
#include "src/space/makesize/extern.h"

#ifdef __cplusplus
  extern "C" {
#endif
/* local operations */
Private void TL (coor_t tl, tile_t *tile);
Private void BR (coor_t xr, coor_t br, tile_t *tile, tile_t *t_bot);
Private void TR (coor_t tr, tile_t *tile, tile_t *t_right, tile_t *t_top);
#ifdef __cplusplus
  }
#endif

extern mask_t thisColor;
static tile_t *freeTile = NULL;

/* tileInsertEdge (edge)
 * pre:  edge is just inserted in scanline
 * post: tiles around this edge are updated
 */
void tileInsertEdge (edge_t *edge)
{
    coor_t thisX = edge -> xl,	/* current X value */
	   thisY = edge -> yl;	/* current Y value */
    edge_t *ebwd = edge -> bwd;

    Debug (rprintEdge ("tileInsertEdge", edge));

    if (!freeTile) {
	coor_t prevY = Y (ebwd, thisX);
	BR (thisX, prevY, ebwd -> tile, ebwd -> bwd -> tile);
	NEW_TILE (ebwd -> tile, thisX, prevY, ebwd -> xr, ebwd -> yr, freeTile -> color);
    }

    TL (thisY, ebwd -> tile);
    NEW_TILE (edge -> tile, thisX, thisY, edge -> xr, edge -> yr, thisColor);
    {
	mask_t color2 = ebwd -> tile -> color;
	COLOR_XOR (color2, edge -> color);
	ASSERT (COLOR_EQ_COLOR (&thisColor, &color2));
    }
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
    coor_t thisX = edge -> xr,
	   thisY = edge -> yr;
    edge_t *ebwd = edge -> bwd;

    Debug (rprintEdge ("tileDeleteEdge", edge));

    if (!freeTile) {
	coor_t prevY = Y (ebwd, thisX);
	BR (thisX, prevY, ebwd -> tile, ebwd -> bwd -> tile);
	NEW_TILE (ebwd -> tile, thisX, prevY, ebwd -> xr, ebwd -> yr, freeTile -> color);
    }

    TR (thisY, freeTile, ebwd -> tile, edge -> tile);
    BR (thisX, thisY, edge -> tile, Null (tile_t *));
}

/* tileCrossEdge (thisX, edge)
 * pre:  edge is crossed in scanline
 *       edge -> xl < thisX < edge -> xr
 * post: if there is a free tile, it is finished.
 *
 *       The scanline intersects the edge, there are at least two tiles,
 *       1 above and 1 below the edge, but each of them may be splitted
 *       because the color to the left and right of the scanline may be
 *       different.
 */
void tileCrossEdge (coor_t thisX, edge_t *edge)
{
    coor_t thisY;
    edge_t *ebwd = edge -> bwd;

    Debug (rprintEdge ("tileCrossEdge", edge));

    if (freeTile) { /* two tiles below edge */
        thisY = Y (edge, thisX);
	TR (thisY, freeTile, ebwd -> tile, edge -> tile);
	TL (thisY, ebwd -> tile);
    }

    if (!COLOR_EQ_COLOR (&edge -> tile -> color, &thisColor)) { /* two tiles above edge */
	thisY = Y (edge, thisX);
	BR (thisX, thisY, edge -> tile, ebwd -> tile);
	NEW_TILE (edge -> tile, thisX, thisY, edge -> xr, edge -> yr, thisColor);
	{
	    mask_t color2 = ebwd -> tile -> color;
	    COLOR_XOR (color2, edge -> color);
	    ASSERT (COLOR_EQ_COLOR (&thisColor, &color2));
	}
    }
}

void tileAdvanceScan ()
{
    if (freeTile) {
	disposeTile (freeTile);
	freeTile = NULL;
    }
}

Private void TL (coor_t tl, tile_t *tile)
{
    Debug (printTile ("TL tile", tile));
    ASSERT (tl >= tile -> bl);
    tile -> tl = tl;
    if (freeTile) enumPair (freeTile, tile, 'v');
}

Private void BR (coor_t xr, coor_t br, tile_t *tile, tile_t *t_bot)
{
    Debug (printTile ("BR tile", tile));
    ASSERT (freeTile == NULL);
    freeTile = tile;
    ASSERT (xr > tile -> xl);
    tile -> xr = xr;
    tile -> br = br;
    if (t_bot && t_bot -> xl < xr) enumPair (t_bot, tile, 'h');
}

Private void TR (coor_t tr, tile_t *tile, tile_t *t_right, tile_t *t_top)
{
    Debug (printTile ("TR tile", tile));
    ASSERT (tile == freeTile);
    ASSERT (t_top -> xr >= tile -> xr);
    ASSERT (t_top -> xl <  tile -> xr);
    tile -> tr = tr;
    enumPair (tile, t_top,   'h');
    enumPair (tile, t_right, 'v');
    enumTile (tile);
    freeTile = NULL;
}
