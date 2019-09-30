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

#include "src/ocean/libocean/graphSteiner.h"
#include "src/ocean/libocean/hashTable.h"
#include "src/ocean/libocean/priorQ.h"

//////////////////////////////////////////////////////////////////////////////
//		V E R T E X   I N F O   L O O K U P   T A B L E             //
//////////////////////////////////////////////////////////////////////////////
//
// We need a lookup table that registers for each vertex which required vertex
// (called "home") is most nearby, and on what distance this required vertex is
// (the "distance" field), and which edge is the first edge along the path from
// this vertex to its home. We use a hash table as the lookup table.

// This is the thing that we are going to put in the hash table:
//
class vertexInfoElmt: public hashTableElmt
{
private:
   graphVertex *thevertex;	// this is the search key
   graphVertex *thehome;	// the most nearby required vertex
   int         thedistance;	// distance to the home vertex
   graphEdge   *thepath;	// edge along path from thevertex to thehome
public:
   vertexInfoElmt(graphVertex *vx);
   graphVertex *vertex()               {return thevertex;}
   graphVertex *home()                 {return thehome;}
   graphVertex *home(graphVertex *hvx) {return thehome = hvx;}
   int         distance()              {return thedistance;}
   int         distance(int d)         {return thedistance = d;}
   graphEdge   *path()                 {return thepath;}
   graphEdge   *path(graphEdge *p)     {return thepath = p;}
};

// This is the hash table itself:
//
class vertexInfo : public hashTable
{
private:
   graphVertex *thekey; // for communication from search() to hasTheRightKey()
   unsigned long hashFunction(graphVertex *v) {
      // we just fiddle a bit with the pointer value ...:
      return (unsigned long) ((long(v) << 19) + long(v));
   }
public:
   // the constructor only passes the table size to the hashTable base class:
   vertexInfo(unsigned long size): hashTable(size) {};
   // insert() puts the thing into the hash table:
   vertexInfo& insert(vertexInfoElmt &thing) {
      hashkey = hashFunction(thing.vertex());
      builtinInsert(thing);
      return *this;
   }
   // search() returns the thing in the hash table that has the specified key:
   vertexInfoElmt *search(graphVertex *key) {
      thekey = key; // builtinSearch calls hasTheRightKey()
      hashkey = hashFunction(key);
      return (vertexInfoElmt *)builtinSearch();
   }
   // remove() deletes the specified thing from the hash table:
   vertexInfo& remove(vertexInfoElmt& thing) {
      hashkey = hashFunction(thing.vertex());
      builtinRemove(thing);
      return *this;
   }
   // this one is called by builtinSearch() which is called by search():
   virtual const int hasTheRightKey(hashTableElmt& thing) {
      return ((vertexInfoElmt &)thing).vertex() == thekey;
   }
};

///////////////////////////////////////////////////////////////////////////////
//		      T H E   P R I O R I T Y   Q U E U E                    //
///////////////////////////////////////////////////////////////////////////////

// The distanceElmt is the thing that we are going to put in the priority
// queue. It stores two vertices and the distance between them. This distance
// also serves as the priority in the queue: the smaller the distance, the
// higher the priority.
//
class distanceElmt : public priorQelmt
{
private:
   graphVertex *_s, *_v;	// required vertex and other vertex
   graphEdge   *_e;		// first edge from v in direction of s
   int         _d;		// the distance from _s to _v
public:
   distanceElmt(graphVertex *s, graphVertex *v, int d, graphEdge *e) { _s = s; _v = v; _d = d; _e = e; }
   virtual const int priority() {return -_d;} // small distance -> high priority
   graphVertex *s() {return _s;}
   graphVertex *v() {return _v;}
   graphEdge   *e() {return _e;}
   int          d() {return _d;}
};

//////////////////////////////////////////////////////////////////////////////
//			      U N I O N - F I N D                           //
//////////////////////////////////////////////////////////////////////////////

class unionFind : public sdfset, public sdfsetElmt
{
public:
   unionFind(): sdfset(NonOrderedSet, NULL, MaintainBackwardReference) {};
   void ufPrint();
   virtual void print() {;}
};

void unionFind::ufPrint()
{
   cout << "<unionFind " << flush;
   sdfsetNextElmtIterator nE(this);
   sdfsetElmt *e;
   while ((e = nE())) e->print();
   cout << ">" << endl;
}

///////////////////////////////////////////////////////////////////////////////
//		     F U N C T I O N   P R O T O T Y P E S                   //
///////////////////////////////////////////////////////////////////////////////

static void initializeSteiner (priorQ& D, sdfset& U, sdfset& requiredVertices);
static void cleanupSteiner (priorQ& D, sdfset& U, sdfset& requiredVertices);
static void handleNilCase (priorQ &D, graphVertex *s, graphVertex *v, int d, graphEdge *path);
static void handleNonNilCase (sdfset& treeEdges, int& length, priorQ& D, sdfset& U,
			     graphVertex *s, graphVertex *v, int d, graphEdge *path);
static int  makeSteinerBranch (sdfset& treeEdges, graphVertex *s, graphVertex *v,
			      int d, graphEdge *ePath, int addToSteInfo);
static int addEdgeToSteinerTree (sdfset& treeEdges, graphEdge *ePath, int addToSteInfo);
static int addPathToSteinerTree (sdfset& treeEdges, graphVertex *startOfPath, int addToSteInfo);
static graphVertex *home (graphVertex *vx);
static unionFind *uf_Find (graphVertex *rvx);
static void uf_Union (sdfset& U, unionFind *uf1, unionFind *uf2);
static void steinError (const char *msg);

///////////////////////////////////////////////////////////////////////////////
//			G L O B A L   V A R I A B L E S                      //
///////////////////////////////////////////////////////////////////////////////

// lookup what is the home and the distance for a graphVertex (1,2):
static vertexInfo vxInfo(1009);

// lookup which treeVertex belongs to a graphVertex:
static treeVertexInfo rvxInfo(1009);

// lookup the treeEdges that are part of the steiner tree:
static treeEdgeInfo steInfo(1009);

///////////////////////////////////////////////////////////////////////////////
//			   T H E   A L G O R I T H M                         //
///////////////////////////////////////////////////////////////////////////////
//
// The numbers in braces refer to the line numbers of the description of the
// multidirectional search method found in "Combinatorial Algorithms for
// Integrated Circuit Layout" by Thomas Lengauer, published by Teubner in
// Stuttgart in 1990, section 3.10.3 "Using Kruskals algorithm". See
// <graphSteiner.h> for more information.

int graphSteiner (graphDescriptor& theGraph,
		sdfset& requiredVertices,	// set of treeVertex objects
		sdfset& treeEdges)		// set of treeEdge objects
{
   theGraph.pushCurrentGraph();
   int        length = 0;	// the length of the steiner tree
   priorQ     D(10000);		// create the priority queue (6)
   sdfset     U;		// create the union-find set (6)
   initializeSteiner (D, U, requiredVertices); // (6,7,8,11,12,13)

   // at this point, the set requiredVertices is empty, but who cares ...
   while (U.size() > 1)	// while more than one union-find set ... (14)
   {
      // set s, v and d to the vertex pair with the shortest distance (15):
      graphVertex  *s, *v; // s is a required vertex, v can be any vertex
      int          d;	   // d is the distance from s to v
      distanceElmt *shortestDist = (distanceElmt *)D.extractFirst(); // (15)
      s = shortestDist->s(); v = shortestDist->v(); d = shortestDist->d();
      graphEdge *ePath = shortestDist->e();   // edge along path from v to s
      delete shortestDist;
      unionFind *findHomeV = uf_Find(home(v)); // (16)
      if (!findHomeV)		       // (17)
      {  // v does not have a home yet and s is the most nearby required
	 // vertex, so we make s the home of v, and we put the neighborhood of
	 // v in the priority queue (so that we can inspect this neighborhood
	 // in a later iteration of this while loop):
	 handleNilCase(D, s, v, d, ePath);    // (17,18,19)
      }
      else if (findHomeV == uf_Find(s))	      // (20)
	 // v and s are already connected by a branch of the steiner tree, so
	 // there is nothing to do!
	 ;
      else
      {  // if v is a required vertex than we have found a new branch of the
	 // steiner tree; if not, then the wavefront expanded around s has just
	 // bumped into the wavefront expanded around the home of v so we have
	 // to put the sum of the two distances into the priority queue:
	 handleNonNilCase(treeEdges, length, D, U, s, v, d, ePath); // (21...25)
      }
   }

   // cleanup data structs and move the req.vert in U back to requiredVertices:
   cleanupSteiner (D, U, requiredVertices);
   popCurrentGraph();
   return length;
}

//////////////////////////////////////////////////////////////////////////////
//		       S U P P O R T   F U N C T I O N S                    //
//////////////////////////////////////////////////////////////////////////////

static void initializeSteiner (priorQ& D, sdfset& U, sdfset& requiredVertices)
{
   // fill the union find set: each required vertex goes into its own unionFind
   // object and the operation requiredvertex.mySet() returns this unionFind
   // object. (6)
   sdfsetNextElmtIterator nextThing(&requiredVertices);
   treeVertex *tvx;

   while ((tvx = (treeVertex *)nextThing()))
   {
      // move the required vertex tvx to the union-find set (6):
      requiredVertices.remove(tvx);
      unionFind *ufe = new unionFind;
      ufe->add(tvx);	       // add the required vertex to the union-find (6)
      U.add(ufe);	       // maintain the set of union-find sets (6)
      rvxInfo.insert(*tvx);    // to quickly map a vertex to its treeVertex
      // put this required vertex also in the lookup table (7,8):
      graphVertex *s = tvx->vertex(); // s is our required vertex
      vertexInfoElmt *vie = new vertexInfoElmt(s);
      vie->home(s);	       // home of a required vertex is it self
      vie->distance(0);	       // ... and the distance is 0 of course.
      vxInfo.insert(*vie);     // enter into lookup table (8)

      // put the edges to the neighborhood into the priority queue (11,12,13):
      graphVertexEdgeIterator nextEdge(s);
      graphEdge *e;
      while ((e = nextEdge())) // (12)
      {
	 graphVertex *v = e->otherSide(s);
	 // take care: do not enter an edge twice into D! This could happen if
	 // v is also a required vertex ...
	 if (vxInfo.search(v))
	    continue;	// we already inserted this edge when we processed v!
	 distanceElmt *svdTriple = new distanceElmt(s, v, e->eWeight(), e);
	 D.insert(*svdTriple);	// enter into priority queue (13)
      }
   }
   // no need to perform the initialization in the lines (9) and (10) of the
   // algorithm because we use a hash table in stead of an array.
}

// Handle the case where find(home[v]) == NULL, lines (17,18,19) in Lengauer:
//
static void handleNilCase (priorQ &D,
			graphVertex *s,  // s is a required vertex
			graphVertex *v,  // v is _not_ a required vertex
			int d,	         // d is distance from s to v
			graphEdge *ePath) // edge from v in direction of s
{
   // v is not a required vertex, and it has no home assigned yet. Since we got
   // the triple (s, v, d) from the priority queue we conclude that s is for v
   // the most nearby required vertex.
   //
   // Store the knowledge that the home of v is s, and that the distance from v
   // to s is d:
   vertexInfoElmt *vie = new vertexInfoElmt(v);
   vie->home(s);		// (17)
   vie->distance(d);		// (17)
   vie->path(ePath);		// remember how to go back from v to s
   vxInfo.insert(*vie);		// (17)
   //
   // Now expand a wavefront around v and put each neighbor vertex w into the
   // priority queue (but not if the neighbor w already has a home). Because we
   // found v by expanding a wavefront around s (the required vertex), we now
   // actually extend this wavefront from s to w, via v:
   graphVertexEdgeIterator nextEdge(v);
   graphEdge *e;
   while ((e = nextEdge())) // (18) iterate all edges e that are incident to v
   {
      graphVertex *w = e->otherSide(v); // e is the edge from v to w
      if (vxInfo.search(w)) // (18)
	 continue;		   // w already has a home
      // The distance from the required vertex s to w  is the distance from s
      // to v PLUS the distance from v to w. In other words, the distance from
      // s to w is "d + e->eWeight()".
      // error: I think in line (19) it should read "d + lambda(v,w)"
      distanceElmt *svdTriple = new distanceElmt(s, w, d + e->eWeight(), e);
      D.insert(*svdTriple);	// enter into priority queue (19)
   }
}

//////////////////////////////////////////////////////////////////////////////
//
// Handle the case where find(home[v]) != NULL, lines (21,22,23,24,25).
//
static void handleNonNilCase(sdfset& treeEdges, int& length, priorQ& D, sdfset& U,
			     graphVertex *s,     // s is a required vertex
			     graphVertex *v,	 // v can be any vertex
			     int d,              // d is distance from s to v
			     graphEdge   *ePath	 // edge along path from v to s
			     )
{
   if (rvxInfo.search(v)) // if v is a required vertex ... (21)
   {
      // both s and v are required vertices, and d is the distance between
      // them. Because we got the triple from the priority queue, it is the
      // shortest distance between the two required vertices s and v, so we add
      // this path from s to v to the steiner tree:
      sdfset branch; // before adding this branch, check that it is not too greedy
      if (graphSteinOpts.installAcceptBranchFunction())
	 makeSteinerBranch(branch, s, v, d, ePath, 0);
      if (graphSteinOpts.acceptBranch(branch, steInfo))
      {  // this branch is accepted ...
	 uf_Union(U, uf_Find(s), uf_Find(v)); // found a shortest path (22)
	 // error: Lengauer says "d + length[v]" but v is in R so this is d ...
	 // error: Lengauer says "home[v]" but v is in R so this is always v...
	 length += makeSteinerBranch(treeEdges, s, v, d, ePath, TRUE); // 23
      }
      branch.delAll();
   }
   else // else if v is not a required vertex (24)
   {
      // The wavefront expanded around the required vertex s has met the
      // wavefront expanded around the home of v. As a consequence we know
      // there exists a path from s to home(v) with length d + length(v).  So
      // we enter the triple (s, home(v), d + length(v)) into the priority
      // queue (25):
      vertexInfoElmt *vie = vxInfo.search(v); // (25)
      distanceElmt *svdTriple =
	 new distanceElmt(s, vie->home(), d + vie->distance(), ePath); // (25)
      D.insert(*svdTriple);	// enter into priority queue (25)
   }
}

static int makeSteinerBranch (sdfset& treeEdges,
				graphVertex *s, // required vertex on one end
				graphVertex *v, // required vertex on other end
				int d,	        // lenght of path from s to v
				graphEdge *ePath, // one side of ePath start chain
				// that goes to s, the other end start chain to v
				int addToSteInfo) // if TRUE then add to steInfo
{
   // the branch from s to v must be assembled from three components:
   // 1. ePath. This is an edge in the middle of the branch;
   // 2. path1, which starts from one end of ePath and goes to s or v;
   // 3. path2, which starts from the other end of ePath and goes to v or s;
   graphVertex *startOfPath1 = ePath->firstTerminal()->thisVertex();
   graphVertex *startOfPath2 = ePath->lastTerminal()->thisVertex();
   // check for internal errors:
   graphVertex *home1 = home(startOfPath1);
   graphVertex *home2 = home(startOfPath2);
   if ((home1 != s || home2 != v) && (home1 != v || home2 != s))
      steinError ("(INTERNAL) unexpected ePath");
   // assemble the branch from the three components; count the total length of
   // edges that are added to the steiner tree:
   int length = addEdgeToSteinerTree(treeEdges,ePath,addToSteInfo);
   length += addPathToSteinerTree(treeEdges,startOfPath1,addToSteInfo);
   length += addPathToSteinerTree(treeEdges,startOfPath2,addToSteInfo);
   // check for internal errors:
   if (length > d)
      steinError ("(INTERNAL) length of steiner branch does not match");
   return length;
}

// return the home of vx:
static graphVertex *home (graphVertex *vx)
{
   if (!vx) return NULL;
   vertexInfoElmt *vie = vxInfo.search(vx);
   if (!vie) return NULL;
   return vie->home();
}

//////////////////////////////////////////////////////////////////////////////
//		      T H E   U N I O N - F I N D   S E T                   //
//////////////////////////////////////////////////////////////////////////////
//
// return the union-find set of the required vertex RVX:
static unionFind *uf_Find(graphVertex *rvx)
{
   if (!rvx) return NULL;
   // we have a graphVertex but we actually need a treeVertex. Damn!
   treeVertex *tRvx = rvxInfo.search(rvx);
   if (!tRvx) return NULL;	// rvx does not seem to be a required vertex!
   unionFind *uf = (unionFind *) (tRvx->mySet());
   uf->ckConsistency();
   return uf;
}

// print the set of union-find sets (for debugging only):
static void uf_print(sdfset &U)
{
   U.ckConsistency();
   cout << "<Set of unionFind> {" << endl;
   sdfsetNextElmtIterator nextUF(&U);
   unionFind *uf;
   while ((uf = (unionFind *)nextUF()))
   {
      uf->ufPrint();
      if (nextUF.more())
	 cout << ", ";
   }
   cout << "\n}" << endl;
   U.ckConsistency();
}

// move all vertices in uf2 to uf1, then get rid of uf2:
static void uf_Union(sdfset& U, unionFind *uf1, unionFind *uf2)
{
   U.ckConsistency();
   uf1->ckConsistency();
   uf2->ckConsistency();
   // uf_print(U);
   uf1->move(uf2);		// move the required vertices from uf2 to uf1
   U.del(uf2);			// remove uf2 from U, then delete uf2
   U.ckConsistency();
   uf1->ckConsistency();
   // uf_print(U);
}

//////////////////////////////////////////////////////////////////////////////

// add the edge ePath to the steiner tree. Return the weight of ePath or 0 if
// ePath was already in the steiner tree.
//
static int addEdgeToSteinerTree(sdfset& treeEdges, graphEdge *ePath,
				int addToSteInfo)
{
   if (steInfo.search(ePath))
      return 0;         // this edge is already in the steiner tree
   treeEdge *te = new treeEdge(ePath); // create a new treeEdge for ePath
   if (addToSteInfo)
      steInfo.insert(*te); // make sure next time we know it is already there
   treeEdges.add(te);   // add to the set of edges that are in the steiner tree
   return ePath->eWeight();
}

// follow the trace from startOfPath to its home, adding all intermediate edges
// to the steiner tree:
static int addPathToSteinerTree (sdfset& treeEdges, graphVertex *startOfPath, int addToSteInfo)
{
   int length = 0;
   graphVertex *vx = startOfPath, *ourHome = home (startOfPath);
   vertexInfoElmt *vie = vxInfo.search(vx);
   graphEdge *e;
   while ((e = vie->path()))
   {
      length += addEdgeToSteinerTree (treeEdges, e, addToSteInfo);
      vx = e->otherSide(vx);
      vie = vxInfo.search(vx);
      if (home(vx) != ourHome) steinError ("(INTERNAL) home is not consistent");
   }
   return length;
}

// print an error message, then abort the program:
static void steinError (const char *msg)
{
   cout << flush;
   cerr << "\nERROR (steiner tree computation): " << msg << endl;
   abort();
}

// constructor of a vertexInfoElmt:
vertexInfoElmt::vertexInfoElmt (graphVertex *vx)
{
   thevertex = vx;
   thehome = NULL;
   thedistance = 0;
   thepath = NULL;
}

// release all memory that we do not need anymore
static void cleanupSteiner (priorQ& D, sdfset& U, sdfset& requiredVertices)
{
   if (U.size() != 1) steinError ("(INTERNAL) unexpected union-find size");
   unionFind *uf_set = (unionFind *)U.firstElmt();
   uf_set->ckConsistency();
   requiredVertices.move(uf_set); // move elmts in uf_set to req.vert.
   U.delAll();			  // delete the uf_set
   vxInfo.cleanup(DeleteHashTableElmts);     // unlink and delete info about vx
   rvxInfo.cleanup(DoNotDeleteHashTableElmts); // unlink req.vx from hash table
   steInfo.cleanup(DoNotDeleteHashTableElmts); // unlink steiner edges f. h.tab
   D.cleanup(DeletePriorQelmts); // empty queue and delete all distanceElmts
}

//////////////////////////////////////////////////////////////////////////////
//				 O P T I O N S                              //
//////////////////////////////////////////////////////////////////////////////

// definition of the only options object that ever gets created:
graphSteinerOptions graphSteinOpts;

graphSteinerOptions::graphSteinerOptions()
{
   acceptBranchFunc = NULL;
}

int graphSteinerOptions::acceptBranch(sdfset& branch, treeEdgeInfo& steInfo)
{
   if (!acceptBranchFunc)
      return TRUE;		// not installed, so we always accept
   else
      return (*acceptBranchFunc)(branch, steInfo);
}

void graphSteinerOptions::installAcceptBranchFunction (int (*f)(sdfset& branch, treeEdgeInfo& steInfo))
{
   acceptBranchFunc = f;
}
