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

long txl, txr, tyb, tyt;

/*
** read the cell calls
*/
void read_mc (struct mc_elmt *pmc)
{
    char *name;
    DM_PROJECT *proj;
    DM_CELL    *key;
    DM_STREAM  *fp_mc;
    DM_STREAM  *fp_info;
    struct mc_elmt  *mct1;
    struct mc_elmt **mctp;
    long    d_x, d_y, tmp;
    register int i, j;

#ifdef DEBUG1
P_E "read cell mc file: %s\n", pmc->name);
#endif
    fp_mc = dmOpenStream (cellkey, "mc", "r");

    mctp = &pmc -> child;

    while (dmGetDesignData (fp_mc, GEO_MC) > 0) {

#ifdef DEBUG
P_E "cell <inst>: %s <%s>\n",gmc.cell_name,gmc.inst_name);
#endif
#ifdef DEBUG1
P_E "mc-bbox[0-3]: %4d, %4d, %4d, %4d\n",gmc.bxl,gmc.bxr,gmc.byb,gmc.byt);
#endif

	if (part_exp == TRUE) {
#ifdef DEBUG1
P_E "=> exp_mcwdw(pmc): %08x\n",pmc);
#endif
	    if (exp_mcwdw (pmc, gmc.bxl,
		    gmc.bxr, gmc.byb, gmc.byt) == FALSE) {
	    /*
	    ** the mc-bbox-coordinates have no overlap
	    ** with the expansion region
	    */
		continue;
	    }
	}

	if (level < usr_chlev && cptr == cptrhead
	    && !gmc.imported && cellkey -> dmproject == project) {
#ifdef DEBUG
P_E "=> upd_ctree(celltree,name): %08x, %s\n", celltree, gmc.cell_name);
#endif
	    upd_ctree (celltree, gmc.cell_name);
	}

	/*
	** allocate and add a mc-elmt to mc-tree
	*/
	ALLOCPTR (mct1, mc_elmt);
	*mctp = mct1;
	mctp = &mct1 -> sibling;

	strcpy (mct1 -> name, gmc.cell_name);
	strcpy (mct1 -> inst_name, gmc.inst_name);
	mct1 -> imported = gmc.imported;
	mct1 -> bbox[0] = gmc.bxl;
	mct1 -> bbox[1] = gmc.bxr;
	mct1 -> bbox[2] = gmc.byb;
	mct1 -> bbox[3] = gmc.byt;
	mct1 -> mtx[0] = gmc.mtx[0];
	mct1 -> mtx[1] = gmc.mtx[1];
	mct1 -> mtx[2] = gmc.mtx[2];
	mct1 -> mtx[3] = gmc.mtx[3];
	mct1 -> mtx[4] = gmc.mtx[4];
	mct1 -> mtx[5] = gmc.mtx[5];
	mct1 -> dx = gmc.dx;
	mct1 -> nx = gmc.nx;
	mct1 -> dy = gmc.dy;
	mct1 -> ny = gmc.ny;
	mct1 -> parent   = pmc;
	mct1 -> child    = NULL;
	mct1 -> sibling  = NULL;
	mct1 -> act_regl = NULL;

	if (level == 1) { /* root level */
	/*
	** determine the overlap between the mc-bbox and
	** the bounding boxes of the other cell calls
	*/
	    txl = gmc.bxl - maxol;
	    tyb = gmc.byb - maxol;

	    if (gmc.nx || gmc.ny) {
		/*
		** read the bounding box of the cell
		*/
		proj = dmFindProjKey (gmc.imported, gmc.cell_name,
			    cellkey -> dmproject, &name, LAYOUT);
		key = dmCheckOut (proj, name,
			    ACTUAL, DONTCARE, LAYOUT, READONLY);
		fp_info = dmOpenStream (key, "info", "r");
		dmGetDesignData (fp_info, GEO_INFO);
		dmCloseStream (fp_info, COMPLETE);
		dmCheckIn (key, COMPLETE);

		d_x = ginfo.bxr - ginfo.bxl;
		d_y = ginfo.byt - ginfo.byb;
		tmp = d_x;
		d_x = gmc.mtx[0] * tmp + gmc.mtx[1] * d_y;
		d_y = gmc.mtx[3] * tmp + gmc.mtx[4] * d_y;
		if (d_x < 0) d_x = -d_x;
		if (d_y < 0) d_y = -d_y;

		if (gmc.dx < 0) gmc.dx = -gmc.dx;
		if (gmc.dy < 0) gmc.dy = -gmc.dy;

		txr = gmc.bxl + d_x + maxol;
		tmp = tyt = gmc.byb + d_y + maxol;

		if (gmc.ny && gmc.dy <= d_y + maxol) {
		    if (gmc.dy < d_y + maxol) {
			add_arl (txl, txr, tyb + gmc.dy, tyt);
		    }
		    if (gmc.nx == 0) goto else_part;
		    tyt = gmc.byt + maxol;
		    gmc.ny = 0;
		}
		if (gmc.nx && gmc.dx <= d_x + maxol) {
		    if (gmc.dx < d_x + maxol) {
			add_arl (txl + gmc.dx, txr, tyb, tmp);
		    }
		    if (gmc.ny == 0) goto else_part;
		    txr = gmc.bxr + maxol;
		    gmc.nx = 0;
		}

		/*
		** do it nx,ny times
		*/
		for (i = 0;;) {
		    for (j = 0;;) {

			add_mc_olap ();

			if (++j > gmc.ny) break;
			tyb += gmc.dy;
			tyt += gmc.dy;
		    }
		    if (++i > gmc.nx) break;
		    if (gmc.ny) {
			tyb = gmc.byb - maxol;
			tyt = tmp;
		    }
		    txl += gmc.dx;
		    txr += gmc.dx;
		}
	    }
	    else {
else_part:
		txr = gmc.bxr + maxol;
		tyt = gmc.byt + maxol;

		add_mc_olap ();
	    }
	}
    }

    dmCloseStream (fp_mc, COMPLETE);
#ifdef DEBUG1
P_E "<= read_mc()\n");
#endif
}

/*
** expand mc-bbox to root level
*/
int exp_mcwdw (struct mc_elmt *pmc, long xl, long xr, long yb, long yt)
{
    register long i, j, tdy;
    register long *m;

    if (!pmc -> parent) { /* root level */
#ifdef DEBUG
P_E "exp. mc-bbox: %ld, %ld, %ld, %ld\n", xl, xr, yb, yt);
#endif
	if (xr <= exp_reg[0] || xl >= exp_reg[1]
	||  yt <= exp_reg[2] || yb >= exp_reg[3]) {
#ifdef DEBUG
P_E "<= exp_mcwdw(): false, not within exp region\n");
#endif
	    return (FALSE);
	}
#ifdef DEBUG
P_E "<= exp_mcwdw(): true, within exp region\n");
#endif
	return (TRUE);
    }

    m = pmc -> mtx;
    i = xl;
    xl = m[0] * i + m[1] * yb + m[2];
    yb = m[3] * i + m[4] * yb + m[5];
    i = xr;
    xr = m[0] * i + m[1] * yt + m[2];
    yt = m[3] * i + m[4] * yt + m[5];

    if (xl > xr) { i = xl; xl = xr; xr = i; }
    if (yb > yt) { i = yb; yb = yt; yt = i; }

    /*
    ** do it nx,ny times
    */
    for (i = 0;;) {
	for (tdy = j = 0;;) {
	    if (exp_mcwdw (pmc -> parent,
		    xl, xr, yb + tdy, yt + tdy) == TRUE) {
		return (TRUE);
	    }
	    if (++j > pmc -> ny) break;
	    tdy += pmc -> dy;
	}
	if (++i > pmc -> nx) break;
	xl += pmc -> dx;
	xr += pmc -> dx;
    }

    return (FALSE);
}

/*
** search for cell call overlaps
** add cell call overlap region to act. region list
*/
void add_mc_olap ()
{
    register struct wdw *el;
    register long *w;
    long bxl, bxr, byb, byt;
    long owdw[4];

    bxl = txl + maxol;
    bxr = txr - maxol;
    byb = tyb + maxol;
    byt = tyt - maxol;

    for (el = mc_bboxl; el; el = el -> next) {
	w = el -> wdw;
	if (bxr > w[0] && bxl < w[1]
	    && byt > w[2] && byb < w[3]) {
	/*
	** add overlap region to act. region list (of pmc)
	*/
	    olap (txl, txr, tyb, tyt, w, owdw);
	    add_arl (owdw[0], owdw[1], owdw[2], owdw[3]);
	}
    }

    /*
    ** add mc-bbox (incl. overlap) to the mc bbox list
    */
    ALLOCPTR (el, wdw);
    w = el -> wdw;
    w[0] = txl;
    w[1] = txr;
    w[2] = tyb;
    w[3] = tyt;
    el -> next = mc_bboxl;
    mc_bboxl = el;
}

/*
** add new region to active region list
*/
void add_arl (long xl, long xr, long yb, long yt)
{
    register struct wdw *el;
    register long *w;

    for (el = *arl_ptr; el; el = el -> next) {
	w = el -> wdw;
	if (xr > w[0] && xl < w[1]
	    && yt > w[2] && yb < w[3]) {
	/*
	** new region has overlap with existing region
	*/
	    if (xl >= w[0] && xr <= w[1]) {
		if (yb >= w[2] && yt <= w[3]) {
		    return;
		}

		if (xl == w[0] && xr == w[1]) {
		    if (yb < w[2]) w[2] = yb;
		    if (yt > w[3]) w[3] = yt;
		    return;
		}
	    }

	    if (yb <= w[2] && yt >= w[3]) {
		if (xl <= w[0] && xr >= w[1]) {
		    w[0] = xl;
		    w[1] = xr;
		    w[2] = yb;
		    w[3] = yt;
		    return;
		}

		if (yb == w[2] && yt == w[3]) {
		    if (xl < w[0]) w[0] = xl;
		    if (xr > w[1]) w[1] = xr;
		    return;
		}
	    }
	}
    }

    ALLOCPTR (el, wdw);
    w = el -> wdw;
    w[0] = xl;
    w[1] = xr;
    w[2] = yb;
    w[3] = yt;
    el -> next = *arl_ptr;
    *arl_ptr = el;
}
