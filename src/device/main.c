/*
 * ISC License
 *
 * Copyright (C) 1992-2018 by
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "src/libddm/dmincl.h"

#define PE fprintf(stderr,
#define PO fprintf(stdout,

char *argv0 = "device";
char *use_msg = "\nUsage: %s [-s | -u] [ cell ... ]\n\n";

int tog_set = 0;
int tog_unset = 0;
int noErrMes = 0;

struct stat buf;
char path[1024];

DM_PROJECT *dmproject = NULL;

void int_hdl (int sig);
void initIntrup (void);
void die (void);

int do_cell (DM_PROJECT *proj, char *cell)
{
    DM_CELL *cellKey;

    noErrMes = 1;
    cellKey = dmCheckOut (proj, cell, WORKING, DONTCARE, CIRCUIT, READONLY);
    noErrMes = 0;
    if (!cellKey) return 1;

    if (dmStat (cellKey, "devmod", &buf) == 0) {
	if (proj == dmproject)
	    PO "   %s\n", cell);
	else
	    PO " * %s\n", cell);
    }
    dmCheckIn (cellKey, COMPLETE);
    return 0;
}

int main (int argc, char *argv[])
{
    DM_CELL *cellKey, *layCellKey;
    DM_STREAM *dsp, *ldsp;
    char **cellNames;
    char **celllist;
    char *s, *cell;
    long lower[2], upper[2];
    int cell_cnt;
    int isDevice;
    int mode;
    int usage = 0;
    int xcontrol = 0;

    cellNames = (char **) calloc (argc, sizeof (char *));

    cell_cnt = 0;
    while (--argc > 0) {
	if ((*++argv)[0] == '-' ) {
	    for (s = *argv + 1; *s != '\0'; s++) {
		switch (*s) {
		    case 's':
			tog_set = 1;
			break;
		    case 'u':
			tog_unset = 1;
			break;
		    default:
			PE "%s: illegal option: %c\n", argv0, *s);
			usage = 1;
		}
	    }
	}
	else {
	    cellNames[cell_cnt++] = *argv;
	}
    }
    cellNames[cell_cnt] = NULL;

    if (tog_set && tog_unset) {
	PE "%s: illegal combination of options\n", argv0);
	usage = 1;
    }
    if (usage) {
	PE use_msg, argv0);
	exit (1);
    }

    dmInit (argv0);
    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);
    xcontrol = (dmStatXData (dmproject, &buf) == 0);

    initIntrup ();

    if (*cellNames == NULL) {
	DM_PROJECT *proj;
	IMPCELL **icl;
	char *proj_notok[20];
	int i, j;

	if (xcontrol) {
	    PE "Warning: To show device status use program xcontrol.\n");
	    PO "\ndevmods:\n\n");
	}
	else
	    PO "\ndevices:\n\n");

	celllist = (char **)dmGetMetaDesignData (CELLLIST, dmproject, CIRCUIT);
	while (*celllist) {
	    (void) do_cell (dmproject, *celllist++);
	}

	i = 0;
	icl = (IMPCELL **)dmGetMetaDesignData (IMPORTEDCELLLIST, dmproject, CIRCUIT);
	if (icl)
	while (*icl) {
/* SdeG4.4
	    if (do_cell (dmproject, (*icl) -> alias)) {
*/
		if (i) {
		    for (j = 0; j < i; ++j) {
			if ((*icl) -> dmpath == proj_notok[j] ||
			    strcmp ((*icl) -> dmpath, proj_notok[j]) == 0)
				goto skip;
		    }
		}
		noErrMes = 1;
		proj = dmOpenProject ((*icl) -> dmpath, PROJ_READ);
		noErrMes = 0;
		if (proj)
		    (void) do_cell (proj, (*icl) -> cellname);
		else
		    proj_notok[i++] = (*icl) -> dmpath;
/* SdeG4.4
	    }
*/
skip:	++icl;
	}

	for (j = 0; j < i; ++j) {
	    PO "cannot open project: %s\n", proj_notok[j]);
	}

	PO "\n");
    }
    else {
	mode = (tog_set || tog_unset)? ATTACH : READONLY;

	if (xcontrol) {
	    if (mode == ATTACH)
		PE "Warning: To %s device status use program xcontrol.\n",
			tog_set? "set" : "unset");
	    s = "devmod";
	}
	else
	    s = "device";

	while (*cellNames) {

	    cell = *cellNames++;

	    if (_dmExistCell (dmproject, cell, CIRCUIT) == 1) {
		cellKey = dmCheckOut (dmproject, cell, WORKING,
					DONTCARE, CIRCUIT, mode);
	    }
	    else if (tog_set) {
		if (_dmExistCell (dmproject, cell, LAYOUT) != 1) {
		    PE "%s: no corresponding layout available, cannot set %s\n",
			cell, s);
		    continue;
		}
		layCellKey = dmCheckOut (dmproject, cell, ACTUAL,
					DONTCARE, LAYOUT, READONLY);
		cellKey = dmCheckOut (dmproject, cell, WORKING,
					DONTCARE, CIRCUIT, UPDATE);
		ldsp = dmOpenStream (layCellKey, "term", "r");
		dsp = dmOpenStream (cellKey, "term", "w");
		cterm.term_attribute = NULL;
		cterm.term_lower = lower;
		cterm.term_upper = upper;
		lower[0] = lower[1] = 0;
		while (dmGetDesignData (ldsp, GEO_TERM)) {
		    strcpy (cterm.term_name, gterm.term_name);
		    if (gterm.nx > 0 && gterm.ny > 0) {
			cterm.term_dim = 2;
			upper[0] = gterm.nx - 1;
			upper[1] = gterm.ny - 1;
		    }
		    else if (gterm.nx > 0) {
			cterm.term_dim = 1;
			upper[0] = gterm.nx - 1;
		    }
		    else if (gterm.ny > 0) {
			cterm.term_dim = 1;
			upper[0] = gterm.ny - 1;
		    }
		    else {
			cterm.term_dim = 0;
		    }
		    dmPutDesignData (dsp, CIR_TERM);
		}
		dmCloseStream (ldsp, COMPLETE);
		dmCheckIn (layCellKey, COMPLETE);
		dmCloseStream (dsp, COMPLETE);

		dsp = dmOpenStream (cellKey, "mc", "w");
		dmCloseStream (dsp, COMPLETE);

		dsp = dmOpenStream (cellKey, "net", "w");
		dmCloseStream (dsp, COMPLETE);
	    }
	    else
		cellKey = 0;

	    isDevice = 0;
	    if (cellKey) {
		if (dmStat (cellKey, "devmod", &buf) == 0) {
		    if (tog_unset) {
			dmUnlink (cellKey, "devmod");
			if (!xcontrol) { /* SdeG4.9 */
			    noErrMes = 1;
			    layCellKey = dmCheckOut (dmproject, cell, WORKING,
						DONTCARE, LAYOUT, mode);
			    noErrMes = 0;
			    if (layCellKey) { /* SdeG4.5, touch cell status */
				dsp = dmOpenStream (layCellKey, "is_macro", "a");
				fprintf (dsp -> dmfp, "\n");
				dmCloseStream (dsp, COMPLETE);
				dmCheckIn (layCellKey, COMPLETE);
			    }
			}
		    }
		    else isDevice = 1;
		}
		else if (tog_set) {
		    isDevice = 1;
		    dsp = dmOpenStream (cellKey, "devmod", "w");
		    dmCloseStream (dsp, COMPLETE);
		}
		dmCheckIn (cellKey, COMPLETE);
	    }

	    PE "%s: %s = %s\n", cell, s, isDevice? "yes" : "no");
	}
    }

    if (dmproject) dmCloseProject (dmproject, COMPLETE);
    dmQuit ();
    exit (0);
    return (0);
}

void initIntrup ()
{
    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
        signal (SIGINT, int_hdl);
        /* only when value was not SIG_IGN, a jump must be done to int_hdl */
#ifdef SIGQUIT
    if (signal (SIGQUIT, SIG_IGN) != SIG_IGN)
        signal (SIGQUIT, int_hdl);
        /* only when value was not SIG_IGN, a jump must be done to int_hdl */
#endif
    signal (SIGTERM, int_hdl);
    signal (SIGILL, int_hdl);
    signal (SIGFPE, int_hdl);
#ifdef SIGBUS
    signal (SIGBUS, int_hdl);
#endif
    signal (SIGSEGV, int_hdl);
}

void int_hdl (int sig) /* interrupt handler */
{
    switch (sig) {
        case SIGILL :
            PE "Illegal instruction\n");
            break;
        case SIGFPE :
            PE "Floating point exception\n");
            break;
#ifdef SIGBUS
        case SIGBUS :
            PE "Bus error\n");
            break;
#endif
        case SIGSEGV :
            PE "Segmentation violation\n");
            break;
        default :
            break;
    }

    die ();
}

void die ()
{
    if (dmproject) dmCloseProject (dmproject, QUIT);
    dmQuit ();
    exit (1);
}

void dmError (char *s)
{
    if (!noErrMes) {
	PE "%s: ", argv0);
	dmPerror (s);
	die ();
    }
}
