/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

#include "src/makeboxl/extern.h"

/*
** read the boxes of the cell
*/

void read_box ()
{
    DM_STREAM *fp;
    long xl, xr, yb, yt;
    register long i, j;

    fp = dmOpenStream (cellkey, "box", "r");

    while (dmGetDesignData (fp, GEO_BOX) > 0) {

	mask_no = gbox.layer_no;

	xl = samples * gbox.xl;
	xr = samples * gbox.xr;
	gbox.yb *= samples;
	gbox.yt *= samples;
	gbox.dx *= samples;
	gbox.dy *= samples;

	for (i = 0;;) {
	    yb = gbox.yb;
	    yt = gbox.yt;
	    for (j = 0;;) {
		exp_box (xl, xr, yb, yt);
		if (++j > gbox.ny) break;
		yb += gbox.dy;
		yt += gbox.dy;
	    }
	    if (++i > gbox.nx) break;
	    xl += gbox.dx;
	    xr += gbox.dx;
	}
    }

    dmCloseStream (fp, COMPLETE);
}
