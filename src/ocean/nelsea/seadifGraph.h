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

#ifndef __SEADIFGRAPH_H
#define __SEADIFGRAPH_H

// This file seadifGraph.h defines a set of objects that represent the C++
// version of a seadif circuit network. SeadifGraph.h borrows almost everything
// from sdfGraph.h. The only difference is that it replaces the gNet class with
// grNet. A grNet provides functionality for DC-equivalent nets, a feature that
// we need for ghoti.

#include "src/ocean/libocean/sdfGraph.h"

// In this header we define the following classes:
class grCircuit;             // like gCircuit, but with statistics
class grCirInst;	     // like gCirInst, but with statistics
class grCirPortRef;	     // like gCirPortRef, but with statistics
class grNet;	             // like gNet, but knows about superNets
class superNet;	             // groups DC-equivalent nets
class ghotiSuperNetIterator; // iterates through DC-equivalent nets
class primCirc;		     // only contains primitive circuits

const long InstanceDeleted = 1;	// used as an argument to the gCirInst::flag()

//////////////////////////////////////////////////////////////////////////////
//				   grCircuit                                //
//////////////////////////////////////////////////////////////////////////////
class grCircuit: public gCircuit
{
   long stat_numberOfDeletionChecks;
public:
   grCircuit(CIRCUITPTR c) : gCircuit(c) { stat_numberOfDeletionChecks = 0; }
   long stat_DeletionCheck(int n =1) { return stat_numberOfDeletionChecks += n; }
   void *operator new(size_t sz);
   void operator delete(void *p, size_t sz);
};

//////////////////////////////////////////////////////////////////////////////
//				   grCirInst                                //
//////////////////////////////////////////////////////////////////////////////
class grCirInst: public gCirInst
{
   long stat_numberOfDeletionChecks;
public:
   grCirInst(CIRINSTPTR ci) : gCirInst(ci) { stat_numberOfDeletionChecks = 0; }
   long stat_DeletionCheck(int n =1) { return stat_numberOfDeletionChecks += n; }
   void *operator new(size_t sz);
   void operator delete(void *p, size_t sz);
};

//////////////////////////////////////////////////////////////////////////////
//				 grCirPortRef                               //
//////////////////////////////////////////////////////////////////////////////
class grCirPortRef: public gCirPortRef
{
   // no extensions to gCirPortRef (yet)
public:
   grCirPortRef(CIRPORTREFPTR cpr) : gCirPortRef(cpr) {}
   void *operator new(size_t sz);
   void operator delete(void *p, size_t sz);
};

//////////////////////////////////////////////////////////////////////////////
//				     grNet                                  //
//////////////////////////////////////////////////////////////////////////////
// We must be able to build a set of grNet objects, that's why we derive from
// setElmt. A set of grNet objects is called a ``superNet'';   it is a set of
// DC-equivalent nets...:
//
class grNet: public gNet, public sdfsetElmt
{
   superNet *thisDcEquiv;    // set of DC-equivalent grNets that we belong to
   graphVertex *cache;	     // caches an interesting vertex that we connect to
public:
   grNet(NETPTR n);
   superNet *dcEquiv() { return thisDcEquiv; }
   superNet *dcEquiv(superNet *dcEq) { return thisDcEquiv = dcEq; }
   graphVertex *cacheContents();
   void *operator new(size_t sz);
   void operator delete(void *p, size_t sz);
   virtual void print() { gNet::print(); sdfsetElmt::print(); }
};

//////////////////////////////////////////////////////////////////////////////
//				     superNet                               //
//////////////////////////////////////////////////////////////////////////////
// A superNet is a set of DC-equivalent grNet objects. We call two grNets DC-
// equivalent if they are connected by a resistor, or by a series chain of
// resistors.  Just for fun, the superNet class keeps track of the set of
// superNets in the static variable superNet::theSuperNets (This is therefore a
// set of sets...). Consequently, we derive from "set" to build a set of grNet
// objects, and we derive from "setElmt" to build a set of superNet objects.
//
class superNet: public sdfset, public sdfsetElmt
{
public:
   static sdfset theSuperNets;	          // here we collect all the superNets
   superNet() { theSuperNets.add(this); } // auto-add to the set of superNets
   ~superNet() { theSuperNets.remove(this); } // auto-remove
   void *operator new(size_t sz);
   void operator delete(void *p, size_t sz);
   virtual void print() { sdfset::print(); sdfsetElmt::print(); }
};

//////////////////////////////////////////////////////////////////////////////
//			     ghotiSuperNetIterator                          //
//////////////////////////////////////////////////////////////////////////////
// This iterator returns all the vertices that connect to a ``supernet''. The
// iterator is initialized by any grNet that is part of the supernet.
//
class ghotiSuperNetIterator
{
   graphEdgeVertexIterator vertexInNetIterator;
   sdfsetNextElmtIterator netInSetIterator;
   superNet *dcEquivalentNets;
public:
   ghotiSuperNetIterator(grNet *);
   void initialize(grNet *);
   void *operator new(size_t sz);
   void operator delete(void *p, size_t sz);
   graphVertex *operator()();
};

//////////////////////////////////////////////////////////////////////////////
//				   primCirc                                 //
//////////////////////////////////////////////////////////////////////////////
// The main purpose of this class is not to spoil the global namespace. A
// reference to a primitive circuit now looks like primCirc::nenh ...:
class primCirc
{
public:
   static CIRCUITPTR nenh;
   static CIRCUITPTR penh;
   static CIRCUITPTR res;
   static CIRCUITPTR cap;
   static STRING cmosGate;	// name of the gate
   static STRING cmosSource;	// name of the source
   static STRING cmosDrain;	// name of the drain
};

//////////////////////////////////////////////////////////////////////////////
//		       P U B L I C    F U N C T I O N S                     //
//////////////////////////////////////////////////////////////////////////////
// The function joinSuperNets(N1,N2) joins the sets associated with the grNets
// N1 and N2. If no sets are associated with N1 and N2 a new set is created.
// This function also assures that grNet.dcEquiv() returns the set that grNet
// currently belongs to. All this superNet business is usefull for grouping
// nets that are DC-equivalent (meaning that there exists a DC-path between
// such grNets).
//
void joinSuperNets(grNet *n1, grNet *n2);

gCircuit    *new_grCircuit(CIRCUITPTR);
gCirInst    *new_grCirInst(CIRINSTPTR);
gCirPortRef *new_grCirPortRef(CIRPORTREFPTR);
gNet        *new_grNet(NETPTR);

#endif
