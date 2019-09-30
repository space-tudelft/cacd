/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#include "src/cldm/extern.h"

int append_tree (char *name, struct name_tree **head)
{
    register int cmp;

    if ((tree_ptr = *head)) {
	cmp = strcmp (name, tree_ptr -> name);
	if (cmp > 0)
	    return (append_tree (name, &(tree_ptr -> rchild)));
	if (cmp < 0)
	    return (append_tree (name, &(tree_ptr -> lchild)));
	return (1); /* found */
    }

    ALLOC (tree_ptr, name_tree);
    strcpy (tree_ptr -> name, name);
    tree_ptr -> errflag = 0;
    tree_ptr -> rchild = tree_ptr -> lchild = NULL;
    *head = tree_ptr;
    return (0); /* not found */
}

int check_tree (char *name, struct name_tree *ptr)
{
    register int cmp;

    if (ptr) {
	cmp = strcmp (name, ptr -> name);
	if (cmp > 0)
	    return (check_tree (name, ptr -> rchild));
	if (cmp < 0)
	    return (check_tree (name, ptr -> lchild));
	tree_ptr = ptr;
	return (1); /* found */
    }
    return (0); /* not found */
}

void rm_tree (struct name_tree *ptr)
{
    if (ptr) {
	rm_tree (ptr -> lchild);
	rm_tree (ptr -> rchild);
	FREE (ptr);
    }
}

void print_tree (char *s, struct name_tree *ptr)
{
    if (ptr) {
	    print_tree (s, ptr -> rchild);
	    print_tree (s, ptr -> lchild);
	P_E "tree %s name: %s\n",s, ptr->name);
    }
}
