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
int dm_extended_format = 1;
#else
#include "src/libddm/dmstd.h"
#endif
#include <stdarg.h>

#define MAXINT  2147483647
#define MAXLONG 9223372036854775807LL

/*
** Own version of isdigit and isspace, for 30% speed up of _dmDoget.
** '\t', '\n', '\r' and ' ' are considered space
*/
#undef isspace
#undef isdigit

#define isspace(c) space[c]	/* watch out for c==EOF */
#define isdigit(c) ((c) >= '0' && (c) <= '9')

static char space[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1
};

#define UNGETC(c,iop) ungetc(c,iop)

int _dmDoget (FILE *iop, char *fmt, ...)
{
    va_list ap;

    register int    ch;
    register long   val;
    register char  *ptr;
    long long vall;
    int     nmatch = 0;
    int     negflg, i;
    double d;

    va_start (ap, fmt);
    ch = getc (iop);

    fmt--;
loop:
    switch (*++fmt) {
	case 'L':
	    negflg = 0;
	    while (ch != EOF && isspace (ch)) ch = getc (iop);

	    if (ch == '-') { negflg++; ch = getc (iop); }
	    else if (ch == '+') ch = getc (iop);

	    if (isdigit (ch)) {
		i = 1;
		vall = ch - '0';
		for (ch = getc (iop); isdigit (ch); ch = getc (iop)) {
		    ++i;
		    vall = 10 * vall + (ch - '0');
		}
		if (i >= 19 && (i > 19 || vall < 0)) {
		    fprintf (stderr, "_dmDoget: warning: int64 overflow\n");
		    vall = MAXLONG;
		}

		if (negflg) vall = -vall;

		*va_arg (ap, long long *) = vall;
		nmatch++;
		goto loop;
	    }
	    break;		/* no match */

	case 'F': 		/* double that was written as scaled int */
            if (dm_extended_format) {
                if (fscanf (iop, "%lf", &d) == 1) {
                    *va_arg (ap, double *) = d;
                    nmatch++;
                    goto loop;
                }
                break;
            }

	case 'D':
	case 'W':
	    negflg = 0;
	    while (ch != EOF && isspace (ch)) ch = getc (iop);

	    if (ch == '-') {
		negflg++;
		ch = getc (iop);
	    }
	    else
		if (ch == '+')
		    ch = getc (iop);

	    if (isdigit (ch)) {
		i = 1;
		val = ch - '0';
		for (ch = getc (iop); isdigit (ch); ch = getc (iop)) {
		    ++i;
		    val = 10 * val + ch - '0';
		}
		if (i >= 10 && (i > 10 || (int)val < 0)) {
		    fprintf (stderr, "_dmDoget: warning: int32 overflow\n");
		    val = MAXINT;
		}

		if (negflg) val = -val;

		if (*fmt == 'F') {
		    *va_arg (ap, double *) = (double) val / 1000.0;
		}
		else {
		    *va_arg (ap, long *) = val;
		}
		nmatch++;
		goto loop;
	    }
	    break;		/* no match */

	case 'A': 		/* string with white space */

	    while (ch != EOF && isspace (ch)) ch = getc (iop);

	    if (!isdigit (ch)) break; /* error */

	    val = ch - '0';
	    for (ch = getc (iop); isdigit (ch); ch = getc (iop))
		val = 10 * val + ch - '0';

	    ptr = va_arg (ap, char *);

	/* skip current ch, it should be a space */

	/* read 'val' characters */
	    while (val-- > 0) {
		if ((ch = getc (iop)) == EOF) { /* premature end-of-file */
		    *ptr = '\0';
		    break;
		}
		*ptr++ = (char) (ch & 0xff);
	    }
	    ch = getc (iop);	/* read ahead next character */

	    *ptr = '\0';
	    nmatch++;
	    goto loop;

	case 'S':
	    while (ch != EOF && isspace (ch)) ch = getc (iop);

	    if (ch != EOF) {
		ptr = va_arg (ap, char *);
		do {
		    *ptr++ = (char) (ch & 0xff);
		} while ((ch = getc (iop)) != EOF && !isspace (ch));
		*ptr = '\0';

		nmatch++;
		goto loop;
	    }
	    break;		/* no match */

	case 'C':
	    if (ch != EOF) {
		nmatch++;
		*va_arg (ap, char *) = (char) (ch & 0xff);
		ch = getc (iop);
		goto loop;
	    }
	    break;		/* end of input, no match */

	case '\0':
            while (ch != EOF && ch != '\n' && isspace (ch)) ch = getc (iop);
            if (ch != EOF) UNGETC (ch, iop);
            return (nmatch);    /* end of input */

	case ' ':
	case '\t':
	case '\n':
	    while (ch != EOF && isspace (ch)) ch = getc (iop);
	    goto loop;

	default:
	    UNGETC (*fmt, iop);
	    return (nmatch);	/* failed to match input */
    }

    if (ch == EOF)
	return (EOF);		/* end of input */
    else {
	UNGETC (ch, iop);
	return (nmatch);	/* failed to match input */
    }
}

#ifdef DRIVER
int main (int argc, char *argv[])
{
    int     r;
    long    i1, i2, i3;
    long long l;
    double  d;
    char    c;
    char    s1[100];
    char    s2[100];
    for (;;) {
	r = _dmDoget (stdin, "CDWSAFDL\n", &c, &i1, &i2, s1, s2, &d, &i3, &l);
	if (r == EOF) break;
	printf ("%c %ld %ld %s \"%s\" %f %ld %lld result: %d\n", c, i1, i2, s1, s2, d, i3, l, r);
    }
    return (0);
}
#endif /* DRIVER */
