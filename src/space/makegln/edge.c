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

#include "src/space/makegln/config.h"
#include <stddef.h>
#include <stdio.h>
#include "src/space/auxil/auxil.h"
#include "src/space/makegln/makegln.h"
#include "src/space/makegln/proto.h"

static edge_t * freeList = NULL;

edge_t * createEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr, slope_t slope, sign_t sign)
{
    edge_t * e;

    if (!(e = freeList)) {
#define CHAIN_SIZE 1000
	freeList = NEW (edge_t, CHAIN_SIZE);
	e = freeList + CHAIN_SIZE - 1;
	e -> fwd = NULL;
	while (e != freeList) {
	    --e;
	    e -> fwd = e + 1;
	}
    }
    freeList = freeList -> fwd;

    e -> fwd = e -> bwd = e -> bundle = e -> next = (edge_t *) NULL;
    e -> xl = xl;
    e -> xr = xr;
    e -> yl = yl;
    e -> yr = yr;
    e -> slope = slope;
    e -> sign = sign;
    e -> signLeft = 0;
    e -> xc = e -> xr;
    e -> xi = INF;
    return (e);
}

void disposeEdge (edge_t *edge)
{
    edge -> fwd = freeList;
    freeList = edge;
}

void printEdge (char *s, edge_t *edge)
{
    fprintf (stderr, "%s: xl: %g yl: %g xr: %g yr: %g xc: %g xi: %g slope: %d sign: %d signLeft: %d\n", s,
	(double) edge -> xl / scale, (double) edge -> yl / scale,
	(double) edge -> xr / scale, (double) edge -> yr / scale,
	(double) edge -> xc / scale, (double) edge -> xi / scale,
	(int) edge -> slope, (int) edge -> sign, (int) edge -> signLeft);

    while (edge -> bundle) {
	edge = edge -> bundle;
	fprintf (stderr, ">  %s: xl: %g yl: %g xr: %g yr: %g xc: %g xi: %g slope: %d sign: %d\n", s,
	    (double) edge -> xl / scale, (double) edge -> yl / scale,
	    (double) edge -> xr / scale, (double) edge -> yr / scale,
	    (double) edge -> xc / scale, (double) edge -> xi / scale,
	    (int) edge -> slope, (int) edge -> sign);
    }
}
