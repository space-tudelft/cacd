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
#include <stdarg.h>

char *dmGetMetaDesignData (int reqid, DM_PROJECT *projectid, ...)
{
    char *cell, *view, *view2, *file_name;
    va_list ap;

    va_start (ap, projectid);

    switch (reqid) {
	case IMPORTEDCELLLIST:
	    view = va_arg (ap, char *);
	    return ((char *) _dmImportedCelllist (projectid, view));
	case CELLEQUIVALENCE:
	    cell = va_arg (ap, char *);
	    view = va_arg (ap, char *);
	    view2 = va_arg (ap, char *);
	    return ((char *) _dmCellEquivalence (projectid, cell, view, view2));
	case PROCESS:
	    return ((char *) _dmGetProcess (projectid));
	case CELLLIST:
	    view = va_arg (ap, char *);
	    return ((char *) _dmCelllist (projectid, view));
	case PROJECTLIST:
	    return ((char *) _dmProjectlist (projectid));
	case PROCPATH:
	    file_name = va_arg (ap, char *);
	    return ((char *) _dmGetProcPath (projectid, file_name));
	case CELLISROOT:
	    cell = va_arg (ap, char *);
	    view = va_arg (ap, char *);
	    return ((char *)(long) _dmCellIsRoot (projectid, cell, view));
	case FATHERCELL:
	    cell = va_arg (ap, char *);
	    view = va_arg (ap, char *);
	    return ((char *) _dmFatherCell (projectid, cell, view));
    }
    return (0);
}
