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
#ifndef __IOSTREAM_H
#include <iostream>
using namespace std;
#define __IOSTREAM_H
#endif

#ifndef __STDLIB_H
#include <stdlib.h>
#define __STDLIB_H
#endif

#ifndef __BASEDEFS_H
#include "src/ocean/madonna/lia/basedefs.h"
#endif

#ifndef __ITEM_H
#include "src/ocean/madonna/lia/item.h"
#endif

#ifndef __BOX_H
#include "src/ocean/madonna/lia/box.h"
#endif

#ifndef __BASEARR_H
#include "src/ocean/madonna/lia/basearr.h"
#endif

#include "src/ocean/libseadif/sea_decl.h" // for sdfexit()

BaseArray::BaseArray(int anUpper, int aLower, sizeType aDelta)
{
  bottombound = whereToAdd = aLower;
  topbound = anUpper;
  delta = aDelta;
  theArray = new Item *[arraySize()];
  for (unsigned int i = 0; i < arraySize(); i++) theArray[i] = NOTHING;
}

BaseArray::~BaseArray()
{
  for (unsigned int i = 0; i < arraySize(); i++)
    if (theArray[i] != NOTHING) delete theArray[i];
  delete theArray;
}

void BaseArray::pickOut(const Item& toPick, int deleteItemToo)
{
  if (toPick == NOITEM) return;

  for (unsigned int i = 0; i < arraySize(); i++) {
    if (theArray[i] != NOTHING && *theArray[i] == toPick) {
      if (deleteItemToo) delete theArray[i];
      theArray[i] = NOTHING;
      itemsInBox--;
      break;
    }
  }
}

void BaseArray::pickOut(int atIndex, int deleteItemToo)
{
  if (theArray[atIndex - bottombound] != NOTHING) {
    if (deleteItemToo) delete theArray[atIndex - bottombound];
    theArray[atIndex - bottombound] = NOTHING;
    itemsInBox--;
  }
}

void BaseArray::reallocate(sizeType newSize)
{
  if (delta == 0) {
    cerr << "Dummy:  Attempting to expand a fixed size array.";
    sdfexit(1);
  }

  if (newSize <= arraySize()) {
     cerr << "assertion failed, file basearr.C\n" << flush;
     sdfexit(1);
  }

  sizeType adjustedSize = newSize + (delta - (newSize % delta));

  Item **newArray = new Item *[adjustedSize];
  if (!newArray) {
    cerr << "Dummy:  Out of Memory";
    sdfexit(1);
  }

  unsigned int i;
  for (i = 0; i < arraySize(); i++) newArray[i] = theArray[i];
  for (    ; i < adjustedSize; i++) newArray[i] = NOTHING;

  delete theArray;
  theArray = newArray;
  topbound = adjustedSize + bottombound - 1;
}

int BaseArray::isEqual(const Item& testItem) const
{
  if (bottombound != ((BaseArray&)testItem).bottombound ||
      topbound    != ((BaseArray&)testItem).topbound) return 0;

  for (unsigned int i = 0; i < arraySize(); i++) {
    if (theArray[i] != NOTHING) {
      if (((BaseArray&)testItem).theArray[i] != NOTHING) {
	if (*theArray[i] != *(((BaseArray &)testItem).theArray[i])) return 0;
      }
      else return 0;
    }
    else if (((BaseArray&)testItem).theArray[i] != NOTHING) return 0;
  }
  return 1;
}

BoxIterator& BaseArray::initIterator() const
{
  return *((BoxIterator *)new ArrayIterator(*this));
}

void BaseArray::printContentsOn(ostream& out) const
{
  BoxIterator& printIterator = initIterator();

  makeHeader(out);

  while (int(printIterator) != 0)
  {
    Item& arrayItem = printIterator++;
    if (arrayItem != NOITEM)
    {
      arrayItem.print(out);

      if (int(printIterator) != 0) makeSeparator(out);
      else break;
    }
  }

  makeFooter(out);
  delete &printIterator;
}

ArrayIterator::~ArrayIterator()
{
}

ArrayIterator::operator int()
{
  return currentIndex <= beingIterated.topbound;
}

ArrayIterator::operator Item&()
{
  if (currentIndex <= beingIterated.topbound) {
    return ((Item&)(beingIterated.itemAt(currentIndex)));
  }
  return NOITEM;
}

Item& ArrayIterator::get()
{
  if (currentIndex <= beingIterated.topbound) {
    return ((Item&)(beingIterated.itemAt(currentIndex)));
  }
  return NOITEM;
}

void ArrayIterator::restart()
{
  currentIndex = beingIterated.bottombound;
}

Item& ArrayIterator::operator ++(int)
{
  if (currentIndex <= beingIterated.topbound) {
    currentIndex++;
    return ((Item&)(beingIterated.itemAt(currentIndex - 1)));
  }
  return NOITEM;
}
