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
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include <math.h>
#include "src/space/makesize/rscan.h"
#include "src/space/makesize/extern.h"

static FILE * fpExt = NULL;
static double inScale = 1;
tile_t * neggrow_list = 0;
static tile_t * neggrow_last = 0;

#define S(x) (x)/inScale

void enumPair (tile_t *tile, tile_t *newerTile, int orientation)
{
    static int tileNumber = 1;
    coor_t x1, y1, x2, y2;

    if (!tile -> known)           tile -> known = tileNumber++;
    if (!newerTile -> known) newerTile -> known = tileNumber++;

    if (orientation == 'v') {
	x1 = x2 = tile -> xr;
	y2 = Min (tile -> tr, newerTile -> tl);
	y1 = Max (tile -> br, newerTile -> bl);
    }
    else {
        ASSERT (orientation == 'h');
	if (tile -> xr < newerTile -> xr) {
	    x2 = tile -> xr;
	    y2 = tile -> tr;
	}
	else {
	    x2 = newerTile -> xr;
	    y2 = newerTile -> br;
	}
	if (tile -> xl > newerTile -> xl) {
	    x1 = tile -> xl;
	    y1 = tile -> tl;
	}
	else {
	    x1 = newerTile -> xl;
	    y1 = newerTile -> bl;
	}
    }

    if (fpExt)
      fprintf (fpExt, "boundary %g %g %g %g %d %d\n",
	S(x1), S(y1), S(x2), S(y2), tile -> known, newerTile -> known);
}

void enumTile (tile_t *tile)
{
    if (tile -> bl == -INF) goto ret;

    if (fpExt) {
	ASSERT (tile -> xr  >  tile -> xl);
	ASSERT (tile -> tl  >= tile -> bl);
	ASSERT (tile -> tr  >= tile -> br);
	ASSERT (tile -> tl > tile -> bl || tile -> tr > tile -> br);

	fprintf (fpExt, "tile %s %g %g %g %g %g %g %d\n",
	    colorOctStr (&tile -> color),
	    S(tile -> xl), S(tile -> bl), S(tile -> tl),
	    S(tile -> xr), S(tile -> br), S(tile -> tr), tile -> known);
    }

    /* EM: if a layout has te be grown, only grow the solid tiles
     *     if a layout has to be shrinked, only grow the space tile
     */
    if (secondTime) {
	if (IS_COLOR (&tile->color)) growLayout (tile, (coor_t)0);
    }
    else {
	resizeCond_t * resizeCond = resizes[resizeIndex].cond;
	while (resizeCond) {
	    if (COLOR_PRESENT (&tile->color, &resizeCond->present) &&
		COLOR_ABSENT  (&tile->color, &resizeCond->absent)) break;
	    resizeCond = resizeCond -> next;
	}
	if (!growPos) {
	    if (!newMask && COLOR_ABSENT (&tile->color, &idcolor)) {
		growLayout (tile, growSize);
	    }
	    else if (!resizeCond) {
		if (newMask) {
		    growLayout (tile, growSize);
		    goto ret;
		}
		if (!neggrow_list) neggrow_list = tile;
		else   neggrow_last -> next_tor = tile;
		neggrow_last = tile;
		return;
	    }
	}
	else if (resizeCond) growLayout (tile, growSize);
	else if (COLOR_PRESENT (&tile->color, &idcolor)) growLayout (tile, (coor_t)0);
    }
ret:
    disposeTile (tile);
}

void initExtract (char *cell, int scale)
{
    inScale = optScale ? scale : 1;

    fpExt = cfopen (mprintf ("%s_%s.ext", cell, resizes[resizeIndex].newmaskname), "w");
    fprintf (fpExt, "# cell %s, scale %g\n", cell, inScale);
    fprintf (fpExt, "# tile: octal_color xl bl tl xr br tr num\n");
    fprintf (fpExt, "# boundary: x1 y1 x2 y2 num1 num2\n");
    openDebugEps (cell, resizes[resizeIndex].newmaskname);
}

void endExtract ()
{
    if (fpExt) {
	fclose (fpExt);
	fpExt = NULL;
	closeDebugEps ();
    }
}
