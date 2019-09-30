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

#ifndef __GRAPH_H
#define __GRAPH_H

// #include "src/ocean/libocean/Object.h"
#include <iostream>
using namespace std;

#ifndef TRUE
#define TRUE 1
#endif

// In this file we declare the following classes:
class graphDescriptor;                // container for a graph
class graphElement;		      // element of a graph, eg vertex and edge
class graphElementIterator;	      // iterates through graph elements
class graphVertex;		      // a vertex in a graph
class graphEdge;		      // an edge in a graph
class graphTerminal;		      // glues a vertex to an edge
class graphVertexNeighborIterator;    // iterates neighbors of a vertex
class graphVertexEdgeIterator;	      // iterates edges connected to a vertex
class graphVertexCommonEdgeIterator;  // iterates edges common to 2 vertices
class graphEdgeVertexIterator;	      // iterates vertices connected to an edge
class graphVertexTerminalIterator;    // iterates terminals of a vertex
class graphEdgeTerminalIterator;      // iterates terminals in an edge

// Furthermore, we define the following enums:
// enum graphTermType;
// enum graphElementType;
// enum graphVertexType;

//////////////////////////////////////////////////////////////////////////////

// The following enum is the default graphTermType. The user can replace this
// enum by her own enum before including graph.h if she also #define's
// GRAPHTERMTYPE_DEFINED.

#ifndef GRAPHTERMTYPE_DEFINED
enum graphTermType
{
   GenericEdge = 0,
   EdgeToFather = 1, EdgeToSibling = 2, EdgeToChild = 3,
   DirectedEdgeOpen = 4, DirectedEdgeClosed = 5,
   EdgeType1 = 11, EdgeType2 = 12, EdgeType3 = 13,
   EdgeType4 = 14, EdgeType5 = 15, EdgeType6 = 16
};
#endif

const int depthOfGraphDescriptorStack = 4; // not enough for recursion...

//////////////////////////////////////////////////////////////////////////////
//                              graphDescriptor                             //
//////////////////////////////////////////////////////////////////////////////
// We only have one graphDescriptor for each graph.

const int SetCurrentGraph = 1;

class graphDescriptor
{
private:
   graphElement           *first,*last;
   char                   *name;
   // New Vertices and Edges are always linked into thecurrentgraph
   static graphDescriptor *thecurrentgraph;
   static int             thestackpointer;
   static graphDescriptor *thestack[depthOfGraphDescriptorStack];
   friend graphDescriptor *getCurrentGraph(),*popCurrentGraph();
   friend class graphElementIterator;
protected:
   int                    elementcount; // the length of the graph element list
   virtual void printHeader() {;}	// additional info from derived class
public:
   graphDescriptor(char* = NULL, int setCurrent = 0);
   virtual ~graphDescriptor();
   graphDescriptor *addElement(graphElement *);
   graphDescriptor *removeElement(graphElement *);
   void setCurrentGraph();	// make this descriptor ``current''
   void pushCurrentGraph();	// likewise, but undo with popCurrentGraph()
   virtual void print();
   void setName(char *);	// associates a string with this descriptor
   char *getName();		// returns a string associated with descriptor
   virtual int ckConsistency();
};

// These are friend functions of class graphDescriptor:
graphDescriptor *getCurrentGraph();
graphDescriptor *popCurrentGraph();

//////////////////////////////////////////////////////////////////////////////
//				 graphElement                               //
//////////////////////////////////////////////////////////////////////////////
// This class provides a common container for graph vertices and edges. It
// links all elements of a graph in a single list.

#ifndef GRAPHELEMENTTYPE_DEFINED
enum graphElementType
{
   GenericGraphElementType = 0,
   VertexGraphElementType = 1, EdgeGraphElementType = 2,
   AllGraphElementTypes = -1
};
#endif // GRAPHELEMENTTYPE_DEFINED

class graphElement // : public Object
{
   graphElement   *next,*prev;
   friend         class graphDescriptor;
   friend         class graphElementIterator;
public:
   graphElement();
   virtual ~graphElement();
   virtual void printElement() {cout << "<graphElement>" << flush;};
   virtual graphElementType graphelementtype() {return GenericGraphElementType;}
   virtual int ckConsistency() {return 0;}
};

//////////////////////////////////////////////////////////////////////////////
//			     graphElementIterator                           //
//////////////////////////////////////////////////////////////////////////////
// This iterator successively returns the graph elements of the graph
// descriptor it was initialize()d with, or of the graph descriptor that was
// passed as an argument to the constructor. Use of this iterator object hides
// the implementation details of the graph, and it helps in writing clean code.
// Example:
//
//   graphElementIterator nextVertex(&gdesc,VertexGraphElementType);
//   graphElement *e;
//   while (e = nextElement())     // iterate trough the vertices of graph gdesc
//      e->printElement();       // print each vertex in the graph
//

class graphElementIterator
{
   graphElement     *currentelm; // next element to be returned by iterator
   graphElementType thetype;	 // type of elements that iterator returns
public:
   graphElementIterator(graphDescriptor *,
			graphElementType = AllGraphElementTypes);
   void initialize(graphDescriptor *,graphElementType = AllGraphElementTypes);
   graphElement *operator()();	              // return the next graph element
   int more() {return currentelm != NULL;}	           // TRUE if any element left
   void skip(graphElement*);
};

//////////////////////////////////////////////////////////////////////////////
//				 graphVertex                                //
//////////////////////////////////////////////////////////////////////////////
// The following enum is the default. The user can replace this enum by her own
// enum before including graph.h if she also #define's GRAPHVERTEXTYPE_DEFINED.

#ifndef GRAPHVERTEXTYPE_DEFINED
enum graphVertexType
{
   GenericVertexType = 0
};
#endif // GRAPHVERTEXTYPE_DEFINED

class graphVertex : public graphElement
{
   graphTerminal *first,*last;	// first and last terminal in the vertex
   graphTerminal *firstTerm(graphTermType = GenericEdge);
   graphTerminal *lastTerm(graphTermType = GenericEdge);
   graphEdge	 *firstEdge(graphTermType = GenericEdge);
   graphEdge	 *lastEdge(graphTermType = GenericEdge);
   graphEdge	 *nextEdge(graphEdge *);
   graphEdge	 *prevEdge(graphEdge *);
   friend        class graphEdge;
   friend        class graphTerminal;
   friend        class graphVertexEdgeIterator;
   friend        class graphVertexTerminalIterator;
   friend        class graphVertexNeighborIterator;
   friend        class graphVertexCommonEdgeIterator;
public:
   graphVertex();
   virtual ~graphVertex();
   graphVertex *addToVertex(graphTerminal *newTerm);
   int hasEdgeTo(graphVertex *, graphTermType = GenericEdge);
   void isolate();	// Isolates this vertex from the other graph components
   virtual int vWeight() {return 1;}
   virtual graphVertexType vertexType() { return GenericVertexType; }
   virtual graphElementType graphelementtype() {return VertexGraphElementType;}
   virtual void print();
   virtual void printElement(); // from base class graphElement
   virtual int  ckConsistency();
};

//////////////////////////////////////////////////////////////////////////////
//				graphTerminal                               //
//////////////////////////////////////////////////////////////////////////////
// The following enum is the default. The user can replace this enum by her own
// enum before including graph.h if she also #define's GRAPHEDGETYPE_DEFINED.

#ifndef GRAPHTERMINALTYPE_DEFINED
enum graphTerminalType
{
   GenericTerminalType = 0
};
#endif

class graphTerminal
{
   graphVertex   *thisvertex;
   graphEdge     *thisedge;
   graphTerminal *nextinthisvertex,*previnthisvertex,
                 *nextinthisedge,*previnthisedge;
   graphTermType termtype;
   friend class graphEdge;
   friend class graphVertex;
public:
   graphTerminal(graphTermType ttype = GenericEdge);
   virtual ~graphTerminal();
   graphVertex    *thisVertex() {return thisvertex;}
   graphEdge      *thisEdge() {return thisedge;}
   graphTermType  type() {return termtype;}
   graphTerminal  *nextInThisVertex(graphTermType);
   graphTerminal  *prevInThisVertex(graphTermType);
   graphTerminal  *nextInThisEdge() {return nextinthisedge;}
   graphTerminal  *prevInThisEdge() {return previnthisedge;}
   graphTerminal  *insertInEdge(graphTerminal *);
   graphTerminal  *insertInVertex(graphTerminal *);
   graphTerminal  *removeFromEdge();
   graphTerminal  *removeFromVertex();
   virtual graphTerminalType terminalType() { return GenericTerminalType; }
   virtual void   print();
};

//////////////////////////////////////////////////////////////////////////////
//				  graphEdge                                 //
//////////////////////////////////////////////////////////////////////////////
// The following enum is the default. The user can replace this enum by her own
// enum before including graph.h if she also #define's GRAPHEDGETYPE_DEFINED.

#ifndef GRAPHEDGETYPE_DEFINED
enum graphEdgeType
{
   GenericEdgeType = 0
};
#endif

class graphEdge : public graphElement
{
   friend class graphTerminal;
   graphTerminal *first,*last;
protected:
   virtual graphTerminal *newTerminal() {return new graphTerminal;}
public:
   graphEdge();
   virtual ~graphEdge();
   graphTerminal *firstTerminal();
   graphTerminal *lastTerminal();
   graphEdge     *addToEdge(graphVertex *, graphTermType = GenericEdge,
			    graphTerminal * = NULL);
   graphEdge     *addToEdge(graphTerminal *);     // initialize term yourself
   graphEdge     *removeFromEdge(graphVertex *);
   graphVertex   *otherSide(graphVertex *); // useful if an edge has 2 terms
   int           numberOfTerminals(); // this is most likely also #vertices
   virtual int   eWeight() {return 1;}
   virtual graphEdgeType edgeType() { return GenericEdgeType; }
   virtual graphElementType graphelementtype() {return EdgeGraphElementType;}
   virtual void  printElement(); // from base class graphElement
   virtual void  print();
   virtual int   ckConsistency();
};

//////////////////////////////////////////////////////////////////////////////
//			  graphVertexNeighborIterator                       //
//////////////////////////////////////////////////////////////////////////////
// This iterator successively returns the neighbor vertices of the vertex
// it was initialize()d with, or of the vertex that was passed as an argument
// to the constructor. Use of this iterator object hides the implementation
// details of the graph, and it helps in writing clean code. Example:
//
//   graphVertexNeighborIterator nextNeighbor(&v1); // create iterator object
//   graphVertex *v;
//   while (v = nextNeighbor()) // iterate trough the neighbors of v1
//      v->print(); // print each neighbor
//

class graphVertexNeighborIterator
{
   graphTerminal *currentneighborterminal;
   graphTerminal *currentthisterminal;
   graphTermType termout,termin; // this filters the edges
   graphVertex   *thevertex;
public:
   graphVertexNeighborIterator(graphVertex *, graphTermType thisSide = GenericEdge,
			       graphTermType otherSide = GenericEdge);
   void initialize(graphVertex *, graphTermType thisSide = GenericEdge,
		   graphTermType otherSide = GenericEdge);        // reset
   graphVertex *operator()();	            // return the next neighbor vertex
   int more() {return currentneighborterminal != NULL;} // TRUE if anything left
   void skip(graphVertex*);
};

//////////////////////////////////////////////////////////////////////////////
//			   graphVertexEdgeIterator                          //
//////////////////////////////////////////////////////////////////////////////
// Example:
//
//   graphVertexEdgeIterator nextEdge(&v2); // create an iterator object
//   while (e = nextEdge()) // iterate through the edge around graphVertex v2
//      e->print();         // print each edge
//

class graphVertexEdgeIterator
{
   graphTerminal *currentterminal;
   graphTermType tt;
public:
   graphVertexEdgeIterator(graphVertex *, graphTermType = GenericEdge);
   void initialize(graphVertex *, graphTermType = GenericEdge);       // reset
   graphEdge *operator()();	                      // return the next edge
   int more() {return currentterminal != NULL;} //TRUE if there is anything left
   void skip(graphEdge*);
};

//////////////////////////////////////////////////////////////////////////////
//			graphVertexCommonEdgeIterator                       //
//////////////////////////////////////////////////////////////////////////////
class graphVertexCommonEdgeIterator
{
   graphVertex   *v2;
   graphTerminal *currentterminal;
   graphTermType tt;
public:
   graphVertexCommonEdgeIterator(graphVertex *, graphVertex *,
				 graphTermType = GenericEdge);
   void initialize(graphVertex *, graphVertex *, graphTermType = GenericEdge);
   graphEdge *operator()();	               // return the next common edge
   int more() {return currentterminal != NULL;} //TRUE if there is anything left
   void skip(graphEdge*);
};

//////////////////////////////////////////////////////////////////////////////
//			    graphEdgeVertexIterator                         //
//////////////////////////////////////////////////////////////////////////////
class graphEdgeVertexIterator
{
   graphTerminal *currentterminal;
   graphTermType tt;
public:
   graphEdgeVertexIterator(graphEdge *, graphTermType = GenericEdge);
   void initialize(graphEdge *, graphTermType = GenericEdge);
   graphVertex *operator()();	        // return the next vertex in the edge
   int more() {return currentterminal != NULL;} //TRUE if there is anything left
   void skip(graphVertex*);
};

//////////////////////////////////////////////////////////////////////////////
//			  graphVertexTerminalIterator                       //
//////////////////////////////////////////////////////////////////////////////
class graphVertexTerminalIterator
{
   graphTerminal *currentterminal;
   graphTermType tt;
public:
   graphVertexTerminalIterator(graphVertex *, graphTermType = GenericEdge);
   void initialize(graphVertex *, graphTermType = GenericEdge);
   graphTerminal *operator()();	    // return the next terminal in the vertex
   int more() {return currentterminal != NULL;} //TRUE if there is anything left
   void skip(graphTerminal*);
};

//////////////////////////////////////////////////////////////////////////////
//			   graphEdgeTerminalIterator                        //
//////////////////////////////////////////////////////////////////////////////
class graphEdgeTerminalIterator
{
   graphTerminal *currentterminal;
   graphTermType tt;
public:
   graphEdgeTerminalIterator(graphEdge *, graphTermType = GenericEdge);
   void initialize(graphEdge *, graphTermType = GenericEdge);
   graphTerminal *operator()();	      // return the next terminal in the edge
   int more() {return currentterminal != NULL;} //TRUE if there is anything left
   void skip(graphTerminal*);
};

void graphError(const char *msg = "unknown error");

#endif
