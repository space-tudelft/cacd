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

#ifndef __PATTERN_H
#define __PATTERN_H

#include "src/ocean/madonna/phil/phil.h"
#include "src/ocean/madonna/phil/layMap.h"
#include "src/ocean/madonna/lia/list.h"
#include "src/ocean/madonna/lia/array.h"

// This structure keeps a map of gridpoints which are tranparent
// for wires going through the cell in this direction.

typedef char transGridType;

class Transparency
{
public:
  Transparency() {}
  Transparency(int sh, int sv);
 ~Transparency();

  Transparency&  operator = (const Transparency&);
  int            freeTracks (int dir);
  Transparency*  transform (short* mtx);

  transGridType* verGridMap;
  transGridType* horGridMap;
    // the size of these arrays is equal to the size of
    // the cell in one of the directions just for security reasons..
  int sizeHor;
  int sizeVer;
};

// It is a list of clusters (Clst) with marked sectors which
// given layout will occupy after applying transformation mtx
// in point (0,0) and with critical points (if any)
//
class Pattern: public List
{
public:
    Pattern(LAYOUT*, IMAGEDESC*, short* mtx, LayMap&, int doCheck = 0,
                                  int isMacro = 0, int doTransAna = 0);
   ~Pattern();

    virtual classType myNo() const { return PatternClass; }
    virtual char*   myName() const { return (char*)"Pattern"; }
    virtual int isEqual(const Item& o) const { return !memcmp(((Pattern&)o).matrix, matrix, 6); }

    operator short*() { return matrix; }
    void  addCriticals (LayMap&);
    void  removeUnnecessary (ImageMap&);

    int   isMacro() const { return isMac; }
    int   getHor()  const { return hor; }
    int   getVer()  const { return ver; }
    Transparency* getTranMaps() { return tranMaps; }
    void  findNewLeftBottom (int, int, short*, int&, int&);

private:
    void  scan     (short*, LAYOUT*, IMAGEDESC*, LayMap&);
    void  scanSlice (short*, SLICE*, IMAGEDESC*, LayMap&);
    void  markPowerLines (IMAGEDESC*);

    char  *cellname;		// just fort debugging...
    short *matrix;
    int    doCheck;
    int    doTransAna;
    int    isMac;		// if it is macro then "ver" and "hor"
				// fields give it's dimentions while
    int    ver;			// our list is empty (matrix should be then
    int    hor;			// always an identity matrix)

    // if transparency analysis of the cell
    // has been requested then here we have it:

    Transparency *tranMaps;	// allocated only if doTransAna
    int    noOfLayers;		// size of the tranMaps;
};

#endif
