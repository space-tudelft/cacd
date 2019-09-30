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

/*
 * Extensions by Theo Smedes for substrate resistances (1995)
 */

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/export.h"
#include "src/space/green/green.h"
#include "src/space/green/extern.h"
#include "src/space/green/gputil.h"
#include "src/space/spider/export.h"

#ifndef DRIVER
extern bool_t offDiagonal;
extern bool_t edgeEffects;
extern coor_t sawDist;
extern coor_t edgeDist;
extern long green_cnt;
extern int maxGreenTerms;
extern int nLayers[];
extern int showIntegration;
extern green_t collocationEps;

static pointR3_t Pp;
static int gf1PLayer, gf1QLayer, gf1G0;
static int printResults = 0;
static int an_turnover = 0;

static bool_t green2D = FALSE;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private green_t simpleDoCollocationGreen (pointR3_t pp0, pointR3_t e1, pointR3_t e2, pointR3_t e3, pointR3_t e4);
Private integrate_t numericCollocation (int g, int i, int j, pointR3_t pp, pointR3_t e1, pointR3_t e2, pointR3_t e3, pointR3_t e4);
Private green_t simpleGf1 (pointR3_t *op);
Private green_t evalGroup (imageGroup_t grp, pointR3_t *p1, pointR3_t *p2);
Private green_t evalGroupACollocation (imageGroup_t grp, pointR3_t pp, pointR3_t e1, pointR3_t e2, pointR3_t e3, pointR3_t e4);
Private green_t seriesSum (green_t term, int termnr);
#ifdef DEBUG
Private green_t aCollocation (int i, int j, pointR3_t pp, pointR3_t e1, pointR3_t e2, pointR3_t e3, pointR3_t e4);
#endif
#ifdef __cplusplus
  }
#endif

static int checkCollocation = -1;
#ifdef DEBUG
static green_t maxGreenErr;
#endif

/*
 * Compute the collocation integral for the
 * potential point given by pp, and the element given by e1-e4.
 *
 * Implements green's function series expression,
 * by repeatedly changing the z-coordinate of pp.
 * pp.z is restored upon exit.
 *
 * For triangular elements, e4 should be equal to e1.
 *
 * If FeModePwl == TRUE, assume a piece-wise linear shape function,
 * being 1.0 at e1 and 0.0 at e2 and e3.
 * With linear shape functions: triangular elements (e4 == e1).
 */
green_t
doCollocationGreen (pointR3_t pp0, pointR3_t e1, pointR3_t e2, pointR3_t e3, pointR3_t e4)
{
    if (checkCollocation == -1) {
	checkCollocation = paramLookupI ("check_collocation", "0");
	an_turnover = paramLookupI ("an_turnover", "100");
    }

    return simpleDoCollocationGreen (pp0, e1, e2, e3, e4);
}

green_t
simpleDoCollocationGreen (pointR3_t pp0, pointR3_t e1, pointR3_t e2, pointR3_t e3, pointR3_t e4)
{
    static int collocationMode = 0;
    imageGroup_t grp;
    integrate_t num_result;
    int g, i, j, k, km, ok;
    pointR3_t pp, mp[5];

    if (collocationMode < 0) collocationMode = paramLookupI ("collocation_mode", "0");

    /* determine medium number of pp.z and e.
    */
    R3Copy (pp0, pp);

    i = mediumNumber4 (e1.z, e2.z, e3.z, e4.z);
    j = mediumNumber (pp.z);

    km = 0;
    if (edgeEffects) { /* greenCase == SUBS */
	for (k = 1; k < 5; k++) {
	    mp[k].x = pp0.x;
	    mp[k].y = pp0.y;
	}
        if ((pp0.x - bbxl + sawDist) < edgeDist)
            mp[km = 1].x = 2.0 * (bbxl - sawDist) - pp0.x;
        if ((bbxr - pp0.x + sawDist) < edgeDist)
            mp[km = 2].x = 2.0 * (bbxr + sawDist) - pp0.x;
        if ((pp0.y - bbyb + sawDist) < edgeDist)
            mp[km = 3].y = 2.0 * (bbyb - sawDist) - pp0.y;
        if ((bbyt - pp0.y + sawDist) < edgeDist)
            mp[km = 4].y = 2.0 * (bbyt + sawDist) - pp0.y;
    }

    double simpleValue = 0.0;
    for (k = 0;;) {
        /* Algorithm:
         * try to do numerical, do term analytic if no convergence
         * try other terms numerical etc.
         */
        ok = 0;
        for (g = 0; g < maxGreenTerms; g++) {
            /*
             * Here we filter the terms to do numeric.
             *     case 1: if (offDiagonal == TRUE || g > 0)
             *     case 2: if (g > 0)
             * Case 2 gives more accurate results. Apparently
             * the first term discloses an optimistic nature of
             * the numerical integration concerning convergence.
             */
            if ((offDiagonal == TRUE
                        || k > 0 /* sidewall images are always offdiagonal */
                        || g > 0) && collocationMode != 1) {
                num_result = numericCollocation (g, i, j, pp, e1, e2, e3, e4);
                if (num_result.error <= collocationEps) {
                    simpleValue += num_result.value;
                    break;
                }
            }

            grp = getImageGroup (i, j, g);
            if (grp.size == 0) { greenNoConvergence (i, j, g); die (); }

            double term;
            simpleValue += (term = evalGroupACollocation (grp, pp, e1, e2, e3, e4));

            if (printResults) fprintf (stderr, "a %2d %e %e\n", g, term, simpleValue);

            /* Generally no need to check convergence here,
             * this is done in numericCollocation (or we exceed maxTerms).
             * However, in DEBUGGING mode we may surpass numericCollocation.
             */

            if (Abs (term / simpleValue) < collocationEps) { if (++ok == 2) break; }
            else ok = 0;
        }

        if (nLayers[greenCase] > 1 && g == maxGreenTerms)
            greenNoConvergence (i, j, g);

        if (k == km) break;
/* T.S. maybe this should be moved after the DEBUG block */
        do k++; while (mp[k].x == pp0.x && mp[k].y == pp0.y);
        if (k > km) break;
        pp.x = mp[k].x;
        pp.y = mp[k].y;
    }

#ifdef DEBUG
    if (simpleValue != -1 && checkCollocation) {
	double exact = aCollocation (i, j, pp, e1, e2, e3, e4);
	if (Abs (1 - simpleValue / exact) > maxGreenErr) {
	    maxGreenErr = Abs (1 - simpleValue / exact);
	    fprintf (stderr, "approx %e <exact> %e <error> %e\n",
		simpleValue, exact, maxGreenErr);

	    showIntegration = 1; printResults = 1; checkCollocation = 0;
	    (void) doCollocationGreen (pp, e1, e2, e3, e4);
	    showIntegration = 0; printResults = 0; checkCollocation = 1;
	}
	if (exact < 0) {
	    fprintf (stderr, "i, j = %d %d\n", i, j);
	    fprintf (stderr, "pp %e %e %e\n", pp.x, pp.y, pp.z);
	    fprintf (stderr, "e1 %e %e %e\n", e1.x, e1.y, e1.z);
	    fprintf (stderr, "e2 %e %e %e\n", e2.x, e2.y, e2.z);
	    fprintf (stderr, "e3 %e %e %e\n", e3.x, e3.y, e3.z);
	    for (g = 0; g <= 5; g++) {
		grp = getImageGroup (i, j, g);
		for (k = 0; k < grp.size; k++) {
		    fprintf (stderr, "%2d %d %e %e %d\n", g, k,
			grp.images[k].strength,
			grp.images[k].distance,
			grp.images[k].zq_sign);
		}
	    }
	    showIntegration = 1; printResults = 1; checkCollocation = 0;
	    (void) doCollocationGreen (pp, e1, e2, e3, e4);
	    showIntegration = 0; printResults = 0; checkCollocation = 1;
	}
    }
#endif /* DEBUG */

    return (simpleValue);
}

/* Collocation for the two-dimensional case.
 * WARNING: this function returns two times the actual result,
 * so that is must be normalized by 1/(4 PI eps0), as in the
 * three-d case. (1/(2 PI eps0) would be for 2-dimensional case.)
 */
green_t doCollocationGreen2D (pointR3_t pp, pointR3_t e1, pointR3_t e2)
{
    green_t result;

    e1.y = e2.y = pp.y = 0;
    green2D = TRUE;
    result = doCollocationGreen (pp, e1, e2, e1, e1);
    green2D = FALSE;

    return (result);
}

/*
 * numeric collocation
 */
Private integrate_t numericCollocation (int g, int i, int j, pointR3_t pp, pointR3_t e1, pointR3_t e2, pointR3_t e3, pointR3_t e4)
{
    integrate_t result;
    gf1PLayer = i;
    gf1QLayer = j;
    gf1G0 = g;
    R3Copy (pp, Pp);

    green_t (*fptr)(pointR3_t *op);
    fptr = simpleGf1;

    if (green2D)
	result = integrate1D (FeModePwl, fptr, &e1, &e2, an_turnover, collocationEps);
    else
	result = integrate2D (FeModePwl, fptr, &e1, &e2, &e3, &e4, an_turnover, collocationEps);
    return (result);
}

Private green_t simpleGf1 (pointR3_t *op)
{
    real_t rx, ry, rz, max1, sum1, max, sum;

    if ((rx = Pp.x - op -> x) < 0) rx = -rx;
    if ((ry = Pp.y - op -> y) < 0) ry = -ry;

    max1 = rx > ry ? rx : ry;
    sum1 = rx * rx + ry * ry;

#ifdef DEBUG
    if (printResults) fprintf (stderr, "op %e %e %e\n", op -> x, op -> y, op -> z);
#endif
    Debug (fprintf (stderr, "Pp %e %e %e\n", Pp.x, Pp.y, Pp.z));
    Debug (fprintf (stderr, "op %e %e %e\n", op -> x, op -> y, op -> z));

    green_cnt++;

    double value = 0;
    double old;
    imageGroup_t grp;
    int g, i;

    for (g = gf1G0; g < maxGreenTerms; g++) {
        grp = getImageGroup (gf1PLayer, gf1QLayer, g);
        if (grp.size == 0) { greenNoConvergence (gf1PLayer, gf1QLayer, g); break; }

        double term = 0;

        if (!green2D) {
            for (i = 0; i < grp.size; i++) {
                rz = op -> z + grp.images[i].distance
                   + Pp.z * grp.images[i].zq_sign;
                if (rz < 0) rz = -rz;

                max = max1 > rz ? max1 : rz;
                sum = sum1 + rz * rz;

                if (sum == 0) return 0;

                ASSERT (max != 0.0);

                /* Compute square root of sum with Newton-Raphson */
                max = 1.73205 * max;
                max = 0.5 * (max + sum / max);
                max = 0.5 * (max + sum / max);
                max = 0.5 * (max + sum / max);
                term += 2.0 * grp.images[i].strength / (max + sum / max);
            }
        }
        else {
            for (i = 0; i < grp.size; i++) {
                rz = op -> z + grp.images[i].distance
                   + Pp.z * grp.images[i].zq_sign;
                Debug (fprintf (stderr, "rx rz s t %e %e %e %e\n",
                    rx, rz, grp.images[i].strength,
                    grp.images[i].strength * -1 * Log (rx * rx + rz * rz)));
                term += grp.images[i].strength * -1 * Log (rx * rx + rz * rz);
            }
        }

        old = value;
        value = seriesSum (term, g - gf1G0 + 1);

#ifdef DEBUG
        if (printResults) fprintf (stderr, "n %2d %e %e\n", g, term, value);
#endif
        Debug (fprintf (stderr, "n %2d %e %e\n", g, term, value));

        term = (old - value) / value;
        if (Abs (term) < collocationEps / 2)
            return value;
    }

    if (nLayers[greenCase] > 1 && g == maxGreenTerms)
        greenNoConvergence (gf1PLayer, gf1QLayer, g);

    return value;
}

Private green_t evalGroupACollocation (imageGroup_t grp, pointR3_t pp, pointR3_t e1, pointR3_t e2, pointR3_t e3, pointR3_t e4)
{
    int i;
    green_t value = 0;
    real_t ppz = pp.z;

/* T.S.: currently, only z==0 allowed for substrate case */
    if (greenCase == SUBS) ASSERT (ppz == 0 && e1.z == 0);

    /*
	fprintf (stderr, "element %g %g %g\n", e1.x, e1.y, e1.z);
	fprintf (stderr, "element %g %g %g\n", e2.x, e2.y, e2.z);
	fprintf (stderr, "element %g %g %g\n", e3.x, e3.y, e3.z);
	fprintf (stderr, "element %g %g %g\n", e4.x, e4.y, e4.z);
    */

    for (i = 0; i < grp.size; i++) {
	pp.z = -(grp.images[i].distance + ppz * grp.images[i].zq_sign);
	if (green2D)
	    value += grp.images[i].strength * analyticPotIntegral2D (FeModePwl, &pp, &e1, &e2);
	else
	    value += grp.images[i].strength * analyticPotIntegral (FeModePwl, &pp, &e1, &e2, &e3, &e4);
	/*
	fprintf (stderr, "image   %g %g %g %g\n", pp.x, pp.y, pp.z, value);
	*/
    }

    pp.z = ppz; /* Firewall */

    return (value);
}

#ifdef DEBUG
Private green_t aCollocation (int i, int j, pointR3_t pp, pointR3_t e1, pointR3_t e2, pointR3_t e3, pointR3_t e4)
{
    green_t term, value = 0;
    imageGroup_t grp;
    int g, ok = 0;

    for (g = 0; g < maxGreenTerms; g++) {

	grp = getImageGroup (i, j, g);

	value += (term = evalGroupACollocation (grp, pp, e1, e2, e3, e4));

	if (value > 0 && (Abs (term) / value < collocationEps/5)) { if (++ok == 3) break; }
	else ok = 0;
    }

    return (value);
}
#endif /* DEBUG */

/*
 * Compute the point-green's function for p1 and p2.
 */
green_t greenFunc (pointR3_t *p1, pointR3_t *p2)
{
    green_t term, value = 0;
    imageGroup_t grp;
    int i, j, g;

    /* determine medium number of p1 -> z and p2 -> z */
    i = mediumNumber (p1 -> z);
    j = mediumNumber (p2 -> z);

    green_cnt++;

    for (g = 0; g < maxGreenTerms; g++) {
	grp = getImageGroup (i, j, g);
	if (grp.size == 0) { greenNoConvergence (i, j, g); break; }

	value += (term = evalGroup (grp, p1, p2));

	if (Abs (term / value) < collocationEps) return (value);
    }

    if (g == maxGreenTerms) greenNoConvergence (i, j, g);

    return (value);
}

green_t greenFunc2D (pointR3_t *p1, pointR3_t *p2)
{
    green_t result;

    green2D = TRUE;
    result = greenFunc (p1, p2);
    green2D = FALSE;

    return (result);
}

Private green_t evalGroup (imageGroup_t grp, pointR3_t *p1, pointR3_t *p2)
{
    green_t r2, rz2, value = 0;
    int i;

/* T.S.: currently, only positions on the interface are allowed */
    if (greenCase == SUBS) ASSERT (p1->z == 0 && p2->z == 0);

    r2 = Dsqr (p1 -> x - p2 -> x) + Dsqr (p1 -> y - p2 -> y);

    for (i = 0; i < grp.size; i++) {
	rz2 = p1 -> z + grp.images[i].distance
	    + p2 -> z * grp.images[i].zq_sign;
	rz2 *= rz2;

	if (green2D)
	    value += -grp.images[i].strength * Log (r2 + rz2);
	else
	    value += grp.images[i].strength / Sqrt (r2 + rz2);
    }

    return (value);
}
#endif /* NO DRIVER */

Private green_t seriesSum (green_t term, int termnr)
{
    static int state = 0;
    static double psum;

    if (state == 0) {
	if (paramLookupB ("accelerate_levin", "off"))
	    state = 1; /* accel */
	else
	    state = 2; /* no accel */
    }

    if (state == 1)  return (levin (term, termnr));
    if (termnr == 1) return (psum = term);
    return (psum += term);
}

#ifdef DRIVER
/*
 * Test driver for seriesSum().
 */
int main ()
{
    int i = 0;
    double t = -1;
    double sum = 0;
    double p = -1;
    paramSetOption2 ((char*)"accelerate_levin", (char*)"on");
    for (i = 1; i < 10; i++) {
        p *= -1;
        t = p / (double) i;
        sum += t;
        fprintf (stderr, "t=%9.6f s=%8.6f l=%8.6f\n", t, sum, seriesSum (t, i));
    }
    return (0);
}
#endif /* DRIVER */
