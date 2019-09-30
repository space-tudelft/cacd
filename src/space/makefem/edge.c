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

#include "src/space/makefem/define.h"
#include "src/space/makefem/extern.h"

static int allocs = 0, frees = 0, max = 0;
static edge_t * list = NULL;

edge_t * createEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
    edge_t *edge;

    ASSERT (xl < xr || (xl == INF && xr == INF));

    if (!(edge = list)) {
#define FITSIZE 100
	list = NEW (edge_t, FITSIZE);
	edge = list + FITSIZE - 1;
	edge -> fwd = NULL;
	while (edge != list) {
	    --edge;
	    edge -> fwd = edge + 1;
	}
    }
    list = list -> fwd;

    edge -> xl =  xl; edge -> xr =  xr;
    edge -> yl =  yl; edge -> yr =  yr;
    edge -> cc = 0;

    edge -> dy = yr - yl;
    if (xr == INF && xl == INF)
        /* To please Slope calculations for sentinal edges. */
        edge -> dx = 1;
    else
        edge -> dx = xr - xl;

    edge -> xc = edge -> xr;
    edge -> xi = INF;

    edge -> bundle = NULL;
    edge -> fwd = edge -> bwd = NULL;
    edge -> tile = NULL;

    if (++allocs - frees > max) max = allocs - frees;

    return (edge);
}

void disposeEdge (edge_t *e)  /* dispose an edge */
{
    frees++;
    e -> fwd = list;
    list = e;
}

/*  printEdge - print edge, prefixed by s
*/
void printEdge (char *s, edge_t *e)
{
    fprintf (stderr, "%s: xl=%g yl=%g xr=%g yr=%g xi=%g xc=%g dy/dx=%g color=%s\n",
	s, (double) e -> xl, (double) e -> yl,
	(double) e -> xr, (double) e -> yr,
	(double) e -> xi, (double) e -> xc,
	(double) e -> dy / e -> dx, colorOctStr (&e -> color));
}

void edgeStatistics (FILE *fp)
{
    fprintf (fp, "overall edge statistics:\n");
    fprintf (fp, "\tedges allocated    : %d\n", allocs);
    fprintf (fp, "\tedges freed        : %d\n", frees);
    fprintf (fp, "\tmax edges in core  : %d\n", max);
}
