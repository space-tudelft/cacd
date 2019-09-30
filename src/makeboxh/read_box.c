/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#include "src/makeboxh/extern.h"

/*
** read the boxes of the cell
*/
void read_box (struct mc_elmt *pmc)
{
    DM_STREAM *fp_box;
    long    xl, xr, yb, yt;
    long    i, j;

    fp_box = dmOpenStream (cellkey, "box", "r");

    while (dmGetDesignData (fp_box, GEO_BOX) > 0) {

	mask_no = gbox.layer_no;

	xl = gbox.xl;
	xr = gbox.xr;
	for (i = 0;;) {
	    yb = gbox.yb;
	    yt = gbox.yt;
	    for (j = 0;;) {
		exp_box (pmc, xl, xr, yb, yt);
		if (++j > gbox.ny) break;
		yb += gbox.dy;
		yt += gbox.dy;
	    }
	    if (++i > gbox.nx) break;
	    xl += gbox.dx;
	    xr += gbox.dx;
	}
    }

    dmCloseStream (fp_box, COMPLETE);
}
