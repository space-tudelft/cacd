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

static int allocs, allocs2, frees, max;
static edge_t * list = NULL;

edge_t *createEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
    edge_t *edge;

    ASSERT (xl < xr || (xl == INF && xr == INF));

    if (!(edge = list)) {
#define FITSIZE 100
	list = NEW (edge_t, FITSIZE);
	allocs2 += FITSIZE;
	edge = list + FITSIZE - 1;
	edge -> fwd = NULL;
	while (edge != list) {
	    --edge;
	    edge -> fwd = edge + 1;
	}
    }
    list = list -> fwd;

    edge -> xl = xl; edge -> xr = xr; edge -> dx = xr - xl;
    edge -> yl = yl; edge -> yr = yr; edge -> dy = yr - yl;
    edge -> cc = 0;
    edge -> xc = xr;
    edge -> xi = INF;

    edge -> bundle = NULL;
    edge -> fwd = edge -> bwd = NULL;
    edge -> tile = NULL;

    Debug (printEdge ("newedge", edge));

    if (++allocs - frees > max) max = allocs - frees;
    return (edge);
}

void disposeEdge (edge_t *e)  /* dispose an edge */
{
    ++frees;
    e -> fwd = list; list = e;
}

void initEdgeStatistics ()
{
    allocs = frees = max = 0;
}

void edgeStatistics (FILE *fp)
{
    fprintf (fp, "overall edge statistics:\n");
    fprintf (fp, "\tedges allocated     : %d (%d)\n", allocs, allocs2);
    fprintf (fp, "\tedges freed         : %d\n", frees);
    fprintf (fp, "\tmax edges in core   : %d\n", max);
}

char *DX (coor_t x)
{
    static char buf[20];
    if (x == -INF) sprintf (buf, "-INF");
    else if (x == INF) sprintf (buf, "+INF");
    else sprintf (buf, "%g", D(x));
    return buf;
}

void printEdge (char *s, edge_t *e)
{
    fprintf (stderr, "%s:", s);
    fprintf (stderr, " xl=%8s", DX(e -> xl));
    fprintf (stderr, " xr=%8s", DX(e -> xr));
    fprintf (stderr, " yl=%8s", DX(e -> yl));
    fprintf (stderr, " yr=%8s", DX(e -> yr));
    fprintf (stderr, " xi=%8s", DX(e -> xi));
    fprintf (stderr, " xc=%8s", DX(e -> xc));
    if (e -> dy == 0) fprintf (stderr, " dy/dx= 0");
    else fprintf (stderr, " dy/dx=%2s", DX(e -> dy / e -> dx));
    fprintf (stderr, " color=%s\n", colorOctStr (&e -> color));
}
