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
#ifndef __BASEDEFS_H
#include "src/ocean/madonna/lia/basedefs.h"
#endif

#ifndef __PACKAGE_H
#include "src/ocean/madonna/lia/package.h"
#endif

#ifndef __BOX_H
#include "src/ocean/madonna/lia/box.h"
#endif

Item& Package::findItem(const Item& testItem) const
{
  BoxIterator& boxIterator = initIterator();

  while (int(boxIterator) != 0)
  {
    Item& listItem = boxIterator++;

    if (listItem == testItem) {
      delete &boxIterator;
      return listItem;
    }
  }
  delete &boxIterator;
  return NOITEM;
}

int Package::hasItem(const Item& testItem) const
{
  return findItem(testItem) != NOITEM;
}
