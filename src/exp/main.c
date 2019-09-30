/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	S. de Graaf
 *	A.J. van Genderen
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/libddm/dmincl.h"

#define P_E fprintf(stderr,

char   *cmd_bxl = "makeboxl";
char   *cmd_bxh = "makeboxh";
char   *cmd_vln = "makevln";
char   *cmd_gln = "makegln";
char    command[512];
char    opt_bxx[256];
char    opt_vln[20];
char   *o_b = opt_bxx;
char   *o_v = opt_vln;
char   *ap;

int exp_depth = 1;
int flag_L = 0;
int flag_v = 0; /* verbose mode (if true) */

char   *argv0 = "exp";	/* program name */
char   *use_msg =	/* command line */
"\nUsage: %s [-B|-V|-G] [-DHLhvg] [-cL] [-d[N]] [-oN] [-wXl,Xr,Yb,Yt] [cell ...]\n\n";

char **mktree (char *cell);
void do_opt (char **pp);
void do_system (char *cmd, char *opt, char *arg);

int main (int argc, char *argv[])
{
    int     iarg;	    /* argument number */
    int     usage  = 0;     /* print use message (if true) */
    int     flag_h = 0;     /* hierarchical mode (if true) */
    int     flagBV = 3;     /* do both makebox and makevln */
    int	    flag_g = 0;	    /* substitute makegln for makevln */
    int     flag_H = 0;     /* run makeboxl -H */
    int     flag_D = 0;
    int     flag_c = 0;
    int     flag_d = 0;
    int     flag_o = 0;
    int     flag_w = 0;
    char   *tmp_ap;
    char  **celllist;
    int     i, j;
    FILE   *fp;

    for (iarg = 1; iarg < argc && argv[iarg][0] == '-'; ++iarg) {
	for (ap = &argv[iarg][1]; *ap; ++ap) {
	    switch (*ap) {
		case 'B':
		    if (flagBV == 2) {
			++usage;
			P_E "%s: use only one of -B, -V and -G\n", argv0);
		    }
		    else flagBV = 1;
		    break;
		case 'H':
		    ++flag_H;
		    flag_h = 0;
		    break;
		case 'G':
		    ++flag_g;
		case 'V':
		    if (flagBV == 1) {
			++usage;
			P_E "%s: use only one of -B, -V and -G\n", argv0);
		    }
		    else flagBV = 2;
		    break;
		case 'g':
		    ++flag_g;
		    break;
		case 'D':
		    if (!flag_D) {
			++flag_D;
			do_opt (&o_v);
		    }
		    break;
		case 'L':
		    if (!flag_L) {
			++flag_L;
			do_opt (&o_b);
		    }
		    break;
		case 'c':
		    if (flag_c) {
			++usage;
			P_E "%s: option -c already specified\n", argv0);
		    }
		    else {
			++flag_c;
			sscanf (ap + 1, "%d", &exp_depth);
			do_opt (&o_b);
		    }
		    break;
		case 'd':
		    if (flag_d) {
			++usage;
			P_E "%s: option -d already specified\n", argv0);
		    }
		    else {
			++flag_d;
			do_opt (&o_b);
		    }
		    break;
		case 'h':
		    ++flag_h;
		    flag_H = 0;
		    break;
		case 'o':
		    if (flag_o) {
			++usage;
			P_E "%s: option -o already specified\n", argv0);
		    }
		    else {
			++flag_o;
			do_opt (&o_b);
		    }
		    break;
		case 'v':
		    if (!flag_v) {
			++flag_v;
			tmp_ap = ap;
			do_opt (&o_b);
			ap = tmp_ap;
			do_opt (&o_v);
		    }
		    break;
		case 'w':
		    if (flag_w) {
			++usage;
			P_E "%s: option -w already specified\n", argv0);
		    }
		    else {
			++flag_w;
			do_opt (&o_b);
		    }
		    break;
		default:
		    ++usage;
		    P_E "%s: -%c: unknown option\n", argv0, *ap);
		    break;
	    }
	}
    }

    if (argc <= iarg && (flagBV & 1)) {
	++usage;
	P_E "%s: no cell name specified\n", argv0);
    }
    if (usage) {
	P_E use_msg, argv0);
	exit (1);
    }

    dmInit (argv0);

    do {
	if (flagBV & 1) {
	    if (flag_h)
		do_system (cmd_bxh, opt_bxx, argv[iarg]);
	    else if (flag_H) {
		celllist = mktree (argv[iarg]);

                /* append option -H */
		sprintf (opt_bxx + strlen (opt_bxx), " -H");

		/* remove option -cL */
		for (i = 0; i < strlen (opt_bxx); i++) {
		    if (opt_bxx[i] == '-' && opt_bxx[i+1] == 'c') {
			j = i;
			i = i + 2;
			while (opt_bxx[i] != ' ' && opt_bxx[i] != '\0') i++;
			while (opt_bxx[i-1] != '\0') {
			    opt_bxx[j++] = opt_bxx[i++];
			}
		    }
		    else if (opt_bxx[i] == 'c') {
			j = i;
			i = i + 1;
			while (opt_bxx[i] != ' ' && opt_bxx[i] != '\0') i++;
			while (opt_bxx[i-1] != '\0') {
			    opt_bxx[j++] = opt_bxx[i++];
			}
		    }
		}

		for (i = 0; celllist[i]; i++) {
		    do_system (cmd_bxl, opt_bxx, celllist[i]);
		}

		fp = fopen ("exp_dat", "w");
		for (i = 0; celllist[i]; i++) {
		    if (fp) fprintf (fp, "%s\n", celllist[i]);
		}
		if (fp) fclose (fp);
	    }
	    else
		do_system (cmd_bxl, opt_bxx, argv[iarg]);
	}
	if (flagBV & 2) {
	    if (flag_g) cmd_vln = cmd_gln;
	    if (flagBV == 2 || flag_g)
		do_system (cmd_vln, opt_vln, argv[iarg]);
	    else
		do_system (cmd_vln, opt_vln, "");
	}
    } while (++iarg < argc);

    dmQuit();
    exit (0);
    return (0);
}

void do_opt (char **pp)
{
    register char *p = *pp;
    register char *a = ap;

    *p++ = ' ';
    *p++ = '-';
    *p++ = *a++;
    while (*a) {
	if (*a >= 'a' && *a <= 'z') break;
	if (*a >= 'A' && *a <= 'Z') break;
	*p++ = *a++;
    }
    *pp = p;
    ap = --a;
}

void do_system (char *cmd, char *opt, char *arg)
{
    sprintf (command, "%s%s %s", cmd, opt, arg);
    if (flag_v) P_E "%s\n", command);

    if (system (command)) {
	P_E "%s: -- aborted: %s %s\n", argv0, cmd, arg);
	exit (1);
    }
}
