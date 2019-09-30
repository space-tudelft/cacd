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

double levin (double term, int termnr)
{
    static double C[50][50]; /* transform coefficients */
    static double part[100]; /* partial sum */
    double D1, Dk, T;
    int i, k;

    if (termnr == 1) {
	Debug (fprintf (stderr, "n %2d straight % e levin % e\n",
	    termnr, term, term));
	return (part[1] = term);
    }
    else
	part[termnr] = part [termnr - 1] + term;

    /* optimization */
    if (termnr == 2) {
	double s1 = part[1];
	return (s1*(term - s1)/(term + term - s1));
    }
    if (termnr == 3) {
	double s1 = part[1];
	double s2 = part[2];
	double s3 = part[3];
	double t2 = s2 - s1;
	double t3 = term;
	return (s1*(s3*t2 - 2*s2*t3 + t2*t3)/(s1*t2 - 2*s1*t3 + t2*t3));
    }

    D1 = 0;
    Dk = 0;
    k = termnr - 1;
    for (i = 0; i <= k; i++) {
	if (C[k][i] == 0.0) {
	    C[k][i] = binomial (k, i)
		     * pow ((double) -1, (double) i)
		     * pow ((double) (i+1), (double) (k-2));
	}
        T = C[k][i] / (part[i + 1] - part[i]);
	Dk += part[i + 1] * T;
	D1 += T;
    }

    Debug (fprintf (stderr, "n %2d straight % e levin % e\n",
	termnr, part[termnr], Dk/D1));

    return (Dk / D1);
}

#ifdef DRIVER
int main ()
{
    double p, sum, t;
    int i;

    p = -1; sum = 0;

    for (i = 1; i < 10; i++) {
	p *= -1;
	t = p / (double) i;
	sum += t;
	printf ("t %+.10e s %+.10e l %+.10e\n", t, sum, levin (t, i));
    }
    return (0);
}
#endif
