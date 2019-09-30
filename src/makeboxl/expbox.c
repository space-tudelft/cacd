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
** expand box/terminal to the root level
** of the mc-tree according to mc-parameters
*/
void exp_box (long Xl, long Xr, long Yb, long Yt)
{
    register long *m;
    register struct tmtx *tm;
    long xl, xr, yb, yt, tmp;

    for (tm = tm_p; tm; tm = tm -> tm_next) {

	m = tm -> mtx;
    if (m[0]) {
	xl = m[0] * Xl + m[2];
	xr = m[0] * Xr + m[2];
	yb = m[4] * Yb + m[5];
	yt = m[4] * Yt + m[5];
    }
    else {
	xl = m[1] * Yb + m[2];
	xr = m[1] * Yt + m[2];
	yb = m[3] * Xl + m[5];
	yt = m[3] * Xr + m[5];
    }
	if (xl > xr) { tmp = xl; xl = xr; xr = tmp; }
	if (yb > yt) { tmp = yb; yb = yt; yt = tmp; }

	if (part_exp) {
	    if (xr <= exp_reg[0] || xl >= exp_reg[1]
	    ||  yt <= exp_reg[2] || yb >= exp_reg[3]) {
	    /*
	    ** the box-coordinates have no overlap
	    ** with the expansion region
	    */
		if (t_mask_no) ++term_no;
		continue;
	    }
	}

	gboxlay.xl = xl;
	gboxlay.xr = xr;
	gboxlay.yb = yb;
	gboxlay.yt = yt;

	if (tm -> allow_all || (mask_no < 64 && (tm -> allowmasks & (1LL<<mask_no)))) {
	    if (!fp_bxx[mask_no]) open_bxx (mask_no, mask_no);
	    dmPutDesignData (fp_bxx[mask_no], GEO_BOXLAY);
	    ++no_bxx[mask_no];
	}

	if (t_mask_no) { /* ROOT cell */
	    gboxlay.chk_type = term_no++;
	    if (!fp_bxx[t_mask_no]) open_bxx (mask_no, t_mask_no);
	    dmPutDesignData (fp_bxx[t_mask_no], GEO_BOXLAY);
	    ++no_bxx[t_mask_no];
	    gboxlay.chk_type = 0;
	}
    }
}
