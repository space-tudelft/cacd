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

#include <sys/types.h>
#include <time.h>
#include <sys/times.h>

#include "src/space/auxil/auxil.h"

#define MAXCLOCKS 1000

static long clk_tck;

struct clock {
    long user, sys, real;
    long ticks;
#ifdef DRIVER
    char name[16];
#else
    char *name;
#endif
} clocks [MAXCLOCKS];

static int maxclocks = 0;
static int clkEnable = 0;
static struct tms tms_buf;

#define  User(i) (clocks[i].user)
#define   Sys(i) (clocks[i].sys)
#define  Real(i) (clocks[i].real)
#define Ticks(i) (clocks[i].ticks)
#define  Name(i) (clocks[i].name)

/* local operations */
static void prtime (FILE *fp, long t);

/*
 * This clock module maintains a number of up to MAXCLOCK
 * timers, identified by a string given in the tick and tock
 * functions.
 * The tick function starts a given timer.
 * The tock function stops a given timer.
 * Timers can be stopped and restarted many times.
 *
 * Example:
 *     ...
 *     clockInit ();
 *     ...
 *     tick ("doit");		-- or inside doit
 *     doit ();
 *     tock ("doit");
 *     ...
 *     clockPrintAll (stdout);
 *     clockPrintTime (stdout)
 *     ...
 *
 * clockInit - start up this module
 * Must be called once at startup of the program,
 * or whenever the timers must be reset.
 *
 */
void clockInit ()
{
    maxclocks = 0;
    clkEnable = 1;

#ifdef CLK_TCK
    clk_tck = CLK_TCK;
#else
    clk_tck = sysconf (_SC_CLK_TCK);
#endif
    tick ("total time");
}

/*
 * Start or restart the timer <s>.
 * No special actions are needed to start
 * a timer for the first time.
 */
void tick (char *s)
{
    int i;

    if (!clkEnable) return;

    for (i = 0; i < maxclocks; i++)
	if (strsame (s, Name (i))) break;

    if (i == maxclocks && maxclocks < MAXCLOCKS) {
#ifdef DRIVER
	strcpy (Name (i), s);
#else
	Name (i) = strsave (s);
#endif
	Ticks (i) = 0;
	Real (i) = 0;
	Sys  (i) = 0;
	User (i) = 0;
	maxclocks++;
    }

    if (i < maxclocks) {
	Ticks (i) += 1;
	Real (i) -= times (&tms_buf);
	Sys  (i) -= tms_buf.tms_stime;
	User (i) -= tms_buf.tms_utime;
    }
}

/*
 * Stop the timer <s>.
 */
void tock (char *s)
{
    int i;

    if (!clkEnable) return;

    for (i = 0; i < maxclocks; i++)
	if (strsame (s, Name (i))) break;

    if (i < maxclocks) {
	Ticks (i) -= 1;
	Real (i) += times (&tms_buf);
	Sys  (i) += tms_buf.tms_stime;
	User (i) += tms_buf.tms_utime;
    }
}

/*
 * Print a table with the real, user and sys times
 * of all timers, to the stream indicated by <fp>
 */
void clockPrintAll (FILE *fp)
{
    double d;
    int i;
    fprintf (fp, "\n%-22s%12s%12s%11s\n", "procedure", "real", "user", "sys");

    ++clkEnable;
    for (i = 0; i < maxclocks; i++) {
	if ((Ticks(i) == 0) && (*Name(i) != '_')) {
	    fprintf (fp, "%-22s", Name (i));
	    prtime  (fp, Real (i));
	    prtime  (fp, User (i));
	    prtime  (fp, Sys  (i));
	    d = User (i);
	    d += Sys (i);
	    if (d && d < Real (i))
		fprintf (fp, "%6.1f%%\n", (100 * d) / Real (i));
	    else
		fprintf (fp, " 100%%\n");
	}
    }
    fprintf (fp, "\n");
    --clkEnable;
}

/*
 * Print total times (user, cpu and sys) to <fp>
 */
void clockPrintTime (FILE *fp)
{
    double d;
    tock ("total time");
    fprintf (fp,   "\tuser time          : "); prtime  (fp, User (0));
    fprintf (fp, "\n\tsystem time        : "); prtime  (fp, Sys  (0));
    fprintf (fp, "\n\treal time          : "); prtime  (fp, Real (0));
    d = User (0);
    d += Sys (0);
    if (d && d < Real (0))
	fprintf (fp, "%6.1f%%\n", (100 * d) / Real (0));
    else
	fprintf (fp, " 100%%\n");
    fprintf (fp, "\n");
    tick ("total time");
}

/* converts a long to a time expression and prints it */
static void prtime (FILE *fp, long t)
{
    double s;
    long h, m, v;
    int flag = 0;

    h = 3600 * clk_tck; /* one hour */
    if (t >= h) {
	v = t / h;
	t = t % h;
        fprintf (fp, "%3ld:", v);
	flag = 1;
    }
    else {
	fprintf (fp, "    ");
    }

    m = 60 * clk_tck; /* one minute */
    if (t >= m || flag) {
	v = t / m;
	t = t % m;
	if (v < 10 && flag)
            fprintf (fp, "0%ld:", v);
	else
            fprintf (fp, "%2ld:", v);
	flag = 1;
    }
    else {
	fprintf (fp, "   ");
    }

    s = t; s /= clk_tck;
    if (flag && s + 0.05 < 10)
	fprintf (fp, clkEnable > 1 ? "0%4.2f" : "0%3.1f", s);
    else
	fprintf (fp, clkEnable > 1 ? "%5.2f" : "%4.1f", s);
}

#ifdef DRIVER

/* test driver and example
 */
int main (int argc, char *argv[])
{
    int i, j, n = 1;

    if (argc > 1) n = atoi (argv[1]);
    n *= 100;
    clockInit ();
    tick ("system");
    for (i = 0; i < n; i++)
	if (system ("echo hello world > ./zdev.null")) printf ("system failed\n");
    tock ("system");

    tick ("loop");
    for (i = 0; i < n; i++)
	for (j = 0; j < 100000; j++) if (strlen ("abcd") != 4) printf ("strlen error\n");
    tock ("loop");

    tick ("syscall");
    for (i = 0; i < n; i++)
	for (j = 0; j < 100; j++)
	    if (access ("./zdev.null", R_OK)) printf ("access error\n");
    tock ("syscall");

    unlink ("./zdev.null");
    clockPrintAll (stdout);
    clockPrintTime (stdout);
    return (0);
}
#endif
