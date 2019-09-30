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

#ifndef tile_h
#define tile_h

typedef int32 	coor_t;
#define INF	MAX32

typedef struct Tile {
    coor_t xl, xr;
    coor_t bl, br;
    coor_t tl, tr;
    mask_t color;
    int   cnt;		/* tile counter */
    int   known;	/* 1 = resistive layer connected left or right (conLR) */
			/* 2 = resistive layer connected at top/bottom (conTB) */
			/* 4 = VBJT element */
			/*32 = high   res present */
    struct Terminal   *terms;
    struct nodePoint  *tlPoints, *rbPoints;
    struct transistor *tor;
    struct Tile *next_tor;
    struct subcontRef *subcont; /* pointer to substrate contact reference */
    struct Tile *next;	/* queue */
    struct Tile *stl, *str, *stb, *stt; /* corner stitches */
#ifdef CAP3D
    struct meshInfo *mesh;
#endif
    struct subnode **cons; /* list of subnode pointers */
} tile_t;

/* The next macro accesses the subnodes of a tile
 * when resistance extraction is off.
 */
#define CONS(tile,i) (((subnode_t *) (tile -> cons + nrOfCondStd)) + i)

#endif /* tile_h */
