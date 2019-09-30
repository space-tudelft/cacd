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

#include "src/nspice/define.h"
#include "src/nspice/type.h"
#include "src/nspice/extern.h"

char *argv0 = "nspice_bs";
char *Usg_msg = "\nUsage: %s cell\n\n";

char *cellname = NULL;

void backsubSpc (void);
void readCir (void);

int main (int argc, char **argv)
{
    char *s;

    while (--argc > 0) {
        ++argv;
	s = *argv;
        if (s[0] == '-' && s[1]) {
	    fprintf (stderr, "%s: illegal option: %c\n", argv0, s[1]);
	    goto usage;
	}
	else if (!cellname) cellname = *argv;
	else goto usage;
    }

    if (!cellname) {
usage:	fprintf (stderr, Usg_msg, argv0);
	exit (1);
    }

    Begin_signal = NULL;
    Begin_print = NULL;
    readCir ();

    backsubSpc ();

    return (0);
}

void backsubSpc ()
{
    char str1[512], str2[512];
    char buf[264];
    char *filename1, *filename2;
    FILE *fp1, *fp2;
    struct node_ref *tab;
    int c, i, found, match, nodenr;
    int errorFound, spice3;
    int hspice = 0;

    NEW (filename1, strlen (cellname) + 5, char);
    sprintf (filename1, "%s.ana", cellname);

    if (!(fp1 = fopen (filename1, "r"))) {
	message ("Cannot open %s", filename1, 0);
	exit (1);
    }

    /* look for error messages and print them if any */

    errorFound = 0;
    while ((c = getc (fp1)) != EOF) {
        ungetc (c, fp1);
	if (c != ' ' && c != '\t' && c != '\n'
	    && fscanf (fp1, "%s", str1) == 1
	    && (str1[0] == '*')
	    && (str1[1] == 'E' || str1[1] == 'e')
	    && (str1[2] == 'R' || str1[1] == 'r')
	    && (str1[3] == 'R' || str1[1] == 'r')
	    && (str1[4] == 'O' || str1[1] == 'o')
	    && (str1[5] == 'R' || str1[1] == 'r')
	    && (str1[6] == '*')) {

            errorFound = 1;

	    fprintf (stderr, "spice: %s", str1);

	    while ((c = getc (fp1)) != '\n') {
		fputc (c, stderr);
	    }
	    fputc ('\n', stderr);
	}
	else {
	    while ((c = getc (fp1)) != '\n');
	}
    }
    rewind (fp1);

    if (errorFound) exit (1);

    NEW (filename2, strlen (cellname) + 5, char);
    sprintf (filename2, "%s.axa", cellname);

    if (!(fp2 = fopen (filename2, "w"))) {
	message ("Cannot open %s", filename2, 0);
	exit (1);
    }

    spice3 = -1;
    found = 0;

    while (1) {
	while ((c = getc (fp1)) != EOF && !isalpha (c)) { putc (c, fp2); }
	if (c == EOF) break;
	ungetc (c, fp1);

	*str1 = '\0';

	match = 0;
	while (!match && fscanf (fp1, "%s", str2) == 1) {

	    fprintf (fp2, str2);

	    if (spice3 == -1) {
                if (strsame (str2, "Circuit:"))
                    spice3 = 1;     /* it is a spice3 ouput file */
                else
                    spice3 = 0;     /* it is a spice2 ouput file */
            }

            if (spice3 == 0 &&
                ((strsame (str1, "TRANSIENT") && strsame (str2, "ANALYSIS"))
	      || (strsame (str1, "transient") && strsame (str2, "analysis")))) {
                match = 1;
		/* read till end of line */
		while ((c = getc (fp1)) != EOF) {
		    putc (c, fp2);
		    if (c == '\n') break;
		}
		if ((c = getc (fp1)) == 'x') {
		    do {
			putc (c, fp2);
			if (c == '\n') break;
		    } while ((c = getc (fp1)) != EOF);
		    hspice = 1;
		}
		else ungetc (c, fp1);
	    }
            if (spice3 == 1 && strsame (str1, "Index") && strsame (str2, "time")) {
                match = 1;
            }

	    strcpy (str1, str2);

	    while ((c = getc (fp1)) != EOF && !isalpha (c)) { putc (c, fp2); }
	    if (c == EOF) break;
	    ungetc (c, fp1);
	}
	if (c == EOF) break;

        if (spice3 == 0) {
	    match = 0;
	    while (!match && fscanf (fp1, "%s", str1) == 1) {

		fprintf (fp2, str1);

		if (strsame (str1, "LEGEND:") || strsame (str1, "legend:")) match = 2;
		else
		if (strsame (str1, "TIME") || strsame (str1, "time")) match = 1;

		while ((c = getc (fp1)) != EOF && !isalpha (c)) { putc (c, fp2); }
		ungetc (c, fp1);
	    }
	    if (match == 2) continue; /* plot_tran */
        }
	if (c == EOF) break;

	if (hspice) {
	    while ((c = getc (fp1)) != EOF) {
		putc (c, fp2);
		if (c == '\n') break;
	    }
	}

	while ((c = getc (fp1)) == ' ' || c == '\t') putc (c, fp2);
	if (c == EOF) break;
	ungetc (c, fp1);

        found = 1;

	while (c != '\n') {

	    fscanf (fp1, "%s", buf);

	    tab = NULL;
	    if ((c = buf[0]) == 'V') c = 'v';
	    if ((c == 'v' && buf[1] == '(') || (hspice && isdigit(c))) {
		nodenr = 0;
		i = (c == 'v')? 2 : 0;
		for (; isdigit(buf[i]); ++i) nodenr = 10 * nodenr + (buf[i] - '0');
		if ((c == 'v' && buf[i] == ')') || (c != 'v' && !buf[i])) {
		    tab = Begin_table;
		    while (tab && tab -> nodenr != nodenr) tab = tab -> next;
		}
	    }
	    if (tab)
		fprintf (fp2, "%s", tab -> name);
	    else
		fprintf (fp2, buf);

	    while ((c = getc (fp1)) == ' ' || c == '\t') { putc (c, fp2); }
	    ungetc (c, fp1);
	}
    }

    if (!found)  {
	message ("Cannot find transient analysis in %s", filename1, 0);
	exit (1);
    }

    fclose (fp1);
    fclose (fp2);

#ifdef WIN32
    unlink (filename1);
#endif
    rename (filename2, filename1);
}
