/*
 * ISC License
 *
 * Copyright (C) 1992-2018 by
 *	Paul Stravers
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

#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/libocean/sdfNameIter.h"
#include "src/ocean/libocean/format.h"
#include <iostream>
using namespace std;
#include <string.h>
#include "src/ocean/gnarp/prototypes.h"

static void addOrRemoveStatusString (STATUSPTR stat, actionType add_or_rm, char *statStr);
static STATUSPTR readStatus (sdfNameIterator& seadifName);
static void writeStatus (sdfNameIterator& seadifName);
static void showStatusLibprim (sdfNameIterator& seadifName);
static void printResult (sdfNameIterator& seadifName);

enum { BeforeAction = 0, AfterAction = 1 }; // used as index for numberOfStatStrings[]
static int numberOfStatStrings[2];

// For each circuit returned by the sdfNameIterator add or remove the string
// statStr to/from the status->program field ...
void modifyStatus (sdfNameIterator& seadifName, actionType add_or_rm,
			char *statStr, sdfNameType obligType)
{
   if (obligType != SeadifNoName && obligType != seadifName.nameType()) {
      cerr << "Must specify a " << sdfNameTypeString (obligType)
	   <<" for this operation ...\n";
      return;
   }

   while (seadifName())
   {
      STATUSPTR status = readStatus (seadifName);
      if (add_or_rm == actionShow)
	 showStatusLibprim (seadifName);
      else {
	 addOrRemoveStatusString (status, add_or_rm, statStr);
	 printResult (seadifName);
	 writeStatus (seadifName);
      }
   }
}

// return a pointer to the STATUS structure of the current seadif object:
static STATUSPTR readStatus (sdfNameIterator& seadifName)
{
   STATUSPTR status = NULL;
   seadifName.sdfread (SDFLIBSTAT | SDFFUNSTAT | SDFCIRSTAT | SDFLAYSTAT, 1);
   switch (seadifName.nameType())
   {
   case SeadifLibraryName:
      status = seadifName.thislib()->status;
      break;
   case SeadifFunctionName:
      status = seadifName.thisfun()->status;
      break;
   case SeadifCircuitName:
      status = seadifName.thiscir()->status;
      break;
   case SeadifLayoutName:
      status = seadifName.thislay()->status;
      break;
   case SeadifNoName:
   default:
      break;
   }
   return status;
}

// write the STATUS field of the curren tSeadif object:
static void writeStatus (sdfNameIterator& seadifName)
{
   seadifName.sdfwrite (SDFLIBSTAT | SDFFUNSTAT | SDFCIRSTAT | SDFLAYSTAT, 1);
}

// Add or remove statStr to/from the stat->program string:
static void addOrRemoveStatusString (STATUSPTR stat, actionType add_or_rm,
				    char *statStr)
{
   const int MaxStat = 300;
   char tmpstr[MaxStat+1], *startOfStr = NULL;

   if (add_or_rm == actionAdd) // action is add
   {
      numberOfStatStrings[BeforeAction] = 0;
      if (!stat) NewStatus (stat); // create a new status struct
      if (!stat->program)
	 stat->program = cs (statStr); // stat->program empty: easy job!
      else if (!(startOfStr = strstr (stat->program, statStr)))
      {  // append statStr with a comma to the end of stat->program:
	 char *s = stat->program;
	 strncpy (tmpstr, s, MaxStat);
	 if (strlen (s) > 0) strncat (tmpstr, ",", MaxStat);
	 strncat (tmpstr, statStr, MaxStat);
	 stat->program = cs (tmpstr);
      }
      else
	 numberOfStatStrings[BeforeAction] = 1; // already in stat->program
      numberOfStatStrings[AfterAction] = 1;
   }
   else // action is remove
   {
      numberOfStatStrings[BeforeAction] = 0;

      if (stat && stat->program && (startOfStr = strstr (stat->program, statStr)))
      {  // stat->program exists and contains statStr. Remove it:
	 numberOfStatStrings[BeforeAction] = 1; // remember that we had one
	 int lengthOfStr = strlen (statStr);
	 if (startOfStr > stat->program && *(startOfStr-1) == ',')
	 {  // also remove the preceeding comma:
	    startOfStr -= 1;
	    lengthOfStr += 1;
	 }
	 int j = 0;
         char *p;
	 for (p = stat->program; p<startOfStr; ++p)
	    tmpstr[j++] = *p; // copy everything before statStr
	 for (p += lengthOfStr; *p; ++p)
	    tmpstr[j++] = *p; // copy everything after statStr
	 tmpstr[j] = 0;
	 fs (stat->program); // forget about the old stat->program
	 stat->program = cs (tmpstr); // insert new stat->program;
      }
      numberOfStatStrings[AfterAction] = 0;
   }
}

static void printResult (sdfNameIterator& seadifName)
{
   printTheBloodyObject (seadifName, 0, (char*)"prev curr",
   /* form() returns a ptr to a static area, have to copy it! */
	cs (form ((char*)"%2d   %2d", numberOfStatStrings[BeforeAction], numberOfStatStrings[AfterAction])));
}

static void showStatusLibprim (sdfNameIterator& seadifName)
{
   STATUSPTR stat = seadifName.thiscir()->status;
   int isLibprim = stat && stat->program && strstr (stat->program, "libprim");
   /* form() returns a ptr to a static area, have to copy it! */
   printTheBloodyObject (seadifName, 0, (char*)"libprim", cs (form ((char*)"%3d", isLibprim)));
}
