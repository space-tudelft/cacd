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
#include <errno.h>

void dmPerror (char *s)
{
    if (s) fprintf (stderr, "%s: ", s);

    if (dmerrno > 0 && dmerrno < dmnerr)
	fprintf (stderr, "%s", dmerrlist[dmerrno]);
    else
	fprintf (stderr, "DMI error number: %d", dmerrno);

    if (dmerrno == DME_SYS) {
	if (errno > 0) fprintf (stderr, ": %s", strerror (errno));
    }
    if (dmerrno == DME_PRDATA) {
	fprintf (stderr, "\n   Check the value of the variable 'ICDPATH' and/or");
	fprintf (stderr, "\n   the process id or path on the second line in the .dmrc file");
    }
    fprintf (stderr, "\n");
}

char * dmStrError (void)
{
    static char* string = 0;
    const int string_capacity = 128;
    if (string == 0) {
	string = (char*) malloc (string_capacity);
	ASSERT (string != 0);
    }

    if (dmerrno > 0 && dmerrno < dmnerr)
	sprintf (string, "%s", dmerrlist[dmerrno]);
    else
	sprintf (string, "DMI error number: %d", dmerrno);

    if (dmerrno == DME_SYS) {
	if (errno > 0) sprintf (string + strlen(string), ": %s", strerror (errno));
    }
#if 0
    if (dmerrno == DME_PRDATA) {
	strcat (string, "-- Check the value of the variable 'ICDPATH' and/or");
	strcat (string, " the process id or path on the second line in the .dmrc file");
    }
#endif
    return string;
}
