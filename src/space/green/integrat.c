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
#include <math.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/green/green.h"
#include "src/space/green/extern.h"
#include "src/space/green/gputil.h"

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private integrate_t integrateTriangle (int do_shape, real_t (*func)(pointR3_t *op),
    pointR3_t *p1, pointR3_t *p2, pointR3_t *p3, int max_refines, real_t eps);
Private integrate_t integrateParalellogram (real_t (*func)(pointR3_t *op),
    pointR3_t *p1, pointR3_t *p2, pointR3_t *p3, pointR3_t *p4, int max_refines, real_t eps);
#ifdef __cplusplus
  }
#endif

static int debug = 0;
int showIntegration = 0;

/* Stroud = Stroud, A.H,
 * approximate calculation of multiple integrals
 * Prentice Hall, 1971, ISBN 13-043893-6
 * DUT Central Library P 1925 5097
 * formulas from this book have been transformed
 * to the domain [0,1]*[0,1].
 */

/* Concerning error control, we use subsequent formulas
 * with an increasing number of points. An approximation
 * of the error is then given by E = |In - Im|, where In
 * and Im are the results using n and m points respectively,
 * n > m. According to Davis and Rabinowitz,
 * Methods of Numerical Integration, ISBN 0-12-206360-0,
 * ET library C 4000.17, page 336-343, this is actually
 * a good approximation to the error of Im and not of In.
 * For In, it is an overestimation. However, we accept In
 * as the result and use E as its estimated error.
 * Overestimating E is not efficient, and we could consider
 * using the approach suggested by Laurie and described in
 * Davis and Rabinowitz to give more accurate error estimates.
 */

/*
 * Numerical integration of a function <func> over
 * a triangle or quadrangle in R3 identified by p1-p4.
 * <func> should return a real_t and should expect one
 * argument, a **pointer to** a pointR3_t.
 *
 * If p4 == NULL: a triangle, a quadrangle otherwise.
 * p1-p4 should be in clockwise or counter-clockwise order.
 *
 * If do_shape == TRUE, then a weighting function is used
 * that is zero at p2 and p3 and nonzero at p1, and has unity
 * weight (a value of 3 at p1).
 * In this case, p4 should be zero (triangular elements).
 *
 * <eps> is the target accuracy, the accuracy of a result
 * is estimated as the difference of this result with one
 * obtained using a smaller number of function evaluations.
 *
 * <max_refines> bounds the number of refinements that is
 * tried in sequence to reach the target accuracy.
 * It can be used to control running time.
 *
 * The return value is a structure (not a pointer to)
 * containing the value (<.value>) and an error estimate (<.error>).
 */
integrate_t integrate2D (bool_t do_shape, real_t (*func)(pointR3_t *op),
	pointR3_t *p1, pointR3_t *p2, pointR3_t *p3, pointR3_t *p4, int max_refines, real_t eps)
{
    integrate_t result;

    ASSERT (max_refines > 0);
    ASSERT (eps >= 0);

    if (p4 == NULL || (p4->x == p1->x && p4->y == p1->y && p4->z == p1->z)) {
	result = integrateTriangle (do_shape, func, p1, p2, p3, max_refines, eps);
    }
    else {
	ASSERT (do_shape == FALSE);
	if (r3Dist2 (p1, p2) == r3Dist2 (p3, p4)
	&&  r3Dist2 (p2, p3) == r3Dist2 (p4, p1)) {
	    result = integrateParalellogram (func, p1, p2, p3, p4, max_refines, eps);
	}
	else {
	    integrate_t r1, r2;

	    r1 = integrateTriangle (FALSE, func, p1, p2, p3, max_refines, eps / 2.0);
	    r2 = integrateTriangle (FALSE, func, p1, p3, p4, max_refines, eps / 2.0);

	    result.value = r1.value + r2.value;
	    result.error = r1.error + r2.error;
	}
    }

    return (result);
}

/*
 * With piecewise linear elements, do_shape == TRUE.
 * p1 is then the point where shape-function is non-zero.
 */
Private integrate_t integrateTriangle (bool_t do_shape, real_t (*func)(pointR3_t *op),
	pointR3_t *p1, pointR3_t *p2, pointR3_t *p3, int max_refines, real_t eps)
{
    integrate_t result;
    int i, j;
    real_t old, val, err;

    static real_t c[][3] = {

    /* trivial one point formula */
	{ 0.33333333, 0.33333333, 1.00000000 },	/* 0 */

    /* formula Stroud T2-2-1 */
	{ 0.16666666, 0.16666666, 0.33333333 },	/* 1 */
	{ 0.16666666, 0.66666666, 0.33333333 },	/* 2 */
	{ 0.66666666, 0.16666666, 0.33333333 },	/* 3 */

    /* formula Stroud T2-5-1 */
	{ 0.33333333, 0.33333333, 0.22500000 }, /* 4 */
	{ 0.79742699, 0.10128651, 0.12593918 },
	{ 0.10128651, 0.79742699, 0.12593918 },
	{ 0.10128651, 0.10128651, 0.12593918 },
	{ 0.05971587, 0.47014206, 0.13239415 },
	{ 0.47014206, 0.05971587, 0.13239415 },
	{ 0.47014206, 0.47014206, 0.13239415 },	/* 10 */

    /* based on formula Stroud T2-7-1, 16 points,
     * but reworked to do it in one loop instead of two enclosed loops
     */
	{0.0571041961, 0.0654669927, 0.0471367364}, /* 11 */
	{0.2768430136, 0.0502101218, 0.0707761358},
	{0.5835904324, 0.0289120834, 0.0451680986},
	{0.8602401357, 0.0097037848, 0.0108464518},
	{0.0571041961, 0.3111645522, 0.0883701770},
	{0.2768430136, 0.2386486597, 0.1326884322},
	{0.5835904324, 0.1374191041, 0.0846794490},
	{0.8602401357, 0.0461220799, 0.0203345191},
	{0.0571041961, 0.6317312517, 0.0883701770},
	{0.2768430136, 0.4845083267, 0.1326884322},
	{0.5835904324, 0.2789904635, 0.0846794490},
	{0.8602401357, 0.0936377844, 0.0203345191},
	{0.0571041961, 0.8774288093, 0.0471367364},
	{0.2768430136, 0.6729468632, 0.0707761358},
	{0.5835904324, 0.3874974834, 0.0451680986},
	{0.8602401357, 0.1300560792, 0.0108464518}	/* 26 */
    };

    static int start [] = {0, 1, 4, 11, 27};	/* where a formula starts. */
#define NUMFORMULAS ((sizeof(start)/sizeof(*start))-1) /* one less than # entries */

#ifdef DEBUG
    if (debug || showIntegration) fprintf (stderr, "triangle");
#endif

    val = 0;
    err = 1.0;

    for (j = 0; j < NUMFORMULAS && j < max_refines; j++) {

	old = val;		/* be sure to only use old when j > 0 */
	val = 0;

	for (i = start[j]; i < start[j+1]; i++) {
	    real_t c0 = c[i][0];
	    real_t c1 = c[i][1];
	    real_t c2 = 1 - c0 - c1;
	    real_t w = c[i][2];
	    pointR3_t p;
	    real_t f;

	    p.x = c0 * p2 -> x + c1 * p3 -> x + c2 * p1 -> x;
	    p.y = c0 * p2 -> y + c1 * p3 -> y + c2 * p1 -> y;
	    p.z = c0 * p2 -> z + c1 * p3 -> z + c2 * p1 -> z;

	    f = (*func) (&p);
	    if (f == 0.0) {
	       result.value = 0;
	       result.error = 1.0;
	       return (result);
	    }
	    f *= w;

	    /* For linear elements, multiply by 3 * third coordinate.
	     * The third coordinate is equal to the value of the shape function.
	     * This might not be an optimal integration formula, but it works.
	     * For linear elements, make sure that p1 is the node where
	     * shape function is non-zero, this is why p1 is associated
	     * with c2;
	     */
	    if (do_shape) f *= 3.0 * c2;

	    val += f;
	}
#ifdef DEBUG
	if (debug || showIntegration) fprintf (stderr, " %13.10f", (double)val);
#endif
	if (j > 0) {
	    err = (val - old) / val;
	    if (err < 0) err = -err;
	    if (err < eps) break;
	}
    }

#ifdef DEBUG
    if (debug || showIntegration) fprintf (stderr, "\n");
#endif

    result.value = val * r3TriArea (p1, p2, p3);
    result.error = err;
    return (result);
}

/* integrateParalellogram
 * A function to integrate over a 2-dimensional paralellogram-shaped domain.
 */
Private integrate_t integrateParalellogram (real_t (*func)(pointR3_t *op),
	pointR3_t *p1, pointR3_t *p2, pointR3_t *p3, pointR3_t *p4, int max_refines, real_t eps)
{
    integrate_t result;
    int i, j;
    real_t old, val, err;

    static real_t c [][3] = {
    /* trivial one-point formula */
	{ 0.50000000, 0.50000000, 1.00 },	/* 0 */

    /* formula Stroud C2-3-1 */
	{ 0.33333333, 0.33333333, 0.25 },	/* 1 */
	{ 0.66666666, 0.33333333, 0.25 },
	{ 0.33333333, 0.66666666, 0.25 },
	{ 0.66666666, 0.66666666, 0.25 },

    /* formula Stroud C2-5-2, 7 points */
	{ 0.50000000, 0.50000000, 0.2857143 },	/* 5 */
	{ 0.84156505, 0.84156505, 0.1488095 },
	{ 0.15843495, 0.15843495, 0.1488095 },
	{ 0.94532720, 0.31287170, 0.1041667 },
	{ 0.05467280, 0.68712830, 0.1041667 },
	{ 0.68712830, 0.05467280, 0.1041667 },
	{ 0.31287170, 0.94532720, 0.1041667 },

    /* formula Stroud C2-7-1, 12 points */
	{ 0.96291000, 0.50000000, 0.0604938 },	/* 12 */
	{ 0.03709000, 0.50000000, 0.0604938 },
	{ 0.50000000, 0.96291000, 0.0604938 },
	{ 0.50000000, 0.03709000, 0.0604938 },
	{ 0.69027720, 0.69027720, 0.1301482 },
	{ 0.30972280, 0.69027720, 0.1301482 },
	{ 0.69027720, 0.30972280, 0.1301482 },
	{ 0.30972280, 0.30972280, 0.1301482 },
	{ 0.90298990, 0.90298990, 0.0593579 },
	{ 0.09701010, 0.90298990, 0.0593579 },
	{ 0.90298990, 0.09701010, 0.0593579 },
	{ 0.09701010, 0.09701010, 0.0593579 }     /* 23 */
    };

    static int start [] = {0, 1, 5, 12, 24};	/* where a formula starts. */
#define NUMFORMULAS ((sizeof(start)/sizeof(*start))-1) /* one less than # entries */

#ifdef DEBUG
    if (debug) fprintf (stderr, "parallelogram");
#endif

    val = 0;
    err = 1.0;

    for (j = 0; j < NUMFORMULAS && j < max_refines; j++) {
	real_t w = 0;
	pointR3_t p;
	old = val;		/* be sure to only use old when j > 0 */
	val = 0;

	for (i = start[j]; i < start[j+1]; i++) {
	    real_t f;
	    real_t c0 = c[i][0];
	    real_t c1 = c[i][1];
	    real_t c2 = 1.0 - c0 - c1;
	    p.x = c0 * p2 -> x + c1 * p4 -> x + c2 * p1 -> x;
	    p.y = c0 * p2 -> y + c1 * p4 -> y + c2 * p1 -> y;
	    p.z = c0 * p2 -> z + c1 * p4 -> z + c2 * p1 -> z;

	    f = (*func) (&p);
	    if (f == 0.0) {
	       result.value = 0;
	       result.error = 1.0;
	       return (result);
	    }
	    if (debug > 1) fprintf (stderr, "eval %g %g %g\n", c0, c1, f);
	    val += c[i][2] * f;	/* c[i][2] contains weight */
	    w += c[i][2];
	}

#ifdef DEBUG
        if (debug) fprintf (stderr, " %13.10f", (double) val);
#endif

	ASSERT (Abs (w - 1.0) < EPS);

	if (j > 0) {
	    err = (val - old) / val;
	    if (err < 0) err = -err;
	    if (err < eps) break;
	}
    }

#ifdef DEBUG
    if (debug) fprintf (stderr, "\n");
#endif

    result.value = val * 2 * r3TriArea (p1, p2, p3);
    result.error = err;
    return (result);
}

integrate_t integrate1D (bool_t do_shape, real_t (*func)(pointR3_t *op),
	pointR3_t *p1, pointR3_t *p2, int max_refines, real_t eps)
{
    integrate_t result;

    /* abscissas normalized over [0, 1] */
    static double ABS [] = { 0.11270167, 0.5, 0.88729833 };

    /* normalized weights */
    static double WGT [] = { 0.5555555556, 0.8888888889, 0.5555555556 };

    /* number of quadrature points */
    int N = 3;

    pointR3_t p, L, U;
    double  old, val, err;
    int     M, i, j;

    ASSERT (do_shape == FALSE); /* for now */

    val = 0.0;
    err = 1.0;

    for (M = 1; M <= max_refines ; M++) {
	old = val;
	val = 0.0;

	for (i = 0; i < M; i++) {
	    double R = (double) i / M;
	    L.x = (1.0 - R) * p1 -> x + R * p2 -> x;
	    L.z = (1.0 - R) * p1 -> z + R * p2 -> z;
	    R = (i + 1.0) / M;
	    U.x = (1.0 - R) * p1 -> x + R * p2 -> x;
	    U.z = (1.0 - R) * p1 -> z + R * p2 -> z;
	    for (j = 0; j < N; j++) {
		real_t f;
		p.x = (1 - ABS[j]) * L.x + ABS[j] * U.x;
		p.z = (1 - ABS[j]) * L.z + ABS[j] * U.z;
		p.y = 0;
		f = (*func) (&p);
		if (f == 0.0) {
		   result.value = 0;
		   result.error = 1.0;
		   return (result);
		}
		val += f * WGT[j];
	    }
	}

	val = 0.5 * val / M;

	Debug (fprintf (stderr, "  iteration %lx %d: %e  difference: %e\n", (long) func, M, val, val - old));

	if (M > 1) {
	    err = (val - old) / val;
	    if (err < 0) err = -err;
	    if (err < eps) break;
	}
    }

    val *= r3Dist (p1, p2);

    if (err > eps) fprintf (stderr, "gauss: no convergence\n");

    result.value = val;
    result.error = err;
    return (result);
}

#ifdef DRIVER

pointR3_t *Op;

Private real_t gf (pointR3_t *p)
{
    return (1.0 / r3Dist (Op, p));
}

/*
 * Test driver for integrate.
 */
int main (int argc, char *argv[])
{
    real_t val;
    pointR3_t p1, p2, p3, op;
    integrate_t result;

    R3Assign (p1, 0, 0, 0);
    R3Assign (p2, 1, 1, 0);
    R3Assign (p3, 1, 0, 0);
    R3Assign (op, 1, 1, 0);

    if (argc == 4) {
	op.x = atof (argv[1]);
	op.y = atof (argv[2]);
	op.z = atof (argv[3]);
    }

    val = analyticPotIntegral (TRUE, &op, &p1, &p2, &p3, &p1);
    fprintf (stderr, "analytic: %g\n", (double) val);

    Op = &op;
    result = integrate2D (TRUE, gf, &p1, &p2, &p3, &p1, 1000, 0.0);
    fprintf (stderr, "numeric: %g, estimated error %g, true error %g\n",
	(double) result.value, (double) result.error,
	(double) Abs ((val - result.value) / val));

    val = analyticPotIntegral (FALSE, &op, &p1, &p2, &p3, &p1);
    fprintf (stderr, "analytic: %g\n", (double) val);

    Op = &op;
    result = integrate2D (FALSE, gf, &p1, &p2, &p3, &p1, 1000, 0.0);
    fprintf (stderr, "numeric: %g, estimated error %g, true error %g\n",
	(double) result.value, (double) result.error,
	(double) Abs ((val - result.value) / val));

    return (0);
}

#endif /* DRIVER */
