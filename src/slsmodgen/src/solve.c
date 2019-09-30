/*
 * ISC License
 *
 * Copyright (C) 1990 by
 *	Arjan van Genderen
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
/*
 * The program "solve" solves the sls transistor model
 * parameters from
 *    (1) A circuit parameter file that is given as an argument
 *        (usually called 'values')
 *    (2) A set of spice simulation output files (FIGNAME.ana)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *buf;

typedef struct parameters {
    double rx;
    char srx[32];
    double cx;
    char scx[32];
    int nx;
    char snx[32];
    double delay;
    double td;
    double id;
    double vds;
} params;

char swn[132];
char sln[132];
char swp[132];
char slp[132];
char skr[132];
char skf[132];

double wn;
double ln;
double wp;
double lp;
double krIn = -1;
double kfIn = -1;

double kf;
double Rn;
double Cgf;
double kr;
double Rp;
double Cgr;

double Rnstat;
double Rnsatu;

double Rpstat;
double Rpsatu;

double Cnc;
double Cnef;
double Cnpf;
double Rnpf;
double Cner;
double Cnpr;
double Rnpr;

double Cpc;
double Cpef;
double Cppf;
double Rppf;
double Cper;
double Cppr;
double Rppr;

params fig5_1;
params fig5_2;
params fig5_2b;
params fig5_4;
params fig5_5;
params fig5_6;
params fig5_7;
params fig5_4_8;
params fig5_4_9;
params fig5_8;
params fig5_9;
params fig5_10;
params fig5_11;
params fig5_12;
params fig5_13;
params fig5_14;
params fig5_15;
params fig5_16;
params fig5_18;
params fig5_19;
params fig5_20;
params fig5_4_8p;
params fig5_4_9p;
params fig5_21;
params fig5_22;
params fig5_23;
params fig5_24;
params fig5_25;
params fig5_26;
params fig5_27;
params fig5_28;

params reads;

char *argv0 = "solve";

FILE *fp_out;
FILE *fp_in;

char *fn_in = NULL;

double btof (char *s);
void chop (char *str, char *par, char *val);
void die (void);
void doChecks (void);
void readParam (void);
void sy (params *dp, params *sp, char *name, double initdelay);

int main (int argc, char *argv[])
{
    char *s;

    fp_out = stdout;

    while (--argc > 0) {
        if ((*++argv)[0] == '-' ) {
	    for (s = *argv + 1; *s != '\0'; s++) {
		fprintf (stderr, "%s: illegal option: %c\n", argv0, *s);
		die ();
	    }
	}
	else {
            if (fn_in == NULL) {
		fn_in = *argv;
	    }
	}
    }

    if (fn_in == NULL) {
	fprintf (stderr, "\nUsage: %s [options] infile\n\n", argv0);
	die ();
    }

    readParam ();

    doChecks ();

    Rnstat = fig5_1.vds / fig5_1.id;
    Rnsatu = (5.0 - fig5_1.vds) / fig5_1.id;

    Rpstat = (5.0 - fig5_16.vds)  / fig5_16.id;
    Rpsatu = (fig5_16.vds) / fig5_16.id;

    if (kfIn < 0)
        kf = (fig5_7.nx * (fig5_6.td / fig5_6.cx) * (fig5_5.td / fig5_5.rx))
	     / fig5_7.td;
    else
        kf = kfIn * 1e+9;

    Cgf = fig5_5.td / (fig5_5.rx * kf);

    Rn = fig5_6.td / (fig5_6.cx * kf);

    if (krIn < 0)
        kr = (fig5_20.nx * (fig5_19.td / fig5_19.cx)
                         * (fig5_18.td / fig5_18.rx))
             / fig5_20.td;
    else
        kr = krIn * 1e+9;

    Cgr = fig5_18.td / (fig5_18.rx * kr);

    Rp = fig5_19.td / (fig5_19.cx * kr);

    Cnef = fig5_8.td / (kf * Rn * 2 * fig5_8.nx);

    Cnpf = (fig5_9.td) / (kf * Rn * 2 * fig5_9.nx);

    Rnpf = (fig5_11.td) / (kf * fig5_11.nx * fig5_11.cx);

    Cner = fig5_12.td / (kr * Rp * 2 * fig5_12.nx);

    Cnpr = (fig5_13.td) / (kr * Rp * 2 * fig5_12.nx);

    Rnpr = (fig5_15.td) / (kr * fig5_15.nx * fig5_15.cx);

    Cpef = fig5_21.td / (kf * Rn * 2 * fig5_21.nx);

    Cppf = (fig5_22.td) / (kf * Rn * 2 * fig5_22.nx);

    Rppf = (fig5_24.td) / (kf * fig5_24.nx * fig5_24.cx);

    Cper = fig5_25.td / (kr * Rp * 2 * fig5_25.nx);

    Cppr = (fig5_26.td) / (kr * Rp * 2 * fig5_26.nx);

    Rppr = (fig5_28.td) / (kr * fig5_28.nx * fig5_28.cx);

    Cnc = (fig5_4_9.td / (kr * Rp * 2)) - Cner;

    Cpc = (fig5_4_9p.td / (kf * Rn * 2)) - Cpef;

    fprintf (fp_out, "\n");
    fprintf (fp_out, "Rnstat = %e   Rnsatu = %e\n", Rnstat, Rnsatu);
    fprintf (fp_out, "\n");
    fprintf (fp_out, "Rpstat = %e   Rpsatu = %e\n", Rpstat, Rpsatu);
    fprintf (fp_out, "\n");
    fprintf (fp_out, "kf = %f   Cgf = %e   Rn = %e\n", 1e-9 * kf, Cgf, Rn);
    fprintf (fp_out, "\n");
    fprintf (fp_out, "kr = %f   Cgr = %e   Rp = %e\n", 1e-9 * kr, Cgr, Rp);
    fprintf (fp_out, "\n");
    fprintf (fp_out, "Cnef = %e   Cnpf = %e  Rnpf = %e\n", Cnef, Cnpf, Rnpf);
    fprintf (fp_out, "\n");
    fprintf (fp_out, "Cner = %e   Cnpr = %e  Rnpr = %e\n", Cner, Cnpr, Rnpr);
    fprintf (fp_out, "\n");
    fprintf (fp_out, "Cpef = %e   Cppf = %e  Rppf = %e\n", Cpef, Cppf, Rppf);
    fprintf (fp_out, "\n");
    fprintf (fp_out, "Cper = %e   Cppr = %e  Rppr = %e\n", Cper, Cppr, Rppr);
    fprintf (fp_out, "\n");
    fprintf (fp_out, "Cnc = %e   Cpc = %e\n", Cnc, Cpc);
    fprintf (fp_out, "\n");
    fprintf (fp_out, "slsmod\n");
    fprintf (fp_out, "\n");
    if (krIn < 0) fprintf (fp_out, "krise=%.3f\n", 1e-9 * kr);
    if (kfIn < 0) fprintf (fp_out, "kfall=%.3f\n", 1e-9 * kf);
    fprintf (fp_out, "\n");

    fprintf (fp_out, "nenh general ");
    fprintf (fp_out, "%s %s : rstat=%.2fk rsatu=%.2fk ", swn, sln, Rnstat * 1e-3, Rnsatu * 1e-3);
    fprintf (fp_out, "cgstat=%.2ff cgrise=%.2ff cgfall=%.2ff ",
                     1e15 * (Cgr + Cgf) / 4, 1e15 * Cgr / 2, 1e15 * Cgf / 2);
    fprintf (fp_out, "cestat=%.2ff cerise=%.2ff cefall=%.2ff\n",
                     1e15 * (Cner + Cnef) / 2, 1e15 * Cner, 1e15 * Cnef);

    fprintf (fp_out, "nenh pulldown ");
    fprintf (fp_out, "%s %s : rdyn=%.2fk cch=%.2ff\n", swn, sln, 1e-3 * Rn, 1e15 * Cnc);

    fprintf (fp_out, "nenh passup ");
    fprintf (fp_out, "%s %s : rdyn=%.2fk cch=%.2ff\n", swn, sln, 1e-3 * Rnpr, 1e15 * Cnpr);

    fprintf (fp_out, "nenh passdown ");
    fprintf (fp_out, "%s %s : rdyn=%.2fk cch=%.2ff\n", swn, sln, 1e-3 * Rnpf, 1e15 * Cnpf);

    fprintf (fp_out, "penh general ");
    fprintf (fp_out, "%s %s : rstat=%.2fk rsatu=%.2fk ", swp, slp, Rpstat * 1e-3, Rpsatu * 1e-3);
    fprintf (fp_out, "cgstat=%.2ff cgrise=%.2ff cgfall=%.2ff ",
                     1e15 * (Cgr + Cgf) / 4, 1e15 * Cgr / 2, 1e15 * Cgf / 2);
    fprintf (fp_out, "cestat=%.2ff cerise=%.2ff cefall=%.2ff\n",
                     1e15 * (Cner + Cnef) / 2, 1e15 * Cper, 1e15 * Cpef);

    fprintf (fp_out, "penh pullup ");
    fprintf (fp_out, "%s %s : rdyn=%.2fk cch=%.2ff\n", swp, slp, 1e-3 * Rp, 1e15 * Cpc);

    fprintf (fp_out, "penh passup ");
    fprintf (fp_out, "%s %s : rdyn=%.2fk cch=%.2ff\n", swp, slp, 1e-3 * Rppr, 1e15 * Cppr);

    fprintf (fp_out, "penh passdown ");
    fprintf (fp_out, "%s %s : rdyn=%.2fk cch=%.2ff\n", swp, slp, 1e-3 * Rppf, 1e15 * Cppf);

    return (0);
}

void readParam ()
{
    char c;
    char figname[132];
    char *buf;
    char *par;
    char *val;
    int i;

    if ((fp_in = fopen (fn_in, "r")) == NULL) {
        fprintf (stderr, "Cannot open %s\n", fn_in);
        die ();
    }

    buf = (char *)calloc (1, 512);

    while (fscanf (fp_in, "%s", buf) == 1) {

        if (buf[0] == 'w' && buf[1] == 'n' && buf[2] == '=') {
            swn[0] = buf[0];
            i = 1;
            while (buf[i] != '\0') {
                swn[i] = buf[i + 1];
                i++;
            }
            sscanf (&swn[2], "%le", &wn);
        }

        if (buf[0] == 'l' && buf[1] == 'n' && buf[2] == '=') {
            sln[0] = buf[0];
            i = 1;
            while (buf[i] != '\0') {
                sln[i] = buf[i + 1];
                i++;
            }
            sscanf (&sln[2], "%le", &ln);
        }

        if (buf[0] == 'w' && buf[1] == 'p' && buf[2] == '=') {
            swp[0] = buf[0];
            i = 1;
            while (buf[i] != '\0') {
                swp[i] = buf[i + 1];
                i++;
            }
            sscanf (&swp[2], "%le", &wp);
        }

        if (buf[0] == 'l' && buf[1] == 'p' && buf[2] == '=') {
            slp[0] = buf[0];
            i = 1;
            while (buf[i] != '\0') {
                slp[i] = buf[i + 1];
                i++;
            }
            sscanf (&slp[2], "%le", &lp);
        }

        if (buf[0] == 'k' && buf[1] == 'r' && buf[2] == '=') {
            strcpy (skr, buf);
            sscanf (&skr[3], "%le", &krIn);
        }

        if (buf[0] == 'k' && buf[1] == 'f' && buf[2] == '=') {
            strcpy (skf, buf);
            sscanf (&skf[3], "%le", &kfIn);
        }

        if (strcmp (buf, "begin") == 0) break;
    }

    par = (char *)calloc (1, 132);
    val = (char *)calloc (1, 132);

    while ((c = getc (fp_in)) == ' ' || c == '\t' || c == '\n');

    while (c != EOF) {

        ungetc (c, fp_in);

        reads.nx = -1;
        reads.cx = -1;
        reads.rx = -1;
        reads.id = -1.0;
        reads.vds = -1.0;

	while (fscanf (fp_in, "%s", buf) == 1 && buf[0] != '$') {

            chop (buf, par, val);

            if (strcmp (par, "nx") == 0) {
                reads.nx = atoi (val);
                strcpy (reads.snx, buf);
            }
            else if (strcmp (par, "cx") == 0) {
                reads.cx = btof (val);
                strcpy (reads.scx, buf);
            }
            else if (strcmp (par, "rx") == 0) {
                reads.rx = btof (val);
                strcpy (reads.srx, buf);
            }
            else if (strcmp (par, "tstep") != 0
                     && strcmp (par, "tend") != 0) {
                fprintf (stderr, "unrecognized: %s\n", buf);
            }
	}

	fscanf (fp_in, "%s", figname);

	if (strcmp (figname, "fig5.1") == 0)
	    sy (&fig5_1, &reads, figname, -1.1);
	else if (strcmp (figname, "fig5.2") == 0)
	    sy (&fig5_2, &reads, figname, -1.1);
	else if (strcmp (figname, "fig5.2b") == 0)
	    sy (&fig5_2b, &reads, figname, -1.1);
	else if (strcmp (figname, "fig5.4") == 0)
	    sy (&fig5_4, &reads, figname, -0.1);
	else if (strcmp (figname, "fig5.5") == 0)
	    sy (&fig5_5, &reads, figname, fig5_4.delay);
	else if (strcmp (figname, "fig5.6") == 0)
	    sy (&fig5_6, &reads, figname, fig5_4.delay);
	else if (strcmp (figname, "fig5.7") == 0)
	    sy (&fig5_7, &reads, figname, fig5_4.delay);
	else if (strcmp (figname, "fig5.4_8") == 0)
	    sy (&fig5_4_8, &reads, figname, -0.1);
	else if (strcmp (figname, "fig5.4_9") == 0)
	    sy (&fig5_4_9, &reads, figname, fig5_4_8.delay);
	else if (strcmp (figname, "fig5.8") == 0)
	    sy (&fig5_8, &reads, figname, fig5_4.delay);
	else if (strcmp (figname, "fig5.9") == 0)
	    sy (&fig5_9, &reads, figname, fig5_8.delay);
	else if (strcmp (figname, "fig5.10") == 0)
	    sy (&fig5_10, &reads, figname, -0.1);
	else if (strcmp (figname, "fig5.11") == 0)
	    sy (&fig5_11, &reads, figname, fig5_10.delay);
	else if (strcmp (figname, "fig5.12") == 0)
	    sy (&fig5_12, &reads, figname, fig5_4.delay);
	else if (strcmp (figname, "fig5.13") == 0)
	    sy (&fig5_13, &reads, figname, fig5_12.delay);
	else if (strcmp (figname, "fig5.14") == 0)
	    sy (&fig5_14, &reads, figname, -0.1);
	else if (strcmp (figname, "fig5.15") == 0)
	    sy (&fig5_15, &reads, figname, fig5_14.delay);
	else if (strcmp (figname, "fig5.16") == 0)
	    sy (&fig5_16, &reads, figname, -1.1);
	else if (strcmp (figname, "fig5.18") == 0)
	    sy (&fig5_18, &reads, figname, fig5_4.delay);
	else if (strcmp (figname, "fig5.19") == 0)
	    sy (&fig5_19, &reads, figname, fig5_4.delay);
	else if (strcmp (figname, "fig5.20") == 0)
	    sy (&fig5_20, &reads, figname, fig5_4.delay);
	else if (strcmp (figname, "fig5.4_8p") == 0)
	    sy (&fig5_4_8p, &reads, figname, -0.1);
	else if (strcmp (figname, "fig5.4_9p") == 0)
	    sy (&fig5_4_9p, &reads, figname, fig5_4_8p.delay);
	else if (strcmp (figname, "fig5.21") == 0)
	    sy (&fig5_21, &reads, figname, fig5_4.delay);
	else if (strcmp (figname, "fig5.22") == 0)
	    sy (&fig5_22, &reads, figname, fig5_21.delay);
	else if (strcmp (figname, "fig5.23") == 0)
	    sy (&fig5_23, &reads, figname, -0.1);
	else if (strcmp (figname, "fig5.24") == 0)
	    sy (&fig5_24, &reads, figname, fig5_23.delay);
	else if (strcmp (figname, "fig5.25") == 0)
	    sy (&fig5_25, &reads, figname, fig5_4.delay);
	else if (strcmp (figname, "fig5.26") == 0)
	    sy (&fig5_26, &reads, figname, fig5_25.delay);
	else if (strcmp (figname, "fig5.27") == 0)
	    sy (&fig5_27, &reads, figname, -0.1);
	else if (strcmp (figname, "fig5.28") == 0)
	    sy (&fig5_28, &reads, figname, fig5_27.delay);

        while ((c = getc (fp_in)) == ' ' || c == '\t' || c == '\n');
    }
}

void chop (char *str, char *par, char *val)
{
    int i;

    strcpy (par, str);

    for (i = 0; par[i] != '=' && par[i] != '\0'; i++) ;

    if (par[i] == '=') {
        par[i] = '\0';
        strcpy (val, &par[i + 1]);
    }
    else
	strcpy (val, "");
}

void sy (params *dp, params *sp, char *name, double initdelay)
{
    int i;
    char com[132];
    FILE *fp;

    char *d = (char *)dp;
    char *s = (char *)sp;

    for (i = 0; i < sizeof (params); i++) {
        *d++ = *s++;
    }

    if (initdelay >= -0.5) {

        sprintf (com, "delay %s.ana > ratatata", name);
        system (com);

        if ((fp = fopen ("ratatata", "r")) == NULL) {
            fprintf (stderr, "Cannot open file ratatata");
            exit (1);
        };
        fscanf (fp, "%le", &dp -> delay);
        fclose (fp);
    }
    else if (strcmp (name, "fig5.1") == 0
             || strcmp (name, "fig5.16") == 0) {

        sprintf (com, "resis %s.ana > ratatata", name);
        system (com);

        if ((fp = fopen ("ratatata", "r")) == NULL) {
            fprintf (stderr, "Cannot open file ratatata");
            exit (1);
        };
        fscanf (fp, "%le", &dp -> id);
        fscanf (fp, "%le", &dp -> vds);

        fclose (fp);
    }

    fprintf (fp_out, "%s: ", name);
    i = strlen (name) + 2;

    if (dp -> nx > 0) {
        fprintf (fp_out, "%s ", dp -> snx);
        i += strlen (dp -> snx) + 1;
    }
    if (dp -> rx > 0) {
        fprintf (fp_out, "%s ", dp -> srx);
        i += strlen (dp -> srx) + 1;
    }
    if (dp -> cx > 0) {
        fprintf (fp_out, "%s ", dp -> scx);
        i += strlen (dp -> scx) + 1;
    }

    while (i <= 35) {
        fprintf (fp_out, " ");
        i++;
    }

    if (initdelay >= -0.5) {
        fprintf (fp_out, " delay: %f", dp -> delay);

        if (initdelay >= 0.0) {
            dp -> td = dp -> delay - initdelay;
            fprintf (fp_out, " : %f", dp -> td);
        }
        else {
            dp -> td = dp -> delay;
        }
    }
    else if (dp -> id > 0) {
        fprintf (fp_out, " id: %e  vds: %e", dp -> id, dp -> vds);
    }

    fprintf (fp_out, "\n");
}

double btof (char *s)
{
    double val = 0;
    char last;

    last = s[ strlen (s) - 1 ];

    switch (last) {
        case 'f':
            val = 1e-15;
            break;
        case 'p':
            val = 1e-12;
            break;
        case 'n':
            val = 1e-9;
            break;
        case 'u':
            val = 1e-6;
            break;
        case 'm':
            val = 1e-3;
            break;
        case 'k':
            val = 1e+3;
            break;
        case 'M':
            val = 1e+6;
            break;
        case '\0':
            val = 1;
    }

    if (val != 1)
        s[ strlen (s) - 1 ] = '\0';

    val = val * atof (s);

    return (val);
}

void doChecks ()
{
    int error = 0;

    if (fig5_2.nx != fig5_2b.nx) {
        fprintf (stderr, "fig5.2.nx != fig5.2b.nx\n");
        error = 1;
    }

    if (fig5_8.nx != fig5_9.nx) {
        fprintf (stderr, "fig5.8.nx != fig5.9.nx\n");
        error = 1;
    }
    if (fig5_10.nx != fig5_11.nx) {
        fprintf (stderr, "fig5.10.nx != fig5.11.nx\n");
        error = 1;
    }
    if (fig5_10.cx != fig5_11.cx) {
        fprintf (stderr, "fig5.10.cx != fig5.11.cx\n");
        error = 1;
    }
    if (fig5_12.nx != fig5_13.nx) {
        fprintf (stderr, "fig5.12.nx != fig5.13.nx\n");
        error = 1;
    }
    if (fig5_14.nx != fig5_15.nx) {
        fprintf (stderr, "fig5.14.nx != fig5.15.nx\n");
        error = 1;
    }
    if (fig5_14.cx != fig5_15.cx) {
        fprintf (stderr, "fig5.14.cx != fig5.15.cx\n");
        error = 1;
    }

    if (fig5_21.nx != fig5_22.nx) {
        fprintf (stderr, "fig5.21.nx != fig5.22.nx\n");
        error = 1;
    }
    if (fig5_23.nx != fig5_24.nx) {
        fprintf (stderr, "fig5.23.nx != fig5.24.nx\n");
        error = 1;
    }
    if (fig5_23.cx != fig5_24.cx) {
        fprintf (stderr, "fig5.23.cx != fig5.24.cx\n");
        error = 1;
    }
    if (fig5_25.nx != fig5_26.nx) {
        fprintf (stderr, "fig5.25.nx != fig5.26.nx\n");
        error = 1;
    }
    if (fig5_27.nx != fig5_28.nx) {
        fprintf (stderr, "fig5.27.nx != fig5.28.nx\n");
        error = 1;
    }
    if (fig5_27.cx != fig5_28.cx) {
        fprintf (stderr, "fig5.27.cx != fig5.28.cx\n");
        error = 1;
    }

    if (error) die ();
}

void die ()
{
    exit (1);
}
