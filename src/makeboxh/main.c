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

#include "src/makeboxh/extern.h"

struct ctree *celltree;	/* root node of cell-tree */
struct cptrl *cptrhead;	/* head node of cell-ptr-list */
struct cptrl *cptrlast;	/* last node of cell-ptr-list */
struct cptrl *cptr;	/* cell-ptr of current cell */
struct wdw  *mc_bboxl;	/* head node of mc-bbox-list */
struct wdw  **arl_ptr;	/* ptr to current act-reg-list */

DM_PROCDATA *process;	/* ptr to process information */
DM_PROJECT  *project;	/* project access key */
DM_CELL     *cellkey;	/* cell access key */
DM_CELL     *top_key;	/* top cell access key */
DM_STREAM  **fp_bxx;	/* pointers to "_bxx" files */
DM_STREAM   *fp_tid;	/* pointer to "exp_tid" file */
char    fn_exp[80];     /* path of "exp_dat" file */

long   *no_bxx;		/* no_of_boxes array */
long    exp_reg[4];     /* expansion region array */
int     part_exp = FALSE; /* partial expansion flag */
long    maxol = 6;      /* default max. overlay value */
long    exp_depth = 0;  /* depth of linear expansion */
long    usr_chlev = 1;  /* hier. check level */
long    term_no;        /* count number of terminals */
long    chtype;         /* current checktype */
long    fchtype;        /* final checktype */
int     mask_no;        /* mask number */
int     level;          /* level of current cell */
char ** coord_list = 0;
char  * topcellname = NULL;	/* cell to be expanded */
int     verbose = FALSE;/* verbose mode flag */

char    fname[256];	/* complete filename path */
char   *argv0 = "makeboxh";	/* program name */
char   *use_msg =		/* command line */
"\nUsage: %s [-v] [-cL] [-oM] [-wXl,Xr,Yb,Yt] cell\n\n";

char *setval (long *ip, char *s);
void sig_handler (int sig);

int main (int argc, char *argv[])
{
    char   *p;
    int     iarg;	    /* argument number */
    int     partial = FALSE;/* partial expansion flag */
    int     usage = 0;

    for (iarg = 1; iarg < argc && argv[iarg][0] == '-'; iarg++) {
	p = &argv[iarg][1];
	while (*p != '\0') {
	    switch (*p++) {
		case 'c':
		    p = setval (&exp_depth, p);
		    if (exp_depth <= 0) {
			++usage;
			errexit (-8, "");
		    }
		    break;
		case 'o':
		    p = setval (&maxol, p);
		    break;
		case 'v':
		    verbose = TRUE;
		    break;
		case 'w':
		    partial = TRUE;
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
		    ||  exp_reg[2] >= exp_reg[3]) {
			++usage;
			errexit (-3, ""); /* incorrect window */
		    }
		    break;
		default:
		    ++usage;
		    P_E "%s: -%c: unknown option\n",
				argv0, *(p - 1));
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

#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN); /* ignore hangup signal */
#endif
    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	signal (SIGINT, sig_handler);
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);
#endif
    signal (SIGTERM, sig_handler);

    dmInit (argv0);
    project = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);
    sprintf (fn_exp, "%s/exp_dat", project -> dmpath);
    unlink (fn_exp);

    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, project);

    if (exp_depth)
	usr_chlev = exp_depth;

    if (verbose) {
	P_E "process: %s (procid = %d)\n",
		process -> pr_name, project -> procid);
	P_E "hierarchical expansion\n");
	if (partial) {
	    P_E "exp. region: %ld, %ld, %ld, %ld\n",
		exp_reg[0], exp_reg[1], exp_reg[2], exp_reg[3]);
	}
	P_E "user check level: %ld\n", usr_chlev);
	P_E "max overlay val.: %ld\n", maxol);
    }

    /* allocate and initialize root nodes of cell-tree
    ** and cell-ptr-list; init also file "exp_dat".
    */
    celltree = upd_cptrl (topcellname);
    cptrhead = cptrlast;

    for (cptr = cptrhead; cptr; cptr = cptr -> next) {
#ifdef DEBUG
pr_cptrl (cptr);
pr_ctree (cptr -> cell);
P_E "=> exp_cell(cptr->cell): %08x (cptr = %08x)\n",
cptr -> cell, cptr);
#endif
	if (verbose)
	    P_E "cell: %s\n", cptr -> cell -> name);
	exp_cell (cptr -> cell, partial);
	partial = FALSE;
    }

    dmCloseProject (project, COMPLETE);
    dmQuit ();
    if (verbose) P_E "%s: -- program finished --\n", argv0);
    exit (0);
    return (0);
}

char *setval (long *ip, char *s)
{
    int  neg = 1;
    long val = 0;

    if (*s == '-') {
	neg = -1;
	++s;
    }
    while (*s >= '0' && *s <= '9')
	val = (10 * val) + (*s++ - '0');
    *ip = val * neg;
    return (s);
}

void sig_handler (int sig) /* signal handler */
{
    char buf[6];
    signal (sig, SIG_IGN); /* ignore signal */
    sprintf (buf, "%d", sig);
    errexit (9, buf);
}
