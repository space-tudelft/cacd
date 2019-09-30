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

#include <sys/types.h>
#include <sys/stat.h>

#include "src/ocean/colaps/colapshead.h"

CIRCUITPTR makeLocalCopy (CIRCUITPTR remote, char *outcircuit)
{
   CIRCUITPTR cirlocal;
   FUNCTIONPTR funclocal;
   LIBRARYPTR liblocal = NULL;
   char buf1[9];
   char buf2[9];
   STRING myCirName;
   STRING myFunName;

   myCirName = buf1;
   myFunName = buf2;

   myCirName = (char*)"Flt";
   myFunName = (char*)"Flt";

// make local library
   if (!liblocal) liblocal = findLocalLib();

   if (outcircuit) {
      myCirName = outcircuit;
      myFunName = outcircuit;
   }
   else {
      char tmp[100];
      strcpy (tmp, remote->name);
      myCirName = cs (concat (tmp, myCirName));
      strcpy (tmp, remote->function->name);
      myFunName = cs (concat (tmp, myFunName));
   }
/*
 * find out whether this circuit already exists
 */
   if (sdfexistscir (myCirName, myFunName, liblocal->name))
   {
      /*  already in database : read it */
      sdfreadcir (SDFCIRBODY, myCirName, myFunName, liblocal->name);
      cirlocal = thiscir;
      funclocal = cirlocal->function;
   }
   else
   {
      // create a new function
      NewFunction (funclocal);
      funclocal->name = cs (myFunName);
      // create a new circuit
      NewCircuit (cirlocal);
      cirlocal->name = cs (myCirName);
   }

// initialize the function
   funclocal->library = liblocal;
// link the local function list
   funclocal->next = liblocal->function;
   liblocal->function = funclocal;

// link the local circuit list
   cirlocal->function = funclocal;
   cirlocal->next = funclocal->circuit;
   funclocal->circuit = cirlocal;
// copy the remote circuit
   cirlocal->cirport = remote->cirport;
   cirlocal->cirinst = remote->cirinst;
   cirlocal->netlist = remote->netlist;
   cirlocal = setCurcirc (cirlocal);

   return cirlocal;
}

LIBRARYPTR findLocalLib ()
{
   static LIBRARYPTR liblocal = NULL;
   STRING libname;

   if (liblocal) return liblocal;

   libname = sdfgetcwd();
   if (!libname) sdfreport (Fatal, "Cannot get the current working directory...");
   libname = onlyLibName (libname);
   if (!*libname) sdfreport (Fatal, "No name, root cannot be project directory...");
   libname = cs (libname);

   if (!sdfexistslib (libname)) {
      putenv ((char*)"NEWSEALIB=seadif/sealib.sdf");
      if (access ("seadif", W_OK | X_OK)) mkdir ("seadif", 0755);
      NewLibrary (liblocal);
      liblocal->name = libname;
   }
   else {
      sdfreadlib (SDFLIBALL, libname);
      liblocal = thislib;
   }
   return liblocal;
}

CIRCUITPTR setCurcirc (CIRCUITPTR cirlocal)
{
   CIRINSTPTR nextInst;
   NETPTR nextNet;

   nextInst = cirlocal->cirinst;
   nextNet = cirlocal->netlist;

   while (nextInst)
   {
      nextInst->curcirc = cirlocal;
      nextInst = nextInst->next;
   }

   while (nextNet)
   {
      nextNet->circuit = cirlocal;
      nextNet = nextNet->next;
   }

   return cirlocal;
}

char *onlyLibName (char *s)
{
   char *p = strrchr (s, '/');
   if (!p) return s;
   return p+1;
}
