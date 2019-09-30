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
struct xlist {
    struct xlist *next;
    Coor xval;
    short rv;
    short side;
};

static struct obj_node *outlist;
static void mktrp (struct xlist *l, struct xlist *r, struct ylist *y);

/*
** Insert an y-value in the y-list. Sort on value.
** INPUT: the inserted y value.
*/
struct ylist * yinsert (struct ylist *root, Coor value)
{
    struct ylist *previous, *p, *new;

    previous = NULL;

    for (p = root; p; p = p -> next) {
	if (value <= p -> yval) break;
	previous = p;
    }

    if (!p || (value != p -> yval)) {
	MALLOC (new, struct ylist);
	new -> yval = value;
	new -> next = p;
	if (!previous) root = new;
	else previous -> next = new;
    }
    return (root);
}

/*
** Insert an x-value in the x-list. Sort on value, rv and side.
** INPUT: the x value, the direction and the side.
*/
struct xlist * xinsert (struct xlist *root, Coor value, short rv, short side)
{
    struct xlist *p, *new, *previous;

    previous = NULL;
    for (p = root; p; p = p -> next) {
	if (value <= p -> xval) break;
	previous = p;
    }

    if (p) {
	while (value == p -> xval) {
	    if ((rv > p -> rv) || ((rv == p -> rv) && (side == RIGHT))) {
		previous = p;
		p = p -> next;
	    }
	    else break;
	    if (!p) break;
	}
    }

    MALLOC (new, struct xlist);
    new -> xval = value;
    new -> next = p;
    new -> side = side;
    new -> rv = rv;
    if (!previous) root = new;
    else previous -> next = new;
    return (root);
}

/*
** This routine inserts new trapezoids into the database,
** clips them against existing trapezoids and merges
** vertically whenever possible.
**
** The database will remain in maximum horizontal strip
** representation.  INPUT: a list of new trapezoids.
*/
struct obj_node * insert (struct obj_node *new_list, int lay)
{
    struct found_list *found, *del_pntr, *tmp;
    struct ylist *ycur, *free_y, *yroot;
    struct xlist *sortx, *free_p, *xp, *xleft, *xright;
    struct obj_node *free_pn, *p, *q, *window;
    Coor interx, px, py, qx, qy, crossy;
    Coor xmin, ymin, xmax, ymax;
    short solid;

    if (!new_list) return (NULL);

    solid = 0;
    yroot = NULL;
    outlist = NULL;
    sortx = NULL;

    /*
    ** calculate search window
    */
    if (!new_list -> next)
	found = quick_search (quad_root[lay], new_list);
    else {
	xmin = new_list -> ll_x1;
	ymin = new_list -> ll_y1;
	xmax = new_list -> ll_x2;
	ymax = new_list -> ll_y2;

	for (p = new_list -> next; p; p = p -> next) {
	    xmin = Min (xmin, p -> ll_x1);
	    ymin = Min (ymin, p -> ll_y1);
	    xmax = Min (xmax, p -> ll_x2);
	    ymax = Min (ymax, p -> ll_y2);
	}
	MALLOC (window, struct obj_node);
	window -> ll_x1 = xmin;
	window -> ll_y1 = ymin;
	window -> ll_x2 = xmax;
	window -> ll_y2 = ymax;
	/*
	** find intersecting trapezoids
	*/
	found = quick_search (quad_root[lay], window);
	FREE (window);
    }

    for (tmp = found; tmp; tmp = tmp -> next) {
	/*
	** insert new y-values in ylist
	*/
	q = tmp -> ptrap;
	yroot = yinsert (yroot, q -> ll_y1);
	yroot = yinsert (yroot, q -> ll_y2);
    }

    for (p = new_list; p; p = p -> next) {

	yroot = yinsert (yroot, p -> ll_y1);
	yroot = yinsert (yroot, p -> ll_y2);

	for (tmp = found; tmp; tmp = tmp -> next) {
	    q = tmp -> ptrap;
	    /*
	    ** find intersection leftside and rightside
	    */
	    px = p -> ll_x1;
	    if (p -> leftside == 1)
		py = p -> ll_y1;
	    else
		py = p -> ll_y2;

	    qx = q -> ll_x2;
	    if (q -> rightside == 1)
		qy = q -> ll_y2;
	    else
		qy = q -> ll_y1;

	    if (p -> leftside != q -> rightside) {

		crossy = qy + (qx - px + p -> leftside * (py - qy))
				    / (p -> leftside - q -> rightside);

		if (crossy > Max (p -> ll_y1, q -> ll_y1)
			&& crossy < Min (p -> ll_y2, q -> ll_y2))
		    yroot = yinsert (yroot, crossy);
	    }

	    /*
	    ** find intersection leftside and leftside
	    */
	    qx = q -> ll_x1;
	    if (q -> leftside == 1)
		qy = q -> ll_y1;
	    else
		qy = q -> ll_y2;

	    if (p -> leftside != q -> leftside) {

		crossy = qy + (qx - px + p -> leftside * (py - qy))
				    / (p -> leftside - q -> leftside);

		if (crossy > Max (p -> ll_y1, q -> ll_y1)
			&& crossy < Min (p -> ll_y2, q -> ll_y2))
		    yroot = yinsert (yroot, crossy);
	    }

	    /*
	    ** find intersection rightside and leftside
	    */
	    px = p -> ll_x2;
	    if (p -> rightside == 1)
		py = p -> ll_y2;
	    else
		py = p -> ll_y1;

	    if (p -> rightside != q -> leftside) {

		crossy = qy + (qx - px + p -> rightside * (py - qy))
				    / (p -> rightside - q -> leftside);

		if (crossy > Max (p -> ll_y1, q -> ll_y1)
			&& crossy < Min (p -> ll_y2, q -> ll_y2))
		    yroot = yinsert (yroot, crossy);
	    }

	    /*
	    ** find intersection rightside and rigthside
	    */
	    qx = q -> ll_x2;
	    if (q -> rightside == 1)
		qy = q -> ll_y2;
	    else
		qy = q -> ll_y1;

	    if (p -> rightside != q -> rightside) {

		crossy = qy + (qx - px + p -> rightside * (py - qy))
				    / (p -> rightside - q -> rightside);

		if (crossy > Max (p -> ll_y1, q -> ll_y1)
			&& crossy < Min (p -> ll_y2, q -> ll_y2))
		    yroot = yinsert (yroot, crossy);
	    }
	}
    }

    /*
    ** link new_list list and found list together
    */
    del_pntr = found;
    p = new_list;
    while (p) {
	MALLOC (tmp, struct found_list);
	tmp -> next = found;
	found = tmp;
	found -> ptrap = p;
	p = p -> next;
	found -> ptrap -> next = NULL;
    }

    /*
    ** scan in y-direction
    */
    for (ycur = yroot; ycur -> next; ycur = ycur -> next) {
	for (; sortx; sortx = free_p) {
	    free_p = sortx -> next;
	    FREE (sortx);
	}
	sortx = NULL;

	for (tmp = found; tmp; tmp = tmp -> next) {

	    p = tmp -> ptrap;
	    if ((p -> ll_y1 <= ycur -> yval) && (p -> ll_y2 > ycur -> yval)) {
		/*
		** intersection between ycur and p->leftside
		*/
		px = p -> ll_x1;
		if (p -> leftside == 1)
		    py = p -> ll_y1;
		else
		    py = p -> ll_y2;
		interx = px + p -> leftside * (ycur -> yval - py);
		sortx = xinsert (sortx, interx, p -> leftside, LEFT);

		/*
		** intersection between ycur->yval and p->rightside
		*/
		px = p -> ll_x2;
		if (p -> rightside == 1)
		    py = p -> ll_y2;
		else
		    py = p -> ll_y1;
		interx = px + p -> rightside * (ycur -> yval - py);
		sortx = xinsert (sortx, interx, p -> rightside, RIGHT);
	    }
	}

	/*
	** scan in x-direction
	*/
	xleft = sortx; /* to suppress compiler warning */
	for (xp = sortx; xp; xp = xp -> next) {

	    if (solid == 0) xleft = xp;
	    if (xp -> side == LEFT)
		solid++;
	    else
		solid--;

	    if (solid == 0) { /* add a trapezoid */
		xright = xp;
		mktrp (xleft, xright, ycur);
	    }
	}
    }
    /*
    ** delete objects from the database
    */
    for (tmp = del_pntr; tmp; tmp = tmp -> next)
	quad_delete (quad_root[lay], tmp -> ptrap);

    for (; sortx; sortx = free_p) {
	free_p = sortx -> next;
	FREE (sortx);
    }

    for (ycur = yroot; ycur; ycur = free_y) {
	free_y = ycur -> next;
	FREE (ycur);
    }

    for (; new_list; new_list = free_pn) {
	free_pn = new_list -> next;
	FREE (new_list);
    }

    for (; found; found = tmp) {
	tmp = found -> next;
	FREE (found);
    }
    return (outlist);
}

/*
** Make a new trapezoid and add it to outlist.
** INPUT: the attributes and the mask layer.
*/
static void mktrp (struct xlist *l, struct xlist *r, struct ylist *y)
{
    struct obj_node *p, *tmp;
    Coor d, x1, x2;
    /*
    ** Calculate the trapezoid.
    */
    d = y -> next -> yval - y -> yval;
    x1 = l -> xval + l -> rv * d;
    x2 = r -> xval + r -> rv * d;

    MALLOC (p, struct obj_node);
    p -> ll_x1 = Min (x1, l -> xval);
    p -> ll_x2 = Max (x2, r -> xval);
    p -> ll_y1 = y -> yval;
    p -> ll_y2 = y -> next -> yval;
    p -> leftside = l -> rv;
    p -> rightside = r -> rv;
    p -> mark = 0;
    p -> next = NULL;

    if (test_trap (p)) {
	FREE (p);
	return;
    }

    /* Merge p with other trapezoids in outlist. */
    for (tmp = outlist; tmp; tmp = tmp -> next)
	if (merge (tmp, p)) {
	    FREE (p);
	    return;
	}

    /* Add p to the outlist list. */
    p -> next = outlist;
    outlist = p;
}

/*
** Merge trapezoids a and b vertically if possible.
** INPUT: pointers to a and b.
*/
int merge (struct obj_node *a, struct obj_node *b)
{
    Coor xmin1, xmin2, xmax1, xmax2;

    if (a -> ll_y2 != b -> ll_y1) return (0);
    if ((a -> leftside != b -> leftside) || (a -> rightside != b -> rightside)) return (0);

    if (a -> leftside != 1)
	xmin1 = a -> ll_x1;
    else
	xmin1 = a -> ll_x1 + (a -> ll_y2 - a -> ll_y1);

    if (b -> leftside != -1)
	xmin2 = b -> ll_x1;
    else
	xmin2 = b -> ll_x1 + (b -> ll_y2 - b -> ll_y1);

    if (a -> rightside != -1)
	xmax1 = a -> ll_x2;
    else
	xmax1 = a -> ll_x2 - (a -> ll_y2 - a -> ll_y1);

    if (b -> rightside != 1)
	xmax2 = b -> ll_x2;
    else
	xmax2 = b -> ll_x2 - (b -> ll_y2 - b -> ll_y1);

    if (xmax1 != xmax2) return (0);
    if (xmin1 != xmin2) return (0);

    /*
    ** a and b can be merged
    */
    a -> ll_y2 = b -> ll_y2;
    a -> ll_x1 = Min (a -> ll_x1, b -> ll_x1);
    a -> ll_x2 = Max (a -> ll_x2, b -> ll_x2);

    return (1);
}

/*
** Test if trapezoid is legal.
** INPUT: pointer to trapezoid.
** OUTPUT: if not legal: 1 else 0.
*/
int test_trap (struct obj_node *p)
{
    if ((p -> ll_x2 <= p -> ll_x1) || (p -> ll_y2 <= p -> ll_y1)) return (1);

    if (Abs (p -> ll_x2 - p -> ll_x1) == Abs (p -> ll_y2 - p -> ll_y1))
	if (p -> leftside != 0 && p -> rightside != 0) return (1);
    return (0);
}
