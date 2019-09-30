/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Simon de Graaf
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
 * SIMPLE CLASS LIBRARY
 *
 * This file contains implementations of classes: Container
 */

#include "src/ocean/libocean/Container.h"

Container::Container ()
{
// does nothing 'cos there won't be any instance of this class
}

Container::~Container ()
{
// does nothing 'cos there won't be any instance of this class
}

Boolean Container::isEqual (const Object& o) const
{
  Iterator iterThis (*this), iterOther ((Container&)o);

  while (int(iterThis) != 0 && int(iterOther) != 0)
  {
    if (int(iterThis != iterOther)) return false;
    ++iterThis;
    ++iterOther;
  }
  if (int(iterThis) == 0 && int(iterOther) == 0)
    return true;
  else
    return false;
}

void Container::doForEach (iFunType f, void *arg)
//
//  We call this function for each memeber of container.
{
  Iterator iter (*this);

  while (int(iter) != 0) {
    f (iter, arg);        // we call for each member
    ++iter;
  }
}

Object& Container::firstSatisfying (cFunType f, void *arg) const
{
  Iterator iter (*this);

  while (int(iter) != 0)
  {
    Object& cur = iter;
    int result = f (iter, arg);
    ++iter;
    if (result == 0)
      return cur;
  }
  return NOTHING;
}

Object& Container::lastSatisfying (cFunType f, void *arg) const
{
  Iterator iter (*this);
  Object* last = nil;

  while (int(iter) != 0)
  {
    Object& cur = iter;
    int result = f (iter, arg);
    ++iter;
    if (result == 0) last = &cur;
  }
  return *last;
}

Object* Container::copy () const
//
//  copy*** functions group duplicates objects from our container (Allocates
//  them), while add*** functions only add them to another container without
//  duplicating.
{
  return shallowCopy ();
}

const Container& Container::addAll (const Container& c)
//
//  copy*** functions group duplicates objects from our container (Allocates
//  them), while add*** functions only add them to another container without
//  duplicating.
{
  c.addContentsTo (*this);
  return c;
}

const Container& Container::addContentsTo (Container& c) const
{
  Iterator iter (*this);

  while (int(iter))
  {
    Object &o = iter;
    ++iter;
    c.add(o);
  }
  return (Container&)*this;
}

void Container::doReset(Iterator& pos) const
//
// Resets given iterator. Derived classes should define (but don't have to) their
// own routine.
{
  pos.index = 0;
  pos.ptr = nil;
}

Boolean Container::includes (const Object& o) const
{
  return Boolean (occurrencesOf(o) != 0);
}

Object& Container::findMember (const Object& o) const
{
  Iterator iter (*this);

  while (int(iter) != 0)
  {
    Object& test = iter;
    ++iter;
    if (test == o) return test;
  }
  return NOTHING;
}

unsigned Container::occurrencesOf (const Object& o) const
{
  Iterator iter (*this);
  unsigned count = 0;
  while (int(iter))
  {
    Object& test = iter;
    ++iter;
    if (test == o) count++;
  }
  return count;
}

void Container::removeAll ()
{
  Iterator iter (*this);
  Object* o;

  while (int(iter))
  {
    o = &(Object&)iter;
    ++iter;
    o = remove (*o);
    if (o->canBeDeleted()) delete o;
  }
}

const Container& Container::removeAll (const Container& c)
{
  Iterator iter (c);
  Object* o;

  while (int(iter))
  {
    o = &(Object&)iter;
    ++iter;
    o = remove (*o);
    if (o->canBeDeleted()) delete o;
  }
  return (Container&)*this;
}

void Container::printOn (ostream &os) const
//
// Print all elements.
{
  Iterator iter (*this);

  printHeader (os);
  while (int(iter))
  {
    Object& o = iter;
    ++iter;
    o.printOn (os);
    if (int(iter)) printSeparator (os);
  }
  printTrailer (os);
}

void Container::printHeader (ostream &os) const
{
  os << "\n{ " << className() << " ";
}

void Container::printSeparator (ostream& os) const
{
  os << " ";
}

void Container::printTrailer (ostream& os) const
{
  os << "}\n";
}

void Container::scanFrom (istream& is)
//
// Scans all object which contains.
{
  Iterator iter (*this);

  while (int(iter))
  {
    iter.scanFrom (is);
    ++iter;
  }
}
