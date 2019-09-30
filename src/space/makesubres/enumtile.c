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

void enumPair (tile_t *tile, tile_t *newerTile, int edgeOrien)
{
    coor_t a, b;
    subcontRef_t *tsc = tile -> subcont;
    subcontRef_t *nsc = newerTile -> subcont;

    if (!nsc) {
	if (IS_COLOR (&newerTile -> color)) {
	    ++tileConCnt;
	    newerTile -> subcont = nsc = subContNew (newerTile);
	    newerTile -> t_face = newFace ();
	    if (newerTile -> cc & 0x200) nsc -> distributed = 1;
	    nsc -> causing_con = newerTile -> cc & 0xff;
	}
	else if (!tsc) return;
    }

    if (edgeOrien == 'v') {
	b = Min (tile -> tr, newerTile -> tl);
	a = Max (tile -> br, newerTile -> bl);
    }
    else {
	b = Min (tile -> xr, newerTile -> xr);
	a = Max (tile -> xl, newerTile -> xl);
    }
    if (b > a) { // tiles have a common boundary (edge len > 0)
	if (tsc && nsc) {
	    if (!tsc -> distributed && !nsc -> distributed
		    && tsc -> causing_con == nsc -> causing_con)
		subContJoin (tile, newerTile);
	    else
		subContGroupJoin (tsc -> subcontInfo, nsc -> subcontInfo);
	}
	spiderPair (tile, newerTile, edgeOrien);
    }
}

void enumTile (tile_t *tile)
{
    ++tileCnt;
    if (tile -> subcont) spiderTile (tile);
}

void clearTile (tile_t *tile)
{
    if (tile -> subcont) subContDel (tile);
    disposeTile (tile);
}
