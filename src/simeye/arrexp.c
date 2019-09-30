/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	A.J. van Genderen
 *	S. de Graaf
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void do_cancel (void);
void do_flush (void);
void do_write (int c);
char *getword (FILE *fp);
int lookahead (FILE *fp);

int main (int argc, char *argv[])
{
    FILE *fp;
    char namesave[256];
    char left[256];
    char right[256];
    char *w;
    int i, l, r;

    if (argc != 2) {
	fprintf (stderr, "Usage: arrexp infile\n");
	exit (1);
    }
    if (!(fp = fopen (argv[1], "r"))) {
	fprintf (stderr, "Cannot open %s\n", argv[1]);
	exit (1);
    }

    while ((w = getword (fp))) {
	right[0] = '\0';
	if (w[0] == '['
	    && ((w = getword (fp)) && isdigit ((int)w[0]) && strcpy (left, w))
	    && (w = getword (fp))
	    && (w[0] == ']' || (w[0] == '.'
		   && ((w = getword (fp)) && w[0] == '.')
		   && ((w = getword (fp)) && isdigit ((int)w[0]) && strcpy (right, w))
		   && ((w = getword (fp)) && w[0] == ']')
		   && lookahead (fp) != '.'))) {

	    l = atoi (left);
	    if (right[0] != '\0')
		r = atoi (right);
	    else
		r = l;

	    if (l < r) {
		for (i = l; i <= r; i++) {
		    if (i > l) printf (" %s", namesave);
		    printf ("_%d_", i);
		}
	    }
	    else {
		for (i = l; i >= r; i--) {
		    if (i < l) printf (" %s", namesave);
		    printf ("_%d_", i);
		}
	    }
	    do_cancel ();
	}
	else {
	    do_flush ();
	}
	strcpy (namesave, w);
    }

    do_flush ();

    exit (0);
    return (0);
}

int lookahead (FILE *fp)
{
    int c = getc (fp);
    if (c != EOF) ungetc (c, fp);
    return (c);
}

char outbuf[8000];
int outbuf_cnt = 0;

char *getword (FILE *fp)
{
    static char buf[256];
    int c, i;

    while ((c = getc (fp)) != EOF && isspace (c)) do_write (c);
    if (c == EOF) return (NULL);
    do_write (c);

    i = 0;
    buf[i++] = c;
    if (c != '[' && c != ']' && c != '.' && c != ',') {
	while ((c = getc (fp)) != EOF && !isspace (c) && c != '[' && c != ']' && c != '.' && c != ',') {
	    buf[i++] = c;
	    do_write (c);
	}
	if (c == EOF) return (NULL);
	ungetc (c, fp);
    }
    buf[i] = '\0';
    return (buf);
}

void do_write (int c)
{
    outbuf[outbuf_cnt++] = c;
}

void do_flush ()
{
    int i;
    for (i = 0; i < outbuf_cnt; i++) putchar (outbuf[i]);
    outbuf_cnt = 0;
}

void do_cancel ()
{
    outbuf_cnt = 0;
}
