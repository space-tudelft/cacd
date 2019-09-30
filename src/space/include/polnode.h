/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Nick van der Meijs
 *	Frederik Beeftink
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

typedef struct pnTorLink {
    int type;
    struct BJT * tor;
    struct pnTorLink * next;
} pnTorLink_t;

typedef struct nodeLink {
    struct Node * n;
    struct polnode * pn;
    struct nodeLink * next_n;
    struct nodeLink * next_pn;
} nodeLink_t;

typedef struct polnode {
    char type; /* 'n' (electrons), 'p' (holes) or 'a' (metal) */
    int  conNr; /* conductor-number */
    coor_t xl, yb;
    double area;
    double length;
    int    ds_tcnt;
    double ds_wtot;
    double ds_area;
    double ds_peri;
    struct pnEdge * edges;
    struct subnode * subs;
    struct junction * juncs;
    struct pnTorLink * tors;
    struct nodeLink * nodes;
#ifndef CONFIG_SPACE2
    struct tileRef * tiles; /* backannotation */
#endif
} polnode_t;
