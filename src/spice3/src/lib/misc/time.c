/**********
Copyright 1990 Regents of the University of California. All rights reserved.
**********/

/*
 * Date and time utility functions
 */

#include "spice.h"
#include <stdio.h>
#include "misc.h"

#include <time.h>

#ifdef HAS_BSDRUSAGE
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#else
#include <sys/types.h>
#include <sys/times.h>
#include <sys/param.h>
#endif

/* Return the date. Return value is static data. */

char * datestring()
{
    static char tbuf[45];
    struct tm *tp;
    char *ap;
    int i;

    time_t tloc;
    time(&tloc);
    tp = localtime(&tloc);
    ap = asctime(tp);
    sprintf(tbuf, "%.20s", ap);
    strcat(tbuf, ap + 19);
    i = strlen(tbuf);
    tbuf[i - 1] = '\0';
    return (tbuf);
}

/* How many seconds have elapsed in running time. */

double seconds()
{
#ifdef HAS_BSDRUSAGE
    struct rusage ruse;

    (void) getrusage(RUSAGE_SELF, &ruse);
    return (ruse.ru_utime.tv_sec + (double) ruse.ru_utime.tv_usec / 1000000.0);
#else
    struct tms tmsbuf;

    times(&tmsbuf);
    return((double) tmsbuf.tms_utime / HZ);
#endif
}
