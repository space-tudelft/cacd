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
#include <stdio.h>		// prototype for sprintf()

// checks for the existing name of nets
int checkName (STRING newName, NETPTR netOrInst, NETPTR lastCurNet)
{
   NETPTR net = netOrInst, lastCheckNet;

   lastCheckNet = lastCurNet->next;

   while (net != lastCheckNet)
   {
      if (net->name == newName) return ITEXISTS;
      net = net->next;
   }
   return MAKEIT;
}

// checks for the exting name of net in non flated nets
int checkName (STRING newName, NonFlatPtr firstNonFlat)
{
   NonFlatPtr nextNonFlat = firstNonFlat;

   while (nextNonFlat)
   {
      if (newName == nextNonFlat->inst) return ITEXISTS;
      nextNonFlat = nextNonFlat->next;
   }
   return MAKEIT;
}

// make name for a net
STRING makeName (NETPTR net, NETPTR lastCurNet)
{
   char *varName;
   char* newName;
   extern long netCounter;
   int step = ITEXISTS;

   while (step == ITEXISTS)
   {
      netCounter += 1;
      varName = long2ascii (netCounter);
      newName = concatName ((char*)"net", varName);
      step = checkName (newName, net, lastCurNet);
   }
   return newName;
}

char *concat (char* fatherName, char* childName)
{
   char buf[14];
   char *newName = new char[14];

   strncpy (buf, fatherName, 9);
   buf[8] = 0;
   strncat (buf, childName, 4);
   buf[13] = 0;
   strcpy (newName, buf);
   newName = cs (newName);
   return newName;
}

char *concatName (char* fatherName, char* childName)
{
   char *newName = new char[14];

   strncpy (newName, fatherName, 5);
   strncat (newName, childName, 8);
   newName = cs (newName);
   return newName;
}

// This function returns a pointer to a static string area
// containing the decimal representation of a long 'n'.
char *long2ascii (long n)
{
   static char str[100];
   sprintf (str, "%ld", n);
   return str;
}
