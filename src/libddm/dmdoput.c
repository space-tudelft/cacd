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

#ifdef DRIVER
#include <stdio.h>
#include <string.h>
int dm_extended_format = 1;
#else
#include "src/libddm/dmstd.h"
#endif
#include <stdarg.h>

#define MAXINT  2147483647
#define MAXLONG 9223372036854775807LL

#define MAXCHAR 100
#define PUT(x) putc((int)x, fp)

static char *tab[] = {
    "00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
    "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
    "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
    "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
    "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69",
    "70", "71", "72", "73", "74", "75", "76", "77", "78", "79",
    "80", "81", "82", "83", "84", "85", "86", "87", "88", "89",
    "90", "91", "92", "93", "94", "95", "96", "97", "98", "99"
};

/*
** This is an efficient routine for writing the records to the files.
*/

int _dmDoput (FILE *fp, char *fmt, ...)
{
    register long   n;
    register long  *np;
    register char  *s;
    double  d;
    long    buf[24];
    long    n1;
    long    w;
    long long l;
    va_list ap;

    va_start (ap, fmt);

    fmt--;
loop:
    switch (*++fmt) {
	default: 		/* ordinary char, usually space */
	    PUT (*fmt);
	    goto loop;

	case 'L': 		/* long long argument */
	    l = va_arg (ap, long long);
	    if (l < 0) {
		PUT ('-');
		if (l < -MAXLONG)
		    l =  MAXLONG;
		else
		    l = -l;
	    }
	    np = buf;
	    while (l > 99) {
		*np++ = l % 100;
		l /= 100;
	    };
	    n = l;
	    goto caseD2;

	case 'F': 		/* double to be written as scaled int */
	    d = va_arg (ap, double);
            if (dm_extended_format) {
                fprintf (fp, "%f", d);
                goto loop;
            }
            else {
	        n = (d == 0.0 ? 0 : (long) (1000.0 * d + (d > 0 ? 0.5 : -0.5)));
	        goto caseD;
            }

	case 'W': 		/* fieldwith 16 */
	case 'D': 		/* integer argument */
	    n = va_arg (ap, long);
	    if (*fmt == 'W') {
	    /*
	     ** could make this more efficient,
	     ** but the format W is not used very often
	     ** and will probably become obsolete soon
	     */
		w = ((n1 = n) < 0) ? 15 : 16;
		while (n1 /= 10) w--;
		while (--w > 0) PUT (' ');
	    }

    caseD:
	    if (n < 0) {
		PUT ('-');
		if (n < -MAXINT) {
		    n =  MAXINT;
		    fprintf (stderr, "_dmDoput: warning: int32 overflow\n");
		}
		else
		    n = -n;
	    }
	    else
		if (n > MAXINT) {
		    n = MAXINT;
		    fprintf (stderr, "_dmDoput: warning: int32 overflow\n");
		}
	    np = buf;
	    while (n > 99) {
		*np++ = n % 100;
		n /= 100;
	    };
    caseD2:
	    if (n < 10)
		PUT ('0' + n);	/* ASCII */
	    else {
		PUT (tab[n][0]);
		PUT (tab[n][1]);
	    }
	    while (np > buf) {
		PUT (tab[*--np][0]);
		PUT (tab[*np][1]);
	    }
	    goto loop;

	case 'A': 		/* string with white space */

	/* This format is mainly used for attribute strings * of the form
	   "name=value; name=value". */

	    s = va_arg (ap, char *);

	/* put first the length of the string */
	    n = s ? (long) strlen (s) : 0;
	    np = buf;
	    while (n > 99) {
		*np++ = n % 100;
		n /= 100;
	    };
	    if (n < 10)
		PUT ('0' + n);	/* ASCII */
	    else {
		PUT (tab[n][0]);
		PUT (tab[n][1]);
	    }
	    while (np > buf) {
		PUT (tab[*--np][0]);
		PUT (tab[*np][1]);
	    }

	/* Now put a space to delimit the number from * the string. */
	    PUT (' ');

	/* Now the string itself, we do not need something * special for
	   the empty string, since we know its length. */
	    if (s != NULL)
		while (*s) PUT (*(s++));

	    goto loop;

	case 'S': 		/* string argument */
	    s = va_arg (ap, char *);
	    if (s != NULL && *s != '\0') {
		while (*s) PUT (*(s++));
	    }
	    else {
		PUT ('~');	/* special, denotes empty string */
	    }
	    goto loop;

	case 'C': 		/* character argument */
	    n = va_arg (ap, int);/* C converts char to int before pushing
				   it on the stack, so pop an int */
	    PUT (n);
	    goto loop;

	case '\0': 		/* format exhausted */
	    return (ferror (fp) ? -1 : 0);
    };
}

#ifdef DRIVER

int main (int argc, char *argv[])
{
    int     r;
    long    i;
    long long l;
    char    c = 'c';

    i = MAXINT;
    l = MAXLONG;
    r = _dmDoput (stdout, "C D W S A F D L\n", 'a', 1234L, i, "abc", "de fg", 1.1115, 0L, l);
    if (r < 0) fprintf (stderr, "error");
    i += 1;
    l += 1;
    r = _dmDoput (stdout, "C D W S A F D L\n", 'b', 123L, i, "abcde", "", 1.1114, -123L, l);
    if (r < 0) fprintf (stderr, "error");
    i += 1;
    l += 1;
    r = _dmDoput (stdout, "C D W S A F D L\n", c, 123L, i, "abc", "a", -1.2, -456L, l);
    if (r < 0) fprintf (stderr, "error");
    i += 1;
    l += 1;
    r = _dmDoput (stdout, "C D W S A F D L\n", c, 123L, i, "abc", "a", -1.2, -789L, l);
    if (r < 0) fprintf (stderr, "error");
    return (0);
}
#endif /* DRIVER */
