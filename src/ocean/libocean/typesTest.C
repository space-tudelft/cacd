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

#include "src/ocean/libocean/Char.h"
#include "src/ocean/libocean/Double.h"
#include "src/ocean/libocean/Int.h"
#include "src/ocean/libocean/Long.h"
#include "src/ocean/libocean/Rectangle.h"
#include "src/ocean/libocean/Point.h"
#include "src/ocean/libocean/Array.h"

main ()
{
  Char c;
  Double d;
  Int i;
  Long l;
  Rectangle r;
  Point p;
  Array a(10,5);

  a[0] = c;
  a[2] = d;
  a[4] = i;
  a[6] = l;
  a[8] = r;
  a[10]= p;

  cout << " Now reading ... " << endl;

  cout << "Char .." << flush;
  cin >> a[0];

  cout << "Double .." << flush;
  cin >> a[2];

  cout << "Int .." << flush;
  cin >> a[4];

  cout << "Long .." << flush;
  cin >> a[6];

  cout << "Rectangle .." << flush;
  cin >> a[8];

  cout << "Point .." << flush;
  cin >> a[10];

  cout << " now printing :" << a << endl;

  cout << " == test " << endl;

  if (a.findMember(c) == NOTHING) cout << " not working ";
  if (a.findMember(d) == NOTHING) cout << " not working ";
  if (a.findMember(i) == NOTHING) cout << " not working ";
  if (a.findMember(l) == NOTHING) cout << " not working ";
  if (a.findMember(r) == NOTHING) cout << " not working ";
  if (a.findMember(p) == NOTHING) cout << " not working ";

  cout << "thats all " <<endl;
}
