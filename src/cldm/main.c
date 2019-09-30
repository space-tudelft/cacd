/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#include "src/cldm/gdef.h"

#define TOOLVERSION "4.32 18-May-2011"

extern int yylineno;

char   *argv0;			/* program name */
char   *use_msg =		/* command line */
#ifdef CLDM
        "\nUsage: %s [-4bforsvx] inputfile\n\n";
#else
        "\nUsage: %s [-4bforstvxz] [-u unit] [-w size] inputfile\n\n";
#endif

static int noerr = 0;

int main (int argc, char *argv[])
{
    char *s, *inputfile = NULL;
    int     i, iarg;
    int     usage = 0;
    double  lambda;

    argv0 = argv[0];
    if ((s = strrchr (argv0, '/'))) argv0 = s + 1;
#ifdef WIN32
    if ((s = strrchr (argv0, '\\'))) argv0 = s + 1;
#endif

    for (iarg = 1; iarg < argc && argv[iarg][0] == '-'; iarg++) {
	for (i = 1; argv[iarg][i] != '\0'; i++) {
	    switch (argv[iarg][i]) {
		case '4':
		    ++mode45;
		    P_E "%s: -4: 45 degree mode\n", argv0);
		    break;
		case 'o':
		    x_mode = 0;
		    P_E "%s: -o: model no-origin mode\n", argv0);
		    break;
		case 'x':
		    x_mode = 1;
		    P_E "%s: -x: model origin mode\n", argv0);
		    break;
		case 'r':
		    ++r_mode;
		    P_E "%s: -r: no rounding messages\n", argv0);
		    break;
		case 's':
		    ++s_mode;
		    P_E "%s: -s: syntax check only\n", argv0);
		    break;
		case 'v':
		    ++v_mode;
		    P_E "%s: -v: verbose\n", argv0);
		    break;
		case 'f':
		    ++f_mode;	/* force cell overwrite */
		    P_E "%s: -f: force cell overwrite\n", argv0);
		    break;
		case 'b':
		    ++b_mode;	/* bell on */
		    break;
#ifndef CLDM
		case 'u':
		    if (argv[iarg+1] && isdigit ((int)(argv[iarg+1][0]))) {
			cifunit = atoi (argv[iarg+1]);
			iarg++;
			for (i = 1; argv[iarg][i] != '\0'; i++);
			i--;
			P_E "%s: -u: cifunit set to: %ld\n", argv0, cifunit);
		    }
		    else
			++usage;
		    break;
		case 'w':
		    if (argv[iarg+1] && isdigit ((int)(argv[iarg+1][0]))) {
			t_width = atoi (argv[iarg+1]);
			iarg++;
			for (i = 1; argv[iarg][i] != '\0'; i++);
			i--;
			if (t_width < 0) t_width = 0;
			P_E "%s: -w: terminal width set to: %ld\n", argv0, t_width);
                    }
		    else
			++usage;
                    break;
		case 't':
		    ++t_mode;   /* generate terminal boxes */
		    P_E "%s: -t: generate terminal boxes\n", argv0);
		    break;
		case 'z':
		    ++delft_old; /* using the old 94 user mode */
		    P_E "%s: -z: using the old 94 user mode\n", argv0);
		    break;
#endif
		default:
		    ++usage;
		    P_E "%s: -%c: unknown option\n", argv0, argv[iarg][i]);
	    }
	}
    }

    if (iarg >= argc || usage) {/* only options ? */
	if (argc < 2) P_E "\n%s %s\n", argv0, TOOLVERSION);
	P_E use_msg, argv0);
	exit (1);
    }
    if (iarg + 1 < argc) {
	pr_exit (04, 21, 0);	/* too many arg's */
	P_E use_msg, argv0);
	exit (1);
    }
    inputfile = argv[iarg];

    if (stat (inputfile, &stat_buf) == -1)
	pr_exit (0107, 22, inputfile);
    if ((stat_buf.st_mode & S_IFREG) != S_IFREG)
	pr_exit (0107, 23, inputfile);
    if (freopen (inputfile, "r", stdin) == NULL)
	pr_exit (0107, 24, inputfile);

#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN);	/* ignore hangup signal */
#endif
    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	signal (SIGINT, sig_handler);
#ifdef SIGQUIT
    if (signal (SIGQUIT, SIG_IGN) != SIG_IGN)
	signal (SIGQUIT, sig_handler);
#endif
    if (signal (SIGTERM, SIG_IGN) != SIG_IGN)
	signal (SIGTERM, sig_handler);
    signal (SIGILL, sig_handler);
    signal (SIGFPE, sig_handler);
#ifdef SIGBUS
    signal (SIGBUS, sig_handler);
#endif
    signal (SIGSEGV, sig_handler);

    if (!s_mode) {

	dmInit (argv0);
	dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);
	lambda = dmproject -> lambda;

    /* get process information */
	process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, dmproject);

    /* read model list into the model tree */
	ini_modtree ();
    }
#ifndef CLDM
    else {
	noerr = 1;
	dmInit (argv0);
	dmproject = dmOpenProject (DEFAULT_PROJECT, PROJ_READ);
	if (dmproject) lambda = dmproject -> lambda;
	else lambda = 0.01;
	dmQuit ();
	noerr = 0;
    }

    cifsf = lambda * cifunit;
    if (v_mode) {
	P_E "-- lambda = %g micron\n", lambda);
	P_E "-- cifunit = %ld (= %g micron)\n", cifunit, (double)(1.0 / cifunit));
	P_E "-- resolution = %g\n", 1.0 / cifsf);
    }
#endif

 /* start parsing LDM/CIF input */
    yylineno = 1;
    yyparse ();

 /* we cannot come here */
    return (0);
}

void dmError (char *s)
{
    if (noerr) return;
    P_E "%s: ", argv0);
    dmPerror (s);
    pr_exit (0107, 0, "error in DMI function");
}

void sig_handler (int sig) /* signal handler */
{
    char    buf[6];
    signal (SIGINT, SIG_IGN);	/* ignore intr signal */
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);	/* ignore quit signal */
#endif
    sprintf (buf, "%d", sig);
    pr_exit (0107, 6, buf);
}
