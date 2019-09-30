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
 * Test of the Array class
 */

#include "src/ocean/libocean/Array.h"

class  Point:public Object
{
public:
     Point() { x = y = 0; }
     Point(int a,int b) { x = a; y = b; }
  virtual ~Point();

  virtual classType       desc() const { return 101; }
  virtual const   char*   className() const { return "Point"; }

  virtual Boolean         isEqual(const Object& ob) const
	{ return Boolean(x == ((Point&)ob).x && y == ((Point&)ob).y); }

  virtual Object*         copy() const { return new Point(x,y); }

  virtual void            printOn (ostream& strm = cout) const;
  virtual void            scanFrom(istream&) {};

  virtual unsigned        hash() const { return 0; }

private:
    int x;
    int y;
};

Point::~Point()
{
  cout << "Removing point " << *this << endl;
}

void Point::printOn(ostream &os) const
{
  os << "{ " << className() << " (" << x << "," << y << ") }";
}

void main()
{
  Array array(10,5);
  Point *p1 = new Point(1,1);
  Point p2(2,2);
  Point p19(19,19);
  cout << "adding some elements using add(),addAt(), and operator=" << endl;

  array.add (*p1);
  array[1] = p2;
  array[19] = p19;
  array.addAt(*new Point(12,12),12).addAt(*new Point(18,18),18);

  cout << " Test of [] operator ..Printing :" << endl;

  cout << array[0] << endl;
  cout << array[1] << endl;
  cout << array[12] << endl;
  cout << array[18] << endl;
  cout << array[19] << endl;
  cout << array[25] << endl;

  cout << "Removing  points from pos 0 12 & 18 ..." << endl;

  array.remove (Point(12,12));
  array.removeFrom(0);

  array[18] = NOTHING;

  cout << array << endl;

  cout << " puting  point 18 at pos.0 ..." << endl;

  array[1] = array[19];

  cout << array << endl;

  cout << "Test of copy constructor" << endl;

  Array array2(array);

  cout << array2;

  cout << "Test of equality operator" << endl;

  if (array == array2)
    cout << " array == array2" << endl;

  cout << "Test of capacity member" << endl;

  cout << "array " << array.capacity() << endl;

  cout << "Test of deepCopy member" << endl;

  Array &array3 = (Array&)*array.deepCopy();

  cout << array3;

  Point &pRef1= (Point&)array[19];
  Point &pRef2= (Point&)array3[19];

  if (!pRef1.isSame(pRef2))
    cout << " pRef1 and pRef2 are different objects" << endl;

  cout << "Test of addContentsTo  member" << endl;

  Array a4(5);

  array3.add(a4);

  array3.addContentsTo(array);

  cout << "array =" << endl;

  cout << array << endl;

  cout << "Test of Iterator" << endl;

  Iterator iter(array);

  array.printHeader(cout);

  while (int(iter))
  {
    Object& o = iter++;
    o.printOn(cout);
    if (int(iter)) array.printSeparator(cout);
  }
  array.printTrailer(cout);

  cout << "Test of includes() member .." << endl;

  if (array3.includes(p19))
    cout << "array3 includes element p1" << endl;

  cout << "Test of findMember() member .." << endl;

  cout << " I found " << array3.findMember(Point(19,19)) << endl;

  cout << "Test of occurrencesOf() member .." << endl;

  array3.add(*new Point(25,25));

  cout << "I think that I have " << array3.occurrencesOf(Point(25,25)) << " occurrences of point (25,25)" << endl;

  cout << "Test of removeAll() member .." << endl;

  array.removeAll();

  cout << " array after cleaning " << array << endl;

  cout << "Test of scanFrom() member .." << endl;

  array.scanFrom(cin);

  cout << "Dumping arrays test" << endl;

  cout << "array = ";
  array.dumpOn();
  cout << "array2 = ";
  array2.dumpOn();
  cout << "array3 = ";
  array3.dumpOn();

  cout << "Now destroying array3..." << endl;

  delete (Object*)&array3;

  cout << "Success !\n";
}
