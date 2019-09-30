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

int _dmExistCell (DM_PROJECT *dmproject, char *cell, const char *view)
{
    struct stat stat_buf;
    char    path[MAXLINE];
    int     rv;

    if ((rv = dmTestname2 (dmproject, cell)) != 0) {
	if (rv < 0) return (-1); /* dmTestname generates dmError () */
	dmerrno = DME_BADNAME; /* name too long; not accepted */
	dmError (cell);
	return (-1);
    }

    if (_dmValidView (view) == -1) return (-1);

    _dmSprintf (path, "%s/%s/%s", dmproject -> dmpath, view, cell);
    if (stat (path, &stat_buf) == 0) {
	dmerrno = DME_EXIST;
	return (1);
    }

    dmerrno = DME_NOCELL;
    return (0);
}

int _dmExistView (DM_PROJECT *dmproject, const char *view)
{
    struct stat stat_buf;
    char    path[MAXLINE];

    if (_dmValidView (view) == -1) return (-1);

    _dmSprintf (path, "%s/%s", dmproject -> dmpath, view);
    if (stat (path, &stat_buf) == 0) {
	dmerrno = DME_EXIST;
	return (1);
    }

    dmerrno = DME_NOVIEW;
    return (0);
}

int _dmValidView (const char *view)
{
    int view_entry;

    for (view_entry = 0; view_entry < DM_NOVIEWS; view_entry++) {
	if (strcmp (view, dmviews[view_entry]) == 0)
	    return (view_entry);
    }
    dmerrno = DME_BADVIEW;
    dmError ((char*)view);
    return (-1);
}
