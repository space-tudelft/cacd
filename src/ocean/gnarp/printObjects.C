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

#include <string.h>
#include <iostream>
using namespace std;
#include "src/ocean/libocean/sdfNameIter.h"
#include "src/ocean/libocean/format.h"
#include "src/ocean/gnarp/prototypes.h"

static void printHeader (sdfNameIterator& theName, STRING bform, STRING fform,
			STRING cform, STRING lform, STRING extra);
static char *makeLine (int lenght);

int printTheBloodyObject (sdfNameIterator& nextName, int printWithParenthesis,
			char *extraHeader, char *extra)
{
   if (operateSilently) return 0;

   STRING bform = (char*)"%-15s%s\n";
   STRING fform = (char*)"%-15s %-15s%s\n";
   STRING cform = (char*)"%-15s %-15s %-15s%s\n";
   STRING lform = (char*)"%-15s %-15s %-15s %-15s%s\n";

   if (printWithParenthesis)
   {
      bform = (char*)"%s\t\t%s\n";
      fform = (char*)"%s(%s)\t\t%s\n";
      cform = (char*)"%s(%s(%s))\t\t%s\n";
      lform = (char*)"%s(%s(%s(%s)))\t\t%s\n";
   }

   STRING ln = nextName.lname();
   STRING cn = nextName.cname();
   STRING fn = nextName.fname();
   STRING bn = nextName.bname();

   static int count = 0;
   count += 1;
   if (count == 1 && !printWithParenthesis)
      printHeader (nextName, bform, fform, cform, lform, extraHeader);

   switch (nextName.nameType())
   {
   case SeadifLibraryName:
      cout << form (bform, bn, extra);
      break;
   case SeadifFunctionName:
      cout << form (fform, fn, bn, extra);
      break;
   case SeadifCircuitName:
      cout << form (cform, cn, fn, bn, extra);
      break;
   case SeadifLayoutName:
      cout << form (lform, ln, cn, fn, bn, extra);
      break;
   case SeadifNoName:
      cerr << "PANIC 378546\n" << flush;
      break;			// should not happen
   }
   return count;
}

static void printHeader (sdfNameIterator& theName, STRING bform,
			STRING fform, STRING cform, STRING lform, STRING extra)
{
   switch (theName.nameType())
   {
   case SeadifLibraryName:
      cout << form (bform, "library", extra)
	   << makeLine (15 + strlen(extra));
      break;
   case SeadifFunctionName:
      cout << form (fform, "function", "library", extra)
	   << makeLine (31 + strlen(extra));
      break;
   case SeadifCircuitName:
      cout << form (cform, "circuit", "function", "library", extra)
	   << makeLine (47 + strlen(extra));
      break;
   case SeadifLayoutName:
      cout << form (lform, "layout", "circuit", "function", "library", extra)
	   << makeLine (63 + strlen(extra));
      break;
   case SeadifNoName:
      cerr << "PANIC 375486\n" << flush; // should not happen
   }
}

// return a character string consisting of LENGTH dashes ...
static char *makeLine (int length)
{
   static char line[300 + 1];
   int j = 0;

   if (length > 300) length = 300;

   while (j < length) line[j++] = '-';
   line[j++] = '\n';
   line[j] = 0;
   return line;
}
