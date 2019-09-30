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

#include "src/ocean/madonna/phil/cricand.h"
#include "src/ocean/madonna/phil/imageMap.h"
#include <malloc.h>

ImageMap::ImageMap (int x, int y, int **stamp): Array(5,0,5), criLines(2,0,2)
//
// Contructor: should call a parser of image.seadif file.
{
  rows = y;
  cols = x;
  tmpStamp = stamp;

  map = (layoutMapType**)allocArray2 (cols, rows, sizeof(layoutMapType));
}

ImageMap::~ImageMap()
{
  freeArray2 (cols, (void**)map);
}

int ImageMap::addFeeds (List& feeds, layerType lay)
//
// Function called by yacc++ parser. It processes list of just read feeds which
// may be either restricted or universal. There can be also the so called
// external feed which when appears means that this feed has an extension
// to the neighbor cluster in this direction. As a flag works field net
// when 0 - normal feed
//      1 - horizontal extension
//      2 - vertical extension
{
  static unsigned int actFeedFlag = 1; // this variable is used to keep a
                                       // a unique identifier of a feed
                                       // which is used to recognize grid
                                       // points of the same feed marked in
                                       // actual function call.
  unsigned char uniCount = 0;
  Boolean horExt = false,
          verExt = false;
  List *lPtr = 0;

  if (lay == DiffusionLayer || lay == Metal1Layer)
    criticalIsland (feeds, lay); // these functions add criLines elements too
  else {
    lPtr = new List();
    uniCount = getItemsInBox();
    criticalFeed (feeds);
  }
                                    // we have to check also if it is
                                    // an external feed (has an
                                    // extension to neighbor cluster
  for (ListIterator cIter(feeds); (Item&)cIter != NOITEM; cIter++)
  {
    CriPoint &test = (CriPoint&)(Item&)cIter;

    if (test.net & 1) horExt = true; // means horizontal extension
    if (test.net & 2) verExt = true; // means vertical extension
  }

  for (ListIterator lIter(feeds); (Item&)lIter != NOITEM; lIter++)
  {
    CriPoint &feed = (CriPoint&)(Item&)lIter;

  if (feed.net == 0)     // External feed flag - we have to skip it
  {
    int x = feed.x, y = feed.y;

    if (x > cols || y > rows) {
      warning ((char*)"ImageMap::addFeeds", EINPDAT);
      return 1;
    }
    map[x][y].layers |= lay;
    map[x][y].termNo = actFeedFlag;  // we use that field for feed
                                             // identifier
    if (lay == PolyLayer) {
      map[x][y].uniNo = uniCount;
      lPtr -> add (*new Point (x, y));
    }
    else // we have to mark some grids in neighborhood
    {
      if (y < rows-1 && map[x][y+1].termNo == actFeedFlag) {
	map[x][y].upWay     |= lay;
	map[x][y+1].downWay |= lay;
      }
      if (y > 0 && map[x][y-1].termNo == actFeedFlag) {
	map[x][y].downWay |= lay;
	map[x][y-1].upWay |= lay;
      }
      if (y == rows-1 && verExt) {
	map[x][y].upWay   |= lay;
	map[x][0].downWay |= lay;
      }
      if (x < cols-1 && map[x+1][y].termNo == actFeedFlag) {
	map[x][y].rightWay  |= lay;
	map[x+1][y].leftWay |= lay;
      }
      if (x > 0 && map[x-1][y].termNo == actFeedFlag) {
	map[x][y].leftWay    |= lay;
	map[x-1][y].rightWay |= lay;
      }
      if (x == cols-1 && horExt) {
	map[x][y].rightWay |= lay;
	map[0][y].leftWay  |= lay;
      }
    }
  }
  }

  if (lay == PolyLayer) add (*lPtr);

  actFeedFlag++;
  return 0;
}

void ImageMap::criticalIsland (List& feeds, layerType layer)
//
// This function returns 0 if this feed doesn't lay under one of mirror
// axis, otherwise number assigned to this critical area and adds all
// points laying under mirror line to "criLines"
{
  List *newList = new List();
  Boolean added = false;

  for (ListIterator lIter(feeds); (Item&)lIter != NOITEM; lIter++)
  {
    CriPoint &prev = (CriPoint&)(Item&)lIter;
    CriPoint p(prev);

    if (p.net > 0) continue; // skip external feed flags

    if (tmpStamp[p.x << 1][p.y << 1] > 0)     // mirror axis
    {
      added = true;
                                         // now recognizing neighbors
      clusterMapType nb = recognizeNeighbors (p.x, p.y);

      newList -> add (*new CriCandidate (p.x, p.y, layer, 0, nb));
    }
  }

  if (added) {
    criLines.add (*newList);
  }
  else {
    delete newList;
  }
}

void ImageMap::criticalFeed (List& feeds)
//
// Does the same job as criticalIsland but for universal feeds.
// This feed is critical if at least one of the points belongs to
// another sector than others or lies on mirror axis.
// Returns number of critical line or 0 if this feed is not critical.
{
  static Boolean warningFlag = false;
  int secNo = -1;

  for (ListIterator lIter(feeds); (Item&)lIter != NOITEM; lIter++)
  {
    CriPoint &prev = (CriPoint&)(Item&)lIter;
    CriPoint p(prev);

    if (p.net > 0)
    {
      if (!warningFlag) {
	cerr << "\nWarning : Universal feeds extending to neighbor basic cells";
	cerr << "\n won't be checked for existance of net conflicts !!!\n";
	warningFlag = true;
      }
    }
    else
    {
      int thisSec = -tmpStamp[p.x << 1][p.y << 1];

      if (tmpStamp[p.x << 1][p.y << 1] > 0 || (secNo != -1 && thisSec != secNo))
      {
	List *newList = new List();
	// first we have to build a temporary
	// pattern needed during critical points recognition

	clusterMapType pat = 0;
	for (lIter.restart(); (Item&)lIter != NOITEM; lIter++)
	{
	  p = (CriPoint&)(Item&)lIter;
	  clusterMapType cm;
	  if ((cm = tmpStamp[p.x << 1][p.y << 1]) > 0)
	    pat |= recognizeNeighbors (p.x, p.y);
	  else
	    pat |= 1 << (-1 - cm);
	  // marking these sectors
	}

	for (lIter.restart(); (Item&)lIter != NOITEM; lIter++)
	{
	  CriPoint &tmp = (CriPoint&)(Item&)lIter;
	  p = tmp;
	  newList -> add (*new CriCandidate (p.x, p.y, PolyLayer, 0, pat));
	}
	criLines.add (*newList);
	return;
      }
      else
	secNo = thisSec;
    }
  }
}

void ImageMap::clearFeedNo (void)
{
  for (int i = 0; i < cols; i++)
  {
    layoutMapType *lPtr = map[i];
    for (int j = 0; j < rows; j++, lPtr++) lPtr -> termNo = 0;
  }
}

clusterMapType ImageMap::recognizeNeighbors (int x, int y)
//
// return variable of type clusterMapType with set neighbor sectors bits
{
  clusterMapType cm, nb = 0;
  int m1 = x << 1, m2 = y << 1;
  int maxX = 2*cols - 1,
      maxY = 2*rows - 1;

  for (int i = m1-1; i <= m1+1; i++)
    for (int j = m2-1; j <= m2+1; j++)
    {
      if (i >= 0 && i < maxX && j >= 0 && j < maxY &&
	  i != m1 && j != m2 && (cm = tmpStamp[i][j]) < 0)
	// means: there is a sector
	nb |= 1 << (-1 - cm);
    }

  cm = nb;
  int highCount = 0;
  for (unsigned int a = 0; a < sizeof(clusterMapType)*8; a++, cm >>= 1)
    if (cm & 1)
      if (++highCount >= 2) return nb;

  return 0; // means: leave this point
}
