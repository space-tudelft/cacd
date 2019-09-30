/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

#include <math.h>
#include <signal.h>
#include "src/makeboxl/extern.h"

DM_PROCDATA *process;
struct clist *celllist = 0;
struct clist *topcell;
struct tmtx  *tm_p, *tm_s;

DM_PROJECT *project;	/* project access key */
DM_CELL    *cellkey;	/* cell access key */
DM_STREAM **fp_bxx;	/* pointers to "bxx" files */
DM_STREAM **fp_nxx;	/* pointers to "nxx" files */
DM_STREAM  *fp_tidpos;  /* pointer to "tidpos" file (position of instances) */
DM_STREAM  *fp_tidnam;  /* pointer to "tidnam" file (long names) */
DM_STREAM  *fp_anno_exp; /* pointer to "anno_exp" file (expanded hierarchical
                            terminals and labels) */
DM_STREAM  *fp_tid;	/* pointer to "tid" file */
DM_STREAM  *fp_spec;	/* pointer to "spec" file */

double *px, *py;	/* pointer to point array x,y */
double  rad01, rad90;	/* number of rad's for x degrees */
char    buf[DM_MAXPATHLEN]; /* temp. string buffer */
char   *cellname;	/* current cell name */
char   *topcellname;	/* top cell name */

long   *no_bxx;		/* number of boxes */
long    exp_reg[4];     /* expansion region */
int     part_exp = 0;   /* partial expansion flag */
int     dflag = 0;      /* discretisize nor-elmts to boxes */
long    exp_depth = -1; /* depth of linear expansion */
long    term_no = 0;    /* count number of terminals */
int     mask_no;        /* mask number */
int     t_mask_no = 0;  /* terminal mask number */
int     level = 1;      /* level of current cell */
int     pt_size = 512;  /* size of point array's */
int     s_mode;         /* special mode flag */
int     Lflag = 0;          /* only Local cells (if true) */
int     verbose = 0;    /* verbose mode flag */
int     noImageMode = 0; /* no automatic image mode for SoG (seadif etc.) */
int     Image_done = 0; /* skip all images for SoG (seadif etc.) */
int	default_status = 0;/* default status for flat mode */
int     flat_mode = 1;  /* flat mode flag */
int     hier_mode = 0;  /* hierarchical mode flag */
int     pseudo_hier_mode = 0;  /* pseudo hierarchical mode flag
                                  Layout is expanded but also cell calls
                                  are preserved in the netlist */
int	lookatdevmod = 1; /* look if a subcell is a device */
int     extraStreams = 1; /* generate streams: tidpos, tidnam and anno_exp */
int     usage = 0;
long    nr_of_s = 0;
char    ** coord_list = NULL;
int     noErrMes = 0;

char * imageName = NULL;

long   samples = 0;

char   *argv0 = "makeboxl";     /* program name */
char   *use_msg =               /* command line */
"\nUsage: %s [-Hh] [-L] [-cL] [-d[N]] [-v] [-wXl,Xr,Yb,Yt] [-Ii] cell\n\n";

/* local operations */
Private char *setval (long *ip, char *s);
void sig_handler (int sig);

int main (int argc, char *argv[])
{
    FILE   *fp_exp;
    char   *p;
    int     iarg;	    /* argument number */

    for (iarg = 1; iarg < argc && argv[iarg][0] == '-'; ++iarg) {
	p = &argv[iarg][1];
	while (*p) {
	    switch (*p++) {
		case 'L':
		    ++Lflag;
		    break;
		case 'c':
		    p = setval (&exp_depth, p);
		    if (exp_depth < 0) {
			++usage;
			errexit (-8, "");
		    }
		    break;
		case 'd':
		    ++dflag;
		    p = setval (&nr_of_s, p);
		    samples = nr_of_s;
		    if (samples != 0 && samples != 1 && samples != 2
			&& samples != 4 && samples != 5
			&& samples != 8 && samples != 10) {
			++usage;
			errexit (-11, "");
		    }
		    break;
		case 'v':
		    ++verbose;
		    break;
		case 'w':
		    ++part_exp;
		    if (*(p = setval (&exp_reg[0], p)) != ',') {
			++usage;
			errexit (-2, "");
			break;
		    }
		    if (*(p = setval (&exp_reg[1], ++p)) != ',') {
			++usage;
			errexit (-2, "");
			break;
		    }
		    if (*(p = setval (&exp_reg[2], ++p)) != ',') {
			++usage;
			errexit (-2, "");
			break;
		    }
		    p = setval (&exp_reg[3], ++p);
		    if (exp_reg[0] >= exp_reg[1]
			|| exp_reg[2] >= exp_reg[3]) {
			++usage;
			errexit (-3, ""); /* incorrect window */
		    }
		    break;
		case 'H':
		    ++hier_mode;
		    break;
		case 'h':
		    ++pseudo_hier_mode;
		    break;
		case 'x':
		    lookatdevmod = 0;
		    break;
		case 'i':
		    ++noImageMode;
		    break;
		case 'I':
		    ++Image_done;
		    break;
		case 'b':
		    extraStreams = 0;
		    break;
		default:
		    ++usage;
		    P_E "%s: -%c: unknown option\n", argv0, *(p - 1));
		    break;
	    }
	}
    }

    if (argc != iarg + 1) {
	++usage;
	if (iarg >= argc)
	    errexit (-1, "");
	else
	    errexit (-5, "");
    }
    if (usage) {
	P_E use_msg, argv0);
	exit (1);
    }

    topcellname = argv[iarg];

    if (hier_mode || pseudo_hier_mode) {
	flat_mode = 0;
	default_status = pseudo_hier_mode ? 2 : 1;
    }

#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN); /* ignore hangup signal */
#endif

#ifdef SIGINT
    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	signal (SIGINT, sig_handler);
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);
#endif
#ifdef SIGTERM
    signal (SIGTERM, sig_handler);
#endif

    dmInit (argv0);
    project = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    sprintf (buf, "%s/exp_dat", project -> dmpath);
    unlink (buf);

    if (dflag && samples == 0) samples = project -> n_samples;

    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, project);

    if (exp_depth < 0) exp_depth = 1000; /* arbitrary big integer */

    if (1) {
        char * fn_m_c;
        FILE * fp_m_c;
        if ((fn_m_c = dmGetMetaDesignData (PROCPATH, project, "mask_copies"))
            && (fp_m_c  = fopen (fn_m_c, "r"))) {
	    P_E "%s: warning: mask_copies not more supported!\n", argv0);
            fclose (fp_m_c);
        }
    }

    if (Image_done) {
	P_E "%s: warning: skipping all fishbone images!\n", argv0);
	imageName = "IMAGE";
    }
    else if (noImageMode == 0) {
	sprintf (buf, "%s/seadif", project -> dmpath);
	if (access (buf, W_OK) == 0) {
	    if (verbose) P_E "%s: running in fishbone image mode\n", argv0);
	    imageName = "IMAGE";
	}
    }

    if (verbose) {
	P_E "process is \"%s\" (procid=%d)\n", process -> pr_name, project -> procid);
	P_E "project is \"%s\"\n", project -> dmpath);
	if (!flat_mode)
	    P_E "mixed linear/hierarchical expansion (lin. exp. depth = %ld)\n", exp_depth);
	else
	    P_E "linear expansion (exp. depth = %ld)\n", exp_depth);
	if (Lflag)
	    P_E "expansion of local cells only\n");
	if (dflag)
	    P_E "discretization (nr. of samples = %ld)\n", samples);
	if (part_exp) {
	    P_E "partial expansion (exp. region = %ld, %ld, %ld, %ld)\n",
		exp_reg[0], exp_reg[1], exp_reg[2], exp_reg[3]);
	}
	P_E "cell: %s\n", topcellname);
    }

    rad90 = 2.0 * atan (1.0);
    rad01 = rad90 / 90.0;

    exp_cell (topcellname);

    sprintf (buf, "%s/exp_dat", project -> dmpath);
    if (!(fp_exp = fopen (buf, "w"))) errexit (4, buf);
    fprintf (fp_exp, "%s\n", topcellname);
    fclose (fp_exp);

    dmCloseProject (project, COMPLETE);
    dmQuit ();

    if (verbose) P_E "%s: -- program finished --\n", argv0);
    exit (0);
    return (0);
}

Private char *setval (long *ip, char *s)
{
    int  neg = 1;
    long i = 0;

    if (*s == '-') {
	neg = -1;
	++s;
    }
    while (*s >= '0' && *s <= '9')
	i = (10 * i) + (*s++ - '0');
    *ip = i * neg;
    return (s);
}

void sig_handler (int sig) /* signal handler */
{
    signal (sig, SIG_IGN); /* ignore signal */
    sprintf (buf, "%d", sig);
    errexit (9, buf);
}
