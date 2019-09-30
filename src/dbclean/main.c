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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "src/libddm/dmincl.h"

#define RELEASE_NUM 302
void update_dmrc (void);
void update_nor (void);
void clean_layout_view (void);
void unlink_2nd_files (char *cell);
void do_chdir (char *path);
void do_unlink (char *file);
void clean_circuit_view (void);
void clean_floorplan_view (void);
void sig_handler (int sig);
void errexit (int err_no, char *s);

DM_PROJECT *project;	/* project access key */
char buf[DM_MAXNAME+11]; /* string buffer */

int     noExit = 0;
int     oldproj = 0;	/* old project mode (no xdata) */
int     eflag;	/* exclusive cells in "exp_dat" file */
int     lflag;	/* only "layout" view */
int     cflag;	/* only "circuit" view */
int     allflag;	/* do all cells of specified viewtypes */
int     fflag;	/* only "floorplan" view */
int     uflag;	/* project update mode */
int     vflag;	/* verbose */
int     Vflag;	/* Verbose */
char  **viewlist = NULL;
char  **specml = NULL;
char   *filename = "?";
char   *Layout = LAYOUT;
char   *Circuit = CIRCUIT;
char   *Floorplan = FLOORPLAN;

char   *argv0 = "dbclean";	/* program name */
char   *use_msg =		/* command line */
"\nUsage: %s [-aelcfuvV] [cell ...]\n\n";

int main (int argc, char *argv[])
{
    char   *p;
    int     iarg;	    /* argument number */
    int     usage = 0;

    for (iarg = 1; iarg < argc && argv[iarg][0] == '-'; ++iarg) {
	p = &argv[iarg][1];
	while (*p != '\0') {
	    switch (*p++) {
		case 'u':
		    ++uflag;
		case 'a':
		    ++allflag;
		    break;
		case 'e':
		    ++eflag;
		    break;
		case 'l':
		    ++lflag;
		    break;
		case 'c':
		    ++cflag;
		    break;
		case 'f':
		    ++fflag;
		    break;
		case 'V':
		    ++Vflag;
		case 'v':
		    ++vflag;
		    break;
		default:
		    ++usage;
		    fprintf (stderr, "%s: -%c: unknown option\n", argv0, *(p - 1));
		    break;
	    }
	}
    }

    if (argc > iarg) { /* cell argument(s) specified */
	if (allflag) {
	    fprintf (stderr, "You must specify -a/-u or cell ...\n");
	    usage++;
	}
	else
	    specml = &argv[iarg];
    }

    if (!allflag && !specml) {
	fprintf (stderr, "You must specify -a/-u or cell ...\n");
	usage++;
    }

    if (usage) {
	fprintf (stderr, use_msg, argv0);
	exit (1);
    }

    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	signal (SIGINT, sig_handler);
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);
#endif
    signal (SIGTERM, sig_handler);
#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN); /* ignore hangup signal */
#endif

    dmInit (argv0);
    if (uflag) dm_extended_format = -1; /* hack */
    project = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);
{
    FILE *fp;
    if ((fp = dmOpenXData (project, "r"))) fclose (fp);
    else oldproj = 1;
}

    if (uflag && project -> release != 3 && project -> release != 301) {
	fprintf (stderr, "%s: cannot update release %d\n", argv0, project -> release);
	dmQuit ();
	exit (1);
    }

    if (uflag) eflag = 0;
    if (uflag || (!lflag && !cflag && !fflag)) {
	++lflag;
	++cflag;
	++fflag;
    }

    if (vflag) fprintf (stderr, "%s: %s\n", argv0, project -> dmpath);
    do_chdir (project -> dmpath);

    if (uflag) {
	noExit = 1;
	update_dmrc ();
	if (project -> release == 3) update_nor ();
	if (uflag) {
	    if (vflag) fprintf (stderr, "%s: update .dmrc\n", argv0);
	    if (rename (".dmrc2", ".dmrc")) errexit (0, "sorry: cannot rename .dmrc");
	}
	else
	    unlink (".dmrc2");
	noExit = 0;
    }

    if (lflag) clean_layout_view ();
    if (cflag) clean_circuit_view ();
    if (fflag) clean_floorplan_view ();

    if (vflag) fprintf (stderr, "%s: -- program finished --\n", argv0);

    dmQuit ();
    exit (0);
    return (0);
}

struct mo_elmt {
    char mo[DM_MAXNAME+1];
    struct mo_elmt *next;
};

struct mo_elmt *emlist;

void update_nor ()
{
    struct stat sbuf;
    FILE *fp1, *fp2;
    char **ml;
    char **mlfirst = NULL;
    char **mllast = NULL;
    int i, k;

    filename = "nor";

    if (_dmExistView (project, Layout) == 1) {
	ml = (char **) dmGetMetaDesignData (CELLLIST, project, Layout);
	if (ml)
	for (; *ml; ++ml)
	{
	    sprintf (buf, "%s/%s", Layout, *ml);
	    if (chdir (buf)) { errexit (-10, buf); goto notok2; }

	    if (stat (filename, &sbuf)) {
		if (vflag) errexit (-5, buf);
		if (!(fp2 = fopen ("nor", "wb"))) goto notok;
		fclose (fp2);
		goto skip;
	    }

	    if (sbuf.st_size == 0) goto skip;
	    if (!mlfirst) mlfirst = ml;
	    mllast = ml;

	    if (vflag) fprintf (stderr, "%s: update %s/%s\n", argv0, buf, filename);
	    if (rename ("nor", "norx")) goto notok;
	    if (!(fp1 = fopen ("norx","rb"))) goto notok;
	    if (!(fp2 = fopen ("nor", "wb"))) { fclose (fp1); goto notok; }
	    dmerrno = 0;
	    while (1) {
		dm_extended_format = 0; k = _dmGet_geo_data (fp1, GEO_NOR_INI);
		if (k <= 0) {
		    if (dmerrno) goto notok;
		    break;
		}
		dm_extended_format = 1; k = _dmPut_geo_data (fp2, GEO_NOR_INI); if (k < 0) goto notok;
		for (i = 0; i < gnor_ini.no_xy; i++) {
		    dm_extended_format = 0; k = _dmGet_geo_data (fp1, GEO_NOR_XY); if (k <= 0) goto notok;
		    dm_extended_format = 1; k = _dmPut_geo_data (fp2, GEO_NOR_XY); if (k < 0) goto notok;
		}
	    }
	    fclose (fp2);
	    fclose (fp1);
skip:
	    do_chdir (project -> dmpath);
	}
	return;
notok:
	errexit (-8, buf);
notok2:
	do_chdir (project -> dmpath);
	/* rewind the changes */
	for (ml = mlfirst; ml; ++ml)
	{
	    sprintf (buf, "%s/%s", Layout, *ml);
	    do_chdir (buf);
	    if (stat ("norx", &sbuf) == 0) {
		if (rename ("norx", "nor")) fprintf (stderr, "%s: %s: rename of norx failed\n", argv0, buf);
	    }
	    do_chdir (project -> dmpath);
	    if (ml == mllast) break;
	}
	fprintf (stderr, "%s: warning: cannot update release %d\n", argv0, project -> release);
	uflag = 0;
    }
    else {
	errexit (-3, Layout);
    }
}

void update_dmrc ()
{
    FILE *fp, *fp2;
    int c, nr;

#ifdef NOPACK
    errexit (0, "sorry: not packed program version");
#else
    if (!(fp  = fopen (".dmrc", "rb"))) errexit (0, "sorry: cannot read .dmrc");
    if (!(fp2 = fopen (".dmrc2","wb"))) errexit (0, "sorry: cannot write .dmrc");

    nr = 1;
    c = fgetc (fp);
    if (c != '3') goto err1;
    c = fgetc (fp);
    if (c != '\n') {
	if (c != '0') goto err1;
	c = fgetc (fp);
	if (c != '1') goto err1;
	c = fgetc (fp);
	if (c != '\n') goto err2;
    }
    fprintf (fp2, "%d\n", RELEASE_NUM);
    nr = 2;
    while ((c = fgetc (fp)) > ' ' && c < 127) fputc (c, fp2);
    if (c != '\n') goto err2;
    fputc (c, fp2);
    nr = 3;
    while ((c = fgetc (fp)) > ' ' && c < 127) fputc (c, fp2);
    if (c != '\n') goto err2;
    fputc (c, fp2);
    nr = 4;
    while ((c = fgetc (fp)) > ' ' && c < 127) fputc (c, fp2);
    if (c != '\n') goto err2;
    fputc (c, fp2);
    nr = 5;
    if (fgetc (fp) != EOF) goto err2;
    fclose (fp2);
    fclose (fp);
    return;
err1:
    fprintf (stderr, "%s: line %d of .dmrc: incorrect release number\n", argv0, nr);
    goto err;
err2:
    fprintf (stderr, "%s: line %d of .dmrc: incorrect char, missing %s\n", argv0, nr, nr < 5 ? "newline" : "eof");
err:
    unlink (".dmrc2");
    errexit (0, "sorry: .dmrc update failed!");
#endif
}

void clean_layout_view ()
{
    register char **ml;
    register struct mo_elmt *p;
    FILE *fp;

    if (eflag) {
	if ((fp = fopen ("exp_dat", "r"))) {
	    while (fscanf (fp, "%s", buf) != EOF) {
		p = (struct mo_elmt *)malloc (sizeof (struct mo_elmt));
		if (!p) errexit (4, "");
		strcpy (p -> mo, buf);
		p -> next = emlist;
		emlist = p;
	    }
	    fclose (fp);
	}
    }

    if (_dmExistView (project, Layout) == 1) {
	if (specml)
	    ml = specml;
	else
	    ml = (char **) dmGetMetaDesignData (CELLLIST, project, Layout);

	if (ml)
	    while (*ml) unlink_2nd_files (*ml++);
    }
    else {
	errexit (-3, Layout);
    }

    if (!eflag) (void) unlink ("exp_dat");
}

void unlink_2nd_files (char *cell)
{
    struct geo_info ginfo1, ginfo2, ginfo3;
    DIR *dirp;
    struct dirent *dp;
    register struct mo_elmt *p;
    FILE *fp;

    sprintf (buf, "%s/%s", Layout, cell);
    if (vflag) fprintf (stderr, "%s: %s\n", argv0, buf);

    if ((p = emlist)) {
	do {
	    if (strcmp (cell, p -> mo) == 0) {
		errexit (-9, cell);
		return;
	    }
	}
	while ((p = p -> next));
    }

    if (!(dirp = opendir (buf))) {
	errexit (-6, buf);
	return;
    }

    do_chdir (buf);

    while ((dp = readdir (dirp))) {
	if (strcmp (dp -> d_name, ".") == 0
		|| strcmp (dp -> d_name, "..") == 0
		|| strcmp (dp -> d_name, "box") == 0
		|| strcmp (dp -> d_name, "info") == 0
		|| strcmp (dp -> d_name, "mc") == 0
		|| strcmp (dp -> d_name, "nor") == 0
		|| strcmp (dp -> d_name, "annotations") == 0
		|| (oldproj && strcmp (dp -> d_name, "is_macro") == 0)
		|| strcmp (dp -> d_name, "term") == 0)
	    continue;

	do_unlink (dp -> d_name);
    }

    closedir (dirp);

    filename = "info";
    if (!(fp = fopen ("info", "rb"))) { errexit (-8, buf); goto ret; }
    _dmGet_geo_data (fp, GEO_INFO);
    ginfo1.bxl = ginfo.bxl;
    ginfo1.bxr = ginfo.bxr;
    ginfo1.byb = ginfo.byb;
    ginfo1.byt = ginfo.byt;
    _dmGet_geo_data (fp, GEO_INFO);
    ginfo2.bxl = ginfo.bxl;
    ginfo2.bxr = ginfo.bxr;
    ginfo2.byb = ginfo.byb;
    ginfo2.byt = ginfo.byt;
    _dmGet_geo_data (fp, GEO_INFO);
    ginfo3.bxl = ginfo.bxl;
    ginfo3.bxr = ginfo.bxr;
    ginfo3.byb = ginfo.byb;
    ginfo3.byt = ginfo.byt;
    fclose (fp);

    if (!(fp = fopen ("info", "wb"))) { errexit (-8, buf); goto ret; }
    ginfo.bxl = ginfo1.bxl;
    ginfo.bxr = ginfo1.bxr;
    ginfo.byb = ginfo1.byb;
    ginfo.byt = ginfo1.byt;
    _dmPut_geo_data (fp, GEO_INFO);
    ginfo.bxl = ginfo2.bxl;
    ginfo.bxr = ginfo2.bxr;
    ginfo.byb = ginfo2.byb;
    ginfo.byt = ginfo2.byt;
    _dmPut_geo_data (fp, GEO_INFO);
    ginfo.bxl = ginfo3.bxl;
    ginfo.bxr = ginfo3.bxr;
    ginfo.byb = ginfo3.byb;
    ginfo.byt = ginfo3.byt;
    _dmPut_geo_data (fp, GEO_INFO);
    fclose (fp);
ret:
    do_chdir (project -> dmpath);
}

void do_chdir (char *path)
{
    if (chdir (path)) errexit (10, path);
}

void do_unlink (char *file)
{
    if (Vflag) fprintf (stderr, "%s: remove %s\n", argv0, file);
    (void) unlink (file);
}

void clean_circuit_view ()
{
    DIR *dirp;
    struct dirent *dp;
    register char **ml, *s;

    if (_dmExistView (project, Circuit) == 1) {
	if (specml)
	    ml = specml;
	else
	    ml = (char **) dmGetMetaDesignData (CELLLIST, project, Circuit);

	if (ml) {
	    for (; *ml; ++ml) {
		sprintf (buf, "%s/%s", Circuit, *ml);
		if (vflag) fprintf (stderr, "%s: %s\n", argv0, buf);
		if (!(dirp = opendir (buf))) { errexit (-6, buf); continue; }
		do_chdir (buf);
		while ((dp = readdir (dirp))) {
		    if ((s = strrchr (dp -> d_name, '_'))) ++s;
		    else s = dp -> d_name;
		    if (strcmp (s, "sls") == 0
		     || strcmp (s, "sls.o") == 0) do_unlink (dp -> d_name);
#if 0
		    if (!oldproj && strcmp (dp -> d_name, "devmod") == 0) do_unlink (dp -> d_name);
#endif
		}
		closedir (dirp);
		do_chdir (project -> dmpath);
	    }
	}
    }
    else {
	errexit (-3, Circuit);
    }
}

void clean_floorplan_view ()
{
    DIR *dirp;
    register char **ml;

    if (_dmExistView (project, Floorplan) == 1) {
	if (specml)
	    ml = specml;
	else
	    ml = (char **) dmGetMetaDesignData (CELLLIST, project, Floorplan);

	if (ml) {
	    while (*ml) {
		sprintf (buf, "%s/%s", Floorplan, *ml);
		if (vflag) fprintf (stderr, "%s: %s\n", argv0, buf);
		if (!(dirp = opendir (buf))) errexit (-6, buf);
		else closedir (dirp);
		++ml;
	    }
	}
    }
    else {
	errexit (-3, Floorplan);
    }
}

void sig_handler (int sig) /* signal handler */
{
    if (noExit) return;
    signal (sig, SIG_IGN); /* ignore signal */
    sprintf (buf, "%d", sig);
    fprintf (stderr, "\n");
    errexit (1, buf);
}

char *err_list[] = {
/* 0 */    "%s",
/* 1 */    "interrupted due to signal: %s",
/* 2 */    "error in DMI function",
/* 3 */    "%s: view does not exist",
/* 4 */    "cannot alloc core",
/* 5 */    "%s: cannot stat \"%s\"",
/* 6 */    "%s: cannot open dir",
/* 7 */    "%s: cannot unlink",
/* 8 */    "%s: cannot rewrite \"%s\"",
/* 9 */    "%s: cell in \"exp_dat\", not processed",
/* 10 */   "%s: cannot chdir",
/* 11 */   "but cannot find error message"
};

void errexit (int err_no, char *s)
{
    int i;

    i = (err_no < 0) ? -err_no : err_no;
    if (i > 11) i = 11;

    fprintf (stderr, "%s: ", argv0);
    if (i > 2) {
	if (err_no < 0)
	    fprintf (stderr, "warning: ");
	else
	    fprintf (stderr, "error: ");
    }
    fprintf (stderr, err_list[i], s, filename);
    fprintf (stderr, "\n");

    if (err_no >= 0) {
	fprintf (stderr, "\n%s: -- program aborted --\n", argv0);
	dmQuit ();
	exit (1);
    }
}

void dmError (char *s)
{
    fprintf (stderr, "%s: ", argv0);
    dmPerror (s);
    if (noExit) return;
    errexit (2, "");
}
