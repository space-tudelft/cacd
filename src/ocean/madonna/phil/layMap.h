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

#ifndef __LAYMAP_H
#define __LAYMAP_H

#include "src/ocean/madonna/phil/cri.h"
#include "src/ocean/madonna/phil/imageMap.h"
#include "src/ocean/madonna/lia/list.h"
#include "src/ocean/madonna/lia/array.h"

class Clst;

// This class - contains map of particular layout which is used
// during searching for net name.
// To avoid useless operations the map of layout is created only
// when somebody will request it, calling findNetId(). Next call to
// this function already doesn\'t do it (only clears out previous
// tracing information).
// Every cell record contains:
//	layers  - bits are set if coresponding layer is present in basic image
//	visited - bit is set if corresponding point was already visited
//	termNo  - number of terminal net in this point. When zero - there is
//		  no terminal here. Value 1 means "constrained" net
//	criAreaNo - if this grid belongs to critical area then here there is
//		  it's number (equal position in criLines-1) otherwise set to 0
//	uniNo   - nr of restricted feed (position in base array) if exists in this point

class LayMap
{
public:
    LayMap(LAYOUT*, ImageMap*, Array*);
   ~LayMap();

    // function members :

    layerType    viaToCriticalArea (int x, int y);
    unsigned int findNetId (int x, int y, layerType lay, short* mtx);
    void         addCriticalPoints (Clst&, short* mtx);
    int          getHor() { return imageMap->cols; }
    int          getVer() { return imageMap->rows; }

private:

    // function members :

    unsigned int search (int x, int y, layerType lay);
    void         create ();
    void         addWires (int x, int y, LAYOUT*, short* mtx);
    void         goDeeper (SLICE*, LAYOUT*, short* mtx);
    void         refresh ();
    void         addTerms (short* mtx);
    void         recognizeTerminals ();
    void         clearFlags ();

    // data members :

    int xMin, xMax, yMin, yMax; // in grid points

    Boolean         initFlag; // contains true if map already created
    layoutMapType **map;
    LAYOUT         *layout;
    short          *actualMatrix;
    ImageMap       *imageMap;
    Array          *freeNets;
};

#endif
