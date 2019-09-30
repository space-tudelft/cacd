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

#define Nodethreshold 40

static qtree_t *enlarge (qtree_t *Q, short quadrant);

/*
** Insert an object into a quad tree.
** INPUT: a pointer 'p' to the trapezoid (object node)
**        a pointer 'Q' to a quad tree node (element).
** OUTPUT: a pointer to a quad tree node.
*/
qtree_t * quad_insert (struct obj_node *p, qtree_t *Q)
{
    /*
    ** If the trapezoid lies outside the quadrant
    ** then enlarge the quad tree above its root.
    */
    if (p -> ll_x1 < Q -> quadrant[0]) {
	if (p -> ll_y1 >= Q -> quadrant[1])
	    Q = enlarge (Q, ULEFT);
	else
	    Q = enlarge (Q, LLEFT);
	Q = quad_insert (p, Q);
	return (Q);
    }

    if (p -> ll_x2 > Q -> quadrant[2]) {
	if (p -> ll_y1 >= Q -> quadrant[1])
	    Q = enlarge (Q, URIGHT);
	else
	    Q = enlarge (Q, LRIGHT);
	Q = quad_insert (p, Q);
	return (Q);
    }

    if (p -> ll_y1 < Q -> quadrant[1]) {
	if (p -> ll_x1 >= Q -> quadrant[0])
	    Q = enlarge (Q, LRIGHT);
	else
	    Q = enlarge (Q, LLEFT);
	Q = quad_insert (p, Q);
	return (Q);
    }

    if (p -> ll_y2 > Q -> quadrant[3]) {
	if (p -> ll_x1 >= Q -> quadrant[0])
	    Q = enlarge (Q, URIGHT);
	else
	    Q = enlarge (Q, ULEFT);
	Q = quad_insert (p, Q);
	return (Q);
    }

    subquad_insert (p, Q);
    return (Q);
}

/*
** Insert a trapezoid into the quad tree.
** INPUT: a pointer to a trapezoid and a pointer
** to a quad tree node.
*/
void subquad_insert (struct obj_node *p, qtree_t *Q)
{
    struct ref_node *r;
    Coor xmid, ymid; /* middle of current quadrant */

    if (Q -> Ncount != -1) {	/* this is a leaf node */
	/*
	** Has this object already been added?
	*/
	if (p -> mark == 1) {
	    /*
	    ** add a pointer to the object in the reference list
	    */
	    MALLOC (r, struct ref_node);
	    r -> ref = p;
	    r -> next = Q -> reference;
	    Q -> reference = r;
	    return;
	}

	p -> next = Q -> object;
	Q -> object = p;
	p -> mark = 1;
	Q -> Ncount++;
	if (Q -> Ncount <= Nodethreshold) return;

	/*
	** don't divide quad node to far
	*/
	if (((Q -> quadrant[2] - Q -> quadrant[0]) < 8) ||
	    ((Q -> quadrant[3] - Q -> quadrant[1]) < 8)) return;

	/*
	** build new quadrants and drop the objects
	** in the appropriate quadrant
	*/
	quad_build (Q);
	return;
    }

    /*
    ** this is a non-leaf node
    */
    xmid = (Q -> quadrant[2] + Q -> quadrant[0]) >> 1;
    ymid = (Q -> quadrant[3] + Q -> quadrant[1]) >> 1;

    if (p -> ll_y2 >= ymid && p -> ll_x1 <= xmid) {
	if (!Q -> Uleft) Q -> Uleft = make (Q, ULEFT);
	subquad_insert (p, Q -> Uleft); /* object is in upper-left quadrant */
    }
    if (p -> ll_x2 >= xmid && p -> ll_y2 >= ymid) {
	if (!Q -> Uright) Q -> Uright = make (Q, URIGHT);
	subquad_insert (p, Q -> Uright); /* object is in upper-right quadrant */
    }
    if (p -> ll_x2 >= xmid && p -> ll_y1 <= ymid) {
	if (!Q -> Lright) Q -> Lright = make (Q, LRIGHT);
	subquad_insert (p, Q -> Lright); /* object is in lower-right quadrant */
    }
    if (p -> ll_x1 <= xmid && p -> ll_y1 <= ymid) {
	if (!Q -> Lleft) Q -> Lleft = make (Q, LLEFT);
	subquad_insert (p, Q -> Lleft); /* object is in lower-left quadrant */
    }
}

/*
** Enlarge quad tree above its root.
** INPUT: pointer to the root node of the quad tree and quadrant.
** OUTPUT: pointer to the new root node.
*/
static qtree_t * enlarge (qtree_t *Q, short quadrant)
{
    qtree_t *newroot;
    Coor dx, dy;

    /*
    ** enlarge quad tree above it's root
    */
    MALLOC (newroot, qtree_t); /* allocate memory for new root */
    newroot -> Uleft = newroot -> Uright = NULL;
    newroot -> Lleft = newroot -> Lright = NULL;
    newroot -> object = NULL;
    newroot -> reference = NULL;
    newroot -> Ncount = -1;

    dx = Q -> quadrant[2] - Q -> quadrant[0];
    dy = Q -> quadrant[3] - Q -> quadrant[1];

    switch (quadrant) {
    case ULEFT:
	newroot -> quadrant[0] = Q -> quadrant[0] - dx;
	newroot -> quadrant[1] = Q -> quadrant[1];
	newroot -> quadrant[2] = Q -> quadrant[2];
	newroot -> quadrant[3] = Q -> quadrant[3] + dy;
	newroot -> Lright = Q;
	break;
    case URIGHT:
	newroot -> quadrant[0] = Q -> quadrant[0];
	newroot -> quadrant[1] = Q -> quadrant[1];
	newroot -> quadrant[2] = Q -> quadrant[2] + dx;
	newroot -> quadrant[3] = Q -> quadrant[3] + dy;
	newroot -> Lleft = Q;
	break;
    case LLEFT:
	newroot -> quadrant[0] = Q -> quadrant[0] - dx;
	newroot -> quadrant[1] = Q -> quadrant[1] - dy;
	newroot -> quadrant[2] = Q -> quadrant[2];
	newroot -> quadrant[3] = Q -> quadrant[3];
	newroot -> Uright = Q;
	break;
    case LRIGHT:
	newroot -> quadrant[0] = Q -> quadrant[0];
	newroot -> quadrant[1] = Q -> quadrant[1] - dy;
	newroot -> quadrant[2] = Q -> quadrant[2] + dx;
	newroot -> quadrant[3] = Q -> quadrant[3];
	newroot -> Uleft = Q;
	break;
    }
    return (newroot);
}
