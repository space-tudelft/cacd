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
 * Double - Double number - object representing double number.
 */

#ifndef __DOUBLE_H
#define __DOUBLE_H

#include "src/ocean/libocean/Sortable.h"

class Double : public Sortable
{
public:
  Double() { x = 0; }
  Double(const double a) { x = a; }
  virtual ~Double() {}

  virtual classType   desc() const { return DoubleClassDesc; }
  virtual const char* className() const { return "Double"; }

  virtual Boolean     isEqual  (const Object& ob) const { return Boolean(x == ((Double&)ob).x); }
  virtual Boolean     isSmaller(const Object& ob) const { return Boolean(x <  ((Double&)ob).x); }

  virtual Object*     copy() const { return new Double(x); }

  virtual void        printOn(ostream& strm = cout) const;
  virtual void        scanFrom(istream& strm);

  virtual unsigned    hash() const { return unsigned(x); }

                      operator double() const { return x; }
          Double&     operator = (const Double& x);

  double x;
};

inline Double& Double::operator = (const Double& other)
{
  x = other.x;
  return *this;
}

#endif
