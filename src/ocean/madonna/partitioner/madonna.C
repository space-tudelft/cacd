/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 *	Paul Stravers
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
#include <iostream>
using namespace std;

#include <malloc.h>
#include <stdlib.h>
#include "src/ocean/libseadif/libstruct.h"
#include "src/ocean/libseadif/sealibio.h"
#include "src/ocean/madonna/partitioner/genpart.h"
#include "src/ocean/madonna/partitioner/cost.h"
#include "src/ocean/madonna/partitioner/part.h"
#include "src/ocean/madonna/partitioner/globRoute.h"
#include "src/ocean/madonna/phil/phil_glob.h"

extern int requestedGridPoints[];
extern int expandableDirection;

extern LIBTABPTR thislibtab; /* set by existslib() */
extern FUNTABPTR thisfuntab; /* set by existsfun() */
extern CIRTABPTR thiscirtab; /* set by existscir() */
extern LAYTABPTR thislaytab; /* set by existslay() */

extern LIBRARYPTR  thislib; /* set by sdfreadlib() sdfreadfun() sdfreadcir() sdfreadlay() */
extern FUNCTIONPTR thisfun; /* set by sdfreadfun() sdfreadcir() sdfreadlay() */
extern CIRCUITPTR  thiscir; /* set by sdfreadcir() sdfreadlay() */
extern LAYOUTPTR   thislay; /* set by sdfreadlay() */

extern char *circuit_name, *function_name, *library_name, *layoutname;
extern double extraplaza;
extern int madonnamakeminiplaza;
extern int madonnamakepartition;

static void printNetlistStatistics (CIRCUITPTR circuit)
{
   NETPTR net;
   int netdist[10], j, total_nets = 0;

   for (j = 0; j < 10; ++j) netdist[j] = 0;

   for (net = circuit->netlist; net; net = net->next)
   {
      j = net->num_term;
      if (j < 0) ;
      else if (j <=  1) netdist[0] += 1;
      else if (j <=  7) netdist[j-1] += 1;
      else if (j <= 10) netdist[7] += 1;
      else if (j <= 20) netdist[8] += 1;
      else netdist[9] += 1;
   }

   for (j = 0; j < 10; ++j) total_nets += netdist[j];

   printf ("------ net distribution (total #nets = %d):\n"
          "        0..1     2     3     4     5     6     7 8..10 11..20  >20\n\n"
          "      ", total_nets);

   for (j = 0; j < 10; ++j) printf (" %5d", netdist[j]);
   printf ("\n\n");
}

/* Create a layout isomorph to the circuit. Call the layout "Madonna". */

CIRCUITPTR madonna (CIRCUIT *circuit)
{
   TOTALPPTR    total = NULL;
   FUNCTIONPTR  savethisfun = thisfun;
   CIRCUITPTR   savethiscir = thiscir, thecircuit, bestcircuit;
   LAYOUTPTR    savethislay = thislay;
   LIBRARYPTR   savethislib = thislib;

   initnetcostinfo ();

   readImageFile ();		    // Here we call the parser to read all
                                    // the stuff about image

   printNetlistStatistics (circuit);

   if (madonnamakepartition)
   {
      madonna_ (&total, circuit, 1);

      bestcircuit = (total ? total->bestpart : NULL);
   }
   else
      bestcircuit = NULL;

   if (!total || !total->bestpart)
      thecircuit = circuit;
   else
      thecircuit = total->bestpart;

   if (madonnamakeminiplaza)
   {
      if (!madonnamakepartition)
      {
	 phil (circuit_name, function_name, library_name,
	      layoutname, extraplaza, NULL, NULL, NULL);
      }
      else
      {
	 if (total && total->nx >= 1 && total->ny >= 1)
	 {
	    // ...and now for the global routing:
	    expansionGrid globgrid (total); // create global grid and globl nets
	    globgrid.routeGlobNets ();	   // route the nets

	    phil (circuit_name, function_name, library_name, layoutname,
		 extraplaza, thecircuit, savethiscir, total->routing);
	 }
	 else
	 {
	    // nothing to do for the global router ...
	    phil (circuit_name, function_name, library_name, layoutname,
		 extraplaza, thecircuit, savethiscir, NULL);
	 }
      }
   }
   thislay = savethislay; thiscir = savethiscir;
   thisfun = savethisfun; thislib = savethislib;

   return bestcircuit;
}
