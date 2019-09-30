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

struct node *Begin_signal = NULL;
struct node_ref *Begin_print = NULL;
struct node_ref *Begin_table = NULL;
struct node_ref *End_table = NULL;

void readCir ()
{
    char str[512];
    int c, match, nr;
    char *filename;
    FILE *fp;
    struct node *sig;
    struct node_ref *pr, *tab;
    int vnbulk = 0;
    int vpbulk = 0;
    int nbulk = 0;
    int pbulk = 0;
    int error = 0;

    NEW (filename, strlen (cellname) + 5, char);
    sprintf (filename, "%s.spc", cellname);

    if (!(fp = fopen (filename, "r"))) {
	message ("Cannot open %s", filename, 0);
	exit (1);
    }

    match = 0;
    c = 'a';
    while (c != EOF && !match) {
	if ((c = getc (fp)) == '*') {
	    while ((c = getc (fp)) == ' ' || c == '\t');
	    if (c != EOF && c != '\n') {
		ungetc (c, fp);
		if (fscanf (fp, "%s", str) == 1 && strsame ("namelist", str)) {
		    while ((c = getc (fp)) == ' ' || c == '\t');
		    if (c != EOF && c != '\n') {
			ungetc (c, fp);
			if (fscanf (fp, "%s", str) == 1 && strsame (cellname, str)) {
			    match = 1;
			}
		    }
		}
	    }
	}
	if (c != '\n')
	    while (c != EOF && (c = getc (fp)) != '\n');
    }

    Begin_table = NULL;

    if (match) { /* Read node names */

	while ((c = getc (fp)) != '\n' && c != EOF);

	while (fscanf (fp, "%s", str) == 1 && str[0] == '*') {

	    while ((c = getc (fp)) == ' ' || c == '\t');

	    if (c != EOF && c != '\n')
		ungetc (c, fp);
	    else
		break; /* empty comment line means end of namelist */

	    while (fscanf (fp, "%d %s", &nr, str) == 2) {

		if (strsame (str, "nbulk") || strsame (str, "NBULK")) nbulk = 1;
		else
		if (strsame (str, "pbulk") || strsame (str, "PBULK")) pbulk = 1;

		if (!Begin_table) {
		    NEW (Begin_table, 1, struct node_ref);
		    End_table = Begin_table;
		}
		else {
		    NEW (End_table -> next, 1, struct node_ref);
		    End_table = End_table -> next;
		}

		NEW (End_table -> name, strlen (str) + 1, char);
		strcpy (End_table -> name, str);
		End_table -> nodenr = nr;
		End_table -> next = NULL;

		while ((c = getc (fp)) == ' ' || c == '\t');
		if (c == '\n') break;
		ungetc (c, fp);
	    }
	}
    }

    if (!Begin_table) {
	message ("Cannot find namelist in %s", filename, 0);
	exit (1);
    }

    while ((c = getc (fp)) != '\n' && c != EOF);
    while (c != EOF && fscanf (fp, "%s", str) == 1) {
	if (strsame (str, "vnbulk") || strsame (str, "VNBULK")) vnbulk = 1;
	if (strsame (str, "vpbulk") || strsame (str, "VPBULK")) vpbulk = 1;
	while ((c = getc (fp)) != '\n' && c != EOF);
    }

    if (nbulk && !vnbulk) {
	message ("Missing 'vnbulk' line, found 'nbulk' in the net-namelist!", 0, 0);
	error = 1;
    }
    if (!nbulk && vnbulk) {
	message ("Missing 'nbulk' in net-namelist, found 'vnbulk' line!", 0, 0);
	error = 1;
    }
    if (pbulk && !vpbulk) {
	message ("Missing 'vpbulk' line, found 'pbulk' in the net-namelist!", 0, 0);
	error = 1;
    }
    if (!pbulk && vpbulk) {
	message ("Missing 'pbulk' in net-namelist, found 'vpbulk' line!", 0, 0);
	error = 1;
    }

    if (error) exit (1);

    fclose (fp);

    for (sig = Begin_signal; sig; sig = sig -> next) {

	tab = Begin_table;
	while (tab && !strsame (sig -> name, tab -> name)) tab = tab -> next;

	if (!tab) {
	    message ("Cannot find node %s in %s", sig -> name, filename);
	    exit (1);
	}
	sig -> nodenr = tab -> nodenr;
    }

    for (pr = Begin_print; pr; pr = pr -> next) {

	tab = Begin_table;
	while (tab && !strsame (pr -> name, tab -> name)) tab = tab -> next;

	if (!tab) {
	    message ("Cannot find node %s in %s", pr -> name, filename);
	    exit (1);
	}
	pr -> nodenr = tab -> nodenr;
    }
}

void message (char *s, char *a1, char *a2)
{
    fprintf (stderr, "%s: ", argv0);
    fprintf (stderr, s, a1, a2);
    fprintf (stderr, "\n");
}
