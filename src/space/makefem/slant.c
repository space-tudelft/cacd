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

void testIntersection (edge_t *e1, edge_t *e2)
{
    coor_t d, xi;
    double s1 = (double) e1 -> dy / (double) e1 -> dx;
    double s2 = (double) e2 -> dy / (double) e2 -> dx;

    /* For efficiency reasons, this function assumes that
     * the test for unequal slopes has been done before calling.
     */
 // ASSERT (compareSlope (e1, !=, e2));

    if (e1 -> xl == -INF || e2 -> xl == -INF) return;

    d = (int) Round (((e2 -> yl - e1 -> yl) + s2 * (e1 -> xl - e2 -> xl)) / (s1 - s2));

    xi = e1 -> xl + d;

    if (xi <= e1 -> xl || xi >= e1 -> xr) return;
    if (xi <= e2 -> xl || xi >= e2 -> xr) return;

    e1 -> xi = Min (xi, e1 -> xi);
    e2 -> xi = Min (xi, e2 -> xi);
}

edge_t * split (edge_t *e1) /* split sublist of intersecting edges */
{
    edge_t * e4 = e1;
    coor_t xi   = e1 -> xi;
    coor_t yi   = Y (e1, xi);
    edge_t * e2 = e1 -> fwd;
    edge_t * e;

    if (e2 -> xr == xi) e2 = e2 -> fwd;  /* special case (AvG) */

    ASSERT (e2 -> xi == xi);

    /* let e2 point to other end of sublist */
    while (e2 -> fwd -> xi == xi && compareSlope (e2 -> fwd, <, e2))
	e2 = e2 -> fwd;

    /* e2 is other end of sublist */

    for (;;) {
	/* ASSERT (Y (e1, xi) == yi); this is not true when rounding !!! */
	if (e1 -> xr > xi) {
	    NEW_EDGE (e, xi, yi, e1 -> xr, e1 -> yr, e1 -> color);
	    e -> xc = e1 -> xc;
	    e -> bundle = e1 -> bundle;
	    e1 -> bundle = (edge_t *) NULL;

	    /* insert above e2 */
	    e -> fwd = e2 -> fwd;
	    e -> bwd = e2;
	    e2 -> fwd -> bwd = e;
	    e2 -> fwd = e;

	    /* chop old edge */
	    e1 -> xr = xi;
	    e1 -> yr = yi;
	    e1 -> xi = INF;
	}
	if (e1 == e2) break;
	e1 = e1 -> fwd;
    }
    return (e4);
}
