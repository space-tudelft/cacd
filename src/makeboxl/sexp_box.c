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
** expand special box/terminal to the root level
** of the mc-tree according to mc-parameters
*/
void s_exp_box (long Xl, long Xr, long Yb, long Yt)
{
    register long *m;
    register struct tmtx *tm;
    long xl, xr, yb, yt, tmp;

    for (tm = tm_s; tm; tm = tm -> tm_next) {

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
		continue;
	    }
	}

	if (s_mode) {
	    sprintf (gspec.name, "%s(%s:%s)", tm -> mc -> name,
		cellkey -> dmproject -> projectid, cellkey -> cell);
	}

	gspec.xl = xl;
	gspec.xr = xr;
	gspec.yb = yb;
	gspec.yt = yt;

	dmPutDesignData (fp_spec, GEO_SPEC);
    }
}
