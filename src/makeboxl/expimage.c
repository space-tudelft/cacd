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

long pmc_dx, pmc_dy, pmc_tx, pmc_ty;
int  pmc_nx, pmc_ny;

/*
** Read the boxes of the image cell
** and write them to the "bxx" files.
*/
void write_image_bxx ()
{
    DM_STREAM *fp, *fp2;
    struct mc_elmt *pmc;
    long dx, dy;
    int  nx, ny;
    register int i, j, k;

    dx = 0; /* suppres uninitialized warning */
    dy = 0; /* suppres uninitialized warning */
    k  = 0; /* suppres uninitialized warning */

    pmc = clp_image -> mc_p;
    pmc_tx = pmc -> mtx[2];
    pmc_ty = pmc -> mtx[5];
    pmc_dx = pmc -> dx;
    pmc_dy = pmc -> dy;
    pmc_nx = pmc -> nx;
    pmc_ny = pmc -> ny;

    if (pmc -> mtx[0] != 1 || pmc -> mtx[1] != 0 ||
	pmc -> mtx[3] != 0 || pmc -> mtx[4] != 1 ||
	pmc_dx < 0 || pmc_dy < 0) errexit (17, clp_image -> name);

    if (part_exp) {
	fp = dmOpenStream (cellkey, "info", "r");
	dmGetDesignData (fp, GEO_INFO);
	dx = ginfo.bxr + pmc_tx;
	dy = ginfo.byt + pmc_ty;
	dmCloseStream (fp, COMPLETE);
	while (pmc_nx && dx <= exp_reg[0]) {
	    --pmc_nx; dx += pmc_dx; pmc_tx += pmc_dx;
	}
	if (pmc_nx) { pmc_nx = 0;
	    dx -= (ginfo.bxr - ginfo.bxl);
	    while ((dx += pmc_dx) < exp_reg[1]) ++pmc_nx;
	}
	while (pmc_ny && dy <= exp_reg[2]) {
	    --pmc_ny; dy += pmc_dy; pmc_ty += pmc_dy;
	}
	if (pmc_ny) { pmc_ny = 0;
	    dy -= (ginfo.byt - ginfo.byb);
	    while ((dy += pmc_dy) < exp_reg[3]) ++pmc_ny;
	}
    }

    fp = dmOpenStream (cellkey, "box", "r");

    while (dmGetDesignData (fp, GEO_BOX) > 0) {

	mask_no = gbox.layer_no;
	if (!clp_image -> all_allowed && !(mask_no < 64 && (clp_image -> freemasks_bits & (1LL<<mask_no)))) continue;

	if (!(fp2 = fp_bxx[mask_no])) fp2 = open_bxx (mask_no, mask_no);

	if ((i = pmc_nx) > 0 && (gbox.xr - gbox.xl) >= pmc_dx) {
	    gbox.xr += pmc_nx * pmc_dx;
	    i = 0;
	}
	gboxlay.xl = gbox.xl + pmc_tx;
	gboxlay.xr = gbox.xr + pmc_tx;
	if (i) {
	    dx = pmc_dx;
	    if (gbox.nx) dx -= gbox.nx * gbox.dx;
	}
	if (pmc_ny) {
	    dy = pmc_dy;
	    if (gbox.ny) {
		dy -= gbox.ny * gbox.dy;
		if ((k = (dy == 0))) dy = gbox.dy;
	    }
	}
	for (;;) {
	    for (nx = gbox.nx;;) {
		gboxlay.yb = gbox.yb + pmc_ty;
		gboxlay.yt = gbox.yt + pmc_ty;
		for (j = pmc_ny;;) {
		    for (ny = gbox.ny;;) {
			dmPutDesignData (fp2, GEO_BOXLAY);
			++no_bxx[mask_no];
			if (--ny < 0) break;
			if (j && k && ny == 0) break;
			gboxlay.yb += gbox.dy;
			gboxlay.yt += gbox.dy;
		    }
		    if (--j < 0) break;
		    gboxlay.yb += dy;
		    gboxlay.yt += dy;
		}
		if (--nx < 0) break;
		gboxlay.xl += gbox.dx;
		gboxlay.xr += gbox.dx;
	    }
	    if (--i < 0) break;
	    gboxlay.xl += dx;
	    gboxlay.xr += dx;
	}
    }

    dmCloseStream (fp, COMPLETE);
}

/*
** Read the not orth. elements of the image cell
** and write them to the "nxx" files.
*/
void write_image_nxx ()
{
    DM_STREAM *fp, *fp2;
    double bdx, bdy;
    long tdx, tdy;
    int  nx, ny, no_xy_pts;
    register int i, j, k;

    fp = dmOpenStream (cellkey, "nor", "r");

    while (dmGetDesignData (fp, GEO_NOR_INI) > 0) {

	k = gnor_ini.layer_no;
	no_xy_pts = gnor_ini.no_xy;

	if (!clp_image -> all_allowed && !(k < 64 && (clp_image -> freemasks_bits & (1LL<<k)))) {
	    for (i = 0; i < no_xy_pts; ++i)
		dmGetDesignData (fp, GEO_NOR_XY);
	    continue;
	}

	switch (gnor_ini.elmt) {
	case POLY_NOR:
	    if (no_xy_pts > pt_size) {
		P_E "%s: too many image poly_nor elements!\n", argv0);
		die ();
	    }
	case RECT_NOR:
	    for (i = 0; i < no_xy_pts; ++i) {
		dmGetDesignData (fp, GEO_NOR_XY);
		px[i] = gnor_xy.x;
		py[i] = gnor_xy.y;
	    }
	    break;
	default:
	    P_E "%s: unsupported image nor element!\n", argv0);
	    die ();
	}

	if (!(fp2 = fp_nxx[k])) fp2 = open_nxx (k);

	gnxx_ini.elmt = gnor_ini.elmt;
	gnxx_ini.no_xy = no_xy_pts;

	for (tdx = pmc_tx, i = pmc_nx;;) {
	    for (bdx = tdx, nx = gnor_ini.nx;;) {
		for (tdy = pmc_ty, j = pmc_ny;;) {
		    for (bdy = tdy, ny = gnor_ini.ny;;) {
			dmPutDesignData (fp2, GEO_NXX_INI);
			for (k = 0; k < no_xy_pts; ++k) {
			    gnxx_xy.x = px[k] + bdx;
			    gnxx_xy.y = py[k] + bdy;
			    dmPutDesignData (fp2, GEO_NXX_XY);
			}
			if (--ny < 0) break;
			bdy += gnor_ini.dy;
		    }
		    if (--j < 0) break;
		    tdy += pmc_dy;
		}
		if (--nx < 0) break;
		bdx += gnor_ini.dx;
	    }
	    if (--i < 0) break;
	    tdx += pmc_dx;
	}
    }

    dmCloseStream (fp, COMPLETE);
}
