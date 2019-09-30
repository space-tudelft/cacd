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
#ifndef __BASEARR_H
#define __BASEARR_H

#ifndef __IOSTREAM_H
#include <iostream>
using namespace std;
#define __IOSTREAM_H
#endif

#ifndef __BASEDEFS_H
#include "src/ocean/madonna/lia/basedefs.h"
#endif

#ifndef __ITEM_H
#include "src/ocean/madonna/lia/item.h"
#endif

#ifndef __PACKAGE_H
#include "src/ocean/madonna/lia/package.h"
#endif

class BaseArray : public Package
{
public:
  BaseArray(int top, int bottom = 0, sizeType aDelta = 0);
  virtual ~BaseArray();

  int             bottomBound() const { return bottombound; }
  int             topBound()    const { return topbound; }
  sizeType        arraySize()   const;

  virtual BoxIterator& initIterator() const;

  virtual void    add(Item&) = 0;
  void            remove(int i) { pickOut(i, 1); }
  void            pickOut(int, int = 0);
  virtual void    pickOut(const Item&, int = 0);

  virtual classType myNo() const = 0;
  virtual char   *myName() const = 0;
  virtual int     isEqual(const Item&) const;
  virtual void    printContentsOn(ostream&) const;

protected:
  Item& itemAt(int i) const { return *theArray[i - bottombound]; }
  void  reallocate(sizeType);
  sizeType delta;
  int      bottombound;
  int      topbound;
  int      whereToAdd;
  Item   **theArray;

  friend class ArrayIterator;
};

inline sizeType BaseArray::arraySize() const
{
  return sizeType(topbound - bottombound + 1);
}

class ArrayIterator : public BoxIterator
{
public:
  ArrayIterator(const BaseArray&);
  virtual ~ArrayIterator();

  virtual operator int();
  virtual operator Item&();
  virtual Item&    get();
  virtual Item& operator ++(int);
  virtual void     restart();

private:
  int   currentIndex;
  const BaseArray& beingIterated;
};

inline ArrayIterator::ArrayIterator(const BaseArray& toIterate) :
    currentIndex(toIterate.bottombound), beingIterated(toIterate)
{
}

#endif
