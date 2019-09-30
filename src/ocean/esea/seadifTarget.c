/*
 * ISC License
 *
 * Copyright (C) 1993-2018 by
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

#include "src/ocean/esea/cedif.h"
#include "src/ocean/libseadif/sdferrors.h"
#include <stdlib.h> /* prototypes exit() */

static int database_is_open = 0;

LIBRARYPTR findSeadifLibrary (STRING libname, SEADIFPTR seadif_tree)
{
   if (!sdfexistslib (libname))
      report (eFatal, "I cannot find the external seadif library \"%s\"...", libname);
   if (!sdfreadlib (SDFLIBBODY, libname))
      report (eFatal, "I cannot read the external seadif library \"%s\"", libname);
   /* move this lib to seadif_tree: */
   thislib->next = seadif_tree->library;
   seadif_tree->library = thislib;
   return thislib;
}

CIRCUITPTR findSeadifCircuit (STRING cirname, LIBRARYPTR lib)
{
   STRING libname = lib->name;
   if (!sdfexistscir (cirname, cirname, libname))
      report (eFatal, "I cannot find the external seadif circuit %s(%s) ...", cirname, libname);
   if (!sdfreadcir (SDFCIRPORT, cirname, cirname, libname))
      report (eFatal, "I cannot read the external seadif circuit %s(%s)", cirname, libname);
   return thiscir;
}

void openSeadif ()
{
   database_is_open = TRUE;
   if (sdfopen() != SDF_NOERROR) report (eFatal, "cannot open the seadif database");
}

void closeSeadif ()
{
   sdfclose();
   database_is_open = 0;
}

void exitSeadif (int exitValue)
{
   if (database_is_open)
      sdfexit (exitValue);
   else
      exit (exitValue);
}

void writeSeadifCircuit (CIRCUITPTR circuit)
{
   if (!sdfwritecir (SDFCIRALL, circuit))
      report (eFatal, "canot write seadif circuit %s(%s)", circuit->name,
	     circuit->function->library->name);
}
