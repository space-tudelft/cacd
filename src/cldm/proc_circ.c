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

void proc_circ (int ixc, int iyc, int ir1, int ir2, int ia1, int ia2, int n_edges)
{
    long bxl, bxr, byb, byt;
    double xc, yc, r1, r2, a1, a2;
    double rad_a1, rad_a2, tmp1, tmp2;
    double d_bxl, d_bxr, d_byb, d_byt;
    int xy_pairs;

    xc = (double)ixc;
    yc = (double)iyc;
    r1 = (double)ir1;
    r2 = (double)ir2;

    if (ia1 == 360000) ia1 = 0;
    if (ia2 == 0) ia2 = 360000;

    if (ia1 == 0 && ia2 == 360000) { /* complete circle */
	xy_pairs = 3;
	bxl = ixc - ir1;
	bxr = ixc + ir1;
	byb = iyc - ir1;
	byt = iyc + ir1;
	a1 = a2 = 0; /* init to suppres compiler warning */
    }
    else {
	xy_pairs = 4;
	a1 = (double)ia1 / 1000;
	a2 = (double)ia2 / 1000;
	rad_a1 = a1 * RAD_DEG;
	rad_a2 = a2 * RAD_DEG;

	tmp1 = xc + r1 * cos (rad_a1);
	tmp2 = xc + r1 * cos (rad_a2);
	if (tmp1 < tmp2) {
	    d_bxl = tmp1;
	    d_bxr = tmp2;
	}
	else {
	    d_bxl = tmp2;
	    d_bxr = tmp1;
	}

	if (ia1 <= 180000 && (ia2 >= 180000 || ia2 < ia1)) d_bxl = xc - r1;
	else if (ia2 >= 180000 && ia2 < ia1) d_bxl = xc - r1;
	if (ia1 == 0 || ia2 == 3600) d_bxr = xc + r1;
	else if (ia1 > ia2) d_bxr = xc + r1;

	tmp1 = yc + r1 * sin (rad_a1);
	tmp2 = yc + r1 * sin (rad_a2);
	if (tmp1 < tmp2) {
	    d_byb = tmp1;
	    d_byt = tmp2;
	}
	else {
	    d_byb = tmp2;
	    d_byt = tmp1;
	}

	if (ia1 <= 270000 && (ia2 < ia1 || ia2 >= 270000)) d_byb = yc - r1;
	if (ia2 >=  90000 && (ia1 > ia2 || ia1 <=  90000)) d_byt = yc + r1;

	if (ir2) {
	    tmp1 = xc + r2 * cos (rad_a1);
	    tmp2 = xc + r2 * cos (rad_a2);
	    if (tmp1 < d_bxl) d_bxl = tmp1;
	    if (tmp2 < d_bxl) d_bxl = tmp2;
	    if (tmp1 > d_bxr) d_bxr = tmp1;
	    if (tmp2 > d_bxr) d_bxr = tmp2;
	    tmp1 = yc + r2 * sin (rad_a1);
	    tmp2 = yc + r2 * sin (rad_a2);
	    if (tmp1 < d_byb) d_byb = tmp1;
	    if (tmp2 < d_byb) d_byb = tmp2;
	    if (tmp1 > d_byt) d_byt = tmp1;
	    if (tmp2 > d_byt) d_byt = tmp2;
	}
	else {
	    if (ia1 >= 270000 && (ia2 > ia1 || ia2 <= 90000)) d_bxl = xc;
	    else if (ia1 <= 90000 && ia2 <= 90000 && ia2 > ia1) d_bxl = xc;
	    if (ia1 >= 90000 && ia2 <= 270000 && ia2 > ia1) d_bxr = xc;
	    if (ia2 <= 180000 && ia2 > ia1) d_byb = yc;
	    if (ia1 >= 180000 && ia2 > ia1) d_byt = yc;
	}
	bxl = roundL (d_bxl);
	bxr = roundh (d_bxr);
	byb = roundL (d_byb);
	byt = roundh (d_byt);
    }

    gnor_ini.bxl = bxl;
    gnor_ini.bxr = bxr;
    gnor_ini.byb = byb;
    gnor_ini.byt = byt;

    if (nx || ny) {
	if (nx) {
	    tmp_i = nx * dx;
	    if (tmp_i < 0)
		bxl += tmp_i;
	    else
		bxr += tmp_i;
	}
	if (ny) {
	    tmp_i = ny * dy;
	    if (tmp_i < 0)
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
    gnor_ini.elmt = CIRCLE_NOR;
    gnor_ini.no_xy = xy_pairs;
    gnor_ini.dx = dx;
    gnor_ini.dy = dy;
    gnor_ini.nx = nx;
    gnor_ini.ny = ny;
    dmPutDesignData (fp_nor, GEO_NOR_INI);

    gnor_xy.x = xc;
    gnor_xy.y = yc;
    dmPutDesignData (fp_nor, GEO_NOR_XY); /* 1 */
    gnor_xy.x = r1;
    gnor_xy.y = r2;
    dmPutDesignData (fp_nor, GEO_NOR_XY); /* 2 */
    gnor_xy.x = n_edges;
    gnor_xy.y = 0;
    dmPutDesignData (fp_nor, GEO_NOR_XY); /* 3 */
    if (xy_pairs == 4) {
	gnor_xy.x = a1;
	gnor_xy.y = a2;
	dmPutDesignData (fp_nor, GEO_NOR_XY); /* 4 */
    }

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
