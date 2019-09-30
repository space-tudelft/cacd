/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#include "src/makevln/incl.h"

DM_PROJECT *project;
DM_PROCDATA *process;
FILE   *vln_file, *teq_file;
struct event_rec *events;
char    tmpvln[20];
char    tmpteq[20];
char    fname[256];
int     term_layer;
int     flag_v;
int     flag_D = 0;     /* def.: do not delete "_bxx" files */

void chg_group (DM_STREAM *fp_vln, DM_STREAM *fp_teq, int term_group_offset);
int  comp_events (const void *event1, const void *event2);
void convert (long max_event_no);
void free_groups (void);
void (*istat)();
void make_vln (char *cell);
int  number_groups (void);
void read_boxes (DM_STREAM *fp_bxx);
void sig_handler (int sig);

char   *argv0 = "makevln";	/* program name */
char   *use_msg =         	/* command line */
"\nUsage: %s [-v] [cell ...]\n\n";

int main (int argc, char *argv[])
{
    FILE    *fp_exp;
    int     i, iarg;
    int     usage = 0;

    for (iarg = 1; iarg < argc && argv[iarg][0] == '-'; iarg++) {
	for (i = 1; argv[iarg][i] != '\0'; i++) {
	    switch (argv[iarg][i]) {
		case 'v':
		    ++flag_v;
		    break;
		default:
		    ++usage;
		    fprintf (stderr,
			"option -%c: unknown option\n", argv[iarg][i]);
	    }
	}
    }

    if (usage) {
	fprintf (stderr, use_msg, argv0);
	exit (1);
    }

    i = getpid ();
    sprintf (tmpvln, "/tmp/mkv.vln%05d", i);
    sprintf (tmpteq, "/tmp/mkv.teq%05d", i);

#ifdef SIGHUP
    signal (SIGHUP , SIG_IGN); /* ignore hangup signal */
#endif
    istat = (void (*)()) signal (SIGINT, SIG_IGN);
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN); /* ignore quit */
#endif
    signal (SIGTERM, sig_handler); /* catch softw terminate */

    dmInit (argv0);
    project = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, project);

    if (iarg >= argc) {
	/* open "exp_dat" file */
	sprintf (fname, "%s/exp_dat", project -> dmpath);
	if (!(fp_exp = fopen (fname, "r")))
	    errexit (1, fname);

	i = 0;
	/* read names of cells to convert */
	while (fscanf (fp_exp, "%s", fname) != EOF) {
	    ++i;
	    make_vln (fname);
	}
	fclose (fp_exp);
	if (i == 0)
	    fprintf (stderr,
		"%s: warning: no _vln files made (empty exp_dat)\n", argv0);
    }
    else {
	while (iarg < argc) {
	    make_vln (argv[iarg++]);
	}
    }

    die (0);
    return (0);
}

void make_vln (char *cell)
{
    DM_CELL   *cellkey;
    DM_STREAM *fp_inf;
    DM_STREAM *fp_inf3;
    DM_STREAM *fp_tidpos;
    DM_STREAM *fp_bxx;
    DM_STREAM *fp_vln;
    DM_STREAM *fp_teq;
    long   *no_of_boxes;
    long   *no_of_groups;
    int     term_group_offset = 0;
    int     i, no_files, no_masks;
    char buf[256];

    /* enable interrupt catching, if not ignored */
    if (istat != (void (*)()) SIG_IGN)
	signal (SIGINT, sig_handler);

    if (flag_v) fprintf (stderr, "-- cell: %s\n", cell);
    cellkey = dmCheckOut (project, cell, WORKING, DONTCARE, LAYOUT, ATTACH);

    no_files = no_masks = process -> nomasks;
    for (i = 0; i < no_masks; ++i) {
	if (process -> mask_type[i] == DM_INTCON_MASK)
	    ++no_files;
    }

    ALLOCARR (no_of_boxes, no_files, long);
    ALLOCARR (no_of_groups, no_files, long);

    fp_inf = dmOpenStream (cellkey, "info2", "r");
    for (i = 0; i < no_files; ++i) {
	dmGetDesignData (fp_inf, GEO_INFO2);
	no_of_boxes[i] = ginfo2.nr_boxes;
	no_of_groups[i] = 0;
    }
    dmCloseStream (fp_inf, COMPLETE);

    /* I don't know how this should work ???
       The tidpos file may contain many lines ! (AvG 290797) */
    fp_tidpos = dmOpenStream (cellkey, "tidpos", "r");
    buf[0] = '\0';
    i = dmScanf (fp_tidpos, "%[^\n]",buf);
    dmCloseStream (fp_tidpos, COMPLETE);

    fp_inf3 = dmOpenStream (cellkey, "info3", "r");
    dmGetDesignData (fp_inf3, GEO_INFO3);
    dmCloseStream (fp_inf3, COMPLETE);

    fp_teq = dmOpenStream (cellkey, "teq", "w");

    no_files = no_masks;
    for (i = 0; i < no_masks; ++i) {
	term_layer = 0; /* false */
	sprintf (fname, "%s_vln", process -> mask_name[i]);
	fp_vln = dmOpenStream (cellkey, fname, "w");

	sprintf (fname, "%s_bxx", process -> mask_name[i]);

	if (no_of_boxes[i] > 0) {
	    ALLOCARR (events, no_of_boxes[i], struct event_rec);

	    fp_bxx = dmOpenStream (cellkey, fname, "r");
	    read_boxes (fp_bxx);
	    dmCloseStream (fp_bxx, COMPLETE);

	    qsort ((char *)events, (unsigned)no_of_boxes[i],
		    sizeof (struct event_rec), comp_events);

	    /* open temp. "vln" file */
	    if (!(vln_file = fopen (tmpvln, "w"))) errexit (2, tmpvln);
	    convert (no_of_boxes[i]);
	    fclose (vln_file);

	    FREE (events);

	    no_of_groups[i] = number_groups ();

	    /* open temp. "vln" file */
	    if (!(vln_file = fopen (tmpvln, "r"))) errexit (1, tmpvln);
	    chg_group (fp_vln, fp_teq, 0);
	    fclose (vln_file);

	    free_groups ();
	}

	dmCloseStream (fp_vln, COMPLETE);

	if (flag_D) {
	    no_of_boxes[i] = 0;
	    dmUnlink (cellkey, fname);
	}

	if (process -> mask_type[i] != DM_INTCON_MASK) continue;

	term_layer = 1; /* true */
	sprintf (fname, "t_%s_vln", process -> mask_name[i]);
	fp_vln = dmOpenStream (cellkey, fname, "w");

	if (no_of_boxes[no_files] > 0) {
	    ALLOCARR (events, no_of_boxes[no_files], struct event_rec);

	    sprintf (fname, "t_%s_bxx", process -> mask_name[i]);
	    fp_bxx = dmOpenStream (cellkey, fname, "r");
	    read_boxes (fp_bxx);
	    dmCloseStream (fp_bxx, COMPLETE);

	    qsort ((char *)events, (unsigned)no_of_boxes[no_files],
		    sizeof (struct event_rec), comp_events);

	    /* open temp. "vln" and "teq" file */
	    if (!(vln_file = fopen (tmpvln, "w"))) errexit (2, tmpvln);
	    if (!(teq_file = fopen (tmpteq, "w"))) errexit (2, tmpteq);
	    convert (no_of_boxes[no_files]);
	    fclose (vln_file);
	    fclose (teq_file);

	    FREE (events);

	    no_of_groups[no_files] = number_groups ();

	    /* open temp. "vln" and "teq" file */
	    if (!(vln_file = fopen (tmpvln, "r"))) errexit (1, tmpvln);
	    if (!(teq_file = fopen (tmpteq, "r"))) errexit (1, tmpteq);
	    chg_group (fp_vln, fp_teq, term_group_offset);
	    fclose (vln_file);
	    fclose (teq_file);

	    term_group_offset += no_of_groups[no_files];

	    free_groups ();
	}
	++no_files;

	dmCloseStream (fp_vln, COMPLETE);
    }

    dmCloseStream (fp_teq, COMPLETE);

    /* disable interrupt catching, if not ignored */
    if (istat != (void (*)()) SIG_IGN)
	signal (SIGINT, SIG_IGN);

    fp_inf = dmOpenStream (cellkey, "info2", "w");
    for (i = 0; i < no_files; ++i) {
	ginfo2.nr_boxes  = no_of_boxes[i];
	ginfo2.nr_groups = no_of_groups[i];
	dmPutDesignData (fp_inf, GEO_INFO2);
    }
    dmCloseStream (fp_inf, COMPLETE);

    fp_tidpos = dmOpenStream (cellkey, "tidpos", "w");
    dmPrintf (fp_tidpos, "%s\n",buf);
    dmCloseStream (fp_tidpos, COMPLETE);

    fp_inf3 = dmOpenStream (cellkey, "info3", "w");
    dmPutDesignData (fp_inf3, GEO_INFO3);
    dmCloseStream (fp_inf3, COMPLETE);

    FREE (no_of_boxes);
    FREE (no_of_groups);

    dmCheckIn (cellkey, COMPLETE);
}

void sig_handler (int sig) /* signal handler */
{
    char buf[6];
    signal (sig, SIG_IGN); /* ignore signal */
    sprintf (buf, "%d", sig);
    errexit (4, buf);
}
