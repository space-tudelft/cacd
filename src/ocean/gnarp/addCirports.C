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
#include "src/ocean/gnarp/prototypes.h"

// For each layout returned by the sdfNameIterator check that the Layports
// have all corresponding Cirports. If not, create the missing Cirports.
void addCirports (sdfNameIterator& seadifName)
{
   while (seadifName())
   {
      if (seadifName.nameType() != SeadifLayoutName) {
	 cerr << "Must specify a layout name for this operation ...\n";
	 return;
      }

      // The seadif library function sdfreadlay() creates itself missing
      // cirports. It therefore suffices to read the layout, and then just
      // write back the circuit to the database ...
      if (!sdfreadcir (SDFCIRPORT, seadifName.cname(),
		      seadifName.fname(), seadifName.bname()))
      {
	 cerr << "Cannot read circuit\n";
	 continue;
      }

      // count the Cirports:
      int previousCirports = 0;
      CIRPORTPTR cp;
      for (cp = thiscir->cirport; cp; cp = cp->next) ++previousCirports;
      if (!seadifName.sdfread (SDFLAYPORT))
      {
	 cerr << "Cannot read layout " << seadifName.sdfName() << "\n" << flush;
	 continue;
      }

      int currentCirports = 0;
      for (cp = thiscir->cirport; cp; cp = cp->next) ++currentCirports;
      if (currentCirports >= previousCirports)
      {
	 printTheBloodyObject (seadifName, 0,
	      form ("%-6s%-5s%-4s", "prev", "cur", "diff"),
	      form ("%2d%6d%5d ", previousCirports, currentCirports, currentCirports - previousCirports));
	 if (!sdfwritecir (SDFCIRPORT, thiscir)) {
	    cerr << "Cannot write circuit\n";
	    sdfexit (1);
	 }
      }
      else if (currentCirports < previousCirports) {
	 cerr << "INTERNAL ERROR: 723648\n";
	 sdfexit (1);
      }
   }
}
