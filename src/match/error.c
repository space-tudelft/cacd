static char *rcsid = "$Id: error.c,v 1.1 2018/04/30 12:17:28 simon Exp $";
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	T. Vogel
 *	A.J. van Genderen
 *	S. de Graaf
 *	A.J. van der Hoeven
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
/*
 * Contains functions which perform the basic error handling.
 */
#include <stdarg.h>
#include "src/match/head.h"
#include "src/match/proto.h"

Import boolean S_opt, V_opt;

/* Prints a message on the stderr.
 * The message cannot be surpressed by option flags.
 * The execution of the program is stopped.
 */
Public void err_mesg (string arg0, ...)
{
    va_list args;

    va_start (args, arg0);

    vfprintf (stderr, arg0, args);

    va_end (args);

    my_exit (1);
}

/* Prints a 'verbose' message on the stderr.
 * The message is only printed if the 'verbose'
 * option is set and the 'silent' option is not set.
 */
Public void verb_mesg (string arg0, ...)
{
    va_list args;

    va_start (args, arg0);

    if (V_opt) vfprintf (stderr, arg0, args);

    va_end (args);
}

/* Prints a 'user' message on the stdout.
 * The message is not printed if 'silent' option is set.
 */
Public void user_mesg (string arg0, ...)
{
    va_list args;

    va_start (args, arg0);

    if (!S_opt) {
	vfprintf (stdout, arg0, args);
	fflush (stdout);
    }

    va_end (args);
}

/* Prints a message on the stdout.
 * This message can not be surpressed by option flags.
 */
Public void prnt_mesg (string arg0, ...)
{
    va_list args;

    va_start (args, arg0);

    vfprintf (stdout, arg0, args);
    fflush (stdout);

    va_end (args);
}

Private string myFile (string file, int *i)
{
    string s;
    file += 5;
	 if ((s = strchr (file, ','))) *i = s - file;
    else if ((s = strchr (file, ' '))) *i = s - file;
    else { file = "??"; *i = 2; }
    return (file);
}

/* Prints a predefined error message on stderr.
 */
Public void p_error (int n, string file, int line)
{
    int i; file = myFile (file, &i);
    fprintf (stderr, "Fatal error in file '%.*s', line %d: ", i, file, line);
    switch (n) {
	case 0:
	    fprintf (stderr, "Not Yet Implemented.\n");
	    break;
	case 1:
	    fprintf (stderr, "Unexpected result.\n");
	    break;
	case 2:
	    fprintf (stderr, "Cannot open file.\n");
	    break;
	case 3:
	    fprintf (stderr, "Cannot access file.\n");
	    break;
	case 4:
	    fprintf (stderr, "Recursive network description.\n");
	    break;
	case 5:
	    fprintf (stderr, "Out of memory.\n");
	    break;
	case 6:
	    fprintf (stderr, "Stack too small.\n");
	    break;
	default:
	    fprintf (stderr, "Unknown error code %d\n", n);
	    break;
    }
    my_exit (1);
}

Public void assert_error (string file, int line, string cond)
{
    int i; file = myFile (file, &i);
    fprintf (stderr, "Assert failed in file '%.*s', line %d: '%s'\n", i, file, line, cond);
    my_exit (1);
}
