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

extern qtree_t **quad_root;
extern struct obj_node **PutBuf;
extern Coor piwl, piwr, piwb, piwt;
extern int  NR_lay;
extern int *def_arr;
extern int *pict_arr;

void addel_cur (Coor xl, Coor xr, Coor yb, Coor yt, int mode)
{
    struct obj_node Box;
    register int lay;
    int first = 1;

    if (xl >= xr || yb >= yt) return;

    /*
    ** Test for defined layers.
    */
    for (lay = 0; lay < NR_lay; ++lay) {
	if (def_arr[lay]) break;
    }
    if (lay == NR_lay) {
	btext ("Set your layer first!");
	sleep (1);
	return;
    }

    Box.ll_x1 = xl;
    Box.ll_x2 = xr;
    Box.ll_y1 = yb;
    Box.ll_y2 = yt;
    Box.sides = 0;
    Box.next  = 0;

    for (lay = 0; lay < NR_lay; ++lay) {

	if (mode == DELETE) clearPutBuf (lay);

	if (def_arr[lay]) {
	    if (mode == ADD) {
		add_new_traps (lay, &Box);
		pict_arr[lay] = DRAW;
	    }
	    else {
		del_traps (&Box, lay);
		if (PutBuf[lay]) {
		    pict_arr[lay] = ERAS_DR;
		    calc_bbox (first, PutBuf[lay]);
		    if (first) first = 0;
		}
	    }
	}
    }
    if (mode == ADD) { /* calc_bbox */
	piwl = xl; piwr = xr;
	piwb = yb; piwt = yt;
    }
}

/*
** Add new trapezoids to the quad tree.
** The trapezoids may overlap other trapezoids.
** INPUT: a list of new trapezoids.
*/
void add_new_traps (int lay, struct obj_node *traplist)
{
    struct obj_node *nwlist;
    /*
    ** Put the trapezoids into the quad tree while maintaining
    ** the maximal horizontal strip representation.
    ** Note: merge intersecting trapezoids with traplist.
    */
    nwlist = insert (traplist, lay, ADD);
    /*
    ** Add the new trapezoids to the 'lay' quad tree of the edited cell.
    */
    add_quad (quad_root, nwlist, lay);
}

/*
** Add a linked list of trapezoids to the 'lay' quad tree of a cell.
** INPUT: the quad_array of the cell, a linked list of trapezoids
** and the mask layer.
*/
void add_quad (qtree_t *quad_array[], struct obj_node *nwlist, int lay)
{
    qtree_t *qroot;
    register struct obj_node *p;

    qroot = quad_array[lay];
    /*
    ** Traverse the list of trapezoids.
    ** Put each trapezoid into the 'lay' quad tree.
    */
    while ((p = nwlist)) {
	nwlist = p -> next;
	qroot = quad_insert (p, qroot);
    }
    quad_array[lay] = qroot;
}
