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

#include "src/ocean/colaps/colapshead.h"

// print the path for the instances
// print only one path for a group of instances which belong
// to the same circuit

void printPath (CIRCUITPTR sdfcircuit, char* outfile, char *outcircuit)
{
   CIRINSTPTR    nextInst, firstInst, lastInst;
   MyFlagPtr     myFlag;
   InstPathPtr   myInstPath, nextPath;
   FILE          *fw;
   char          outputFile[30], *name;
   char          *myCirName;

   name = sdfcircuit->name;

   myCirName = concat (name, (char*)"Flt");

   if (!outfile)
   {
      if (!outcircuit)
	 sprintf (outputFile, "%s.list", myCirName);
      else
	 sprintf (outputFile, "%s.list", outcircuit);
   }
   else
      strcpy (outputFile, outfile);

   cout << "------ writing output file " <<  outputFile << " ------\n";

   fw = fopen (outputFile, "w");

   nextInst = sdfcircuit->cirinst;

   while (!nextInst->flag.p && nextInst)
      nextInst = nextInst->next;

   while (nextInst)
   {
      myFlag = (MyFlagPtr)nextInst->flag.p;
      myInstPath = myFlag->instPath;
      firstInst = nextInst;
      do
      {
	 lastInst = nextInst;
	 nextInst = nextInst->next;
	 myFlag = (MyFlagPtr)nextInst->flag.p;
	 nextPath = myFlag->instPath;
      }
      while (!nextPath && myFlag);

      if (firstInst->name == lastInst->name)
	 fprintf (fw, "%s: ", firstInst->name);
      else
	 fprintf (fw, "%s to %s: ", firstInst->name, lastInst->name);
      nextPath = myInstPath;
      while (nextPath)
      {
	 if (nextPath->next)
	    fprintf (fw, "%s/", nextPath->circuitName);
	 else
	    fprintf (fw, "%s\n", nextPath->circuitName);
	 nextPath = nextPath->next;
      }

      while (!nextInst->flag.p && nextInst)
	 nextInst = nextInst->next;
   }
   nextInst = sdfcircuit->cirinst;
   while (nextInst)
   {
      nextInst->flag.p = NULL;
      nextInst = nextInst->next;
   }
   fclose (fw);
}
