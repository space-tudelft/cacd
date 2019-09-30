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
#include <iostream>
using namespace std;
#include "src/ocean/gnarp/prototypes.h"

static STRING operation_addCirports = cs ((char*)"addCirports");
static STRING operation_mkLibprim   = cs ((char*)"mkLibprim");
static STRING operation_rmLibprim   = cs ((char*)"rmLibprim");
static STRING operation_showLibprim = cs ((char*)"showLibprim");

void doGnarp (sdfNameIterator& seadifName, STRING operation)
{
   if (operation == operation_addCirports)
      addCirports (seadifName);
   else if (operation == operation_mkLibprim)
      modifyStatus (seadifName, actionAdd, (char*)"libprim", SeadifCircuitName);
   else if (operation == operation_rmLibprim)
      modifyStatus (seadifName, actionRm,  (char*)"libprim", SeadifCircuitName);
   else if (operation == operation_showLibprim)
      modifyStatus (seadifName, actionShow, (char*)"libprim", SeadifCircuitName);
   else
      cerr << "unrecognized operation \"" << operation << "\" ..."
	   << " (option -h for help)\n";
}
