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

#include <stdarg.h>
#include "src/space/auxil/auxil.h"

#define BFSIZE 2000

/*
 * mprintf - sprintf in static buffer
 *
 * Example:
 *    int i = ...;
 *    ...
 *    fp = cfopen (mprintf ("abc%d", i), "w");
 *
 * HOWEVER: There is only one internal buffer.
 */
char * mprintf (const char *format, ...)
{
    static char buf[BFSIZE];
    va_list args;

    va_start (args, format);

    vsprintf (buf, format, args);

    if (strlen (buf) + 1 >= BFSIZE) {
#ifdef DRIVER
	printf ("mprintf: string too long\n"), exit (1);
#else
	say ("mprintf: string too long"), die ();
#endif
    }

    va_end (args);

    return (buf);
}

#ifdef DRIVER
/* test driver and example */
int main ()
{
    char * s = mprintf ("%s %d %g\n", "abc", 1, 2.345);
    printf ("next lines should be equal:\n");
    printf (s);
    printf ("%s %d %g\n", "abc", 1, 2.345);
    return (0);
}
#endif /* DRIVER */
