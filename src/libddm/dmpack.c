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

/*
** This file contains two efficient routines for
** writing and reading of
**
**	(signed) integers
**	7-bit variable length ascii strings
**	8-bit bytes
**
** If it were desired to write floats, the mantisse and
** the exponent should be treated as two signed integers.
**
** A data reduction of more than 50% when compared to ascii files,
** is achieved by converting numbers into a radix-13 representation
** and storing the digits in only 4 bits (as opposed to 8).
** Further data reduction is achieved by dropping of trailing zeros.
**
** USAGE (see the test driver):
**
**	 _dmPack (fp, fmt, va_alist);
**	 _dmUnpack (fp, fmt, va_alist);
**
** A test driver is included: gcc -DDRIVER ...
*/

#ifdef DRIVER
#include <stdio.h>
#include <stdlib.h>
#else
#include "src/libddm/dmstd.h"
#endif
#include <stdarg.h>

#define BASE   0xD		/* Radix */
#define MIN    0xD		/* Separator for Negative Num */
#define SEP    0xE		/* Field Separator */
#define EOR    0xF		/* End of Record */

#define EVEN   0
#define ODD    1

static unsigned  CHAR;
static int parity;
#ifdef DRIVER
static int width = 6;
#endif

#define PUTNIBBLE(x) (parity==ODD?\
    (parity = EVEN, putc ((CHAR|((x)&017)), fp)):\
    (parity = ODD,  CHAR = (unsigned)((x) << 4)))

/*
   @@: keesjan: Note that `__DM_DEBUG' should actually be `DM_DEBUG', but something
   goes wrong when we set __DM_DEBUG. Should look into this ...
*/

#ifdef __DM_DEBUG
#define TOGGLE(p) (p = (p == ODD ? EVEN : ODD))
#undef PUTNIBBLE
#define PUTNIBBLE(x) (putnibble (x, fp))
void putnibble (x, fp) int x; FILE *fp; { fprintf(fp,"%x",x); TOGGLE(parity); }
#endif

/*
** On a two's complement machine and when EOF == -1 (like on HP),
** EOF as returned by getc looks like EOR and this is just what we want.
*/
#define GETNIBBLE(fp) (parity==ODD?\
    (parity = EVEN, (unsigned)CHAR&017):\
    (parity = ODD,  (unsigned)((CHAR=getc(fp))>>4)&017))

#ifdef __DM_DEBUG
#undef GETNIBBLE
#define GETNIBBLE(fp) (getnibble (fp))
unsigned int getnibble (fp) FILE *fp; {
    int s = getc (fp);
    s -= (s < 'a' ? '0' : 'a' - 10);
    /* fprintf (stderr,"nibble=%d (%x)\n",s,s); */
    TOGGLE(parity); return s; }
#endif

#define MAXDOUBLE 2147483647.0
#define MAXINT    2147483647
#define MAXLONG   9223372036854775807LL

#ifdef DRIVER
#define DME_PUT 1
#define DME_GET 2
#define DME_DDM 3
int dmerrno;
int dm_extended_format = 1;
int dm_maxname = 256;
void dmError (char *s) { fprintf (stderr, "%s\n", s); }
#endif

int _dmPack (FILE *fp, char *fmt, ...)
{
    char   *s;
    char    buf[36], format[36];
    double  d, e;
    int     sep = 0;	/* separator required? */
    int     nil_cnt = 0;
    int     w, i, j, leading_zeros = 0, min = 0;
    long long n, m = -1;

    va_list ap;

    parity = EVEN;	/* even-odd nibble */

    va_start (ap, fmt);

    fmt--;
loop:
    switch (*++fmt) {
	default:
	    dmerrno = DME_DDM;
	    dmError ("_dmPack: bad fmt");
	    return (-1);

	case ' ':
	case '\t':
	case '\n':
	    goto loop;

	case 'F':		   /* double to be written as scaled int */
	    /* For dm_extended_format the double is written as two int's */
	    d = va_arg (ap, double);
	    if (d < 0) { d = -d; min = 1; }

	    if (dm_extended_format) {
                if (d == 0.0) {
                    nil_cnt += 2;
                    goto loop;
                }
		if (d > MAXDOUBLE) goto overflow;

                /* We assume a precision of 15 decimal digits for a double. */
                j = 15;
                for (e = d; e > 0.999999999999999; e *= 0.1) --j;

		n = (long long)(d + 0.5 * 1e-15 * d); /* rounding */
		e = d - n;
		sprintf (format, "%%.%dlf", j); /* precision */
		sprintf (buf, format, e);
#ifdef DM_DEBUG
                fprintf (stderr, "d=%f, n=%lld, e=%f, buf=%s, format=%s\n", d, n, e, buf, format);
#endif

		for (i = 0; buf[i] != '.'; i++);

                /* Count the leading zeros of the mantissa,
                ** they should be stored before the mantissa.
                ** The mantissa 'm' is stored as second integer.
                ** If the whole buffer contains only zeros,
                ** then we don't set 'leading_zeros' (SdeG).
                */
                for (j = i+1; buf[j] == '0'; j++);

		if (buf[j] && e > 0) {
		    w = j - i; /* j > i */
		    leading_zeros = w - 1;
		    while (buf[++j]) if (buf[j] != '0') w = j - i;
		    e *= 10.0;
		    while (--w) e *= 10.0;
		    while ((e + 0.5) >= MAXDOUBLE + 1.0) e *= 0.1;
		    e = e + 0.5;
		    m = (long long)e;
		}
		else m = 0;
#ifdef DM_DEBUG
                fprintf (stderr, "e=%f, m=%lld, leading_zeros=%d\n", e, m, leading_zeros);
#endif
	    }
	    else {
		e = 1000 * d + 0.5;
		if (e >= MAXDOUBLE + 1.0) goto overflow;
		n = (long long) e;
#ifdef DM_DEBUG
		fprintf (stderr, "d=%f e=%f n=%lld\n", d, e, n);
#endif
	    }
	    goto caseD;

	case 'L': 		/* long long argument */
	    n = va_arg (ap, long long);
	    if (n < 0) {
		if (n < -MAXLONG)
		    n =  MAXLONG;
		else
		    n = -n;
		min = 1;
	    }
	    goto caseD;

	case 'D': 		/* integer argument */
	case 'W': 		/* fieldwith 16 bytes */
	    n = va_arg (ap, long);
	    if (n < 0) {
		if (n < -MAXINT) goto overflow;
		n = -n; min = 1;
	    }
	    else if (n > MAXINT) goto overflow;

	caseD:
	    /* If the number is 0 and the format is not 'W',
	    ** this field will be stripped of if it is the last
	    ** field or if it is followed by zeros only.
	    ** Therefore, we do not write a 0 now, but just
	    ** remember that it should be written if followed
	    ** by a non-zero.
	    */
	    if (n == 0 && *fmt != 'W') {
		nil_cnt++;
		if (m >= 0) { /* extended_format F: also write the fraction */
		    n = m; m = -1; goto caseD;
		}
		goto loop;
	    }
	    else {
		for ( ; nil_cnt; nil_cnt--) {
		    if (sep) PUTNIBBLE (SEP);
		    else sep = 1;
		    PUTNIBBLE (0);
		}
	    }

	    /* If the format is 'W', the field must be 16 bytes
	    ** wide and the record 4*16+1=65 bytes.
	    ** Extra, leading, SEP nibbles are first added
	    ** in order to make the field 16 bytes wide.
	    ** The +1 byte is automatically OK because of the
	    ** record separator (and the fill-nibble).
	    */
	    if (*fmt == 'W') {
		w = (min || sep) ? 31 : 32;
		m = n;
		while (m > BASE - 1) { --w; m /= BASE; }
		while (--w > 0) PUTNIBBLE (SEP);
		m = -1;
	    }

	    /* If the number is negative, only a MIN nibble is
	    ** needed, no SEP nibble.
	    */
	    if (min) { min = 0; PUTNIBBLE (MIN); }
	    else if (sep) PUTNIBBLE (SEP);

	    /* Here comes the actual conversion.
	    ** Numbers are stored with the
	    ** least significant digits first!!!
	    */
	    while (n > BASE - 1) {
		PUTNIBBLE (n % BASE);
		n /= BASE;
	    }
	    PUTNIBBLE (n);
	    sep = 1;

	    if (m >= 0) { /* extended_format F: also write the fraction */
		n = m; m = -1; goto caseD;
	    }
	    while (leading_zeros) { /* extended_format F: write fraction */
		PUTNIBBLE (0);
		leading_zeros--;
	    }
	    goto loop;

	case 'A':
	case 'S':
	    /* 7-bits ascii string argument
	    ** The string has variable length, and is
	    ** terminated by the SEP or EOR nibble.
	    ** Therefore, the characters of the string
	    ** must be written in coded form.
	    ** For case 'A', the string can contain white space.
	    */
	    s = va_arg (ap, char *);
	    for ( ; nil_cnt; nil_cnt--) { /* write zeros first */
		if (sep) PUTNIBBLE (SEP);
		else sep = 1;
		PUTNIBBLE (0);
	    }
	    if (sep) PUTNIBBLE (SEP);
	    else sep = 1;
	    s--;
	    i = 1;
	    while (++s && *s) {
		PUTNIBBLE (*s / BASE);
		PUTNIBBLE (*s % BASE);
		if (*fmt == 'S' && i >= dm_maxname) break;
		i++;
	    }
	    goto loop;

	case 'C':
	    /* 8-bits byte argument
	    ** This argument is of fixed length, and
	    ** therefore does not need a terminator.
	    ** So, no coding is needed but the byte is
	    ** stored literally.
	    ** The byte is always stored in two parts,
	    ** but this is not needed in case the byte starts
	    ** at an even nibble position.
	    */
	    n = va_arg (ap, int); /* C converts char to int before pushing
				     it on the stack, so pop an int */

	    for ( ; nil_cnt; nil_cnt--) { /* write zeros first */
		if (sep) PUTNIBBLE (SEP);
		else sep = 1;
		PUTNIBBLE (0);
	    }
	    if (sep) PUTNIBBLE (SEP);
	    sep = 0;		/* No separator after single byte */
	    PUTNIBBLE (n >> 4);	/* NIBBLE macro does masking */
	    PUTNIBBLE (n);
	    goto loop;

	case '\0': 		/* format exhausted */
	    /* throw away trailing zeros */

	    /* The total record always occupies a whole number
	    ** of bytes, so add a fill nibble if the number
	    ** of nibbles now in the record is even.
	    */
            if (parity == EVEN) PUTNIBBLE (SEP);
            PUTNIBBLE (EOR);
	    if (ferror (fp)) {
		dmerrno = DME_PUT;
		dmError ("_dmPack: ferror");
		return (-1);
	    }
	    return (0);
    }
overflow:
    dmerrno = DME_PUT;
    dmError ("_dmPack: Integer Overflow");
    return (-1);
}

int _dmUnpack (FILE *fp, char *fmt, ...)
{
    register long *ip;
    register char *cp;
    long long *lp, lf;
    double *dp, e;
    long    itmp, f;
    unsigned int nibble;
    char   *Fmt;
    int     sep = 0;		/* separator required? */
    int     nmatch = 0;		/* no matched formats */
    int     leading_zeros, min = 0;

    va_list ap;

    parity = EVEN;	/* even-odd nibble */

    va_start (ap, fmt);

    nibble = GETNIBBLE (fp);

    fmt--;
    Fmt = fmt;
loop:
    if (feof (fp)) {
	if (fmt == Fmt) return (EOF);
	dmerrno = DME_GET;
	dmError ("_dmUnpack: premature End Of File");
	return (EOF);
    }
    switch (*++fmt) {
	default:
	    dmerrno = DME_DDM;
	    dmError ("_dmUnpack: bad fmt");
	    return (EOF);

	case ' ': 		/* white space */
	case '\t':
	case '\n':
	    goto loop;

	case 'F':		 /* double that was written as scaled int */
	    /* For dm_extended_format the double was written as two int's */
	    dp = va_arg (ap, double *);
	    nmatch++;

	    if (nibble == EOR) { *dp = 0; goto loop; } /* deleted trailing zero */

	    if (nibble == MIN) {
		min = 1;
		nibble = GETNIBBLE (fp);
	    }
	    else if (sep) nibble = GETNIBBLE (fp);
	    sep = 1;

	    f = 1;
	    itmp = 0;
	    while (nibble < BASE) { /* if true, a valid digit */
		itmp += f * nibble;
		f *= BASE;
		nibble = GETNIBBLE (fp);
	    }
	    *dp = itmp;

	    if (dm_extended_format) {
		if (nibble != EOR) { /* add fraction */
		    if (nibble == MIN) min = 1;
		    f = 1;
		    itmp = 0;
		    leading_zeros = 0;
		    while ((nibble = GETNIBBLE (fp)) < BASE) { /* if true, a valid digit */
			if (nibble == 0)
			    leading_zeros++;
			else
			    leading_zeros = 0;
			itmp += f * nibble;
			f *= BASE;
		    }
#ifdef DRIVER
		    width = leading_zeros;
#endif
		    e = itmp;
		    while (e >= 1.0) {
			e *= 0.1;
#ifdef DRIVER
			++width;
#endif
		    }
		    /* Convert trailing zeros to leading ones.
		    */
		    while (leading_zeros-- > 0) e *= 0.1;

		    *dp += e;
		}
	    }
	    else {
		*dp /= 1000;
	    }
	    if (min) { *dp = -*dp; min = 0; }
	    goto loop;

	case 'L': 		/* long long argument */
	    lp = va_arg (ap, long long *);
	    *lp = 0;
	    nmatch++;

#ifdef DRIVER
	    width = 0;
#endif
	    if (nibble == EOR) goto loop; /* deleted trailing zero */

	    if (nibble == MIN) {
		lf = -1;
		nibble = GETNIBBLE (fp);
#ifdef DRIVER
		++width;
#endif
	    }
	    else {
		lf = 1;
		if (sep) nibble = GETNIBBLE (fp);
	    }
	    sep = 1;

	    while (nibble < BASE) { /* if true, a valid digit */
		*lp += lf * nibble;
		lf *= BASE;
		nibble = GETNIBBLE (fp);
#ifdef DRIVER
		++width;
#endif
	    }
	    goto loop;

	case 'D': 		/* integer argument */
	case 'W': 		/* fieldwith 16 bytes */
	    ip = va_arg (ap, long *);
	    *ip = 0;
	    nmatch++;

#ifdef DRIVER
	    width = 0;
#endif
	    if (nibble == EOR) goto loop; /* deleted trailing zero */

	    if (*fmt == 'W') { /* get leading SEP nibbles first */
		while (nibble == SEP) {
		    nibble = GETNIBBLE (fp);
#ifdef DRIVER
		    ++width;
#endif
		}
	    }

	    if (nibble == MIN) {
		f = -1;
		nibble = GETNIBBLE (fp);
#ifdef DRIVER
		++width;
#endif
	    }
	    else {
		f = 1;
		if (sep && *fmt != 'W') nibble = GETNIBBLE (fp);
	    }
	    sep = 1;

	    while (nibble < BASE) { /* if true, a valid digit */
		*ip += f * nibble;
		f *= BASE;
		nibble = GETNIBBLE (fp);
#ifdef DRIVER
		++width;
#endif
	    }
	    goto loop;

	case 'S': 		/* string argument */
	case 'A':
	    cp = va_arg (ap, char *);
	    nmatch++;
	    if (nibble == EOR) { *cp = '\0'; goto loop; }
	    if (sep) nibble = GETNIBBLE (fp);
	    sep = 1;

	    while (nibble < BASE) {
		*cp = nibble * BASE;
		*cp++ += GETNIBBLE (fp);
		nibble = GETNIBBLE (fp);
	    }
	    *cp = '\0';
	    goto loop;

	case 'C': 		/* byte argument */
	    cp = va_arg (ap, char *);
	    if (sep) nibble = GETNIBBLE (fp);
	    sep = 0;		/* No separator after single byte */

	    *cp = nibble << 4;
	    *cp |= GETNIBBLE (fp);
	    nibble = GETNIBBLE (fp);
	    nmatch++;
	    goto loop;

	case '\0': 		/* format exhausted */
	    /* The current nibble is possibly the EOR
	    ** nibble or the fill nibble, but there is no
	    ** need to get the EOR nibble in the latter
	    ** case, because with a new call to _dmUnpack
	    ** the next byte is got.
	    ** A new record always starts on a byte-boundary.
	    */
	    if (ferror (fp)) {
		dmerrno = DME_GET;
		dmError ("_dmUnpack: ferror");
		return (EOF);
	    }
	    return (nmatch);
    }
}

#ifdef DRIVER

int main (int argc, char *argv[])
{
    FILE *fp;
    char  s1[100], s2[100];
    char  c, *arg, *dformat = NULL;
    double d, d2;
    long  i, i1, i2, i3, i4, i5;
    long long l;
    int   r;

    if (argc == 3) {
	if (argv[1][0] != '-') goto usage;
	switch (argv[1][1]) {
	    case 'd': dformat = "D\n"; break;
	    case 'w': dformat = "W\n"; break;
	    case 'e': dm_extended_format = 0;
	    case 'f': break;
	    default: goto usage;
	}
	arg = argv[2];
    }
    else if (argc == 2) arg = argv[1];
    else goto usage;

    if (argc == 3 || arg[0] != '-' || (arg[1] >= '0' && arg[1] <= '9')) {
	fp = fopen ("test", "w");
	if (dformat) {
	    i = atol (arg); r = _dmPack (fp, dformat, i);
	} else {
	    d = atof (arg); r = _dmPack (fp, "F\n", d);
	}
	if (r < 0) { fprintf (stderr, "pack: write error\n"); return (1); }
	fclose (fp);

	fp = fopen ("test", "r");
	if (dformat) {
	    r = _dmUnpack (fp, dformat, &i);
	} else {
	    r = _dmUnpack (fp, "F\n", &d);
	}
	if (r != 1) { fprintf (stderr, "unpack: read error\n"); return (1); }
	if (dformat) {
	    fprintf (stdout, "d=%ld (width=%d)\n", i, width);
	} else {
	    if (width < 6) width = 6;
	    fprintf (stdout, "f=%.*f\n", width, d);
	}
    }
    else if (arg[1] == 'p') {
	fp = fopen ("test", "w");
	if (!fp) { fprintf (stderr, "error: cannot write file 'test'\n"); return (1); }
	fprintf (stderr, "pack: output written to file 'test', use -u to unpack\n");
	r = _dmPack (fp, "C D D W S A F D D F L\n", 'x', 0L, 123L, 0L, "abc", "de fg", 0.1, 0L, 0L, -0.25, 0LL);
	if (r < 0) { fprintf (stderr, "pack: write error\n"); return (1); }
	r = _dmPack (fp, "C D D W S A F D D F L\n", 'x', 123L, 0L, 456L, "abcefg", "", 1.2, 0L, 1L, -0.50, 1LL);
	if (r < 0) { fprintf (stderr, "pack: write error\n"); return (1); }
	i1 = MAXINT;
	l = MAXLONG;
	r = _dmPack (fp, "C D D W S A F D D F L\n", 'x', i1, 0L, 456L, "simon", "a", -17.12345, 0L, 0L, -0.75, l);
	if (r < 0) { fprintf (stderr, "pack: write error\n"); return (1); }
	r = _dmPack (fp, "C D D W S A F D D F L\n", 'x',-i1, 0L, 456L, "degraaf", "", 17.12345, 0L, 0L, -1.0, -l);
	if (r < 0) { fprintf (stderr, "pack: write error\n"); return (1); }
    }
    else if (arg[1] == 'u') {
	fp = fopen ("test", "r");
	if (!fp) { fprintf (stderr, "error: cannot read file 'test'\n"); return (1); }
      for (;;) {
	r = _dmUnpack (fp, "CDDWSAFDDFL\n", &c, &i1, &i2, &i3, s1, s2, &d, &i4, &i5, &d2, &l);
	if (r == EOF) break;
	if (r != 11) { fprintf (stderr, "unpack: read error\n"); return (1); }
	fprintf (stdout, "%c %ld %ld %ld \"%s\" \"%s\" %f %ld %ld %f %lld\n", c, i1, i2, i3, s1, s2, d, i4, i5, d2, l);
      }
    }
    else {
usage:	fprintf (stderr, "usage: dmpack [[-defw] float|int] | -p | -u\n");
    }
    return (0);
}
#endif /* DRIVER */
