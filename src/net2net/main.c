/*
 * ISC License
 *
 * Copyright (C) 2016 by
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

#include "src/net2net/incl.h"

#define TOOLVERSION "1.03 21-Jul-2016"

char *argv0 = "net2net";
char *use_msg = "\nUsage: %s [-cilntv] cell\n\n";

DM_PROJECT *dmproject = NULL;

long lower[10], upper[10];
char attribute_string[512];

int inst_change = 1;
int coordinates = 1;
int no_lays = 0;
int nrtimes = 0;
int nr_cds  = 0;
int verbose = 0;

char *nameGND = "GND";
char *nameSUB = "SUBSTR";

static void initIntrup (void);

int main (int argc, char *argv[])
{
    char *s, *network = NULL;

    if (argc <= 1) fprintf (stderr, "%s %s\n", argv0, TOOLVERSION);

    while (--argc > 0) {
	if ((*++argv)[0] == '-') {
	    for (s = *argv + 1; *s; s++) {
		switch (*s) {
		    case 'c': coordinates = 0; break;
		    case 'i': inst_change = 0; break;
		    case 'l': ++no_lays; break;
		    case 'n': ++nr_cds;  break;
		    case 't': ++nrtimes; break;
		    case 'v': ++verbose; break;
		    default:
			fprintf (stderr, "%s: illegal option: %c\n", argv0, *s);
			exit (1);
		}
	    }
	}
	else if (!network) network = strsave (*argv);
	else { network = NULL; break; } /* exit */
    }

    if (!network) { fprintf (stderr, use_msg, argv0); exit (1); }

    dmInit ("xspf");
    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    dm_get_do_not_alloc = 1; /* fast circuit streams read */

    cmc.inst_attribute = attribute_string;
    cmc.inst_lower = lower;
    cmc.inst_upper = upper;

    initIntrup ();

    xnetwork (network, dmproject);

    if (dmproject) dmCloseProject (dmproject, COMPLETE);
    dmQuit ();

    return (0);
}

void warning (char *s1)
{
    fprintf (stderr, "%s: warning: %s\n", argv0, s1);
}

void warning2 (char *s1, char *s2)
{
    fprintf (stderr, "%s: warning: ", argv0);
    fprintf (stderr, s1, s2);
    fprintf (stderr, "\n");
}

void verbose3 (char *s1, char *s2, char *s3)
{
    if (!verbose) return;
    fprintf (stderr, "%s: ", argv0);
    fprintf (stderr, s1, s2, s3);
    fprintf (stderr, "\n");
}

void fatalErr (char *s1, char *s2)
{
    fprintf (stderr, "%s:", argv0);
    if (s1 && *s1) fprintf (stderr, " %s", s1);
    if (s2 && *s2) fprintf (stderr, " %s", s2);
    fprintf (stderr, "\n");
    die ();
}

void int_hdl (int sig) /* interrupt handler */
{
    char *s;
    switch (sig) {
#ifdef SIGILL
	case SIGILL: s = "Illegal instruction"; break;
#endif
#ifdef SIGFPE
	case SIGFPE: s = "Floating point exception"; break;
#endif
#ifdef SIGBUS
	case SIGBUS: s = "Bus error"; break;
#endif
#ifdef SIGSEGV
	case SIGSEGV: s = "Segmentation violation"; break;
#endif
	default: s = "Unknown signal"; break;
    }
    fatalErr (s, NULL);
}

static void initIntrup ()
{
#define install_handler(sig) signal (sig, int_hdl)
#ifdef SIGINT
    if (signal (SIGINT, SIG_IGN) != SIG_IGN) install_handler (SIGINT);
#endif
#ifdef SIGQUIT
    if (signal (SIGQUIT, SIG_IGN) != SIG_IGN) install_handler (SIGQUIT);
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
}

void dmError (char *s)
{
    fprintf (stderr, "%s: ", argv0);
    dmPerror (s);
    die ();
}

void die ()
{
    dmQuit ();
    exit (1);
}

void cannot_die (int nr, char *fn)
{
    char *s;
    switch (nr) {
	case 1: s = "Cannot allocate"; fn = "storage"; break;
	case 2: s = "Cannot read file:"; break;
	case 3: s = "Cannot write file:"; break;
	default:
	case 4: s = "Internal error:"; break;
    }
    fatalErr (s, fn);
}
