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
extern struct obj_node **PutBuf;

/*
** Delete a linked list of trapezoids from the quad tree.
** INPUT: the trapezoids and their mask layer.
*/
void del_traps (struct obj_node *del_list, int lay)
{
    struct obj_node *clip_head; /* head of list of remaining traps */
    register struct obj_node *dtrap, *itrap, *ilist;

    while ((dtrap = del_list)) {
	del_list = dtrap -> next;
	/*
	** Search for intersecting trapezoids into the quad tree
	** and put these in maximal hor. strip representation.
	** Note: Don't merge intersecting trapezoids with dtrap.
	*/
	dtrap -> next = NULL;
	ilist = insert (dtrap, lay, DELETE);
	dtrap -> next = del_list;

	clip_head = NULL;
	while ((itrap = ilist)) {
	    /* clip two trapezoids */
	    clip_head = clip (itrap, dtrap, clip_head);

	    /* yank two trapezoids, add to PutBuf */
	    PutBuf[lay] = yank_traps (itrap, dtrap, PutBuf[lay]);
	    ilist = itrap -> next;
	    FREE (itrap);
	}
	if (clip_head) add_quad (quad_root, clip_head, lay);
    }
}
