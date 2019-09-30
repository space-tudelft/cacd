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
#include "src/space/spider/define.h"
#include "src/space/spider/export.h"

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private green_t greenPwc (spider_t *sp1, spider_t *sp2);
Private green_t greenPwl (spider_t *sp1, spider_t *sp2);
Private green_t PointGreen (spider_t *sp1, spider_t *sp2);
Private green_t CollocationGreenPwc (spider_t *spo, spider_t *spc);
Private green_t CollocationGreenPwl (spider_t *spo, spider_t *spc);
Private green_t GalerkinGreenPwc (spider_t *spo, spider_t *spc);
Private green_t GalerkinGreenPwl (spider_t *spo, spider_t *spc);
#ifdef __cplusplus
  }
#endif

extern int numPointGreen;
extern int numCollocationGreen;
extern int numGalerkinGreen;

extern bool_t offDiagonal;
extern bool_t printGreenTerms;
extern bool_t useMultiPoles, testMultiPoles;

extern real_t pointGreenDist;
extern real_t collocationGreenDist;
extern real_t asymCollocationGreenDist;
#define R3Set(gp,sp) R3Assign(gp, sp->act_x, sp->act_y, sp->act_z)

#ifdef TEST_MULTIPOLE_METHOD
Private int counter = 0;   /* used in connexion with testMultiPoles */
#endif

/*
 * Computes the value in the elastance matrix for the
 * two boundary element nodes sp1 and sp2.
 *
 * Main interface function, called from the spider module.
 */
double green (spider_t *sp1, spider_t *sp2)
{
    green_t val;
#ifdef TEST_MULTIPOLE_METHOD
    static int firstCall = 1, counterThreshold = 0;

    if (testMultiPoles) {
	if (firstCall) {
	    firstCall = 0;
	    counter = 0;
	    printf ("Enter threshold value for counter \n");
	    scanf ("%d", &counterThreshold);
	}
	if (++counter < counterThreshold) return (1);
    }
#endif

    tick ("green");

#if 0
    fprintf(stderr,"green: sp1=(%g,%g,%g) sp2=(%g,%g,%g)\n",
            sp1 -> act_x, sp1 -> act_y, sp1 -> act_z,
            sp2 -> act_x, sp2 -> act_y, sp2 -> act_z);
#endif

    offDiagonal = (sp1 == sp2 ? FALSE : TRUE);

    val = FeModePwl ? greenPwl (sp1, sp2) : greenPwc (sp1, sp2);

    tock ("green");

    // We have to check for a negative green's value. If the value is
    // negative, it can spoil the complete Schur inversion, even though the
    // value itself may be only of small importance.
    //
    if(val < 0) val = 0;

    return (val);
}

Private green_t greenPwc (spider_t *sp1, spider_t *sp2)
{
    green_t val;
    real_t d, r;

    r = sp1 -> face -> len + sp2 -> face -> len;
    d = spiderDist (sp1, sp2);

    if (printGreenTerms)
	fprintf(stderr,"greenPwc: d=%g r=%g sp1=(%g,%g,%g) sp2=(%g,%g,%g)\n", d, r,
		sp1 -> act_x, sp1 -> act_y, sp1 -> act_z,
		sp2 -> act_x, sp2 -> act_y, sp2 -> act_z);

    if (d >= r * pointGreenDist && d > 0) {
	val = PointGreen (sp1, sp2);
	if (printGreenTerms)
	    fprintf(stderr,"greenPwc: PointGreen(val=%12.9f)\n", val);
	numPointGreen++;
    }
    else if (d >= r * collocationGreenDist) {
        val = CollocationGreenPwc (sp1, sp2);
	if (printGreenTerms)
	    fprintf(stderr,"greenPwc: CollocationGreenPwc(val=%12.9f)\n", val);
	numCollocationGreen++;
	if (d < r * asymCollocationGreenDist && sp1 != sp2) {
	    /* take arithmetic mean, experiments have shown
	     * that harmonic mean does not make much difference.
	     */
	    green_t val2 = CollocationGreenPwc (sp2, sp1);
	    val = (val + val2) / 2.0;
	    if (printGreenTerms)
		fprintf(stderr,"greenPwc: CollocationGreenPwc(val=%12.9f) val=%12.9f\n", val2, val);
	    numCollocationGreen++;
	}
    }
    else {
	val = GalerkinGreenPwc (sp1, sp2);
	if (printGreenTerms)
	    fprintf(stderr,"greenPwc: GalerkinGreenPwc(val=%12.9f)\n", val);
	numGalerkinGreen++;
    }

    return (val);
}

Private green_t greenPwl (spider_t *sp1, spider_t *sp2)
{
    green_t val;
    real_t d, r, r1, r2;
    spiderEdge_t *e;
#ifdef DEBUG
    static int testComputation = 0;

    if (testComputation == 0) {
	testComputation = 1;
	if (paramLookupB ("test_computation", "off")) testComputation = 2;
    }
#endif /* DEBUG */

    r1 = 0;
    for (e = sp1 -> edge; e; e = NEXT_EDGE (sp1, e)) {
	r = spiderDist (sp1, e -> oh -> sp);
	if (r > r1) r1 = r;
    }

    if (sp2 == sp1) {
	d = 0;
	r = r1 + r1;
    }
    else {
	r2 = 0;
	for (e = sp2 -> edge; e; e = NEXT_EDGE (sp2, e)) {
	    r = spiderDist (sp2, e -> oh -> sp);
	    if (r > r2) r2 = r;
	}
	d = spiderDist (sp1, sp2);
	r = r1 + r2;
    }

    if (printGreenTerms)
	fprintf(stderr,"greenPwl: d=%g r=%g sp1=(%g,%g,%g) sp2=(%g,%g,%g)\n", d, r,
		sp1 -> act_x, sp1 -> act_y, sp1 -> act_z,
		sp2 -> act_x, sp2 -> act_y, sp2 -> act_z);

#ifdef DEBUG
    if (testComputation == 2) {
	green_t p = (d > 0 ? PointGreen (sp1, sp2) : 0.0);
	green_t c = CollocationGreenPwl (sp1, sp2);
	green_t g = GalerkinGreenPwl (sp1, sp2);
	green_t d1 = GalerkinGreenPwl (sp1, sp1);
	green_t d2 = GalerkinGreenPwl (sp2, sp2);
	fprintf (stderr, "eps %g %g %g %g %g %g\n",
	     g, d1, d2, (g - c) / g, (g - p) / g, d / r);
	return (g);
    }
#endif /* DEBUG */

    if (d >= r * pointGreenDist && d > 0) {
	val = PointGreen (sp1, sp2);
	if (printGreenTerms)
	    fprintf(stderr,"greenPwl: PointGreen(val=%12.9f)\n", val);
	numPointGreen++;
    }
    else if (d >= r * collocationGreenDist) {
	val = CollocationGreenPwl (sp1, sp2);
	if (printGreenTerms)
	    fprintf(stderr,"greenPwl: CollocationGreenPwl(val=%12.9f)\n", val);
	numCollocationGreen++;
	if (d < r * asymCollocationGreenDist && sp1 != sp2) {
	    /* take arithmetic mean, experiments have shown
	     * that harmonic mean does not make much difference.
	     */
	    green_t val2 = CollocationGreenPwl (sp2, sp1);
	    val = (val + val2) / 2.0;
	    if (printGreenTerms)
		fprintf(stderr,"greenPwl: CollocationGreenPwl(val=%12.9f) val=%12.9f\n", val2, val);
	    numCollocationGreen++;
	}
    }
    else {
	val = GalerkinGreenPwl (sp1, sp2);
	if (printGreenTerms)
	    fprintf(stderr,"greenPwl: GalerkinGreenPwl(val=%12.9f)\n", val);
	numGalerkinGreen++;
    }

    return (val);
}

Private green_t PointGreen (spider_t *sp1, spider_t *sp2)
{
    pointR3_t p1, p2;
    R3Set (p1, sp1);
    R3Set (p2, sp2);
    return (greenFunc (&p1, &p2));
}

/*
 * spo is observation point (potential point)
 * spc is charge point
 */
Private green_t CollocationGreenPwc (spider_t *spo, spider_t *spc)
{
    green_t area, val;
    pointR3_t opp, cp1, cp2, cp3, cp4;
    spider_t **c;

    /* When greenMpole runs into trouble it returns FALSE;
     * then use traditional numerical integration.
     */
    if (useMultiPoles && greenMpole (spo, spc, &val)) return val;

    area = spc -> face -> area;
    c = spc -> face -> corners;
    R3Set (opp, spo);
    R3Set (cp1, c[0]);
    R3Set (cp2, c[1]);
    R3Set (cp3, c[2]);
    if (c[3]) R3Set (cp4, c[3]); /* quadrilateral */
    else cp4 = cp1; /* triangle */

    val = doCollocationGreen (opp, cp1, cp2, cp3, cp4);

    Debug ((void) fprintf (stderr, "area: %g val: %g val/area: %g\n",
		(double) area, (double) val, (double) val/area));

    val /= area; /* normalize so that weight of shape function becomes 1 */

#ifdef TEST_MULTIPOLE_METHOD
    if (testMultiPoles) TestMultipoles (spo, spc, val, counter);
#endif

    return (val);
}

Private green_t CollocationGreenPwl (spider_t *spo, spider_t *spc)
{
    face_t *fc;
    green_t area, val;
    pointR3_t opp, cp1, cp2, cp3;
    spider_t *sp1, *sp2, *sp3;
    spiderEdge_t *e;

    /* When greenMpole runs into trouble it returns FALSE;
     * then use traditional numerical integration.
     */
    if (useMultiPoles && greenMpole (spo, spc, &val)) return (val);

    area = val = 0;

    sp1 = spc;
    for (e = sp1 -> edge; e; e = NEXT_EDGE (sp1, e)) {
	if (!(fc = e -> face)) continue; /* happens with sheet conductors */

	sp2 = ccwa (sp1, fc);
	sp3 = ccwa (sp2, fc);
	ASSERT (sp1 != sp3 && sp1 != sp2 && sp2 != sp3);

	R3Set (opp, spo);
	R3Set (cp1, sp1);
	R3Set (cp2, sp2);
	R3Set (cp3, sp3);

	val  += doCollocationGreen (opp, cp1, cp2, cp3, cp1);
	area += fc -> area;
    }

    Debug ((void) fprintf (stderr, "area: %g val: %g val/area: %g\n",
		(double) area, (double) val, (double) val/area));

    val /= area; /* normalize so that weight of shape function becomes 1 */

#ifdef TEST_MULTIPOLE_METHOD
    if (testMultiPoles) TestMultipoles (spo, spc, val, counter);
#endif

    return (val);
}

/*
 * spo is observation point (potential point)
 * spc is charge point
 */
Private green_t GalerkinGreenPwc (spider_t *spo, spider_t *spc)
{
    green_t area, val;
    pointR3_t op1, op2, op3, op4;
    pointR3_t cp1, cp2, cp3, cp4;
    spider_t **c;

    /* When greenMpole runs into trouble it returns FALSE;
     * then use traditional numerical integration.
     */
    if (useMultiPoles && greenMpole (spo, spc, &val)) return (val);

    area  = spc -> face -> area;
    area *= spo -> face -> area;

    c = spc -> face -> corners;
    R3Set (cp1, c[0]);
    R3Set (cp2, c[1]);
    R3Set (cp3, c[2]);
    if (c[3]) R3Set (cp4, c[3]); /* quadrilateral */
    else cp4 = cp1; /* triangle */

    c = spo -> face -> corners;
    R3Set (op1, c[0]);
    R3Set (op2, c[1]);
    R3Set (op3, c[2]);
    if (c[3]) R3Set (op4, c[3]); /* quadrilateral */
    else op4 = op1; /* triangle */

    val = doGalerkinGreen (op1, op2, op3, op4, cp1, cp2, cp3, cp4);

    Debug ((void) fprintf (stderr, "area: %g val: %g val/area: %g\n",
		(double) area, (double) val, (double) val/area));

    val /= area; /* normalize so that weight of shape function becomes 1 */

#ifdef TEST_MULTIPOLE_METHOD
    if (testMultiPoles) TestMultipoles (spo, spc, val, counter);
#endif

    return (val);
}

Private green_t GalerkinGreenPwl (spider_t *spo, spider_t *spc)
{
    face_t *fc;
    green_t area, ao, val, oval;
    pointR3_t op1, op2, op3;
    pointR3_t cp1, cp2, cp3;
    spider_t *sp1, *sp2, *sp3;
    spiderEdge_t *ec, *eo;

    /* When greenMpole runs into trouble it returns FALSE;
     * then use traditional numerical integration.
     */
    if (useMultiPoles && greenMpole (spo, spc, &val)) return (val);

    area = val = 0;

    for (ec = spc -> edge; ec; ec = NEXT_EDGE (spc, ec)) {
	if (!(fc = ec -> face)) continue; /* happens with sheet conductors */

	area += fc -> area;

	sp1 = spc;
	sp2 = ccwa (sp1, fc);
	sp3 = ccwa (sp2, fc);
	ASSERT (sp1 != sp3 && sp1 != sp2 && sp2 != sp3);

	R3Set (cp1, sp1);
	R3Set (cp2, sp2);
	R3Set (cp3, sp3);

	ao = oval = 0;

	for (eo = spo -> edge; eo; eo = NEXT_EDGE (spo, eo)) {
	    if (!(fc = eo -> face)) continue; /* happens with sheet conductors */

	    ao += fc -> area;

	    sp1 = spo;
	    sp2 = ccwa (sp1, fc);
	    sp3 = ccwa (sp2, fc);
	    ASSERT (sp1 != sp3 && sp1 != sp2 && sp2 != sp3);

	    R3Set (op1, sp1);
	    R3Set (op2, sp2);
	    R3Set (op3, sp3);

	    oval += doGalerkinGreen (op1, op2, op3, op1, cp1, cp2, cp3, cp1);
	}
	val += oval / ao;
    }

    Debug ((void) fprintf (stderr, "area: %g val: %g val/area: %g\n",
		(double) area, (double) val, (double) val/area));

    val /= area; /* normalize so that weight of shape function becomes 1 */

#ifdef TEST_MULTIPOLE_METHOD
    if (testMultiPoles) TestMultipoles (spo, spc, val, counter);
#endif

    return (val);
}
