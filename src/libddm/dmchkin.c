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

int dmCheckIn (DM_CELL *key, int mode)
{
    char path[MAXLINE];
    FILE *fp;

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "cell: '%s', keyno: %d mode: %d\n", key -> cell, key -> keyno, mode);
#endif

    if (_dmCh_key (key) != 0) {
	dmError ("dmCheckIn");
	return (-1);
    }

    dmCloseCellStreams (key, mode);

    switch (mode) {

	case CONTINUE:
	case COMPLETE:
	    if (key -> mode & INT_CREATE) {
		_dmSprintf (path, "%s/%s/celllist", key -> dmproject -> dmpath, key -> view);
		if (!(fp = fopen (path, "a"))) {
		    _dmFatal ("dmCheckIn: cannot update celllist for cell: %s", key -> cell, "");
		}
		fprintf (fp, "%s\n", key -> cell);
		fclose (fp);
	    }
	    if (mode == CONTINUE) {
		key -> mode &= ~INT_CREATE;
	    }
	    break;

	case QUIT:
	    if (key -> mode & INT_CREATE) {
		_dmSprintf (path, "%s/%s/%s", key -> dmproject -> dmpath, key -> view, key -> cell);
                if (_dmRmDir (key -> dmproject, path)) {
		    return (-1);
		}
	    }
	    break;

	default:
	    dmerrno = DME_BADARG;
	    dmError ("dmCheckIn");
	    return (-1);
    }

    if (mode != CONTINUE) {
	_dmRm_cellkey (key);
    }
    return (0);
}
