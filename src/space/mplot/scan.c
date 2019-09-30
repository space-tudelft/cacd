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

static edge_t *initScan (void);
static void delete (edge_t *edge);
static void insert (edge_t *newEdge, edge_t *edge);
static edge_t *find (edge_t *edge);
static void merge (edge_t *e1, edge_t *e2);

#ifdef DEBUG
static void checkSR (edge_t *head, int x)
{
    edge_t * e = head -> fwd;
    while (e != head -> bwd) {
	ASSERT (Y (e -> fwd, x) >= Y (e, x));
	e = e -> fwd;
    }
}
#endif /* DEBUG */

void scan ()
{
    edge_t * edge,
           * head    = initScan (),
           * newEdge = fetch (),
	   * fwdEdge,
           * vert    = NULL,
	   * e1,
	   * e2;
    int    black     = 0;
    coor_t thisX     = newEdge -> xl,
	   thisY,
	   nextX     = INF;

    while (thisX < INF) {
#ifdef DEBUG
	if (DEBUG) fprintf (stderr, "thisX=%d\n", thisX);
#endif /* DEBUG */
	edge = head -> fwd;
	ASSERT (vert == NULL);
	ASSERT (black == 0);
	while (edge -> yr < INF || newEdge -> xl == thisX) {

	    thisY = Y (edge, thisX);

	    if (newEdge -> xl == thisX && newEdge -> yl < thisY) {
		insert (newEdge, edge);

		black ^= 1;

		if (!vert)
		    vert = newEdge;
		else {
		    merge (find (vert), newEdge);
		    if (black) vert -> link = newEdge;
		    else       newEdge -> link = vert;
		    vert = NULL;
		}

		nextX = Min (nextX, newEdge -> xr);
		newEdge = fetch ();
	    }

	    else if (edge -> xr == thisX) {
		fwdEdge = edge -> fwd;
		delete (edge);

		if (!vert)
		    vert = edge;
		else {
		    e1 = find (edge);
		    e2 = find (vert);
		    if (e1 != e2) {
			merge (e1, e2);
			if (black) edge -> link = vert;
			else       vert -> link = edge;
		    }
		    else {
			if (black) plotContour (vert);	/* contour of hole */
			else       plotContour (edge);	/* external contour */
		    }
		    vert = NULL;
		}
		edge = fwdEdge;
	    }

	    else {
		nextX = Min (nextX, edge -> xr);
		edge = edge -> fwd;
		black ^= 1;
	    }
	}
#ifdef DEBUG
	if (DEBUG) {
	    fprintf (stderr, "checking stateruler\n");
	    checkSR (head, thisX);
	}
#endif /* DEBUG */

	thisX = Min (nextX, newEdge -> xl);
	nextX = INF;
    }
}

static edge_t * initScan ()
{
    static edge_t _head, _tail;
    edge_t * head = &_head, * tail = &_tail;

    head -> xl = -INF; head -> xr =  INF;
    head -> yl = -INF; head -> yr = -INF;
    tail -> xl = -INF; tail -> xr =  INF;
    tail -> yl =  INF; tail -> yr =  INF;
    head -> fwd = head -> bwd = tail;
    tail -> fwd = tail -> bwd = head;

    return (head);
}

static void delete (edge_t *edge) /* delete edge from scanline */
{
#ifdef DEBUG
    if (DEBUG) printEdge ("delete", edge);
#endif /* DEBUG */
    edge -> bwd -> fwd = edge -> fwd;
    edge -> fwd -> bwd = edge -> bwd;
}

static void insert (edge_t *newEdge, edge_t *edge) /* insert newEdge below edge */
{
#ifdef DEBUG
    if (DEBUG) printEdge ("insert", newEdge);
    if (DEBUG) printEdge ("below", edge);
#endif /* DEBUG */
    newEdge -> bwd = edge -> bwd;
    newEdge -> fwd = edge -> bwd -> fwd;
    edge -> bwd -> fwd = newEdge;
    edge -> bwd = newEdge;
}

static edge_t * find (edge_t *edge) /* find contour of which edge is part */
{
    edge_t * t, * i = edge;
    while (edge -> contour)		/* find the root */
	edge = edge -> contour;
    while (i -> contour) {		/* compress */
	t = i; i = i -> contour; t -> contour = edge;
    }
    ASSERT (edge -> contour == NULL);	/* edge is a root */
    return (edge);
}

static void merge (edge_t *e1, edge_t *e2) /* merge two contours */
{
    /* should do union by size */
    ASSERT (e1 -> contour == NULL);	/* e1 is a root */
    ASSERT (e2 -> contour == NULL);	/* e2 is a root */
    ASSERT (e1 != e2);
    e2 -> contour = e1;
}
