/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

void proc_poly ()
{
    register int i, ind_4;
#ifdef CHECK_INTERSECT
    register int j, result;
    double  xa, ya, xb, yb, xp, yp, xq, yq;
    double  DXab, DYab, DXpq, DYpq;
    double  numnab, numnpq, numxs, numys, denomxs, denomys;
    long    x, y;
#endif /* CHECK_INTERSECT */
    long    b_xl, b_xr, b_yb, b_yt;
    long    x0, y0, x1, y1, flag = mode45;
/*
** DY contains the difference between the y-values.
** DX between the x-values.  numn contains the value of
** the numerator of n; denomn contains the value of the
** denominator of n (see the main expression of a line:
** y = mx + n).  numxs till denomys contain the values of
** the numerator and the denominator of the coordinates of
** the intersection.  result denotes whether a certain point
** is an element of a linepart (1) or not (0).
*/
    ind_4 = int_ind - 4;

    /* check on coordinates */

    if (int_ind < 8) goto ret;

    if (int_ind & 1) {
	pr_exit (014, 35, "poly"); /* odd # of coords */
	return;
    }

    if (int_val[0] != int_val[int_ind - 2]
	    || int_val[1] != int_val[int_ind - 1]) {
	pr_exit (014, 36, 0);	/* last != first */
	return;
    }

#ifdef CHECK_INTERSECT
    /* check on intersections & equal coordinate pairs */

    /* (1) check all adjacent lineparts */
    for (i = 0; i < ind_4;) {
	xa = int_val[i];
	ya = int_val[i + 1];
	xb = int_val[i + 2];
	yb = int_val[i + 3];
	xq = int_val[i + 4];
	yq = int_val[i + 5];
	if ((yb - ya) * (xq - xb) == (yq - yb) * (xb - xa)) {
	    if ((int_ind -= 2) < 8) goto ret;
	    for (j = i + 2; j <= ind_4; j += 2) {
		int_val[j    ] = int_val[j + 2];
		int_val[j + 1] = int_val[j + 3];
	    }
	    ind_4 -= 2;
	    if (i > 0) i -= 2;
	}
	else i += 2;
    }

    /* (2) check last linepart against first linepart */
    xa = int_val[0];
    ya = int_val[1];
    xb = int_val[2];
    yb = int_val[3];
    for (;;) {
	xp = int_val[ind_4];
	yp = int_val[ind_4 + 1];
	if ((yb - ya) * (xa - xp) == (ya - yp) * (xb - xa)) {
	    if ((int_ind -= 2) < 8) goto ret;
	    int_val[0] = xa = xp;
	    int_val[1] = ya = yp;
	    ind_4 -= 2;
	}
	else break;
    }

    /* (3) check all NOT adjacent lineparts */
    for (i = 0; i < ind_4; i += 2) {
	xa = int_val[i];
	ya = int_val[i + 1];
	xb = int_val[i + 2];
	yb = int_val[i + 3];
	for (j = i + 4; j <= ind_4 && j != i + ind_4; j += 2) {
	    /* two NOT adjacent lineparts */
	    xp = int_val[j];
	    yp = int_val[j + 1];
	    xq = int_val[j + 2];
	    yq = int_val[j + 3];
	    if ((yb - ya) * (xq - xp) == (yq - yp) * (xb - xa)) {
		/* the same directioncoefficients */
		/* real parallel lineparts or
			lineparts with the same carrier */

		if ((xb * ya - xa * yb) * (xq - xp) ==
			    (xq * yp - xp * yq) * (xb - xa)) {
		    /* the lineparts have the same carrier;
		       check whether the lineparts overlap or not */

		    /* point q on linepart ab ? */
		    result = test (xa, ya, xb, yb, xq, 1.0, yq, 1.0);
		    if (result == 1) {
			/* the lineparts (partly) overlap */
			pr_exit (014, 33, 0);
			return;
		    }
		}
	    }
	    else { /* the LINES intersect, the lineparts maybe */
		/* check on intersecting or touching */
		DYab = yb - ya;
		DXab = xb - xa;
		numnab = xb * ya - xa * yb;
		DYpq = yq - yp;
		DXpq = xq - xp;
		numnpq = xq * yp - xp * yq;

		/* determining the intersection of the LINES */

		/* first the x-coordinate */
		numxs = numnpq * DXab - numnab * DXpq;
		denomxs = DYab * DXpq - DYpq * DXab;
		/* the y-coordinate */
		numys = numnpq * DYab - numnab * DYpq;
		denomys = denomxs;

		/* check whether the intersection is
		    an element of both lineparts or not */
		result = test (xa, ya, xb, yb, numxs, denomxs, numys, denomys);
		if (result == 1) {
		    /* the intersection is an element of the linepart ab */
		    result = test (xp, yp, xq, yq,
					numxs, denomxs, numys, denomys);
		    if (result == 1) {
			/* the intersection is an element of both lineparts */
			pr_exit (014, 39, 0);
			return;
		    }
		}
	    }
	}
    }
#endif /* CHECK_INTERSECT */

    /* (4) check for orthogonal box */
    if (ind_4 == 6) {
	if (int_val[0] == int_val[2]) i = 5;
	else i = 3;
	while (i < int_ind) {
	    if (int_val[i - 2] != int_val[i]) break;
	    if (i & 1) ++i;
	    else i += 3;
	}
	if (i >= int_ind) { /* orth. */
	    i = 0;
	    if (int_val[i] == int_val[i+2]) i = 2;
	    if (int_val[i] % 4 == 0 && int_val[i+2] % 4 == 0
	     && int_val[i+3] % 4 == 0 && int_val[i+5] % 4 == 0) {
		/* orth. and on lambda grid */
		proc_box (int_val[i] / 4, int_val[i+2] / 4,
			    int_val[i+3] / 4, int_val[i+5] / 4);
		return;
	    }
	}
    }

    /* calculating the bounding box */
    x0 = b_xr = b_xl = int_val[0];
    y0 = b_yt = b_yb = int_val[1];

    for (i = 2; i < int_ind;) {
	x1 = int_val[i++];
	y1 = int_val[i++];
	if (x1 < b_xl) b_xl = x1;
	else if (x1 > b_xr) b_xr = x1;
	if (y1 < b_yb) b_yb = y1;
	else if (y1 > b_yt) b_yt = y1;
	if (flag) {
	    if ((x0 -= x1) < 0) x0 = -x0;
	    if ((y0 -= y1) < 0) y0 = -y0;
	    if (x0 && y0 && x0 != y0) {
		flag = 0;
		pr_exit (0614, 53, "poly"); /* not 45 degree */
	    }
	    x0 = x1; y0 = y1;
	}
    }

    b_xl = gnor_ini.bxl = roundL ((double) b_xl / 4.0);
    b_xr = gnor_ini.bxr = roundh ((double) b_xr / 4.0);
    b_yb = gnor_ini.byb = roundL ((double) b_yb / 4.0);
    b_yt = gnor_ini.byt = roundh ((double) b_yt / 4.0);

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

    /* updating model bounding box */
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

    int_ind -= 2;
    gnor_ini.layer_no = lay_code;
    gnor_ini.elmt  = POLY_NOR;
    gnor_ini.no_xy = int_ind / 2;
    gnor_ini.dx = (double) dx;
    gnor_ini.nx = nx;
    gnor_ini.dy = (double) dy;
    gnor_ini.ny = ny;
    dmPutDesignData (fp_nor, GEO_NOR_INI);

    for (i = 0; i < int_ind; ) {
	gnor_xy.x = (double) int_val[i++] / 4.0;
	gnor_xy.y = (double) int_val[i++] / 4.0;
	dmPutDesignData (fp_nor, GEO_NOR_XY);
    }
    return;
ret:
    pr_exit (014, 34, "poly"); /* too less coords */
}

int test (double x0, double y0, double x1, double y1, double numx, double denomx, double numy, double denomy)
/*
** This function tests whether a point (denoted by numx
** till denomy) is an element of a linepart (denoted by x0
** till y1) or not.  If it is an element of the linepart,
** then the return code is 1, otherwise 0.
*/
{
    double  xmin, xmax, ymin, ymax;

    xmin = Min (x0, x1);
    xmax = Max (x0, x1);
    ymin = Min (y0, y1);
    ymax = Max (y0, y1);

/* check whether the point is an element of the linepart
** between the points (x0,y0) and (x1,y1)
*/
    if (denomx >= 0.0) {
	if (numx < xmin * denomx || numx > xmax * denomx)
	    return (0);
	if (numy < ymin * denomy || numy > ymax * denomy)
	    return (0);
    }
    else { /* negative denominators, the signs have to be switched */
	if (numx > xmin * denomx || numx < xmax * denomx)
	    return (0);
	if (numy > ymin * denomy || numy < ymax * denomy)
	    return (0);
    }
    return (1);
}
