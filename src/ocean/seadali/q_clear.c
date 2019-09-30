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
** Clear the quad_tree and free all nodes.
** INPUT: a pointer to a tree node.
*/
void quad_clear (qtree_t *Q)
{
    struct obj_node *p, *tp;
    struct ref_node *r, *tr;

    if (!Q) return;

    /* clear linked lists on this level */

    for (r = Q -> reference; r; r = tr) { tr = r -> next; FREE (r); }
    for (p = Q -> object   ; p; p = tp) { tp = p -> next; FREE (p); }

    /* clear quad nodes recursively */
    if (Q -> Uleft ) quad_clear (Q -> Uleft);
    if (Q -> Lleft ) quad_clear (Q -> Lleft);
    if (Q -> Uright) quad_clear (Q -> Uright);
    if (Q -> Lright) quad_clear (Q -> Lright);

    /* free this node */
    FREE (Q);
}
