/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van der Hoeven
 *	P. van der Wolf
 *	P. Bingley
 *	T.G.R. van Leuken
 *	T. Vogel
 *	F. Beeftink
 *	M. Grueter
 *	E.F. Matthijssen
 *	G.W. Sloof
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

#include "src/libddm/dmstd.h"

static int init_debug(FILE *dfp);
static int check(char *file, int line);

static char *namelist[64];

int _dmIfdebug (char *file, int line)
{
    static int init = 2;
    static FILE *dfp = NULL;

    if (init == 0)
	return (0);
    else if (init == 1)
	return (check (file, line));

    if (getenv ("DM_DEBUG") && (dfp = fopen (getenv ("DM_DEBUG"), "r"))) {
	init = 1;
	init_debug (dfp);
	return (check (file, line));
    }
    init = 0;
    return (0);
}

static int init_debug (FILE *dfp)
{
    register int i = 0;
    char buf[MAXLINE];

    while (fgets (buf, MAXLINE, dfp)) {
	buf[strlen (buf) - 1] = '\0';
	namelist[i++] = _dmStrSave (buf);
	if (i == 64) _dmFatal ("_dmIfdebug", "too many names", NULL);
    }
    namelist[i] = NULL;
    return 0;
}

static int check (char *file, int line)
{
    register int i;

    for (i = 0; namelist[i] != NULL; i++) {
	if (strcmp (namelist[i], file) == 0) {
	    fprintf (stderr, "---%s, %d:\n", file, line);
	    return (1);
	}
    }
    return (0);
}

#ifdef DRIVER
int main ()
{
    int i = 3;
    while (i-- > 0) {
	IFDEBUG fprintf (stderr, "i: %d\n", i);
    }
    return (0);
}
#endif /* DRIVER */
