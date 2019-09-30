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

// change the name of instances and internal terminals
void cleanUp (CIRCUITPTR sdfcircuit, NonFlatPtr firstNonFlat)
{
   CIRINSTPTR    nextInst;
   CIRPORTPTR    firstCirport;
   NETPTR        nextNet, lastNet;
   ShortFlagPtr  circuitFlag;
   NonFlatPtr    nextNonFlat = firstNonFlat;
   char          *nameFix;
   char          *nameVar;
   char          *newName;
   int           checker = 1;

   long counter = 1;
   circuitFlag = (ShortFlagPtr)sdfcircuit->flag.p;

   nextInst = sdfcircuit->cirinst;
   firstCirport = sdfcircuit->cirport;
   lastNet = circuitFlag->net;
   nextNet = lastNet->next;

   nameFix = (char*)"inst";

   while (nextInst)
   {
      if (checkInstName (nextInst->name, firstNonFlat) == MAKEIT)
      {
	 while (checker)
	 {
	    nameVar = long2ascii (counter);
	    newName = concatEnd (nameFix, nameVar);
	    if (checkInstName (newName, firstNonFlat) == MAKEIT)
	    {
	       nextInst->name = NULL;
	       nextInst->name = newName;
	       counter += 1;
	       checker = 0;
	    }
	    else
	       counter += 1;
	 }
	 checker = 1;
      }
      nextInst = nextInst->next;
   }

   counter = 1;
   nameFix = (char*)"net";
   checker = 1;

   while (nextNet)
   {
      if (checkNameSf (nextNet->name, lastNet) == MAKEIT)
      {
	 while (checker)
	 {
	    nameVar = long2ascii (counter);
	    newName = concatEnd (nameFix, nameVar);
	    if (checkNameSf (newName, lastNet) == MAKEIT)
	    {
	       nextNet->name = NULL;
	       nextNet->name = newName;
	       counter += 1;
	       checker = 0;
	    }
	    else
	       counter += 1;
	 }
	 checker = 1;
      }
      nextNet = nextNet->next;
   }
}

int checkInstName (STRING name, NonFlatPtr firstNon)
{
   NonFlatPtr nextNonFlat;

   nextNonFlat = firstNon;

   while (nextNonFlat)
   {
      if (nextNonFlat->inst)
      {
	 if (nextNonFlat->inst == name)
	    return ITEXISTS;
      }
      nextNonFlat = nextNonFlat->next;
   }
   return MAKEIT;
}

// checks if a net name is the same with the cirport name
int checkNameSf (STRING newName, NETPTR lastNet)
{
   NETPTR nextNet = lastNet->circuit->netlist;

   while (nextNet && (nextNet != lastNet->next))
   {
      if (newName == nextNet->name)
	 return ITEXISTS;
      nextNet = nextNet->next;
   }
   return MAKEIT;
}

char *concatEnd (char* fatherName, char* childName)
{
   char *newName = new char[14];

   strncpy (newName, fatherName, 5);
   strncat (newName, childName, 8);
   newName = cs (newName);
   return newName;
}
