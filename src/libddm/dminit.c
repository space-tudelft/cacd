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
#include "src/libddm/dmglobal.h"

int dmInit_flag = 0;

int dmInit (char *progname)
{
    char linebuf[DM_MAXPATHLEN];
    char *name, *path;

    if (dmInit_flag) {
#if 0
	dmerrno = DME_INIT;
	dmError ("dmInit");
	return (-1);
#endif
	return 0; /* Already initialized. */
    }

    name = "Please set your environment ICDPATH correctly.";

    if (!(path = getenv ("ICDPATH"))) {
	fprintf (stderr, "Cannot get environment ICDPATH.\n%s\n", name);
	exit (1);
    }

    if (!getcwd (linebuf, DM_MAXPATHLEN)) {
	fprintf (stderr, "Cannot get cwd correctly.\n");
	exit (1);
    }
    if (chdir (path)) {
	fprintf (stderr, "Cannot chdir to ICDPATH `%s'.\n%s\n", path, name);
	exit (1);
    }
    if (!getcwd (icdpath, DM_MAXPATHLEN)) { /* get absolute path */
	fprintf (stderr, "Cannot get the ICDPATH.\n%s\n", name);
	exit (1);
    }
    if (*icdpath != '/') {
	fprintf (stderr, "No absolute ICDPATH.\n%s\n", name);
	exit (1);
    }
    if (chdir (linebuf)) {
	fprintf (stderr, "Cannot chdir to cwd `%s'.\n", linebuf);
	exit (1);
    }

    dmInit_flag = 1;

    return (0);
}

/* Function below is called by _dmFatal and can be called by the program
 * to check in all outstanding keys, to prevent locks remaining set.
 */
int dmQuit ()
{
    if (dmInit_flag) {
	dmInit_flag = 0;
	_dmClose_allproj (QUIT);
    }
    return (0);
}
