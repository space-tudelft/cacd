/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van der Hoeven
 *	P. van der Wolf
 *	P. Bingley
 *	T.G.R. van Leuken
 *	T. Vogel
 *	F. Beeftink
 *	M. Grueter
 *	E.F. Matthijssen
 *	G.W. Sloof
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

#include "src/libddm/dmstd.h"

extern int vfscanf ();

#include <stdarg.h>

#define    BASE  10
#define MAXCHAR  100
#define  PUT(x)  (*ptr++ = (unsigned) (x))
#define UNPUT()  (ptr--)

/*
** This is a fast sprintf, only %s and %d format.
*/

#ifndef _dmSprintf /* in dmincl.h we can #define _dmSprintf sprintf ... */

int _dmSprintf (char *ptr, char *fmt, ...)
{
    register char  *s;
    register char  *cp;
    register int    n;
    char    prbuf[11];
    va_list ap;

    va_start (ap, fmt);

    fmt--;
loop:
    if (*++fmt == '\0') {
	PUT ('\0');
	return (0);
    }
    else
	if (*fmt != '%') {
	    PUT (*fmt);		/* ordinary char */
	    goto loop;
	}

    if (*++fmt == 's') {
	s = va_arg (ap, char *);
	while (PUT (*s++));
	UNPUT ();
	goto loop;
    }
    else
	if (*fmt == 'd') {
	    n = va_arg (ap, int);

	    if (n < 0) {
		PUT ('-');
		n = (unsigned) (-n);
	    }

	    cp = prbuf;
	    do {
		*cp++ = '0' + n % BASE;/* ASCII */
	    } while (n /= BASE);

	    do {
		PUT (*--cp);
	    } while (cp > prbuf);
	    goto loop;
	}
	else {
	    dmerrno = DME_DDM;
	    dmError2 ("_dmSprintf", fmt);
	    PUT ('\0');
	    return (-1);
	}
}

#endif /* _dmSprintf */

int dmPrintf (DM_STREAM *iop, char *format, ...)
{
    int count;
    va_list ap;

    va_start (ap, format);

    count = vfprintf (iop -> dmfp, format, ap);

    va_end (ap);
    return (ferror (iop -> dmfp) ? EOF : count);
}

int dmScanf (DM_STREAM *iop, char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);

    return vfscanf (iop -> dmfp, fmt, ap);
}
