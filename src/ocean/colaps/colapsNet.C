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

// manager for lifting the nets
void liftNets (CIRCUITPTR circuit)
{
   NETPTR         nextNet, oldNet, bufNet;
   extern long    netCounter;
   extern int     verbose;

   nextNet = circuit->netlist;
   oldNet = circuit->netlist;
   netCounter = 0;

   if (circuit->flag.p) return;

   if (!circuit->netlist) return;

   if (verbose) cout << "flattening circuit " << circuit->name << "\n";

   while (nextNet)  // first work on the own nets of father circuit
   {
      nextNet = flatNetlist (nextNet);
      if (nextNet->num_term <= 1) // remove nets with only one terminal
      {
	 findCirport (nextNet);
	 bufNet = nextNet->next;
	 deleteNet (nextNet);
	 nextNet = bufNet;
      }
      else {
	 oldNet = nextNet;
	 nextNet = nextNet->next;
      }
   }

   oldNet = addNewNets (oldNet); // add the hidden nets
// make all myWorkFlag NULL in cirinsts and delete dummy cirinsts
   cleanUp (circuit->cirinst);
}

// find the cirport of a net with only one terminal and put its net NULL
void findCirport (NETPTR net)
{
   CIRPORTPTR    nextCirport;

   nextCirport = net->circuit->cirport;

   while (nextCirport)
   {
      if (nextCirport->net == net)
      {
	 nextCirport->net = NULL;
	 return;
      }
      nextCirport = nextCirport->next;
   }
   return;
}

// flatNetlist is flating a netlist by lifting the terminals
NETPTR flatNetlist (NETPTR netToBeFlat)
{
   CIRPORTREFPTR nextTerm, fatherNext;
   int term_nr = 0; // how many terminals do we have now?

   nextTerm = netToBeFlat->terminals; // first terminal to be flated
   nextTerm->flag.l = 0; // use the flag for counting the terminals

   while (nextTerm)
   {
      if (nextTerm->cirinst) // nothing to lift for self reference
      {
	 fatherNext = nextTerm->next;
	 nextTerm = flatTerminal (nextTerm); // lift the terminals
	 if (nextTerm) // if NULL - a dummy terminal from a net {}; delete it
	    // a dummy terminal belongs to a dummy cirinst
	    term_nr = nextTerm->flag.l;     // actual terminal number
	 nextTerm = fatherNext;
      }
      else
      {
	 term_nr += 1;  // count terminals
	 nextTerm = nextTerm->next;
      }
      if (nextTerm) nextTerm->flag.l = term_nr;
   }
   netToBeFlat->num_term = term_nr;
   return netToBeFlat;
}

// flatTerminal is flattening a terminal: replaces fatherTerminal with
// the first childTerminal and inserts the next
CIRPORTREFPTR flatTerminal (CIRPORTREFPTR termToBeFlat)
{
   NETPTR           curNet, childNet;
   CIRPORTPTR       fatherCirport;
   CIRINSTPTR       fatherInst;
   CIRPORTREFPTR    fatherNext;
   CIRPORTREFPTR    nextTerm, oldTerm, newTerm;
   int              term_nr, selfTerm, externTermNr;
   MyFlagPtr        myFlag;
   WorkFlagPtr      myWorkFlag;
   STRING           netName;
   ShortFlagPtr     bufShort, newShort;

   int stepper = 1;
   selfTerm = 0;
   externTermNr = 0;

/* NOT A TERMINAL TO BE FLAT: belongs to a not_flated instance */
   if (!termToBeFlat->cirinst->flag.p) {
      termToBeFlat->flag.l += 1;
      return termToBeFlat;
   }
   else {
/* NOT A TERMINAL TO BE FLAT: belongs to a primitive circuit */
      myFlag = (MyFlagPtr)termToBeFlat->cirinst->flag.p;
      if (!myFlag->workFlag) {
	 termToBeFlat->flag.l += 1;
	 return termToBeFlat;
      }
   }

/*
 * a cirport with a NULL net reference
 */
   if (!termToBeFlat->cirport->net)
   {
      oldTerm = deleteTerm (termToBeFlat);
      return oldTerm; // return NULL
   }

/*
 * this net has more then 2 self_reference terminals,
 * maybe it was already copied !! short
 */
   if (termToBeFlat->cirport->net->flag.l == NOCOPY)
   {
      netName = termToBeFlat->cirport->net->name;
      myFlag = (MyFlagPtr)termToBeFlat->cirinst->flag.p;
      myWorkFlag = myFlag->workFlag;
      bufShort = myWorkFlag->shortFlag;
      if (myWorkFlag->shortFlag)
      {
	 while (bufShort)
	 {
	    if (netName == bufShort->name) //check for copied
 	    {
	       termToBeFlat = colapsNets (termToBeFlat, bufShort);
	       return termToBeFlat;
	    }
	    bufShort = bufShort->next;
	 }
      }
   }
/*
 * OK, a normal terminal but it could be a net {}
 * save the information from fatherTerminal
 */
   curNet = termToBeFlat->net;  // father netlist
   fatherInst = termToBeFlat->cirinst; // fatherInstance already changed
                                     // with a new one
   fatherCirport = termToBeFlat->cirport; // this cirport will disappear
   fatherNext = termToBeFlat->next;

   myFlag = (MyFlagPtr)fatherInst->flag.p;
   myWorkFlag = myFlag->workFlag;
   if (myWorkFlag->circuit)
      fatherCirport = addInstForCirport (fatherInst, fatherCirport);

   term_nr = termToBeFlat->flag.l; // there are l terminals till now
   nextTerm = fatherCirport->net->terminals; // from fatherCirport to the netlist
                                // and then to childTerminals to be lifted
   childNet = fatherCirport->net;
   oldTerm = termToBeFlat;

   while (stepper && nextTerm) // still at the first childTerminal
   {
      if (nextTerm->cirinst)  // skip self reference terminals
      {
	 termToBeFlat->cirport = nextTerm->cirport; // lift the cirport
	 termToBeFlat->cirinst = findInst (fatherInst, nextTerm->cirinst);
	 oldTerm = termToBeFlat;
	 term_nr += 1;
	 stepper = 0;
	 externTermNr += 1;
      }
      else
	 selfTerm += 1;
      nextTerm = nextTerm->next;
   }

//work on the next childTerminals
   while (nextTerm)
   {
      if (nextTerm->cirinst)  // skip self reference terminals
      {
	 NewCirportref (newTerm); // insert a new terminal
	 oldTerm->next = newTerm;
	 newTerm->net = curNet;  // keep the father netlist
	 newTerm->cirport = nextTerm->cirport;  // lift the cirport
	 newTerm->cirinst = findInst (fatherInst, nextTerm->cirinst);
	 oldTerm = newTerm;
	 term_nr += 1;
	 externTermNr += 1;
      }
      else
	 selfTerm += 1;
      nextTerm = nextTerm->next;
   }
   oldTerm->next = fatherNext; // back to the father terminals
   oldTerm->flag.l = term_nr;  // send the number of terminals
// Oh, a problematic net with net {}
   if (selfTerm >= 2) // net with more then 2 self_terminals
   {
      bufShort = myWorkFlag->shortFlag;//shortFlag is a list net{} in a circuit
      if (bufShort)
      {
	 while (bufShort->next)
	    bufShort = bufShort->next;
	 NewShortFlag (newShort); // add a new short on the list
	 bufShort->next = newShort;
      }
      else
      {
	 NewShortFlag (newShort);  // the first short on the list
	 myWorkFlag->shortFlag = newShort;
      }

      newShort->net = curNet;  // this net is a short
      newShort->name = childNet->name;//net name in actual reference circuit
      newShort->next = NULL;
      if (!myWorkFlag->circuit || !externTermNr)
	 oldTerm = deleteTerm (oldTerm); // delete the dummy terminal
      myFlag->workFlag = myWorkFlag;
//    fatherInst->flag.p = (void *)myFlag;
      childNet->flag.l = NOCOPY;
   }
   return oldTerm; // NULL if it was a dummy terminal
}

// delete a terminal in a net; first find the terminal then delete it
// and save the terminal list
CIRPORTREFPTR deleteTerm (CIRPORTREFPTR shortTerm)
{
   NETPTR           currentNet;
   CIRPORTREFPTR    nextTerm, nextFlat, prevTerm;

   nextFlat = shortTerm->next;
   currentNet = shortTerm->net;
   nextTerm = currentNet->terminals;

   if (nextTerm == shortTerm) // if dummy terminal is first in the list
   {
      currentNet->terminals = nextFlat;
      FreeCirportref (shortTerm);
      return NULL;
   }
// it is not the first => find previous
   prevTerm = nextTerm;
   nextTerm = nextTerm->next;

   while (nextTerm)
   {
      if (nextTerm == shortTerm)
      {
	 prevTerm->next = nextTerm->next;
	 FreeCirportref (shortTerm);
	 return NULL;
      }
      prevTerm = nextTerm;
      nextTerm = nextTerm->next;
   }
// error, terminal does not exist
   colapsError ("in deleteTerm, terminal does not exist");
   return NULL;
}

// delete a net with only one terminal
NETPTR deleteNet (NETPTR net)
{
   NETPTR        nextNet, prevNet, nextFlat;
   CIRCUITPTR    fatherCircuit;

   fatherCircuit = net->circuit;
   nextNet = fatherCircuit->netlist;
   nextFlat = net->next;

   if (nextNet == net) // if the net is first in the list
   {
      fatherCircuit->netlist = nextFlat;
      FreeNet (net);
      return NULL;
   }
// the net is not the first => find previous
   prevNet = nextNet;
   nextNet = nextNet->next;

   while (nextNet)
   {
      if (nextNet == net)
      {
	 prevNet->next = nextNet->next;
	 FreeNet (net);
	 return NULL;
      }
      prevNet = nextNet;
      nextNet = nextNet->next;
   }
// error, net does not exist
   colapsError ("in deleteNet, net does not exist");
   return NULL;
}

// set cirinst->flag to NULL and deletes dummy instances
void cleanUp (CIRINSTPTR firstInst)
{
   MyFlagPtr         myFlag;
   WorkFlagPtr       myWorkFlag;
   ShortFlagPtr      shortFlag;
   extern int        doNotMakePathList;

   CIRINSTPTR        nextInst, prevInst;
   CIRCUITPTR        fatherCircuit;
   CIRPORTPTR        nextCirport;
   NETPTR            nextNet;

   fatherCircuit = firstInst->curcirc;
   nextInst = firstInst;
   prevInst = NULL;

   while (nextInst)
   {
      if (nextInst->flag.p)
      {
	 myFlag = (MyFlagPtr)nextInst->flag.p;
	 myWorkFlag = myFlag->workFlag;
	 shortFlag = myWorkFlag->shortFlag;
	 if (myWorkFlag)
	 {
	    if (!myWorkFlag->circuit)
	    {
	       if (prevInst)
	       {
		  prevInst->next = nextInst->next;
		  nextInst->flag.p = NULL;
		  FreeCirinst (nextInst);
		  nextInst = prevInst->next;
	       }
	       else   // cirinst to be deleted is the first
	       {
		  fatherCircuit->cirinst = nextInst->next;
		  nextInst->flag.p = NULL;
		  FreeCirinst (nextInst);
		  nextInst = fatherCircuit->cirinst;
	       }
	    }
	    else
	    {
	       nextCirport = myWorkFlag->circuit->cirport;
	       while (nextCirport)
	       {
		  nextCirport->flag.p = NULL;
		  nextCirport = nextCirport->next;
	       }
	       nextNet = myWorkFlag->circuit->netlist;
	       while (nextNet)
	       {
		  nextNet->flag.p = NULL;
		  nextNet = nextNet->next;
	       }
	       shortFlag = NULL;
	       myWorkFlag->next = NULL;
	       myWorkFlag->circuit = NULL;
	       myWorkFlag->refInst = NULL;
	       myWorkFlag = NULL;
	       myFlag->workFlag = NULL;
	    }
	 }
      }
      if (doNotMakePathList == TRUE)
      {
	 myFlag = NULL;
	 nextInst->flag.p = NULL;
      }
      prevInst = nextInst;
      nextInst = nextInst->next;
   }
}

// colaps two nets (oldNet in newNet)
CIRPORTREFPTR colapsNets (CIRPORTREFPTR shortTerm, ShortFlagPtr bufShort)
{
   NETPTR           oldNet, prevNet, nextNet, newNet;
   CIRCUITPTR       fatherCircuit;
   CIRPORTREFPTR    nextTerm = NULL, nextNewNetTerm;
   CIRPORTPTR       cirports;
   CIRINSTPTR       oldInst;
   int              term_nr;
   int              stepper = 1, netInfo;

   oldInst = shortTerm->cirinst;
   newNet = shortTerm->net;
   oldNet = bufShort->net;
   nextNet = oldNet->next;
   fatherCircuit = oldNet->circuit;
   prevNet = fatherCircuit->netlist;
   nextNewNetTerm = shortTerm->next;
   term_nr = shortTerm->flag.l;
   netInfo = NONET;

   if (oldNet != newNet)
   {
      // check for the existence of the old net
      // oldNet out of the list
      if (fatherCircuit->netlist == oldNet)
      {
	 fatherCircuit->netlist = nextNet;
	 netInfo = NETEXIST;
      }
      else
      {
	 while (stepper && prevNet)
	 {
	    if (prevNet->next == oldNet)
	    {
	       prevNet->next = nextNet;
	       stepper = 0;
	       netInfo = NETEXIST;
	    }
	    else
	       prevNet = prevNet->next;
	 }
      }

      // net exist but old net is different from the new one (two
      // terminals on the same nets which point to a non NULL cirinstance)
      if (netInfo == NETEXIST)
      {
	 // copy terminals
	 nextTerm = oldNet->terminals;
	 shortTerm->cirport = nextTerm->cirport;  //shortTerminal is replace with
	 shortTerm->cirinst = nextTerm->cirinst;  // a new one; it is deleted
	 shortTerm->next = nextTerm->next;
	 FreeCirportref (nextTerm);
	 nextTerm = shortTerm;
	 while (nextTerm->next)
	 {
	    nextTerm = nextTerm->next;
	    nextTerm->net = newNet;
	 }
	 nextTerm->next = nextNewNetTerm;
	 // change cirport reference
	 cirports = fatherCircuit->cirport;
	 while (cirports)
	 {
	    if (cirports->net == oldNet)
	       cirports->net = newNet;
	    cirports = cirports->next;
	 }
	 //set new number of terminals
	 term_nr += oldNet->num_term;
	 nextTerm->flag.l = term_nr;
	 // delete old instance
	 FreeNet (oldNet);
      }
   }
   else
      nextTerm = deleteTerm (shortTerm);
   bufShort->net = newNet;
   return nextTerm;
}

CIRINSTPTR findInst (CIRINSTPTR prevInst, CIRINSTPTR refInst)
{
   CIRINSTPTR     nextInst, compInst, lastInst;
   MyFlagPtr      myFlag;
   WorkFlagPtr    myWorkFlag;

   // start with this instance - address of fatherInstance
   if (!prevInst->flag.p) { // non flatted instance
      colapsError ("in findInst, can not find circuit instance1");
      return NULL;
   }
   else {
      myFlag = (MyFlagPtr)prevInst->flag.p;
      if (!myFlag->workFlag) { // a primitive instance
	 colapsError ("in findInst, can not find circuit instance2");
	 return prevInst;
      }
   }

   myFlag = (MyFlagPtr)prevInst->flag.p;
   myWorkFlag = myFlag->workFlag;
   lastInst = myWorkFlag->next;
   nextInst = prevInst;
   if (!lastInst)
   {
      while (nextInst)
      {
	 myFlag = (MyFlagPtr)nextInst->flag.p;
	 myWorkFlag = myFlag->workFlag;
	 compInst = myWorkFlag->refInst;
	 if (compInst == refInst) // find the right instance pointer
	    return nextInst;
	 else
	    nextInst = nextInst->next;
      }
   }
   else
   {
      while (nextInst != lastInst)
      {
	 myFlag = (MyFlagPtr)nextInst->flag.p;
	 myWorkFlag = myFlag->workFlag;
	 compInst = myWorkFlag->refInst;
	 if (compInst == refInst) // find the right instance pointer
	    return nextInst;
	 else
	    nextInst = nextInst->next;
      }
   }
   colapsError ("in findInst, can not find circuit instance3");
   return prevInst;
}

// time to add the missing nets
NETPTR addNewNets (NETPTR lastCurNet)
{
   NETPTR          newNet, onlineNet, lastNet;
   CIRCUITPTR      fatherCircuit, lostCircuit;
   CIRINSTPTR      onlineInst;
   CIRPORTPTR      onlineCirport;
   MyFlagPtr       myFlag;
   WorkFlagPtr     myWorkFlag;
   ShortFlagPtr    circuitFlag;
   extern long     netCounter;

   fatherCircuit = lastCurNet->circuit;
   onlineInst = fatherCircuit->cirinst;
   lastNet = lastCurNet;

   if (!fatherCircuit->flag.p)
      NewShortFlag (circuitFlag); // put the last net in the circuit flag
   else
      circuitFlag = (ShortFlagPtr)fatherCircuit->flag.p;
   circuitFlag->net = lastCurNet;// useful for changing the net name at the end
   circuitFlag->name = fatherCircuit->name;
   fatherCircuit->flag.p = (void *)circuitFlag;

   while (onlineInst)
   {
      myFlag = (MyFlagPtr)onlineInst->flag.p;
      if (myFlag && (myWorkFlag = myFlag->workFlag))
      {
	 lostCircuit = myWorkFlag->circuit;
	 onlineNet = lostCircuit->netlist;
	 onlineCirport = lostCircuit->cirport;
	 while (onlineNet)
	 {
	    if (checkNet (onlineNet, onlineCirport, onlineInst))
	    {                    // check for hidden instances
	       NewNet (newNet);  // add them
	       lastNet->next = newNet;
	       // give it a name
	       newNet->name = makeName (fatherCircuit->netlist, lastCurNet);
	       newNet->num_term = onlineNet->num_term;  // copy terminals number
	       newNet->circuit = fatherCircuit;
	       newNet = addNewTerm (newNet, onlineNet, onlineInst); //copy terminals
	       lastNet = newNet;
	    }
	    onlineNet = onlineNet->next;
	 }
	 onlineInst = myWorkFlag->next;  //goto next deleted cirinst
      }
      else
	 onlineInst = onlineInst->next; // if myFlag=NULL it is a non flattened
   }                                  // instance so go to the next
   lastNet->next = NULL;
   return lastNet;
}

//check if a net already exists
int checkNet (NETPTR onlineNet, CIRPORTPTR onlineCirport, CIRINSTPTR inst)
{
   CIRPORTPTR        localCirport;
   CirportFlagPtr    cirportFlag;

   localCirport = onlineCirport;

   if ((strcmp(onlineNet->name, "vdd") == 0) ||
	(strcmp(onlineNet->name, "vss") == 0))
      return ITEXISTS;            // is a vdd or vss net

   while (localCirport)
   {
      if (onlineNet->name == localCirport->name)
      {
	 if (!localCirport->flag.p)
	 {
	    if (onlineNet->num_term > 2)
	       return MAKEIT;
	    else
	       return ITEXISTS;
	 }
	 cirportFlag = (CirportFlagPtr)localCirport->flag.p;
	 while (cirportFlag)    //check if the inst is in the cirport list
	 {
	    if (cirportFlag->inst == inst)
	       return ITEXISTS;
	    cirportFlag = cirportFlag->next;
	 }
	 if (onlineNet->num_term > 2)
	    return MAKEIT;
	 else
	    return ITEXISTS;
      }
      localCirport = localCirport->next;
   }
   return MAKEIT;
}

NETPTR addNewTerm (NETPTR newNet, NETPTR onlineNet, CIRINSTPTR onlineInst)
{
   CIRPORTREFPTR newTerm, onlineTerm, oldTerm = NULL;

   int stepper = 1;
   int term_nr = onlineNet->num_term;

// initializing the first terminal
   onlineTerm = onlineNet->terminals;

   while (stepper && onlineTerm)
   {
      if (onlineTerm->cirinst)  // skip self reference terminals
      {
	 NewCirportref (newTerm);
	 newNet->terminals = newTerm;
	 newTerm->net = newNet;
	 newTerm->cirport = onlineTerm->cirport;
	 newTerm->cirinst = findInst (onlineInst, onlineTerm->cirinst);
	 oldTerm = newTerm;
	 stepper = 0;
      }
      else
	 term_nr -= 1;
      onlineTerm = onlineTerm->next;
   }

// if there are more terminals, copy them too
   while (onlineTerm)
   {
      if (onlineTerm->cirinst)  // skip self reference terminals
      {
	 NewCirportref (newTerm);
	 oldTerm->next = newTerm;
	 newTerm->net = newNet;
	 newTerm->cirport = onlineTerm->cirport;
	 newTerm->cirinst = findInst (onlineInst, onlineTerm->cirinst);
	 oldTerm = newTerm;
      }
      else
	 term_nr -= 1;
      onlineTerm = onlineTerm->next;
   }
   oldTerm->next = NULL;
   newNet->num_term = term_nr;
   return newNet;
}

// the net coresponding to this cirport was already copied so put
// the coresponding instance pointer in the cirport flag
CIRPORTPTR addInstForCirport (CIRINSTPTR inst, CIRPORTPTR cirport)
{
   CirportFlagPtr cirportFlag, nextFlag, prevFlag;

   if (!cirport->flag.p) // first flag in the list
   {
      NewCirportFlag (cirportFlag);
      cirportFlag->inst = inst;
      cirportFlag->next = NULL;
      cirport->flag.p = (void *)cirportFlag;
   }
   else
   {
      prevFlag = (CirportFlagPtr)cirport->flag.p;
      nextFlag = prevFlag;
      while (nextFlag)  // find previous flag
      {
	 prevFlag = nextFlag;
	 nextFlag = nextFlag->next;
      }
      NewCirportFlag (cirportFlag);
      prevFlag->next = cirportFlag;
      cirportFlag->inst = inst;
      cirportFlag->next = NULL;
   }
   return cirport;
}
