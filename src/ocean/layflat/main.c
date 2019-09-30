/*
 * ISC License
 *
 * Copyright (C) 2000-2018 by
 *	Simon de Graaf
 *	Kees-Jan van der Kolk
 *	Patrick Groeneveld
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
#include <unistd.h>
#include "src/libddm/dmincl.h"
#include "src/ocean/layflat/layflat.h"
#include "src/ocean/layflat/prototypes.h"

extern int   optind;
extern char *optarg;

int liftsubterm = 0;
char *argv0 = "layflat";
char *FilterLib;

static DM_PROCDATA *processinfo;

static int  mask_atoi (char *mask);
static void printhelp (char *progname);

/* masklist[j] is 1 if mask j is to be included and 0 if
 * mask j is to be excluded from the flattened output cell */
int masklist[DM_MAXNOMASKS];

int main (int argc, char *argv[])
{
    char *oldcellname = NULL;
    char  newcellname[DM_MAXNAME+1];
    char  filtermc[100]; /* PATRICK: name of dir to be filtered */
    int	  c, j, n_ilist = 0, n_xlist = 0, opterr = 0;
    DM_PROJECT *projectkey;

    strcpy (newcellname, "testflat");
    strcpy (filtermc, "/celllibs/");
    FilterLib = NULL;

    dmInit (argv0);

    if (!(projectkey = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE))) {
	fprintf (stderr, "Cannot open project\n");
	exit (1);
    }

    if (!(processinfo = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, projectkey))) {
	fprintf (stderr, "Cannot read process data\n");
	exit (1);
    }

    /* default: include all masks in the output, except masks specified with -x */
    for (j = 0; j < processinfo->nomasks; ++j) masklist[j] = 1;

    while ((c = getopt (argc, argv, "hlL:so:i:x:")) != EOF) {
    switch (c) {
      case 'h':  /* print help */
	printhelp (argv0);
	exit (0);
      case 'l':  /* filter out libraries */
	FilterLib = &filtermc[0];
	break ;
      case 'L':  /* filter lib */
	if (strlen (optarg) > 99) {
	    fprintf (stderr, "ERROR: too long library name\n");
	    opterr = 1;
	    break ;
	}
	strcpy (filtermc, optarg);
	FilterLib = &filtermc[0];
	break ;
      case 's': /* lift all subterminals as well */
	liftsubterm = 1;
	break ;
      case 'o': /* specifies name of outputcell */
	strcpy (newcellname, optarg);
	break;
      case 'i': /* specifies a layer to include */
	if (n_xlist) { /* also specified -x option ... */
	    fprintf (stderr, "ERROR: the options -i and -x are mutually exclusive...\n");
	    printhelp (argv0);
	    exit (1);
	}
	if (!n_ilist) {
	    /* include no masks in the output except masks specified with -i */
	    for (j = 0; j < processinfo->nomasks; ++j) masklist[j] = 0;
	}
	n_ilist = 1;
	if ((j = mask_atoi (optarg)) >= 0) masklist[j] = 1;
	else {
	    fprintf (stderr, "ERROR: design mask \"%s\" is not defined for %s\n", optarg, processinfo->pr_name);
	    opterr = 1;
	}
	break;
      case 'x':  /* specifies a layer to exclude */
	if (n_ilist) { /* also specified -i option ... */
	    fprintf (stderr, "ERROR: the options -i and -x are mutually exclusive...\n");
	    printhelp (argv0);
	    exit (1);
	}
	n_xlist = 1;
	if ((j = mask_atoi (optarg)) >= 0) masklist[j] = 0;
	else {
	    fprintf (stderr, "ERROR: design mask \"%s\" is not defined for %s\n", optarg, processinfo->pr_name);
	    opterr = 1;
	}
	break;
      case '?':
	opterr = 2;
      }
    }

    if (optind >= argc) {
	fprintf (stderr, "ERROR: you must specify a cell name!\n");
	printhelp (argv0);
	exit (1);
    }
    oldcellname = argv[optind++];

    if (opterr == 2 || optind < argc) {
	if (optind < argc) fprintf (stderr, "ERROR: use only one argument!\n");
	printhelp (argv0);
	exit (1);
    }

    if (opterr) exit (1);

    if (FilterLib) {
	printf ("%s: Not flattening cells of which the library path contains '%s'\n", argv0, FilterLib);
    }

    printf ("%s: flat layout goes into cell '%s'\n", argv0, newcellname);

    flatten_mc (newcellname, oldcellname, projectkey);

    dmCloseProject (projectkey, COMPLETE);
    dmQuit ();
    return (0);
}

static void printhelp (char *progname)
{
    extern char *thedate;

printf("\nThis is %s\nversion %s, compiled %s\n\n", progname, "1.12", thedate);
printf("usage: %s [...options...] oldcellname\n", progname);
printf("options:\n"
       "       -o <name>  Name of cell to be created: default \"testflat\".\n"
       "       -s         Lift all the subterminals to the top level as well\n"
       "                  (default: only the top level terminals are included).\n"
       "       -x <mask>  Exclude all occurrences of <mask> from the output.\n"
       "                  Multiple -x options are allowed.\n"
       "       -i <mask>  Only include <mask> in the output. Multiple -i options\n"
       "                  are allowed, but -x and -i are mutually exclusive.\n"
       "       -L <pat>   Exclude library cells that are imported from a project\n"
       "                  whose path contains the string <pat>.\n"
       "       -l         Equivalent to the option \"-L /celllibs/\".\n"
       "       -h         Print this help screen and exit.\n\n");
}

/* convert the ascii string MASK to its equivalent integer mask number */
static int mask_atoi (char *mask)
{
    int j;
    for (j = 0; j < processinfo->nomasks; ++j)
	if (strcmp (mask, processinfo->mask_name[j]) == 0) return j;
    return -1; /* mask does not exist for this process */
}
