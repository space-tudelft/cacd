/*
 * ISC License
 *
 * Copyright (C) 1994-2018 by
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

#include "src/flatten/extern.h"

#define TOOLVERSION "1.6 (20 Jun 2012)"

struct stat   stat_buf;	/* file status buffer */
struct clist *celllist;
struct tmtx  *tm_p, *tm_s;

DM_PROCDATA *process;
DM_PROJECT  *project;	/* project access key */
DM_CELL     *cellkey;	/* cell access key */
DM_STREAM   *fp_box, *fp_info, *fp_mc, *fp_nor, *fp_term, *fp_anno;
FILE        *fp_ldm;	/* pointer to output file */

double  rad01, rad90;	/* number of rad's for x degrees */
char   *cellext = "flat";
char   *ldmfile;	/* name of LDM file (file mode) */

int     dmErrorOFF = 0;
int     exp_depth = -1; /* flatten depth */
int     level = 1;      /* level of current cell */
int     Lflag;          /* only Local cells (if true) */
int     tflag;          /* terminals of sub-cells (if true) */
int     pflag;          /* only terminal postfix, if needed */
int     verbose = 0;    /* verbose mode flag */

char   *argv0 = "flatten";	/* program name */
char   *use_msg =		/* command line */
"\nUsage: %s [-L] [-dN] [-f file] [-h] [-n name] [-s] [-t] [-p] [-v] cell\n\n";

int main (int argc, char *argv[])
{
    DM_CELL *new_key = 0;
    int     iarg = 1;
    int     usage = 0;
    char   *dflag = NULL;
    int     Fflag = 0;
    int     fflag = 0;
    int     hflag = 0;
    int     nflag = 0;
    int     sflag = 0;
    int     len;
    int     Vnr = DONTCARE;
    char   *cellname;
    char    newname[DM_MAXNAME+1];
    char   *cp;

    for (cp = argv[iarg]; iarg < argc && *cp == '-'; cp = argv[++iarg]) {
	while (*++cp)
	switch (*cp) {
	    case 'L':
		++Lflag;
		break;
	    case 'd':
		dflag = cp+1;
		goto nextarg;
	    case 'F':
		++Fflag;
	    case 'f':
		fflag = ++iarg;
		break;
	    case 'h':
		++hflag;
		break;
	    case 'n':
		nflag = ++iarg;
		break;
	    case 's':
		++sflag;
		break;
	    case 't':
		++tflag;
		break;
	    case 'p':
		++pflag;
		break;
	    case 'v':
		++verbose;
		break;
	    default:
		++usage;
		PE "%s: -%c: Unknown option\n", argv0, *cp);
	}
nextarg:;
    }

    if (hflag) {
	PE "\n%s %s\n", argv0, TOOLVERSION);
	PE use_msg, argv0);
	PE "Options:\n");
	PE "-L        Flatten only LOCAL sub-cells\n");
	PE "-dN       Flatten depth (default N=1000)\n");
	PE "-f file   Output to LDM file (no overwrite)\n");
	PE "-F file   Output to LDM file (overwrite)\n");
	PE "-h        This help display\n");
	PE "-n name   New cell name (def: ...flat)\n");
	PE "-s        Silent mode\n");
	PE "-t        Flatten terminals of sub-cells\n");
	PE "-p        No terminal postfix (only if needed)\n");
	PE "-v        Verbose mode\n\n");
	exit (1);
    }

    if (argc != iarg + 1) {
	++usage;
	if (iarg >= argc) pr_err (-1, "");
	else pr_err (-5, "");
    }
    if (usage) {
	PE use_msg, argv0);
	exit (1);
    }

    if (fflag) ldmfile = argv[fflag];
    if (dflag) exp_depth = atoi (dflag);
    if (exp_depth < 0) exp_depth = 1000; /* arbitrary big integer */

    cellname = argv[iarg];
    len = strlen (cellname);
    if (len < 1 || len > DM_MAXNAME) pr_err (9, "");
    dmErrorOFF = 1;
    if (dmTestname (cellname) < 0) pr_err (11, cellname);
    dmErrorOFF = 0;

    if (nflag) {
	len = strlen (argv[nflag]);
	if (len < 1 || len > DM_MAXNAME) pr_err (9, "new ");
	dmErrorOFF = 1;
	if (dmTestname (argv[nflag]) < 0) pr_err (11, argv[nflag]);
	dmErrorOFF = 0;
	strcpy (newname, argv[nflag]);
    }
    else {
	if (len > DM_MAXNAME - strlen (cellext)) pr_err (10, cellext);
	sprintf (newname, "%s%s", cellname, cellext);
    }

    if (!sflag) {
	PE "%s: Flatten cell: %s\n", argv0, cellname);
	PE "%s: Flatten depth: %d\n", argv0, exp_depth);
	if (Lflag) PE "%s: Flatten only local sub-cells\n", argv0);
	if (tflag) PE "%s: Flatten sub-cell terminals (%s postfix)\n", argv0, pflag?"w/o":"add");
	PE "%s: ", argv0);
	if (ldmfile) {
	    if (*ldmfile) PE "LDM output to file: %s", ldmfile);
	    else PE "LDM output to stdout");
	}
	else PE "Result written to NELSIS database");
	PE "\n%s: Update cell name: %s\n", argv0, newname);
	if (verbose) PE "%s: Verbose mode\n", argv0);
    }

#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN); /* ignore hangup signal */
#endif
    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	signal (SIGINT, sig_handler);
#ifdef SIGQUIT
    signal (SIGQUIT, sig_handler);
#endif
    signal (SIGTERM, sig_handler);

    if (ldmfile && *ldmfile && !Fflag)
	if (stat (ldmfile, &stat_buf) != -1) pr_err (2, ldmfile);

    dmInit (argv0);
    project = dmOpenProject (DEFAULT_PROJECT, ldmfile ? PROJ_READ : DEFAULT_MODE);

    rad90 = 2.0 * atan (1.0);
    rad01 = rad90 / 90.0;

if (ldmfile) {
    time_t tsec = time (NULL);
    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, project);

    if (*ldmfile) {
	if (!(fp_ldm = fopen (ldmfile, "w"))) pr_err (8, ldmfile);
    }
    else
	fp_ldm = stdout;

    PO ":: Delft Layout Description Modified, LDM V3.2\n");
    PO ":: Generated with the program %s V%s\n", argv0, TOOLVERSION);
    PO ":: at %.24s\n", ctime (&tsec));
    PO ":: by the Delft NELSIS IC Design System Release 3\n");
    PO ":: use cldm with -o\n");
    PO ":: project = %s\n", project -> dmpath);
    PO ":: process = %s\n", process -> pr_name);
    PO ":: lambda = %g micron\n", project -> lambda);
    PO ":: flatten all related %scells of cell: %s",
	(Lflag ? "LOCAL " : ""), cellname);
    PO "\n:: flatten depth = %d\n", exp_depth);
    PO "ms %s\n", newname);
}
else {
    new_key = dmCheckOut (project, newname, WORKING, DONTCARE, LAYOUT, UPDATE);
    fp_box  = dmOpenStream (new_key, "box" , "w");
    fp_info = dmOpenStream (new_key, "info", "w");
    fp_mc   = dmOpenStream (new_key, "mc"  , "w");
    fp_nor  = dmOpenStream (new_key, "nor" , "w");
    fp_term = dmOpenStream (new_key, "term", "w");
    fp_anno = dmOpenStream (new_key, "annotations", "w");
}
    flat_cell (cellname, Vnr);
if (ldmfile) {
    PO "me\n");
    PO ":: eof\n");
    if (*ldmfile) fclose (fp_ldm);
}
else {
    dmCheckIn (new_key, COMPLETE);
}
    dmQuit ();
    if (!sflag) PE "%s: -- program finished --\n", argv0);
    exit (0);
    return (0);
}
