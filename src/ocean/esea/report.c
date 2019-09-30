/*
 * ISC License
 *
 * Copyright (C) 1993-2018 by
 *	Paul Stravers
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

#include "src/ocean/esea/cedif.h"
#include <stdarg.h>		/* var arg stuff */
#include <stdio.h>		/* vfprintf(), fflush(), stdout, stderr. */
#include <stdlib.h>		/* exit() */

/* General purpose error reporting function. If level is Fatal, then this
 * function exits the program. The format of the message is identical to the
 * format accepted by printf(). Example usage:
 *
 *   report(eWarning, "line %d: skipped weird statement", lineno);
 */
void report(const xErrorLevel level, const char *message, ...)
{
   va_list argp;
   va_start(argp, message);
   fflush(stdout);
   fflush(stderr);
   if (level == eWarning) fprintf(stderr,"WARNING: ");
   if (level == eFatal)   fprintf(stderr,"ERROR: ");
   vfprintf(stderr, message, argp);
   fprintf(stderr, "\n");
   fflush(stderr);
   if (level == eFatal)
   {
      if (targetLanguage == NelsisLanguage)
	 exitNelsis(1);
      else if (targetLanguage == SeadifLanguage)
	 exitSeadif(1);
      else
	 exit(1);
   }
}