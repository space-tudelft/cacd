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

extern char *argv0;

void onsig (int sig)
{
    char *s;

    signal (sig, SIG_IGN);
    switch (sig) {
#ifdef SIGABRT
	case SIGABRT: s = "Abort signal"; break;
#endif
#ifdef SIGILL
	case SIGILL:  s = "Illegal instruction"; break;
#endif
#ifdef SIGFPE
	case SIGFPE:  s = "Floating point exception"; break;
#endif
#ifdef SIGBUS
	case SIGBUS:  s = "Bus error"; break;
#endif
#ifdef SIGSEGV
	case SIGSEGV: s = "Segmentation violation"; break;
#endif
#ifdef SIGINT
	case SIGINT:  s = "Interrupt"; break;
#endif
#ifdef SIGQUIT
	case SIGQUIT: s = "Quit"; break;
#endif
#ifdef SIGTERM
	case SIGTERM: s = "Termination signal"; break;
#endif
	default:      s = "Unknown signal";
    }

    say ("%s", s);
    die ();
}

/*
 * Set up signal handling for the usual signals:
 *      SIGILL SIGFPE SIGBUS SIGSEGV SIGINT SIGQUIT
 * but only if they exist in the system.
 * Everyting is included between ifdefs.
 *
 * The associated signal handler will print a message
 * (using say) and then invoke die ().
 *
 * This lib contains a default die (), to be redefined
 * in an application.
 */
void catchSignals ()
{
#ifdef OLD
#define install_handler(sig) signal (sig, onsig)
#else
#define install_handler(sig) sigaction (sig, &act, NULL)
    struct sigaction act;
    act.sa_handler = onsig;
    act.sa_flags = SA_SIGINFO;
#endif

#ifdef SIGINT
    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	install_handler (SIGINT);
#endif

#ifndef DEBUG

#ifdef SIGQUIT
    if (signal (SIGQUIT, SIG_IGN) != SIG_IGN)
        install_handler (SIGQUIT);
#endif
#ifdef SIGTERM
    install_handler (SIGTERM);
#endif
#ifdef SIGILL
    install_handler (SIGILL);
#endif
#ifdef SIGFPE
    install_handler (SIGFPE);
#endif
#ifdef SIGBUS
    install_handler (SIGBUS);
#endif
#ifdef SIGSEGV
    install_handler (SIGSEGV);
#endif
#ifdef SIGABRT
    install_handler (SIGABRT);
#endif

#endif /* DEBUG */
}

#ifdef DRIVER

/* To test: compile and run, then send signal from the keyboard
 * or with kill.
 */
int main ()
{
    argv0 = "testsignal";

    catchSignals ();
    say ("10 seconds to kill me (pid=%d)", (int)getpid());
    sleep (10);
    return (0);
}
#endif
