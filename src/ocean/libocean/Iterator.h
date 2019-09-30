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
 * Iterator
 */

#ifndef __ITERATOR_H
#define __ITERATOR_H

#include "src/ocean/libocean/Container.h"

class Iterator : public Object
{
public:
    Iterator(const Container&);
   ~Iterator();

  virtual classType       desc() const { return IteratorClassDesc; }
  virtual const   char*   className() const { return "Iterator"; }

  virtual Object*         copy() const;

  virtual void            printOn(ostream& strm = cout) const;
  virtual void            scanFrom(istream& strm);

  virtual unsigned        hash() const;

          void            reset();	// reset to beginning of Collection
#if ! defined(__GNUG__)
                          operator Object& () const { return *ptr; }
					// return current object pointer
#endif
          Object&         get(void) const { return *ptr; }
          Object&         operator++ ();
          const Container* container() const { return cntr; }
                          operator Boolean () const { return Boolean(ptr != nil); }
                          operator int () const { return ptr != nil; }

//Data members:

          int             index;	// index of next Object
          Object*	  ptr;		// pointer to current Object or NULL

private:

  virtual Boolean         isEqual(const Object& ob) const; // should not implement

    const Container* cntr;
};

inline  Object& Iterator::operator++ ()
{
  Object* cur = ptr;
  ptr = cntr->doNext(*this);
  return *cur;
}

#endif
