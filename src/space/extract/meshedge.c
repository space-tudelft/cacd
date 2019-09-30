/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
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

#include <math.h>
#include <stdio.h>
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"

extern tileBoundary_t *bdr;

static DM_STREAM * stream_aln;
static coor_t x_aln, y1_aln, y2_aln;
static tile_t *t1_aln, *t2_aln;

void initMeshEdge (DM_CELL *layoutKey)
{
    x_aln = INF;
    stream_aln = dmOpenStream (layoutKey, "mesh_aln", "w");
    if (optResMesh == 2) {
	DM_STREAM *str_gln = dmOpenStream (layoutKey, "mesh_gln", "r");
	while (dmGetDesignData (str_gln, GEO_GLN) > 0) dmPutDesignData (stream_aln, GEO_GLN);
	dmCloseStream (str_gln, COMPLETE);
    }
}

void endMeshEdge ()
{
    if (x_aln != INF) {
	ggln.xl = ggln.xr = x_aln;
	ggln.yl = y1_aln; ggln.yr = y2_aln;
	dmPutDesignData (stream_aln, GEO_GLN);
    }
    dmCloseStream (stream_aln, COMPLETE);
}

void putMeshEdge (coor_t xl, coor_t xr, coor_t yl, coor_t yr)
{
    ggln.xl = xl; ggln.xr = xr; ggln.yl = yl; ggln.yr = yr;
    dmPutDesignData (stream_aln, GEO_GLN);
}

void meshEdge (tile_t *tile, tile_t *newerTile, int resPresent)
{
    if (x_aln < bdr -> x2) {
	ggln.xl = ggln.xr = x_aln;
	ggln.yl = y1_aln; ggln.yr = y2_aln;
	dmPutDesignData (stream_aln, GEO_GLN);
	x_aln = INF;
    }

    if (resPresent == 3) {
	/* disturbation of at least one interconnection */

	ggln.xl = ggln.xr = bdr -> x1;
	ggln.yl = ggln.yr = bdr -> y1;
	dmPutDesignData (stream_aln, GEO_GLN);

	ggln.xl = ggln.xr = bdr -> x2;
	ggln.yl = ggln.yr = bdr -> y2;
	dmPutDesignData (stream_aln, GEO_GLN);
	return;
    }

    /* boundary of an interconnection and/or
       vertical boundary between different interconnections */

    if (bdr -> x1 == bdr -> x2) { /* vertical edge */
	if (x_aln != INF) {
	    if (x_aln == bdr -> x1 && y2_aln == bdr -> y1) {
		if (t1_aln == tile) {
		    if (!t2_aln -> known && !newerTile -> known) goto merge;
		}
		else
		if (t2_aln == newerTile) {
		    if (!t1_aln -> known && !tile -> known) goto merge;
		}
	    }
	    ggln.xl = ggln.xr = x_aln;
	    ggln.yl = y1_aln; ggln.yr = y2_aln;
	    dmPutDesignData (stream_aln, GEO_GLN);
	}
	t1_aln = tile;
	t2_aln = newerTile;
	x_aln  = bdr -> x1;
	y1_aln = bdr -> y1;
merge:
	y2_aln = bdr -> y2;
    }
    else {
	ggln.xl = bdr -> x1; ggln.xr = bdr -> x2;
	ggln.yl = bdr -> y1; ggln.yr = bdr -> y2;
	dmPutDesignData (stream_aln, GEO_GLN);
    }
}
