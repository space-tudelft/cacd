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

#include <math.h>
#include <stdio.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"
#include "src/space/lump/export.h"
#include "src/space/scan/export.h"

#ifdef CONFIG_SPACE2
#define KEEPNODE(node) node -> term |= 2
#else
#define KEEPNODE(node) node -> term = 2
#endif

extern tileBoundary_t *bdr;
extern double compensate_lat_part;
extern elemDef_t **otherElemSpace2;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void updateLatEdgeCap (tile_t *prevTile, tile_t *tile, coor_t shadowLow, coor_t shadowHigh, int level);
Private double latcapval  (capElemDef_t *ced, double dx);
Private double edgecapval (capElemDef_t *ced, double dx);
Private void findNearestPointsH (tile_t *tile, coor_t x1, coor_t x2, nodePoint_t **p1, nodePoint_t **p2);
Private void findNearestPointsV (tile_t *tile, coor_t y1, coor_t y2, nodePoint_t **p1, nodePoint_t **p2);
Private void compensateEdgeCap (capElemDef_t *lced, double lcap,
				double d, int lcon,
				subnode_t *lsn1, subnode_t *lsn2,
				nodePoint_t *lp1, nodePoint_t *lp2,
				tile_t *tile, tile_t *tileAdj,
				double factor, elemDef_t **elem);
#ifdef __cplusplus
  }
#endif

static elemDef_t **startElem;
static tile_t *startTile, *secondTile;
static double Dy;
static int non45count;
static int v_mode;
static int useTL;

typedef struct eref {
    elemDef_t *el;
    struct eref *sb;
    struct eref *next;
} eref_t;

typedef struct tref {
    tile_t *tile;
    struct eref *el_l;
    struct tref *next;
} tref_t;

static tref_t *tile_list;

Private void warn_non45 ()
{
    if (non45count++ < 10) fprintf (stderr, "warning: updateLateralCap: non-45 degree layout!");
}

void updateLateralCap (tile_t *newerTile, tile_t *tile, elemDef_t **elem, int edgeOrien, int i)
{
    eref_t *el_l, *el_list;
    elemDef_t *el, *e;
    coor_t sL, sH;

    if (edgeOrien == 'v') {
	if (tile -> xl <= -INF) return;
	v_mode = 1;
	if (bdr -> y2 > bdr -> y1) {
	    sL = bdr -> y1; sH = bdr -> y2; }
	else {
	    sL = bdr -> y2; sH = bdr -> y1; }
	Dy = (double)bdr -> x2 - bdr -> x1;
	if (Dy) {
	    double Dx = (double)bdr -> y2 - bdr -> y1;
		 if (Dy == Dx) Dy = 1;
	    else if (-Dy == Dx) Dy = -1;
	    else { Dy = Dx / Dy; warn_non45 (); }
	}
    }
    else { /* 'h' */
	if (tile -> bl <= -INF) return;
	Dy = (double)bdr -> y2 - bdr -> y1;
	if (Dy) {
	    double Dx = (double)bdr -> x2 - bdr -> x1;
		 if (Dy == Dx) Dy = 1;
	    else if (-Dy == Dx) Dy = -1;
	    else { Dy /= Dx; warn_non45 (); }
	}
	v_mode = 0; sL = bdr -> x1; sH = bdr -> x2;
    }
    startElem = elem;
    startTile = newerTile;
    secondTile = tile;

    el_list = NULL;
    e = startElem[i++];
    while ((el = e)) {
	el_l = NEW (eref_t, 1);
	el_l -> el = el;
	el_l -> sb = NULL;
	el_l -> next = el_list;
	el_list = el_l;
	/* filter (remove) equal elements */
	while ((e = startElem[i++]) && e -> id == el -> id && e -> s.cap.pCon == el -> s.cap.pCon) ;
    }
    tile_list = NEW (tref_t, 1);
    tile_list -> next = NULL;
    tile_list -> el_l = el_list;

    updateLatEdgeCap (newerTile, tile, sL, sH, 0);
}

Private void print_tiles (tile_t *tile, tile_t *otherTile)
{
    fprintf (stderr, "tile: xl=%d xr=%d bl=%d br=%d tl=%d tr=%d\n", tile->xl/4, tile->xr/4, tile->bl/4, tile->br/4, tile->tl/4, tile->tr/4);
    tile = otherTile;
    fprintf (stderr, "otile:xl=%d xr=%d bl=%d br=%d tl=%d tr=%d\n", tile->xl/4, tile->xr/4, tile->bl/4, tile->br/4, tile->tl/4, tile->tr/4);
}

static elemDef_t **otherElem;

Private double VAL (double y, tile_t *t)
{
    if (v_mode) {
	double xl = t -> xl;
	if (y > t -> tl) return (xl + (y - t -> tl));
	if (y < t -> bl) return (xl + (t -> bl - y));
	return (xl);
    }
    else {
	double bl = t -> bl;
	if (t -> bl < t -> br) return (bl + (y - t -> xl));
	if (t -> bl > t -> br) return (bl - (y - t -> xl));
	return (bl);
    }
}

Private double calc_latcap (double y, eref_t *el_o, eref_t *el_l, double *d)
{
    tref_t *tref;
    double xb, x1, x2;
    double cap, currDist;

    tref = tile_list;

    xb = v_mode ? bdr->x1 : bdr->y1 + Dy * (y - bdr->x1);
    if (v_mode && bdr->x2 > bdr->x1) {
	if (y > bdr->y1)
	    xb += (y - bdr->y1);
	else
	    xb += (bdr->y1 - y);
    }

    x1 = VAL (y, tref -> tile);
    currDist = xb - x1;
    *d = currDist * meters;
    ASSERT (*d > 0);
    cap = latcapval (&(el_o -> el -> s.cap), *d);

    if (el_l && (tref = tref -> next)) {
	x2 = VAL (y, tref -> tile);
	cap *= (x2 - x1) / currDist;
	while (tref) {
	    el_o = el_o -> sb;
	    x1 = x2;
	    if ((tref = tref -> next)) x2 = VAL (y, tref -> tile);
	    else x2 = xb;
	    cap += (x2 - x1) / currDist * latcapval (&(el_o -> el -> s.cap), *d);
	}
    }
    return (cap);
}

Private void do_latcap (eref_t *el_o, tile_t *prevTile, tile_t *tile, coor_t shadowLow, coor_t shadowHigh)
{
    eref_t *el_l;
    elemDef_t *el;
    capElemDef_t *ced;
    double lcap, cap2, d, d2, factor, sH, sL, dy;
    coor_t x, y;

    el = el_o -> el;
    ced = &(el -> s.cap);
    ++el -> el_recog_cnt;

    sH = shadowHigh;
    sL = shadowLow;

    if (v_mode) {
	useTL = 0;
	d = bdr -> x1 - prevTile -> xl;
	if (shadowHigh > prevTile -> tl) {
	    if (Dy <= 0) {
		d -= (shadowLow - prevTile -> tl);
		if (Dy < 0) d += (bdr -> y1 - shadowLow);
		if (d > effectDist) {
		    if (Dy == 0) sL += d - effectDist;
		    else sL += (d - effectDist)/2;
		}
	    }
	}
	else if (shadowHigh > prevTile -> bl) {
	    if (Dy > 0) {
		d += (shadowHigh - bdr -> y1);
		if (d > effectDist) sH -= d - effectDist;
	    }
	    if (Dy < 0) {
		d += (bdr -> y1 - shadowLow);
		if (d > effectDist) sL += d - effectDist;
	    }
	}
	else {// shadowHigh <= prevTile -> bl
	    useTL = 1;
	    if (Dy >= 0) {
		d -= (prevTile -> bl - shadowHigh);
		if (Dy > 0) d += (shadowHigh - bdr -> y1);
		if (d > effectDist) {
		    if (Dy == 0) sH -= d - effectDist;
		    else sH -= (d - effectDist)/2;
		}
	    }
	}
    }
    else { // h_mode
	if (prevTile -> bl != prevTile -> br) {
	    dy = (prevTile -> bl < prevTile -> br)? 1 : -1;
	}
	else dy = 0;
	if (Dy != dy) {
	    d = bdr -> y1 - prevTile -> bl;
	    if (Dy > dy) {
		if (Dy > 0) d += sH - bdr -> x1;
		if (dy < 0) d += sH - prevTile -> xl;
		if (d > effectDist) {
		    if (Dy > 0 && dy < 0)
			sH -= (d - effectDist)/2;
		    else
			sH -= d - effectDist;
		}
	    }
	    else {
		if (Dy < 0) d -= sL - bdr -> x1;
		if (dy > 0) d -= sL - prevTile -> xl;
		if (d > effectDist) {
		    if (Dy < 0 && dy > 0)
			sL += (d - effectDist)/2;
		    else
			sL += d - effectDist;
		}
	    }
	}
    }

    /* We recognized a lateral capacitance with an id that has not been
	recognized before or that has a different connection. */

    sH -= sL;
    ASSERT (sH > 0);
    factor = sH * meters;

    el_l = el_o; /* are there different ids? */
    while ((el_l = el_l -> sb) && el_l -> el -> id == el -> id) ;
    /* if el_l == NULL, there are no different ids */

    sH /= 4;
    sL += sH;
    lcap = calc_latcap (sL, el_o, el_l, &d);
    sL += sH * 2;
    cap2 = calc_latcap (sL, el_o, el_l, &d2);
#ifdef DEBUG
    if (d != d2) {
	if (v_mode)
	    fprintf(stderr, "v: x1=%d x2=%d y1=%d y2=%d sL=%d sH=%d\n", bdr->x1/4, bdr->x2/4, bdr->y1/4, bdr->y2/4, shadowLow/4, shadowHigh/4);
	else
	    fprintf(stderr, "h: x1=%d x2=%d y1=%d y2=%d sL=%d sH=%d\n", bdr->x1/4, bdr->x2/4, bdr->y1/4, bdr->y2/4, shadowLow/4, shadowHigh/4);
	//return;
    }
#endif

    /* startTile will correspond to conductor 'p'
       and tile will correspond to conductor 'n'. */

    ASSERT (ced -> pOccurrence == EDGE); /* see gettech.cc */

    if (optRes) {
	subnode_t *subnA1, *subnA2, *subnB1, *subnB2;
	nodePoint_t *pA1, *pA2, *pB1, *pB2;

	if (v_mode) {
	    findNearestPointsV (startTile, shadowLow, shadowHigh, &pA1, &pA2);
	    findNearestPointsV (tile, shadowLow, shadowHigh, &pB1, &pB2);
	}
	else { /* 'h' */
	    findNearestPointsH (startTile, shadowLow, shadowHigh, &pA1, &pA2);
	    findNearestPointsH (tile, shadowLow, shadowHigh, &pB1, &pB2);
	}

	subnA1 = pA1 ? pA1 -> cons[ced -> pCon] : NULL;
	if (!subnA1) {
	    if (v_mode) { x = startTile -> xl; y = shadowLow; }
	    else { x = shadowLow; y = startTile -> bl; if (Dy) y += Dy * ((double)x - startTile -> xl); }
	    missingCon (ced -> pMask, ced -> pOccurrence, NULL, startTile, tile, el, x, y); /* die */
	}
	subnA2 = pA2 -> cons[ced -> pCon];
	ASSERT (subnA2);

	subnB1 = pB1 ? pB1 -> cons[ced -> nCon] : NULL;
	if (!subnB1) {
	    if (v_mode) { x = tile -> xr; y = shadowLow; }
	    else { x = shadowLow; y = tile -> tl; if (Dy) y += Dy * ((double)x - tile -> xl); }
	    missingCon (ced -> nMask, ced -> nOccurrence, NULL, startTile, tile, el, x, y); /* die */
	}
	subnB2 = pB2 -> cons[ced -> nCon];
	ASSERT (subnB2);

	if (ced -> pKeep) {
	    KEEPNODE (subnA1 -> node);
	    KEEPNODE (subnA2 -> node);
	}
	if (ced -> nKeep) {
	    KEEPNODE (subnB1 -> node);
	    KEEPNODE (subnB2 -> node);
	}

	factor /= 2;
	if (lcap) capAdd (subnA1, subnB1, lcap * factor, ced -> sortNr);
	if (cap2) capAdd (subnA2, subnB2, cap2 * factor, ced -> sortNr);

	/* And subtract 'lcap' from the edge capacitances
	   that are connected to tile edges of subnA and subnB */

	if (compensate_lat_part > 0) {
	    lcap += cap2; lcap /= 2; if (!lcap) return;
	    /* WARNING: startElem must be in otherElemSpace */
	    if (!otherElem) otherElem = recogEdge (prevTile, tile, otherElemSpace2);
	    d += d2; d /= 2;
	    compensateEdgeCap (ced, lcap, d, ced -> pCon, subnA1, subnA2, pA1, pA2, startTile, secondTile, factor, startElem);
	    compensateEdgeCap (ced, lcap, d, ced -> nCon, subnB1, subnB2, pB1, pB2, tile, prevTile, factor, otherElem);
	}
    }
    else
    {
	subnode_t *subnA, *subnB;
	if (!(subnA = startTile -> cons[ced -> pCon]))
	    missingCon (ced -> pMask, ced -> pOccurrence, NULL, startTile, tile, el, bdr -> x1, bdr -> y1); /* die */

	if (!(subnB = tile -> cons[ced -> nCon]))
	    missingCon (ced -> nMask, ced -> nOccurrence, /* die */
		NULL, startTile, tile, el,
		v_mode ? tile -> xr : shadowLow,
		v_mode ? shadowLow : (coor_t)(tile -> tl + Dy * ((double)shadowLow - tile -> xl)));

	if (ced -> pKeep) KEEPNODE (subnA -> node);
	if (ced -> nKeep) KEEPNODE (subnB -> node);

	lcap += cap2; lcap /= 2; if (!lcap) return;
	capAdd (subnA, subnB, lcap * factor, ced -> sortNr);

	/* And subtract 'lcap' from the edge capacitances
	   that are connected to tile edges of subnA and subnB */

	if (compensate_lat_part > 0) {
	    /* WARNING: startElem must be in otherElemSpace */
	    if (!otherElem) otherElem = recogEdge (prevTile, tile, otherElemSpace2);
	    d += d2; d /= 2;
	    compensateEdgeCap (ced, lcap, d, ced -> pCon, subnA, NULL, NULL, NULL, startTile, secondTile, factor, startElem);
	    compensateEdgeCap (ced, lcap, d, ced -> nCon, subnB, NULL, NULL, NULL, tile, prevTile, factor, otherElem);
	}
    }
}

Private elemDef_t * get_prev_el (elemDef_t *el)
{
    elemDef_t *e;
    if (el != elemDefTab)
    for (e = el - 1; e -> id == el -> id && e -> s.cap.pCon == el -> s.cap.pCon; --e) {
        if (COLOR_PRESENT (&startTile -> color, &e -> eBitPresent)
         && COLOR_ABSENT  (&startTile -> color, &e -> eBitAbsent)) return (e);
	if (e == elemDefTab) break;
    }
    return (NULL);
}

Private elemDef_t * get_next_el (elemDef_t *el)
{
    elemDef_t *e;
    for (e = el + 1; e -> id == el -> id && e -> s.cap.pCon == el -> s.cap.pCon; ++e) {
        if (COLOR_PRESENT (&startTile -> color, &e -> eBitPresent)
         && COLOR_ABSENT  (&startTile -> color, &e -> eBitAbsent)) return (e);
    }
    return (NULL);
}

Private elemDef_t * match_oe (elemDef_t *el, tile_t *tile)
{
    capElemDef_t *ced;
    elemDef_t *e = el;
    while (e) {
	ced = &(e -> s.cap);
	if (COLOR_PRESENT (&tile -> color, &ced -> oBitPresent)
	 && COLOR_ABSENT  (&tile -> color, &ced -> oBitAbsent)) return (e);
	if (!el) e = get_prev_el (e);
	else if (!(e = get_next_el (e))) { e = get_prev_el (el); el = NULL; }
    }
    return (e);
}

Private elemDef_t * match_surf (elemDef_t *el, tile_t *tile)
{
    capElemDef_t *ced, *mced;
    elemDef_t *e = el;
    int prev = 0;

    while (e) {
	ced = &(e -> s.cap);
	if (COLOR_PRESENT (&tile -> color, &ced -> sBitPresent)
	 && COLOR_ABSENT  (&tile -> color, &ced -> sBitAbsent)) return (e);
	if (prev) e = get_prev_el (e);
	else if (!(e = get_next_el (e))) { e = get_prev_el (el); prev = 1; }
    }

    mced = &(el -> s.cap);

    /* try to find another latcap */
    if ((e = hasLatCap (tile, startTile))) {
	do { // forall latcaps
	    ced = &e -> s.cap;
	    if (ced -> pCon == mced -> pCon &&
		ced -> nCon == mced -> nCon &&
		ced -> sortNr == mced -> sortNr) return (e);
	} while ((e = nextLatCap ()));
    }
    return (NULL);
}

/* Latcap algorithm
 *
 * Definition: A tile is lc-transparent with respect to a latcap element
 * if it (based on its color) does not shield the fieldlines between
 * the two tile edges.
 *
 * enumpair performs the following algorithm.
 * for each latcap element
 *     if the tile to the left or below the current tile is
 *         lc-transparent, recursively continue until
 *         either the tile contains the latcap element
 *         or it is too far.
 *
 * L-transparency is checked with 2 bitmasks:
 *     absent  -> these color bits must be off.
 *     present -> these color bits must be on.
 * thus:
 *     ((color & present) == present) && ((color & absent) = 0)
 *
 * Only exactly parallel edges can have lateral coupling capacitances.
 *
 * Args:	(see also drawing below)
 *	startTile  -- tile with edge mask
 *	secondTile -- tile adjacent to startTile
 *	prevTile   -- previous tile
 *	tile       -- current tile
 *                                   | |
 *               +-------------------+ +--------------------+
 *                                   | |
 *
 *     +---------+-----------+---- ....... ----+------------+-----------+
 *     |         |           |                 |            |           |
 *     |  =mask  |           |                 |            |   -mask   |
 *     |  tile   | prevTile  |                 | secondTile | startTile |
 *     |         |           |                 |            |           |
 *     +---------+-----------+---- ....... ----+------------+-----------+
 */
Private void updateLatEdgeCap (tile_t *prevTile, tile_t *tile,
	coor_t shadowLow, coor_t shadowHigh, int level)
{
    tref_t *tref;
    eref_t *el_list, *el_l, *el_o;
    elemDef_t *el;
    double d, dl, dy;
    tile_t *otherTile;

    ASSERT (shadowHigh > shadowLow);

    tref = tile_list;

    /* Lateral coupling capacitances are only detected for parallel wires !!!
       E.g. for wires both 0 degrees, both 90 degress or both 45 degrees !!!
    */
    if (level > 0) {
	el_list = NULL;
	if (COLOR_EQ_COLOR (&tile -> color, &prevTile -> color)) {
	    for (el_o = tref -> el_l; el_o; el_o = el_o -> next) {
		el_l = NEW (eref_t, 1);
		el_l -> el = el_o -> el;
		el_l -> sb = el_o;
		el_l -> next = el_list;
		el_list = el_l;
	    }
	}
	else {
	    otherElem = NULL;
	    for (el_o = tref -> el_l; el_o; el_o = el_o -> next) { /* for all to invest lateral caps */
		if (match_oe (el_o -> el, tile)) {
		    do_latcap (el_o, prevTile, tile, shadowLow, shadowHigh);
		}
		else if ((el = match_surf (el_o -> el, tile))) {
		    el_l = NEW (eref_t, 1);
		    el_l -> el = el;
		    el_l -> sb = el_o;
		    el_l -> next = el_list;
		    el_list = el_l;
		}
	    }
	}
	if (!el_list) return;
	tref = NEW (tref_t, 1);
	tref -> el_l = el_list;
	tref -> next = tile_list;
	tile_list = tref;
    }

    tref -> tile = tile;

    /* This was not the last tile to be investigated.
     * Move left or down.
     */
    if (v_mode) { /* edgeOrien == 'v' */
	coor_t yL, yH, y;

	yL = Max (shadowLow,  tile -> bl);
	yH = Min (shadowHigh, tile -> tl);

	dl = (double)bdr -> x1 - tile -> xl;

	if (yH > yL && dl <= effectDist) {

	    if (Dy > 0) {
		d = dl + (yL - bdr -> y1);
		y = (coor_t)(effectDist - d);
		if (y <= 0) goto do_bl;
		if (yL + y < yH) yH = yL + y;
	    }
	    if (Dy < 0) {
		d = dl + (bdr -> y1 - yH);
		y = (coor_t)(effectDist - d);
		if (y <= 0) goto do_bl;
		if (yH - y > yL) yL = yH - y;
	    }

	    /* update everyting between y = tile -> bl and y = tile -> tl */

	    otherTile = tile -> stl; /* stitch-left is only valid if otherTile->xr == tile->xl */
	    ASSERT (otherTile); /* there is always a stitch-left */
	    if (otherTile -> xl > -INF)
	    while (otherTile -> xr == tile -> xl && otherTile -> br < yH) {
		if ((y = otherTile -> tr) > yH) y = yH;
		if (y > yL) {
		    updateLatEdgeCap (tile, otherTile, yL, y, 1);
		    if ((yL = y) == yH) break;
		}
		otherTile = otherTile -> stt;
		if (!otherTile) {
say ("warning: latcap missing otherTile: v-edge, bdr(x1=%d x2=%d y1=%d y2=%d), yL=%d yH=%d\n",
bdr -> x1/4, bdr -> x2/4, bdr -> y1/4, bdr -> y2/4, yL/4, yH/4);
		    break;
		}
	    }
	}

do_bl:
	if (tile -> bl > tile -> br && tile -> bl > shadowLow) {

	    /* tile has a slanting bottom edge with right end lower than left end;
	       also update everything between y = tile -> br and y = tile -> bl */

	    yL = shadowLow;
	    yH = Min (shadowHigh, tile -> bl);
	    dy = -1;

	    otherTile = tile -> stb;
	    while (otherTile && otherTile -> tl > yL) {
		if (otherTile -> xl >= tile -> xr) {
		    print_tiles (tile, otherTile);
		    ASSERT (otherTile -> xl < tile -> xr);
		}
		if ((y = otherTile -> tr) == INF || y < yL) y = yL;
		if (y < yH) {
		    d = dl;
		    d -= tile -> bl - y;
		    if (Dy < 0) d += bdr -> y1 - y;
		    if (Dy > 0) d += y - bdr -> y1;
		    if (d < effectDist || (d == effectDist && Dy == dy))
			updateLatEdgeCap (tile, otherTile, y, yH, 1);
		    if ((yH = y) == yL) break;
		}
		otherTile = otherTile -> str;
	    }
	    if (!otherTile) {
say ("warning: latcap missing otherTile: v-edge, bdr(x1=%d x2=%d y1=%d y2=%d), yL=%d yH=%d\n",
bdr -> x1/4, bdr -> x2/4, bdr -> y1/4, bdr -> y2/4, yL/4, yH/4);
	    }
	}

	if (tile -> tr > tile -> tl && tile -> tl < shadowHigh) {

	    /* tile has a slanting top edge with left end lower than right end;
	       also update everything between y = tile -> tl and y = tile -> tr */

	    yL = Max (shadowLow,  tile -> tl);
	    yH = shadowHigh;
	    dy = 1;

	    otherTile = tile -> stt;
	    while (otherTile && otherTile -> br > yL) {
		if (otherTile -> xr <= tile -> xl) {
		    print_tiles (tile, otherTile);
		    ASSERT (otherTile -> xr > tile -> xl);
		}
		if ((y = otherTile -> bl) < yL) y = yL;
		if (y < yH) {
		    d = dl;
		    d -= yH - tile -> tl;
		    if (Dy < 0) d += bdr -> y1 - yH;
		    if (Dy > 0) d += yH - bdr -> y1;
		    if (d >= effectDist && (d > effectDist || Dy != dy)) break;
		    updateLatEdgeCap (tile, otherTile, y, yH, 1);
		    if ((yH = y) == yL) break;
		}
		otherTile = otherTile -> stl;
	    }
	    if (!otherTile) {
say ("warning: latcap missing otherTile: v-edge, bdr(x1=%d x2=%d y1=%d y2=%d), yL=%d yH=%d\n",
bdr -> x1/4, bdr -> x2/4, bdr -> y1/4, bdr -> y2/4, yL/4, yH/4);
	    }
	}
    }
    else { /* edgeOrien == 'h' */
	coor_t xL, xH, xS;

	dy = (double)tile -> br - tile -> bl;
	if (dy) {
	    d = (double)tile -> xr - tile -> xl;
		 if (dy == d) dy = 1;
	    else if (-dy == d) dy = -1;
	    else { dy /= d; warn_non45 (); }
	}
	dl = (double)bdr -> y1 - tile -> bl;

	otherTile = tile -> stb;
	while (otherTile && otherTile -> xr <= shadowLow) otherTile = otherTile -> str;
	ASSERT (otherTile); // otherTile -> xr > shadowLow
	xL = Max (shadowLow, otherTile -> xl);
	while (xL < shadowHigh) {
	    if ((xH = otherTile -> xr) > shadowHigh) xH = shadowHigh;
	    d = dl;
	    if (dy != Dy) { /* calc. shortest new distance */
		xS = (dy > Dy)? xH : xL;
		if (Dy) d += Dy * (xS - bdr -> x1);
		if (dy) d -= dy * (xS - tile-> xl);
	    }
	    if (d >= effectDist) {
		if (dy > Dy) goto skip2;
		if (dy < Dy || d > effectDist) goto ret;
	    }
	    if (otherTile -> bl > -INF) updateLatEdgeCap (tile, otherTile, xL, xH, 1);
skip2:
	    if ((xL = xH) == shadowHigh) break;
	    otherTile = otherTile -> str;
	    if (!otherTile) {
say ("warning: latcap missing otherTile: h-edge, bdr(x1=%d x2=%d y1=%d y2=%d), xL=%d xH=%d\n",
bdr -> x1/4, bdr -> x2/4, bdr -> y1/4, bdr -> y2/4, xL/4, xH/4);
		break;
	    }
	}
    }

ret:
    el_l = tref -> el_l;
    while ((el_o = el_l)) {
	el_l = el_o -> next;
	DISPOSE (el_o, 1);
    }
    tile_list = tref -> next;
    DISPOSE (tref, 1);
}

Private double latcapval (capElemDef_t *ced, double dx)
{
    xy_abp_t *xy_el = ced -> mval;

    if (!xy_el) return (ced -> val / dx);

    while (xy_el -> next && dx >= xy_el -> next -> x) xy_el = xy_el -> next;

    if (dx == xy_el -> x) return (xy_el -> y);

    return (xy_el -> a * pow (dx, -(xy_el -> p)) + xy_el -> b);
}

Private double edgecapval (capElemDef_t *ced, double dx)
{
    xy_abp_t *xy_el = ced -> mval;

    /* Table of values is defined as a function of dx. */

    while (xy_el -> next && dx >= xy_el -> next -> x) xy_el = xy_el -> next;

    /* General interpolation function: Ce = Cmax * (1 - exp (-dx * p))
     * i.e. Ce=0 for dx=0; Ce=Cmax for dx->infty; Ce=0.5*Cmax for dx=0.7/p
     */
    return (xy_el -> a * (1.0 - xy_el -> b * exp (-dx * xy_el -> p)));
}

Private void findNearestPointsH (tile_t *tile, coor_t x1, coor_t x2, nodePoint_t **p1, nodePoint_t **p2)
{
    nodePoint_t *p;
    int followPrev;
    coor_t x;

    if (tile == startTile) { /* use points at the bottom edge */
	*p2 = p = tile -> rbPoints;
	*p1 = p -> next ? p -> next : p;
	return;
    }
    /* investigate points at the top edge of the tile */
    *p1 = NULL;
    if (!(p = tile -> tlPoints)) return; /* no conductor */
    *p2 = p;
    if ((x = p -> x) <= x1) { *p1 = p; return; } /* low-res conductor, only one point */
    followPrev = p -> prev ? 1 : 0;
    while ((p = followPrev ? p -> prev : p -> next)) {
	ASSERT (p -> x < x); /* left movement */
	x = p -> x;
	if (x >= x2) { *p2 = p; }
	else
	if (x <= x1) { *p1 = p; return; }
    }
}

Private void findNearestPointsV (tile_t *tile, coor_t y1, coor_t y2, nodePoint_t **p1, nodePoint_t **p2)
{
    nodePoint_t *p;
    int followPrev;
    coor_t y;

    *p1 = NULL;
    if (tile == startTile) { /* investigate points at the left edge */
	if (Dy < 0) { /* slanting up movement */
	    *p1 = p = tile -> rbPoints;
	    *p2 = p -> next ? p -> next : p;
	    return;
	}
	p = tile -> tlPoints;
    } else { /* investigate points at the right edge of the tile */
	if (useTL) { /* slanting up movement */
	    if (!(p = tile -> tlPoints)) return; /* no conductor */
	    *p1 = p;
	    if (p -> x == tile -> xl) { *p2 = p; return; }
	    followPrev = p -> prev ? 1 : 0;
	    y = p -> y;
	    while ((p = followPrev ? p -> prev : p -> next)) {
		ASSERT (p -> y > y);
		y = p -> y;
		if (y <= y1) { *p1 = p; }
		else
		if (y >= y2) { *p2 = p; return; }
	    }
	    *p1 = NULL;
	    return;
	}
	if (!(p = tile -> rbPoints)) return; /* no conductor */
    }
    *p2 = p;
    if ((y = p -> y) <= y1 || !p -> next) { *p1 = p; return; } /* low-res conductor, only one point */
    while ((p = p -> next)) {
	ASSERT (p -> y < y); /* down movement */
	y = p -> y;
	if (y >= y2) { *p2 = p; }
	else
	if (y <= y1) { *p1 = p; return; }
    }
}

Private void compensateEdgeCap (capElemDef_t *lced, double lcap, double d, int lcon,
	subnode_t *lsn1, subnode_t *lsn2, nodePoint_t *lp1, nodePoint_t *lp2,
	tile_t *tile, tile_t *tileAdj, double factor, elemDef_t **elem)
{
    capElemDef_t *eced;
    int econ, k, k1, k2;
    subnode_t *esubn;
    coor_t x, y;
    nodePoint_t *ep;
    double compCap, ecap, totEdgeCap;
    tile_t *etile;

    /* First compute that total edge capacitance that is connected to
       conductor 'lcon'. */

    totEdgeCap = 0;
    k1 = k2 = -1;
    for (k = 0; elem[k]; k++) {
	if (elem[k] -> type == EDGECAPELEM) {
	    eced = &elem[k] -> s.cap;
	    if ((eced -> pCon == lcon || (eced -> nCon == lcon && eced -> nOccurrence == EDGE))
			&& eced -> sortNr == lced -> sortNr
	    ) {
		if (k1 < 0) k1 = k;
		k2 = k;
		totEdgeCap += eced -> val;
		eced -> done = 1;
	    }
	    else eced -> done = 0;
	}
    }
    if (k1 < 0) return;

    /* Second, compute compCap: the total compensation for the edge
       capacitances */

    compCap = lcap * compensate_lat_part;
    if (totEdgeCap < compCap) compCap = totEdgeCap;

    /* For each edge capacitance 'Cedge' connected to 'lcon',
       subtract (Cedge / totEdgeCap) * compCap */

    for (k = k1; k <= k2; k++) {
	    eced = &elem[k] -> s.cap;
	    if (eced -> done) {

                /* If distance,cap pairs are specified and lateral coup.
                   cap. is between two conductors of the same type, then
                   compensate based on distance,cap pairs.
		   If only one lateral cap. value is specified, then
		   compensate by subtracting a fraction of the edge cap. */

		ecap = 0;
		if (eced -> mval && lced -> nCon == lced -> pCon)
		    ecap = eced -> val - edgecapval (eced, d);
		if (ecap <= 0)
		    ecap = (eced -> val / totEdgeCap) * compCap;

		ecap *= factor;
		if (ecap <= 0) continue;

		/* Although it is unlikely to occur in practice, we want to
		   make sure that we do not introduce negative capacitances
		   in initial RC network. Therefor we use capAddNEG. */

		if (eced -> nCon < 0) {
		    esubn = (eced -> nCon <= -2)? subnSUB : subnGND;
		    capAddNEG (lsn1, esubn, ecap, eced -> sortNr);
		    if (optRes) capAddNEG (lsn2, esubn, ecap, eced -> sortNr);
		}
		else {
		    if (eced -> pCon == lcon) {
			econ = eced -> nCon;
                        etile = (eced -> nOccurrence == EDGE)? tile : tileAdj;
		    }
		    else {
			econ = eced -> pCon;
                        etile = tile;
                    }
		    if (optRes) {
			ASSERT (etile -> rbPoints);

			/* Find point 'ep' in tile 'etile' that has the same
			   position as 'lp2'. */
			x = lp2 -> x;
			y = lp2 -> y;
			for (ep = etile -> rbPoints; ep; ep = ep -> next)
			    if (ep -> x == x && ep -> y == y) break;
			if (!ep) {
			    for (ep = etile -> tlPoints; ep; ep = ep -> next)
				if (ep -> x == x && ep -> y == y) break;
			    if (!ep) ep = etile -> rbPoints;
			}
			esubn = ep -> cons[econ];
			ASSERT (esubn);
			capAddNEG (lsn2, esubn, ecap, eced -> sortNr);

			/* Find point 'ep' in tile 'etile' that has the same
			   position as 'lp1'. */
			x = lp1 -> x;
			y = lp1 -> y;
			for (ep = etile -> rbPoints; ep; ep = ep -> next)
			    if (ep -> x == x && ep -> y == y) break;
			if (!ep) {
			    for (ep = etile -> tlPoints; ep; ep = ep -> next)
				if (ep -> x == x && ep -> y == y) break;
			    if (!ep) ep = etile -> rbPoints;
			}
			esubn = ep -> cons[econ];
		    }
		    else
			esubn = etile -> cons[econ];
		    ASSERT (esubn);
		    capAddNEG (lsn1, esubn, ecap, eced -> sortNr);
		}
	    }
    }
}
