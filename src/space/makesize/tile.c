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
#include "src/space/makesize/rscan.h"

static int allocs = 0,
	   frees  = 0,
	   max    = 0;

static tile_t * list = NULL;

/*  Create a fresh tile
*/
tile_t * createTile (coor_t xl, coor_t bl, coor_t xr, coor_t br)
{
    tile_t * t;

#ifdef DEBUG
    /* the xr-coordinate is the right bbox coordinate of the cell.
     * An assertion xr>xl fails if xl==right bounding box,
     * but it should not fail otherwise.
     */
    static coor_t right;
    ASSERT (xr > xl || xl == right);
    right = xr;
#endif

    if (list) {
	t = list;
	list = list -> next_tor;
    }
    else {
	t = NEW (tile_t, 1);
    }

    t -> xl = xl; t -> bl = bl;
    t -> xr = xr; t -> br = br;
    t -> known = 0;
    t -> tl = t -> tr = INF;
    t -> next_tor = NULL;

    Debug (printTile ("new tile", t));

    if (++allocs - frees > max) max = allocs - frees;
    return (t);
}

/*  Dispose a tile
*/
void disposeTile (tile_t *t)
{
    frees++;
    t -> next_tor = list;
    list = t;
}

/*  Print a tile
*/
void printTile (char *s, tile_t *t)
{
    fprintf (stderr, "%s: xl=%g bl=%g tl=%g xr=%g br=%g tr=%g\n",
	s, (double) t -> xl, (double) t -> bl,
 	   (double) t -> tl, (double) t -> xr,
	   (double) t -> br, (double) t -> tr);
}

void tileStatistics (FILE *fp)
{
    fprintf (fp, "\ttiles allocated    : %d\n", allocs);
    fprintf (fp, "\ttiles freed        : %d\n", frees);
    fprintf (fp, "\tmax tiles in core  : %d\n", max);
}
