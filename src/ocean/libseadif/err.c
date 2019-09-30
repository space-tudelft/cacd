/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
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
#include <stdio.h>
#include <stdlib.h>
#include "src/ocean/libseadif/syssig.h"
#include "src/ocean/libseadif/libstruct.h"
#include "src/ocean/libseadif/sealibio.h"
#include "src/ocean/libseadif/sea_decl.h"
#include <unistd.h>

#ifndef _NFILE
#define _NFILE 20 /* Some stdio.h's do not define this */
#endif

extern SDFFILEINFO sdffileinfo[]; /* 0...MAXFILES */

void err (int errcode, char *errstring)
{
    fprintf (stderr, "\n%s\n", errstring);
    sdfexit (errcode);
}

/* Try to produce a core image on disk and exit. */
void dumpcore ()
{
    int j;

    /* Flush and close all buffers except stderr so we can print if the core dump fails.
    */
    fprintf (stderr, "%s",
	"\n\n"
	"Due to a fatal programming error, this program is about to crash.\n"
	"Please report this bug (e-mail to space-support-ewi@tudelft.nl).\n"
	"\n"
	"Thanks for your cooperation! Then now the time has come to write\n"
	"a core image into the current working directory ...\n"
	"\n"
	"busy dumping core...\n\n");

    /* remove all the lock files */
    for (j = 1; j <= MAXFILES; ++j)
	if (sdffileinfo[j].lockname) unlink (sdffileinfo[j].lockname);

    /* remove the scratch file */
    unlink (sdffileinfo[0].name);

#ifdef SIGQUIT
    if (kill (getpid(), SIGQUIT) == -1) /* try to kill yourself with a signal that produces a core file */
	fprintf (stderr, "\tSorry, it did not work out that way.\n\tPlease call the OCEAN customers support.\n\n");
#endif

    exit (5); /* only executed if core dump fails */
}

/* Remove all the seadif lock files, then exit. */
void sdfexit (int code)
{
    int j;

    for (j = 1; j <= MAXFILES; ++j)
	if (sdffileinfo[j].lockname) unlink (sdffileinfo[j].lockname);

    /* remove the scratch file */
    unlink (sdffileinfo[0].name);

    exit (code);
}
