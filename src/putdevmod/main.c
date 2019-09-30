/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	A.J. van Genderen
 *	S. de Graaf
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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "src/libddm/dmincl.h"

// #define DRIVER

char *argv0 = "putdevmod";
char *use_msg = "\nUsage: %s [-v] file [...]\n\n";

DM_PROJECT *dmproject = NULL;
int lineno;
int spicemod;
int verbose = 0;
int wordOnNewline = 0;

void die (void);
void fatalErr (char *s1, char *s2);
int  getword (FILE *fp, char *buf, int onSameLine, int onNewLine);
void initIntrup (void);
void int_hdl (int sig);
void parseDesc (char *infile);

int main (int argc, char *argv[])
{
    struct stat sbuf;
    char *s;
    int k;
    int nr_infile = 0;

    for (k = 1; k < argc; k++) {
	if (argv[k][0] == '-') {
	    for (s = argv[k] + 1; *s; s++) {
		switch (*s) {
		    case 'v':
			verbose = 1;
			break;
		    default:
			fprintf (stderr, "%s: illegal option: %c\n", argv0, *s);
			exit (1);
		}
	    }
	}
	else {
	    nr_infile++;
	}
    }
    if (nr_infile == 0) {
	fprintf (stderr, use_msg, argv0);
	exit (1);
    }

#ifndef DRIVER
    dmInit (argv0);
    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);
    if (dmStatXData (dmproject, &sbuf) == 0) { /* SdeG4.18 */
	fprintf (stderr, "%s: Warning: Use program xcontrol to set device status!\n", argv0);
    }
#endif

    initIntrup ();

    for (k = 1; k < argc; k++) {
        if (argv[k][0] != '-') parseDesc (argv[k]);
    }

#ifndef DRIVER
    if (dmproject) dmCloseProject (dmproject, COMPLETE);
    dmQuit ();
#endif
    return (0);
}

void parseDesc (char *infile)
{
    struct stat sbuf;
    long lower[3], upper[3];
    char *s, devname[80];
    char terminals[40][80];   /* maximum 40 terminals of size 79 */
    int model = 0;
    int term_cnt;
    int total_cnt;
    int sterm_cnt = 0;
    int prefix;
    FILE *fpin, *fpout;
    char keyword[81];
    char buf[81];
    int c, i, n, mod;
    DM_STREAM *dsp_term;
    DM_STREAM *dsp_devmod;
    DM_STREAM *dsp;
    DM_CELL *dmkey_cir;

    if (!(fpin = fopen (infile, "r"))) fatalErr ("Cannot open", infile);

    lineno = 1;

    if (getword (fpin, keyword, 0, 0) > 0 && strcmp (keyword, "device") == 0) {
	if (getword (fpin, devname, 1, 0) <= 0) {
	    sprintf (buf, "%d", lineno);
	    fatalErr ("syntax error at line", buf);
	}
    }
    else {
	sprintf (buf, "%d", lineno);
	fatalErr ("syntax error at line", buf);
    }

#ifdef DRIVER
    verbose = 1;
#endif
    if (verbose) printf ("%s: device \"%s\"\n", argv0, devname);
    spicemod = mod = 0;
    term_cnt = 0;
    prefix = 0;

    while (getword (fpin, keyword, 0, 1) > 0) { /* on new line */
#ifdef DRIVER
	printf ("-- keyword=\"%s\" spicemod=%d wordOnNewline=%d\n", keyword, spicemod, wordOnNewline);
#endif
        if (mod) {
	    if (spicemod == 1 && strcmp (keyword, "terminals") == 0) {
		while (getword (fpin, keyword, 1, 0) > 0) { /* on same line */
		    if (verbose) printf ("%s: terminal \"%s\"\n", argv0, keyword);
		    if (term_cnt > 39) fatalErr ("Sorry, cannot handle more than 40 terminals", 0);
		    if (!isalpha (*keyword)) fatalErr ("Sorry, incorrect terminal name:", keyword);
		    for (n = 0; n < term_cnt; ++n) {
			if (strcmp (terminals[n], keyword) == 0)
			    fatalErr ("Sorry, duplicate terminal name:", keyword);
		    }
		    strcpy (terminals[term_cnt], keyword);
		    term_cnt++;
		}
	    }
	    else if (spicemod == 1 && strcmp (keyword, "prefix") == 0) {
		if (prefix) fatalErr ("Sorry, more than one prefix specified!", 0);
		if (getword (fpin, keyword, 1, 0) <= 0)
		    fatalErr ("Sorry, missing prefix character!", 0);
		prefix = tolower (*keyword);
	    }
	    else if (spicemod == 1 && strcmp (keyword, ".model") == 0) {
		spicemod = 2;
		if (getword (fpin, keyword, 1, 0) <= 0 || strcmp (devname, keyword))
		    fatalErr ("Sorry, .model name not found or incorrect", 0);
		if (getword (fpin, keyword, 1, 0) <= 0)
		    fatalErr ("Sorry, .model type not found", 0);
		for (s = keyword; *s; ++s) *s = tolower (*s);
		n = 0;
		     if (strcmp (keyword, "c") == 0) { model = 1; if (prefix && prefix != 'c') n = 1; }
		else if (strcmp (keyword, "d") == 0) { model = 1; if (prefix && prefix != 'd') n = 1; }
		else if (strcmp (keyword, "r") == 0) { model = 1; if (prefix && prefix != 'r') n = 1; }
		else if (strcmp (keyword, "l") == 0) { model = 1; if (prefix && prefix != 'l') n = 1; }
		else if (strcmp (keyword, "nmos") == 0 || strcmp (keyword, "pmos") == 0) { model = 2;
		    if (prefix && prefix != 'm') n = 1;
		}
		else if (strcmp (keyword, "npn") == 0 || strcmp (keyword, "pnp") == 0) { model = 3;
		    if (prefix && prefix != 'q') n = 1;
		}
		else if (strcmp (keyword, "njf") == 0 || strcmp (keyword, "pjf") == 0) { model = 2;
		    if (prefix && prefix != 'j') n = 1;
		}
		else { model = 4;
		    fprintf (stderr, "%s: Warning: unknown .model type: %s\n", argv0, keyword);
		}
		if (n) fprintf (stderr, "%s: Warning: .model %s incorrect prefix: %c\n", argv0, keyword, prefix);
		if (model == 1) {
		    if (term_cnt != 2) fatalErr ("Error: incorrect number of terminals for model", keyword);
		    if (strcmp (terminals[0], "p")) fatalErr ("Error: terminal[0] != p for model", keyword);
		    if (strcmp (terminals[1], "n")) fatalErr ("Error: terminal[1] != n for model", keyword);
		}
		if (model == 2) {
		    if (term_cnt < 3 || term_cnt > 4) fatalErr ("Error: incorrect number of terminals for model", keyword);
		    if (strcmp (terminals[0], "d")) fatalErr ("Error: terminal[0] != d for model", keyword);
		    if (strcmp (terminals[1], "g")) fatalErr ("Error: terminal[1] != g for model", keyword);
		    if (strcmp (terminals[2], "s")) fatalErr ("Error: terminal[2] != s for model", keyword);
		    if (term_cnt > 3)
		    if (strcmp (terminals[3], "b")) fatalErr ("Error: terminal[3] != b for model", keyword);
		}
		if (model == 3) {
		    if (term_cnt < 3 || term_cnt > 4) fatalErr ("Error: incorrect number of terminals for model", keyword);
		    if (strcmp (terminals[0], "c")) fatalErr ("Error: terminal[0] != c for model", keyword);
		    if (strcmp (terminals[1], "b")) fatalErr ("Error: terminal[1] != b for model", keyword);
		    if (strcmp (terminals[2], "e")) fatalErr ("Error: terminal[2] != e for model", keyword);
		    if (term_cnt > 3)
		    if (strcmp (terminals[3], "s")) fatalErr ("Error: terminal[3] != s for model", keyword);
		}
	    }
	    else if (spicemod == 1 && strcmp (keyword, ".subckt") == 0) {
		spicemod = 3;
		if (getword (fpin, keyword, 1, 0) <= 0 || strcmp (devname, keyword))
		    fatalErr ("Sorry, .subckt name not found or incorrect", 0);
		do {
		    while (getword (fpin, keyword, 1, 0) > 0) ++sterm_cnt;
		}
		while (getword (fpin, keyword, 0, 1) > 0 && *keyword == '+');
		if (!sterm_cnt) fatalErr ("Sorry, .subckt terminals missing!", 0);
		if (prefix && prefix != 'x')
		    fprintf (stderr, "%s: Warning: .subckt incorrect prefix: %c\n", argv0, prefix);
	    }
	    else if (strcmp (keyword, "end") == 0 && wordOnNewline)
		mod = 0;
	}
	else if (strcmp (keyword, "begin") == 0) {
	    if (getword (fpin, keyword, 1, 0) > 0) { /* on same line */
		if (strcmp (keyword, "spicemod") == 0) {
		    if (spicemod) fatalErr ("Sorry, second simulation model:", keyword);
		    spicemod = mod = 1;
		}
		else if (strcmp (keyword, "spectremod") == 0) {
		    if (!spicemod) fatalErr ("Sorry, expecting simulation model:", "spicemod");
		    mod = 1;
		}
		else fatalErr ("unknown simulation model:", keyword);
	    }
	    else {
		sprintf (buf, "%d", lineno);
		fatalErr ("syntax error at line", buf);
	    }
	}
	else {
	    sprintf (buf, "%d", lineno);
	    fatalErr ("syntax error at line", buf);
	}
    }

    if (!term_cnt) fatalErr ("Sorry, no terminals specified!", 0);
    if (spicemod < 2) {
	keyword[0] = prefix;
	keyword[1] = 0;
	if (!prefix) fprintf (stderr, "%s: Warning: no prefix specified!\n", argv0);
	else if (prefix == 'c' || prefix == 'd' || prefix == 'r' || prefix == 'l') {
	    if (term_cnt != 2) fatalErr ("Error: incorrect number of terminals for prefix", keyword);
	    if (strcmp (terminals[0], "p")) fatalErr ("Error: terminal[0] != p for prefix", keyword);
	    if (strcmp (terminals[1], "n")) fatalErr ("Error: terminal[1] != n for prefix", keyword);
	}
	else if (prefix == 'm' || prefix == 'j') {
	    if (term_cnt < 3 || term_cnt > 4) fatalErr ("Error: incorrect number of terminals for prefix", keyword);
	    if (strcmp (terminals[0], "d")) fatalErr ("Error: terminal[0] != d for prefix", keyword);
	    if (strcmp (terminals[1], "g")) fatalErr ("Error: terminal[1] != g for prefix", keyword);
	    if (strcmp (terminals[2], "s")) fatalErr ("Error: terminal[2] != s for prefix", keyword);
	    if (term_cnt > 3)
	    if (strcmp (terminals[3], "b")) fatalErr ("Error: terminal[3] != b for prefix", keyword);
	}
	else if (prefix == 'q') {
	    if (term_cnt < 3 || term_cnt > 4) fatalErr ("Error: incorrect number of terminals for prefix", keyword);
	    if (strcmp (terminals[0], "c")) fatalErr ("Error: terminal[0] != c for prefix", keyword);
	    if (strcmp (terminals[1], "b")) fatalErr ("Error: terminal[1] != b for prefix", keyword);
	    if (strcmp (terminals[2], "e")) fatalErr ("Error: terminal[2] != e for prefix", keyword);
	    if (term_cnt > 3)
	    if (strcmp (terminals[3], "s")) fatalErr ("Error: terminal[3] != s for prefix", keyword);
	}
	if (prefix)
	    fprintf (stderr, "%s: Warning: no .%s %s specified!\n", argv0,
		prefix != 'x'? "model" : "subckt", devname);
    }

#ifndef DRIVER
    dmkey_cir = dmCheckOut (dmproject, devname, WORKING, DONTCARE, CIRCUIT, UPDATE);

    total_cnt = 0;
    dsp_term = dmOpenStream (dmkey_cir, "term", "w");
    i = 0;
    cterm.term_lower = lower;
    cterm.term_upper = upper;
    while (i < term_cnt) {
	s = terminals[i];
	c = 0;
	mod = 1;
	if ((s = strchr (terminals[i], '['))) {
	    *s = 0;
	    do {
		++s;
		if (!isdigit(*s) || c > 2) fatalErr ("Sorry, incorrect terminal dim specified!", 0);
		n = (*s++ - '0'); while (isdigit(*s)) n = n*10 + (*s++ - '0');
		if (*s == '.') { ++s;
		    if (*s++ != '.') fatalErr ("Sorry, incorrect terminal range specified!", 0);
		    if (!isdigit(*s)) fatalErr ("Sorry, incorrect terminal dim specified!", 0);
		    lower[c] = n;
		    n = (*s++ - '0'); while (isdigit(*s)) n = n*10 + (*s++ - '0');
		    if (lower[c] > n) mod *= (lower[c] + 1 - n);
		    else mod *= (n + 1 - lower[c]);
		}
		else {
		    lower[c] = 0;
		    mod *= n;
		    if (--n < 0) fatalErr ("Sorry, incorrect terminal range specified!", 0);
		}
		upper[c++] = n;
		if (*s == ']') ++s;
	    }
	    while (*s == ',' || *s == '[');
	}
	total_cnt += mod;
	sprintf (cterm.term_name, "%s", terminals[i]);
	cterm.term_dim = c;
	dmPutDesignData (dsp_term, CIR_TERM);
	i++;
    }
    dmCloseStream (dsp_term, COMPLETE);

    if (sterm_cnt && sterm_cnt != total_cnt)
	fprintf (stderr,  "%s: Warning: .subckt term_cnt != total_cnt (%d <-> %d)\n", argv0, sterm_cnt, total_cnt);
    if (spicemod == 2 && total_cnt > 4)
	fprintf (stderr,  "%s: Warning: .model unexpected term_cnt: %d\n", argv0, total_cnt);

    if (dmStat (dmkey_cir, "mc", &sbuf)) {
	dsp = dmOpenStream (dmkey_cir, "mc", "w");
	dmCloseStream (dsp, COMPLETE);
    }

    if (dmStat (dmkey_cir, "net", &sbuf)) {
	dsp = dmOpenStream (dmkey_cir, "net", "w");
	dmCloseStream (dsp, COMPLETE);
    }

    /* copy full contents of inputfile to devmod */

    dsp_devmod = dmOpenStream (dmkey_cir, "devmod", "w");
    fpout = dsp_devmod -> dmfp;
#else
    fpout = stdout;
#endif
    rewind (fpin);
    while ((c = getc (fpin)) != EOF) putc (c, fpout);

#ifndef DRIVER
    dmCloseStream (dsp_devmod, COMPLETE);
    dmCheckIn (dmkey_cir, COMPLETE);
#endif
}

int getword (FILE *fp, char *buf, int onSameLine, int onNewLine)
{
    static int c;

    wordOnNewline = 0;

    if (onNewLine) { /* start on new line, skip chars */
        while (c != '\n' && c != EOF) c = getc (fp);
    }
    else if (!onSameLine) c = getc (fp); /* first time */

    while (c != EOF) {
	while (isspace (c)) {
	    if (c == '\n') {
		if (onSameLine) return (0);
		lineno++;
		wordOnNewline = 1;
	    }
	    c = getc (fp);
	}
	if (c == EOF) break;

	if (c == '*' && spicemod && wordOnNewline) { /* skip comment */
	    c = getc (fp);
	    wordOnNewline = 0;
	}
	else {
	    int n = 0;
	    while (c > ' ' && c < 127) {
		if (n > 78) {
		    sprintf (buf, "%d", lineno);
		    fatalErr ("buffer overflow at line", buf);
		}
		buf[n++] = c;
		c = getc (fp);
	    }
	    buf[n] = 0;
	    if (!isspace (c)) {
		sprintf (buf, "%d (c = %d)", lineno, c);
		fatalErr ("illegal character at line", buf);
	    }
	    return (1);
	}
    }

    return (-1);
}

void fatalErr (char *s1, char *s2)
{
    fprintf (stderr, "%s: ", argv0);
    if (s1 && *s1) fprintf (stderr, "%s", s1);
    if (s2 && *s2) fprintf (stderr, " %s", s2);
    fprintf (stderr, "\n");
    die ();
}

void initIntrup ()
{
    if (signal (SIGINT, SIG_IGN) != SIG_IGN) signal (SIGINT, int_hdl);
        /* only when value was not SIG_IGN, a jump must be done to int_hdl */
#ifdef SIGQUIT
    if (signal (SIGQUIT, SIG_IGN) != SIG_IGN) signal (SIGQUIT, int_hdl);
#endif
    signal (SIGTERM, int_hdl);
    signal (SIGILL,  int_hdl);
    signal (SIGFPE,  int_hdl);
#ifdef SIGBUS
    signal (SIGBUS,  int_hdl);
#endif
    signal (SIGSEGV, int_hdl);
}

void int_hdl (int sig) /* interrupt handler */
{
    switch (sig) {
        case SIGILL :
            fprintf (stderr, "Illegal instruction\n");
            break;
        case SIGFPE :
            fprintf (stderr, "Floating point exception\n");
            break;
#ifdef SIGBUS
        case SIGBUS :
            fprintf (stderr, "Bus error\n");
            break;
#endif
        case SIGSEGV :
            fprintf (stderr, "Segmentation violation\n");
            break;
    }
    die ();
}

void dmError (char *s)
{
    dmPerror (s);
    die ();
}

void die ()
{
    if (dmproject) dmCloseProject (dmproject, COMPLETE);
    dmQuit ();
    exit (1);
}
