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

#include "src/makeboxh/extern.h"

char *err_list[] = {
/* 0 */    "%s",
/* 1 */    "no cell specified",
/* 2 */    "incorrect number of arguments",
/* 3 */    "incorrect window specified",
/* 4 */    "cannot create file: %s",
/* 5 */    "too many arguments specified",
/* 6 */    "sorry: cannot allocate core",
/* 7 */    "error: unknown mask name: %s",
/* 8 */    "error: check level must be >= 1",
/* 9 */    "interrupted due to signal: %s",
/* 10 */   "error in ddm interface function",
/* 11 */   "error: but cannot find error message"
};

void errexit (int err_no, char *cs)
{
    int i;

    i = (err_no < 0) ? -err_no : err_no;
    if (i > 11) i = 11;

    P_E "%s: ", argv0);
    P_E err_list[i], cs);
    P_E "\n");

    if (err_no >= 0) die ();
}

void dmError (char *s)
{
    P_E "%s: ", argv0);
    dmPerror (s);
    errexit (10, "");
}

void die ()
{
    P_E "%s: -- program aborted --\n", argv0);
    if (project) dmCloseProject (project, COMPLETE);
    dmQuit ();
    exit (1);
}
