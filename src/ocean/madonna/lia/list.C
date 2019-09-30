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
#ifndef __IOSTREAM_H
#include <iostream>
using namespace std;
#define __IOSTREAM_H
#endif

#ifndef __BASEDEFS_H
#include "src/ocean/madonna/lia/basedefs.h"
#endif

#ifndef __ITEM_H
#include "src/ocean/madonna/lia/item.h"
#endif

#ifndef __BOX_H
#include "src/ocean/madonna/lia/box.h"
#endif

#ifndef __LIST_H
#include "src/ocean/madonna/lia/list.h"
#endif

#ifndef __LSTELEM_H
#include "src/ocean/madonna/lia/lstelem.h"
#endif

List::~List()
{
  while (head) {
    ListElement *temp = head;
    head = head -> next;
    delete temp;
  }
}

void List::add(Item& toAdd)
{
  ListElement *newElement = new ListElement(&toAdd);

  newElement -> next = head;
  head = newElement;
  itemsInBox++;
}

void List::pickOut(const Item& toDetach, int deleteItemToo)
{
  ListElement *cursor = head;

  if (*(head -> data) == toDetach)
  {
    head = head -> next;
  }
  else
  {
    ListElement *trailer = head;

    cursor = head -> next;
    while (cursor)
    {
      if (*(cursor -> data) == toDetach)
      {
	trailer -> next = cursor -> next;
	break;
      }
      else
      {
        trailer = cursor;
        cursor = cursor -> next;
      }
    }
  }

  if (cursor) {
    itemsInBox--;
    if (!deleteItemToo) cursor -> data = 0;
    delete cursor;
  }
}

classType List::myNo() const
{
  return listClass;
}

char *List::myName() const
{
  return (char*)"List";
}

BoxIterator& List::initIterator() const
{
  return *((BoxIterator *)new ListIterator(*this));
}

ListIterator::~ListIterator()
{
}

ListIterator::operator int()
{
  return currentElement != NULL;
}

ListIterator::operator Item&()
{
  if (currentElement) return ((Item&)(*(currentElement -> data)));
  return NOITEM;
}

Item& ListIterator::get()
{
  if (currentElement) return ((Item&)(*(currentElement -> data)));
  return NOITEM;
}

Item& ListIterator::operator ++(int)
{
  if (currentElement) {
    ListElement *trailer = currentElement;
    currentElement = currentElement -> next;
    return ((Item&)(*(trailer -> data)));
  }
  return NOITEM;
}

void ListIterator::restart()
{
  currentElement = startingElement;
}
