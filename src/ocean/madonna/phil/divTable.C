/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 *	Paul Stravers
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

#include "src/ocean/madonna/phil/divTable.h"
#include <fstream>

using namespace std;

DivTable::DivTable (char *fileName, char *fileName2): Array (10, 0, 10)
//
// Constructor: reads our table from file.
{
  int i, x, y;
  char c;
  ifstream ins;
  Point *newDiv;

  ins.open (fileName);
  if (!ins) ins.open (fileName2);
  if (!ins)
  {
    cerr << "\n WARNING : Cannot find file " << fileName << " or " << fileName2 << "\n";
    cerr << "   The default way of dividing a rectangle into n\n";
    cerr << "   parts will be used .." << endl << endl;

    newDiv = new Point ( 2, 2); addAt (*newDiv, 4);
    newDiv = new Point ( 3, 3); addAt (*newDiv, 9);
    newDiv = new Point ( 4, 4); addAt (*newDiv, 16);
    newDiv = new Point ( 5, 4); addAt (*newDiv, 20);
    newDiv = new Point ( 6, 4); addAt (*newDiv, 24);
    newDiv = new Point ( 8, 4); addAt (*newDiv, 32);
    newDiv = new Point (16, 4); addAt (*newDiv, 64);
    newDiv = new Point (16, 8); addAt (*newDiv, 128);
    newDiv = new Point (32, 8); addAt (*newDiv, 256);
    newDiv = new Point (64, 8); addAt (*newDiv, 512);
  }

  while (ins)
  {
    ins >> i >> x >> y;
    if (ins.rdstate() & ios::eofbit) return;
    if (!ins) usrErr ((char*)"DivTable::DivTable", EINPDAT);

    // now skip everything to the end of the line
    while (ins.get(c) && c != '\n');

    newDiv = new Point (x, y); addAt (*newDiv, i);
  }
}
