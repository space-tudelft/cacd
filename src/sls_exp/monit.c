/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van Genderen
 *	S. de Graaf
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

/*
 * Routines for monitoring program run time in a file called mon
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/times.h>
#include "src/libddm/dmincl.h"

#define TRUE   1
#define FALSE  0

#define HZ clk_tck

static void printtime (long t);

struct tms timebuf;

FILE * monit;

time_t starttime;
time_t stoptime;
long ttprev;

static long clk_tck;

/* at the start this routine has to be called */
void startmonitime (char *pname)
{
    char buf[DM_MAXNAME+5];

#ifdef CLK_TCK
    clk_tck = CLK_TCK;
#else
    clk_tck = sysconf (_SC_CLK_TCK);
#endif

    sprintf (buf, "%s.mon", pname);
    if ( (monit = fopen (buf, "w")) == NULL ) {
	fprintf (stderr, "Cannot open file %s\n", buf);
	exit (1);
    }

    fprintf (monit, "procedure             "); /* 22 */
    fprintf (monit, "        sys"); /* 11 */
    fprintf (monit, "       user");
    fprintf (monit, "      total");
    fprintf (monit, "        inc\n");
    starttime = time (0);
    ttprev = 0;
}

/* prints str, together with current run times */
/* str shouldn't be larger than 21 characters  */
void monitime (char *str)
{
    long ttnow;
    int i;

    fprintf (monit, "%s", str);
    i = strlen (str);
    while (i < 22) {
	fprintf (monit, " ");
	i++;
    }

    times (&timebuf);

    printtime (timebuf.tms_stime);

    printtime (timebuf.tms_utime);

    ttnow = timebuf.tms_utime + timebuf.tms_stime;
    printtime (ttnow);

    printtime (ttnow - ttprev);

    fprintf (monit, "\n");

    ttprev = ttnow;
}

/* converts a long to a time expression and prints it */
static void printtime (long t)
{
    long h, m, s, d;
    int flag = FALSE;

    h = t / (3600 * HZ);
    m = (t - h * 3600 * HZ) / (60 * HZ);
    s = (t - h * 3600 * HZ - m * 60 * HZ) / HZ;
    d = (t - h * 3600 * HZ - m * 60 * HZ - s * HZ) / (HZ * 0.10);

    if (h > 0) {
        fprintf (monit, "%3ld:", h);
	flag = TRUE;
    }
    else {
	fprintf (monit, "    ");
    }

    if (m > 0 || flag) {
	if (m < 10 && flag)
            fprintf (monit, "0%1ld:", m);
	else
            fprintf (monit, "%2ld:", m);
	flag = TRUE;
    }
    else {
	fprintf (monit, "   ");
    }

    if (s < 10 && flag)
        fprintf (monit, "0%1ld.", s);
    else
        fprintf (monit, "%2ld.", s);
    fprintf (monit, "%ld", d);
}

/* at the end of the monitoring this routine has to be called */
void stopmonitime ()
{
    stoptime = time (0);
    times (&timebuf);
    fprintf (monit, "effective time: %2ld%%\n",
	(timebuf.tms_utime + timebuf.tms_stime) * 100 / (HZ * (long)(stoptime - starttime)));
    fclose (monit);
}
