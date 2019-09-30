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

extern coor_t thisX, thisY;

coor_t calcY (edge_t *e, coor_t x)
{
    double y;

    if (x > e -> xl) {
	if (x == e -> xr) return e -> yr;
	y = e -> yl + Slope(e) * ((double)x - e -> xl);
	ASSERT (y > -INF && y < INF);
	return (coor_t)y;
    }
    ASSERT (x == e -> xl);
    return e -> yl;
}

void testIntersection (edge_t *e1, edge_t *e2)
{
    coor_t d, xi;
    double s1 = e1 -> dy;
    double s2 = e2 -> dy;

    /* For efficiency reasons, this function assumes that
     * the test for unequal slopes has been done before calling.
     *
     * ASSERT (compareSlope (e1, !=, e2));
     */

    if (e1 -> xl == -INF || e2 -> xl == -INF) return;

    d = (int) Round (((e2 -> yl - e1 -> yl) + s2 * (e1 -> xl - e2 -> xl)) / (s1-s2));

    xi = e1 -> xl + d;

    if (xi <= e1 -> xl || xi >= e1 -> xr) return;
    if (xi <= e2 -> xl || xi >= e2 -> xr) return;

    e1 -> xi = Min (xi, e1 -> xi);
    e2 -> xi = Min (xi, e2 -> xi);
}

#if 0
int smallerAtX (coor_t x, edge_t *e1, edge_t *e2) /* return 1 if e2 <x e1, 0 otherwise */
{
    ASSERT ((e1 -> xr > x) || (e1 -> bundle == NULL));
    if (e1 -> xl > x || e1 -> xr < x) return (0);
    if (e2 -> xl > x || e2 -> xr < x) return (0);
    /* e1 and e2 are comparable at x */
    if (Y (e2, x) < Y (e1, x))        return (1);
    if (Y (e2, x) > Y (e1, x))        return (0);
    if (e1 -> xr == x)                return (0); /* !!! */
    if (compareSlope (e2, <, e1))     return (1);
    return (0);
}
#endif

void split (edge_t *e1) /* split sublist of intersecting edges */
{
    edge_t *e, *e2;

    e2 = e1 -> fwd;
    if (e2 -> xr == thisX) e2 = e2 -> fwd;  /* special case (AvG) */

    ASSERT (e2 -> xi == thisX);

    /* let e2 point to other end of sublist */
    while (e2 -> fwd -> xi == thisX && compareSlope (e2 -> fwd, <, e2)) e2 = e2 -> fwd;

    /* e2 is other end of sublist */

    for (;;) {
	/* ASSERT (Y (e1, thisX) == thisY); this is not true when rounding? */
	if (e1 -> xr > thisX) {
	    NEW_EDGE (e, thisX, thisY, e1 -> xr, e1 -> yr, e1 -> color);
	    e -> xc = e1 -> xc;
	    e -> bundle = e1 -> bundle;
	    e1 -> bundle = NULL;

	    /* insert above e2 */
	    e -> fwd = e2 -> fwd;
	    e -> bwd = e2;
	    e2 -> fwd -> bwd = e;
	    e2 -> fwd = e;

	    /* chop old edge */
	    e1 -> xr = thisX;
	    e1 -> yr = thisY;
	    e1 -> xi = INF;
	}
	if (e1 == e2) break;
	e1 = e1 -> fwd;
    }
}
