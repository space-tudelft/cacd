/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	P. van der Wolf
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

/*
** The rmpr (remove project) command should be used by the designer to
** remove an empty project (i.e. a project that contains no cells).
** It shows much resemblance with the UNIX rmdir command.
** If the directory is a project directory, as verified by dmOpenProject(),
** rmpr checks whether it contains no cells. If so, it removes
** the files .dmrc and projlist.
** It also removes the view directories with the files celllist and impcelllist.
** If this causes the UNIX directory to become empty, rmpr will
** also remove the project directory.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include "src/libddm/dmincl.h"
#include "signal.h"

void die (int status);
int empty_view (char *view);
void rm_all (char *dir);
int rmfile (char *name);

#ifdef BSD
extern char *optarg;
extern int optind, opterr;
#endif

int level = 0;
int forceIfCells = 0;
int silence = 0;
int verbose = 0;
int pathlen = 0;

struct stat st_buf;
#define MAXLPATH DM_MAXPATHLEN+DM_MAXPATHLEN
char  pathname[MAXLPATH];

char *argv0 = "rmpr";	/* program name */
char *use_msg =		/* command line */
"\nUsage: %s [-fsv] project\n\n";

int main (int argc, char *argv[])
{
    DM_PROJECT * proj_key;
    static char *view_str[] = {
	LAYOUT,
	CIRCUIT,
	FLOORPLAN
    };
    int i, errflg = 0;

    while ((i = getopt (argc, argv, "fsv")) != -1) {
        switch (i) {
            case 'f': forceIfCells = 1; break;
            case 's': silence = 1; break;
            case 'v': ++verbose; break;
            case '?': errflg++; break;
        }
    }

    if (errflg) {
        fprintf (stderr, use_msg, argv0);
        exit (1);
    }

    if (optind != (argc - 1)) {
        fprintf (stderr, use_msg, argv0);
        exit (1);
    }

    /* dmOpenProject() requires an absolute path else CWD is used */
    if (getenv ("CWD")) {
	putenv ("CWD"); /* unsetenv */
	if (getenv ("CWD")) {
	    fprintf (stderr, "%s: cannot unsetenv CWD\n", argv0);
	    die (1);
	}
    }

    /* Ignore signals. */
#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN);
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);
#endif
    signal (SIGTERM, SIG_IGN);
    signal (SIGINT, SIG_IGN);

    /* now initialize and open project: .dmrc will be read
    ** and a lock will be set
    */
    dmInit (argv0);
    proj_key = dmOpenProject (argv[optind], PROJ_WRITE);

    pathlen = strlen (proj_key -> dmpath);
    if (pathlen > DM_MAXPATHLEN) {
	fprintf (stderr, "%s: too long absolute project path\n", argv0);
	die (1);
    }
    strcpy (pathname, proj_key -> dmpath);

    /* dmOpenProject succeeded, so the directory is a project.
    ** See if the different views are empty.
    */
    for (i = 0; i < sizeof (view_str)/sizeof (char *); i++) if (!empty_view (view_str[i])) {
	errflg++;
	if (!silence) fprintf (stderr, "%s: %s: not empty\n", argv0, pathname);
    }
    if (errflg) {
	if (!forceIfCells) {
	    if (!silence) fprintf (stderr, "%s: use option -f to remove a project that contains cells\n", argv0);
	    die (1);
	}
	if (!silence) {
	    fprintf (stderr, "%s: force removal of ALL cells, are you sure? [y/n]: ", argv0);
	    if (getchar() != 'y') die (1);
	}
    }

    /* The celllists are empty or forceIfCells == TRUE.
    ** Now remove .dmrc and projlist, thereby turning the project
    ** directory into an ordinary directory.
    ** Then try to remove the view directories.
    */
    /* .dmrc was already found by dmOpenProject() */
    if (rmfile (".dmrc") == -1) {
	fprintf (stderr, "%s: %s: cannot remove\n", argv0, pathname);
	die (1);
    }
    rmfile (".dmxdata"); /* XCONTROL */
    rmfile ("projlist");
    rmfile ("exp_dat");
    rmfile ("display.out");

    for (i = 0; i < sizeof (view_str)/sizeof (char *); i++) {
	rm_all (view_str[i]);
    }

    rm_all ("seadif"); /* OCEAN */
    rm_all ("sls_prototypes");
    rmfile (".cslsrc");
    rmfile (".dalirc");
    rmfile (".seadalirc");
    rmfile (".simeyerc");
    rmfile (".space.def.p");

    /* Now the project directory might be empty. Try to remove it
    ** by a rmdir. If this not succeeds the directory is probably
    ** not empty (we already know pathname is a legal directory).
    ** No further actions have to be taken in this case.
    */
    pathname[pathlen] = 0;
    if (verbose) printf ("%s: dir. %s\n", argv0, pathname);
    if (rmdir (pathname)) {
	if (!silence) fprintf (stderr, "%s: %s: project not removed, not empty\n", argv0, pathname);
	die (1);
    }
    /* if following unlink is successful, it must be a symbolic link */
    if (unlink (argv[optind]) == 0 && !silence && verbose) {
	printf ("%s: removed dir. link %s\n", argv0, argv[optind]);
    }
    die (0);
    return (0);
}

void dmError (char *s)
{
    fprintf (stderr, "%s: ", argv0);
    dmPerror (s);
    fprintf (stderr, "%s: error in DMI function\n", argv0);
    die (1);
}

void die (int status)
{
    fprintf (stderr, "%s: -- program %s --\n", argv0, status ? "aborted" : "finished");
    dmQuit ();
    exit (status);
}

int empty_view (char *view)
{
    DIR *dp = NULL;
    struct dirent *e;
    char *s;
    int k;

    pathname[pathlen] = '/';
    strcpy (pathname+pathlen+1, view);

    k = lstat (pathname, &st_buf);
    if (!k && !S_ISDIR (st_buf.st_mode)) return 1;

    if (k || !(dp = opendir (pathname))) {
	if (forceIfCells) return 1;
	fprintf (stderr, "%s: %s: cannot access\n", argv0, pathname);
	die (1);
    }
    k = 1;
    while ((e = readdir (dp))) {
	s = e -> d_name;
	if (!strcmp (s, ".")) continue;
	if (!strcmp (s, "..")) continue;
	if (!strcmp (s, "celllist")) continue;
	if (!strcmp (s, "impcelllist")) continue;
	k = 0; /* not empty */
	break;
    }
    closedir (dp);
    return k;
}

void dodir (int len)
{
    DIR *dp;
    struct dirent *e;
    char *s;
    int k;

    if (!(dp = opendir (pathname))) return;
    pathname[len] = '/';

    ++level;
    while ((e = readdir (dp))) {
	s = e -> d_name;
	if (!strcmp (s, ".") || !strcmp (s, "..")) continue;
	k = strlen (s) + len + 1;
	if (k >= MAXLPATH) {
	    fprintf (stderr, "%s: pathname too long >= %d\n", argv0, MAXLPATH);
	    die (1);
	}
	strcpy (pathname+len+1, s);
	if (lstat (pathname, &st_buf)) {
	    fprintf (stderr, "%s: %s: cannot access\n", argv0, pathname);
	    die (1);
	}
	if (S_ISDIR (st_buf.st_mode)) dodir (k);
	else {
	    if (verbose > level) printf ("%s: file %s\n", argv0, pathname);
	    unlink (pathname);
	}
    }
    --level;

    pathname[len] = 0;
    closedir (dp);
    if (verbose) printf ("%s: dir. %s\n", argv0, pathname);
    rmdir (pathname);
}

void rm_all (char *dir)
{
    pathname[pathlen] = '/';
    strcpy (pathname+pathlen+1, dir);
    if (lstat (pathname, &st_buf)) return;
    if (S_ISDIR (st_buf.st_mode)) dodir (pathlen+1 + strlen (dir));
    else {
	if (verbose) printf ("%s: dir. %s\n", argv0, pathname);
	unlink (pathname);
    }
}

int rmfile (char *name)
{
    pathname[pathlen] = '/';
    strcpy (pathname+pathlen+1, name);
    if (lstat (pathname, &st_buf)) return -1;
    if (verbose > 1) printf ("%s: file %s\n", argv0, pathname);
    return unlink (pathname);
}
