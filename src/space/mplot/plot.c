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

static int scale = 1;
static FILE * fpPlot;

void plotPass (int pass) {}
void plotBbox () {}

void mplotInit (DM_CELL *key)
{
    DM_STREAM *info;

    /* fpPlot = cfopen (mprintf ("%s.plot", key -> cell), "w"); */
    fpPlot = cfopen ("plotfile", "w");

    info = dmOpenStream (key, "info3", "r");
    dmGetDesignData (info, GEO_INFO3);
    dmCloseStream (info, COMPLETE);

    if (ginfo3.nr_samples != 0)
	scale = ginfo3.nr_samples;
    else
	scale = SCALE;

    bxl = Scale (ginfo3.bxl);
    bxr = Scale (ginfo3.bxr);
    byb = Scale (ginfo3.byb);
    byt = Scale (ginfo3.byt);

    fprintf (fpPlot, "%s\n", key -> cell);
    fprintf (fpPlot, "min %ld %ld\n", bxl, byb);
    fprintf (fpPlot, "max %ld %ld\n", bxr, byt);
    if (scale > 1) fprintf (fpPlot, "*r %d\n", scale);
    fprintf (fpPlot, "*l %g\n", key -> dmproject -> lambda);
    fprintf (fpPlot, "*ol1\n"); /* enable plot option -l */
}

void mplotEnd ()
{
    fclose (fpPlot), fpPlot = NULL;
}

void plotMask (char *name, int color)
{
    static int s_printed = 0;
    fprintf (fpPlot, "lay %s %d\n", name, color);
    if (!s_printed++) fprintf (fpPlot, "s 20 28\n");
}

void plotTerminal (long xl, long xr, long yb, long yt, char *name)
{
    xl = Scale (xl);
    xr = Scale (xr);
    yb = Scale (yb);
    yt = Scale (yt);

    fprintf (fpPlot, "r%ld %ld %ld %ld\n",   xl, yb, xr - xl, yt - yb);
    fprintf (fpPlot, "m%ld %ld\nd%ld %ld\n", xl, yb, xr, yt);
    fprintf (fpPlot, "m%ld %ld\nd%ld %ld\n", xr, yb, xl, yt);
}

static void plotRect (edge_t *edge)
{
    edge_t * e;
    int x0 = edge -> xr, y0 = edge -> yr;
    int x = edge -> xl, y = edge -> link -> yl;

    _dmDoput (fpPlot, "rD D D D\n", x0, y0, x - x0, y - y0);

    e = edge -> link; DISPOSE (edge, sizeof(edge_t)); edge = e;
    while ((e = edge -> link)) {
	x = (edge -> xl == x) ? edge -> xr : edge -> xl;
	y = e -> yl;
	_dmDoput (fpPlot, "xD D\n", x - x0, y - y0);
	DISPOSE (edge, sizeof(edge_t)); edge = e;
    }
    DISPOSE (edge, sizeof(edge_t));
}

static void plotPolygon (edge_t *edge)
{
    edge_t * e;
    int x0 = edge -> xr, y0 = edge -> yr;
    int x  = edge -> xl, y  = edge -> yl;

    _dmDoput (fpPlot, "pD D\nxD D\n", x0, y0, x - x0, y - y0);

    e = edge -> link; DISPOSE (edge, sizeof(edge_t)); edge = e;
    while (edge) {
        if (edge -> xl == x) {
	    if (edge -> yl != y) {	/* vert */
		x = edge -> xl, y = edge -> yl;
		_dmDoput (fpPlot, "xD D\n", x - x0, y - y0);
	    }
	    x = edge -> xr, y = edge -> yr;
	    if (x != x0 || y != y0)	/* do not close contour */
		_dmDoput (fpPlot, "xD D\n", x - x0, y - y0);
	}
	else {
	    ASSERT (edge -> xr == x);
	    if (edge -> yr != y) {	/* vert */
		x = edge -> xr, y = edge -> yr;
		_dmDoput (fpPlot, "xD D\n", x - x0, y - y0);
	    }
	    x = edge -> xl, y = edge -> yl;
	    if (x != x0 || y != y0)	/* do not close contour */
		_dmDoput (fpPlot, "xD D\n", x - x0, y - y0);
	}
	e = edge -> link; DISPOSE (edge, sizeof(edge_t)); edge = e;
    }
}

void plotContour (edge_t *edge)
{
    edge_t * e;

    for (e = edge; e != NULL; e = e -> link) {
	if (e -> yl != e -> yr) {
	    plotPolygon (edge);
	    return;
	}
    }
    plotRect (edge);
}

void printLinks (edge_t *edge) /* print contour (use when in debugger) */
{
    while (edge) {
        printf ("%d %d %d %d\n", edge -> xl, edge -> xr, edge -> yl, edge -> yr);
	edge = edge -> link;
    }
}
