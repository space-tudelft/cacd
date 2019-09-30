/*
 * ISC License
 *
 * Copyright (C) 1982-2018 by
 *	T.G.R. van Leuken
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

#include "src/drc/dimcheck/dimcheck.h"

#define GROUP_FLAG	1
#define CORNER_FLAG	2
#define ACHECK		1
#define DCHECK		2

/* This is the main procedure of the check program.      */
/* First the variable zero_flag is set according to the  */
/* options given to the program. If zero_flag = ON 	 */
/* errors of areas having one point in common are	 */
/* reported, if it is OFF they are not.			 */
/* Then, after initialisation of the datastructrures	 */
/* (routine ini ) it opens two files:			 */
/* 1. The EXPDATA file in which the names of the models  */
/*    to be checked are recorded.			 */
/* 2. The "dimcheckdata" file, which contains the data	 */
/* about the gaps and widths permitted for each layer.	 */
/* While reading the data from these files it sets up    */
/* two loops:					       	 */
/* 1. The outer loop: check each model recorded in the   */
/*    EXPDATA file.					 */
/* 2. The inner loop: check each layer recorded in the   */
/*    "dimcheckdata" file.				 */
/* In these loops 4 flags are updated:			 */
/* 1. The exgapflag. If this flag is 1 it means that for */
/*                   tracks laying parallel to each other*/
/*                   for only a short distance the gap   */
/*                   can be smaller then for tracks      */
/*                   laying parallel to each other for a */
/*                   longer distance.			 */
/*                   If this flag is 0 it means there is */
/*                   no difference between the two.	 */
/* 2. The widthflag. If this flag is 1 width_checks have */
/*                   to be carried out. If it is 0	 */
/*                   width_checks are not needed.	 */
/* 3. The maxwidthflag. If this flag is 1 width_checks	 */
/*                   have to be carried out to see if 	 */
/*                   the width of an element is not too	 */
/*                   large.				 */
/* 4. The gapflag.   If this flag is 1 gap_checks have   */
/*                   to be carried out . If it is 0      */
/*                   gap_checks are not needed.		 */
/* Furthermore the variable MAXINFLUENCE is updated.     */
/* This variable gives the maximum x_value over which the*/
/* events of a layer may have an influence on the other  */
/* events.If the exgapflag is 1 it is set to the maximum */
/* of exgaplength, gap and width. If exgapflag is 0 it is*/
/* set to the maximum of gap and width.			 */
/* After these variables have been set the real work is  */
/* done in the procedure main_check and the stateruler   */
/* is freed (free_state_ruler)				 */
/* The loops are closed then and the program stops.	 */

static void addstring (char *main_string, char *sub_string);
static void check_cell (char *modelname, int group_check_all);
static void sig_handler (int sig);

char   *argv0 = "dimcheck";	/* program name */
char   *use_msg =		/* command line */
        "\nUsage: %s [-a|-d] [-f] [-g] [-t] [cell_name]\n\n";

DM_PROJECT * dmproject;
int     test;
char    header_errstr[MAXLINE + 1];
int     prog_flag;

int main (int argc, char *argv[])
{
    int     iarg;		/* argument number */
    char   *p;
    char   *desrul_file;
    char    filename[256];
    char    modelname[DM_MAXNAME + 1];
    int     group_check_all;
    int     file_flag;
    int     cell_name_flag;
    char    excl_option;

    excl_option = '\0';
    test = OFF;
    file_flag = OFF;
    group_check_all = ON;
    cell_name_flag = OFF;
    prog_flag = ACHECK;

    for (iarg = 1; iarg < argc && argv[iarg][0] == '-'; ++iarg) {
	for (p = &argv[iarg][1]; *p; ++p) {
	    switch (*p) {
		case 'g':
		    group_check_all = OFF;
		    break;
		case 'a':
		    if (excl_option == 'd') {
			fprintf (stderr, "\nthe -a and -d options are mutually exclusive");
			fprintf (stderr, use_msg, argv0);
			exit (1);
		    }
		    prog_flag = ACHECK;
		    excl_option = 'a';
		    break;
		case 't':
		    test = ON;
		    break;
		case 'd':
		    if (excl_option == 'a') {
			fprintf (stderr, "\nthe -a and -d options are mutually exclusive");
			fprintf (stderr, use_msg, argv0);
			exit (1);
		    }
		    prog_flag = DCHECK;
		    excl_option = 'd';
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
	    fprintf (stderr, "\ntoo many arguments");
	    fprintf (stderr, use_msg, argv0);
	    exit (1);
	}
	}

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
    ALLOC (b_pntr, buff);
    if (prog_flag == ACHECK) {
	if (file_flag == ON) {
	    if (access ("./dimcheckdata1", 0) == 0) {
		OPEN (pdat, "dimcheckdata1", "r");
	    }
	    else {
		fprintf (stderr, "\ndimcheckdata1 not in WD; i'll take the standard one\n");
		file_flag = OFF;
	    }
	}
	if (file_flag == OFF) {
	    desrul_file = dmGetMetaDesignData (PROCPATH, dmproject, "dimcheckdata1");
	    OPEN (pdat, desrul_file, "r");
	}
    }
    else {
	if (file_flag == ON) {
	    if (access ("./dimcheckdata2", 0) == 0) {
		OPEN (pdat, "dimcheckdata2", "r");
	    }
	    else {
		fprintf (stderr, "\ndimcheckdata2 not in WD; i'll take the standard one\n");
		file_flag = OFF;
	    }
	}
	if (file_flag == OFF) {
	    desrul_file = dmGetMetaDesignData (PROCPATH, dmproject, "dimcheckdata2");
	    OPEN (pdat, desrul_file, "r");
	}
    }
    if (cell_name_flag == OFF) {
	sprintf (filename, "%s/%s", dmproject -> dmpath, EXPDATA);
	OPEN (pexp, filename, "r");
	while (fscanf (pexp, "%s", modelname) != EOF) {
	    check_cell (modelname, group_check_all);
	}
    }
    else {
	check_cell (argv[argc - 1], group_check_all);
    }
    die (0);
    return (0);
}

static void check_cell (char *modelname, int group_check_all)
{
    DM_STREAM * p_info;
#ifdef DRCERRORSTREAM
    DM_STREAM * drcstream;
#endif
    DM_CELL * mod_key;
    char    rulename[MAXLINE];
    char    maskfile[DM_MAXNAME + 1];
    char    helpfile[MAXLINE];
    char    descrip[MAXLINE];
    char    sub_errstr[MAXLINE];
    int     c, kind, total_errs;

    mod_key = dmCheckOut (dmproject, modelname, WORKING, DONTCARE, LAYOUT, ATTACH);

    /* determine factor nr_samples.			 */
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

    total_errs = 0;
    descrip[0] = '\0';
#ifdef DRCERRORSTREAM
    if (prog_flag == ACHECK)
        drcstream = dmOpenStream (mod_key, "dim1", "w");
    else
        drcstream = dmOpenStream (mod_key, "dim2", "w");
    pout = drcstream->dmfp;
#else
    pout = stdout;
#endif
    fprintf (pout, "\ncell: %s\n\n", modelname);

    fseek (pdat, 0L, 0);	/* rewind */

    while (1) {
	while ((c = getc (pdat)) == '\n');
	if (c == '#') {
	    /* remove comment string */
	    fgets (line, MAXLINE, pdat);
	    continue;
	}
	else {
	    ungetc (c, pdat);
	}

	if (fscanf (pdat, "%s%s%d%d%d%d%d%s%[^\n]", maskfile, helpfile,
	    &width, &gap, &exgap, &exlength, &kind, rulename, descrip) == EOF) {
	    break;
	}

	if ((group_check_all == OFF) || ((kind & GROUP_FLAG) != 0))
	    group_check = OFF;
	else
	    group_check = ON;

	if ((kind & CORNER_FLAG) != 0)
	    zero_flag = ON;
	else
	    zero_flag = OFF;

/* get the model_layout info_file to see if the model		 */
/* has been multiplied with a factor to cope with slanted boxes	 */
/* or polygons.							 */

	head_err = NULL;

	pvln[0] = dmOpenStream (mod_key, maskfile, "r");
	if (strcmp (helpfile, "NOFILE") != 0)
	    pvln[1] = dmOpenStream (mod_key, helpfile, "r");
	else
	    pvln[1] = NULL;

	if (strncmp (maskfile, "bool", 4) == 0)
	    sprintf (header_errstr, "Rule no.:%8s Mask: %s ",
		    rulename, maskfile);
	else
	    sprintf (header_errstr, "Rule no.:%8s Mask: %.2s ",
		    rulename, maskfile);
	if (width != 0 && width != 10000 && width != -1)
	    sprintf (sub_errstr, "Width: %d ", width);
	addstring (header_errstr, sub_errstr);
	if (width == 10000 || width == -1) {
            /* This is to simulate the fact that initially a contant string
               was given as a second argument to addstring (). That caused
               a segmentation violation on linux machines for the statement
               sub_string[0] = '\0';
            */
            char buf[32];
            static int flag = 0;
            if (flag == 0) strcpy (buf, "Not exists "), flag = 1;
	    addstring (header_errstr, buf);
	    width = MAXINT;
	}
	if (width == 0)
	    widthflag = 0;
	else {
	    widthflag = 1;
	}
	if (gap != 0) {
	    sprintf (sub_errstr, "Gap: %d ", gap);
	    addstring (header_errstr, sub_errstr);
	    gapflag = 1;
	}
	else
	    gapflag = 0;
	if (gap != exgap && gap != 0 && exgap > 0)
	    sprintf (sub_errstr, "Exgap: %d Exlength: %d ", exgap, exlength);
	addstring (header_errstr, sub_errstr);
	if (exgap < 0) {
	    sprintf (sub_errstr, "maxwidth: %d", exlength);
	    addstring (header_errstr, sub_errstr);
	}
	sprintf (sub_errstr, "%s\n", descrip);
	addstring (header_errstr, sub_errstr);

/* now update the values of gap, width etc. according to the	 */
/* value of nr_samples found.					 */

	if (nr_samples != 0) {
	    if (gap != 0)
		gap = gap * nr_samples - 1;
	    if (width != 0)
		width = width * nr_samples - 1;
	    if (exgap != 0)
		exgap = exgap * nr_samples - 1;
	    if (exlength != 0)
		exlength = exlength * nr_samples + 1;
	}

	if (exgap < 0) {
	    maxwidth = exlength;
	    maxwidthflag = 1;
	    exgapflag = 0;
	    MAXINFLUENCE = MAX (maxwidth, gap);
	}
	else {
	    if (exgap == 0)
		exgap = gap;
	    if (exgap >= gap) {
		exgapflag = 0;
		maxwidthflag = 0;
		MAXINFLUENCE = MAX (width, gap);
	    }
	    else {
		exgapflag = 1;
		maxwidthflag = 0;
		MAXINFLUENCE = MAX (exlength + 1, MAX (width, gap));
	    }
	}
	Errno = 0;
	main_check ();
	free_state_ruler ();
	if (head_err) filter_err ();
	free_errs (head_err);
	total_errs = total_errs + Errno;
	if (Errno) fprintf (pout, "\n");
	dmCloseStream (pvln[0], COMPLETE);
	if (pvln[1]) dmCloseStream (pvln[1], COMPLETE);
    }
    if (total_errs == 0) fprintf (pout, "no errors found\n\n");
#ifdef DRCERRORSTREAM
    dmCloseStream (drcstream, COMPLETE);
#endif
    dmCheckIn (mod_key, COMPLETE);
}

static void sig_handler (int sig) /* signal handler */
{
    signal (sig, SIG_IGN); /* ignore signal */
    fprintf (stderr, "%s: interrupted due to signal: %d\n", argv0, sig);
    die (1);
}

static void addstring (char *main_string, char *sub_string)
{
    if ((strlen (main_string) + strlen (sub_string)) > (MAXLINE + 1)) {
	fprintf (stderr, "head_error_string:\n'%s.....\ngets too long", main_string);
	exit (1);
    }
    strcat (main_string, sub_string);
    sub_string[0] = '\0';
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
    if (status) fprintf (stderr, "%s: -- program aborted --\n", argv0);
    if (dmproject) dmCloseProject (dmproject, COMPLETE);
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
    if ((zero_flag == ON) || (p1 != p3) || (p2 != p4)) {
	if (Errno == 0) fprintf (pout, "%s", header_errstr);
	fprintf (pout, "error: %8s%8d,%8d%8d,%8d\n", str, p1, p2, p3, p4);
	Errno++;
    }
}
