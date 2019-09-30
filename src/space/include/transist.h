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

typedef struct transistor {
    int tor_nr;
    double totPerimeter; /* l */
    double dsPerimeter;  /* w */
    double surface;
    coor_t xl, yb;
    struct elemDef * type;
#ifndef CONFIG_SPACE2
    char *instName;
#endif
    struct subnode * gate;
    struct subnode * bulk;
    struct Tile * subs;
    struct dsBoundary * boundaries;
#ifndef CONFIG_SPACE2
    struct transTileInfo * tileInfo;
#endif
} transistor_t;

typedef struct dsBoundary {
    struct dsCoor * points;
    struct subnode * dsCond;
    struct polnode * pn;
    int type; /* 'd', 's' or 'x' */
    struct dsBoundary * next;
} dsBoundary_t;

typedef struct dsCoor {
    coor_t x, y;
    struct dsCoor * next;
} dsCoor_t;

typedef struct transTileInfo {
    struct tileRef * tiles;
} transTileInfo_t;

