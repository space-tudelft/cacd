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
extern int  NR_lay;
extern int *def_arr;
extern int *pict_arr;
extern Coor piwl, piwr, piwb, piwt;
extern int ImageMode;

void addel_cur (Coor xl, Coor xr, Coor yb, Coor yt, int ad_mode)
{
    register int lay;

    /* if image, and we snap: snap the crd on grid
     */
    if (ImageMode == TRUE) snap_box_to_grid (&xl, &xr, &yb, &yt);

    if (xl == xr || yb == yt) return;

    if (ad_mode == DELETE) fill_buffer (xl, xr, yb, yt, FALSE);

    for (lay = 0; lay < NR_lay; ++lay) {
	if (def_arr[lay]) {
	    if (ad_mode == ADD) {
		add_new_trap (lay, xl, yb, xr, yt, 0, 0);
		pict_arr[lay] = DRAW;
	    }
	    else {
		del_box (lay, xl, yb, xr, yt);
		pict_arr[lay] = ERAS_DR;
	    }
	}
    }
    piwl = xl;
    piwr = xr;
    piwb = yb;
    piwt = yt;
}

/*
** Add a trapezoid to the quad tree.
** The trapezoid may overlap other trapezoids.
** INPUT: the coordinats of the box and its mask layer.
*/
void add_new_trap (int lay, Coor xl, Coor yb, Coor xr, Coor yt, short lside, short rside)
{
    struct obj_node *p;
    struct obj_node *nwlist;

    if (xl == xr || yb == yt) return;

    MALLOC (p, struct obj_node);
    p -> ll_x1 = Min (xl, xr);
    p -> ll_y1 = Min (yb, yt);
    p -> ll_x2 = Max (xl, xr);
    p -> ll_y2 = Max (yb, yt);
    p -> leftside = lside;
    p -> rightside = rside;
    p -> mark = 0;
    p -> next = NULL;

    /*
    ** Put the trapezoid into the quad tree while maintaining
    ** the maximal horizontal strip representation.
    */
    nwlist = insert (p, lay);

    /*
    ** Add the new trapezoids to the 'lay' quad tree
    ** of the edited cell.
    */
    add_quad (quad_root, nwlist, lay);
}

/*
** Add a linked list of trapezoids to the 'lay' quad tree of a cell.
** The trapezoids are non-overlapping.
** INPUT: the quad_array of the cell, a linked list of trapezoids
** and the mask layer.
*/
void add_quad (qtree_t *quad_array[], struct obj_node *p, int lay)
{
    qtree_t *quad_insert ();
    struct obj_node *pn;

    /*
    ** Traverse the list of trapezoids.
    */
    while (p) {
	pn = p -> next;
	p -> next = NULL;
	p -> mark = 0;
	/*
	** Put the trapezoid into the 'lay' quad tree.
	*/
	quad_array[lay] = quad_insert (p, quad_array[lay]);
	p = pn;
    }
}
