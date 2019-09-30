/*
 * ISC License
 *
 * Copyright (C) 1995-2018 by
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

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include "src/space/include/config.h"
#include "src/libddm/dmincl.h"
#include "src/space/auxil/auxil.h"
#include "src/space/makegln/makegln.h"
#include "src/space/makedela/makedela.h"
#include "src/space/auxil/proto.h"
#include "src/space/makedela/dproto.h"

#ifdef __cplusplus
  extern "C" {
#endif

edge_t *fetchEdge (void);
vertex_t *newVertex (coor_t x, coor_t y);
meshedge_t *newMeshEdge (vertex_t *vA, vertex_t *vB, int fixed, int connect);
void insertMeshEdge (vertex_t *v, meshedge_t *e);
void testMeshEdge (meshedge_t *e, meshedge_t *eo1, meshedge_t *eo2);
void deleteMeshEdge (meshedge_t *e);
hullmem_t *add2Hull (hullmem_t *hprev, vertex_t *vnew);
void rmFromHull (hullmem_t *h);
void drawMeshEdgesOf (FILE *fp_out, vertex_t *v, int dbformat);
void resetMeshEdgeFlags (vertex_t *v);
double angleVertices (vertex_t *va, vertex_t *vb, vertex_t *vc);
void searchContact (vertex_t *v, contact_t *cont);

#ifdef __cplusplus
  }
#endif

#define L(v) (long)(v)

hullmem_t *beginHull = NULL;

meshedge_t *HereEEprev = (meshedge_t *)0;

struct Vertex *Begin_vertices = NULL;

void scan ()
{
    coor_t thisX;
    coor_t nextX;
    int colorL;
    int colorR;
    int nextColorL = 0; /* init, else compiler warning */
    int nextColorR = 0; /* init, else compiler warning */
    int prevColorL;
    vertex_t *vA;
    vertex_t *vB;
    vertex_t *newV;
    hullmem_t *followH;
    hullmem_t *aboveH;
    hullmem_t *belowH;
    hullmem_t *lookH;
    hullmem_t *helpH;
    hullmem_t *newH1;
    hullmem_t *newH2;
    meshedge_t *e;
    meshedge_t *newFwd;
    int firstXstop;
    int fixed;
    int connect;
    int left, right, area;
    vertex_t dummyV;
    edge_t * inln;

    inln = fetchEdge ();
    thisX = inln -> xl;
    firstXstop = 1;

    while (thisX < INF) {

if (optDebug) fprintf (stderr, "thisX = %ld\n", L(thisX));

	nextX = INF;

	/* Colors (left and to the right of nextX) immediately
	   below followH if followH -> forward exists.
	   Otherwise, colors just above followH (different from
	   below if followH -> forward has been deleted.
	*/

	colorL = 0;
	colorR = 0;

	if (beginHull == NULL) {
	    vA = newVertex (inln -> xl, inln -> yl);
	    beginHull = add2Hull ((hullmem_t *)NULL, vA);
	    vB = newVertex (inln -> xr, inln -> yr);
	    beginHull -> forward = newMeshEdge (vA, vB, 1, 1);
	    inln = fetchEdge ();
	}

	lookH = beginHull;
	aboveH = NULL;
	belowH = beginHull;
	followH = beginHull;

	while (lookH || inln -> xl == thisX) {

            /* Look for next point to be added */

	    while (lookH && (lookH -> forward == NULL ||
		(lookH -> forward && lookH -> forward -> vB -> x > thisX))) {

		lookH = lookH -> nextHull;
	    }

            newV = NULL;
	    newFwd = NULL;

            left = right = area = 0;

            if (lookH && lookH -> forward
		&& lookH -> forward -> vB -> x == thisX
		&& inln -> xl == thisX
		&& inln -> yl == lookH -> forward -> vB -> y
		&& (inln -> yr - inln -> yl)/(inln -> xr - inln -> xl) ==
		   (lookH -> forward -> vB -> y - lookH -> forward -> vA -> y)
		   /(lookH -> forward -> vB -> x - lookH -> forward -> vA -> x)
		) {

		/* extend lookH -> forward with inln edge. */

if (optDebug) fprintf (stderr, "extend lookH -> forward\n");

		lookH -> forward -> vB -> x = inln -> xr;
		lookH -> forward -> vB -> y = inln -> yr;

		inln = fetchEdge ();
	    }
	    else if (lookH && lookH -> forward
		&& lookH -> forward -> vB -> x == thisX
		&& !(inln -> xl == thisX
		     && inln -> yl < lookH -> forward -> vB -> y)) {

		/* Add right point of edge to set */
if (optDebug) fprintf (stderr, "add right point\n");

		left = 1;

		newV = lookH -> forward -> vB;

		while (followH != lookH) {
		    if (followH -> forward) {

			if (followH -> forward -> vA -> x < thisX)
			    colorL = !colorL;

                        ASSERT (followH -> forward -> vB -> x > thisX);
			colorR = !colorR;

			if (followH -> forward -> vB -> x < nextX)
			    nextX = followH -> forward -> vB -> x;
		    }

		    followH = followH -> nextHull;
if (optDebug) fprintf (stderr, "(a) followH = %d %d  colors: %d %d\n",
followH -> v -> x, followH -> v -> y, colorL, colorR);
		}

		insertMeshEdge (followH -> forward -> vA,
			    followH -> forward);
		insertMeshEdge (followH -> forward -> vB,
			    followH -> forward);
		followH -> forward = NULL;
		colorL = !colorL;

		nextColorL = colorL;
		nextColorR = colorR;
	    }
	    else if (inln -> xl == thisX) {

                if (inln -> xl == inln -> xr && inln -> yl == inln -> yr) {

		    /* Area information is added */

		    area = 1;

                    newV = &dummyV;
		    newV -> x = inln -> xl;
		    newV -> y = inln -> yl;
		}
		else {

		    /* Add left point of edge to set */
if (optDebug) fprintf (stderr, "add left point\n");

		    right = 1;

		    vA = newVertex (inln -> xl, inln -> yl);
		    vB = newVertex (inln -> xr, inln -> yr);
		    newFwd = newMeshEdge (vA, vB, 1, 1);
		    if (vB -> x < nextX)
			nextX = vB -> x;

		    newV = vA;
		}

		inln = fetchEdge ();

		if (!firstXstop) {

		    aboveH = belowH;

		    while (aboveH
			   && (aboveH -> forward == NULL
			       || (aboveH -> forward
				   && YM (aboveH -> forward, thisX)
							     < newV -> y))) {
			if (aboveH -> forward)
			    belowH = aboveH;
			aboveH = aboveH -> nextHull;
		    }

		    if (belowH) {
			while (followH != belowH) {

			    if (followH -> forward) {

				if (followH -> forward -> vA -> x < thisX)
				    colorL = !colorL;

				ASSERT (followH -> forward -> vB -> x
								    > thisX);
				colorR = !colorR;

				if (followH -> forward -> vB -> x < nextX)
				    nextX = followH -> forward -> vB -> x;
			    }

			    followH = followH -> nextHull;

if (optDebug) fprintf (stderr, "(b) followH = %d %d  colors: %d %d\n",
			followH -> v -> x, followH -> v -> y, colorL, colorR);

			    ASSERT (followH);
			}
		    }
		}

		while (followH -> nextHull
		       && (followH -> forward == NULL
			   || (followH -> forward
			       && YM (followH -> forward, thisX)
							       < newV -> y))
		       && !(followH -> nextHull -> forward
			    && YM (followH -> nextHull -> forward, thisX)
							       > newV -> y
			    && SLOPE (followH -> v, followH -> nextHull -> v)
				> SLOPE (followH -> v, newV))
		       && (DIST2 (followH -> nextHull -> v, newV)
				       < DIST2 (followH -> v, newV)
			   || (followH -> nextHull -> v -> x
						  > followH -> v -> x
			       && SLOPE (followH -> v, followH -> nextHull -> v)
				   < SLOPE (followH -> v, newV)))) {

		    if (followH -> forward
		        && YM (followH -> forward, thisX) <= newV -> y) {

			if (followH -> forward -> vA -> x < thisX)
			    colorL = !colorL;

                        ASSERT (followH -> forward -> vB -> x > thisX);
			colorR = !colorR;

			if (followH -> forward -> vB -> x < nextX)
			    nextX = followH -> forward -> vB -> x;
		    }

		    followH = followH -> nextHull;
if (optDebug) fprintf (stderr, "(c) followH = %d %d  colors: %d %d\n",
			followH -> v -> x, followH -> v -> y, colorL, colorR);

		}

		if (followH -> forward
		    && YM (followH -> forward, thisX) <= newV -> y) {

		    if (followH -> forward -> vA -> x < thisX)
			nextColorL = !colorL;
		    else
			nextColorL = colorL;

		    ASSERT (followH -> forward -> vB -> x > thisX);
		    nextColorR = !colorR;
		}
		else {
		    nextColorL = colorL;
		    nextColorR = colorR;
		}

		ASSERT (!(followH -> v -> x == newV -> x
			  && followH -> v -> y > newV -> y));

                if (area) {

		}
		else {
		    fixed = 0;
		    if (followH -> v -> x == newV -> x) {
			if ((followH -> v -> y < newV -> y
			     && nextColorL != nextColorR)
			    || (followH -> v -> y == newV -> y))
			fixed = 1;
		    }
		    connect = fixed;
		    /*
		    This one is not always true.
		    Moreover, I think it is not necessary that it is true.

		    ASSERT (followH -> v -> y <= newV -> y);
		    */
		    if (nextColorL) connect = 1;
		    e = newMeshEdge (followH -> v, newV, fixed, connect);
		    insertMeshEdge (followH -> v, e);
		    insertMeshEdge (newV, e);
		}
	    }

if (optDebug) fprintf (stderr, "colorLR = %d %d  nextColorLR = %d %d\n",
			colorL, colorR, nextColorL, nextColorR);

            if (!area && newV) {
if (optDebug) fprintf (stderr, "start newV = %ld %ld\n", L(newV -> x), L(newV -> y));
ASSERT (followH -> v != newV);

		newH1 = add2Hull (followH, newV);
		newH2 = add2Hull (newH1, followH -> v);

		/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
		/* The following currently only works for orthogonal layout */
		if (followH -> forward
		    && newV -> y < followH -> forward -> vB -> y) {
		    newH2 -> forward = followH -> forward;
		    followH -> forward = NULL;
		}

		if (newFwd)
		    newH1 -> forward = newFwd;

                ASSERT (left || right);

		if (left) {
		    ASSERT (followH -> forward == NULL);
		    prevColorL = !colorL;
		}
		else
		    prevColorL = colorL;

		if (followH -> forward) {

                    /* Already update colorL and colorR
		       (followH will become newH1 after the following). */

                    if (followH -> forward -> vA -> x < thisX)
			colorL = !colorL;

		    ASSERT (followH -> forward -> vB -> x > thisX);
if (!(followH -> forward -> vB -> x > thisX))
fprintf (stderr, "followH -> forward %p\n", followH -> forward);
		    colorR = !colorR;

if (optDebug) fprintf (stderr, "followH = %d %d  followH -> forward = %d %d colors: %d %d\n",
followH -> v -> x, followH -> v -> y,
followH -> forward -> vB -> x,
followH -> forward -> vB -> y,
colorL, colorR);

		    if (followH -> forward -> vB -> x < nextX)
			nextX = followH -> forward -> vB -> x;
		}

if (optDebug) fprintf (stderr, "colorLR = %d %d  nextColorLR = %d %d\n",
			colorL, colorR, nextColorL, nextColorR);

		helpH = newH2;
		while (helpH -> nextHull
		       && (helpH -> forward == NULL
			   || helpH -> forward -> vB == newV
			   || helpH -> v -> y < newV -> y)
		       && SLOPE (helpH -> nextHull -> v, newV)
					        < SLOPE (helpH -> v, newV)) {

		    helpH = helpH -> nextHull;

		    fixed = 0;
		    if (helpH -> v -> x == newV -> x) {
			if ((left && ((helpH -> v -> y > newV -> y
				       && nextColorL != nextColorR)
				      || ((helpH -> v -> y < newV -> y
				           && nextColorL == nextColorR))))
			    || (right && ((helpH -> v -> y > newV -> y
					   && nextColorL == nextColorR)
					  || ((helpH -> v -> y < newV -> y
					       && nextColorL != nextColorR)))))
			fixed = 1;
		    }
		    connect = fixed;
		    if (colorL)
			connect = 1;
		    e = newMeshEdge (helpH -> v, newV, fixed, connect);
		    insertMeshEdge (helpH -> v, e);
		    insertMeshEdge (newV, e);

		    testMeshEdge (e -> prevA, e, e -> nextB);

		    if (helpH -> prevHull != beginHull) {
			rmFromHull (helpH -> prevHull);
		    }
		}

		helpH = followH;
		while (helpH -> prevHull
		       && (helpH -> forward == NULL
			   || helpH -> forward -> vB == newV
			   || helpH -> v -> y > newV -> y)
		       && SLOPE (helpH -> prevHull -> v, newV)
						> SLOPE (helpH -> v, newV)) {

		    helpH = helpH -> prevHull;

		    fixed = 0;
		    if (helpH -> v -> x == newV -> x) {
			if ((left && ((helpH -> v -> y > newV -> y
				       && nextColorL != nextColorR)
				      || ((helpH -> v -> y < newV -> y
				           && nextColorL == nextColorR))))
			    || (right && ((helpH -> v -> y > newV -> y
					   && nextColorL == nextColorR)
					  || ((helpH -> v -> y < newV -> y
					       && nextColorL != nextColorR)))))
			fixed = 1;
		    }
		    connect = fixed;
		    if (prevColorL)
			connect = 1;
		    e = newMeshEdge (helpH -> v, newV, fixed, connect);
		    insertMeshEdge (helpH -> v, e);
		    insertMeshEdge (newV, e);

		    testMeshEdge (e -> nextA, e, e -> prevB);

		    if (helpH -> nextHull != beginHull) {
			rmFromHull (helpH -> nextHull);
		    }
		}

		followH = newH1;
		lookH = newH1;

if (optDebug) fprintf (stderr, "end newV\n");
	    }

	    belowH = followH;
	}

	while (followH) {
	    if (followH -> forward) {

		if (followH -> forward -> vA -> x < thisX)
		    colorL = !colorL;

		ASSERT (followH -> forward -> vB -> x > thisX);
		colorR = !colorR;

if (optDebug) fprintf (stderr, "(c) followH = %d %d  colors: %d %d\n",
			followH -> v -> x, followH -> v -> y, colorL, colorR);

		if (followH -> forward -> vB -> x < nextX)
		    nextX = followH -> forward -> vB -> x;
	    }
	    followH = followH -> nextHull;
	}

        ASSERT (colorL == 0);
        ASSERT (colorR == 0);

	if (inln -> xl < nextX) nextX = inln -> xl;

        if (firstXstop) firstXstop = 0;

	thisX = nextX;
    }
}

vertex_t *newVertex (coor_t x, coor_t y)
{
    vertex_t *v_l;
    vertex_t *v = NEW (vertex_t, 1);

    v -> x = x;
    v -> y = y;
    v -> edges = NULL;
    v -> next = NULL;
    v -> contbound_next = NULL;
    v -> contbound = NEW (contactBoundary_t, 1);
/*
fprintf (stderr, "NEW contactboundary %x\n", v -> contbound);
*/
    v -> contbound -> vertices = v;
    v -> contbound -> bound_next = NULL;
    v -> contbound -> cont = NEW (contact_t, 1);
/*
fprintf (stderr, "NEW contact %x\n", v -> contbound -> cont);
*/
    v -> contbound -> cont -> nr = -1;
    v -> contbound -> cont -> boundaries = v -> contbound;
    v -> contbound -> cont -> neighbors = NULL;

    if (Begin_vertices == NULL) {
	Begin_vertices = v;
    }
    else {
	v_l = Begin_vertices;
	if (v_l -> x > v -> x || (v_l -> x == v -> x && v_l -> y > v -> y)) {
	    v -> next = Begin_vertices;
	    Begin_vertices = v;
	}
	else {
	    while (v_l -> next
		   && ((v_l -> next -> x < v -> x)
			|| (v_l -> next -> x == v -> x
			    && v_l -> next -> y < v -> y)))
		v_l = v_l -> next;
	    v -> next = v_l -> next;
	    v_l -> next = v;
	}
    }

if (optDebug) fprintf (stderr, "newVertex: %ld %ld\n", L(v -> x), L(v -> y));

    return (v);
}

meshedge_t *newMeshEdge (vertex_t *vA, vertex_t *vB, int fixed, int connect)
{
    vertex_t *v1, *v2, *vr, *vn, *vh;
    contactBoundary_t *bdr;
    meshedge_t *e = NEW (meshedge_t, 1);

    e -> vA = vA;
    e -> vB = vB;
    e -> fixed = fixed;
    e -> flag = 0;

    ASSERT (vB -> x > vA -> x || (vA -> x == vB -> x && vB -> y > vA -> y));

if (optDebug) fprintf (stderr, "newMeshEdge: %ld %ld - %ld %ld (%d %d)\n",
	L(e -> vA -> x), L(e -> vA -> y), L(e -> vB -> x), L(e -> vB -> y), fixed, connect);

    if (connect && vA -> contbound -> cont != vB -> contbound -> cont) {
/*
fprintf (stderr, "MERGE CONTACTS %x %x boundaries %x %x\n",
vA -> contbound -> cont, vB -> contbound -> cont,
vA -> contbound, vB -> contbound);
*/
	/* Merge contacts. */
	for (bdr = vA -> contbound -> cont -> boundaries;
	     bdr -> bound_next; bdr = bdr -> bound_next);
	bdr -> bound_next = vB -> contbound -> cont -> boundaries;
/*
fprintf (stderr, "DISPOSE contact %x\n", vB -> contbound -> cont);
*/
	DISPOSE (vB -> contbound -> cont, sizeof(contact_t));
/*
{   contactBoundary_t *bdr2;
    bdr2 = vA -> contbound  -> cont -> boundaries;
    while (bdr2 != bdr -> bound_next) {
	fprintf (stderr, "BDR %x cont %x \n", bdr2, bdr2 -> cont);
	bdr2 = bdr2 -> bound_next;
    }
}
*/
	for (bdr = bdr -> bound_next; bdr; bdr = bdr -> bound_next) {
/*
fprintf (stderr, "BDR %x cont %x -> %x  bdr of new %x\n",
bdr, bdr -> cont, vA -> contbound -> cont, vA -> contbound);
*/
	    bdr -> cont = vA -> contbound -> cont;
	}
    }

    if (fixed && vA -> contbound != vB -> contbound) {
/*
fprintf (stderr, "MERGE BOUNDARIES %x %x\n", vA -> contbound, vB -> contbound);
*/
	/* Merge contact boundaries. */
	ASSERT (connect);
	for (v1 = vA -> contbound -> vertices;
	     v1 -> contbound_next; v1 = v1 -> contbound_next);
	for (v2 = vB -> contbound -> vertices;
	     v2 -> contbound_next; v2 = v2 -> contbound_next);
	/* v1 and v2 are the last entries of the lists of
	   vA -> contbound -> vertices and vB -> contbound -> vertices
	   respectively.
	*/
	ASSERT (vA -> contbound -> vertices == vA || v1 == vA);
	ASSERT (vB -> contbound -> vertices == vB || v2 == vB);
	/* We want vA at the end of the list */
	if (vA -> contbound -> vertices == vA) {
	    /* reverse list */
	    vh = vA -> contbound -> vertices;
	    vr = NULL;
	    while (vh) {
		vn = vh -> contbound_next;
		vh -> contbound_next = vr;
		vr = vh;
		vh = vn;
	    }
            v1 = vA -> contbound -> vertices;
	    vA -> contbound -> vertices = vr;
	}
	/* We want vB at the beginning of the list */
	if (v2 == vB) {
	    /* reverse list */
	    vh = vB -> contbound -> vertices;
	    vr = NULL;
	    while (vh) {
		vn = vh -> contbound_next;
		vh -> contbound_next = vr;
		vr = vh;
		vh = vn;
	    }
            v2 = vB -> contbound -> vertices;
	    vB -> contbound -> vertices = vr;
	}
	ASSERT (v1 == vA && vB -> contbound -> vertices == vB);
	ASSERT (v1 -> contbound_next == NULL);
	v1 -> contbound_next = vB -> contbound -> vertices;

	/* Remove vB -> contbound from boundary list. */

	ASSERT (vA -> contbound -> cont == vB -> contbound -> cont);

        bdr = vA -> contbound -> cont -> boundaries;
/*
if (bdr == NULL)
fprintf (stderr, "%x %x\n", vA -> contbound, vA -> contbound -> cont);
*/
	if (bdr == vB -> contbound)
	    vA -> contbound -> cont -> boundaries = bdr -> bound_next;
	else {
	    while (bdr -> bound_next != vB -> contbound)
		bdr = bdr -> bound_next;
	    bdr -> bound_next = bdr -> bound_next -> bound_next;
	}
/*
fprintf (stderr, "DISPOSE contactboundary %x\n", vB -> contbound);
*/
	DISPOSE (vB -> contbound, sizeof(contactBoundary_t));

	for (v1 = v1 -> contbound_next; v1; v1 = v1 -> contbound_next) {
	    v1 -> contbound = vA -> contbound;
	}
    }

    return (e);
}

void insertMeshEdge (vertex_t *v, meshedge_t *e)
{
    meshedge_t *ev;
    meshedge_t *prev_ev;
    double e_slope;
    int firsttime;

    ev = v -> edges;
    if (ev) {
	e_slope = ESLOPE (e); firsttime = 1;
	while (!(ev == v -> edges && !firsttime)
	       && ((e -> vA == v && ev -> vA == v && ESLOPE (ev) < e_slope)
	           || (e -> vB == v && ev -> vB == v && ESLOPE (ev) < e_slope)
	           || (e -> vB == v && ev -> vA == v))) {
	    ev = NEXT_E (ev, v);
	    firsttime = 0;
	}
	if (e -> vA == v) {
	    e -> nextA = ev;
	    e -> prevA = PREV_E (ev, v);
	}
	else {
	    e -> nextB = ev;
	    e -> prevB = PREV_E (ev, v);
	}

	if (ev -> vA == v)
	    prev_ev = ev -> prevA;
	else
	    prev_ev = ev -> prevB;

	if (prev_ev -> vA == v)
	    prev_ev -> nextA = e;
	else
	    prev_ev -> nextB = e;

	if (ev -> vA == v)
	    ev -> prevA = e;
	else
	    ev -> prevB = e;

	if (firsttime)
	    v -> edges = e;
    }
    else {
	v -> edges = e;
	if (e -> vA == v) {
	    e -> prevA = e;
	    e -> nextA = e;
	}
	else {
	    e -> prevB = e;
	    e -> nextB = e;
	}
    }
}

void testMeshEdge (meshedge_t *e, meshedge_t *eo1, meshedge_t *eo2)
{
    vertex_t *v1, *v2, *v3, *v4;
    meshedge_t *e12, *e23, *e34, *e41, *enew;
    double a, minAngleCurrent, minAngleOther;
    int sign1, sign3;

    if (e -> fixed) return;

    v1 = e -> vA;
    e41 = e -> nextA;
    e12 = e -> prevA;

    if (e41 == NULL || e12 == e41) return;

    if (e41 -> vA == v1)
	v4 = e41 -> vB;
    else
	v4 = e41 -> vA;

    v3 = e -> vB;
    e23 = e -> nextB;
    e34 = e -> prevB;

    if (e23 == NULL || e23 == e34) return;

    if (e23 -> vA == v3)
	v2 = e23 -> vB;
    else
	v2 = e23 -> vA;

    if (e12 -> vA != v2 && e12 -> vB != v2) return;

    if (e34 -> vA != v4 && e34 -> vB != v4) return;

    if (v2 -> x == v4 -> x) {
	if (v1 -> x < v2 -> x)
	    sign1 = -1;
	else if (v1 -> x > v2 -> x)
	    sign1 = 1;
	else
	    sign1 = 0;
	if (v3 -> x < v2 -> x)
	    sign3 = -1;
	else if (v3 -> x > v2 -> x)
	    sign3 = 1;
	else
	    sign3 = 0;
    }
    else {
	if (v4 -> y + (v1 -> x - v4 -> x)
		      * (v2 -> y - v4 -> y)/(v2 -> x - v4 -> x) > v1 -> y)
	    sign1 = -1;
	else if (v4 -> y + (v1 -> x - v4 -> x)
		           * (v2 -> y - v4 -> y)/(v2 -> x - v4 -> x) < v1 -> y)
	    sign1 = 1;
	else
	    sign1 = 0;
	if (v4 -> y + (v3 -> x - v4 -> x)
		      * (v2 -> y - v4 -> y)/(v2 -> x - v4 -> x) > v3 -> y)
	    sign3 = -1;
	else if (v4 -> y + (v3 -> x - v4 -> x)
		           * (v2 -> y - v4 -> y)/(v2 -> x - v4 -> x) < v3 -> y)
	    sign3 = 1;
	else
	    sign3 = 0;
    }

    if ((sign1 >= 0 && sign3 >= 0) || (sign1 <= 0 && sign3 <= 0)) return;

    minAngleCurrent = M_PI;
    a = angleVertices (v1, v2, v3);
    if (a < minAngleCurrent) minAngleCurrent = a;
    a = angleVertices (v2, v1, v3);
    if (a < minAngleCurrent) minAngleCurrent = a;
    a = angleVertices (v3, v1, v2);
    if (a < minAngleCurrent) minAngleCurrent = a;
    a = angleVertices (v1, v3, v4);
    if (a < minAngleCurrent) minAngleCurrent = a;
    a = angleVertices (v3, v1, v4);
    if (a < minAngleCurrent) minAngleCurrent = a;
    a = angleVertices (v4, v1, v3);
    if (a < minAngleCurrent) minAngleCurrent = a;

    minAngleOther = M_PI;
    a = angleVertices (v1, v2, v4);
    if (a < minAngleOther) minAngleOther = a;
    a = angleVertices (v2, v1, v4);
    if (a < minAngleOther) minAngleOther = a;
    a = angleVertices (v4, v1, v2);
    if (a < minAngleOther) minAngleOther = a;
    a = angleVertices (v3, v2, v4);
    if (a < minAngleOther) minAngleOther = a;
    a = angleVertices (v2, v3, v4);
    if (a < minAngleOther) minAngleOther = a;
    a = angleVertices (v4, v3, v2);
    if (a < minAngleOther) minAngleOther = a;

    if (minAngleCurrent < minAngleOther) {

if (optDebug) fprintf (stderr, "CHANGED: %ld %ld - %ld %ld into %ld %ld - %ld %ld\n",
		L(e -> vA -> x), L(e -> vA -> y), L(e -> vB -> x), L(e -> vB -> y),
		L(v2 -> x), L(v2 -> y), L(v4 -> x), L(v4 -> y));

        deleteMeshEdge (e);

	if (v2 -> x > v4 -> x || (v2 -> x == v4 -> x && v2 -> y > v4 -> y))
	    enew = newMeshEdge (v4, v2, 0, 0);
	else
	    enew = newMeshEdge (v2, v4, 0, 0);

	insertMeshEdge (v2, enew);
	insertMeshEdge (v4, enew);

	if (eo1 == e23 || eo1 == e12) {
	    testMeshEdge (e34, enew, e23);
	    testMeshEdge (e41, enew, e12);
	}
	else {
	    testMeshEdge (e23, enew, e34);
	    testMeshEdge (e12, enew, e41);
	}
    }
}

void deleteMeshEdge (meshedge_t *e)
{
    if (e -> prevA -> vA == e -> vA)
	e -> prevA -> nextA = e -> nextA;
    else
	e -> prevA -> nextB = e -> nextA;
    if (e -> nextA -> vA == e -> vA)
	e -> nextA -> prevA = e -> prevA;
    else
	e -> nextA -> prevB = e -> prevA;
    if (e -> vA -> edges == e)
	e -> vA -> edges = e -> nextA;

    if (e -> prevB -> vA == e -> vB)
	e -> prevB -> nextA = e -> nextB;
    else
	e -> prevB -> nextB = e -> nextB;
    if (e -> nextB -> vA == e -> vB)
	e -> nextB -> prevA = e -> prevB;
    else
	e -> nextB -> prevB = e -> prevB;
    if (e -> vB -> edges == e)
	e -> vB -> edges = e -> nextB;

    e -> nextA = NULL;
    e -> prevA = NULL;
    e -> nextB = NULL;
    e -> prevB = NULL;

    DISPOSE (e, sizeof(meshedge_t));
}

hullmem_t *add2Hull (hullmem_t *hprev, vertex_t *vnew)
{
    hullmem_t *h = NEW (hullmem_t, 1);
/* fprintf (stderr, "NEW hull %x\n", h); */

    h -> v = vnew;
    h -> forward = NULL;

    if (hprev == NULL) {
	h -> nextHull = NULL;
	h -> prevHull = NULL;
    }
    else {
	h -> nextHull = hprev -> nextHull;
	if (h -> nextHull)
	    h -> nextHull -> prevHull = h;
	hprev -> nextHull = h;
	h -> prevHull = hprev;
    }

if (optDebug) {
    hullmem_t *h2 = beginHull;
    fprintf (stderr, "HULL: added %ld %ld :", L(h -> v -> x), L(h -> v -> y));
    while (h2) {
	fprintf (stderr, ": %ld %ld", L(h2 -> v -> x), L(h2 -> v -> y));
	h2 = h2 -> nextHull;
    }
    fprintf (stderr, "\n");
}
    return (h);
}

void rmFromHull (hullmem_t *h)
{
    if (h -> nextHull)
	h -> nextHull -> prevHull = h -> prevHull;
    if (h -> prevHull)
	h -> prevHull -> nextHull = h -> nextHull;
    h -> nextHull = NULL;
    h -> prevHull = NULL;

if (optDebug) {
    hullmem_t *h2 = beginHull;
    fprintf (stderr, "HULL: rm %ld %ld :", L(h -> v -> x), L(h -> v -> y));
    while (h2) {
	fprintf (stderr, ": %ld %ld", L(h2 -> v -> x), L(h2 -> v -> y));
	h2 = h2 -> nextHull;
    }
    fprintf (stderr, "\n");
}
/* fprintf (stderr, "DISPOSE hull %x\n", h); */
    DISPOSE (h, sizeof(hullmem_t));
}

double angleVertices (vertex_t *va, vertex_t *vb, vertex_t *vc)
{
    double alpha, l1, l2, x, y;

    y = vb -> y - va -> y;
    x = vb -> x - va -> x;
    l1 = atan2 (y, x);
/*
if (optDebug) fprintf (stderr, "x = %le  y = %le  l1 = %le\n", x, y, l1);
*/

    y = vc -> y - va -> y;
    x = vc -> x - va -> x;
    l2 = atan2 (y, x);
/*
if (optDebug) fprintf (stderr, "x = %le  y = %le  l2 = %le\n", x, y, l2);
*/

    alpha = l1 - l2;

    if (alpha < 0) alpha = -alpha;

    if (2 * M_PI - alpha < alpha)
	return (2 * M_PI - alpha);
    else
	return (alpha);
}

void drawGraph (FILE *fp_out, int dbformat)
{
    if (!dbformat) {
	fprintf (fp_out, ".PS 6i\n");
	fprintf (fp_out, "dashwid = 3i\n");
    }

    if (beginHull) {
	drawMeshEdgesOf (fp_out, beginHull -> v, dbformat);
        resetMeshEdgeFlags (beginHull -> v);
    }

    if (!dbformat) fprintf (fp_out, ".PE\n");
}

void resetMeshEdgeFlags (vertex_t *v)
{
    meshedge_t *e;
    int notfirst;

    e = v -> edges;

    if (e == NULL) return;

    notfirst = 0;

    while (!(e == v -> edges && notfirst)) {
	notfirst = 1;
	if (e -> flag == 1) {
	    e -> flag = 0;
	    if (e -> vA == v)
		resetMeshEdgeFlags (e -> vB);
	    else
		resetMeshEdgeFlags (e -> vA);
	}
	e = NEXT_E (e, v);
    }
}

void drawMeshEdgesOf (FILE *fp_out, vertex_t *v, int dbformat)
{
    meshedge_t *e;
    int notfirst;

    e = v -> edges;

    if (e == NULL) return;

    notfirst = 0;

    while (!(e == v -> edges && notfirst)) {
	notfirst = 1;
	if (e -> flag == 0) {
	    if (e -> fixed || e -> vA -> contbound -> cont != e -> vB -> contbound -> cont) {
		if (dbformat)
		    fprintf (fp_out, "%s %ld %ld %ld %ld\n", (e -> fixed ? "solid" : "dashed"),
			L(e -> vA -> x), L(e -> vA -> y), L(e -> vB -> x), L(e -> vB -> y));
		else
		    fprintf (fp_out, "line %s from %ld,%ld to %ld,%ld\n", (e -> fixed ? "solid" : "dashed"),
			L(e -> vA -> x), L(e -> vA -> y), L(e -> vB -> x), L(e -> vB -> y));
	    }
	    e -> flag = 1;
	    if (e -> vA == v)
		drawMeshEdgesOf (fp_out, e -> vB, dbformat);
	    else
		drawMeshEdgesOf (fp_out, e -> vA, dbformat);
	}
	e = NEXT_E (e, v);
    }
}

void scanPrintInfo (FILE *fp)
{
}
