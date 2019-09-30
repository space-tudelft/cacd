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
#include <time.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/scan.h"
#include "src/space/scan/extern.h"
#include "src/space/extract/export.h"
#ifdef CAP3D
#include "src/space/spider/export.h"
#endif
#ifdef DISPLAY
#include "src/space/X11/export.h"
#endif

#ifdef __cplusplus
  extern "C" {
#endif

/* local operations */
Private void bundle (edge_t *newEdge, edge_t *edge);
Private int  unbundle (edge_t *edge);
#ifdef DEBUG
Private void checkSR (edge_t *head, coor_t x);
#endif

#ifdef __cplusplus
  }
#endif

#define smallerAtX(e1,e2) (e2 -> yl < thisY || (e2 -> yl == thisY && e1 -> xr > thisX && compareSlope (e2, <, e1)))
/*
 * Comment added NvdM, 891213
 * the comparison with e1 -> xr is (probably!!!) necessary because
 * the ordering in the stateruler can become wrong
 * in cases where an edge under +45 straddles the scanline
 * in say, (x,y) and an edge under -45 ends in (x,y).
 * This can, however give other problems because horizontal
 * edges ending and starting at (x,y) will not be bundled.
 */
#define equalAtX(e1,e2) (e2 -> xl == thisX && e2 -> yl == thisY && compareSlope (e1, ==, e2) && e1 -> xr > thisX)

#define TestIntersection(e1,e2) if(compareSlope (e1, !=, e2)) testIntersection (e1, e2)

extern int optAlarmInterval;
extern int scan_srl;
extern int scan_xpos;

static char cell[100];
static coor_t c_xl, c_xr;
static int flag = 0;
static time_t oldtime, newtime, begtime;

coor_t thisX = INF;
coor_t thisY = INF;
mask_t thisColor;

void extraSay ()
{
    static int say_coor = 0;

    if (say_coor == 0) say_coor = getenv ("SAY_COORDINATES") ? 1 : -1;

    if (say_coor > 0 && thisX > -INF && thisX < INF && thisY > -INF && thisY < INF) {
	say ("now at position %s", strCoorBrackets (thisX, thisY));
    }
}

void setContext (char *c, coor_t l, coor_t r)
{
    int i;

    c_xl = l; c_xr = r;

    for (i = 0; i < 99 && *c; ++i) cell[i] = *c++;
    cell[i] = 0;
}

void catchAlarm ()
{
    coor_t here;
    int perc;

    if (flag) {
	newtime = time (NULL);
	if (optAlarmInterval < 0) {
	    if (newtime < oldtime - optAlarmInterval) return;
	    oldtime = newtime;
	}
	here = thisX;
	if (here < c_xl) here = c_xl;
	if (here > c_xr) here = c_xr;
	perc = (int)((100 * (double)(here - c_xl))/(c_xr - c_xl));
	message ("progress: %s, x = %s (%d%%) %3d sec.\n", cell, strCoor (here), perc, newtime - begtime);
    }
}

void scan ()
{
    edge_t *edge, *newEdge, *head, *tail, *ebwd;
    terminal_t *newTerm;
    coor_t nextX, termX, termY;
    int termSplit;
#ifdef CAP3D
    int stripSplit;
    coor_t nextStrip, nextStrip1, nextStrip2;
#endif
#ifdef DEBUG
    static int no_split = -1;
#ifdef DEBUG_STRIP
    coor_t debug_min = 0, debug_max = 0;
    char *s;
    if ((s = paramLookupS ("debug_strip", "")) && *s) {
	sscanf (s, "%d %d", &debug_min, &debug_max);
	say ("debugging between %d and %d", debug_min, debug_max);
    }
#endif
    if (no_split == -1) no_split = paramLookupB ("no_split", "off") ? 1 : 0;
#endif

    begtime = time (NULL);
    oldtime = 0;

    /* init head and tail of stateruler */
    NEW_EDGE (head, -INF, -INF, INF, -INF, cNull);
    NEW_EDGE (tail, -INF,  INF, INF,  INF, cNull);

    head -> fwd = head -> bwd = tail;
    tail -> fwd = tail -> bwd = head;

    NEW_TILE (head -> tile, -INF, -INF, INF, -INF, cNull, NULL, NULL, 0);
    NEW_TILE (tail -> tile, -INF,  INF, INF,  INF, cNull, NULL, NULL, 0);
    tail -> tile -> tl = tail -> tile -> tr = -INF;

    newEdge = fetchEdge ();
    newTerm = fetchTerm ();
    termX = newTerm -> x;
    termY = newTerm -> y;
#ifdef DISPLAY
    if (goptDrawEdge) drawEdge (&newEdge -> color, newEdge -> xl, newEdge -> yl, newEdge -> xr, newEdge -> yr);
#endif

    thisX = newEdge -> xl;
    if (thisX > termX) thisX = termX;
    if (thisX == INF) goto ret;

    flag = 1; /* enable catchAlarm */

#ifdef CAP3D
    /*  Strip positions are used for 3D substrate resistance extraction.
	This scanline positions must be used during all subsequent passes.
	Strip positions are also used for 3D capacitance extraction in
	the extract pass. These positions may be different to the 3D
	substrate positions and must also be taken into account during
	the 3D substrate pass. This is important for consequent substrate
	contact splitting.  The selective resistance extraction pass must
	also know all scanline positions for correct tile numbering.
    */
    if (optCap3D) { /* prePass1 || extrPass */
	nextStrip1 = cap3dStart ();
	while (nextStrip1 <= thisX) nextStrip1 = cap3dStrip ();
	if (prePass1 && optCap3DSave)
	     nextStrip2 = findStripForCapStart (thisX);
	else if (extrPass && optSubRes)
	     nextStrip2 = findStripForSubStart (thisX);
	else nextStrip2 = INF;
	nextStrip = nextStrip1 < nextStrip2 ? nextStrip1 : nextStrip2;
    }
    else if (prePass != 1) {
	if (optSubRes)
	     nextStrip1 = findStripForSubStart (thisX);
	else nextStrip1 = INF;
	if (optCap3DSave)
	     nextStrip2 = findStripForCapStart (thisX);
	else nextStrip2 = INF;
	nextStrip = nextStrip1 < nextStrip2 ? nextStrip1 : nextStrip2;
    }
    else nextStrip = nextStrip1 = nextStrip2 = INF;

    Debug (fprintf (stderr, "nextStrip=%d\n", nextStrip));
    ASSERT (nextStrip > thisX);
    stripSplit = 0;
#endif

    while (thisX < INF) {
	nextX = INF;

#ifdef CAP3D
	if (thisX >= nextStrip) { thisX = nextStrip; stripSplit = 1; }
#endif

	Debug (fprintf (stderr, "thisX=%d\n", thisX));

	if (optAlarmInterval < 0) catchAlarm ();

#ifdef DEBUG_STRIP
	if (debug_min != debug_max) {
	    if (thisX < outScale * debug_min || thisX > outScale * debug_max)
		_IfDebug = FALSE;
	    else
		_IfDebug = TRUE;
	}
#endif

	scan_xpos++;

#ifndef CONFIG_SPACE2
	if (doTileXY) {
	    if (peekTileXY () <= thisX) {
		/* Walk along the tiles the see if one (or more) of them
		   contain an XY point.
		*/
		edge = head -> fwd;
		while (edge != tail && edge -> tile -> tl < INF) {
		    testTileXY (edge -> tile, edge -> fwd -> tile, thisX);
		    edge = edge -> fwd;
		}
	    }
	}
#endif

	edge = head -> fwd;

	while (edge != tail || newEdge -> xl == thisX || termX == thisX) {

	    thisY = Y (edge, thisX);
#ifdef DISPLAY
	    if (goptDrawPosition) drawScanPosition (thisX, thisY);
#endif

	    /* The following tests for a special case:
	       An intersection between edges before and after an edge
	       that ends at thisX (see also split ()) (AvG)
	    */
	    if (edge -> xl < thisX && edge -> xr > thisX
		&& edge -> fwd -> xr == thisX
		&& edge -> fwd -> yr == thisY
		&& Y (edge -> fwd -> fwd, thisX) == thisY) {
		TestIntersection (edge, edge -> fwd -> fwd);
	    }

	    if (edge -> xi == thisX) split (edge);

	    if (edge -> xc == thisX && edge -> bundle) termSplit = unbundle (edge);
	    else termSplit = 0;

	    if (newEdge -> xl == thisX) {
		if (smallerAtX (edge, newEdge)) {
		    /* insert newEdge below edge */
		    Debug (printEdge ("insert", newEdge));
		    Debug (printEdge ("below", edge));
		    newEdge -> bwd = edge -> bwd;
		    newEdge -> fwd = edge;
		    edge -> bwd -> fwd = newEdge;
		    edge -> bwd = newEdge;

		    edge = newEdge; newEdge = fetchEdge ();
		    thisY = edge -> yl;
#ifdef DISPLAY
		    if (goptDrawEdge) drawEdge (&newEdge -> color, newEdge -> xl, newEdge -> yl, newEdge -> xr, newEdge -> yr);
		    if (goptDrawPosition) drawScanPosition (thisX, thisY);
#endif
		}
		if (equalAtX (edge, newEdge)) {
		    coor_t old_xr = edge -> xr;
		    do {
#ifdef CAP3D
			if (prePass1 == 2 && !(newEdge -> cc & 0xc00)) termSplit = 1;
#endif
			bundle (newEdge, edge);
			newEdge = fetchEdge ();
#ifdef DISPLAY
			if (goptDrawEdge) drawEdge (&newEdge -> color, newEdge -> xl, newEdge -> yl, newEdge -> xr, newEdge -> yr);
#endif
		    } while (equalAtX (edge, newEdge));
		    if (edge -> xr > old_xr) {
			if (edge -> tile) {
			    edge -> tile -> xr = edge -> xr;
			    edge -> tile -> br = edge -> yr;
			}
			TestIntersection (edge, edge -> fwd);
			TestIntersection (edge, edge -> bwd);
			if (edge -> xi == thisX && edge -> fwd -> xi == thisX) split (edge);
		    }
		}
	    }

	    if (termX == thisX && termY <= thisY) {
		/* The following will split the tile 'edge -> bwd -> tile'
		   when necessary.  It will also perform a down slicing
		   when appropriate.  Connecting terminals to tiles is NOT
		   done here.
		*/
		tileAddTerm (edge -> bwd, termY);

		/* We split a tile immediately above a terminal because
		   otherwise the (point) terminal may not be on a left
		   or right boundary of a tile.
		   A tile immediately below a terminal is also split.
		*/
		do {
		    if (termY == thisY) termSplit = 1;
		    newTerm = fetchTerm ();
		    termX = newTerm -> x;
		    termY = newTerm -> y;
		} while (termX == thisX && termY <= thisY);
	    }

	    scan_srl++;

	    if (edge -> xr == thisX) {
		tileDeleteEdge (edge);

		/* delete edge from scanline */
		Debug (printEdge ("delete", edge));
		ebwd = edge -> bwd;
		ebwd -> fwd = edge -> fwd;
		edge -> fwd -> bwd = ebwd;
		disposeEdge (edge);
		edge = ebwd -> fwd;

		TestIntersection (edge, ebwd);
	    }
            else if (edge != tail) {
		COLOR_XOR (thisColor, edge -> color);
		if (edge -> xl == thisX) {
		    TestIntersection (edge, edge -> bwd);
		    tileInsertEdge (edge);
		}
		else {
		    if (edge -> bwd -> xl == thisX) TestIntersection (edge, edge -> bwd);
#ifdef CAP3D
#ifdef DEBUG
                    if (no_split) tileCrossEdge (edge, termSplit);
		    else
#endif
		    tileCrossEdge (edge, (stripSplit || termSplit));
#else /* not CAP3D */
                    tileCrossEdge (edge, termSplit);
#endif /* not CAP3D */
		}

		if (edge -> xi < nextX) nextX = edge -> xi;
		if (edge -> xc < nextX) nextX = edge -> xc;

		edge = edge -> fwd;
	    }
	}

	/* important test: thisColor must be zero */
	if (IS_COLOR (&thisColor)) ASSERT (!IS_COLOR (&thisColor));

	ASSERT (nextX > thisX && newEdge -> xl > thisX && termX > thisX);

	tileAdvanceScan (edge);

	Debug2 (checkSR (head, thisX));

	if (nextX > termX) nextX = termX;
	if (nextX > newEdge -> xl) nextX = newEdge -> xl;

#ifdef CAP3D
	if (stripSplit) { /* thisX == nextStrip */
	    stripSplit = 0;
	    if (nextStrip1 == thisX) {
		if (optCap3D) nextStrip1 = cap3dStrip ();
		else nextStrip1 = findStripForSubNext (thisX);
	    }
	    if (nextStrip2 == thisX) {
		if (extrPass)
		     nextStrip2 = findStripForSubNext (thisX);
		else nextStrip2 = findStripForCapNext (thisX);
	    }
	    nextStrip = nextStrip1 < nextStrip2 ? nextStrip1 : nextStrip2;
	    Debug (fprintf (stderr, "nextStrip=%d\n", nextStrip));
	    ASSERT (nextStrip > thisX);
	}
#endif
	ASSERT (nextX > thisX);
	thisX = nextX;
    }
    thisY = INF;

    if (optAlarmInterval < 0) catchAlarm ();

#ifndef CONFIG_SPACE2
    if (doTileXY) advanceTileXY (thisX);
#endif

#ifdef CAP3D
    if (optCap3D) {
	while (nextStrip != INF) nextStrip = cap3dStrip ();
	cap3dStop ();
    }
#endif
    flag = 0; /* disable catchAlarm */
ret:
    tileStopScan (head);
    disposeEdge (newEdge); /* EOF edge */

    /* Remove head and tail of stateruler.
     * These can not be retained, since the tiles
     * have a cons array depending on resistance extraction.
     * With display mode, resistance extraction can be turned
     * on or off.
     */
    disposeEdge (tail);
    disposeEdge (head);
}

Private void bundle (register edge_t *newEdge, register edge_t *edge) /* insert overlapping edge */
{
    register edge_t *e;
#ifdef DEBUG
    edge_t *e1 = edge;
#endif
    Debug (printEdge ("bundle", newEdge));
    Debug (printEdge ("with",  edge));

 // ASSERT (compareSlope (edge, ==, newEdge)); /* see: equalAtX */
 // ASSERT (edge -> xr > newEdge -> xl); /* see: equalAtX */
 // ASSERT (newEdge -> xc == newEdge -> xr); /* see: createEdge */

    if (edge -> xr < newEdge -> xr) {
	edge -> xr = newEdge -> xr;
	edge -> yr = newEdge -> yr;
    }

    if (edge -> xc <= newEdge -> xr) {
	COLOR_ADD (edge -> color, newEdge -> color);
	if (!(newEdge -> cc & 0x400)) edge -> cc = newEdge -> cc;
	while ((e = edge -> bundle) && e -> xc <= newEdge -> xr) {
	    edge = e;
	    COLOR_ADD (edge -> color, newEdge -> color);
	    if (!(newEdge -> cc & 0x400)) edge -> cc = newEdge -> cc;
	}

	if (edge -> xc == newEdge -> xr) disposeEdge (newEdge);
	else { /* insert newEdge after edge */
	    // ASSERT (edge -> xc < newEdge -> xr);
	    if (e) { /* not last: e -> xc > newEdge -> xr */
		COLOR_ADD (newEdge -> color, e -> color);
		newEdge -> bundle = e;
	    }
	    edge -> bundle = newEdge;
	}
    }
    else { /* edge -> xc > newEdge -> xr */
	mask_t m = newEdge -> color;
	newEdge -> xc = edge -> xc;
	edge -> xc = newEdge -> xr;
	if (!(newEdge -> cc & 0x400)) edge -> cc = newEdge -> cc;
	newEdge -> color = edge -> color;
	COLOR_ADD (edge -> color, m);
	newEdge -> bundle = edge -> bundle;
	edge -> bundle = newEdge;
    }

#ifdef DEBUG
    for (e = e1; e -> bundle; e = e -> bundle) { ASSERT (e -> xc < e -> bundle -> xc); }
    ASSERT (e -> xc == edge -> xr);
#endif
}

Private int unbundle (edge_t *edge) /* delete overlapping edge */
{
    int split;
    edge_t * b = edge -> bundle;

    Debug (printEdge ("unbundle", edge));

    if (COLOR_EQ_COLOR (&b -> color, &edge -> color)) split = 1;
    else { edge -> color = b -> color; split = 0; }
    edge -> xc     = b -> xc;
    edge -> bundle = b -> bundle;
    disposeEdge (b);
    return split;
}

#ifdef DEBUG
Private void checkSR (edge_t *head, coor_t x)
{
    edge_t * e;
    for (e = head -> fwd; e != head -> bwd; e = e -> fwd) {
	if (DEBUG) printEdge ("edge", e);
	ASSERT (e -> xr > x);
	ASSERT (e -> xl <= x);
	ASSERT (Y (e -> fwd, x) > Y (e, x) ||
	   (Y (e -> fwd, x) == Y (e, x) && compareSlope (e -> fwd, >, e)));
    }
}
#endif /* DEBUG */
