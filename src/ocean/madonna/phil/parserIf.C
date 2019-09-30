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
 * ParserInterface - This class serves as an interface to "image.seadif"
 * parser. It reads informations about image from file and places them
 * in two data structures :
 *     thisImage - used by partitioner and in detailed placement
 *     imageMap  - used in detailed placement only (conflicts checking)
 */

#include "src/ocean/madonna/phil/parserIf.h"
#include <stdio.h>

extern int pplineno;

// I have to use this instead of <yacc.h> 'cos I've changed all names

extern "C" {
   extern void pperror(const char *);
   extern int  pplex();
}

// the yacc++ generated program declares this to have C++ linkage:
extern int  ppparse();

ParserInterface::ParserInterface()
//
// Creates empty object.
{
  imageMap = NULL;
  thisImage.overlap[HOR] = 0;
  thisImage.overlap[VER] = 0;
  thisImage.numlayers   = 0;
  thisImage.routeorient = NULL;
  thisImage.estransp    = NULL;
  thisImage.powerlines  = NULL;
}

ParserInterface::~ParserInterface()
{
  if (imageMap) delete imageMap;
  if (thisImage.routeorient) delete thisImage.routeorient;
  if (thisImage.estransp   ) delete thisImage.estransp;
  // I do not free powerlines because i am too lazy for that ... (leak !!!)
}

void ParserInterface::read()
//
// Reads data from "image.seadif" and fills in internal structures.
{
  thisImage.size[HOR] = -1;        // this for checking during parsing
                                   // if it's everything read
  thisImage.mirroraxis = NULL;
  thisImage.numaxis = 0;
  STRING  imagefilename;

  // Yacc parser reads:
  //           size of image
  //           mirror axis'es (after that calls initimagedesc
  //              and creates ImageMap
  //           all feeds (CridConnectList)

  if (!(imagefilename = sdfimagefn())) usrErr ((char*)"Plcm::Plcm", EINPDAT);

  if (!freopen (imagefilename, "r", stdin)) {
    cerr << "Cannot open image description file \"" << imagefilename << "\"... " << endl;
    usrErr ((char*)"Plcm::Plcm", EINPDAT);
  }

  cout << "\nReading image description file \"" << imagefilename << "\"...\n\n" << endl;
  pplineno = 1;
  if (ppparse()) usrErr ((char*)"ParserInterface::ParserInterface", EINPDAT);

  imageMap->clearFeedNo();         // clear field termNo used temporary
                                   // during creating image
}

void ParserInterface::setDim (int x, int y)
//
// Called by parser during ImageMap constructor - sets image size
{
  thisImage.size[HOR] = x;
  thisImage.size[VER] = y;
}

void ParserInterface::addMirrorAxis (MIRROR *m)
//
// Called by parser during ImageMap constructor - attaches new mirror axis
{
   m->next = thisImage.mirroraxis;
   thisImage.mirroraxis = m;
   thisImage.numaxis++;
}

void ParserInterface::initImages ()
//
// Called by parser. Creates IMAGEDESC info's and empty map inside ImageMap
{
  initimagedesc(&thisImage);

  removeDuplicates ();

  if (!(imageMap = new ImageMap(thisImage.size[HOR], thisImage.size[VER], thisImage.stamp)))
    usrErr ((char*)"ParserInterface::ParserInterface", ENOTMEM);
}

void ParserInterface::addImageOverlap(int x, int y)
//
// called by parser
{
   thisImage.overlap[HOR] = x;
   thisImage.overlap[VER] = y;
}

int ParserInterface::processFeeds (List& f, layerType l)
//
// called by parser
{
  if (thisImage.size[HOR] == -1)  // input data error : no Size section
    usrErr ((char*)"ParserInterface::ParserInterface", EINPDAT);
  return imageMap->addFeeds (f, l);
}

void ParserInterface::removeDuplicates ()
//
// Removes duplicate transformations.
{
  int sizX = thisImage.size[HOR] * 2;
  int sizY = thisImage.size[VER] * 2;
  MIRROR *prevPtr = NULL;

  for (MIRROR *mPtr = thisImage.mirroraxis; mPtr; )
  {
    int *axis = mPtr->axis;

    if ((axis[X1] == axis[X2] && (axis[X1] == 0 || axis[X2] == sizX)) ||
        (axis[Y1] == axis[Y2] && (axis[Y1] == 0 || axis[Y2] == sizY)))
    {
      // we have also to check if it isn't the only one
      short *mtx = mPtr->mtx;
      int cnt = 0;

      for (MIRROR *tPtr = thisImage.mirroraxis; tPtr; tPtr = tPtr->next)
      {
	if (mtx[A11] == tPtr->mtx[A11] && mtx[A12] == tPtr->mtx[A12] &&
            mtx[A21] == tPtr->mtx[A21] && mtx[A22] == tPtr->mtx[A22]) cnt++;
      }
      if (cnt > 1)
      {
	if (prevPtr)
	{
	  prevPtr->next = mPtr->next;
	  FreeMirror (mPtr);
	  mPtr = prevPtr->next;
	}
	else
	{
	  prevPtr = mPtr->next;
	  FreeMirror (mPtr);
	  thisImage.mirroraxis = mPtr = prevPtr;
	  prevPtr = NULL;
	}
      }
    }
    else
    {
      prevPtr = mPtr;
      mPtr = mPtr->next;
    }
  }
}

void ParserInterface::setNumLayers (int numlay)
{
   thisImage.numlayers = numlay;
   // allocate array for wire orientations and estimated transparencies:
   thisImage.routeorient = new int [numlay];
   thisImage.estransp    = new int [numlay];
}

void ParserInterface::setRouteOrient(int layer, int orient)
{
   if (layer < 0 || layer >= thisImage.numlayers)
      usrErr ((char*)"ParserInterface::setRouteOrient", EINPDAT);
   if (orient != HOR && orient != VER)
      usrErr ((char*)"ParserInterface::setRouteOrient", EINPDAT);
   thisImage.routeorient[layer] = orient;
}

void ParserInterface::setTransparency(int layer, int transp)
{
   if (layer < 0 || layer >= thisImage.numlayers)
      usrErr ((char*)"ParserInterface::setRouteOrient", EINPDAT);
   if (transp < 0)
      usrErr ((char*)"ParserInterface::setRouteOrient", EINPDAT);
   thisImage.estransp[layer] = transp;
}

void ParserInterface::addPowerLine(int orient, char *name, int layer, int r_or_c)
{
   POWERLINEPTR pl = new POWERLINE;
   pl->orient = orient;
   pl->row_or_column = r_or_c;
   pl->name  = cs(name);
   pl->layer = layer;
   pl->next  = thisImage.powerlines;
   thisImage.powerlines = pl;
}
