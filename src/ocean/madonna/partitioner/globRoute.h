/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 *	Paul Stravers
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
#ifndef __GLOBROUTE_H
#define __GLOBROUTE_H

#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/libocean/format.h"
#include "src/ocean/libocean/sdfset.h"
#include "src/ocean/libocean/graph.h"
#include "src/ocean/libocean/graphSteiner.h"
#include "src/ocean/madonna/partitioner/genpart.h"
#include "src/ocean/madonna/partitioner/matrixGrid.h"
#include "src/ocean/madonna/phil/image.h"

class expansionGridTerminal;
class expansionGridEdge;
class expansionGridVertex;
class globNet;
class globTerm;
class expansionGrid;

typedef enum { Nowhere, South, East, North, West } pointOfCompass;
typedef enum { HorizontalEdge, VerticalEdge } edgeDirection;
typedef enum { PartitionGV, HorizontalChannelGV, VerticalChannelGV,
			ChannelCrossingGV } expansionGridVertexType;

class expansionGridTerminal: public graphTerminal
{
private:
   pointOfCompass _compass;
public:
   expansionGridTerminal (pointOfCompass cmpss) { _compass = cmpss; }
   pointOfCompass compass() { return _compass; }
};

class expansionGridEdge: public graphEdge
{
private:
   int _capacity, _occupation, _length;
   int underCapacityPenalty, overCapacityReward;
   int _x, _y;
   edgeDirection _direction;
public:
   expansionGridEdge (int edgeLength, int _x_, int _y_, edgeDirection dir);
   int addToOccupation (int d) { return _occupation += d; }
   int addToCapacity (int d)   { return _capacity += d; }
   const edgeDirection direction() { return _direction; }
   const int x() { return _x; }
   const int y() { return _y; }
   const int length() { return _length; }
   virtual int eWeight();
   virtual void print();
};

class expansionGridVertex: public graphVertex
{
private:
   int _x_coord, _y_coord;
   expansionGridVertexType _type;
   CIRCUITPTR _partition;
public:
   expansionGridVertex (CIRCUITPTR partition, expansionGridVertexType t,
		       int x_coord, int y_coord);
   expansionGridVertex *neighbor (pointOfCompass);
   expansionGridEdge   *edge (pointOfCompass);
   int xCoord() { return _x_coord; }
   int yCoord() { return _y_coord; }
   expansionGridVertexType type() { return _type; }
   virtual void print() { cout << form ("egVx(%d,%d) ", _x_coord, _y_coord); }
};

// A globNet ("global net") is a set of treeVertices that are visited
// by a net.
class globNet: public sdfset, public sdfsetElmt
{
private:
   NETPTR _net;
   int    _weight;
public:
   globNet (NETPTR n) : sdfset(), sdfsetElmt(), route() { _net = n; _weight = 0; }
   sdfset route;			// this contains the global route
   const NETPTR net() { return _net; }
   int weight () { return _weight; }
   int weight (const int w) { return _weight = w; }
   virtual void print() { cout << "<globNet>"; }
   int isInRoute (expansionGridEdge *e); // TRUE if e is in the global route
   int isInNet (expansionGridVertex *v);	// TRUE if v is a required vertex
   // empty
};

// this is a terminal of a globNet, i.e. this is a pointer to a gridVertex:
class globTerm: public treeVertex
{
public:
   globTerm (expansionGridVertex *egv): treeVertex (egv) {;}
   expansionGridVertex *gridVx() { return (expansionGridVertex *)vertex(); }
};

class expansionGrid
{
private:
   graphDescriptor thegraph;
   sdfset          theglobnets;
   matrixGrid      thematrix;
   TOTALPPTR       totalp;
   // heavyWeight does not really represent a routing cost; it must be
   // sufficiently large so that it is never part of the minimal steiner tree,
   // except to connect a terminal to a PartitionGV:
   int             heavyWeight;
   // the lightWeights represent the cost/unit_length of a channel:
   int             lightWeightHorizontal, lightWeightVertical;
   IMAGEDESC *image;	// the image description
   void buildExpansionGrid();
   void buildExpansionGridVertices();
   void buildExpansionGridEdges();
   void makeEdge (int x, int y, edgeDirection dir, int weight);
   void buildGlobNets();
   void makeRoutingInfo (ROUTING_INFO& rinfo, int x, int y);
   void sumRoutingInfo (int /* HOR or VER */, int i, ROUTING_CHANNEL& channel_i);
   void printRouteGlobNet (globNet *gnet);
   void printRouteSummary();
   void EGerror (const char *msg);
public:
   expansionGrid (TOTALPPTR total);
   const int nx() { return 2*totalp->nx - 1; } // # of x-coords in the grid
   const int ny() { return 2*totalp->ny - 1; } // # of y-coords in the grid`
   void instance2xy (CIRINSTPTR cirinst, int& x, int& y);
   void routeGlobNet (globNet *);	// route one global net
   void routeGlobNets();	        // route all global nets
   void summarizeGlobNets();		// compute totalp->routing
   int nwires (pointOfCompass poc1, pointOfCompass poc2,
	      int x, int y, int *length = NULL); // #wires through [x][y]
   expansionGridEdge *findEdge (expansionGridVertex *, pointOfCompass);
   int roundWiresToCells (int direction, int n_wires);
   void printRoute (globNet *, ostream& = cout); // print route of net in ASCII
   void printRouting (ostream& = cout); // print the summary in totalp->routing
};

#endif
