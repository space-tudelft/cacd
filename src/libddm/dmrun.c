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

#include <stdarg.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "src/libddm/dmstd.h"

/*
** This function runs a subprogram with the given
** arguments.  These arguments must be of type "char *",
** the last must be NULL.
** See the example in the test driver.
** No system shells are started.
*/

int _dmRun2 (char* path, char* argv[])
{
    int status = 0;

#ifdef DEBUG
    if (DEBUG) {
        int i;
	fprintf (stderr, "RUNNING [%s]\n", path);
	for (i = 0; argv[i]; ++i) fprintf (stderr, "ARGV[%d]=[%s]\n", i, argv[i]);
    }
#endif /* DEBUG */

    switch (vfork ()) {
	default: /* PARENT */
	    wait(&status);
	    break;
	case 0: /* CHILD */
	    execvp (path, argv);
	    fprintf (stderr, "_dmRun: ");
	    perror (path);
	    _exit (1);
	    break;
	case -1: /* ERROR: can't fork */
	    perror ("_dmRun");
	    exit (1);
	    break;
    }
    return (status);
}

int _dmRun (char *path, ...)
{
    va_list ap;
    int     argno = 0;
    char   *argv[64];

    va_start (ap, path);

    argv[argno++] = path;
    while ((argv[argno++] = va_arg (ap, char *))) {
	if (argno == 64) _dmFatal ("_dmRun", "too many arguments", NULL);
    }

    va_end (ap);

    return _dmRun2 (path, argv);
}

#ifdef DRIVER
int main (int argc, char *argv[])
{
    if (argc != 2) {
	fprintf (stderr, "usage: %s progpath\n", argv[0]);
	return 1;
    }
    return _dmRun (argv[1], "a", "b", "c", "d", NULL);
}
#endif /* DRIVER */
