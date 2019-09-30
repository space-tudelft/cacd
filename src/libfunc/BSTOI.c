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
#include <string.h>

/* assume 8 bits per byte and highest bit is sign bit */
#define MAXINT ~(1 << (b32 - 1))

/* BSTOI() : converts a bit string into an integer using the radix-2 */
/*         : representation                                          */

int BSTOI (char s[])
{
    int res = 0;
    int i, w;
    int len = strlen (s);
    int b32 = sizeof(int)*8;

    if (len > b32){
	fprintf (stderr, "WARNING: BSTOI: bit string contains more then %d bits\n", b32);
	return (MAXINT);
    }
    for (i = 0, w = 1 << (len - 1); i < len; i++, w /= 2) {
        if (s[i] == 'I')
            res += w;
	else if (s[i] == 'X')
	    return (MAXINT);
    }
    return (res);
}
