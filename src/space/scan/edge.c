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
#include "src/space/scan/scan.h"
#include "src/space/scan/extern.h"

static unsigned int allocs = 0;
static unsigned int frees  = 0;
static unsigned int slant  = 0;
static unsigned int max    = 0;

static edge_t * list = NULL;

edge_t * createEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
    register edge_t *edge;

    if (!(edge = list)) {
#define FITSIZE 100
	list = NEW (edge_t, FITSIZE);
	edge = list + FITSIZE - 1;
	edge -> fwd = NULL;
	while (edge != list) { --edge; edge -> fwd = edge + 1; }
	max += FITSIZE;
    }
    list = list -> fwd;

    edge -> xl = xl; edge -> xr = xr; edge -> xc = xr;
    edge -> yl = yl; edge -> yr = yr; edge -> xi = INF;
    edge -> cc = 0;

    if (yr != yl) {
	++slant;
	ASSERT (xr > xl);
	/* 45 degree layout */
	if (yr > yl) {
	    ASSERT ((double)yr - yl == (double)xr - xl);
	    edge -> dy = 1;
	}
	else {
	    ASSERT ((double)yl - yr == (double)xr - xl);
	    edge -> dy = -1;
	}
    }
    else {
	ASSERT (xr > xl || (xl == INF && xr == INF));
	edge -> dy = 0;
    }

    edge -> bundle = NULL;
 // edge -> fwd = edge -> bwd = NULL;
    edge -> tile = NULL;

    ++allocs;

    return (edge);
}

void disposeEdge (edge_t *e) /* dispose an edge */
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
	(double) e -> dy, colorOctStr (&e -> color));
}

void edgeStatistics (FILE *fp)
{
    fprintf (fp, "overall edge statistics:\n");
    fprintf (fp, "\tedges created      : %u\n", allocs);
    fprintf (fp, "\tedges disposed     : %u\n", frees);
    fprintf (fp, "\tedges allocated    : %u of %d byte\n", max, (int)sizeof(edge_t));
 // fprintf (fp, "\tmax edges in core  : %u\n", max);
    fprintf (fp, "\tslanting edges     : %u\n", slant);
}
