/*
 * ISC License
 *
 * Copyright (C) 1993-2018 by
 *	Viorica Simion
 *	Patrick Groeneveld
 *	Simon de Graaf
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
#include <stdlib.h>
#include <iostream>
using namespace std;
#include <string.h>
#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/libseadif/sdferrors.h"
#include "src/ocean/libocean/sdfNameIter.h"
#include "src/ocean/colaps/thedate.h"
#include "src/ocean/colaps/colapshead.h"

static char *bname (char *path); // directory base name of a path
static void printHelp (char *progName, int exitstat = 1); // print help and exit

int main (int argc, char *argv[])
{
   char            *cname = NULL, *fname = NULL, *pname = NULL,
                   *infile = NULL, *outfile = NULL, *outcircuit = NULL,
                   *firstArg, *secondArg, *argv0;
   char            buf1[50], buf2[50];
   int             option;
   extern char     *optarg;
   extern int      optind;
   int             wholeWords = TRUE;
   int             toCellLib = 0;
   extern long     netCounter;
   extern int      verbose;
   extern int      doNotMakePathList;

   firstArg = buf1;
   secondArg = buf2;

   netCounter = 0;
   verbose = TRUE;
   doNotMakePathList = TRUE;

   argv0 = bname (argv[0]);

   while ((option = getopt (argc, argv, "cf:hk:lo:q")) != EOF)
   {
      switch (option)
      {
      case 'h':			// print help
	 printHelp (argv0, 0);
	 exit (0);
	 break;
      case 'l':
	 doNotMakePathList = 0;
	 break;
      case 'q':			// do not embed regexp between ^ and $
	 verbose = 0;
	 break;
      case 'c':
	 toCellLib = TRUE;
	 break;
      case '?':			// illegal option
	 cout << "\nuse option -h for help\n";//getopt() already prints message
	 exit (1);
	 break;
      case 'k':
	 infile = cs (optarg);
	 if (!infile || strlen (infile) == 0)
	    sdfreport (Fatal, "This program needs a <keepfile> name from you!");
	 break;
      case 'f':
	 outfile = cs (optarg);
	 if (!outfile || strlen (outfile) == 0)
	    sdfreport (Fatal, "This program needs a <trackfile> name from you!");
	 break;
      case 'o':
	 outcircuit = cs (optarg);
	 if (!outcircuit || strlen (outcircuit) == 0)
	    sdfreport (Fatal, "This program needs a <outputcell> name from you!");
	 break;
      default:
	 break;
      }
   }

   if (outcircuit && (strlen (outcircuit) > 14))
      fprintf (stderr, "WARNING: output cell name too long!\n");
/*
 * read arguments
 */
   if (optind <= argc - 1)
   {
      //copy first argument
      strcpy (firstArg, argv[optind]);
      optind += 1;
      if (optind <= argc - 1)
      {
	 // copy second argument
	 strcpy (secondArg, argv[optind]);
	 pname = cs (firstArg);
	 cname = cs (secondArg);
      }
      else
	 cname = cs (firstArg);
   }

   if (!cname || strlen (cname) == 0)
	sdfreport (Fatal, "This program needs a circuit name from you!");

   if (optind < argc - 1)
	fprintf (stderr, "WARNING: anything after argument \'%s\' was ignored.\n"
              "(use only two arguments)\n", argv[optind]);

   if (!fname && cname)
      fname = cs (cname); // default function name is the circuit name

   if (!pname)
   {
      if (!fname && !cname) pname = cs ((char*)".*");
      else {
	 pname = sdfgetcwd (); // get current working directory
	 if (!pname) sdfreport (Fatal, "Cannot get the current working directory...");
	 pname = bname (pname); // default lib name is directory name
	 if (!*pname) sdfreport (Fatal, "No name, root cannot be project directory...");
	 pname = cs (pname);
      }
   }

   // open the seadif database
   int errstatus;
   if ((errstatus = sdfopen()) != SDF_NOERROR)
   {
      if (errstatus != SDFERROR_FILELOCK)
	 sdfreport (Fatal, "I cannot open the seadif database.");
   }

   // create an iterator that returns the matching seadif objects ...:
   sdfNameIterator seadifName (pname, fname, cname, NULL, wholeWords);

   // quit this thing if the user did not give us a circuit:
   if (seadifName.nameType() != SeadifCircuitName)
   {
      sdfreport (Fatal, "This program needs a circuit name from you!");
   }

   cout << "------ opening seadif ------" << "\n";

   // check that there is at least one seadif object:
   if (!seadifName.more())
   {
      sdfreport (Fatal, "No seadif %s matches %s(%s(%s)) ...",
		seadifName.nameTypeString(), cname, fname, pname);
   }

   CIRCUITPTR localCir;
   LIBRARYPTR liblocal;
   NonFlatPtr firstNon;

   while (seadifName())
   {
      firstNon = readNonFlat (infile);
      // Now read this thing!
      cout << "------ reading circuit " << seadifName.sdfName() << " ------\n";
      seadifName.sdfreadall (SDFCIRALL);
      liblocal = findLocalLib();
      if (seadifName.bname() == liblocal->name && !outcircuit)
	 sdfreport (Fatal, "If you want me to overwrite circuit \"%s\" in\n"
		   "the current project, use option \"-o %s\"...", cname, cname);
      // ...and now call our super-hack function !!
      cout << "------ starting colaps ------" << "\n";
      colaps (seadifName.thiscir(), firstNon, toCellLib, outfile, outcircuit);
      localCir = makeLocalCopy (seadifName.thiscir(), outcircuit);
      cout << "------ writing circuit " << localCir->name
	   << "(" << localCir->function->name
	   << "(" << localCir->function->library->name
	   << ")) ------\n";
      sdfwritecir (SDFCIRALL, localCir);
   }

   sdfclose();			// update and close the seadif database
   cout << "------ closing seadif and exit ------" << "\n";
   exit (0);
   return 0;
}

static char *bname (char *path)
{
   char *p = strrchr (path, '/');
   if (p) return p+1;
   return path;
}

static void printHelp (char *progname, int exitstat)
{
   cout << progname << ", compiled " << thedate << "\n\n";
   cout
      << "usage: " << progname << " [...options...] [project] <inputcell>\n\n"
      << "  project             Name of the library project, if not specified\n"
      << "                      local library name is taken\n"
      << "  <inputcell>         Name of the cell of the library project\n\n"
      << "options:\n"
      << "\n"
      << "  -k <keepfile>       Input file name containing a list with\n"
      << "                      circuits and/or instances to be kept\n"
      << "  -c                  Collapse everything as far as the\n"
      << "                      primary cell level\n"
      << "  -l                  Keep track of the flattened paths\n"
      << "  -f <trackfile>      Output file name containing a list of\n"
      << "                      flattened paths and their instances\n"
      << "  -o <outputcell>     Name of the flattened circuit, if not\n"
      << "                      specified remote name is taken\n"
      << "  -q                  Do not print the state of the program\n"
      << "  -h                  Print this help screen, then exit\n\n";
   exit (exitstat);
}
