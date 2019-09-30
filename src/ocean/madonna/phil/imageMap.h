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
/*
 * Contains: ImageMap
 */

#ifndef __IMAGEMAP_H
#define __IMAGEMAP_H

#include "src/ocean/madonna/phil/phil.h"
#include "src/ocean/madonna/phil/cluster.h"
#include "src/ocean/madonna/lia/list.h"
#include "src/ocean/madonna/lia/array.h"

class LayMap;

// Contains basic image information. It has the size of cluster.
// Each grid point is represented as one array cell.
// Base array contains universal feeds. One universal feed is a list of points.
// Array criLines contains lists of critical points CriCandidate laying
// under one of the mirror axises or universal feeds belonging to
// different sectors.
// Array map contains image information for each grid point:
//	layers - bits are set if coresponding layer is present in basic image
//	visited - not used (set to 0)
//	termNo  - not used (set to 0)
//	criAreaNo - if this grid belongs to critical area then here
//		there's it's number (equal position in criLines +1)
//		otherwise set to 0
//	uniNo - number of restricted feed (position in base array)
//		if exists in this point
//	xxxWay - these fields are filled if around there are other
//		 points connected with actual point.

class ImageMap : public Array
{
public:
  ImageMap(int, int, int** stamp);
 ~ImageMap();

  virtual classType myNo() const { return ImageMapClass; }
  virtual char*   myName() const { return (char*)"ImageMap"; }

  friend  class LayMap;

  int             addFeeds (List&, layerType);
  void            criticalIsland (List&, layerType);
  void            criticalFeed (List&);
  void            clearFeedNo (void);
  clusterMapType  recognizeNeighbors (int, int);

private:
  layoutMapType** map;
  Array           criLines;
  int**           tmpStamp;

  int rows, cols;
};

#endif
