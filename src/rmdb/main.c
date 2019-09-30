/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
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
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>

#include "src/libddm/dmincl.h"

void die (int status);
int dodir (int len);
int rem_cell (char *cell, char *view);
int rem_view (char *view);

int     sw_verb = 0;
int     sw_imp = 0;
int     sw_force = 0;
int     no_exit = 0;
int     usage = 0;
int     sw_all = 0;
int     sw_lay = 0;
int     sw_cir = 0;
int     sw_flp = 0;
int     sw_cell = 0;
char   *projname = NULL;
char ** viewlist = NULL;
char  * cells[40];

DM_PROJECT * project;
char    path[80];

char   *argv0 = "rmdb";		/* program name */
char   *use_msg =		/* command line */
"\nUsage: %s [-aifv] [-c cellname] [-p project] [layout] [circuit] [floorplan]\n\n";

int main (int argc, char *argv[])
{
    char *p;
    int iarg, status;

    for (iarg = 1; iarg < argc && (p = argv[iarg]); ++iarg) {
	if (*p == '-') {
	    while (*++p) {
		switch (*p) {
		case 'a':
		    ++sw_all;
		    break;
		case 'i':
		    ++sw_imp;
		    break;
		case 'v':
		    ++sw_verb;
		    break;
		case 'f':
		    ++sw_force;
		    break;
		case 'c':
		    if (!argv[++iarg]) {
			fprintf (stderr, "%s: -c: missing cell name\n", argv0);
			fprintf (stderr, use_msg, argv0);
			die (1);
		    }
		    if (sw_cell == 40) {
			fprintf (stderr, "%s: -c: too many cell names\n", argv0);
			die (1);
		    }
		    cells[sw_cell++] = argv[iarg];
		    break;
		case 'p':
		    if (projname) {
			fprintf (stderr, "%s: -p: project already specified\n", argv0);
			fprintf (stderr, use_msg, argv0);
			die (1);
		    }
		    if (!(projname = argv[++iarg])) {
			fprintf (stderr, "%s: -p: missing project path\n", argv0);
			fprintf (stderr, use_msg, argv0);
			die (1);
		    }
		    break;
		default:
		    ++usage;
		    fprintf (stderr, "%s: -%c: unknown option\n", argv0, *p);
		    break;
		}
	    }
	}
	else if (strncmp (p, "lay", 3) == 0) ++sw_lay;
	else if (strncmp (p, "cir", 3) == 0) ++sw_cir;
	else if (strncmp (p, "flo", 3) == 0) ++sw_flp;
	else {
	    ++usage;
	    fprintf (stderr, "%s: %s: unknown view-type\n", argv0, p);
	}
    }

    if (usage || !(sw_all || sw_lay || sw_cir || sw_flp)) {
	if (sw_cell && !(sw_all || sw_lay || sw_cir || sw_flp)) {
	    fprintf (stderr, "%s: -c: must be combined with -a or view-type(s)\n", argv0);
	}
	fprintf (stderr, use_msg, argv0);
	if (argc < 2) return (0);
	die (1);
    }

    if (sw_cell && sw_imp) {
	fprintf (stderr, "%s: -i: cannot be combined with option -c\n", argv0);
	die (1);
    }

    if (sw_all) {
	if (sw_lay || sw_cir || sw_flp) sw_all = 0;
	else sw_lay = sw_cir = sw_flp = 1;
    }

    /* dmOpenProject() requires an absolute path else CWD is used */
    if (!projname) projname = DEFAULT_PROJECT;
    else if (getenv ("CWD")) {
	putenv ("CWD"); /* unsetenv */
	if (getenv ("CWD")) {
	    fprintf (stderr, "%s: cannot unsetenv CWD\n", argv0);
	    die (1);
	}
    }

    dmInit (argv0);
    project = dmOpenProject (projname, DEFAULT_MODE);
    chdir (project -> dmpath);

    if (!sw_force) {
	fprintf (stderr, "%s: using project %s\n", argv0, project -> dmpath);
	fprintf (stderr, "%s: removing %s cell(s) of %s view(s), are you sure? [y/n]: ",
	    argv0, sw_cell ? "specified" : "ALL", sw_all ? "ALL" : "specified");
	if (getchar() != 'y') die (1);
    }

#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN); /* ignore hangup signal */
#endif
    signal (SIGINT, SIG_IGN); /* ignore interrupt */

    status = 0;

    if (!sw_cell) { /* remove all cells */
	if (sw_lay && rem_view (LAYOUT)) status = 1;
	if (sw_cir && rem_view (CIRCUIT)) status = 1;
	if (sw_flp && rem_view (FLOORPLAN)) status = 1;
    }
    else { /* remove all specified cells */
	no_exit = 1;
	for (iarg = 0; iarg < sw_cell; ++iarg) {
	    if (sw_lay && rem_cell (cells[iarg], LAYOUT)) status = 2;
	    if (sw_cir && rem_cell (cells[iarg], CIRCUIT)) status = 2;
	    if (sw_flp && rem_cell (cells[iarg], FLOORPLAN)) status = 2;
	}
    }

    die (status); /* end of main */
    return (0);
}

void dmError (char *s)
{
    fprintf (stderr, "%s: ", argv0);
    dmPerror (s);
    if (no_exit == 0) {
	fprintf (stderr, "%s: error in DMI function\n", argv0);
	die (1);
    }
}

void die (int status)
{
    if (!sw_force) {
	fprintf (stderr, "%s: -- program %s --\n", argv0, status == 1 ? "aborted" : "finished");
    }
    if (project) dmCloseProject (project, QUIT);
    dmQuit ();
    exit (status);
}

int rem_cell (char *cell, char *view)
{
    if (sw_verb) fprintf (stderr, "%s: removing cell %s of view '%s'\n", argv0, cell, view);

    if (dmRemoveCell (project, cell, ACTUAL, DONTCARE, view)) {
	fprintf (stderr, "%s: warning: cannot remove cell '%s' of view '%s'\n", argv0, cell, view);
	return (1);
    }
    return (0);
}

char pathname[DM_MAXPATHLEN];
static int level;
struct stat st_buf;

int rem_view (char *view)
{
    if (sw_verb) fprintf (stderr, "%s: removing all cells of view '%s'\n", argv0, view);

    if (strcmp (view, LAYOUT) == 0) {
	unlink ("exp_dat");
	unlink ("display.out");
    }

    if (lstat (view, &st_buf)) {
	fprintf (stderr, "%s: cannot access view %s\n", argv0, view);
	return 1;
    }
    if (S_ISDIR (st_buf.st_mode)) {
	level = 0;
	strcpy (pathname, view);
	return dodir (strlen (pathname));
    }
    return 0;
}

int dodir (int len)
{
    DIR  *dp;
    FILE *fp;
    struct dirent *e;
    char *s;
    int k, status = 0;

    if (!(dp = opendir (pathname))) {
	fprintf (stderr, "%s: cannot opendir %s\n", argv0, pathname);
	return 2;
    }
    pathname[len] = '/';

    while ((e = readdir (dp))) {
	s = e -> d_name;
	if (!strcmp (s, ".") || !strcmp (s, "..")) continue;
	k = strlen (s) + len + 1;
	if (k >= DM_MAXPATHLEN) {
	    fprintf (stderr, "%s: pathname too long >= %d\n", argv0, DM_MAXPATHLEN);
	    return 2;
	}
	strcpy (pathname+len+1, s);

	if (lstat (pathname, &st_buf)) {
	    fprintf (stderr, "%s: warning: cannot access %s\n", argv0, pathname);
	    status = 1;
	    continue;
	}
	if (S_ISDIR (st_buf.st_mode)) { /* directory */
	    ++level;
	    if ((k = dodir (k)) > 1) return k;
	    --level;
	    if (sw_verb > 1) fprintf (stderr, "%s: removing %s\n", argv0, pathname);
	    if (rmdir (pathname)) {
		fprintf (stderr, "%s: cannot remove %s %s\n", argv0, level ? "directory" : "cell", pathname);
		return 2;
	    }
	    if (k) status = 1;
	}
	else { /* file */
	    if (!level && (strcmp (s, "celllist") == 0 || strcmp (s, "impcelllist") == 0)) {
		if (st_buf.st_size == 0) continue;
		if (*s == 'i' && !sw_imp) continue;
		if (sw_verb > 1) fprintf (stderr, "%s: cleaning %s\n", argv0, pathname);
		if (!(fp = fopen (pathname, "w"))) {
		    fprintf (stderr, "%s: cannot rewrite %s\n", argv0, pathname);
		    status = 1;
		}
		else fclose (fp);
	    }
	    else {
		if (sw_verb > 2) fprintf (stderr, "%s: removing %s\n", argv0, pathname);
		if (unlink (pathname)) {
		    fprintf (stderr, "%s: cannot remove file %s\n", argv0, pathname);
		    return 2;
		}
	    }
	}
    }

    pathname[len] = 0;
    closedir (dp);
    return status;
}
