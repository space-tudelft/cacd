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

#ifndef  __PARSERINTERFACE_H
#define  __PARSERINTERFACE_H

#include "src/ocean/madonna/phil/phil.h"
#include "src/ocean/madonna/phil/imageMap.h"

class  ParserInterface
{
public:
    ParserInterface();
   ~ParserInterface();

    void	read();
    ImageMap*	getImageMap () { return imageMap; }
    IMAGEDESC*	getImageDesc() { return &thisImage; }

    void	setDim (int x, int y);
    void	initImages ();
    void	addMirrorAxis (MIRROR*);
    int 	processFeeds (List& f, layerType l);
    void	addImageOverlap (int x, int y);
    void	setNumLayers (int numlay);
    int 	numLayers () { return thisImage.numlayers; }
    void	setRouteOrient  (int layer, int orient);
    void	setTransparency (int layer, int transp);
    void	addPowerLine (int orient, STRING name, int layer, int r_or_c);

private:
    void	removeDuplicates ();

    ImageMap*	imageMap;
    IMAGEDESC	thisImage;
};

#endif
