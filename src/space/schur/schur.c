/*
 * ISC License
 *
 * Copyright (C) 1989-2018 by
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

#include <math.h>

#include <stdio.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/schur.h"
#include "src/space/schur/define.h"
#include "src/space/schur/extern.h"

/*
 * The Schur module (schur.a) provides routines to compute an
 * (approximate) inverse for a (partly specified) positive definite
 * matrix.
 * The interface between the Schur module and the application program
 * consists of the following routines:
 *
 * initSchur ()   : defined in schur.a, called in the application.
 *                  Initializes the memory of the Schur module such
 *                  that all matrices with a certain maximum bandwidth
 *                  can be handled.
 *
 * schurRowIn ()  : defined in schur.a, called in the application.
 *                  Provides a row for the input matrix.
 *
 * schurRowOut () : defined in the application, called in schur.a
 *                  Provides a row of the output matrix.
 *
 */

bool_t schurShowProgress = FALSE;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void execSchurRow (void);
#ifdef __cplusplus
  }
#endif

#ifdef TEST_SCHUR
int scDebug = 0;
bool_t luFact = FALSE;
bool_t coefficient = FALSE;
bool_t printCoef = FALSE;
#endif
static int globalMaxo = -1;
static int calls = 0;

static int maxdimOcc = 0;
static int maxordOcc = 0;
static int maxrowOcc = 0;
static int localmaxordOcc;
extern int maxSchurSize;
#ifdef TEST_SCHUR
static schur_t pmax = 0;
static schur_t ptot = 0;
static int pcnt = 0;
#endif

int maxrow;
int maxorder;

static int k;
static int k2;
static int schur_row = -1;
static int max_used = 0;

/*
 * Initialize Schur module for (full or approximate) matrix inversion.
 *
 * The argument maxo specifies the maximum order (or bandwidth) of the
 * matrix that is to be inverted.
 * Maxo = 0 if only the main diagonal is inverted and
 * maxo = nr_of_rows - 1 if a full matrix is inverted.
 */
void initSchur (int maxr, int maxo)
{
    int max;
#ifdef TEST_SCHUR
    if (scDebug) fprintf (stderr, "INIT SCHUR %d %d\n", maxr, maxo);
    if (luFact) fprintf (stderr, "USING LU DECOMPOSITION\n");
#endif
    if (schur_row >= 0) say ("Ho, incorrect initSchur (schur busy)"), die ();
    if (maxr < 0 || maxo < 0 || maxo > maxr) say ("Ho, incorrect initSchur (dim/order)"), die ();

    max = 2 * maxo + 1;
    if (max > maxr + 1) max = maxr + 1;

    if (maxo > globalMaxo || max > max_used) {
	/* init or increase memory */
	max_used = newSchurMem (maxr, maxo);
	ASSERT (max_used > maxo);
	globalMaxo = maxo;
    }

    /* do one-inversion initializations */

    maxrow = maxr;
    maxorder = maxo;
    if (maxrow   > maxdimOcc) maxdimOcc = maxrow;
    if (maxorder > maxordOcc) maxordOcc = maxorder;

    schur_row = 0;
    localmaxordOcc = 0;
    k = k2 = 0;
 // scM[maxorder][1] = 1;

    calls++;
}

/*
 * Input one row of the matrix to be inverted.
 *
 * arguments: kr = row number, starting with 0
 *             r = row
 *           ord = order (bandwidth) of this row.
 *                 order = 0 means only the main diagonal, etc.
 */
void schurRowIn (int kr, schur_t *r, int ord)
{
    schur_t *inSave, val;
    int j, n;

#ifdef TEST_SCHUR
    if (scDebug) {
	fprintf (stderr, "SCHUR %d ", kr);
	printUpperMatrix (stderr, kr, r, ord);
    }
#endif
    if (kr < 0 || kr != schur_row++) say ("Ho, incorrect row %d", kr), die ();

    if (ord < 0 || ord > maxorder) say ("Ho, incorrect order %d", ord), die ();

    n = kr + ord;
    if (n > localmaxordOcc) {
	localmaxordOcc = n;
	if (n > maxrow) say ("Ho, too much values on row %d", kr), die ();
    }
    else if (n < localmaxordOcc) {
	say ("Ho, too less values on row %d", kr); die ();
    }

    n = kr - k2;
    if (n > maxrowOcc) {
	maxrowOcc = n; /* statistics */
	if (n >= max_used) say ("Ho, problems with schur memory"), die ();
    }

    for (j = 0; j <= ord; ++j) scIN[n][j] = r[j];
    scOrder[n] = ord;

#ifdef TEST_SCHUR
    if (luFact) {
	if (kr == maxorder) { /* complete matrix is known */
	    LU (maxorder + 1); /* perform LU decomposition */
	}
	return;
    }
#endif

    scDIAG[n] = 1 / sqrt (r[0]);

    if (schurShowProgress)
        fprintf (stderr, "progress: Schur row %d                        \r", kr);

    /* compute columns of L
    */
    while (kr - k >= scOrder[k - k2]) {

        if (schurShowProgress)
	    fprintf (stderr, "progress: Schur row %d; computing row %d\r", kr, k);

        /* execute schur for row k */
        execSchurRow ();
        if (++k > kr) break;
    }

    if (kr == maxrow) { /* last row */
	if (schurShowProgress) fprintf (stderr, "\n");
	if (kr >= k) say ("Ho, not all rows computed"), die ();
	schur_row = -1;
    }

    /* compute rows of the approximate inverse
    */
    while (k - k2 > scOrder[0]) {

#ifdef TEST_SCHUR
	if (scDebug > 1) printResult (k, k2);
#endif

	/* compute the entries of row k2 and call schurRowOut
	 */
	inSave = scIN[0];
	for (j = 0; j <= scOrder[0]; ++j) {
	    val = 0;
	    for (n = j; n <= scOrder[0]; ++n) {
		val += scIN[n][maxorder - n] * scIN[n][maxorder - n + j];
	    }
	    inSave[j] = val;
	}
	schurRowOut (k2, inSave, scOrder[0]);
	if (++k2 == k) break;

	/* push memory */
	n = 0;
	while (n <= kr - k2) {
	     scIN[n] = scIN[n+1];
	     scDIAG[n] = scDIAG[n+1];
	     scOrder[n] = scOrder[n+1];
	     n++;
	}
	scIN[n] = inSave;
        /* end push memory */
    }
}

/*
 * Execute the Schur algorithm for row k of the input matrix
 */
Private void execSchurRow ()
{
    int     i, j;
    schur_t e, l, d;
    int max_i;
    int kk;

    kk = k - k2;

#define scOUT scIN
#define scV   scM

    d = scDIAG[kk];
    /* normalize */
    for (j = 1; j <= scOrder[kk]; j++) scIN[kk][j] = scIN[kk][j] * d * scDIAG[kk + j];

    if (kk > 0) { // scOrder[kk - 1] > 0
        //j = 0;
	scP[1] = l = scV[0][0];
	e = sqrt (1 - l * l);
	scP1[1] = 1 / e;
	for (i = 2; i <= kk && i <= scOrder[kk - i]; i++) {
	    l = scV[0][i - 1];
	    scP[i] = l / e;
	    scP1[i] = 1 / sqrt (1 - scP[i] * scP[i]);
	    e = sqrt (e * e - l * l);
	}
	max_i = i - 1;

	for (j = 1; j < scOrder[kk - 1]; j++) {
	    scV[j - 1][0] = l = scIN[kk][j];
	    i = 1;
	    do {
		e = scV[j][i - 1];
		scV[j - 1][i] = (e - scP[i] * l) * scP1[i];
		l = (l - scP[i] * e) * scP1[i];
	    }
	    while (++i <= kk && i <= scOrder[kk - i] - j);
	}
	for (; j <= scOrder[kk]; j++) scV[j - 1][0] = scIN[kk][j];

	for (j = maxorder - max_i; j < maxorder - 1; j++) {
	    i = maxorder - j;
	    e = scM[j + 1][i - 1];
	    scM[j][i] = e * scP1[i];
	    l = -scP[i] * e * scP1[i];
	    while (++i <= max_i) {
		e = scM[j + 1][i - 1];
		scM[j][i] = (e - scP[i] * l) * scP1[i];
		l = (l - scP[i] * e) * scP1[i];
	    }
	    scOUT[kk][j] = l * scDIAG[kk + j - maxorder];
	    if (isnan (scOUT[kk][j])) say ("Encountered NAN in schur module."), die ();
	}

	// j = maxorder - 1
	scM[j][1] = scP1[1];
	l = -scP[1] * scP1[1];
	for (i = 2; i <= max_i; ++i) {
	    e = scM[j + 1][i - 1];
	    scM[j][i] = (e - scP[i] * l) * scP1[i];
	    l = (l - scP[i] * e) * scP1[i];
	}
	scOUT[kk][j] = l * scDIAG[kk - 1];
	if (isnan (scOUT[kk][j])) say ("Encountered NAN in schur module."), die ();
	j++;

	// j = maxorder
	l = scP1[1];
	scM[j][1] = -scP[1] * l;
	for (i = 2; i <= max_i; ++i) {
	    l = l * scP1[i];
	    scM[j][i] = -scP[i] * l;
	}
	d = d * l;
    }
    else {
	for (j = 1; j <= scOrder[kk]; j++) scV[j - 1][0] = scIN[kk][j];
    }
    scOUT[kk][maxorder] = d;
    if (isnan (d)) say ("Encountered NAN in schur module."), die ();
}

/*
 * Send statistics of Schur algorithm to file fp.
 */
void schurStatistics (FILE *fp)
{
	fprintf (fp, "overall schur statistics:\n");
#ifdef TEST_SCHUR
    if (luFact)
	fprintf (fp, "\tLU calls           : %d\n", calls);
    else
#endif
	fprintf (fp, "\tschur calls        : %d\n", calls);
	fprintf (fp, "\tmax. dimension     : %d\n", maxdimOcc + 1);
	fprintf (fp, "\tmax. maxorder      : %d\n", maxordOcc);
	fprintf (fp, "\tmax. int. rows     : %d\n", maxrowOcc + 1);
	fprintf (fp, "\tmax. matrix memory : %d bytes\n", maxSchurSize);
#ifdef TEST_SCHUR
    if (coefficient && !luFact) {
	fprintf (fp, "\tavg. refl. coef.   : %f\n", pcnt > 0 ? ptot / pcnt : 0);
	fprintf (fp, "\tmax. refl. coef.   : %f\n", pmax);
    }
#endif
}
