/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	A.J. van Genderen
 *	S. de Graaf
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

#include "src/csls/sys_incl.h"

#include "src/csls/class.h"
#include "src/csls/mkdbdefs.h"
#include "src/csls/mkdbincl.h"

Stack * stackcpy (Stack * stck)
{
    if (stck)
	return (stck -> copy ());
    else
	return (stck);
}

char * strsav (char * s)
{
    char *p;
    p = new char [strlen (s) + 1];
#ifdef DMEM
    char_nbyte += strlen(s) + 1;
    char_maxnbyte = char_nbyte > char_maxnbyte ? char_nbyte : char_maxnbyte;
#endif
    strcpy (p, s);
    return (p);
}

void bmove (char *to, char *from, int l)
{
    while (l--) *(to++) = *(from++);
}

void prxs (Stack *xs)
{
    int i, xs_len;

    if (!xs) return;

    xs_len = xs -> length ();
    fprintf (stderr, "stack: %p, length: %d\n", xs, xs_len);
    for (i = 0; i < xs_len; ++i) {
	Xelem * xel = (Xelem *) xs -> access (i);
	fprintf (stderr, "lb: %d, rb: %d\n",
	    xel -> left_bound, xel -> right_bound);
    }
}

void stackfree (Stack *xs, int type)
{
    char *p;

    if (xs) {
	while (!xs -> empty ()) {
	    p = xs -> pop ();
	    switch (type) {
	    case XELEM:
		if (p) delete (Xelem *) p;
		break;
	    case STRING:
		if (p) {
#ifdef DMEM
		    char_nbyte -= strlen (p) + 1;
#endif
		    delete p;
		}
		break;
	    }
	}
    }
}
