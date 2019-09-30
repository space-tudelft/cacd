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

static int        solveFunctionRef (FUNCTIONPTR);
static int        solveCircuitRef (CIRCUITPTR);
static int        solveLayoutRef (LAYOUTPTR);
static CIRCUITPTR findCircuit (INSTANCE_TPTR);
static LIBRARYPTR findLibrary (STRING libname, SEADIFPTR whereToPut);
static int        findCirport (CIRPORTREFPTR cpr, STRING cpname,
			      STRING ciname, CIRCUITPTR thiscircuit);
static LIBRARYPTR makeStubLibrary (STRING libname, SEADIFPTR whereToPut);
static CIRCUITPTR makeStubCircuit (STRING circuitname, LIBRARYPTR lib);
static void       *notUsed (void *);

static LIBRARYPTR  current_library = NULL;
static SEADIFPTR   edif_tree;

/* Solve symbolic references in edif_local; Lookup and add to seadif_tree the
 * references to cells that do not appear in edif_local.
 */
int solveRef (SEADIF *edif_t)
{
   edif_tree = edif_t;
   current_library = edif_tree->library;
   for (; current_library; current_library = current_library-> next)
   {
      FUNCTIONPTR function = current_library->function;
      for (; function; function = function->next)
      {
	 CIRCUITPTR circuit = function->circuit;
	 if (!solveFunctionRef (function))
	    return 0;
	 for (; circuit; circuit = circuit->next)
	 {
	    LAYOUTPTR layout = circuit->layout;
	    if (!solveCircuitRef (circuit))
	       return 0;
	    for (; layout; layout = layout->next)
	       if (!solveLayoutRef (layout)) return 0;
	 }
      }
   }
   return TRUE;
}

/* solve all symbolic references in the function: */
int solveFunctionRef (FUNCTIONPTR f)
{
   notUsed (f);
   return TRUE;			/* nothing to do! */
}

/* solve all symbolic references in the circuit: */
int solveCircuitRef (CIRCUITPTR circuit)
{
   NETPTR     net;
   CIRINSTPTR ci;
   /* two things to do:
    *     1. replace the INSTANCE_TPTR in the cirinst->circuit field
    *        by a  CIRCUITPTR.
    *     2. replace the STRINGs in cirportref->cirport and cirportref->cirinst
    *        by a CIRPORTPTR and a CIRINSTPTR respectively.
    */
   /* start with solving the CIRINSTances: */
   for (ci = circuit->cirinst; ci; ci = ci->next)
   {
      INSTANCE_TPTR it = (INSTANCE_TPTR) ci->circuit;
      ci->circuit = findCircuit (it); /* find the CIRCUITPTR */
      if (!ci->circuit)
	 return 0;
      FreeInstance_t (it);
   }
   /* next, solve the CIRPORTREFs: */
   for (net = circuit->netlist; net; net = net->next)
   {
      CIRPORTREFPTR cpr = net->terminals;
      for (; cpr; cpr = cpr->next)
      {
	 STRING cirportname = (STRING) cpr->cirport;
	 STRING cirinstname = (STRING) cpr->cirinst;
	 if (!findCirport (cpr, cirportname, cirinstname, circuit)) return 0;
	 if (!cpr->cirinst) cpr->cirport->net = net;
	 fs (cirportname);
	 if (cirinstname) fs (cirinstname);
      }
   }
   return TRUE;
}

/* solve all symbolic references in the layout: */
int solveLayoutRef (LAYOUTPTR layout)
{
   notUsed (layout);
   return TRUE;			/* not implemented (yet)*/
}

static CIRCUITPTR findCircuit (INSTANCE_TPTR it)
{
   STRING circuitname = it->cell_ref;
   STRING libraryname = it->library_ref;
   LIBRARYPTR  lib;
   FUNCTIONPTR fun = NULL;
   CIRCUITPTR  cir = NULL;
   if (!(lib = findLibrary (mapL (libraryname), edif_tree))) return NULL;
   for (fun = lib->function; fun; fun = fun->next)
      for (cir = fun->circuit; cir; cir = cir->next)
	 if (cir->name == circuitname)
	    return cir;		  /* found the circuit */
   if (!cir) /* circuit not found: get a new one and add it to the lib */
      switch (targetLanguage)
      {
      case NelsisLanguage:
	 cir = findNelsisCircuit (circuitname, lib);
	 break;
      case SeadifLanguage:
	 cir = findSeadifCircuit (circuitname, lib);
	 break;
      case PseudoSeadifLanguage:
	 cir = makeStubCircuit (circuitname, lib);
	 break;
      case NoLanguage:
      default:
	 cir = NULL;
      }
   return cir;
}

static LIBRARYPTR findLibrary (STRING libname, SEADIFPTR whereToPut)
{
   LIBRARYPTR lib = edif_tree->library;
   for (; lib; lib = lib->next)
      if (lib->name == libname)
	 break;
   if (!lib)
   {
      /* library not found, has to be an external lib ... */
      switch (targetLanguage)
      {
      case NelsisLanguage:
	 lib = findNelsisLibrary (libname, whereToPut);
	 break;
      case SeadifLanguage:
	 lib = findSeadifLibrary (libname, whereToPut);
	 break;
      case PseudoSeadifLanguage:
	 lib = makeStubLibrary (libname, whereToPut);
	 break;
      case NoLanguage:
      default:
	 lib = NULL;
      }
      if (lib) lib->flag.l |= EXTERNAL_LIBRARY;
   }
   return lib;
}

static int findCirport (CIRPORTREFPTR cpr, STRING cpname,
		       STRING ciname, CIRCUITPTR thiscirc)
{
   CIRINSTPTR cinst = NULL;
   CIRPORTPTR cp    = NULL;
   /* first get a pointer to the circuit: */
   CIRCUITPTR circuit = thiscirc;
   if (ciname) /* no instance name means "this circuit" */
   {
      /* search the instance list for an instance named ciname: */
      for (cinst = thiscirc->cirinst; cinst; cinst = cinst->next)
	 if (cinst->name == ciname)
	    break;
      if (!cinst)
	 report (eFatal, "circuit %s(%s): "
		"net %s refers to non-existing instance %s",
		thiscirc->name, current_library->name,
		cpr->net->name, ciname);
      else
	 circuit = cinst->circuit;
   }
   /* now that we have the circuit, look for the cirport named cpname: */
   for (cp = circuit->cirport; cp; cp=cp->next)
      if (cp->name == cpname)
	 break;
   if (!cp) { /* funny, there is no cirport named cpname... */
      if (!(circuit->flag.l & EXTERNAL_STUB))
	 report (eFatal, "circuit %s(%s): net %s refers to non existing port %s\n"
		"on instance %s, circuit %s",
		thiscirc->name, current_library->name,
		cpr->net->name, cpname,
		ciname, circuit->name);
      else
      {  /* this is a stub circuit, no wonder it does not have the cirport */
	 NewCirport (cp);
	 cp->name = cs (cpname);
	 cp->next = circuit->cirport;
#ifdef SDF_PORT_DIRECTIONS
         cp->direction = SDF_PORT_UNKNOWN;
#endif
	 circuit->cirport = cp;
      }
   }
   cpr->cirinst = cinst;
   cpr->cirport = cp;
   return TRUE;
}

LIBRARYPTR makeStubLibrary (STRING libname, SEADIFPTR whereToPut)
{
   LIBRARYPTR lib;
   NewLibrary (lib);
   lib->name = cs (libname);
   lib->flag.l |= EXTERNAL_STUB;
   lib->next = whereToPut->library;
   whereToPut->library = lib;
   return lib;
}

CIRCUITPTR makeStubCircuit (STRING circuitname, LIBRARYPTR lib)
{
   FUNCTIONPTR fun;
   CIRCUITPTR  cir;
   NewFunction (fun);
   NewCircuit (cir);
   fun->circuit = cir;
   cir->function = fun;
   fun->name = cs (circuitname);
   cir->name = cs (circuitname);
   fun->library = lib;
   fun->next = lib->function;
   lib->function = fun;
   cir->flag.l |= EXTERNAL_STUB;
   return cir;
}

/* list of mappings of external library names: */
static STRING libMap[1+MAXEXTERNLIBS][2];
static int    nmap = 0;

/* possible values for the second index of libMap[][] */
enum _libMapIndex {FromLib = 0, ToLib = 1};

/* if a mapping for FROMLIB has been defined (with the function makeMapL) then
 * return the name that FROMLIB is mapped to. Else return FROMLIB itself.
 */
STRING mapL (STRING fromlib)
{
   int i = 0;
   for (; i < nmap; ++i)
      if (libMap[i][FromLib] == fromlib)
	 return libMap[i][ToLib]; /* found a mapping, return new name */
   return fromlib;		  /* no mapping defined for this lib */
}

/* add a mapping from FROMLIB to TOLIB */
void makeMapL (STRING fromLib, STRING toLib)
{
   if (nmap > MAXEXTERNLIBS)
      report (eFatal, "too many library mappings specified (> %d)", MAXEXTERNLIBS);
   libMap[nmap][FromLib] = cs (fromLib);
   libMap[nmap++][ToLib] = cs (toLib);
}

void *notUsed (void *ptr)
{
   return ptr;
}
