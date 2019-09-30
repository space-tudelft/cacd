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

/*
 * gammln from Numerical recipes
 */
double gammln (double xx)
{
    double x, tmp, ser;
    static double cof[6] = {
	76.18009173, -86.50532033, 24.01409822,
	-1.231739516, 0.120858003e-2, -0.536382e-5
    };
    int     j;

    x = xx - 1.0;
    tmp = x + 5.5;
    tmp -= (x + 0.5) * log (tmp);
    ser = 1.0;
    for (j = 0; j <= 5; j++) {
	x += 1.0;
	ser += cof[j] / x;
    }
    return -tmp + log (2.50662827465 * ser);
}
