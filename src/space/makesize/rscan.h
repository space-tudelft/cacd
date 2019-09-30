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

#include <sys/types.h>

typedef struct Edge {
    coor_t xl, yl, xr, yr;
    coor_t dy;
    mask_t color;
    struct Tile * tile;
    struct Edge * fwd, * bwd;
    struct Edge * bundle;
    coor_t xi, xc;
} edge_t;

#define Y(e,x) (e->dy == 0 ? e->yl : (e->yl + (x - e->xl) * e->dy))

#define compareSlope(e1, op, e2)   (e1->dy op e2->dy)

#define NEW_EDGE(edge,xl,yl,xr,yr,co) edge = rcreateEdge(xl,yl,xr,yr), edge->color = co
#define NEW_TILE(tile,xl,bl,xr,br,co) tile = createTile(xl,bl,xr,br), tile->color = co

