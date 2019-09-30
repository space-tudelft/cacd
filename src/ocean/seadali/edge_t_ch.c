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

extern int act_mask_lay;

/* pointer to transformation matrix of trapezoids */
extern Coor *Mtx;

typedef struct {
    Coor w_x, w_y;
} xy_pair;	/* coordinates */

/*
** Insert the non-horizontal edges of p into prelist.
** INPUT: a pointer to trapezoid p.
*/
void sub_to_checker (struct obj_node *p)
{
    struct x_lst *new_edge;
    xy_pair line[6];
    Coor p_line[8];
    Coor xmin, xmax, ymin, ymax;
    Coor x1, x2, y1, y2;
    int  d, rv[5];
    register int i, n;
    short dir, side[4];

    if (!p -> leftside && !p -> rightside) {
	/*
	** The trapezoid is a rectangle.
	*/
	x1 = Mtx[0] * p -> ll_x1 + Mtx[1] * p -> ll_y1 + Mtx[2];
	y1 = Mtx[3] * p -> ll_x1 + Mtx[4] * p -> ll_y1 + Mtx[5];
	x2 = Mtx[0] * p -> ll_x2 + Mtx[1] * p -> ll_y2 + Mtx[2];
	y2 = Mtx[3] * p -> ll_x2 + Mtx[4] * p -> ll_y2 + Mtx[5];

	xmin = Min (x1, x2);
	xmax = Max (x1, x2);
	ymin = Min (y1, y2);
	ymax = Max (y1, y2);

	to_precheck (act_mask_lay, xmin, xmax, ymin, ymax);
	return;
    }
    /*
    ** Convert trapezoid to polygon.
    */
    if (trap_to_poly (p_line, p) == -1) {
	ptext ("Illegal trapezoid!");
	return;
    }
    /*
    ** Transform the corner points up to root coordinates.
    */
    n = 0;
    for (i = 0; i < 8; i = i + 2) {
	line[n].w_x = p_line[i] * Mtx[0] + p_line[i+1] * Mtx[1] + Mtx[2];
	line[n].w_y = p_line[i] * Mtx[3] + p_line[i+1] * Mtx[4] + Mtx[5];
	if (n > 0 && line[n].w_x == line[n - 1].w_x && line[n].w_y == line[n - 1].w_y)
	    /* nothing */ ;
	else
	    n++;
    }
    line[n].w_x = line[0].w_x;
    line[n].w_y = line[0].w_y;

    /*
    ** Calculate side (LEFT or RIGHT) and direction of edges.
    */
    for (i = 0; i < n; i++) {
	x1 = line[i].w_x;
	y1 = line[i].w_y;
	x2 = line[i + 1].w_x;
	y2 = line[i + 1].w_y;
	if (y1 == y2) {
	    rv[i] = (x1 > x2) ? 0 : 4;
	}
	else if (x1 == x2) {
	    rv[i] = (y1 > y2) ? 6 : 2;
	}
	else if (x1 > x2) {
	    rv[i] = (y1 > y2) ? 7 : 1;
	}
	else {
	    rv[i] = (y1 > y2) ? 5 : 3;
	}
    }
    rv[i] = rv[0];

    for (i = 0; i < n; i++) {

	d = rv[i + 1] - rv[i];

	if (rv[i] < 4) {
	    if (d > 0 && d < 4)
		side[i] = LEFT;
	    else
		side[i] = RIGHT;
	}
	else {
	    if (d > 0 || d < -4)
		side[i] = RIGHT;
	    else
		side[i] = LEFT;
	}
    }

    for (i = 0; i < n; i++) {

	if (rv[i] < 5) {
	    xmin = line[i].w_x;
	    ymin = line[i].w_y;
	    xmax = line[i + 1].w_x;
	    ymax = line[i + 1].w_y;
	    switch (rv[i]) {
		case 1:
		    dir = -1;
		    break;
		case 2:
		    dir = 0;
		    break;
		case 3:
		    dir = 1;
		    break;
		default:
		    dir = -2;
		    break;
	    }
	}
	else {
	    xmax = line[i].w_x;
	    ymax = line[i].w_y;
	    xmin = line[i + 1].w_x;
	    ymin = line[i + 1].w_y;
	    switch (rv[i]) {
		case 5:
		    dir = -1;
		    break;
		case 6:
		    dir = 0;
		    break;
		case 7:
		    dir = 1;
		    break;
		default:
		    dir = -2;
		    break;
	    }
	}
	/*
	** Is it a non-horizontal edge?
	*/
	if (dir != -2) {
	    y_insert (ymin, act_mask_lay);
	    y_insert (ymax, act_mask_lay);

	    MALLOC (new_edge, struct x_lst);
	    new_edge -> xs = xmin;
	    new_edge -> ys = ymin;
	    new_edge -> xe = xmax;
	    new_edge -> ye = ymax;
	    new_edge -> side = side[i];
	    new_edge -> dir = dir;
	    new_edge -> layer = act_mask_lay;
	    edge_insert (new_edge);
	}
    }
}

/*
** Convert non-orthogonal elements of root cell into edges for checker.
*/
void trap_to_checker (struct obj_node *p)
{
    struct x_lst *new_edge;
    Coor line[8];

    /*
    ** Convert trapezoid to polygon.
    */
    if (trap_to_poly (line, p) == -1) {
	ptext ("Illegal trapezoid!");
	return;
    }

    /* left edge */
    MALLOC (new_edge, struct x_lst);
    new_edge -> xs = line[0];
    new_edge -> ys = line[1];
    new_edge -> xe = line[6];
    new_edge -> ye = line[7];
    new_edge -> side = LEFT;
    new_edge -> dir = p -> leftside;
    new_edge -> layer = act_mask_lay;
    edge_insert (new_edge);

    y_insert (p -> ll_y1, act_mask_lay);
    y_insert (p -> ll_y2, act_mask_lay);

    /* right edge */
    MALLOC (new_edge, struct x_lst);
    new_edge -> xs = line[2];
    new_edge -> ys = line[3];
    new_edge -> xe = line[4];
    new_edge -> ye = line[5];
    new_edge -> side = RIGHT;
    new_edge -> dir = p -> rightside;
    new_edge -> layer = act_mask_lay;
    edge_insert (new_edge);
}

/*
** Convert rectangular objects into edges for checker.
*/
void to_precheck (int lay, Coor xmin, Coor xmax, Coor ymin, Coor ymax)
{
    struct x_lst *new_edge;
    MALLOC (new_edge, struct x_lst);
    new_edge -> xs = xmin;
    new_edge -> ys = ymin;
    new_edge -> xe = xmin;
    new_edge -> ye = ymax;
    new_edge -> side = LEFT;
    new_edge -> dir = 0;
    new_edge -> layer = lay;
    edge_insert (new_edge);

    MALLOC (new_edge, struct x_lst);
    new_edge -> xs = xmax;
    new_edge -> ys = ymin;
    new_edge -> xe = xmax;
    new_edge -> ye = ymax;
    new_edge -> side = RIGHT;
    new_edge -> dir = 0;
    new_edge -> layer = lay;
    edge_insert (new_edge);

    y_insert (ymin, lay);
    y_insert (ymax, lay);
}
