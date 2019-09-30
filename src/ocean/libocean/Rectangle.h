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
 * Rectangle - a window, a rectangle or whatever described by
 *		point coordinates and dimensions
 */

#ifndef __RECTANGLE_H
#define __RECTANGLE_H

#include "src/ocean/libocean/Object.h"

class Rectangle : public Object
{
public:
     Rectangle() { x = y = hor = ver = 0; }
     Rectangle(const int a, const int b, const int c, const int d) { x = a; y = b; hor = c; ver = d; }
     virtual ~Rectangle() {}

  virtual classType       desc() const { return RectangleClassDesc; }
  virtual const   char*   className() const { return "Rectangle"; }

  virtual Boolean         isEqual(const Object& ob) const
                              { return Boolean(x == ((Rectangle&)ob).x &&
					       y == ((Rectangle&)ob).y &&
					     hor == ((Rectangle&)ob).hor &&
					     ver == ((Rectangle&)ob).ver); }

  virtual Object*         copy() const { return new Rectangle(x,y,hor,ver); }

  virtual void            printOn(ostream& strm = cout) const;
  virtual void            scanFrom(istream& strm);

  virtual unsigned        hash() const { return x^y^hor^ver; }

          Rectangle&      operator= (const Rectangle& x);

    int x;
    int y;
    int hor;
    int ver;
};

inline Rectangle& Rectangle::operator= (const Rectangle& other)
{
  x = other.x;
  y = other.y;
  hor = other.hor;
  ver = other.ver;
  return *this;
}

#endif
