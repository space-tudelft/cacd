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

struct group_tree *merge_groups (struct group_tree *group1, struct group_tree *group2)
{
/* Merge two groups, i.e. make one the parent of the other */

    struct group_tree  *large, *small;

    if (group1 == NULL && group2 == NULL) {
	fprintf (stderr, " Function : merge_groups \n");
	fprintf (stderr, " ??? Unidentified situation\n");
	return (NULL);
    }
    else
	if (group1 == NULL)
	    return (group2);
	else
	    if (group2 == NULL)
		return (group1);
	    else {
		group1 = fdgrp_ptr (group1);
		group2 = fdgrp_ptr (group2);
		if (group1 == group2)
		    return (group1);
		if (group1 -> tree_count <= group2 -> tree_count) {
		    large = group2;
		    small = group1;
		}
		else {
		    large = group1;
		    small = group2;
		}
		small -> parent = large;
		large -> tree_count += small -> tree_count;
		return (large);
	    }
}
