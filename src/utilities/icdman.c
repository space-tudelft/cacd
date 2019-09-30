/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "src/libddm/dmincl.h"

char   *section = "123453i";
char   *keyword = NULL;
int     kflag = 0;

char   *argv0 = "icdman";	/* Program Name */
char   *use_msg =		/* Command Line */
        "\nUsage: %s [-k | sections] keyword\n\n";

char    manpath[BUFSIZ];
char    keyfile[BUFSIZ];
char    command[BUFSIZ];

int main (int argc, char *argv[])
{
    struct stat statbuf;
    FILE *fp;
    int firsttry, found, i, len, L, U;
    register char *cp, *kp, *sp;
    char *pager;

    if (argc == 3) {
	if (*argv[1] == '-') {
	    if (strcmp (argv[1], "-k") == 0) {
		--argc;
		kflag = 1;
	    }
	}
	else {
	    --argc;
	    section = argv[1];
	}
	keyword = argv[2];
    }
    else
	keyword = argv[1];

    if (argc != 2) {
	fprintf (stderr, use_msg, argv0);
	exit (1);
    }

    dmInit (argv0);
    dmQuit ();

    len = strlen (keyword);

    if (kflag == 1) {
	sprintf (keyfile, "%s/share/man/whatis", icdpath);
	if (!(fp = fopen (keyfile, "r"))) {
	    fprintf (stderr, "%s: cannot open %s\n", argv0, keyfile);
	    exit (1);
	}
	U = *keyword;
	L = tolower (U);
	U = toupper (U);
	while (fgets (command, BUFSIZ, fp)) {
	    for (cp = command; *cp; ++cp)
		if ((*cp == L || *cp == U) && !strncasecmp (keyword, cp, len)) {
		    printf ("%s", command);
		    break;
		}
	}
	fclose (fp);
	exit (0);
    }

    if (!isatty (1)) {
	fprintf (stderr, "%s: output must go to a tty\n", argv0);
	exit (1);
    }

    sprintf (manpath, "%s/share/man/catXX", icdpath);
    cp = manpath + strlen (manpath) - 2;

    sprintf (keyfile, "%s.XX", keyword);
    kp = keyfile + len + 1;

    if (!(pager = getenv ("PAGER"))) {
#if 0
	pager = "/bin/less";
	if (stat (pager, &statbuf) < 0) {
	    pager = "/usr/bin/less";
	    if (stat (pager, &statbuf) < 0)
		pager = "more";
	}
#else
	pager = "more";
#endif
    }

    firsttry = 1;
    found = 0;

search_manpage:
    for (sp = section; *sp; ++sp) {

	*kp = *cp = *sp;
	if (strcmp (sp, "3i") == 0) {
	    *(kp + 1) = 'i'; *(cp + 1) = '\0';
	    sp++;
	}
	else {
	    *(kp + 1) = *(cp + 1) = '\0';
	}

	if (chdir (manpath) == 0 && (fp = fopen (keyfile, "r"))) {
	    fclose (fp);
	    i = _dmRun (pager, "-s", keyfile, (char *) NULL);
	    if (i) {
		fprintf (stderr, "%s: cannot run %s\n", argv0, pager);
		exit (1);
	    }
	    found = 1;
	}
    }

    if (!found) {
	if (firsttry) {
	    for (i = 0; i < len; i++) {
		U = keyfile[i];
		if (isupper (U)) {
		    keyfile[i] = tolower (U);
		    firsttry = 0;
		}
	    }
	    if (!firsttry) goto search_manpage;
	}
	if (len > 10) { /* try also for different len */
	    if (--len > 12) len = 12;
	    kp = keyfile;
	    for (i = 0; i < len; i++) *kp++ = keyword[i];
	    *kp++ = '.';
	    *(kp + 2) = '\0';
	    firsttry = 1;
	    goto search_manpage;
	}
	fprintf (stderr, "\n%s: cannot find manual page for: %s\n\n", argv0, keyword);
	exit (1);
    }

    exit (0);
    return (0);
}
