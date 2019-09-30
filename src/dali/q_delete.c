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

#define Quad_outside_window \
Q -> quadrant[0] > window -> ll_x2 || Q -> quadrant[1] > window -> ll_y2 || \
Q -> quadrant[2] < window -> ll_x1 || Q -> quadrant[3] < window -> ll_y1

#define LeafNode Q -> Ncount >= 0

static struct obj_node *window;	/* search window */
static int Object_found = 0;

static void _quad_delete (qtree_t *Q);

/*
** Delete an element from the quad tree.
** INPUT: a pointer to a quad tree node and to the object
** (the object also serves as the search window).
*/
void quad_delete (qtree_t *Q, struct obj_node *p)
{
    if (!Q) return;
    window = p;
    Object_found = 0;
    _quad_delete (Q);
    if (Object_found) FREE (window); /* Mark van Doesburg (UT) 9-8-1997 */
}

static void _quad_delete (qtree_t *Q)
{
    register struct obj_node *p, *prev;
    register struct ref_node *r, *rprev;

    if (Quad_outside_window) return;

    if (LeafNode) {
	/*
	** check the object references
	*/
	if ((r = Q -> reference)) {
	    rprev = NULL;
	    do {
		if (r -> ref == window) { /* same address, same object */
		    /*
		    ** delete the trapezoid from the reference list
		    */
		    if (rprev) rprev -> next = r -> next;
		    else      Q -> reference = r -> next;

		    FREE (r); /* reference node */
		    break; /* only one reference of object in this quad */
		}
		rprev = r;
	    } while ((r = r -> next));
	}

	/*
	** check the objects for the object to be deleted
	*/
	if (!Object_found && (p = Q -> object)) {
	    prev = NULL;
	    do {
		if (p == window) { /* same address, same object */
		    /*
		    ** delete the trapezoid from the object list
		    */
		    if (prev) prev -> next = p -> next;
		    else       Q -> object = p -> next;
#if 0
                    /* We must do this in quad_delete, it still gets
                       referenced in subsequent calls to _quad_delete
                       Mark van Doesburg (UT) 9-8-1997
                     */
                    FREE (p); /* the object */
#endif
		    Q -> Ncount--; /* lower the object counter */
		    Object_found = 1;
		    break; /* requested object found */
		}
		prev = p;
	    } while ((p = p -> next));
	}
    }
    else { /* no LeafNode */
	if (Q -> Uleft ) _quad_delete (Q -> Uleft);
	if (Q -> Lleft ) _quad_delete (Q -> Lleft);
	if (Q -> Uright) _quad_delete (Q -> Uright);
	if (Q -> Lright) _quad_delete (Q -> Lright);
    }
}
