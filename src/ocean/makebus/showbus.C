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

#include <iostream>

using namespace std;

#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/makebus/prototypes.h"

void showbus(CIRCUITPTR thecircuit, int showNets)
{
   int numbuses = 0;
   BUSPTR thebus = thecircuit->buslist;

   for (; thebus; thebus = thebus->next) ++numbuses;

   if (numbuses < 1) cout << "No buses\n";
   else if (numbuses == 1) cout << "1 bus:\n\n";
   else cout << numbuses << " buses:\n\n";

   for (thebus = thecircuit->buslist; thebus; thebus = thebus->next)
   {
      cout << thebus->name << "\n";
      if (showNets)
      {
	 cout << "\t";
	 for (NETREFPTR nr = thebus->netref; nr; nr = nr->next)
	    cout << " " << nr->net->name;
	 cout << "\n";
      }
   }
}
