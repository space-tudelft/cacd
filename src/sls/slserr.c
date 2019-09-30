/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
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

#include "src/sls/extern.h"

#define  MAXERRORS  20
int nbr_errs = 0;

void slserror (char *fn, int lineno, int errtype, char *str1, char *str2)
{
    if (fn == NULL)
	fprintf (stderr, "%s:", argv0);
    else {
	if (errtype == ERROR1)
	    fprintf (stderr, "error in \"%s\"", fn);
	else
	    fprintf (stderr, "\"%s\"", fn);
        if (lineno > 0)
            fprintf (stderr, ", line %d:", lineno);
        else
            fprintf (stderr, ":");
    }

    if (errtype == WARNING) fprintf (stderr, " (warning)");
    if (str1) fprintf (stderr, " %s", str1);
    if (str2) fprintf (stderr, " %s", str2);
    fprintf (stderr, "\n");

    if (errtype == ERROR1) die (1);
    if (errtype == ERROR2) {
	fatalerror = TRUE;
	if (++nbr_errs > MAXERRORS) die (1);
    }
}
