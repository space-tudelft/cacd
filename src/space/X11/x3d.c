/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include <stdio.h>
#include <math.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"

#include <X11/Intrinsic.h>
#include "src/space/X11/pane.h"
#include "src/space/include/tile.h"
#include "src/space/X11/extern.h"

#define m3 my_canvas -> matrix3
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern canvas_t * my_canvas;
extern int camParallel;
extern double camDistance;

void X3dSetViewReferencePoint (wcoor x, wcoor y, wcoor z)
{
    my_canvas -> view_xr = x;
    my_canvas -> view_yr = y;
    my_canvas -> view_zr = z;
}

void X3dProject (wcoor x, wcoor y, wcoor z, wcoor *x_return, wcoor *y_return)
{
    wcoor xr, yr, zr, z_dist, pf;

    xr = x - my_canvas -> view_xr;
    yr = y - my_canvas -> view_yr;
    zr = z - my_canvas -> view_zr;

    /* X3dViewplaneTransform */
    *x_return = xr * m3[0][0] + yr * m3[1][0];
    *y_return = xr * m3[0][1] + yr * m3[1][1] + zr * m3[2][1];

    if (!camParallel) { /* X3dPerspectiveTransform */
	z = xr * m3[0][2] + yr * m3[1][2] + zr * m3[2][2];
	z_dist = (camDistance + 0.5) * my_canvas -> view_distance;
	if (fabs ((double) (z_dist - z)) < ROUND_OFF) pf = VERY_LARGE;
	else pf = z_dist / (z_dist - z);
	*x_return *= pf;
	*y_return *= pf;
    }
}

void X3dProjectReverse (wcoor x, wcoor y, wcoor z, wcoor *x_return, wcoor *y_return)
{
    wcoor xr, yr, zr, z_dist, nm;
    wcoor M00, M10, M20, M01, M11, M21;

    M00 = m3[0][0]; M10 = m3[1][0]; M20 = 0;
    M01 = m3[0][1]; M11 = m3[1][1]; M21 = m3[2][1];

    if (!camParallel) {
	z_dist = (camDistance + 0.5) * my_canvas -> view_distance;
	M00 += x * m3[0][2] / z_dist;
	M10 += x * m3[1][2] / z_dist;
	M20 += x * m3[2][2] / z_dist;
	M01 += y * m3[0][2] / z_dist;
	M11 += y * m3[1][2] / z_dist;
	M21 += y * m3[2][2] / z_dist;
    }

    zr = z - my_canvas -> view_zr;
    nm = M00 * M11 - M01 * M10;
    xr = (x * M11 - y * M10 + zr * (M21*M10 - M20*M11)) / nm;
    yr = (y * M00 - x * M01 - zr * (M21*M00 + M20*M01)) / nm;
    *x_return = xr + my_canvas -> view_xr;
    *y_return = yr + my_canvas -> view_yr;
}

void X3dDrawLine (GC gc, wcoor x1, wcoor y1, wcoor z1, wcoor x2, wcoor y2, wcoor z2)
{
     X3dProject (x1, y1, z1, &x1, &y1);
     X3dProject (x2, y2, z2, &x2, &y2);
     X2dDrawLine (gc, x1, y1, x2, y2);
}

/* Camera position in polar coordinates
 * relative to view reference point.
 */
void X3dSetSimplePerspective (wcoor longitude, wcoor latitude, wcoor distance)
{
    double cosla, coslo, sinla, sinlo;

    latitude = latitude / 180.0 * M_PI;
    cosla = cos (latitude);
    sinla = sin (latitude);
    longitude = longitude / 180.0 * M_PI;
    coslo = cos (longitude);
    sinlo = sin (longitude);

    if (distance > 0) my_canvas -> view_distance = distance;

    /* X3dMakeViewplaneTransformation:
     * Start with general rotation matrix
     */
    m3[0][0] = -sinlo;         m3[1][0] =  coslo;         m3[2][0] = 0;
    m3[0][1] = -coslo * sinla; m3[1][1] = -sinlo * sinla; m3[2][1] = cosla;
    m3[0][2] =  coslo * cosla; m3[1][2] =  sinlo * cosla; m3[2][2] = sinla;
}

void X3dSetWorldCoordinates (wcoor x1, wcoor x2, wcoor y1, wcoor y2, wcoor z1, wcoor z2)
{
    wcoor x_return, y_return;
    wcoor x_min, x_max, y_min, y_max;

    X3dProject (x1, y1, z1, &x_return, &y_return);
    x_min = x_max = x_return;
    y_min = y_max = y_return;
#if 0
    X3dProject (x1, y1, z2, &x_return, &y_return);
    x_min = Min (x_min, x_return); x_max = Max (x_max, x_return);
    y_min = Min (y_min, y_return); y_max = Max (y_max, y_return);
#endif
    X3dProject (x1, y2, z1, &x_return, &y_return);
    x_min = Min (x_min, x_return); x_max = Max (x_max, x_return);
    y_min = Min (y_min, y_return); y_max = Max (y_max, y_return);
#if 0
    X3dProject (x1, y2, z2, &x_return, &y_return);
    x_min = Min (x_min, x_return); x_max = Max (x_max, x_return);
    y_min = Min (y_min, y_return); y_max = Max (y_max, y_return);
#endif
    X3dProject (x2, y1, z1, &x_return, &y_return);
    x_min = Min (x_min, x_return); x_max = Max (x_max, x_return);
    y_min = Min (y_min, y_return); y_max = Max (y_max, y_return);
#if 0
    X3dProject (x2, y1, z2, &x_return, &y_return);
    x_min = Min (x_min, x_return); x_max = Max (x_max, x_return);
    y_min = Min (y_min, y_return); y_max = Max (y_max, y_return);
#endif
    X3dProject (x2, y2, z1, &x_return, &y_return);
    x_min = Min (x_min, x_return); x_max = Max (x_max, x_return);
    y_min = Min (y_min, y_return); y_max = Max (y_max, y_return);
#if 0
    X3dProject (x2, y2, z2, &x_return, &y_return);
    x_min = Min (x_min, x_return); x_max = Max (x_max, x_return);
    y_min = Min (y_min, y_return); y_max = Max (y_max, y_return);
#endif
    X2dSetWorldCoordinates (x_min, x_max, y_min, y_max);
}

void X3dInit (canvas_t *canvas)
{
    X3dSetViewReferencePoint (0.0, 0.0, 0.0);
    X3dSetSimplePerspective (270.0, 90.0, 2.0);
    X3dSetWorldCoordinates (0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
}
