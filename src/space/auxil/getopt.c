/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#include "src/space/auxil/auxil.h"

int	opterr = 1;
int	optind = 1;
int	optopt;
char	*optarg;

/*
 * xgetopt - extended version of getopt.
 * This version of getopt is based on the AT&T public domain
 * source for getopt(3).
 *
 * However, it is modified it to understand a ? for optional optargs.
 * Such optional arguments should be supplied without a space
 * between the option letter and the option argument:
 * Mandatory option arguments still can have the space.
 * Optarg is set (unset) when there is (is no) option argument.
 *
 * E.g. optstring = -ab:c?
 *      prog -a -b2 -c		   no value for c option -> optarg == NULL;
 *      prog -a -b 2 -c3	   value 3 for c option  -> optarg != NULL;
 *      prog -a -b 2 -c 3	   no value for c option -> optarg == NULL;
 *
 * Error messages using say ().
 */
int xgetopt (int argc, char **argv, char *opts)
{
    static int sp = 1;
    register int c;
    register char *cp;

    if (sp == 1) {
	if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
	    return (EOF);
	else if (strsame (argv[optind], "--")) {
	    optind++;
	    return (EOF);
	}
    }

    optopt = c = argv[optind][sp];

    if (c == ':' || c == '?' || (cp = strchr (opts, c)) == NULL) {
	if (opterr)
	    say ("illegal option -- %c", c);
	if (argv[optind][++sp] == '\0') {
	    optind++;
	    sp = 1;
	}
	return ('?');
    }

    if (*++cp == ':') {
	if (argv[optind][sp + 1] != '\0')
	    optarg = &argv[optind++][sp + 1];
	else  if (++optind >= argc) {
	    if (opterr)
		say ("option requires an argument -- %c", c);
	    sp = 1;
	    return ('?');
	} else
	    optarg = argv[optind++];
	sp = 1;
    }
    else if (*cp == '?') {
	if (argv[optind][sp + 1] != '\0')
	    optarg = &argv[optind][sp + 1];
	else {
	    optarg = NULL;
	}
	optind++;
	sp = 1;
    }
    else  {
	if (argv[optind][++sp] == '\0') {
	    sp = 1;
	    optind++;
	}
	optarg = NULL;
    }
    return (c);
}

#ifdef DRIVER

/* test driver and example */
int main (int argc, char *argv[])
{
    int c;
    if (argc == 1) {
	system ("getopt -a");
	system ("getopt -b");
	system ("getopt -b3");
	system ("getopt -c");
	system ("getopt -c3");
    }
    while ((c = xgetopt (argc, argv, "ab:c?")) != EOF) {
	switch (c) {
	    case 'a': printf ("a\n"); break;
	    case 'b': printf ("b %s\n", optarg); break;
	    case 'c': printf ("c %s\n", optarg ? optarg : ""); break;
	    default : printf ("%c\n", c); break;
	}
    }
    return (0);
}
#endif /* DRIVER */
