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

#ifndef __CLST_H
#define __CLST_H

#include "src/ocean/madonna/phil/cluster.h"
#include "src/ocean/madonna/phil/cri.h"
#include "src/ocean/madonna/phil/layMap.h"
#include "src/ocean/madonna/lia/array.h"
#include "src/ocean/madonna/lia/list.h"

// Contains all informations about cluster including critical points
// positions. They are represented as a list of critical points.
//
class Clst : public Cluster
{
public:
  Clst(int gX, int gY, int hor, int ver): Cluster(gX, gY, hor, ver) {};
  Clst(int gx, int gy, int hor, int ver, int **stamp): Cluster(gx, gy, hor, ver, stamp), criticalPoints() {};
  Clst(int, int, int, int, int**, LayMap&, layerType, short*, int check = 0);

  virtual classType  myNo()   const { return ClstClass; }
  virtual char      *myName() const { return (char*)"Clst"; }
  virtual void       print(ostream&) const;

  operator Boolean() { return (Boolean)(pattern || criticalPoints.getItemsInBox() > 0); }
  operator List&()   { return criticalPoints; }

  Clst&    operator |=(Clst&);
  int      operator  &(Clst&);
  int      operator  &(List&);

// data members:

  List criticalPoints;
};

#endif
