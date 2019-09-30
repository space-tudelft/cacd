/*
 * ISC License
 *
 * Copyright (C) 2004-2018 by
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

#include "src/space/makesubres/define.h"
#include "src/space/makesubres/extern.h"

static DM_STREAM *Stream;

edge_t * openInput (DM_CELL *cellKey)
{
    Stream = dmOpenStream (cellKey, "cont_bln", "r");
    if (!Stream) say ("cont_bln open error"), die ();

    initEdgeStatistics ();
    initTileStatistics ();

    return fetchEdge ();
}

void closeInput ()
{
    dmCloseStream (Stream, COMPLETE);
}

edge_t * fetchEdge ()
{
    edge_t *edge;
    mask_t color;
    int k = dmGetDesignData (Stream, GEO_BOXLAY);
    if (k > 0) {
	if (gboxlay.chk_type & 0x100) /* interior edge */
	    color = cNull;
	else {
	    k = gboxlay.chk_type & 0xff; /* conductor_nr */
	    ASSERT (k >= 0 && k < 64);
	    COLORINITINDEX (color, k);
	    ASSERT (IS_COLOR (&color));
	}
	NEW_EDGE (edge, (coor_t) gboxlay.xl, (coor_t) gboxlay.yb,
			(coor_t) gboxlay.xr, (coor_t) gboxlay.yt, color);
	edge -> cc = gboxlay.chk_type;
    }
    else {
	if (k < 0) say ("cont_bln read error"), die ();
	NEW_EDGE (edge, INF, INF, INF, INF, cNull); /* EOF */
    }
    return edge;
}
