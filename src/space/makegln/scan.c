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
#include "src/space/auxil/auxil.h"
#include "src/space/makegln/makegln.h"
#include "src/space/makegln/proto.h"

static struct {
    ecnt_t xpos;		/* # x-positions */
    ecnt_t srl;			/* stateruler length */
    ecnt_t edges;		/* total number of input edges */
} scanInfo;

#ifdef MAKEMESH
#define equalAtX(edge, newEdge) \
(newEdge -> xl == thisX && newEdge -> yl == thisY \
&& (newEdge -> slope == edge -> slope \
    || (edge -> xr == edge -> xl && edge -> yr == edge -> yl) \
    || (newEdge -> xr == newEdge -> xl && newEdge -> yr == newEdge -> yl)))
#else
#define equalAtX(edge, newEdge) \
(newEdge -> xl == thisX && newEdge -> yl == thisY && newEdge -> slope == edge -> slope)
#endif

/* local operations */
Private void bundle (edge_t *newEdge, edge_t *edge);
Private void unbundle (edge_t *edge);
#ifdef DEBUG
Private void checkSR (edge_t *head, coor_t x);
#endif

void scan ()
{
    static edge_t *head = NULL;
    static edge_t *tail = NULL;
    edge_t *edge, *efwd, *newEdge;
    coor_t thisX, nextX, thisY, xr;

    /* initScan */
    if (head) {
	ASSERT (head -> fwd == tail && head -> bwd == tail);
	ASSERT (tail -> fwd == head && tail -> bwd == head);
    } else {
	head = createEdge (-INF, -INF, INF, -INF, 0, 0);
	tail = createEdge (-INF,  INF, INF,  INF, 0, 0);
	head -> fwd = head -> bwd = tail;
	tail -> fwd = tail -> bwd = head;
    }

    newEdge = fetchEdge ();
    thisX = newEdge -> xl;
    nextX = INF;

    if (optSortOnly) {
	while (thisX < INF) {
	    outputEdge (newEdge);
	    disposeEdge (newEdge);
	    newEdge = fetchEdge ();
	    thisX = newEdge -> xl;
	}
	return;
    }

    while (thisX < INF) {

	Debug (fprintf (stderr, "thisX=%g\n", (double) thisX / scale));

	scanInfo.xpos++;

	edge = head -> fwd;

	while (edge != tail || newEdge -> xl == thisX) {

	    if (edge != tail) {
		thisY = Y(edge, thisX);

		if (edge -> xi == thisX) edge = split (edge, thisX, thisY);

		if (edge -> bundle && edge -> xc == thisX) unbundle (edge);

#if 0
		if (newEdge -> xl == thisX && (newEdge -> yl < thisY || (newEdge -> yl == thisY
		    && edge -> xr > thisX && newEdge -> slope < edge -> slope))) goto smallerAtX;
#else
		if (newEdge -> xl == thisX) {
		    if (newEdge -> yl < thisY) goto smallerAtX;
		    if (newEdge -> yl == thisY && newEdge -> slope < edge -> slope) {
			/*
			 * Maybe there is another edge with equal slope (thus we can bundle)
			 */
			int ok = 1;
again:
			efwd = edge -> fwd;
			while (Y(efwd, thisX) == thisY) {
			    if (newEdge -> slope == efwd -> slope) { ok = 2; break; }
			    efwd = efwd -> fwd;
			}
			while (ok == 2) {
			    bundle (newEdge, efwd);
			    newEdge = fetchEdge ();
			    ok = 0;
			    if (newEdge -> xl == thisX && newEdge -> yl == thisY) {
				ok = 1;
				if (newEdge -> slope == efwd -> slope) ok = 2;
				else if (newEdge -> slope < edge -> slope) goto again;
			    }
			}
			if (ok && newEdge -> slope < edge -> slope) goto smallerAtX;
		    }
		}
#endif
	    }
	    else {
		/* insert newEdge below edge */
smallerAtX:
		Debug (printEdge ("insert", newEdge));
		Debug (printEdge ("below", edge));
		newEdge -> bwd = edge -> bwd;
		newEdge -> fwd = edge -> bwd -> fwd;
		edge -> bwd -> fwd = newEdge;
		edge -> bwd = newEdge;

		thisY = newEdge -> yl;
		edge = newEdge;
		newEdge = fetchEdge ();
		scanInfo.edges++;
	    }

	    efwd = edge -> fwd;

	    if (equalAtX (edge, newEdge)) {
		if (Y(efwd, thisX) > thisY || efwd -> xr == thisX) {
		    xr = edge -> xr;
		    do {
			bundle (newEdge, edge);
			newEdge = fetchEdge ();
			scanInfo.edges++;
		    } while (equalAtX (edge, newEdge));

		    if (edge -> xr > xr) {
			if (edge -> slope > efwd -> slope) testIntersect (thisX, edge, efwd);
		    }
		}
	    }

	    scanInfo.srl++;

	    glnUpdate (edge, thisX);

	    if (edge -> xr == thisX) {
		/* remove edge from scanline */
		Debug (printEdge ("exsert", edge));
		edge -> bwd -> fwd = efwd;
		efwd -> bwd = edge -> bwd;
		disposeEdge (edge);
		edge = efwd;
		if (edge -> slope < edge -> bwd -> slope) testIntersect (thisX, edge, edge -> bwd);
	    }
	    else {
		if (edge -> slope < edge -> bwd -> slope) testIntersect (thisX, edge, edge -> bwd);
		if (edge -> xi < nextX) nextX = edge -> xi;
		if (edge -> xc < nextX) nextX = edge -> xc;
		if (edge -> xr < nextX) nextX = edge -> xr;
		edge = efwd;
	    }
	}

#ifdef DEBUG
	if (DEBUG) {
	    fprintf (stderr, "checking stateruler\n");
	    checkSR (head, thisX);
	}
	else checkSR (head, thisX);
#endif /* DEBUG */

	scanAdvance ();

	if (newEdge -> xl < nextX) nextX = newEdge -> xl;
	ASSERT (nextX > thisX);
	thisX = nextX;
	nextX = INF;
    }
}

Private void bundle (edge_t *newEdge, edge_t *edge) /* insert overlapping edge */
{
    edge_t *e, *e1;

    Debug (printEdge ("bundle", newEdge));
    Debug (printEdge ("with",  edge));

    ASSERT (newEdge -> xr == newEdge -> xc);

    e = edge;

#ifdef MAKEMESH
    if (newEdge -> xl == newEdge -> xr && newEdge -> yl == newEdge -> yr) {
        disposeEdge (newEdge);
        goto post;
    }
    if (e -> xl == e -> xr && e -> yl == e -> yr) {
        e -> xl = newEdge -> xl;
        e -> xr = newEdge -> xr;
        e -> yl = newEdge -> yl;
        e -> yr = newEdge -> yr;
        e -> xc = newEdge -> xc;
        e -> slope = newEdge -> slope;
        e -> sign = newEdge -> sign;
        e -> signLeft = newEdge -> signLeft;
        disposeEdge (newEdge);
        goto post;
    }
#endif
    ASSERT (edge -> slope == newEdge -> slope);

    if (e -> xr == newEdge -> xl) {
	ASSERT (e -> bundle == NULL);
	e -> xr = newEdge -> xr;
	e -> yr = newEdge -> yr;
	e -> xc = newEdge -> xc;
	e -> sign = newEdge -> sign;
	disposeEdge (newEdge);
	goto post;
    }

    if (e -> xr < newEdge -> xr) {
	e -> xr = newEdge -> xr;
	e -> yr = newEdge -> yr;
    }

    if (e -> xc <= newEdge -> xr) {
	if (e -> xc > newEdge -> xl) e -> sign += newEdge -> sign;
	e1 = e -> bundle;
	while (e1 && e1 -> xc <= newEdge -> xr) {
	    e = e1; e -> sign += newEdge -> sign;
	    e1 = e -> bundle;
	}
	/* e is the last edge in bundle with e->xc <= newEdge->xr
	 */
	if (e -> xc == newEdge -> xr) {
	    disposeEdge (newEdge);
	}
	else { /* e -> xc < newEdge -> xc */
	    if (e -> bundle) newEdge -> sign += e -> bundle -> sign;
	    newEdge -> bundle = e -> bundle; e -> bundle = newEdge;
	}
    }
    else { /* e -> xc > newEdge -> xc, e = edge */
	sign_t ns = newEdge -> sign;
	newEdge -> sign = e -> sign;
	newEdge -> xc = e -> xc;
	e -> xc = newEdge -> xr;
	e -> sign += ns;
	newEdge -> bundle = e -> bundle; e -> bundle = newEdge;
    }

post:
#ifdef DEBUG
    for (e = edge; e -> bundle; e = e -> bundle) {
	ASSERT (e -> xc < e -> bundle -> xc);
    }
    ASSERT (e -> xc == edge -> xr);
#endif
    return;
}

Private void unbundle (edge_t *edge) /* delete overlapping edge */
{
    edge_t * b = edge -> bundle;

    Debug (printEdge ("unbundle", edge));

    edge -> signLeft = edge -> sign;
    edge -> sign   = b -> sign;
    edge -> xc     = b -> xc;
    edge -> bundle = b -> bundle;
    disposeEdge (b);
}

#ifdef DEBUG
Private void checkSR (edge_t *head, coor_t x)
{
    coor_t y, y2;
    edge_t * e;
    sign_t sign = 0;

    for (e = head -> fwd; e != head -> bwd; e = e -> fwd) {
#ifdef MAKEMESH
        if (e -> sign > 0) sign = !sign;
#else
	sign += e -> sign;
#endif
	if (DEBUG) printEdge ("edge", e);
	ASSERT (e -> xr > x);
	ASSERT (e -> xl <= x);
	y  = Y (e, x);
	y2 = Y (e -> fwd, x);
	ASSERT (y2 > y || (y2 == y && e -> fwd -> slope > e -> slope));
    }
    ASSERT (sign == 0);
}
#endif /* DEBUG */

void scanPrintInfo (FILE *fp)
{
    fprintf (fp, "\tnumber of x pos.    : %ld\n", (long) scanInfo.xpos);
    fprintf (fp, "\taverage sr length   : %ld\n", (long) (scanInfo.xpos ? scanInfo.srl/scanInfo.xpos : 0));
    fprintf (fp, "\taccumul. sr length  : %ld\n", (long) scanInfo.srl);
    fprintf (fp, "\ttotal # input edges : %ld\n", (long) scanInfo.edges);
}
