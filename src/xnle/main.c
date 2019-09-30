
/*
 * ISC License
 *
 * Copyright (C) 1987-2015 by
 *	Arjan van Genderen
 *	Peter Elias
 *	Sander de Graaf
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include <time.h>
#include "src/xnle/incl.h"

#define TOOLVERSION "3.03 10-Aug-2015"

char *argv0 = "xnle";
char *use_msg = "\nUsage: %s [-fhlv -n nbulk -p pbulk -C file -D name -F outfile] cell\n\n";

extern struct model_info *Netws;
extern struct cir *beg_extl, *end_extl;
extern struct model_info *Devs, *mlast;
extern struct model_info *Funcs;
extern struct model_info *preFuncs, *flast; /* predefined functions */

struct cir *allExts = NULL;
struct model_info *allDevs = NULL;
struct model_info *allFuncs = NULL;
struct model_info *allpreFuncs = NULL;

DM_PROJECT *dmproject = NULL;

FILE *fp_out;
long *Nil = NULL;

int year2; /* 2 digits year value */

int tog_longlist = 0;
int xtree = 0;
int verbose = 0;
int ofile = 0;
char *outFile = NULL;
char ofname[BUFSIZ];
char ofsname[BUFSIZ];
char edifline[BUFSIZ];

char *controlFile = NULL;
char *controlLabel = NULL;

struct lib_model *lm_head = NULL;

char *snbulk = NULL;
char *spbulk = NULL;
char **globNets;
#ifdef ADDGLOBALNETS
int *globNetsCheck;
char **globConA;
char **globConB;
#endif
int globNets_cnt = 0;

#define GLOBNETFILE "global_nets"

time_t tval;
struct tm *tstruct;

static void readGlobalNets (void);
static void initIntrup (void);

static char *mon2s (int nr)
{
    switch (nr) {
	case  0: return ("Jan");
	case  1: return ("Feb");
	case  2: return ("Mar");
	case  3: return ("Apr");
	case  4: return ("May");
	case  5: return ("Jun");
	case  6: return ("Jul");
	case  7: return ("Aug");
	case  8: return ("Sep");
	case  9: return ("Oct");
	case 10: return ("Nov");
	case 11: return ("Dec");
    }
    return ("???");
}

void nle_subnetwork (int mode, struct cir *cl)
{
    static long fpos, npos;

    if (mode == 0) { /* begin */
	char *name;
	char *subnetwork = cl -> name;

        if (outFile)
            sprintf (ofsname, outFile);
        else if (ofile)
	    sprintf (ofsname, "%s.nle", subnetwork);
	else
	    sprintf (ofsname, "%s.nle.%d", subnetwork, (int)getpid ());
	OPENW (fp_out, ofsname);

	if (!(name = getenv ("LOGNAME")))
	   if (!(name = getenv ("USER"))) name = "user_x";

	sprintf (edifline, " \"%d-%s-%02d\" \"%d:%02d:%02d\"",
		tstruct -> tm_mday, mon2s (tstruct -> tm_mon),
		year2, tstruct -> tm_hour,
		tstruct -> tm_min,  tstruct -> tm_sec);

	fprintf (fp_out, "#cell2 * %s nle * 1 any 0 NELSISr3\n", subnetwork);
	fprintf (fp_out, "#%s%s %s * .\n", edifline, edifline, name);
	fpos = ftell (fp_out);
	fprintf (fp_out, "writer        1\n");
	fprintf (fp_out, "file_version  1\n");
	fprintf (fp_out, "sections      1\n");
	npos = strlen (subnetwork);
	fprintf (fp_out, "netlist   1: %10ld %s\n\n\n", npos + 75, subnetwork);

	fprintf (fp_out, "B contents:  %s\n", subnetwork);
	fprintf (fp_out, "netlist    %10ld\n", 2 * npos + 234);
	npos = ftell (fp_out);
	fprintf (fp_out, "cells      %10d\n", 0);
	fprintf (fp_out, "switches   %10d\n", 0);
	fprintf (fp_out, "attributes %10d\n", 0);
	fprintf (fp_out, "physical   %10d\n", 0);
	fprintf (fp_out, "history    %10d\n", 0);
	fprintf (fp_out, "E contents\n\n\n");
    }
    else { /* end */
	struct model_info *ntw;
	long apos, cpos, pos1, pos2, pos3;
	DM_PROCDATA *process;

	process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, dmproject);

	outPos ();
	cpos = ftell (fp_out);
	fprintf (fp_out, "B subcells\n");
	for (cl = beg_extl; cl; cl = cl -> next) {
	    if (cl -> imported == IMPORTED)
		fprintf (fp_out, "MDE %s %s\n", cl -> orig_name, projname (cl -> proj));
	    else
		fprintf (fp_out, "NLE %s *\n", cl -> orig_name);
	}
	for (ntw = Devs; ntw; ntw = ntw -> next) {
	    if (!*ntw -> prefix) {
		if (ntw -> imported == IMPORTED)
		    fprintf (fp_out, "MDE %s %s\n", ntw -> orig_name, projname (ntw -> proj));
		else
		    fprintf (fp_out, "NLE %s *\n", ntw -> orig_name);
	    }
	}
	for (ntw = Funcs; ntw; ntw = ntw -> next) {
	    fprintf (fp_out, "NLE %s *\n", ntw -> orig_name);
	}
	for (ntw = preFuncs; ntw; ntw = ntw -> next) {
	    fprintf (fp_out, "NLE %s *\n", ntw -> orig_name);
	}
	fprintf (fp_out, "E subcells\n\n\n");

	apos = ftell (fp_out);
	fprintf (fp_out, "B attribute_list\n");
	fprintf (fp_out, "%10d program\n", 0);
	fprintf (fp_out, "%10d technology\n", 0);
	fprintf (fp_out, "%10d lambda\n", 0);
	fprintf (fp_out, "E attribute_list\n\n");
	pos1 = ftell (fp_out);
	fprintf (fp_out, "B A: program\n");
	fprintf (fp_out, "1 NELSIS%s %s;\n", argv0, TOOLVERSION);
	fprintf (fp_out, " 0;\nE\n\n");
	pos2 = ftell (fp_out);
	fprintf (fp_out, "B A: technology\n");
	fprintf (fp_out, "1    %s;\n", process -> pr_name);
	fprintf (fp_out, " 0;\nE\n\n");
	pos3 = ftell (fp_out);
	fprintf (fp_out, "B A: lambda\n");
	fprintf (fp_out, "1    %g;\n", dmproject -> lambda);
	fprintf (fp_out, " 0;\nE\n\n");

	fseek (fp_out, npos, 0);
	fprintf (fp_out, "cells      %10ld\n", cpos - fpos);
	fprintf (fp_out, "switches   %10d\n", 0);
	fprintf (fp_out, "attributes %10ld\n", apos - fpos);

	fseek (fp_out, apos, 0);
	fprintf (fp_out, "B attribute_list\n");
	fprintf (fp_out, "%10ld program\n", pos1 - fpos);
	fprintf (fp_out, "%10ld technology\n", pos2 - fpos);
	fprintf (fp_out, "%10ld lambda\n", pos3 - fpos);

	if (!ofile) {
	    fclose (fp_out);
	    if ((fp_out = fopen (ofsname, "r"))) {
		while (fgets (edifline, BUFSIZ, fp_out))
		    fputs (edifline, stdout);
		fclose (fp_out);
	    }
	    unlink (ofsname);
	    *ofsname = 0;
	}
    }
}

int main (int argc, char *argv[])
{
    struct model_info *ntw;
    DM_PROCDATA *process;
    FILE *fp_top;
    char *s, *t;
    char *network;
    long apos, cpos, pos1, pos2, pos3;
    long fpos = 0, npos = 0;
    struct cir *cl;

    network = NULL;

    if ((s = strrchr (*argv, '/'))) ++s;
    else if ((s = strrchr (*argv, '\\'))) ++s;
    else s = *argv;
#ifdef WIN32
    if ((t = strchr (s, '.'))) *t = 0;
#endif

    if (argc <= 1) P_E "%s %s\n", argv0, TOOLVERSION);

    while (--argc > 0) {
	if ((*++argv)[0] == '-') {
	    for (s = *argv + 1; *s; s++) {
		switch (*s) {
		    case 'C':
			controlFile = *++argv;
			if (*(s + 1) || --argc <= 0 || !*controlFile) {
			    P_E use_msg, argv0);
			    exit (1);
			}
			break;
		    case 'D':
			t = *++argv;
			if (*(s + 1) || --argc <= 0 || !*t) {
			    P_E use_msg, argv0);
			    exit (1);
			}
			controlLabel = t;
			while (*t) { *t = toupper (*t); ++t; }
			break;
		    case 'f':
			ofile = 1;
			break;
		    case 'F':
			ofile = 1;
			outFile = *++argv;
			if (*(s + 1) || --argc <= 0 || !*outFile) {
			    P_E use_msg, argv0);
			    exit (1);
			}
			break;
		    case 'h':
			xtree = 1;
			break;
		    case 'v':
			verbose = 1;
			break;
		    case 'l':
			tog_longlist = 1;
			break;
		    case 'n':
			snbulk = *++argv;
			if (*(s + 1) || --argc <= 0 || !*snbulk) {
			    P_E use_msg, argv0);
			    exit (1);
			}
			break;
		    case 'p':
			spbulk = *++argv;
			if (*(s + 1) || --argc <= 0 || !*spbulk) {
			    P_E use_msg, argv0);
			    exit (1);
			}
			break;
		    default:
			P_E "%s: illegal option: %c\n", argv0, *s);
			exit (1);
		}
	    }
	}
	else {
	    if (!network) network = *argv;
	    else {
		P_E use_msg, argv0);
		exit (1);
	    }
	}
    }
    if (!network) {
	P_E use_msg, argv0);
	exit (1);
    }

    dmInit (argv0);
    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    dm_get_do_not_alloc = 1; /* fast circuit streams read */

    initIntrup ();

    if (ofile) {
	if (outFile) sprintf (ofname, outFile);
	else sprintf (ofname, "%s.nle", network);
    }
    else {
	sprintf (ofname, "%s.nle.%d", network, (int)getpid ());
    }
    OPENW (fp_out, ofname);

    fp_top = fp_out;

    readGlobalNets ();

    initDevs ();

    readControl ();

    time (&tval);
    tstruct = localtime (&tval);

    if ((year2 = tstruct -> tm_year) > 99) year2 -= 100;

    if (!(s = getenv ("LOGNAME")))
       if (!(s = getenv ("USER"))) s = "user_x";

    sprintf (edifline, " \"%d-%s-%02d\" \"%d:%02d:%02d\"",
	    tstruct -> tm_mday, mon2s (tstruct -> tm_mon),
	    year2, tstruct -> tm_hour,
	    tstruct -> tm_min,  tstruct -> tm_sec);

    fprintf (fp_out, "#cell2 * %s nle * 1 any 0 NELSISr3\n", network);
    fprintf (fp_out, "#%s%s %s * .\n", edifline, edifline, s);
    fpos = ftell (fp_out);
    fprintf (fp_out, "writer        1\n");
    fprintf (fp_out, "file_version  1\n");
    fprintf (fp_out, "sections      1\n");
    npos = strlen (network);
    fprintf (fp_out, "netlist   1: %10ld %s\n\n\n", npos + 75, network);

    fprintf (fp_out, "B contents:  %s\n", network);
    fprintf (fp_out, "netlist    %10ld\n", 2 * npos + 234);
    npos = ftell (fp_out);
    fprintf (fp_out, "cells      %10d\n", 0);
    fprintf (fp_out, "switches   %10d\n", 0);
    fprintf (fp_out, "attributes %10d\n", 0);
    fprintf (fp_out, "physical   %10d\n", 0);
    fprintf (fp_out, "history    %10d\n", 0);
    fprintf (fp_out, "E contents\n\n\n");

    cl = cirTree (network);

    if (xtree) {
	struct model_info *tmp;

	allExts = beg_extl;
	allDevs = Devs;
	allFuncs = Funcs;
	allpreFuncs = preFuncs;

	for (; cl; cl = cl -> next) {
	    if (verbose) P_E "Extracting %s\n", cl -> name);

	    beg_extl = end_extl = NULL;
	    for (tmp = Netws; tmp; tmp = tmp -> next) {
		if (tmp -> dkey) { dmCheckIn (tmp -> dkey, COMPLETE); tmp -> dkey = NULL; }
	    }
	    for (tmp = Devs; tmp; tmp = tmp -> next) {
		if (tmp -> dkey) { dmCheckIn (tmp -> dkey, COMPLETE); tmp -> dkey = NULL; }
	    }
	    for (tmp = Funcs; tmp; tmp = tmp -> next) {
		if (tmp -> dkey) { dmCheckIn (tmp -> dkey, COMPLETE); tmp -> dkey = NULL; }
	    }
	    for (tmp = preFuncs; tmp; tmp = tmp -> next) {
		if (tmp -> dkey) { dmCheckIn (tmp -> dkey, COMPLETE); tmp -> dkey = NULL; }
	    }
	    Netws = Devs = mlast = Funcs = preFuncs = flast = NULL;
	    scanInst (cl -> orig_name, cl -> proj);
	    if (cl -> next) nle_subnetwork (0, cl);
	    else fp_out = fp_top;

	    xnetwork (cl -> name, cl -> proj, cl -> imported, cl -> orig_name, cl -> next ? 1 : 0);

	    if (cl -> next) {
		nle_subnetwork (1, cl);
		if (end_extl) { end_extl -> next = allExts; allExts = beg_extl; }
		if (mlast)    { mlast    -> next = allDevs; allDevs = Devs; }
		if (flast)    { flast    -> next = allpreFuncs; allpreFuncs = preFuncs; }
		if (Funcs) {
		    struct model_info *f = Funcs;
		    while (f -> next) f = f -> next;
		    f -> next = allFuncs;
		    allFuncs = Funcs;
		}
	    }
	}
    }
    else {
	scanInst (cl -> orig_name, cl -> proj);

	if (verbose) P_E "Extracting %s\n", network);

	xnetwork (cl -> name, cl -> proj, cl -> imported, cl -> orig_name, 0);
    }

    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, dmproject);

    outPos ();
    cpos = ftell (fp_out);
    fprintf (fp_out, "B subcells\n");
    for (cl = beg_extl; cl; cl = cl -> next) {
	if (cl -> imported == IMPORTED)
	    fprintf (fp_out, "MDE %s %s\n", cl -> orig_name, projname (cl -> proj));
	else
	    fprintf (fp_out, "NLE %s *\n", cl -> orig_name);
    }
    for (ntw = Devs; ntw; ntw = ntw -> next) {
	if (!*ntw -> prefix) {
	    if (ntw -> imported == IMPORTED)
		fprintf (fp_out, "MDE %s %s\n", ntw -> orig_name, projname (ntw -> proj));
	    else
		fprintf (fp_out, "NLE %s *\n", ntw -> orig_name);
	}
    }
    for (ntw = Funcs; ntw; ntw = ntw -> next) {
	fprintf (fp_out, "NLE %s *\n", ntw -> orig_name);
    }
    for (ntw = preFuncs; ntw; ntw = ntw -> next) {
	fprintf (fp_out, "NLE %s *\n", ntw -> orig_name);
    }
    fprintf (fp_out, "E subcells\n\n\n");

    apos = ftell (fp_out);
    fprintf (fp_out, "B attribute_list\n");
    fprintf (fp_out, "%10d program\n", 0);
    fprintf (fp_out, "%10d technology\n", 0);
    fprintf (fp_out, "%10d lambda\n", 0);
    fprintf (fp_out, "E attribute_list\n\n");
    pos1 = ftell (fp_out);
    fprintf (fp_out, "B A: program\n");
    fprintf (fp_out, "1 NELSIS%s %s;\n", argv0, TOOLVERSION);
    fprintf (fp_out, " 0;\nE\n\n");
    pos2 = ftell (fp_out);
    fprintf (fp_out, "B A: technology\n");
    fprintf (fp_out, "1    %s;\n", process -> pr_name);
    fprintf (fp_out, " 0;\nE\n\n");
    pos3 = ftell (fp_out);
    fprintf (fp_out, "B A: lambda\n");
    fprintf (fp_out, "1    %g;\n", dmproject -> lambda);
    fprintf (fp_out, " 0;\nE\n\n");

    fseek (fp_out, npos, 0);
    fprintf (fp_out, "cells      %10ld\n", cpos - fpos);
    fprintf (fp_out, "switches   %10d\n", 0);
    fprintf (fp_out, "attributes %10ld\n", apos - fpos);

    fseek (fp_out, apos, 0);
    fprintf (fp_out, "B attribute_list\n");
    fprintf (fp_out, "%10ld program\n", pos1 - fpos);
    fprintf (fp_out, "%10ld technology\n", pos2 - fpos);
    fprintf (fp_out, "%10ld lambda\n", pos3 - fpos);

    CLOSE (fp_out);

    if (!ofile) {
	if ((fp_out = fopen (ofname, "r"))) {
	    while (fgets (edifline, BUFSIZ, fp_out))
		fputs (edifline, stdout);
	    fclose (fp_out);
	}
	unlink (ofname);
	*ofname = 0;
    }

    if (dmproject) dmCloseProject (dmproject, COMPLETE);
    dmQuit ();

    return (0);
}

int isCurrentDialect (char *buf)
{
    char *s = buf;
    while (*s) { *s = toupper (*s); ++s; }

    if (controlLabel && strcmp (buf, controlLabel) == 0) return (1);

    return (strcmp (buf, "NLE") == 0);
}

static void readGlobalNets ()
{
    FILE *fp;
    int cnt, i;
    char *fn;
    char buf[128];

    if (!(fp = fopen (GLOBNETFILE, "r"))) {
	fn = (char *) dmGetMetaDesignData (PROCPATH, dmproject, GLOBNETFILE);
	fp = fopen (fn, "r");
    }
    if (fp) {
	cnt = 0;
	while (fscanf (fp, "%s", buf) > 0) cnt++;
	rewind (fp);

	PALLOC (globNets, cnt, char *);
#ifdef ADDGLOBALNETS
	PALLOC (globNetsCheck, cnt, int);
	PALLOC (globConA, cnt, char *);
	PALLOC (globConB, cnt, char *);
#endif /* ADDGLOBALNETS */

	cnt = 0;
	while (fscanf (fp, "%s", buf) > 0) {
	    for (i = 0; i < cnt; i++) {
		if (strcmp (globNets[i], buf) == 0)
		    break; /* double specification of this global net */
	    }
	    if (i == cnt) globNets[cnt++] = strsave (buf);
	}
	globNets_cnt = cnt;

	fclose (fp);
    }
}

void fatalErr (char *s1, char *s2)
{
    P_E "%s:", argv0);
    if (s1 && *s1) P_E " %s", s1);
    if (s2 && *s2) P_E " %s", s2);
    P_E "\n");
    if (ofile && fp_out) {
	oprint (0, ""); /* to empty the buffer */
	fprintf (fp_out, "\nfatalError:");
	if (s1 && *s1) fprintf (fp_out, " %s", s1);
	if (s2 && *s2) fprintf (fp_out, " %s", s2);
	fprintf (fp_out, "\n");
    }
    die ();
}

void int_hdl (int sig) /* interrupt handler */
{
    char *s;
    switch (sig) {
#ifdef SIGILL
	case SIGILL: s = "Illegal instruction"; break;
#endif
#ifdef SIGFPE
	case SIGFPE: s = "Floating point exception"; break;
#endif
#ifdef SIGBUS
	case SIGBUS: s = "Bus error"; break;
#endif
#ifdef SIGSEGV
	case SIGSEGV: s = "Segmentation violation"; break;
#endif
	default: s = "Unknown signal"; break;
    }
    P_E "%s\n", s);
    if (ofile && fp_out) {
	oprint (0, ""); /* to empty the buffer */
	fprintf (fp_out, "\ninterruptError: Some signal occurred!\n");
    }
    die ();
}

static void initIntrup ()
{
#define install_handler(sig) signal (sig, int_hdl)
#ifdef SIGINT
    if (signal (SIGINT, SIG_IGN) != SIG_IGN) install_handler (SIGINT);
#endif
#ifdef SIGQUIT
    if (signal (SIGQUIT, SIG_IGN) != SIG_IGN) install_handler (SIGQUIT);
#endif
#ifdef SIGTERM
    install_handler (SIGTERM);
#endif
#ifdef SIGILL
    install_handler (SIGILL);
#endif
#ifdef SIGFPE
    install_handler (SIGFPE);
#endif
#ifdef SIGBUS
    install_handler (SIGBUS);
#endif
#ifdef SIGSEGV
    install_handler (SIGSEGV);
#endif
}

void dmError (char *s)
{
    P_E "%s: ", argv0);
    dmPerror (s);
    if (ofile && fp_out) {
	oprint (0, ""); /* to empty the buffer */
	fprintf (fp_out, "\ndmError: %s: Error in database function!\n", s);
    }
    die ();
}

void die ()
{
    if (ofile && fp_out) { CLOSE (fp_out); fp_out = NULL; }
    dmQuit ();
    if (!ofile) {
	if (*ofname) unlink (ofname);
	if (*ofsname) unlink (ofsname);
    }
    exit (1);
}

void cannot_die (int nr, char *fn)
{
    char *s;

    switch (nr) {
	case 1: s = "Cannot allocate"; fn = "storage"; break;
	case 2: s = "Cannot read file:"; break;
	case 3: s = "Cannot write file:"; break;
	default:
	case 4: s = "Internal error:"; break;
    }
    P_E "%s: %s %s\n", argv0, s, fn);
    if (ofile && fp_out) {
	oprint (0, ""); /* to empty the buffer */
	fprintf (fp_out, "\nError: %s %s!\n", s, fn);
    }
    die ();
}
