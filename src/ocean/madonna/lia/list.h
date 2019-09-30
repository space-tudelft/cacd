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
#ifndef __LIST_H
#define __LIST_H

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

#ifndef __LSTELEM_H
#include "src/ocean/madonna/lia/lstelem.h"
#endif

#ifndef __PACKAGE_H
#include "src/ocean/madonna/lia/package.h"
#endif

#ifndef __BOX_H
#include "src/ocean/madonna/lia/box.h"
#endif

class List : public Package
{
public:
  List() { head = NULL; }
  virtual ~List();

  Item&         peekHead() const { return *(head -> data); }

  void          add(Item&);
  void          pickOut(const Item&, int = 0);
  void          remove (const Item& l) { pickOut(l, 1); }

  virtual classType    myNo() const;
  virtual char        *myName() const;
  virtual BoxIterator& initIterator() const;

private:
  ListElement *head;
  friend class ListIterator;
};

class ListIterator : public BoxIterator
{
public:
  ListIterator(const List&);
  virtual ~ListIterator();

  virtual           operator int();
  virtual           operator Item&();
  virtual Item&     get();
  virtual Item&     operator ++(int);
  virtual void      restart();

private:
  ListElement *currentElement;
  ListElement *startingElement;
};

inline ListIterator::ListIterator(const List& toIterate)
{
  currentElement = startingElement = toIterate.head;
}

#endif
