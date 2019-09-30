/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	S. de Graaf
 *	A.J. van Genderen
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
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "src/libddm/dmincl.h"

void die (int nr, char *s);
void lview (char *cellName, char *view, int nr);
void sig_handler (int sig);

char *argv0 = "dblist";

DM_PROJECT *project;
char *cellName = NULL;
int  NoErr = 0;
int  dflag = 0;
int  hflag = 0;
int  rflag = 0;
int  layout = 0;
int  circuit = 0;
int  floorplan = 0;
int  oneview = 0;
int  columns = 4;
char ** viewlist = NULL;

int main (int argc, char *argv[])
{
    char *s;

    if (!isatty (fileno (stdout))) columns = 1;

    while (--argc > 0) {
        if ((*++argv)[0] == '-' ) {
	    for (s = *argv + 1; *s != '\0'; s++) {
	        switch (*s) {
		    case 'h':
			hflag = 1;
			break;
		    case 'd':
			dflag = 1;
			break;
		    case 'r':
			rflag = 1;
			break;
		    case 'l':
			layout = 1;
			break;
		    case 'c':
			circuit = 1;
			break;
		    case 'f':
			floorplan = 1;
			break;
		    case 'i': /* obsolete */
		    case 's':
		    case 't':
			break;
		    default:
			die (0, argv0);
	        }
	    }
	}
	else if (cellName == NULL) {
	    cellName = *argv;
	}
	else {
	    die (0, argv0);
	}
    }

    if (layout) ++oneview;
    if (circuit) ++oneview;
    if (floorplan) ++oneview;

    if (!oneview) {
	floorplan = 1;
	circuit = 1;
	layout = 1;
    }
    else if (oneview > 1) {
	oneview = 0;
    }

#ifdef SIGHUP
    signal (SIGHUP,  SIG_IGN); /* ignore hangup signal */
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);
#endif
    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	signal (SIGINT, sig_handler);
    signal (SIGTERM, sig_handler);

    dmInit (argv0);
    project = dmOpenProject (DEFAULT_PROJECT, PROJ_READ);

    if (layout)    lview (cellName, LAYOUT,    1);
    if (circuit)   lview (cellName, CIRCUIT,   2);
    if (floorplan) lview (cellName, FLOORPLAN, 3);

    if (!oneview) printf ("\n");

    dmQuit ();

    exit (0);
    return (0);
}

void sig_handler (int sig) /* signal handler */
{
    static char buf[8];
    signal (sig, SIG_IGN); /* ignore signal */
    sprintf (buf, "%d", sig);
    fprintf (stderr, "\n");
    die (1, buf);
}

void dmError (char *s)
{
    if (NoErr) return;
    fprintf (stderr, "%s: ", argv0);
    dmPerror (s);
    die (2, "");
}

char *errlist[] = {
/* 0 */  "\nUsage: %s [-lcf] [-h] [-d] [-r] [cell]\n\n",
/* 1 */  "interrupted due to signal: %s",
/* 2 */  "error in DMI function",
/* 3 */  "readcell: %s: cannot alloc core",
/* 4 */  "unknown error"
};

void die (int nr, char *s)
{
    if (nr < 0 || nr > 4) nr = 4;
    if (nr) fprintf (stderr, "%s: ", argv0);
    fprintf (stderr, errlist[nr], s);
    if (nr) {
	dmQuit ();
	fprintf (stderr, "\n%s: -- program aborted --\n", argv0);
    }
    exit (1);
}
