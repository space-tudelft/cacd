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

#include <iostream>
using namespace std;
#include <iomanip>
#include "src/ocean/madonna/phil/clst.h"

Clst::Clst(int gx, int gy, int hor, int ver, int **stamp, LayMap &lMap,
	                   layerType lay, short* mtx, int check):
	                   Cluster(gx, gy, hor, ver, stamp), criticalPoints()

// Constructor: Creates one new cluster with some sectors marked
//  (if point gx,gy lays in this sector) and with an empty list of critical points.
// If gx,gy lays in a critical area then some points are added to this list.
//  iMap - image map for this layout
//  lay  - layer
//  check - when 1 then create list of potential critical points.
//
{
  int a, b;

//  layerType criLay;

  a = gx % hor;   // coordinates inside cluster
  b = gy % ver;
  if (a < 0) a += hor;
  if (b < 0) b += ver;

// first check if it's not a wire on mirror axis (then Cluster::pattern == 0)

  if (check && lay != ViaLayer && !pattern) // metal 1, metal 2 or via between them
  {
    int net = lMap.findNetId (gx, gy, lay, mtx);
    if (net) {
      CriPoint *criPtr = new CriPoint (a, b, lay, net);
      criticalPoints.add (*criPtr);
    }
  }
}

Clst& Clst::operator |=(Clst& other)
//
// performs an "or" operation on patterns and copies critical points from
// other (only these which are not in common).
{
  pattern |= other.pattern;
//criticalPoints |= other.criticalPoints;
  if (other.criticalPoints.getItemsInBox()) {
    for (ListIterator crIter(other.criticalPoints); (Item&)crIter != NOITEM; crIter++) {
      if (!criticalPoints.hasItem ((Item&)crIter)) {
	CriPoint &newRef = *new CriPoint((CriPoint&)crIter);
	criticalPoints.add (newRef);
      }
    }
  }
  return *this;
}

int Clst::operator &(Clst& other)
//
// returns true if patterns have common sectors or some conflict
// critical points exist.
{
  if (pattern & other.pattern) return 1;
//return criticalPoints & other.criticalPoints;
  if (other.criticalPoints.getItemsInBox()) {
    for (ListIterator crIter(other.criticalPoints); (Item&)crIter != NOITEM; crIter++) {
      CriPoint &foundRef = (CriPoint&)criticalPoints.findItem((Item&)crIter);
      if (foundRef != NOITEM) {
	CriPoint &curRef= (CriPoint&)(Item&)crIter;
	if (curRef.net == 1 || foundRef.net == 1 || curRef.net != foundRef.net)
		// .net == 0 means constrained net
	  return 1;
      }
    }
  }
  return 0;
}

int Clst::operator &(List& other)
{
  if (other.getItemsInBox()) {
    for (ListIterator crIter(other); (Item&)crIter != NOITEM; crIter++) {
      CriPoint &foundRef = (CriPoint&)criticalPoints.findItem((Item&)crIter);
      if (foundRef != NOITEM) {
	CriPoint &curRef= (CriPoint&)(Item&)crIter;
	if (curRef.net == 1 || foundRef.net == 1 || curRef.net != foundRef.net)
		// .net == 0 means constrained net
	  return 1;
      }
    }
  }
  return 0;
}

void Clst::print(ostream& os) const
{
  os << "Clst(cX=" << cX << ",cY=" << cY;
  os << ",pattern = " << hex << pattern << " " << dec;
  os << criticalPoints << ")";
}
