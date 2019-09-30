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
 * Nil - No object class implementation
 */

#include "src/ocean/libocean/Object.h"
#include "src/ocean/libocean/ClsDesc.h"

class Nil : public Object
{
public:
  Nil() {};
 ~Nil() {};

  virtual classType   desc() const { return NilClassDesc; }
  virtual const char* className() const { return "Nil"; }
  virtual Boolean     isEqual(const Object& ob) const { return isSame(ob); }

protected:

  virtual Object*     copy() const { return nil; }
  virtual void        printOn(ostream& strm = cout) const;
  virtual void        scanFrom(istream& strm);
  virtual unsigned    hash() const { return 0; }
};

//----------------------------------------------------------------------------
// Nil class members implementation.
//----------------------------------------------------------------------------

void Nil::printOn (ostream& strm) const
{
  strm << "NIL";
}

void Nil::scanFrom (istream& /* strm */)
//
// We can't read error object.
{
  criticalError ((char*)"You can't read this object", *this);
}

//----------------------------------------------------------------------------
// The only one instance of Nil object - referenced by Object::nil
//----------------------------------------------------------------------------

Nil theNilObject;

Object* const Object::nil = (Object*)&theNilObject;

