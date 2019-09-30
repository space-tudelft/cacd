/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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

#include "src/makevln/incl.h"

struct group_list {
    struct group_tree  *group;
    struct group_list  *next;
};

/* Find the root node of the group_tree from which the
** group is part. Collaps path to root node, and return
** pointer to root node.
*/
struct group_tree *fdgrp_ptr (struct group_tree *group)
{
    struct group_list  *head = NULL;
    struct group_list  *tmp;

    while (group -> parent && group -> parent -> parent) {
	ALLOCPTR (tmp, group_list);
	tmp -> group = group;
	tmp -> next = head;
	head = tmp;
	group = group -> parent;
    }

    if (group -> parent)
	group = group -> parent;

    while (head) {
	head -> group -> parent = group;
	tmp = head;
	head = head -> next;
	FREE (tmp);
    }

    return (group);
}

/* Find the root node of the group_tree from which the
** group is part. Collaps path to root node, and return
** name of root node.
*/
int fdgrp_name (struct group_tree *group)
{
    struct group_list  *head = NULL;
    struct group_list  *tmp;

    while (group -> parent && group -> parent -> parent) {
	ALLOCPTR (tmp, group_list);
	tmp -> group = group;
	tmp -> next = head;
	head = tmp;
	group = group -> parent;
    }

    if (group -> parent)
	group = group -> parent;

    while (head) {
	head -> group -> parent = group;
	tmp = head;
	head = head -> next;
	FREE (tmp);
    }

    return (group -> tree_count);
}
