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

/* need # include "auxil/auxil.h" */

typedef int  slope_t;
typedef int  sign_t;
typedef int  coor_t;
typedef int  ecnt_t; /* to count edges */

#define MAXCOOR		MAX32

/* The top part of this structure MUST match
 * that of edge_part_t in output.c
 */
typedef struct Edge {
    coor_t xl, yl, xr, yr;
    slope_t slope;
    coor_t xi, xc;
    sign_t sign;
    bool_t ready;
    bool_t remain;
    struct Edge * fwd, * bwd;
    struct Edge * bundle;
    struct Edge * next;
    sign_t signLeft;
} edge_t;

#define INF		MAXCOOR

#define Y(e,x) (e->slope==(slope_t)0?e->yl:(e->yl+(coor_t)(e->slope*(x-e->xl))))

typedef struct _edge {
    coor_t xl, yl, xr, yr;
    slope_t slope;
    sign_t sign;
} _edge_t;

/* Compare two edges lexicographically
 *
 * larger (e1, e2) returns true if e1 > e2
 * smaller(e1, e2) returns true if e1 < e2
 */

#define larger(e1, e2) ( \
    ((e1) -> xl > (e2) -> xl) ? ( 1) : ( \
    ((e1) -> xl < (e2) -> xl) ? ( 0) : ( \
    ((e1) -> yl > (e2) -> yl) ? ( 1) : ( \
    ((e1) -> yl < (e2) -> yl) ? ( 0) : ( \
    ((e1) -> slope > (e2) -> slope))))))

#define smaller(e1, e2) larger(e2, e1)

/* signs of input edges */
#define STOP  -1
#define START  1

#define unpack4D(fp, d0, d1, d2, d3)  _dmUnpack (fp, "DDDD", d0, d1, d2, d3)
#define pack4D(fp, d0, d1, d2, d3) _dmPack (fp, "D D D D\n", d0, d1, d2, d3)
