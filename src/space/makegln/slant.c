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

void testIntersect (coor_t x, edge_t *e1, edge_t *e2)
{
    coor_t d, xi;

    if (e1 -> xl == -INF || e2 -> xl == -INF) return; /* head || tail */

    d = ((e2 -> yl - e1 -> yl) + e2 -> slope * (e1 -> xl - e2 -> xl)) / (e1 -> slope - e2 -> slope);
    xi = e1 -> xl + d;
    if (xi <= x) return;

    if (xi <= e1 -> xl || xi >= e1 -> xr) return;
    if (xi <= e2 -> xl || xi >= e2 -> xr) return;

    e1 -> xi = Min (xi, e1 -> xi);
    e2 -> xi = Min (xi, e2 -> xi);
}

edge_t *split (edge_t *e, coor_t xi, coor_t yi) /* split sublist of intersecting edges */
{
    edge_t *enew;
    edge_t *efwd = e -> fwd;
    edge_t *ebwd = e -> bwd;

    if (efwd -> xi != xi) {
	printEdge ("split", efwd);
	ASSERT (efwd -> xi == xi);
    }
    while (efwd -> fwd -> xi == xi && efwd -> fwd -> slope < efwd -> slope) efwd = efwd -> fwd;
    /* efwd is end of sublist */

    for (;;) {
	ASSERT (Y (e, xi) == yi);
	if (e -> xr > xi) {
	    enew = createEdge (xi, yi, e -> xr, e -> yr, e -> slope, e -> sign);
	    enew -> xi = INF;
	    enew -> xc = e -> xc;
	    enew -> bundle = e -> bundle; e -> bundle = NULL;

	    /* insert above ebwd */
	    enew -> fwd = ebwd -> fwd; enew -> bwd = ebwd;
	    ebwd -> fwd -> bwd = enew;
	    ebwd -> fwd = enew;

	    e -> xr = xi;
	    e -> yr = yi;
	    e -> xi = INF;
	}
	if (e == efwd) break; /* sublist done */
	e = e -> fwd;
    }
    return (ebwd -> fwd);
}
