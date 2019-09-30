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
 * Array - C style, dynamic, one-dimensional array of arbitrary objects
 */

#include "src/ocean/libocean/Array.h"
#include "src/ocean/libocean/Sortable.h"

Array::Array(int size, unsigned incr) // constructor
{
  sz = size;
  delta = incr;
  whereToAdd = 0;
  itemsInContainer = 0;
  if (sz == 0)
    criticalError ((char*)"Request to allocate zero size array", *this);

  v = new Object*[sz];

  register int i = sz;
  register Object** vp = v;
  while (i--) *vp++ = nil;
}

Array::Array(const Array& other)
//
// Creates  shallow copy of other array.
{
  register int i = sz = other.capacity();
  itemsInContainer = other.contains();
  delta = other.delta;
  whereToAdd = other.whereToAdd;

  v = new Object*[sz];
  register Object** vp = v;
  register Object** av = other.v;
  while (i--)
  {
    if (*av != nil)
      (**av).ref(); // we have to increase reference count because
                    // now this object will belong to two arrays
    *vp++ = *av++;
  }
}

Array::~Array() // destructor
{
  for (int i = 0; i < (int)sz; i++)
  {
    Object* toDestroy = v[i];
    if (toDestroy != nil)
    {
      toDestroy->unref();
      if (toDestroy->canBeDeleted())
	delete toDestroy;
    }
  }
  delete []v;
}

Boolean Array::isEqual(const Object& o) const
//
// Two arrays are equal if they are of the same size and contain the
// same elements.
{
  if (sz != ((Array&)o).sz) return false;

  for (int i = 0; i < (int)sz; i++)
  {
    if (*v[i] != *((Array&)o).v[i]) return false;
  }
  return true;
}

Object* Array::shallowCopy() const
{
  return new Array(*this);
}

Object* Array::deepCopy() const
//
// The same as above but duplicates of objects from array are also created
{
  Array* na = new Array(*this);

  for (int i = 0; i < (int)sz; i++)
    if (v[i] != nil)
    {
      Object& copied = *v[i];
      Object* dest = copied.copy();
      na->addAt(*dest, i);
    }
  return na;
}

Object& Array::add(Object& o)
//
// Adds a new object at first available free position.
{
  while (v[whereToAdd] != nil && whereToAdd < (int)sz)
    whereToAdd++;
  if (whereToAdd == (int)sz)
    reSize(sz+delta);
  v[whereToAdd++] = &o;
  o.ref();
  itemsInContainer++;
  return o;
}

const Container& Array::addContentsTo(Container& c) const
{
  for (int i = 0; i < (int)sz; i++)
  {
    Object& o = *v[i];
    if (o != NOTHING) ((Array&)c).addAt(o, i);
  }
  return *this;
}

Array& Array::addAt(Object& o, int i)
//
// Adds given element at specified position.
//
// **** CAUTION !!!! ****
//
// If this position points already to other object, it will be replaced
// by new one without freeing.
{
  if (i >= (int)sz) reSize(i);
  if (v[i] != nil)
  {
    v[i]->unref();
    itemsInContainer--;
    if (v[i]->canBeDeleted()) delete v[i];
  }
  v[i] = &o;
  o.ref();
  itemsInContainer++;
  return *this;
}

void Array::doReset(Iterator& pos) const
{
  pos.index = 0;
  pos.ptr = v[0];
  while (v[pos.index] == nil && ++pos.index < (int)capacity());
  if (pos.index < (int)capacity())
  {
    pos.ptr = v[pos.index];
    v[pos.index]->ref();
  }
}

Object* Array::doNext(Iterator& pos) const
{
  pos.ptr->unref();
  if (pos.ptr->canBeDeleted()) delete pos.ptr;
  while (++pos.index < (int)capacity() && v[pos.index] == nil);

  if (pos.index < (int)capacity())
  {
    v[pos.index]->ref();
    return v[pos.index];
  }
  else
    return nil;
}

Object* Array::remove(const Object& o)
//
// Removes an object from array but doesn't delete it. Returns pointer
// to removed object.
//
// *********** CAUTION ! ******************************************
// *                                                              *
// *  This is not always true:             returnedPointer == &o  *
// *  ------------------------                                    *
// ****************************************************************
{
  if (o == NOTHING) return (Object*)&o;

  for (int i = 0; i < (int)sz; i++)
  {
    if (v[i] != nil && *v[i] == o)
    {
      Object *tmp = v[i];
      tmp->unref();
      v[i] = nil;
      itemsInContainer--;
      if (i < whereToAdd) whereToAdd = i;
      return tmp;
    }
  }
  return nil;
}

Object* Array::removeFrom(int i)
//
// Removes an object from position i of the array  but doesn't delete it.
// Returns pointer to removed object.
//
// *********** CAUTION ! ******************************************
// *                                                              *
// *  This is not always true:             returnedPointer == &o  *
// *  ------------------------                                    *
// ****************************************************************
//
{
  if (i >= (int)sz || i < 0) criticalError (EINDEX, *this);
  Object* tmp = v[i];
  tmp->unref();
  v[i] = nil;
  itemsInContainer--;
  if (i < whereToAdd) whereToAdd = i;
  return tmp;
}

unsigned Array::hash() const
{
  return sz ^ delta ^ whereToAdd;
}

void Array::printOn(ostream& os) const
{
  printHeader (os);

  for (int i = 0; i < (int)sz; i++)
  {
    Object& current= *v[i];
    os << "( [" << i << "] = " << current << " )\n";
  }
  printTrailer (os);
}

void Array::dumpOn(ostream &os) const
{
  printOn(os);
  os << "[capacity = " << sz << " itemsInContainer = " << contains() << " delta = "
     << delta << " where to add = " << whereToAdd << " ]" << endl;
}

void Array::reSize(unsigned newSize)
{
  int i;
  if (delta == 0)
    criticalError ((char*)"Attempting to expand fixed size array", *this);

  unsigned nsz= newSize + (delta - (newSize % delta));

  Object **nv = new Object* [nsz];
  if (nv == 0) criticalError (ENOTMEM, *this);
  for (i = 0; i < (int)sz; i++) nv[i] = v[i];
  for (; i < (int)nsz; i++) nv[i] = nil;
  delete []v;

  v = nv;
  sz = nsz;
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// These routines support heap-sort algorithm as described in :             //
//                                                                          //
// "Introduction to Algorithms".  Cormen,Leisserson,Rivest. McGraw-Hill 1990//
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

void Array::heapify(int i)
//
// Creates heap root at position i of our array.
{
  int l = left(i),
      r = right(i);
  int largest;
  Sortable *a = (Sortable*)(Object*)v[0];

  int ii = i-1;

  if (l <= heapSize && *(Sortable*)v[l-1] > *(Sortable*)v[ii])
                                     // this -1 is because c-style arrays start
    largest = l;		     // at index 0
  else
    largest = i;

  if (r <= heapSize && (Sortable&)*v[r-1] > (Sortable&)*v[largest-1])
    largest = r;

  if (largest != i)
  {
    Object* o= v[ii];

    v[ii] = v[largest-1];
    v[largest-1] = o;

    heapify(largest);
  }
}

void Array::buildHeap()
//
// This routine builds our heap from unsorted array.
{
  heapSize = contains();

  for (int i = contains()/2; i >= 1; i--) heapify(i);
}

void Array::sort()
//
// The heap sort routine.
{
  // first check if all element in array can be sorted
  Object** ptr = v;
  int i;

  for (i = 0; i < (int)contains(); i++, ptr++)
    if (!(*ptr)->isSortable())
      criticalError ((char*)"Only objects of class Sortable can be sorted", *this);

  buildHeap();

  for (i = contains(); i >= 2; i--)
  {
    Object* o= v[0];

    v[0] = v[i-1];
    v[i-1] = o;

    heapSize--;
    heapify(1);
  }
}
