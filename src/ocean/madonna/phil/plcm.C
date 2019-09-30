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
 * PLCM Placement process
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "src/ocean/madonna/phil/plcm.h"
#include "src/ocean/madonna/phil/cluster.h"
#include "src/ocean/madonna/phil/plane.h"
#include "src/ocean/madonna/phil/protArea.h"

#include <sys/types.h>

extern "C" {
   pid_t getpid (void);
}

#define TRANS_DEBUG  1
// #define SLICE_DEBUG  0

Plcm::Plcm(
	char* cName,
	char* fName,
	char* lName,
	IMAGEDESC* imageDesc,
	ImageMap* imMap,
	char* oName,
	CIRCUIT* partitioned,
	GLOBAL_ROUTING* g_rout,
	Boolean sl,
	Boolean da,
	Boolean v,
	Boolean p,
	int sr,
	int mm): set_srand (sr),
		random_points (p),
		macroMinSize (mm),
		thisImage (*imageDesc),
		imageMap (imMap),
		freeNets (5, 0, 5),
		globRouting (g_rout),
		slicingLayout (sl),
		doTransAna (da)
//
// Constructor: Assumes that database is already opened.
{

  srand(int(set_srand > 0 ? set_srand : getpid()));

  if (slicingLayout && doTransAna) random_points = false;

  if (doTransAna) globRouting = NULL;

  circuitName  = cs(cName);
  functionName = cs(fName);
  libraryName  = cs(lName);
  layoutName   = !oName ? cs(cName) : cs(oName);

  verboseMode = v;
  fromPartitioner = partitioned;
  layInstancesList = NULL;

  plane = NULL; // for future checking if "read" and "prepare" were called

  // defining default "free" nets names

  freeNets.add (*new CriNet(cs((char*)"vdd")));
  freeNets.add (*new CriNet(cs((char*)"vss")));
  freeNets.add (*new CriNet(cs((char*)"gnd")));
  freeNets.add (*new CriNet(cs((char*)"power")));
  freeNets.add (*new CriNet(cs((char*)"Vdd")));
  freeNets.add (*new CriNet(cs((char*)"Vdd")));
}

void Plcm::read()
//
// Reads input circuit data from database and runs parser.
{
  if (!fromPartitioner)
  {
    if (sdfreadallcir(SDFCIRALL, cs(circuitName), cs(functionName), cs(libraryName)) == 0)
      usrErr ((char*)"Plcm::Plcm", EINPDAT);

    givenCircuit = thiscir;
  }
  else
    givenCircuit = fromPartitioner;

				// that's non partitioned circuit...
				// don't do slicing.

  if (givenCircuit->cirinst->circuit->status &&
     (strstr (givenCircuit->cirinst->circuit->status->program, "mad_prim") ||
      strstr (givenCircuit->cirinst->circuit->status->program, "libprim")))
  {
    slicingLayout = false;
    globRouting = NULL;
    doTransAna = false;
  }
}

void Plcm::prepare()
{
  if (!imageMap) usrErr ((char*)"Plcm::prepare", EINPDAT);
                                   // number of sectors in cluster can not
                                   // be greater then number of bits in
                                   // data of type clusterMapType
  if (thisImage.numsector > 8 * (int)sizeof(clusterMapType))
    usrErr ((char*)"Plcm::Plcm", ETASKIMP);
                                   // if this occurs you should change typedef
                                   // definition for clusterMapType to data
                                   // of bigger size

  clearFlags(givenCircuit);

  totalArea = readLayouts(givenCircuit);  // also in each circuit->flag.l
                                          // leaves number of children

                                   // now create empty layout for our circuit

  if (!NewLayout(layoutToBuild)) usrErr ((char*)"Plcd", ENOTMEM);

  layoutToBuild->name = layoutName;
  layoutToBuild->layport = NULL;
  layoutToBuild->bbx[HOR] = 0;	// we don\'t know yet
  layoutToBuild->bbx[VER] = 0;
  layoutToBuild->off[HOR] = 0;
  layoutToBuild->off[VER] = 0;
  layoutToBuild->wire = NULL;

  layoutToBuild->linkcnt = 1;

  if (!NewStatus(layoutToBuild->status)) usrErr ((char*)"Plcd", ENOTMEM);
				// only temporary - just to find out
				// slicing configuration

  time(&layoutToBuild->status->timestamp);
  layoutToBuild->status->author = cs((char*)"Madonna");
  layoutToBuild->status->program = cs((char*)"phil");
  layoutToBuild->next = NULL;

  SLICE *slPtr;

  if (!(slPtr = NewSlice(layoutToBuild->slice))) usrErr ((char*)"Plcd", ENOTMEM);

  if (slicingLayout)
  {
    Cluster*tmpcls = new Cluster(0, 0, thisImage.size[HOR], thisImage.size[VER]);
    Window  tmpWindow(*tmpcls, 100, 100);
    int tmpInt;

    tmpWindow.getDiv(givenCircuit->name, &slicesInHor, &slicesInVer, &tmpInt, &tmpInt);

				// here building 2-level slicing structure

    slPtr->ordination = VERTICAL;
    slPtr->chld_type = SLICE_CHLD;
    slPtr->chld.slice = NULL;	// only this one will be used
    slPtr->next = NULL;

    for (int i = 0; i < slicesInHor; i++)
    {
      SLICE *slvPtr;
      if (!NewSlice(slvPtr)) usrErr ((char*)"Plcd", ENOTMEM);
				// vertical slices

      slvPtr->ordination = HORIZONTAL; // orietation is horizontal
      slvPtr->chld_type = SLICE_CHLD;
      slvPtr->chld.slice = NULL;
      slvPtr->next = slPtr->chld.slice;
      slPtr->chld.slice = slvPtr;

				// now horizontal slices

      for (int j = 0; j < slicesInVer; j++)
      {
	SLICE *slhPtr;
	if (!NewSlice(slhPtr)) usrErr ((char*)"Plcd", ENOTMEM);
	slhPtr->ordination = CHAOS; // these are CHAOS already
	slhPtr->chld_type = LAYINST_CHLD;
	slhPtr->chld.layinst = NULL; // here we\'ll later attach lay instances

	slhPtr->next = slvPtr->chld.slice;
	slvPtr->chld.slice = slhPtr;
      }
    }
  }
  else
  {
    slPtr->ordination = CHAOS;
    slPtr->chld_type = LAYINST_CHLD;
    slPtr->next = NULL;
    slPtr->chld.layinst = NULL;
    layInstancesList = NULL;
  }
}

Plcm::~Plcm()
{
// Assumes that database will be closed externally.

 // freeFlagItem (givenCircuit);

    delete plane;
}

int Plcm::doPlacement(CIRCUIT* circuitPtr, Boolean first)
//
// main placement routine. Returns 1 if placement impossible.
{
  if (!circuitPtr->cirinst) return 0; // empty partition - nothing to do
  if (!circuitPtr->cirinst->circuit) usrErr ((char*)"Plcm::doPlacement", EUNKNOW);

  if (circuitPtr->cirinst->circuit->status &&
     (strstr (circuitPtr->cirinst->circuit->status->program, "mad_prim") ||
     strstr (circuitPtr->cirinst->circuit->status->program, "libprim")))
  {
                                // this circuit has only libprim children
                                // so we can start placement
                                // but ...
				// before we start placing let\'s better sort the
				// elements so that biggest one will come first

    circuitPtr->cirinst = sortGroup(circuitPtr->cirinst);

    if (!slicingLayout) layInstancesList = &layoutToBuild->slice->chld.layinst;

    if (placeGroup(circuitPtr->cirinst, *(Window *)circuitPtr->flag.p)) return 1;
  }
  else
  {
    CIRINST *ciPtr;

    if (first && slicingLayout)	// we have to set slice info
    {				        // for each of slices
      SLICE *thisSlice;

      for (ciPtr = circuitPtr->cirinst; ciPtr; ciPtr = ciPtr->next)
      {
	thisSlice = findSlice(ciPtr);
				// also we have to store slice info
				// in the flag field of the slice
	SLICE_INFO *tr_data = new SLICE_INFO;
	Window *wPtr = (Window*)ciPtr->circuit->flag.p;
	tr_data->cX = wPtr->cX;
	tr_data->cY = wPtr->cY;
	tr_data->width = wPtr->width;
	tr_data->height = wPtr->high;
	tr_data->layerTrans = NULL;
	thisSlice->flag.p = (void*)tr_data;
      }
    }

    for (ciPtr = circuitPtr->cirinst; ciPtr; ciPtr = ciPtr->next)
    {
      if (first && slicingLayout)
      {
	SLICE *sPtr = findSlice(ciPtr);
	layInstancesList = &sPtr->chld.layinst;
      }
      else if (first) layInstancesList = &layoutToBuild->slice->chld.layinst;

      if (doPlacement(ciPtr->circuit)) return 1;
    }
  }
#ifdef SLICE_DEBUG

  if (first)
  {
    SLICE *slvPtr, *slhPtr;

    for (slvPtr = layoutToBuild->slice->chld.slice; slvPtr; slvPtr = slvPtr->next)
      for (slhPtr = slvPtr->chld.slice; slhPtr; slhPtr = slhPtr->next)
      {
	SLICE_INFO* siPtr = (SLICE_INFO*)slhPtr->flag.p;
	cout << "Slice, ord " << slhPtr->ordination << " ("
	     << siPtr->cX << "," << siPtr->cY << ")" << endl;
	for (LAYINST* liPtr = slhPtr->chld.layinst; liPtr; liPtr = liPtr->next)
	  cout << liPtr->name << ",";
	cout << endl;
      }
  }
#endif
  return 0;
}

int Plcm::placeGroup(CIRINST* cInstList, Window& windowRef)
//
// Tries to place group of cells (libprim) in given window.
// Returns 1 if there is not enough place.
{
  int width = windowRef.width;
  int high  = windowRef.high;
  int x = windowRef.cX;
  int y = windowRef.cY;
  int clustersInWindow = width * high;
  int tooBigCellInGroup = 0;
  CIRINST *ciPtr;

  // first lets find if we don't have a huge element in this group

  for (ciPtr = cInstList; ciPtr; ciPtr = ciPtr->next)
  {
    LAYOUT *layPtr = ciPtr->circuit->layout;

    if (!strstr (layPtr->name, "Tmp_Cell_")) // this layouts are generraly ignored
    {
      List& listRef = *(List*)layPtr->flag.p;
      ListIterator lIter(listRef);
      Pattern& patRef = (Pattern&)(Item&)lIter;

      if (patRef.getItemsInBox() > 0.25 * clustersInWindow || patRef.isMacro())
      {
	tooBigCellInGroup = 1;
	break;
      }
    }
  }

  if (verboseMode) {
    cout << "Placing group inside window (" << x << "," << y << "," << x + width
         << "," << y + high << ")" << endl;
  }

  if (random_points && !tooBigCellInGroup)
  {
    for (int security = 0; security < width * high; security++)
    {
      int i = x + (rand() % width);
      int j = y + (rand() % high);

      Cluster cluster(i, j, thisImage.size[HOR], thisImage.size[VER]);

      if (tryThisCluster(cInstList, cluster, (ProtArea&)NOITEM)) return 0;
      // all cells placed
      // end of job here
    }
  }
  // if still there's something to place or random placement disabled

  // because it may happen that there's no place for all elements from
  // this group in current window after trying it, surrounding area is also
  // tried (going through all rings of one window width around).

  int outerX1 = x,
      outerY1 = y,
      innerX1 = windowRef.cX + width,
      innerY1 = windowRef.cY + high,
      outerX2 = innerX1,
      outerY2 = innerY1,
      innerX2 = x,
      innerY2 = y,
      xMax = plane->cols * thisImage.size[HOR],
      yMax = plane->rows * thisImage.size[VER];

  for (int c = 0; innerX1 > 0 || innerX2 < xMax || innerY1 > 0 || innerY2 < yMax;
                  c++, innerX1 -= width, outerX1 -= width,
                  innerX2 += width, outerX2 += width,
                  innerY1 -= high, outerY1 -= high,
                  innerY2 += high, outerY2 += high)
  {
    if (c == 1 && verboseMode)
    {
      cerr << "\n There's too few place in "
       << "window at (" << x << "," << y << ") for this group of elements "
       << "\n searching in the surroundings for place for cell[s]:\n";

      for (ciPtr = cInstList; ciPtr; ciPtr = ciPtr->next) {
	  if (ciPtr->flag.l == 0) cerr << "  " << ciPtr->name << endl;
      }
    }

    int i, j, iStart, jStart;
    if ((iStart = outerX1) < 0) iStart = 0;
    if ((jStart = outerY1) < 0) jStart = 0;

    Cluster lbc(iStart, jStart, thisImage.size[HOR], thisImage.size[VER]);
    ProtArea pa(lbc); // this is a so called "protected area"
		      // here you should try to place anymore

    for (i = iStart; i < outerX2 && i < xMax; i++)
    for (j = jStart; j < outerY2 && j < yMax; j++)
      if (((i >= outerX1 && i < innerX1) || (i >= innerX2 && i < outerX2) ||
           (j >= outerY1 && j < innerY1) || (j >= innerY2 && j < outerY2)) &&
	 !pa.is(i,j))  // do try this cluster if it's not within protected area
      {
	Cluster cluster (i, j, thisImage.size[HOR], thisImage.size[VER]);

	if (tryThisCluster (cInstList, cluster, pa))
	{
	  if (c >= 1 && verboseMode) cerr << "\n  ====== SUCCESS ! ===== " << endl;
	  return 0; // all cells placed, end of job here
	}
      }
  }

  for (ciPtr = cInstList; ciPtr; ciPtr = ciPtr->next) {
    if (ciPtr->flag.l == 0) {
      cerr << " Can't place cell :";
      cerr << ciPtr->name << "  ";
      cerr << "in window at (" << x << "," << y << ")"<< endl;
    }
  }

//  cerr << "\n SORRY, apparently the magnification coefficient is too small. \n";
//  cerr << " Try to run program once again or change coefficient. \n";

  cerr << "\n Apparently the magnification coefficient is too small. \n";
  cerr << " Let's try to run the algorithm for bigger magn. coefficient. \n";

  return 1;
}

Boolean Plcm::tryThisCluster(CIRINST* ciPtr, Cluster& here, ProtArea& pa)
//
// Tries to place group of cells in this cluster.
// Translation point in bottom-left corner of cluster.
// Return true if all cell placed here.
// Updates "protected area".
{
  Boolean successFlag = true;

  for (; ciPtr; ciPtr = ciPtr->next)
    if (ciPtr->flag.l == 0)         //still not placed
    {
      for (LAYOUT *lPtr = ciPtr->circuit->layout; lPtr; lPtr = lPtr->next)
      {
	if (!strstr (lPtr->name, "Tmp_Cell_")) // don't consider nelsis's tmp cells
	{
	  Pattern& good = tryThisCellHere (lPtr, here, pa);

	  if (good != NOITEM)      // we can place this cell here
	  {
	    placeOneCell (ciPtr, lPtr, good, here, pa);
	    break;			// only one layout should be selected
	  }
	  else
	    successFlag = false; // to indicate that at least one cell still not placed
	}
      }
    }
  return successFlag;
}

Pattern& Plcm::tryThisCellHere(LAYOUT* lPtr, Cluster& here, ProtArea& pa)
//
// Tries to place this layout here by trying all patterns
// and checking if obtained points belong to still free sectors
// Returns ref to pattern on success and NOITEM if failed.
// Also checks if conflict critical points exist.
{
  List& patListRef = *(List*)lPtr->flag.p;
  int x = here.cX;
  int y = here.cY;

  for (ListIterator patIter(patListRef); (Item&)patIter != NOITEM; patIter++)
  {
    Pattern& patternRef = (Pattern&)(Item&)patIter;

    if (!patternRef.isMacro())
    {
      Boolean failed = false;

      for (ListIterator cluIter(patternRef); (Item&)cluIter != NOITEM; cluIter++)
      {
	Clst &cluRef = (Clst&)(Item&)cluIter;

        /* I have changed clusterMapType(cluRef) into cluRef.pattern
           because it appeared that the first one did not always return
           the correct value when gcc is used.  AvG 17081999 */

	if (cluRef.pattern & plane->getPattern (x + cluRef.cX, y + cluRef.cY)) {
	                                // if even one of sectors
	                                // is already occupied then
	                                // we must try another pattern
	  failed = true;
	  break;
	}
	List& criList = plane->getCriticals (x + cluRef.cX, y + cluRef.cY);
        if (criList != NOITEM && cluRef & criList) {
	                                // also compares lists of critical points.
	                                // For details look operator &(List&,List&)
	  failed = true;
	  break;
	}
      }
      if (!failed) return patternRef; // no sectors in common at all - success!
    }
    else // is a macro
    {
      Boolean failed = false;
      for (int i = 0; i < patternRef.getHor(); i++)
	for (int j = 0; j < patternRef.getVer(); j++)
	  if (plane->getPattern(x + i, y + j) != 0) // it's occupied
	  {
	    failed = true;
	    pa.update(x + i, y + j);
	    break;
	  }
      if (!failed) return patternRef;
    }
  }
  return (Pattern&)NOITEM; // Placement of this cell in this cluster impossible
}

void Plcm::placeOneCell(CIRINST* ciPtr, LAYOUT* lPtr, Pattern& patRef, Cluster& here, ProtArea& pa)
//
// Marks new sectors in plane and adds new layout instance to layout
// being generated. Updates protected area too.
{
  int  x = here.cX,
       y = here.cY,
       minx = x,
       miny = y;

    // first we have to find out what is the real offset
    // now marking placement plane

  if (!patRef.isMacro())		// ordinary cell
  {
    for (ListIterator cluIter(patRef); (Item&)cluIter != NOITEM; cluIter++)
    {
      Clst &cluRef = (Clst&)(Item&)cluIter;

      plane->mark(x + cluRef.cX, y + cluRef.cY, cluRef.pattern, cluRef.criticalPoints);

      if (minx > x + cluRef.cX) minx = x + cluRef.cX;
      if (miny > y + cluRef.cY) miny = y + cluRef.cY;
    }
  }
  else // that's a macro
  {
    if (verboseMode) cout << "Placing " << ciPtr->name << " as a macro." << endl;

    clusterMapType pat = 0;
    pat = ~pat; // this sets all our bits to 1
    List em;
    for (int i = 0; i < patRef.getHor(); i++)
      for (int j = 0; j < patRef.getVer(); j++)
	plane->mark(x + i, y + j, pat, em);
    pa.update(x + patRef.getHor(), y + patRef.getVer()); // the highest point
  }

    // now we should add new layout instance
  LAYINST *liPtr;

  if (!NewLayinst(liPtr)) usrErr ((char*)"Plcm::placeOneCell", ENOTMEM);

  liPtr->name = cs(ciPtr->name);
  liPtr->layout = lPtr;
  liPtr->flag.s[0] = minx; // needed during compaction
  liPtr->flag.s[1] = miny;

  memcpy (liPtr->mtx, (short*)patRef, 6 * sizeof(short));

				// this offset is composed of three parts:
				// 1. real coeficient for transformation
				// 2. offset to move it back to (0,0) after trans.
				// 3. offset within placement plane

// liPtr->mtx[B1] += here.x - coX;
// liPtr->mtx[B2] += here.y - coY;

  liPtr->mtx[B1] += here.x;
  liPtr->mtx[B2] += here.y;
  Boolean placed = false;
  SLICE *slvPtr, *slhPtr;
  int i, j;

  if (slicingLayout) // we have to find out which slice it actually is..
  {
    for (i = 0, slvPtr = layoutToBuild->slice->chld.slice;
	      slvPtr && i < slicesInHor; i++, slvPtr = slvPtr->next)
      for (j = 0, slhPtr = slvPtr->chld.slice;
	      slhPtr && j < slicesInVer; j++, slhPtr = slhPtr->next)
      {
	SLICE_INFO* siPtr = (SLICE_INFO*)slhPtr->flag.p;
	if (siPtr->cX <= here.cX && here.cX <siPtr->cX + siPtr->width &&
	    siPtr->cY <= here.cY && here.cY <siPtr->cY + siPtr->height)
	{
	  liPtr->next = slhPtr->chld.layinst;
	  slhPtr->chld.layinst = liPtr;
	  placed = true;
	  break;
	}
      }
  //if (!placed) this may happen when we have channels (windows are smaller)
  }

  if (!placed) {
    liPtr->next = *layInstancesList;
    *layInstancesList = liPtr;
  }

  liPtr->layout->linkcnt++;

  if (verboseMode) cout << "Cell : " << ciPtr->name << " placed.\n";

  // now we only have to mark this one as already placed
  ciPtr->flag.l = 1;
}

void Plcm::freeFlagItem1(CIRCUIT* cPtr)
//
// This routine should free all :
//   Pattern's attached to each libprim layout.
{
  if (cPtr->layout && cPtr->status &&
      (strstr (cPtr->status->program, "mad_prim") ||
       strstr (cPtr->status->program, "libprim")))
  {
    for (LAYOUT *lPtr = cPtr->layout; lPtr; lPtr = lPtr->next)
      if (!strstr (lPtr->name, "Tmp_Cell_")) // don't consider nelsis tmp cells
	delete (List*)lPtr->flag.p;

    cPtr->layout->flag.p = NULL; // to be sure that we will not free twice
    return;
  }

  for (CIRINST *ciPtr = cPtr->cirinst; ciPtr; ciPtr = ciPtr->next)
  {
    freeFlagItem1 (ciPtr->circuit);
  }
}

void Plcm::freeFlagItem2(CIRCUIT* cPtr)
//
// This routine should free all :
//   Window's attached to each circuit which has cirinst of libprim type.
{
  delete (Window*)cPtr->flag.p;

  if (!cPtr->cirinst ||
      (cPtr->cirinst->circuit->status &&
       (strstr (cPtr->cirinst->circuit->status->program, "mad_prim") ||
	strstr (cPtr->cirinst->circuit->status->program, "libprim"))))
  {
    return; // this window was not divided
  }

  for (CIRINST* ciPtr = cPtr->cirinst; ciPtr; ciPtr = ciPtr->next)
  {
    freeFlagItem2 (ciPtr->circuit);
  }
}

void Plcm::write(CIRCUIT* cirPtr, Boolean doWrite)
//
// Writes created layout back to database.
// In the database we have two circuits : artificial created by partitioner
// called name_p and normal circuit name. We have to attach our new layout
// to this second one.
// If we're in "in core partitioning" then cirPtr must contains ptr
// to the real circuit to which we have to attach this new layout.
{
  if (slicingLayout) removeEmptySlices();

  // now we must change circuit name to indicate that it is already
  // not partitioned

  if (!fromPartitioner)
  {
    char *sPtr = strdup (givenCircuit->name);
    char *tokenPtr = strstr (sPtr, "_p");

    if (!tokenPtr)
    {
      cerr << "\nWas it the right kind of circuit ? " << endl;
      usrErr ((char*)"Plcm::write", EUNKNOW);
    }
    *tokenPtr = '\0';   // truncating last _p characters

    sPtr = cs(sPtr);

    if (sdfreadallcir(SDFCIRSTAT, sPtr, cs(functionName), cs(libraryName)) == 0)
      usrErr ((char*)"Plcm::write", EUNKNOW);

    thiscir->layout = layoutToBuild;
    layoutToBuild->circuit = thiscir;

    if (doWrite) sdfwritelay(SDFLAYALL, layoutToBuild);
  }
  else
  {
    cirPtr->layout = layoutToBuild;
    layoutToBuild->circuit = cirPtr;

    if (doWrite) sdfwritelay(SDFLAYALL, layoutToBuild);
  }
}

CIRINSTPTR Plcm::sortGroup(CIRINSTPTR cList)
//
// This routine sorts the elements on the list "cList" in decreasing order
// Returns the pointer to the new head. I\'m afraid this algorithm is half quadratic
// however we don\'t expect the list to be too long.
{
  CIRINST *head = NULL,
          *begin = cList,
          *current,
          *previous,
          *oneBefore = NULL,
          *smallest = NULL;
  int a;

  while (begin) // until all cells processed
  {
				// first let\'s find the smallest left
    int size = MAXINT;
    previous = NULL;

    for (current = begin; current; previous = current, current = current->next)
    {
      LAYOUT *lay = current->circuit->layout;
      if ((a = lay->bbx[HOR]*lay->bbx[VER]) < size)
      {
	size = a;
	smallest = current;
	oneBefore = previous;
      }
    }
				// now let\'s remove it from the list
    if (oneBefore)
      oneBefore->next = smallest->next;
    else
      begin = smallest->next;
				// finally we only have to add it to our
				// new sorted list
    previous = head;
    head = smallest;
    smallest->next = previous;
  }
  return head;
}

void Plcm::createPlane(int hor, int ver)
//
// This routine creates placement plane of the right size and assigns
// window to groups.
{
    // first adjust sizes to be multiplication
    // of basic cell

  if (hor % thisImage.size[HOR] != 0)
  {
    hor = (hor / thisImage.size[HOR] + 1) * thisImage.size[HOR];
  }
  if (ver % thisImage.size[VER] != 0)
  {
    ver = (ver / thisImage.size[VER] + 1) * thisImage.size[VER];
  }
				// this is the size of our chip
				// as adjusted by partitioner

  int  clustersInHor = hor / thisImage.size[HOR];
  int  clustersInVer = ver / thisImage.size[VER];

  clearFlags(givenCircuit);
                                         // clean two kinds of flags - used for
					 // windows and ones used for marking
					 // placed cells

  Cluster* cls = new Cluster(0, 0, thisImage.size[HOR], thisImage.size[VER]);

  if (globRouting) { // we have to reserve some space for routing channels
    Window tempW(*cls, clustersInHor, clustersInVer);

    int c, r, w, h, i;

    tempW.getDiv(givenCircuit->name, &c, &r, &w, &h);
    for (i = 0; i < c-1; i++) clustersInHor += globRouting->vertical_channels[i].ncells;
    for (i = 0; i < r-1; i++) clustersInVer += globRouting->horizontal_channels[i].ncells;
  }

  Window *bigWindow = new Window(*cls, clustersInHor, clustersInVer);

  givenCircuit->flag.p = (void*)bigWindow;

  plane = new Plane(clustersInVer, clustersInHor);

  makeWindows(givenCircuit, Boolean(globRouting != NULL));
                                // true - means add channels

  // check if this is a cleaned layout (all layout instances removed)
  if (layInstancesList) usrErr ((char*)"Plcm", EUNKNOW);

  // that\'s all, I suppose ...
}

void Plcm::recover()
//
// This routine removes all layout instances that have been placed, placement plane
// and windows, so that placement can be started again for the same circuit.
{
  int i, j;

  if (slicingLayout)
  {
    SLICE *slvPtr, *slhPtr;

    for (i = 0, slvPtr = layoutToBuild->slice->chld.slice;
	slvPtr && i < slicesInHor; i++, slvPtr = slvPtr->next)
      for (j = 0, slhPtr = slvPtr->chld.slice;
	      slhPtr && j < slicesInVer; j++, slhPtr = slhPtr->next)
      {
	for (LAYINSTPTR lPtr = slhPtr->chld.layinst; lPtr;)
	{
	  LAYINSTPTR tmp = lPtr;

	  lPtr = lPtr->next;
	  FreeLayinst(tmp);		// and get rid of it
	}
	slhPtr->chld.layinst = NULL;

				// also delete SLICE_INFO structures

	SLICE_INFO* tdPtr = (SLICE_INFO*)slhPtr->flag.p;
	delete tdPtr->layerTrans;
	delete tdPtr;
	slhPtr->flag.p = NULL;
      }
  }
  else
  {
    for (LAYINSTPTR lPtr = *layInstancesList; lPtr;)
    {
      LAYINSTPTR tmp = lPtr;

      lPtr = lPtr->next;
      FreeLayinst(tmp);		// and get rid of it
    }
    *layInstancesList = NULL;
  }
  layInstancesList = NULL;

  delete plane;

  freeFlagItem2(givenCircuit);	// Let\'s remove only windows
}

void Plcm::setBbx()
{
  int x, y;

  plane->getEffSize (x, y);

  layoutToBuild->bbx[HOR] = x * thisImage.size[HOR] + thisImage.overlap[HOR];
  layoutToBuild->bbx[VER] = y * thisImage.size[VER] + thisImage.overlap[VER];
}

int* Plcm::calcTransp(SLICE* theSlice)
//
// This function creates an array with each position saying what are
// the free numbers of tracks for each layer
// for this slice. If the layer 1 has orietation horizontal then we will
// get in array[1] number of horizontal wires that could through  this slice.
// The size of the slices in clusters have been previously saved in flag
// field of this slice (the whole Window object).
{
    // slice size in grid points

  SLICE_INFO* sliceInfo = (SLICE_INFO*)theSlice->flag.p;
  int sliceWidth = sliceInfo->width * thisImage.size[HOR],
      sliceHeight = sliceInfo->height * thisImage.size[VER],
      cXstart = sliceInfo->cX,
      cYstart = sliceInfo->cY;

				// first let\'s allocate the array

  int *theArray = new int [thisImage.numlayers];

				// ... and temporary arrays

  Transparency* trTable = new Transparency[thisImage.numlayers];

  for (int i = 0; i < thisImage.numlayers; i++)
    trTable[i] = Transparency (sliceWidth, sliceHeight);

  if (!trTable || !theArray) usrErr ((char*)"Plcm", ENOTMEM);

				// now filling-in these structures.

  for (LAYINST* liPtr = theSlice->chld.layinst; liPtr; liPtr = liPtr->next)
  {
    List& listRef = *(List*)liPtr->layout->flag.p;
    ListIterator lIter(listRef);
    Pattern& patRef = (Pattern&)(Item&)lIter;
				// the first one contains transMaps;

    Transparency *tranMaps = patRef.getTranMaps();

    if (!tranMaps) usrErr ((char*)"Plcm::calcTransp", EUNKNOW);

    int newCx, newCy;
				// find the new left bottom bottom corner of
				// the cell

    patRef.findNewLeftBottom(liPtr->layout->bbx[HOR], liPtr->layout->bbx[VER],
			     liPtr->mtx, newCx, newCy);

				// now for every layer

    for (int l = 0; l < thisImage.numlayers; l++)
    {
      Transparency* tranMap = &tranMaps[l];

				// now a small detail - the cell could be
				// placed upside down or even mirrored -
				// this may require some transformations

      Transparency* transformedMap = tranMap->transform(liPtr->mtx);


      int offset;

      transGridType* destPtr, *end, *srcPtr;

      if (thisImage.routeorient[l] == HOR)
      {
	offset = newCy - (cYstart * thisImage.size[VER]);
	destPtr = trTable[l].verGridMap + offset;
	if (offset + transformedMap->sizeVer < trTable[l].sizeVer)
	  end = destPtr + transformedMap->sizeVer;
	else
	  end = trTable[l].verGridMap + trTable[l].sizeVer;
	if (offset < 0)		// cell is partially located below
	{			// the slice
	  destPtr = trTable[l].verGridMap;
	  srcPtr = transformedMap->verGridMap - offset;
	}
	else
	  srcPtr = transformedMap->verGridMap;
      }
      else
      {
	offset = newCx - (cXstart * thisImage.size[HOR]);
	destPtr = trTable[l].horGridMap + offset;
	if (offset + transformedMap->sizeHor < trTable[l].sizeHor)
	  end = destPtr + transformedMap->sizeHor;
	else
	  end = trTable[l].horGridMap + trTable[l].sizeHor;
	if (offset < 0)
	{
	  destPtr = trTable[l].horGridMap;
	  srcPtr = transformedMap->horGridMap - offset;
	}
	else
	  srcPtr = transformedMap->horGridMap;
      }
				// now let\'s do logical and-ing

      for (; destPtr < end; destPtr++, srcPtr++) *destPtr |= *srcPtr;

      delete transformedMap;
    }
  }

				// now let's fill in the output array

  for (int lay = 0; lay < thisImage.numlayers; lay++)
  {
    Transparency &tPtr = trTable[lay];

    theArray[lay] = tPtr.freeTracks(thisImage.routeorient[lay]);
  }
  delete trTable;

  return theArray;
}

void Plcm::doTranspAnalysis()
//
// Perform transparency analysis for every slice.
{
  int i, j;

  if (!slicingLayout)
    usrErr ((char*)"Plcm::doTranspAnalysis", EUNKNOW);
    SLICE *slvPtr, *slhPtr;

    for (i = 0, slvPtr = layoutToBuild->slice->chld.slice;
	slvPtr && i < slicesInHor; i++, slvPtr = slvPtr->next)
      for (j = 0, slhPtr = slvPtr->chld.slice;
	      slhPtr && j < slicesInVer; j++, slhPtr = slhPtr->next)
      {
	SLICE_INFO* tdPtr = (SLICE_INFO*)slhPtr->flag.p;
	tdPtr->layerTrans = calcTransp(slhPtr);

#ifdef TRANS_DEBUG

        cout << "(" << tdPtr->cX << ","
	     << tdPtr->cY << ","
	     << tdPtr->width << ","
	     << tdPtr->height << ")";

	for (int k = 0; k < thisImage.numlayers; k++)
	  cout << "[" << tdPtr->layerTrans[k] << "]";
	cout << endl;
#endif

      }
}

SLICE* Plcm::findSlice(CIRINST* ciPtr)
{
  int     partNo = atoi(ciPtr->name) - 1;
  int     x, y;
  x = partNo % slicesInHor; // row & column no. (from 0)
  y = partNo / slicesInHor;

				// now we have to find the right slice
				// layinst pointer
  SLICE* slvPtr, *slhPtr;
  int i, j;

  for (i = 0, slvPtr = layoutToBuild->slice->chld.slice;
      slvPtr && i < x; i++, slvPtr = slvPtr->next);
  for (j = 0, slhPtr = slvPtr->chld.slice;
      slhPtr && j < y; j++, slhPtr = slhPtr->next);
				// now shlPtr points to the right slice
  return slhPtr;
}

void Plcm::removeEmptySlices()
//
// Some slices may turn out to be empty - we remove them.
{
  if (slicingLayout)
  {
    SLICE *slvPtr, *slhPtr, *prevPtr, *prevVPtr;
    int i, j;

    prevVPtr = layoutToBuild->slice;

    for (i = 0, slvPtr = layoutToBuild->slice->chld.slice;
	slvPtr && i < slicesInHor; i++)
    {
      prevPtr = slvPtr;
      for (j = 0, slhPtr = slvPtr->chld.slice;
	      slhPtr && j < slicesInVer; j++)
      {
	if (!slhPtr->chld.layinst) // we have to unlink that one
	{
	  if (prevPtr == slvPtr)
	    slvPtr->chld.slice = slhPtr->next;
	  else
	    prevPtr->next = slhPtr->next;
	  SLICE *tmp = slhPtr;
	  slhPtr = slhPtr->next;

	  SLICE_INFO* tdPtr = (SLICE_INFO*)tmp->flag.p;
	  if (doTransAna) delete tdPtr->layerTrans;
	  delete tdPtr;
	  FreeSlice(tmp);
	}
	else
	{
	  prevPtr = slhPtr;
	  slhPtr = slhPtr->next;
	}
      }				// it may also happen that we
				// remove all vertical subslices
      if (!slvPtr->chld.slice)
      {				// unplug this vertical slice
				// because it\'s empty
	if (prevVPtr == layoutToBuild->slice)
	  layoutToBuild->slice->chld.slice = slvPtr->next;
	else
	  prevVPtr->next = slvPtr->next;
	SLICE *vTmp = slvPtr;
	slvPtr = slvPtr->next;
	FreeSlice(vTmp);
      }
      else
      {
	prevVPtr = slvPtr;
	slvPtr = slvPtr->next;
      }
    }
  }
}
