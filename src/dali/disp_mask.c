/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	P. van der Wolf
 *	H.T. Fassotte
 *	S. de Graaf
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

#include "src/dali/header.h"

extern Trans *Mtx; /* transformation matrix */
extern qtree_t **quad_root;
extern Coor Q_xl, Q_xr, Q_yb, Q_yt; /* quad search window */
extern int  CLIP_FLAG;
extern int  CHECK_FLAG;
extern int *vis_arr;
extern int *fillst;
extern int  act_mask_lay;

/*
** Paint a trapezoid on the screen.
*/
void paint_trap (struct obj_node *p)
{
    Coor dy, line_x[4], line_y[4];

    if (!p -> sides) {
	paint_box ((float) p -> ll_x1, (float) p -> ll_x2, (float) p -> ll_y1, (float) p -> ll_y2);
	return;
    }

    line_y[3] = line_y[0] = p -> ll_y1;
    line_y[2] = line_y[1] = p -> ll_y2;
    dy = p -> ll_y2 - p -> ll_y1;

    line_x[1] = line_x[0] = p -> ll_x1;
    switch (p -> sides >> 2) { /* leftside */
    case 0: break;
    case 1: line_x[1] += dy; break;
    case 2: line_x[0] += dy; break;
    default:
	btext ("Illegal trapezoid!");
	return;
    }

    line_x[3] = line_x[2] = p -> ll_x2;
    switch (p -> sides & 3) { /* rightside */
    case 0: break;
    case 1: line_x[3] -= dy; break;
    case 2: line_x[2] -= dy; break;
    default:
	btext ("Illegal trapezoid!");
	return;
    }

    draw_trap (line_x, line_y);
}

/*
** Find those parts of p inside search window.
*/
void clip_paint (struct obj_node *p)
{
    Coor a[4], crossy, x1, x2, y1, y2, pl, pr;
    Coor xmin, xmax, oldxmin, oldxmax, yold;
    register int count, n;
    int ls, rs;

    x1 = p -> ll_x1;
    x2 = p -> ll_x2;
    y1 = p -> ll_y1;
    y2 = p -> ll_y2;

    if (!p -> sides) {
	if (x1 < Q_xl) x1 = Q_xl;
	if (x2 > Q_xr) x2 = Q_xr;
	if (y1 < Q_yb) y1 = Q_yb;
	if (y2 > Q_yt) y2 = Q_yt;
	mtx_draw_box (x1, x2, y1, y2);
	return;
    }

    ls = p -> sides;
    if ((rs = ls & 3) > 1) rs = -1;
    if ((ls >>= 2)    > 1) ls = -1;

    pl = (ls == 1) ? y1 : y2;
    pr = (rs == 1) ? y2 : y1;

    if (y1 < Q_yb) {
	if (ls ==  1) { x1 += Q_yb - y1; pl = Q_yb; }
	if (rs == -1) { x2 -= Q_yb - y1; pr = Q_yb; }
	y1 = Q_yb;
    }
    if (y2 > Q_yt) {
	if (ls == -1) { x1 += y2 - Q_yt; pl = Q_yt; }
	if (rs ==  1) { x2 -= y2 - Q_yt; pr = Q_yt; }
	y2 = Q_yt;
    }
    if (x1 >= Q_xr) {
	if (ls || x1 > Q_xr) return;
    }
    if (x2 <= Q_xl) {
	if (rs || x2 < Q_xl) return;
    }

    count = 0;

    if (ls) {
	/* find intersection between p->leftside and Q_xr */
	if (x2 > Q_xr) {
	    crossy = pl + ls * (Q_xr - x1);
	    if (crossy > y1 && crossy < y2) {
		if (ls > 0) y2 = crossy;
		else        y1 = crossy;
	    }
	}
	/* find intersection between p->leftside and Q_xl */
	if (x1 < Q_xl) {
	    crossy = pl + ls * (Q_xl - x1);
	    if (crossy > y1 && crossy < y2) a[++count] = crossy;
	}
    }

    if (rs) {
	/* find intersection between p->rightside and Q_xl */
	if (x1 < Q_xl) {
	    crossy = pr + rs * (Q_xl - x2);
	    if (crossy > y1 && crossy < y2) {
		if (rs > 0) y1 = crossy;
		else        y2 = crossy;
	    }
	}
	/* find intersection between p->rightside and Q_xr */
	if (x2 > Q_xr) {
	    crossy = pr + rs * (Q_xr - x2);
	    if (crossy > y1 && crossy < y2) {
		if (count && crossy <= a[1]) {
		    if (crossy < a[1]) { /* swap */
			a[++count] = a[1];
			a[1] = crossy;
		    }
		}
		else a[++count] = crossy;
	    }
	}
    }

    a[++count] = y2;

    /* intersection between current y1 and p->leftside */
    xmin = x1;
    if (ls) xmin += ls * (y1 - pl);
    if (xmin < Q_xl) xmin = Q_xl;
    /* intersection between current y1 and p->rightside */
    xmax = x2;
    if (rs) xmax += rs * (y1 - pr);
    if (xmax > Q_xr) xmax = Q_xr;

    for (n = 1; n <= count; ++n) {

	yold = y1;
	oldxmin = xmin;
	oldxmax = xmax;
	y1 = a[n];

	/* intersection between current y1 and p->leftside */
	xmin = x1;
	if (ls) xmin += ls * (y1 - pl);
	if (xmin < Q_xl) xmin = Q_xl;
	/* intersection between current y1 and p->rightside */
	xmax = x2;
	if (rs) xmax += rs * (y1 - pr);
	if (xmax > Q_xr) xmax = Q_xr;

	if (oldxmin == xmin && oldxmax == xmax)
	    mtx_draw_box (xmin, xmax, yold, y1);
	else
	    mtx_draw_trap (oldxmin, xmin, xmax, oldxmax, yold, y1);
    }
}

/*
** Display all trapezoids of parent cell that are
** within specified window and of specified mask lay.
*/
void disp_mask_quad (int lay, Coor xmin, Coor xmax, Coor ymin, Coor ymax)
{
    if (!vis_arr[lay]) return;

    ggSetColor (lay);
    d_fillst (fillst[lay]);
    act_mask_lay = lay;

    Q_xl = xmin; Q_xr = xmax;
    Q_yb = ymin; Q_yt = ymax;
    Mtx  = NULL;

    if (CHECK_FLAG)
	quad_search (quad_root[lay], trap_to_checker);
    else if (CLIP_FLAG)
	quad_search (quad_root[lay], clip_paint);
    else
	quad_search (quad_root[lay], paint_trap);
}

void mtx_draw_box (Coor x1, Coor x2, Coor y1, Coor y2)
{
    if (Mtx) {
	if (Mtx[0]) { /* R0 or R180 */
	    paint_box ( (float) (Mtx[0] * x1 + Mtx[2]),
			(float) (Mtx[0] * x2 + Mtx[2]),
			(float) (Mtx[4] * y1 + Mtx[5]),
			(float) (Mtx[4] * y2 + Mtx[5]));
	}
	else {
	    paint_box ( (float) (Mtx[1] * y1 + Mtx[2]),
			(float) (Mtx[1] * y2 + Mtx[2]),
			(float) (Mtx[3] * x1 + Mtx[5]),
			(float) (Mtx[3] * x2 + Mtx[5]));
	}
    }
    else {
	paint_box ((float) x1, (float) x2, (float) y1, (float) y2);
    }
}

void mtx_draw_trap (Coor x0, Coor x1, Coor x2, Coor x3, Coor y0, Coor y1)
{
    Coor line_x[4], line_y[4], z;

    if (Mtx) {
	if (Mtx[4] > 0 || Mtx[1] < 0) { /* R0 or R180+MX || R90 */
	    if (Mtx[0] < 0 || Mtx[3] < 0) { /* R180+MX || R90+MX */
		z = x0; x0 = x3; x3 = z;
		z = x1; x1 = x2; x2 = z;
	    }
	}
	else { /* R0+MX or R180 || R270 */
	    if (Mtx[0] > 0 || Mtx[3] > 0) { /* R0+MX || R270+MX */
		z = x0; x0 = x1; x1 = z;
		z = x2; x2 = x3; x3 = z;
	    }
	    else { /* R180 || R270 */
		z = x0; x0 = x2; x2 = z;
		z = x1; x1 = x3; x3 = z;
	    }
	    z = y0; y0 = y1; y1 = z;
	}
	if (Mtx[0]) {
	    line_x[0] = x0 * Mtx[0] + Mtx[2];
	    line_x[1] = x1 * Mtx[0] + Mtx[2];
	    line_x[2] = x2 * Mtx[0] + Mtx[2];
	    line_x[3] = x3 * Mtx[0] + Mtx[2];
	    line_y[3] = line_y[0] = y0 * Mtx[4] + Mtx[5];
	    line_y[2] = line_y[1] = y1 * Mtx[4] + Mtx[5];
	}
	else {
	    line_y[0] = x0 * Mtx[3] + Mtx[5];
	    line_y[1] = x1 * Mtx[3] + Mtx[5];
	    line_y[2] = x2 * Mtx[3] + Mtx[5];
	    line_y[3] = x3 * Mtx[3] + Mtx[5];
	    line_x[3] = line_x[0] = y0 * Mtx[1] + Mtx[2];
	    line_x[2] = line_x[1] = y1 * Mtx[1] + Mtx[2];
	}
    }
    else {
	line_x[0] = x0;
	line_x[1] = x1;
	line_x[2] = x2;
	line_x[3] = x3;
	line_y[3] = line_y[0] = y0;
	line_y[2] = line_y[1] = y1;
    }
    draw_trap (line_x, line_y);
}
