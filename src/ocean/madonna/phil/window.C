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

#include "src/ocean/madonna/phil/window.h"
#include <math.h>
#include <fstream>
#include <sstream>

using namespace std;

// Function getDiv returns in which way we should divide the window, if
// we want to obtain a new window. The information about the way it
// should be partitioned is in string "str" and has a form: n=HORxVER

void Window::getDiv (char* str, int* colNum, int* rowNum, int* newWidth, int* newHigh)
{
  int num, x, y;
  char c;
  istringstream is(str);

  is >> num >> c >> x >> c >> y;
  if (!is || x <= 0 || y <= 0) usrErr ((char*)"Window::getDiv", EINPDAT);

  *newWidth = width / x;
  *newHigh  = high  / y;
  *colNum = x;
  *rowNum = y;
}
