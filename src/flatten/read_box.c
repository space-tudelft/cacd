/*
 * ISC License
 *
 * Copyright (C) 1994-2018 by
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

#include "src/flatten/extern.h"

static int do_term;
static int last_nr;
static int nr;

static void exp_box (long Xl, long Xr, long Yb, long Yt, long Dx, long Nx, long Dy, long Ny);

/*
** read the boxes of the cell
*/
void read_box ()
{
    DM_STREAM *fp = dmOpenStream (cellkey, "box", "r");

    while (dmGetDesignData (fp, GEO_BOX) > 0) {
	exp_box (gbox.xl, gbox.xr, gbox.yb, gbox.yt,
		 gbox.dx, gbox.nx, gbox.dy, gbox.ny);
    }

    dmCloseStream (fp, COMPLETE);
}

/*
** read the terminals of the cell
*/
void read_term ()
{
    DM_STREAM *fp = dmOpenStream (cellkey, "term", "r");

    do_term = tflag;
    while (dmGetDesignData (fp, GEO_TERM) > 0) {
    /*
    ** write also a box to the interconnect mask
    ** corresponding with the terminal mask
    */
	if (!tflag && (gterm.xl == gterm.xr || gterm.yb == gterm.yt)) continue;
	nr = last_nr;
	gbox.layer_no = gterm.layer_no;
	exp_box (gterm.xl, gterm.xr, gterm.yb, gterm.yt,
		 gterm.dx, gterm.nx, gterm.dy, gterm.ny);
    }
    do_term = 0;
    last_nr = nr;

    dmCloseStream (fp, COMPLETE);
}

/*
** expand box/terminal to the root level
** of the mc-tree according to mc-parameters
*/
static void exp_box (long Xl, long Xr, long Yb, long Yt, long Dx, long Nx, long Dy, long Ny)
{
    register long *m;
    register struct tmtx *tm;
    char postfix[20];
    long xl, xr, yb, yt, dx, nx, dy, ny, tmp;
    int  second = 0;
    int  add, tlen = do_term ? strlen (gterm.term_name) : 0;

    for (tm = tm_p; tm; tm = tm -> tm_next) {
again:
	m = tm -> mtx;
	xl = m[0] * Xl + m[1] * Yb + m[2];
	yb = m[3] * Xl + m[4] * Yb + m[5];
	xr = m[0] * Xr + m[1] * Yt + m[2];
	yt = m[3] * Xr + m[4] * Yt + m[5];

	if (xl > xr) { tmp = xl; xl = xr; xr = tmp; }
	if (yb > yt) { tmp = yb; yb = yt; yt = tmp; }

	dx = dy = nx = ny = 0;
	if (Nx) {
	    if (m[0]) { nx = Nx; dx = m[0] * Dx; }
	    else      { ny = Nx; dy = m[3] * Dx; }
	}
	if (Ny) {
	    if (m[4]) { ny = Ny; dy = m[4] * Dy; }
	    else      { nx = Ny; dx = m[1] * Dy; }
	}

	if (do_term) {
	    gterm.term_name[tlen] = 0;
	    add = 1;
	    if (pflag) {
		if (append_tree (gterm.term_name, &tnam_tree))
		    PE "warning: %s: sub-cell terminal already used, adding postfix\n", gterm.term_name);
		else add = 0;
	    }
	    if (add) {
		sprintf (postfix, "_sub%d", ++nr);
		if (tlen + strlen (postfix) > dm_maxname) {
		    PE "%s: too long terminal name, can't add postfix\n", gterm.term_name);
		}
		else strcpy (gterm.term_name+tlen, postfix);
		if (append_tree (gterm.term_name, &tnam_tree)) {
		    PE "warning: %s: sub-cell terminal already used\n", gterm.term_name);
		}
	    }
	}
	if (ldmfile) {
	    if (do_term) {
		PO "term %s %ld %ld %ld %ld %s", process -> mask_name[gbox.layer_no], xl, xr, yb, yt, gterm.term_name);
	    }
	    else {
		PO "box %s %ld %ld %ld %ld", process -> mask_name[gbox.layer_no], xl, xr, yb, yt);
	    }
	    if (nx) PO " cx %ld %ld", dx, nx);
	    if (ny) PO " cy %ld %ld", dy, ny);
	    PO "\n");
	}
	else {
	    if (do_term) {
		gterm.xl = xl; gterm.xr = xr; gterm.yb = yb; gterm.yt = yt;
		gterm.dx = dx; gterm.nx = nx;
		gterm.dy = dy; gterm.ny = ny;
	    }
	    else {
		gbox.xl = xl; gbox.xr = xr; gbox.yb = yb; gbox.yt = yt;
		gbox.dx = dx; gbox.nx = nx;
		gbox.dy = dy; gbox.ny = ny;
	    }

	    if (nx) {
		if ((tmp = nx * dx) < 0)
		    xl += tmp;
		else
		    xr += tmp;
	    }
	    if (ny) {
		if ((tmp = ny * dy) < 0)
		    yb += tmp;
		else
		    yt += tmp;
	    }
	    if (do_term) {
		gterm.bxl = xl; gterm.bxr = xr; gterm.byb = yb; gterm.byt = yt;
	    }
	    else {
		gbox.bxl = xl; gbox.bxr = xr; gbox.byb = yb; gbox.byt = yt;
	    }

	    if (xl < elbb_xl) elbb_xl = xl;
	    if (xr > elbb_xr) elbb_xr = xr;
	    if (yb < elbb_yb) elbb_yb = yb;
	    if (yt > elbb_yt) elbb_yt = yt;

	    if (do_term)
		dmPutDesignData (fp_term, GEO_TERM);
	    else
		dmPutDesignData (fp_box, GEO_BOX);
	}
    }
    if (!second++ && (tm = tm_s)) goto again;
}
