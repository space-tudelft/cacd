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

#define Quad_complete_inside_window \
Q_xl < Q -> quadrant[0] && Q_yb < Q -> quadrant[1] && \
Q_xr > Q -> quadrant[2] && Q_yt > Q -> quadrant[3]

#define Quad_outside_window \
Q -> quadrant[0] > Q_xr || Q -> quadrant[1] > Q_yt || \
Q -> quadrant[2] < Q_xl || Q -> quadrant[3] < Q_yb

#define Object_outside_window \
p -> ll_x1 > Q_xr || p -> ll_y1 > Q_yt || \
p -> ll_x2 < Q_xl || p -> ll_y2 < Q_yb

#define LeafNode Q -> Ncount >= 0

Coor Q_xl, Q_xr, Q_yb, Q_yt;	/* quad search window */

static void (*funct) ();
static short Mark = 3; /* unique initial value */

static void fast_search (qtree_t *Q);
static void lim_q_search (qtree_t *Q);
static void lim_unmark (qtree_t *Q);
static void presearch (qtree_t *Q);
static void q_search (qtree_t *Q);
static void unmark (qtree_t *Q);

/*
** Search a specified area for trapezoids.
** IN: a pointer to a quad tree node and the search window and
**     a pointer to function that is to be executed for found trapezoids.
** OUT: a list of pointers to trapezoids intersecting the search window.
*/
void quad_search (qtree_t *Q, void (*function)())
{
    if (!Q) return;
    funct = function;

    if (Quad_complete_inside_window) {
	fast_search (Q);
    }
    else if (Mark) {
	q_search (Q);
	++Mark; /* set new unique mark */
    }
    else {
	unmark (Q);
	Mark = 2; /* unique value for this strategy */
	q_search (Q);
	Mark = 0; /* use always unmark strategy */
    }
}

static void fast_search (qtree_t *Q)
{
    register struct obj_node *p;

    if (LeafNode) {
	for (p = Q -> object; p; p = p -> next) (*funct) (p);
    }
    else { /* no LeafNode */
	if (Q -> Uleft ) fast_search (Q -> Uleft);
	if (Q -> Uright) fast_search (Q -> Uright);
	if (Q -> Lright) fast_search (Q -> Lright);
	if (Q -> Lleft ) fast_search (Q -> Lleft);
    }
}

/*
** Unmark the trapezoids intersecting the search window.
*/
static void unmark (qtree_t *Q)
{
    register struct obj_node *p;
    register struct ref_node *r;

    if (Quad_outside_window) return;

    if (Quad_complete_inside_window) {
	lim_unmark (Q);
    }
    else if (LeafNode) {
	for (r = Q -> reference; r; r = r -> next) r -> ref -> mark = 0;

	for (p = Q -> object; p; p = p -> next) p -> mark = 0;
    }
    else { /* no LeafNode */
	if (Q -> Uleft ) unmark (Q -> Uleft);
	if (Q -> Uright) unmark (Q -> Uright);
	if (Q -> Lright) unmark (Q -> Lright);
	if (Q -> Lleft ) unmark (Q -> Lleft);
    }
}

static void lim_unmark (qtree_t *Q)
{
    register struct obj_node *p;

    if (LeafNode) {
	for (p = Q -> object; p; p = p -> next) p -> mark = 0;
    }
    else { /* no LeafNode */
	if (Q -> Uleft ) lim_unmark (Q -> Uleft);
	if (Q -> Uright) lim_unmark (Q -> Uright);
	if (Q -> Lright) lim_unmark (Q -> Lright);
	if (Q -> Lleft ) lim_unmark (Q -> Lleft);
    }
}

/*
** Recursively search the quad tree.
*/
static void q_search (qtree_t *Q)
{
    register struct obj_node *p;
    register struct ref_node *r;

    if (Quad_outside_window) return;

    if (Quad_complete_inside_window) {
	lim_q_search (Q);
    }
    else if (LeafNode) {
	for (r = Q -> reference; r; r = r -> next) {
	    p = r -> ref;
	    if (p -> mark == Mark || Object_outside_window) continue;
	    p -> mark = Mark;
	    (*funct) (p);
	}
	for (p = Q -> object; p; p = p -> next) {
	    if (p -> mark == Mark || Object_outside_window) continue;
	    p -> mark = Mark;
	    (*funct) (p);
	}
    }
    else { /* no LeafNode */
	if (Q -> Uleft ) q_search (Q -> Uleft);
	if (Q -> Uright) q_search (Q -> Uright);
	if (Q -> Lright) q_search (Q -> Lright);
	if (Q -> Lleft ) q_search (Q -> Lleft);
    }
}

static void lim_q_search (qtree_t *Q)
{
    register struct obj_node *p;

    if (LeafNode) {
	for (p = Q -> object; p; p = p -> next) {
	    if (p -> mark == Mark) continue;
	    p -> mark = Mark;
	    (*funct) (p);
	}
    }
    else { /* no LeafNode */
	if (Q -> Uleft ) lim_q_search (Q -> Uleft);
	if (Q -> Uright) lim_q_search (Q -> Uright);
	if (Q -> Lright) lim_q_search (Q -> Lright);
	if (Q -> Lleft ) lim_q_search (Q -> Lleft);
    }
}

static struct found_list *pre_list;

/*
** Search a specified region for elements
** intersecting "srch" and of the same layer.
** INPUT: a pointer to a quad tree node and the search window.
** OUT: a list of pointers to trapezoids in the search window.
*/
struct found_list * quick_search (qtree_t *Q, struct obj_node *srch)
{
    pre_list = NULL;
    if (Q && srch) {
	do {
	    Q_xl = srch -> ll_x1;
	    Q_xr = srch -> ll_x2;
	    Q_yb = srch -> ll_y1;
	    Q_yt = srch -> ll_y2;
	    presearch (Q);
	} while ((srch = srch -> next));
	/*
	** Unmark of found trapezoids normally not needed,
	** because they are afterwards deleted!
	*/
    }
    return (pre_list);
}

/*
** Search for intersecting objects.
** Found objects are given an unique mark and put one time in pre_list.
*/
static void presearch (qtree_t *Q)
{
    register struct obj_node *p;
    register struct ref_node *r;
    register struct found_list *m;

    if (Quad_outside_window) return;

    if (LeafNode) {
	for (r = Q -> reference; r; r = r -> next) {
	    p = r -> ref;
	    if (p -> mark == 1 || Object_outside_window) continue;
	    p -> mark = 1; /* unique value for this strategy */
	    MALLOC (m, struct found_list);
	    m -> ptrap = p;
	    m -> next = pre_list;
	    pre_list = m;
	}
	for (p = Q -> object; p; p = p -> next) {
	    if (p -> mark == 1 || Object_outside_window) continue;
	    p -> mark = 1; /* unique value for this strategy */
	    MALLOC (m, struct found_list);
	    m -> ptrap = p;
	    m -> next = pre_list;
	    pre_list = m;
	}
    }
    else { /* no LeafNode */
	if (Q -> Uleft ) presearch (Q -> Uleft);
	if (Q -> Lleft ) presearch (Q -> Lleft);
	if (Q -> Uright) presearch (Q -> Uright);
	if (Q -> Lright) presearch (Q -> Lright);
    }
}
