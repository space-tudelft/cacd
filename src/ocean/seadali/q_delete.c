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

static void del_nodes (qtree_t *quad_el, struct obj_node *p, struct obj_node *previous);
static void del_ref   (qtree_t *quad_el, struct ref_node *p, struct ref_node *previous);
static int equal (struct obj_node *a, struct obj_node *b);

/*
** Delete an element from the quad tree.
** INPUT: a pointer 'Q' to a quad tree node
**        a pointer 'srch' to the object which must be deleted
**        (which element serves as the search range).
*/
void quad_delete (qtree_t *Q, struct obj_node *srch)
{
    struct obj_node *p, *pn, *prev;
    struct ref_node *r, *rn, *rprev;

    if (!Q) return;	/* empty node */

    /*
    ** if current quadrant and desired range don't intersect: return
    */
    if (Q -> quadrant[0] > srch -> ll_x2 ||
	Q -> quadrant[1] > srch -> ll_y2 ||
	Q -> quadrant[2] < srch -> ll_x1 ||
	Q -> quadrant[3] < srch -> ll_y1) return;

    /*
    ** check for object references intersecting the desired quadrant
    */
    rprev = NULL;
    for (r = Q -> reference; r; r = rn) {
	rn = r -> next;
	if (equal (r -> ref, srch))
	    del_ref (Q, r, rprev);
	else
	    rprev = r;
    }

    /*
    ** check for nodes intersecting the desired quadrant in this quad
    */
    prev = NULL;
    for (p = Q -> object; p; p = pn) {
	pn = p -> next;
	if (equal (p, srch))
	    del_nodes (Q, p, prev);
	else
	    prev = p;
    }

    if (Q -> Uleft ) quad_delete (Q -> Uleft, srch);
    if (Q -> Lleft ) quad_delete (Q -> Lleft, srch);
    if (Q -> Uright) quad_delete (Q -> Uright, srch);
    if (Q -> Lright) quad_delete (Q -> Lright, srch);
}

/*
** Are trapezoids a and b equal?
** INPUT: pointers to trapezoids a and b.
** OUTPUT: FALSE if not equal, TRUE if equal.
*/
static int equal (struct obj_node *a, struct obj_node *b)
{
    if (a -> ll_x1 == b -> ll_x1 &&
	a -> ll_y1 == b -> ll_y1 &&
	a -> ll_x2 == b -> ll_x2 &&
	a -> ll_y2 == b -> ll_y2 &&
	a -> leftside == b -> leftside &&
	a -> rightside == b -> rightside) return (TRUE);
    else
	return (FALSE);
}

/*
** Delete a trapezoid from the object list.
** INPUT: a quad tree node, a pointer to the trapezoid and a pointer
** to the previous trapezoid in the object list.
*/
static void del_nodes (qtree_t *quad_el, struct obj_node *p, struct obj_node *previous)
{
    if (!previous)
	quad_el -> object = p -> next;
    else
	previous -> next = p -> next;
    FREE (p);
    if (quad_el -> Ncount != -1) quad_el -> Ncount--;
}

/*
** Delete a trapezoid from the reference list.
** INPUT: a quad tree node, a pointer to the trapezoid and a pointer
** to the previous trapezoid in the reference list.
*/
static void del_ref (qtree_t *quad_el, struct ref_node *p, struct ref_node *previous)
{
    if (!previous)
	quad_el -> reference = p -> next;
    else
	previous -> next = p -> next;
    FREE (p);
}
