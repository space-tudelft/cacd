/*
 * ISC License
 *
 * Copyright (C) 1990 by
 *	Arjan van Genderen
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
 * The program 'delay' determines the delay time from
 * a spice output
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *argv0 = "delay";

FILE *fp_out;
FILE *fp_in;

char *fn_in = NULL;
char str1[512];
char str2[512];

void die (void);

int main (int argc, char *argv[])
{
    char *s, buf[512];
    double t, tprev, t1, t2;
    double v1, v1prev, v2, v2prev;
    int match, end;
    char c;

    fp_out = stdout;

    while (--argc > 0) {
        if ( (*++argv)[0] == '-' ) {
	    for (s = *argv + 1; *s != '\0'; s++) {
		fprintf (stderr, "%s: illegal option: %c\n", argv0, *s);
		die ();
	    }
	}
	else {
            if (fn_in == NULL) {
		fn_in = *argv;
	    }
	}
    }

    if (fn_in == NULL) {
	fprintf (stderr, "\nUsage: %s [options] infile\n\n", argv0);
	die ();
    }

    if ((fp_in = fopen (fn_in, "r")) == NULL) {
        fprintf (stderr, "Cannot open %s\n", fn_in);
        die ();
    }

    *str1 = 0;
    match = 0;
    while (!match && fscanf (fp_in, "%s", str2) == 1) {

        if (strcmp (str1, "Transient") == 0
            && strcmp (str2, "Analysis") == 0) {

	    while ((c = getc (fp_in)) != '\n' && c != EOF);
	    while ((c = getc (fp_in)) != '\n' && c != EOF);
	    if (fscanf (fp_in, "%s", buf) == 1
		&& strcmp (buf, "Index") == 0) {
		while ((c = getc (fp_in)) != '\n' && c != EOF);
		while ((c = getc (fp_in)) != '\n' && c != EOF);
		match = 1;
	    }
        }

        strcpy (str1, str2);
    }

    if (match) {
	tprev = 0;
	v1prev = 0;
	v2prev = 0;

	t1 = -1;
	t2 = -1;

	end = 0;
	while (!end && fscanf (fp_in, "%*d %le %le %le", &t, &v1, &v2) > 0) {

	    if (v1prev <= 2.5 && v1 >= 2.5) {
		t1 = tprev + ((2.5 - v1prev) / (v1 - v1prev)) * (t - tprev);
	    }

	    if (v2prev <= 2.5 && v2 >= 2.5) {
		t2 = tprev + ((2.5 - v2prev) / (v2 - v2prev)) * (t - tprev);
	    }

	    tprev = t;
	    v1prev = v1;
	    v2prev = v2;

	    while ((c = getc (fp_in)) != '\n' && c != EOF);
	    while ((c = getc (fp_in)) == ' ' || c == '\t');
	    if (c < '0' || c > '9') {
		while ((c = getc (fp_in)) != '\n' && c != EOF);
		if (fscanf (fp_in, "%s", buf) == 1
		    && strcmp (buf, "Index") == 0) {
		    /* new page */
		    while ((c = getc (fp_in)) != '\n' && c != EOF);
		    while ((c = getc (fp_in)) != '\n' && c != EOF);
		}
		else
		    end = 1;
	    }
	    else
		ungetc (c, fp_in);
	}

	if (t2 > t1 && t1 > 0 && t2 > 0) {
	    fprintf (fp_out, "%.5f\n", (t2 - t1) * 1e9);  /* in nsec */
	}
	else {
	    fprintf (fp_out, "-999\n");
	}
    }

    return (0);
}

void die ()
{
    exit (1);
}
