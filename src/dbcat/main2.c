/*
 * ISC License
 *
 * Copyright (C) 2015-2018 by
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
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "src/libddm/dmincl.h"

#define PE fprintf(stderr,

void readc_mc (char *file);
void readc_term (char *file);
void readc_net (char *file);
void header (char *file);
#ifdef DO_SIG
void sig_handler (int sig);
#endif
void error (int nr, char *s);

DM_PROJECT *project;
DM_CELL    *key;
DM_STREAM  *fp, *fp2;
char  *cell;
char  *view = "circuit";
char  strbuf[40];
char *m_mode = NULL;
char *s_mode = NULL;
int   v_mode;
int   c_mode;
int   r_mode;
int   n_mode;

long lower[10], lower1[10], lower2[10];
long upper[10], upper1[10], upper2[10];
char attribute_string[512];

char *argv0 = "dbcir";		/* program name */
char *use_msg =			/* command line */
"\nUsage: %s [-crnv] [-s stream] [-m name] cell\n\n";

int main (int argc, char *argv[])
{
    register int i, j, iarg;
    int  usage = 0;

    dm_get_do_not_alloc = 1;

    for (iarg = 1; iarg < argc && argv[iarg][0] == '-'; ++iarg) {
	j = iarg;
	for (i = 1; argv[j][i] != '\0'; ++i) {
	    switch (argv[j][i]) {
		case 'c':
		    ++c_mode;
		    r_mode = 0;
		    PE "%s: -c: show capacitors\n", argv0);
		    break;
		case 'r':
		    ++r_mode;
		    c_mode = 0;
		    PE "%s: -r: show resistors\n", argv0);
		    break;
		case 'n':
		    ++n_mode;
		    PE "%s: -c: show negative\n", argv0);
		    break;
		case 'v':
		    ++v_mode;
		    PE "%s: -v: verbose mode\n", argv0);
		    break;
		case 's':
		    s_mode = argv[++iarg];
		    PE "%s: -s: read only stream: %s\n", argv0, s_mode);
		    break;
		case 'm':
		    m_mode = argv[++iarg];
		    PE "%s: -n: list only name: %s\n", argv0, m_mode);
		    break;
		default:
		    ++usage;
		    PE "%s: -%c: unknown option\n", argv0, argv[j][i]);
	    }
	}
    }

    if (argc <= iarg) {
	PE "%s: no cell name given\n", argv0);
	++usage;
    }
    if (argc > iarg+1) {
	PE "%s: too many cells given\n", argv0);
	++usage;
    }
    if (usage) {
	PE use_msg, argv0);
	exit (1);
    }

#ifdef DO_SIG
    signal (SIGHUP , SIG_IGN); /* ignore hangup signal */
    signal (SIGQUIT, SIG_IGN);
    signal (SIGTERM, sig_handler);

    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	signal (SIGINT, sig_handler);
#endif

    dmInit ("dbcat");
    project = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    cell = argv[iarg];

    if (cell) {
	key = dmCheckOut (project, cell, WORKING, DONTCARE, CIRCUIT, ATTACH);

	if ((c_mode || r_mode) && !s_mode) {
	     readc_mc ("mc");
	}
	else if (s_mode) {
		 if (strcmp (s_mode, "mc")    == 0) readc_mc ("mc");
	    else if (strcmp (s_mode, "mc2")   == 0) readc_mc ("mc2");
	    else if (strcmp (s_mode, "net")   == 0) readc_net ("net");
	    else if (strcmp (s_mode, "net2")  == 0) readc_net ("net2");
	    else if (strcmp (s_mode, "term")  == 0) readc_term ("term");
	    else if (strcmp (s_mode, "fterm") == 0) readc_term ("fterm");
	    else error (2, s_mode);
	}
	else {
	    struct stat buf;
	    int f, n;
	    f  = dmStat (key, "fterm", &buf);
	    n  = dmStat (key, "net", &buf);
	    if (f < 0 || n == 0) {
		readc_mc ("mc");
		readc_term ("term");
		readc_net ("net");
	    }
	    if (f == 0) {
		readc_term ("fterm");
	    }
	}
	dmCheckIn (key, COMPLETE);
    }
    dmQuit ();
    return (0);
}

char **names = NULL;
short *found = NULL;
int    count = 0;

void readc_mc (char *file)
{
    register long i, nr;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if ((c_mode || r_mode) && n_mode && !s_mode) {
	if (!(fp2 = dmOpenStream (key, "mc2", "w"))) return;
    }

    cmc.inst_attribute = attribute_string;
    cmc.inst_lower = lower;
    cmc.inst_upper = upper;

    nr = 0;
    while (dmGetDesignData (fp, CIR_MC) > 0) {
	if (c_mode || r_mode) {
	    i = 0;
	    if (cmc.inst_name[0] == '_' &&
		    ((c_mode && cmc.inst_name[1] == 'C') ||
		     (r_mode && cmc.inst_name[1] == 'R'))) {
		if (n_mode && strncmp (cmc.inst_attribute, "v=-", 3)) ++i;
		else ++nr;
	    }
	    else ++i;
	    if (i) {
		if (fp2) dmPutDesignData (fp2, CIR_MC);
		continue;
	    }
	}
	else if (m_mode) {
	    if (strcmp (m_mode, cmc.cell_name)) continue;
	}
	printf ("cell:\"%s\" inst:\"%s\" attr:\"%s\" dim:%ld\n",
	    cmc.cell_name, cmc.inst_name, cmc.inst_attribute, cmc.inst_dim);
	for (i = 0; i < cmc.inst_dim; ++i) {
	    printf ("lower,upper[%ld]: %ld, %ld\n", i, cmc.inst_lower[i], cmc.inst_upper[i]);
	}
    }
    if (nr) {
	names = malloc (nr * sizeof (char *));
	found = malloc (nr * sizeof (short));
	dmSeek (fp, 0, SEEK_SET);
	i = 0;
	while (dmGetDesignData (fp, CIR_MC) > 0) {
	    if (c_mode) {
		if (cmc.inst_name[0] == '_' && cmc.inst_name[1] == 'C')
		if (strncmp (cmc.inst_attribute, "v=-", 3) == 0) {
		    names[i] = malloc (strlen (cmc.inst_name) + 1);
		    strcpy (names[i], cmc.inst_name);
		    found[i++] = 0;
		}
	    }
	}
	if (i != nr) error (3, file);
    }
    dmCloseStream (fp, QUIT);
    if (fp2) dmCloseStream (fp2, COMPLETE);

    if (nr) {
	count = nr;
	readc_net ("net");
	for (i = 0; i < nr; ++i) printf ("-- i=%ld n=%s f=%d\n", i, names[i], found[i]);
    }
}

void readc_net (char *file)
{
    register long i, j, nr, last;
    int skip, done = 0;

    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;

    if (c_mode || r_mode) fp2 = dmOpenStream (key, "net2", "w");
    else fp2 = NULL;

    cnet.net_attribute = attribute_string;
    cnet.net_lower  = lower;
    cnet.net_upper  = upper;
    cnet.inst_lower = lower1;
    cnet.inst_upper = upper1;
    cnet.ref_lower  = lower2;
    cnet.ref_upper  = upper2;

    last = 0;
    while (dmGetDesignData (fp, CIR_NET_ATOM) > 0) {
	if (!fp2) {
	    if (m_mode) {
		if (done) break;
		if (strcmp (m_mode, cnet.net_name)) continue;
		++done;
	    }
	    printf ("net:\"%s\" inst:\"%s\" attr:\"%s\" subnets:%ld\n",
		cnet.net_name, cnet.inst_name, cnet.net_attribute, cnet.net_neqv);
	    for (i = 0; i < cnet.net_dim; ++i) {
		printf ("net_lower,upper[%ld]: %ld, %ld\n", i, cnet.net_lower[i], cnet.net_upper[i]);
	    }
	    for (i = 0; i < cnet.inst_dim; ++i) {
		printf ("inst_lower,upper[%ld]: %ld, %ld\n", i, cnet.inst_lower[i], cnet.inst_upper[i]);
	    }
	    for (i = 0; i < cnet.ref_dim; ++i) {
		printf ("ref_lower,upper[%ld]: %ld, %ld\n", i, cnet.ref_lower[i], cnet.ref_upper[i]);
	    }
	}
	nr = cnet.net_neqv;

	skip = 0;
	for (j = 0; j < nr; ++j) {
	    if (dmGetDesignData (fp, CIR_NET_ATOM) <= 0) error (3, file);
	    if (!fp2) {
		printf ("    subnet[%ld]:\"%s\" inst:\"%s\" attr:\"%s\"\n", j,
		    cnet.net_name, cnet.inst_name, cnet.net_attribute);
		for (i = 0; i < cnet.net_dim; ++i) {
		    printf ("    net_lower,upper[%ld]: %ld, %ld\n", i, cnet.net_lower[i], cnet.net_upper[i]);
		}
		for (i = 0; i < cnet.inst_dim; ++i) {
		    printf ("    inst_lower,upper[%ld]: %ld, %ld\n", i, cnet.inst_lower[i], cnet.inst_upper[i]);
		}
		for (i = 0; i < cnet.ref_dim; ++i) {
		    printf ("    ref_lower,upper[%ld]: %ld, %ld\n", i, cnet.ref_lower[i], cnet.ref_upper[i]);
		}
	    }
	    else if (cnet.inst_name[0] == '_') {
		if ((c_mode && cnet.inst_name[1] == 'C') || (r_mode && cnet.inst_name[1] == 'R')) {
		    for (i = 0; i < count; ++i) {
			if (strcmp (cnet.inst_name, names[i]) == 0) { ++found[i]; ++skip; break; }
		    }
		}
	    }
	}
	if (fp2) {
	    dmSeek (fp, last, SEEK_SET);
	    if (dmGetDesignData (fp, CIR_NET_ATOM) <= 0) error (3, file);
	    if (nr != cnet.net_neqv) error (3, file);
	    cnet.net_neqv -= skip;
	    dmPutDesignData (fp2, CIR_NET_ATOM);
	    for (j = 0; j < nr; ++j) {
		if (dmGetDesignData (fp, CIR_NET_ATOM) <= 0) error (3, file);
		done = 0;
		if (skip && cnet.inst_name[0] == '_')
		if ((c_mode && cnet.inst_name[1] == 'C') || (r_mode && cnet.inst_name[1] == 'R')) {
		    for (i = 0; i < count; ++i) if (strcmp (cnet.inst_name, names[i]) == 0) { ++done; break; }
		}
		if (!done) dmPutDesignData (fp2, CIR_NET_ATOM);
	    }
	    last = dmTell (fp);
	}
    }

    dmCloseStream (fp, QUIT);
    if (fp2) dmCloseStream (fp2, COMPLETE);
}

void readc_term (char *file)
{
    register long i;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;

    cterm.term_attribute = attribute_string;
    cterm.term_lower = lower;
    cterm.term_upper = upper;

    while (dmGetDesignData (fp, CIR_TERM) > 0) {
	printf ("term:\"%s\" attr:\"%s\" dim:%ld\n",
	    cterm.term_name, cterm.term_attribute, cterm.term_dim);
	for (i = 0; i < cterm.term_dim; ++i) {
	    printf ("lower,upper[%ld]: %ld, %ld\n", i, cterm.term_lower[i], cterm.term_upper[i]);
	}
    }
    dmCloseStream (fp, QUIT);
}

void header (char *file)
{
    printf ("=> %s/%s/%s\n", view, cell, file);
}

#ifdef DO_SIG
void sig_handler (int sig) /* signal handler */
{
    signal (sig, SIG_IGN); /* ignore signal */
    PE "\n");
    error (0, "");
}
#endif

void dmError (char *s)
{
    if (dmerrno == DME_FOPEN) {
	printf ("%s: %s ==== cannot open ====\n", argv0, s);
	return;
    }
    PE "%s: ", argv0);
    dmPerror (s);
    error (1, "");
}

char *errlist[] = {
/* 0 */  "received interrupt signal",
/* 1 */  "error in DMI function",
/* 2 */  "unknown stream: %s",
/* 3 */  "%s: read error",
/* 4 */  "unknown error"
};

void error (int nr, char *s)
{
    if (nr < 0 || nr > 4) nr = 4;
    PE "%s: ", argv0);
    PE errlist[nr], s);
    PE "\n%s: -- program aborted --\n", argv0);
    dmQuit ();
    exit (1);
}
