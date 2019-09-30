/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
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

#include "src/drc/dubcheck/dubcheck.h"

/* This is the main procedure of the program dubcheck.   */
/* After initialisation of the datastructrures		 */
/* (routine ini ) it opens two files:			 */
/* 1. The EXPDATA file in which the names of the models  */
/*    to be checked are recorded.			 */
/* 2. The "dubcheckdata" file, which contains the data	 */
/* about the minimum gaps and overlaps for each layer.	 */
/* While reading the data from these files it sets up    */
/* two loops:					       	 */
/* 1. The outer loop: check each model recorded in the   */
/*    EXPDATA file.					 */
/* 2. The inner loop: check each layer recorded in the   */
/*    "dubcheckdata" file.				 */
/* In these loops 3 flags are updated:			 */
/* 1. The exgapflag. If this flag is 1 it means that for */
/*                   tracks laying parallel to each other*/
/*                   for only a short distance the gap   */
/*                   can be smaller then for tracks      */
/*                   laying parallel to each other for a */
/*                   longer distance.			 */
/*                   If this flag is 0 it means there is */
/*                   no difference between the two.	 */
/* 2. The overlapflag. If this flag is 1 overlap_checks	 */
/*		     have to be carried out. If it is 0	 */
/*                   overlap_checks are not needed.	 */
/* 3. The gapflag.   If this flag is 1 gap_checks have   */
/*                   to be carried out . If it is 0      */
/*                   gap_checks are not needed.		 */
/* Furthermore the variable MAXINFLUENCE is updated.     */
/* This variable gives the maximum x_value over which the*/
/* events of a layer may have an influence on the other  */
/* events.If the exgapflag is 1 it is set to the maximum */
/* of exgaplength, gap and overlap. If exgapflag is 0 it */
/* is set to the maximum of gap and overlap.		 */
/* After these variables have been set the real work is  */
/* done in the procedure main_check and the stateruler   */
/* is freed (free_state_ruler)				 */
/* The loops are closed then and the program stops.	 */

static void check_cell (char *modelname);
static void sig_handler (int sig);

char    temp_file[20];
char    maskfile1[DM_MAXNAME + 1];
char    maskfile2[DM_MAXNAME + 1];
char    maskfile3[DM_MAXNAME + 1];
char    header_err[MAXLINE + 1];
char   *argv0 = "dubcheck";
int     test;

DM_PROJECT * dmproject;

int main (int argc, char *argv[])
{
    int     iarg;		/* argument number */
    char   *p;
    char    filename[256];
    char    modelname[DM_MAXNAME + 1];
    char   *desrul_file;
    char   *use_msg = "\nUsage: %s [-f] [-t] [cell_name]\n\n";
    int     file_flag;
    int     cell_name_flag;

    test = OFF;
    file_flag = OFF;
    cell_name_flag = OFF;

    for (iarg = 1; iarg < argc && argv[iarg][0] == '-'; ++iarg) {
	for (p = &argv[iarg][1]; *p; ++p) {
	    switch (*p) {
		case 't':
		    test = ON;
		    break;
		case 'f':
		    file_flag = ON;
		    break;
		default:
		    fprintf (stderr, "\nunknown option: %c", *p);
		    fprintf (stderr, use_msg, argv0);
		    exit (1);
	    }
	}
    }
    if (iarg == argc - 1)
	cell_name_flag = ON;
    if (iarg < (argc - 1)) {
	if (argv[argc - 1][0] == '-') {
	    fprintf (stderr, "\ncell_name must be the last argument");
	    fprintf (stderr, use_msg, argv0);
	    exit (1);
	}
	else {
	    fprintf (stderr, "\n too many arguments specified");
	    fprintf (stderr, use_msg, argv0);
	    exit (1);
	}
    }

    sprintf (temp_file, "/tmp/duberr.%05d", (int)getpid ());

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
    ini ();

    if (file_flag == ON) {
	if (access ("./dubcheckdata", 0) == 0) {
	    OPEN (pdat, "dubcheckdata", "r");
	}
	else {
	    fprintf (stderr, "\ndubcheckdata not in the WD; i'll take the usual one\n");
	    file_flag = OFF;
	}
    }
    if (file_flag == OFF) {
	desrul_file = dmGetMetaDesignData (PROCPATH, dmproject, "dubcheckdata");
	OPEN (pdat, desrul_file, "r");
    }
    if (cell_name_flag == OFF) {
	sprintf (filename, "%s/%s", dmproject -> dmpath, EXPDATA);
	OPEN (pexp, filename, "r");
	while (fscanf (pexp, "%s", modelname) != EOF) {
	    check_cell (modelname);
	}
    }
    else {
	check_cell (argv[argc - 1]);
    }
    die (0);
    return (0);
}

static void check_cell (char *modelname)
{
    DM_CELL * mod_key;
    char    rulename[MAXLINE];
    char    descrip[MAXLINE];
    char    sub_err[MAXLINE];
    DM_STREAM * p_info;
#ifdef DRCERRORSTREAM
    DM_STREAM * drcstream;
#endif
    int     total_errs;
    int     i;

    mod_key = dmCheckOut (dmproject, modelname, WORKING, DONTCARE, LAYOUT, ATTACH);

    /* determine factor nr_samples.		 */
    p_info = dmOpenStream (mod_key, "info", "r");

    /* first read the boundingboxes of the model, which	 */
    /* are not used here.				 */
    dmGetDesignData (p_info, GEO_INFO);
    dmGetDesignData (p_info, GEO_INFO);
    dmGetDesignData (p_info, GEO_INFO);

    /* then read the data needed.			 */
    dmGetDesignData (p_info, GEO_INFO3);
    nr_samples = ginfo3.nr_samples;

    /* and close the stream again.			 */
    dmCloseStream (p_info, COMPLETE);

    if ((nr_samples != 0) && (nr_samples < 3)) {
	fprintf (stderr, "nr_samples (= %d) is too small", nr_samples);
	die (1);
    }

#ifdef DRCERRORSTREAM
    drcstream = dmOpenStream (mod_key, "dub", "w");
    pout = drcstream->dmfp;
#else
    pout = stdout;
#endif
    fprintf (pout, "\ncell: %s\n\n", modelname);
    total_errs = 0;
    descrip[0] = '\0';
    fseek (pdat, 0L, 0);	/* rewind */

    while (1) {
	while ((c=getc (pdat)) == '\n');
	if (c == '#') {
	    /* remove comment string */
	    fgets (line, MAXLINE, pdat);
	    continue;
	}
	else {
	    ungetc (c, pdat);
	}

	if (fscanf (pdat, "%s%s%s%d%d%d%d%d%s%[^\n]", maskfile1, maskfile2,
		maskfile3, &overlap, &gap, &exgap, &exlength,
		&kind, rulename, descrip) == EOF) {
	    break;
	}

/* first get the model_layout info_file to see if the model	 */
/* has been multiplied with a factor to cope with slanted boxes	 */
/* or polygons.							 */

	grow_fact = 0;

	OPEN (bufil, temp_file, "w")
	    pvln[0] = dmOpenStream (mod_key, maskfile1, "r");
	pvln[1] = dmOpenStream (mod_key, maskfile2, "r");
	if (strcmp (maskfile3, "NOFILE") != 0)
	    pvln[2] = dmOpenStream (mod_key, maskfile3, "r");
	else
	    pvln[2] = NULL;

	if ((kind != 4) && (kind != 5)) {
	    if (strncmp (maskfile1, "bool", 4) == 0) {
		if (strncmp (maskfile2, "bool", 4) == 0)
		    sprintf (header_err, "Rule no.:%8s Masks: %s %s ",
			    rulename, maskfile1, maskfile2);
		else
		    sprintf (header_err, "Rule no.:%8s Masks: %s %.2s ",
			    rulename, maskfile1, maskfile2);
	    }
	    else {
		if (strncmp (maskfile2, "bool", 4) == 0)
		    sprintf (header_err, "Rule no.:%8s Masks: %.2s %s ",
			    rulename, maskfile1, maskfile2);
		else
		    sprintf (header_err, "Rule no.:%8s Masks: %.2s %.2s ",
			    rulename, maskfile1, maskfile2);
	    }
	    if (overlap != 0) {
		sprintf (sub_err, "Overlap: %d ", overlap);
		addstring (header_err, sub_err);
		overlapflag = 1;
	    }
	    else
		overlapflag = 0;
	    if (gap != 0) {
		sprintf (sub_err, "Gap: %d ", gap);
		addstring (header_err, sub_err);
		gapflag = 1;
	    }
	    else
		gapflag = 0;
	    if (exgap == (-2)) {/* dist from grown item */
		grow_fact = exlength;
		sprintf (sub_err, "grow_factor: %d ", exlength);
		addstring (header_err, sub_err);
	    }
	    else
		if (gap != exgap && gap != 0 && exgap != 0) {
		    sprintf (sub_err, "Exgap: %d Exlength: %d ",
			    exgap, exlength);
		    addstring (header_err, sub_err);
		}
	    sprintf (sub_err, "%s\n", descrip);
	    addstring (header_err, sub_err);
	}

/* now the values of gap, overlap etc. will be updated		 */
/* according to the value of nr_samples found.			 */

	if (nr_samples != 0) {
	    if (overlap != 0)
		overlap = overlap * nr_samples - 1;
	    if (gap != 0)
		gap = gap * nr_samples - 1;
	    if (exgap != 0)
		exgap = exgap * nr_samples - 1;
	    if (exlength != 0)
		exlength = exlength * nr_samples + 1;
	}

	if (exgap == 0)
	    exgap = gap;
	if ((exgap < 0) || (exgap >= gap)) {
	    exgapflag = 0;
	    MAXINFLUENCE = MAX (gap, overlap);
	}
	else {
	    exgapflag = 1;
	    MAXINFLUENCE = MAX (overlap, MAX (exlength + 1, gap));
	}
	head_err = NULL;
	Errno = 0;
	main_check ();
	free_state_ruler ();
	if (kind == 3) {
	    for (i = 0; i < MAX_GROUP; i++)
		conn_dir[i] = 0;
	}
	CLOSE (bufil);

	OPEN (bufil, temp_file, "r");
	print_err (kind);
	rmv_errstr ();
	filter_err ();
	free_errs (head_errlist);
	if (Errno != 0)
	    fprintf (pout, "\n");
	total_errs = total_errs + Errno;
	head_errlist = NULL;
	dmCloseStream (pvln[0], COMPLETE);
	dmCloseStream (pvln[1], COMPLETE);
	if (pvln[2] != NULL)
	    dmCloseStream (pvln[2], COMPLETE);
	CLOSE (bufil);
    }
    if (total_errs == 0)
	fprintf (pout, "no errors found\n\n");
#ifdef DRCERRORSTREAM
    dmCloseStream (drcstream, COMPLETE);
#endif
    dmCheckIn (mod_key, COMPLETE);
}

void addstring (char *main_string, char *sub_string)
{
    if ((strlen (main_string) + strlen (sub_string)) > (MAXLINE + 1)) {
	fprintf (stderr, "header_error_string:\n'%s ....'\ngets too long\n\n", main_string);
	exit (1);
    }
    strcat (main_string, sub_string);
}

void error (char *s)
{
    fprintf (stderr, "%s: %s\n", argv0, s);
    die (1);
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
    unlink (temp_file);
    if (status) {
	fprintf (stderr, "%s: -- program aborted --\n", argv0);
    }
    if (dmproject) dmCloseProject (dmproject, QUIT);
    dmQuit ();
    exit (status);
}

void ERROR (char *str, int p1, int p2, int p3, int p4)
{
    if (nr_samples != 0) {
	if (p1 >= 0)
	    p1 = (p1 + nr_samples / 2) / nr_samples;
	else
	    p1 = (p1 - nr_samples / 2) / nr_samples;
	if (p2 >= 0)
	    p2 = (p2 + nr_samples / 2) / nr_samples;
	else
	    p2 = (p2 - nr_samples / 2) / nr_samples;
	if (p3 >= 0)
	    p3 = (p3 + nr_samples / 2) / nr_samples;
	else
	    p3 = (p3 - nr_samples / 2) / nr_samples;
	if (p4 >= 0)
	    p4 = (p4 + nr_samples / 2) / nr_samples;
	else
	    p4 = (p4 - nr_samples / 2) / nr_samples;
    }
    if (Errno == 0) fprintf (pout, "%s", header_err);
    fprintf (pout, "error: %8s%8d,%8d%8d,%8d\n", str, p1, p2, p3, p4);
    Errno++;
}
