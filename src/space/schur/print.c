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

#include <stdio.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/schur.h"
#include "src/space/schur/define.h"
#include "src/space/schur/extern.h"

/*
 * Print the specified entries of the upper part of the matrix.
 */
void printUpperMatrix (FILE *fp, int k, schur_t *r, int ord)
{
    for (k = 0; k <= ord; ++k) fprintf (fp, "%e ", r[k]);
    fprintf (fp, "\n");
    fflush (fp);
}

#ifdef TEST_SCHUR
/*
 * Print all entries of the matrix (for unspecified entries a zero is printed).
 */
void printFullMatrix (FILE *fp, int k, schur_t *r, int ord, int dim, schur_t **tmat)
{
    int x, y;

    for (y = k; y < dim; y++) {
	tmat[y][k] = tmat[k][y] = (y - k <= ord)? r[y - k] : 0;
    }

    if (k == dim - 1) {
	fprintf (fp, "%d\n", dim);
	for (x = 0; x < dim; x++) {         /* x = row */
	    for (y = 0; y < dim; y++) {     /* y = column */
		fprintf (fp, "%e ", tmat[x][y]);
	    }
	    fprintf (fp, "\n");
	}
    }
}

/*
 * Print one row of a matrix.
 */
void printResult (int k, int k2)
{
    static int inOrd = 0;
    int m, n, j;
    static FILE *fp = NULL;

    if (!fp && !(fp = fopen ("result", "w"))) { say ("cannot open result"); die (); }

    if (scOrder[0] < inOrd) return;
    inOrd = scOrder[0];

    fprintf (fp, "k=%d k2=%d\n", k, k2);
    for (n = 0; n <= inOrd; ++n) {
	for (m = 0, j = maxorder - n; j <= maxorder; ++j, ++m) {
	 //  fprintf (fp, "%e ", scOUT[n][j] / scDIAG[n + j - maxorder]);
	     fprintf (fp, "%e ", scIN[n][j] / scDIAG[m]);
	}
	fprintf (fp, "\n");
    }
    fprintf (fp, "\n");
    fflush (fp);
}

/*
 * Print reflections coefficients.
 */
void printCoefs (int k, int imax)
{
    int i;
    static int fileNr = 1;
    static FILE *fp = NULL;
    char fn[20];

    if (k == 1) {
	if (fp) fclose (fp);

	sprintf (fn, "coef%d", fileNr++);
	if (!(fp = fopen (fn, "w"))) { say ("cannot open", fn); die (); }
    }

    fprintf (fp, "%3d", k);
    for (i = 1; i <= imax; i++) fprintf (fp, " %e", scP[i]);
    fprintf (fp, "\n");
}
#endif
