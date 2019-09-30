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
 * This file contains implementations of classes:
 */

#include "src/ocean/libocean/Object.h"
#include <stdlib.h>

int Root::errMsgNum = 13;

/////////////////////////////////////////////////////////////////////////////
//
// Here there're definitions of typical error which can be referenced
// in functions from *error* group only by giving a number.
// Error numbers are defined in file Root.h. If You wan't to extend this
// table You should:
//   1. increase Root::errMsgNum
//   2. add Your new error string to Root::errMsgs
//   3. define number of error in file Root.h
//
/////////////////////////////////////////////////////////////////////////////

const char *Root::errMsgs[] = {
/*  NO_ERROR    0   */    " ",
/*  EDOS        1   */    "Operating System Dummy",
/*  ESYS        2   */    "C++ System Dummy",
/*  ENOTMEM     3   */    "Not enough memory",
/*  EWRCFG      4   */    "Wrong parameter",
/*  EINPDAT     5   */    "Input data error",
/*  EFNFND      6   */    "File not found",
/*  EUNKNOW     7   */    "Unknown error",
/*  EWRITE      8   */    "Can not write data",
/*  ETASKIMP    9   */    "Can not do task",
/*  EINDEX     10   */    "Index out of range",
/*  ENOTIMP    11   */    "Sorry, not implemented",
/*  ESHNIMP    12   */    "Should not be implemented"
};

void Root::criticalError (int err, Object& o)
{
  error (err, o);
  exit (1);
}

void Root::criticalError (char* msg, Object& o)
{
  error (msg, o);
  exit (1);
}

void Root::error (int err, Object& o)
{
  cerr << "\n Error : " << errMsgs[err] << " ! ";
  if (&o != Object::nil)
  {
    cerr << " in object of " << o.className() << " class.";
  }
  cerr << endl;
}

void Root::error (char* msg, Object& o)
{
  cerr << "\n Error : " << msg << " ! ";
  if (&o != Object::nil)
  {
    cerr << " in object of " << o.className() << " class.";
  }
  cerr << endl;
}

void Root::warning (int err, Object& o)
{
  cerr << "\n Warning : " << errMsgs[err] << " ! ";
  if (o != NOTHING)
  {
    cerr << " in object of " << o.className() << " class.";
  }
  cerr << endl;
}

void Root::warning (char *msg, Object& o)
{
  cerr << "\n Warning : " << msg << " ! ";
  if (o != NOTHING)
  {
    cerr << " in object of " << o.className() << "class.";
  }
  cerr << endl;
}

// allocates two dimensional array of data objects of requested size.
void **Root::allocArray2 (int xsize, int ysize, int itemSize)
{
  void **array;

  if (!(array = (void**)calloc (xsize, sizeof(void*))))
    criticalError (ENOTMEM, NOTHING);

  void **colPtr = array;
  for (int i = 0; i < xsize; i++, colPtr++)
  {
    if (!(*colPtr = calloc (ysize, itemSize)))
      criticalError (ENOTMEM, NOTHING);
  }
  return array ;
}

void Root::freeArray2 (int xsize, void **toFree)
{
  void **array = toFree;

  for (int i = 0; i < xsize; i++, toFree++) free (*toFree);
  free (array);
}
