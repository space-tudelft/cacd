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

// flatCirinst is flattening an instance by lifting the instances of
// the coresponding circuit (actual reference to the instance)
CIRINSTPTR flatCirinst (_CIRINST *instToBeFlat)
{

// the first childInstance replaces fatherInstance and
// the other instances are inserted after the first one

   CIRCUITPTR      fatherCurcirc, fatherCircuit;
   CIRINSTPTR      childNext, fatherNext, refInst;
   MyFlagPtr       myFlag, childFlag;
   WorkFlagPtr     myWorkFlag;
   InstPathPtr     myInstPath = NULL, firstPath;
   CIRINSTPTR      newInst, oldInst;
   extern int      doNotMakePathList;

/*
 * YOU HAVE A NULL INSTANCE AND A NULL NETLIST!!! => a primitive circuit
 */
   if (!instToBeFlat->circuit->cirinst && !instToBeFlat->circuit->netlist)
   {
      if (instToBeFlat->flag.p)
      {
	 myFlag = (MyFlagPtr)instToBeFlat->flag.p;
	 myFlag->workFlag = NULL;
	 return instToBeFlat;
      }
      instToBeFlat->flag.p = NULL;
      return instToBeFlat;  // nothing to be flat
   }
/*
 * YOU HAVE ONLY A NULL INSTANCE but a netlist => a circuit with only nets {}
 * this is a dummy instance; do not delete it yet!!
 */
   if (!instToBeFlat->circuit->cirinst && instToBeFlat->circuit->netlist)
   {
      NewMyFlag (myFlag);  // initializing my own flag
      NewWorkFlag (myWorkFlag);
      myWorkFlag->next = instToBeFlat->next;
      myWorkFlag->circuit = NULL; // dummy instance
      myWorkFlag->refInst = NULL;
      myWorkFlag->shortFlag = NULL;
      myFlag->workFlag = myWorkFlag;
      myFlag->instPath = NULL;
      instToBeFlat->flag.p = (void *)myFlag; // put it in a right place
      return instToBeFlat;  // nothing to be flat
   }

/*
 * OK, a normal instance
 */
// saving the instance name, the coresponding circuit pointer
// and the other useful information
   fatherCircuit = instToBeFlat->circuit;   // don't lose the actual circuit
   fatherNext = instToBeFlat->next;
   fatherCurcirc = instToBeFlat->curcirc;

/*
 * lifting the first childInstance
 */
   refInst = fatherCircuit->cirinst;
   instToBeFlat->name = NULL;

// actual reference to the instance and the attribute are lifted
   instToBeFlat->attribute = instToBeFlat->circuit->cirinst->attribute;
   childFlag = (MyFlagPtr)instToBeFlat->circuit->cirinst->flag.p;
   instToBeFlat->circuit = instToBeFlat->circuit->cirinst->circuit;

   NewMyFlag (myFlag);  // initializing my own flag
   if (childFlag)
      firstPath = childFlag->instPath;
   else
      firstPath = NULL;
   NewWorkFlag (myWorkFlag);
   myWorkFlag->next = fatherNext;
   myWorkFlag->refInst = refInst;
   myWorkFlag->circuit = fatherCircuit;
   myWorkFlag->shortFlag = NULL;
   myFlag->workFlag = myWorkFlag;
   if (doNotMakePathList != TRUE)
   {
      NewInstPath (myInstPath);
      myFlag->instPath = addCircuitPath (firstPath, myInstPath, fatherCircuit);
   }
   else
      myFlag->instPath = NULL;
   instToBeFlat->flag.p = (void *)myFlag; // put it in a right place
   oldInst = instToBeFlat;
/*
 * now, work on the other childInstances
 */
   childNext = fatherCircuit->cirinst->next;
   while (childNext)
   {
      NewCirinst (newInst);
      oldInst->next = newInst;
      newInst->name = NULL;
      newInst->circuit = childNext->circuit; // lift the circuit
      newInst->curcirc = fatherCurcirc; // keep the current circuit
      newInst->attribute = childNext->attribute; // lift the attribute
      childFlag = (MyFlagPtr)childNext->flag.p;

      NewMyFlag (myFlag);  // initializing my own flag
      if (childFlag)
	 firstPath = childFlag->instPath;
      else
	 firstPath = NULL;
      NewWorkFlag (myWorkFlag);
      myWorkFlag->next = NULL;
      myWorkFlag->refInst = childNext;
      myWorkFlag->circuit = fatherCircuit;
      myWorkFlag->shortFlag = NULL;
      myFlag->workFlag = myWorkFlag;
      if ((firstPath || childNext->circuit->cirinst) &&
	   (doNotMakePathList != TRUE))
	 myFlag->instPath = addCircuitPath (firstPath, myInstPath);
      else
	 myFlag->instPath = NULL;
      newInst->flag.p = (void *)myFlag;
      childNext = childNext->next;
      oldInst = newInst;
   }
   oldInst->next = fatherNext; // back to the father cirinst list
   return oldInst;
}

InstPathPtr addCircuitPath (InstPathPtr firstPath, InstPathPtr newPath,
			    CIRCUITPTR circuit)
{
   InstPathPtr nextPath, addPath, prevPath, firstAdd;

   if (firstPath)
   {
      NewInstPath (firstAdd);
      firstAdd->circuitName = firstPath->circuitName;
      firstAdd->next = NULL;
      nextPath = firstPath;
      prevPath = firstAdd;

      while (nextPath->next)    // find the last element in the list
      {
	 nextPath = nextPath->next;
	 NewInstPath (addPath);
	 prevPath->next = addPath;
	 addPath->circuitName = nextPath->circuitName;
	 addPath->next = NULL;
	 prevPath = addPath;
      }

      prevPath->next = newPath;
   }
   else    // the first path in the list
      firstAdd = newPath;
   newPath->circuitName = circuit->name;
   newPath->next = NULL;
   return firstAdd;
}

InstPathPtr addCircuitPath (InstPathPtr firstPath, InstPathPtr newPath)
{
   InstPathPtr nextPath, prevPath, firstAdd, addPath;

   if (firstPath)
   {
      NewInstPath (firstAdd);
      firstAdd->circuitName = firstPath->circuitName;
      firstAdd->next = NULL;
      nextPath = firstPath;
      prevPath = firstAdd;

      while (nextPath->next)    // find the last element in the list
      {
	 nextPath = nextPath->next;
	 NewInstPath (addPath);
	 prevPath->next = addPath;
	 addPath->circuitName = nextPath->circuitName;
	 addPath->next = NULL;
	 prevPath = addPath;
      }
      prevPath->next = newPath;
   }
   else
      firstAdd = newPath;
   return firstAdd;
}
