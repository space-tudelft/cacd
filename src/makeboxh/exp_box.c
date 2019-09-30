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
** expand a box statement to the root level
** of the mc-tree according to mc-parameters
*/
void exp_box (struct mc_elmt *pmc, long xl, long xr, long yb, long yt)
{
    register struct wdw *t1;
    register long i, j, tdy;

    if (!pmc -> parent) { /* root level */
	if (part_exp == TRUE) {
	    if (xr <= exp_reg[0] || xl >= exp_reg[1]
	    ||  yt <= exp_reg[2] || yb >= exp_reg[3]) {
	    /*
	    ** the box-coordinates have no overlap
	    ** with the expansion region
	    */
		return;
	    }
	}

	if (level == 1) { /* root level */
	/*
	** add wdw to the active region list with
	** max overlap size
	*/
	    add_arl (xl - maxol, xr + maxol,
			    yb - maxol, yt + maxol);
	}
	else {
	/*
	** walk through the act reg list
	*/
	    for (t1 = *arl_ptr; t1; t1 = t1 -> next) {
		if (xr > t1 -> wdw[0] && xl < t1 -> wdw[1]
		&&  yt > t1 -> wdw[2] && yb < t1 -> wdw[3]) {
		/*
		** the box-coordinates have an overlap
		** with a wdw in the active region list
		*/
		    goto prt;
		}
	    }
	    /* the box-coordinates have no overlap
	    ** with a wdw in the active region list
	    */
	    return;
	}
prt:
	gboxlay.xl = xl;
	gboxlay.xr = xr;
	gboxlay.yb = yb;
	gboxlay.yt = yt;
	gboxlay.chk_type = fchtype;
	dmPutDesignData (fp_bxx[mask_no], GEO_BOXLAY);
	++no_bxx[mask_no];
	return;
    }

    i = xl;
    xl = pmc -> mtx[0] * i + pmc -> mtx[1] * yb + pmc -> mtx[2];
    yb = pmc -> mtx[3] * i + pmc -> mtx[4] * yb + pmc -> mtx[5];
    i = xr;
    xr = pmc -> mtx[0] * i + pmc -> mtx[1] * yt + pmc -> mtx[2];
    yt = pmc -> mtx[3] * i + pmc -> mtx[4] * yt + pmc -> mtx[5];

    if (xl > xr) { i = xl; xl = xr; xr = i; }
    if (yb > yt) { i = yb; yb = yt; yt = i; }

    /* repeat box nx,ny times */

    for (i = 0;;) {
	for (tdy = j = 0;;) {
	    if (!pmc -> parent -> parent) {
		/*
		** determine final checktype
		*/
		fchtype = chtype +
			(i >= 2 ? 2 : i) + (j >= 2 ? 6 : 3 * j);
	    }
	    exp_box (pmc -> parent, xl, xr, yb + tdy, yt + tdy);
	    if (++j > pmc -> ny) break;
	    tdy += pmc -> dy;
	}
	if (++i > pmc -> nx) break;
	xl += pmc -> dx;
	xr += pmc -> dx;
    }
}
