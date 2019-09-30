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
#include "src/ocean/madonna/partitioner/matrixGrid.h"
#include <iostream>
using namespace std;

// constructor
matrixGrid::matrixGrid (int dimension1, int dimension2)
{
   createAndInitialize (dimension1, dimension2);
}

// copy operator
// this does nothing special, but i just like to make explicit
// that this does _NOT_ allocate new memory for a new array
matrixGrid::matrixGrid (matrixGrid& orig)
{
   dim1 = orig.dim1;
   dim2 = orig.dim2;
   thematrix = orig.thematrix; // do not copy the array, just the pointer
}

// destructor
matrixGrid::~matrixGrid ()
{
   for (int i = 0; i < dim1; ++i) delete thematrix[i];
   delete thematrix;
}

// create a 2 dimensional matrixGrid[dim1][dim2] at run time
void matrixGrid::createAndInitialize (int dimension1, int dimension2)
{
   dim1 = dimension1;
   dim2 = dimension2;
   thematrix = new expansionGridVertex** [dim1];
   for (int i = 0; i < dim1; ++i) {
      expansionGridVertex **subarray = new expansionGridVertex* [dim2];
      for (int j = 0; j < dim2; ++j) subarray[j] = NULL; // init matrix element
      thematrix[i] = subarray;
   }
}
