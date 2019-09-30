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
#include "src/ocean/madonna/lia/item.h"
#endif
#ifndef __STDLIB_H
#include <stdlib.h>
#endif

#ifdef USE_MNEW_ALLOCATOR
#define EXTRASIZ sizeof(double)
#include "src/ocean/libseadif/sea_decl.h" // prototypes mnew() and mfree()
#endif

Item::Item()
{
   int x = 4;
}

Item::~Item()
{
}

void *Item::operator new(size_t s)
{
#ifdef USE_MNEW_ALLOCATOR
   void *allc = mnew(EXTRASIZ + s);
   if (allc) {
      *((int *)allc) = s + EXTRASIZ; // remember the size of this object
      char *cp = (char *)allc;
      cp += EXTRASIZ;		// now allc points to the requested object
      allc = (void *)cp;
      ((Item *)cp)->Item();
   }
#else
  void *allc = ::operator new(s);
#endif
  if (!allc) {
    cerr << "\n Cannot allocate new object.\n";
    exit(1);
    return (void *)NOTHING;
  }
  return allc;
}

void Item::operator delete(void* toFree)
{
#ifdef USE_MNEW_ALLOCATOR
   char *cp = (char *)toFree;
   cp -= EXTRASIZ;		// Point to the real start of this thing
   int size = *((int *)cp);	// Find out what the size of this object is
   mfree((char **)cp,size);	// Release it
#else
   ::operator delete(toFree);
#endif
}

Dummy::~Dummy()
{
}

void Dummy::operator delete(void *)
{
}

classType Dummy::myNo() const
{
  return dummyClass;
}

char *Dummy::myName() const
{
  return (char*)"Dummy";
}

void Dummy::print(ostream& out) const
{
  out << "Dummy\n";
}

int Dummy::isEqual(const Item& testItem) const
{
  return &testItem == this;
}

Dummy theDummyItem;

Item *Item::NOTHING = (Item *)&theDummyItem;
