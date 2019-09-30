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

#include <unistd.h>		// prototypes getopt()
#include <stdlib.h>		// prototypes exit()
#include <iostream>
using namespace std;
#include <string.h>		// prototypes strcmp()
#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/libseadif/sdferrors.h"
#include "src/ocean/libocean/sdfNameIter.h"
#include "src/ocean/libocean/format.h"
#include "src/ocean/seedif/thedate.h"

static char *basename_l (char *path); // directory name of a path
static void printHelp (char *progName); // print help and exit
static int  printTheBloodyObjects (sdfNameIterator& seadifName, int printWithParenthesis);
static int  askForPermission (sdfNameType theSeadifObjectName, int count);
static int  finallyRemoveTheBloodyObjects (sdfNameIterator& seadifName);
static void printHeader (sdfNameIterator& theName, STRING bform,
			STRING fform, STRING cform, STRING lform);

static int removeSeadifObjects;

int main (int argc, char *argv[])
{
   const char *optionstring;
   char *cname = NULL, *fname = NULL, *lname = NULL, *bname = NULL, *argv0;
   extern char *optarg;
   extern int  optind;
   int  option, printWithParenthesis = 0, wholeWords = TRUE;
   int  removeSilently = 0;

   // extern int sdfmakelockfiles; sdfmakelockfiles = 0; // for debugging

   argv0 = basename_l (argv[0]);

   // if removeSeadifObjects is TRUE, then we REMOVE the named seadif objects
   // from the database in stead of just listing them... :
   removeSeadifObjects = (strcmp (argv0, "freedif") == 0);

   if (removeSeadifObjects)
      optionstring = "hpwb:f:c:l:BFCLs";
   else
      optionstring = "hpwb:f:c:l:BFCL";

   while ((option = getopt (argc, argv, optionstring)) != EOF)
   {
      switch (option)
      {
      case 'h':			// print help
	 printHelp (argv0);
	 exit (0);
      case 'l':			// layout name
	 lname = cs (optarg);
	 break;
      case 'c':			// circuit name
	 cname = cs (optarg);
	 break;
      case 'f':			// function name
	 fname = cs (optarg);
	 break;
      case 'b':			// library name
	 bname = cs (optarg);
	 break;
      case 'L':			// layout wildcard
	 lname = cs ((char*)".*");
	 break;
      case 'C':			// circuit wildcard
	 cname = cs ((char*)".*");
	 break;
      case 'F':			// function wildcard
	 fname = cs ((char*)".*");
	 break;
      case 'B':			// library wildcard
	 bname = cs ((char*)".*");
	 break;
      case 'p':			// print in parenthesized format
	 printWithParenthesis = TRUE;
	 break;
      case 'w':			// do not embed regexp between ^ and $
	 wholeWords = 0;
	 break;
      case 's':			// freedif option: remove silently
	 removeSilently = TRUE;
	 break;
      case '?':			// illegal option
      default:
	 cout << "\nuse option -h for help\n"; // getopt() already prints message
	 exit (1);
      }
   }

   if (!cname && lname) cname = cs (lname); // default circuit name is layout name

   if (!fname && cname) fname = cs (cname); // default function name is the circuit name

   if (!bname) {
     if (!fname && !cname && !lname) bname = cs ((char*)".*");
     else {
	if (!(bname = sdfgetcwd ())) {
	   cout << "Cannot get the current working directory...\n" << flush;
	   exit (SDFERROR_SEADIF);
	}
	bname = basename_l (bname); // default lib name is directory name
	if (!*bname) {
	   cout << "No name, root cannot be the project directory...\n" << flush;
	   exit (SDFERROR_SEADIF);
	}
	bname = cs (bname);
     }
   }

   // open the seadif database
   int errstatus;
   if ((errstatus = sdfopen()) != SDF_NOERROR) {
      if (errstatus != SDFERROR_FILELOCK)
	 cout << "I cannot open the seadif database.\n" << flush;
      sdfexit (errstatus);
   }

   // create an iterator that returns the matching seadif objects ...:
    sdfNameIterator seadifName ((const char *)bname,
				(const char *)fname,
				(const char *)cname,
				(const char *)lname, wholeWords);

   // check that there is at least one seadif object:
   if (!seadifName.more()) {
      cerr << "No seadif " << seadifName.nameTypeString() << " matches that name ...\n" << flush;
      sdfexit (1);
   }

   int numberOfObjects = 0;

   // print the seadif goodies on standard output:
   if (!(removeSeadifObjects && removeSilently))
      numberOfObjects = printTheBloodyObjects (seadifName, printWithParenthesis);

   // maybe delete \'m:
   if (removeSeadifObjects && (numberOfObjects > 0 || removeSilently))
      if (removeSilently || askForPermission (seadifName.nameType(), numberOfObjects))
      {
	 // re-initialize the name iterator ...
	 seadifName.initialize ((const char *)bname,
				(const char *)fname,
				(const char *)cname,
				(const char *)lname, wholeWords);
	 // ... and stuff it!
	 int count = finallyRemoveTheBloodyObjects (seadifName);
	 if (!removeSilently)
	    cout << count << " " << seadifName.nameTypeString (count != 1)
		 << " removed\n" << flush;
      }

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

static void printHelp (char *progname)
{
   cout << progname << ", compiled " << thedate << "\n\n";
   if (removeSeadifObjects)
      cout << "Remove objects from the seadif database\n\n";
   else
      cout << "List the contents of the seadif database\n\n";
   cout
      << "usage: " << progname << " [...options...]\n"
      << "options:\n"
      << "  -l <layoutname>     eregexp layout name\n"
      << "  -c <circuitname>    eregexp circuit name (default: <layoutname>)\n"
      << "  -f <functionname>   eregexp function name (default: <circuitname>)\n"
      << "  -b <libraryname>    eregexp library name (default: current working\n"
      << "                      directory name if any of <layoutname>, <circuitname>\n"
      << "                      or <functionname> is specified, else it is \".*\")\n"
      << "  -L                  equivalent to \"-l .*\", that is: all layouts\n"
      << "  -C                  equivalent to \"-c .*\", that is: all circuits\n"
      << "  -F                  equivalent to \"-f .*\", that is: all functions\n"
      << "  -B                  equivalent to \"-b .*\", that is: all libraries\n"
      << "  -p                  print seadif objects in parenthesized format\n"
      << "  -w                  do not embed each eregexp within ^ and $\n";
   if (removeSeadifObjects)
      cout << "  -s                  remove silently, do not ask for confirmation\n";
   cout << "  -h                  print this help screen, then exit\n\n";
}

static int printTheBloodyObjects (sdfNameIterator& seadifName, int printWithParenthesis)
{
   STRING bform = (char*)"%-15s\n";
   STRING fform = (char*)"%-15s %-15s\n";
   STRING cform = (char*)"%-15s %-15s %-15s\n";
   STRING lform = (char*)"%-15s %-15s %-15s %-15s\n";
   int count = 0;

   while (seadifName())
   {
      count += 1;
      STRING ln = seadifName.lname();
      STRING cn = seadifName.cname();
      STRING fn = seadifName.fname();
      STRING bn = seadifName.bname();
      if (count == 1)
      {
	 if (printWithParenthesis)
	 {
	    bform = (char*)"%s\n";
	    fform = (char*)"%s(%s)\n";
	    cform = (char*)"%s(%s(%s))\n";
	    lform = (char*)"%s(%s(%s(%s)))\n";
	 }
	 else
	    printHeader (seadifName, bform, fform, cform, lform);
      }
      switch (seadifName.nameType())
      {
      case SeadifLibraryName:
	 cout << form (bform, bn);
	 break;
      case SeadifFunctionName:
	 cout << form (fform, fn, bn);
	 break;
      case SeadifCircuitName:
	 cout << form (cform, cn, fn, bn);
	 break;
      case SeadifLayoutName:
	 cout << form (lform, ln, cn, fn, bn);
	 break;
      case SeadifNoName:
      default:
	 cerr << "PANIC 738456\n" << flush;
	 break;			// should not happen
      }
   }
   return count;
}

static void printHeader (sdfNameIterator& theName, STRING bform,
			STRING fform, STRING cform, STRING lform)
{
   switch (theName.nameType())
   {
   case SeadifLibraryName:
      cout << form (bform, "library")
	   << "---------------\n";
      break;
   case SeadifFunctionName:
      cout << form (fform, "function", "library")
	   << "-------------------------------\n";
      break;
   case SeadifCircuitName:
      cout << form (cform, "circuit", "function", "library")
	   << "-----------------------------------------------\n";
      break;
   case SeadifLayoutName:
      cout << form (lform, "layout", "circuit", "function", "library")
	   << "---------------------------------------------------------------\n";
      break;
   case SeadifNoName:
   default:
      cerr << "PANIC 375486\n" << flush;
      break;			// should not happen
   }
}

// Ask the user for permission to remove the Seadif objects. Return TRUE if
// permission is granted, 0 otherwise.
static int askForPermission (sdfNameType theSeadifObjectName, int count)
{
   if (count > 1)
      cout << "\nRemove all these " << sdfNameTypeString (theSeadifObjectName, TRUE);
   else
      cout << "\nRemove this " << sdfNameTypeString (theSeadifObjectName, 0);
   cout << " ? (y/n) ";
   char response[100];
   cin >> response;
   if (strcmp (response, "y") == 0) return TRUE;
   cout << "\nNothing removed...\n";
   return 0;
}

static int finallyRemoveTheBloodyObjects (sdfNameIterator& seadifName)
{
   int count = 0;
   while (seadifName())
      if (seadifName.sdfremove()) count += 1;
   return count;
}
