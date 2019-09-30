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
 * Point - Point on 2-dim plane - or a pair of two integers,
 * or whatever ...
 */

#ifndef __POINT_H
#define __POINT_H

#include "src/ocean/libocean/Object.h"

class Point : public Object
{
public:
     Point() { x = y = 0; }
     Point(const int a, const int b) { x = a; y = b; }
     virtual  ~Point() {}


  virtual classType       desc() const { return PointClassDesc; }
  virtual const   char*   className() const { return "Point"; }

  virtual Boolean         isEqual(const Object& ob) const
	{ return Boolean(x == ((Point&)ob).x && y == ((Point&)ob).y); }

  virtual Object*         copy() const { return new Point(x,y); }

  virtual void            printOn(ostream& strm = cout) const;
  virtual void            scanFrom(istream& strm);

  virtual unsigned        hash() const { return x^y; }

          Point&          operator= (const Point&);

    int x;
    int y;
};

inline Point& Point::operator= (const Point& other)
{
  x = other.x;
  y = other.y;
  return *this;
}

#endif
