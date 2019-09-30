/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

#include "src/makeboxl/extern.h"

char *err_list[] = {
/* 0 */    "%s",
/* 1 */    "no cell specified",
/* 2 */    "incorrect number of arguments",
/* 3 */    "incorrect window specified",
/* 4 */    "cannot create file: %s",
/* 5 */    "too many arguments specified",
/* 6 */    "sorry: cannot allocate core",
/* 7 */    "error: unknown mask name: %s",
/* 8 */    "error: illegal check level",
/* 9 */    "interrupted due to signal: %s",
/* 10 */   "error in ddm interface function",
/* 11 */   "error: illegal number of samples",
/* 12 */   "error: illegal mask number %s in annotations stream",
/* 13 */   "error: recursion to cell \"%s\"",
/* 14 */   "error: too long hier. name",
/* 15 */   "error: cannot find imported cell \"%s\"",
/* 16 */   "error: cannot stat \"%s\"",
/* 17 */   "error: illegal \"%s\" image transf. matrix!",
/* 18 */   "error: but cannot find error message"
};

void errexit (int err_no, char *s)
{
    int i = (err_no < 0) ? -err_no : err_no;
    if (i > 18) i = 18;

    P_E "%s: ", argv0);
    P_E err_list[i], s);
    P_E "\n");

    if (err_no >= 0) die ();
}

void dmError (char *s)
{
    if (!noErrMes) {
	P_E "%s: ", argv0);
	dmPerror (s);
	errexit (10, "");
    }
}

void die ()
{
    extern char * current_cell;
    extern DM_PROJECT * project;

    if (project) dmCloseProject (project, QUIT);
    dmQuit ();

    if (current_cell) {
	P_E "cleaning up %s (running dbclean)\n", current_cell);
	_dmRun ("dbclean", "-l", current_cell, (char *) NULL);
    }
    P_E "%s: -- program aborted --\n", argv0);
    exit (1);
}
