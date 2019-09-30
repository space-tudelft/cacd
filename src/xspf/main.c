
/*
 * ISC License
 *
 * Copyright (C) 1997-2016 by
 *	Arjan van Genderen
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
#include "src/xspf/incl.h"

#define TOOLVERSION "3.09 16-Aug-2016"

char *argv0 = "xspf";
char *use_msg = "\nUsage: %s [-defghikonpruvxy -w width -z name -C file -D name -F outfile -O name -X lib] cell\n\n";

extern struct model_info *Netws;

DM_PROJECT *dmproject = NULL;

struct conduct *conducts = NULL;
struct contact *contacts = NULL;
int nr_of_conducts = 0;
int nr_of_contacts = 0;

FILE *fp_out;

int *Nil = NULL;
long lower[10], upper[10];
char attribute_string[512];

int omitres = 1;
int maxLL = 80; /* maximum output line length */
int use_spf = 1;
int use_spef = 0;
int year4; /* 4 digits year value */

int coordinates = 0;
int mask_info = 0;
int driver = 0;
int gnd_node = 0;
int choosebulk = 1;
int incl_model = 1;
int tog_pnod = 0;
int tog_nnod = 0;
int tog_nobrack = 0;
int tog_use0 = 0;
int tog_use0_save = 0;
int tog_vss0 = 0;
int tog_gnd0 = 0;
int tog_prExt = 1;
int xtree = 0;
int alsoImport = 0;
int verbose = 0;
int ofile = 0;
char *outFile = NULL;
int onlySubckt = 0;
int groundVnet = 0;
int useDBinames = 0;
char ofname[BUFSIZ];
char node0[32];
char *node0ptr = NULL;

char *nameGND   = NULL;
char *c_unit    = "FF";
char *divider   = "/";
char *delimiter = ":";
char *controlFile = NULL;
char *controlLabel = NULL;
char *maskFile = NULL;

char *excl_lib[40];
int excl_lib_cnt = 0;

char *snbulk = NULL;
char *spbulk = NULL;
int   searchInst = 0;
int   useInst = 1;
int   usePort = 1;
int   optS = 0;

time_t tval;
struct tm *tstruct;

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

void spefOpenNetwork (char *name)
{
    static int firsttime = 1;

    if (firsttime) firsttime = 0;
    else { /* empty the buffer */
	if (fp_out == stdout) oprint (0, "\n");
	oprint (0, "");
    }

    if (outFile) {
	if (fp_out) CLOSE (fp_out);
	sprintf (ofname, "%s", outFile);
	OPENW (fp_out, ofname);
    }
    else if (ofile) {
	if (fp_out) CLOSE (fp_out);
	sprintf (ofname, "%s.spef", name);
	OPENW (fp_out, ofname);
    }

    fprintf (fp_out, "*SPEF \"IEEE 1481-1999\"\n");
    fprintf (fp_out, "*DESIGN \"%s\"\n", name);
    fprintf (fp_out, "*DATE \"%d-%s-%d %d:%02d:%02d\"\n",
	tstruct -> tm_mday, mon2s (tstruct -> tm_mon), year4,
	tstruct -> tm_hour, tstruct -> tm_min,  tstruct -> tm_sec);
    fprintf (fp_out, "*VENDOR \"SPACE\"\n");
    fprintf (fp_out, "*PROGRAM \"%s\"\n", argv0);
    fprintf (fp_out, "*VERSION \"%s\"\n", TOOLVERSION);
    fprintf (fp_out, "*DESIGN_FLOW \"Example\"\n");
    fprintf (fp_out, "*DIVIDER %s\n", divider);
    fprintf (fp_out, "*DELIMITER %s\n", delimiter);
    fprintf (fp_out, "*BUS_DELIMITER [ ]\n");
    fprintf (fp_out, "*T_UNIT 1 NS\n");
    fprintf (fp_out, "*C_UNIT 1 %s\n", c_unit);
    fprintf (fp_out, "*R_UNIT 1 OHM\n");
    fprintf (fp_out, "*L_UNIT 1 HENRY\n\n");
 // fprintf (fp_out, "*POWER_NETS VDD\n");
    if (nameGND && !optS) fprintf (fp_out, "*GROUND_NETS %s\n", nameGND);
}

int main (int argc, char *argv[])
{
    FILE *fp_top;
    char *s, *t;
    char *network;
    struct cir *cl;

    network = NULL;

    if ((s = strrchr (*argv, '/'))) ++s;
    else if ((s = strrchr (*argv, '\\'))) ++s;
    else s = *argv;
#ifdef WIN32
    if ((t = strchr (s, '.'))) *t = 0;
#endif

    if (strncmp (s, "xspef", 5) == 0) {
	if (use_spf) {
	    use_spef = 1;
	    argv0 = "xspef";
	    use_msg = "\nUsage: %s [-cdefgGhilopqrsvxy -z name -C file -D name -F outfile -M file -O name -X lib] cell\n\n";
	}
	else {
	    P_E "%s incorrectly linked to %s!\n", s, argv0);
	    exit (1);
	}
	choosebulk = 0;
	incl_model = 0;
    }

    if (argc <= 1) P_E "%s %s\n", argv0, TOOLVERSION);

    while (--argc > 0) {
	if ((*++argv)[0] == '-') {
	    for (s = *argv + 1; *s; s++) {
		switch (*s) {
		    case 'M':
			if (!use_spef || maskFile) goto illopt;
			maskFile = *++argv;
			if (*(s + 1) || --argc <= 0 || !*maskFile) {
			    P_E use_msg, argv0);
			    exit (1);
			}
			break;
		    case 'C':
			controlFile = *++argv;
			if (*(s + 1) || --argc <= 0 || !*controlFile) {
			    P_E use_msg, argv0);
			    exit (1);
			}
			break;
		    case 'w':
			if (use_spef) goto illopt;
		    case 'D':
			t = *++argv;
			if (*(s + 1) || --argc <= 0 || !*t) {
			    P_E use_msg, argv0);
			    exit (1);
			}
			if (*s == 'w') {
			    maxLL = atoi (t);
			    if (maxLL && maxLL < 40) maxLL = 40;
			    break;
			}
			controlLabel = t;
			while (*t) { *t = toupper (*t); ++t; }
			break;
		    case 'F':
			ofile = 1;
			outFile = *++argv;
			if (*(s + 1) || --argc <= 0 || !*outFile) {
			    P_E use_msg, argv0);
			    exit (1);
			}
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
		    case 'c':
			if (!use_spef) goto illopt;
			coordinates = 1;
			break;
		    case 'd':
			if (use_spef) driver = 1;
			else useDBinames = 1;
			break;
		    case 'e': tog_nobrack = 1; break;
		    case 'f': ofile = 1; break;
		    case 'G': gnd_node = 1;
		    case 'g': groundVnet = 1; break;
		    case 'h': xtree = 1; break;
		    case 'i': alsoImport = 1; break;
		    case 'k': if (use_spef) goto illopt;
			      onlySubckt = 1; break;
		    case 'l': if (!use_spef) goto illopt;
			      searchInst = 1; break;
		    case 'n': if (use_spef) goto illopt;
			      tog_nnod = 1; break;
		    case 'o': if (!use_spef) incl_model = 0; else
			      omitres = 0; break;
		    case 'p': if (!use_spef) tog_pnod = 1;
			      else
			      usePort = 0; break;
		    case 'q': if (!use_spef) goto illopt;
			      useInst = 0; break;
		    case 'r': tog_prExt = 0; break;
		    case 's': optS = 1; break;
		    case 'u': if (use_spef) goto illopt;
			      choosebulk = 0; break;
		    case 'v': verbose = 1; break;
		    case 'x': tog_gnd0 = 1; break;
		    case 'y': tog_vss0 = 1; break;
		    case 'z':
		    case 'O':
			++argv;
			if (*(s + 1) || --argc <= 0 || !**argv) {
			    P_E use_msg, argv0);
			    exit (1);
			}
			if ((*s == 'z' && node0ptr) || (*s == 'O' && nameGND)) {
			    P_E "%s: option -%c twice specified\n", argv0, *s);
			    exit (1);
			}
			if (*s == 'z') node0ptr = *argv;
			else nameGND = strsave (*argv);
			break;
		    default:
illopt:
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

    if (driver && !coordinates) driver = 0;

    if (node0ptr) {
	if (strlen (node0ptr) > 30) {
	    P_E "%s: too long prefix: %s\n", argv0, node0ptr);
	    exit (1);
	}
	strcpy (node0, node0ptr);
    }

    if (maskFile) { /* spef only */
	FILE *fp;
	char buf[256];
	double val;
	int i, lay, lay2;
	int line = 0;

	OPENR (fp, maskFile, 1);
	while (fgets (buf, 256, fp)) {
	    ++line;
	    s = buf;
	    while (*s == ' ' || *s == '\t') ++s;
	    if (*s == '#' || *s == '\r' || *s == '\n') continue;
	    if (!(mask_info & 1) && strncmp (s, "conductors", 10) == 0) {
		nr_of_conducts = atoi (s+10);
		if (nr_of_conducts > 0) {
		    PALLOC (conducts, nr_of_conducts, struct conduct);
		    for (i = 0; i < nr_of_conducts;) {
			if (fgets (buf, 256, fp)) {
			    ++line;
			    s = buf;
			    while (*s == ' ' || *s == '\t') ++s;
			    if (*s == '#' || *s == '\r' || *s == '\n') continue;
			    while (*s != ' ' && *s != '\t') ++s; // skip name
			    if (sscanf (s, "%d %le", &lay, &val) == 2) {
				conducts[i].lay = lay;
				conducts[i].res = val;
				++i; continue;
			    }
			}
			fprintf (stderr, "%s: maskFile: conducts read error\n", argv0);
			exit (1);
		    }
		}
		mask_info |= 1;
	    }
	    else
	    if (!(mask_info & 2) && strncmp (s, "contacts", 8) == 0) {
		nr_of_contacts = atoi (s+8);
		if (nr_of_contacts > 0) {
		    PALLOC (contacts, nr_of_contacts, struct contact);
		    for (i = 0; i < nr_of_contacts;) {
			if (fgets (buf, 256, fp)) {
			    ++line;
			    s = buf;
			    while (*s == ' ' || *s == '\t') ++s;
			    if (*s == '#' || *s == '\r' || *s == '\n') continue;
			    while (*s != ' ' && *s != '\t') ++s; // skip name
			    if (sscanf (s, "%d %d %le", &lay, &lay2, &val) == 3) {
				contacts[i].lay1 = lay;
				contacts[i].lay2 = lay2;
				contacts[i].res = val;
				++i; continue;
			    }
			}
			fprintf (stderr, "%s: maskFile: contacts read error\n", argv0);
			exit (1);
		    }
		}
		mask_info |= 2;
	    }
	    else
		fprintf (stderr, "%s: maskFile: line %d: unexpected line/keyword\n", argv0, line);
	}
	if (!(mask_info & 1)) fprintf (stderr, "%s: maskFile: conducts not found\n", argv0);
	if (!(mask_info & 2)) fprintf (stderr, "%s: maskFile: contacts not found\n", argv0);
	CLOSE (fp);
    }

    dmInit (argv0);
    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    dm_get_do_not_alloc = 1; /* fast circuit streams read */

    cmc.inst_attribute = attribute_string;
    cmc.inst_lower = lower;
    cmc.inst_upper = upper;

    initIntrup ();

    if (ofile) {
        if (outFile) {
            sprintf (ofname, outFile);
        }
        else if (!use_spef) {
	    sprintf (ofname, "%s.spf", network);
        }
	if (!use_spef)
	    OPENW (fp_out, ofname);
    }
    else {
	fp_out = stdout;
    }

    fp_top = fp_out;

    if (!nameGND) nameGND = "GND";
    if (nameGND || tog_gnd0 || tog_vss0 || node0[0]) {
	tog_use0_save = tog_use0 = node0[0] ? strlen (node0) : 1;
    }

    initDevs ();

    readControl ();

    time (&tval);
    tstruct = localtime (&tval);

    year4 = tstruct -> tm_year + 1900;

    if (!use_spef) {
	fprintf (fp_out, "*|DSPF 1.0\n");
	fprintf (fp_out, "*|DESIGN \"%s\"\n", network);
	fprintf (fp_out, "*|DATE \"%d-%s-%d %d:%02d:%02d\"\n",
	    tstruct -> tm_mday, mon2s (tstruct -> tm_mon), year4,
	    tstruct -> tm_hour, tstruct -> tm_min,  tstruct -> tm_sec);
	fprintf (fp_out, "*|VENDOR \"SPACE\"\n");
	fprintf (fp_out, "*|PROGRAM \"%s\"\n", argv0);
	fprintf (fp_out, "*|VERSION \"%s\"\n", TOOLVERSION);
	fprintf (fp_out, "*|DIVIDER %s\n", divider);
	fprintf (fp_out, "*|DELIMITER %s\n", delimiter);
    }

    cl = cirTree (network);

    if (xtree) {
	if (choosebulk || incl_model) interDevs ();

	for (; cl; cl = cl -> next) {
	    if (verbose) P_E "Extracting %s\n", cl -> name);

	    if (use_spef) spefOpenNetwork (cl -> name);

	    xnetwork (cl -> name, cl -> proj, cl -> imported, cl -> orig_name, cl -> next ? 1 : 0);
	}
    }
    else {
	if (use_spef) spefOpenNetwork (cl -> name);
	if (choosebulk || incl_model) {
	    scanInst (cl -> orig_name, cl -> proj);
	    interDevs ();
	}
	if (verbose) P_E "Extracting %s\n", network);

	xnetwork (cl -> name, cl -> proj, cl -> imported, cl -> orig_name, 0);
    }

    prImpFunc ();
    prImpNetw ();  /* should actually be placed before xnetwork;
		      but then it is not (yet) working correctly */
    prImpDev ();

    oprint (0, "\n");  /* to empty the buffer */

    endDevs ();

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

    if (use_spef) return (strcmp (buf, "SPEF") == 0);
    return (strcmp (buf, "SPF") == 0);
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
