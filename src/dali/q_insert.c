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

#define Nodethreshold 40

#define LeafNode Q -> Ncount >= 0

static int Mark = 0;

static void quad_build (qtree_t *Q);
static void subquad_insert (struct obj_node *p, qtree_t *Q);

/*
** Enlarge quad tree above its root.
** INPUT: pointer to the root node of the quad tree and quadrant.
** OUTPUT: pointer to the new root node.
*/
static qtree_t * enlarge (qtree_t *Q, int quadrant)
{
    register qtree_t *newroot;
    Coor dx, dy;

    /*
    ** enlarge quad tree above it's root
    */
    MALLOC (newroot, qtree_t); /* allocate memory for new root */
    newroot -> Uleft  = NULL;
    newroot -> Uright = NULL;
    newroot -> Lleft  = NULL;
    newroot -> Lright = NULL;
    newroot -> object = NULL;
    newroot -> reference = NULL;
    newroot -> Ncount = -1;    /* this is a non-leaf node */

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

/*
** Insert an object into a quad tree.
** INPUT: a pointer to the trapezoid
** and a pointer to a quad tree node.
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
    Mark = 0; /* object not yet added */
    p -> mark = 0; /* set init value */
    subquad_insert (p, Q);
    return (Q);
}

/*
** Create a new quadrant.
** INPUT: a pointer to the parent tree node and the desired quadrant.
** OUTPUT: a pointer to the new quadrant.
*/
static qtree_t * make (qtree_t *Q, int quadrant)
{
    register qtree_t *newnode;
    Coor xmid, ymid;

    xmid = (Q -> quadrant[0] + Q -> quadrant[2]) >> 1;
    ymid = (Q -> quadrant[1] + Q -> quadrant[3]) >> 1;

    MALLOC (newnode, qtree_t);
    newnode -> Uleft = newnode -> Uright = NULL;
    newnode -> Lleft = newnode -> Lright = NULL;
    newnode -> reference = NULL;
    newnode -> object = NULL;
    newnode -> Ncount = 0; /* this is a leaf node */

    switch (quadrant) {
    case LLEFT:
	newnode -> quadrant[0] = Q -> quadrant[0];
	newnode -> quadrant[1] = Q -> quadrant[1];
	newnode -> quadrant[2] = xmid;
	newnode -> quadrant[3] = ymid;
	break;
    case ULEFT:
	newnode -> quadrant[0] = Q -> quadrant[0];
	newnode -> quadrant[1] = ymid;
	newnode -> quadrant[2] = xmid;
	newnode -> quadrant[3] = Q -> quadrant[3];
	break;
    case LRIGHT:
	newnode -> quadrant[0] = xmid;
	newnode -> quadrant[1] = Q -> quadrant[1];
	newnode -> quadrant[2] = Q -> quadrant[2];
	newnode -> quadrant[3] = ymid;
	break;
    case URIGHT:
	newnode -> quadrant[0] = xmid;
	newnode -> quadrant[1] = ymid;
	newnode -> quadrant[2] = Q -> quadrant[2];
	newnode -> quadrant[3] = Q -> quadrant[3];
	break;
    }
    return (newnode);
}

/*
** Insert a trapezoid into the quad tree.
** INPUT: a pointer to a trapezoid and a pointer
** to a quad tree node.
*/
static void subquad_insert (struct obj_node *p, qtree_t *Q)
{
    struct ref_node *r;
    Coor xmid, ymid; /* middle of current quadrant */

    if (LeafNode) {
	/*
	** Has this object already been added?
	*/
	if (Mark++) {
	    /*
	    ** add a pointer to the object in the reference list
	    */
	    MALLOC (r, struct ref_node);
	    r -> ref = p;
	    r -> next = Q -> reference;
	    Q -> reference = r;
	    return;
	}
	/*
	** add the object
	*/
	p -> next = Q -> object;
	Q -> object = p;
	if (++Q -> Ncount <= Nodethreshold) return;

	/*
	** don't divide quad node to far
	*/
	if (Q -> quadrant[2] - Q -> quadrant[0] < 16) return;

	/*
	** build new quadrants and drop the objects
	** in the appropriate quadrant
	*/
	quad_build (Q);
    }
    else { /* no LeafNode */
	xmid = (Q -> quadrant[2] + Q -> quadrant[0]) >> 1;
	ymid = (Q -> quadrant[3] + Q -> quadrant[1]) >> 1;

	if (p -> ll_y2 >= ymid) {
	    if (p -> ll_x1 <= xmid) {
		if (!Q -> Uleft) Q -> Uleft = make (Q, ULEFT);
		subquad_insert (p, Q -> Uleft);
	    }
	    if (p -> ll_x2 >= xmid) {
		if (!Q -> Uright) Q -> Uright = make (Q, URIGHT);
		subquad_insert (p, Q -> Uright);
	    }
	}
	if (p -> ll_y1 <= ymid) {
	    if (p -> ll_x2 >= xmid) {
		if (!Q -> Lright) Q -> Lright = make (Q, LRIGHT);
		subquad_insert (p, Q -> Lright);
	    }
	    if (p -> ll_x1 <= xmid) {
		if (!Q -> Lleft) Q -> Lleft = make (Q, LLEFT);
		subquad_insert (p, Q -> Lleft);
	    }
	}
    }
}

/*
** Build new quadrants and put all the objects
** in the appropriate quadrants.
** INPUT: a pointer to a leaf tree node.
*/
static void quad_build (qtree_t *Q)
{
    register struct obj_node *p, *pn;
    register struct ref_node *q, *qn;
    Coor xmid, ymid;

    xmid = (Q -> quadrant[0] + Q -> quadrant[2]) >> 1;
    ymid = (Q -> quadrant[1] + Q -> quadrant[3]) >> 1;

    /*
    ** insert all objects of Q->object in the child node
    */
    for (pn = Q -> object; (p = pn);) {
	pn = p -> next;
	Mark = 0; /* object not yet added */
	if (p -> ll_y2 >= ymid) {
	    if (p -> ll_x1 <= xmid) {
		if (!Q -> Uleft) Q -> Uleft = make (Q, ULEFT);
		subquad_insert (p, Q -> Uleft);
	    }
	    if (p -> ll_x2 >= xmid) {
		if (!Q -> Uright) Q -> Uright = make (Q, URIGHT);
		subquad_insert (p, Q -> Uright);
	    }
	}
	if (p -> ll_y1 <= ymid) {
	    if (p -> ll_x2 >= xmid) {
		if (!Q -> Lright) Q -> Lright = make (Q, LRIGHT);
		subquad_insert (p, Q -> Lright);
	    }
	    if (p -> ll_x1 <= xmid) {
		if (!Q -> Lleft) Q -> Lleft = make (Q, LLEFT);
		subquad_insert (p, Q -> Lleft);
	    }
	}
    }

    /*
    ** insert all objects of reference list in child node
    */
    Mark = 1; /* ref. nodes: don't add */
    for (qn = Q -> reference; (q = qn);) {
	qn = q -> next;
	if (q -> ref -> ll_y2 >= ymid) {
	    if (q -> ref -> ll_x1 <= xmid) {
		if (!Q -> Uleft) Q -> Uleft = make (Q, ULEFT);
		subquad_insert (q -> ref, Q -> Uleft);
	    }
	    if (q -> ref -> ll_x2 >= xmid) {
		if (!Q -> Uright) Q -> Uright = make (Q, URIGHT);
		subquad_insert (q -> ref, Q -> Uright);
	    }
	}
	if (q -> ref -> ll_y1 <= ymid) {
	    if (q -> ref -> ll_x2 >= xmid) {
		if (!Q -> Lright) Q -> Lright = make (Q, LRIGHT);
		subquad_insert (q -> ref, Q -> Lright);
	    }
	    if (q -> ref -> ll_x1 <= xmid) {
		if (!Q -> Lleft) Q -> Lleft = make (Q, LLEFT);
		subquad_insert (q -> ref, Q -> Lleft);
	    }
	}
	FREE (q);
    }
    Q -> object = NULL;
    Q -> reference = NULL;
    Q -> Ncount = -1;		/* this is a non-leaf node */
}
