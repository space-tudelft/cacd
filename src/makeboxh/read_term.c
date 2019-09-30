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

long    txl, txr, tyb, tyt;

/*
** read the terminals of the cell
*/
void read_term (struct mc_elmt *pmc)
{
    DM_STREAM *fp_term;
    long    nx, dx, ny, dy;
    long    tdx, tdy, mcdx, mcdy;
    long    xl, xr, yb, yt;
    register long    i, j, k, l;

#ifdef DEBUG1
P_E "read cell term file: %s\n", pmc -> name);
#endif
    fp_term = dmOpenStream (cellkey, "term", "r");

    if (level <= 2) {
	gtid.term_offset = -1;
	strcpy (gtid.cell_name, pmc -> name);
	strcpy (gtid.inst_name, pmc -> inst_name);
	gtid.m_nx = pmc -> nx;
	gtid.m_ny = pmc -> ny;
	dmPutDesignData (fp_tid, GEO_TID);
    }

    while (dmGetDesignData (fp_term, GEO_TERM) > 0) {

	xl = gterm.xl;
	xr = gterm.xr;
	yb = gterm.yb;
	yt = gterm.yt;
	dx = gterm.dx;
	nx = gterm.nx;
	dy = gterm.dy;
	ny = gterm.ny;

	/*
	** write a box to the interconnect mask
	** corresponding with the terminal mask
	*/
	mask_no = gterm.layer_no;

	for (tdx = i = 0;;) {
	    for (tdy = j = 0;;) {
		exp_box (pmc, xl + tdx, xr + tdx, yb + tdy, yt + tdy);
		if (++j > ny) break;
		tdy += dy;
	    }
	    if (++i > nx) break;
	    tdx += dx;
	}

	if (level > 2) continue;

	/* determine index of term_lay from intcon_lay mask_no */
	mask_no = process -> mask_no[mask_no];

	if (level > 1) { /* not root level */
	    i = xl;
	    xl = pmc -> mtx[0] * i + pmc -> mtx[1] * yb + pmc -> mtx[2];
	    yb = pmc -> mtx[3] * i + pmc -> mtx[4] * yb + pmc -> mtx[5];
	    i = xr;
	    xr = pmc -> mtx[0] * i + pmc -> mtx[1] * yt + pmc -> mtx[2];
	    yt = pmc -> mtx[3] * i + pmc -> mtx[4] * yt + pmc -> mtx[5];

	    if (xl > xr) { i = xl; xl = xr; xr = i; }
	    if (yb > yt) { i = yb; yb = yt; yt = i; }

	    i = dx;
	    dx = pmc -> mtx[0] * i + pmc -> mtx[1] * dy;
	    dy = pmc -> mtx[3] * i + pmc -> mtx[4] * dy;
	    if (pmc -> mtx[0] == 0) { i = nx; nx = ny; ny = i; }
	}

	gtid.term_offset = term_no;
	strcpy (gtid.term_name, gterm.term_name);
	gtid.t_nx = nx;
	gtid.t_ny = ny;
	dmPutDesignData (fp_tid, GEO_TID);

	for (mcdx = i = 0; i <= pmc -> nx; ++i) {
	    for (mcdy = j = 0; j <= pmc -> ny; ++j) {
		tdx = mcdx;
		for (k = 0; k <= nx; ++k) {
		    txl = xl + tdx;
		    txr = xr + tdx;
		    tdy = mcdy;
		    for (l = 0; l <= ny; ++l) {
			tyb = yb + tdy;
			tyt = yt + tdy;
			if (part_exp == TRUE) {
			    if (txr <= exp_reg[0] || txl >= exp_reg[1]
			    ||  tyt <= exp_reg[2] || tyb >= exp_reg[3]) {
				goto nxt_term;
			    }
			}
			gtermlay.xl = txl;
			gtermlay.xr = txr;
			gtermlay.yb = tyb;
			gtermlay.yt = tyt;
			gtermlay.term_number = term_no;
			dmPutDesignData (fp_bxx[mask_no], GEO_TERMLAY);
			++no_bxx[mask_no];
nxt_term:
			++term_no;
			tdy += dy;
		    }
		    tdx += dx;
		}
		mcdy += pmc -> dy;
	    }
	    mcdx += pmc -> dx;
	}
    }

    dmCloseStream (fp_term, COMPLETE);
#ifdef DEBUG1
P_E "<= read_term()\n");
#endif
}
