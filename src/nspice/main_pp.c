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

#include "src/nspice/define.h"
#include "src/nspice/type.h"
#include "src/nspice/extern.h"

char *argv0 = "nspice_pp";
char *Usg_msg = "\nUsage: %s cell commandfile\n\n";

char *cellname = NULL;
char *commandfile = NULL;

void readCir (void);
void readCom (char *filename);
int  readDouble (FILE *fp, double *var);
void writeSpiceCom (void);

int main (int argc, char **argv)
{
    char *s;

    while (--argc > 0) {
        ++argv;
	s = *argv;
        if (s[0] == '-' && s[1]) {
	    fprintf (stderr, "%s: illegal option: %c\n", argv0, s[1]);
	    goto usage;
	}
	else if (!cellname) cellname = *argv;
	else if (!commandfile) commandfile = *argv;
	else goto usage;
    }

    if (!commandfile) {
usage:	fprintf (stderr, Usg_msg, argv0);
	exit (1);
    }

    readCom (commandfile);

    readCir ();

    writeSpiceCom ();

    return (0);
}

void writeSpiceCom ()
{
    struct node *sig;
    struct sig_value *sval;
    struct node_ref *nref;
    double ttran, t, tp, prevt, v, prevv, SimEndtime, time;
    double trise  = -1.0;
    double tfall  = -1.0;
    double tstep  = -1.0;
    double tstart = -1.0;
    double vhigh  = 5.0; /* default */
    double vlow   = 0.0; /* default */
    int linenr = 1;
    int error = 0;
    int fs_warning = 0;
    int uic = 0;
    int c, i, match;
    char buf[128];
    char keyw[128];
    char name[128];
    char help[128];
    FILE *fp;

    SimEndtime = Endtime * Timescaling;

    if (vhigh_vh_spec) vhigh = vhigh_vh;

    c = EOF;
    match = 0;
    fp = fopen (commandfile, "r");

    if (fp) {
	c = getc (fp);
	while (!match && c != EOF) {         /* c is first character on line */
            if (c == '*' && (c = getc (fp)) == '%') match = 1;
	    while (c != '\n' && c != EOF) c = getc (fp);
	    linenr++;
	    if (c != EOF) c = getc (fp);
	}

        match = 0;
	while (!match && c != EOF) {         /* c is first character on line */
            if (c == '*' && (c = getc (fp)) == '%') {
                match = 1;
                while ((c = getc (fp)) != '\n' && c != EOF);
            }
            else {
	        while (c == ' ' || c == '\t') c = getc (fp);
	        while (c != '\n' && c != EOF) {
		    ungetc (c, fp);
		    fscanf (fp, "%s", keyw);
		    if (strsame (keyw, "trise")) {
		        if (readDouble (fp, &trise) != 1) error = 1;
		    }
		    else if (strsame (keyw, "tfall")) {
		        if (readDouble (fp, &tfall) != 1) error = 1;
		    }
		    else if (strsame (keyw, "tstep")) {
		        if (readDouble (fp, &tstep) != 1) error = 1;
		    }
		    else if (strsame (keyw, "tstart")) {
		        if (readDouble (fp, &tstart) != 1) error = 1;
		    }
		    else if (strsame (keyw, "vhigh")) {
			if (vhigh_vh_spec) {
			    message ("specification of 'vhigh' overwrites value of 'option vh'", 0, 0);
			    vhigh_vh_spec = 0;
			}
		        if (readDouble (fp, &vhigh) != 1) error = 1;
		    }
		    else if (strsame (keyw, "vlow")) {
		        if (readDouble (fp, &vlow) != 1) error = 1;
		    }
		    else if (strsame (keyw, "uic")) {
		        uic = 1;
		    }
		    else {
		        error = 1;
		    }
		    if (error) {
		        sprintf (buf, "%s, line %d: error in reading spice info", commandfile, linenr);
		        message (buf, 0, 0);
		        exit (1);
		    }
		    while ((c = getc (fp)) == ' ' || c == '\t');
                }
	    }
	    if (c != EOF) c = getc (fp);
	    linenr++;
	}
    }

    if (tstep <= 0 && simlevel == 3) {
	message ("must specify tstep in %s", commandfile, 0);
	exit (1);
    }

    for (sig = Begin_signal; sig; sig = sig -> next) {

	if (sig -> nodenr == 0) continue; /* ground node */

	fprintf (stdout, "v");
	for (i = 0; sig -> name[i]; i++) {
	    if (sig -> name[i] == ',' || sig -> name[i] == '[' || sig -> name[i] == ']')
		fprintf (stdout, "_");
	    else
		fprintf (stdout, "%c", sig -> name[i]);
	}
	fprintf (stdout, " %d 0 pwl (", sig -> nodenr);

	prevv = 0;
        prevt = -1;
        time = 0;
	ttran = tstep;

	for (sval = sig -> begin_value; sval; sval = sval -> next) {

	    ttran = -1;

	    switch (sval -> value) {
	    case 0:
		v = vlow;
		ttran = tfall;
		break;
	    case 1:
		v = vlow + vhigh / 2;
		break;
	    case 2:
		v = vlow + vhigh;
		ttran = trise;
		break;
	    default:
		v = vlow + vhigh / 2;
		if (!fs_warning) {
		    sprintf (help, "%le", v);
		    message ("Using free state -> %s Volts", help, 0);
		    fs_warning = 1;
		}
	    }

	    if (ttran <= 0) ttran = tstep; /* ttran = sval -> time * Timescaling * 0.00002; */

	    if (prevt >= 0) {
		time = sval -> time * Timescaling;
		tp = time - ttran / 2;
                if (tp <= prevt * 1.000000001) {
                    sprintf (help, "signal \"%s\" at t=%le: %%s", sig -> name, time);
                    message (help, "transition not after stabilization of previous one; reduce trise and/or tfall!", 0);
                    exit (1);
                }
	        t = time + ttran / 2;
		fprintf (stdout, "\n+     %le %le %le %le", tp, prevv, t, v);
	    }
	    else {
                t = 0;
		fprintf (stdout, "%le %le", t, v);
	    }
            prevv = v;
            prevt = t;
	}
	if (time < SimEndtime) {
	    t = SimEndtime - ttran / 2;
	    if (t > prevt * 1.000000001) fprintf (stdout, "\n+     %le %le", t, prevv);
	    t = SimEndtime + ttran / 2;
	    fprintf (stdout, " %le %le", t, prevv);
	}
	fprintf (stdout, ")\n");
    }

    if (Begin_print && simlevel == 3) {
	fprintf (stdout, ".print tran");
        i = 0;
	for (nref = Begin_print; nref; nref = nref -> next) {
	    if (nref -> nodenr == 0) continue; /* ground node */

	    if (i && i % 8 == 0) fprintf (stdout, "\n.print tran");
	    fprintf (stdout, " v(%d)", nref -> nodenr);
	    i++;
	}
	fprintf (stdout, "\n");
    }

    if (fp && match) {

        match = 0;
	while (!match && c != EOF) {

	    if (c == '"') {
		if (fscanf (fp, "%s", name) != 1) {
		    sprintf (buf, "%s, line %d: error in reading spice info", commandfile, linenr);
		    message (buf, 0, 0);
		    exit (1);
		}
		for (i = 0; name[i] != '\0' && name[i] != '"'; i++);
		if (name[i] == '"') {
		    name[i] = '\0';
		}
		else {
		    sprintf (buf, "%s, line %d: error in reading spice info", commandfile, linenr);
		    message (buf, 0, 0);
		    exit (1);
		}

                nref = Begin_table;
		while (nref && !strsame (nref -> name, name)) nref = nref -> next;

                if (nref)
		    fprintf (stdout, "%d", nref -> nodenr);
		else {
		    sprintf (buf, "file %s, line %d: no node with name %s", commandfile, linenr, name);
		    message (buf, 0, 0);
		    exit (1);
		}

		fprintf (stdout, "%s", &name[i+1]);

	        c = getc (fp);
	    }
	    else if (c == '\n') {
                linenr++;
		putc (c, stdout);
                if ((c = getc (fp)) == '*') {
                    if ((c = getc (fp)) == '%')
                        match = 1;
                    else {
		        putc ('*', stdout);
                    }
                }
            }
	    else {
		putc (c, stdout);
	        c = getc (fp);
	    }
	}

	if (!match) {
	    message ("file %s: missing *%%", commandfile, 0);
	    exit (1);
	}

	fclose (fp);
    }

    if (simlevel == 3) {
	fprintf (stdout, ".tran %le %le", tstep, SimEndtime);
	if (tstart > 0) fprintf (stdout, " %le", tstart);
	if (uic) fprintf (stdout, " uic");
	fprintf (stdout, "\n");
    }

    fprintf (stdout, ".end\n");
}

int readDouble (FILE *fp, double *var)
{
    int c;

    if (fscanf (fp, "%le", var) != 1) return (0);
    c = getc (fp);
    switch (c) {
	case 'T': *var *= 1e12; break;
	case 'G': *var *= 1e9; break;
	case 'M': *var *= 1e6; break;
	case 'k': *var *= 1e3; break;
	case 'm': *var *= 1e-3; break;
	case 'u': *var *= 1e-6; break;
	case 'n': *var *= 1e-9; break;
	case 'p': *var *= 1e-12; break;
	case 'f': *var *= 1e-15; break;
	default:
	    if (c != EOF) ungetc (c, fp);
    }
    return (1);
}
