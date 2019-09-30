/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
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

// The classes defined in sdfgraph.h provide a C++ equivalent of the Seadif
// structure. The function buildCircuitGraph(CIRCUITPTR circ) builds a graph
// that is equivalent to the CIRCUIT graph "circ". The graph classes are
// defined in graph.h and they are documented in the paper "Graph -- A set
// of C++ base classes for hypergraphs" by Paul Stravers.

#ifndef __SDFGRAPH_H
#define __SDFGRAPH_H

enum graphTermType
{
   GenericEdge = 0,
   EdgeToFather = 1, EdgeToSibling = 2, EdgeToChild = 3,
   DirectedEdgeOpen = 4, DirectedEdgeClosed = 5,
   SignalNet = 21, EdgeToCircuit = 22, EdgeToCirInst = 23,
};

#define GRAPHTERMTYPE_DEFINED

enum graphElementType
{
   GenericGraphElementType = 0,
   VertexGraphElementType = 1, EdgeGraphElementType = 2,
   AllGraphElementTypes = -1,
   GCircuitGraphElementType = 10,
   GCirInstGraphElementType = 11,
   GNetGraphElementType = 12,
   GCirPortRefGraphElementType = 13
};

#define GRAPHELEMENTTYPE_DEFINED

enum graphVertexType
{
   GenericVertexType = 0,
   CirInstVertexType = 1,
   CircuitVertexType = 2
};

#define GRAPHVERTEXTYPE_DEFINED

enum graphEdgeType
{
   GenericEdgeType = 0,
   SignalNetEdgeType = 1
};

#define GRAPHEDGETYPE_DEFINED

enum graphTerminalType
{
   GenericTerminalType = 0,
   CirPortRefTerminalType = 1
};

#define GRAPHTERMINALTYPE_DEFINED

#include "src/ocean/libseadif/libstruct.h"
#include "src/ocean/libocean/graph.h"
#include "src/ocean/libocean/sdfset.h"

// In this header we declare/define the following classes:
class sdfGraphDescriptor; // slightly enhanced graphDescriptor
class gCircuit;	   // corresponds to the seadif CIRCUIT
class gCirInst;	   // corresponds to the seadif CIRINST
class gNet;	   // corresponds to the seadif NET
class gCirPortRef; // corresponds to the seadif CIRPORTREF

//////////////////////////////////////////////////////////////////////////////
//			      sdfGraphDescriptor                            //
//////////////////////////////////////////////////////////////////////////////

class sdfGraphDescriptor: public graphDescriptor
{
public:
   sdfGraphDescriptor(char *name = NULL, int setCurrent = 0);
   void *operator new(size_t sz);
   void operator delete(void *p, size_t sz);
   virtual void print();
};

//////////////////////////////////////////////////////////////////////////////
//				   gCircuit				    //
//////////////////////////////////////////////////////////////////////////////

class gCircuit: public graphVertex
{
   CIRCUITPTR circ;
public:
   gCircuit(CIRCUITPTR c) {circ = c; c->linkcnt += 1;}
   ~gCircuit() {circ->linkcnt -= 1;}
   CIRCUITPTR circuit() {return circ;}
   virtual graphElementType graphelementtype() {return GCircuitGraphElementType;}
   virtual graphVertexType vertexType() {return CircuitVertexType;}
   void *operator new(size_t sz);
   void operator delete(void *p, size_t sz);
   virtual void print();
};

//////////////////////////////////////////////////////////////////////////////
//				   gCirInst				    //
//////////////////////////////////////////////////////////////////////////////

class gCirInst: public graphVertex
{
   CIRINSTPTR cinst;
   long       theflag;		// extra storage to support algorithms
public:
   gCirInst(CIRINSTPTR ci) {cinst = ci; theflag = 0L;}
   CIRINSTPTR cirInst() {return cinst;}
   long flag(long mask = -1L) {return theflag & mask;}
   void setFlag(long mask = -1L) {theflag |= mask;}
   void clearFlag(long mask = -1L) {theflag &= ~mask;}
   virtual graphElementType graphelementtype() {return GCirInstGraphElementType;}
   virtual graphVertexType vertexType() {return CirInstVertexType;}
   void *operator new(size_t sz);
   void operator delete(void *p, size_t sz);
   virtual void print();
};

//////////////////////////////////////////////////////////////////////////////
//				     gNet                                   //
//////////////////////////////////////////////////////////////////////////////

class gNet: public graphEdge
{
   NETPTR nt;
public:
   gNet(NETPTR n) { nt = n; }
   NETPTR net() { return nt; }
   void clear() { nt = NULL; }
   virtual graphElementType graphelementtype() { return GNetGraphElementType; }
   virtual graphEdgeType edgeType() { return SignalNetEdgeType; }
   void *operator new(size_t sz);
   void operator delete(void *p, size_t sz);
   virtual void print();
   virtual void printElement();
};

//////////////////////////////////////////////////////////////////////////////
//				  gCirPortRef                               //
//////////////////////////////////////////////////////////////////////////////

class gCirPortRef: public graphTerminal
{
   CIRPORTREFPTR cirportref;
public:
   gCirPortRef(CIRPORTREFPTR cpr);
   CIRPORTREFPTR cirPortRef() {return cirportref;}
   void clear() {cirportref = NULL;}
   virtual graphTerminalType terminalType() { return CirPortRefTerminalType; }
   void *operator new(size_t sz);
   void operator delete(void *p, size_t sz);
   virtual void print();
};

//////////////////////////////////////////////////////////////////////////////
//		       P U B L I C    F U N C T I O N S                     //
//////////////////////////////////////////////////////////////////////////////

// The function buildCircuitGraph() builds a C++ graph that corresponds to the
// seadif CIRCUIT.  The objects in the C++ graph point to their corresponding
// seadif objects.  Only four seadif objects have a corresponding C++ object:
//
//      struct CIRCUIT     <-->  class gCircuit      (a graphVertex)
//      struct CIRINST     <-->  class gCirInst      (a graphVertex)
//      struct NET         <-->  class gNet          (a graphEdge)
//      struct CIRPORTREF  <-->  class gCirPortRef   (a graphTerminal)
//
// A call to buildCircuitGraph() returns a pointer to an object of the class
// gCircuit. All the FLAG.p fields of the seadif structs point to their
// corresponding graph object.
//
// The last five arguments optionally specify a function that create new graph
// objects. They are provided so that you can derive new classes from the base
// classes gCircuit, gCirInst, gNet and gCirPortRef and still use
// buildCircuitGraph() with these new classes.  If you do not specify such
// creation function (i.e. you specify NULL) then an internal default function
// (new) is taken.
//
gCircuit *buildCircuitGraph(CIRCUITPTR circuit,
			    gCircuit *(*new_gCircuit)(CIRCUITPTR) = NULL,
			    gCirInst *(*new_gCirInst)(CIRINSTPTR) = NULL,
			    gNet *(*new_gNet)(NETPTR) = NULL,
			    gCirPortRef *(*new_gCirPortRef)(CIRPORTREFPTR) = NULL,
			    graphEdge *(*new_gHierarchy)(void) = NULL
			    );

#endif
