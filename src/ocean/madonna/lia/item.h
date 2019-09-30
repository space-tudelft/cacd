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
#ifndef __ITEM_H
#define __ITEM_H

#ifndef __IOSTREAM_H
#include <iostream>
using namespace std;
#define __IOSTREAM_H
#endif

#ifndef __STDDEF_H
#include <stddef.h>
#define __STDDEF_H
#endif

#ifndef __BASEDEFS_H
#include "src/ocean/madonna/lia/basedefs.h"
#endif

#ifndef __MALLOC_H
#include <malloc.h>
#endif

class Item
{
public:
  Item();
  Item(Item&) {};
  virtual ~Item();

  virtual classType myNo() const = 0;
  virtual char   *myName() const = 0;
  virtual int     isEqual(const Item&) const = 0;
  virtual void    print(ostream&) const = 0;
  void  *operator new(size_t s);
  void   operator delete(void* s);

  static Item *NOTHING;

protected:
  friend ostream& operator <<(ostream&, const Item&);
};

#define NOITEM *(Item::NOTHING)

class Dummy : private Item
{
public:
  Dummy() {};
  virtual ~Dummy();

  virtual classType myNo() const;
  virtual char     *myName() const;
  virtual int      isEqual(const Item&) const;
  virtual void     print(ostream&) const;
  void    operator delete(void *);
};

inline int operator ==(const Item& t1, const Item& t2)
{
  return ((t1.myNo() == t2.myNo()) && t1.isEqual(t2));
}

inline int operator !=(const Item& t1, const Item& t2)
{
  return (!(t1 == t2));
}

inline ostream& operator <<(ostream& out, const Item& anItem)
{
  anItem.print(out);
  return out;
}

#endif
