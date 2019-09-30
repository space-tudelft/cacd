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

#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/makebus/seadifGraph.h"
#include "src/ocean/makebus/prototypes.h"

static void decideWhatWeCallBus (sdfset& theBuses);
static int  createSeadifBuses (CIRCUITPTR circuit, sdfset& theBuses);

int makebus (CIRCUITPTR cir)
{
   graphDescriptor gdesc ((char*)"MakeBus", SetCurrentGraph); // create a new graph
   // build a graph of that represents the circuit, use grNet in stead of gNet:
   gCircuit *theCircuit = buildCircuitGraph (cir, NULL, NULL, new_grNet);
   if (!theCircuit) return 0;
   initializeDataStructures();
   sdfset theBuses;		// this contains the buses of type anotherBus
   graphElementIterator nextNet (&gdesc, GNetGraphElementType);
   grNet *theNet;
   while ((theNet = (grNet *)nextNet())) // iterate all the nets
   {
      classifyNet (theBuses, theNet); // theNet may be added to anotherBus
   }
   decideWhatWeCallBus (theBuses); // not all sets of nets are real buses
   return createSeadifBuses (cir, theBuses); // convert back to seadif
}

static void decideWhatWeCallBus (sdfset& theBuses)
{
   sdfsetNextElmtIterator nextBus (&theBuses);
   anotherBus *thebus;
   while ((thebus = (anotherBus *)nextBus()))
      if (thebus->size() <= 1)	// not very useful...
      {
	 theBuses.remove (thebus); // remove the bus from the set
	 delete thebus;		  // get rid of it altogether
      }
}

// convert theBuses to seadif BUS structures:
static int createSeadifBuses (CIRCUITPTR circuit, sdfset& theBuses)
{
   sdfdeletebuslist (circuit->buslist); // delete the old buses
   circuit->buslist = NULL;
   int numberOfBuses = 0;
   sdfsetNextElmtIterator nextBus (&theBuses);
   anotherBus *thebus;
   while ((thebus = (anotherBus *)nextBus()))
   {
      BUSPTR seadifbus;
      NewBus (seadifbus);
      seadifbus->name = cs (thebus->busName());
      seadifbus->netref = NULL;
      sdfsetNextElmtIterator nextNet (thebus);
      grNet *thenet;
      while ((thenet = (grNet *)nextNet()))
      {
	 NETREFPTR seadifnetref;
	 NewNetRef (seadifnetref);
	 seadifnetref->net = thenet->net();
	 seadifnetref->next = seadifbus->netref;
	 seadifbus->netref = seadifnetref;
      }
      seadifbus->next = circuit->buslist;
      circuit->buslist = seadifbus;
      ++numberOfBuses;
   }
   return numberOfBuses;
}
