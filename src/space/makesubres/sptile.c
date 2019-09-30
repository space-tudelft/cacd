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

void spiderTile (tile_t *tile)
{
    int i;
    face_t *face;
    meshCoor_t xl, xr;
    spider_t *tr, *sp, *sp2, *c[4];

    Debug (fprintf (stderr, "tile: %p xl=%d xr=%d bl=%d br=%d\n", tile,
	I(tile -> xl), I(tile -> xr), I(tile -> bl), I(tile -> br)));

    xl = tile -> xl;
    xr = tile -> xr;

    if ((face = tile -> t_face)) {
	tr = face -> corners[0];
	ASSERT (tr);

	/* For all spiders around the current face,
	 * select those which are at a corner of the tile.
	 */
	c[i = 0] = tr;
	for (sp = ccwa (tr, face); sp != tr; sp = sp2) {
	    sp2 = ccwa (sp, face);
	    if (sp -> act_x == xl) {
		if (sp -> act_y == tile -> bl || sp -> act_y == tile -> tl)
		    c[++i] = sp; /* tile corner spider */
	    }
	    else if (sp -> act_x == xr) {
		if (sp -> act_y == tile -> br || sp -> act_y == tile -> tr)
		    c[++i] = sp; /* tile corner spider */
	    }
	    else {
		spiderEdge_t *e1, *e2;
		e1 = sp -> edge;
		e2 = e1 -> nb;
		if (!e2 -> nb) {
		    e1 -> oh -> oh = e2 -> oh;
		    e2 -> oh -> oh = e1 -> oh;
		    /* Note: e1 and e2 stay in the strip and are later disposed.
		       The sp is set to zero to flag its status for drawing. */
		    e1 -> sp = 0;
		    e2 -> sp = 0;
#ifdef DEBUG
		    fprintf (stderr, "spiderTile: removing spider %p (%8g,%8g)\n",
			sp, D(sp -> act_x), D(sp -> act_y));
#endif
		    stripFreeSpider (sp);
		}
	    }
	}

	ASSERT (i == 2 || i == 3); /* triangle or quadrilateral */
	if (i == 2) c[3] = NULL;
	meshSetCorners (face, c[0], c[1], c[2], c[3]);
	face -> sc_subn = tile -> subcont;
    }
}
