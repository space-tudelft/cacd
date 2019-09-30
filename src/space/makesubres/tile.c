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

static int allocs, frees, max;
static tile_t * free_tile = NULL;

tile_t *createTile (coor_t xl, coor_t bl, coor_t xr, coor_t br, int cc)
/* cc -- for special prepass: init causing conductor */
{
    tile_t * t;

    if (free_tile) free_tile = (t = free_tile) -> t_next;
    else t = NEW (tile_t, 1);

    t -> xl = xl; t -> bl = bl; t -> tl = INF;
    t -> xr = xr; t -> br = br; t -> tr = INF;
    t -> cc = cc;
    t -> subcont = NULL;
    t -> t_face = NULL;
    t -> t_next = NULL;

    Debug (printTile ("newtile", t));

    if (++allocs - frees > max) max = allocs - frees;
    return (t);
}

void disposeTile (tile_t *t)
{
    ++frees;
    t -> t_next = free_tile; free_tile = t;
}

void initTileStatistics ()
{
    allocs = frees = max = 0;
}

void tileStatistics (FILE *fp)
{
    fprintf (fp, "overall tile statistics:\n");
    fprintf (fp, "\ttiles allocated     : %d\n", allocs);
    fprintf (fp, "\ttiles freed         : %d\n", frees);
    fprintf (fp, "\tmax tiles in core   : %d\n", max);
}

void printTile (char *s, tile_t *t)
{
    fprintf (stderr, "%s:", s);
    fprintf (stderr, " xl=%8s", DX(t -> xl));
    fprintf (stderr, " xr=%8s", DX(t -> xr));
    fprintf (stderr, " bl=%8s", DX(t -> bl));
    fprintf (stderr, " br=%8s", DX(t -> br));
    fprintf (stderr, " tl=%8s", DX(t -> tl));
    fprintf (stderr, " tr=%8s\n", DX(t -> tr));
}
