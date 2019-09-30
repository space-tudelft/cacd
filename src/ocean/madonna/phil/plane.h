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
 * PLANE map of already used sectors
 */

#ifndef __PLANE_H
#define __PLANE_H

#include "src/ocean/madonna/phil/clst.h"
#include "src/ocean/madonna/lia/list.h"

class PlaneCell : public List
{
public:
  PlaneCell(): List() { pattern = 0; }
 ~PlaneCell() {};

  virtual classType myNo()   const { return PlaneCellClass; }
  virtual char     *myName() const { return (char*)"PlaneCell"; }

  clusterMapType pattern;
};

// An array of cluster maps in placement area. One cell represents
// one cluster of sectors. Coordinates counted from bottom-left corner.
//
class Plane
{
public:
  Plane(int rows, int cols);
 ~Plane();

  inline clusterMapType        getPattern(int x, int y);
  inline List&                 getCriticals(int x, int y);
  inline void                  mark(int x, int y, clusterMapType, List&);
  void                         getEffSize(int& x, int& y);

  int  rows;
  int  cols;

private:
  inline Boolean               chk(int x, int y);

  PlaneCell **array;
};

inline Boolean Plane::chk(int x, int y)
//
// returns 0 if legal cluster coord.
{
  return (Boolean)(x < 0 || x >= cols || y < 0 || y >= rows);
}

inline clusterMapType Plane::getPattern(int x, int y)
//
// Returns given cluster map.If coordinates are out of range
// returns filled out map what means that this one is already occupied.
{
  if (chk(x,y)) return ~0x0; // to set all bits
  return array[x][y].pattern;
}

inline List& Plane::getCriticals(int x, int y)
//
// returns list of critical points already in cluster (x,y)
{
  if (chk(x,y)) return (List&)NOITEM;
  return (List&)array[x][y];
}

inline void Plane::mark(int x, int y, clusterMapType map, List& critListRef)
//
// marks new used sectors and adds list of critical points to our list.
{
  if (!chk(x,y)) {
    array[x][y].pattern |= map;
    if (critListRef.getItemsInBox()) {
	List& list = (List&) array[x][y];
	for (ListIterator crIter(critListRef); (Item&)crIter != NOITEM; crIter++) {
	  if (!list.hasItem ((Item&)crIter)) {
	    CriPoint &newRef = *new CriPoint((CriPoint&)crIter);
	    list.add (newRef);
	  }
	}
    }
  }
}

#endif
