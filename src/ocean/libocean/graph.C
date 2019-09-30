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

#include <iostream>
using namespace std;
#include <string.h>
#include <stdlib.h>
#include "src/ocean/libocean/graph.h"

///////////////////////////////////////////////////////////////////////////////
//				graphDescriptor                              //
///////////////////////////////////////////////////////////////////////////////

// definition and initialization of static members
graphDescriptor *graphDescriptor::thecurrentgraph = NULL;

int graphDescriptor::thestackpointer = -1;

graphDescriptor *graphDescriptor::thestack[depthOfGraphDescriptorStack];

// Constructor
graphDescriptor::graphDescriptor (char *thename, int setCurrent)
{
   first = last = NULL;
   elementcount = 0;
   name = NULL;
   setName (thename);
   if (setCurrent == SetCurrentGraph) setCurrentGraph();
}

void graphDescriptor::setName (char *thename)
{
   if (name) delete name; // first delete the previous name

   if (!thename) name = NULL;
   else {
      name = new char[strlen(thename) + 1];
      strcpy (name, thename);
   }
}

char *graphDescriptor::getName()
{
   return name;
}

graphDescriptor::~graphDescriptor()
{
   // Here we clean up the entire graph...
   graphElementIterator nextElm (this, AllGraphElementTypes);
   graphElement *e;
   while ((e = nextElm()))
   {
      // To see if this is the last reference to e we first have to unref()
      // it and then test canBeDeleted().
      //$e->unref();
      //$if (e->canBeDeleted())
      //${
	 //$e->ref();
	 //$delete e;		// destructor for e calls removeElement()
      //$}
      //$else
      //${
	 //$e->ref();
	 removeElement(e);
      //$}
   }
}

graphDescriptor *graphDescriptor::addElement (graphElement *gc)
{
   if (gc)
   {
      gc->next = NULL;
      gc->prev = last;
      if (last) last->next = gc;
      last = gc;
      if (!first) first = gc;
      elementcount += 1;	// Keep track of total number of elements
      //$gc->ref();			// reference to Object base class
   }
   return this;
}

// remove a graph element from the descriptor. Do NOT DELETE gc.
graphDescriptor *graphDescriptor::removeElement (graphElement *gc)
{
   if (gc)
   {
      if (!gc->prev)
      {
	 // special case, gc is the first graph element in the list
	 if (first != gc) graphError ("graph element not in this graphDescriptor");
	 first = gc->next;
	 if (!first)	// check for only one element in the list
	    last = NULL;
	 else
	    first->prev = NULL;
      }
      else if (!gc->next)
      {
	 // special case, gc is last graph element in the list
	 last = gc->prev; // prev MUST be != NULL since we tested this above...
	 last->next = NULL;
      }
      else
      {
	 // general case, gc is in the middle of the graph element list
	 gc->prev->next = gc->next;
	 gc->next->prev = gc->prev;
      }
      gc->next = gc->prev = NULL;
      elementcount -= 1;	// Keep track of total number of elements
      //$gc->unref();		// reference to Object base class
   }
   return this;
}

void graphDescriptor::setCurrentGraph()
{
   thecurrentgraph = this;
}

// This is a friend function of the graphDescriptor class
graphDescriptor *getCurrentGraph()
{
   return graphDescriptor::thecurrentgraph;
}

void graphDescriptor::pushCurrentGraph()
{
   if (++thestackpointer >= depthOfGraphDescriptorStack)
      graphError ("graphDescriptor::pushCurrentGraph() -- stack overflow");
   thestack[thestackpointer] = getCurrentGraph();
   setCurrentGraph();
}

// This is a friend function of the graphDescriptor class
graphDescriptor *popCurrentGraph()
{
   if (graphDescriptor::thestackpointer < 0)
      graphError ("graphDescriptor::popCurrentGraph() -- stack underflow");
   graphDescriptor *tos = getCurrentGraph();
   graphDescriptor::thestack[graphDescriptor::thestackpointer--]->setCurrentGraph();
   return tos;
}

// print the contents of a graph
void graphDescriptor::print()
{
   cout << "GRAPH \"" << (!name ? "<unnamed>" : name) << "\"" << flush;
   printHeader();
   cout << " {\n";
   graphElementIterator nextEdge (this, EdgeGraphElementType);
   graphElement *e;
   while ((e = nextEdge()))	// for all edges in the this graph...
      e->printElement();	// ...print 'm!
   cout << "}\n" << flush;
}

int graphDescriptor::ckConsistency()
{
   int n = 0;
   graphElement *olde = NULL;
   for (graphElement *e = first; e; e = e->next)
   {
      if (e->prev != olde) {
	 cout << "graphDescriptor::ckConsistency --- error in prev\n" << flush;
	 return 0;
      }
      if (!e->ckConsistency()) return 0;
      olde = e;
   }
   return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//				 graphElement                                //
///////////////////////////////////////////////////////////////////////////////

// Constructor
graphElement::graphElement()
{
   next = prev = (graphElement*)0;
   getCurrentGraph()->addElement(this);
}

// Destructor
graphElement::~graphElement()
{
   // this is a little tricky: if we first explicitly remove a graphElement
   // and then later delete it, we must not remove it again from the graph
   // descriptor.
   if (!this->prev && !this->next)
      ; // do not remove! this one does not belong to the any graph
   else
      getCurrentGraph()->removeElement(this);
}

//////////////////////////////////////////////////////////////////////////////
//			     graphElementIterator                           //
//////////////////////////////////////////////////////////////////////////////

// Constructor
graphElementIterator::graphElementIterator (graphDescriptor *desc, graphElementType etype)
{
   initialize (desc, etype);
}

// This iterator always contains the next element to be returned. The function
// initialize() sets currentelm to the element to be returned next.
void graphElementIterator::initialize (graphDescriptor *desc, graphElementType etype)
{
   if (!desc) {
      currentelm = NULL;
      return;
   }
   currentelm = desc->first;
   thetype = etype;
   if (thetype != AllGraphElementTypes)
      while (currentelm && currentelm->graphelementtype() != thetype)
	 currentelm = currentelm->next;
}

// Successively returns the next graphElement from the graphDescriptor
graphElement *graphElementIterator::operator() ()
{
   graphElement *elm = currentelm; // elm will be returned
   if (currentelm)
   {
      currentelm = currentelm->next;
      if (thetype != AllGraphElementTypes)
	 while (currentelm && currentelm->graphelementtype() != thetype)
	    currentelm = currentelm->next;
   }
   return elm;
}

// if ELM is the next thing to return, then forget about it and advance the
// iterator to the next thing. This is a useful method to call when we are
// deleting elements of a graph while iterating them at the same time.
void graphElementIterator::skip (graphElement* elm)
{
   if (elm == currentelm) operator()();
}

///////////////////////////////////////////////////////////////////////////////
//				  graphVertex                                //
///////////////////////////////////////////////////////////////////////////////

graphVertex::graphVertex()
{
   first = last = NULL;
}

// The destructor of a graphVertex
graphVertex::~graphVertex()
{
   // here we clean up all the terminals connected to this vertex.
   graphTerminal *next_t;
   for (graphTerminal *t = first; t; t = next_t)
   {
      next_t = t->nextinthisvertex;
      delete t;
   }
}

// Add a terminal to this vertex:
graphVertex *graphVertex::addToVertex (graphTerminal *newTerm)
{
   if (newTerm->nextinthisvertex || newTerm->previnthisvertex || newTerm->thisvertex)
      graphError ("graphVertex::addToVertex() -- newTerm already in other vertex");
   if (!last)
   {
      first = last = newTerm;
      newTerm->thisvertex = this;
   }
   else
      last = last->insertInVertex(newTerm);
   return this;
}

// Return 0 if supposedNeighbor and this vertex share no common Edge.  Return
// TRUE if supposedNeighbor is a neigbor of this vertex.
int graphVertex::hasEdgeTo (graphVertex *supposedNeighbor, graphTermType tt)
{
   graphVertexCommonEdgeIterator commonEdge (this, supposedNeighbor, tt);
   return commonEdge() != NULL;
}

graphTerminal *graphVertex::firstTerm (graphTermType tt)
{
   graphTerminal *t = first;
   if (tt != GenericEdge)
      while (t && t->type() != tt) t = t->nextinthisvertex;
   return t;
}

graphTerminal *graphVertex::lastTerm (graphTermType tt)
{
   graphTerminal *t = last;
   if (tt != GenericEdge)
      while (t && t->type() != tt)
	 t = t->previnthisvertex;
   return t;
}

// Return a pointer to the first edge of this vertex
graphEdge *graphVertex::firstEdge (graphTermType tt)
{
   graphTerminal *t = firstTerm(tt);
   if (!t)
      return NULL;
   else
      return t->thisEdge();
}

// Return a pointer to the last edge of this vertex
graphEdge *graphVertex::lastEdge (graphTermType tt)
{
   graphTerminal *t = last;
   if (t) t = t->prevInThisVertex(tt);
   if (!t)
      return NULL;
   else
      return t->thisEdge();
}

// Return a pointer to the next edge of this vertex
graphEdge *graphVertex::nextEdge (graphEdge *thisedge)
{
   if (!thisedge)
      graphError ("graphVertex::nextEdge() -- this edge is NULL");
   // first we have to find the terminal that points to thisvertex
   graphTerminal *t = thisedge->firstTerminal();
   for (; t && t->thisVertex() != this; t = t->nextInThisEdge())
      ;
   if (!t)
      graphError ("graphVertex::nextEdge() -- edge not incident to vertex");
   // look for the next terminal with the same type as the current terminal
   graphTermType tt = t->type();
   t = t->nextInThisVertex(tt);
   if (!t)
      return NULL;		// this edge already was last edge in vertex
   graphEdge *e;
   if (!(e = t->thisEdge()))
      graphError ("graphVertex::nextEdge() -- terminal without edge");
   return e;
}

// Return a pointer to the previous edge of this vertex
graphEdge *graphVertex::prevEdge (graphEdge *thisedge)
{
   if (!thisedge)
      graphError ("graphVertex::prevEdge() -- this edge is NULL");
   // first we have to find the terminal that points to thisvertex
   graphTerminal *t = thisedge->firstTerminal();
   for (; t && t->thisVertex() != this; t = t->nextInThisEdge())
      ;
   if (!t)
      graphError ("graphVertex::prevEdge() -- edge not incident to vertex");
   // look for the previous terminal with the same type as the current terminal
   graphTermType tt = t->type();
   t = t->prevInThisVertex(tt);
   if (!t)
      return NULL;		// this edge already was first edge in vertex
   graphEdge *e;
   if (!(e = t->thisEdge()))
      graphError ("graphVertex::prevEdge() -- terminal without edge");
   return e;
}

// disconnect this vertex from all edges in the graph
void graphVertex::isolate()
{
   for (graphTerminal *nextt = NULL, *t = first; t; t = nextt)
   {
      nextt = t->nextinthisvertex;
      t->removeFromVertex()->removeFromEdge();
      delete t;
   }
}

void graphVertex::print()
{
   cout << "<graphVertex>" << flush;
}

// redefinition of graphElement::printElement()
void graphVertex::printElement()
{
   cout << "graphVertex { ";
   graphTerminal *t = first;
   while (t)
   {
      t->thisEdge()->print();
      if (t->nextInThisVertex(GenericEdge))
	 cout << ", " ;
      t = t->nextInThisVertex(GenericEdge);
   }
   cout << " }\n";
}

// Perform a consistency check on the vertex datastructure.
int graphVertex::ckConsistency()
{
   if (!first && !last) return TRUE;
   if (!first || !last) {
      cout << "graphVertex::ckConsistency --- first/last inconsistency\n" << flush;
      return 0; // one 0, but not the other...
   }
   if (first->previnthisvertex) {
      cout << "graphVertex::ckConsistency --- first->previnthisvertex != NULL\n" << flush;
      return 0;
   }
   if (last->nextinthisvertex) {
      cout << "graphVertex::ckConsistency --- last->nextinthisvertex != NULL\n" << flush;
      return 0;
   }

   graphTerminal *prev = NULL;

   for (graphTerminal *t = first; t; t = t->nextinthisvertex)
   {
      if (t->thisvertex != this) {
	 cout << "graphVertex::ckConsistency --- t->thisvertex != this\n" << flush;
	 return 0;
      }
      if (t->previnthisvertex != prev) {
	 cout << "graphVertex::ckConsistency --- t->previnthisvertex != prev\n" << flush;
	 return 0;
      }
      prev = t;
   }

   if (last != prev) {
      cout << "graphVertex::ckConsistency --- last is not last terminal\n" << flush;
      return 0;
   }
   return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//				   graphEdge                                 //
///////////////////////////////////////////////////////////////////////////////

// The constructor for a graphEdge
graphEdge::graphEdge()
{
   first = last = NULL;
}

// The destructor of a graphEdge
graphEdge::~graphEdge()
{
   // Here we clean up all the terminals in the edge
   graphTerminal *next_t;
   for (graphTerminal *t = first; t; t = next_t)
   {
      next_t = t->nextinthisedge;
      delete t;
   }
}

// return a pointer to the first terminal in the hyperedge
graphTerminal *graphEdge::firstTerminal()
{
   return first;
}

// return a pointer to the last terminal in the hyperedge
graphTerminal *graphEdge::lastTerminal()
{
   return last;
}

// Add thevertex to this hyperedge. In order to do so, addToEdge() must create
// a new terminal. It therefore calls the !!virtual!!  function
// graphEdge::newTerminal(), so that if you redefine this virtual function in a
// derived graphEdge class, addToEdge will use the derived graphTerminal in
// stead of the base class terminal.  You can also pass your own terminal as
// the third optional argument, in that case newTerminal() won't be called.
graphEdge *graphEdge::addToEdge (graphVertex *thevertex, graphTermType tt, graphTerminal *newterminal)
{
   if (!newterminal) newterminal = newTerminal();
   newterminal->termtype = tt;
   // first link newterminal in this edge
   addToEdge(newterminal);
   // and now link newterminal in thevertex\'s list of terminals
   thevertex->addToVertex(newterminal);
   return this;
}

// Add newTerm to this hyper edge:
graphEdge *graphEdge::addToEdge (graphTerminal *newTerm)
{
   if (newTerm->nextinthisedge || newTerm->previnthisedge || newTerm->thisedge)
      graphError ("graphEdge::addToEdge() -- newTerm already in other edge");
   if (!last)
   {
      first = last = newTerm;
      newTerm->thisedge = this;
   }
   else
      last = last->insertInEdge(newTerm);
   return this;
}

graphEdge *graphEdge::removeFromEdge (graphVertex *thevertex)
{
   graphTerminal *t = first;
   // here we look for the terminal that belongs to thevertex
   for (; t && t->thisVertex() != thevertex; t = t->nextInThisEdge())
      ;
   if (!t)
      graphError ("graphEdge::removeFromEdge() -- edge not incident to vertex");
   // now t points to the terminal to be removed
   t->removeFromEdge()->removeFromVertex();
   // ...and get rid of it
   delete t;
   return this;
}

// This function is useful for edges that only connect to 2 vertices, i.e. for
// graphs that are not hypergraphs. It returns the vertex connected to this
// edge that is not the vertex on thisSide.
// The function's behavior is undefined if this edge does not contain exactly
// two vertices.
graphVertex *graphEdge::otherSide (graphVertex *thisSide)
{
   if (first->thisvertex == thisSide)
      return last->thisvertex;
   else
      return first->thisvertex;
}

// This returns the number of terminal in the edge. If you don\'t connect the
// same edge two or more times to the same vertex, than this is also the number
// of vertices that the edge connects to (The "degree" of the edge).
int graphEdge::numberOfTerminals()
{
   int count = 0;
   for (graphTerminal *t = first; t; t = t->nextInThisEdge())
      ++count;
   return count;
}

void  graphEdge::print()
{
   cout << "<graphEdge>" << flush;
}

// redefinition of graphElement::printElement()
void graphEdge::printElement()
{
   cout << "graphEdge { ";
   for (graphTerminal *t = firstTerminal(); t;)
   {
      t->thisVertex()->print();
      if ((t = t->nextInThisEdge()))
	 cout << ", " ;
   }
   cout << " }\n";
}

// Perform a consistency check on the edge datastructure.
int graphEdge::ckConsistency()
{
   if (!first && !last) return TRUE;
   if (!first || !last) {
      cout << "graphEdge::ckConsistency --- first/last inconsistency\n" << flush;
      return 0; // one 0, but not the other...
   }
   if (first->previnthisedge) {
      cout << "graphEdge::ckConsistency --- first->previnthisedge != NULL\n" << flush;
      return 0;
   }
   if (last->nextinthisedge) {
      cout << "graphEdge::ckConsistency --- last->nextinthisedge != NULL\n" << flush;
      return 0;
   }

   graphTerminal *prev = NULL;

   for (graphTerminal *t = first; t; t = t->nextinthisedge)
   {
      if (t->thisedge != this) {
	 cout << "graphEdge::ckConsistency --- t->thisedge != this\n" << flush;
	 return 0;
      }
      if (t->previnthisedge != prev) {
	 cout << "graphEdge::ckConsistency --- t->previnthisedge != prev\n" << flush;
	 return 0;
      }
      prev = t;
   }

   if (last != prev) {
      cout << "graphEdge::ckConsistency --- last is not last terminal\n" << flush;
      return 0;
   }
   return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//				 graphTerminal                               //
///////////////////////////////////////////////////////////////////////////////

// Constructor
graphTerminal::graphTerminal (graphTermType ttype)
{
   termtype = ttype;
   thisvertex = NULL;
   thisedge = NULL;
   nextinthisvertex = previnthisvertex = nextinthisedge = previnthisedge = NULL;
}

// Destructor
graphTerminal::~graphTerminal()
{
   // here we unlink this graphTerminal from the vertex list and the edge list
   if (thisvertex)  // this terminal still belongs to a vertex
   {
      if (previnthisvertex)
	 previnthisvertex->nextinthisvertex = nextinthisvertex;
      else
	 thisvertex->first = nextinthisvertex;
      if (nextinthisvertex)
	 nextinthisvertex->previnthisvertex = previnthisvertex;
      else
	 thisvertex->last = previnthisvertex;
   }
   if (thisedge) // this terminal still belongs to an edge
   {
      if (previnthisedge)
	 previnthisedge->nextinthisedge = nextinthisedge;
      else
	 thisedge->first = nextinthisedge;
      if (nextinthisedge)
	 nextinthisedge->previnthisedge = previnthisedge;
      else
	 thisedge->last = previnthisedge;
   }
}

// Return a pointer to the next terminal in this vertex with the same termtype
// as this terminal.
graphTerminal *graphTerminal::nextInThisVertex (graphTermType tt)
{
   graphTerminal *t = nextinthisvertex;
   if (tt != GenericEdge)
      while (t && t->termtype != tt) t = t->nextinthisvertex;
   return t;
}

// Return a pointer to the previous terminal in this vertex with the same
// termtype as this terminal.
graphTerminal *graphTerminal::prevInThisVertex (graphTermType tt)
{
   graphTerminal *t = previnthisvertex;
   if (tt != GenericEdge)
      while (t && t->termtype != tt) t = t->previnthisvertex;
   return t;
}

// Insert nextterminal just after this terminal in the graphEdge list
// and return a pointer to nextterminal.
graphTerminal *graphTerminal::insertInEdge (graphTerminal *nextterminal)
{
   if ((nextterminal->nextinthisedge = nextinthisedge))
      nextinthisedge->previnthisedge = nextterminal;
   nextinthisedge = nextterminal;
   nextterminal->previnthisedge = this;
   nextterminal->thisedge = thisedge;
   return nextterminal;
}

graphTerminal *graphTerminal::removeFromEdge()
{
   // handle special case: this is the only terminal in the edge
   if (thisedge->first == thisedge->last)
   {
      thisedge->first = thisedge->last = NULL;
      thisedge = NULL;
      return this;
   }
   // special case: this is first terminal in edge with >= 2 terminals
   if (thisedge->first == this)
   {
      thisedge->first = nextinthisedge;
      nextinthisedge->previnthisedge = NULL;
      nextinthisedge = NULL;
      thisedge = NULL;
      return this;
   }
   // special case: this is last terminal in edge with >= 2 terminals
   if (thisedge->last == this)
   {
      thisedge->last = previnthisedge;
      previnthisedge->nextinthisedge = NULL;
      previnthisedge = NULL;
      thisedge = NULL;
      return this;
   }
   // general case: this is a terminal somewhere in the middle of the edge
   previnthisedge->nextinthisedge = nextinthisedge;
   nextinthisedge->previnthisedge = previnthisedge;
   previnthisedge = nextinthisedge = NULL;
   thisedge = NULL;
   return this;
}

graphTerminal *graphTerminal::removeFromVertex()
{
   // handle special case: this is the only terminal in the vertex
   if (thisvertex->first == thisvertex->last)
   {
      thisvertex->first = thisvertex->last = NULL;
      thisvertex = NULL;
      return this;
   }
   // special case: this is first terminal in vertex with >= 2 terminals
   if (thisvertex->first == this)
   {
      thisvertex->first = nextinthisvertex;
      nextinthisvertex->previnthisvertex = NULL;
      nextinthisvertex = NULL;
      thisvertex = NULL;
      return this;
   }
   // special case: this is last terminal in vertex with >= 2 terminals
   if (thisvertex->last == this)
   {
      thisvertex->last = previnthisvertex;
      previnthisvertex->nextinthisvertex = NULL;
      previnthisvertex = NULL;
      thisvertex = NULL;
      return this;
   }
   // general case: this is a terminal somewhere in the middle of the vertex
   previnthisvertex->nextinthisvertex = nextinthisvertex;
   nextinthisvertex->previnthisvertex = previnthisvertex;
   previnthisvertex = nextinthisvertex = NULL;
   thisvertex = NULL;
   return this;
}

// Insert nextterminal just after this terminal in the graphVertex's list
// of terminal and return a pointer to nextterminal.
graphTerminal *graphTerminal::insertInVertex (graphTerminal *nextterminal)
{
   if ((nextterminal->nextinthisvertex = nextinthisvertex))
      nextinthisvertex->previnthisvertex = nextterminal;
   nextinthisvertex = nextterminal;
   nextterminal->previnthisvertex = this;
   nextterminal->thisvertex = thisvertex;
   return nextterminal;
}

void graphTerminal::print()
{
   cout << "<graphTerminal>" << flush;
}

//////////////////////////////////////////////////////////////////////////////
//			  graphVertexNeighborIterator                       //
//////////////////////////////////////////////////////////////////////////////

// Constructor
graphVertexNeighborIterator::graphVertexNeighborIterator
            (graphVertex *v, graphTermType termOut,
             graphTermType termIn)
{
   initialize (v, termOut, termIn);
}

void graphVertexNeighborIterator::initialize (graphVertex *v, graphTermType termOut, graphTermType termIn)
{
   termout = termOut;
   termin = termIn;
   thevertex = v;
   if (!v)
   {
      currentneighborterminal = NULL;
      return;
   }
   currentthisterminal = v->firstTerm(termout);
   if (!currentthisterminal)
   {
      currentneighborterminal = NULL;
      return;
   }
   currentneighborterminal = currentthisterminal->thisEdge()->firstTerminal();
   while (currentneighborterminal &&
	  (currentneighborterminal->thisVertex() == v ||
	   (termIn != GenericEdge && currentneighborterminal->type() != termIn)))
   {
      do
	 currentneighborterminal = currentneighborterminal->nextInThisEdge();
      while (currentneighborterminal &&
	     termIn != GenericEdge &&
	     currentneighborterminal->type() != termIn);
      if (!currentneighborterminal)
      {
	 currentthisterminal = currentthisterminal->nextInThisVertex(termout);
	 if (!currentthisterminal) break;
	 currentneighborterminal = currentthisterminal->thisEdge()->firstTerminal();
      }
   }
}

graphVertex *graphVertexNeighborIterator::operator() ()
{
   // This iterator always contains the next neighbor vertex to be returned.
   // So, first we save this neighbor in a local variable...
   if (!currentneighborterminal) return NULL;
   graphVertex *theneighbor = currentneighborterminal->thisVertex();
   // ...and then we compute the next state of the iterator:
   do
   {
      if (termin == GenericEdge)
	 currentneighborterminal = currentneighborterminal->nextInThisEdge();
      else
      {
	 do
	    currentneighborterminal = currentneighborterminal->nextInThisEdge();
	 while (currentneighborterminal &&
		currentneighborterminal->type() != termin);
      }
      if (!currentneighborterminal)
      {
	 currentthisterminal = currentthisterminal->nextInThisVertex(termout);
	 if (!currentthisterminal) break;
	 currentneighborterminal = currentthisterminal->thisEdge()->firstTerminal();
      }
   } while (currentneighborterminal &&
	    (currentneighborterminal->thisVertex() == thevertex ||
	     (termin != GenericEdge && currentneighborterminal->type() != termin)));
   return theneighbor;
}

// if NEIGHBOR is the next thing to return, then forget about it and advance
// the iterator to the next thing. This is a useful method to call when we
// are deleting neighbor vertices while iterating them at the same time.
void graphVertexNeighborIterator::skip (graphVertex* neighbor)
{
   if (currentneighborterminal->thisVertex() == neighbor) operator()();
}

//////////////////////////////////////////////////////////////////////////////
//			    graphVertexEdgeIterator                         //
//////////////////////////////////////////////////////////////////////////////

graphVertexEdgeIterator::graphVertexEdgeIterator (graphVertex *v, graphTermType tt)
{
   initialize (v, tt);
}

void graphVertexEdgeIterator::initialize (graphVertex *v, graphTermType ttype)
{
   tt = ttype;
   if (!v) {
      currentterminal = NULL;
      return;
   }
   currentterminal = v->firstTerm(tt);
}

graphEdge *graphVertexEdgeIterator::operator()()
{
   // This iterator always contains the next edge to be returned.
   // So, first we save this edge in a local variable...
   if (!currentterminal) return NULL;
   graphEdge *thisedge = currentterminal->thisEdge();
   // ...and then we compute the next state of the iterator:
   currentterminal = currentterminal->nextInThisVertex(tt);
   return thisedge;
}

// if INCIDENT_EDGE is the next thing to return, then forget about it and
// advance the iterator to the next thing. This is a useful method to call
// when we are deleting incident edges while iterating them at the same time.
void graphVertexEdgeIterator::skip (graphEdge* incident_edge)
{
   if (currentterminal->thisEdge() == incident_edge) operator()();
}

///////////////////////////////////////////////////////////////////////////////
//			 graphVertexCommonEdgeIterator			     //
///////////////////////////////////////////////////////////////////////////////
graphVertexCommonEdgeIterator::graphVertexCommonEdgeIterator
   (graphVertex *v1, graphVertex *v2, graphTermType tt)
{
   initialize (v1, v2, tt);
}

void graphVertexCommonEdgeIterator::initialize (graphVertex *v1, graphVertex *vb, graphTermType ttype)
{
   tt = ttype;
   v2 = vb;
   if (!v1)
   {
      currentterminal = NULL;
      return;
   }
   currentterminal = v1->firstTerm(tt);
   while (currentterminal)
   {
      graphTerminal *t = currentterminal->thisEdge()->firstTerminal();
      for (; t; t = t->nextInThisEdge()) // iterate all terms of this edge
      {
         if (t == currentterminal)
            continue;           // without this we lose when v1 == vb
	 if (t->thisVertex() == v2)
	    return;		// !!!! GOTCHA !!!!
      }
      currentterminal = currentterminal->nextInThisVertex(tt);
   }
}

// Return the next edge that is common to the vertices v1 and vb (see above).
graphEdge *graphVertexCommonEdgeIterator::operator()()
{
   if (!currentterminal) return NULL;
   // this iterator's state contains the edge to be returned:
   graphEdge *thecommonedge = currentterminal->thisEdge();
   // now compute the next state of the iterator:
   currentterminal = currentterminal->nextInThisVertex(tt);
   while (currentterminal)
   {
      graphTerminal *t = currentterminal->thisEdge()->firstTerminal();
      for (; t; t = t->nextInThisEdge()) // iterate the terms of this edge
      {
         if (t == currentterminal)
            continue;
	 if (t->thisVertex() == v2)
	    return thecommonedge; // ...and leave iterator's state OK!
      }
      currentterminal = currentterminal->nextInThisVertex(tt);
   }
   return thecommonedge;
}

// if COMMON_EDGE is the next thing to return, then forget about it and
// advance the iterator to the next thing. This is a useful method to call
// when we are deleting common edges while iterating them at the same time.
void graphVertexCommonEdgeIterator::skip (graphEdge* common_edge)
{
   if (currentterminal->thisEdge() == common_edge) operator()();
}

//////////////////////////////////////////////////////////////////////////////
//			    graphEdgeVertexIterator                         //
//////////////////////////////////////////////////////////////////////////////

// Constructor
graphEdgeVertexIterator::graphEdgeVertexIterator (graphEdge *e, graphTermType tt)
{
   initialize (e, tt);
}

void graphEdgeVertexIterator::initialize (graphEdge *e, graphTermType ttype)
{
   tt = ttype;
   if (!e) {
      currentterminal = NULL;
      return;
   }
   currentterminal = e->firstTerminal();
   if (tt != GenericEdge)
      while (currentterminal && currentterminal->type() != tt)
	 currentterminal = currentterminal->nextInThisEdge();
}

graphVertex *graphEdgeVertexIterator::operator()()
{
   // This iterator always contains the next vertex to be returned.
   // So, first we save this vertex in a local variable...
   if (!currentterminal) return NULL;
   graphVertex *thisvertex = currentterminal->thisVertex();
   // ...and then we compute the next state of the iterator:
   currentterminal = currentterminal->nextInThisEdge();
   if (tt != GenericEdge)
      while (currentterminal && currentterminal->type() != tt)
	 currentterminal = currentterminal->nextInThisEdge();
   return thisvertex;
}

// if VERTEX is the next thing to return, then forget about it and advance
// the iterator to the next thing. This is a useful method to call when we
// are deleting incident vertices while iterating them at the same time.
void graphEdgeVertexIterator::skip (graphVertex* vertex)
{
   if (currentterminal->thisVertex() == vertex) operator()();
}

//////////////////////////////////////////////////////////////////////////////
//			  graphVertexTerminalIterator                       //
//////////////////////////////////////////////////////////////////////////////

graphVertexTerminalIterator::graphVertexTerminalIterator (graphVertex *vx, graphTermType ttype)
{
   initialize (vx, ttype);
}

void graphVertexTerminalIterator::initialize (graphVertex *vx, graphTermType ttype)
{
   tt = ttype;
   if (!vx) {
      currentterminal = NULL;
      return;
   }
   currentterminal = vx->firstTerm(tt);
}

graphTerminal *graphVertexTerminalIterator::operator()()
{
   if (!currentterminal) return NULL;
   graphTerminal *theterminal = currentterminal;
   currentterminal = currentterminal->nextInThisVertex(tt);
   return theterminal;
}

// if TERM is the next thing to return, then forget about it and advance the
// iterator to the next thing. This is a useful method to call when we are
// deleting terminals while iterating them at the same time.
void graphVertexTerminalIterator::skip (graphTerminal* term)
{
   if (currentterminal == term) operator()();
}

//////////////////////////////////////////////////////////////////////////////
//			   graphEdgeTerminalIterator                        //
//////////////////////////////////////////////////////////////////////////////

graphEdgeTerminalIterator::graphEdgeTerminalIterator (graphEdge *e, graphTermType ttype)
{
   initialize (e, ttype);
}

void graphEdgeTerminalIterator::initialize (graphEdge *e, graphTermType ttype)
{
   tt = ttype;
   if (!e) {
      currentterminal = NULL;
      return;
   }
   currentterminal = e->firstTerminal();
   if (tt != GenericEdge)
      while (currentterminal && currentterminal->type() != tt)
	 currentterminal = currentterminal->nextInThisEdge();
}

graphTerminal *graphEdgeTerminalIterator::operator()()
{
   if (!currentterminal) return NULL;
   graphTerminal *theterminal = currentterminal;
   currentterminal = currentterminal->nextInThisEdge();
   if (tt != GenericEdge)
      while (currentterminal && currentterminal->type() != tt)
	 currentterminal = currentterminal->nextInThisEdge();
   return theterminal;
}

// if TERM is the next thing to return, then forget about it and advance the
// iterator to the next thing. This is a useful method to call when we are
// deleting terminals while iterating them at the same time.
void graphEdgeTerminalIterator::skip (graphTerminal* term)
{
   if (currentterminal == term) operator()();
}

void graphError (const char *msg)
{
   cout << flush;
   cerr << "\n/\\/\\/\\/ graphError: " << msg << endl;
   abort();
}
