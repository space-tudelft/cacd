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

char ** _dmCellEquivalence (DM_PROJECT *dmproject, char *dom_cell, char *dom_view, char *range_view)
{
    char **celll_p;

    /* This dummy function always returns an empty (null-terminated) list */
    if (!(celll_p = (char **) malloc ((unsigned) (1 * sizeof (char *))))) {
	dmerrno = DME_CORE;
	dmError ("_dmCellEquivalence");
	return (NULL);
    }
    *celll_p = NULL;
    return (celll_p);
}

int _dmAddCellEquivalence (DM_PROJECT *dmproject, char *dom_cell, char *dom_view, char *range_cell, char *range_view)
{
    /* dummy function, returns always success */
    return (0);
}
