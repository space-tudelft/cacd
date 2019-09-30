/*
 * ISC License
 *
 * Copyright (C) 2003-2018 by
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

void GenSubRes (void);
void mkSpace3dElemFile (void);
long StatFile (char *path);
double Recalc (char *val);
int run (char *prog, char *arg1, char *arg2, char *arg3);
void RunAllSpace3d (void);
void RunSpace3d (char *cellname, int nr_contacts);
void concat (char *f1, char *f2, char *d);

char *argv0 = "subresgen";

#define SIZE_T1 7
#define SIZE_T2 4

int t1sizefac[SIZE_T1] = { 1, 2, 4, 8, 16, 32, 64 };
int t2sizefac[SIZE_T2] = { 1, 2, 4, 32 };
int t2distfac[SIZE_T2] = { 2, 4, 8, 32 };

double subrestable[SIZE_T1];
double subres_rval;
double coupres_rval, coupsubres_rval;

int   nr_experim = 0;
long  controltime;
char *inputfile   = "space.def.s";
char *minsizefile = ".min_term_size";
char *layerfile   = "process/sublayers";
char *layerfiletmp= "process/sublayers.tmp";
char *paramfile   = "process/space.def.p";
char *headerfile  = "process/space.header.s";
char *space3dfile = "process/space.def.s";
FILE *DOSPACE;
int   minsize = 0;
int   opt_c = 0;
int   opt_v = 0;

int main (int argc, char *argv[])
{
    FILE *IN;
    char *s;
    long layertime;
    int  i, err = 0;

    for (i = 1; i < argc && (s = argv[i]) && *s == '-'; ++i) {
        while (*++s) {
            switch (*s) {
	    case 'c': opt_c = 1; break;
	    case 'v': opt_v = 1; break;
	    default:  err = 1;
		printf ("%s: -%c: unknown option\n", argv0, *s);
	    }
	}
    }

    if (err || i < argc) {
	if (i < argc) printf ("%s: unknown argument(s)\n", argv0);
	printf ("\nUsage: %s [-cv]\n\n", argv0);
	return (1);
    }

    if (!(IN = fopen (minsizefile, "r"))) {
        printf ("%s: ERROR: Can't open \"%s\"\n", argv0, minsizefile);
        printf ("%s: Verify that the current directory was created by 'subresmkdir'\n", argv0);
	return (1);
    }
    fscanf (IN, "%d", &minsize);
    fclose (IN);

    if (minsize < 1 || minsize > 100000000) {
        printf ("%s: File \"%s\" is corrupted.\n", argv0, minsizefile);
	return (1);
    }

    controltime = StatFile (paramfile);

    mkSpace3dElemFile ();

    layertime = StatFile (layerfile);
    if (layertime > controltime) controltime = layertime;

    if (opt_c && !(DOSPACE = fopen ("dospace", "w"))) {
	printf ("%s: ERROR: Can't create \"dospace\" file\n", argv0);
	return (1);
    }

    RunAllSpace3d ();

    if (opt_c) {
	fclose (DOSPACE);
	printf ("%s: Command file \"dospace\" created.\n", argv0);
    }
    else {
	printf ("%s: Updating \"%s\" with \"selfsubres\" and \"coupsubres\" entries.\n", argv0, inputfile);
	GenSubRes ();
    }

    printf ("%s: -- job finished --\n", argv0);
    return (0);
}

void mkSpace3dElemFile ()
{
    char line[512];
    char curr_line[512];
    FILE *IN;
    FILE *SUBLAYERS;
    FILE *SUBLAYERSTMP;
    int   sublayersDifferent = 0;

    if (!(IN = fopen (inputfile, "r"))) {
        printf ("%s: ERROR: Can't open %s\n", argv0, inputfile);
        exit (1);
    }
    if (!(SUBLAYERS = fopen (layerfile, "r"))) {
        sublayersDifferent = 2;
    }
    SUBLAYERSTMP = fopen (layerfiletmp, "w");

    while (fgets (line, 512, IN) && strncmp (line, "sublayers", 9)) ;
    while (fgets (line, 512, IN) && *line != '\n') {
        fprintf (SUBLAYERSTMP, line);
        if (sublayersDifferent < 2) {
	    fgets (curr_line, 512, SUBLAYERS);
            if (strcmp (line, curr_line)) sublayersDifferent = 1;
        }
    }
    fclose (IN);
    fclose (SUBLAYERSTMP);
    if (SUBLAYERS) fclose (SUBLAYERS);

    if (sublayersDifferent) {
	rename (layerfiletmp, layerfile);
	concat (headerfile, layerfile, space3dfile);
        printf ("=====> tecc -p process %s <=====\n", space3dfile);
	if (run ("tecc", "-p", "process", space3dfile)) {
	    printf ("%s: ERROR: Cannot run tecc\n", argv0);
	    exit (1);
	}
    }
}

int length, width;

void RunAllSpace3d ()
{
    char cellname[56];
    int i, j;

    for (i = 0; i < SIZE_T1; i++) {
        width = minsize * t1sizefac[i];
        length = width;

        sprintf (cellname, "t1_%dx%d", width, length);
        RunSpace3d (cellname, 1);

        width = width / 2;
        length = length * 2;
        sprintf (cellname, "t1_%dx%d", width, length);
        RunSpace3d (cellname, 1);

        length = length * 3 / 4;
        sprintf (cellname, "t1_%dx%d", width, length);
        RunSpace3d (cellname, 1);
    }

    for (i = 0; i < SIZE_T2; i++) {
        length = width = minsize * t2sizefac[i];
        for (j = 0; j < SIZE_T2; j++) {
	    sprintf (cellname, "t2_%dx%d_%d", width, length, width * t2distfac[j]);
            RunSpace3d (cellname, 2);
        }
    }
}

void RunSpace3d (char *cellname, int nr_contacts)
{
    char command[300], arg[10];
    long sourcetime, resulttime;
    int total_nr_experim;
    int perc_finished;

    nr_experim++;

    sprintf (command, "layout/%s/mc", cellname);
    sourcetime = StatFile (command);
    sprintf (command, "circuit/%s/mc", cellname);
    resulttime = StatFile (command);

    if (resulttime > sourcetime && resulttime > controltime) {
        printf ("=====> '%s' already extracted, space3d run skipped <=====\n", cellname);
        return;
    }

    sprintf (arg, "-Bn");
    if (opt_v) sprintf (arg, "-Bnv");
    sprintf (command, "-Ssub3d.max_be_area=%g", 1e-7 * width * length);

    if (opt_c) fprintf (DOSPACE, "space3d %s %s %s\n", arg, command, cellname);
    else {
        printf ("=====> space3d %s %s %s <=====\n", arg, command, cellname);
        if (run ("space3d", arg, command, cellname)) {
            printf ("\n%s: -- job aborted --\n\n", argv0);
            exit (1);
        }

        total_nr_experim = SIZE_T1 * 3 + SIZE_T2 * SIZE_T2;

        perc_finished = (nr_experim * 100) / total_nr_experim;

        printf ("\n=====> %d %% of the experiments are finished <=====\n\n", perc_finished);
    }
}

long StatFile (char *path)
{
    struct stat buf;
    long mtime = -1;

    if (stat (path, &buf) == 0) mtime = buf.st_mtime;
    return mtime;
}

void ReadSLS (char *cellname, int nr_contacts)
{
    char line[512], res[20], val[40], from[20], to[20];
    FILE *SLS;

    if (run ("xsls", "-f", cellname, NULL)) {
        printf ("%s: ERROR: Cannot start xsls on cell %s\n", argv0, cellname);
        exit (1);
    }

    sprintf (line, "%s.sls", cellname);
    if (!(SLS = fopen (line, "r"))) {
        printf ("%s: ERROR: Cannot read %s\n", argv0, line);
        exit (1);
    }
    unlink (line);

    while (fgets (line, 512, SLS) && *line != '{') ;

    if (*line == '{')
    while (fgets (line, 512, SLS) && *line != '}') {
        sscanf (line, "%s %s %s %s", res, val, from, to);

        if (strcmp (res, "res") == 0) {
            if (nr_contacts < 2) {
                if (*to == 'S' || from[1] == 'S') {
		    subres_rval = Recalc (val);
                    // printf ("%s SUBSTR %g %s %s\n", cellname, subres_rval, from, to);
                }
            }
            else {
                if (*to == 'S' || from[1] == 'S') {
	            coupsubres_rval = Recalc (val);
                    // printf ("%s coup SUBSTR %g %s %s\n", cellname, coupsubres_rval, from, to);
                }
                else {
	            coupres_rval = Recalc (val);
                    // printf ("%s coup %g %s %s\n", cellname, coupres_rval, from, to);
                }
            }
        }
    }
    fclose (SLS);
}

void GenSubRes ()
{
    time_t t;
    char cellname[56], fmt[56], fmt2[56], fmtf[56], fmtg[56];
    char line[512], *rest, *s;
    FILE *IN, *OUT;
    char *tmpfile, *localtime;
    int i, j, k, na, np, passed_coupsubres = 0;
    double area, perim, w, l, decr;
    double coupres[SIZE_T2 * SIZE_T2];
    double coupsubres[SIZE_T2 * SIZE_T2];

    tmpfile = "space.def.new.s";
    if (!(OUT = fopen (tmpfile, "w"))) {
        printf ("%s: ERROR: Cannot write output file '%s'\n", argv0, tmpfile);
        exit (1);
    }

    if (!(IN = fopen (inputfile, "r"))) {
        printf ("%s: ERROR: Cannot read input file '%s'\n", argv0, inputfile);
        exit (1);
    }
    while (fgets (line, 512, IN) && strncmp (line, "selfsubres", 10)
				 && strncmp (line, "coupsubres", 10)) {
        fprintf (OUT, line);
    }
    if (strncmp (line, "coupsubres", 10) == 0) { passed_coupsubres = 1; }
    while (fgets (line, 512, IN) && *line != '\n') ;

    t = time (NULL);
    localtime = ctime (&t);

    fprintf (OUT, "selfsubres :\n");
    fprintf (OUT, "#   Generated by subresgen on %s", localtime);
    fprintf (OUT, "#     area    perim          r   rest\n");

    w = 0.5e-3 * minsize * t1sizefac[0];
    sprintf (line, "%g", w);
    k = 0;
    if ((s = strchr (line, '.'))) while (*++s) ++k;
    w *= 2;
    sprintf (line, "%g", w);
    np = 0;
    if ((s = strchr (line, '.'))) while (*++s) ++np;

    na = 2 * np;
    sprintf (fmt2, "%%12.%df %%6.%df", 2*k, np); /* k > np */
    sprintf (fmt , "%%10.%df %%8.%df", na, np);
    sprintf (fmtf, " %%10.0f %%6s  # w=%%g l=%%g\n");
    sprintf (fmtg, " %%10g %%6s  # w=%%g l=%%g\n");
    rest = "0.01";

    for (i = 0; i < SIZE_T1; i++) {
        width = minsize * t1sizefac[i];
        length = width;
        sprintf (cellname, "t1_%dx%d", width, length);
	subres_rval = 0;
	ReadSLS (cellname, 1);
        subrestable[i] = subres_rval;
        if (subres_rval <= 0) continue;
        w = width * 1e-3;
        l = length * 1e-3;
        area = w * l; perim = 2 * (w + l);

	sprintf (line, "%g", subres_rval);
	fprintf (OUT, fmt, area, perim);
	if (strchr (line, 'e'))
	    fprintf (OUT, fmtf, subres_rval, rest, w, l);
	else
	    fprintf (OUT, fmtg, subres_rval, rest, w, l);

        width = width / 2;
        length = length * 2;
        sprintf (cellname, "t1_%dx%d", width, length);
	subres_rval = 0;
	ReadSLS (cellname, 1);
        if (subres_rval <= 0) continue;
        w = w / 2;
        l = l * 2;
        area = w * l; perim = 2 * (w + l);

	sprintf (line, "%g", subres_rval);
	fprintf (OUT, fmt, area, perim);
	if (strchr (line, 'e'))
	    fprintf (OUT, fmtf, subres_rval, rest, w, l);
	else
	    fprintf (OUT, fmtg, subres_rval, rest, w, l);

        length = length * 3 / 4;
        sprintf (cellname, "t1_%dx%d", width, length);
	subres_rval = 0;
	ReadSLS (cellname, 1);
        if (subres_rval <= 0) continue;
        l = l * 3 / 4;
        area = w * l; perim = 2 * (w + l);

	sprintf (line, "%g", subres_rval);
	if (i == 0 && k > np)
	    fprintf (OUT, fmt2, area, perim);
	else
	    fprintf (OUT, fmt, area, perim);
	if (strchr (line, 'e'))
	    fprintf (OUT, fmtf, subres_rval, rest, w, l);
	else
	    fprintf (OUT, fmtg, subres_rval, rest, w, l);
    }

    fprintf (OUT, "\n");

    if (!passed_coupsubres) {
	while (fgets (line, 512, IN) && strncmp (line, "coupsubres", 10)) {
	    fprintf (OUT, line);
        }
	while (fgets (line, 512, IN) && *line != '\n') ;
    }

    fprintf (OUT, "coupsubres :\n");
    fprintf (OUT, "#   Generated by subresgen on %s", localtime);
    fprintf (OUT, "#    area1      area2     dist          r    decr\n");

    for (k = i = 0; i < SIZE_T2; i++) {
        width = minsize * t2sizefac[i];
        for (j = 0; j < SIZE_T2; j++) {
	    sprintf (cellname, "t2_%dx%d_%d", width, width, width * t2distfac[j]);
            coupres_rval = coupsubres_rval = 0;
            ReadSLS (cellname, 2);
	    coupres[k] = coupres_rval;
	    coupsubres[k] = coupsubres_rval;
	    ++k;
        }
    }

    sprintf (fmtf, "%%10.%df %%10.%df %%8g %%10.0f %%9.6f  # w=%%g\n", na, na);
    sprintf (fmtg, "%%10.%df %%10.%df %%8g %%10g %%9.6f  # w=%%g\n", na, na);

    for (k = i = 0; i < SIZE_T2; i++) {
        for (j = 0; j < SIZE_T1; j++) {
	    if (t1sizefac[j] == t2sizefac[i]) break;
	}
	subres_rval = subrestable[j];
	w = 1e-3 * minsize * t2sizefac[i];
        area = w * w;
        for (j = 0; j < SIZE_T2; j++) {
	    l = w * t2distfac[j];
	    coupres_rval = coupres[k];
	    coupsubres_rval = coupsubres[k];
	    ++k;
            if (coupres_rval > 0 && subres_rval > 0 && coupsubres_rval > 0) {
                decr = ((coupres_rval / subres_rval) - (coupres_rval / coupsubres_rval));
		sprintf (line, "%g", coupres_rval);
		if (strchr (line, 'e'))
		    fprintf (OUT, fmtf, area, area, l, coupres_rval, decr, w);
		else
		    fprintf (OUT, fmtg, area, area, l, coupres_rval, decr, w);
            }
	}
    }

    fprintf (OUT, "\n");

    while (fgets (line, 512, IN)) fprintf (OUT, line);

    fclose (IN);
    fclose (OUT);

    rename (tmpfile, inputfile);
}

double Recalc (char *val)
{
    int j = strlen (val);

    switch (val[j-1]) {
	case 'G': return (1e9   * atof (val));
	case 'M': return (1e6   * atof (val));
	case 'k': return (1e3   * atof (val));
	case 'm': return (1e-3  * atof (val));
	case 'u': return (1e-6  * atof (val));
	case 'n': return (1e-9  * atof (val));
	case 'p': return (1e-12 * atof (val));
	case 'f': return (1e-15 * atof (val));
    }
    return (atof (val));
}

void concat (char *f1, char *f2, char *d)
{
    FILE *IN1, *IN2, *OUT;
    int c;

    if (!(IN1 = fopen (f1, "r"))) { printf ("cannot read %s\n", f1); exit (1); }
    if (!(IN2 = fopen (f2, "r"))) { printf ("cannot read %s\n", f2); exit (1); }
    if (!(OUT = fopen (d,  "w"))) { printf ("cannot write %s\n", d); exit (1); }
    while ((c = fgetc (IN1)) != EOF) fputc (c, OUT);
    while ((c = fgetc (IN2)) != EOF) fputc (c, OUT);
    fclose (OUT);
    fclose (IN2);
    fclose (IN1);
}

int run (char *prog, char *arg1, char *arg2, char *arg3)
{
    char *argv[6];
    int status = 0;

    argv[0] = prog;
    argv[1] = arg1;
    argv[2] = arg2;
    argv[3] = arg3;
    argv[4] = NULL;

    if (vfork () == 0) { /* child */
	execvp (prog, argv);
	_exit (1);
    }
    wait (&status);

    return (status);
}
