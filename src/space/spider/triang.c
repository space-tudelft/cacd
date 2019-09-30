/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include <stdio.h>
#include <math.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"

#include "src/space/spider/define.h"
#include "src/space/spider/recog.h"
#include "src/space/spider/extern.h"

extern strip_t *stripR;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void mkTriangle (spider_t *sp1, spider_t *sp2, spider_t *sp3, face_t *face);
#ifdef __cplusplus
  }
#endif

/* Face is a trapezoidal face.
 * This routine will break the face down intro triangles.
 * Triangles are marked as such.
 */
void triangulate (face_t *face)
{
    spider_t *tl, *tln, *br, *brn;

    Debug (meshPrintFace (face));

#define AdvanceCcw (tl = tln, tln = ccwa (tl, face))
#define AdvanceCw  (br = brn, brn =  cwa (br, face))

    /* start in first face corner */
    tl = SP(0); tln = ccwa (tl, face);
    br = SP(0); brn =  cwa (br, face);

    if (spiderDist (tln, br) < spiderDist (tl, brn)) {
	AdvanceCcw;
    }
    else {
	AdvanceCw;
    }

    while (tln != brn) {
	if (spActLinear (br, tl, tln)	       /* may happen in first corner */
	||  spActLinear (tln, brn, br)) {      /* may happen in last  corner */
	    mkTriangle (tl, brn, br, face);
	    AdvanceCw;
	}
	else if (spActLinear (brn, br, tl)     /* may happen in first corner */
	     ||  spActLinear (tl, tln, brn)) { /* may happen in last  corner */
	    mkTriangle (tln, br, tl, face);
	    AdvanceCcw;
	}
	else if ((spiderDist (tln, br) < DEVIATION_FACTOR * spiderDist (brn, tl))
		 || ((weirdoSlope (tln,br) > weirdoSlope (brn, tl)
		      && DEVIATION_FACTOR * spiderDist (tln, br) < spiderDist (brn, tl)))) {
	    mkTriangle (tln, br, tl, face);
	    AdvanceCcw;
	}
	else {
	    mkTriangle (tl, brn, br, face);
	    AdvanceCw;
	}
    }

#ifdef DEBUG
    ASSERT (tln == brn);
    ASSERT (ccwa (tln, face) == br);
    ASSERT ( cwa (tln, face) == tl);
    ASSERT (ccwa (br,  face) == tl);
    ASSERT ( cwa (tl,  face) == br);
    ASSERT (!spActLinear (tln, br, tl));
#endif /* DEBUG */

    /* The remaining triangle in the left bottom corner
     */
    meshSetCorners (face, tln, br, tl, NULL);
    faceRecur (face);
}

/* sp1, sp2 and sp3 are the corner-spiders of an unfinished triangle.
 * A new face is created for the triangle.
 * The new edge must be created from sp1 to sp2
 */
Private void mkTriangle (spider_t *sp1, spider_t *sp2, spider_t *sp3, face_t *face)
{
    face_t * newface = newFace ();
    newface -> sc_subn = face -> sc_subn;
    newface -> type = face -> type;
    meshMakeEdge (sp1, sp2, INTERNALEDGE);
    meshSetFaces (sp1, sp2, newface, face);
    meshSetFace (sp2, sp3, newface);
    meshSetFace (sp3, sp1, newface);

    meshSetCorners (newface, sp1, sp2, sp3, NULL);
    stripAddFace   (newface, stripR);

#ifdef DEBUG
    /* Well-formedness of the triangle that has been
     * split off is checked in meshSetCorners.
     * Partially check well-formedness of the remaining part here.
     */
    ASSERT ( cwa (sp1, face) == sp2);
    ASSERT (ccwa (sp2, face) == sp1);
#endif /* DEBUG */

    faceRecur (newface);
}
