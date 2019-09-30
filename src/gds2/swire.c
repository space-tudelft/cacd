/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
 *	R. Paulussen
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
#include <stdio.h>
#include <stdlib.h>
#include "src/gds2/defs.h"

/* should be defined in defs.h instead of in incl.h */
#define Round(x) (((x) > 0) ? ((x)+0.5) : ((x)-0.5))

/* sqrt (2.0) -> M_SQRT2  */
#ifndef M_SQRT2
#define M_SQRT2         1.41421356237309504880
#endif

extern int  width;
extern int  nr_coor;		/* Note an X, Y pair is counted as 2 */
extern int  lay_code;
extern int  pathtype;
extern int  int_val[];
extern double   resolution;

static int  *ese_int_val = NULL;
static int  ese_int_val_size = 0;
static int  ese_nr_coor;
static int  ese_width;

void pr_exit (int mode, int err_no, char *cs);
void stop_or_end_of_wire (int x1, int Y1, int x2, int y2, double *dx1, double *dy1, double *dx2, double *dy2);
int segment_of_wire (int x1, int Y1, int x2, int y2, int x3, int y3, double *dx1, double *dy1,
                        double *dx2, double *dy2, double *dx3, double *dy3, double *dx4, double *dy4);
double distance (double x1, double Y1, double x2, double y2);
void put_coor_on_grid (double *dx, double *dy);
int do_swire (void);
int check_wire (int pts_x[], int pts_y[], int nr_pts, int mode45, int incr);
void add_wire (int stack_x[], int stack_y[], int width, int npoints, int non_orth, int wire_ext);
void add_wire_part (int x1, int y1, int x2, int y2, int ls, int rs);

void quadragon (int layer_code, double x1, double Y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
    lay_code = layer_code;

    put_coor_on_grid (&x1, &Y1);
    put_coor_on_grid (&x2, &y2);
    put_coor_on_grid (&x3, &y3);
    put_coor_on_grid (&x4, &y4);

    if (!(x1 == x2 && Y1 == y2 && x3 == x4 && y3 == y4)
	&& !(x1 == x4 && Y1 == y4 && x2 == x3 && y2 == y3)) {
	/* Only write quadragon if its area is larger than zero. */
	_quadragon (x1, Y1, x2, y2, x3, y3, x4, y4);
    }
}

int ese_swire ()
{
    int     i, x1, Y1, x2, y2, x3, y3;
    double  dx1, dy1, dx2, dy2, dx3, dy3, dx4, dy4, dx5, dy5, dx6, dy6;

 /* copy array */
    ese_nr_coor = nr_coor;
    if (ese_nr_coor > ese_int_val_size) {
        ese_int_val_size = ese_nr_coor;
        /* minimum size is 512 */
        if (ese_int_val_size <= 512) ese_int_val_size = 512;
        if (!(ese_int_val = (int *)realloc (ese_int_val,
                                     sizeof (int) * ese_int_val_size)))
            pr_exit (A, 17, 0);
    }
    for (i = 0; i < ese_nr_coor; i++) {
	ese_int_val[i] = (int) Round (int_val[i] * resolution);
    }
    ese_width = (int) Round (width * resolution);

 /* Check for 45 degree and progress */
    for (i = 2; i < ese_nr_coor; i += 2) {
	x1 = ese_int_val[i - 2];
	Y1 = ese_int_val[i - 1];
	x2 = ese_int_val[i];
	y2 = ese_int_val[i + 1];

	if (x1 != x2 && Y1 != y2 && x1 - x2 != Y1 - y2 && x1 - x2 != y2 - Y1) {
	    PE "*** Error: Not a 45 degree angle design\n");
	/* pr_exit( W, 35, 0 ); */
	    return (1);
	}
	if (x1 == x2 && Y1 == y2) {
	    PE "*** Error: No increment in wire\n");
	/* pr_exit( W, 24, 0 ); */
	    return (1);
	}
    }

    x1 = ese_int_val[0];
    Y1 = ese_int_val[1];
    x2 = ese_int_val[2];
    y2 = ese_int_val[3];

    stop_or_end_of_wire (x1, Y1, x2, y2, &dx1, &dy1, &dx2, &dy2);

    for (i = 4; i < ese_nr_coor; i += 2) {
	x1 = ese_int_val[i - 4];
	Y1 = ese_int_val[i - 3];
	x2 = ese_int_val[i - 2];
	y2 = ese_int_val[i - 1];
	x3 = ese_int_val[i];
	y3 = ese_int_val[i + 1];

	if (pathtype == 1) {
	    stop_or_end_of_wire (x2, y2, x1, Y1, &dx3, &dy3, &dx4, &dy4);
	    quadragon (lay_code, dx1, dy1, dx2, dy2, dx3, dy3, dx4, dy4);
	    stop_or_end_of_wire (x2, y2, x3, y3, &dx1, &dy1, &dx2, &dy2);
	}
	else {
	/* If there is a bend in the wire */
	    if (segment_of_wire (x1, Y1, x2, y2, x3, y3,
		    &dx3, &dy3, &dx4, &dy4, &dx5, &dy5, &dx6, &dy6)) {
		quadragon (lay_code, dx1, dy1, dx2, dy2, dx3, dy3, dx4, dy4);
		dx2 = dx5;
		dy2 = dy5;
		dx1 = dx6;
		dy1 = dy6;
	    }
	}
    }

    x1 = ese_int_val[ese_nr_coor - 4];
    Y1 = ese_int_val[ese_nr_coor - 3];
    x2 = ese_int_val[ese_nr_coor - 2];
    y2 = ese_int_val[ese_nr_coor - 1];

    stop_or_end_of_wire (x2, y2, x1, Y1, &dx3, &dy3, &dx4, &dy4);
    quadragon (lay_code, dx1, dy1, dx2, dy2, dx3, dy3, dx4, dy4);
    return (0);
}

void stop_or_end_of_wire (int x1, int Y1, int x2, int y2, double *dx1, double *dy1, double *dx2, double *dy2)
{
    double  dx3, dy3, dx4, dy4;
    double  vx = 0.0, vy = 0.0;
    double  wx, wy;
    double  length;

 /* Calculate the basic vector direction */
    if (x2 - x1) vx = (x2 - x1) / abs (x2 - x1);
    if (y2 - Y1) vy = (y2 - Y1) / abs (y2 - Y1);

 /* Make the length of vector 1 */
    length = sqrt (vx * vx + vy * vy);
    vx /= length;
    vy /= length;

 /* Offset plus basic vector rotated 90 degree's */
    *dx1 = x1 - vy * ese_width / 2;
    *dy1 = Y1 + vx * ese_width / 2;

 /* Offset plus basic vector rotated 270 degree's */
    *dx2 = x1 + vy * ese_width / 2;
    *dy2 = Y1 - vx * ese_width / 2;

    if (pathtype == 1) {
	wx = vx * ese_width / (2 * M_SQRT2 + 2);
	wy = vy * ese_width / (2 * M_SQRT2 + 2);

	*dx1 -= wx;
	*dy1 -= wy;
	*dx2 -= wx;
	*dy2 -= wy;

	wx *= M_SQRT2;
	wy *= M_SQRT2;

	dx3 = *dx2 - wx - wy;
	dy3 = *dy2 - wy + wx;
	dx4 = *dx1 - wx + wy;
	dy4 = *dy1 - wy - wx;

	quadragon (lay_code, *dx1, *dy1, *dx2, *dy2, dx3, dy3, dx4, dy4);
    }
    else
	if (pathtype == 2) {
	    *dx1 -= (vx * ese_width / 2);
	    *dy1 -= (vy * ese_width / 2);
	    *dx2 -= (vx * ese_width / 2);
	    *dy2 -= (vy * ese_width / 2);
	}
}

int segment_of_wire (int x1, int Y1, int x2, int y2, int x3, int y3,
	double *dx1, double *dy1, double *dx2, double *dy2,
	double *dx3, double *dy3, double *dx4, double *dy4)
{
    double  vx  = 0.0, vy  = 0.0;
    double  vx1 = 0.0, vy1 = 0.0;
    double  vx2 = 0.0, vy2 = 0.0;
    double  length;

 /* Calculate the basic vector direction */
    if (x1 - x2) vx1 = (x1 - x2) / abs (x1 - x2);
    if (Y1 - y2) vy1 = (Y1 - y2) / abs (Y1 - y2);

 /* Make the length of vector 1 */
    length = sqrt (vx1 * vx1 + vy1 * vy1);
    vx1 /= length;
    vy1 /= length;

 /* Calculate the basic vector direction */
    if (x3 - x2) vx2 = (x3 - x2) / abs (x3 - x2);
    if (y3 - y2) vy2 = (y3 - y2) / abs (y3 - y2);

 /* Make the length of vector 1 */
    length = sqrt (vx2 * vx2 + vy2 * vy2);
    vx2 /= length;
    vy2 /= length;

    vx = vx1 + vx2;
    vy = vy1 + vy2;

 /* There is no bend */
    if (vx == 0.0 && vy == 0.0) return (0);

 /* Give it the proper lenght this is not 1 !!! */
    length = (vx1 * vx2 + vy1 * vy2) * (vx1 * vx2 + vy1 * vy2)
					* (M_SQRT2 - 1) * 2.0 + 1.0;
    vx *= length;
    vy *= length;

 /* Corner is offsett plus plus basic vector */
    *dx1 = x2 + vx * ese_width / 2;
    *dy1 = y2 + vy * ese_width / 2;
    *dx3 = x2 - vx * ese_width / 2;
    *dy3 = y2 - vy * ese_width / 2;

 /* The direction of the corners from the offset */
    if ((vx1 * vy2 - vy1 * vx2) > 0.0) {
	*dx2 = *dx1 + vy1 * ese_width;
	*dy2 = *dy1 - vx1 * ese_width;
	*dx4 = *dx1 - vy2 * ese_width;
	*dy4 = *dy1 + vx2 * ese_width;

	quadragon (lay_code, *dx1, *dy1, *dx2, *dy2, *dx3, *dy3, *dx4, *dy4);

	*dx3 = *dx1;
	*dy3 = *dy1;
    }
    else {
	*dx2 = *dx1 - vy1 * ese_width;
	*dy2 = *dy1 + vx1 * ese_width;
	*dx4 = *dx1 + vy2 * ese_width;
	*dy4 = *dy1 - vx2 * ese_width;

	quadragon (lay_code, *dx1, *dy1, *dx2, *dy2, *dx3, *dy3, *dx4, *dy4);

	*dx3 = *dx4;
	*dy3 = *dy4;
	*dx4 = *dx1;
	*dy4 = *dy1;
	*dx1 = *dx2;
	*dy1 = *dy2;
	*dx2 = *dx4;
	*dy2 = *dy4;
    }
    return (1); /* Succesfull return */
}

double distance (double x1, double Y1, double x2, double y2)
{
    return (sqrt ((x2 - x1) * (x2 - x1) + (y2 - Y1) * (y2 - Y1)));
}

void put_coor_on_grid (double *dx, double *dy)
{
    double  dis = 2.0;          /* two is a big number here */
    double  tx, ty;

/* This is the area where the coordinate is in */

    int     Xmin;
    int     Ymin;
    int     Xmax;
    int     Ymax;

    double dX = floor (*dx * 4.0);
    double dY = floor (*dy * 4.0);

    Xmin = (int) dX;
    Ymin = (int) dY;
    Xmax = (int) dX + 1;
    Ymax = (int) dY + 1;

    tx = ty = 0; /* for compiler uninit warning */

/* check if coordinate is OK */
    if ((abs (Xmin) & 1) == (abs (Ymin) & 1) &&
	    dis > distance ((double) Xmin, (double) Ymin, *dx * 4, *dy * 4)) {
	dis = distance ((double) Xmin, (double) Ymin, *dx * 4, *dy * 4);
	tx = Xmin / 4.0;
	ty = Ymin / 4.0;
    }
    if ((abs (Xmin) & 1) == (abs (Ymax) & 1) &&
	    dis > distance ((double) Xmin, (double) Ymax, *dx * 4, *dy * 4)) {
	dis = distance ((double) Xmin, (double) Ymax, *dx * 4, *dy * 4);
	tx = Xmin / 4.0;
	ty = Ymax / 4.0;
    }
    if ((abs (Xmax) & 1) == (abs (Ymin) & 1) &&
	    dis > distance ((double) Xmax, (double) Ymin, *dx * 4, *dy * 4)) {
	dis = distance ((double) Xmax, (double) Ymin, *dx * 4, *dy * 4);
	tx = Xmax / 4.0;
	ty = Ymin / 4.0;
    }
    if ((abs (Xmax) & 1) == (abs (Ymax) & 1) &&
	    dis > distance ((double) Xmax, (double) Ymax, *dx * 4, *dy * 4)) {
	dis = distance ((double) Xmax, (double) Ymax, *dx * 4, *dy * 4);
	tx = Xmax / 4.0;
	ty = Ymax / 4.0;
    }
    /* ASSERT (dis < 2.0); */

    *dx = tx;
    *dy = ty;
}

/*
** These routines are based on the routines from dali,
** XDraw-calls changed into database calls (FB).
*/

#define MAX_PTS 128
#define QUAD_LAMBDA 4
/* #define QUAD_LAMBDA 1 */
#define Sign(a) (((a) >= 0) ? (((a) == 0) ? 0 : 1) : -1)
#define Min(a,b) ((a)<(b)?(a):(b))

extern int mode45;
static int Firsttime = 1;

int do_swire ()
{
    int i, nr_points;
    int points_x[MAX_PTS];
    int points_y[MAX_PTS];
    int wire_ext;
    int wire_width;
    double grid_res;
    double d;

    /* ASSERT ((nr_coor > 3) && ((nr_coor % 2) == 0)); */

    /*
    ** Originally the 'ceil' statements below were 'Round' statements,
    ** however, this can endanger the 45 degree angles that are in the
    ** original description if one side of the line has negative
    ** coordinates and the other side positive ones. (FB)
    **
    ** Another thing is that we can use the internal grid that is used,
    ** i.e. coordinates * QUAD_LAMBDA should be integers (???)
    */

    grid_res = resolution * QUAD_LAMBDA;

    nr_points = nr_coor / 2;
    for (i = 0; i < nr_points; i++) {
        double tmp_val;
        tmp_val = ceil (int_val[(2*i)] * grid_res);
        if (tmp_val > MAX32 || tmp_val < MIN32) {
            fprintf (stderr, "**** Error: scaled integers become too large\n");
            fprintf (stderr, "**** Suggestion: decrease lambda with clambda\n");
            return (1);
        }
	points_x[i] = (int) tmp_val;
        tmp_val = ceil (int_val[(2*i+1)] * grid_res);
        if (tmp_val > MAX32 || tmp_val < MIN32) {
            fprintf (stderr, "**** Error: scaled integers become too large\n");
            fprintf (stderr, "**** Suggestion: decrease lambda with clambda\n");
            return (1);
        }
	points_y[i] = (int) tmp_val;
    }
    wire_ext = (pathtype == 2 ? 1 : 0);
    d = ceil (width * grid_res);
    wire_width = (int) d;

    /* check wire path conform 45-degree and points incremental */
    if (!(check_wire (points_x, points_y, nr_points, mode45, 1))) return (1);

    add_wire (points_x, points_y, nr_points, wire_width, 0, wire_ext);

    return (0);
}

int check_wire (int pts_x[], int pts_y[], int nr_pts, int mode45, int incr)
{
    int i, x1, y1, x2, y2;

    for (i = 1; i < nr_pts; i++) {
        x1 = pts_x[i-1];
        y1 = pts_y[i-1];
        x2 = pts_x[i];
        y2 = pts_y[i];

        if (mode45) {
            /* Check also on 45-degree angles */
            if (x1 != x2 && y1 != y2 && Abs (x1 - x2) != Abs (y1 - y2)) {
                fprintf (stderr, "*** Error: Not a 45 degree angle design\n");
                return (0);
            }
        }
        /* Always chech on incrementality */
        if (incr) {
            if (x1 == x2 && y1 == y2) {
                fprintf (stderr, "*** Error: No increment in wire\n");
                return (0);
            }
        }
    }
    return (1);
}

/*
** Build a wire out of a list of points.
** INPUT: the wire width, an array with center points
** and the number of center points.
*/
void add_wire (int stack_x[], int stack_y[], int npoints, int width, int non_orth, int wire_ext)
{
    int dx, dy;
    int ext1 = 0, ext2 = 0;
    int px, py, qx, qy;
    int px2, py2, qx2, qy2;
    int rvprev, rv = 0;
    int i, sx, sy;
    int sxp = 0, syp = 0;

    /* w == width from center of wire to surroundings of wire */
    int w = width / 2;
    int nw = (int) ((double) w / sqrt(2.0) + 1.0);
    int lw;

    if (non_orth) non_orth = 1;

    /* extension of wires at the start and stop coordinates */
    if (wire_ext) {
        ext1 = w;
    }
    else if (!non_orth) {
        ext1 = ((width / QUAD_LAMBDA) % 2 == 0) ? 0 : QUAD_LAMBDA / 2;
    }

    Firsttime = 1;
    rvprev = -100;      /* prevents gap fill in first loop pass */

    for (i = 0; i < npoints - 1; ++i) {
        dx = stack_x[i + 1] - stack_x[i];
        dy = stack_y[i + 1] - stack_y[i];
        sx = Sign (dx);
        sy = Sign (dy);

        /* if (!non_orth) ASSERT (sx * sy == 0); */

        if (dy == 0) rv = (dx >= 0) ? 0 : 4;
        if (dx == 0) rv = (dy >= 0) ? 2 : 6;
        if (dx && dy) {
            if (dx > 0) rv = (dy > 0) ? 1 : 7;
            if (dx < 0) rv = (dy > 0) ? 3 : 5;
        }
        px = stack_x[i];
        py = stack_y[i];
        qx = stack_x[i + 1];
        qy = stack_y[i + 1];

        if (i == npoints - 2) { /* last point */
            if (wire_ext) {
                ext2 = w;
            }
            else if (!non_orth) {
                ext2 = ((width / QUAD_LAMBDA) % 2 == 0) ? 0 : QUAD_LAMBDA / 2;
            }
        }

        /*
        ** Calculate the wire parts.
        ** The slanting wire-parts are generated by
        ** three trapezoids two of which are triangles.
        ** The formulas for the longer parts are:
        ** (px + 3 * sxnw, py + synw, px - sxnw, py - synw, -sy, sy)
        ** (qx + sxnw, qy - synw, px - sxnw, py + synw, sx*sy, sx*sy)
        ** (qx + sxnw, qy + synw, qx - 3 * sxnw, qy - synw, sy, -sy)
        **
        ** To connect well with the adjoining parts we take:
        **  px' = px - sx * (w - nw)       py' = py - sy * (w - nw)
        **  qx' = qx + sx * (w - nw)       qy' = qy + sy * (w - nw)
        */
        switch (rv) {
        case 0:
            add_wire_part (px - ext1, py - w, qx + ext2, qy + w, 0, 0);
            break;
        case 2:
            add_wire_part (px - w, py - ext1, qx + w, qy + ext2, 0, 0);
            break;
        case 4:
            add_wire_part (qx - ext2, py - w, px + ext1, qy + w, 0, 0);
            break;
        case 6:
            add_wire_part (px - w, qy - ext2, qx + w, py + ext1, 0, 0);
            break;
        case 1:
        case 3:
        case 5:
        case 7:
            if (i == 0) { /* first point */
                if (wire_ext) {
                    px2 = px - sx * nw;
                    py2 = py - sy * nw;
                }
                else {
                    px2 = px;
                    py2 = py;
                }
            }
            else {
                px2 = px - sx * (w - nw);
                py2 = py - sy * (w - nw);
            }
            if (i == npoints - 2) { /* last point */
                if (wire_ext) {
                    qx2 = qx + sx * nw;
                    qy2 = qy + sy * nw;
                }
                else {
                    qx2 = qx;
                    qy2 = qy;
                }
            }
            else {
                qx2 = qx + sx * (w - nw);
                qy2 = qy + sy * (w - nw);
            }

            lw = sy * (qy2 - py2);
            if (lw > 2 * nw) { /* 'long' part */
                add_wire_part (px2 + 3 * sx * nw, py2 + sy * nw,
                                px2 - sx * nw, py2 - sy * nw, -sy, sy);
                add_wire_part (qx2 + sx * nw, qy2 - sy * nw,
                                px2 - sx * nw, py2 + sy * nw, sx*sy, sx*sy);
                add_wire_part (qx2 + sx * nw, qy2 + sy * nw,
                                qx2 - 3 * sx * nw, qy2 - sy * nw, sy, -sy);
            }
            else {             /* length smaller than width */
                /* we now use p?2 and q?2 for two corner points */
                px2 += sx * (nw + lw);
                py2 -= sy * (nw - lw);
                qx2 -= sx * (nw + lw);
                qy2 += sy * (nw - lw);

                add_wire_part (px2, py2, px2 - 2 * sx * lw, py2 - sy * lw,
                                                            -sy, sy);
                if (py2 != qy2) {
                    add_wire_part (px2, py2, qx2, qy2, -sx * sy, -sx * sy);
                }
                add_wire_part (qx2 + 2 * sx * lw, qy2 + sy * lw, qx2, qy2,
                                                            sy, -sy);
            }
        }

        /*
        ** Now we have to create small triangles to fill up
        ** the gaps. Several cases have to be distinguished.
        */
        if (Abs (rvprev - rv) == 1 || Abs (rvprev - rv) == 7) {
            if (rv == 0 || rv == 2 || rv == 4 || rv == 6) {
                /*
                ** From slanting to orthogonal, pretend reverse case
                ** sxp and syp have been set during previous loop pass.
                */
                sxp = -sxp;
                syp = -syp;
            }
            else {
                sxp = sx;
                syp = sy;
            }
            /* ASSERT (sxp != 0 && syp != 0); */
            if (rvprev == 0 || rvprev == 4 || rv == 0 || rv == 4) {
                add_wire_part (px + sxp * (2 * nw - w), py - syp * w,
                        px, py - syp * 2 * (w - nw),
                        (sxp < 0) ? (-sxp * syp) : 0,
                        (sxp > 0) ? (-sxp * syp) : 0);
            }
            else {
                /* ASSERT (rvprev == 2 || rvprev == 6 || rv == 2 || rv == 6); */
                add_wire_part (px - sxp * w, py + syp * (2 * nw - w),
                        px - sxp * 2 * (w - nw), py,
                        (sxp < 0) ? (-sxp * syp) : 0,
                        (sxp > 0) ? (-sxp * syp) : 0);
            }
        }
        else if (Abs (rvprev - rv) == 3 || Abs (rvprev - rv) == 5) {
            if (rv == 0 || rv == 2 || rv == 4 || rv == 6) {
                /* from slanting to orthogonal, pretend reverse case */
                sxp = -sxp;
                syp = -syp;
            }
            else {
                sxp = sx;
                syp = sy;
            }
            /* ASSERT (sxp != 0 && syp != 0); */
            /* add triangle, only depends on slanting part */
            add_wire_part (px - sxp * (w - 2 * nw), py - syp * w,
                    px - sxp * w, py - syp * (w - 2 * nw),
                    (sxp < 0) ? (-sxp * syp) : 0,
                    (sxp > 0) ? (-sxp * syp) : 0);
        }
        else if ((Abs (rvprev - rv) == 2 || Abs (rvprev - rv) == 6)
                    && (rv == 0 || rv == 2 || rv == 4 || rv == 6)) {
            /* 90 degrees angle between orthogonal wire parts */
            if (rvprev == 2 || rvprev == 6) {
                /* from vertical to horizontal, pretend reverse case */
                sxp = -sx;
                syp = -syp;
            }
            else {
                sxp = sxp;
                syp = sy;
            }
            /* ASSERT (sxp != 0 && syp != 0); */
            /* fill with box if orth, triangle if non_orth */
            add_wire_part (px + sxp * w, py, px, py - syp * w,
                                    (sxp < 0) ? (non_orth * sxp * syp) : 0,
                                    (sxp > 0) ? (non_orth * sxp * syp) : 0);
        }
        else if ((Abs (rvprev - rv) == 2 || Abs (rvprev - rv) == 6)
                    && (rv == 1 || rv == 3 || rv == 5 || rv == 7)) {
            /* 90 degrees angle between non-orthogonal wire parts */
            /* ASSERT (sx != 0 && sy != 0); */
            if (sy == -syp) {   /* triangle at bottom or top side */
                add_wire_part (px - w + 2 * nw, py - sy * w,
                        px + w - 2 * nw, py - sy * 2 * (w - nw), sy, -sy);
            }
            else {      /* fill left or right side with small box */
                add_wire_part (px - sx * w, py - w + 2 * nw,
                        px - sx * 2 * (w - nw), py + w - 2 * nw, 0, 0);
            }
        }
        rvprev = rv;
        sxp = sx;
        syp = sy;
        ext1 = 0;
    }
}

void add_wire_part (int x1, int y1, int x2, int y2, int ls, int rs)
{
    int nx1, ny1, nx2, ny2, tmp, step;
    int dx1, dy1, dx2, dy2, dx3, dy3, dx4, dy4;

    nx1 = x1;
    ny1 = y1;
    nx2 = x2;
    ny2 = y2;

    /* Make sure (x1,y1) is bottom left, and (x2,y2) top right */
    if (nx2 < nx1) {
	tmp = nx1; nx1 = nx2; nx2 = tmp;
    }
    if (y2 < y1) {
	tmp = ny1; ny1 = ny2; ny2 = tmp;
    }

#ifdef DEBUG
    fprintf (stderr, "Box: %d %d %d %d ls=%d rs=%d\n",
             nx1, ny1, nx2, ny2, ls, rs);
#endif

    /*
    ** From the encoding of ls and rs (left-slant and right-slant),
    ** determine the other points of the polygon, which can be either
    ** a triangle or a trapezoid (quadragon).
    */

    step = Min (Abs (nx2 - nx1), Abs (ny2 - ny1));

    if (ls == -1) {
	dx2 = nx1; dx1 = dx2 + step;
	dy2 = ny2; dy1 = dy2 - step;
    }
    else if (ls == 0) {
	dx1 = nx1; dx2 = nx1;
	dy1 = ny1; dy2 = ny2;
    }
    else {
	/* ASSERT (ls == 1); */
	dx1 = nx1; dx2 = dx1 + step;
	dy1 = ny1; dy2 = dy1 + step;
    }

    if (rs == -1) {
	dx4 = nx2; dx3 = dx4 - step;
	dy4 = ny1; dy3 = dy4 + step;
    }
    else if (rs == 0) {
	dx3 = nx2; dx4 = nx2;
	dy3 = ny2; dy4 = ny1;
    }
    else {
	/* ASSERT (rs == 1); */
	dx3 = nx2; dx4 = dx3 - step;
	dy3 = ny2; dy4 = dy3 - step;
    }

#ifdef DEBUG
    {
        int pts_x[4], pts_y[4];

        pts_x[0] = dx1; pts_x[1] = dx2; pts_x[2] = dx3; pts_x[3] = dx4;
        pts_y[0] = dy1; pts_y[1] = dy2; pts_y[2] = dy3; pts_y[3] = dy4;

        check_wire (pts_x, pts_y, 4, mode45, 0);

	fprintf (stderr, "Qua: %d %d %d %d %d %d %d %d, s=%d\n",
                 dx1, dy1, dx2, dy2, dx3, dy3, dx4, dy4, step);
	fprintf (stderr, "Qua: %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f\n",
                 ((double) dx1 / QUAD_LAMBDA), ((double) dy1 / QUAD_LAMBDA),
                 ((double) dx2 / QUAD_LAMBDA), ((double) dy2 / QUAD_LAMBDA),
                 ((double) dx3 / QUAD_LAMBDA), ((double) dy3 / QUAD_LAMBDA),
                 ((double) dx4 / QUAD_LAMBDA), ((double) dy4 / QUAD_LAMBDA));
    }
#endif

    /*
    ** Recorrect the coordinates with the QUAD_LAMBDA factor and
    ** store polygon bounding box and update cell bounding box
    */
    _quadragon (((double) dx1 / QUAD_LAMBDA), ((double) dy1 / QUAD_LAMBDA),
                ((double) dx2 / QUAD_LAMBDA), ((double) dy2 / QUAD_LAMBDA),
                ((double) dx3 / QUAD_LAMBDA), ((double) dy3 / QUAD_LAMBDA),
                ((double) dx4 / QUAD_LAMBDA), ((double) dy4 / QUAD_LAMBDA));
}
