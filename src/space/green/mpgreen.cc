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

/*****************************************************************************
*  Routine for calculating influence matrix elements
*  using Cartesian multipole expansion.
*
*  Version     Apr. 12, 1996
*  May   21, 1996:  Bug fix, oMedium type was not set correctly for
*                                 collocation
*  Sept.     1996:  Joined with file mputil.c
*
*                   Structure spider_t augmented by a pointer  *carmom_t.
*
*                   Multipoles once calculated now stored, accessible via the
*                   spider at the point with respect to which the moments were
*                   calculated.  In the course of this operation, data type
*                   of some variables changed (e.g. from carmom_t to *carmom_t).
*
*                   Type of greenMpole changed to boolean, result of calculation
*                   passed via pointer.  If result == FALSE, the multipole
*                   routine couldn't handle it (occurs at present e.g. for
*                   zero-thickness conductors and 1g); then the conventional
*                   calculation of the Green function takes over.  Previously,
*                   the routine greenMpole (and/or children) would just exit
*                   or crash when encountering such a situation.
*
*                   Put stuff only important for testing the multipole method
*                   by comparison with the conventional method between
*                   #ifdef TEST_MULTIPOLE_METHOD  .... #endif
*******************************************************************************/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/include/spider.h"
#include "src/space/green/green.h"
#include "src/space/green/extern.h"
#include "src/space/green/gputil.h"
#include "src/space/spider/define.h"
#include "src/space/spider/export.h"

extern int maxGreenTerms, MaxMpOrder, MaxMpNr;
extern int numMultipoleImages;
extern int numMultipoleImagesTraditional;
extern green_t collocationEps;
extern int nLayers[];
extern long green_cnt;
extern real_t multipolesMindist;
extern bool_t useAdptvIntgrtn;
extern bool_t useMeanGreenValues;
extern bool_t forceMpExInt;
extern bool_t forceMpAnaInt;
extern bool_t testMultiPoles;
extern bool_t printGreenTerms;
extern bool_t printGreenGTerms;

struct Carmom {
/* Provide storage for cartesian multipole moments. */
  real_t *dpl, pieArea, radi;
  int diel;
};

typedef struct Carmom carmom_t;

/* The macro R3Set is defined in green.c, copied from there. */
#define R3Set(gp,sp) R3Assign(gp, sp->act_x, sp->act_y, sp->act_z)

#define max_nr_peripheral_points 20
#define two_times_max_nr_peripheral_points 40

/*  global variables needed for communication between functions
 *  (these typically cannot be passed as argument because some
 *  functions serve as integrand in an integration routine, and
 *  their argument structure is therefore constrained).
 *
 *    aPoint is used as 2nd argument of (free space) Green function
 *  when integrating over 1st argument;
 *    centerPoint and Peripoints describe the source pie in the
 *  functions  potFromPie (observPoint)  and  potFromPieA (observPoint),
 *  and Nrpp is the number of peripheral points of this pie;
 *     potFromPieReqRelPrec communicates to the function potFromPie
 *  the required precision of numerical integration, and potFromPieError
 *  communicates back the error estimate.
 *    FeModePwl indicates the type of shape function (piecewise constant
 *  for FeModePwl == FALSE, piecewise linear for FeModePwl == TRUE).
 */
Private pointR3_t *aPoint, *periPoints, *centerPoint;
Private real_t potFromPieReqRelPrec, potFromPieError;
Private int Nrpp;

#ifdef TEST_MULTIPOLE_METHOD
/*  global variables for communication with the diagnostic function `TestMultipoles' */
Private real_t smallestDistratio, minRadius;
#endif

/* Scalar product for vectors defines as arrays (not pointR3_t) */
#define VV(a,b) (a[0] * b[0] + a[1] * b[1] + a[2] * b[2])

/* Vector . Matrix . Vector  */
#define VMV(v,q,u) \
    ( v[0] * (q[0] * u[0] + q[1] * u[1] + q[2] * u[2]) \
    + v[1] * (q[3] * u[0] + q[4] * u[1] + q[5] * u[2]) \
    + v[2] * (q[6] * u[0] + q[7] * u[1] + q[8] * u[2]) )

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void carmomMirror (real_t *mom, real_t *mir);
Private real_t carmomMake (real_t *mom, pointR3_t *p1, pointR3_t *p2, pointR3_t *p3);
Private real_t mpConnect (real_t *cMom, real_t *oMom, pointR3_t *Rvec, real_t R);
Private void makeTotalMoments (carmom_t *total_moments, pointR3_t *c, pointR3_t *p, int nrpp);
Private real_t invDist_aPoint (pointR3_t *secondPoint);
Private real_t potFromPie  (pointR3_t *observPoint);
Private real_t potFromPieA (pointR3_t *observPoint);
Private real_t intPieGreenPie (
	pointR3_t cCenter, pointR3_t *cPeriPoints, int cNrpp, real_t cPieArea,
	pointR3_t oCenter, pointR3_t *oPeriPoints, int oNrpp, real_t oPieArea,
	real_t reqRelPrecision);
Private bool_t getPiePoints (spider_t *s, pointR3_t *center, pointR3_t *p, int *nrpp);
#ifdef __cplusplus
  }
#endif

void disposeSpiderMoments (spider_t *spider)
{
    DISPOSE (spider -> moments -> dpl, MaxMpNr * sizeof(real_t));
    DISPOSE (spider -> moments, sizeof(carmom_t));
}

/***************************************************************************
*
* Print out first multipole moments.  Auxiliary function for test purposes.
*
****************************************************************************/
void carmomPrint (char *s, carmom_t *m)
{
    int i;
    real_t *dpl = m->dpl;

    fprintf (stderr, "%s:\n", s);
    fprintf (stderr, "monopole: 1\n");
    if (MaxMpOrder < 1) return;

    fprintf (stderr, "dpl:   %g %g %g\n", dpl[0], dpl[1], dpl[2]);
    if (MaxMpOrder < 2) return;

    dpl += 3;
    fprintf (stderr, "qdrpl: %g %g %g %g %g %g %g %g %g\n",
	dpl[0], dpl[1], dpl[2],
	dpl[3], dpl[4], dpl[5],
	dpl[6], dpl[7], dpl[8]);
    if (MaxMpOrder < 3) return;

    dpl += 9;
    for (i = 0; i < 27; i += 9)
    fprintf (stderr, "octpl: %g %g %g %g %g %g %g %g %g\n",
	dpl[i+0], dpl[i+1], dpl[i+2],
	dpl[i+3], dpl[i+4], dpl[i+5],
	dpl[i+6], dpl[i+7], dpl[i+8]);
}

/***********************************************************************
*
* Return mirror image with respect of z-coordinate of multipole moments;
* i.e., all tensor components with an odd number of z-indices are multiplied by -1.
*
************************************************************************/
Private void carmomMirror (real_t *dpl, real_t *mir)
{
    int i, j, k;

    if (MaxMpOrder < 1) return;

    mir[0] =  dpl[0];
    mir[1] =  dpl[1];
    mir[2] = -dpl[2];
    if (MaxMpOrder < 2) return;

    mir += 3;
    dpl += 3;
    mir[0+0] =  dpl[0+0];
    mir[0+1] =  dpl[0+1];
    mir[0+2] = -dpl[0+2];
    mir[3+0] =  dpl[3+0];
    mir[3+1] =  dpl[3+1];
    mir[3+2] = -dpl[3+2];
    mir[6+0] = -dpl[6+0];
    mir[6+1] = -dpl[6+1];
    mir[6+2] =  dpl[6+2];
    if (MaxMpOrder < 3) return;

    mir += 9;
    dpl += 9;
    /* if an odd number of indices equal 2 (== z index), multiply by -1 */
    for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
    for (k = 0; k < 3; k++) {
	mir[i*9+j*3+k] = (i / 2 + j / 2 + k / 2 == 1)? -dpl[i*9+j*3+k] : dpl[i*9+j*3+k];
    }
    mir[2*9+2*3+2] = -dpl[2*9+2*3+2];
}

/***********************************************************************
*
* Calculate cartesian multipole moments (up to and including order
* MaxMpOrder) with respect to point p1 of a charge density on the triangle
* p1, p2, p3.  For FeModePwl == FALSE the charge density is constant and
* equal to unity.  Otherwise, the charge density equals unity at p1
* and decreases linearly, vanishing on the line (p2, p3).  The area
* of the triangle is calculated, too.
*
************************************************************************/
Private real_t carmomMake (real_t *dpl, pointR3_t *p1, pointR3_t *p2, pointR3_t *p3)
{
    int i, j, k;
    real_t a[3], b[3], aa, bb, ab, n[3], nn, area;
    /*  a, b = vectors from p1 to p2 and from p1 to p3;
     *  n = normal to a-b, such that scalar products (a,n) and (b,n) are positive.
     */
    real_t fdpl, fqdrpl, foctpl;     /* scale factors */
    real_t *qdrpl, *octpl;

    a[0] = p2->x - p1->x;
    a[1] = p2->y - p1->y;
    a[2] = p2->z - p1->z;
    b[0] = p3->x - p1->x;
    b[1] = p3->y - p1->y;
    b[2] = p3->z - p1->z;

    aa = VV (a, a);
    bb = VV (b, b);
    ab = VV (a, b);

    nn = 0;
    for (i = 0; i < 3; i++) {
        n[i] = a[i] * bb + b[i] * aa - (a[i] + b[i]) * ab;
        nn += n[i] * n[i];
    }
    nn = Sqrt (nn);
    ASSERT (nn > 0); /* else degenerate triangle */
    for (i = 0; i < 3; i++) n[i] /= nn;

    area = Sqrt (aa - 2 * ab + bb) / 2 * VV (a, n);
    ASSERT (area > 0);

    qdrpl = dpl + 3;
    if (MaxMpOrder <= 2) {
	if (MaxMpOrder < 1) return (area);
	if (FeModePwl) {
	    fdpl = area / 4; fqdrpl = area / 40;
	} else {
	    fdpl = area / 3; fqdrpl = area / 24;
	}
	dpl[0] += fdpl * (a[0] + b[0]);
	dpl[1] += fdpl * (a[1] + b[1]);
	dpl[2] += fdpl * (a[2] + b[2]);
	if (MaxMpOrder > 1)
	for (i = 0; i < 3; i++) {
	    for (j = 0; j < 3; j++) {
		qdrpl[i*3+j] += fqdrpl * (2 * a[i] * a[j]
					    + a[i] * b[j]
					    + b[i] * a[j]
					+ 2 * b[i] * b[j]);
	    }
	}
    }
    else {
	if (FeModePwl) {
	    fdpl = area / 4; fqdrpl = area / 40; foctpl = area / 360;
	} else {
	    fdpl = area / 3; fqdrpl = area / 24; foctpl = area / 180;
	}
	octpl = dpl + 12;
	for (i = 0; i < 3; i++) {
	    dpl[i] += fdpl * (a[i] + b[i]);
	    for (j = 0; j < 3; j++) {
		qdrpl[i*3+j] += fqdrpl * (2 * a[i] * a[j]
					    + a[i] * b[j]
					    + b[i] * a[j]
					+ 2 * b[i] * b[j]);
		for (k = 0; k < 3; k++) {
		    octpl[i*9+j*3+k] += foctpl * (3 * a[i] * a[j] * a[k]
						    + a[i] * a[j] * b[k]
						    + a[i] * b[j] * a[k]
						    + a[i] * b[j] * b[k]
						    + b[i] * a[j] * a[k]
						    + b[i] * a[j] * b[k]
						    + b[i] * b[j] * a[k]
						+ 3 * b[i] * b[j] * b[k]);
		}
	    }
	}
    }
    return (area);
}

/***********************************************************************
*
* Contract charge- and observation multipole moments, with the
* proper powers of the inverse distance of the pie-centers included.
*     For FeModeGalerkin == FALSE, on the observation side only the
* monopole is taken into account.
*     The vector Rvec (length R)  points  from the center of the charge
* pie to the center of the obervation pie.
*     For an explanation of the calculation see "Acceleration of the
* boundary element method for capacitance extraction by multipole
* expansion" by U. Geigenmuller (Internal Report Space project, March 25, 1996).
*
***********************************************************************/
Private real_t mpConnect (real_t *c_dpl, real_t *o_dpl, pointR3_t *Rvec, real_t R)
{
    real_t u[3], result;
    /* Comment on nomenclature of the variables that follow:
     * 'C' and 'O' stand for "charge" and "observertion", respectively;
     * a number stands for the multipole order (e.g. 2 -> 2^2 pole = quadrupole);
     * an 'u' stands for contraction with the unit vector u (= Rvec/R);
     * an "tr" stands for a (partial) trace.
     * Examples:    uO1   =  \sum_{i}      u_i (oMom->dpl)_{i,j,j}
     *              utrO3 =  \sum_{i,j}    u_i (oMom->octpl)_{i,j,j}
     *              uO3C2 =  \sum_{i,j,k}  u_i (oMom->octpl)_{i,j,k} (cMom->qdrpl)_{k,j}
     *              uuO3_i = \sum_{j,k}    u_j U_k (oMom->octpl)_{k,j,i}
     */
    real_t uC1, uO1, tmp,
           uO2u, uC2u, trO2, trC2, uO2[3], uC2[3], O2C2,
           trO3[3], trC3[3], uuO3[3], uuC3[3], utrO3, utrC3,
           uuuC3, uuuO3, uO3C2, uC3O2, O3C3, uO3C3u;
    real_t R2, R3, R4, R5, R6;
    real_t *c_qdrpl, *c_octpl;
    real_t *o_qdrpl, *o_octpl;
    int i, j, k, l;

 // ASSERT (R > 0);

    /**** monopoles ****/
    result = 1 / R;

    if (MaxMpOrder < 1) return result;

    u[0] = Rvec->x / R;
    u[1] = Rvec->y / R;
    u[2] = Rvec->z / R;

    /**** add dipoles ****/
    R2 = R * R;
    R3 = R2 * R;
    uC1 = VV (u, c_dpl);

    if (FeModeGalerkin) {
	uO1 = VV (u, o_dpl);
	/* 1/R^2 term */
	result += (-uO1 + uC1) / R2;
	/* 1/R^3 term */
	result += -(3 * uO1 * uC1 - VV (c_dpl, o_dpl)) / R3;
    }
    else {
	uO1 = 0;
	result += uC1 / R2;
    }

    if (MaxMpOrder < 2) return result;

    c_qdrpl = c_dpl + 3;
    o_qdrpl = o_dpl + 3;

    /**** add quadrupoles ****/
    R4 = R3 * R;
    R5 = R4 * R;
    uC2u = VMV (u, c_qdrpl, u);
    trC2 = c_qdrpl[0] + c_qdrpl[4] + c_qdrpl[8];

    if (FeModeGalerkin) {
        uO2u = VMV (u, o_qdrpl, u);
        trO2 = o_qdrpl[0] + o_qdrpl[4] + o_qdrpl[8];

        /* 1/R^3 term */
        result += ((3 * uO2u - trO2) + (3 * uC2u - trC2)) / R3;

        /* 1/R^4 term */
        for (i = 0; i < 3; i++) {
            uO2[i] = uC2[i] = 0;
            for (j = 0; j < 3; j++) {
                uO2[i] += u[j] * o_qdrpl[j*3+i];
                uC2[i] += u[j] * c_qdrpl[j*3+i];
            }
        }
        result += (-trO2 * uC1 - 2 * VV (uO2, c_dpl) + 5 * uO2u * uC1
		  + trC2 * uO1 + 2 * VV (uC2, o_dpl) - 5 * uC2u * uO1) * 3 / R4;

        /* 1/R^5 term */
        O2C2 = 0;
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 3; j++) O2C2 += o_qdrpl[i*3+j] * c_qdrpl[j*3+i];
        }
        result += (trO2 * trC2 + 2 * O2C2
                   - 5 * (uO2u * trC2 + trO2 * uC2u + 4 * VV (uO2, uC2))
                   + 35 * uO2u * uC2u) * 3 / R5;
    }
    else {
	uO2u = trO2 = 0;
        result += (3 * uC2u - trC2) / R3;
    }

    if (MaxMpOrder < 3) return result;

    c_octpl = c_dpl + 12;
    o_octpl = o_dpl + 12;

    /**** add octopoles ****/
    if (FeModeGalerkin) {
        /* 1/R^4 term */
        for (i = 0; i < 3; i++) {
            uuO3[i] = uuC3[i] = trO3[i] = trC3[i] = 0;
            for (j = 0; j < 3; j++) {
                trO3[i] += o_octpl[i*9+j*3+j];
                trC3[i] += c_octpl[i*9+j*3+j];
                for (k = 0; k < 3; k++) {
                    uuO3[i] += o_octpl[i*9+j*3+k] * u[k] * u[j];
                    uuC3[i] += c_octpl[i*9+j*3+k] * u[k] * u[j];
                }
            }
        }
        utrO3 = VV (u, trO3);
        uuuO3 = VV (u, uuO3);
        utrC3 = VV (u, trC3);
        uuuC3 = VV (u, uuC3);

	result += ((9 * utrO3 - 15 * uuuO3)
		 - (9 * utrC3 - 15 * uuuC3)) / R4;

        /* 1/R^5 term */
	result += (-9 * VV (trO3, c_dpl) +  45 * utrO3 * uC1
		 + 45 * VV (uuO3, c_dpl) - 105 * uuuO3 * uC1
		 -  9 * VV (trC3, o_dpl) +  45 * utrC3 * uO1
		 + 45 * VV (uuC3, o_dpl) - 105 * uuuC3 * uO1) / R5;

        /* 1/R^6 term (can be optimized!!) */
	uC3O2 = 0;
	uO3C2 = 0;
	O3C3  = 0;
	uO3C3u = 0;
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 3; j++) {
                for (k = 0; k < 3; k++) {
                    tmp = u[i] * o_octpl[i*9+j*3+k];
                    uO3C2 += tmp * c_qdrpl[j*3+k];
                    uC3O2 += u[i] * c_octpl[i*9+j*3+k] * o_qdrpl[j*3+k];
                    O3C3  += o_octpl[i*9+j*3+k] * c_octpl[k*9+j*3+i];
                    for (l = 0; l < 3; l++) {
                        uO3C3u += tmp * c_octpl[k*9+j*3+l] * u[l];
                    }
                }
            }
        }
        R6 = R5 * R;
        result += (-45 * utrO3 * trC2 - 90 * VV (trO3, uC2) - 90 * uO3C2
		+ 105 * uuuO3 * trC2 + 630 * VV (uuO3, uC2)
		+ 315 * utrO3 * uC2u - 945 * uuuO3 * uC2u
		+  45 * utrC3 * trO2 +  90 * VV (trC3, uO2) + 90 * uC3O2
		- 105 * uuuC3 * trO2 - 630 * VV (uuC3, uO2)
		- 315 * utrC3 * uO2u + 945 * uuuC3 * uO2u) / R6;

        R6 *= R; /* 1/R^7 term */
        result += (9 * VV (trO3, trC3) + 6 * O3C3
		- 63 * VV (uuO3, trC3) - 63 * VV (trO3, uuC3)
		- 63 * utrO3 * utrC3 - 126 * uO3C3u
		+ 189 * uuuO3 * utrC3 + 189 * utrO3 * uuuC3
		+ 567 * VV (uuO3, uuC3)
		- 693 * uuuO3 * uuuC3) * 15 / R6;
    }
    else {
        /* 1/R^4 term */
        for (i = 0; i < 3; i++) {
            uuC3[i] = trC3[i] = 0;
            for (j = 0; j < 3; j++) {
                trC3[i] += c_octpl[i*9+j*3+j];
                for (k = 0; k < 3; k++) {
                    uuC3[i] += c_octpl[i*9+j*3+k] * u[k] * u[j];
                }
            }
        }
        utrC3 = VV (u, trC3);
        uuuC3 = VV (u, uuC3);
        result += -(9 * utrC3 - 15 * uuuC3) / R4;
    }
    return result;
}

/***********************************************************************
*
* Calculate cartesian multipole moments of `pie', by first calculating
* them for each triangular piece of the pie and then adding the results.
* The area of the pie is also calcuated.
*
***********************************************************************/
Private void makeTotalMoments (carmom_t *total_moments, pointR3_t *c, pointR3_t *p, int nrpp)
/*
 * c is the center of a pie,                p[0]=p[4]________p[3]
 * p[] contains the peripheral                      |\      /|
 * corners of the pie.  For constant shape          | \    / |
 * function, c is in the center of a boundary       |  \  /  |
 * element, and the 3 or 4 corners of               |   \/   |
 * the boundary element are the peripheral          |   c    |
 * points.  For linear shape function,              |   /\   |
 * c is a node of the mesh, and the peripheral      |  /  \  |
 * points are all meshpoints connected to           | /    \ |
 * c by an edge of the mesh.                        |/______\|
 * The number of peripheral points                p[1]      p[2]
 * is nrpp, and p[0]=p[nrpp].
 *   The routine calculates the total (cartesian) multipoles moments
 * of a surface charge density on the pie.  The total charge equals unity.
 * For linear shape function (FeModePwl == TRUE) the charge density
 * is continuous, varies linearly on each piece of pie, is non-zero at the center,
 * and vanishes on the periphery.  For piecewise constant shape function (FeModePwl == FALSE),
 * the charge density is constant on the pie.  Outside the pie the charge density vanishes.
 */
{
    real_t pie_area;
    int i;

    for (i = 0; i < MaxMpNr; i++) total_moments -> dpl[i] = 0; /* init */

    pie_area = 0;
    for (i = 1; i <= nrpp; i++) {
	pie_area += carmomMake (total_moments -> dpl, c, &p[i], &p[i-1]);
    }
    total_moments -> pieArea = pie_area;

    for (i = 0; i < MaxMpNr; i++) total_moments -> dpl[i] /= pie_area; /* scale */
}

/**********************************************************************
*
* Calculate the inverse of the distance between the point *secondPoint
* and the point *aPoint.  The latter is declared globally, because the
* present function may be needed as integrand in an integration routine,
* and must therefore take one argument only.
*
***********************************************************************/
Private real_t invDist_aPoint (pointR3_t *secondPoint)
{
    static real_t dx, dy, dz, max1, max, sum, root;

    dx = (secondPoint->x > aPoint->x)? secondPoint->x - aPoint->x : aPoint->x - secondPoint->x;
    dy = (secondPoint->y > aPoint->y)? secondPoint->y - aPoint->y : aPoint->y - secondPoint->y;
    dz = (secondPoint->z > aPoint->z)? secondPoint->z - aPoint->z : aPoint->z - secondPoint->z;

/*  Calculate square root using Newton Raphson, which is faster than
 *  sqrt from math library when compiled with optimization option
 */
    max1 = dx > dy ? dx : dy;
    max = max1 > dz ? max1 : dz;
    sum = dx * dx + dy * dy + dz * dz;
    root = 1.73205 * max;
    root = 0.5 * (root + sum / root);
    root = 0.5 * (root + sum / root);
    root = 0.5 * (root + sum / root);

    return (1. / root);
}

/**************************************************************************
*
* Calculate by   n u m e r i c a l   i n t e g r a t i o n  the potential
* created at point *observPoint by a charge-density on the `pie' of boundary
* elements with center *centerPoint and peripheral points
* periPoints[0], ..., periPoints[Nrpp-1].  The points describing
* the pie are declared as global variables, because the present function
* may be needed as integrand of an integration routine and must therefore
* take one argument only.
*
***************************************************************************/
Private real_t potFromPie (pointR3_t *observPoint)
{
    int maxRefines = 100, ic;
    integrate_t result, term;

    aPoint = observPoint;

    if (FeModePwl) {
	result.value = 0;
	result.error = 0;
	for (ic = 0; ic < Nrpp; ic++) {
	    term = integrate2D (FeModePwl, invDist_aPoint,
		centerPoint, periPoints + ic, periPoints + ic + 1, NULL,
		maxRefines, potFromPieReqRelPrec);
	    result.value += term.value;
	    result.error += Fabs (term.value * term.error);
	}
	result.error = Fabs (result.error / result.value);
    }
    else {
	result = integrate2D (FeModePwl, invDist_aPoint,
		periPoints, periPoints + 1, periPoints + 2, periPoints + 3,
		maxRefines, potFromPieReqRelPrec);
    }
    potFromPieError = result.error;
    return result.value;
}

/**************************************************************************
*
* Calculate   a n a l y t i c a l l y   the potential
* created at point *observPoint by a charge-density on the `pie' of boundary
* elements with center *centerPoint and peripheral points
* periPoints[0], ..., periPoints[Nrpp-1].  The points describing
* the pie are declared as global variables, because the present function
* may be needed as integrand of an integration routine and must therefore
* take one argument only.
*
***************************************************************************/
Private real_t potFromPieA (pointR3_t *observPoint)
{
    int ic;
    real_t result;

    if (FeModePwl) {
	result = 0;
	for (ic = 0; ic < Nrpp; ic++) {
	    result += analyticPotIntegral (FeModePwl, observPoint,
		centerPoint, periPoints + ic, periPoints + ic + 1, NULL);
	}
    }
    else {
	result = analyticPotIntegral (FeModePwl, observPoint,
		periPoints, periPoints + 1, periPoints + 2, periPoints + 3);
    }
    return result;
}

/**************************************************************************
*
* Calculate the integal of the Coulomb potential over both charge(image)-
* and observation-pie.  Needed for the cases when both pies are too
* close for multipole expansion to be applied.
*
* The integration method to be used is controlled by the Booleans
* forceMpAnaInt, forceMpExInt, and useAdptvIntgrtn.
*
* Inner (collocation) integral:
* - For forceMpAnaInt == FALSE and forceMpExInt == FALSE, this integral is
*   performed using the routine integrate2D.
* - If the estimated error does not meet the precision requirement, or
*   if forceMpAnaInt == TRUE, the inner integral is done analytically.
*
* Outer (Galerkin) integral:
* - If forceMpExInt == FALSE, this integral is done using the routine integrate2D.
* - If the estimated error does not meet the precision requirement and
*   useAdptvIntgrtn == TRUE, or if forceMpAnaInt == TRUE, the inner integral
*   is done analytically and the outer one using the routine integrate2DAdptv.
*
***************************************************************************/
Private real_t intPieGreenPie (pointR3_t cCenter, pointR3_t *cPeriPoints, int cNrpp, real_t cPieArea,
		pointR3_t oCenter, pointR3_t *oPeriPoints, int oNrpp, real_t oPieArea, real_t reqRelPrecision)
{
    int pieOverlap, ic, io;
    integrate_t term, result;

    result.value = 0;
    result.error = 0;
    pieOverlap = 0;
    potFromPieReqRelPrec = reqRelPrecision;

    Nrpp = cNrpp;                /* copy geometry info of (image of) */
    periPoints = cPeriPoints;    /* c-pie to global variables        */
    centerPoint = &cCenter;

    if (FeModePwl) { /* In case of piecewise linear shape function */

	if (!forceMpExInt && !forceMpAnaInt) {
	    /* If source and observation pie overlap, this gives
	     * problems with the numerical integration because of the
	     * singularity of 1/r.  Then analytical treatment of the
	     * inner (collocation) integral is needed.  Check for overlap,
	     * but only if analytic integration is not enforced anyway
	     * (switch forceMpExInt).
	     */
	    if (oCenter.x == cCenter.x && oCenter.y == cCenter.y && oCenter.z == cCenter.z)
		pieOverlap = 1;
	    else
	    for (ic = 0; ic < cNrpp; ic++)
		if (oCenter.x == cPeriPoints[ic].x && oCenter.y == cPeriPoints[ic].y
						   && oCenter.z == cPeriPoints[ic].z) {
		    pieOverlap = 1; break;
		}
	}

	if (FeModeGalerkin) { /* Galerkin method, piecewise linear shape function */
	    int analytic = (pieOverlap || forceMpAnaInt);

	    for (io = 0; io < oNrpp; io++) {
		if (!forceMpExInt) {
		    if (!analytic) {
			term = integrate2D (1, potFromPie, &oCenter, oPeriPoints + io,
					oPeriPoints + io + 1, NULL, 100, reqRelPrecision);
			Debug (fprintf (stderr, "1g, both integrals Stroud formulas\n"));
		    }
		    if (analytic || (Fabs (term.error) > reqRelPrecision)) {
			/* calculate inner integral analytically */
			term = integrate2D (1, potFromPieA, &oCenter, oPeriPoints + io,
					oPeriPoints + io + 1, NULL, 100, reqRelPrecision);
			Debug (fprintf (stderr, "1g, inner integral analytic, outer Stroud formulas\n"));
		    }
		}
		if (forceMpExInt || ((Fabs (result.error) > reqRelPrecision) && useAdptvIntgrtn)) {
		    term = integrate2DAdptv (1, potFromPieA, &oCenter, oPeriPoints + io,
					oPeriPoints + io + 1, NULL, 10000, reqRelPrecision);
		    Debug (fprintf (stderr, "1g, inner integral analytic, outer adaptive\n"));
		}

		/* To get the proper relative error for the pie, first sum in result.error
		 * the absolute error. After the summation over all pieces of the pie is done,
		 * calculate the pie's relative error, and store it in result.error.
		 */
		result.value += term.value;
		result.error += (term.error * term.value);
	    }
	    result.error = Fabs (result.error / result.value);
	    result.value = result.value / cPieArea / oPieArea;
	}
	else { /* Collocation method, piecewise linear shape function */
	    int analytic = (forceMpExInt || pieOverlap || forceMpAnaInt);

	    if (!analytic) {
		result.value = potFromPie (&oCenter);
		result.error = potFromPieError;
	    }
	    if (analytic || (Fabs (result.error) > reqRelPrecision)) {
		/* calculate inner integral analytically */
		result.value = potFromPieA (&oCenter);
		result.error = 0;
	    }
	    result.value = result.value / cPieArea;
	}
    }
    else { /* In case of constant shape function */

	if (!forceMpExInt && !forceMpAnaInt) {
	    /* If source and observation pie overlap, this gives
	     * problems with the numerical integration because of the
	     * singularity of 1/r.  Then analytical treatment of the
	     * inner (collocation) integral is needed.  Check for overlap,
	     * but only if analytic integration is not enforced anyway
	     * (switch forceMpExInt).
	     */
	    if (cCenter.x == oCenter.x && cCenter.y == oCenter.y && cCenter.z == oCenter.z)
		pieOverlap = 1;
	}

	if (FeModeGalerkin) { /* Galerkin method, constant shape function */
	    if (!forceMpExInt) {
		int analytic = (pieOverlap || forceMpAnaInt);

		if (!analytic) {
		    result = integrate2D (0, potFromPie, oPeriPoints, oPeriPoints + 1,
				oPeriPoints + 2, oPeriPoints + 3, 10, reqRelPrecision);
		}
		if (analytic || (Fabs (result.error) > reqRelPrecision)) {
		    /* calculate inner integral analytically */
		    result = integrate2D (0, potFromPieA, oPeriPoints, oPeriPoints + 1,
				oPeriPoints + 2, oPeriPoints + 3, 10, reqRelPrecision);
		}
	    }
	    if (forceMpExInt || ((Fabs (result.error) > reqRelPrecision) && useAdptvIntgrtn)) {
		result = integrate2DAdptv (0, potFromPieA, oPeriPoints, oPeriPoints + 1,
				oPeriPoints + 2, oPeriPoints + 3, 10000, reqRelPrecision);
	    }
	    result.value = result.value / cPieArea / oPieArea;
	}
	else { /* Collocation method, constant shape function */
	    int analytic = (forceMpExInt || pieOverlap || forceMpAnaInt);

	    if (!analytic) {
		/* calculate inner integral numerically */
		potFromPieReqRelPrec = reqRelPrecision;
		result.value = potFromPie (&oCenter);
		result.error = potFromPieError;
	    }
	    if (analytic || (Fabs (result.error) > reqRelPrecision)) {
		/* calculate inner integral analytically */
		result.value = potFromPieA (&oCenter);
		result.error = 0;
	    }
	    result.value = result.value / cPieArea;
	}
    }

    return result.value;
}

/**************************************************************************
*
* For a given spider characterizing an index of the influence matrix,
* determine the center and the peripheral points of the corresponding
* `pie' (a boundary element for constant shape function, and a set of
* triangular boundary elements for piecewise linear shape function).
* Also determine the dielectic embedding the pie, and the pie's radius.
* If everything is OK for use of multipole expansion return TRUE,
* otherwise (e.g. when dealing with a conductor of thickness 0, or
* when the pie has parts in regions of different dielectric constant)
* return FALSE.
*
***************************************************************************/
Private bool_t getPiePoints (spider_t *s, pointR3_t *center, pointR3_t *p, int *nrpp)
{
    static spider_t *sAux, *tSp[two_times_max_nr_peripheral_points];
    int i, j, n, dielec;
    spiderEdge_t *e;
    face_t *fc;
    spider_t **cornerSpiders;
    carmom_t *m;
    real_t aux, radius;

    if (!(m = s -> moments)) {
	s -> moments = m = NEW (carmom_t, 1);
	m -> dpl = NEW (real_t, MaxMpNr);
	m -> diel = -1;
    }
    else if (m -> diel < 0) return (FALSE);

    R3Set (*center, s);

    if (FeModePwl) { /* piecewise linear shape function */
	i = 0;
        for (e = s->edge; e; e = NEXT_EDGE (s, e)) {
            fc = e->face;
            if (fc == NULL) {
                /* fc can be NULL with sheet conductors
                 * This case can at present not be handled by the multipole routine,
                 */
                return (FALSE);
            }

            if (i >= 2 * max_nr_peripheral_points) {
                /*
                printf ("There seem to be more than %d peripheral points\n", i / 2);
                printf ("of a pie - can't handle that!");
                */
                return (FALSE);
            }
	    j = i + 1;
	    tSp[i] = ccwa (s, fc);
	    tSp[j] = ccwa (tSp[i], fc);
	    sAux   = ccwa (tSp[j], fc); /* check for triangular face */
	    if (sAux != s) {
		ASSERT (sAux == s);
		printf ("non-triangular face!!\n");
		printf ("%10.3e %10.3e %10.3e\n", s->act_x, s->act_y, s->act_z);
		s = tSp[i];
		printf ("%10.3e %10.3e %10.3e\n", s->act_x, s->act_y, s->act_z);
		s = tSp[j];
		printf ("%10.3e %10.3e %10.3e\n", s->act_x, s->act_y, s->act_z);
		s = sAux;
		printf ("%10.3e %10.3e %10.3e\n", s->act_x, s->act_y, s->act_z);
		return (FALSE);
	    }
	    i += 2;
        }
	*nrpp = i / 2;

        /* The array tSp (triangular Spiders) now contains pointers to all peripheral
         * spiders of the pie.  s together with entries 0 and 1, s together
         * with entries 2 and 3, etc. are the corners of the triangular faces.
         * Since each peripheral corner is member of two triangles, the
         * corresponding spider pointers are contained twice among the
         * nt elements of tSp.  Now extract the ordered sequence
         * of peripheral points.
         */

        i = 0;
        for (n = 1; n <= *nrpp; n++) {
            R3Set (p[n], tSp[i]);
            for (j = 0; (tSp[i] != tSp[j]) || (j == i); j++);
            tSp[i] = NULL;
            i = (j % 2 == 0) ? j + 1 : j - 1;
        }
    }
    else {
        /*  piecewise constant shape function.  Code for getting boundary
         *  element corners copied from GalerkinGreenPwc (spo, spc).
         */
        cornerSpiders = s->face->corners;
        R3Set (p[1], cornerSpiders[0]);
        R3Set (p[2], cornerSpiders[1]);
        R3Set (p[3], cornerSpiders[2]);
        if (cornerSpiders[3]) {
            R3Set (p[4], cornerSpiders[3]);
            *nrpp = 4; /* boundary element is quadrilateral */
        }
        else {
            *nrpp = 3; /* boundary element is triangle */
        }
    }

    R3Copy (p[*nrpp], p[0]);

    if (m -> diel >= 0) return (TRUE);

    /*    check for uniform medium, and determine radius of pie.   */
    dielec = mediumNumber2 (center->z, center->z);
    radius = 0;

    Debug (fprintf (stderr, "number of peripheral points = %d\n", *nrpp));
    Debug (fprintf (stderr, "center: (%g, %g, %g)\n", center->x, center->y, center->z));
    for (n = 1; n <= *nrpp; n++) {
        Debug (fprintf (stderr, "p-point[%2d]: (%g, %g, %g)\n", n, p[n].x, p[n].y, p[n].z));
        if (p[n].z != center->z && dielec != mediumNumber2 (p[n].z, center->z)) {
            /*  The case of a conductor crossing a dielectric interface
             *  at present cannot be handled using the multipole expansion
             */
            return (FALSE);
        }
        aux = r3Dist (center, &p[n]);
        if (aux > radius) radius = aux;
    }

    makeTotalMoments (m, center, p, *nrpp);
    m -> diel = dielec;
    m -> radi = radius;

#ifdef TEST_MULTIPOLE_METHOD
    if (testMultiPoles) {
        /* Determine the ratio of maximum to minimum radius. "maximum radius" is what
         * was called "radius" above, "minimum radius" is the minimum distance between the
         * center and periphery.  Their ratio is interesting if one wants to study
         * how much influence a deviation of the pie from radial symmetry has on the
         * precision of the multipole method.
         */
        pointR3_t a, b, nu, aoutnu, boutnu;
        real_t ab, aa, bb, nua, nub, distance;
        for (n = 1; n <= *nrpp; n++) {
            /* calculate outer normal nu on line joining p[n-1] and p[n] */
            R3Copy (p[n-1], a);
            R3Copy (p[n], b);
            R3Subtract (a, *center, a);
            R3Subtract (b, *center, b);
	    aa = R3Dot (a, a);
            ab = R3Dot (a, b);
            bb = R3Dot (b, b);
            if (Fabs ((ab - bb) / bb) < 1.e-6) R3Copy (b, nu);
            else {
                nua = 1.;
                nub = nua * (aa - ab) / (bb - ab);
                nu.x = nua * a.x + nub * b.x;
                nu.y = nua * a.y + nub * b.y;
                nu.z = nua * a.z + nub * b.z;
            }
            R3Normalize (nu);
            /* check whether the normal nu lies in between a and b */
            R3Cross (a, nu, aoutnu);
            R3Cross (b, nu, boutnu);
            if (R3Dot (aoutnu, boutnu) < 0) {
                /* normal does lies in between a and b */
                distance = R3Dot (a, nu);
            }
            else {
		/* Minimum distance between pie center and line joining a and b is either a or b.
		 * We calculate this for test purposes, to see how much influence a deviation of
		 * the pie from radial symmetry influences the precision of the multipole expansion.
		 */
		distance = (aa < bb) ? Sqrt (aa) : Sqrt (bb);
            }
	    if (n == 1) minRadius = distance;
	    else if (distance < minRadius) minRadius = distance;
        }
    }
#endif
    return (TRUE);
}

/**********************************************************************************
*
* Calculate the influence matrix element indexed by the spiders  spo, spc
* with the aid of a multipole-expansion.  If this succeeds, return TRUE;
* the result of the calculation is stored in *val.  If it fails (e.g. because
* one of the conductors has thickness 0) return FALSE; then the calculation
* of the influence matrix element will be done the traditional way.
*
***********************************************************************************/
bool_t greenMpole (spider_t *spo, spider_t *spc, green_t *val)
{
    pointR3_t oPeriPoints[max_nr_peripheral_points + 1], oCenter,
              cPeriPoints[max_nr_peripheral_points + 1], cCenter, R;
    real_t oPieArea, cPieArea, convergenceRadius, gterm, Rabs, distratio;
    real_t cZMin, cZMax, oZMin, oZMax, rMin, rMax, dx, dy, r;
    real_t distance, strength, xxyy, cMomMir_dpl[40], *oMom_dpl;
    int cNrpp, oNrpp, oMedium, cMedium, i, j, g, ic, zq_sign;
    carmom_t *oMom, *cMom;
    imageGroup_t grp;

    if (!getPiePoints (spc, &cCenter, cPeriPoints, &cNrpp)) {
        Debug (fprintf (stderr, "c-pie not suitable for multipole method\n"));
        return (FALSE);
    }
    cMom = spc -> moments;
    cPieArea = cMom -> pieArea;
    cMedium  = cMom -> diel;
    convergenceRadius = cMom -> radi;

    if (FeModeGalerkin) {
        if (!getPiePoints (spo, &oCenter, oPeriPoints, &oNrpp)) {
            Debug (fprintf (stderr, "o-pie not suitable for multipole method\n"));
            return (FALSE);
        }
	oMom = spo -> moments;
	oMom_dpl = oMom -> dpl;
	oPieArea = oMom -> pieArea;
	oMedium  = oMom -> diel;
	convergenceRadius += oMom -> radi;
    }
    else {
	oMom_dpl = NULL;
	oPieArea = 0;
	R3Set (oCenter, spo);
	oMedium = mediumNumber2 (oCenter.z, oCenter.z);
    }

    carmomMirror (cMom->dpl, cMomMir_dpl); /* create mirrored source moments */

    /*  R = vector pointing from center of image of charge-pie
     *      to center of observation pie
     */
    R.x = oCenter.x - cCenter.x;
    R.y = oCenter.y - cCenter.y;
    xxyy = R.x * R.x + R.y * R.y;

    green_cnt++;

    if (printGreenTerms)
	fprintf (stderr, "greenMpole: spo=(%g,%g,%g) spc=(%g,%g,%g) cnt=%ld convRadius=%g gs%d%d%d%d\n",
		spo -> act_x, spo -> act_y, spo -> act_z,
		spc -> act_x, spc -> act_y, spc -> act_z, green_cnt, convergenceRadius,
		greenType, nLayers[greenCase], cMedium+1, oMedium+1);

#ifdef DEBUG
    if (DEBUG) {
	std::cerr << "q(" << cCenter.x << "," << cCenter.y << "," << cCenter.z << "); ";
	std::cerr << "p(" << oCenter.x << "," << oCenter.y << "," << oCenter.z << ")" << std::endl;
    }
#endif /* DEBUG */

    {
	// Calculate using the "simple" green's function approach.
	//
	green_t term = 0, termold = 0, tabs = 0, v = 0;

	for (j = g = 0; g < maxGreenTerms; g++) {
	    grp = getImageGroup (oMedium, cMedium, g);
	    if (grp.size == 0) { greenNoConvergence (cMedium, oMedium, g); break; }

	    termold = term;
	    term = 0;
	    for (i = 0; i < grp.size; i++) {
		zq_sign  = grp.images[i].zq_sign;
		distance = grp.images[i].distance;
		strength = grp.images[i].strength;

		if (zq_sign < 0)
		    R.z = oCenter.z - cCenter.z + distance;
		else
		    R.z = oCenter.z + cCenter.z + distance;
		Rabs = Sqrt (xxyy + R.z * R.z);
		distratio = Rabs / convergenceRadius;

		if (distratio > multipolesMindist) { /* use multipole expansion */
#ifdef TEST_MULTIPOLE_METHOD
		    if (distratio < smallestDistratio || smallestDistratio < 0) smallestDistratio = distratio;
#endif
		    if (zq_sign < 0) /* NO reflection */
			gterm = strength * mpConnect (cMom->dpl, oMom_dpl, &R, Rabs);
		    else
			gterm = strength * mpConnect (cMomMir_dpl, oMom_dpl, &R, Rabs);
		    numMultipoleImages++;
		}
		else { /* use `traditional' integration */
		    if (zq_sign < 0) {
			cCenter.z -= distance;
			for (ic = 0; ic <= cNrpp; ic++) cPeriPoints[ic].z -= distance;
		    } else {
			cCenter.z = -cCenter.z - distance;
			for (ic = 0; ic <= cNrpp; ic++) cPeriPoints[ic].z = -cPeriPoints[ic].z - distance;
		    }
		    gterm = strength * intPieGreenPie (
			    cCenter, cPeriPoints, cNrpp, cPieArea,
			    oCenter, oPeriPoints, oNrpp, oPieArea, collocationEps);
		    if (zq_sign < 0) {
			cCenter.z += distance;
			for (ic = 0; ic <= cNrpp; ic++) cPeriPoints[ic].z += distance;
		    } else {
			cCenter.z = -cCenter.z - distance;
			for (ic = 0; ic <= cNrpp; ic++) cPeriPoints[ic].z = -cPeriPoints[ic].z - distance;
		    }
		    numMultipoleImagesTraditional++;
		}

		if (printGreenGTerms)
		    fprintf(stderr,"g=%d i=%2d str=%10.4f dis=%7.0f zq=%2d R.z=%7.0f Rabs=%6.0f distR=%4.0f %s=%12.5e\n",
			g, i, strength, distance, zq_sign, R.z, Rabs, distratio,
			distratio > multipolesMindist ? "gtm" : "gti", gterm);

		term += gterm;
	    }

	    v += term;
	    if ((tabs = term / v) < 0) tabs = -tabs;
	    j += i;

	    if (printGreenTerms)
		fprintf(stderr,"greenMpole(%2d,%3d): v=%12.9f term=%12.9f eps=%f\n", g, j, v, term, tabs);

	    if (tabs < collocationEps) break;
	}

	if (g == maxGreenTerms && nLayers[greenCase] > 1) {
	    if (printGreenTerms)
		fprintf(stderr, "greenMpole: Greens function truncated after %d terms (eps=%g)\n", g, tabs);
	    greenNoConvergence (cMedium, oMedium, g);
	}

	if (g > 1 && useMeanGreenValues) {
	    if ((termold > 0 && term > 0) || (termold < 0 && term < 0))
		*val = v + term / 2;
	    else
		*val = v - term / 2;
	}
	else *val = v;
    }

    return TRUE;
}

#ifdef TEST_MULTIPOLE_METHOD
/*
* Test multipole expansion by comparing with traditionally calculated result.
*/
void TestMultipoles (spider_t *spo, spider_t *spc, green_t val, int cnt)
{
    static int firstCall = 1;
    static int extend = 0;
    static FILE *freldev = NULL;
    FILE *pfile;
    green_t mpresult;

    if (firstCall) {
        firstCall = 0;
        printf ("Do you want terse or extended comparison between traditional- & multipole-method?\n");
        printf ("0 = only relative deviation and distance ratio\n");
        printf ("1 = also results of  trad. and mp. method\n");
        printf ("2 = also pie-centers\nYour choice:");
        scanf ("%d", &extend);

        freldev = fopen ("reldev", "w");
	if ((pfile = fopen ("u.p", "r"))) {
	    int c;
	    fprintf (freldev, "# Parameter file u.p:\n");
	    fprintf (freldev, "# ===================\n");
	    while ((c = getc (pfile)) != EOF) putc (c, freldev);
	    fclose (pfile);
	}
        fprintf (freldev, "# Relative deviation of multipole method from existing one.\n");
        fprintf (freldev, "# Format of data:  (1) counter  (2) relative deviation\n");
        fprintf (freldev, "#                  (3) smallest distance/(sum radii) from image group\n");
        if (extend > 0)
            fprintf (freldev, "#                  (4) trad. result, (5) mp result\n");
        if (extend > 1)
            fprintf (freldev, "#                  (6,7,8) c-center, (9,10,11) o-center\n");
    }

    smallestDistratio = -1;	/* if this stays -1, no mp calculation was used. */
    if (greenMpole (spo, spc, &mpresult)) {
/*
	real_t Rabs = Sqrt ((spo->act_x - spc->act_x) * (spo->act_x - spc->act_x)
			+ (spo->act_y - spc->act_y) * (spo->act_y - spc->act_y)
			+ (spo->act_z - spc->act_z) * (spo->act_z - spc->act_z));
*/
	fprintf (freldev, "%5d %10.5e %10.5e",
		cnt, Fabs (mpresult / val - 1), smallestDistratio);
	if (extend > 0) fprintf (freldev, " %10.5e %10.5e", val, mpresult);
	if (extend > 1) fprintf (freldev, " %10.5e %10.5e %10.5e   %10.5e %10.5e %10.5e",
		spc->act_x, spc->act_y, spc->act_z, spo->act_x, spo->act_y, spo->act_z);
	fprintf (freldev, "\n");
	fflush (freldev);
    }
}
#endif
