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

#include "src/space/makegln/config.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "src/libddm/dmincl.h"
#include "src/space/auxil/auxil.h"
#include "src/space/makegln/makegln.h"
#include "src/space/makegln/proto.h"

static int inputScale;

#define Scale(x)      ((coor_t) (inputScale * (x)))
#define RoundScale(x) ((coor_t) ((x) > 0 ? (inputScale * (x) + 0.5) : (inputScale * (x) - 0.5)))

/* local operations */
Private void readBxx (DM_CELL *cellKey, char *mask, int term);
Private void readNxx (DM_CELL *cellKey, char *mask);
Private void polyEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr, slope_t slope, sign_t sign);
Private void polySort (void);

void readEdges (DM_CELL *cellKey, char *mask, int masktype, int scaleFactor)
{
    inputScale = scaleFactor;

    readBxx (cellKey, mask, 0);
    readNxx (cellKey, mask);
    if (masktype == DM_INTCON_MASK) readBxx (cellKey, mask, 1);
}

Private void readBxx (DM_CELL *cellKey, char *mask, int term)
{
    coor_t xl, yb, xr, yt;
    DM_STREAM *stream;

    if (term)
	stream = dmOpenStream (cellKey, mprintf ("t_%s_bxx", mask), "r");
    else
	stream = dmOpenStream (cellKey, mprintf ("%s_bxx", mask), "r");

    while (dmGetDesignData (stream, GEO_BOXLAY) > 0) {
	if (gboxlay.xl == gboxlay.xr || gboxlay.yb == gboxlay.yt) continue; /* it may be a point terminal */
	if (gboxlay.chk_type != 0 && !term) continue; /* for space extract!!! */

	xl = Scale (gboxlay.xl);
	yb = Scale (gboxlay.yb);
	xr = Scale (gboxlay.xr);
	yt = Scale (gboxlay.yt);
	sortEdge (xl, yb, xr, yb, 0, START);
	sortEdge (xl, yt, xr, yt, 0, STOP);
    }

    dmCloseStream (stream, COMPLETE);
    if (optDelete && !term) dmUnlink (cellKey, mprintf ("%s_bxx", mask));
}

static _edge_t *eBuf = NULL;
static ecnt_t   eCnt = 0;

Private void readNxx (DM_CELL *cellKey, char *mask)
{
    DM_STREAM  *stream;
    char       *el;
    long        noXy;
    slope_t     slope = 0; /* init, else compiler warning */
    coor_t      x0, y0, x1, y1, x2, y2;
    struct      stat st_buf;
    int   skip, rounded = 0;

    /* return immediately if nxx file does not exist */
    if (dmStat (cellKey, mprintf ("%s_nxx", mask), &st_buf) == -1) return;

    stream = dmOpenStream (cellKey, mprintf ("%s_nxx", mask), "r");

    while (dmGetDesignData (stream, GEO_NXX_INI) > 0) {
	noXy = gnxx_ini.no_xy;

	switch (gnxx_ini.elmt) {
	    case POLY_NOR:
	    case RECT_NOR: break; /* these are OK */
	    default:
		/* skip unsupported element types */
		if (gnxx_ini.elmt == SBOX_NOR) el = "SBOX_NOR";
		else if (gnxx_ini.elmt == WIRE_NOR) el = "WIRE_NOR";
		else if (gnxx_ini.elmt == CIRCLE_NOR) el = "CIRCLE_NOR";
		else el = "UNKNOWN_NOR";

		while (--noXy >= 0L) dmGetDesignData (stream, GEO_NXX_XY);
		say ("Warning: cell %s, mask %s: element %s skipped!", cellKey -> cell, mask, el);
		continue;
	}

	dmGetDesignData (stream, GEO_NXX_XY);
	x0 = x1 = RoundScale (gnxx_xy.x);
	y0 = y1 = RoundScale (gnxx_xy.y);

	if (!rounded && ((Scale (gnxx_xy.x) != x1) || (Scale (gnxx_xy.y) != y1))) {
	    say ("Warning: cell %s, mask %s: feature(s) not on grid, rounded", cellKey -> cell, mask);
	    rounded = 1;
	}

	skip = 0;

	while (noXy-- > 0L) {
	    if (noXy > 0L) {
		dmGetDesignData (stream, GEO_NXX_XY);
		x2 = RoundScale (gnxx_xy.x);
		y2 = RoundScale (gnxx_xy.y);
		if (!rounded && ((Scale (gnxx_xy.x) != x2) || (Scale (gnxx_xy.y) != y2))) {
		    say ("Warning: cell %s, mask %s: feature(s) not on grid, rounded", cellKey -> cell, mask);
		    rounded = 1;
		}
	    }
	    else {
		x2 = x0;
		y2 = y0;
	    }

	    if (x2 == x1) { y1 = y2; continue; } /* vertical edge, skip it */
	    if (y2 == y1)
		slope = 0;
	    else if (y2 - y1 == x2 - x1)
		slope = 1;
	    else if (y2 - y1 == x1 - x2)
		slope = -1;
	    else if (!skip) { skip = 1;
		say ("Warning: cell %s, mask %s: non-45 degree feature(s), skipped", cellKey -> cell, mask);
	    }

	    if (x2 > x1) polyEdge (x1, y1, x2, y2, slope, START);
	    else	 polyEdge (x2, y2, x1, y1, slope, STOP);

	    x1 = x2;
	    y1 = y2;
	}

	if (!skip) polySort ();
	eCnt = 0;
    }

    dmCloseStream (stream, COMPLETE);
    if (optDelete) dmUnlink (cellKey, mprintf ("%s_nxx", mask));
}

Private void polyEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr, slope_t slope, sign_t sign)
{
    static ecnt_t eSize = 0;
    _edge_t * e;

    if (eCnt >= eSize) {
	unsigned to = eSize == 0 ? 100 : 2 * eSize;
	eBuf = GROW (_edge_t, eBuf, eSize, to);
	eSize = to;
    }
    e = eBuf + eCnt++;
    e -> xl = xl; e -> xr = xr;
    e -> yl = yl; e -> yr = yr;
    e -> slope = slope;
    e -> sign = sign;
}

Private void polySort ()
{
    ecnt_t i;
    _edge_t * min = eBuf;

    for (i = 1; i < eCnt; i++) {
	if (smaller (eBuf + i, min)) min = eBuf + i;
    }

    if (min -> sign == STOP) {
	for (i = 0; i < eCnt; i++) {
	    if (eBuf[i].sign == STOP)
		eBuf[i].sign  = START;
	    else
		eBuf[i].sign  = STOP;
	}
    }

    ASSERT (min -> sign == START);

    for (i = 0; i < eCnt; i++) {
	sortEdge (eBuf[i].xl, eBuf[i].yl, eBuf[i].xr, eBuf[i].yr, eBuf[i].slope, eBuf[i].sign);
    }
}
