/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Patrick Groeneveld
 *	Paul Stravers
 *	Simon de Graaf
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
 * Print error message and quit.
 */

#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/prototypes.h"

extern int Nelsis_open, Seadif_open;

void error (int errortype, char *string)
{
    fprintf (stderr, "\n");

    switch (errortype) {
    case ERROR:
	fprintf (stderr, "ERROR (non-fatal): %s\n", string);
	return;
    case WARNING:
	fprintf (stderr, "WARNING: %s\n", string);
	return;
    case FATAL_ERROR:
    default:
	fprintf (stderr, "ERROR %d (fatal): %s\n", errortype, string);
    }

    fflush (stdout);
    fflush (stderr);

    /* close the databases */
    if (Nelsis_open) close_nelsis ();

    /* close seadif without writing */
    if (Seadif_open) sdfexit (errortype);

    exit (errortype);
}

/* Exit in a nice way and leave remove lockfiles.
 */
void myfatal (int errortype)
{
    fflush (stdout);
    fflush (stderr);

    /* close the databases
     */
    if (Nelsis_open) close_nelsis ();

    /* close seadif without writing
     */
    if (Seadif_open) sdfexit (errortype);

    exit (errortype);
}
