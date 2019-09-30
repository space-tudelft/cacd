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

#ifndef __BASEDEFS_H
#include "src/ocean/madonna/lia/basedefs.h"
#endif

#ifndef __BOX_H
#include "src/ocean/madonna/lia/box.h"
#endif

Box::Box(const Box& /* toCopy */)
{
}

int Box::isEqual(const Item& testBox) const
{
  BoxIterator& thisIterator = initIterator();
  BoxIterator& testBoxIterator = ((Box &)(testBox)).initIterator();

  while (int(thisIterator) != 0 && int(testBoxIterator) != 0)
  {
    int itemsAreNotEqual = (thisIterator++ != testBoxIterator++);
    if (itemsAreNotEqual)
    {
      delete &testBoxIterator;
      delete &thisIterator;
      return 0;
    }
  }

  if (int(thisIterator) != 0 || int(testBoxIterator) != 0) {
    delete &testBoxIterator;
    delete &thisIterator;
    return 0;
  }
  else {
    delete &testBoxIterator;
    delete &thisIterator;
    return 1;
  }
}

void Box::print(ostream& out) const
{
  BoxIterator& printIterator = initIterator();

  makeHeader(out);

  while (int(printIterator) != 0)
  {
    printIterator++.print(out);
    if (int(printIterator) != 0) makeSeparator(out);
    else break;
  }

  makeFooter(out);
  delete &printIterator;
}

void Box::makeHeader(ostream& out) const
{
  out << myName() << " { ";
}

void Box::makeSeparator(ostream& out) const
{
  out << ",\n    ";
}

void Box::makeFooter(ostream& out) const
{
  out << " }\n";
}

BoxIterator::~BoxIterator()
{
}
