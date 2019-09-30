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
** read the terminals of the cell
*/

int tms;

Private void parse_term_stream (struct clist *clp)
{
    DM_STREAM *fp;
    long    xl, xr, yb, yt;
    register long i, j;

    fp = dmOpenStream (cellkey, "term", "r");

    while (dmGetDesignData (fp, GEO_TERM) > 0) {

	/*
	** write also a box to the interconnect mask
	** corresponding with the terminal mask
	*/

	mask_no = gterm.layer_no;

	if (tm_p && !clp -> mc_p) { /* ROOT cell */
	    t_mask_no = process -> mask_no[mask_no];
	    gtid.term_offset = term_no;
	    strcpy (gtid.term_name, gterm.term_name);
	    gtid.t_nx = gterm.nx;
	    gtid.t_ny = gterm.ny;
	    dmPutDesignData (fp_tid, GEO_TID);
	}

	if (tms) {
	    strcpy (gspec.layer, process -> mask_name[mask_no]);
	    strcpy (gspec.name, gterm.term_name);
	}

	xl = samples * gterm.xl;
	xr = samples * gterm.xr;
	gterm.yb *= samples;
	gterm.yt *= samples;
	gterm.dx *= samples;
	gterm.dy *= samples;

	for (i = 0;;) {
	    yb = gterm.yb;
	    yt = gterm.yt;
	    for (j = 0;;) {
		if (tm_p) exp_box (xl, xr, yb, yt);
		if (tms) s_exp_box (xl, xr, yb, yt);
		if (++j > gterm.ny) break;
		yb += gterm.dy;
		yt += gterm.dy;
	    }
	    if (++i > gterm.nx) break;
	    xl += gterm.dx;
	    xr += gterm.dx;
	}
    }

    t_mask_no = 0;

    dmCloseStream (fp, COMPLETE);
}

void read_term (struct clist *clp)
{
    DM_STREAM *fp;

    tms = (tm_s && flat_mode);

    if (tms) {
	s_mode = 0;
	if (!part_exp) {
	    fp = dmOpenStream (cellkey, "info", "r");
	    dmGetDesignData (fp, GEO_INFO);
	    ginfo.bxl *= samples;
	    ginfo.bxr *= samples;
	    ginfo.byb *= samples;
	    ginfo.byt *= samples;
	    dmCloseStream (fp, COMPLETE);
	}

	strcpy (gspec.layer, "bb");
	if (cellkey -> dmproject != project) s_mode = 1;
	else strcpy (gspec.name, cellkey -> cell);

	s_exp_box (ginfo.bxl, ginfo.bxr, ginfo.byb, ginfo.byt);
	s_mode = 0;
    }

    parse_term_stream (clp);
}
