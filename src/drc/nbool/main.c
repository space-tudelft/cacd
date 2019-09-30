/*
 * ISC License
 *
 * Copyright (C) 1982-2018 by
 *	T.G.R. van Leuken
 *	J. Liedorp
 *	S. de Graaf
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

#include "src/drc/nbool/nbool.h"

#define USAGE fprintf(stderr,use_msg,argv0);exit(1)
DM_PROJECT * dmproject;

static void bool_cell (char *modelname, int file_flag);
static void sig_handler (int sig);

char   *argv0 = "nbool";	/* program name */
char   *use_msg =		/* command line */
        "\nUsage: %s [-c|-n] [-f] [cell_name]\n\n";

int main (int argc, char *argv[])
{
    char    modelname[DM_MAXNAME + 1];
    int     iarg;		/* argument number */
    char   *p;
    int     file_flag;
    int     cell_name_flag;
    char    excl_option;

    excl_option = '\0';
    pid = getpid ();
    chk_flag = CHK_HRCHY;
    file_flag = OFF;
    cell_name_flag = OFF;

    for (iarg = 1; iarg < argc && argv[iarg][0] == '-'; ++iarg) {
	for (p = &argv[iarg][1]; *p; ++p) {
	    switch (*p) {
		case 'c':
		    if (excl_option == 'n') {
			fprintf (stderr, "\nthe -c and -n options are mutually exclusive");
			USAGE;
		    }
		    chk_flag = CHK_HRCHY;
		    excl_option = 'c';
		    break;
		case 'f':
		    file_flag = ON;
		    break;
		case 'n':
		    if (excl_option == 'c') {
			fprintf (stderr, "\nthe -c and -n options are mutually exclusive");
			USAGE;
		    }
		    chk_flag = NO_CHK_HRCHY;
		    excl_option = 'n';
		    break;
		default:
		    fprintf (stderr, "\nunknown option: %c", *p);
		    USAGE;
	    }
	}
    }
    if (iarg == argc - 1)
	cell_name_flag = ON;
    if (iarg < (argc - 1)) {
	if (argv[argc - 1][0] == '-')
	    fprintf (stderr, "\ncell_name must be the last argument");
	else
	    fprintf (stderr, "\ntoo many arguments");
	USAGE;
    }
#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN);	/* ignore hangup signal */
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);
#endif
    signal (SIGTERM, sig_handler);

    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	signal (SIGINT, sig_handler);

    dmInit (argv0);
    if ((dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE)) == NULL)
	exit (1);

    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, dmproject);

    mask_terms = 0;

    if (cell_name_flag == OFF) {

    /* perform nbool for each model from the EXPDATA file */

	sprintf (filename, "%s/%s", dmproject -> dmpath, EXPDATA);
	OPEN (pexp, filename, "r");
	while (fscanf (pexp, "%s", modelname) != EOF) {
	    bool_cell (modelname, file_flag);
	}
    }
    else {
	bool_cell (argv[argc - 1], file_flag);
    }
    die (0);
    return (0);
}

static void bool_cell (char *modelname, int file_flag)
{
    register struct form *f_pntr; /* ptr to form struct */
#ifdef DRCERRORSTREAM
    DM_STREAM *drcstream;
#endif

    mod_key = dmCheckOut (dmproject, modelname, WORKING, DONTCARE, LAYOUT, ATTACH);
#ifdef DRCERRORSTREAM
    drcstream = dmOpenStream (mod_key, "nbool", "w");
    pout = drcstream->dmfp;
#else
    pout = stdout;
#endif
    fprintf (pout, "\ncell: %s\n\n", modelname);
    mk_formstruct (file_flag);
    main_bool ();
    free_state_ruler ();
    f_pntr = fp_head;
    while (f_pntr) {
	add_grpnbr (f_pntr -> f_nbr);
	f_pntr = f_pntr -> next;
    }
    free_formstruct ();
#ifdef DRCERRORSTREAM
    dmCloseStream (drcstream, COMPLETE);
#endif
    dmCheckIn (mod_key, COMPLETE);
}

static void sig_handler (int sig) /* signal handler */
{
    signal (sig, SIG_IGN); /* ignore signal */
    fprintf (stderr, "%s: interrupted due to signal: %d\n", argv0, sig);
    die (1);
}

void dmError (char *s)
{
    fprintf (stderr, "%s: ", argv0);
    dmPerror (s);
    fprintf (stderr, "%s: error in dbm interface function\n", argv0);
    die (1);
}

void die (int status)
{
    register struct form *f_pntr; /* ptr to form struct */

    if (status) {
	f_pntr = fp_head;
	while (f_pntr) {
	    sprintf (fr_name, TEMP_ONE, f_pntr -> f_nbr, pid);
	    unlink (fr_name);
	    f_pntr = f_pntr -> next;
	}
	unlink (fi_name);
	fprintf (stderr, "%s: -- program aborted --\n", argv0);
    }
    if (dmproject) dmCloseProject (dmproject, COMPLETE);
    dmQuit ();
    exit (status);
}
