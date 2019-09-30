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
 * Object.h - the root of SCL class library
 */

#ifndef __OBJECT_H
#define __OBJECT_H

#include <iostream>
using namespace std;
#include <malloc.h>

#if defined(__GNUC__) && ((__GNUC__ >= 2 && __GNUC_MINOR__ >= 6) || __GNUC__ >= 3)
typedef bool Boolean;
#else
typedef enum { false = int(0), true = int(1) } Boolean;
#endif

typedef int (*cFunType)(const class Object&, void *);
typedef int (*iFunType)(const class Object&, void *);

#define NOTHING *Object::nil

#include "src/ocean/libocean/ClsDesc.h"
#include "src/ocean/libocean/Root.h"
#include "src/ocean/libocean/Reference.h"

#include "src/ocean/libseadif/systypes.h"

class Object : public Reference
{
public:
           Object() {}
           Object(Object&) {}
  virtual ~Object() {}

  virtual classType       desc() const { return ObjectClassDesc; }
  virtual const   char*   className() const { return "Object"; }

  virtual Boolean     isEqual(const Object& ob) const = 0;
          Boolean     isSame(const Object& ob) const { return Boolean(this == &ob); }
          Boolean     isSpecies(const Object& ob) const { return Boolean(desc() == ob.desc()); }
  virtual Boolean     isSortable() const { return false; }

          void*           operator new (size_t s);
          void            operator delete (void*);

  virtual Object*         copy() const = 0;

  virtual void            dumpOn(ostream& strm = cerr) const;
  virtual void            printOn(ostream& strm = cout) const = 0;
  virtual void            scanFrom(istream& strm);

  virtual unsigned        hash() const = 0;

  static Object* const nil; // pointer to sole instance of nil object
};

//----------------------------------------------------------------------------
//  Operators for Object class.
//----------------------------------------------------------------------------

inline istream& operator >> (istream& strm, Object& ob)
{
    ob.scanFrom(strm);
    return strm;
}

inline ostream& operator << (ostream& strm, const Object& ob)
{
    ob.printOn(strm);
    return strm;
}

inline Boolean operator == (const Object& test1, const Object& test2)
{
    return (Boolean(test1.isSpecies(test2) && test1.isEqual(test2)));
}

inline int operator != (const Object& test1, const Object& test2)
{
    return (!(test1 == test2));
}

#endif
