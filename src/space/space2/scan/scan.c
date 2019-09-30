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
static int clen = 2;
static time_t oldtime, newtime, begtime;

int progress_mem = -1;
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

    if (optAlarmInterval) { /* calculate max length of coord */
	if (l < 0 && -l > r) r = -l;
	sprintf (cell, "%s", strCoor(r));
	clen = strlen (cell) + 1;
    }

    for (i = 0; i < 99 && *c; ++i) cell[i] = *c++;
    cell[i] = 0;
}

void catchAlarm ()
{
    char buf[8], *p, *c;
    coor_t here;
    int perc, sec;

    if (flag) {
	newtime = time (NULL);
	if (flag == 1) { ++flag;
	    if (progress_mem < 0) progress_mem = paramLookupB ("progress_mem", "off");
	}
	else if (optAlarmInterval < 0) {
	    if (newtime < oldtime - optAlarmInterval) return;
	    oldtime = newtime;
	}
	here = thisX;
	if (here < c_xl) here = c_xl;
	if (here > c_xr) here = c_xr;
	c = strCoor (here);
	perc = (int)((100 * (double)(here - c_xl))/(c_xr - c_xl));
	p = buf; sprintf (p, "(%2d%%)", perc);
	if (perc > 99) ++p;
	sec = newtime - begtime;
	if (progress_mem > 0)
	message ("progress: %s, x =%*s %s %4d sec. %8.3f Mbyte\n", cell, clen, c, p, sec, allocatedMbyte ());
	else
	message ("progress: %s, x =%*s %s %4d sec.\n", cell, clen, c, p, sec);
    }
}

void scan ()
{
    edge_t *edge, *newEdge, *head, *tail, *ebwd;
    terminal_t *newTerm;
    coor_t nextX, termX, termY;
    int termSplit;

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

    thisX = newEdge -> xl;
    if (thisX > termX) thisX = termX;

    if (thisX == INF) goto ret;

    if (optAlarmInterval) {
	begtime = time (NULL);
	oldtime = 0;
	flag = 1; /* enable catchAlarm */
	catchAlarm ();
    }

    while (thisX < INF) {
	nextX = INF;

	Debug (fprintf (stderr, "thisX=%d\n", thisX));

	if (optAlarmInterval < 0) catchAlarm ();

	scan_xpos++;

	edge = head -> fwd;

	while (edge != tail || newEdge -> xl == thisX || termX == thisX) {

	    thisY = Y (edge, thisX);

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
		}
		if (equalAtX (edge, newEdge)) {
		    coor_t old_xr = edge -> xr;
		    do {
			bundle (newEdge, edge);
			newEdge = fetchEdge ();
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
		/* The following will split the 'edge -> bwd -> tile' when necessary.
		   It will also perform a down slicing when appropriate.
		   Connecting terminals to tiles is NOT done here.
		*/
		tileAddTerm (edge -> bwd, termY);

		/* We split a tile immediately above a terminal because otherwise the
		   (point) terminal may not be on a left or right boundary of a tile.
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
		    tileCrossEdge (edge, termSplit);
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

	ASSERT (nextX > thisX);
	thisX = nextX;
    }
    thisY = INF;

    if (optAlarmInterval < 0) catchAlarm ();

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
