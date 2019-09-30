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

/*
** Find those parts of p outside q.
** INPUT: pointers to trapezoids p and q and  pointer to
** a list of trapezoids to which the result must be added.
** OUTPUT: a pointer to the list of resulting trapezoids.
*/
struct obj_node * clip (struct obj_node *p, struct obj_node *q, struct obj_node *clip_head)
{
    struct obj_node *new;
    Coor a[8];
    Coor crossy1, crossy2, crossy3, crossy4;
    Coor interx1, interx2, interx3, interx4;
    Coor px, py, qx, qy;
    Coor lxmin, lxmax, loldxmin, loldxmax;
    Coor rxmin, rxmax, roldxmin, roldxmax;
    Coor yold, prev1, prev3;
    register int count, teller;

    if (q -> ll_y1 >= p -> ll_y2 || q -> ll_y2 <= p -> ll_y1 ||
	q -> ll_x1 >= p -> ll_x2 || q -> ll_x2 <= p -> ll_x1) {
	/*
	** no intersection: p lies completely outside q: return p
	*/
	MALLOC (new, struct obj_node);
	new -> ll_x1 = p -> ll_x1;
	new -> ll_y1 = p -> ll_y1;
	new -> ll_x2 = p -> ll_x2;
	new -> ll_y2 = p -> ll_y2;
	new -> leftside = p -> leftside;
	new -> rightside = p -> rightside;
	new -> mark = 0;
	new -> next = clip_head;
	return (new);
    }

    count = 0;

    /*
    ** find y scanline values
    */
    a[count++] = p -> ll_y1;
    a[count] = p -> ll_y2;
    if (q -> ll_y1 > p -> ll_y1 && q -> ll_y1 < p -> ll_y2) a[++count] = q -> ll_y1;
    if (q -> ll_y2 > p -> ll_y1 && q -> ll_y2 < p -> ll_y2) a[++count] = q -> ll_y2;

    /*
    ** find intersection between p->leftside and q->rightside
    */
    if (p -> leftside != q -> rightside) {
	px = p -> ll_x1;
	py = (p -> leftside  == 1)? p -> ll_y1 : p -> ll_y2;
	qx = q -> ll_x2;
	qy = (q -> rightside == 1)? q -> ll_y2 : q -> ll_y1;

	crossy1 = qy + (qx - px + p -> leftside * (py - qy)) / (p -> leftside - q -> rightside);

	if (crossy1 > Max (p -> ll_y1, q -> ll_y1) && crossy1 < Min (p -> ll_y2, q -> ll_y2))
	    a[++count] = crossy1;
    }

    /*
    ** find intersection between p->leftside and q->leftside
    */
    if (p -> leftside != q -> leftside) {
	px = p -> ll_x1;
	py = (p -> leftside == 1)? p -> ll_y1 : p -> ll_y2;
	qx = q -> ll_x1;
	qy = (q -> leftside == 1)? q -> ll_y1 : q -> ll_y2;

	crossy4 = qy + (qx - px + p -> leftside * (py - qy)) / (p -> leftside - q -> leftside);

	if (crossy4 > Max (p -> ll_y1, q -> ll_y1) && crossy4 < Min (p -> ll_y2, q -> ll_y2))
	    a[++count] = crossy4;
    }

    /*
    ** find intersection between p->rightside and q->leftside
    */
    if (p -> rightside != q -> leftside) {
	px = p -> ll_x2;
	py = (p -> rightside == 1)? p -> ll_y2 : p -> ll_y1;
	qx = q -> ll_x1;
	qy = (q -> leftside  == 1)? q -> ll_y1 : q -> ll_y2;

	crossy2 = qy + (qx - px + p -> rightside * (py - qy)) / (p -> rightside - q -> leftside);

	if (crossy2 > Max (p -> ll_y1, q -> ll_y1) && crossy2 < Min (p -> ll_y2, q -> ll_y2))
	    a[++count] = crossy2;
    }

    /*
    ** find intersection between p->rightside and q->rigthside
    */
    if (p -> rightside != q -> rightside) {
	px = p -> ll_x2;
	py = (p -> rightside == 1)? p -> ll_y2 : p -> ll_y1;
	qx = q -> ll_x2;
	qy = (q -> rightside == 1)? q -> ll_y2 : q -> ll_y1;

	crossy3 = qy + (qx - px + p -> rightside * (py - qy)) / (p -> rightside - q -> rightside);

	if (crossy3 > Max (p -> ll_y1, q -> ll_y1) && crossy3 < Min (p -> ll_y2, q -> ll_y2))
	    a[++count] = crossy3;
    }

    /* sort the y-values in a[]
    */
    quicksort (a, 0, count);

    /* suppress compiler warnings */
    yold = 0;
    loldxmin = loldxmax = 0;
    roldxmin = roldxmax = 0;
    prev1 = prev3 = 0;

    /* scan in the y-direction
    */
    for (teller = 0; teller <= count; ++teller) {
	/*
	** intersection between current y and p->leftside
	*/
	px = p -> ll_x1;
	py = (p -> leftside == 1)? p -> ll_y1 : p -> ll_y2;
	interx1 = px + p -> leftside * (a[teller] - py);

	/* intersection between current y and q->leftside
	*/
	qx = q -> ll_x1;
	qy = (q -> leftside == 1)? q -> ll_y1 : q -> ll_y2;
	interx2 = qx + q -> leftside * (a[teller] - qy);

	/* intersection between current y and p->rightside
	*/
	px = p -> ll_x2;
	py = (p -> rightside == 1)? p -> ll_y2 : p -> ll_y1;
	interx3 = px + p -> rightside * (a[teller] - py);

	/* intersection between current y and q->rightside
	*/
	qx = q -> ll_x2;
	qy = (q -> rightside == 1)? q -> ll_y2 : q -> ll_y1;
	interx4 = qx + q -> rightside * (a[teller] - qy);

	lxmin = interx1;
	lxmax = Min (interx2, interx3);
	rxmax = interx3;
	rxmin = Max (interx4, interx1);

	if (teller) {
	    if (a[teller] == q -> ll_y1 && yold == p -> ll_y1)
		clip_head = out (prev1, interx1, prev3, interx3, yold, a[teller], clip_head);
	    else if (a[teller] == p -> ll_y2 && yold == q -> ll_y2)
		clip_head = out (prev1, interx1, prev3, interx3, yold, a[teller], clip_head);
	    else {
		if (interx1 <= interx2) clip_head = out (loldxmin, lxmin, loldxmax, lxmax, yold, a[teller], clip_head);
		if (interx3 >= interx4) clip_head = out (roldxmin, rxmin, roldxmax, rxmax, yold, a[teller], clip_head);
	    }
	}
	yold = a[teller];
	loldxmin = lxmin;
	loldxmax = lxmax;
	roldxmin = rxmin;
	roldxmax = rxmax;
	prev1 = interx1;
	prev3 = interx3;
    }
    return (clip_head);
}
