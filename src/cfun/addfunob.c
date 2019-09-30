/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
 *	O. Hol
 *	P.E. Menchen
 *	S. de Graaf
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

#include "src/cfun/func_parsedefs.h"

void print_av (char **av)
{
    fprintf (stderr, "%s", *av++);
    while (*av) fprintf (stderr, " %s", *av++);
    fprintf (stderr, "\n");
}

/* Function addfun_obj()
** first compiles the created C-code and then places
** the object file, with the name 'sls.o' in the database.
*/
void addfun_obj (char fistr[], int k)
{
    char newpath[DM_MAXLINE];
    int i = 0;

    fistr[k] = 'c';
#ifdef __GNUC__
    C_options[i++] = "gcc";
#else
    C_options[i++] = "cc";
#endif
    while (C_options[i]) ++i;
    C_options[i++] = "-c";
    C_options[i++] = fistr;
    C_options[i] = NULL;

    if (verbose) print_av (C_options);
    i = _dmRun2 (C_options[0], C_options);
    if (i) die (3, fistr, ""); /* no successful compilation */

    if (!kflag) rmexec (fistr);

    fistr[k] = 'o';
    sprintf (newpath, "%s/%s/%s/sls.o", key -> dmproject -> dmpath, key -> view, key -> cell);

    if (verbose) fprintf (stderr, "mv %s %s\n", fistr, newpath);
    rename (fistr, newpath);
}

/* Function cppexec()
** executes the C preprocessor to get out the defines
*/
void cppexec (char cppin[], char cppout[])
{
    struct stat statBuf;
    char *cpp;
    int i = 0;

    cpp = "/usr/lib/cpp";
    if (stat (cpp, &statBuf) == 0) goto doit;
    cpp = "/lib/cpp";
    if (stat (cpp, &statBuf) == 0) goto doit;
    cpp = "cpp";
doit:
    P_options[i++] = cpp;
    while (P_options[i]) ++i;
    P_options[i++] = cppin;
    P_options[i++] = cppout;
    P_options[i] = NULL;

    if (verbose) print_av (P_options);
    i = _dmRun2 (P_options[0], P_options);
}

/* Function rmexec() unlinks given arg
*/
void rmexec (char arg[])
{
    struct stat statbuf;

    if (stat (arg, &statbuf) == 0) {
	/*
	if (verbose) fprintf (stderr, "unlink %s\n", arg);
	*/
	unlink (arg);
    }
}
