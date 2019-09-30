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

/* The mkpr (make project) command should be used by the designer to
** create a project before actually starting to design.
** It shows much resemblance with the UNIX mkdir command.
** mkpr creates a directory with a file '.dmrc' in it, containing
** a 'release number' of the software by means of which this project was
** created (which will be checked automatically by dmOpenProject),
** the 'process identifier', 'lambda', and the 'nr_samples per lambda'.
** To prevent that the designer starts working in a directory which
** is not a project, dmOpenProject() should verify the existence of '.dmrc'.
**
** In the nearby future various other facilities can be incorporated
** in the mkpr command, such as project-administration, support for
** a hierarchical project organization, initialization of design data
** management facilities (creating e.g directories and tables).
*/

#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <strings.h>
#include "src/libddm/dmstd.h"

#define	RELEASE_NUM	302	/* softw. rel. nr.; see dmOpenProject */
#define DEF_N_SAMP	8	/* default number of samples */

#define PE fprintf(stderr,

#ifdef WIN32
#define mkDir(dir) mkdir (dir)
#define strCmp strcasecmp
#else
#define mkDir(dir) mkdir (dir, 0777)
#define strCmp strcmp
#endif

void init_proj (char *project, int sel_proc_id, double sel_lambda);
void read_dmrc (char *project, int mode);
int  test_proj (char *project);
int ask_proc_id (char *processname);
double ask_lambda (char *lambda);
void dmError (char *s);
int myCreat (char *path, int mode);

char *argv0 = "mkpr";	/* program name */
char *use_msg =		/* command line */
"\nUsage: %s [-p process] [-l lambda] [-u project] project\n\n";

char  *optProject = NULL;
char  *optProcess = NULL;
char  *optLambda = NULL;
int   printProcess = 1;
int   printLambda = 1;

int Extended = 1;
int rel_nr = RELEASE_NUM;
int n_samples = DEF_N_SAMP;
double pLambda = 0;

char buf[MAXLINE];
char pProcess[MAXLINE];
char proc_name[MAXLINE];
char processlist[MAXLINE];
char dir_path[MAXLINE];

extern int optind;
extern char *optarg;

static char *view_str[] = {
    LAYOUT,
    CIRCUIT,
    FLOORPLAN
};

int main (int argc, char *argv[])
{
    char   *hp, *project_name;
    char    dmpath[MAXLINE];
    char    projpath[MAXLINE];
    char    lambda_buf[MAXLINE];
    int     make_dir_path, sel_proc_id, c;
    int     errflg = 0;
    double  sel_lambda = 0;

    if (dmInit (argv0) == -1) exit (1);

    while ((c = getopt (argc, argv, "oep:P:l:u:")) != -1) {
	switch (c) {
	case 'o': Extended = 0; break;
	case 'e': Extended = 1; break;
	case 'P':
	case 'p': optProcess = optarg; printProcess = 0; break;
	case 'l': optLambda  = optarg; printLambda = 0; break;
	case 'u': optProject = optarg; break;
	case '?': ++errflg; break;
	}
    }

    if (Extended == 0) {
	Extended = 1;
	PE "%s: warning: -o is obsolete (using extended)!\n", argv0);
    }

    if (errflg || optind != argc - 1) {
	PE use_msg, argv0);
	exit (1);
    }

    hp = argv[argc-1];
    if (strlen (hp) >= MAXLINE) {
	PE "%s: path name too long\n", argv0);
	exit (1);
    }
    strcpy (projpath, hp);
#if defined(__CYGWIN__) || defined(WIN32)
    hp = projpath;
    while ((hp = strchr (hp, '\\'))) *hp = '/';
#endif

    project_name = strrchr (projpath, '/');
    if (project_name) {
	if (project_name == projpath) strcpy (dmpath, "/");
	else { *project_name = 0; strcpy (dmpath, projpath); }
#if defined(__CYGWIN__) || defined(WIN32)
	if (dmpath[1] == ':' && !dmpath[2]) { dmpath[2] = '/'; dmpath[3] = 0; }
#endif
	project_name += 1;
    }
    else {
	project_name = projpath;
#if defined(__CYGWIN__) || defined(WIN32)
	if (project_name[0] && project_name[1] == ':') {
	    dmpath[0] = project_name[0];
	    dmpath[1] = ':';
	    dmpath[2] = '.';
	    dmpath[3] = 0;
	    project_name += 2;
	}
	else
#endif
	strcpy (dmpath, ".");
    }

    if (optProject)
	read_dmrc (optProject, 1);
    else
	read_dmrc (optProject = ".", 0);

    dm_extended_format = 1;

    dm_maxname = DM_MAXNAME;

    if (dmTestname (project_name) == -1) exit (1);
    /* dmTestname might have shorten the project name */

#ifdef DM_DEBUG
    IFDEBUG PE "dmpath: '%s'\nproject_name: '%s'\n", dmpath, project_name);
#endif

    if (dmpath[strlen (dmpath) - 1] == '/')
	_dmSprintf (dir_path, "%s%s", dmpath, project_name);
    else
	_dmSprintf (dir_path, "%s/%s", dmpath, project_name);

    /* We do not create the directory yet. We will only do that after
     * the designer has provided all information in the right way.
     * However, we will now check the possibility to create the directory
     * to prevent unnecessary failures at the end of the program.
     */

    make_dir_path = test_proj (dir_path);

    if (!optProcess && !*pProcess)
	optProcess = getenv ("ICDPROCESS");

    if (optProcess) {
	if ((sel_proc_id = ask_proc_id (optProcess)) < 0) {
	    DIR *dp;
	    /* 'optProcess' is not in $ICDPATH/share/lib/process/processlist */
	    if ((dp = opendir (optProcess))) {
		/* 'optProcess' is a path to a process directory */
		struct dirent *e;
		while ((e = readdir (dp))) if (strcmp (e -> d_name, "maskdata") == 0) break;
		closedir (dp);
		if (!e) {
		    PE "%s: invalid process '%s'.\n   File 'maskdata' not found.\n", argv0, optProcess);
		    goto abort;
		}
	    }
	    else {
		if (strchr (optProcess, '/'))
		    PE "%s: '%s' is not a readable process directory\n", argv0, optProcess);
		else
		    PE "%s: invalid process '%s'.\n   Not in %s.\n", argv0, optProcess, processlist);
abort:
		printf ("   Program aborted.\n");
		exit (1);
	    }
	}
    }
    else if (*pProcess) {
	sel_proc_id = -1;
	hp = optProcess = pProcess;
	while (*hp >= '0' && *hp <= '9') ++hp;
	if (!*hp) {
	    sel_proc_id = atoi (pProcess);
	    if (ask_proc_id (pProcess) >= 0) optProcess = proc_name;
	    else PE "%s: warning: unknown process id '%s'.\n   Not in %s.\n",
		argv0, pProcess, processlist);
	}
    }
    else
	sel_proc_id = ask_proc_id (NULL);

    if (!optLambda) {
	if (pLambda > 0)
	    sel_lambda = pLambda;
	else {
	    FILE *fp;
	    if (sel_proc_id < 0)
		sprintf (buf, "%s/default_lambda", optProcess);
	    else
		strcpy (buf, _dmDoGetProcPath (sel_proc_id, "default_lambda", NULL));
	    if ((fp = fopen (buf, "r"))) {
		fscanf (fp, "%s", lambda_buf);
		optLambda = lambda_buf;
		fclose (fp);
	    }
	}
    }
    if (sel_lambda <= 0)
	sel_lambda = ask_lambda (optLambda);

/* Now that we have all information we will try to make
** the directory. First of all ignore signals.
*/
#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN);
#endif
#ifdef SIGINT
    signal (SIGINT, SIG_IGN);
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);
#endif
#ifdef SIGTERM
    signal (SIGTERM, SIG_IGN);
#endif

    if (make_dir_path) {
	if (mkDir (dir_path)) goto sys_err;
    }

    /* now try to lock the new project immediately, to prevent
     * that others try to use it right away.
     * We did not use dmOpenProject() for this purpose because:
     * (1) the lock of dmOpenProject() is the single-user lock of release 2,
     *     which can be omitted in a multi-user implementation.
     * (2) dmOpenProject() verifies the legality of the project by searching
     *     for the file '.dmrc' which we still have to create.
     */
#ifdef PRLOCK
    if (_dmLockProject (dir_path) == -1) {
	/* this should of course never happen in this new directory */
	exit (1);
    }
#endif

    init_proj (dir_path, sel_proc_id, sel_lambda);

#ifdef PRLOCK
    _dmUnlockProject (dir_path);
#endif

    dmQuit ();
    printf ("%s: -- project created --\n", argv0);
    exit (0);

sys_err:
    dmerrno = DME_SYS;
    dmError2 ("can't mkdir", dir_path);
    exit (1);
    return (0);
}

void adjustpath (char *path, char *inpath)
{
    int c;
    DIR *dp;
    char *s = inpath;
    char *t = path;

    if (*s != '/' && *s != '\\') { /* relative path */
#if defined(__CYGWIN__) || defined(WIN32)
	if (s[1] == ':') { /* device */
	    *t++ = *s++;
	    *t++ = *s++;
	    if (*s != '/' && *s != '\\') --s;
	}
	else
#endif
	{ *t++ = '.'; *t++ = '.'; --s; }
    }
    *t = '/';
    while (*++s) {
	c = *s;
#if defined(__CYGWIN__) || defined(WIN32)
	if (c == '\\') c = '/';
#endif
	if (*t == '/') {
	    if (c == '.') {
		if (!*++s) break;
		c = *s;
#if defined(__CYGWIN__) || defined(WIN32)
		if (c == '\\') c = '/';
#endif
		if (c != '/') { *++t = '.'; *++t = c; }
	    }
	    else if (c != '/') *++t = c;
#if defined(__CYGWIN__) || defined(WIN32)
	    else if (s == inpath+1) *++t = c; // UNC-path
#endif
	}
	else *++t = c;
    }
    if (*t == '/' && t > path) --t;
    *++t = '\0';

    if (*path == '.') { /* rel. process path */
	char p_use[MAXLINE];
	char p_new[MAXLINE];
	char *p;

	if (!getcwd (p_new, MAXLINE)) {
	    PE "%s: error: can't get cwd!\n", argv0);
	    exit (1);
	}

	s = path + 2;
	sprintf (p_use, "%s%s", optProject, s);
	if (chdir (p_use)) {
	    PE "%s: can't chdir to '%s'!\n", argv0, p_use);
	    if (!*s) goto ready;
	    if (chdir (optProject)) {
		PE "%s: error: can't chdir to '%s'!\n", argv0, optProject);
		exit (1);
	    }
	    if (!getcwd (p_use, MAXLINE)) {
		PE "%s: error: can't getdir '%s'!\n", argv0, optProject);
		exit (1);
	    }
	    p = p_use + strlen (p_use);
	    t = s;
	    while (*++t == '.' && *++t == '.' && (*++t == '/' || !*t)) {
		while (*--p != '/');
		s = t;
		if (!*s) {
		    if (p == p_use) ++p;
		    break;
		}
		if (p == p_use) break;
	    }
	    strcpy (p, s);
	}
	else if (!getcwd (p_use, MAXLINE)) {
	    PE "%s: error: can't getdir '%s'!\n", argv0, optProject);
	    exit (1);
	}
ready:
	if (chdir (p_new)) { /* back to cwd */
	    PE "%s: error: can't chdir to '%s'!\n", argv0, p_new);
	    exit (1);
	}
	if (chdir (dir_path)) { /* new project */
	    PE "%s: error: can't chdir to '%s'!\n", argv0, dir_path);
	    exit (1);
	}
	if (!getcwd (p_new, MAXLINE)) {
	    PE "%s: error: can't getdir '%s'!\n", argv0, dir_path);
	    exit (1);
	}

	s = p = p_use;
	t = p_new;
	while (*++p == *++t) if (*p == '/') s = p; else if (!*p) break;
	if (!*t && (!*p || *p == '/')) strcpy (path+1, p);
	else {
	    if (*t == '/') { if (!*p) s = p; else --t; }
	    p = path + 2;
	    if (s > p_use) {
		while (*++t) if (*t == '/') {
		    *p++ = '/'; *p++ = '.'; *p++ = '.';
		}
	    }
	    if (*s) {
		if ((s - p_use) < (p - path))
		    strcpy (path, p_use);
		else
		    strcpy (p, s);
	    }
	}
    }

    if ((dp = opendir (path))) closedir (dp);
    else PE "%s: warning: can't opendir process '%s'.\n", argv0, path);
}

void exit_entry_exist (char *proj, char *entry)
{
    PE "%s: entry '%s' already exists!\n", argv0, entry);
    PE "%s: cannot turn into project %s\n", argv0, proj);
    exit (1);
}

int test_proj (char *proj)
{
    DIR *d;
    struct dirent *e;
    char *s;
    int i;

    if (!(d = opendir (proj))) return (1);
    while ((e = readdir (d))) {
	s = e -> d_name;
	if (*s == '.') {
	    if (strCmp (s, ".dmrc") == 0) exit_entry_exist (proj, s);
	    if (strCmp (s, ".dmxdata") == 0) exit_entry_exist (proj, s);
	}
	else {
	    if (strCmp (s, "projlist") == 0) exit_entry_exist (proj, s);
	    for (i = 0; i < sizeof (view_str)/sizeof (char *); ++i)
		if (strCmp (s, view_str[i]) == 0) exit_entry_exist (proj, s);
	}
    }
    closedir (d);
    return (0);
}

void init_proj (char *project, int sel_proc_id, double sel_lambda)
/*
** Creates the files '.dmrc' and 'projlist' in the newly created directory.
** Furthermore it creates the view directories with the empty files
** 'celllist' and 'impcelllist' in it.
*/
{
    char    path[MAXLINE];
    FILE   *fp;
    int     fd, i;

    for (i = 0; i < sizeof (view_str)/sizeof (char *); i++) {
	_dmSprintf (path, "%s/%s", project, view_str[i]);

	if (mkDir (path)) {
	    dmerrno = DME_SYS;
	    dmError2 ("can't mkdir", path);
	    exit (1);
	}

	/* create an empty celllist in the view directory: every legal
	 * project should have celllists in its view-dirs from now on.
	 */
	_dmSprintf (path, "%s/%s/celllist", project, view_str[i]);
        if ((fd = myCreat (path, 0666)) == -1) {
	    PE "%s: cannot create %s\n", argv0, path);
	    exit (1);
	}
	close (fd);

	/* create an empty impcelllist in the view directory.
	 */
	_dmSprintf (path, "%s/%s/impcelllist", project, view_str[i]);
        if ((fd = myCreat (path, 0666)) == -1) {
	    PE "%s: cannot create %s\n", argv0, path);
	    exit (1);
	}
	close (fd);
    }

    /* create an empty projlist
     */
    _dmSprintf (path, "%s/projlist", project);
    if ((fd = myCreat (path, 0666)) == -1) {
	PE "%s: cannot create %s\n", argv0, path);
	exit (1);
    }
    close (fd);

    /* create an empty xdata file
     */
    _dmSprintf (path, "%s/.dmxdata", project);
    if ((fd = myCreat (path, 0666)) == -1) {
	PE "%s: cannot create %s\n", argv0, path);
	exit (1);
    }
    close (fd);

    /* create and initialize the file '.dmrc'.
     */
    _dmSprintf (path, "%s/.dmrc", project);
    if (!(fp = fopen (path, "w"))) {
	PE "%s: cannot fopen %s\n", argv0, path);
	exit (1);
    }

#ifndef NOPACK
    fprintf (fp, "%d\n", RELEASE_NUM);
#else
    fprintf (fp, "%dU\n", RELEASE_NUM); /* UnPACKed */
#endif
    if (sel_proc_id < 0) {
	adjustpath (path, optProcess);
	fprintf (fp, "%s\n", path);
	if (printProcess) printf ("%s: using process `%s'\n", argv0, path);
    }
    else {
	fprintf (fp, "%d\n", sel_proc_id);
	if (printProcess) printf ("%s: using process `%d'\n", argv0, sel_proc_id);
    }
    fprintf (fp, "%.8g\n", sel_lambda);
    if (printLambda) printf ("%s: using lambda `%.8g'\n", argv0, sel_lambda);
    fprintf (fp, "%d\n", n_samples);
    fclose (fp);
}

void read_dmrc (char *project, int mode)
{
    FILE *fp;
    int c, i, ok;
    _dmSprintf (buf, "%s/.dmrc", project);
    if (!(fp = fopen (buf, "r"))) {
	if (mode) {
	    PE "%s: cannot fopen %s\n", argv0, buf);
	    exit (1);
	}
	return;
    }
    ok = 0;
    if (fscanf (fp, "%d", &rel_nr) != 1) goto err;
    c = fgetc (fp);
    if (c == 'U' ) c = fgetc (fp);
    if (c == '\r') c = fgetc (fp);
    if (c != '\n') goto err;
    i = 0;
    while ((c = fgetc (fp)) != '\n') {
	if (c < ' ' || c >= 127) {
	    if (c == '\r' && fgetc (fp) == '\n') break;
	}
	pProcess[i++] = c;
    }
    if (!i || pProcess[0] == ' ' || pProcess[i-1] == ' ') goto err;
    pProcess[i] = 0;
    if (fscanf (fp, "%lf", &pLambda) != 1 || pLambda <= 0) goto err;
    if (fscanf (fp, "%d", &n_samples) != 1) goto err;
    ok = 1;
err:
    if (!ok) {
	dmerrno = DME_FMT;
	dmError (buf);
	*pProcess = 0;
	pLambda = 0;
	n_samples = DEF_N_SAMP;
    }
    fclose (fp);
}

#define IS_PROC 1
#define NO_PROC 0

int ask_proc_id (char *processname)
{
    FILE  *proc_fp;
    char   proc_id[MAXLINE];
    char  *hp;
    int    pr_id_arr[DM_MAXPROCESSES];
    int    i, maxprocid, sel_id, count;

    if (processname) {
	if (strchr (processname, '/')) return (-1);
#if defined(__CYGWIN__) || defined(WIN32)
	if (strchr (processname, '\\')) return (-1);
#endif
    }

    _dmSprintf (processlist, "%s/share/lib/process/processlist", icdpath);
    if (!(proc_fp = fopen (processlist, "r"))) {
	PE "%s: cannot open %s\n", argv0, processlist);
	PE "   Check if the variable ICDPATH has been set correctly.\n");
	exit (1);
    }

    for (i = 0; i < DM_MAXPROCESSES; i++) pr_id_arr[i] = NO_PROC;

    if (!processname) {
	if (isatty (fileno (stdin))) {
	    printf ("available processes:\n");
	    printf ("process id       process name\n");
	}
    }

    maxprocid = 0;

    /* read process id's and process names of the supported processes
     */
    while (fgets (buf, MAXLINE, proc_fp)) {
	if (sscanf (buf, "%s%s", proc_id, proc_name) != 2) continue;
	if (*proc_id == '#') continue;
	if ((hp = strchr (proc_name, '#')))
	    *hp = '\0';		/* strip comment right after proc_name */

	i = atoi (proc_id);

	/* If process does not exist, is unreadable etc, skip it */
	if (_dmDoGetProcess (i, NULL) == NULL) continue;

	/* Process id must be in range [0 - (DM_MAXPROCESSES - 1)] */
	if (i < 0 || i >= DM_MAXPROCESSES) {
	    dmerrno = DME_PRREAD;
	    dmError2 ("illegal process-id", proc_id);
	    exit (1);
	}

	pr_id_arr[i] = IS_PROC;

	if (i > maxprocid) maxprocid = i;

	if (processname) {
	    if (strcmp (processname, proc_name) == 0
                || strcmp (processname, proc_id) == 0) {
                fclose (proc_fp);
                return (i);
            }
	}
	else {
	    if (isatty (fileno (stdin)))
		printf ("%10d       %s\n", i, proc_name);
	}
    }
    fclose (proc_fp);

    if (processname) return (-1);

    printProcess = 0;

    count = 0;
    for (;;) {
	if (isatty (fileno (stdin)))
	    printf ("select process id (1 - %d): ", maxprocid);
	if (fgets (buf, MAXLINE, stdin) && sscanf (buf, "%d", &sel_id) == 1) {
	    if (sel_id < 0 || sel_id >= DM_MAXPROCESSES
		|| pr_id_arr[sel_id] == NO_PROC) {
		if (++count > 1) {
		    printf ("wrong again, sorry program aborted.\n");
		    exit (1);
		}
		printf ("no valid process id, try once more!\n");
	    }
	    else break;
	}
        if (++count > 5) {
	    printf ("invalid input, program aborted.\n");
	    exit (1);
	}
    }
    return (sel_id);
}

double ask_lambda (char *lambda)
{
    double sel_lambda;
    int count = 0;

    if (lambda) {
	sel_lambda = atof (lambda);
	if (sel_lambda <= 0.0) {
	    PE "%s: '%s' not a legal lambda value, sorry program aborted!\n",
		argv0, lambda);
	    exit (1);
	}
	return (sel_lambda);
    }

    printLambda = 0;

    for (;;) {
	if (isatty (fileno (stdin)))
	    printf ("enter lambda (layout grid) in microns: ");
	if (fgets (buf, MAXLINE, stdin) && sscanf (buf, "%lf", &sel_lambda) == 1) {
	    if (sel_lambda <= 0.0) {
		if (++count > 1) {
		    printf ("wrong again, sorry program aborted.\n");
		    exit (1);
		}
		printf ("not a legal value, try once more!\n");
	    }
	    else break;
	}
    }
    return (sel_lambda);
}

void dmError (char *s)
{
    if (dmerrno == DME_PRDATA) return;
    PE "%s: ", argv0);
    dmPerror (s);
}

int myCreat (char *path, int mode)
{
    return (creat (path, mode));
}
