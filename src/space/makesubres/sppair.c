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

#define LFT_TOP 0
#define RGT_BOT 1

extern double subcontZposition;

Private spider_t * spiderFindNew (tile_t *tile_l, tile_t *tile_r,
		int side, meshCoor_t x, meshCoor_t y, spider_t *spider)
{
    face_t *face;
    tile_t *from, *tile = (side == RGT_BOT)? tile_l : tile_r;
    meshCoor_t z = subcontZposition;

    face = tile -> t_face;
    ASSERT (face);
    if (spider) { from = (tile == tile_l)? tile_r : tile_l; goto found; }

    /* Lookup spider, and create new spider if not found.
    */
    spider = tile_r -> t_face ? tile_r -> t_face -> corners[LFT_TOP] : 0;
    if (spider && Nearby (spider, x, y, z)) { from = tile_r; goto found; }

    spider = tile_l -> t_face ? tile_l -> t_face -> corners[RGT_BOT] : 0;
    if (spider && Nearby (spider, x, y, z)) { from = tile_l; goto found; }

    spider = stripFindSpider (x, y, z);
    from = NULL;
found:
    Debug (fprintf (stderr, "spider at (%g,%g,%g) %s in tile %p from %p\n",
	D(x), D(y), D(z), spider ? "found" : "not found", tile, from));

    if (!spider) {
	ASSERT (tile -> subcont);
	spider = newSpider (x, y, z, tile -> subcont, NULL);
	ASSERT (spider);
	Debug (fprintf (stderr, "New spider at (%g,%g,%g) created\n", D(x), D(y), D(z)));
    }

    /* Store position of spider in the tile.  The storage scheme is based
    ** on the order of enumeration of tile boundaries.
    */
    Debug (fprintf (stderr, "cache (%g,%g,%g) in %s of %p\n",
	D(x), D(y), D(z), side == LFT_TOP ? "lft_top" : "rgt_bot", tile));

    if (!face -> corners[side]) {
	face -> corners[side == LFT_TOP ? RGT_BOT : LFT_TOP] = spider;
	Debug (fprintf (stderr, "cache (%g,%g,%g) in %s also\n",
		D(x), D(y), D(z), side == LFT_TOP ? "rgt_bot" : "lft_top"));
    }
    face -> corners[side] = spider;
    return (spider);
}

/* Note that tile_l is left tile and tile_r is right tile
 * when traversing their common boundary from x1,y1 to x2,y2.
 */
void spiderPair (tile_t *tile_l, tile_t *tile_r, int orientation)
{
    meshCoor_t x1, y1, x2, y2;
    spider_t *tl1, *tl2, *tr1, *tr2;

    if (orientation == 'v') {
	x1 = x2 = tile_l -> xr;
	y2 = Min (tile_l -> tr, tile_r -> tl);
	y1 = Max (tile_l -> br, tile_r -> bl);
    }
    else {
	if (tile_l -> xr < tile_r -> xr) {
		x2 = tile_l -> xr; y2 = tile_l -> tr; }
	else {	x2 = tile_r -> xr; y2 = tile_r -> br; }
	if (tile_l -> xl > tile_r -> xl) {
		x1 = tile_l -> xl; y1 = tile_l -> tl; }
	else {	x1 = tile_r -> xl; y1 = tile_r -> bl; }

	Swap (tile_t *, tile_l, tile_r);
    }

    Debug (fprintf (stderr, "spiderPair: %c (%g,%g) (%g,%g) tiles: %p, %p\n",
	orientation, D(x1), D(y1), D(x2), D(y2), tile_l, tile_r));

    tl1 = tr1 = tl2 = tr2 = NULL;
    if (orientation == 'v' && tile_l -> subcont) {
	tl1 = spiderFindNew (tile_l, tile_r, RGT_BOT, x1, y1, tr1);
	tl2 = spiderFindNew (tile_l, tile_r, RGT_BOT, x2, y2, tr2);
	ASSERT (tl1 != tl2);
    }
    if (tile_r -> subcont) {
	tr1 = spiderFindNew (tile_l, tile_r, LFT_TOP, x1, y1, tl1);
	tr2 = spiderFindNew (tile_l, tile_r, LFT_TOP, x2, y2, tl2);
	ASSERT (tr1 != tr2);
    }
    if (orientation != 'v' && tile_l -> subcont) {
	tl1 = spiderFindNew (tile_l, tile_r, RGT_BOT, x1, y1, tr1);
	tl2 = spiderFindNew (tile_l, tile_r, RGT_BOT, x2, y2, tr2);
	ASSERT (tl1 != tl2);
    }

    if (tl1 && tr1) {
	ASSERT (tl1 == tr1 && tl2 == tr2);
	meshMakeEdge (tl1, tl2, INTERNALEDGE);
	meshSetFace (tl1, tl2, tile_l -> t_face);
	meshSetFace (tr2, tr1, tile_r -> t_face);
    }
    else if (tl1) {
	meshMakeEdge (tl1, tl2, CONDUCTOREDGE);
	meshSetFace (tl1, tl2, tile_l -> t_face);
    }
    else {
	meshMakeEdge (tr1, tr2, CONDUCTOREDGE);
	meshSetFace (tr2, tr1, tile_r -> t_face);
    }
}
