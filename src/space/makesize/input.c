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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "src/libddm/dmincl.h"

#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/makesize/rscan.h"
#include "src/space/makesize/extern.h"

struct e {
    int mask_no;
    edge_t    * edge;
    DM_STREAM * stream;
};

static struct e *edges;
static int nrOfStreams;
static int nr_of_masks;

#ifdef __cplusplus
  extern "C" {
#endif
Private edge_t * doFetch (DM_STREAM *stream, int mask_no);
#ifdef __cplusplus
  }
#endif

void openInput (DM_CELL *cellKey, mask_t *colorp)
{
    DM_STREAM *stream;
    edge_t *edge;
    int i;
    mask_t cmsk;
    char *name;

    nr_of_masks = (!growPos && secondTime) ? 1 : nrOfMasks;

    /* Allocate the buffers */
    edges = NEW (struct e, nr_of_masks);

    nrOfStreams = 0;

    /* Open the streams which must be resized */
    for (i = 0; i < nr_of_masks; i++) {
	if (secondTime) {
	    COLORINITINDEX (cmsk, i);
	    masktable[i].color = cmsk;
	    name = "makesize";
	}
	else {
	    if (!masktable[i].gln) continue;
	    COLORINITINDEX (cmsk, i);
	    if (COLOR_ABSENT (colorp, &cmsk)) continue;
	    masktable[i].color = cmsk;
	    name = masktable[i].name;
	}
	stream = dmOpenStream (cellKey, mprintf ("%s_gln", name), "r");
	if (stream && (edge = doFetch (stream, i))) {
	    if (edge -> xl == -INF) {
		ASSERT (shrinkEdgeIndex == 0);
		ASSERT (edge -> yl == -INF);
		COLOR_ADD (shrinkEdge[0] -> color, edge -> color);
		rdisposeEdge (edge);
		edge = doFetch (stream, i);
		ASSERT (edge && edge -> xl == -INF && edge -> yl == INF);
		COLOR_ADD (shrinkEdge[1] -> color, edge -> color);
		rdisposeEdge (edge);
		edge = doFetch (stream, i);
	    }
	    if (edge) {
		edges[nrOfStreams].mask_no = i;
		edges[nrOfStreams].stream = stream;
		edges[nrOfStreams].edge = edge;
		++nrOfStreams;
	    }
	}
    }
}

void closeInput ()
{
    if (secondTime) {
	tile_t *tile;
	while ((tile = neggrow_list)) {
	    neggrow_list = tile -> next_tor;
	    growLayout (tile, (coor_t)0);
	    disposeTile (tile);
	}
    }
    ASSERT (!nrOfStreams);
    DISPOSE (edges, sizeof(struct e) * nr_of_masks);
}

edge_t * rfetchEdge ()
{
    edge_t *edge, *edge2;
    int i, min = 0;

    if (shrinkEdgeIndex < 2) return shrinkEdge[shrinkEdgeIndex++];

    if (!nrOfStreams) {
	NEW_EDGE (edge, INF, INF, INF, INF, cNull); /* EOF */
	return edge;
    }

    edge = edges[min].edge;
    for (i = 1; i < nrOfStreams; i++) {
	edge2 = edges[i].edge;
	if (edge2 -> xl > edge -> xl) continue;
	if (edge2 -> xl < edge -> xl) { edge = edge2; min = i; continue; }
	if (edge2 -> yl > edge -> yl) continue;
	if (edge2 -> yl < edge -> yl) { edge = edge2; min = i; continue; }
	if (compareSlope (edge2, <, edge)) { edge = edge2; min = i; }
    }

    if (!(edges[min].edge = doFetch (edges[min].stream, edges[min].mask_no))) {
	for (i = min; ++i < nrOfStreams;) {
	    edges[i-1] = edges[i];
	}
	--nrOfStreams;
    }

    Debug (rprintEdge ("rfetchEdge", edge));

    ASSERT (edge -> xl < edge -> xr
	|| (edge -> xl == INF && edge -> xr == INF));
    return (edge);
}

Private edge_t * doFetch (DM_STREAM *stream, int mask_no)
{
    edge_t *edge;
    int k = dmGetDesignData (stream, GEO_GLN);
    if (k > 0) {
	NEW_EDGE (edge, (coor_t) ggln.xl, (coor_t) ggln.yl,
			(coor_t) ggln.xr, (coor_t) ggln.yr, masktable[mask_no].color);
	return edge;
    }
    if (k < 0) say ("gln read error"), die ();
    dmCloseStream (stream, COMPLETE);
    return NULL;
}
