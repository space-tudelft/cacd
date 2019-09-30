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

#ifndef __GRAPHSTEINER_H
#define __GRAPHSTEINER_H

#include "src/ocean/libocean/graph.h"
#include "src/ocean/libocean/sdfset.h"
#include "src/ocean/libocean/hashTable.h"

// the graphSteiner() function computes a close-to-minimal steiner tree. It
// uses the multidirectional search method described in "Combinatorial
// Algorithms for Integrated Circuit Layout" by Thomas Lengauer, published by
// Teubner in Stuttgart in 1990, section 3.10.3 "Using Kruskals algorithm".
// The run time complexity of the algorithm is O(m log n), where m is the
// number of edges in the graph and n is the number of vertices in the graph.
//
// The input is a set of requiredVertices which is a graph and a subset of this
// graph (so called requiredVertices) for which we want to build a minimal
// steiner tree. The graph must be simple and may _not_ contain hyper edges.
//
// The steiner tree is returned as the set of edges that are in the minimal
// steiner tree.
//
// The return value of graphSteiner() is the sum of the edge weights in the
// steiner tree.
//
// Note that the elements of all sets discussed here are either of the type
// treeVertex or treeEdge. No ordinary setElmts are allowed. The weight of an
// edge is determined by its virtual member graphEdge.eWeight().
//

int graphSteiner (graphDescriptor& theGraph,
		sdfset& requiredVertices, // set of treeVertex objects
		sdfset& treeEdges);	// set of treeEdge objects

// elements of the set of vertices that are in a steiner tree:
class treeVertex: public sdfsetElmt, public hashTableElmt
{
private:
   graphVertex *thevertex;
public:
   treeVertex(graphVertex *vx) {thevertex = vx;}
   graphVertex *vertex() {return thevertex;}
   virtual void print() {cout << "<treeVertex ";
			 thevertex->print(); cout << "> " << flush;}
};

// elements of the set of edges that are in a steiner tree:
class treeEdge: public sdfsetElmt, public hashTableElmt
{
private:
   graphEdge *theedge;
public:
   treeEdge(graphEdge *e) {theedge = e;}
   graphEdge *edge() {return theedge;}
};

//////////////////////////////////////////////////////////////////////////////
//	    R E Q U I R E D   V E R T E X   L O O K U P   T A B L E         //
//////////////////////////////////////////////////////////////////////////////
//
// This hash table stores treeVertex objects. It takes the graphVertex as the
// key and then stores or returns the corresponding treeVertex.
//
class treeVertexInfo: public hashTable
{
private:
   graphVertex *thekey; // for communication from search() to hasTheRightKey()
   unsigned long hashFunction(graphVertex *v) {
      // we just fiddle a bit with the pointer value ...:
      return (unsigned long) ((long(v) << 19) + long(v));
   }
public:
   // the constructor only passes the table size to the hashTable base class:
   treeVertexInfo(unsigned long size): hashTable(size) {};
   // insert() puts the thing into the hash table:
   treeVertexInfo& insert(treeVertex& thing) {
      hashkey = hashFunction(thing.vertex());
      builtinInsert(thing);
      return *this;
   }
   // search() returns the thing in the hash table that has the specified key:
   treeVertex *search(graphVertex *key) {
      thekey = key; // builtinSearch calls hasTheRightKey()
      hashkey = hashFunction(key);
      return (treeVertex *)builtinSearch();
   }
   // remove() deletes the specified thing from the hash table:
   treeVertexInfo& remove(treeVertex& thing) {
      hashkey = hashFunction(thing.vertex());
      builtinRemove(thing);
      return *this;
   }
   // this one is called by builtinSearch() which is called by search():
   virtual const int hasTheRightKey(hashTableElmt& thing) {
      return ((treeVertex &)thing).vertex() == thekey;
   }
};

//////////////////////////////////////////////////////////////////////////////
//		  G R A P H E D G E   L O O K U P   T A B L E               //
//////////////////////////////////////////////////////////////////////////////
//
// The purpose of this hash table is to quickly check whether a certain
// graphEdge has already been put into the output set of treeEdges. (In other
// words: all edges that belong to the minimal steiner tree eventually end up
// in this treeEdgeInfo table.)
//
class treeEdgeInfo: public hashTable
{
private:
   graphEdge *thekey; // for communication from search() to hasTheRightKey()
   unsigned long hashFunction(graphEdge *e) {
      // we just fiddle a bit with the pointer value ...:
      return (unsigned long) ((long(e) << 18) + long(e));
   }
public:
   // the constructor only passes the table size to the hashTable base class:
   treeEdgeInfo(unsigned long size): hashTable(size) {};
   // insert() puts the thing into the hash table:
   treeEdgeInfo& insert(treeEdge& thing) {
      hashkey = hashFunction(thing.edge());
      builtinInsert(thing);
      return *this;
   }
   // search() returns the thing in the hash table that has the specified key:
   treeEdge *search(graphEdge *key) {
      thekey = key; // builtinSearch calls hasTheRightKey()
      hashkey = hashFunction(key);
      return (treeEdge *)builtinSearch();
   }
   // remove() deletes the specified thing from the hash table:
   treeEdgeInfo& remove(treeEdge& thing) {
      hashkey = hashFunction(thing.edge());
      builtinRemove(thing);
      return *this;
   }
   // this one is called by builtinSearch() which is called by search():
   virtual const int hasTheRightKey(hashTableElmt& thing) {
      return ((treeEdge &)thing).edge() == thekey;
   }
};

//////////////////////////////////////////////////////////////////////////////
//
// Only one object of the graphSteinerOptions class exists, and it can be
// manipulated to alter the behavior of the graphSteiner() function.
//
typedef int (*abFunc)(sdfset&, treeEdgeInfo&);

class graphSteinerOptions
{
private:
   int (*acceptBranchFunc)(sdfset& branch, treeEdgeInfo& steInfo);
public:
   graphSteinerOptions();
   // you can install a function f() that graphSteiner() calls when it is about
   // to add a new branch to the treeEdges (steiner tree). If f() returns TRUE
   // then the branch is accepted, else it is rejected and graphSteiner()
   // continues:
   //void installAcceptBranchFunction(int (*f)(sdfset&, treeEdgeInfo&));
   void installAcceptBranchFunction(abFunc);
   abFunc installAcceptBranchFunction() {return acceptBranchFunc;}
   int acceptBranch(sdfset& branch, treeEdgeInfo& steInfo); // this evaluates f()
};

// Declare the only existing graphSteinerOptions object:
extern graphSteinerOptions graphSteinOpts;

#endif
