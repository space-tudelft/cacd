/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	Pieter van der Wolf
 *	Simon de Graaf
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
#include <time.h>
#include "src/ocean/seadali/header.h"

extern char *argv0;
extern  DM_PROJECT * dmproject;
extern FILE *ErrorMsgFile; /* file where dmError writes its messages */
extern int DmErrorIsFatal; /* FALSE if a call do dmError should return */

extern int  rmode;
extern int  d_apollo;
extern char *DisplayName;
extern char *geometry;
int     graphic_mode = 0;	/* not in graphic mode */

extern char Input_cell_name[];	/* name of pre-specified cell */
extern char cirname[];

static void setup_dmerror ();
static void initialize_clk_tck ();
extern long Clk_tck;

#if defined(SIGALRM)
/* this function does nothing special, it just ignores the SIGALRM */
void alarm_handler (int sig)
{
    signal (SIGALRM, alarm_handler);
}
#endif

void sig_handler (int sig) /* signal handler */
{
    char str[MAXCHAR];
    signal (sig, SIG_IGN);	/* ignore signal */
    sprintf (str, "Program interrupted! (sig = %d)", sig);
    ptext (str);
    sleep (3);
    stop_show (1);
}

static void usage ()
{
    PE "\nUsage: %s [-A] [-r] [-h host:dnr[.snr]] [=geo] [-c <cir_cell_name>] [<cell_name>]\n\n", argv0);
    exit (1);
}

void real_main (int argc, char *argv[])
{
#ifdef SIGQUIT
    void    (*istat) ();
#endif
    int     i;

    argv0 = "seadali";

    strcpy (Input_cell_name, ""); /* default: no input cell name */
    strcpy (cirname, ""); /* default cirname off */

    initialize_clk_tck (); /* initialize the clock tick for statistics */

    setup_dmerror (); /* setup the behavior of dmError() */

    DisplayName = NULL;
    geometry = NULL;
    /* I think the following is no longer necessary (Arjan v G) */
 // system ("setcmap -3"); /* PATRICK: ugly, but its effective to get better color management */

    for (i = 1; i < argc; ++i) {
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {
		case 'r':
		    rmode = 1;	/* read only mode */
		    break;
		case 'A':
		    d_apollo = 1;/* Apollo mode */
		    break;
		case 'h':
		    DisplayName = argv[++i];
		    if (!DisplayName) usage ();
		    break;
		case 'c':
	            if (i + 1 >= argc)
		        usage ();
                    else if (strlen (argv[++i]) > DM_MAXNAME) {
		        fprintf (stderr, "input name '%s' is too long\n", argv[i]);
		        usage ();
                    }
                    else {
                        strcpy (cirname, argv[i]);
                    }
		    break;
		default:
		    usage ();
	    }
	}
	else if (argv[i][0] == '=') geometry = argv[i];
	else {
	    /* cell name */
	    if (strlen (argv[i]) > DM_MAXNAME) {
		fprintf (stderr, "input name '%s' is too long\n", argv[i]);
		usage ();
	    }
	    strcpy (Input_cell_name, argv[i]);
	}
    }

    get_gterm ();

#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN);
#endif
    signal (SIGINT, SIG_IGN);
#ifdef SIGQUIT
    istat = signal (SIGQUIT, SIG_IGN);
#endif
    signal (SIGTERM, sig_handler);
#if defined(SIGALRM)
/* for some reason, we receive unexpected SIGALRMs on linux ... */
    signal (SIGALRM, alarm_handler);
#endif

 /* initialisation */

    if (dmInit (argv0)) {
	fprintf (stderr, "Uhhh.... Are you sure that you're in a project directory??\n");
	exit (1);
    }

    dmproject = dmOpenProject (DEFAULT_PROJECT, rmode ? PROJ_READ : (PROJ_READ | PROJ_WRITE));
    if (!dmproject || chdir (dmproject -> dmpath)) {
	fprintf (stderr, "Uhhh.... Are you sure that you're in a project directory??\n");
	stop_show (1);
    }
    if (!dmGetMetaDesignData (PROCESS, dmproject)) stop_show (1);

    graphic_mode = 1;

    init_graph ();
    initwindow ();
    init_txtwdw ("the Delft Advanced Layout Interface");

    init_colmenu ();
    init_mem ();

 /* enable interrupt catching, if not ignored */
#ifdef SIGQUIT
    if (istat != SIG_IGN) signal (SIGQUIT, sig_handler);
#endif
    signal (SIGSEGV, sig_handler);

    command ();			/* let's work */
 /* function command does not return */
}

void stop_show (int exitstatus)
{
    dmQuit ();
    if (graphic_mode) exit_graph ();
    exit (exitstatus);
}

void fatal (int Errno, char *str)
{
    dmQuit ();
    if (graphic_mode) exit_graph ();
    PE "%s: fatal error %d in routine %s\n", argv0, Errno, str);
    exit (1);
}

#if 0

void dmError (char *s)
{
    char err_str[MAXCHAR];

    if (!graphic_mode) {
	PE "%s: ", argv0);
	dmPerror (s);
	PE "%s: error in DMI function\n", argv0);
	return;
    }
    if (!s) s = "";
    if (dmerrno > 0 && dmerrno < dmnerr)
	sprintf (err_str, "%s: %s", s, dmerrlist[dmerrno]);
    else
	sprintf (err_str, "%s: Unknown DMI error (no = %d)!", s, dmerrno);
    ptext (err_str);
    fprintf (stderr, "%s: DMI error: %s\n", argv0, err_str);
}

#endif /* 0 */

void print_assert (char *file_str, char *line_str)
{
    char ass_str[MAXCHAR];
    sprintf (ass_str, "Assertion failed! (file %s, line %s)\n", file_str, line_str);
    ptext (ass_str);
}

static void initialize_clk_tck ()
{
#ifdef CLK_TCK
    Clk_tck = CLK_TCK;
#else
    Clk_tck = sysconf (_SC_CLK_TCK);
#endif
}

/* This function sets up the behavior of dmError(), refer to nelsea/nelsis.c */
static void setup_dmerror ()
{
    if (!(ErrorMsgFile = fopen ("seadif/seadali.errors", "w"))) ErrorMsgFile = stderr;
    DmErrorIsFatal = FALSE; /* do NOT exit if a dm error occurs! */
}
