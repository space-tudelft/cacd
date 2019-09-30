/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Patrick Groeneveld
 *	Paul Stravers
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

#include "src/ocean/libseadif/sdferrors.h"
#include <string.h>
#include <iostream>
using namespace std;

#include "src/ocean/nelsea/seadifGraph.h"
#include "src/ocean/nelsea/prototypes.h"
#include "src/ocean/nelsea/globals.h" // import the "verbose" variable

static void groupResistorConnectedNets (graphVertexNeighborIterator& allTheResistors);
static void checkCircuitInstances (graphVertexNeighborIterator&);
static int  preprocessCircuitInstances (grCircuit *gcirc, int numberOfPreProcessSteps);
static int  checkCircuitInstancesNonRecursively (graphVertexNeighborIterator&);
static void deleteInstance (grCirInst *);
static void removeDeletedInstances (grCircuit *);
static void removeCirportrefAndMaybeNet (NETPTR *netlist);
static void printStatistics (grCircuit *gcirc, char *id = NULL, int extradeleted = 0);

// The function ghoti() takes a seadif circuit. It creates a "shadow" graph
// that mirrors the seadif circuit. It then runs an algorithm on this graph
// that marks certain instances as "InstanceDeleted". Finally, ghoti updates
// the seadif circuit bij removing the seadif circuit instances whos "shadow"
// in the graph are marked as "InstanceDeleted". The criterion for deleting an
// instance is dependend on the type of the instance. If it is a CMOS
// transistor, the transistor is deleted if at least one of its terminals
// (gate, drain or source) does not connect to another circuit instance or to a
// terminal on the top circuit. The problem of course with this definition is
// that the removal of one transistor may cause its neighbor transistor to also
// become ready for deletion. Even if that was not the case before. To tackle
// this problem, ghoti contains some indirect recursive calls.

void ghoti (grCircuit *gcirc, int ghotiCheck, int numberOfPreProcessSteps)
{
   // set up the booleans that guide instanceMustBeDeleted()...:
   ghotiSetUpDeletePolicy (ghotiCheck);
   // Create an iterator from circuit instances...:
   graphVertexNeighborIterator allTheInstances (gcirc, EdgeToCirInst);
   // Group nets that are connected by resistors...:
   groupResistorConnectedNets (allTheInstances);
   // Before starting ghoti in its recursive mode, we first remove all the
   // instances that are already mature for deletion. This is especially
   // profitable in a sea of gates design with large fields of unused
   // transistors. There you don\'t want to check the neighbors of each unused
   // transistor after you\'ve removed it...:
   if (preprocessCircuitInstances (gcirc, numberOfPreProcessSteps) == TRUE)
   {
      // The last preprocessing step still removed instances, so we are
      // propably not ready yet. Make one final (recursive) pass through all
      // the circuit instances...:
      allTheInstances.initialize (gcirc, EdgeToCirInst);
      checkCircuitInstances (allTheInstances);
      // print statistics
      printStatistics (gcirc, (char*)"exit");
   }
   // remove instances marked as "deleted"
   removeDeletedInstances (gcirc);
   if (!getCurrentGraph()->ckConsistency())
      cout << "\n\nGHOTI INTERNAL ERROR: consistency check failed...\n" << flush;
}

// Check all circuit instances that the nextInstance object returns to see if
// these instances must be deleted. The function instanceMustBeDeleted()
// determines whether an instance shall live or not. If it is decided that an
// instance must be deleted, than all the neigbor instances of this instance
// will also be checked to see it they must be deleted. (Two instances are
// neigbors if they connect to a common net.)
static void checkCircuitInstances (graphVertexNeighborIterator& nextInstance)
{
   graphVertex *vx;
   while ((vx = nextInstance()))
   {
      // A SignalNet also connects to the parent grCircuit, be aware...:
      if (vx->vertexType() != CirInstVertexType) continue;
      grCirInst *gcinst = (grCirInst *)vx; // only now we are sure we can cast
      // If an instance has been deleted, this is marked in its flag...:
      if (!gcinst->flag (InstanceDeleted))
	 // This instance has not been deleted (yet).
	 if (instanceMustBeDeleted (gcinst)) deleteInstance (gcinst);
   }
}

static int preprocessCircuitInstances (grCircuit *gcirc, int numberOfPreProcessSteps)
{
   char buf[256];

   printStatistics (gcirc, (char*)"init");

   graphVertexNeighborIterator allTheInstances (gcirc, EdgeToCirInst);

   for (int n = 1; n <= numberOfPreProcessSteps; ++n)
   {
      allTheInstances.initialize (gcirc, EdgeToCirInst);
      int numdeleted = checkCircuitInstancesNonRecursively (allTheInstances);
      // print statistics
      snprintf (buf, 256, "pre%d", n);
      printStatistics (gcirc, buf, numdeleted);
      if (numdeleted == 0) return 0;
   }
   return TRUE;
}

// This one is like checkCircuitInstances(), but does not contain recursive
// calls. Useful for a quick preprocessing step.
static int checkCircuitInstancesNonRecursively (graphVertexNeighborIterator& nextInstance)
{
   int n = 0;
   graphVertex *vx;

   while ((vx = nextInstance()))
   {
      // A SignalNet also connects to the parent grCircuit, be aware...:
      if (vx->vertexType() != CirInstVertexType) continue;
      grCirInst *gcinst = (grCirInst *)vx; // only now we are sure we can cast
      // If an instance has been deleted, this is marked in its flag...:
      if (!gcinst->flag (InstanceDeleted))
	 // This instance has not been deleted (yet).
	 if (instanceMustBeDeleted (gcinst))
	 {
	    // Marking gcinst as InstanceDeleted is necessary for
	    // removeDeletetInstances() to work properly. Isolating gcinst
	    // speeds up the process of iterating through the instances.
	    gcinst->setFlag (InstanceDeleted);
	    gcinst->isolate(); // delete it from the network and the hierarchy graph
	    n += 1;
	 }
   }
   return n;
}

// deleteInstance() marks the instance as "deleted". It does not really delete
// the instance, because this could confuse the iterators. A consequence of
// marking this instance as deleted is that neighboring instances that only
// escaped deletion because they connected to this instance now also must be
// deleted. This process is recursive.
static void deleteInstance (grCirInst *gcinst)
{
   gcinst->setFlag (InstanceDeleted); // mark the instance as deleted
   graphVertexNeighborIterator nextNeighbor (gcinst, SignalNet);
   checkCircuitInstances (nextNeighbor);
}

// This function deletes all the circuit instances belonging to GCIRC that are
// marked as "deleted".
static void removeDeletedInstances (grCircuit *gcirc)
{
   // We isolate all the instances marked as "deleted" from the graph. This is
   // not really necessary, but is seems a nice thing to do.  For instance, we
   // can use gdesc.print() to check that the graph has been processed
   // correctly.
   // First we create an iterator to enumerate all the circuit instances...:
   graphVertexNeighborIterator nextInstance (gcirc, EdgeToCirInst);
   grCirInst *gcinst = NULL;
   while ((gcinst = (grCirInst *)nextInstance()))
      if (gcinst->flag (InstanceDeleted))
	 gcinst->isolate();	// isolate the instance from the graph

   // All the CIRPORTREFs in all the NETs that reference an instance marked as
   // "deleted" must be removed from their NET.  Then we check this NET to see
   // if it is still of any use. A NET is considered not usefull if it connects
   // to less than two terminals of non-deleted instances.
   CIRCUITPTR circuit = gcirc->circuit();
   removeCirportrefAndMaybeNet (&circuit->netlist);

   // ...and finally we remove all the CIRINSTances from the seadif structure
   // whos grCirInst equivalent are marked as "deleted"...:
   for (CIRINSTPTR prevci = NULL, nextci = NULL, ci = circuit->cirinst; ci; ci = nextci)
   {
      nextci = ci->next;
      if (((grCirInst *)ci->flag.p)->flag (InstanceDeleted))
      {
	 // delete the current circuit instance
         if (!prevci)
            circuit->cirinst = nextci;
         else
            prevci->next = nextci;
	 FreeCirinst (ci);				 // free the storage
      }
      else
	 prevci = ci; // if ci is not deleted then it becomes the "previous ci"
   }
}

// All the CIRPORTREFs in all the NETs that reference an instance marked as
// "deleted" must be removed from their NET.  Then we check this NET to see if
// it is still of any use. A NET is considered not usefull if it connects to
// less than two terminals of non-deleted instances.
static void removeCirportrefAndMaybeNet (NETPTR *netlist)
{
   for (NETPTR prevnet = NULL, nextnet = NULL, net = *netlist; net; net = nextnet)
   {
      nextnet = net->next;	// have to save cause maybe we delete net
      int numberOfInterestingTerminals = 0;
      CIRPORTREFPTR prevcpr = NULL, nextcpr = NULL, cpr = net->terminals;
      for (; cpr; cpr = nextcpr)
      {
	 nextcpr = cpr->next;
	 if (cpr->cirinst && ((grCirInst *)cpr->cirinst->flag.p)->flag (InstanceDeleted))
	 {
	    // Get rid of this cirportref (it refers to a "deleted" instance)
	    ((grCirPortRef *)cpr->flag.p)->clear(); // invalidate ref in grCirPortRef
	    if (!prevcpr)
               net->terminals = cpr->next;
            else
               prevcpr->next = cpr->next;
	    FreeCirportref (cpr);
	    net->num_term -= 1; // I easily forget this one...
	 }
	 else
	 {
	    numberOfInterestingTerminals += 1;
	    prevcpr = cpr; // if cpr is not deleted it becomes the previous cpr
	 }
      }
      if (numberOfInterestingTerminals < 2)
      {
	 // Get rid of this net. First delete all the remaining cirportrefs...:
	 CIRPORTREFPTR nextcpr = NULL;
	 for (cpr = net->terminals; cpr; cpr = nextcpr)
	 {
	    nextcpr = cpr->next;
	    FreeCirportref (cpr);
	    net->num_term -= 1; // I easily forget this one...
	 }
	 // ...and then delete the net...:
	 ((grNet *)net->flag.p)->clear(); // invalidate the reference in grNet
	 if (!prevnet)
            *netlist = net->next;
         else
            prevnet->next = net->next;
	 FreeNet (net);
      }
      else
	 prevnet = net; // if net is not deleted then it becomes the "previous net"
   }
}

// Ghoti is often used to preprocess a network so that the SPICE simulator gets
// safely through its DC analysis. Sinds the only real problem with DC analysis
// is undefined nodes, we tell ghoti that if one side of a resistor has a
// defined voltage level, than the other side is also defined. To deal with
// this, we call nets that are interconnected by resistors "DC-equivalent" and
// we'll have ghoti --sometimes-- treat them as one big supernet.  The function
// below takes all resistors that the allTheResistors iterator returns and
// builds sets of DC-equivalent nets from the nets connected to these
// resistors. On return of this function, each grNet->netSet() either is NULL
// (if no resistors connect it to another grNet) or it is the set of
// DC-equivalent nets that this grNet belongs to.
static void groupResistorConnectedNets (graphVertexNeighborIterator& allTheResistors)
{
   grCirInst *gci = NULL;
   while ((gci = (grCirInst *)allTheResistors()))
   {
      if (gci->cirInst()->circuit != primCirc::res) continue;
      graphVertexEdgeIterator nextNet (gci, SignalNet);
      grNet *n1 = (grNet *)nextNet();	// one side of the resistor...
      grNet *n2 = (grNet *)nextNet();	// ...and the other side!
      if (!n1 || !n2) continue;	// resistor is dangling...
      joinSuperNets (n1, n2);
   }
}

// print statistics to the standard output...:
static void printStatistics (grCircuit *gcirc, char *id, int extradeleted)
{
   if (!verbose) return;
   if (!gcirc) return;

   long numberOfChecks = 0;
   long numberOfInstances = 0;
   long numberOfDeletedInstances = 0;

   graphVertexNeighborIterator allTheInstances (gcirc, EdgeToCirInst);
   grCirInst *gci;

   while ((gci = (grCirInst *)allTheInstances()))
   {
      numberOfInstances += 1;
      numberOfChecks += gci->stat_DeletionCheck (0);
      if (gci->flag (InstanceDeleted)) numberOfDeletedInstances += 1;
   }

   if (id)
      cout << id << " - instances = " << numberOfInstances;
   else
      cout << "instances = " << numberOfInstances;

#if 0
   if (numberOfInstances > 0)
	cout << ", checks/instance = " << float (numberOfChecks) / numberOfInstances;
   else
	cout << ", checks/instance = " << "x";
#endif

   if ((id && strcmp (id, "init") == 0) ||
       (id && strcmp (id, "exit") == 0 && (numberOfDeletedInstances + extradeleted) == 0))
      cout << "\n" << flush;
   else
      cout << ", deleted = " << numberOfDeletedInstances + extradeleted << "\n" << flush;
}
