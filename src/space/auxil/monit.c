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
#include "src/space/auxil/auxil.h"
#include <sys/times.h>

static struct tms timebuf;

static FILE * monit;
static long clk_tck;

static time_t starttime;
static time_t ttprev;

/* local operations */
static void printtime (time_t t);

/*
 * Routines for monitoring program run time in a file
 * called <pname>.mon
 *
 * startmonitime - init monitoring of runtimes
 */
void startmonitime (char *pname)
{
    char buf[20];

#ifdef CLK_TCK
    clk_tck = CLK_TCK;
#else
    clk_tck = sysconf (_SC_CLK_TCK);
#endif

    sprintf (buf, "%s.mon", pname);
#ifdef DRIVER
    monit = fopen (buf, "w");
    if (!monit) return;
#else
    monit = cfopen (buf, "w");
#endif
    fprintf (monit, "%-22s%11s%11s%11s%11s\n", "procedure", "sys", "user", "total", "inc");
    fflush  (monit);
    ttprev = starttime = times (&timebuf);
}

/*
 * print <str>, together with current run times to .mon file.
 * <str> shouldn't be larger than 21 characters
 */
void monitime (char *str)
{
    time_t ttnow = times (&timebuf);

    fprintf (monit, "%-22s", str);

    printtime (timebuf.tms_stime);  /* sys */
    printtime (timebuf.tms_utime);  /* user */
    printtime (ttnow - starttime);  /* total */
    printtime (ttnow - ttprev);	    /* inc */

    fprintf (monit, "\n");
    fflush  (monit);

    ttprev = ttnow;
}

/*
 * converts a time_t to a time expression and prints it
 */
static void printtime (time_t t)
{
    double s;
    long h, m, v;
    int flag = 0;

    h = 3600 * clk_tck; /* one hour */
    if (t >= h) {
	v = t / h;
	t = t % h;
	fprintf (monit, "%3ld:", v);
	flag = 1;
    }
    else {
	fprintf (monit, "    ");
    }

    m = 60 * clk_tck; /* one minute */
    if (t >= m || flag) {
	v = t / m;
	t = t % m;
	if (v < 10 && flag)
	    fprintf (monit, "0%ld:", v);
	else
	    fprintf (monit, "%2ld:", v);
	flag = 1;
    }
    else {
	fprintf (monit, "   ");
    }

    s = t; s /= clk_tck;
    if (flag && s + 0.5 < 10)
	fprintf (monit, "0%3.1f", s);
    else
	fprintf (monit, "%4.1f", s);
}

/*
 * Terminate monitoring.
 *
 * When startmonitime was called, this routine
 * should also be called (at the end of the program).
 */
void stopmonitime ()
{
    time_t stoptime = times (&timebuf);
    if (stoptime > starttime)
	fprintf (monit, "effective time: %3.0f%%\n",
	    100.0 * (timebuf.tms_utime + timebuf.tms_stime) / (stoptime - starttime));
    else
	fprintf (monit, "effective time: 100%%\n");
    fclose (monit);
}

#ifdef DRIVER
/* test driver and example */
int main ()
{
    int n;
    startmonitime ("monit");
    if (!monit) {
	printf ("cannot write monit.mon\n");
	return (1);
    }
    monitime ("before sleep");
    sleep (1);
    monitime ("after sleep");
    n = 1000000; while (--n >= 0) if (access ("monit.mon", R_OK)) printf ("access error\n");
    monitime ("after system");
    sleep (2);
    monitime ("after sleep2");
    sleep (3);
    monitime ("after sleep3");
    stopmonitime ();
    printf ("results in monit.mon\n");
    return (0);
}
#endif /* DRIVER */
