/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
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

#include "src/libddm/dmstd.h"
#include "signal.h"

#define P_E fprintf (stderr,

void ck_proj_loop (char *path);
void die (int status);

int     cflag = 0;

char   *projlist = "./projlist";
char   *projpath;
char   *cp_path;
char    buf[MAXLINE + 60];
char  Plist[MAXLINE + 60];

char   *argv0 = "addproj";	/* program name */
char   *use_msg =		/* command line */
        "\nUsage: %s [-c] project\n\n";

int main (int argc, char *argv[])
{
    FILE *fp;
    DM_PROJECT *lib_key, *cp_key;
    char *lib_path;
    int   proj_cnt;

    if (!(argc == 2 || argc == 3)) {
	P_E use_msg, argv0);
	exit (1);
    }

    if (argv[1][0] == '-' && argv[1][1] == 'c') {
	cflag = 1;
	*argv++;
    }
    projpath = argv[1];

    if (strlen (projpath) >= MAXLINE) {
	P_E "%s: too long project path name\n", argv0);
	exit (1);
    }

 /* ignore all signals */
#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN);
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);
#endif
    signal (SIGTERM, SIG_IGN);
    signal (SIGINT, SIG_IGN);

    if (dmInit (argv0) == -1) exit (1);

    if (!(lib_key = dmOpenProject (projpath, PROJ_READ))) {
	P_E "%s: %s is no valid project\n", argv0, projpath);
	die (1);
    }
    lib_path = lib_key -> dmpath;

    if (!(cp_key = dmOpenProject (".", PROJ_WRITE))) {
	P_E "%s: the cwd is no valid project\n", argv0);
	die (1);
    }
    cp_path = cp_key -> dmpath;

    if (lib_key == cp_key) {
	P_E "%s: current project can not be added\n", argv0);
	die (1);
    }

    if (!cflag) {
	if (lib_key -> procid != cp_key -> procid) {
	    P_E "%s: process id's are not equal\n", argv0);
	    die (1);
	}

	if (lib_key -> lambda != cp_key -> lambda) {
	    P_E "%s: process lambda values are not equal\n", argv0);
	    die (1);
	}
    }

    if (!(fp = fopen (projlist, "r+"))) {
	P_E "%s: cannot open file: %s\n", argv0, projlist);
	die (1);
    }

    proj_cnt = 0;
    while (fscanf (fp, "%s", buf) != EOF) {
	++proj_cnt;
	if (strcmp (buf, lib_path) == 0) {
	    P_E "%s: %s already in projlist\n", argv0, lib_path);
	    die (1);
	}
    }

    if (++proj_cnt > DM_MAXPROJECTS) {
	P_E "%s: max projects reached, project not added\n", argv0);
	die (1);
    }

    ck_proj_loop (lib_path);

    fseek (fp, 0L, 2);
    fprintf (fp, "%s\n", lib_path);
    fclose (fp);

    die (0);
    return (0);
}

void ck_proj_loop (char *path)
{
    FILE *fp;

    _dmSprintf (Plist, "%s/projlist", path);

    if (!(fp = fopen (Plist, "r"))) {
	P_E "%s: cannot read file: %s\n", argv0, Plist);
	die (1);
    }

    while (fscanf (fp, "%s", buf) != EOF) {
	if (strcmp (buf, cp_path) == 0) {
	    P_E "%s: current project already in %s\n", argv0, Plist);
	    die (1);
	}
	ck_proj_loop (buf);
    }

    fclose (fp);
}

void dmError (char *s)
{
    P_E "%s: ", argv0);
    dmPerror (s);
}

void die (int status)
{
    dmQuit ();
    if (status) P_E "%s: -- program aborted --\n", argv0);
    exit (status);
}
