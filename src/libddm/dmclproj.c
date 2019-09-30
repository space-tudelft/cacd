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

int dmCloseProject (DM_PROJECT *dmproject, int mode)
{
    if (_dmCh_project (dmproject) != 0) {
	dmError ("dmCloseProject");
	return (-1);
    }
    if (!(mode == COMPLETE || mode == QUIT || mode == CONTINUE)) {
	dmerrno = DME_BADARG;
	dmError ("dmCloseProject");
	return (-1);
    }
#ifdef PRLOCK
    if (dmproject -> mode == PROJ_WRITE) {
	if (dmproject -> dmpath) {
	    _dmUnlockProject (dmproject -> dmpath);
	}
	else {
	    dmerrno = DME_PRLOCK;
	    dmError ("dmCloseProject");
	    return (-1);
	}
    }
#endif /* PRLOCK */
    if (mode == CONTINUE) return (dmCloseProjectContinue (dmproject));
    _dmRm_projectkey (dmproject);
    return (0);
}
