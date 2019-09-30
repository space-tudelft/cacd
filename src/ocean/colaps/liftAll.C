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

/*
 * recursion algorithm  for flating a circuit
 */

#include "src/ocean/colaps/colapshead.h"

// program manager for flattening a circuit; first, the circuit
// instances are flattened (changed with the instances of corespondent
// circuit); second, the visibil terminals are flattened and third the
// internal terminals of the flattened circuit are added to the netlist
void colaps (CIRCUITPTR sdfCircuit, NonFlatPtr firstNon, int toCellLib,
	    char *outfile, char *outcircuit)
{
   CIRINSTPTR    firstInst, lastInst;
   NonFlatPtr    firstNonFlat;
   extern int    doNotMakePathList;

   firstNonFlat = firstNon;

   sdfCircuit->flag.p = NULL;
   firstInst = sdfCircuit->cirinst;
   if (!firstNonFlat)
   {
      if (!toCellLib)
	 lastInst = liftAll (firstInst);
      else
	 lastInst = liftAllToCell (firstInst);
   }
   else
   {
      if (!toCellLib)
	 lastInst = liftAll (firstInst, firstNonFlat);
      else
	 lastInst = liftAllToCell (firstInst, firstNonFlat);
   }

   cleanUp (sdfCircuit, firstNon);

   if (doNotMakePathList != TRUE)
      printPath (sdfCircuit, outfile, outcircuit);
}

/*
 * recursion algorithm  for flattening a circuit
 */
CIRINSTPTR liftAll (CIRINSTPTR inst)
{
   if (!inst)
      return inst;
   liftAll (inst->circuit->cirinst);
   inst = flatCirinst (inst);
   liftAll (inst->next);
   if (!inst->next)
      liftNets (inst->curcirc);
   return inst;
}

CIRINSTPTR liftAll (CIRINSTPTR inst, NonFlatPtr firstNon)
{
   if (!inst) return inst;

   if (checkInst (inst, firstNon) == MAKEIT)
   {
      liftAll (inst->circuit->cirinst, firstNon);
      inst = flatCirinst (inst);
      liftAll (inst->next, firstNon);
      if (!inst->next)
	 liftNets (inst->curcirc);
      return inst;
   }
   liftAll (inst->next, firstNon);
   if (!inst->next)
      liftNets (inst->curcirc);
   return inst;
}

CIRINSTPTR liftAllToCell (CIRINSTPTR inst)
{
   if (!inst) return inst;

   if (checkForLibCell (inst) == MAKEIT)
   {
      liftAllToCell (inst->circuit->cirinst);
      inst = flatCirinst (inst);
      liftAllToCell (inst->next);
      if (!inst->next)
	 liftNets (inst->curcirc);
      return inst;
   }
   liftAllToCell (inst->next);
   if (!inst->next)
      liftNets (inst->curcirc);
   return inst;
}

CIRINSTPTR liftAllToCell (CIRINSTPTR inst, NonFlatPtr firstNon)
{
   if (!inst) return inst;

   if (checkInst (inst, firstNon) == MAKEIT && checkForLibCell (inst) == MAKEIT)
   {
      liftAllToCell (inst->circuit->cirinst, firstNon);
      inst = flatCirinst (inst);
      liftAllToCell (inst->next, firstNon);
      if (!inst->next)
	 liftNets (inst->curcirc);
      return inst;
   }
   liftAllToCell (inst->next, firstNon);
   if (!inst->next)
      liftNets (inst->curcirc);
   return inst;
}

int checkInst (CIRINSTPTR inst, NonFlatPtr firstNon)
{
   NonFlatPtr nextNonFlat;

   nextNonFlat = firstNon;

   while (nextNonFlat)
   {
      if (!nextNonFlat->inst)
      {
	 if (nextNonFlat->circuit == inst->circuit->name)
	 {
	    inst->flag.p = NULL;
	    return ITEXISTS;
	 }
      }
      else
      {
	 if ((nextNonFlat->circuit == inst->curcirc->name) &&
	      (nextNonFlat->inst == inst->name))
	 {
	    inst->flag.p = NULL;
	    return ITEXISTS;
	 }
      }
      nextNonFlat = nextNonFlat->next;
   }
   return MAKEIT;
}

int checkForLibCell (CIRINSTPTR inst)
{
   CIRINSTPTR nextInst;

   nextInst = inst->circuit->cirinst;

   while (nextInst)
   {
      if (!nextInst->circuit->cirinst && !nextInst->circuit->netlist)
	 nextInst = nextInst->next;
      else
	 return MAKEIT;
   }
   return ITEXISTS;
}

CIRINSTPTR findNextInst (CIRINSTPTR inst)
{
   CIRINSTPTR  nextInst;
   MyFlagPtr   myFlag;
   WorkFlagPtr myWorkFlag;

   if (inst->flag.p)
   {
      myFlag = (MyFlagPtr)inst->flag.p;
      myWorkFlag = myFlag->workFlag;
      if (myWorkFlag)
      {
	 nextInst = myWorkFlag->next;
	 return nextInst;
      }
   }
   nextInst = inst->next;
   return nextInst;
}

// type an error message and exit
void colapsError (const char *s)
{
   cout << flush;
   cerr << "\nInternal collapse error: " << s << endl;
   abort ();
}
