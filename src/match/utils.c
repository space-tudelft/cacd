static char *rcsid = "$Id: utils.c,v 1.1 2018/04/30 12:17:54 simon Exp $";
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	T. Vogel
 *	A.J. van Genderen
 *	S. de Graaf
 *	A.J. van der Hoeven
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

#include <stdio.h>
#include <sys/times.h>
#include <unistd.h>

#include "src/match/mod.h"

extern boolean g_opt;

void time_progress (string line, string name)
{
    static double no_ticks, total;
    static long long old_time;
    struct tms tbuf;
    double interval;
    long long new_time;

    if (!g_opt) return;

    times (&tbuf);
    new_time = tbuf.tms_utime + tbuf.tms_stime;

    if (!line) {
	no_ticks = sysconf (_SC_CLK_TCK);
	old_time = new_time;
	return;
    }

    interval = (double) (new_time - old_time) / no_ticks;

    total += interval;

    fprintf (stdout, "%s", line);
    if (name) fprintf (stdout, " '%s'", name);
    fprintf (stdout, " in %.3f seconds (acc. %.3f seconds)\n", interval, total);

    old_time = new_time;
}
