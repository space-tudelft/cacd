/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	R. van der Valk
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

#include "src/cmsk/main.h"

#define BUFLEN 80

extern int yylineno;

char    buf[BUFLEN];
char    buf1[DM_MAXNAME+1];
char    buf2[DM_MAXNAME+1];

char   *argv0 = "cmsk";		/* program name */
char   *use_msg =		/* command line */
"\nUsage: %s [-sv] [-m mlist] cmkfile\n\n";

int main (int argc, char *argv[])
{
    register int c, i, j, k;
    int     usage = 0;
    char   *bmfile;
    FILE   *fpml = 0;
    DM_PROCDATA *process;

#ifdef SIGHUP
    signal (SIGHUP,  SIG_IGN); /* ignore hangup signal */
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);
#endif
    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	signal (SIGINT, sig_handler);
    signal (SIGTERM, sig_handler);
    signal (SIGILL,  sig_handler);
    signal (SIGFPE,  sig_handler);
#ifdef SIGBUS
    signal (SIGBUS,  sig_handler);
#endif
    signal (SIGSEGV, sig_handler);
#ifdef SIGPIPE
    signal (SIGPIPE, sig_handler);
#endif

    v_mode = 0;		/* false */
    makedatabase = 1;	/* true */
    bmfile = 0;		/* no bmlist-file specified */

    for (i = 1; i < argc && argv[i][0] == '-'; ++i) {
	k = i;
	for (j = 1; (c = argv[k][j]) != '\0'; ++j) {
	    switch (c) {
	    case 'm':
		bmfile = argv[++i];
		pr_opt (c, "no default masklist");
		break;

	    /***************
	    case 'o':
		pr_opt (c, "old patterns are overwritten");
		break;
	    ***************/

	    case 's':
		makedatabase = 0; /* false */
		pr_opt (c, "patterns are only syntax checked");
		break;

	    case 'v':
		++v_mode;
		pr_opt (c, "verbose mode");
		break;

	    default:
		++usage;
		pr_opt (c, "unknown option");
	    }
	}
    }

    if (i + 1 != argc || usage) {
	if (i + 1 < argc)
	    fprintf (stderr, "%s: too many arguments specified\n", argv0);
	if (i + 1 > argc)
	    fprintf (stderr, "%s: no cmkfile specified\n", argv0);

	fprintf (stderr, use_msg, argv0);
	exit (1);
    }

    if (!freopen (argv[i], "r", stdin)) {
	fprintf (stderr, "%s: cannot open cmkfile: %s\n", argv0, argv[i]);
	exit (1);
    }

    dmInit (argv0);
    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, dmproject);
    if (!process) {
	fprintf (stderr, "%s: cannot read process data\n", argv0);
	die ();
    }

    if (!bmfile) {
	bmfile = (char *) dmGetMetaDesignData (PROCPATH, dmproject, "bmlist.cmk");
	if (stat (bmfile, &st_buf) == -1) {
	    fprintf (stderr, "%s: warning: cannot read file: %s\n", argv0, bmfile);
	    bmfile = "";
	}
    }

    if (*bmfile) {
	if (!(fpml = fopen (bmfile, "r"))) {
	    fprintf (stderr, "%s: cannot open bmlist-file: %s\n", argv0, bmfile);
	    die ();
	}
    }

    resol = dmproject -> lambda;

    if (v_mode) {
	fprintf (stderr, "%s: using process: %s (procid = %d)\n",
	    argv0, process -> pr_name, dmproject -> procid);
	fprintf (stderr, "%s: using resolution factor: %g\n", argv0, resol);
    }

    d_i = int_res = d_f = 1 / resol;
    if (d_i != d_f) int_res = 0;

    nrofmasks = process -> nomasks;
    ldmlay = process -> mask_name;

    CALLOC_ARR (cmklay, char*, nrofmasks);
    CALLOC_ARR (layset, int  , nrofmasks);

    CALLOC_ARR (edges1[0], struct edge *, nrofmasks);
    CALLOC_ARR (edges1[1], struct edge *, nrofmasks);
    CALLOC_ARR (edges1[2], struct edge *, nrofmasks);

    CALLOC_ARR (edges2[0], struct edge *, nrofmasks);
    CALLOC_ARR (edges2[1], struct edge *, nrofmasks);
    CALLOC_ARR (edges2[2], struct edge *, nrofmasks);

    if (*bmfile) {
    while (fgets (buf, BUFLEN, fpml)) {
	if (*buf == '#') continue;
	if (sscanf (buf, "%s%s", buf1, buf2) != 2) {
	    fprintf (stderr, "%s: read error in bmlist-file: %s\n", argv0, bmfile);
	    die ();
	}
	for (i = 0; i < nrofmasks; ++i) {
	    if (strcmp (buf2, ldmlay[i]) == 0) break;
	}
	if (i >= nrofmasks) {
	    fprintf (stderr, "%s: masklist: unknown mask: %s\n", argv0, buf2);
	    die ();
	}
	if (layset[i]) {
	    fprintf (stderr, "%s: masklist: already used mask: %s\n", argv0, buf2);
	    die ();
	}
	for (j = 0; j < nrofmasks; ++j) {
	    if (layset[j])
	    if (strcmp (buf1, cmklay[j]) == 0) {
		fprintf (stderr, "%s: masklist: already used mask: %s\n", argv0, buf1);
		die ();
	    }
	}
	layset[i] = 1;
	cmklay[i] = strsave (buf1, (char *)0);
    }
    fclose (fpml);
    }

    for (i = 0; i < nrofmasks; ++i) {
	if (!layset[i]) {
	    for (j = 0; j < nrofmasks; ++j) {
		if (layset[j])
		if (strcmp (ldmlay[i], cmklay[j]) == 0) {
		    fprintf (stderr, "%s: cannot make cross reference for mask: %s (already used)\n",
			argv0, ldmlay[i]);
		    die ();
		}
	    }
	    layset[i] = 1;
	    cmklay[i] = ldmlay[i];
	}
    }

    init_pat_tree ();

    rad45 = atan (1.0);

    yylineno = 1;
    if (yyparse ()) {
	pr_err ("error: the last defined pattern", pat_name, "has no trailing 'end'");
	killpattern ();
    }

    fprintf (stderr, "%s: -- program finished --\n", argv0);
    if (dmproject) dmCloseProject (dmproject, COMPLETE);
    dmQuit ();
    exit (0);
    return (0);
}

void pr_opt (int opt, char *str)
{
    fprintf (stderr, "%s: option -%c: %s\n", argv0, opt, str);
}

char *mpn (char *s)
{
    register char *cs = buf;

    *cs++ = '\'';
    for (; *s != '\0'; ++s) {
	if (*s == '_') *cs++ = '-';
	else           *cs++ = *s;
    }
    *cs++ = '\'';
    *cs = '\0';

    return (buf);
}

void killpattern ()
{
    if (makepattern) {
	makepattern = 0;

	fprintf (stderr, "%s: pattern %s not stored in the database!\n",
	    argv0, mpn (pat_name));

	add_pat_tree ();
	tree_ptr -> correct = 0;

	if (pat_key) closepatdir (QUIT);
    }
}

void init_pat_tree () /* initialize the pattern tree */
{
    register char **ml;
    register IMPCELL **iml;

    if ((ml = (char **) dmGetMetaDesignData (CELLLIST, dmproject, LAYOUT))) {
	while ((pat_name = *ml++)) {
	    add_pat_tree ();
	    ++cell_no;
	}
    }

    if ((iml = (IMPCELL **) dmGetMetaDesignData (IMPORTEDCELLLIST, dmproject, LAYOUT))) {
	while (*iml) {
	    pat_name = (*iml) -> alias;
	    add_pat_tree ();
	    tree_ptr -> impcell = *iml++;
	}
    }

    pat_name = pat_buffer;
}

void sig_handler (int sig) /* signal handler */
{
    signal (sig, SIG_IGN); /* ignore signal */
    sprintf (buf, "%d", sig);
    fprintf (stderr, "\n");
    pr_err2 ("interrupted due to signal:", buf);
    killpattern ();
    die ();
}

char *strsave (char *s1, char *s2)
{
    char   *d; /* address of memory space */
    int     l1 = 0;
    int     l2 = 0;

    if (s1 && *s1) l1 = strlen (s1);
    if (s2 && *s2) l2 = strlen (s2);
    if (!(d = malloc ((unsigned) (l1 + l2 + 1)))) cant_alloc ();
    *d = '\0';
    if (l1) strcpy (d, s1);
    if (l2) strcpy (d + l1, s2);
    return (d);
}

void cant_alloc ()
{
    fprintf (stderr, "%s: cannot allocate memory\n", argv0);
    die ();
}

void dmError (char *s)
{
    fprintf (stderr, "%s: ", argv0);
    dmPerror (s);
    fprintf (stderr,
	"%s: error in ddm interface function\n", argv0);
    die ();
}

void die ()
{
    fprintf (stderr, "%s: -- program aborted --\n", argv0);
    if (dmproject) dmCloseProject (dmproject, QUIT);
    dmQuit ();
    exit (1);
}
