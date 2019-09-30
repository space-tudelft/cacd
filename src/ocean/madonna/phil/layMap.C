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

#include <iostream>
using namespace std;
#include "src/ocean/madonna/phil/cricand.h"
#include "src/ocean/madonna/phil/layMap.h"
#include "src/ocean/madonna/phil/clst.h"
#include <string.h>

LayMap::LayMap(LAYOUT* l, ImageMap* i, Array* n): layout(l), imageMap(i), freeNets(n)
//
// Constructor
{
  initFlag = false;
  actualMatrix = NULL; // to ensure that no wires in "map"
  map = NULL;
}

LayMap::~LayMap() // Destructor
{
  if (initFlag) freeArray2 (xMax - xMin + 2, (void**)map);
}

/*
layerType LayMap::viaToCriticalArea (int x, int y)
//
// Returns layer if there's a critical area under this point or value
// NoneLayer otherwise. The coordinate should be inside cluster.
{
  layoutMapType *grid = &(imageMap->map[x][y]);

  if (grid->criAreaNo) return layerType(grid->layers & (DiffusionLayer | PolyLayer));
			// critical area may be only of Diffusion or Poly layertype
			// so via between metals does not mean any danger to us.
  return NoneLayer;
}
*/

unsigned int LayMap::findNetId (int x, int y, layerType layer, short *mtx)
//
// Performs search for external terminal point connected with this point.
// If it is a first call creates layout map, otherwise only refreshes old one.
// Returns terminal id or 1 (what means constrained critical net) if
// this point belongs to one of the internal nets.
// All this stuff below here is because i wanted to avoid useless operations.
// For example it can happen that there's no critical area at all and then
// to all all that job is unnecessary.
// Very important is here that mtx pointer should really point to another
// matrix when we want to start considering another transposition of our layout.
{
  if (!initFlag)
  {
    create ();
    recognizeTerminals ();
    addWires (0, 0, layout, mtx);
    addTerms (mtx);
    initFlag = true;
    actualMatrix = mtx;
  }
  else
  {
    if (actualMatrix == mtx) clearFlags ();
    else
    {
      freeArray2 (xMax-xMin+2, (void**)map);
      actualMatrix = mtx;
      create ();
      addWires (0, 0, layout, mtx);
      addTerms (mtx);
    }
  }

  if (x >= xMin && x <= xMax && y >= yMin && y <= yMax) return search (x, y, layer);
  return 0;
}

void LayMap::addCriticalPoints (Clst &c, short *mtx)
//
// adds points belonging to critical area and laying under mirror axis
// if it's really critical
{
  int ver = imageMap->rows,
      hor = imageMap->cols;
  List& list = c.criticalPoints;
  int x = c.cX, y = c.cY;

  ArrayIterator aIter(imageMap->criLines);

  for (; (Item&)aIter != NOITEM; aIter++)
  {
    List &criLine = (List&)(Item&)aIter;

    ListIterator criIter(criLine);

    for (; (Item&)criIter != NOITEM; criIter++)
    {
      CriCandidate &cp = (CriCandidate&)(Item&)criIter;

      if (cp.neighbors == 0 || (cp.neighbors & c.pattern) != cp.neighbors)
      {
	// we don't have to check points laying
	// on mirror axis between two sectors
	// taken by this cell or belonging
	// to critical universal feeds laying
	// inside this cell
	// cp.neighbors in this place contains set of
	// bits on positions of sectors on
	// both sides of axis for restricted feed
	// or sectors to which belong critical
	// universal feed

	int net = findNetId(x*hor+cp.x, y*ver+cp.y, layerType(cp.layer), mtx);

	if (net != 0)                    // this point is connected
                                         // to one of terminals
	{
	  CriPoint &newCp = * new CriPoint(cp.x, cp.y, layerType(cp.layer), net);
	  if (!list.hasItem (newCp))
	    list.add (newCp);
	  else
	    delete &newCp;
	}
      }
    }
  }
}

unsigned  int LayMap::search(int x, int y, layerType layer)
//
// Method of searching is very simple : this function is called recursively
// for all still not visited points in the vicinity of (x,y), or other points
// in universal feed or tries to change layer if there's a via. If all these
// possibilities are already checked it returns 0.
{
  unsigned int net;
  layoutMapType *m = &map[x-xMin][y-yMin];

  if (m->visited & layer) return 0; // we've already been here!

  if (m->termNo)
    switch (layer)
    {
    case DiffusionLayer:
    case PolyLayer:
                  if (!(m->layers & Metal1Layer)&& !(m->layers & Metal2Layer))
		    return m->termNo;
		  break;
    case Metal1Layer:
                  if (!(m->layers & Metal2Layer))
		    return m->termNo;
		  break;
    case Metal2Layer:
		  return m->termNo;
    default: // NoneLayer, MetalsViaLayer, ViaLayer
		  break;
    }

  m->visited |= layer;               // mark that place

  // visiting points belonging to the same layer :

  if (layer != PolyLayer)             // restricted feeds && metal segments
  {
                                     // we have only to check this neighbors
                                     // which have connection with our on
                                     // the same layer
    if (m->leftWay & layer && x > xMin)
      if ((net = search (x-1, y, layer))) return net;
    if (m->upWay & layer && y < yMax)
      if ((net = search (x, y+1, layer))) return net;
    if (m->rightWay & layer && x < xMax)
      if ((net = search (x+1, y, layer))) return net;
    if (m->downWay & layer && y > yMin)
      if ((net = search (x, y-1, layer))) return net;
  }
  else	// for universal feed we have to
	// go to visit all of the points from the list
  {
    ListIterator rIter((List&)(*imageMap)[m->uniNo]);
    int a = x / imageMap->cols,
        b = y / imageMap->rows;

    if (x < 0) a--;
    if (y < 0) b--;

    a *= imageMap->cols;
    b *= imageMap->rows;
                          // now we have coordinates of left-bottom point
                          // in cluster

    for (; (Item&)rIter != NOITEM; rIter++)
    {
      Point &p = (Point&)(Item&)rIter;
      if ((x != a+p.x || y != b+p.y) && (net = search(a+p.x, b+p.y, layer)) > 0) return net;
    }
  }

  // if all points on this layer visited lets try go to the other

  if ((m->layers & ViaLayer)) { // normal via exists
    if ((layer & Metal1Layer) || (layer & Metal2Layer))
    {
      if ((net = search(x, y, (m->layers & PolyLayer)>0 ? PolyLayer : DiffusionLayer)))
	return net;
    }
    else
    {
      if ((net = search(x, y, (m->layers & Metal1Layer) ? Metal1Layer : Metal2Layer)))
	return net;
    }
  }

  if ((m->layers & MetalsViaLayer) > 0) { // via between two metals
    if ((layer & Metal1Layer) > 0) {
      if ((net = search(x, y, Metal2Layer))) return net;
    }
    else {
      if ((net = search(x, y, Metal1Layer))) return net;
    }
  }

  // we cannot go further - must return

  return 0;
}

void LayMap::create ()
//
// Creates true layout map for layout pointed by "layout"
// It's implemented as an array of side 3x bonding box of layout.
// Each grid point is modeled as record of layoutMapType type.
{
  int bHigh = layout->bbx[VER],
      bWidth = layout->bbx[HOR],
      iHigh = imageMap->rows,
      iWidth = imageMap->cols;

  if (bHigh  < iHigh)  bHigh  = iHigh;
  if (bWidth < iWidth) bWidth = iWidth;

  xMin = -bWidth;
  xMax = 2*bWidth - 1;
  yMin = -bHigh;
  yMax = 2*bHigh - 1;

  map = (layoutMapType**)allocArray2 (xMax-xMin+2, yMax-yMin+2, sizeof(layoutMapType));

  // now we have to fill it out

  layoutMapType *sPtr, *dPtr;

  for (int i = xMin; i <= xMax; i++)  // for all columns
  {
    int x = i % iWidth;
    int y = yMin % iHigh;

    if (y < 0) y += iHigh;
    if (x < 0) x += iWidth; // x,y - coordinates inside cluster

    dPtr = &map[i+bWidth][0];     // destination column pointer

    sPtr = &imageMap->map[x][y];

    int j = yMin;             // i,j - coordinates inside new array
    int numToCopy = iHigh - y;

    while (j <= yMax)
    {
      memcpy (dPtr, sPtr, numToCopy* sizeof(layoutMapType));

      sPtr = &imageMap->map[x][0];
      dPtr += numToCopy;
      j += numToCopy;

      if (yMax - j > iHigh)  // if last time
	numToCopy = iHigh;
      else
	numToCopy = yMax+1-j;
    }
  }
}

void LayMap::addWires (int xoffs, int yoffs, LAYOUT *lPtr, short *mtx)
//
// Adds to image metal layers in places where they will be after applying matrix mtx.
{
  for (SLICE *slPtr = lPtr->slice; slPtr; slPtr = slPtr->next)
    goDeeper (slPtr, lPtr, mtx);

  for (WIRE *wPtr = lPtr->wire; wPtr; wPtr = wPtr->next)
  {
    short out[4];                   // wire after transformation
    char buf[1024];
    layerType layer;
    int  xBegin, xEnd, yBegin, yEnd;

    mtxapplytocrd (out, wPtr->crd, mtx);

    layer = recognizeLayer (wPtr->layer);

    if (out[XL] <= out[XR])           // after transformation relation between
    {                                 // points may change
      xBegin = out[XL]-xMin;
      xEnd = out[XR]-xMin;
    }
    else
    {
      xBegin = out[XR]-xMin;
      xEnd = out[XL]-xMin;
    }
    if (out[YB] <= out[YT])
    {
      yBegin = out[YB]-yMin;
      yEnd = out[YT]-yMin;
    }
    else
    {
      yBegin = out[YT]-yMin;
      yEnd = out[YB]-yMin;
    }
				// layout instances may be shifted
    yBegin += yoffs;
    yEnd   += yoffs;
    xBegin += xoffs;
    xEnd   += xoffs;

    if (xBegin < 0 || yBegin < 0 || yEnd > yMax-yMin || xEnd > xMax-xMin)
    {
       // cerr << "\n  Wrong wire segment - coordinates out of range !\n";
       // users really tend to get upset if you do not tell \'m WHERE the cause
       // of their problem is. So why not print the NAME OF THE LAYOUT that we
       // are complaining about?
       snprintf (buf, 1024, "\n Layout %s(%s(%s(%s))), Wire [%d %d %d %d %d] "
		    "coordinates out of range\n",
		    lPtr->name, lPtr->circuit->name, lPtr->circuit->function->name,
		    lPtr->circuit->function->library->name, wPtr->layer,
		    wPtr->crd[XL], wPtr->crd[XR], wPtr->crd[YB], wPtr->crd[YT]);
       cerr << buf;
       usrErr ((char*)"LayMap::addWires", EINPDAT);
    }

    for (int x = xBegin; x <= xEnd; x++)
    for (int y = yBegin; y <= yEnd; y++)
      {
	layoutMapType *m = &map[x][y];
	m->layers |= layer;

	// mark connections between grid points
	if (y < yEnd)   m->upWay    |= layer;
	if (y > yBegin) m->downWay  |= layer;
	if (x < xEnd)   m->rightWay |= layer;
	if (x > xBegin) m->leftWay  |= layer;
      }
  }
}

void LayMap::goDeeper (SLICE *slice, LAYOUT *lPtr, short *matrix)
{
  if (slice->chld_type == SLICE_CHLD)
  {
    for (SLICE* cslPtr = slice->chld.slice; cslPtr; cslPtr = cslPtr->next)
      goDeeper (cslPtr, lPtr, matrix);
  }
  else
  {
    for (LAYINST *liPtr = slice->chld.layinst; liPtr; liPtr = liPtr->next)
      if (!strstr (liPtr->layout->name, "Tmp_Cell_")) // don't consider nelsis's tmp cells
      {
	short out[6], *mtxb = liPtr->mtx;

	// transformation for this cell is a multiplication of
	// transformation of parent and transformation of child within parent
	//
	out[A11] = matrix[A11] * mtxb[A11] + matrix[A12] * mtxb[A21];
	out[A12] = matrix[A11] * mtxb[A12] + matrix[A12] * mtxb[A22];
	out[A21] = matrix[A21] * mtxb[A11] + matrix[A22] * mtxb[A21];
	out[A22] = matrix[A21] * mtxb[A12] + matrix[A22] * mtxb[A22];
	out[B1]  = matrix[A11] * mtxb[B1]  + matrix[A12] * mtxb[B2] + matrix[B1];
	out[B2]  = matrix[A21] * mtxb[B1]  + matrix[A22] * mtxb[B2] + matrix[B2];

	addWires (0, 0, liPtr->layout, out);

	// addWires (liPtr->mtx[B1], liPtr->mtx[B2], liPtr->layout, matrix);
      }
      else {
	cerr << "\n Layouts cannot be build of nelsis's temporary cells\n";
	usrErr ((char*)"LayMap::goDeeper", EINPDAT);
      }
  }
}

void LayMap::refresh()
//
//  Clears metal layers, terminals and flag fields.
{
  int toClear = Metal1Layer | Metal2Layer | MetalsViaLayer | ViaLayer;
  toClear = ~toClear;

  for (int i = 0; i <= xMax-xMin; i++)
  {
    layoutMapType *lPtr = map[i];
    for (int j = 0; j <= yMax-yMin; j++, lPtr++)
    {
      lPtr->visited = 0;
      lPtr->termNo  = 0;
      lPtr->layers   &= toClear;
      lPtr->upWay    &= toClear;
      lPtr->downWay  &= toClear;
      lPtr->rightWay &= toClear;
      lPtr->leftWay  &= toClear;
    }
  }
}

void LayMap::addTerms (short* mtx)
//
// Adds terminals after applying matrix mtx.
{
  int out[2], in[2];

  for (LAYPORT* pPtr = layout->layport; pPtr; pPtr = pPtr->next)
  {
    in[HOR] = pPtr->pos[HOR];
    in[VER] = pPtr->pos[VER];

    mtxapply (out, in, mtx);
    map[out[HOR]-xMin][out[VER]-yMin].termNo = (unsigned int)pPtr->flag.l;
    // here there is previously recognized "free" net nr or 1 when this "contrained"
  }
}

void LayMap::recognizeTerminals ()
//
// Recognizes terminals of this layout : Each gets a number. When a terminal
// name is one of user defined "free" nets then number = position in array+2.
// It's because in map 0 means "no terminal" and "1" "contrained" net,
// otherwise number == 1.
{
  for (LAYPORT *pPtr = layout->layport; pPtr; pPtr = pPtr->next)
  {
    ArrayIterator aIter(*freeNets);
    int pos = 0;

    for (; (Item&)aIter != NOITEM; aIter++, pos++)
    {
      CriNet &net = (CriNet&)(Item&)aIter;
      if (pPtr->cirport->name == net.name) break;
    }
    if ((Item&)aIter == NOITEM) // no such name => "constrained" net
      pPtr->flag.l = 1;
    else
      pPtr->flag.l = pos+2;
  }
}

void LayMap::clearFlags ()
//
// Clears "visited" flags.
{
  for (int i = 0; i <= xMax-xMin; i++)
  {
    layoutMapType *lPtr = map[i];

    for (int j = 0; j <= yMax-yMin; j++, lPtr++) lPtr->visited = 0;
  }
}
