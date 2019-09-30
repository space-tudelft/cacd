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

NonFlatPtr readNonFlat (STRING infile)
{
   FILE *fp;
   NonFlatPtr nextNonFlat, firstNonFlat = NULL, prevNonFlat = NULL;
   char scanline[100];
   char cir[100];
   char inst[100];
   STRING circuit = cir;
   STRING instance = inst;

   cir[0] = 0;
   inst[0] = 0;
   int first = 1;

   if (!infile) return NULL;

   if ((fp = fopen (infile, "r")))
   {
      while (fgets (scanline, 100, fp))
      {
	 sscanf (scanline, " %s %s", circuit, instance);
	 if (cir[0] == '%')
	 {
	    cir[0] = 0;
	    inst[0] = 0;
	    continue;
	 }
	 else
	 {
	    if (first)
	    {
	       first = 0;
	       NewNonFlat (firstNonFlat);
	       firstNonFlat->circuit = cs (circuit);
	       if (inst[0]) {
		  if (inst[0] == '%')
		     firstNonFlat->inst = NULL;
		  else
		     firstNonFlat->inst = cs (instance);
	       }
	       else
		  firstNonFlat->inst = NULL;
	       prevNonFlat = firstNonFlat;
	    }
	    else
	    {
	       NewNonFlat (nextNonFlat);
	       prevNonFlat->next = nextNonFlat;
	       nextNonFlat->circuit = cs (circuit);
	       if (inst[0]) {
		  if (inst[0] == '%')
		     firstNonFlat->inst = NULL;
		  else
		     nextNonFlat->inst = cs (instance);
	       }
	       else
		  nextNonFlat->inst = NULL;
	       prevNonFlat = nextNonFlat;
	    }
	 }
	 cir[0] = 0;
	 inst[0] = 0;
      }
      prevNonFlat->next = NULL;
      fclose (fp);
      return firstNonFlat;
   }
   else
   {
      fprintf (stderr, "ERROR: cannot open input file '%s'\n", infile);
      exit (1);
   }
   return NULL;
}
