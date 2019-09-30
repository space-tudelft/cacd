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
struct xlist {
    struct xlist *next;
    Coor xval;
    short rv;
    short side;
};

static struct obj_node *outlist;

static void mktrp (short ls, Coor xmin, Coor y1, Coor y2, struct xlist *r);

/*
** Insert an y-value in the y-list. Sort on value.
** INPUT: the inserted y value.
*/
struct ylist * yinsert (struct ylist *root, Coor value)
{
    register struct ylist *p, *new, *previous;

    previous = NULL;
    for (p = root; p; p = p -> next) {
	if (value <= p -> yval) break;
	previous = p;
    }

    if (!p || value != p -> yval) {
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
static struct xlist * xinsert (struct xlist *root, Coor value, short rv, short side)
{
    register struct xlist *p, *new, *previous;

    previous = NULL;
    for (p = root; p; p = p -> next) {
	if (value <= p -> xval) break;
	previous = p;
    }

    if (p) {
	while (value == p -> xval) {
	    if (rv > p -> rv || (rv == p -> rv && side == RIGHT)) {
		previous = p;
		if (!(p = p -> next)) break;
	    }
	    else break;
	}
    }

    MALLOC (new, struct xlist);
    new -> xval = value;
    new -> next = p;
    new -> side = side;
    new -> rv   = rv;
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
struct obj_node * insert (struct obj_node *new_list, int lay, int mode)
{
    struct found_list *found;
    register struct found_list *fp, *fq, *nlist;
    register struct ylist *yroot, *ynext;
    register struct xlist *xroot, *xcur;
    register struct obj_node *p, *q;
    Coor px, py, qy, yc, yn, ymin, ymax;
    short pls, prs, qls, qrs;
    int solid, n;

    if (!new_list) return (NULL);

    yroot = NULL;
    /*
    ** find intersecting trapezoids
    */
    found = quick_search (quad_root[lay], new_list);

    /*
    ** link new_list list and found list together
    */
    nlist = found;
    if (mode == ADD) {
	for (p = new_list; p; p = p -> next) {
	    MALLOC (fp, struct found_list);
	    fp -> ptrap = p;
	    fp -> next = nlist;
	    nlist = fp;
	}
    }

    for (fp = nlist; fp; fp = fp -> next) {
	p = fp -> ptrap;

	yroot = yinsert (yroot, p -> ll_y1);
	yroot = yinsert (yroot, p -> ll_y2);

	pls = p -> sides;
	if ((prs = pls & 3) > 1) prs = -1;
	if ((pls >>= 2)     > 1) pls = -1;

	for (fq = fp -> next; fq; fq = fq -> next) {
	    q = fq -> ptrap;

	    ymin = Min (p -> ll_y2, q -> ll_y2);
	    ymax = Max (p -> ll_y1, q -> ll_y1);
	    if (ymax >= ymin) continue;

	    qls = q -> sides;
	    if ((qrs = qls & 3) > 1) qrs = -1;
	    if ((qls >>= 2)     > 1) qls = -1;

	    py = (pls == 1) ? p -> ll_y1 : p -> ll_y2;

	    /* find intersection p->leftside and q->rightside */
	    if ((n = pls - qrs)) {
		qy = (qrs == 1) ? q -> ll_y2 : q -> ll_y1;
		yc = qy + (q -> ll_x2 - p -> ll_x1 + pls * (py - qy)) / n;
		if (yc > ymax && yc < ymin) yroot = yinsert (yroot, yc);
	    }

	    qy = (qls == 1) ? q -> ll_y1 : q -> ll_y2;

	    /* find intersection p->leftside and q->leftside */
	    if ((n = pls - qls)) {
		yc = qy + (q -> ll_x1 - p -> ll_x1 + pls * (py - qy)) / n;
		if (yc > ymax && yc < ymin) yroot = yinsert (yroot, yc);
	    }

	    py = (prs == 1) ? p -> ll_y2 : p -> ll_y1;

	    /* find intersection p->rightside and q->leftside */
	    if ((n = prs - qls)) {
		yc = qy + (q -> ll_x1 - p -> ll_x2 + prs * (py - qy)) / n;
		if (yc > ymax && yc < ymin) yroot = yinsert (yroot, yc);
	    }

	    /* find intersection p->rightside and q->rightside */
	    if ((n = prs - qrs)) {
		qy = (qrs == 1) ? q -> ll_y2 : q -> ll_y1;
		yc = qy + (q -> ll_x2 - p -> ll_x2 + prs * (py - qy)) / n;
		if (yc > ymax && yc < ymin) yroot = yinsert (yroot, yc);
	    }
	}
    }

    outlist = NULL;
    if (yroot) {
    /*
    ** scan in y-direction
    */
    solid = 0;
    px = 0; pls = 0; /* init, to suppress compiler warning */
    xroot = NULL;
    yn = yroot -> yval;
    ynext = yroot -> next;
    FREE (yroot);
    while ((yroot = ynext)) {
	yc = yn;
	yn = yroot -> yval;
	ynext = yroot -> next;
	FREE (yroot);

	for (fp = nlist; fp; fp = fp -> next) {
	    p = fp -> ptrap;
	    if (p -> ll_y1 <= yc && p -> ll_y2 > yc) {
		pls = p -> sides;
		if ((prs = pls & 3) > 1) prs = -1;
		if ((pls >>= 2)     > 1) pls = -1;

		/* intersection between yc and p->leftside */
		px = p -> ll_x1;
		if (pls) {
		    py = (pls == 1) ? p -> ll_y1 : p -> ll_y2;
		    px += pls * (yc - py);
		}
		xroot = xinsert (xroot, px, pls, LEFT);

		/* intersection between yc and p->rightside */
		px = p -> ll_x2;
		if (prs) {
		    py = (prs == 1) ? p -> ll_y2 : p -> ll_y1;
		    px += prs * (yc - py);
		}
		xroot = xinsert (xroot, px, prs, RIGHT);
	    }
	}

	/*
	** scan in x-direction
	*/
	while ((xcur = xroot)) {
	    if (solid == 0) { /* leftside */
		pls = xcur -> rv;
		px = xcur -> xval;
	    }
	    if (xcur -> side == LEFT) ++solid;
	    else --solid;
	    if (solid == 0) mktrp (pls, px, yc, yn, xcur);
	    xroot = xcur -> next;
	    FREE (xcur);
	}
    }
    }
    /*
    ** delete found objects from the database
    */
    for (fp = found; fp; fp = fp -> next) {
	quad_delete (quad_root[lay], fp -> ptrap);
    }
    while ((fp = nlist)) {
	nlist = fp -> next;
	FREE (fp);
    }
    return (outlist);
}

/*
** Make a new trapezoid and add it to outlist.
** INPUT: the attributes and the mask layer.
*/
static void mktrp (short ls, Coor xmin, Coor y1, Coor y2, struct xlist *r)
{
    register struct obj_node *p;
    Coor dx, dy, xmax;
    short rs, sides;

    dy = y2 - y1;
    if (ls < 0) xmin += ls * dy;
    xmax = r -> xval;
    if ((rs = r -> rv) > 0) xmax += rs * dy;

    /* test_trap */
    if ((dx = xmax - xmin) <= 0) return;
    if (dx == dy && ls && rs) return;

    if (rs < 0) rs = 2;
    if (ls < 0) ls = 2;
    sides = rs + (ls << 2);

    /*
    ** merge trapezoid with other trapezoids in outlist vertically
    */
    for (p = outlist; p; p = p -> next) {

	if (p -> ll_y2 != y1 || p -> sides != sides) continue;

	dx = p -> ll_y2 - p -> ll_y1;

	switch (ls) {
	case 0:
	    if (p -> ll_x1 != xmin) continue;
	    break;
	case 1:
	    if (p -> ll_x1 + dx != xmin) continue;
	    break;
	case 2:
	    if (p -> ll_x1 != xmin + dy) continue;
	}
	switch (rs) {
	case 0:
	    if (p -> ll_x2 != xmax) continue;
	    break;
	case 1:
	    if (p -> ll_x2 != xmax - dy) continue;
	    break;
	case 2:
	    if (p -> ll_x2 - dx != xmax) continue;
	}
	/*
	** trapezoid can be merged
	*/
	p -> ll_y2 = y2;
	if (ls == 2) p -> ll_x1 = xmin;
	if (rs == 1) p -> ll_x2 = xmax;
	return;
    }

    MALLOC (p, struct obj_node);
    p -> ll_x1 = xmin;
    p -> ll_x2 = xmax;
    p -> ll_y1 = y1;
    p -> ll_y2 = y2;
    p -> sides = sides;
    p -> next = outlist; /* add p to the outlist */
    outlist = p;
}
