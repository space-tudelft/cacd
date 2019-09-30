/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	S. de Graaf
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

#include "src/cldm/extern.h"

void proc_sbox (int ix1, int iy1, int ix2, int iy2)
{
    long bxl, bxr, byb, byt;
    int  x1, x2, x3, x4, y1, y2, y3, y4;
    int  idx, idy;

    idx = ix2 - ix1;
    idy = iy2 - iy1;

    if (idx == idy || idx == -idy) {
	pr_exit (014, 30, "sbox"); /* illegal coord's */
	return;
    }

    tmp_i = idx + idy;
    x1 = 2 * ix1;
    y1 = 2 * iy1;
    x3 = 2 * ix2;
    y3 = 2 * iy2;
    x2 = x1 + tmp_i;
    y2 = y1 + tmp_i;
    x4 = x3 - tmp_i;
    y4 = y3 - tmp_i;

    if (tmp_i > 0) {
	bxl = (x1 < x4) ? x1 : x4;
	bxr = (x3 > x2) ? x3 : x2;
	byb = (y1 < y4) ? y1 : y4;
	byt = (y3 > y2) ? y3 : y2;
    }
    else {
	bxl = (x2 < x3) ? x2 : x3;
	bxr = (x1 > x4) ? x1 : x4;
	byb = (y2 < y3) ? y2 : y3;
	byt = (y1 > y4) ? y1 : y4;
    }

    bxl = roundL ((double) bxl / 2.0);
    bxr = roundh ((double) bxr / 2.0);
    byb = roundL ((double) byb / 2.0);
    byt = roundh ((double) byt / 2.0);

    gnor_ini.bxl = bxl;
    gnor_ini.bxr = bxr;
    gnor_ini.byb = byb;
    gnor_ini.byt = byt;

    if (nx || ny) {
	if (nx) {
	    if ((tmp_i = nx * dx) < 0)
		bxl += tmp_i;
	    else
		bxr += tmp_i;
	}
	if (ny) {
	    if ((tmp_i = ny * dy) < 0)
		byb += tmp_i;
	    else
		byt += tmp_i;
	}
	gnor_ini.r_bxl = bxl;
	gnor_ini.r_bxr = bxr;
	gnor_ini.r_byb = byb;
	gnor_ini.r_byt = byt;
    }

    gnor_ini.layer_no = lay_code;
    gnor_ini.elmt = SBOX_NOR;
    gnor_ini.no_xy = 2;
    gnor_ini.dx = dx;
    gnor_ini.dy = dy;
    gnor_ini.nx = nx;
    gnor_ini.ny = ny;
    dmPutDesignData (fp_nor, GEO_NOR_INI);

    gnor_xy.x = (double) ix1;
    gnor_xy.y = (double) iy1;
    dmPutDesignData (fp_nor, GEO_NOR_XY);
    gnor_xy.x = (double) ix2;
    gnor_xy.y = (double) iy2;
    dmPutDesignData (fp_nor, GEO_NOR_XY);

    /* update model-bbox */
    if (ini_bbbox) {
	bbnd_xl = bxl;
	bbnd_xr = bxr;
	bbnd_yb = byb;
	bbnd_yt = byt;
	ini_bbbox = 0;
    }
    else {
	if (bxl < bbnd_xl) bbnd_xl = bxl;
	if (bxr > bbnd_xr) bbnd_xr = bxr;
	if (byb < bbnd_yb) bbnd_yb = byb;
	if (byt > bbnd_yt) bbnd_yt = byt;
    }
}
