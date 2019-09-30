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

struct mc_elmt *mct;

/*
** make an active region list for each child of pmc,
** i.e. determine the overlapping wdws of the pmc->act_regl
** and the expanded bounding boxes of the cell calls
*/
void make_actreg (struct mc_elmt *pmc)
{
    for (mct = pmc -> child; mct; mct = mct -> sibling) {
#ifdef DEBUG1
P_E "exp_wdw(mct,bbox): %08x (%4d, %4d, %4d, %4d)\n",
mct,mct->bbox[0],mct->bbox[1],mct->bbox[2],mct->bbox[3]);
#endif
	/* expand mc bbox */
	exp_wdw (pmc, mct -> bbox[0], mct -> bbox[1],
			mct -> bbox[2], mct -> bbox[3]);

#ifdef DEBUG1
pr_wdwl(mct->act_regl,"act reg list");
#endif
    }

#ifdef DEBUG1
P_E "<= make_actreg()\n");
#endif
}

/*
** expand mct mc-bbox till root level, and add overlaps
** of the act regions of the parent to mct act reg list
*/
void exp_wdw (struct mc_elmt *pmc, long xl, long xr, long yb, long yt)
{
    register struct wdw *t1;
    register long i, j, *w, tdy;
    struct wdw *el;

    if (!pmc -> parent) { /* root level */
	for (t1 = *arl_ptr; t1; t1 = t1 -> next) {
	/*
	** compare active regions with expanded bbox
	*/
	    w = t1 -> wdw;

	    if (xr > w[0] && xl < w[1]
	    &&  yt > w[2] && yb < w[3]) {
	    /*
	    ** add overlap to act reg list of mct
	    */
		ALLOCPTR (el, wdw);
		olap (xl, xr, yb, yt, w, el -> wdw);
		el -> next = mct -> act_regl;
		mct -> act_regl = el;
	    }
	}
	return;
    }

    w = pmc -> mtx;
    i = xl;
    xl = w[0] * i + w[1] * yb + w[2];
    yb = w[3] * i + w[4] * yb + w[5];
    i = xr;
    xr = w[0] * i + w[1] * yt + w[2];
    yt = w[3] * i + w[4] * yt + w[5];

    if (xl > xr) { i = xl; xl = xr; xr = i; }
    if (yb > yt) { i = yb; yb = yt; yt = i; }

    /*
    ** do it nx,ny times
    */
    for (i = 0;;) {
	for (tdy = j = 0;;) {
	    exp_wdw (pmc -> parent, xl, xr, yb + tdy, yt + tdy);
	    if (++j > pmc -> ny) break;
	    tdy += pmc -> dy;
	}
	if (++i > pmc -> nx) break;
	xl += pmc -> dx;
	xr += pmc -> dx;
    }
}
