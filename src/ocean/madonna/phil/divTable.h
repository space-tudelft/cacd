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

#ifndef __DIVTABLE_H
#define __DIVTABLE_H

#include "src/ocean/madonna/lia/array.h"
#include "src/ocean/madonna/phil/point.h"

// Contains table of pairs (Point) of numbers, on position num is
// the recommended way of dividing rectangle.
// Items of class Point act the way:
//	x - suggested number of sections in horizontal dir
//	y - suggested number of sections in vertical dir
//
class DivTable : public Array
{
public:
  DivTable(char*fileName = (char*)"div.tab", char*fileName2 = (char*)"seadif/div.tab");
 ~DivTable() {};

  virtual classType myNo() const { return DivTabClass; }
  virtual char*   myName() const { return (char*)"DivTab"; }
};

#endif
