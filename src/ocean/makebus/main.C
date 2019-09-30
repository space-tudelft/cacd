/*
 * ISC License
 *
 * Copyright (C) 1992-2018 by
 *	Paul Stravers
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
#include "src/ocean/makebus/prototypes.h"
#include "src/ocean/makebus/globals.h"

static int progShowbus = 0;

static char *basename_l (char *path); // directory name of a path
static void printHelp (char *progName, int exitstat); // print help and exit
static char *escesc (char *str);

int main (int argc, char *argv[])
{
   const char *optionstring;
   char *cname = NULL, *fname = NULL, *lname = NULL, *argv0;
   extern char *optarg;
   extern int  optind;
   int option;
   int showTheBuses = TRUE, showTheNets = 0, removeTheBuses = TRUE, makeNewBuses = TRUE;

   extern int sdfmakelockfiles; sdfmakelockfiles = 0; // for debugging

   argv0 = basename_l (argv[0]);

   // if this program is called "showbus" than only show buses:
   progShowbus = (strcmp (argv0, "showbus") == 0);

   if (progShowbus)
      optionstring = "l:f:nh";	         // showbus options
   else
      optionstring = "l:f:Rsnhe:";       // makebus options

   while ((option = getopt (argc, argv, optionstring)) != EOF)
   {
      switch (option)
      {
      case 'h':			// print help
	 printHelp (argv0, 0);
	 break;
      case 'f':			// function name
	 fname = cs (optarg);
	 break;
      case 'l':			// library name
	 lname = cs (optarg);
	 break;
      case 'R':			// (makebus only) only remove buses
	 makeNewBuses = 0;
	 showTheBuses = 0;
	 break;
      case 's':			// (makebus only) do not show buses
	 showTheBuses = 0;
	 break;
      case 'n':			// also print the nets when printing buses
	 showTheNets = TRUE;
	 break;
      case 'e':			// set regular expression
	 if (strlen (optarg) > MAXPATTERN)
	    cout << "WARNING -- argument to -e option too long (ignored)\n";
	 else
	    strncpy (reArrayPattern, optarg, MAXPATTERN+1);
	 break;
      case '?':			// illegal option
      default:
	 cout << "\n";
	 printHelp (argv0, 1); // getopt() already prints message
	 break;
      }
   }

   if (optind == argc - 1)	// circuit name is specified as last argument
      cname = cs (argv[optind]);
   else {
      cout << "please specify a circuit name ..."
	   << " (option -h for help)\n" << flush;
      exit (SDFERROR_SEADIF);
   }

   if (!fname) fname = cs (cname); // default function name is the circuit name

   if (!lname) {
      if (!(lname = sdfgetcwd())) {
	 cout << "Cannot get the current working directory...\n" << flush;
	 exit (SDFERROR_SEADIF);
      }
      lname = basename_l (lname); // default lib name is directory name
      if (!*lname) {
	 cout << "No name, root cannot be the project directory...\n" << flush;
	 exit (SDFERROR_SEADIF);
      }
      lname = cs (lname);
   }

   // open the seadif database
   int errstatus;
   if ((errstatus = sdfopen()) != SDF_NOERROR)
   {
      if (errstatus != SDFERROR_FILELOCK)
	 cout << "I cannot open the seadif database.\n" << flush;
      sdfexit (errstatus);
   }

   // read the seadif circuit
   if (!sdfreadcir (SDFCIRALL, cname, fname, lname))
      sdfexit (SDFERROR_SEADIF); // sealib already prints a message
   CIRCUITPTR theCircuit = thiscir; // copy global variable into local

   if (!progShowbus)
   {
      int reallyRemovedBuses = 0;
      if (removeTheBuses) {
	 if (theCircuit->buslist) reallyRemovedBuses = 1;
	 sdfdeletebuslist (theCircuit->buslist);
	 theCircuit->buslist = NULL;
      }

      // makebus() returns < 0 on error and
      // > 0 if result can be written to database
      int numberOfNewBuses = 0;
      if (makeNewBuses) numberOfNewBuses = makebus (theCircuit);

      if (numberOfNewBuses > 0 || reallyRemovedBuses)
	 if (!sdfwritecir (SDFCIRSTAT+SDFCIRBUS, theCircuit)) {
	    cout << "\nI cannot write the buses to the database!\n" << flush;
	    sdfexit (SDFERROR_SEADIF);
	 }
   }
   if (showTheBuses) showbus (theCircuit, showTheNets);
   sdfclose();			// update and close the seadif database
   exit (0);
   return 0;
}

static char *basename_l (char *path)
{
    char *p = strrchr (path, '/');
    if (p) return p+1;
    return path;
}

static void printHelp (char *progname, int exitstat)
{
   extern const char *thedate;

   cout << progname << ", compiled " << thedate << "\n\n";

   if (!progShowbus)
      cout
	 << "Append information to a circuit in the seadif database describing\n"
	 << "which nets form buses. Both structural analysis of the network and\n"
	 << "plain guessing based on the net names is performed.\n\n";
   else
      cout
	 << "Show the buses of a circuit. This is essentially a print-out of the\n"
	 << "seadif BusList structure associated with the circuit.\n\n";
   cout
      << "usage: " << progname << " [...options...] <circuitname>\n"
      << "options:\n"
      << "  -f <functionname>   Seadif function name (default: <circuitname>)\n"
      << "  -l <libraryname>    Seadif library name (default: current working\n"
      << "                      directory name)\n";
   if (!progShowbus)
   {
      cout
      << "  -R                  Remove all buses, do not create new ones\n"
      << "  -s                  Silent --do not print which buses were found\n"
      << "  -e <regexp>         Set the Extended Regular Expression (see regexp(5))\n"
      << "                      that matches names of nets that belong to buses.\n"
      << "                      The name of the bus must be the first parenthesized\n"
      << "                      subexpression. The default is \"" << REARRAYPATTERNDEFAULT << "\"\n";
   }
   cout
      << "  -n                  Also print the nets contained in each bus\n"
      << "  -h                  Print this help screen, then exit\n"
      << flush;
   exit (exitstat);
}

// print all occurences of backslash in STR twice ...:
static char *escesc (char *str)
{
   static char escapedStr[MAXPATTERN+1];
   int esc = '\\';
   int c, j = 0;

   for (; (c = *str) && j < MAXPATTERN; ++str) {
      if (c == esc) escapedStr[j++] = esc;
      escapedStr[j++] = c;
   }
   escapedStr[j] = 0;

   return escapedStr;
}
