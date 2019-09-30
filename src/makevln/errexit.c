/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#include "src/makevln/incl.h"

extern char tmpvln[];
extern char tmpteq[];
extern char *argv0;
extern int  flag_v;
extern DM_PROJECT *project;

char *errlist[] = {
/* 0 */  "%s",
/* 1 */  "cannot open file: %s",
/* 2 */  "cannot create file: %s",
/* 3 */  "cannot allocate enough core",
/* 4 */  "interrupted due to signal: %s",
/* 5 */  "error in ddm interface function",
/* 6 */  "unknown error"
};

void errexit (int err_no, char *s)
{
    int i = (err_no < 0) ? -err_no : err_no;
    if (i > 6) i = 6;

    fprintf (stderr, "%s: ", argv0);
    fprintf (stderr, errlist[i], s);
    fprintf (stderr, "\n");

    if (err_no >= 0) die (1);
}

void dmError (char *s)
{
    fprintf (stderr, "%s: ", argv0);
    dmPerror (s);
    errexit (5, "");
}

void die (int status)
{
    unlink (tmpvln);
    unlink (tmpteq);

    if (project) {
	if (status) {
	    dmCloseProject (project, QUIT);
	}
	else {
	    dmCloseProject (project, COMPLETE);
	}
    }
    dmQuit ();

    if (status) {
	fprintf (stderr, "%s: -- program aborted --\n", argv0);
    }
    else if (flag_v) {
	fprintf (stderr, "%s: -- program finished --\n", argv0);
    }
    exit (status);
}
