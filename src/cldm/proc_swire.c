/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
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

void proc_swire ()
{
    double rad45;
    long   b_xl, b_xr, b_yb, b_yt;
    double bxl, bxr, byb, byt;
    double d_x, d_y, a1, a2;
    double dx1, dy1, dx2, dy2;
    double c1xr, c1yr, a1xy, b1xy;
    double c2xl, c2yl, c2xr, c2yr, a2xy, b2xy;
    double aspx, aspy, bspx, bspy;
    register int i, j;
    int x, y, flag = mode45;

    if (w_width <= 0) {
	pr_exit (014, 32, "swire");
	return;
    }

    if (int_ind < 2) {
	pr_exit (014, 34, "swire"); /* too less coords */
	return;
    }

    if (int_ind & 1) {
	pr_exit (014, 35, "swire"); /* odd # of coords */
	return;
    }

    rad45 = atan (1.0);

    c1xr = w_x;
    c1yr = w_y;

    bxl = bxr = byb = byt = 0; /* init, suppres compiler warning */
    dx1 = dy1 = a1 = 0; /* init, suppres compiler warning */
    a1xy = b1xy = 0; /* init, suppres compiler warning */

    for (i = 0;;) {
	dx2 = x = int_val[i];
	dy2 = y = int_val[i+1];
	if (x == 0) {
	    if (y == 0) {
		pr_exit (014, 31, "swire"); /* incr. values are 0 */
		return;
	    }
	    if (y > 0) a2 = 2 * rad45;
	    else a2 = 6 * rad45;
	}
	else {
	    a2 = atan (dy2 / dx2);
	    if (x > 0) {
		if (y < 0) { a2 += 8 * rad45; y = -y; }
	    }
	    else { /* x < 0 */
		a2 += 4 * rad45;
		if (y > 0) y = -y;
	    }
	    if (flag && y && x != y) {
		flag = 0;
		pr_exit (0614, 53, "swire"); /* not 45 degree */
	    }
	}
	d_x = w_width * sin (a2);
	d_y = w_width * cos (a2);
	c2xl = c1xr;
	c2yl = c1yr;
	c2xr = c2xl + dx2;
	c2yr = c2yl + dy2;
	a2xy = (c2xr - d_x) * (c2yl + d_y) - (c2xl - d_x) * (c2yr + d_y);
	b2xy = (c2xr + d_x) * (c2yl - d_y) - (c2xl + d_x) * (c2yr - d_y);

	if (i == 0) {
	    if (dx2 > 0) {
		bxl = (d_x > 0) ? (c2xl - d_x) : (c2xl + d_x);
		bxr = (d_x > 0) ? (c2xr + d_x) : (c2xr - d_x);
	    }
	    else {
		bxl = (d_x > 0) ? (c2xr - d_x) : (c2xr + d_x);
		bxr = (d_x > 0) ? (c2xl + d_x) : (c2xl - d_x);
	    }
	    if (dy2 > 0) {
		byb = (d_y > 0) ? (c2yl - d_y) : (c2yl + d_y);
		byt = (d_y > 0) ? (c2yr + d_y) : (c2yr - d_y);
	    }
	    else {
		byb = (d_y > 0) ? (c2yr - d_y) : (c2yr + d_y);
		byt = (d_y > 0) ? (c2yl + d_y) : (c2yl - d_y);
	    }
	}
	else {
	    tmp_d = (a1 > a2) ? (a1 - a2) : (a2 - a1);
	    if (tmp_d > (2 * rad45) && tmp_d < (6 * rad45)) {
		pr_exit (014, 50, ""); /* too big direction change */
		return;
	    }
	    tmp_d = dx2 * dy1 - dx1 * dy2;
	    if (tmp_d == 0) {
		int_val[i-2] += int_val[i];
		int_val[i-1] += int_val[i+1];
		for (j = i+2; j < int_ind; j += 2) {
		    int_val[j-2] = int_val[j];
		    int_val[j-1] = int_val[j+1];
		}
		i -= 2;
		int_ind -= 2;
	    }
	    else {
		aspx = (dx1 * a2xy - dx2 * a1xy) / tmp_d;
		aspy = (dy1 * a2xy - dy2 * a1xy) / tmp_d;
		bspx = (dx1 * b2xy - dx2 * b1xy) / tmp_d;
		bspy = (dy1 * b2xy - dy2 * b1xy) / tmp_d;
		if (aspx > bspx) {
		    if (bspx < bxl) bxl = bspx;
		    if (aspx > bxr) bxr = aspx;
		}
		else {
		    if (aspx < bxl) bxl = aspx;
		    if (bspx > bxr) bxr = bspx;
		}
		if (aspy > bspy) {
		    if (bspy < byb) byb = bspy;
		    if (aspy > byt) byt = aspy;
		}
		else {
		    if (aspy < byb) byb = aspy;
		    if (bspy > byt) byt = bspy;
		}
	    }
	}

	i += 2;
	if (i >= int_ind) break;
	a1 = a2;
	dx1 = dx2;
	dy1 = dy2;
	a1xy = a2xy;
	b1xy = b2xy;
	c1xr = c2xr;
	c1yr = c2yr;
    }

    if (dx2 > 0) {
	tmp_d = (d_x > 0) ? (c2xr + d_x) : (c2xr - d_x);
	if (tmp_d > bxr) bxr = tmp_d;
    }
    else {
	tmp_d = (d_x > 0) ? (c2xr - d_x) : (c2xr + d_x);
	if (tmp_d < bxl) bxl = tmp_d;
    }
    if (dy2 > 0) {
	tmp_d = (d_y > 0) ? (c2yr + d_y) : (c2yr - d_y);
	if (tmp_d > byt) byt = tmp_d;
    }
    else {
	tmp_d = (d_y > 0) ? (c2yr - d_y) : (c2yr + d_y);
	if (tmp_d < byb) byb = tmp_d;
    }

    b_xl = gnor_ini.bxl = roundL (bxl / 2.0);
    b_xr = gnor_ini.bxr = roundh (bxr / 2.0);
    b_yb = gnor_ini.byb = roundL (byb / 2.0);
    b_yt = gnor_ini.byt = roundh (byt / 2.0);

    if (nx || ny) {
	if (nx) {
	    if ((tmp_i = nx * dx) < 0)
		b_xl += tmp_i;
	    else
		b_xr += tmp_i;
	}
	if (ny) {
	    if ((tmp_i = ny * dy) < 0)
		b_yb += tmp_i;
	    else
		b_yt += tmp_i;
	}
	gnor_ini.r_bxl = b_xl;
	gnor_ini.r_bxr = b_xr;
	gnor_ini.r_byb = b_yb;
	gnor_ini.r_byt = b_yt;
    }

    gnor_ini.layer_no = lay_code;
    gnor_ini.elmt = WIRE_NOR;
    gnor_ini.no_xy = int_ind / 2 + 2;
    gnor_ini.dx = dx;
    gnor_ini.dy = dy;
    gnor_ini.nx = nx;
    gnor_ini.ny = ny;
    dmPutDesignData (fp_nor, GEO_NOR_INI);

    gnor_xy.x = (double) w_width;
    gnor_xy.y = 0;
    dmPutDesignData (fp_nor, GEO_NOR_XY);
    gnor_xy.x = (double) w_x / 2.0;
    gnor_xy.y = (double) w_y / 2.0;
    dmPutDesignData (fp_nor, GEO_NOR_XY);

    for (i = 0; i < int_ind; i += 2) {
	gnor_xy.x = (double) int_val[i] / 2.0;
	gnor_xy.y = (double) int_val[i+1] / 2.0;
	dmPutDesignData (fp_nor, GEO_NOR_XY);
    }

    /* update model-bbox */
    if (ini_bbbox) {
	bbnd_xl = b_xl;
	bbnd_xr = b_xr;
	bbnd_yb = b_yb;
	bbnd_yt = b_yt;
	ini_bbbox = 0;
    }
    else {
	if (b_xl < bbnd_xl) bbnd_xl = b_xl;
	if (b_xr > bbnd_xr) bbnd_xr = b_xr;
	if (b_yb < bbnd_yb) bbnd_yb = b_yb;
	if (b_yt > bbnd_yt) bbnd_yt = b_yt;
    }
}
