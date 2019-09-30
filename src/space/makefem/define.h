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

#define SCALE           4       /* Resolution of geo-coordinates, should
				 * be equal to SCALE in makegln. */
#define MAX_ELEM_NAME  32	/* Max length of element name */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>		/* for INT_MAX and friends in portable.h */
#include "src/libddm/dmincl.h"
#include "src/space/makefem/auxil.h"

typedef int32 coor_t;
#define INF MAX32

#define Y(e,x) (e -> dy == 0 ? e -> yl : (e -> yl + (x-e->xl) * ((e->dy) / (e->dx))))
#define Slope(e) (e->dy == 0 ? 0 : (e->dy)/(e->dx))
#define compareSlope(e1, op, e2)   (Slope (e1) op Slope (e2))

#define NEW_EDGE(edge,xl,yl,xr,yr,co) edge = createEdge(xl,yl,xr,yr), edge->color = co
#define NEW_TILE(tile,xl,bl,xr,br,co,cc) tile = createTile(xl,bl,xr,br,cc), tile->color = co

typedef struct Edge {
    coor_t xl, yl, xr, yr;
    coor_t dx, dy;
    mask_t color;
    struct Tile * tile;
    struct Edge * fwd, * bwd;
    struct Edge * bundle;
    coor_t xi, xc;
    int cc;  /* causing conductor for substrate */
} edge_t;

typedef struct Tile {
    coor_t xl, xr, bl, br, tl, tr;
    mask_t color;
    int32  cc;
    struct subcRef *subcont;
    struct Tile *next_t; // freelist
} tile_t;

typedef struct substrate {
    char name[MAX_ELEM_NAME + 1];
    double conduc;
    double top;
} substrate_t;

typedef struct scCoor {
    coor_t x, y;
    struct scCoor *next;
} scCoor_t;

typedef struct scBound {
    struct scCoor *pb, *pt;
    struct scBound *next;
} scBound_t;

typedef struct subcont {
    coor_t xl, yb;
    int group;
    struct scBound *boundaries;
    struct subcRef *subcontRefs;
    struct subcont *next, *prev;
    struct subcont *nextGroup, *prevGroup;
} subcont_t;

typedef struct subcRef {
    int causing_con, distributed, nr;
    struct subcont *subcontInfo;
    struct subcRef *nextRef;
} subcontRef_t;
