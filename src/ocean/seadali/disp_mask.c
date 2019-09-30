/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	Pieter van der Wolf
 *	Simon de Graaf
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

#include "src/ocean/seadali/header.h"

extern qtree_t **quad_root;
extern Coor piwl, piwr, piwb, piwt;
extern short CLIP_FLAG;
extern short CHECK_FLAG;
extern int *vis_arr;
extern int *fillst;
extern int  act_mask_lay;
extern int  interrupt_flag;

static struct obj_node *window;

/*
** Display all trapezoids of parent cell that are
** within specified window and of specified mask.
*/
void disp_mask_quad (int mask, Coor xmin, Coor xmax, Coor ymin, Coor ymax)
{
    struct obj_node window;

    if (!vis_arr[mask]) return;

    ggSetColor (mask);
    d_fillst (fillst[mask]);
    act_mask_lay = mask;

    window.ll_x1 = xmin;
    window.ll_y1 = ymin;
    window.ll_x2 = xmax;
    window.ll_y2 = ymax;

    if (CHECK_FLAG == FALSE)
	quad_search (quad_root[mask], &window, paint_trap);
    else
	quad_search (quad_root[mask], &window, trap_to_checker);
}

/*
** Paint a trapezoid on the screen.
** INPUT: a pointer to the trapezoid.
*/
void paint_trap (struct obj_node *p)
{
    Coor dy, line_x[4], line_y[4];

    ASSERT (p);

    if (stop_drawing () == TRUE) return;

    if (CLIP_FLAG && (p -> leftside || p -> rightside)) {
	clip_paint (p, piwl, piwr, piwb, piwt, NULL, ROOT_TRAP);
	return;
    }

    /* calculate the corner points of the trapezoid */
    if (CLIP_FLAG) {
	line_y[1] = line_y[0] = Max (p -> ll_y1, piwb);
	line_y[2] = line_y[3] = Min (p -> ll_y2, piwt);
    }
    else {
	line_y[1] = line_y[0] = p -> ll_y1;
	line_y[2] = line_y[3] = p -> ll_y2;
    }

    dy = p -> ll_y2 - p -> ll_y1;

    switch (p -> leftside) {
    case 0:
	line_x[0] = line_x[3] = Max (p -> ll_x1, piwl);
	break;
    case 1:
	line_x[0] = p -> ll_x1;
	line_x[3] = p -> ll_x1 + dy;
	break;
    case -1:
	line_x[0] = p -> ll_x1 + dy;
	line_x[3] = p -> ll_x1;
	break;
    default:
	ptext ("Illegal trapezoid!");
	return;
    }

    switch (p -> rightside) {
    case 0:
	line_x[1] = line_x[2] = Min (p -> ll_x2, piwr);
	if (p -> leftside == 0) {
	    paint_box ((float) line_x[0], (float) line_x[2], (float) line_y[0], (float) line_y[2]);
	    return;
	}
	break;
    case 1:
	line_x[1] = p -> ll_x2 - dy;
	line_x[2] = p -> ll_x2;
	break;
    case -1:
	line_x[1] = p -> ll_x2;
	line_x[2] = p -> ll_x2 - dy;
	break;
    default:
	ptext ("Illegal trapezoid!");
	return;
    }

    /* paint the trapezoid on the screen */
    draw_trap (line_x, line_y);
}

/*
** Find those parts of p inside window.
** INPUT: pointer to trapezoid p and window.
** For trapezoids of instances: its transformation matrix Mtx.
*/
void clip_paint (struct obj_node *p, Coor wxmin, Coor wxmax, Coor wymin, Coor wymax, Trans *Mtx, int mode)
{
    Coor line_x[4], line_y[4];
    Coor crossy, interx1, interx2, px, py;
    Coor xmin, xmax, oldxmin, oldxmax, yold;
    Coor a[8];
    register int count, teller;

    if (wymin >= p -> ll_y2) return;
    if (wymax <= p -> ll_y1) return;

    count = 0;
    a[count++] = Max (wymin, p -> ll_y1);
    a[count] = Min (wymax, p -> ll_y2);

    /* find intersection between p->leftside and wxmax */
    px = p -> ll_x1;
    py = (p -> leftside == 1)? p -> ll_y1 : p -> ll_y2;

    if (p -> leftside && p -> ll_x2 > wxmax) {
	crossy = py + (wxmax - px) * p -> leftside;
	if (crossy > Max (p -> ll_y1, wymin)
	 && crossy < Min (p -> ll_y2, wymax)) a[++count] = crossy;
    }

    /* find intersection between p->leftside and wxmin */
    if (p -> leftside && p -> ll_x1 < wxmin) {
	crossy = py + (wxmin - px) * p -> leftside;
	if (crossy > Max (p -> ll_y1, wymin)
	 && crossy < Min (p -> ll_y2, wymax)) a[++count] = crossy;
    }

    /* find intersection between p->rightside and wxmin */
    px = p -> ll_x2;
    py = (p -> rightside == 1)? p -> ll_y2 : p -> ll_y1;

    if (p -> rightside && p -> ll_x1 < wxmin) {
	crossy = py + (wxmin - px) * p -> rightside;
	if (crossy > Max (p -> ll_y1, wymin)
	 && crossy < Min (p -> ll_y2, wymax)) a[++count] = crossy;
    }

    /* find intersection between p->rightside and wxmax */
    if (p -> rightside && p -> ll_x2 > wxmax) {
	crossy = py + (wxmax - px) * p -> rightside;
	if (crossy > Max (p -> ll_y1, wymin)
	 && crossy < Min (p -> ll_y2, wymax)) a[++count] = crossy;
    }

    quicksort (a, 0, count); /* sort the y values */

    yold = oldxmin = oldxmax = 0; /* suppress compiler warning */

    for (teller = 0; teller <= count; ++teller) {

	/* intersection between current y and p->leftside */
	px = p -> ll_x1;
	py = (p -> leftside == 1)? p -> ll_y1 : p -> ll_y2;
	interx1 = px + p -> leftside * (a[teller] - py);

	/* intersection between current y and p->rightside */
	px = p -> ll_x2;
	py = (p -> rightside == 1)? p -> ll_y2 : p -> ll_y1;
	interx2 = px + p -> rightside * (a[teller] - py);

	xmin = Max (interx1, wxmin);
	xmax = Min (interx2, wxmax);

	if (teller && xmax <= wxmax && oldxmax <= wxmax &&
		xmin >= wxmin && oldxmin >= wxmin && oldxmin <= oldxmax && xmin <= xmax) {

	    if (mode == SUB_TRAP) {
		/*
		** Multiply the trapezoid with the accumulated
		** transformation matrix.
		*/
		line_x[0] = oldxmin * Mtx[0] + yold * Mtx[1] + Mtx[2];
		line_y[0] = oldxmin * Mtx[3] + yold * Mtx[4] + Mtx[5];
		line_x[1] = xmin * Mtx[0] + a[teller] * Mtx[1] + Mtx[2];
		line_y[1] = xmin * Mtx[3] + a[teller] * Mtx[4] + Mtx[5];
		line_x[2] = xmax * Mtx[0] + a[teller] * Mtx[1] + Mtx[2];
		line_y[2] = xmax * Mtx[3] + a[teller] * Mtx[4] + Mtx[5];
		line_x[3] = oldxmax * Mtx[0] + yold * Mtx[1] + Mtx[2];
		line_y[3] = oldxmax * Mtx[3] + yold * Mtx[4] + Mtx[5];
	    }
	    else {
		line_x[0] = oldxmin;
		line_y[0] = yold;
		line_x[1] = xmin;
		line_y[1] = a[teller];
		line_x[2] = xmax;
		line_y[2] = a[teller];
		line_x[3] = oldxmax;
		line_y[3] = yold;
	    }

	    /* paint the trapezoid on the screen */
	    draw_trap (line_x, line_y);
	}
	yold = a[teller];
	oldxmin = xmin;
	oldxmax = xmax;
    }
}
