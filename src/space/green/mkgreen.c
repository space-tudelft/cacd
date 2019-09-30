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
 * Extended and modified by Theo Smedes to include substrate resistances (1995)
 *
 * Introduction of greenType, greenCase for switching between the two
 * possible cases with 4 types of Greens functions.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/green/green.h"
#include "src/space/green/extern.h"

/* T.S. definition of greenType, greenCase, DIEL and SUBS */
#include "src/space/spider/export.h"

#ifdef DRIVER
int greenType, greenCase = DIEL;
int maxGreenTerms;
int numImages;
int numImageGroups;
bool_t useLowestMedium;
bool_t useOldImages;
bool_t mergedImages = 1;
real_t greenMeters[2];
#else
extern int maxGreenTerms;
extern int numImages;
extern int numImageGroups;
extern bool_t useLowestMedium;
extern bool_t useOldImages;
extern bool_t mergedImages;
extern real_t greenMeters[2];
#endif

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private imageGroup_t makeGreen (int i, int j, int group_num);
Private imageGroup_t directTerms (int iLayer, int jLayer);
Private imageGroup_t makeGreenTerms (int iLayer, int jLayer, int n, int parity);
Private imageGroup_t makeGreenTerm (int iLayer, int jLayer, int coefficient, int *powers, int num_vars);
Private double kij (int i, int n);
#ifdef __cplusplus
  }
#endif

/* T.S. these arrays are now 2D (DIEL and SUBS) */
int nLayers[2];
static double matconst[2][GREEN_MAX_LAYERS];
static real_t matbounds[2][GREEN_MAX_LAYERS];

/*
 * 2 identical routines: one for cap3d and one for sub3d
 * Initialize the green's function module
 * n   - number of dielectrics, 0<=n<=3;
 * eps - relative dielectric constants for layers 0...n-1;
 * bot - z-coordinates of bottom of layer i, i = 0...n-1,
 *        bot[0]==0;
 */
void cap3dSettings (int n, double mat[], double bnd[])
{
    int i;

    nLayers[DIEL] = n;
    ASSERT (n >= 0 && n <= GREEN_MAX_LAYERS);
    ASSERT (bnd[0] == 0);

    if (n == 0) { /* vacuum */
	matbounds[DIEL][0] = 0;
	matconst[DIEL][0] = 1.0;
    }
    else
    for (i = 0; i < n; i++) {
	matbounds[DIEL][i] = bnd[i];
	matconst[DIEL][i] = mat[i];
    }
}

static int    subcontMedium;
static real_t subcontPosition;

void sub3dSettings (int n, double mat[], double bnd[])
{
    int i;

    nLayers[SUBS] = n;
    ASSERT (n > 0 && n <= GREEN_MAX_LAYERS);
    ASSERT (bnd[0] == 0);

    for (i = 0; i < n; i++) {
	matbounds[SUBS][i] = bnd[i];
	matconst[SUBS][i] = mat[i];
    }
    subcontMedium = 0;
    subcontPosition = 0;
    if (n > 1 && greenType == 3) { /* grounded SUBS */
	subcontPosition = matbounds[SUBS][n-1];
	ASSERT (subcontPosition > 0);
	subcontMedium = useLowestMedium ? n-2 : n-1;
    }
}

/*
 * Return the <group_num>'th image group
 * for the layers <iLayer> and <jLayer>
 * <group_num> should start a 0, in which case
 * it returns the free-space and first mirror image.
 */
imageGroup_t getImageGroup (int iLayer, int jLayer, int group_num)
{
    static imageGroup_t *imageHead[2];
    static int Size[2];
    imageGroup_t group;
    int idx, nL = nLayers[greenCase];

 // if (greenCase == DIEL) ASSERT (nL <= GREEN_MAX_DIELECTRICS_SIMPLE);
 // if (greenCase == SUBS) ASSERT (nL <= GREEN_MAX_SUBSTRATE_LAYERS_SIMPLE+1);

    if (group_num >= GREEN_MAX_TERMS || (nL < 2 && group_num >= 1)) {
	group.size = 0;
	group.images = NULL;
	return (group);
    }

    if (!imageHead[greenCase]) {
	if (greenCase == SUBS)
	    Size[greenCase] = nL < 2 ? 1 : GREEN_MAX_TERMS;
	else
	    Size[greenCase] = nL < 2 ? 1 : (nL * nL * GREEN_MAX_TERMS);
	imageHead[greenCase] = NEW (imageGroup_t, Size[greenCase]);
	CLEAR (imageHead[greenCase], sizeof (imageGroup_t) * Size[greenCase]);
    }

    if (greenCase == SUBS)
	idx = group_num;
    else
	idx = (iLayer * nL + jLayer) * GREEN_MAX_TERMS + group_num;
    if (idx >= Size[greenCase]) { ASSERT (0); die (); }

    group = imageHead[greenCase][idx];

    if (group.size == 0) {
	group = makeGreen (iLayer, jLayer, group_num);
	imageHead[greenCase][idx] = group;
    }

    numImages += group.size;
    numImageGroups++;
    return (group);
}

#define B09 0.9999999999 * matbounds[greenCase][i]
#define B11 1.0000000001 * matbounds[greenCase][i]

/* Return the number of the dielectric medium
 * in which <z> is located.
 */
int mediumNumber (real_t z)
{
    int i;

    if (greenCase == SUBS) {
	if (z != subcontPosition) { ASSERT (0); die (); }
	return (subcontMedium);
    }

    ASSERT (z >= 0.0);

    for (i = 1; i < nLayers[greenCase]; i++)
	if (z < matbounds[greenCase][i])
	    break;
    if (--i > 1 && z <= B11 && useLowestMedium) --i;
    return (i);
}

int mediumNumber2 (real_t z, real_t z2)
{
    int i;

    if (greenCase == SUBS) {
	if (z != subcontPosition) { ASSERT (0); die (); }
	return (subcontMedium);
    }

    ASSERT (z >= 0.0);

    for (i = 1; i < nLayers[greenCase]; i++)
	if (z < matbounds[greenCase][i])
	    break;
    if (i > 1 && z == matbounds[greenCase][i-1]) {
	if (useLowestMedium) {
	    if (z >= z2) --i;
	}
	else if (z > z2) --i;
    }
    return (i-1);
}

/* Return the number of the dielectric medium
 * in which <z1..4> are located.
 */
int mediumNumber4 (real_t z1, real_t z2, real_t z3, real_t z4)
{
    static int mess_done = 0;
    int i;
    real_t z_min, z_max;

    z_min = Min (z1, z2); z_min = Min (z_min, z3); z_min = Min (z_min, z4);
    z_max = Max (z1, z2); z_max = Max (z_max, z3); z_max = Max (z_max, z4);

    if (greenCase == SUBS) {
	if (z_min != z_max || z_max != subcontPosition) { ASSERT (0); die (); }
	return (subcontMedium);
    }

    ASSERT (z_min >= 0.0);

    for (i = 1; i < nLayers[greenCase]; i++) {
	if (z_min < B09) {
	    if (z_max > B11 && !mess_done) {
		real_t f = greenMeters[greenCase] * 1e6;
		say ("illegal boundary element (zmin=%g zmax=%g) crosses dielectric interface at z=%g",
			z_min * f, z_max * f, matbounds[greenCase][i] * f);
		mess_done = 1;
	    }
	    break;
	}
    }
    if (--i > 1 && z_min <= B11 && useLowestMedium) --i;
    return (i);
}

double crossing_z;
double crossing_z09;
double crossing_z11;
int crossingIndex = -1;

int faceCrossesDielBoundary (face_t *face, int next)
{
    static int crossing_i;
    double z[4];
    register int i, j, k;
    int max = 0, min = 0;

    k = face -> corners[3] ? 3 : 2;
    z[0] = face -> corners[0] -> act_z;
    for (i = 1; i <= k; ++i) {
	z[i] = face -> corners[i] -> act_z;
	if (z[i] > z[max]) max = i;
	else
	if (z[i] < z[min]) min = i;
    }

    if ((i = max - 1) < 0) if (z[i = k] == z[max]) --i;
    if ((j = min - 1) < 0) if (z[j = k] == z[min]) --j;

	 if (z[i] == z[min]) j = i;
    else if (z[j] == z[max]) i = j;
    else if (z[i] < z[j]) k = i;
    else { k = j; i = max; j = min; }

    if (!next) crossing_i = 0;
    while (++crossing_i < nLayers[DIEL]) {
	crossing_z = matbounds[DIEL][crossing_i];
	crossing_z11 = 1.0000000001 * crossing_z;
	if (z[max] <= crossing_z11) break;
	crossing_z09 = 0.9999999999 * crossing_z;
	if (z[min] < crossing_z09) {
	    crossingIndex = (i != j && z[k] < crossing_z09)? i : j;
	    return (1);
	}
    }
    return (0);
}

#define Even(n) ((n)%2 == 0)

Private imageGroup_t makeGreen (int i, int j, int group_num)
{
    imageGroup_t group;

    ASSERT (nLayers[greenCase] >= 0 &&
              (group_num == 0 || nLayers[greenCase] >= 2));

    if (group_num == 0) {
	group = directTerms (i, j);
    }
    else {
	int parity = Even (group_num) ? -1 : 1;
	group = (greenType != 2
                 ? makeGreenTerms (i, j, group_num - 1, parity)
		 : makeGreenTerms (i, j, group_num, 1));
    }

    if (group.size == 0) {
	say ("no Green's function for greenSeries(%d,%d,%d,%d)",
	    greenType, nLayers[greenCase], i+1, j+1);
	die ();
    }

    return (group);
}

Private imageGroup_t directTerms (int iLayer, int jLayer)
{
    imageGroup_t group;
    double e;

    group.size = (nLayers[greenCase] < 1 || (greenType == 2 && !useOldImages)) ? 1 : 2;

    e = matconst[greenCase][jLayer];

    if (nLayers[greenCase] == 2 && greenType != 2 &&
	iLayer == 1 && jLayer == 1 && !useOldImages) ++group.size;
    group.images = NEW (image_t, group.size);

    group.images[0].strength = (greenType == 2 && !useOldImages ? 2 : 1) / e;
    group.images[0].distance = 0;
    group.images[0].zq_sign  = -1;
    group.images[0].zp_sign  =  1;

    if (group.size >= 2) {
	group.images[1].strength = (greenType == 2 ? 1 : -1) / e;
	group.images[1].distance = 0;
	group.images[1].zq_sign  = 1;
	group.images[1].zp_sign  = 1;
    }

    if (nLayers[greenCase] == 2 && greenType != 2 &&
	(iLayer > 0 || jLayer > 0) && !useOldImages) {
	if (iLayer != jLayer) {
	    e = (matconst[greenCase][0] + matconst[greenCase][1]) / 2;
	    group.images[0].strength =  1 / e;
	    group.images[1].strength = -1 / e;
	}
	else {
	    group.images[2].strength = -kij(0,1) / e;
	    group.images[2].distance = -2 * matbounds[greenCase][1];
	    group.images[2].zq_sign  = 1;
	    group.images[2].zp_sign  = 1;
	    e = (matconst[greenCase][0] + matconst[greenCase][1]) / 2;
	    group.images[1].strength = -1 / e;
	}
    }

    return (group);
}

/*
 * Multinomial expansion
 *
 * Effectively generates the multinomial,
 * coefficients and powers of (p1 + p2 + ... pk)^n.
 *
 *
 * Algorithm: simulate k-1 nested counters in c[0] ... c[k-2].
 */
Private imageGroup_t makeGreenTerms (int iLayer, int jLayer, int n, int parity)
{
    /* This array contains the number of terms
     * in the multinomial expansion, indexed by nLayers.
     * Defined by:
     *    Table [Sum [Binomial [n-1, k], {k, 1, n-1}], {n, 0, 7}]
     */
    static int  kk[] = {
	0, 0, 1, 3, 7, 15, 31, 63
    };

#define MAXNUMTERMS 63 /* max of array kk above */

    int     c[MAXNUMTERMS + 2];
    int     powers[MAXNUMTERMS + 2];
    int     coef, i, j, k, size, gti, gtj;
    static imageGroup_t group;
    imageGroup_t gt;

    ASSERT ((sizeof (kk) / sizeof (*kk) -1) >= nLayers[greenCase]);
    ASSERT (nLayers[greenCase] <= GREEN_MAX_DIELECTRICS_SIMPLE);
    ASSERT (nLayers[greenCase] >= 2);

    k = kk[nLayers[greenCase]];
    if (k < 1) { ASSERT (0); die (); }

 /* sentinel */
    c[k - 1] = n;

 /* reset all counters to zero */
    for (i = 0; i < k - 1; i++) c[i] = 0;

    size = 0;

    /* Loop until highest counter would exceed n.
     * nLayers == 2 (==> k == 1) is a special case,
     * a break occurs inside the loop.
     */
    while (k == 1 || c[k - 2] <= n) {

    /* this gives the powers of each term */
	powers[0] = c[0];
	for (i = 1; i <= k - 1; i++)
	    powers[i] = c[i] - c[i - 1];

    /* and this the coefficients of each term */
	coef = multinomial (n, powers, k);

	/* check special case */
	ASSERT (k != 1 || (coef == 1 && powers[0] == n));

        gt = makeGreenTerm (iLayer, jLayer, parity * coef, powers, k);

	if (gt.size == 0) {
	    group.size = 0;
	    break;
	}

	if (size == 0) {
	    group.size = gt.size * binomial (n+k-1, n);
	    group.images = NEW (image_t, group.size);
	}
	if (k > 1 && mergedImages) { /* sort by distance, try to merge (SdeG) */
	    real_t z;
	    for (gti = 0; gti < gt.size; gti++) {
		z = gt.images[gti].distance;
		for (gtj = 0; gtj < size; gtj++) {
		    if (z < group.images[gtj].distance) {
			int si = size++;
			do {
			    group.images[si] = group.images[si-1];
			} while (--si > gtj);
			group.images[si] = gt.images[gti];
			goto gti_done;
		    }
		    if (z == group.images[gtj].distance) {
			if (gt.images[gti].zq_sign != group.images[gtj].zq_sign) continue;
			group.images[gtj].strength += gt.images[gti].strength;
			--group.size;
			goto gti_done;
		    }
		}
		group.images [size++] = gt.images [gti];
gti_done:	;
	    }
	}
	else {
	    for (gti = 0; gti < gt.size; gti++) {
		group.images [size++] = gt.images [gti];
	    }
	}

    /* a special case */
	if (k == 1 || size == group.size) break;

    /* lookup the counter that must be stepped and step it */
	for (j = 0; j < k - 2; j++) {
	    if (c[j] < c[j + 1]) break;
	}
	c[j]++;

    /* If one counter makes a step, reset the lower ones. */
	for (i = 0; i < j; i++) c[i] = 0;
    }

 /* The number of terms */
    ASSERT (size == group.size);

    return (group);
}

Private imageGroup_t makeGreenTerm (int iLayer, int jLayer, int coefficient, int *powers, int num_vars)
{
    imageGroup_t group;
    static image_t images[50];
    double S, D;
    int m, n, u, i = 0;

#ifdef DEBUG
    if (DEBUG) {
	fprintf (stderr, "makeGreenTerm %d %d %d, %d vars, powers",
	    iLayer, jLayer, coefficient, num_vars);
	for (n = 0; n < num_vars; n++)
	    fprintf (stderr, " %d", powers[n]);
	fprintf (stderr, "\n");
    }
#endif

#define image(s,x,p) \
    (images[i].strength = coefficient * s, \
     images[i].distance = x, \
     images[i].zq_sign  = p, \
     images[i].zp_sign  = 1, i++)

#define k(i,p) kij(i-1, p)
#define p(i)   powers[i]
#define e(i)   matconst[greenCase][i-1]
#define s(i)   matconst[SUBS][i-1]
#define t(i)   matbounds[greenCase][i]
#define b(i)   matbounds[SUBS][i]
#define greenSeries(g, n, i, j) \
    if (/*greenType == g &&*/ nLayers[greenCase] == n \
        && iLayer == i-1 && jLayer == j-1)

#include "src/space/green/images.h"

#undef k
#undef p
#undef e
#undef s
#undef t
#undef b
#undef greenSeries
#undef image

ret:
   group.size = i;
   group.images = images;

   return (group);
}

Private double kij (int i, int n)
{
    /* static arrays auto init to 0, 0, ... */
    static int    ntop[2][GREEN_MAX_DIELECTRICS_SIMPLE-1];
    static double    a[2][GREEN_MAX_DIELECTRICS_SIMPLE-1][100];

    ASSERT (n >= 0);
    ASSERT (i < GREEN_MAX_DIELECTRICS_SIMPLE - 1);

    if (ntop[greenCase][i] == 0) {
	ntop[greenCase][i] = 1;
	a[greenCase][i][0] = 1.0;
	a[greenCase][i][1] =
		(matconst[greenCase][i] - matconst[greenCase][i+1]) /
		(matconst[greenCase][i] + matconst[greenCase][i+1]);
    }

    if (a[greenCase][i][1] == 0) return (0.0);

    while (ntop[greenCase][i] < n
           && ntop[greenCase][i] < (sizeof (a[greenCase][i])
                                    / sizeof (*(a[greenCase][i])) - 1)) {
	int k = ntop[greenCase][i]++;
	a[greenCase][i][ntop[greenCase][i]] = a[greenCase][i][k]
                                             * a[greenCase][i][1];
    }

    return (n <= ntop[greenCase][i] ? a[greenCase][i][n]
	: a[greenCase][i][ntop[greenCase][i]]
          * pow ((double) a[greenCase][i][1],
                 (double) (n - ntop[greenCase][i])));
}

#ifdef DRIVER
Private void printGreen (imageGroup_t group)
{
    int i;
    for (i = 0; i < group.size; i++)
	fprintf (stderr, "%-+12g %+4g %+3d\n",
	    group.images[i].strength,
	    group.images[i].distance,
	    group.images[i].zq_sign);
}

/*
 * Test driver.
 */
int main (int argc, char *argv[])
{
    int g;
    if (argc < 3) {
	fprintf (stderr, "usage: mkgreen nLayers maxGreenTerms\n"); die ();
    }
    if ((nLayers[DIEL] = atoi (argv[1])) > GREEN_MAX_DIELECTRICS_SIMPLE) {
	fprintf (stderr, "mkgreen: nLayers > %d\n", GREEN_MAX_DIELECTRICS_SIMPLE); die ();
    }
    if ((maxGreenTerms = atoi (argv[2])) > GREEN_MAX_TERMS) {
	fprintf (stderr, "mkgreen: maxGreenTerms > %d\n", GREEN_MAX_TERMS); die ();
    }
    matconst[DIEL][0] = 3.9; matconst[DIEL][1] = 4.5; matconst[DIEL][2] = 1.0;
    matbounds[DIEL][0] = 0.0; matbounds[DIEL][1] = 1.0; matbounds[DIEL][2] = 2.0;

    for (g = 0; g < maxGreenTerms; g++) {
	fprintf (stderr, "group %d\n", g);
	printGreen (getImageGroup (0, 0, g));
    }

    /*
    compare with Table [((4.5-1.0)/(4.5+1.0))^n, {n,0,10}]
    int n;

    for (n = 0; n <= 10; n++)
	fprintf (stderr, "%g\n", kij (1, n));
    fprintf (stderr, "\n");
    */
    return (0);
}
#endif /* DRIVER */
