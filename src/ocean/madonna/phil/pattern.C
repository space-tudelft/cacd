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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "src/ocean/madonna/phil/pattern.h"
#include "src/ocean/madonna/phil/clst.h"

Pattern::Pattern(LAYOUT* l, IMAGEDESC* i, short* m, LayMap& lMap, int dc, int im, int da)
//
// Constructor: also creates complete layout map for this cell
// after applying this matrix.
{
  matrix = new short[6];

  for (int j = 0; j < 6; j++) matrix[j] = m[j];

  doCheck = dc;
  isMac   = im;
  doTransAna = da;
  cellname = l->name;

  tranMaps = NULL;

  if (isMac)			// hurry! we don't need to do too much..
  {				// rounding up
    hor = (l->bbx[HOR] - i->overlap[HOR] + i->size[HOR] - 1) / i->size[HOR];
    ver = (l->bbx[VER] - i->overlap[VER] + i->size[VER] - 1) / i->size[VER];
  }
  else
  {
				// first lets determine if we don't have to
				// add a special offset to move rotated cell
				// to the origin (0,0)
    int coX, coY;

    findNewLeftBottom (l->bbx[HOR], l->bbx[VER], matrix, coX, coY);

    // if this difference is more than basic cell size - move it

    if (abs(coX) > i->size[HOR]) matrix[B1] -= (coX / i->size[HOR]) * i->size[HOR];
    if (abs(coY) > i->size[VER]) matrix[B2] -= (coY / i->size[VER]) * i->size[VER];

    scan (matrix, l, i, lMap);
  }
}

Pattern::~Pattern()
{
  if (tranMaps) delete tranMaps;

  delete matrix;
}

void Pattern::scan (short* mtx, LAYOUT* lPtr, IMAGEDESC* iPtr, LayMap& lMap)
//
// Performs a scan of all wire points for this layout and child
// instances and creates cluster patterns after applying transformation
// mtx - transformation matrix for parent cell. It may be different than
//       "matrix" for this pattern which is for the top level cell
{
  int hor = iPtr->size[HOR],
      ver = iPtr->size[VER], **stamp = iPtr->stamp;

  if (doTransAna && !tranMaps) // if requested and that's a first
  {				    // call create empty transparency map
    noOfLayers = iPtr->numlayers;
    tranMaps = new Transparency[noOfLayers];
    for (int i = 0; i < noOfLayers; i++)
      tranMaps[i] = Transparency (lPtr->bbx[HOR], lPtr->bbx[VER]);
    markPowerLines (iPtr);
  }

  for (SLICE* slPtr = lPtr->slice; slPtr; slPtr = slPtr->next)
    scanSlice (mtx, slPtr, iPtr, lMap);

  for (WIRE* wPtr = lPtr->wire; wPtr; wPtr = wPtr->next)
  {
    short out[4];                   // wire after transformation
    layerType layer;

    mtxapplytocrd (out, wPtr->crd, mtx);
    if (out[XL] > out[XR])           // after transformation relation between
    {                                // points may change
      int tmp = out[XL];
      out[XL] = out[XR];
      out[XR] = tmp;
    }
    if (out[YB] > out[YT])
    {
      int tmp = out[YB];
      out[YB] = out[YT];
      out[YT] = tmp;
    }

    layer = recognizeLayer (wPtr->layer);

    for (int x = out[XL]; x <= out[XR]; x++)
    for (int y = out[YB]; y <= out[YT]; y++)
    {
	Clst& newClst = *new Clst(x, y, hor, ver, stamp, lMap, layer, matrix, doCheck);
	                   // Cluster with critical points list
	                   // which come from metal segments on mirror
	                   // axis if doCheck == 1

	if (doTransAna && wPtr->layer < 100) { // that's not a via
	  Transparency& tm = tranMaps[wPtr->layer-1];

	  if (y < tm.sizeVer && x < tm.sizeHor) {
	    tm.verGridMap[y] = 1;
	    tm.horGridMap[x] = 1;
	  }
	  else
	    cerr << "Warning: is you bounding box right? (cell " << cellname << " )" << endl;
	}

	Clst& oldClst = (Clst&)findItem (newClst);

	// to clusters are equal if they have the same bottom left coord.

	if (oldClst == NOITEM)  // first point in this cluster
	  add (newClst);
	else {                  // we must only mark new sector
	  oldClst |= newClst;
	  delete &newClst;
	}
    }
  }
}

void Pattern::scanSlice (short* mtx, SLICE* slPtr, IMAGEDESC* iPtr, LayMap& lMap)
//
// Searching slicing structures.
// mtx - transformation matrix for parent cell.
{
  if (!this) cerr << "Help\n" << flush;

  if (slPtr->chld_type == SLICE_CHLD)
  {
    for (SLICE* cslPtr = slPtr->chld.slice; cslPtr; cslPtr = cslPtr->next)
      scanSlice (mtx, cslPtr, iPtr, lMap);
  }
  else
  {
    for (LAYINST* liPtr = slPtr->chld.layinst; liPtr; liPtr = liPtr->next)
      if (!strstr (liPtr->layout->name, "Tmp_Cell_")) // don't consider nelsis's tmp cells
      {
	short out[6], *mtxb = liPtr->mtx;

	// transformation for this cell is a multiplication of
	// transformation of parent and transformation of child within parent
	//
	out[A11] = mtx[A11] * mtxb[A11] + mtx[A12] * mtxb[A21];
	out[A12] = mtx[A11] * mtxb[A12] + mtx[A12] * mtxb[A22];
	out[A21] = mtx[A21] * mtxb[A11] + mtx[A22] * mtxb[A21];
	out[A22] = mtx[A21] * mtxb[A12] + mtx[A22] * mtxb[A22];
	out[B1]  = mtx[A11] * mtxb[B1]  + mtx[A12] * mtxb[B2] + mtx[B1];
	out[B2]  = mtx[A21] * mtxb[B1]  + mtx[A22] * mtxb[B2] + mtx[B2];

	scan (out, liPtr->layout, iPtr, lMap);
      }
      else {
	cerr << "\n Layouts cannot be build of nelsis's temporary cells\n";
	usrErr ((char*)"Pattern::scanSlice", EINPDAT);
      }
  }
}

layerType recognizeLayer (short layer)
{
  switch (layer) {
    case 100: return ViaLayer;
    case   1: return Metal1Layer;
    case   2: return Metal2Layer;
    case 101: return MetalsViaLayer;
    default : return Metal2Layer;
  }
}

void Pattern::addCriticals (LayMap& lMap)
//
// Adds critical points to each cluster.
{
  int x1 = MAXINT, x2 = MININT;
  int y1 = MAXINT, y2 = MININT;

  for (ListIterator cIter(*this); (Item&)cIter != NOITEM; cIter++)
  {
    Clst& clstRef = (Clst&)(Item&)cIter;

    if (clstRef.cX < x1) x1 = clstRef.cX;
    if (clstRef.cX > x2) x2 = clstRef.cX;
    if (clstRef.cY < y1) y1 = clstRef.cY;
    if (clstRef.cY > y2) y2 = clstRef.cY;
  }
  // now x1,x2,y1,y2 contain cluster coordinates of bounding box

  int hor = lMap.getHor();
  int ver = lMap.getVer();

  for (int i = x1; i <= x2; i++)
    for (int j = y1; j <= y2; j++)
    {
      Clst& newClst = *new Clst(i, j, hor, ver);
                                 // adding critical points from mirror axis

      Clst& oldClst = (Clst&)findItem (newClst);

      // to clusters are equal if they have the same bottom left coord.

      if (oldClst == NOITEM)  // first point in this cluster
      {
	lMap.addCriticalPoints (newClst, matrix);
	add (newClst);
      }
      else // we must only mark new sector and add critical points
      {
	lMap.addCriticalPoints (oldClst, matrix);
	delete &newClst;
      }
    }
}

void Pattern::removeUnnecessary (ImageMap& iMap)
//
// This routine removes unnecessary critical points which are laying on
// metal layer on mirror axis between two sectors of the same cell.
{
  for (ListIterator clstIter(*this); (Item&)clstIter != NOITEM; clstIter++)
  {
    Clst& clstRef = (Clst&)(Item&)clstIter;
    for (ListIterator criIter(clstRef.criticalPoints); (Item&)criIter != NOITEM; criIter++)
    {
      CriPoint& test = (CriPoint&)(Item&)criIter;
      clusterMapType nb = iMap.recognizeNeighbors (test.x, test.y);

      if (nb && (nb & clstRef.pattern) == nb) clstRef.criticalPoints.remove (test);
    }
  }
}

void Pattern::markPowerLines (IMAGEDESC* iPtr)
//
// This routine marks power lines as described in iPtr into tranMaps.
{
  for (POWERLINE* pPtr = iPtr->powerlines; pPtr; pPtr = pPtr->next)
  {
    Transparency& tRef = tranMaps[pPtr->layer];

    if (iPtr->routeorient[pPtr->layer] == HOR)
    {
      for (int i = 0; i < tRef.sizeHor; i++)
	tRef.horGridMap[i] = 1;
      for (int j = pPtr->row_or_column; j < tRef.sizeVer; j += iPtr->size[VER])
	tRef.verGridMap[j] = 1;
    }
    else
    {
      for (int i = 0; i < tRef.sizeVer; i++)
	tRef.verGridMap[i] = 1;
      for (int j = pPtr->row_or_column; j < tRef.sizeHor; j += iPtr->size[HOR])
	tRef.horGridMap[j] = 1;
    }
  }
}

void Pattern::findNewLeftBottom (int x, int y, short* mx, int& cx, int& cy)
//
// This routine tries to find out what will be the new left bottom corner
// of a cell with (x,y) dimensions after applying the matrix. The result
// is returned in (cx,cx);
{
  short in[4], out[4];

  in[XL] = in[YB] = 0;		// left bottom corner
  in[XR] = x;	                // right-top corner
  in[YT] = y;

  mtxapplytocrd (out, in, mx);

  // lets find new left-bottom corner
  cx = (out[XL] < out[XR])? out[XL] : out[XR];
  cy = (out[YB] < out[YT])? out[YB] : out[YT];
}

Transparency::Transparency (int sh, int sv):sizeHor(sh), sizeVer(sv)
{
  verGridMap = new transGridType [sv];
  horGridMap = new transGridType [sh];
  memset (horGridMap, 0, sizeHor * sizeof(char));
  memset (verGridMap, 0, sizeVer * sizeof(char));
}

Transparency& Transparency::operator =(const Transparency& other)
{
  sizeVer = other.sizeVer;
  sizeHor = other.sizeHor;
  verGridMap = new transGridType [sizeVer];
  horGridMap = new transGridType [sizeHor];
  memcpy (horGridMap, other.horGridMap, sizeHor * sizeof(char));
  memcpy (verGridMap, other.verGridMap, sizeVer * sizeof(char));
  return *this;
}

Transparency::~Transparency()
{
  delete verGridMap;
  delete horGridMap;
}

Transparency* Transparency::transform (short* mtx)
//
// It creates a copy of itself but after applying matrix operation
// specified in mtx.
// First we have to recognize kind of necessary transformations
// For real examples only 4 kinds of mirror axes may exist
//
// axis           matrix         descr.       verGridMap horGridMap
// ----------------------------------------------------------------
// no transform.  [ 1, 0, 0, 1]  nothing       nothing   nothing
// horizontal     [ 1, 0, 0,-1]  mx            reverse   nothing
// vertical       [-1, 0, 0, 1]  my            nothing   reverse
// y = x          [ 0, 1, 0, 1]  mx & rotate   hor.      ver.
// y = -x + b     [ 0,-1,-1, 0]  my & rotate   rev.hor.  rev.ver.
{
  Boolean reverseHor = false, reverseVer = false, doSwitch = false;

  if (mtx[A11] == 0) doSwitch = true;
  if (mtx[A11] == -1 || mtx[A12] == -1) reverseHor = true;
  if (mtx[A12] ==  1 || mtx[A22] == -1) reverseVer = true;

  // so now when we know what we should do let\'s do it

  Transparency* output;
  transGridType* srcPtr;

  if (doSwitch)
    output = new Transparency (sizeVer, sizeHor);
  else
    output = new Transparency (sizeHor, sizeVer);

  // first do the horGridMap

  srcPtr = doSwitch ? verGridMap : horGridMap;

  for (int i = 0; i < output->sizeHor; i++) {
    if (reverseHor)
      output->horGridMap[i] = srcPtr[output->sizeHor - i - 1];
    else
      output->horGridMap[i] = srcPtr[i];
  }

  // and the same for vertical map

  srcPtr = doSwitch ? horGridMap : verGridMap;

  for (int i = 0; i < output->sizeVer; i++) {
    if (reverseVer)
      output->verGridMap[i] = srcPtr[output->sizeVer - i - 1];
    else
      output->verGridMap[i] = srcPtr[i];
  }

  return output;
}

int Transparency::freeTracks (int dir)
//
// Calculates number of free tracks in direction "dir".
{
  transGridType* ptr;
  transGridType* end;

  if (dir == VER) {
    ptr = horGridMap; end = ptr + sizeHor;
  } else {
    ptr = verGridMap; end = ptr + sizeVer;
  }

  int sum = 0;
  for (; ptr < end; ptr++) sum += !(*ptr);

  return sum;
}
