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

#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/green/green.h"
#include "src/space/green/extern.h"
#include "src/space/green/gputil.h"

/*
 * Normally, I don't want debugging code here,
 * since the efficiency of this routine is extremely important.
 * So, turn debugging of even if it was defined in config.h
 */
#undef  DEBUG /* comment out this line for debugging */
#ifndef DEBUG
#undef  Debug
#define Debug(s) /*nothing*/
#endif  /* DEBUG */

/*
 * Analyticaly compute the potential in <vr> caused by
 * a uniform or linear source distribution over the element
 * given by (vr1, vr2, vr3, vr4).
 *
 * For triangular elements, vr4 should be NULL.
 * (For backwards compatibility, vr4 can also be equal to vr1.)
 *
 * It does not implement a particular potential (green's) function,
 * just 1/r where r is the distance. No permitivities etc.
 *
 * If <do_shape> == TRUE, assume a linear source distribution,
 * being 1.0 at vr1 and 0.0 at vr2 and vr3.
 * With linear shape functions: triangular elements (vr4==vr1).
 *
 * This function implements formulas 5 and 6 of
 * ``Potential Integrals for Uniform and Linear Source Distributions
 * on Polygonal and Polyhedral Domains'', by Wilton et al,
 * IEEE AP-32, No 3, March 1984.
 */
real_t analyticPotIntegral (bool_t do_shape, pointR3_t *vr, pointR3_t *vr1, pointR3_t *vr2, pointR3_t *vr3, pointR3_t *vr4)
{
    real_t val = 0;
    real_t pwc_val = 0;

    /* Nomenclature:
     * vxx	vector
     * uxx	unit vector
     * xxp	superscript +
     * xxm	superscript -
     * xx2	squared variable
     *
     * vr	Observation point
     * vr[1234] Description of S
     * un	Unit vector normal to plane of S
     * d	heigth of vr above S, along un.
     */
    pointR3_t *vrp[4];
    pointR3_t *vrm[4];

    real_t Po, Pm2, Pp2;
    real_t Ro2, Rm, Rp, lm, lp;

    pointR3_t uPo, un, ul;
    pointR3_t vtmp1, vtmp2;
    pointR3_t vs, vsp, vsm, vpwl;
    static pointR3_t uu, vsp1, vsp2, vsp3; /* init by static to suppres compiler warning */
    real_t sign, d, d2, x;
    int i, nSegments;

    R3Subtract (*vr1, *vr3, vtmp1);
    R3Subtract (*vr1, *vr2, vtmp2);
    R3Cross (vtmp1, vtmp2, un);
    R3Normalize (un);

    R3Subtract (*vr, *vr1, vtmp1);
    d = R3Dot (un, vtmp1);
    d2 = d * d;
    d = Abs(d);

    if (vr4 == NULL
    || (vr4 == vr1)
    || (vr1->x == vr4->x && vr1->y == vr4->y && vr1->z == vr4->z)) {
	/* it is a triangle */
	/* do not change this ordering, pwl depends on it. */
	nSegments = 3;
	vrp[0] = vr1, vrm[0] = vr2;
	vrp[1] = vr3, vrm[1] = vr1;
	vrp[2] = vr2, vrm[2] = vr3;
    }
    else {
	/* it is a quadrilateral */
	ASSERT (do_shape == FALSE);
	nSegments = 4;
	vrp[0] = vr1, vrm[0] = vr2;
	vrp[1] = vr2, vrm[1] = vr3;
	vrp[2] = vr3, vrm[2] = vr4;
	vrp[3] = vr4, vrm[3] = vr1;
    }

    pwc_val = 0;

    x = R3Dot (un, *vr);
    R3Scalar (un, x, vtmp1);
    R3Subtract (*vr, vtmp1, vs);

#ifdef DEBUG
    if (DEBUG) {
	fprintf (stderr, "d: %g\n", d);
	r3Print ("un", &un);
	r3Print ("vr", vr);
	r3Print ("vs", &vs);
    }
#endif /* DEBUG */

    R3Assign (vpwl, 0, 0, 0);

    /* In the paper, many variables are indexed.
     * However, in the final summation, only one is needed at a time.
     * Hence, in the for-loop below, we do not compute all variables
     * at once.
     */
    for (i = 0; i < nSegments; i++) {
	R3Subtract (*vrp[i], *vrm[i], ul);
	R3Normalize (ul);

	R3Cross (ul, un, uu);

	x = R3Dot (un, *vrp[i]);
	R3Scalar (un, x, vtmp1);
	R3Subtract (*vrp[i], vtmp1, vsp);

	x = R3Dot (un, *vrm[i]);
	R3Scalar (un, x, vtmp1);
	R3Subtract (*vrm[i], vtmp1, vsm);

	R3Subtract (vsp, vs, vsp);
	R3Subtract (vsm, vs, vsm);

	lp = R3Dot (ul, vsp);
	lm = R3Dot (ul, vsm);

	Po = R3Dot (vsp, uu);
	if (Po < 0) Po = -Po;

	Pp2 = R3Norm2 (vsp);
	Pm2 = R3Norm2 (vsm);

	Ro2 = Po * Po + d2;
	Rp = Sqrt (Pp2 + d2);
	Rm = Sqrt (Pm2 + d2);

#ifdef DEBUG
	if (DEBUG) {
	    fprintf (stderr, "\n");
	    r3Print ("vrp", vrp[i]);
	    r3Print ("vrm", vrm[i]);
	    r3Print ("ul", &ul);
	    r3Print ("uu", &uu);
	    r3Print ("vsp", &vsp);
	    r3Print ("vsm", &vsm);
	}
#endif /* DEBUG */

	if (Po < EPS || Abs (Rp+lp) < EPS || Abs (Rm+lm) < EPS) {
	    /* uPo below is undefined, however, the
	     * contribution of this term to val vanishes.
	     */
	    Debug (fprintf (stderr, "Po = %g, pwc_term -> 0\n", Po));
	}
	else {
	    /* The next statements can be optimized,
	     * I think that uPo need not be normalized,
	     * but sign = x > 0 ? 1 : -1; would be OK.
	     */
	    R3Scalar (ul, lp, vtmp1);
	    R3Subtract (vsp, vtmp1, uPo);
	    R3Normalize (uPo);
	    Debug (r3Print ("uPo", &uPo));
	    sign = R3Dot (uPo, uu);

	    x  = Po * Log ((Rp + lp) / (Rm + lm));
	    if (d > 0) {
		x += d * (  Atan (Po * lm / (Ro2 + d * Rm))
		          - Atan (Po * lp / (Ro2 + d * Rp)));
	    }
	    x *= sign;
	    pwc_val += x;

	    Debug ((fprintf (stderr, "lp %g lm %g Po %g Rp %g Rm %g\n",
			lp, lm, Po, Rp, Rm),
		    fprintf (stderr, "\t\t\tsign: %g pwc_term: %g\n",
			sign, x)));
	}

	if (do_shape) {
	    if (i == 0) {
		R3Copy (vsp, vsp1);
		R3Copy (vsm, vsp2);
	    }
	    if (i == 1) {
		R3Copy (vsp, vsp3);
	    }

	    x = Ro2 > EPS ? Ro2 * Log ((Rp + lp) / (Rm + lm)) : 0;
	    x += lp * Rp - lm * Rm;
	    x *= 0.5; /* Can this be done outside this loop? */

	    R3Scalar (uu, x, vtmp1);
	    R3Add (vpwl, vtmp1, vpwl);

#ifdef DEBUG
	    if (DEBUG) {
		fprintf (stderr, "lp %g lm %g Ro %g Rp %g Rm %g x %g\n",
		    lp, lm, Sqrt (Ro2), Rp, Rm, x);
		r3Print ("pwl_term", &vtmp1);
            }
#endif /* DEBUG */
	}

    }

    if (do_shape) {
	real_t v1, v2;
#ifdef DEBUG
	real_t v3;
#endif

	v1 = R3Dot (vsp1, uu);
	v2 = R3Dot (vsp2, uu);
	x  = R3Dot (vpwl, uu);

#ifdef DEBUG
	v3 = R3Dot (vsp3, uu);
	ASSERT (Abs (v2 - v3) < EPS);
	if (DEBUG) {
	    fprintf (stderr, "val[123]: %g %g %g\n", v1, v2, v3);
	    r3Print ("vpwl", &vpwl);
	}
#endif /* DEBUG */

	val = (3 / (v1 - v2)) * (x - v2 * pwc_val);
    }
    else {
	val = pwc_val;
    }

    Debug (fprintf (stderr, "analytic collocation: %g\n", val));

    // Note: we cannot assert here that val >= 0, because in some situations
    // the value is negligbly small, but negative. Negative green's values
    // will be clipped to zero by the main function "green".

    return (val);
}

real_t analyticPotIntegral2D (int do_shape, pointR3_t *vr, pointR3_t *vrm, pointR3_t *vrp)
{
    real_t val = 0;
    double eps;

    /* Nomenclature:
     * vxx	vector
     * uxx	unit vector
     * xxp	superscript +
     * xxm	superscript -
     * xx2	squared variable
     *
     * vr	Observation point
     * vr[mp]   Description of L
     */

    pointR3_t ul, uu, vrpminr, vrmminr;
    real_t lp, lm, pp, pm, p0;

    /* compute ul */
    R3Subtract (*vrp, *vrm, ul);
    R3Normalize (ul);

    /* compute uu */
    R3Assign (uu, -ul.z, 0, ul.x);

    R3Subtract  (*vrp, *vr, vrpminr);
    R3Subtract  (*vrm, *vr, vrmminr);
    lp = R3Dot  (vrpminr, ul);
    lm = R3Dot  (vrmminr, ul);
    pp = R3Norm (vrpminr);
    pm = R3Norm (vrmminr);
    p0 = R3Dot  (vrpminr, uu);
    if (p0 < 0) p0 = -p0;

#ifdef DEBUG
    if (DEBUG) {
	r3Print ("vr", vr);
	r3Print ("vrp", vrp);
	r3Print ("vrm", vrm);
	r3Print ("uu", &uu);
	r3Print ("ul", &ul);
	fprintf (stderr, "l+ l-: %e %e,\n p+ p-: %e %e,\n p0: %e\n",
	    lp, lm, pp, pm, p0);
    }
#endif /* DEBUG */


    /* scale eps to problem dimensions */
    eps = EPS * (lp - lm);

    val = lm - lp;
    if (Abs (lp) > eps) val += lp * Log (pp);
    if (Abs (lm) > eps) val -= lm * Log (pm);
    if (p0 > eps)       val += p0 * (Atan (lp / p0) - Atan (lm / p0));

    /* normalize so that Log (1/d^2) is returned (see gf2D below) */
    val *= -2.0;

    Debug (fprintf (stderr, "analytic collocation: %g\n", val));

    return (val);
}

#ifdef DRIVER

pointR3_t *Op;

Private real_t gf (pointR3_t *p)
{
    return (1.0 / r3Dist (Op, p));
}

Private real_t gf2D (pointR3_t *p)
{
    double d = r3Dist (Op, p);
    return (Log (1.0 / (d * d)));
}

Private void checkLine (real_t ox, real_t oz, real_t x1, real_t z1, real_t x2, real_t z2)
{
    real_t val;
    pointR3_t p1, p2, op;
    integrate_t result;

    R3Assign (p1, x1, 0, z1);
    R3Assign (p2, x2, 0, z2);
    R3Assign (op, ox, 0, oz);

    fprintf (stderr, "o %g %g r1 %g %g r2 %g %g\n", ox, oz, x1, z1, x2, z2);

    val = analyticPotIntegral2D (FALSE, &op, &p1, &p2);
    fprintf (stderr, "analytic: %g\n", (double) val);

    Op = &op;
    result = integrate1D (FALSE, gf2D, &p1, &p2, 3, 1e-5);
    fprintf (stderr, " numeric: %g, estimated error %g, true error %g\n",
	(double) result.value, (double) result.error,
	(double) Abs ((val - result.value) / val));
}

/*
 * Test driver.
 */
int main ()
{
    real_t val;
    pointR3_t p1, p2, p3, op;
    integrate_t result;

    R3Assign (p1, 0, 0, 0);
    R3Assign (p2, 4, 1, 0);
    R3Assign (p3, 1, 0, 0);
    R3Assign (op, 1, 1, 0);

    val = analyticPotIntegral (TRUE, &op, &p1, &p2, &p3, NULL);
    fprintf (stderr, "analytic: %g\n", (double) val);

    Op = &op;
    result = integrate2D (TRUE, gf, &p1, &p2, &p3, NULL, 1000, 0.0);
    fprintf (stderr, " numeric: %g, estimated error %g, true error %g\n",
	(double) result.value, (double) result.error,
	(double) Abs ((val - result.value) / val));

    val = analyticPotIntegral (FALSE, &op, &p1, &p2, &p3, NULL);
    fprintf (stderr, "analytic: %g\n", (double) val);

    Op = &op;
    result = integrate2D (FALSE, gf, &p1, &p2, &p3, NULL, 1000, 0.0);
    fprintf (stderr, " numeric: %g, estimated error %g, true error %g\n",
	(double) result.value, (double) result.error,
	(double) Abs ((val - result.value) / val));

    fprintf (stderr, "next two should be identical\n");
    checkLine (1.0,  1.0, 1.0, 1.0, 2.0, 1.0);
    checkLine (0.0,  1.0, 0.0, 1.0, 1.0, 1.0);

    checkLine (0.0,  1.0, 1.0, 1.0, 2.5, 1.0);
    checkLine (0.0,  0.0, 1.0, 1.0, 2.0, 1.0);
    checkLine (0.99, 1.0, 1.0, 1.0, 2.0, 1.0);
    checkLine (10.0, 1.0, 1.0, 1.0, 2.0, 1.0);
    checkLine (1.5,  1.1, 1.0, 1.0, 2.0, 1.0);
    checkLine (1.5, 10.0, 1.0, 1.0, 2.0, 1.0);
    checkLine (1.5, 10.0, 1.0, 1.0, 2.0, 0.0);
    return (0);
}

#endif /* DRIVER */
