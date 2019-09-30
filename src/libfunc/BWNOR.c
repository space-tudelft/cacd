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
#include <stdarg.h>
#include <string.h>

char *adm_bsalloc (int p, char mode);

/* BWNOR() : returns a bit string containing the result of a bit-wise */
/*         : nor-operation on the given bit strings                   */

char *BWNOR (char *arg0, ...)
{
    va_list ap;
    char *str, *res;
    int	i, lenstr;

    va_start (ap, arg0);
    str = arg0;
    lenstr = strlen (str);

    res = adm_bsalloc (lenstr + 1, 'p');

    for (i = 0; i < lenstr; i++) res[i] = 'I';
    res[i] = '\0';

    do {
	for (i = 0; i < lenstr; i++) {
	    if (res[i] != 'O') {
	    	if (str[i] == 'I')
	            res[i] = 'O';
	    	else if (str[i] == 'X')
	            res[i] = 'X';
	    }
	}
    }
    while (strcmp ((str = va_arg (ap, char *)), "ENDVAR") != 0);
    va_end (ap);
    return (res);
}
