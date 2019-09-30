/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	A.J. van Genderen
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

#include "src/csls/sys_incl.h"
#include "src/csls/class.h"
#include "src/csls/mkdbdefs.h"
#include "src/csls/mkdbincl.h"

extern char fn_incl[];
int sls_errcnt;
int sls_errno;
int sls_nerr = 24;

const char *sls_errlist[] = {
    0,
    "errors in network definition",
    "actual terminals < formal terminals",
    "actual terminals > formal terminals",
    "destination net < source net",
    "destination net > source net",
    "syntax error",
    "illegal range",
    "terminal already defined",
    "unknown type",
    "illegal network",
    "instance already defined",
    "unknown network",
    "undefined function",
    "network already referenced, defined or declared",
    "unknown terminal",
    "unknown device",
    "failed to make or use equivalent circuit",
    "unknown terminal",
    "inconsistent range for terminal",
    "not specified as a network or extern network",
    "sorry, network names starting with a capital are not allowed",
    "instance name already used as net name",
    "net name already used as instance name",
    0
};

void sls_perror (char *s)
{
    if (s && *s != '\0')
	fprintf (stderr, "%s: ", s);

    if (sls_errno > 0 && sls_errno <= sls_nerr)
	fprintf (stderr, "%s", sls_errlist[sls_errno]);
    else
	fprintf (stderr, "SLS_MKDB error number: %d", sls_errno);
}

void sls_error (int lineno, int e_no, char * e_str)
{
    char linenostr[16];

    if (fn_incl[0] != '\0')
        fprintf(stderr,"\"%s\", ", fn_incl);

    if(lineno)
        sprintf(linenostr,"line %d", lineno);

    sls_errno = e_no;

    sls_perror(linenostr);

    if(e_str && *e_str)
        fprintf(stderr,"; %s", e_str);

    fprintf(stderr,"\n");
}
