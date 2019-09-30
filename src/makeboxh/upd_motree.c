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

#include "src/makeboxh/extern.h"

/*
** search cell tree for cell name and update it
** if the cell is not already in it
*/
void upd_ctree (struct ctree *cell, char *name)
{
    int cmp = strcmp (name, cell -> name);

    if (cmp < 0) {
	if (cell -> lchild == NULL) {
	    cell -> lchild = upd_cptrl (name);
	}
	else {
	    upd_ctree (cell -> lchild, name);
	}
	return;
    }
    if (cmp > 0) {
	if (cell -> rchild == NULL) {
	    cell -> rchild = upd_cptrl (name);
	}
	else {
	    upd_ctree (cell -> rchild, name);
	}
    }
}

/*
** add cell to cell tree and make an entry in the
** cell-ptr-list, so that the cell will be expanded
*/
struct ctree *upd_cptrl (char *name)
{
    struct ctree *ct_tmp;
    struct cptrl *cp_tmp;

    ALLOCPTR (ct_tmp, ctree);
    strcpy (ct_tmp -> name, name);
    ct_tmp -> lchild = NULL;
    ct_tmp -> rchild = NULL;

    ALLOCPTR (cp_tmp, cptrl);
    cp_tmp -> cell = ct_tmp;
    cp_tmp -> next = NULL;
    if (cptrlast)
	cptrlast -> next = cp_tmp;
    cptrlast = cp_tmp;

    return (ct_tmp);
}
