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

/*
** Clear the quad_tree and free all nodes.
** INPUT: a pointer to a tree node.
*/
void quad_clear (qtree_t *Q)
{
    register struct obj_node *p, *pn;
    register struct ref_node *r, *rn;

    if (!Q) return;
    /*
    ** clear linked lists on this level:
    */
    for (rn = Q -> reference; (r = rn);) {
	rn = r -> next;
	FREE (r);
    }
    for (pn = Q -> object; (p = pn);) {
	pn = p -> next;
	FREE (p);
    }

    /*
    ** clear quad nodes recursively:
    */
    if (Q -> Uleft)  quad_clear (Q -> Uleft);
    if (Q -> Lleft)  quad_clear (Q -> Lleft);
    if (Q -> Uright) quad_clear (Q -> Uright);
    if (Q -> Lright) quad_clear (Q -> Lright);

    FREE (Q); /* free this node */
}
