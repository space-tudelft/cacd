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

#ifndef __WINDOW_H
#define __WINDOW_H

#include "src/ocean/madonna/phil/cluster.h"
#include "src/ocean/madonna/phil/divTable.h"

// Contains all information about current window.
//
class Window : public Cluster
{
public:

  Window(Cluster& c, int w, int h): Cluster(c), width(w), high(h) {};
 ~Window() {};

  virtual classType myNo() const { return WindowClass; }
  virtual char*   myName() const { return (char*)"Window"; }

  int  width; // width in clusters
  int  high;  // high  in clusters

  void getDiv(char*, int*, int*, int*, int*);
};

#endif
