/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	O. Hol
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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

char *adm_bsalloc (int p, char mode);

/* ITOBS() : converts an integer into a p-bits long bit string           */
/*         : using the radix-2 representation                            */
/*         : a warning is given when integer is too 'big' for bit string */

char *ITOBS (int i, int p)
{
    char *s;
    int n, w;

    s = adm_bsalloc (p + 1, 'p');

    if (i < 0) {
        fprintf (stderr, "ERROR: ITOBS: integer %d should be positive\n", i);
	return (s);
    }

    if (p < 8 * sizeof (int) && i > ((1 << p) - 1)) {
        fprintf (stderr, "WARNING: ITOBS: integer %d too big for %d bits\n", i, p);
    }

    for (n = 0, w = 1; n < p && n < 8 * sizeof (int) - 1; n++, w *= 2) {
        if (i % (2 * w) > 0) {
            s[p - n - 1] = 'I';
            i -= w;
        }
        else {
            s[p - n - 1] = 'O';
        }
    }
    while (p - n - 1 >= 0) {
        s[p - n - 1] = 'O';
	n++;
    }
    s[p] = '\0';

    return (s);
}
