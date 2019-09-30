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

#include <math.h>
#include "src/space/auxil/auxil.h"

int gauss_eval = 0;
#define COUNT() gauss_eval++

/* abscissas normalized over [-1, 1] */
double ABS [200] = {
    -0.7745966692,
     0.0,
     0.7745966692
};

/* normalized weights */
double WGT [200] = {
    0.5555555556,
    0.8888888889,
    0.5555555556
};

/* default number of quadrature points */
int N = 3;

/*
** gauss  - 1-D gaussian quadrature
**
** func   - integrand;
** low    - lower limit of the integral;
** upp    - upper limit;
** EPS    - relative accuracy;
*/
double gauss (double (*func)(double), double low, double upp, double EPS)
{
    double  C, Q, G, R, H, X;
    int     M, i, j;

    M = 0;
    C = fabs (low) + fabs (upp);
    G = 0.0;

    do {
	Q = G;
	G = 0.0;
	H = (upp - low) / (double) (++M);

	if (C + H == C) {
	    if (H) fprintf (stderr, "gauss: no convergence\n");
	    break;
	}

	for (i = 0; i < M; i++) {
	    R = low + H * (double) (i);
	    for (j = 0; j < N; j++) {
		COUNT ();
		X  = R + H * (ABS[j] + 1) / 2.0;
		G += (*func) (X) * WGT[j];
	    }
	}

	G = 0.5 * G * H;

	fprintf (stderr, "  iteration: %e  difference: %e\n", G, G - Q);

    } while (fabs (G - Q) / (fabs (G) + 1.0) > EPS || M < 2);

    return (G);
}
