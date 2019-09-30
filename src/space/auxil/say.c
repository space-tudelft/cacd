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

#include <ctype.h>
#include <stdarg.h>
#include "src/space/auxil/auxil.h"

static int verboseLevel = 0; /* FALSE */

static FILE * fpnew = NULL;
static FILE * fpout = NULL;
static FILE * fperr = NULL;

static int messageCnt = 0;
static int maxMessageCnt = -1;
static int fp_both = 0;

char * argv0 = NULL;

void verboseSetErrStream (FILE *fp)
{
    fperr = fp;
}

void verboseSetOutStream (FILE *fp)
{
    fpout = fp;
}

void verboseSetNewStream (FILE *fp)
{
    if ((fpnew = fp)) fp_both = 1;
}

void swapVerboseStreams ()
{
    if (fp_both) {
	fpout = fpnew; fp_both = 0;
    } else {
	fpout = stdout; if (fpnew) fp_both = 1;
    }
}

void setMaxMessageCnt (int cnt)
{
    if (cnt < INT_MAX) maxMessageCnt = cnt;
}

/* If message begins with a tab,
 * it is assumed to be a continuation message.
 * In that case, don't print progname in front.
 * Add \n if not already at the end.
 */
void say (const char *format, ...)
{
    static int level = 0;
    va_list args;
    va_start (args, format);

    level++;

    if (!fperr) fperr = stderr;

    if (maxMessageCnt >= 0 && ++messageCnt > maxMessageCnt) {
	if (!argv0) argv0 = "??";
	fprintf (fperr, "\n%s: maximum number of messages (%d) execeeded\n\n",
		argv0, maxMessageCnt);
	die ();
    }

    if (argv0 && *format != '\t')
	fprintf (fperr, "%s: ", argv0);

    vfprintf (fperr, format, args);

    if (!(isspace ((int)*(format + strlen (format) - 1))))
	fprintf (fperr, "\n");

    fflush (fperr);

    va_end (args);

    /* Since 'extraSay()' may call 'say()' again and we want to prevent
       a loop, we only call 'extraSay()' at level 1. */

    if (level == 1) extraSay ();

    level--;
}

void verboseSetMode (bool_t mode)
{
    verboseLevel = mode;
}

void verboseSetLevel (int level)
{
    verboseLevel = level;
}

int verboseGetLevel ()
{
    return verboseLevel;
}

void raiseVerbosity ()
{
    ++verboseLevel;
}

void lowerVerbosity ()
{
    --verboseLevel;
}

/*
 * If message <s> is not ended by whitespace, append
 * newline.
 */
void verbose (const char *format, ...)
{
    va_list args;
    va_start (args, format);

    if (verboseLevel > 0) {

	if (maxMessageCnt >= 0 && ++messageCnt > maxMessageCnt) {
	    if (!fperr) fperr = stderr;
	    if (!argv0) argv0 = "??";
	    fprintf (fperr, "\n%s: maximum number of messages (%d) execeeded\n\n",
			argv0, maxMessageCnt);
	    die ();
	}

        if (!fpout) fpout = stdout;
	vfprintf (fpout, format, args);
	if (!(isspace ((int)*(format + strlen (format) - 1))))
	    fprintf (fpout, "\n");
	fflush (fpout);
    }

    va_end (args);
}

/*
 * If message <s> is not ended by whitespace, append
 * newline.
 */
void message (const char *format, ...)
{
    va_list args;
    va_start (args, format);

    if (!fpout) fpout = stdout;
    vfprintf (fpout, format, args);
    if (!(isspace ((int)*(format + strlen (format) - 1))))
	fprintf (fpout, "\n");
    fflush (fpout);

if (fp_both) {
    va_start (args, format);
    vfprintf (fpnew, format, args);
    if (!(isspace ((int)*(format + strlen (format) - 1))))
	fprintf (fpnew, "\n");
    fflush (fpnew);
}
    va_end (args);
}

#ifdef DRIVER
//#define testVerbose

int main ()
{
#ifdef testVerbose
#define say verbose
    verboseSetMode (TRUE);
#endif
    argv0 = "testsay";
 // setMaxMessageCnt (1);

    say ("int 123='%d', string <abc>='%s'", (int) 123, "abc");

    say ("double 1.2345='%g'", 1.2345); /* should also work */

    return (0);
}

void extraSay ()
{
    /* Can be defined by an application program
       to give an extra message after say(). */
    say ("\tis this correct?");
}

void die ()
{
    exit (1);
}
#endif /* DRIVER */
