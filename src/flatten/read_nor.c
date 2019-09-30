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

static char   *laycode;
static double  gr1, gr2;
static double *px, *py;     /* pointer to point array x,y */
static int     pt_size = 0; /* size of point array's */
static int     no_xy_pts;

static void exp_poly (void);

void newpxpy (int size)
{
    if (pt_size) {
	FREE (px);
	FREE (py);
    }
    pt_size = size;
    ALLOCARR (px, pt_size, double);
    ALLOCARR (py, pt_size, double);
}

/*
** read the not orth. elements of the cell
** and put them in the "_nxx" files.
*/
void read_nor ()
{
    DM_STREAM *fp;
    double  xc, yc;
    double  a1, a2, a, step, step1;
    double  dx, dy;
    double  tmp, width;
    double  d_x, d_y;
    double  dx1, dy1, dx2, dy2;
    double  c1xr, c1yr, a1xy, b1xy;
    double  c2xl, c2yl, c2xr, c2yr, a2xy, b2xy;
    register int i, j;

    a1 = a2 = step1 = 0; /* suppres uninitialized warning */
    dx1 = dy1 = 0; /* suppres uninitialized warning */
    a1xy = b1xy = 0; /* suppres uninitialized warning */

    fp = dmOpenStream (cellkey, "nor", "r");

    while (dmGetDesignData (fp, GEO_NOR_INI) > 0) {

	if (ldmfile) laycode = process -> mask_name[gnor_ini.layer_no];

	no_xy_pts = gnor_ini.no_xy;

	switch (gnor_ini.elmt) {
	case SBOX_NOR:
	    dmGetDesignData (fp, GEO_NOR_XY);
	    xc = gnor_xy.x;
	    yc = gnor_xy.y;
	    dmGetDesignData (fp, GEO_NOR_XY);
	    dx = gnor_xy.x;
	    dy = gnor_xy.y;
	    a = ((dx - xc) + (dy - yc)) / 2;
	    px[0] = xc;
	    py[0] = yc;
	    px[1] = xc + a;
	    py[1] = yc + a;
	    px[2] = dx;
	    py[2] = dy;
	    px[3] = dx - a;
	    py[3] = dy - a;
	    no_xy_pts = 4;
	    break;

	case WIRE_NOR:
	    dmGetDesignData (fp, GEO_NOR_XY);
	    width = gnor_xy.x / 2;
	    if (width <= 0) pr_err (0, "wire: illegal width");

	    dmGetDesignData (fp, GEO_NOR_XY);
	    c1xr = gnor_xy.x; /* xs */
	    c1yr = gnor_xy.y; /* ys */

	    no_xy_pts -= 2;
	    j = 2 * (no_xy_pts + 1);
	    if (j > pt_size) newpxpy (j);

	    for (i = 0;;) {
		dmGetDesignData (fp, GEO_NOR_XY);
		dx2 = gnor_xy.x;
		dy2 = gnor_xy.y;

		if (dx2 == 0) {
		    if (dy2 == 0) pr_err (0, "wire: incr. values are 0");
		    if (dy2 > 0) a2 = rad90;
		    else a2 = 3 * rad90;
		}
		else {
		    a2 = atan (dy2 / dx2);
		    if (dx2 > 0) {
			if (dy2 < 0) a2 += 4 * rad90;
		    }
		    else {
			a2 += 2 * rad90;
		    }
		}

		d_x = width * sin (a2);
		d_y = width * cos (a2);
		c2xl = c1xr;
		c2yl = c1yr;
		c2xr = c2xl + dx2;
		c2yr = c2yl + dy2;
		a2xy = (c2xr - d_x) * (c2yl + d_y)
			- (c2xl - d_x) * (c2yr + d_y);
		b2xy = (c2xr + d_x) * (c2yl - d_y)
			- (c2xl + d_x) * (c2yr - d_y);

		if (i == 0) {
		    px[0] = c2xl + d_x;
		    py[0] = c2yl - d_y;
		    px[--j] = c2xl - d_x;
		    py[j] = c2yl + d_y;
		}
		else {
		    tmp = (a1 > a2) ? (a1 - a2) : (a2 - a1);
		    if (tmp > rad90 && tmp < (3 * rad90))
			pr_err (0, "wire: too big direction change");

		    tmp = dx2 * dy1 - dx1 * dy2;
		    if (tmp == 0) pr_err (0, "wire: no direction change");
		    px[i] = (dx1 * b2xy - dx2 * b1xy) / tmp;
		    py[i] = (dy1 * b2xy - dy2 * b1xy) / tmp;
		    px[--j] = (dx1 * a2xy - dx2 * a1xy) / tmp;
		    py[j] = (dy1 * a2xy - dy2 * a1xy) / tmp;
		}

		if (++i >= no_xy_pts) break;
		a1 = a2;
		dx1 = dx2;
		dy1 = dy2;
		a1xy = a2xy;
		b1xy = b2xy;
		c1xr = c2xr;
		c1yr = c2yr;
	    }

	    px[i] = c2xr + d_x;
	    py[i] = c2yr - d_y;
	    px[--j] = c2xr - d_x;
	    py[j] = c2yr + d_y;
	    no_xy_pts = 2 * (no_xy_pts + 1);
	    break;

	case POLY_NOR:
	    if (no_xy_pts > pt_size) newpxpy (no_xy_pts);
	case RECT_NOR:
	    for (i = 0; i < no_xy_pts; ++i) {
		dmGetDesignData (fp, GEO_NOR_XY);
		px[i] = gnor_xy.x;
		py[i] = gnor_xy.y;
	    }
	    break;

	case CIRCLE_NOR:
	    dmGetDesignData (fp, GEO_NOR_XY);
	    xc = gnor_xy.x;
	    yc = gnor_xy.y;
	    dmGetDesignData (fp, GEO_NOR_XY);
	    gr1 = gnor_xy.x;
	    gr2 = gnor_xy.y;
	    if (gr1 <= gr2 || gr2 < 0) {
		pr_err (0, "circle: illegal radius");
	    }
	    step = 0;
	    if (no_xy_pts > 2) {
		dmGetDesignData (fp, GEO_NOR_XY);
		step  = gnor_xy.x;
		step1 = gnor_xy.y;
		if (step1 == 0) step1 = step;
	    }
	    if (step == 0) {
		step = step1 = 360.0 / 64;
	    }
	    a = 360;
	    if (no_xy_pts > 3) {
		dmGetDesignData (fp, GEO_NOR_XY);
		a1 = gnor_xy.x;
		a2 = gnor_xy.y;
		if (a1 == 360) a1 = 0;
		if (a2 == 0) a2 = 360;
		if (a1 < 0 || a1 > 360
		 || a2 < 0 || a2 > 360 || a1 == a2) {
		    pr_err (0, "circle: illegal angles");
		}
		if ((a = a2 - a1) < 0) a = -a;
	    }
	    i = (a - step1) / step + 2;
	    if (gr2 > 0) i *= 2;
	    if (i > pt_size) newpxpy (i + 4);
	    j = 0;
	    if (a == 360) { /* complete circle */
		px[0] = xc + gr1;
		py[0] = yc;
		for (a = step1; a < 360; a += step) {
		    px[++j] = xc + gr1 * cos (a * rad01);
		    py[j] = yc + gr1 * sin (a * rad01);
		}
		if (gr2 > 0) {
		    px[++j] = px[0];
		    py[j] = py[0];
		    px[++j] = xc + gr2;
		    py[j] = py[0];
		    for (a = step1; a < 360; a += step) {
			px[++j] = xc + gr2 * cos (a * rad01);
			py[j] = yc + gr2 * sin (a * rad01);
		    }
		    px[++j] = xc + gr2;
		    py[j] = py[0];
		}
	    }
	    else { /* not a complete circle */
		px[0] = xc + gr1 * cos (a1 * rad01);
		py[0] = yc + gr1 * sin (a1 * rad01);
		step1 += a1;
		for (a = step1; a < a2; a += step) {
		    px[++j] = xc + gr1 * cos (a * rad01);
		    py[j] = yc + gr1 * sin (a * rad01);
		}
		px[++j] = xc + gr1 * cos (a2 * rad01);
		py[j] = yc + gr1 * sin (a2 * rad01);

		if (gr2 > 0) {
		    px[++j] = xc + gr2 * cos (a2 * rad01);
		    py[j] = yc + gr2 * sin (a2 * rad01);
		    for (a -= step; a >= step1; a -= step) {
			px[++j] = xc + gr2 * cos (a * rad01);
			py[j] = yc + gr2 * sin (a * rad01);
		    }
		    px[++j] = xc + gr2 * cos (a1 * rad01);
		    py[j] = yc + gr2 * sin (a1 * rad01);
		}
		else {
		    px[++j] = xc;
		    py[j] = yc;
		}
	    }
	    no_xy_pts = ++j;
	    px[no_xy_pts] = xc;
	    py[no_xy_pts] = yc;
	    break;
	default:
	    pr_err (0, "unknown nor element");
	}

	exp_poly ();
    }

    dmCloseStream (fp, COMPLETE);
}

static long roundh (double d)
{
    long i;
    i = (d < 0) ? (d - 0.0005) : (d + 0.9995);
    return (i);
}

static long roundL (double d)
{
    long i;
    i = (d < 0) ? (d - 0.9995) : (d + 0.0005);
    return (i);
}

/*
** expand poly to the root level
** of the mc-tree according to mc-parameters
*/
static void exp_poly ()
{
    register long *m;
    register int  k;
    register struct tmtx *tm;
    long Nx, Ny, nx, ny, px0, py0;
    double dx, dy, Dx, Dy;
    double Xl, Xr, Yb, Yt, xl, xr, yb, yt, tmp;
    int  second = 0;

    Dx = gnor_ini.dx;
    Nx = gnor_ini.nx;
    Dy = gnor_ini.dy;
    Ny = gnor_ini.ny;
    if (!ldmfile) {
	Xl = Xr = px[0];
	Yb = Yt = py[0];
	for (k = 1; k < no_xy_pts; ++k) {
	    if (px[k] < Xl) Xl = px[k];
	    else if (px[k] > Xr) Xr = px[k];
	    if (py[k] < Yb) Yb = py[k];
	    else if (py[k] > Yt) Yt = py[k];
	}
    }
    else { Xl = Xr = Yb = Yt = 0; } /* suppres uninitialized warning */

    for (tm = tm_p; tm; tm = tm -> tm_next) {
again:
	m = tm -> mtx;
	dx = dy = 0;
	nx = ny = 0;
	if (Nx) {
	    if (m[0]) { nx = Nx; dx = m[0] * Dx; }
	    else      { ny = Nx; dy = m[3] * Dx; }
	}
	if (Ny) {
	    if (m[4]) { ny = Ny; dy = m[4] * Dy; }
	    else      { nx = Ny; dx = m[1] * Dy; }
	}

	if (ldmfile) {
	    px0 = (long) (m[0] * px[0] + m[1] * py[0] + m[2]);
	    py0 = (long) (m[3] * px[0] + m[4] * py[0] + m[5]);
	    PO "poly %s %ld %ld", laycode, px0, py0);
	    for (k = 1; k < no_xy_pts; ++k) {
		PO " %ld %ld",
		    (long) (m[0] * px[k] + m[1] * py[k] + m[2]),
		    (long) (m[3] * px[k] + m[4] * py[k] + m[5]));
	    }
	    PO " %ld %ld", px0, py0);
	    if (nx) PO " cx %g %ld", dx, nx);
	    if (ny) PO " cy %g %ld", dy, ny);
	    PO "\n");
	}
	else {
	    xl = m[0] * Xl + m[1] * Yb + m[2];
	    yb = m[3] * Xl + m[4] * Yb + m[5];
	    xr = m[0] * Xr + m[1] * Yt + m[2];
	    yt = m[3] * Xr + m[4] * Yt + m[5];
	    if (xl > xr) { tmp = xl; xl = xr; xr = tmp; }
	    if (yb > yt) { tmp = yb; yb = yt; yt = tmp; }
	    gnor_ini.bxl = roundL (xl);
	    gnor_ini.bxr = roundh (xr);
	    gnor_ini.byb = roundL (yb);
	    gnor_ini.byt = roundh (yt);

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
	    gnor_ini.r_bxl = roundL (xl);
	    gnor_ini.r_bxr = roundh (xr);
	    gnor_ini.r_byb = roundL (yb);
	    gnor_ini.r_byt = roundh (yt);

	    gnor_ini.elmt  = POLY_NOR;
	    gnor_ini.no_xy = no_xy_pts;
	    gnor_ini.dx = dx;
	    gnor_ini.nx = nx;
	    gnor_ini.dy = dy;
	    gnor_ini.ny = ny;

	    if (gnor_ini.r_bxl < elbb_xl) elbb_xl = gnor_ini.r_bxl;
	    if (gnor_ini.r_bxr > elbb_xr) elbb_xr = gnor_ini.r_bxr;
	    if (gnor_ini.r_byb < elbb_yb) elbb_yb = gnor_ini.r_byb;
	    if (gnor_ini.r_byt > elbb_yt) elbb_yt = gnor_ini.r_byt;

	    dmPutDesignData (fp_nor, GEO_NOR_INI);

	    for (k = 0; k < no_xy_pts; ++k) {
		gnor_xy.x = m[0] * px[k] + m[1] * py[k] + m[2];
		gnor_xy.y = m[3] * px[k] + m[4] * py[k] + m[5];
		dmPutDesignData (fp_nor, GEO_NOR_XY);
	    }
	}
    }
    if (!second++ && (tm = tm_s)) goto again;
}
