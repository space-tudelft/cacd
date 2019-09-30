/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include <math.h>

/* Begin copy from makegln.h */
typedef int slope_t;
typedef int sign_t;

/* signs of input edges */
#define STOP  -1
#define START  1
/* End copy from makegln.h */

#ifdef __cplusplus
  extern "C" {
#endif
void sortEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr, slope_t slope, sign_t sign);
#ifdef __cplusplus
  }
#endif

extern bool_t optPS;
extern coor_t bxl, bxr, byb, byt;
extern coor_t growSize, Inf, InfXl, InfXr;

static FILE * fpEps;
static coor_t e_xl, e_yl, e_xr, e_yr;
static sign_t e_sign;
static slope_t e_slope;

Private void processEdge ()
{
    // updateBoundingBox
    if (e_xl < bxl && e_xl != InfXl) bxl = e_xl;
    if (e_xr > bxr && e_xr != InfXr) bxr = e_xr;
    if (e_yl < byb && e_yl != -Inf) byb = e_yl;
    if (e_yl > byt && e_yl !=  Inf) byt = e_yl;
    if (e_yr < byb && e_yr != -Inf) byb = e_yr;
    if (e_yr > byt && e_yr !=  Inf) byt = e_yr;

    sortEdge (e_xl, e_yl, e_xr, e_yr, e_slope, e_sign);

#define L(x) (long)(x)
#ifdef DEBUG
   if (DEBUG)
     fprintf (stderr, "processEdge %ld %ld %ld %ld %ld %ld\n",
	L(e_xl), L(e_yl), L(e_xr), L(e_yr), L(e_slope), L(e_sign));
#endif

    if (fpEps) {
	fprintf (fpEps, "%ld %ld moveto\n", L(e_xl), L(e_yl));
	fprintf (fpEps, "%ld %ld lineto\n", L(e_xr), L(e_yr));
	fprintf (fpEps, "stroke\n");
    }
}

#define calcSlope ((double) (e_yr - e_yl)) / ((double) (e_xr - e_xl))

void growLayout (tile, grow) tile_t *tile; coor_t grow;
{
    e_sign = STOP;
    e_slope = 0;

    /* Top side of tile */
    if (tile -> tl == tile -> tr) {
	if ((e_xl = tile -> xl) != InfXl) e_xl -= grow;
	if ((e_xr = tile -> xr) != InfXr) e_xr += grow;
	if ((e_yl = tile -> tl) !=  Inf) e_yl += grow;
	e_yr = e_yl;
	processEdge ();
    }
    else if (tile -> tl > tile -> tr) {
	e_xl = tile -> xl - grow;
	e_xr = tile -> xl + grow;
	e_yl = tile -> tl + grow;
	e_yr = e_yl;
	processEdge ();

	e_xl = e_xr;
	e_xr = tile -> xr + grow;
	e_yr = tile -> tr + grow;
	e_slope = calcSlope;
	processEdge ();
    }
    else { /* top is at rigth side of edge */
	e_xl = tile -> xr - grow;
	e_xr = tile -> xr + grow;
	e_yr = tile -> tr + grow;
	e_yl = e_yr;
	processEdge ();

	e_xr = e_xl;
	e_xl = tile -> xl - grow;
	e_yl = tile -> tl + grow;
	e_slope = calcSlope;
	processEdge ();
    }

    e_sign = START;
    e_slope = 0;

    /* Bottom side of tile */
    if (tile -> bl == tile -> br) {
	if ((e_xl = tile -> xl) != InfXl) e_xl -= grow;
	if ((e_xr = tile -> xr) != InfXr) e_xr += grow;
	if ((e_yl = tile -> bl) != -Inf) e_yl -= grow;
	e_yr = e_yl;
	processEdge ();
    }
    else if (tile -> bl > tile -> br) {
	e_xl = tile -> xr - grow;
	e_xr = tile -> xr + grow;
	e_yr = tile -> br - grow;
	e_yl = e_yr;
	processEdge ();

	e_xr = e_xl;
	e_xl = tile -> xl - grow;
	e_yl = tile -> bl - grow;
	e_slope = calcSlope;
	processEdge ();
    }
    else { /* top is at rigth side of edge */
	e_xl = tile -> xl - grow;
	e_xr = tile -> xl + grow;
	e_yl = tile -> bl - grow;
	e_yr = e_yl;
	processEdge ();

	e_xl = e_xr;
	e_xr = tile -> xr + grow;
	e_yr = tile -> br - grow;
	e_slope = calcSlope;
	processEdge ();
    }
}

void openDebugEps (cell, newmask) char *cell, *newmask;
{
    if (optPS) {
	fpEps = cfopen (mprintf ("%s_%s.eps", cell, newmask), "w");
	fprintf (fpEps, "%%!PS-Adobe-1.0\n");
	fprintf (fpEps, "%%%%BoundingBox: %ld %ld %ld %ld\n",
	    L(bxl - growSize), L(byb - growSize),
	    L(bxr + growSize), L(byt + growSize));
	fprintf (fpEps, "%%%%Pages: 1\n");
	fprintf (fpEps, "%%%%EndComments\n");
    }
}

void closeDebugEps ()
{
    if (fpEps) {
	fprintf (fpEps, "showpage\n");
	fclose (fpEps);
	fpEps = NULL;
    }
}
