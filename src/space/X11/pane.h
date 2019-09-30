/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
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

#define FromBorder 5		/* number of Pixels free around picture */

#define ROUND_OFF  1.0e-20
#define VERY_LARGE 1.0e20

typedef double wcoor;			/* type of world coordinates */

typedef struct canvas {
    Dimension Cw, Ch;			/* size of canvas */
    wcoor     Ww, Wh;			/* size of world */
    wcoor     Wx, Wy;			/* origin of world  */
    int    deltaX, deltaY;
    double scale;
    double matrix3[3][3];
    wcoor view_xr, view_yr, view_zr;	/* camera view reference point */
    wcoor view_distance;		/* distance between view ref point
					    and view-plane */
} canvas_t;

struct draw_object {
    wcoor xl, yl, xr, yr;
    struct draw_object *next;
};
