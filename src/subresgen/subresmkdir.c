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

char *argv0 = "subresmkdir";
char *usage = "\nUsage: %s dirname min_term_size\n\n";

int t1sizefac[] = { 1, 2, 4, 8, 16, 32, 64, 0 };
int t2sizefac[] = { 1, 2, 4, 32, 0 };
int t2distfac[] = { 2, 4, 8, 32, 0 };

FILE *fopen_w (char *file);
void mkLayout (char *cellname, int nr_contacts);
void mkTechDir (void);
void mkProjDir (void);
void run (char *prog, char *arg1, char *arg2, char *arg3);

char *tech_dir = "./process";
int minsize;

int main (int argc, char *argv[])
{
    struct stat sbuf;
    FILE *SIZEFILE;
    char *sizefile;
    char *new_dir = argv[1];

    if (argc != 3) {
	printf ("%s: wrong number of arguments\n", argv0);
	printf (usage, argv0);
	return (1);
    }

    minsize = atoi (argv[2]);

    if (minsize < 10 || minsize > 10000) {
	printf ("%s: wrong min_term_size, must be >= 10 and <= 10000 [nm]\n", argv0);
	printf (usage, argv0);
	return (1);
    }

    if (lstat (new_dir, &sbuf) == 0) {
        printf ("%s: %s: directory already exists\n", argv0, new_dir);
	goto ret;
    }

    mkdir (new_dir, 0755);
    if (chdir (new_dir)) {
	printf ("%s: %s: cannot chdir\n", argv0, new_dir);
	goto ret;
    }

    sizefile = ".min_term_size";
    SIZEFILE = fopen_w (sizefile);
    fprintf (SIZEFILE, "%d\n", minsize);
    fclose (SIZEFILE);
    chmod (sizefile, 0444);

    mkTechDir ();

    mkProjDir ();

    printf ("\nDirectory '%s' has been created for you.\n", new_dir);
    printf ("Please go to that directory, put your element definition file 'space.def.s'\n");
    printf ("with an appropriate 'sublayers' list at that location, and run subresgen.\n\n");
    printf ("%s: -- job finished --\n", argv0);
    return (0);
ret:
    printf ("%s: -- job aborted --\n", argv0);
    return (1);
}

void mkTechDir ()
{
    FILE *fp, *MASKDATA, *BMLIST, *SPACEDEFP, *SPACEHEADERS;

    mkdir (tech_dir, 0755);
    if (chdir (tech_dir)) {
	printf ("%s: %s: cannot chdir\n", argv0, tech_dir);
	exit (1);
    }

    fp = fopen_w ("default_lambda");
    fprintf (fp, "0.001\n");
    fclose (fp);

    MASKDATA = fopen_w ("maskdata");
    fprintf (MASKDATA, "\"cmos\"  \"cmos process for substrate resistance generation\"\n");
    fprintf (MASKDATA, "#---------------------------------------------+---------------------\n");
    fprintf (MASKDATA, "# Layer     | PG-Tape | CMask | Dali  | Plot  | Comment\n");
    fprintf (MASKDATA, "#-----------+---------+-------+-------+-------+---------------------\n");
    fprintf (MASKDATA, "# name      | job     | color | color | pen   |\n");
    fprintf (MASKDATA, "# |    type | |  type | | fill| | fill| | fill|\n");
    fprintf (MASKDATA, "# |    |    | |  |    | |  |  | |  |  | |  |  |\n");
    fprintf (MASKDATA, "# 1    2    | 3  4    | 5  6  | 7  8  | 9  10 |\n");
    fprintf (MASKDATA, "#-v----v----+-v--v----+-v--v--+-v--v--+-v--v--+---------------------\n");
    fprintf (MASKDATA, "cmf    1      3  1      3  1    4  1    4  0   \"metal\"\n");
    fclose (MASKDATA);

    BMLIST = fopen_w ("bmlist.gds");
    fprintf (BMLIST, "cmf       49\n");
    fprintf (BMLIST, "cmf:label 49 9\n");
    fclose (BMLIST);

    SPACEDEFP = fopen_w ("space.def.p");
    fprintf (SPACEDEFP, "BEGIN sub3d             # Data for 3D substrate resistance extraction\n");
    fprintf (SPACEDEFP, "be_shape           4\n");
    fprintf (SPACEDEFP, "be_mode            0g\n");
    fprintf (SPACEDEFP, "max_be_area        0.002\n");
    fprintf (SPACEDEFP, "edge_be_ratio      0.01\n");
    fprintf (SPACEDEFP, "edge_be_split      0.2\n");
    fprintf (SPACEDEFP, "saw_dist           0\n");
    fprintf (SPACEDEFP, "edge_dist          0\n");
    fprintf (SPACEDEFP, "be_window          inf\n");
    fprintf (SPACEDEFP, "END sub3d\n");
    fprintf (SPACEDEFP, "\n");
    fprintf (SPACEDEFP, "compression        off\n");
    fprintf (SPACEDEFP, "\n");
    fprintf (SPACEDEFP, "BEGIN disp           # Data for Xspace\n");
    fprintf (SPACEDEFP, "save_prepass_image  on\n");
    fprintf (SPACEDEFP, "draw_be_mesh        on\n");
    fprintf (SPACEDEFP, "END disp\n");
    fclose (SPACEDEFP);

    SPACEHEADERS = fopen_w ("space.header.s");
    fprintf (SPACEHEADERS, "unit vdimension    1e-6  # um\n");
    fprintf (SPACEHEADERS, "\n");
    fprintf (SPACEHEADERS, "colors :\n");
    fprintf (SPACEHEADERS, "    cmf   blue\n");
    fprintf (SPACEHEADERS, "    @sub  pink\n");
    fprintf (SPACEHEADERS, "\n");
    fprintf (SPACEHEADERS, "conductors :\n");
    fprintf (SPACEHEADERS, "  # name    : condition     : mask : resistivity : type\n");
    fprintf (SPACEHEADERS, "    cond_mf : cmf           : cmf  : 0.0         : m    # first metal\n");
    fprintf (SPACEHEADERS, "\n");
    fprintf (SPACEHEADERS, "contacts :\n");
    fprintf (SPACEHEADERS, "  # name   : condition  : lay1 lay2 : resistivity\n");
    fprintf (SPACEHEADERS, "    cont_b : cmf        : cmf  @sub : 0   # metal to subs\n");
    fprintf (SPACEHEADERS, "\n");
    fprintf (SPACEHEADERS, "sublayers :\n");
    fclose (SPACEHEADERS);

    chdir ("..");
}

int width, length, dist;

void mkProjDir ()
{
    char cellname[56];
    int i, j;

    run ("mkpr", "-p", tech_dir, ".");

    for (i = 0; t1sizefac[i]; i++) {
        length = width = minsize * t1sizefac[i];
        sprintf (cellname, "t1_%dx%d", width, length);
        mkLayout (cellname, 1);

        width = width / 2;
        length = length * 2;
        sprintf (cellname, "t1_%dx%d", width, length);
        mkLayout (cellname, 1);

        length = length * 3 / 4;
        sprintf (cellname, "t1_%dx%d", width, length);
        mkLayout (cellname, 1);
    }

    for (i = 0; t2sizefac[i]; i++) {
        length = width = minsize * t2sizefac[i];
        for (j = 0; t2distfac[j]; j++) {
            dist = width * t2distfac[j];
            sprintf (cellname, "t2_%dx%d_%d", width, length, dist);
            mkLayout (cellname, 2);
        }
    }
}

void mkLayout (char *cellname, int nr_contacts)
{
    int xl, xr;
    FILE *LDM = fopen_w ("tmp.ldm");

    fprintf (LDM, "ms %s\n", cellname);
    fprintf (LDM, "term cmf 0 %d 0 %d a\n", width, length);
    if (nr_contacts == 2) {
	xl = width + dist;
	xr = xl + width;
	fprintf (LDM, "term cmf %d %d 0 %d b\n", xl, xr, length);
    }
    fprintf (LDM, "me\n");
    fclose (LDM);

    run ("cldm", "tmp.ldm", NULL, NULL);
}

FILE *fopen_w (char *file)
{
    FILE *fp = fopen (file, "w");
    if (!fp) { printf ("%s: %s: cannot write\n", argv0, file); exit (1); }
    return (fp);
}

void run (char *prog, char *arg1, char *arg2, char *arg3)
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
        printf ("%s: cannot run %s\n", argv0, prog);
	_exit (1);
    }
    wait (&status);

    if (status) printf ("%s: warning: %s status != 0\n", argv0, prog);
}
