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

/*
** This function tries to unlink the (2ndary) stream
** of the cell with the specified key.
** -1 is returned by error condition.
*/

int dmUnlink (DM_CELL *key, char *stream)
{
    char path[MAXLINE];

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "keyno: %d, stream: %s\n", key -> keyno, stream);
#endif

    if (_dmCh_key (key) != 0) {
	dmError2 ("dmUnlink: stream", stream);
	return (-1);
    }

    _dmSprintf (path, "%s/%s/%s/%s", key -> dmproject -> dmpath, key -> view, key -> cell, stream);

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "path: %s\n", path);
#endif
    if (unlink (path)) {
	dmerrno = DME_SYS;
	return (-1);
    }
    return (0);
}
