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
 *
 * Object
 * Reference::refCountError
 */

#ifdef USE_MNEW_ALLOCATOR
#define EXTRASIZ sizeof(double)
#include "src/ocean/libseadif/sea_decl.h" // prototypes mnew() and mfree()
#endif

#include "src/ocean/libocean/Object.h"
#include <unistd.h>
// #include <osfcn.h> // sometimes this is where the _exit() prototype lives

short Reference::ourNewFlag = 0;

void* Object::operator new (size_t size)
{
#ifdef USE_MNEW_ALLOCATOR
   void *a = mnew (EXTRASIZ + size);
   if (!a) return nil;
   *((int *)a) = size + EXTRASIZ; // remember the size of this object
   char *cp = (char *)a;
   cp += EXTRASIZ;		// now a points to the requested object
   a = (void *)cp;
#else
   void *a = ::operator new (size);
   if (!a) return nil;
#endif
   ((Object*)a)->setVariableType();
   return a;
}

void Object::operator delete (void *toFree)
{
  if (toFree == nil || !toFree) // we can't delete Nil object
     warning ((char*)"You can't delete this object", NOTHING);
  else
    if (!((Object*)toFree)->canBeDeleted())
    {
      cerr << "reference count corrupted\n"
	   << "or attempted to free automatic object\n"
	   << endl;
      _exit(1);
    }
    else
#ifdef USE_MNEW_ALLOCATOR
    {
       char *cp = (char *)toFree;
       cp -= EXTRASIZ;		     // Point to the real start of this thing
       int size = *((int *)cp);	     // Find out what the size of this object is
       mfree ((char **)cp, size);    // Release it
    }
#else
    ::operator delete (toFree);
#endif
}

void Object::dumpOn (ostream& os) const
//
//  Like printOn but appeneds brackets and class name.
{
  os << "(" << className() << " ";
  printOn (os);
  os << " )";
}

void Object::scanFrom (istream& /* is */)
{
  warning (ENOTIMP, *this);
}

void Reference::refCountError (const char *n)
{
  cout << "Reference count corrupted in object of class " << n << "\n"
       << " see manual /usr/ocean/src/libocean/doc/SCL.tex" << endl;
  _exit(1);
}
