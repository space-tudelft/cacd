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

void ini_modtree ()
{
    register char **ml;
    register IMPCELL **iml;

    mod_tree = NULL;
    if ((ml = (char **) dmGetMetaDesignData (CELLLIST, dmproject, LAYOUT)) &&
    (iml = (IMPCELL **) dmGetMetaDesignData (IMPORTEDCELLLIST, dmproject, LAYOUT))) {
	for (; *ml; ++ml) {
	    if (append_tree (*ml, &mod_tree)) {
		pr_exit (0107, 4, *ml);
	    }
	    tree_ptr -> bbox = NULL;
	    tree_ptr -> impcell = 0;
	}

	for (; *iml; ++iml) {
	    if (append_tree ((*iml) -> alias, &mod_tree)) {
		pr_exit (0107, 4, (*iml) -> alias);
	    }
	    tree_ptr -> bbox = NULL;
	    tree_ptr -> impcell = *iml;
	}
    }
}
