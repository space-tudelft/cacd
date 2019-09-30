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

/* patrick: added interrupt capability */
extern int interrupt_flag;

static struct obj_node *window;	/* search range */
static void (*funct) ();
static void fast_search  (qtree_t *Q);
static void lim_q_search (qtree_t *Q);
static void lim_unmark   (qtree_t *Q);
static void notice (struct obj_node *p, struct obj_node *srch);
static void presearch (qtree_t *Q, struct obj_node *srch);
static void q_search  (qtree_t *Q, short limited);
static int  q_touches_region (struct obj_node *p);
static void q_unmark  (qtree_t *Q, short limited);
static struct found_list *search_list (void);

/*
** Search a specified area for trapezoids.
** INPUT: a pointer to a quad tree node and the search window and
** a pointer to function that is to be executed for found trapezoids.
** OUTPUT: a list of pointers to trapezoids intersecting the search window.
*/
void quad_search (qtree_t *Q, struct obj_node *srch, void (*function) ())
{
    if (!Q) return;

    window = srch;
    funct = function;

    if (srch -> ll_x1 < Q -> quadrant[0] &&
	srch -> ll_y1 < Q -> quadrant[1] &&
	srch -> ll_x2 > Q -> quadrant[2] &&
	srch -> ll_y2 > Q -> quadrant[3]) {
	/*
	** display area covers complete quad-tree: do a fast search
	*/
	fast_search (Q);
	return;
    }

    q_unmark (Q, FALSE); /* unmark the quad tree */
    q_search (Q, FALSE); /* search the quad tree */
}

/*
** Unmark the trapezoids intersecting the search window.
** INPUT: a pointer to a quad tree node and a flag indicating
** that a limited search must be accomplished.
** The search window is a global variable.
*/
static void q_unmark (qtree_t *Q, short limited)
{
    struct obj_node *p;
    struct ref_node *r;

    if (!Q) return;

    if (!limited) {
	/*
	** if current quadrant and desired range don't intersect: return
	*/
	if (Q -> quadrant[0] > window -> ll_x2 ||
	    Q -> quadrant[1] > window -> ll_y2 ||
	    Q -> quadrant[2] < window -> ll_x1 ||
	    Q -> quadrant[3] < window -> ll_y1) return;

	if (window -> ll_x1 < Q -> quadrant[0] &&
	    window -> ll_y1 < Q -> quadrant[1] &&
	    window -> ll_x2 > Q -> quadrant[2] &&
	    window -> ll_y2 > Q -> quadrant[3]) limited = TRUE;
    }

    /*
    ** check for object references intersecting the desired quadrant
    */
    if (limited) {
	lim_unmark (Q);
	return;
    }

    for (r = Q -> reference; r; r = r -> next) r -> ref -> mark = 0;

    /*
    ** check for objects intersecting the desired quadrant in this quad
    */
    for (p = Q -> object; p; p = p -> next) p -> mark = 0;

    /*
    ** unmark each quadrant with a new defining region
    */
    if (Q -> Uleft ) q_unmark (Q -> Uleft, limited);
    if (Q -> Uright) q_unmark (Q -> Uright, limited);
    if (Q -> Lright) q_unmark (Q -> Lright, limited);
    if (Q -> Lleft ) q_unmark (Q -> Lleft, limited);
}

static void lim_unmark (qtree_t *Q)
{
    struct obj_node *p;

    if (!Q) return;

    /*
    ** if current quadrant and desired range don't intersect: return
    */
    if (Q -> quadrant[0] > window -> ll_x2 ||
	Q -> quadrant[1] > window -> ll_y2 ||
	Q -> quadrant[2] < window -> ll_x1 ||
	Q -> quadrant[3] < window -> ll_y1) return;

    /*
    ** check for objects intersecting the desired quadrant in this quad
    */
    for (p = Q -> object; p; p = p -> next) p -> mark = 0;

    /*
    ** unmark each quadrant with a new defining region
    */
    if (Q -> Uleft ) lim_unmark (Q -> Uleft);
    if (Q -> Uright) lim_unmark (Q -> Uright);
    if (Q -> Lright) lim_unmark (Q -> Lright);
    if (Q -> Lleft ) lim_unmark (Q -> Lleft);
}

/*
** Recursively search the quad tree.
** INPUT: a pointer to a quad tree node and a flag
** indicating if a limited search must be accomplished.
** The search window is a global variable.
*/
static void q_search (qtree_t *Q, short limited)
{
    struct obj_node *p;
    struct ref_node *r;

    if (!Q) return;
    if (stop_drawing() == TRUE) return;

    if (!limited) {
	/*
	** if current quadrant and desired range don't intersect: return
	*/
	if (Q -> quadrant[0] > window -> ll_x2 ||
	    Q -> quadrant[1] > window -> ll_y2 ||
	    Q -> quadrant[2] < window -> ll_x1 ||
	    Q -> quadrant[3] < window -> ll_y1) return;

	if (window -> ll_x1 < Q -> quadrant[0] &&
	    window -> ll_y1 < Q -> quadrant[1] &&
	    window -> ll_x2 > Q -> quadrant[2] &&
	    window -> ll_y2 > Q -> quadrant[3]) limited = TRUE;
    }

    /*
    ** does search window cover the sub quad tree
    */
    if (limited) {
	lim_q_search (Q);
	return;
    }

    for (r = Q -> reference; r && interrupt_flag != TRUE; r = r -> next)
	if (q_touches_region (r -> ref)) (*funct) (r -> ref);

    /*
    ** check for nodes intersecting the desired region in this quad
    */
    for (p = Q -> object; p && interrupt_flag != TRUE ; p = p -> next)
	if (q_touches_region (p)) (*funct) (p);

    if (Q -> Uleft ) q_search (Q -> Uleft, limited);
    if (Q -> Uright) q_search (Q -> Uright, limited);
    if (Q -> Lright) q_search (Q -> Lright, limited);
    if (Q -> Lleft ) q_search (Q -> Lleft, limited);
}

static void lim_q_search (qtree_t *Q) /* quad tree element */
{
    struct obj_node *p;

    if (!Q) return;
    if (stop_drawing() == TRUE) return;

    /*
    ** if current quadrant and desired range don't intersect: return
    */
    if (Q -> quadrant[0] > window -> ll_x2 ||
	Q -> quadrant[1] > window -> ll_y2 ||
	Q -> quadrant[2] < window -> ll_x1 ||
	Q -> quadrant[3] < window -> ll_y1) return;

    /*
    ** check for nodes intersecting the desired region in this quad
    */
    for (p = Q -> object; p && interrupt_flag != TRUE; p = p -> next)
	if (q_touches_region (p)) (*funct) (p);

    if (Q -> Uleft ) lim_q_search (Q -> Uleft);
    if (Q -> Uright) lim_q_search (Q -> Uright);
    if (Q -> Lright) lim_q_search (Q -> Lright);
    if (Q -> Lleft ) lim_q_search (Q -> Lleft);
}

static void fast_search (qtree_t *Q) /* quad tree element */
{
    struct obj_node *p;

    if (!Q) return;
    if (stop_drawing() == TRUE) return; /* PATRICK: added interrupt.. */
    /*
    ** check for nodes intersecting the desired region in this quad
    */
    for (p = Q -> object; p && interrupt_flag != TRUE; p = p -> next) (*funct) (p);

    if (Q -> Uleft ) fast_search (Q -> Uleft);
    if (Q -> Uright) fast_search (Q -> Uright);
    if (Q -> Lright) fast_search (Q -> Lright);
    if (Q -> Lleft ) fast_search (Q -> Lleft);
}

/*
** Test whether a given item p intersects the search range.
** If so then mark the object.
** INPUT: a pointer to a trapezoid.
*/
static int q_touches_region (struct obj_node *p)
{
    if (p -> mark == 1) return (FALSE);

    if (p -> ll_x1 >= window -> ll_x2 ||
	p -> ll_y1 >= window -> ll_y2 ||
	p -> ll_x2 <= window -> ll_x1 ||
	p -> ll_y2 <= window -> ll_y1) return (FALSE);

    p -> mark = 1;

    return (TRUE);
}

struct found_list *pre_list;

/*
** Search a specified region for elements
** intersecting "srch" and of the same layer.
** INPUT: a pointer to a quad tree node and the search window.
** OUTPUT: a list of pointers to trapezoids
** intersecting the search window.
*/
struct found_list * quick_search (qtree_t *Q, struct obj_node *srch)
{
    pre_list = NULL;
    presearch (Q, srch);
    return (search_list ());
}

/*
** Unmark the intersecting objects.
** INPUT: a pointer to a quad tree node and the search window.
*/
static void presearch (qtree_t *Q, struct obj_node *srch)
{
    struct obj_node *p;
    struct ref_node *r;

    if (!Q) return;

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
    for (r = Q -> reference; r; r = r -> next) notice (r -> ref, srch);

    /*
    ** check for objects intersecting the desired quadrant in this quad
    */
    for (p = Q -> object; p; p = p -> next) notice (p, srch);

    if (Q -> Ncount != -1) return; /* this is a leaf node */

    /*
    ** presearch each quadrant recursively
    */
    if (Q -> Uleft ) presearch (Q -> Uleft, srch);
    if (Q -> Lleft ) presearch (Q -> Lleft, srch);
    if (Q -> Uright) presearch (Q -> Uright, srch);
    if (Q -> Lright) presearch (Q -> Lright, srch);
}

/*
** Search found_list for unmarked objects.
** Objects found are marked.
*/
static struct found_list * search_list ()
{
    struct found_list *m;
    struct found_list *next_m;
    struct found_list *list;
    struct found_list *found;

    found = NULL;

/* Patrick & Paul: solved bug with freeing */
    for (m = pre_list; m; m = next_m) {
	if (!m -> ptrap -> mark) {
	    MALLOC (list, struct found_list);
	    list -> ptrap = m -> ptrap;
	    list -> next = found;
	    found = list;
	    m -> ptrap -> mark = 1;
	}
        next_m = m -> next;
	FREE (m);
    }
    return (found);
}

/*
** If p intersects window srch then add a pointer to p to found_list.
** INPUT: a pointer to a trapezoid and the search window.
*/
static void notice (struct obj_node *p, struct obj_node *srch)
{
    struct found_list *m;

    if (p -> ll_x1 > srch -> ll_x2 || p -> ll_x2 < srch -> ll_x1 ||
	p -> ll_y1 > srch -> ll_y2 || p -> ll_y2 < srch -> ll_y1) return;

    p -> mark = 0;

    MALLOC (m, struct found_list);
    m -> next = pre_list;
    m -> ptrap = p;
    pre_list = m;
}
