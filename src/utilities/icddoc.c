/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "src/libddm/dmincl.h"

char   *argv0 = "icddoc";	/* Program Name */
char   *use_msg =		/* Command Line */
        "\nUsage: %s [document]\n\n";

char    command[256];

int main (int argc, char *argv[])
{
    FILE *fp;
    char *docfile;
    int i;

    if (argc != 2) {
	fprintf (stderr, use_msg, argv0);
	if (argc != 1) return (1);
    }

    dmInit (argv0);
    dmQuit ();

    sprintf (command, "%s/share/doc", icdpath);
    if (chdir (command) == -1) {
	fprintf (stderr, "%s: cannot chdir to: %s\n", argv0, command);
	return (1);
    }

    if (argc == 1) {
	sprintf (command, "ls *.pdf; ls */*");
	printf ("Following documents are available:\n");
	printf ("==================================\n");
	if (system (command)) return (1);
	return (0);
    }

    docfile = argv[1];
    if (!(fp = fopen (docfile, "r"))) {
	fprintf (stderr, "\n%s: cannot open file: %s\n\n", argv0, docfile);
	return (1);
    }
    fclose (fp);

    i = strlen (docfile);
    if (i > 3 && strcmp (docfile + i-4, ".pdf") == 0)
	sprintf (command, "evince -s %s", docfile);
    else
	sprintf (command, "more -s %s", docfile);
    if (system (command)) return (1);
    return (0);
}
