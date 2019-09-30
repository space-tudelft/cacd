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

#include "src/ocean/libseadif/sealib.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int sdfprintwarnings = 1;

/* General purpose error reporting function.
 * If level is Fatal, then this function exits the program.
 * The format of the message is identical to the format accepted by printf().
 *
 * Example usage:
 *   sdfreport (Warning, "line %d: skipped weird statement", lineno);
 */
void sdfreport (errorLevel level, const char *message, ...)
{
   va_list argp;

   if (level == Warning && !sdfprintwarnings) return; /* printing of warnings is currently off */

   va_start (argp, message);

   fflush (stdout);
   fflush (stderr);

   if (level == Warning) fprintf (stderr, "WARNING: ");
   if (level == Error)   fprintf (stderr, "ERROR: ");
   if (level == Fatal)   fprintf (stderr, "FATAL ERROR: ");
   vfprintf (stderr, message, argp);
   fprintf (stderr, "\n");
   fflush (stderr);

   if (level == Fatal) sdfexit (1);
}
