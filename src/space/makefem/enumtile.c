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

#include "src/space/makefem/define.h"
#include "src/space/makefem/extern.h"

//#define DEBUG

static int tileSubBB;
static int tileSubCnt;
static int scGroupCnt;

subcont_t *subcont_list_begin;
subcont_t *subcont_list_end;
subcont_t *subcont_list_begin2;
subcont_t *subcont_list_end2;
static subcont_t *subcont_list_free;
static subcontRef_t *scRef_free;
static scCoor_t *points_free;

void initSubstr ()
{
    tileSubBB = 0;
    tileSubCnt = scGroupCnt = 0;
}

void endSubstr ()
{
    subcontRef_t *scRef;
    subcont_t *sc;

    if (subcont_list_begin2) {
	if (subcont_list_end)
	    subcont_list_end -> next = subcont_list_begin2;
	else
	    subcont_list_begin = subcont_list_begin2;
	subcont_list_end = subcont_list_end2;
	subcont_list_begin2 = subcont_list_end2 = 0;
    }

    for (sc = subcont_list_begin; sc; sc = sc -> next) {
	scRef = sc -> subcontRefs;
	ASSERT (scRef);
	while (scRef -> nextRef) scRef = scRef -> nextRef;
	scRef -> nextRef = scRef_free;
	scRef_free = sc -> subcontRefs;
    }

    if (subcont_list_end) {
	ASSERT (subcont_list_begin);
	subcont_list_end -> next = subcont_list_free;
	subcont_list_free = subcont_list_begin;
	subcont_list_begin = 0;
	subcont_list_end = 0;
    }
    else ASSERT (!subcont_list_begin);
}

subcontRef_t * subContNew (tile_t *tile)
{
    subcontRef_t *scRef;
    subcont_t *sc;

    if ((scRef = scRef_free)) scRef_free = scRef -> nextRef;
    else scRef = NEW (subcontRef_t, 1);
    tile -> subcont = scRef;
    scRef -> causing_con = tile -> cc & 0xff;
    scRef -> distributed = tile -> cc & 0x200;
    scRef -> nextRef = NULL;
    scRef -> nr = ++tileSubCnt;

    ++tileConCnt;
    if ((sc = subcont_list_free)) subcont_list_free = sc -> next;
    else sc = NEW (subcont_t, 1);
    scRef -> subcontInfo = sc;

    sc -> subcontRefs = scRef;
    sc -> xl = tile -> xl;
    sc -> yb = tile -> bl;
    sc -> boundaries = 0;

    //  subContGroupNew (sc);
    sc -> group = ++scGroupCnt;
    sc -> prevGroup = NULL;
    sc -> nextGroup = NULL;

    //  add2subcont_list (sc);
    if (subcont_list_end) {
	ASSERT (subcont_list_end -> xl < sc -> xl ||
	    (subcont_list_end -> xl == sc -> xl && subcont_list_end -> yb <= sc -> yb));
	subcont_list_end -> next = sc;
    }
    else subcont_list_begin = sc;
    sc -> prev = subcont_list_end;
    sc -> next = 0;
    subcont_list_end = sc;

    return (scRef);
}

void rmFromSubcont_list (subcont_t *sc)
{
    if (sc -> next) sc -> next -> prev = sc -> prev;
    else subcont_list_end = sc -> prev;
    if (sc -> prev) sc -> prev -> next = sc -> next;
    else subcont_list_begin = sc -> next;
}

void subContGroupDel (subcont_t *sc)
{
    if (sc -> prevGroup) sc -> prevGroup -> nextGroup = sc -> nextGroup;
    if (sc -> nextGroup) sc -> nextGroup -> prevGroup = sc -> prevGroup;
}

void subContGroupJoin (subcont_t *scA, subcont_t *scB)
{
    subcont_t *sc, *scN;
    int gA = scA -> group;
    int gB = scB -> group;

    if (gA == gB) return;
    if (gB < gA) { sc = scB; scB = scA; scA = sc; gA = gB; }
    scN = scA -> nextGroup;
    sc = scB; sc -> group = gA;
    while (sc -> prevGroup) { sc = sc -> prevGroup; sc -> group = gA; }
    sc -> prevGroup = scA; scA -> nextGroup = sc;
    sc = scB;
    while (sc -> nextGroup) { sc = sc -> nextGroup; sc -> group = gA; }
    if (scN) { sc -> nextGroup = scN; scN -> prevGroup = sc; }
}

void subContJoin (subcont_t *scA, subcont_t *scB)
{
    subcontRef_t *scRef, *lastR;

    if (scA == scB) return;

    if (scB -> xl < scA -> xl || (scB -> xl == scA -> xl && scB -> yb < scA -> yb)) {
	Swap (subcont_t *, scA, scB);
    }
    ASSERT (!scB -> boundaries);

    subContGroupJoin (scA, scB);
    subContGroupDel (scB);

    lastR = NULL;
    for (scRef = scB -> subcontRefs; scRef; scRef = scRef -> nextRef) {
	scRef -> subcontInfo = scA;
	lastR = scRef;
    }
    ASSERT (lastR);
    lastR -> nextRef  = scA -> subcontRefs;
    scA -> subcontRefs = scB -> subcontRefs;

    --tileConCnt;
    rmFromSubcont_list (scB);
    scB -> next = subcont_list_free;
    subcont_list_free = scB;
}

void merge_boundaries (subcontRef_t *ct, subcontRef_t *cn, coor_t x1, coor_t y1, coor_t x2, coor_t y2)
{
    subcont_t *sct, *scn;
    scBound_t *bcn, *bct;
    scCoor_t *pb, *pt, *pn, *pnend;

    ASSERT (x1 == x2); // 'v'

    sct = ct -> subcontInfo;
    scn = cn -> subcontInfo;
    if (sct == scn) return;
    if (scn -> xl < sct -> xl || (scn -> xl == sct -> xl && scn -> yb < sct -> yb)) {
	Swap (subcont_t *, sct, scn);
    }

    bcn = scn -> boundaries;
    if (!bcn) return;

    pnend = pn = 0;
    bct = 0;

    // find (x1,y1) on scn
    do {
	pt = bcn -> pt;
	if (pt -> x == x1 && pt -> y == y1) {
f1:
	    pb = (pt == bcn -> pt)? bcn -> pb : bcn -> pt;
	    pnend = pt -> next;
	    pt -> next = points_free;
	    points_free = pt;
	    pn = pb;
	    while (pb -> next) pb = pb -> next;
	    if (!pnend) pnend = pb;
	    else {
		pt = swappoints (pnend);
		if (pt -> y != pb -> y) pb -> next = pt;
		else if (pt -> next) pb -> next = pt -> next;
		else pnend = pb;
	    }
	    break;
	}
	pt = bcn -> pb;
	if (pt -> x == x1 && pt -> y == y1) goto f1;
    } while ((bcn = (bct = bcn) -> next));

    ASSERT (bcn);
    if (bct) bct -> next = bcn -> next;
    else scn -> boundaries = bcn -> next;

    if (pn) { // find (x1,y1) on sct
	bct = sct -> boundaries;
	ASSERT (bct);
	do {
	    pb = bct -> pb;
	    if (pb -> x == x1 && pb -> y == y1) {
		bct -> pb = pn;
		pnend -> next = pb;
		break;
	    }
	    pt = bct -> pt;
	    if (pt -> x == x1 && pt -> y == y1) {
		bct -> pt = pn;
		pnend -> next = pt;
		break;
	    }
	} while ((bct = bct -> next));
	ASSERT (bct);
    }

    if (scn -> boundaries) {
	while (bct -> next) bct = bct -> next;
	bct -> next = scn -> boundaries;
	scn -> boundaries = 0;
    }
}

#define NEW_POINT(p) {\
    if (!(p = points_free)) p = NEW (scCoor_t, 1);\
    else points_free = points_free -> next;\
}

scBound_t *init_boundary (coor_t x1, coor_t y1, coor_t x2, coor_t y2)
{
    scCoor_t *p1, *p2;
    scBound_t *bnd = NEW (scBound_t, 1);
    NEW_POINT (p1); p1 -> x = x1; p1 -> y = y1; p1 -> next = 0;
    NEW_POINT (p2); p2 -> x = x2; p2 -> y = y2; p2 -> next = 0;
    ASSERT (x1 == x2); // 'v'
    bnd -> pb = p1;
    bnd -> pt = p2;
    bnd -> next = 0;
    return (bnd);
}

void update_boundary (subcontRef_t *cn, coor_t x1, coor_t y1, coor_t x2, coor_t y2)
{
    scBound_t *bnd;
    scCoor_t *p, *pn;

    bnd = cn -> subcontInfo -> boundaries;

    if (x1 == x2) { // 'v'
l1:
	p = bnd -> pt;
	if (p -> x == x1 && (p -> y == y1 || p -> y == y2)) {
l2:
	    if (y1 == y2) goto ret;
	    if (p -> y == y2) y2 = y1;
	    if (!p -> next || p -> next -> x != p -> x) {
		NEW_POINT (pn);
		pn -> x = x2;
		pn -> y = y2;
		pn -> next = p;
		if (p == bnd -> pt)
		    bnd -> pt = pn;
		else
		    bnd -> pb = pn;
	    }
	    else p -> y = y2;
	    goto ret;
	}
	p = bnd -> pb;
	if (p -> x == x1 && (p -> y == y1 || p -> y == y2)) goto l2;
	if (!bnd -> next) {
	    bnd -> next = init_boundary (x1, y1, x2, y2);
	    goto ret;
	}
	bnd = bnd -> next;
	goto l1;
    }
    else { // 'h'
	coor_t len = x2 - x1;
l3:
	p = bnd -> pb;
	if (p -> x == x1 && p -> y == y1) {
l4:
	    if (p -> next) {
		x1 = (y2 - y1) * (p -> x - p -> next -> x);
		y1 = len * (p -> y - p -> next -> y);
	    }
	    else { y1 = len; x1 = 0; }
	    if (y1 != x1) {
		NEW_POINT (pn);
		pn -> next = p;
		if (p == bnd -> pb)
		    bnd -> pb = pn;
		else
		    bnd -> pt = pn;
		p = pn;
	    }
	    p -> x = x2;
	    p -> y = y2;
	    goto ret;
	}
	p = bnd -> pt;
	if (p -> x == x1 && p -> y == y1) goto l4;
	bnd = bnd -> next;
	ASSERT (bnd);
	goto l3;
    }
ret:;
#ifdef DEBUG
    fprintf (stderr, "update_boundary:\n");
    bnd = cn -> subcontInfo -> boundaries;
    p = bnd -> pt;
    do { fprintf (stderr, "pt: %d,%d\n", p->x, p->y); } while (p = p -> next);
    p = bnd -> pb;
    do { fprintf (stderr, "pb: %d,%d\n", p->x, p->y); } while (p = p -> next);
#endif
}

void enumPair (tile_t *tile, tile_t *newt, int edgeOrien)
{
    coor_t x1, x2, y1, y2, len;
    subcontRef_t *cn, *ct;

    ct = tile -> subcont;
    cn = newt -> subcont;
    if (!cn && IS_COLOR (&newt -> color)) cn = subContNew (newt);

    if (edgeOrien == 'v') {
	x1 = x2 = tile -> xr;
	y1 = Max (tile -> br, newt -> bl);
	y2 = Min (tile -> tr, newt -> tl);
	len = y2 - y1;
    }
    else { // 'h'
	if (tile -> xr < newt -> xr) {
	       x2 = tile -> xr; y2 = tile -> tr; }
	else { x2 = newt -> xr; y2 = newt -> br; }
	if (tile -> xl > newt -> xl) {
	       x1 = tile -> xl; y1 = tile -> tl; }
	else { x1 = newt -> xl; y1 = newt -> bl; }
	len = x2 - x1;
    }

#ifdef DEBUG
#define D (double)
    fprintf (stderr, "enumPair(%c): %g,%g %g,%g len=%g (ct=%p cn=%p)\n",
	edgeOrien,D x1/4,D y1/4,D x2/4,D y2/4,D len/4, ct, cn);
#endif

    if (!cn) {
	if (!ct) return;
	ASSERT (ct -> subcontInfo -> boundaries);
	update_boundary (ct, x1, y1, x2, y2);
	return;
    }
    if (!ct) {
	if (!cn -> subcontInfo -> boundaries) {
	    ASSERT (edgeOrien == 'v');
	    cn -> subcontInfo -> boundaries = init_boundary (x1, y1, x2, y2);
	    return;
	}
	update_boundary (cn, x1, y1, x2, y2);
	return;
    }

    if (len > 0) {
	if (!ct -> distributed && !cn -> distributed
		&& ct -> causing_con == cn -> causing_con) {
	    merge_boundaries (ct, cn, x1, y1, x2, y2);
	    subContJoin (ct -> subcontInfo, cn -> subcontInfo);
	}
	else {
	    update_boundary (ct, x1, y1, x2, y2);
	    if (!cn -> subcontInfo -> boundaries) {
		ASSERT (edgeOrien == 'v');
		cn -> subcontInfo -> boundaries = init_boundary (x1, y1, x2, y2);
	    }
	    else
		update_boundary (cn, x1, y1, x2, y2);
	    subContGroupJoin (ct -> subcontInfo, cn -> subcontInfo);
	}
    }
    else if (!cn -> subcontInfo -> boundaries) {
	ASSERT (edgeOrien == 'v');
	cn -> subcontInfo -> boundaries = init_boundary (x1, y1, x2, y2);
    }
}

void enumTile (tile_t *tile)
{
    ++tileCnt;
    if (tile -> subcont) {
	if (!tileSubBB) {
	    tileSubBB = 1;
	    c_xl = tile -> xl;
	    c_xr = tile -> xr;
	    c_yb = tile -> bl < tile -> br ? tile -> bl : tile -> br;
	    c_yt = tile -> tl > tile -> tr ? tile -> tl : tile -> tr;
	}
	else {
	    if (tile -> xr > c_xr) c_xr = tile -> xr;
	    if (tile -> bl < c_yb) c_yb = tile -> bl;
	    if (tile -> br < c_yb) c_yb = tile -> br;
	    if (tile -> tl > c_yt) c_yt = tile -> tl;
	    if (tile -> tr > c_yt) c_yt = tile -> tr;
	}
    }
    disposeTile (tile);
}
