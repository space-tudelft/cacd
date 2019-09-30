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

char *adm_bsalloc (int p, char mode);

/* BSROTATE() : bit string is rotated in direction left or right */

char *BSROTATE (char s[], char direction)
{
    char *res;
    int	i;
    int	len = strlen (s);

    res = adm_bsalloc (len + 1, 'p');

    switch (direction) {
	case 'l':
	    for (i = 0; i < (len - 1); i++) res[i] = s[i+1];
	    res[i] = s[0];
	    res[i+1] = '\0';
	    break;
	case 'r':
	    for (i = 0; i < (len - 1); i++) res[i+1] = s[i];
	    res[0] = s[i];
	    res[i+1] = '\0';
	    break;
	default:
	    fprintf (stderr, "ERROR: illegal direction in BSROTATE\n");
	    return (NULL);
    }
    return (res);
}
