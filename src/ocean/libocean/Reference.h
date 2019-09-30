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
 * Reference
 */

#ifndef __REFERENCE_H
#define __REFERENCE_H

#include "src/ocean/libocean/Root.h"

class Reference : public Root
{
public:
    Reference() { count = 0; automatic = !ourNewFlag; ourNewFlag = 0; }

    virtual const   char*   className() const { return "Reference"; }

           void  ref();
           short unref();
inline     Boolean canBeDeleted() const;
           void  setVariableType();
           short getCount() const { return count; }
           short isAutomatic() const { return automatic; }
           short getFlag() const { return ourNewFlag; }

     ~Reference() { if (count>0) refCountError (className()); }

private:
        void  refCountError (const char* n);

        short count;
        short automatic;
static  short ourNewFlag;
};

inline void Reference::ref ()
{
  count++;
}

inline short Reference::unref ()
{
  return count--;
}

inline void Reference::setVariableType()
{
  // call this function to mark that it is not of auto type.
  ourNewFlag = 1;
}

inline Boolean Reference::canBeDeleted() const
{
  return Boolean (automatic == 0 && count <= 0);
}

#endif
