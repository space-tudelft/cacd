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
/*
 * Classes for dealing with critical points.
 * Contains:
 *         CriPoint
 *         CriNet
 */
#ifndef __CRI_H
#define __CRI_H

#include "src/ocean/madonna/phil/point.h"
#include "src/ocean/madonna/lia/list.h"

// Describes one critical point inside cluster.
// Critical point is a point laying on one of mirror axis where
// there\'s a layer connected to one of the potencial points.
// x,y -represent coordinates of point inside cluster.
// layer - number of layer
// kind - type of critical point

class CriPoint : public Point
{
public:
  CriPoint(): Point() {};
  CriPoint(int x, int y, layerType lay, unsigned int k): Point(x,y), layer(lay), net(k) {};
 ~CriPoint() {};

  virtual classType myNo()   const { return CriPointClass; }
  virtual char     *myName() const { return (char*)"CriPoint"; }
  virtual void      print(ostream&) const;
  virtual int       isEqual(const Item& o) const { return  ((CriPoint&)o).x == x
							&& ((CriPoint&)o).y == y
							&& ((CriPoint&)o).layer == layer; }
  friend List&      operator |=(List&, List&);
  friend int        operator & (List&, List&);

// data members

  layerType  layer : 6;    // layer number
  unsigned int net : 4;    // critical "free"net identyfier (max 15)
			   // 1 - means "constrained" net
};

//  Critical net is such a net with is electrically connected
//  with one of critical points.There can be two kinds of them :
//  1. constrained - two such nets connot meet together in critical point
//              (for example  in or out) always have id == 0
//  2. free        - otherwise (for example: vdd , vss)
//  Constrained crit. nets can be user defined and are placed in static
//  array which is a member of class Pattern
//
class CriNet : public Item
{
public:
  CriNet(char *n): name(n) {}; // Caution !! name should be canonized
 ~CriNet() {};

  virtual classType myNo()   const { return CriNetClass; }
  virtual char     *myName() const { return (char*)"CriNet"; }
  virtual void      print(ostream& os) const { os << "(" << name << ")"; };
  virtual int       isEqual(const Item& o) const { return ((CriNet&)o).name == name; }

// data members:

  char *name;
};

#endif
