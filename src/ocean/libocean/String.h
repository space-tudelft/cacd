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
 * String - Pascal-style string with dynamically controlled amount of used memory.
 */

#include "src/ocean/libocean/Sortable.h"
#include <string.h>

class String : public Sortable
{
public:
  String (const char* s);
  String (const String& s);
 ~String ();

  virtual classType       desc() const { return StringClassDesc; }
  virtual const   char*   className() const { return "String"; }

  virtual Boolean         isEqual(const Object& ob) const
	{ return Boolean (sz == ((String&)ob).sz && !strncmp (str, ((String&)ob).str, sz)); }

  virtual Boolean         isSmaller(const Object& ob) const
	{ return Boolean (strncmp (str, ((String&)ob).str, sz) < 0); }

  virtual Object*         copy() const { return new String (str); }

  virtual void            printOn (ostream& strm = cout) const;
  virtual void            scanFrom(istream& strm);

  virtual unsigned        hash() const;
          int             len () const { return sz; }

                          operator const char* () const { return str; }
           String&        operator= (const String& x);

private:
   int sz;
   char* str;
};
