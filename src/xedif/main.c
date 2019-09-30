/*
 * ISC License
 *
 * Copyright (C) 1987-2013 by
 *	Arjan van Genderen
 *	Bastiaan Sneeuw
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
#include "src/xedif/incl.h"

#define TOOLVERSION "3.02 5-Mar-2013"

char *argv0 = "xedif";
char *use_msg = "\nUsage: %s [-Icefhisv -C file -D name -F outfile -X lib] cell\n\n";

extern struct model_info *Netws;
extern struct cir *beg_extl, *end_extl;
extern struct model_info *Devs, *mlast;
extern struct model_info *Funcs;
extern struct model_info *preFuncs, *flast; /* predefined functions */

DM_PROJECT *dmproject = NULL;
DM_PROJECT *currproj = NULL;

FILE *fp_out;
long *Nil = NULL;
int dialectCds = 0;
int tog_range = 0;
int tog_irange = 0;
int tog_srange = 0;
int tog_nobrack = 0;
int tog_arrayExpand = 0;
int xtree = 0;
int alsoImport = 0;
int verbose = 0;
int ofile = 0;

char ofname[BUFSIZ];
char edifline[BUFSIZ];

char *outFile = NULL;
char *controlFile = NULL;
char *controlLabel = NULL;

struct lib_model *lm_head = NULL;

char *excl_lib[40];
int excl_lib_cnt = 0;

static void initIntrup (void);

static void writeHeader (char *network)
{
    struct tm *tstruct;
    time_t tval;
    int year;

    time (&tval);
    tstruct = localtime (&tval);
    year = tstruct -> tm_year + 1900;

    oprint (0, "(edif ");
    oprint (0, network);
    oprint (0, "(edifVersion 2 0 0)(edifLevel 0)");
    oprint (0, "(keywordMap(keywordLevel 0))");
    oprint (0, "(status(written");
    sprintf (edifline, "(timeStamp %d %d %d %d %d %d)",
	year, tstruct -> tm_mon + 1, tstruct -> tm_mday,
	tstruct -> tm_hour, tstruct -> tm_min, tstruct -> tm_sec);
    oprint (0, edifline);
    /* oprint (0, "(author \"Dimes Design and Test Centre\")"); */
    sprintf (edifline, "(program \"%s %s\")", argv0, TOOLVERSION);
    oprint (0, edifline);
    oprint (0, "))");
}

/* procedure to print technology information from file libref.bas
   The information in the file libref.bas is between the tokens
   '{' and '}'. First the procedure scans for the '{' token to begin
   reading. If this token is found mode will be set to 1.  When the '}'
   token is reached the procedure ends. All the information between the
   tokens '{' and '}' is printed in the edif file using the oprint command.
*/
void prCdsTechnology ()
{
    FILE *proc_fp;
    int mode = 0;
    char buf[BUFSIZ];
    char *c;

    /* open the file with the Nelsis to CADENCE device/technology information */

    if (!(proc_fp = fopen (CADENCE_INIT_FILE, "r")))
    if (!(proc_fp = fopen ((char *)dmGetMetaDesignData (PROCPATH, dmproject, CADENCE_INIT_FILE), "r")))
	fatalErr ("Cannot open file:", CADENCE_INIT_FILE);

    while (fgets (buf, BUFSIZ, proc_fp)) {

	for (c = buf; *c && isspace ((int)*c); c++);
	if (!*c) continue;
	if (*c == '{' && !mode) { mode = 1; continue; }
	if (*c == '}' && mode) break;

	if (mode) oprint (0, c);
    }

    fclose (proc_fp);
}

int main (int argc, char *argv[])
{
    char *s, *t, *network;
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
		    case 'i':
			alsoImport = 1;
			break;
		    case 'X':
			t = *++argv;
			if (*(s + 1) || --argc <= 0 || !*t) {
			    P_E use_msg, argv0);
			    exit (1);
			}
			if (excl_lib_cnt < 40)
			    excl_lib[excl_lib_cnt++] = strsave (t);
			break;
		    case 'v':
			verbose = 1;
			break;
		    case 'I': dialectCds = 2; break; /* Schematic CADENCE */
		    case 'c': dialectCds = 1; break; /* NETLIST view */
		    case 'e': tog_arrayExpand = 1; break;
		    case 's': tog_srange = 1; break;
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
        else sprintf (ofname, "%s.edf", network);
	OPENW (fp_out, ofname);
    }
    else fp_out = stdout;

    if (tog_arrayExpand) tog_nobrack = 1;
    else tog_range = tog_irange = 1;

    initDevs ();

    readControl ();

    writeHeader (network);

    cl = cirTree (network);

    if (xtree) {
	/* Sneeuw: In case of CADENCE read file cds_nls_reflib */
	if (dialectCds) Cds_initDevs ();
	printDevs ();
	prExt ();

	for (; cl; cl = cl -> next) {
	    if (verbose) P_E "Extracting %s\n", cl -> name);

	    if (!currproj || cl -> proj != currproj) {
		if (currproj) oprint (0, ")");
		oprint (0, "(library ");
		oprint (0, projname (currproj = cl -> proj));
		/* Sneeuw: In case of Schematic CADENCE, print technology information */
		if (dialectCds > 1) prCdsTechnology ();
		else {
		    oprint (0, "(edifLevel 0)");
		    oprint (0, "(technology(numberDefinition))");
		}
	    }

	    xnetwork (cl -> name, cl -> proj, cl -> imported, cl -> orig_name, cl -> next ? 1 : 0);
	}
    }
    else {
	scanInst (cl -> orig_name, cl -> proj);

	/* Sneeuw: In case of CADENCE read file cds_nls_reflib */
	if (dialectCds) Cds_initDevs ();
	printDevs ();
	prExt ();

	oprint (0, "(library ");
	oprint (0, projname (currproj = dmproject));
	oprint (0, "(edifLevel 0)");
	oprint (0, "(technology(numberDefinition))");

	/* Sneeuw: In case of Schematic CADENCE, print technology information */
	if (dialectCds > 1) prCdsTechnology ();

	if (verbose) P_E "Extracting %s\n", network);

	xnetwork (cl -> name, cl -> proj, cl -> imported, cl -> orig_name, 0);
    }

    oprint (0, ")");
    oprint (0, "(design root(cellRef ");
    oprint (0, network);
    oprint (0, "(libraryRef ");
    oprint (0, projname (dmproject));
    oprint (0, "))))");
    oprint (0, "\n");  /* to empty the buffer */

    fprintf (fp_out, "\n");

    if (ofile) CLOSE (fp_out);

    if (dmproject) dmCloseProject (dmproject, COMPLETE);
    dmQuit ();

    return (0);
}

int isCurrentDialect (char *buf)
{
    char *s = buf;
    while (*s) { *s = toupper (*s); ++s; }

    if (controlLabel && strcmp (buf, controlLabel) == 0) return (1);

    if (dialectCds) {
	if (strcmp (buf, "CDS") == 0) return (1);
	return (strcmp (buf, "CADENCE") == 0);
    }
    return (strcmp (buf, "EDIF") == 0);
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
