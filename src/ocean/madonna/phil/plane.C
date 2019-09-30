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
/*
 * PLANE map of already used sectors
 */

#include "src/ocean/madonna/phil/plane.h"
#include <malloc.h>

Plane::Plane(int r, int c)
//
// Constructor: Creates placement map and sets all sectors as free
//              and all lists of critical points empty.
{
  rows = r;
  cols = c;

  if (!(array = new PlaneCell*[cols]))
    usrErr ((char*)"Plane::Plane", ENOTMEM);

  for (int i = 0; i < cols; i++)
    if (!(array[i] = new PlaneCell[rows]))
      usrErr ((char*)"Plane::Plane", ENOTMEM);
}

Plane::~Plane()
//
// Destructor: frees allocated memory
{
  for (int i = 0; i < cols; i++) delete [] array[i];
  delete [] array;
}

void Plane::getEffSize(int& x, int& y)
//
// Returns the effective size of the placement plane (rectangle used by cells)
{
  x = y = 0;

  for (int i = 0; i < rows; i++)
  for (int j = 0; j < cols; j++)
      if (getPattern (j, i)) // there is something here
      {
	if (i > y) y = i;
	if (j > x) x = j;
      }
  x++;
  y++;
}
