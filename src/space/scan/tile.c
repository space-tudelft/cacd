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
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/scan.h"
#include "src/space/scan/extern.h"

static unsigned int allocs = 0;
static unsigned int frees  = 0;
static unsigned int max    = 0;
static unsigned int size   = 0;

static tile_t * list = NULL;

#ifdef __cplusplus
  extern "C" {
#endif

Private tile_t *allocTile (void);

#ifdef __cplusplus
  }
#endif

/* Create a fresh tile.
   args:
	stb -- stitch bottom-bottom
	stl -- stitch bottom-left
	cc  -- for special prepass: init causing conductor
*/
tile_t * createTile (coor_t xl, coor_t bl, coor_t xr, coor_t br, tile_t *stb, tile_t *stl, int cc)
{
    register int i;
    tile_t *t;
#ifdef DEBUG
    /* the xr-coordinate is the right bbox coordinate of the cell.
     * An assertion xr>xl fails if xl==right bounding box,
     * but it should not fail otherwise.
     */
    static coor_t right;
    ASSERT (xr > xl || xl == right);
    right = xr;
#endif

    if (!(t = list)) t = allocTile ();
    else list = list -> next_tor;

    t -> xl = xl; t -> bl = bl; t -> tl = INF;
    t -> xr = xr; t -> br = br; t -> tr = INF;
    t -> cnt = 0;
    t -> known = cc;
    t -> tor = NULL; t -> next_tor = NULL;
    t -> rbPoints = NULL;
    t -> tlPoints = NULL;
    t -> terms = NULL;
    t -> subcont = NULL;

    for (i = 0; i < nrOfCondStd; ++i) t -> cons[i] = NULL;

#ifdef CAP3D
    if (optCap3D) {
	for (i = 0; i < nrOfSpiderLevels; ++i) {
	    t -> mesh -> spider[0][i] = NULL;
	    t -> mesh -> spider[1][i] = NULL;
	    t -> mesh -> faces[i] = NULL;
	}
	t -> mesh -> spider[0][i] = NULL;
	t -> mesh -> spider[1][i] = NULL;
    }
#endif

    ASSERT ((stl == NULL && xl == -INF) || (stl -> xr == xl));
    t -> stb = stb; if (stb) stb -> stt = t;
    t -> stl = stl;
    t -> stt = NULL;
    t -> str = NULL;
    t -> next = NULL;

    Debug (printTile ("new tile", t));

    ++allocs;
    return (t);
}

/*  Dispose a tile
*/
void disposeTile (tile_t *t)
{
    frees++;
    t -> next_tor = list;
    list = t;
}

/*  Print a tile
*/
void printTile (char *s, tile_t *t)
{
    fprintf (stderr, "%s: xl=%g bl=%g tl=%g xr=%g br=%g tr=%g\n",
	s, (double) t -> xl, (double) t -> bl,
 	   (double) t -> tl, (double) t -> xr,
	   (double) t -> br, (double) t -> tr);
}

void tileStatistics (FILE *fp)
{
    fprintf (fp, "overall tile statistics:\n");
    fprintf (fp, "\ttiles created      : %u\n", allocs);
    fprintf (fp, "\ttiles disposed     : %u\n", frees);
    fprintf (fp, "\ttiles allocated    : %u of %u byte\n", max, size);
 // fprintf (fp, "\tmax tiles in core  : %u\n", max);
}

Private tile_t * allocTile ()
/*
 * Allocate a tile and add a cons array.
 * Allignment is a tricky issue;
 * This routine works on all hardware we have tried
 * thanks to the fact that all items in the allocated structs
 * have a size that is a multiple of 4 and can be allocated
 * on a 4-byte boundary.
 * However, allignment is easily testable with the test driver
 * included in this file, compile with -DDriver.
 */
{
#ifdef CAP3D
    static int alloc_mesh;
#endif
    tile_t * tile;

    /* It is difficult to do this in
     * a type-clean manner, e.g. using unions as is done in malloc.
     * The problem is that i need to attach two pieces of memory
     * of a size *unknown at compile time*.
     *
     * These data are contiguous in memory for improved VM performance.
     */
    if (!size) {
	size = sizeof (tile_t)
		+ nrOfCondStd * sizeof (subnode_t *)
		+ nrOfCondStd * sizeof (subnode_t);
#ifdef CAP3D
	alloc_mesh = optCap3D || optCap3DSave || optSubRes;
#endif
    }
    ++max;

    tile = (tile_t *) MALLOC (size);

    tile -> cons = (subnode_t **) (tile + 1);

#ifdef CAP3D
    if (alloc_mesh) {
	tile -> mesh = NEW (meshInfo_t, 1);
	tile -> mesh -> spider[0] = NEW (spider_t *, nrOfSpiderLevels+1);
	tile -> mesh -> spider[1] = NEW (spider_t *, nrOfSpiderLevels+1);
	tile -> mesh -> faces     = NEW (face_t *, nrOfSpiderLevels);
    }
#endif

    return (tile);
}
