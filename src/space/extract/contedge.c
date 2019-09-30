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
#include <stdlib.h>
#include <string.h>
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/export.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"

typedef struct cEdge {
    coor_t xl, yl, xr, yr;
    int cc, di, top;
    struct cEdge *next;
} cEdge_t;

extern tileBoundary_t *bdr;

static cEdge_t *eListBegin;
static cEdge_t *eListEnd;
static int nrOfEdges;

static DM_STREAM *stream_cont;

void initContEdge (DM_CELL *lkey)
{
    stream_cont = dmOpenStream (lkey, optSubResSave ? "cont_bln" : "cont_aln", "w");
    eListBegin = NULL;
    nrOfEdges = 0;
}

Private int compareEdge (const void *c1, const void *c2)
{
    coor_t dx1, dx2, dy1, dy2;
    cEdge_t ** e1 = (cEdge_t **) c1;
    cEdge_t ** e2 = (cEdge_t **) c2;
    if ((*e2) -> xl > (*e1) -> xl) return (-1);
    if ((*e2) -> xl < (*e1) -> xl) return ( 1);
    if ((*e2) -> yl > (*e1) -> yl) return (-1);
    if ((*e2) -> yl < (*e1) -> yl) return ( 1);
    // compare also the slopes
    dx1 = (*e1) -> xr - (*e1) -> xl;
    dy1 = (*e1) -> yr - (*e1) -> yl;
    dx2 = (*e2) -> xr - (*e2) -> xl;
    dy2 = (*e2) -> yr - (*e2) -> yl;
    if (dy2 * dx1 > dy1 * dx2) return (-1);
    if (dy2 * dx1 < dy1 * dx2) return ( 1);
    return (0);
}

void endContEdge ()
{
    cEdge_t *e, **eArray;
    int i;

    if (eListBegin) { /* optSubResSave */
	eArray = NEW (cEdge_t *, nrOfEdges);
	e = eListBegin;
	for (i = 0; i < nrOfEdges; i++) {
	    eArray[i] = e;
	    e = e -> next;
	}
	qsort ((char *)eArray, nrOfEdges, sizeof (cEdge_t *), compareEdge);

	for (i = 0; i < nrOfEdges; i++) {
	    e = eArray[i];
	    gboxlay.xl = e -> xl;
	    gboxlay.yb = e -> yl;
	    gboxlay.xr = e -> xr;
	    gboxlay.yt = e -> yr;
	    gboxlay.chk_type = e -> cc + (e -> di << 8);
	    if (e -> top) gboxlay.chk_type += 0x400;
	    dmPutDesignData (stream_cont, GEO_BOXLAY);
	    DISPOSE (e, sizeof(cEdge_t));
	}
	DISPOSE (eArray, sizeof(cEdge_t *) * nrOfEdges);
    }
    dmCloseStream (stream_cont, COMPLETE);
}

void contTileEdge (int top, coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
    coor_t dx1, dx2, dy1, dy2;
    cEdge_t *e, *p = 0, *e1 = 0;

    for (e = eListBegin; e; e = p -> next) {
	if (e -> top == top && e -> xl == xl && e -> yl == yl) {
	    if (e -> xr == xr && e -> yr == yr) {
		if (e1) {
		    if (e1 -> di == e -> di) {
			e1 -> xr = xr;
			e1 -> yr = yr;
			if (eListEnd == e) eListEnd = p;
			p -> next = e -> next;
			DISPOSE (e, sizeof(cEdge_t)); nrOfEdges--;
		    }
		    else if (e1 -> di < e -> di) {
			e -> xl = e1 -> xl;
			e -> yl = e1 -> yl;
		    }
		    else {
			e1 -> xr = xr;
			e1 -> yr = yr;
			if (!top) e -> di += 8; // soft begin
		    }
		}
		return;
	    }
	    dx1 = e -> xr - e -> xl;
	    dx2 = xr - xl;
	    dy1 = e -> yr - e -> yl;
	    dy2 = yr - yl;
	    if (dx1 * dy2 == dx2 * dy1) {
		xl = e -> xr;
		yl = e -> yr;
		if (e1) {
		    if (e1 -> di == e -> di) {
			e1 -> xr = xl;
			e1 -> yr = yl;
			p -> next = e -> next;
			DISPOSE (e, sizeof(cEdge_t)); nrOfEdges--;
		    }
		    else if (e1 -> di < e -> di) {
			e -> xl = e1 -> xl;
			e -> yl = e1 -> yl;
			e1 = e;
		    }
		    else {
			e1 -> xr = xl;
			e1 -> yr = yl;
			if (!top) e -> di += 8; // soft begin
		    }
		}
		else e1 = p = e;
	    }
	    else p = e;
	}
	else p = e;
    }
    ASSERT (0);
}

void contTile (tile_t *tile)
{
    if (tile -> subcont -> distributed) {
	contTileEdge (0, tile->xl, tile->bl, tile->xr, tile->br);
	contTileEdge (1, tile->xl, tile->tl, tile->xr, tile->tr);
    }
}

void contEdge (tile_t *tile, tile_t *newerTile)
{
    if (optSubResSave) {
	coor_t dx1, dx2, dy1, dy2, xl, yl;
	cEdge_t *e;
	int cc, di;

	if (tile -> subcont) {
	    cc = tile -> subcont -> causing_con;
	    di = (newerTile -> subcont && newerTile -> subcont -> causing_con == cc);
	    if (cc < 0) cc = nrOfCondStd;

	    if (tile -> subcont -> distributed) di |= 2;

	    xl = bdr -> x1;
	    yl = bdr -> y1;

	    if (di < 2) {
		for (e = eListBegin; e; e = e -> next) {
		    if (e -> top && e -> xr == bdr -> x1 && e -> yr == bdr -> y1) {
			dx1 = e -> xr - e -> xl;
			dx2 = bdr -> x2 - bdr -> x1;
			dy1 = e -> yr - e -> yl;
			dy2 = bdr -> y2 - bdr -> y1;
			if (dx1 * dy2 == dx2 * dy1) { /* same angle */
			    if (e -> di > 1 || e -> cc != cc) break;
			    if (e -> di >= di) {
				e -> xr = bdr -> x2;
				e -> yr = bdr -> y2;
				if (e -> di == di) goto ret;
			    }
			    else {
				xl = e -> xl;
				yl = e -> yl;
			    }
			    break;
			}
		    }
		}
	    }
	    e = NEW (cEdge_t, 1);
	    e -> xl = xl;
	    e -> yl = yl;
	    e -> xr = bdr -> x2;
	    e -> yr = bdr -> y2;
	    e -> cc = cc;
	    e -> di = di;
	    e -> top = 1;
	    e -> next = NULL;
	    if (eListBegin) eListEnd -> next = e;
	    else eListBegin = e;
	    eListEnd = e;
	    nrOfEdges++;
	}
ret:
	if (newerTile -> subcont) {
	    cc = newerTile -> subcont -> causing_con;
	    di = (tile -> subcont && tile -> subcont -> causing_con == cc);
	    if (cc < 0) cc = nrOfCondStd;

	    if (newerTile -> subcont -> distributed) di |= 2;

	    xl = bdr -> x1;
	    yl = bdr -> y1;

	    if (di < 2) {
		for (e = eListBegin; e; e = e -> next) {
		    if (e -> top == 0 && e -> xr == bdr -> x1 && e -> yr == bdr -> y1) {
			dx1 = e -> xr - e -> xl;
			dx2 = bdr -> x2 - bdr -> x1;
			dy1 = e -> yr - e -> yl;
			dy2 = bdr -> y2 - bdr -> y1;
			if (dx1 * dy2 == dx2 * dy1) { /* same angle */
			    if (e -> di > 1 || e -> cc != cc) break;
			    if (e -> di >= di) {
				e -> xr = bdr -> x2;
				e -> yr = bdr -> y2;
				if (e -> di == di) return;
				di += 8; // soft begin
			    }
			    else {
				xl = e -> xl;
				yl = e -> yl;
			    }
			    break;
			}
		    }
		}
	    }
	    e = NEW (cEdge_t, 1);
	    e -> xl = xl;
	    e -> yl = yl;
	    e -> xr = bdr -> x2;
	    e -> yr = bdr -> y2;
	    e -> cc = cc;
	    e -> di = di;
	    e -> top = 0;
	    e -> next = NULL;
	    if (eListBegin) eListEnd -> next = e;
	    else eListBegin = e;
	    eListEnd = e;
	    nrOfEdges++;
	}
	return;
    }

    if (bdr -> y1 != bdr -> y2) {
	char buf[128];
	strcpy (buf, strCoorBrackets (bdr -> x1, bdr -> y1));
	say ("sorry, current version cannot handle non-othogonal boundaries for substrate contacts");
	say ("non-orthogonal edge detected that runs from %s to %s", buf,
	     strCoorBrackets (bdr -> x2, bdr -> y2));
	die ();
    }
    ggln.xl = bdr -> x1;
    ggln.yl = bdr -> y1;
    ggln.xr = bdr -> x2;
    ggln.yr = bdr -> y2;
    dmPutDesignData (stream_cont, GEO_GLN);
}
