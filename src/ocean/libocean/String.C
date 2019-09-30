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
 * String - Pascal style string with dynamically controlled
 *	      amount of used memory.
 */

#include "src/ocean/libocean/String.h"
#include <ctype.h>

String::String (const char* s)
{
    sz = strlen (s);
    str = new char[sz+1];
    str[sz] = 0;
    strncpy (str, s, sz);
}

String::String (const String& s)
{
    sz = s.len();
    str = new char[sz+1];
    str[sz] = 0;
    strncpy (str, s.str, sz);
}

String::~String ()
{
    delete str;
}

void String::printOn (ostream& strm) const
{
    char *s = str, *e = str + sz;
    while (s != e) strm << *s++;
}

void String::scanFrom (istream& strm)
{
    char c;
    int cnt = 0;

    while (strm.get (c) && isspace (c)); // skip white space

    strm.putback (c);

    int bufSize = 200;
    char *buf = new char[bufSize+1], *s = buf;

    while (strm.get (c) && !isspace (c))
    {
	if (cnt == bufSize)
	{
	    bufSize *= 2;
	    char *newBuf = new char[bufSize+1];
	    strncpy (newBuf, buf, bufSize/2);
	    delete buf;
	    buf = newBuf;
	    s = buf + bufSize/2;
	}
	*s++ = c;
	cnt++;
    }
    *s = 0;
    delete str;
    str = buf;
    sz = cnt;
    // in fact we could have allocated slighty to much memory but who cares
}

String& String::operator= (const String& x)
{
    int newLen = x.len();

    if (sz < newLen)	// we have to make more space
    {			// we don't reallocate if we have too much space
	if (sz) delete str;
	if (newLen) {
	    str= new char[newLen+1];
	    str[newLen] = 0;
	}
    }
    sz = newLen;
    strncpy (str, x, sz);
    return *this;
}

unsigned String::hash () const
{
    unsigned h = 0;

    for (int i = 0; i < sz; i++) {
	h ^= str[i];
	h = h << 1;
    }
    return h;
}
