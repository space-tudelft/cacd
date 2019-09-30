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
#include "src/ocean/madonna/partitioner/matrixInt.h"
#include "src/ocean/libocean/format.h"
#include <iostream>
using namespace std;

// constructor
matrixInt::matrixInt (int dimension1, int dimension2)
{
   createAndInitialize (dimension1, dimension2);
}

// copy operator
// this does nothing special, but i just like to make explicit
// that this does _NOT_ allocate new memory for a new array
matrixInt::matrixInt (matrixInt& orig)
{
   dim1 = orig.dim1;
   dim2 = orig.dim2;
   thematrix = orig.thematrix; // do not copy the array, just the pointer
}

// destructor
matrixInt::~matrixInt ()
{
   for (int i = 0; i < dim1; ++i) delete thematrix[i];
   delete thematrix;
}

// This function creates a 2 dimensional integer matrixInt[dim1][dim2] at run time
// and initializes it to zero.
void matrixInt::createAndInitialize (int dimension1, int dimension2)
{
   dim1 = dimension1;
   dim2 = dimension2;
   thematrix = new int* [dim1];
   for (int i = 0; i < dim1; ++i) {
      int *subarray = new int [dim2];
      for (int j = 0; j < dim2; ++j) subarray[j] = 0; // init matrix element
      thematrix[i] = subarray;
   }
}

// exchange two rows in the matrix:
matrixInt& matrixInt::exchangeRows (int a, int b)
{
   int *tmpRow  = thematrix[a];
   thematrix[a] = thematrix[b];
   thematrix[b] = tmpRow;
   return *this;
}

// exchange two columns in the matrix:
matrixInt& matrixInt::exchangeColumns (int a, int b)
{
   int i = 0;
   while (i < dim1) {
      int *thisRow = thematrix[i++];
      int tmp    = thisRow[a];
      thisRow[a] = thisRow[b];
      thisRow[b] = tmp;
   }
   return *this;
}

// Print the matrix, if \(transp != NULL\) exchange x and y coordinates.
void matrixInt::print (Transposition transp)
{
   int vertical   = transp == Transposed ? dim2 : dim1;
   int horizontal = transp == Transposed ? dim1 : dim2;
   for (int i = 0; i < vertical; ++i) {
      cout << "\n";
      for (int j = 0; j < horizontal; ++j)
	 cout << form (" %3d", transp == Transposed ? thematrix[j][i] : thematrix[i][j]) << flush;
   }
   cout << "\n\n" << flush;
}
