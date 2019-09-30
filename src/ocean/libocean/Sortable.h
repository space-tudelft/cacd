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
 * Sortable.h - the object which can be sorted of SCL class library
 */

#ifndef __SORTABLE_H
#define __SORTABLE_H

#include "src/ocean/libocean/Object.h"

class Sortable : public Object
{
public:
  Sortable() {};
  virtual ~Sortable() {};

  virtual classType       desc() const { return SortableClassDesc; }
  virtual const   char*   className() const { return "Sortable"; }

  virtual Boolean         isEqual(const Object& ob) const = 0;
  virtual Boolean         isSmaller(const Object&) const = 0;
  virtual Boolean         isSortable() const;

  virtual Object*         copy() const = 0;
  virtual unsigned        hash() const = 0;

  virtual void            printOn(ostream& strm = cout) const = 0;
};

Boolean operator <  (const Sortable&, const Sortable&);
Boolean operator >  (const Sortable&, const Sortable&);
Boolean operator <= (const Sortable&, const Sortable&);
Boolean operator >= (const Sortable&, const Sortable&);

inline Boolean operator < (const Sortable& el1, const Sortable& el2)
{
  return Boolean (el1.isSpecies(el2) && el1.isSmaller(el2));
}

inline Boolean operator > (const Sortable& el1, const Sortable& el2)
{
  return Boolean (!(el1 < el2) && el1 != el2);
}

inline Boolean operator >= (const Sortable& el1, const Sortable& el2)
{
  return Boolean (!(el1 < el2));
}

inline Boolean operator <= (const Sortable& el1, const Sortable& el2)
{
  return Boolean (el1 < el2 || el1 == el2);
}

#endif
