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

#include "src/ocean/madonna/phil/cri.h"

// Operator |= for lists of CriPoints.
// Adds to l1 only this elements of l2 which are not in common.
//
// If we want to use this operator for list of CriPoint and CriCandidate
// then l1 must be of type CriPoint and l2 of type CriCandidate !
//
List& operator |=(List& l1, List& l2)
{
  if (l2.getItemsInBox())
  {
    for (ListIterator crIter(l2); (Item&)crIter != NOITEM; crIter++)
    {
      // because member function hasItem is checking first if types
      // of objects are the same we have to create temporary object
      // This is because we're using this operator for CriCandidates too

      CriPoint &prev = (CriPoint&)(Item&)crIter;
      CriPoint p2(prev);

      if (! l1.hasItem (p2))
      {
	CriPoint &prev = (CriPoint&)(Item&)crIter;
	CriPoint &newRef = *new CriPoint(prev);
	l1.add (newRef);
      }
    }
  }
  return l1;
}

// Operator & for lists of CriPoints. Returns 1 if both lists contain
// at least one common point which is either connected with contrained net
// or to two different free nets.
//
int operator &(List& l1, List& l2)
{
  if (l2.getItemsInBox())
  {
    for (ListIterator crIter(l2); (Item&)crIter != NOITEM; crIter++)
    {
      CriPoint &foundRef = (CriPoint&)l1.findItem((Item&)crIter);
      if (foundRef != NOITEM)
      {
	CriPoint &curRef= (CriPoint&)(Item&)crIter;

	if (curRef.net == 1 || foundRef.net == 1 || curRef.net != foundRef.net)
		// .net == 0 means constrained net
	  return 1;
      }
    }
  }
  return 0;
}

void CriPoint::print(ostream &os) const
{
  os << "CriPoint(x=" << x << ",y="<< y << ",l=" << (int)layer << ",net=" << (int)net <<  ")";
}
