/*
 * ISC License
 *
 * Copyright (C) 1995-2018 by
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

#define SLOPE(vA,vB) (vA -> x == vB -> x ? INF : ((vB -> y - vA -> y) / (double)(vB -> x - vA -> x)))

#define ESLOPE(e)    SLOPE (e -> vA, e -> vB)

#define NEXT_E(e,v)  (e -> vA == v ? e -> nextA : e -> nextB)
#define PREV_E(e,v)  (e -> vA == v ? e -> prevA : e -> prevB)

#define YM(e,xc)     (e -> vA -> y + ESLOPE (e) * (xc - e -> vA -> x))

#define DIST2(v1,v2) (Dsqr((v2)->x - (v1)->x) + Dsqr((v2)->y - (v1)->y))

typedef struct Vertex {
    coor_t x;
    coor_t y;
    struct Meshedge *edges;
    struct Vertex *next;
    struct ContactBoundary *contbound;
    struct Vertex *contbound_next;
} vertex_t;

typedef struct Meshedge {
    struct Vertex *vA;
    struct Vertex *vB;
    short fixed;
    short flag;
    struct Meshedge *nextA;
    struct Meshedge *prevA;
    struct Meshedge *nextB;
    struct Meshedge *prevB;
} meshedge_t;

typedef struct Hullmem {
    struct Vertex *v;
    struct Meshedge *forward;
    struct Hullmem *prevHull;
    struct Hullmem *nextHull;
} hullmem_t;

typedef struct in_line {
    long xl;
    long yl;
    long xr;
    long yr;
    int mask;
    double val;    /* for transferring the area */
} in_line_t;

typedef struct ContactBoundary {
    struct Vertex *vertices;
    struct Contact *cont;
    struct ContactBoundary *bound_next;
} contactBoundary_t;

typedef struct Contact {
    int nr;
    /* We can have different boundaries for a contact
       because there may be holes in the contact. */
    struct ContactBoundary *boundaries;
 // struct ContactRef *neighbors;
    struct Contact    *neighbors;
} contact_t;

typedef struct ContactRef {
    struct Contact *cont;
    struct ContactRef *next;
} contactRef_t;

