/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#ifdef DRIVER
#define DEBUG
#endif

#include "src/space/auxil/auxil.h"

/* local operations */
static void   setup (FILE *dfp);
static bool_t check (char *file, int line);

/*
 * The initial value of _IfDebug should be 1 so that
 * cIfDebug is called at least once
 */
bool_t _IfDebug = TRUE;

static char *namelist[BUFSIZ];

/*
 * DESCRIPTION
 *
 * cIfDebug - debugging.
 *
 * The file :debug in the current working directory (or the
 * file named in the environment variable DEBUG) can contain
 * the names of the .c source files
 * on a separate line.
 * For each file named in the :debug file, the debugging mode
 * is turned on.
 *
 * Suppose the "foo.c" is in the :debug file. Then, the call
 *
 *   cIfDebug ("foo.c", 100)
 *
 * will print (without a newline, but with a tab)
 *
 *   ---foo.c, 100:\t
 *
 * and returns TRUE.
 *
 * Typically, the call to cIfDebug is in an if-statement,
 * and by means of the macro DEBUG to fill in the name
 * of the current source file and the current line number
 * automatically.
 * Example:
 *
 * #define DEBUG _IfDebug && cIfDebug (__FILE__, __LINE__)
 * ...
 * #ifdef DEBUG
 *     if (DEBUG) {
 *         print (some diagnostics);
 *     }
 * #endif
 *
 * The :debug file is read only once, and if it can't be
 * opened the overhead is very low.
 *
 * Only the first BUFSIZ files mentioned in the :debug file
 * are honored.
 */
bool_t cIfDebug (char *file, int line)
{
    static int init = 1;
    FILE *dfp;
    char *p;

    /* strip everything up to the last '/' character */
    if ((p = strrchr (file, '/'))) file = p+1;

    if (init) {
	init = 0;
	if (getenv ("DEBUG"))
	    dfp = fopen (getenv ("DEBUG"), "r");
	else
	    dfp = fopen (":debug", "r");
	if (dfp) {
	    setup (dfp);
	    fclose (dfp);
	}
	else {
	    _IfDebug = FALSE; /* no debugging, set global var */
	}
    }

    if (_IfDebug)
	return (check (file, line));
    else
	return (FALSE);
}

static void setup (FILE *dfp)
{
    char buf[BUFSIZ];
    int  i = 0;

    while (fgets (buf, BUFSIZ, dfp)) {
	buf[strlen (buf) - 1] = '\0';
	if (i < (sizeof (namelist) / sizeof (*namelist) - 2)) {
	    namelist[i++] = strsave (buf);
	}
	else {
	    say ("Ignoring debugging of file %s", buf);
	}
    }
    namelist[i] = NULL;
}

static bool_t check (char *file, int line)
{
    int i;

    for (i = 0; namelist[i]; i++) {
	if (strsame (namelist[i], file)) {
	    fprintf (stderr, "# %s%4ld:\t", file, (long) line);
	    return (TRUE);
	}
    }
    return (FALSE);
}

/*
 * cSetDebug can be called from an interactive debugger
 * or from a program to toggle debugging.
 */
void cSetDebug (char *file)
{
    int i, j;

    _IfDebug = TRUE;

    for (i = 0; namelist[i]; i++) {
	if (strsame (namelist[i], file)) {
	    /* turn of debugging:
	     * move last name in namelist in this position
	     */
	    for (j = i + 1; namelist[j]; j++);
	    if (j != i + 1) j--;
	    namelist[i] = namelist[j];
	    namelist[j] = NULL;
	    return;
	}
    }
    namelist[i] = strsave (file);
    namelist[++i] = NULL;
}

#ifdef DRIVER
int main () /* test driver */
{
    if (DEBUG) fprintf (stderr, "blah.\n");
    return (0);
}
#endif
