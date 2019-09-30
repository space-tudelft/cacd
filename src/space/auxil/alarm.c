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

#include <signal.h>
#include "src/space/auxil/auxil.h"
#ifdef DRIVER
#include <time.h>
#endif

#ifdef SIGALRM
static void (*Handler) (void);
static int Sec;

static void catcher (int sig)
{
    (*Handler) ();
    signal (sig, catcher);
    if (Sec) alarm (Sec);
}
#endif

/* setAlarmInterval
 *    Set up a handler for the SIGALRM signal,
 *    and send an alarm every sec seconds,
 *    so that handler is invoked.
 *    If sec <= 0, do not send signals automatically but
 *    set up handler anyway to respond to externally (keyboard)
 *    generated alarms.
 */
int setAlarmInterval (int sec, void (*handler)(void))
{
#ifdef SIGALRM
    if (signal (SIGALRM, SIG_IGN) != SIG_IGN) {
	if (sec > 0) Sec = sec;
	Handler = handler;
	signal (SIGALRM, catcher);
	if (Sec) alarm (Sec);
    }
    return (0);
#else
    return (1);
#endif
}

#ifdef DRIVER

static void onAlarm ()
{
    time_t ck = time (0);
    printf ("%s", ctime (&ck));
}

/* a test driver and an example.
 */
int main (int argc, char *argv[])
{
    int seconds = 5;

    if (argc > 1) seconds = atoi (argv[1]);

    printf ("pid=%d\n", (int)getpid ());
    onAlarm ();
    setAlarmInterval (seconds, onAlarm);

    while (1) ; /* loop forever */
    return (0);
}
#endif /* DRIVER */
