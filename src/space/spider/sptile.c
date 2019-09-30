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
#include <math.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"

#include "src/space/spider/define.h"
#include "src/space/spider/recog.h"
#include "src/space/spider/extern.h"

extern bool_t prePass1;

/* Tile has become ready.
 */
void spiderTile (tile_t *tile)
{
    int i, level;
    face_t *face;
    meshCoor_t xl, xr;
    spider_t *tr, *sp, *sp2, *c[4];

    Debug (fprintf (stderr, "tile: %d %d %d %d\n",
	 (int) tile -> xl, (int) tile -> xr, (int) tile -> bl, (int) tile -> br));

    xl = tile -> xl;
    xr = tile -> xr;
    for (level = 0; level < nrOfSpiderLevels; level++) {

	if ((face = tile -> mesh -> faces[level])) {
	    tr = tile -> mesh -> spider[0][level];

	    /* Of all spiders around the current face,
	     * select those which are at a corner of the tile.
	     */
	    c[i = 0] = tr;
	    stripAddSpider (tr);
	    for (sp = ccwa (tr, face); sp != tr; sp = sp2) {
		/*
		 * TRUE if sp is at one of the corners of the tile.
		 */
		sp2 = ccwa (sp, face);
		if ((sp -> nom_x == xl && (sp -> nom_y == tile -> bl
					|| sp -> nom_y == tile -> tl)) ||
		    (sp -> nom_x == xr && (sp -> nom_y == tile -> br
					|| sp -> nom_y == tile -> tr))) {
		    c[++i] = sp;
		    stripAddSpider (sp);
		}
		else if (prePass1 == 2) {
		    spiderEdge_t *e1, *e2;
		    e1 = sp -> edge;
		    e2 = e1 -> nb;
		    if (e2 -> nb || sp -> nom_x == xl || sp -> nom_x == xr)
			stripAddSpider (sp);
		    else {
			e1 -> oh -> oh = e2 -> oh;
			e2 -> oh -> oh = e1 -> oh;
			/* Note: e1 and e2 stay in the strip and are later disposed.
			   The sp is set to zero to flag its status for drawing. (SdeG) */
			e1 -> sp = 0;
			e2 -> sp = 0;
#ifdef DEBUG
fprintf (stderr, "spiderTile: disposeSpider %p (%g,%g,%g)\n", sp, sp->nom_x, sp->nom_y, sp->nom_z);
#endif
			disposeSpider (sp);
		    }
		}
		else stripAddSpider (sp);
	    }

	    /* It is triangle or a quadrilateral, respectively. */
	    ASSERT (i == 2 || i == 3);

	    /* If it is a triangle, make the fourth spider NULL. */
	    if (i == 2) c[3] = NULL;

	    /* Now, c[] contains the corner spiders.
	     * Put these into face.
	     */
	    meshSetCorners (face, c[0], c[1], c[2], c[3]);
	}
    }
}
