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
 * Test no.2 of the Array class: assignment.
 */

#include "src/ocean/libocean/Array.h"

class Point : public Object
{
public:
    Point() { x = y = 0; }
    Point(int a,int b) { x = a; y = b; }
    virtual ~Point();

    virtual classType	desc() const { return 101; }
    virtual const char*	className() const { return "Point"; }

    virtual Boolean	isEqual(const Object& ob) const
	{ return Boolean(x == ((Point&)ob).x && y == ((Point&)ob).y); }

    virtual Object*	copy() const { return new Point(x,y); }

    virtual void	printOn(ostream& strm = cout) const;
    virtual void	scanFrom(istream&) {};
    virtual unsigned	hash() const { return 0; }

private:
    int x;
    int y;
};

Point::~Point ()
{
    cout << "Removing point " << *this << endl;
}

void Point::printOn (ostream &os) const
{
    os << "{ " << className() << " (" << x << "," << y << ") }";
}

void main ()
{
  Array array(10,5);
  Point *p1 = new Point(1,1);
  Point p2(2,2), p3(12,12), p4(18,18);

  cout << " Test of [] operator .." << endl;

  array[0] = *p1;
  array[2] = p2;
  array[12] = p3;
  array[18] = p4;

  cout << " Printing : " << endl;

  cout << array[0] << endl;
  cout << array[1] << endl;
  cout << array[12] << endl;
  cout << array[18] << endl;
  cout << array[25] << endl;

  cout << " and whole array .. " << array << endl;

  cout << " Now lets make some mess : " << endl;

  array[1] = NOTHING;
  array[18] = NOTHING;
  array[2] = array[1];
  array[3] = array[0];
  array[4] = array[34];

  cout << " and again whole array .. " << array << endl;

  cout << "that was all ." << endl;
}
