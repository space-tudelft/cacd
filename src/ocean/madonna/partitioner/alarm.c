/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 *	Paul Stravers
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

#include <signal.h>
#include <stdio.h>
#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/libseadif/sysdep.h"
#include "src/ocean/madonna/partitioner/part.h"

int alarm_flag = 0;
int alarm_flag_reset = 1;

void raise_alarm_flag (int dummyARG)
{
    fprintf (stderr, "\n**** Caught signal ALRM, please wait for current thing to finish....\n");
    fprintf (stderr, "     (send signal USR1 to disable the partitioner entirely)\n");
    alarm_flag = 1;
}

void enable_reset_alarm_flag (int dummyARG)
{
    fprintf (stderr, "\n**** Caught signal USR1, signal ALRM now aborts recursively....\n");
    alarm_flag_reset = 0;
}

void initsignals (void)
{
#ifdef SIGALRM
    if ((void*)signal (SIGALRM, (SIG_PF_TYPE)raise_alarm_flag) == (void*)SIG_ERR)
	perror ("setalarm SIGALRM");
#endif
#ifdef SIGUSR1
    if ((void*)signal (SIGUSR1, (SIG_PF_TYPE)enable_reset_alarm_flag) == (void*)SIG_ERR)
	perror ("setalarm SIGUSR1");
#endif
}
