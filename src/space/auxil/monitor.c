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

#if defined (DRIVER) && ! defined (MONITOR)
#define MONITOR
#endif

/* #define MONITOR		if the system supports it */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "src/space/include/config.h"
#include "src/space/auxil/monitor.h"

#ifdef MONITOR

/* Primitive interface for profiling,
 * No function call counts, but only used time.
 * Produces a mon.out file, to be processed by prof (1).
 * See also monitor (3c) and profil (2).
 *
 * Example:
 *    ...
 *    monitorStart ();
 *    ...
 *    doMyTrick ();
 *    ...
 *    monitorStop ();
 *    ...
 */
void monitorStart ()
{
    WORD *buffer;
    size_t bufsize;
    extern etext;

    bufsize = ((size_t) &etext) / 5;

    buffer = NEW (WORD, bufsize);

    monitor ((int(*)())2, (int(*)())&etext, buffer, bufsize, 0);
}

/*
 * Stop monitoring
 */
void monitorStop ()
{
    monitor ((int(*)())0, (int(*)())0, 0, 0, 0);
}
#endif /* MONITOR */

#ifdef DRIVER
Private void func ()
{
    long i;
    for (i = 0; i < 1000000L; i++) strlen ("abcd");
}

/* test driver and example */
int main ()
{
    int i;

    monitorStart ();

    func ();

    for (i = 0; i < 10000; i++) access ("/usr/abc/def", R_OK);

    monitorStop ();
    printf ("a mon.out file is produced, analyze with prof(1)\n");

    return (0);
}
#endif /* DRIVER */
