/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

#include <unistd.h>
#include "src/sls_exp/extern.h"

#define  SLSFUNF  "sls.funlist"
#define  FUNCC    "deffunc.c"
#define  FUNCOBJ  "deffunc.o"
#ifdef __GNUC__
#define  CCCOM    "gcc"
#define  LDCOM    "g++"
#else
#define  CCCOM    "cc"
#define  LDCOM    "cc"
#endif

#define  CCOPT    "-c -I"
#define  CCOPTG   "-g -c -I"

#define  LDOPT    ""
#define  LDOPTG   "-g"
#ifdef __GNUC__
#define  SIMOBJ   "*.o -lm"
#else
#define  SIMOBJ   "*.o -ll"
#endif
#define  LDARGF   "sls.ld_arg"

/* If neccessary, mksim makes a new simulator executable.
   When a new one is made 1 will be returned, otherwise 0.
*/

extern int strcmp_quick (char *s1, char *s2);

extern int debugcomp;

int mksim ()
{
    struct stat buf;
    int strend;
    int make;
    int i, j;
    int found;
    int unknown;
    int endreached;
    char fname[NAMESIZE];
    FILE * fp;
    char c;
    char *syscom;
    int syscom_size;
    int len;
    char *simname = "funcsls";
 // char *extralibs = "-lpthread";
    char *extralibs = "";

    if (FD_cnt == 0) { /* No functions in network. */
	/* Remove the file. It should not be used. */
	if (stat (simname, &buf) == 0) {
	    if (!silent) fprintf (stderr, "Removing %s\n", simname);
	    unlink (simname);
	}
	return (0);
    }

    if (!force_exp && stat (simname, &buf) == 0) {
        if (buf.st_mtime <= newest_ftime)
            make = TRUE;
        else {
            if ((fp = fopen (SLSFUNF, "r")) == NULL) {
                fprintf (stderr, "Can't read %s\n", SLSFUNF);
                make = TRUE;
            }
            else {
                unknown = FALSE;
                for (i = 0; i < FD_cnt && ! unknown; i++) {
                    rewind (fp);
                    found = FALSE;
                    fname[0] = 'a';
                    while (!found && fscanf (fp, "%s", fname) == 1) {
                        if (strcmp_quick (FD[i].name, fname) == 0)
                            found = TRUE;
                    }
                    if (!found) unknown = TRUE;
                }
                if (unknown)
                    make = TRUE;
                else
                    make = FALSE;
            }
            fclose (fp);
        }
    }
    else
        make = TRUE;

    if (make) {
        if (!silent) fprintf (stderr, "Making %s\n", simname);

        OPENW (fp, FUNCC);

        fprintf (fp, "#include <stdio.h>\n\n");
        fprintf (fp, "#include \"src/libddm/dmincl.h\"\n\n");
	fprintf (fp, "#include \"src/sls_exp/gndefine.h\"\n\n");
	fprintf (fp, "#include \"src/sls_exp/gntype.h\"\n\n");

        for (i = 0; i < FD_cnt; i++) {
            fprintf (fp, "extern void L%s (char *S);\n", FD[i].name);
            fprintf (fp, "extern void I%s (char *S);\n", FD[i].name);
            fprintf (fp, "extern void E%s (char *S);\n", FD[i].name);
        }

        fprintf (fp, "\nFUNCDESCR FD[] = {\n");
        for (i = 0; i < FD_cnt; i++) {
            endreached = FALSE;
            for (j = 0; j < NAMESIZE; j++) {
                if (endreached || FD[i].name[j] == '\0') {
                    fprintf (fp, "0,");
                    endreached = TRUE;
                }
                else {
                    fprintf (fp, "'%c',", FD[i].name[j]);
                }
            }
            endreached = FALSE;
            for (j = 0; j < MAXDMPATH + 1; j++) {
                if (endreached || FD[i].dmpath[j] == '\0') {
                    fprintf (fp, "0,");
                    endreached = TRUE;
                }
                else {
                    fprintf (fp, "'%c',", FD[i].dmpath[j]);
                }
            }
            fprintf (fp, "0,");  /* FD[i].help */
            fprintf (fp, "%d,", FD[i].fvx);
            fprintf (fp, "%d,", FD[i].fvx_cnt);
            fprintf (fp, "%d", FD[i].offsx);
            if (i != FD_cnt - 1)
                fprintf (fp, ",");
            fprintf (fp, "\n");
        }
        fprintf (fp, "};\n");

        fprintf (fp, "\nFUNCVAR FV[] = {\n");
        for (i = 0; i < FV_cnt; i++) {
            endreached = FALSE;
            for (j = 0; j < NAMESIZE; j++) {
                if (endreached || FV[i].name[j] == '\0') {
                    fprintf (fp, "0,");
                    endreached = TRUE;
                }
                else {
                    fprintf (fp, "'%c',", FV[i].name[j]);
                }
            }
            fprintf (fp, "0,");
            fprintf (fp, "%d,", FV[i].type);
            fprintf (fp, "%d,%d", FV[i].ind[0], FV[i].ind[1]);
            if (i != FV_cnt - 1)
                fprintf (fp, ",");
            fprintf (fp, "\n");
        }
        fprintf (fp, "};\n");

        fprintf (fp, "\nint FD_cnt = %d;\n", FD_cnt);
        fprintf (fp, "int FV_cnt = %d;\n", FV_cnt);

        fprintf (fp, "\nvoid loaddf (int fx, char *S)\n");
        fprintf (fp, "{\n");
        fprintf (fp, "switch (fx) {\n");
        for (i = 0; i < FD_cnt; i++) {
            fprintf (fp, "case %d: ", i);
            fprintf (fp, "L%s (S); ", FD[i].name);
            fprintf (fp, "break;\n");
        }
        fprintf (fp, "}\n");
        fprintf (fp, "}\n");

        fprintf (fp, "\nvoid initdf (int fx, char *S)\n");
        fprintf (fp, "{\n");
        fprintf (fp, "switch (fx) {\n");
        for (i = 0; i < FD_cnt; i++) {
            fprintf (fp, "case %d: ", i);
            fprintf (fp, "I%s (S); ", FD[i].name);
            fprintf (fp, "break;\n");
        }
        fprintf (fp, "}\n");
        fprintf (fp, "}\n");

        fprintf (fp, "\nvoid evaldf (int fx, char *S)\n");
        fprintf (fp, "{\n");
        fprintf (fp, "switch (fx) {\n");
        for (i = 0; i < FD_cnt; i++) {
            fprintf (fp, "case %d: ", i);
            fprintf (fp, "E%s (S); ", FD[i].name);
            fprintf (fp, "break;\n");
        }
        fprintf (fp, "}\n");
        fprintf (fp, "}\n");

        CLOSE (fp);

        syscom_size = 1200 + FD_cnt * 80;
	PALLOC (syscom, syscom_size, char);

	if ( debugcomp )
            sprintf (syscom, "%s %s%s/share %s", CCCOM, CCOPTG, icdpath, FUNCC);
	else
            sprintf (syscom, "%s %s%s/share %s", CCCOM, CCOPT, icdpath, FUNCC);

        if (!silent) fprintf (stderr, "%s\n", syscom);

        if (system (syscom) != 0) {
            fprintf (stderr, "\nNo successful compilation of %s\n\n", FUNCC);
            die (1);
        }

	if ( debugcomp ) {

	    sprintf (syscom, "%s %s %s/lib/sls/%s %s",
		     LDCOM, LDOPTG, icdpath, SIMOBJ, FUNCOBJ);

            /* remove the -s option if it is present */

	    len = strlen (syscom);
	    for (i = 0; i < len; i++) {
		if (syscom[i] == '-' && syscom[i+1] == 's') {
		    while (i + 3 <= len) {
			syscom[i] = syscom[i+3];
			i++;
		    }
		}
	    }
	}
	else {
	    sprintf (syscom, "%s %s %s/lib/sls/%s %s",
		     LDCOM, LDOPT, icdpath, SIMOBJ, FUNCOBJ);
        }

        strend = strlen (syscom);
        sprintf (syscom + strend, " %s/lib/libddm.a", icdpath);

	if (*extralibs) {
	    strend = strlen (syscom);
	    sprintf (syscom + strend, " %s", extralibs);
	}

        for (i = 0; i < FD_cnt; i++) {
            strend = strlen (syscom);
	    sprintf (syscom + strend, " %s/circuit/%s/%s",
		     FD[i].dmpath, FD[i].name, sls_o_fn);
        }

        strend = strlen (syscom);
        sprintf (syscom + strend, " %s/lib/libfunc.a", icdpath);

#ifdef OLIBS
        strend = strlen (syscom);
        sprintf (syscom + strend, " %s", OLIBS);
#endif

        if ((fp = fopen (LDARGF, "r")) != NULL) {
            strend = strlen (syscom);
            *(syscom + strend) = ' ';
            strend++;
            while ((c = fgetc (fp)) != EOF) {
                if (c == '\n') c = ' ';
                if (c != ' ' || *(syscom + strend - 1) != ' ') {
                    *(syscom + strend) = c;
                    strend++;
                }
            }
            if (*(syscom + strend - 1) == ' ')
                *(syscom + strend - 1) = '\0';
            else
                *(syscom + strend) = '\0';
        }

        strend = strlen (syscom);
        sprintf (syscom + strend, " -o %s", simname);

        if (! silent)
            fprintf (stderr, "+ %s\n", syscom);

        if (system (syscom) != 0) {
            fprintf (stderr, "\nNo successful linking of %s\n\n", simname);
            die (1);
        }

        OPENW (fp, SLSFUNF);
        for (i = 0; i < FD_cnt; i++) {
             fprintf (fp, "%s\n", FD[i].name);
        }
        CLOSE (fp);
    }

    if (make)
	return (1);
    else
	return (0);
}
