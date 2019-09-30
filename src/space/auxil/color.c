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

#ifndef ZCOLOR
#include "src/space/auxil/auxil.h"
#endif

int Ncol = NCOL;

void setNcol (int n)
{
    Ncol = n;
}

int isCOLOR_EQ_COLOR (mask_t *x, mask_t *y)
{
    int n = 0;
    do {
	if (x->color[n] != y->color[n]) return 0;
    } while (++n < Ncol);
    return 1;
}

int isCOLOR_PRESENT (mask_t *x, mask_t *y)
{
    int n = 0;
    do {
	if ((x->color[n] & y->color[n]) != y->color[n]) return 0;
    } while (++n < Ncol);
    return 1;
}

int isCOLOR_ABSENT (mask_t *x, mask_t *y)
{
    int n = 0;
    do {
	if (x->color[n] & y->color[n]) return 0;
    } while (++n < Ncol);
    return 1;
}

int isCOLOR (mask_t *x)
{
    int n = 0;
    do {
	if (x->color[n]) return 1;
    } while (++n < Ncol);
    return 0;
}

char *colorBitStr (mask_t *a)
{
    static char buf[NCOL * NSIZE + 1];
    int i, j = 0, n = Ncol;
    while (--n >= 0) {
	for (i = NSIZE; --i >= 0;)
	    buf[j++] = ((a->color[n] >> i) & 1)? '1' : '0';
    }
    buf[j] = 0;
    return buf;
}

char *colorOctStr (mask_t *a)
{
    static char buf[700];
    int c, i, j = 700, n = 0, k = 0;
    buf[--j] = 0;
    do {
	for (i = k; i < NSIZE;) {
	    c = (int)((a->color[n] >> i) & 7);
	    i += 3;
	    k = i - NSIZE;
	    if (k == 1) {
		if (c > 3) c &= 3;
		if (n+1 < Ncol) c += (int)((a->color[n+1] << 2) & 7);
	    }
	    else if (k == 2) {
		if (c > 1) c &= 1;
		if (n+1 < Ncol) c += (int)((a->color[n+1] << 1) & 7);
	    }
	    if (j == 0) say ("colorOctStr: illegal j"), die ();
	    buf[--j] = c + '0';
	}
    } while (++n < Ncol);
    while (buf[j] == '0') ++j;
    if (!buf[j]) --j;
    return buf + j;
}

char *colorHexStr (mask_t *a)
{
    static char buf[600];
    int c, i, j = 0, n = Ncol;
    while (--n >= 0) {
	for (i = NSIZE; (i -= 4) >= 0;) {
	    c = (int)((a->color[n] >> i) & 0xf);
	    if (c >= 10) c += 'a' - 10;
	    else {
		if (!j && !c) continue;
		c += '0';
	    }
	    buf[j++] = c;
	}
    }
    if (!j) buf[j++] = '0';
    buf[j] = 0;
    return buf;
}

char *colorIntStr (mask_t *a)
{
    static char buf2[801];
    char buf1[801];
    int j, i, n, b, b1, b2;

    buf1[b1=800] = 0;
    buf2[b2=800] = 0;
    buf2[--b2] = '0';
    for (n = 0; n < Ncol; ++n) {
	for (i = 0; i < NSIZE; ++i) {
	    if (b1 == 800) { buf1[--b1] = '1'; }
	    else {
		b = 0;
		for (j = 800; --j >= b1;) {
		    b += 2 * (buf1[j] - '0');
		    if (b < 10) { buf1[j] = '0' + b; b = 0; }
		    else { buf1[j] = '0' + (b - 10); b = 1; }
		}
		if (b) {
		    if (b1 == 0) say ("colorIntStr: illegal b1"), die ();
		    buf1[--b1] = '1';
		}
	    }
	    b = (int)((a->color[n] >> i) & 1);
	    if (b) {
		b = 0;
		for (j = 800; --j >= b2;) {
		    b += (buf1[j] - '0') + (buf2[j] - '0');
		    if (b < 10) { buf2[j] = '0' + b; b = 0; }
		    else { buf2[j] = '0' + (b - 10); b = 1; }
		}
		while (--b2 >= b1) {
		    b += (buf1[b2] - '0');
		    if (b < 10) { buf2[b2] = '0' + b; b = 0; }
		    else { buf2[b2] = '0' + (b - 10); b = 1; }
		}
		++b2;
		if (b) buf2[--b2] = '1';
	    }
	}
    }
    return buf2 + b2;
}

int colorindex (mask_t *a)
{
    int i, n = Ncol;
    while (--n >= 0) {
	if (a->color[n]) {
	    for (i = NSIZE; --i > 0;)
		if (a->color[n] & ((mask_base_t)1<<i)) break;
	    return n * NSIZE + i;
	}
    }
    return -1;
}

void initcolorhex (mask_t *a, char *hex)
{
    mask_base_t d;
    char *h;
    int n, m;

    if ((h = hex)) h += strlen (hex);
    for (n = 0; n < Ncol; ++n) {
	a->color[n] = 0;
	for (m = 0; m < NSIZE && h > hex; m += 4) {
	    if (*--h != '0') {
		if (*h >= 'a') d = 10 + (*h - 'a');
		else if (*h >= 'A') d = 10 + (*h - 'A');
		else if (*h >= '0') d = (*h - '0');
		else d = 16;
		if (d > 15) say ("initcolor: illegal hex string"), die ();
		a->color[n] += d << m;
	    }
	}
    }
}

void initcolorint (mask_t *a, char *ins)
{
    char buf1[801];
    char *s;
    int i, j, c, d, n = 0;
    if (!ins || !*ins) {
	do a->color[n] = 0; while (++n < Ncol);
    }
    else {
	j = c = 0;
	for (s = ins; *s; ++s) {
	    if (*s >= '0') d = (*s - '0');
	    else d = 10;
	    if (d > 9) say ("initcolor: illegal int string"), die ();
	    c += d >> 1;
	    if (c || j) buf1[j++] = '0' + c;
	    c = (d & 1)? 5 : 0;
	}
	if (j >= 801) say ("initcolor: illegal j"), die ();
	buf1[j] = 0;
	do {
	    a->color[n] = 0;
	    for (i = 0; i < NSIZE; ++i) {
		if (c) a->color[n] |= (mask_base_t)1 << i;
		c = j = 0;
		if (!*(s = buf1)) break;
		do {
		    d = (*s - '0');
		    c += d >> 1;
		    if (c || j) buf1[j++] = '0' + c;
		    c = (d & 1)? 5 : 0;
		} while (*++s);
		if (j >= 801) say ("initcolor: illegal j"), die ();
		buf1[j] = 0;
	    }
	} while (++n < Ncol);
    }
}

void initcolorbits (mask_t *a, char *bit)
{
    int n, m, b = strlen (bit);
    for (n = 0; n < Ncol; ++n) {
	a->color[n] = 0;
	for (m = 0; m < NSIZE && b > 0; ++m) {
	    if (bit[--b] == '1') a->color[n] += (mask_base_t)1 << m;
	}
    }
}
