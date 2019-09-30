/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Nick van der Meijs
 *	Arjan van Genderen
 *	Simon de Graaf
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

#include "src/space/mplot/config.h"

extern long bxl, bxr, byb, byt;

static DM_STREAM * stream = NULL;

void openInput (DM_CELL *cellKey, char *mask)
{
    stream = dmOpenStream (cellKey, mprintf ("%s_gln", mask), "r");
}

void closeInput ()
{
    dmCloseStream (stream, COMPLETE), stream = NULL;
}

edge_t * fetch ()
{
    int k;
    edge_t * edge;

    edge = NEW (edge_t, 1);
    edge -> fwd = edge -> bwd = NULL;
    edge -> contour = edge -> link = NULL;

    k = dmGetDesignData (stream, GEO_GLN);
    if (k > 0) {
	if (ggln.xl == -INF) {
	    ASSERT (ggln.xr == INF);
	    ggln.xl = bxl;
	    ggln.xr = bxr;
	    if (ggln.yl == -INF) {
		ggln.yl = ggln.yr = byb;
	    }
	    else {
		ASSERT (ggln.yl == INF);
		ggln.yl = ggln.yr = byt;
	    }
	}
	edge -> xl = ggln.xl, edge -> xr = ggln.xr;
	edge -> yl = ggln.yl, edge -> yr = ggln.yr;
    }
    else if (k == 0) { /* EOF */
	edge -> xl =  INF, edge -> xr = INF;
	edge -> yl =  INF, edge -> yr = INF;
    }
    else say ("read error"), die ();

    return (edge);
}

void printEdge (char *s, edge_t *edge)
{
    fprintf (stderr, "%s: xl=%d yl=%d xr=%d yr=%d\n",
	s, edge -> xl, edge -> yl, edge -> xr, edge -> yr);
}
