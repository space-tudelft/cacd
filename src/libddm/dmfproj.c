/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van der Hoeven
 *	P. van der Wolf
 *	P. Bingley
 *	T.G.R. van Leuken
 *	T. Vogel
 *	F. Beeftink
 *	M. Grueter
 *	E.F. Matthijssen
 *	G.W. Sloof
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

#include "src/libddm/dmstd.h"

DM_PROJECT * dmFindProjKey (int loc_imp, char *local_name, DM_PROJECT *father_proj, char **rem_namep, const char *view)
{
    IMPCELL ** imp_p;
    DM_PROJECT * projkey;
    int view_entry;

    if ((view_entry = _dmValidView (view)) == -1) return (NULL);

    if (loc_imp == LOCAL) {
	*rem_namep = local_name;
	return (father_proj);
    }
    else if (loc_imp != IMPORTED) {
	dmerrno = DME_BADARG;
	dmError ("dmFindProjKey");
	return (NULL);
    }

    if ((imp_p = father_proj -> impcelllist[view_entry]) == NULL) {
	/* Obtain (pointer to) imported celllist. */
        if ((imp_p = (IMPCELL **) dmGetMetaDesignData (IMPORTEDCELLLIST, father_proj, (char*)view)) == NULL) {
	    return (NULL);
        }
    }

    /* Now scan the imported celllist that was just found. */
    /* From this list the project and the remote name can  */
    /* be decoded given the local name.			   */
    for ( ; imp_p && *imp_p; imp_p++) {
	if (strcmp (local_name, (*imp_p) -> alias) == 0) {
	    /* Alias found. Obtain project key. If already */
	    /* present the existing key will be returned.  */
	    if ((projkey = dmOpenProject ((*imp_p) -> dmpath, PROJ_READ)) == NULL) {
		return (NULL);
	    }
	    *rem_namep = (*imp_p) -> cellname;
	    return (projkey);
	}
    }
    dmerrno = DME_NOCELL;
    dmError ("dmFindProjKey");
    return (NULL);
}
