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
#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/madonna/partitioner/genpart.h"
#include "src/ocean/madonna/partitioner/part.h"
#include "src/ocean/madonna/partitioner/cost.h"
#include <math.h>
#include "src/ocean/madonna/phil/phil_glob.h"

#define BBX_MEAN_X	0
#define BBX_MEAN_Y	1
#define BBX_MAX_X	2
#define BBX_MAX_Y	3
#define BBX_N_CELL	4
#define BBX_AREA	5
#define BBX_AVAIL	6
#define BBX_ARRAY_SIZE	7

#define NOTINITIALIZED -1

enum RoundingPolicy { NoRounding, RoundUp, RoundDown, RoundNear };

static int oldfashionedwayofdoingthings (CIRCUITPTR circuit, int *nx, int *ny, CFUNC *costfunctions);
static void computeMeanBbx (CIRCUITPTR circuit, double *meanX, double *meanY);
static int otherDirection (int dir);
static double gridPointsToCoreCells (int gridpoints, int direction, RoundingPolicy rounding = NoRounding);
static int coreCellsToGridPoints (double coreCells, int direction);
static void computeBbxStatistics (CIRCUITPTR circuit, double bbxStatistics[]);
static void thinkAboutGridSize (double stats[], double extraplaza);

extern "C"
int findrectangleandcostfunctions (CIRCUITPTR circuit, int *nx, int *ny, CFUNC *costfunctions, int calldepth);

extern int makeChannels; // TRUE if we must make routing channels

#define MEAN_CELLS_PER_AREA 3.0
#define MEAN_CELLS_PER_AREA_IF_CHANNELED 10.0
#define MAXIMUM_NUMBER_OF_PLACEMENT_AREAS 512

static CFUNC thecostfunctions[2] = { (CFUNC)netstatecost, (CFUNC)netstatecost2 };

// Following imported parameters specify the requested size and shape of the
// area for the total placement. The area may be expanded only in the
// expandableDirection. In other words,
// requestedGridPoints[expandableDirection] specifies a minimum length, wheras
// requestedGridPoints[otherDirection(expandableDirection)] specifies a maximum
// length.
extern int requestedGridPoints[]; // set by main from argv
extern int expandableDirection;   // set by main from argv
extern double extraplaza;	  // set by main, either by default or from argv

// This results from requestedGridPoints[] and expandableDirection and the
// actual area occupied by the cells to be placed and the shape of these
// cells... It's all computed in findrectangleandcostfunctions()...:
static int    targetGridPoints[2];
static double targetAreas[2];

// This function takes a look at the number and shape of the cells to be
// placed, and tries to think of a reasonable partitioning scheme. The output
// of the function is the number of partitions in horizontal (nx) and vertical
// (ny) direction, as well as an array of costfunctions that can evaluate the
// cost of a netdistribution throughout such a partitioning space.
int findrectangleandcostfunctions (CIRCUITPTR circuit, int *nx, int *ny,
				  CFUNC *costfunctions, int calldepth)
{
   // return oldfashionedwayofdoingthings (circuit, nx, ny, costfunctions);

   // currently we do not support recursive partitioning...:
   if (calldepth > 1) return 0;

   double stats[BBX_ARRAY_SIZE];
   computeBbxStatistics (circuit, stats);

   thinkAboutGridSize (stats, extraplaza); // maybe modify requestedGridPoints[] ...

   if (stats[BBX_N_CELL] <= 1) return 0; // no cells...

   // if we also make routing channels, than this is probably a standard-cell
   // like placement. Because flipflops are about 8 times as big as inverters,
   // we want more cells_per_area if the number of flipflops is small.
   double cells_per_area;
   if (makeChannels && stats[BBX_N_CELL] > 100)
      cells_per_area = MEAN_CELLS_PER_AREA_IF_CHANNELED;
   else
      cells_per_area = MEAN_CELLS_PER_AREA;
   int numberOfPlacementAreas = 1 + int ((stats[BBX_N_CELL] -1) / cells_per_area);
   if (numberOfPlacementAreas < 2) return 0; // nothing to partition...
   if (numberOfPlacementAreas > MAXIMUM_NUMBER_OF_PLACEMENT_AREAS)
      // we don\'t want too many areas because it\'s too time consuming...
      numberOfPlacementAreas = MAXIMUM_NUMBER_OF_PLACEMENT_AREAS;

   // The shapeCoefficient tells us something about the shape of the cells:
   double shapeCoef = stats[BBX_MEAN_Y] / stats[BBX_MEAN_X];
   // Now think of a reasonable partitioning. We must be aware that
   //   1. targetAreas[HOR] * targetAreas[VER] == numberOfPlacementAreas
   //   2. if cells are very wide we want targetAreas[HOR] << targetAreas[VER]
   //                                               (and the other way around)
   targetAreas[HOR] = sqrt (numberOfPlacementAreas * shapeCoef);
   targetAreas[VER] = sqrt (numberOfPlacementAreas / shapeCoef);

   // initialize the targetGridPoints...:
   targetGridPoints[HOR] = requestedGridPoints[HOR];
   targetGridPoints[VER] = requestedGridPoints[VER];

   // And now we take into account the area that the user specified...:
   // First make sure that the requested area is large enough. If it is already
   // too large, do not make it smaller...
   double requiredSurface =
      stats[BBX_MEAN_X] * stats[BBX_MEAN_Y] * stats[BBX_N_CELL];
   double requestedSurface = requestedGridPoints[HOR] * requestedGridPoints[VER];
   if (requiredSurface > requestedSurface)
   {
      // Expand the requested surface in the direction that the user specified
      // as the "minimum" size. Don\'t touch the other side!
      targetGridPoints[expandableDirection] =
	 int (0.5 + requestedGridPoints[expandableDirection] *
	     (requiredSurface / requestedSurface));
   }
   // At this point we have two potentially conflicting requirements:
   //  1. targetAreas[HOR..VER] is based on the shapes of the cells to be placed
   //  2. targetGridPoints[HOR..VER] is based on the user-supplied sizes
   // Our problem is to find a targetCoreCells[HOR..VER] such that it fits (in
   // both directions) an integer number of times in targetAreas[HOR..VER]. Of
   // course, targetCoreCells must be as close as possible to targetGridPoints
   // AND it must respect the expandableDirection specified by the user...
   //
   //                     ____ D O N \'T   P A N I C ____

   // first create and initialize targetCoreCells...:
   double targetCoreCells[2], f;
   int dir = expandableDirection;

   targetCoreCells[dir] = gridPointsToCoreCells (targetGridPoints[dir], dir);
   dir = otherDirection (expandableDirection); /* fixed direction */
   targetCoreCells[dir] = gridPointsToCoreCells (targetGridPoints[dir], dir);

   // Well, actually we are only interested in the number of image core cells
   // in a "scaled" fashion.   Suppose most cells have a height of 10 times a
   // core cell, then we rather have something like:
   if ((f = gridPointsToCoreCells ((int)stats[BBX_MEAN_X], HOR)) > 1) targetCoreCells[HOR] /= f;
   if ((f = gridPointsToCoreCells ((int)stats[BBX_MEAN_Y], VER)) > 1) targetCoreCells[VER] /= f;

   // Find out what the critical direction is. This is the direction where the
   // "resolution" is the smallest, i.e. where targetCoreCells/targetAreas is
   // smallest...:
   int criticalDirection =
	targetCoreCells[HOR]/targetAreas[HOR] < targetCoreCells[VER]/targetAreas[VER] ? HOR : VER;

   if (targetAreas[criticalDirection] >= targetCoreCells[criticalDirection])
   {
      // we have more areas than we have core cells in the critical direction...
      if (criticalDirection == expandableDirection)
	 // fortunately, we can expand the number of core cells....:
	 targetCoreCells[criticalDirection] = int (targetAreas[criticalDirection]);
      else
      {
	 // not allowed to expand number of core cells.... the only thing we
	 // can do here is lower targetAreas to match targetCoreCells...:
	 targetAreas[criticalDirection] = targetCoreCells[criticalDirection];
	 targetAreas[expandableDirection] = float (numberOfPlacementAreas)/targetAreas[criticalDirection];
      }
   }
   else // I think this "else" part is rubbish...:
   {
      // we have more core cells than areas in the critical direction...
      if (criticalDirection == expandableDirection)
	 // it\'s our lucky day: we are allowed to increase targetCoreCells...:
	 targetCoreCells[criticalDirection] =
	    int (ceil (targetCoreCells[criticalDirection]/targetAreas[criticalDirection])
		* targetAreas[criticalDirection]);
      else
      {
	 // not allowed to expand, so we make it smaller...:
	 double shapeCorrection =
	    floor (targetCoreCells[criticalDirection]/targetAreas[criticalDirection]);
	 targetCoreCells[criticalDirection] =
	    int (targetAreas[criticalDirection] * shapeCorrection);
	 targetCoreCells[expandableDirection] =
	    int (targetAreas[expandableDirection] / shapeCorrection);
      }
   }

   // output the results....:
   if ((*nx = int (targetAreas[HOR] + 0.5)) < 1) *nx = 1;
   if ((*ny = int (targetAreas[VER] + 0.5)) < 1) *ny = 1;

   cout << "------ partitioning " << int (stats[BBX_N_CELL])
        << " cells into (hor x vert) = ("
 	<< *nx << " x " << *ny << ") partitions\n" << endl;

   // and make sure that requestedGridPoints[] is consistent with our choice
   // of number of areas in horizontal (nx) and vertical (ny) direction:
   int k = 1;
   while (*nx * coreCellsToGridPoints (k, HOR) < requestedGridPoints[HOR]) ++k;
   // k is the number of core cells per area in horizontal direction:
   requestedGridPoints[HOR] = (*nx * coreCellsToGridPoints (k, HOR));
   k = 1;
   while (*ny * coreCellsToGridPoints (k, VER) < requestedGridPoints[VER]) ++k;
   // k is the number of core cells per area in vertical direction:
   requestedGridPoints[VER] = (*ny * coreCellsToGridPoints (k, VER));

   // on the other hand, we loose badly if all the code above somehow arranged
   // for a grid that in one or two dimensions is smaller than the largest cell
   // that we are going to place... Assert that this is not the case:
   if (requestedGridPoints[HOR] < stats[BBX_MAX_X]) {
      k = int (ceil (stats[BBX_MAX_X]));
      requestedGridPoints[HOR] = coreCellsToGridPoints (gridPointsToCoreCells (k, RoundUp), HOR);
   }
   if (requestedGridPoints[VER] < stats[BBX_MAX_Y]) {
      k = int (ceil (stats[BBX_MAX_Y]));
      requestedGridPoints[VER] = coreCellsToGridPoints (gridPointsToCoreCells (k, RoundUp), VER);
   }
   cout << "------ requested grid points = (" << requestedGridPoints[HOR]
	<< ", "	<< requestedGridPoints[VER] << ")\n" << endl;

   costfunctions[COSTSTATE] = thecostfunctions[COSTSTATE];
   costfunctions[COSTVEC]   = thecostfunctions[COSTVEC];

   return TRUE;
}

#define TOO_SMALL 0.00001
#define TOO_BIG   10000.0

// This function tries to think of reasonable grid sizes when the user did not
// specify anything...
static void thinkAboutGridSize (double stats[], double extraplaza)
{
   if (extraplaza < TOO_SMALL || extraplaza > TOO_BIG)
      err (5, (char*)"unreasonable value for extraplaza");

   int hor = requestedGridPoints[HOR];
   int ver = requestedGridPoints[VER];

   if (hor == NOTINITIALIZED && ver == NOTINITIALIZED) {
      // no preferred size in both directions... assume square
      hor = ver = int (0.5 + sqrt (stats[BBX_AREA] * extraplaza));
   }
   else if (hor == NOTINITIALIZED) {
      hor = int (0.5 + sqrt (stats[BBX_AREA] * extraplaza / ver));
   }
   else if (ver == NOTINITIALIZED) {
      ver = int (0.5 + sqrt (stats[BBX_AREA] * extraplaza / hor));
   }

   // check for extreme cases
   if (hor < stats[BBX_MAX_X]) hor = int (ceil (stats[BBX_MAX_X]));
   if (ver < stats[BBX_MAX_Y]) ver = int (ceil (stats[BBX_MAX_Y]));

   // round requestedGridPoints to a multiple of the core cell size
   hor = int (gridPointsToCoreCells (hor, HOR, RoundUp));
   if ((hor = coreCellsToGridPoints (hor, HOR)) < 1) hor = 1;
   ver = int (gridPointsToCoreCells (ver, VER, RoundUp));
   if ((ver = coreCellsToGridPoints (ver, VER)) < 1) ver = 1;

   requestedGridPoints[HOR] = hor;
   requestedGridPoints[VER] = ver;

   // if for some reason the requested area is far too small, increase it to
   // match a magnification of newAreaQuotient
#define tooSmallAreaQuotient 0.5
#define newAreaQuotient 0.7
   double areaQuotient = (hor * ver) / (stats[BBX_N_CELL] * stats[BBX_MEAN_X] * stats[BBX_MEAN_Y]);

   if (areaQuotient < tooSmallAreaQuotient) {
      requestedGridPoints[expandableDirection] =
	 int (double (requestedGridPoints[expandableDirection]) * (newAreaQuotient / areaQuotient));
   }
}

// this static variable contains the sizes of a core cell...:
static int coreCellSize[] = {NOTINITIALIZED, NOTINITIALIZED, NOTINITIALIZED};

static int coreCellOverlap[] = {NOTINITIALIZED, NOTINITIALIZED, NOTINITIALIZED};

// convert #gridpoints to #core cells...:
static double gridPointsToCoreCells (int gridpoints, int direction, RoundingPolicy round)
{
   if (gridpoints <= 0) return 0;

   if (coreCellSize[HOR] == NOTINITIALIZED)
   {
      IMAGEDESC *id = getImageDesc(); // get stuff from image.seadif
      coreCellSize[HOR]    = id->size[HOR];
      coreCellSize[VER]    = id->size[VER];
      coreCellOverlap[HOR] = id->overlap[HOR];
      coreCellOverlap[VER] = id->overlap[VER];
   }

   double fgridpoints = double (gridpoints - coreCellOverlap[direction]);
   if (fgridpoints < 1.0) fgridpoints = 1.0;
   double fcorecellsize = double (coreCellSize[direction]);
   if (fcorecellsize < 1.0) fcorecellsize = 1.0;

   switch (round)
   {
   case NoRounding:
      return fgridpoints / fcorecellsize;
   case RoundDown:
      return floor (fgridpoints / fcorecellsize);
   case RoundUp:
      return ceil (fgridpoints / fcorecellsize);
   case RoundNear:
      return floor (0.5 + fgridpoints / fcorecellsize);
   default:
      err (5, (char*)"(INTERNAL) gridPointsToCoreCells: illegal rounding policy specified");
      return -1;		// not reached
   }
}

static int coreCellsToGridPoints (double coreCells, int direction)
{
   if (coreCells <= 0) return 0;

   if (coreCellSize[HOR] == NOTINITIALIZED)
   {
      IMAGEDESC *id = getImageDesc(); // get stuff from image.seadif
      coreCellSize[HOR] = id->size[HOR];
      coreCellSize[VER] = id->size[VER];
   }

   return int (0.5 + coreCellSize[direction] * coreCells);
}

// This function turns the direction 90 degrees...:
static int otherDirection (int dir)
{
   if (dir == HOR) return VER;
   if (dir == VER) return HOR;
   err (5, (char*)"(internal error) illegal direction: HOR or VER expected");
   return 0;
}

#define VERY_NEGATIVE -10000

// Compute some stats of the bounding boxes...:
static void computeBbxStatistics (CIRCUITPTR circuit, double bbxStatistics[])
{
   int totalx = 0, totaly = 0, totalarea = 0;
   int maximumx = VERY_NEGATIVE, maximumy = VERY_NEGATIVE;
   CIRINSTPTR cirinst = circuit->cirinst;

   static STRING tmpcell = cs ((char*)"Tmp_Cell_");
   // if (!tmpcell) tmpcell = cs ("Tmp_Cell_");	// skip these layouts

   // integer n counts the number of circuits, integer n2 counts the total
   // number of layouts that are available for these circuits.
   int n, n2 = 0;
   for (n = 0; cirinst; cirinst = cirinst->next, ++n)
   {
      LAYOUTPTR lay = cirinst->circuit->layout;
      // OK, what to do? There can be more than one layout for this cicuit.
      // Maybe it\'s best to take them all into account...:
      int old_n2;
      for (old_n2 = n2; lay; lay = lay->next)
      {
	 if (lay->name == tmpcell) continue;
	 ++n2;
	 totalx += lay->bbx[HOR];
	 totaly += lay->bbx[VER];
	 totalarea += lay->bbx[HOR] * lay->bbx[VER];
	 if (lay->bbx[HOR] > maximumx) maximumx = lay->bbx[HOR];
	 if (lay->bbx[VER] > maximumy) maximumy = lay->bbx[VER];
      }
      if (old_n2 == n2) { // this should never happen
	 char s[200];
	 sprintf (s, "(INTERNAL) findrect: circuit (%s(%s(%s))) w/o layout",
		 cirinst->circuit->name, cirinst->circuit->function->name,
		 cirinst->circuit->function->library->name);
	 err (5, s);
      }
   }
   // avoid division by zero (even if n = 0)
   if (n2 == 0) ++n2;
   bbxStatistics[BBX_MEAN_X] = double (totalx) / n2;
   bbxStatistics[BBX_MEAN_Y] = double (totaly) / n2;
   bbxStatistics[BBX_MAX_X]  = (double) (maximumx < 0 ? 0 : maximumx);
   bbxStatistics[BBX_MAX_Y]  = (double) (maximumy < 0 ? 0 : maximumy);
   bbxStatistics[BBX_AREA]   = (double) totalarea;
   bbxStatistics[BBX_AVAIL]  = double (n) / n2;
   bbxStatistics[BBX_N_CELL] = (double) n;
}
