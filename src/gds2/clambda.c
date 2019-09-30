/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
 *	R. Paulussen
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
 * PROCESS DESCRIPTION:
 *	This program changes the old value of lambda
 *	into the specified value.
 */

#include <stdlib.h>
#include <stdio.h>

#define PE fprintf(stderr,
#define BL 20

char  buf1[BL];
char  buf2[BL*100];
char  buf3[BL];
char  buf4[BL];

char *argv0 = "clambda";	/* Program Name */

int main (int argc, char *argv[])
{
    double  lambda = 0;
    FILE   *fp_dmrc;

    if (argc > 2) {
	PE "\nUsage: %s [lambda]\n\n", argv0);
	exit (1);
    }
    if (argc != 2) goto read_step;

    lambda = atof (argv[1]);
    if (lambda <= 0) {
	PE "%s: new value of lambda incorrect (<= 0)\n", argv0);
	exit (1);
    }

read_step:
    if ((fp_dmrc = fopen (".dmrc", "r")) == NULL) {
	PE "%s: current directory is no project\n", argv0);
	exit (1);
    }

    if (!fgets (buf1, BL, fp_dmrc)) goto read_err;
    if (!fgets (buf2, BL*100, fp_dmrc)) goto read_err;
    if (!fgets (buf3, BL, fp_dmrc)) goto read_err;
    if (!fgets (buf4, BL, fp_dmrc)) goto read_err;
    if (getc (fp_dmrc) != EOF) {
	PE "%s: more than four lines in file .dmrc\n", argv0);
	exit (1);
    }
    fclose (fp_dmrc);

    if (argc != 2) {
	printf ("%s: current value of lambda: %s", argv0, buf3);
	exit (0);
    }

    if ((fp_dmrc = fopen (".dmrc", "w")) == NULL) {
	PE "%s: cannot overwrite file .dmrc\n", argv0);
	exit (1);
    }

    printf ("%s: old value of lambda: %s", argv0, buf3);
    printf ("%s: new value of lambda: %.8g\n", argv0, lambda);

    fputs (buf1, fp_dmrc);
    fputs (buf2, fp_dmrc);
    fprintf (fp_dmrc, "%.8g\n", lambda);
    fputs (buf4, fp_dmrc);
    fclose (fp_dmrc);

    exit (0);

read_err:
    PE "%s: read error in file .dmrc\n", argv0);
    exit (1);
    return (1);
}
