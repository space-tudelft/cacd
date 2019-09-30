/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
 *	O. Hol
 *	P.E. Menchen
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

#include "src/cfun/func_parsedefs.h"

extern int yyparse (void);
FILE   *yyin, *yyout;
char   *infile;
char   baseinfile[200+4];

DM_CELL *key;
DM_STREAM *fdes;
DM_PROJECT *dmproject;

char   *argv0 = "cfun";	/* Program Name */

int     verbose = 1;
int     kflag = 0;
char   *C_options[40+4];
char   *P_options[40+4];

static int pflag = 0;

int main (int argc, char *argv[])
{
/*  int     cursflag = 0; */
    int     i, k;
    char   *s;
    struct stat buf;

    infile = NULL;
    i = k = 0;
    C_options[i++] = "cc";
    P_options[k++] = "cpp";

    while (--argc && ++argv) {
	s = *argv;
	if (*s == '-') { /* option */
	    while (*++s) {
		switch (*s) {
		    case 'C':
		    case 'P':
			if (*(s+1)) {
			    if (*s == 'C') {
				if (i < 40) C_options[i++] = s;
			    } else {
				if (k < 40) P_options[k++] = s;
			    }
			    *s = '-';
			    while (*++s) ;
			    --s;
			}
			break;
		    case 'k':
			kflag = 1;
			break;
		    case 'p':
			pflag = 1;
			break;
		    case 's':
			verbose = 0;
			break;
		    case 'y':
			yydebug = 1;
			break;
		    default:
			usage ("bad option:", *s);
		}
	    }
	}
	else if (!infile) {
	    infile = s;
#ifdef __CYGWIN__
	    for (; *s; ++s) if (*s == '\\') *s = '/';
#endif
	}
	else {
	    usage ("too many arguments", ' ');
	}
    }

    C_options[i] = NULL;
    P_options[k] = NULL;

    if (infile) {
	if (stat (infile, &buf) != 0)
	    usage ("cannot find input file", ' ');
    }
    else {
	usage ("no input file present", ' ');
    }

    k = i = strlen (infile) - 1;
    while (i > 0 && infile[i] != '/' && infile[i] != '.') --i;

    if (infile[i] != '.' || i == k) {
	usage ("input file does not end with an extension", ' ');
    }
    if (infile[i+1] == 'c') usage ("input file does not have a correct extension", ' ');
    if (infile[i+1] == 'p') pflag = 1;
    k = i;
    while (--i >= 0 && infile[i] != '/') ;
    ++i;
    if ((k -= i) > 200) k = 200;
    if (k < 1) usage ("input file does not have a correct name", ' ');

    strncpy (baseinfile, &infile[i], k);
    baseinfile[k++] = '.';
    baseinfile[k++] = 'p';
    baseinfile[k--] = 0;

    if (!pflag) {
	cppexec (infile, baseinfile);
	s = baseinfile;
    }
    else s = infile;

    yyin = fopen (s, "r");
    if (!yyin) die (2, s, "");

    baseinfile[k] = 'c';
    yyout = fopen (baseinfile, "w");
    if (!yyout) die (2, baseinfile, "");

/*
    fprintf (yyout, "#include <stdio.h>\n");
    if (cursflag != 0)
	fprintf (yyout, "#include <curses.h>\n");
*/
    dmInit (argv0);   /* to obtain icdpath */

    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);
    /* do it already now to read maximum name length. */

    fprintf (yyout, "#include \"%s/share/src/sls/func.h\"\n", icdpath);
    if (pflag) lineno (1);

    if (verbose) {
	baseinfile[k] = 0;
	if (pflag)
	    fprintf (stderr, "Parsing %s ; making %sc\n", infile, baseinfile);
	else
	    fprintf (stderr, "Parsing %sp ; making %sc\n", baseinfile, baseinfile);
    }

    baseinfile[k] = 'p';
    yylineno = 1;
    yyparse ();
    fclose (yyin);
    fclose (yyout);

    if (!pflag && !kflag) rmexec (baseinfile);

    key = dmCheckOut (dmproject, Func_name, WORKING, DONTCARE, CIRCUIT, UPDATE);

    addfun_term (ftrm_list);
    addfun_obj (baseinfile, k);

    dmCheckIn (key, COMPLETE);
    dmQuit ();
    exit (0);
    return (0);
}

void usage (char *s1, char c)
{
    fprintf (stderr, "%s: %s %c\n", argv0, s1, c);
    fprintf (stderr, "\nUsage: %s [-kps] [-C\"option\"] [-P\"option\"] infile\n\n", argv0);
    exit (1);
}

void yyerror (char *s)
{
    fprintf (stderr, "%s: %s, in line %d: %s\n", argv0, infile, yylineno, s);
    if (yydebug == 0) die (5, "", "");
}

void lineno (int line)
{
    fprintf (yyout, "#line %d ", line);
    if (infile) fprintf (yyout, "\"%s\"", infile);
    fprintf (yyout, "\n");
}

void dmError (char *s)
{
    fprintf (stderr, "%s: ", argv0);
    dmPerror (s);
    die (0, "", "");
}

char *errlist[] = {
/* 0 */  "error in DMI function",
/* 1 */  "cannot allocate memory",
/* 2 */  "cannot open file '%s'",
/* 3 */  "no successful compilation of '%s'",
/* 4 */  "C preprocessor on '%s' unsuccesfull",
/* 5 */  "yyerror exit",
/* 6 */  "index of terminal '%s' must be an integer",
/* 7 */  "terminal name '%s' is already used",
/* 8 */  "terminal '%s' in routine delay is not output or inout",
/* 9 */  "terminal '%s', illegal call of routine %s",
/* 10 */ "cannot find terminal '%s' in routine %s",
/* 11 */ "terminal '%s' in routine cap_add is not input or inread",
/* 12 */ "terminal '%s' in routine %s_val is not a terminal",
/* 13 */ "terminal '%s', illegal call of routine %s_val",
/* 14 */ "internal error in evaluating routine %s",
/* 15 */ "unknown error"
};

void die (int nr, char *s1, char *s2)
{
    if (nr < 0 || nr > 15) nr = 15;
    fprintf (stderr, "%s: ", argv0);
    fprintf (stderr, errlist[nr], s1, s2);
    fprintf (stderr, "\n%s: -- program aborted --\n", argv0);
    dmQuit ();
    exit (1);
}
