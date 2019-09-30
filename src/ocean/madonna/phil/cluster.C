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

#include "src/ocean/madonna/phil/cluster.h"

Cluster::Cluster(int gx, int gy, int hor, int ver, int **stamp)
//
// Constructor : creates cluster with one marked sector
//	gx, gy   - point coordinates
//	hor, ver - cluster dimension
//	stamp    - image pattern
{
  cX = gx / hor;
  cY = gy / ver;

  if (gx < 0) cX--;
  if (gy < 0) cY--;

  x = gx % hor;
  y = gy % ver;

  if (x < 0) x += hor;
  if (y < 0) y += ver;

  if ((gx = stamp[x<<1][y<<1]) < 0)
    pattern = 1 << (-1-gx);
  else
    pattern = 0;

  x = cX * hor;
  y = cY * ver;
}
