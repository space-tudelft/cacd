/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	P. van der Wolf
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

#include <signal.h>
#include "src/dali/header.h"

extern char *argv0;
extern DM_PROJECT *dmproject;

extern int  fmode;
extern int  rmode;
extern int  omode;

extern int  colormap;
extern int  d_apollo;
extern char *DisplayName;
extern char *geometry;

int   NOdm_msg = 0;
int   graphic_mode = 0; /* not in graphic mode */
char *Input_cell_name = NULL; /* name of pre-specified cell */

static void usage ()
{
    PE "\nUsage: %s [-A] [-C] [-f] [-o] [-h host:d#[.s#]] [=geo] [cell_name]\n\n", argv0);
    exit (1);
}

int main (int argc, char *argv[])
{
    int  i;
    void sig_handler ();
    void (*istat) ();

    argv0 = "dali";
    DisplayName = NULL;
    geometry = NULL;

    for (i = 1; i < argc; ++i) {
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {
		case 'f':
		    fmode = 1;	/* read dali_drc from CWD */
		    break;
		case 'o':
		    omode = 1;	/* animate output mode */
		    break;
		case 'r':
		    rmode = 1;	/* read only mode */
		    break;
		case 'A':
		    d_apollo = 1;/* Apollo mode */
		    break;
		case 'C':
		    colormap = 0;/* don't use Colormap */
		    break;
		case 'h':
		    DisplayName = argv[++i];
		    if (!DisplayName) usage ();
		    break;
		default:
		    usage ();
	    }
	}
	else {
	    if (argv[i][0] == '=') {
		geometry = argv[i];
	    }
	    else {
		Input_cell_name = argv[i];
	    }
	}
    }

    if (Input_cell_name) {
	if (strlen (Input_cell_name) > DM_MAXNAME) {
	    PE "input name '%s' is too long\n", Input_cell_name);
	    usage ();
	}
    }

    get_gterm ();

    signal (SIGHUP, SIG_IGN);
    signal (SIGINT, SIG_IGN);
    istat = signal (SIGQUIT, SIG_IGN);
    signal (SIGTERM, sig_handler);

    if (dmInit (argv0)) exit (1);

    dmproject = dmOpenProject (DEFAULT_PROJECT, rmode ? PROJ_READ : (PROJ_READ | PROJ_WRITE));

    if (!dmproject) stop_show (1);
    if (!dmGetMetaDesignData (PROCESS, dmproject)
        || dmproject -> maskdata == NULL
        || dmproject -> maskdata -> nomasks == 0) stop_show (1);

    if (!rmode) {
	char old_path[MAXCHAR], new_path[MAXCHAR];
	sprintf (old_path, "%s/.dmrc", dmproject -> dmpath);
	sprintf (new_path, "%s/.dmrc%d", dmproject -> dmpath, (int)getpid ());
	if (link (old_path, new_path) == -1) {
	    PE "%s: no write permission: read only mode!\n", argv0);
	    rmode = 1;
	}
	unlink (new_path);
    }

    graphic_mode = 1;

    open_dalirc ();
    init_graph ();
    initwindow ();
    save_oldw (); /* there is no prev_w */
    init_txtwdw ("Starting...");
    init_colmenu ();
    init_mem ();

 /* enable interrupt catching, if not ignored */
    if (istat != SIG_IGN) signal (SIGQUIT, sig_handler);
    signal (SIGSEGV, sig_handler);

    command ();			/* let's work */
 /* function command does not return */

    return (0);
}

void stop_show (int exitstatus)
{
    dmQuit ();
    exit (exitstatus);
}

void fatal (int Errno, char *str)
{
    dmQuit ();
    PE "%s: fatal error %d in routine %s\n", argv0, Errno, str);
    exit (1);
}

void dmError (char *s)
{
    char err_str[MAXCHAR];

    if (NOdm_msg) return;

    if (!graphic_mode) {
	PE "%s: ", argv0);
	dmPerror (s);
	PE "%s: error in DMI function\n", argv0);
	return;
    }

    if (dmerrno > 0 && dmerrno <= dmnerr)
	sprintf (err_str, "%s: %s", s, dmerrlist[dmerrno]);
    else
	sprintf (err_str, "%s: Unknown DMI error (no = %d)!", s, dmerrno);
    btext (err_str);
    sleep (3);
}

void sig_handler (int sig) /* signal handler */
{
    char str[MAXCHAR];
    signal (sig, SIG_IGN); /* ignore signal */
    sprintf (str, "Program interrupted! (sig = %d)", sig);
    ptext (str);
    sleep (3);
    stop_show (1);
}

void print_assert (char *file_str, char *line_str)
{
    char str[MAXCHAR];
    sprintf (str, "Assertion failed! (file %s, line %s)\n", file_str, line_str);
    ptext (str);
}

#ifdef STATIC
/* libX11.a fix for static linking */
void *dlopen (const char *filename, int flag)
{
    return NULL;
}
#endif
