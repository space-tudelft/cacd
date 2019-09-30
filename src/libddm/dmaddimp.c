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

int _dmAddImportedCell (DM_PROJECT *dmproject, char *cellname, char *alias, char *view, char *dmpath)
{
    FILE   *fp;
    struct stat stat_buf;
    char    path[MAXLINE];
    int     rv;

    if ((rv = dmTestname2 (dmproject, alias)) != 0) {
	if (rv < 0) return (-1); /* dmTestname generates dmError () */
	dmerrno = DME_BADNAME; /* name too long; not accepted */
	dmError (alias);
	return (-1);
    }
    if ((rv = dmTestname (cellname)) != 0) {
	if (rv < 0) return (-1); /* dmTestname generates dmError () */
	dmerrno = DME_BADNAME; /* name too long; not accepted */
	dmError (cellname);
	return (-1);
    }

    _dmSprintf (path, "%s/%s/%s", dmpath, view, cellname);
    if (stat (path, &stat_buf) != 0) {
	dmerrno = DME_NOCELL;
	dmError ("_dmAddImportedCell");
	return (-1);
    }

    _dmSprintf (path, "%s/%s/impcelllist", dmproject -> dmpath, view);
    if (!(fp = fopen (path, "a"))) {
	dmerrno = DME_NOIMPCL;
	dmError ("_dmAddImportedCell: cannot add to imported celllist");
	return (-1);
    }

    /* format: alias, cellname, project-path */
    fprintf (fp, "%s %s %s\n", alias, cellname, dmpath);

    fclose (fp);
    return (0);
}
