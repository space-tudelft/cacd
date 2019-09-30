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
 * This file contains implementations of classes: Iterator
 */

#include "src/ocean/libocean/Iterator.h"

Iterator::Iterator (const Container &toIterate)
//
// Reset our iterator to make it point to first element of container.
{
  index = 0;
  ptr = NULL;
  cntr = &toIterate;
  cntr->doReset (*this);
}

Iterator::~Iterator ()
{
  if (ptr != nil) ptr->unref();
  if (ptr->canBeDeleted()) delete ptr;
}

Object* Iterator::copy () const
{
  return new Iterator ((Iterator&)*this);
}

void Iterator::printOn (ostream &os) const
//
// When printing Iterator the element it is pointing to is being printed.
{
  if (ptr) os << *ptr;
}

void Iterator::scanFrom (istream & /* is */)
{
  warning (ENOTIMP, *this);
}

unsigned Iterator::hash () const
//
// just to produce an interesting value
{
  return (unsigned)((unsigned long)cntr ^ (unsigned long)ptr) ^ index;
}

Boolean Iterator::isEqual (const Object& /* o */) const
//
// We won't ever compare two iterators this way.
{
  warning (ESHNIMP, (Object&)*this);
  return false;
}

void Iterator::reset ()
{
  cntr->doReset (*this);
}
