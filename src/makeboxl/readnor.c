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

#include <math.h>
#include "src/makeboxl/extern.h"

#define	Abs(a)		((a) < 0 ? -(a) : (a))
#define Min(a,b)	((a) < (b) ? (a) : (b))
#define	Max(a,b)	((a) > (b) ? (a) : (b))
#define Sign(n)	        ((n)>0?1:((n)<0?-1:0))

struct edge {
    long   x;
    struct edge *next;
};

struct edge **edge_array;

long   iymin, iymax;
long   Dx, Dy, Nx, Ny;
int    elmt_no;
int    no_xy_pts;
int    c_c;
double gr1, gr2;

/* local operations */
Private void disc_poly (void);
Private void StoreEdge (long x, long y);
Private void do_exp_box (long xl, long xr, long yb, long yt);
Private void disc_circ (double dx, double dy, long nx, long ny);
Private void exp_poly (double dx, double dy, double a1, double a2);
Private void newpxpy (int size);
Private void msg_die (char *msg);

Private long l_round (double d)
{
    long l = (d < 0) ? (long)(d - 0.5001) : (long)(d + 0.5001);
    return (l);
}

/*
** read the not orth. elements of the cell
** and make discrete boxes of them
*/
void read_nor ()
{
    DM_STREAM *fp;
    double  ybot, ytop;
    double  x1, y1, x2, y2;
    double  tmp, width;
    double  d_x, d_y, a1, a2;
    double  dx1, dy1, dx2, dy2;
    double  c1xr, c1yr, a1xy, b1xy;
    double  c2xl, c2yl, c2xr, c2yr, a2xy, b2xy;
    register int i, j;

    ybot = ytop = a1 = 0; /* suppres uninitialized warning */
    dx1 = dy1 = a1xy = b1xy = 0; /* suppres uninitialized warning */

    fp = dmOpenStream (cellkey, "nor", "r");

    while (dmGetDesignData (fp, GEO_NOR_INI) > 0) {

	mask_no = gnor_ini.layer_no;
	no_xy_pts = gnor_ini.no_xy;

	gnor_ini.dx *= samples;
	gnor_ini.dy *= samples;

	switch (gnor_ini.elmt) {
	case SBOX_NOR:
	    dmGetDesignData (fp, GEO_NOR_XY);
	    x1 = samples * gnor_xy.x;
	    y1 = samples * gnor_xy.y;
	    dmGetDesignData (fp, GEO_NOR_XY);
	    x2 = samples * gnor_xy.x;
	    y2 = samples * gnor_xy.y;
	    tmp = ((x2 - x1) + (y2 - y1)) / 2;
	    px[0] = x1;
	    py[0] = y1;
	    px[1] = x1 + tmp;
	    py[1] = y1 + tmp;
	    px[2] = x2;
	    py[2] = y2;
	    px[3] = x2 - tmp;
	    py[3] = y2 - tmp;
	    px[4] = x1;
	    py[4] = y1;
	    no_xy_pts = 4;

	    if (tmp > 0) {
		ybot = (y1 < py[3]) ? y1 : py[3];
		ytop = (py[1] > y2) ? py[1] : y2;
	    }
	    else {
		ybot = (py[1] < y2) ? py[1] : y2;
		ytop = (y1 > py[3]) ? y1 : py[3];
	    }

	    Dx = l_round (gnor_ini.dx);
	    Dy = l_round (gnor_ini.dy);
	    Nx = gnor_ini.nx;
	    Ny = gnor_ini.ny;
	    iymin = l_round (ybot);
	    iymax = l_round (ytop);
	    disc_poly ();
	    break;

	case WIRE_NOR:
	    dmGetDesignData (fp, GEO_NOR_XY);
	    width = samples * gnor_xy.x / 2;
	    if (width <= 0) msg_die ("wire: illegal width");

	    dmGetDesignData (fp, GEO_NOR_XY);
	    c1xr = samples * gnor_xy.x; /* xs */
	    c1yr = samples * gnor_xy.y; /* ys */

	    no_xy_pts -= 2;
	    j = 2 * (no_xy_pts + 1);
	    if (j >= pt_size) newpxpy (j + 1);

	    for (i = 0;;) {
		dmGetDesignData (fp, GEO_NOR_XY);
		dx2 = samples * gnor_xy.x;
		dy2 = samples * gnor_xy.y;

		if (dx2 == 0) {
		    if (dy2 == 0) msg_die ("wire: incr. values are 0");
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
		    if (d_y > 0) {
			ybot = py[0]; ytop = py[j];
		    }
		    else {
			ybot = py[j]; ytop = py[0];
		    }
		}
		else {
		    tmp = (a1 > a2) ? (a1 - a2) : (a2 - a1);
		    if (tmp > rad90 && tmp < (3 * rad90))
			msg_die ("wire: too big direction change");

		    tmp = dx2 * dy1 - dx1 * dy2;
		    if (tmp == 0) msg_die ("wire: no direction change");
		    px[i] = (dx1 * b2xy - dx2 * b1xy) / tmp;
		    py[i] = (dy1 * b2xy - dy2 * b1xy) / tmp;
		    if (py[i] < ybot) ybot = py[i];
		    else if (py[i] > ytop) ytop = py[i];
		    px[--j] = (dx1 * a2xy - dx2 * a1xy) / tmp;
		    py[j] = (dy1 * a2xy - dy2 * a1xy) / tmp;
		    if (py[j] < ybot) ybot = py[j];
		    else if (py[j] > ytop) ytop = py[j];
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
	    if (py[i] < ybot) ybot = py[i];
	    else if (py[i] > ytop) ytop = py[i];
	    px[--j] = c2xr - d_x;
	    py[j] = c2yr + d_y;
	    if (py[j] < ybot) ybot = py[j];
	    else if (py[j] > ytop) ytop = py[j];

	    no_xy_pts = 2 * (no_xy_pts + 1);
	    px[no_xy_pts] = px[0];
	    py[no_xy_pts] = py[0];

	    Dx = l_round (gnor_ini.dx);
	    Dy = l_round (gnor_ini.dy);
	    Nx = gnor_ini.nx;
	    Ny = gnor_ini.ny;
	    iymin = l_round (ybot);
	    iymax = l_round (ytop);
	    disc_poly ();
	    break;

	case POLY_NOR:
	    if (no_xy_pts >= pt_size) newpxpy (no_xy_pts + 1);
	case RECT_NOR:
	    dmGetDesignData (fp, GEO_NOR_XY);
	    px[0] = samples * gnor_xy.x;
	    py[0] = samples * gnor_xy.y;
	    ytop = ybot = py[0];
	    for (i = 1; i < no_xy_pts; ++i) {
		dmGetDesignData (fp, GEO_NOR_XY);
		px[i] = samples * gnor_xy.x;
		py[i] = samples * gnor_xy.y;
		if (py[i] < ybot) ybot = py[i];
		else if (py[i] > ytop) ytop = py[i];
	    }
	    px[no_xy_pts] = px[0];
	    py[no_xy_pts] = py[0];

	    Dx = l_round (gnor_ini.dx);
	    Dy = l_round (gnor_ini.dy);
	    Nx = gnor_ini.nx;
	    Ny = gnor_ini.ny;
	    iymin = l_round (ybot);
	    iymax = l_round (ytop);
	    disc_poly ();
	    break;

	case CIRCLE_NOR:
	    for (i = 0; i < no_xy_pts; ++i) {
		dmGetDesignData (fp, GEO_NOR_XY);
		px[i] = gnor_xy.x;
		py[i] = gnor_xy.y;
	    }
	    disc_circ (gnor_ini.dx, gnor_ini.dy,
		gnor_ini.nx, gnor_ini.ny);
	    break;

	default:
	    msg_die ("unknown nor element");
	}
    }

    dmCloseStream (fp, COMPLETE);
}

Private void disc_poly ()
{
    register struct edge *ep, *epn;
    register int k, no_el;
    double  a, b, d1, d2, adx;
    int     outp, again, x_dir;
    long    x1n, x2n;
    long    x1, x2, y1, y2;
    long    dx, dy;

    adx = 0; /* suppres uninitialized warning */
    x1 = x2 = y1 = dy = 0; /* suppres uninitialized warning */

    no_el = iymax - iymin;
    ALLOCARR (edge_array, no_el, struct edge *);
    for (k = 0; k < no_el; ++k) edge_array[k] = 0;

    /* For each line segment in the 'px/py' array
     */
    for (k = 0; k < no_xy_pts; ++k) {
	x1 = l_round (px[k]);
	y1 = l_round (py[k]);
	x2 = l_round (px[k + 1]);
	y2 = l_round (py[k + 1]);

	/* Compute horizontal and vertical step values
	 */
	dx = Sign (x2 - x1);
	dy = Sign (y2 - y1);

	if (dy == 0) continue;

	if (dx == 0) {
	    /* If line is vertical, a special case.
	     * If the distances are initially equal,
	     * then in combination with inital value of
	     * x_dir=0, all goes well.
	     */
	    d1 = d2 = 0;
	}
	else {

	    /* Compute 'a' and 'b' from 'y = ax + b'.
	     */
	    a = (py[k + 1] - py[k])
		/ (px[k + 1] - px[k]);
	    b = py[k] - a * px[k];

	    /* Distance from point (x,y) to line y = ax + b is now
	     * proportional to
	     *			abs (a*x - y + b)
	     *
	     * Compute 'signed' distance to the line for both
	     * possible next points (x1+dx, y1) and (x, y1+dx)
	     * and the distance incrementor.
	     * The signed distance is the term inside the abs() above.
	     * Then, when a step is made, this signed distance
	     * can be efficiently updated by incrementing with
	     * 'a * dx' if the step was horizontal, or decrementing
	     * with 'dy' if the step was vertical.
	     * If you don't believe this, check it.
	     */
	    d1 = a * (x1 + dx) - y1 + b;
	    d2 = a * x1 - y1 - dy + b;
	    adx = a * dx;
	}

	x_dir = 0;

	/* Now, process the line segment.
	 * Need only to test on y values,
	 * because horizontal steps don't produce output.
	 * If abs (d1) == abs (d2), a 45-degrees line.
	 * Should then step in the same direction as previous step,
	 * to avoid having a staircase totally on one side of the line.
	 */
	while (y1 != y2) {
	    if (Abs (d1) < Abs (d2)
		    || (Abs (d1) == Abs (d2) && x_dir)) {
		/* step horizontally */
		x1 += dx;
		x_dir = 1;
		d1 += adx;
		d2 += adx;
	    }
	    else  {
		/* step vertically */
		StoreEdge (x1, Min (y1, y1 + dy));
		y1 += dy;
		x_dir = 0;
		d1 -= dy;
		d2 -= dy;
	    }
	}
    }

    do {
	again = outp = 0;
	for (k = 0; k < no_el; ++k) {
	    ep = edge_array[k];
	    if (ep) {
		if (!outp) {
		    x1 = ep -> x;
		    epn = ep -> next;
		    FREE (ep);
		    x2 = epn -> x;
		    if ((ep = epn -> next)) ++again;
		    edge_array[k] = ep;
		    FREE (epn);
		    y1 = k + iymin;
		    dy = 1;
		    ++outp;
		}
		else {
		    x1n = ep -> x;
		    epn = ep -> next;
		    FREE (ep);
		    x2n = epn -> x;
		    if ((ep = epn -> next)) ++again;
		    edge_array[k] = ep;
		    FREE (epn);
		    if (x1 == x1n && x2 == x2n) {
			++dy;
		    }
		    else {
			do_exp_box (x1, x2, y1, y1 + dy);
			x1 = x1n;
			x2 = x2n;
			y1 = k + iymin;
			dy = 1;
			++outp;
		    }
		}
	    }
	    else if (outp) {
		outp = 0;
		do_exp_box (x1, x2, y1, y1 + dy);
	    }
	}
	if (outp) {
	    do_exp_box (x1, x2, y1, y1 + dy);
	}
    } while (again);

    FREE (edge_array);
}

/*
** insert an x value in the linked list for the y value
*/
Private void StoreEdge (long x, long y)
{
    register struct edge *ep, *epn;

    ALLOCPTR (epn, edge);

    epn -> x = x;

    ep = edge_array[y - iymin];
    if (!ep) {
	epn -> next = 0;
	edge_array[y - iymin] = epn;
    }
    else if (ep -> x >= x) {
	if (ep -> x == x) {
	    edge_array[y - iymin] = ep -> next;
	    FREE (epn);
	    FREE (ep);
	}
	else {
	    epn -> next = ep;
	    edge_array[y - iymin] = epn;
	}
    }
    else {
	while (ep -> next) {
	    if (ep -> next -> x >= x) {
		if (ep -> next -> x == x) {
		    FREE (epn);
		    epn = ep -> next;
		    ep -> next = epn -> next;
		    FREE (epn);
		}
		else {
		    epn -> next = ep -> next;
		    ep -> next = epn;
		}
		return;
	    }
	    ep = ep -> next;
	}
	epn -> next = 0;
	ep -> next = epn;
    }
}

Private void do_exp_box (long xl, long xr, long yb, long yt)
{
    register long i, j, dy;

    for (i = 0;;) {
	for (dy = j = 0;;) {
	    exp_box (xl, xr, yb + dy, yt + dy);
	    if (++j > Ny) break;
	    dy += Dy;
	}
	if (++i > Nx) break;
	xl += Dx;
	xr += Dx;
    }
}

Private void disc_circ (double dx, double dy, long nx, long ny)
{
    double xc, yc, r1, r2, a1, a2;
    double t_dx, t_dy, xl_d, xr_d;
    double x, y1t, y1b, y2t, y2b, r1_s, r2_s, x_s;
    double dy1, dy2, x_r2_a1, x_r2_a2;
    double tmp, xmin, xmax, rad_a, rad_a1, rad_a2;
    long   xl, xr, yb, yt;
    register int i, j, two, bg1;

    xc = samples * px[0];
    yc = samples * py[0];
    r1 = samples * px[1];
    r2 = samples * py[1];
    if (r1 <= r2 || r2 < 0) {
	msg_die ("circle: illegal radius");
    }

    a1 = a2 = 0; /* suppres uninitialized warning */
    x_r2_a1 = x_r2_a2 = 0; /* suppres uninitialized warning */
    rad_a1 = rad_a2 = 0; /* suppres uninitialized warning */
    y2t = y2b = 0; /* suppres uninitialized warning */

    c_c = 1;
    if (no_xy_pts > 3) {
	c_c = 0;
	a1 = px[3];
	a2 = py[3];
	if (a1 == 360) a1 = 0;
	if (a2 == 0) a2 = 360;
	if (a1 < 0 || a1 > 360 || a2 < 0 || a2 > 360 || a1 == a2) {
	    msg_die ("circle: illegal angles");
	}
	if (a1 == 0 && a2 == 360) c_c = 1;
    }

    if (c_c) {
	xmin = -r1;
	xmax =  r1;
    }
    else {
	rad_a1 = a1 * rad01;
	rad_a2 = a2 * rad01;
	xmin = r1 * cos (rad_a1);
	xmax = r1 * cos (rad_a2);
	if (xmax < xmin) {
	    tmp = xmin; xmin = xmax; xmax = tmp;
	}
	if (a1 <= 180 && (a2 >= 180 || a1 > a2)) xmin = -r1;
	else if (a2 >= 180 && a1 > a2) xmin = -r1;
	if (a1 == 0 || a2 == 360 || a1 > a2) xmax = r1;
	if (r2) {
	    x_r2_a1 = r2 * cos (rad_a1);
	    if (x_r2_a1 < xmin) xmin = x_r2_a1;
	    else if (x_r2_a1 > xmax) xmax = x_r2_a1;
	    x_r2_a2 = r2 * cos (rad_a2);
	    if (x_r2_a2 < xmin) xmin = x_r2_a2;
	    else if (x_r2_a2 > xmax) xmax = x_r2_a2;
	}
	else {
	    if (a1 >= 270 && (a2 > a1 || a2 <= 90)) xmin = 0;
	    else if (a1 <= 90 && a2 <= 90 && a2 > a1) xmin = 0;
	    if (a1 >= 90 && a2 <= 270 && a2 > a1) xmax = 0;
	}
    }

    r1_s = r1 * r1;
    r2_s = r2 * r2;

    xl_d = xc + xmin;
    for (x = -r1 + 0.5; x < xmax; x += 1) {
	if (x < xmin) continue;
	xr_d = xc + x + 0.5;
	x_s = x * x;
	dy1 = sqrt (r1_s - x_s);
	two = 0;

	if (c_c) {
	    y1t = yc + dy1;
	    y1b = yc - dy1;
	    if (r2 && x > -r2 && x < r2) {
		dy2 = sqrt (r2_s - x_s);
		y2t = yc + dy2;
		y2b = yc - dy2;
		++two;
	    }
	}
	else {
	    bg1 = 0;
	    rad_a = acos (x / r1);

	    if ((rad_a <= rad_a2 && rad_a >= rad_a1)
	     || (rad_a2 < rad_a1 && (rad_a <= rad_a2 || rad_a >= rad_a1))) {
		bg1 = 1;
		y1t = yc + dy1;
	    }
	    else if (x < 0) {
		if (r2 && x_r2_a1 < 0 && x > x_r2_a1)
		    y1t = yc - sqrt (r2_s - x_s);
		else
		    y1t = yc + x * tan (rad_a1);
	    }
	    else {
		if (r2 && x_r2_a2 > 0 && x < x_r2_a2)
		    y1t = yc - sqrt (r2_s - x_s);
		else
		    y1t = yc + x * tan (rad_a2);
	    }

	    rad_a = 4 * rad90 - rad_a;

	    if ((rad_a <= rad_a2 && rad_a >= rad_a1)
	     || (rad_a2 < rad_a1 && (rad_a <= rad_a2 || rad_a >= rad_a1))) {
		y1b = yc - dy1;

		if (bg1) {
		    if ((x > 0 && a1 < 90 && a2 > 270)
		     || (x < 0 && a2 < a1 && a2 > 90 && a1 < 270)) {
			if (x > 0) {
			    if (r2 && x < x_r2_a1)
				y2t = yc + sqrt (r2_s - x_s);
			    else
				y2t = yc + x * tan (rad_a1);

			    if (r2 && x < x_r2_a2)
				y2b = yc - sqrt (r2_s - x_s);
			    else
				y2b = yc + x * tan (rad_a2);
			}
			else {
			    if (r2 && x > x_r2_a2)
				y2t = yc + sqrt (r2_s - x_s);
			    else
				y2t = yc + x * tan (rad_a2);

			    if (r2 && x > x_r2_a1)
				y2b = yc - sqrt (r2_s - x_s);
			    else
				y2b = yc + x * tan (rad_a1);
			}
			++two;
		    }
		    else if (r2 && x > -r2 && x < r2) {
			dy2 = sqrt (r2_s - x_s);
			y2t = yc + dy2;
			y2b = yc - dy2;
			++two;
		    }
		}
		else if (r2 && x > -r2 && x < r2) {
		    dy2 = sqrt (r2_s - x_s);
		    y2t = yc + dy2;
		    y2b = yc - dy2;
		    if (y2t < y1t) ++two;
		}
	    }
	    else if (x < 0) {
		if (r2 && x_r2_a2 < 0 && x > x_r2_a2)
		    y1b = yc + sqrt (r2_s - x_s);
		else {
		    y1b = yc + x * tan (rad_a2);
		    if (r2 && x > -r2) {
			dy2 = sqrt (r2_s - x_s);
			y2t = yc + dy2;
			y2b = yc - dy2;
			if (y2t > y1b) ++two;
		    }
		}
	    }
	    else {
		if (r2 && x_r2_a1 > 0 && x < x_r2_a1)
		    y1b = yc + sqrt (r2_s - x_s);
		else {
		    y1b = yc + x * tan (rad_a1);
		    if (r2 && x < r2) {
			dy2 = sqrt (r2_s - x_s);
			y2t = yc + dy2;
			y2b = yc - dy2;
			if (y2t > y1b) ++two;
		    }
		}
	    }
	}

	if (two) {
	    for (i = 0; i <= nx; ++i) {
		t_dx = i * dx;
		xl = l_round (xl_d + t_dx);
		xr = l_round (xr_d + t_dx);
		for (j = 0; j <= ny; ++j) {
		    t_dy = j * dy;
		    yt = l_round (y2b + t_dy);
		    yb = l_round (y1b + t_dy);
		    if (yb < yt) exp_box (xl, xr, yb, yt);
		    yt = l_round (y1t + t_dy);
		    yb = l_round (y2t + t_dy);
		    if (yb < yt) exp_box (xl, xr, yb, yt);
		}
	    }
	}
	else {
	    for (i = 0; i <= nx; ++i) {
		t_dx = i * dx;
		xl = l_round (xl_d + t_dx);
		xr = l_round (xr_d + t_dx);
		for (j = 0; j <= ny; ++j) {
		    t_dy = j * dy;
		    yt = l_round (y1t + t_dy);
		    yb = l_round (y1b + t_dy);
		    if (yb < yt) exp_box (xl, xr, yb, yt);
		}
	    }
	}

	xl_d = xr_d;
    }
}

/*
** read the not orth. elements of the cell
** and put them in the "_nxx" files.
*/
void read_nor2 ()
{
    DM_STREAM *fp;
    double  xc, yc;
    double  a1, a2, a, a360, step, step1;
    double  dx, dy;
    double  la1, la2;
    double  tmp, width;
    double  d_x, d_y;
    double  dx1, dy1, dx2, dy2;
    double  c1xr, c1yr, a1xy, b1xy;
    double  c2xl, c2yl, c2xr, c2yr, a2xy, b2xy;
    register int i, j;
    int  nx, ny;

    a1 = a2 = 0; /* suppres uninitialized warning */
    la1 = la2 = 0; /* suppres uninitialized warning */
    dx1 = dy1 = 0; /* suppres uninitialized warning */
    a1xy = b1xy = 0; /* suppres uninitialized warning */

    fp = dmOpenStream (cellkey, "nor", "r");

    while (dmGetDesignData (fp, GEO_NOR_INI) > 0) {

	mask_no = gnor_ini.layer_no;
	no_xy_pts = gnor_ini.no_xy;

	elmt_no = gnor_ini.elmt;

	switch (elmt_no) {
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
	    if (width <= 0) msg_die ("wire: illegal width");

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
		    if (dy2 == 0) msg_die ("wire: incr. values are 0");
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
			msg_die ("wire: too big direction change");

		    tmp = dx2 * dy1 - dx1 * dy2;
		    if (tmp == 0) msg_die ("wire: no direction change");
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
	    gr1 = gnor_xy.x; /* outside radius */
	    gr2 = gnor_xy.y; /*  inside radius */
	    if (gr1 <= gr2 || gr2 < 0) {
		msg_die ("circle: illegal radius");
	    }
	    a360 = 360;
	    step = a360 / 32; /* default */
	    if (no_xy_pts > 2) {
		dmGetDesignData (fp, GEO_NOR_XY);
		if ((step = gnor_xy.x) >= 8) {
		    i = (int)step;
		    if (i % 8) i = (int)(a360 / step);
		}
		else if (step == 0) i = 32000; /* maximum */
		else i = (int)(a360 / step);
		if (i > 8) {
		    i /= 8; i *= 8; /* make it modulo 8 */
		    step = a360 / i;
		}
		if (step < 0.01 || step > 45) {
		    msg_die ("circle: illegal step angle");
		}
	    }
	    if (no_xy_pts > 3) {
		dmGetDesignData (fp, GEO_NOR_XY);
		a1 = gnor_xy.x;
		if ((a2 = gnor_xy.y) == 0) a2 = 360;
		if (a1 < 0 || a1 >= 360
		 || a2 <= 0 || a2 > 360 || a1 == a2) {
		    msg_die ("circle: illegal angles");
		}
		la1 = a1; la2 = a2;
		if (a1 >= a2) a1 -= 360;
		a360 = a2 - a1;
	    }

	    j = 0;
	    if (a360 == 360) { /* complete circle */
		c_c = 1;
		la1 = la2 = 0;
		i = (int)(a360 / step); /* nr_of_edges */
		if (gr2 > 0) i = 2 * i + 2;
		if (i > pt_size) newpxpy (i + 4);

		px[0] = xc + gr1;
		py[0] = yc;
		for (a = step; a < 360 - 0.005; a += step) {
		    px[++j] = xc + gr1 * cos (a * rad01);
		    py[j] = yc + gr1 * sin (a * rad01);
		}
		if (gr2 > 0) { /* inner radius */
		    px[++j] = px[0];
		    py[j] = py[0];
		    px[++j] = xc + gr2;
		    py[j] = py[0];
		    for (a = 360 - step; a > 0.005; a -= step) {
			px[++j] = xc + gr2 * cos (a * rad01);
			py[j] = yc + gr2 * sin (a * rad01);
		    }
		    px[++j] = xc + gr2;
		    py[j] = py[0];
		}
	    }
	    else { /* not a complete circle */
		c_c = 0;
		for (step1 = 270; step1 > a1;) step1 -= 90;
		while (step1 <= a1) step1 += step;
		step1 -= a1; /* first step angle > 0 */
		i = (int)((a360 - step1) / step);
		if (i * step + step1 < a360) ++i;
		i += 2; /* nr_of_edges + 1 */
		if (gr2 > 0) i *= 2;
		else if (a360 != 180) ++i;
		if (i > pt_size) newpxpy (i + 4);

		px[0] = xc + gr1 * cos (a1 * rad01);
		py[0] = yc + gr1 * sin (a1 * rad01);
		a2 -= 0.005;
		for (a = a1 + step1; a < a2; a += step) {
		    px[++j] = xc + gr1 * cos (a * rad01);
		    py[j] = yc + gr1 * sin (a * rad01);
		}
		a2 += 0.005;
		px[++j] = xc + gr1 * cos (a2 * rad01);
		py[j] = yc + gr1 * sin (a2 * rad01);

		if (gr2 > 0) { /* inner radius */
		    px[++j] = xc + gr2 * cos (a2 * rad01);
		    py[j] = yc + gr2 * sin (a2 * rad01);
		    a1 += 0.005;
		    for (a -= step; a > a1; a -= step) {
			px[++j] = xc + gr2 * cos (a * rad01);
			py[j] = yc + gr2 * sin (a * rad01);
		    }
		    a1 -= 0.005;
		    px[++j] = xc + gr2 * cos (a1 * rad01);
		    py[j] = yc + gr2 * sin (a1 * rad01);
		}
		else if (a360 != 180) {
		    px[++j] = xc;
		    py[j] = yc;
		}
	    }
	    no_xy_pts = ++j;
	    px[no_xy_pts] = xc;
	    py[no_xy_pts] = yc;
	    break;
	default:
	    msg_die ("unknown nor element");
	}

	nx = gnor_ini.nx + 1;
	ny = gnor_ini.ny + 1;

	for (i = 0; i < nx; ++i) {
	    dx = i * gnor_ini.dx;
	    for (j = 0; j < ny; ++j) {
		dy = j * gnor_ini.dy;
		exp_poly (dx, dy, la1, la2);
	    }
	}
    }

    dmCloseStream (fp, COMPLETE);
}

/*
** expand poly to the root level
** of the mc-tree according to mc-parameters
*/
Private void exp_poly (double dx, double dy, double a1, double a2)
{
    register long *m;
    register int  k;
    struct tmtx *tm;
    double la1, la2, ax, ay;
    int mir, rot;

    for (tm = tm_p; tm; tm = tm -> tm_next)
    if (tm -> allow_all || (mask_no < 64 && (tm -> allowmasks & (1LL<<mask_no)))) {

	m = tm -> mtx;

	if (elmt_no == CIRCLE_NOR) {
	    la1 = a1;
	    la2 = a2;
	    if (!c_c) { /* not a complete circle */
		mir = 0;
		if (m[0] == 0) {
		    if (m[3] > 0) {
			rot = 90;
			if (m[1] > 0) ++mir;
		    }
		    else {
			rot = 270;
			if (m[1] < 0) ++mir;
		    }
		}
		else {
		    if (m[0] > 0) {
			rot = 0;
			if (m[4] < 0) ++mir;
		    }
		    else {
			rot = 180;
			if (m[4] > 0) ++mir;
		    }
		}
		if (mir) {
		    la1 = 360 - a2;
		    la2 = 360 - a1;
		}
		if (rot) {
		    if ((la1 += rot) >= 360) la1 -= 360;
		    if ((la2 += rot) >  360) la2 -= 360;
		}
	    }

	    ax = px[no_xy_pts] + dx;
	    ay = py[no_xy_pts] + dy;
	    gnxx_ini.xc = m[0] * ax + m[1] * ay + m[2];
	    gnxx_ini.yc = m[3] * ax + m[4] * ay + m[5];
	    gnxx_ini.r1 = gr1;
	    gnxx_ini.r2 = gr2;
	    gnxx_ini.a1 = la1;
	    gnxx_ini.a2 = la2;
	}

	gnxx_ini.elmt = elmt_no;
	gnxx_ini.no_xy = no_xy_pts;
	if (!fp_nxx[mask_no]) open_nxx (mask_no);
	dmPutDesignData (fp_nxx[mask_no], GEO_NXX_INI);

	if (m[0]) {
	    ax = m[0] * dx + m[2];
	    ay = m[4] * dy + m[5];
	    for (k = 0; k < no_xy_pts; ++k) {
		gnxx_xy.x = m[0] * px[k] + ax;
		gnxx_xy.y = m[4] * py[k] + ay;
		dmPutDesignData (fp_nxx[mask_no], GEO_NXX_XY);
	    }
	}
	else {
	    ay = m[1] * dy + m[2];
	    ax = m[3] * dx + m[5];
	    for (k = 0; k < no_xy_pts; ++k) {
		gnxx_xy.x = m[1] * py[k] + ay;
		gnxx_xy.y = m[3] * px[k] + ax;
		dmPutDesignData (fp_nxx[mask_no], GEO_NXX_XY);
	    }
	}
    }
}

Private void newpxpy (int size)
{
    FREE (px);
    FREE (py);
    pt_size = size;
    ALLOCARR (px, pt_size, double);
    ALLOCARR (py, pt_size, double);
}

Private void msg_die (char *msg)
{
    P_E "%s: %s\n", argv0, msg);
    die ();
}
