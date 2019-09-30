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
#include <string.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/auxil/debug.h"
#include "src/space/include/type.h"
#ifdef PLOT_CIR_MODE
#include "src/space/auxil/plot.h"
#endif
#include "src/space/scan/export.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"
#include "src/space/lump/define.h"
#include "src/space/lump/export.h"

#ifdef DISPLAY
#include "src/space/X11/export.h"
#endif

#ifdef CAP3D
#include "src/space/spider/export.h"
#endif

#include "src/space/bipolar/define.h"
#include "src/space/bipolar/export.h"

#define POS_TL   1
#define POS_TR   2
#define POS_BL   3
#define POS_BR   4

#define tNull (tile_t *)0
#define D(x) (double)(x)

#define conLR(tile) (tile -> known & (1+2))
#define conLR1(tile) (tile -> known & 1)
#define conLR2(tile) (tile -> known & 2)
#define conLR25(tile) ((tile -> known & 514) == 514)
#define conTB(tile) (tile -> known & (4+8))
#define conTB1(tile) (tile -> known & 4)
#define conTB2(tile) (tile -> known & 8)

extern mask_t resBitmask;
#define HasResC(color) !COLOR_ABSENT (&color, &resBitmask)

extern int   *helpArray;
extern int    equi_line_new;
extern int    equi_line_old;
extern double use_corner_ratio;
extern double min_res;

static elemDef_t **elem;
static tile_t * currTile;
static int doSurfCap;
static int equiMode;
static int cap_i1, cap_i2;
static int cnt_i1, cnt_i2;
static double surface;

static int extraPoints, hPoint, vPoint;
static int firstA, firstB, lastA, lastB;
nodePoint_t *pTR, *pBR, *pBL, *pTL;
static nodePoint_t *pTRb;
static nodePoint_t *pf1S, *pf2S, *pbS;
int corner_line_node = 0;
int contact_join = 0;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
extern  void contTile (tile_t *tile);
Private void pp0EnumTile (tile_t *tile);
Private void resEnumTile (tile_t *tile);
Private void triangular (tile_t *tile);
Private void triangular_core (nodePoint_t *rbP);
Private int onSameLine (nodePoint_t *p1, nodePoint_t *p2, nodePoint_t *p3);
Private double edgeLength (nodePoint_t *p1, nodePoint_t *p2);
Private void tryTriangle (nodePoint_t *pf1, nodePoint_t *pf2, nodePoint_t *pb);
Private void doRectangle (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP);
Private void doEquiRectangle (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP);
Private void doTriangle (nodePoint_t *p1, nodePoint_t *p2, nodePoint_t *p3);
Private nodePoint_t * newNodePoint (coor_t x, coor_t y, nodePoint_t *point);
Private int RsideOK (coor_t w, mask_t co);
Private int LsideOK (coor_t w, mask_t co);
void missingSubArea (int mask, nodePoint_t *p, elemDef_t *el);
Private void parPlateCap (nodePoint_t *pA, nodePoint_t *pB, elemDef_t *el, double surf);
Private int makeContact (nodePoint_t *p, int position, elemDef_t *el, double surf, double contval);
#ifdef __cplusplus
  }
#endif

Private void missingContact (int n, elemDef_t *el, coor_t x, coor_t y)
{
    static int wcnt = 0;
    if (++wcnt > 4) return;
    n = (n == 1)? el -> s.cont.mask1 : el -> s.cont.mask2;
    say ("warning: missing subnode for conductor '%s'", masktable[n].name);
    say ("\tat position %s (contact '%s' skipped).", strCoorBrackets (x, y), el -> name);
}

/* enumTile (tile)
 *
 * will update the network description for the influence
 * of the surface of 'tile'.
 *
 * actions performed :
 *
 * - A list of recognized elements is fetched for 'tile'.
 *
 * - For each contact element that is present in 'tile',
 *   joins are made between the subnodes of the
 *   corresponding conductor layers that are connected
 *   by the contact.
 *
 * - For a transistor that is present in 'tile', a subnode
 *   in the gate (conductor) layer is used as gate node.
 *   Also, surface information for the transistor is
 *   updated.
 *
 * - For each surface capacitance that is present in 'tile',
 *   capacitance values are calculated and assigned to
 *   the subnodes of the corresponding conductor layers.
 *
 * - When a resistance extraction is performed,
 *   a triangularization is done for 'tile' and resistances
 *   are calculated and assigned between the subnodes of each
 *   relevant conductor layer.
 */
void enumTile (tile_t *tile)
{
    register elemDef_t *el;
    register subnode_t *sn1, *sn2;
    int i, j, cx, cy;
    capElemDef_t *cap;

    i = HasConduc (tile);
#ifdef DISPLAY
    if (optDisplay && prePass != 1) drawTile (tile, i);
#endif
    if (!i) return;
    if (prePass) {
	if (prePass == 1 && bandWidth2) pp0EnumTile (tile);
	return;
    }

#ifdef CAP3D
    if (optCap3D && !optSubResSave) spiderTile (tile);
#endif

    surface = meters * meters
	* (D(tile -> xr) - D(tile -> xl))
	* (D(tile -> tl) + D(tile -> tr)
	 - D(tile -> bl) - D(tile -> br)) / 2.0;

    if (prePass1) {
	if (optSimpleSubRes && tile -> subcont) {
	    tile -> subcont -> subcontInfo -> area += surface;
	    if (optSubResSave) contTile (tile);
	}
	return;
    }
    ASSERT (extrPass);

    elem = recogSurface (tile);

    doSurfCap = extrSurfCaps;

    cnt_i1 = -1;
    for (i = 0; (el = elem[i]); i++) {
	if (el -> type == CONTELEM) {
	    cnt_i2 = i; if (cnt_i1 < 0) cnt_i1 = i;
	  if (optRes) {
	    if ((j = el -> s.cont.con1) >= 0 && (sn1 = tile -> cons[j])) sn1 -> highres |= 4;
	    if ((j = el -> s.cont.con2) >= 0 && (sn2 = tile -> cons[j])) sn2 -> highres |= 4;
	  }
	}
	else if (el -> type == SURFCAPELEM || el -> type == SURFCAP3DELEM) break;
    }

if (doSurfCap) {
    if (!el) doSurfCap = 0;
    else {
	int h = 0;
	cap_i1 = cap_i2 = i;
	do {
		++i;
		++el -> el_recog_cnt; /* extrPass && !substrRes */
		cap = &el -> s.cap;
		if (cap -> dsCap) {
		    cap -> done = 1;
		    cx = cap -> pCon; /* check for double dscap for cx */
		    for (j = 0; j < h; ++j) if (helpArray[j] == cx) break;
		    if (j == h) {
			helpArray[h++] = cx;
			if (optRes) sn1 = tile -> rbPoints ? tile -> rbPoints -> cons[cx] : NULL;
			else sn1 = tile -> cons[cx];
			ASSERT (sn1 && sn1 -> pn);
			sn1 -> pn -> ds_area += surface;
		    }
		}
		else if ((el -> type == SURFCAP3DELEM && cap -> val <= 0)) {
		    cap -> done = 1;
		}
		else {
		    cap -> done = 0;
		    cap_i2 = i;
		}
		el = elem[i];
	} while (el && (el -> type == SURFCAPELEM || el -> type == SURFCAP3DELEM));

	if (cap_i2 == cap_i1) doSurfCap = 0;
    }
}

    if (optRes) {
	resEnumTile (tile); /* only extrPass */
	return;
    }

    joiningX = tile -> xl;
    joiningY = tile -> bl;

    if (hasBipoElem || hasSurfConnect) bipoTile (tile, elem, surface);

    if (cnt_i1 >= 0) { /* do the contacts */
	for (i = cnt_i1; i <= cnt_i2; i++) { el = elem[i];

		joiningCon = cx = el -> s.cont.con1;
		sn1 = (cx >= 0 && cx < nrOfCondStd)? tile -> cons[cx] : NULL;

		cx = el -> s.cont.con2;
		if (cx >= 0 && cx < nrOfCondStd) sn2 = tile -> cons[cx];
		else sn2 = (cx <= -2)? subnSUB : NULL;

		++el -> el_recog_cnt;

		if (!sn1 || !sn2) {
		    missingContact (!sn1 ? 1 : 2, el, tile -> xl, tile -> bl);
		    continue;
		}

		if (el -> s.cont.keep1) sn1 -> node -> term = 2;
		if (el -> s.cont.keep2) sn2 -> node -> term = 2;

		subnodeJoin (sn1, sn2);
	}
    }

    if (doSurfCap) {
	elemDef_t *jun_el = NULL;

	for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
	    cap = &el -> s.cap;
	    if (!cap -> done) {
		if ((cx = cap -> pCon) < nrOfCondStd && (sn1 = tile -> cons[cx])) {
		    if (cap -> pKeep) sn1 -> node -> term = 2;
		}
		else {
		    missingCon (cap -> pMask, cap -> pOccurrence, /* die */
			tile, tNull, tNull, el, tile -> xl, tile -> bl);
		    continue;
		}

		if ((cx = cap -> nCon) < 0)
		    sn2 = (cx <= -2)? subnSUB : subnGND;
		else if (cx < nrOfCondStd && (sn2 = tile -> cons[cx])) {
		    if (cap -> nKeep) sn2 -> node -> term = 2;
		}
		else {
		    missingCon (cap -> nMask, cap -> nOccurrence, /* die */
			tile, tNull, tNull, el, tile -> xl, tile -> bl);
		    continue;
		}

		if (capPolarityTab[cap -> sortNr] != 'x') {
		    if (jun_el) checkDoubleJuncCaps (jun_el, elem + i, tile -> xl, tile -> bl);
		    jun_el = el;
		}

		capAdd (sn1, sn2, cap -> val * surface, cap -> sortNr);
	    }
	}
    }

    if (tile -> tor) {
	int last;
	el = tile -> tor -> type;

	if (!(sn1 = tile -> cons[el -> s.tor.gCon]))
	    missingCon (el -> s.tor.gMask, /* die */
		SURFACE, tile, tNull, tNull, el, tile -> xl, tile -> bl);
	last = !tile -> next_tor && tile -> tor -> subs == tile;
	if (last) portAdd (sn1, tile, 'g');

	if ((cx = el -> s.tor.bCon) != -1) {
	    if (cx < 0) {
		if (last) portAdd (subnSUB, tile, 'b');
	    }
	    else {
		if (!(sn1 = tile -> cons[cx]))
		    missingCon (el -> s.tor.bMask, /* die */
			SURFACE, tile, tNull, tNull, el, tile -> xl, tile -> bl);
		if (last) portAdd (sn1, tile, 'b');
	    }
	}

	tile -> tor -> surface += surface;

	if (optBackInfo > 1) subtorTile (tile, el -> s.tor.gCon);
	subtorDel (tile);
    }
}

Private void pp0EnumTile (tile_t *tile)
{
    coor_t xl, xr, br, tr, dx, dy, br2, tr2, xr2;
    mask_t co;
    tile_t *t, *tt;

    if (!conLR (tile)) return; /* not Left/Right both same highres */

    br = tile -> br;
    tr = tile -> tr;
    if (tile -> bl != br || tile -> tl != tr) return; /* no rectangle */
    xl = tile -> xl;
    xr = tile -> xr;
    dx = xr - xl;
    dy = tr - br;

    currTile = tile;

    if (optResMesh == 2) {
	if (dy <= dx) return; /* ratio not ok */
	pTR = tile -> rbPoints;
	ASSERT (pTR);
	for (pTL = tile -> tlPoints; pTL && pTL -> x != xl; pTL = pTL -> next) ;
	ASSERT (pTL);
	ASSERT (pTL -> y == tr);
	ASSERT (pTR -> y == tr);

	for (;;) {
	    if (pTL -> y == tr) { pTL = pTL -> next; ASSERT (pTL && pTL -> y >= br); }
	    if (pTR -> y == tr) { pTR = pTR -> next; ASSERT (pTR && pTR -> y >= br); }
	    if (pTL -> y > pTR -> y) tr = pTL -> y; else tr = pTR -> y;
	    if (tr == br) break;
	    if (tr == pTL -> y) {
		for (t = tile -> stl; t && t -> tr < tr && t -> xr == xl; t = t -> stt);
		if (t && t -> tr == tr && t -> xr == xl) { /* knik? */
		    co = t -> color;
		    for (t = t -> stt; t && t -> tr == tr && t -> xr == xl; t = t -> stt);
		    if (t && t -> br == tr && t -> xr == xl) {
			COLOR_XOR (co, t -> color);
			if (IS_COLOR (&co)) {
			    COLOR_AND (co, tile -> color);
			    if (HasResC(co) && RsideOK(tr,co)) { /* highres knik */
				putMeshEdge (xl, xr, tr, tr);
				continue;
			    }
			}
		    }
		}
	    }
	    if (tr == pTR -> y) {
		for (t = tile -> str; t && t -> bl > tr; t = t -> stb);
		ASSERT (t);
		if (t -> bl == tr) { /* knik? */
		    co = t -> color;
		    for (t = t -> stb; t && t -> bl == tr; t = t -> stb);
		    ASSERT (t && t -> tl == tr);
		    COLOR_XOR (co, t -> color);
		    if (IS_COLOR (&co)) {
			COLOR_AND (co, tile -> color);
			if (HasResC(co) && LsideOK(tr,co)) {
			    putMeshEdge (xl, xr, tr, tr);
			}
		    }
		}
	    }
	}
	return;
    }

    if (dy > dx) { /* ratio ok */

	/* check leftside of tile for fixed (terminal) points */
	for (pTL = tile -> tlPoints; pTL; pTL = pTL -> next) {
	    if (pTL -> x == xl && pTL -> fixed) {
		br2 = pTL->y;
		t = tile -> stl;
		while (t && t -> xr == xl && t -> tr < br2) t = t -> stt;
		if (t && t -> xr == xl && t -> br <= br2) {
		    for (pTR = t -> rbPoints; pTR && pTR->y >= br2 && pTR->x == xl; pTR = pTR -> next) {
			if (pTR -> fixed && pTR->y == br2) {
			    if (br2 < tr - dy/2) {
				tr2 = br + (xr - t->xl);
				if (br2 < tr2) br2 = tr2;
			    } else {
				tr2 = tr - (xr - t->xl);
				if (br2 > tr2) br2 = tr2;
			    }
			    putMeshEdge (t->xl, xr, br2, br2);
			    break;
			}
		    }
		}
	    }
	}

	/* check leftside of tile at bottom */
	t = tile -> stl;
	while (t && t -> xr == xl && t -> tr <= br) t = t -> stt;
	if (t && t -> xr == xl) { /* t -> tr > br */
	    tt = t;
	    if (t -> br < br) {
		xl = xr;
		//br2 = t -> br;
		//xr2 = t -> xr;
try_br:
		if (t -> bl == t -> br && t -> tl == t -> tr && conLR (t)) {
		    if (t -> tr < tr) tr = t -> tr;
		 // if (t -> br > br2) br2 = t -> br;
		 // if (D(tr - br)/(xr - t -> xl) >= 1 || D(br - br2)/(xr2 - t -> xl) >= 1)
		    if (D(tr - br)/(xr - t -> xl) >= 1) {
			xl = t -> xl;
			t = t -> stl;
			while (t && t -> xr == xl && t -> tr <= br) t = t -> stt;
			if (t && t -> xr == xl && t -> br < br) goto try_br;
		    }
		}
		if (xl < xr) putMeshEdge (xl, xr, br, br);
		xl = tile -> xl;
		tr = tile -> tr;
	    }
	}
	else tt = NULL;

	/* check leftside of tile at top */
	t = tt;
	while (t && t -> tr < tr) t = t -> stt;
	if (t && t -> xr == xl) { /* t -> tr >= tr */
	    if (t -> tr > tr && t -> br < tr) {
		xl = xr;
		tr2 = t -> tr;
		xr2 = t -> xr;
try_tr:
		if (t -> bl == t -> br && t -> tl == t -> tr && conLR (t)) {
		    if (t -> br > br) br = t -> br;
		    if (t -> tr < tr2) tr2 = t -> tr;
		 // if (D(tr - br)/(xr - t -> xl) >= 1 || D(tr2 - tr)/(xr2 - t -> xl) >= 1)
		    if (D(tr - br)/(xr - t -> xl) >= 1) {
			xl = t -> xl;
			t = t -> stl; while (t && t -> tr <= tr) t = t -> stt;
			if (t && t -> br < tr) goto try_tr; // t -> tr > tr
		    }
		}
		if (xl < xr) putMeshEdge (xl, xr, tr, tr);
		xl = tile -> xl;
		br = tile -> br;
	    }
	}

	/* rightside checks (only possible by delayed enumtile) */
	t = tile -> str;
	while (t && t -> bl == tr) t = t -> stb;
	if (t && t -> bl < tr && t -> xl == xr) {
	    /* check rigtside of tile at top */
	    tt = t;
	    if (t -> tl > tr) {
		xr2 = xl;
		tr2 = t -> tr;
try_tr2:
		if (t -> bl == t -> br && t -> tl == t -> tr && conLR (t)) {
		    if (t -> br > br) br = t -> br;
		    if (t -> tr < tr2) tr2 = t -> tr;
		 // if (D(tr - br)/(t -> xr - xl) >= 1 || D(tr2 - tr)/(t -> xr - xr) >= 1)
		    if (D(tr - br)/(t -> xr - xl) >= 1) {
			xr2 = t -> xr;
			br2 = t -> tr;
			t = t -> str; while (t && t -> bl == br2) t = t -> stb;
			if (t && t -> bl < tr && t -> xl == xr2 && t -> tl > tr) goto try_tr2;
		    }
		}
		if (xr2 > xl) putMeshEdge (xl, xr2, tr, tr);
		br = tile -> br;
	    }
	    /* check rigtside of tile at bottom */
	    t = tt;
	    while (t && t -> bl >= br && t -> xl == xr) t = t -> stb;
	    if (t && t -> bl < br && t -> tl > br && t -> xl == xr) {
		xr2 = xl;
		br2 = t -> br;
try_br2:
		if (t -> bl == t -> br && t -> tl == t -> tr && conLR (t)) {
		    if (t -> tr < tr) tr = t -> tr;
		    if (t -> br > br2) br2 = t -> br;
		 // if (D(tr - br)/(t -> xr - xl) >= 1 || D(br - br2)/(t -> xr - xr) >= 1)
		    if (D(tr - br)/(t -> xr - xl) >= 1) {
			xr2 = t -> xr;
			tr2 = t -> tr;
			t = t -> str; while (t && t -> bl == tr2) t = t -> stb;
			while (t && t -> bl >= br && t -> xl == xr2) t = t -> stb;
			if (t && t -> bl < br && t -> tl > br && t -> xl == xr2) goto try_br2;
		    }
		}
		if (xr2 > xl) putMeshEdge (xl, xr2, br, br);
	    }
	}
    }
}

Private void do_the_contacts (int split)
{
    register int i, cx;
    register elemDef_t *el;
    subnode_t *sn;
    double cval;

    for (i = cnt_i1; i <= cnt_i2; i++) { el = elem[i];

	if ((cval = el -> s.cont.val) > 0) {
	    cx = el -> s.cont.con1;
	    if (optPrick && cx < nrOfCondStd) {
		sn = pTR -> cons[cx];
		if (sn && Grp(sn -> node) -> prick) {
			if (optInvertPrick) cval = 0;
		} else if (!optInvertPrick) cval = 0;
	    }
	}

	++el -> el_recog_cnt;
	if (split) {
	    if (makeContact (pTR, POS_TR, el, surface / 4, cval)) continue;
	    if (makeContact (pTL, POS_TL, el, surface / 4, cval)) continue;
	    if (makeContact (pBR, POS_BR, el, surface / 4, cval)) continue;
	    makeContact (pBL, POS_BL, el, surface / 4, cval);
	}
	else {
	    makeContact (pTR, 0, el, surface, cval);
	}
    }
}

Private void resEnumTile (tile_t *tile)
{
    register int i, cx;
    register elemDef_t *el;
    int split;
    nodePoint_t *p;
    resElemDef_t *res;
    subnode_t *sn, *sn1, *sn2;
    static int nbrWarn2 = 0;

    if (!(pTR = tile -> rbPoints)) return; /* no conductor */
    pTRb = tile -> tlPoints;
    pTL = NULL;
    currTile = tile;

    joiningX = pTR -> x;
    joiningY = pTR -> y;

    extraPoints = 0;
    if ((split = (pTRb != pTR))) { /* high res tile */

	/* Find the conductance values for the different conductor layers.
         * And join subnodes of the two points in the upper-right corner.
	 */
	lastA = 0; firstB = -1;

	for (joiningCon = 0; joiningCon < nrOfCondStd; ++joiningCon) {
	    if ((sn = tile -> cons[joiningCon])) {
		res = sn -> cond;
		conSort[joiningCon] = res -> sortNr;
		sn1 = pTR  -> cons[joiningCon];
		sn2 = pTRb -> cons[joiningCon];
		subnodeJoin (sn1, sn2);

		if (sn -> highres & 1) { /* HIGH_RES */
		    conNums[lastA++] = joiningCon;
		    conVal[joiningCon] = res -> val;
		}
		else { /* LOW_RES */
		    for (p = pTRb -> next; p; p = p -> next) subnodeJoin (sn1, p -> cons[joiningCon]);
		    for (p = pTR  -> next; p; p = p -> next) subnodeJoin (sn1, p -> cons[joiningCon]);
		    makeAreaNode (sn1);
		}
	    }
	}
	ASSERT (lastA > 0); /* there must be a high res conductor */

        /* perform triangularization and update the surface capacitances */

	triangular (tile);
    }
    else {
	if (hasBipoElem || hasSurfConnect) resBipoTile (tile, elem, surface);
	//if (cnt_i1 >= 0) do_the_contacts (split);
    }

    if (cnt_i1 >= 0) do_the_contacts (split);

    if (doSurfCap && !split) { /* add surface capacitances */
	for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
	    if (!el -> s.cap.done) parPlateCap (pTR, pTR, el, surface);
	}
    }

    if (tile -> tor) {
	int last;
	el = tile -> tor -> type;
	cx = el -> s.tor.gCon;
	if (!pTR -> cons[cx])
	    missingCon (el -> s.tor.gMask, /* die */
		SURFACE, tile, tNull, tNull, el, tile -> xl, tile -> bl);

	last = !tile -> next_tor && tile -> tor -> subs == tile;
	if (last) {
	    portAdd (pTR -> cons[cx], tile, 'g');
	    /* the right top node is used as gate (see also lump.c) */
	}

	if ((cx = el -> s.tor.bCon) >= 0) {
	    if (!pTR -> cons[cx])
		missingCon (el -> s.tor.bMask, /* die */
		    SURFACE, tile, tNull, tNull, el, tile -> xl, tile -> bl);
	    if (last)
		portAdd (pTR -> cons[cx], tile, 'b');
	}
	else if (cx <= -2) {
	    if (last) {
		if (tile -> subcont)
		    portAdd (tile -> subcont -> subn, tile, 'b');
		else
		    portAdd (subnSUB, tile, 'b');
	    }
	}

	tile -> tor -> surface += surface;

	if (optBackInfo > 1) subtorTile (tile, el -> s.tor.gCon);
	subtorDel (tile);
    }
}

Private int rmPoint (nodePoint_t *p, nodePoint_t *pPrev, nodePoint_t *pNext)
{
    register int cx, i;
    double dl1, dl2;
    extern bool_t outputNoDangling;
    node_t *n;

    ASSERT (pPrev && pNext);

    /* Can this point really be removed? */
    cx = -1;
    for (i = 0; i < nrOfCondStd; ++i)
	if (p -> cons[i]) {
	    n = p -> cons[i] -> node;
	    if (n -> netEq &&
		pPrev -> cons[i] -> node != n &&
		pNext -> cons[i] -> node != n) return (0); /* no */
	    if (cx < 0) cx = i;
	}

    ASSERT (cx >= 0); /* point must have a conductor */

    dl1 = edgeLength (pPrev, p);
    dl2 = edgeLength (p, pNext);

    outputNoDangling = TRUE;
    for (; cx < nrOfCondStd; ++cx) {
	if (p -> cons[cx]) {
	    subnodeReconnect (p -> cons[cx], pPrev -> cons[cx], pNext -> cons[cx], dl1, dl2);
	    subnodeDel (p -> cons[cx]);
	}
    }
    outputNoDangling = FALSE;

    disposeNodePoint (p);
    return (1); /* ok */
}

#ifdef DISPLAY
Private void drawCross (coor_t x, coor_t y, coor_t d)
{
    d /= 8;
    drawTriangleEdge (x - d, y + d, x + d, y - d);
    drawTriangleEdge (x - d, y - d, x + d, y + d);
}
#endif

Private int conTB_lr (tile_t *tile)
{
    tile_t *stt, *stb;
    int rv = 0;
    if (conTB1 (tile)) {
	stt = tile -> stt;
	while (stt && stt -> xr > tile -> xl && stt -> bl == tile -> tr) {
	    if (conLR (stt)) { rv |= 1; break; }
	    stt = stt -> stl;
	}
    }
    if (conTB2 (tile)) {
	stb = tile -> stb;
	while (stb && stb -> xl < tile -> xr && stb -> tr == tile -> br) {
	    if (conLR (stb)) { rv |= 2; break; }
	    stb = stb -> str;
	}
    }
    return rv;
}

Private int conLR_tb (tile_t *tile)
{
    tile_t *stl, *str;
    int rv = 0;
    if (conLR1 (tile)) {
	str = tile -> str;
	while (str && str -> xl == tile -> xr && str -> tl > tile -> br) {
	    if (str -> bl < tile -> tr && str -> bl != str -> tl)
	    if (conTB (str)) { rv |= 1; break; }
	    str = str -> stb;
	}
    }
    if (conLR25 (tile)) rv |= 2;
    return rv;
}

Private void triangular (tile_t *tile)
{
    extern bool_t remove_unfixed;
    static nodePoint_t *tlP, *trP, *bprev, *bnext, *tprev, *tnext;
    nodePoint_t *point, *pPrev, *pNext, *blP, *brP;
    coor_t xl, xr, br, tr;
    int rmUnfixed = 0;

    hPoint = vPoint = 0;

    xl = tile -> xl;
    xr = tile -> xr;
    br = tile -> br;
    tr = tile -> tr;

#ifdef CAP3D
    /* In case of 3D capacitance extraction we cannot do it since
       since spiders may already be pointing at it. */
    if (!optCap3D)
#endif
    if (remove_unfixed) rmUnfixed = 1;

    /* Set tile corner points pTL, pBR and pBL and remove
     * points that are not necessary for triangularization.
     * And we will change the datastructure that links the points of
     * the tile. For both rbPoints and tlPoints the prev pointer is set.
     * For tlPoints the next pointer and prev pointer are swapped.
     * The rbPoints / tlPoints are connected in one-double linked list.
     */
    pBR = pPrev = NULL;
    for (point = pTR; point; point = pNext) {
	pNext = point -> next;   /* save next */
	if (point -> x == xr) {
	    if (point -> y != tr && point -> y != br) {
		if (rmUnfixed && !point -> fixed && rmPoint (point, pPrev, pNext)) {
		    pPrev -> next = pNext;
		    continue;
		}
		vPoint = 1;
	    }
	    else pBR = point;
	}
	else if (point -> x != xl) {
	    if (rmUnfixed && !point -> fixed && rmPoint (point, pPrev, pNext)) {
		pPrev -> next = pNext;
		continue;
	    }
	    hPoint = 1;
	}
	point -> prev = pPrev;
	pPrev = point;
    }
    ASSERT (pBR);

    pTL = pPrev = NULL;
    for (point = pTRb; point; point = pNext) {
	pNext = point -> next;   /* save next */
	if (point -> x == xl) {
	    if (point -> y != tile -> tl && point -> y != tile -> bl) {
		if (rmUnfixed && !point -> fixed && rmPoint (point, pPrev, pNext)) {
		    continue;
		}
		vPoint |= 2;
	    }
	    else if (!pTL) pTL = point;
	}
	else if (point -> x != xr) {
	    if (rmUnfixed && !point -> fixed && rmPoint (point, pPrev, pNext)) {
		continue;
	    }
	    hPoint |= 2;
	}
	point -> next = pPrev;   /* change direction */
	if (pPrev) pPrev -> prev = point;
	pPrev = point;
    }
    ASSERT (pTL);
    pBL = pPrev;

    pTRb -> fixed = 2;
    pTL -> fixed = 2;
    pBR -> fixed = 2;
    pBL -> fixed = 2;

    if (hasBipoElem || hasSurfConnect) resBipoTile (tile, elem, surface);

    extraPoints = hPoint || vPoint;

    //if (cnt_i1 >= 0) do_the_contacts (1);

    /* Here we check if the tile can be covered by an equi-potential
       line to introduce a node with an articulation degree > 1. */

    pTR -> prev = pTRb -> prev;

    if (tile -> bl == br && tile -> tl == tr) { /* Rectangle */
	coor_t dx, dy, w, h;
	node_t *nBL;
	subnode_t *sn;
	tile_t *str, *stt;
	int i, ok;

	dx = xr - xl;
	dy = tr - br;

#define HAS_CONTACT(j) (tile -> cons[j] -> highres & 4)

#define HAS_CON_R(j)  (tile -> cons[j] -> highres & 0x10)
#define HAS_CON_L(j)  (tile -> cons[j] -> highres & 0x20)
#define HAS_CON_LR(j) (tile -> cons[j] -> highres & 0x30)

#define HAS_CON_T(j)  (tile -> cons[j] -> highres & 0x40)
#define HAS_CON_B(j)  (tile -> cons[j] -> highres & 0x80)
#define HAS_CON_BT(j) (tile -> cons[j] -> highres & 0xc0)

	if (equi_line_old) {
	    if (conTB(tile)) { if (!vPoint && !conLR(tile)) {
		if (D(dy)/dx >= equi_line_ratio) equiMode = 1;
		else if (optResMesh == 2 && dy >= dx/2) {
		    if (conTB_lr (tile)) equiMode = 1;
		}
	    }}
	    else if (!hPoint && conLR(tile)) {
		if (D(dx)/dy >= equi_line_ratio) equiMode = 2;
		else if (optResMesh == 2 && dx >= dy/2) {
		    if (conLR_tb (tile)) equiMode = 2;
		}
	    }
	    if (equiMode && use_corner_ratio > 0) {
		i = lastA;
		while (--i >= 0) if (HAS_CONTACT (conNums[i])) { equiMode = 0; break; }
	    }
	}

	if (equi_line_new) {
	    ok = 0;
	    firstB = lastA;
	    for (i = 0; i < firstB; ++i) {
		joiningCon = conNums[i];
		sn = pBL -> cons[joiningCon];
		nBL = sn -> node;

		if (!HAS_CONTACT (joiningCon)) {

		    if (HAS_CON_LR(joiningCon)) goto reinitH; /* left or right same conductor */

		    if ((h = nBL -> node_h)) {
			//ASSERT (HAS_CON_B(joiningCon));
			if (!(hPoint & 1) && !nBL -> area) { /* down no extra point && !lowres */
			    if (D(h + dy) / dx >= equi_line_ratio) { ok |= 1;
				subnodeJoin (sn, pBR -> cons[joiningCon]);
				if (equi_line_area) makeLineNode (sn);
				if (equiMode && equi_line_new == 2) {
				    --firstB;
				    conNums[i] = conNums[firstB];
				    conNums[firstB] = joiningCon;
				    if (firstB == 0) equiMode = 0;
				    --i;
				}
			    }
			}
			else h = 0;
		    }

		    /* is there up the same conductor && no extra point */
		    if (HAS_CON_T(joiningCon) && hPoint < 2) pTL -> cons[joiningCon] -> node -> node_h = h + dy;
		    continue;
reinitH:
		    nBL -> node_h = 0;

		    if (HAS_CON_BT(joiningCon)) goto reinitW; /* down or up same conductor */

		    if ((w = nBL -> node_w)) {
			//ASSERT (HAS_CON_L(joiningCon));
			if (vPoint < 2 && !nBL -> area) { /* left no extra point && !lowres */
			    if (D(w + dx) / dy >= equi_line_ratio) { ok |= 2;
				subnodeJoin (sn, pTL -> cons[joiningCon]);
				if (equi_line_area) makeLineNode (sn);
				if (equiMode && equi_line_new == 2) {
				    --firstB;
				    conNums[i] = conNums[firstB];
				    conNums[firstB] = joiningCon;
				    if (firstB == 0) equiMode = 0;
				    --i;
				}
			    }
			}
			else w = 0;
		    }

		    /* is there right the same conductor && no extra point */
		    if (HAS_CON_R(joiningCon) && !(vPoint & 1)) pBR -> cons[joiningCon] -> node -> node_w = w + dx;
		}
		else {
		    nBL -> node_h = 0;
reinitW:
		    nBL -> node_w = 0;
		}
	    }
	    if (firstB == lastA) firstB = -1;
#ifdef DISPLAY
	    if (ok && optDisplay) {
		if (ok & 1) drawCross (xl + dx/2, br, dx);
		if (ok & 2) drawCross (xl, br + dy/2, dy);
	    }
#endif
	}

	if (equiMode) {
	    int doit = 0;
	    if (use_corner_ratio > 0) {
		if (equiMode == 1) { /* conTB */
		    if (D(dy)/dx >= 2 * use_corner_ratio) doit = conTB_lr (tile);
		    else corner_line_node = 1;
		} else { /* conLR */
		    if (D(dx)/dy >= 2 * use_corner_ratio) doit = conLR_tb (tile);
		    else corner_line_node = 1;
		}
	    }
	    if (doit) {
		if (equiMode == 1) { /* conTB */
		    coor_t ym, d = use_corner_ratio * dx;
		    if (d < 2) d = 2;
		    if (doit != 3) {
			if (doit & 1) ym = tr - d; else ym = br + d;
		    } else {
			if (dy < 3*d) ym = br + dy/2;
			else { ym = tr - d; doit += 4; }
		    }
				    trP = newNodePoint (xr, ym, pTR);
				    trP -> prev = pTR;
				    pTR -> next = trP;
				    pBR -> prev = trP;
				    tlP = newNodePoint (xl, ym, pTL);
				    tlP -> next = pTL;
				    pTL -> prev = tlP;
				    pBL -> next = tlP;
		    trP -> next = tlP;
		    tlP -> prev = trP;
		    corner_line_node = (doit & 1);
		    doEquiRectangle (tlP, trP, pTL, pTR);
#ifdef DISPLAY
		    if (equi_line_new && optDisplay) drawCross (xl + d, ym, dx);
#endif
		    if (doit == 7) {
				    ym = br + d;
				    brP = newNodePoint (xr, ym, trP);
				    trP -> next = brP;
				    brP -> next = pBR;
				    pBR -> prev = brP;
				    blP = newNodePoint (xl, ym, tlP);
				    tlP -> prev = blP;
				    blP -> prev = pBL;
				    pBL -> next = blP;
			equiMode = 0;
			doRectangle (blP, brP, tlP, trP);
			equiMode = 1;
			brP -> prev = blP;
			blP -> next = brP;
			corner_line_node = 1;
			doEquiRectangle (pBL, pBR, blP, brP);
				    brP -> prev = trP;
				    blP -> next = tlP;
#ifdef DISPLAY
			if (equi_line_new && optDisplay) drawCross (xl + d, ym, dx);
#endif
		    }
		    else {
			brP = blP = NULL;
				    trP -> next = pBR;
				    tlP -> prev = pBL;
			trP -> prev = tlP;
			tlP -> next = trP;
			corner_line_node = (doit & 2);
			doEquiRectangle (pBL, pBR, tlP, trP);
				    trP -> prev = pTR;
				    tlP -> next = pTL;
		    }
		}
		else { /* conLR */
		    coor_t xm, d = use_corner_ratio * dy;
		    if (d < 2) d = 2;
		    if (doit != 3) {
			if (doit & 1) xm = xr - d; else xm = xl + d;
		    } else {
			if (dx < 3*d) xm = xl + dx/2;
			else { xm = xr - d; doit += 4; }
		    }
				    trP = newNodePoint (xm, tr, pTR);
				    trP -> next = pTRb;
				    pTL -> next = trP;
				    pTR -> prev = trP;
				    pTRb -> prev = trP;
				    tlP = newNodePoint (xm, br, pBR);
				    tlP -> prev = pBR;
				    pBL -> prev = tlP;
				    pBR -> next = tlP;
		    trP -> prev = tlP;
		    tlP -> next = trP;
		    corner_line_node = (doit & 1);
		    doEquiRectangle (tlP, pBR, trP, pTR);
#ifdef DISPLAY
		    if (equi_line_new && optDisplay) drawCross (xm, br + d, dy);
#endif
		    if (doit == 7) {
				    xm = xl + d;
				    brP = newNodePoint (xm, tr, trP);
				    trP -> prev = brP;
				    brP -> prev = pTL;
				    pTL -> next = brP;
				    blP = newNodePoint (xm, br, tlP);
				    tlP -> next = blP;
				    blP -> next = pBL;
				    pBL -> prev = blP;
			equiMode = 0;
			doRectangle (blP, tlP, brP, trP);
			equiMode = 2;
			brP -> next = blP;
			blP -> prev = brP;
			corner_line_node = 1;
			doEquiRectangle (pBL, blP, pTL, brP);
				    brP -> next = trP;
				    blP -> prev = tlP;
#ifdef DISPLAY
			if (equi_line_new && optDisplay) drawCross (xm, br + d, dy);
#endif
		    }
		    else {
			brP = blP = NULL;
				    trP -> prev = pTL;
				    tlP -> next = pBL;
			trP -> next = tlP;
			tlP -> prev = trP;
			corner_line_node = (doit & 2);
			doEquiRectangle (pBL, tlP, pTL, trP);
				    trP -> next = pTRb;
				    tlP -> prev = pBR;
		    }
		}
		if (equi_line_new) {
		    corner_line_node = 0;
		    if (firstB < 0) firstB = lastA;
		    for (i = 0; i < firstB; ++i) {
			joiningCon = conNums[i];
			if (brP) {
			    sn = brP -> cons[joiningCon];
			    subnodeJoin (sn, blP -> cons[joiningCon]);
			    if (equi_line_area) makeLineNode (sn);
			}
			sn = trP -> cons[joiningCon];
			subnodeJoin (sn, tlP -> cons[joiningCon]);
			if (equi_line_area) makeLineNode (sn);
		    }
		}
	    }
	    else {
		doEquiRectangle (pBL, pBR, pTL, pTR);
	    }
	    corner_line_node = 0;
	    equiMode = 0;
	    goto ret;
        }
	if (!extraPoints) {
	    doRectangle (pBL, pBR, pTL, pTR);
	    goto ret;
	}
    }

    triangular_core (pTR);

ret:
    pTR -> prev = NULL;
}

Private int LsideOK (coor_t w, mask_t co)
{
    tile_t *tile;
    for (tile = currTile -> stl; tile && tile -> tr < w; tile = tile -> stt);
    if (!tile) return 0;
    if (tile -> tr > w && tile -> br < w) {
	COLOR_AND (co, tile -> color);
	if (HasResC(co)) return 0;
    }
    return 1;
}

Private int RsideOK (coor_t w, mask_t co)
{
    tile_t *tile;
    for (tile = currTile -> str; tile && tile -> bl > w; tile = tile -> stb);
    if (!tile) return 0;
    if (tile -> bl < w && tile -> tl > w) {
	COLOR_AND (co, tile -> color);
	if (HasResC(co)) return 0;
    }
    return 1;
}

Private nodePoint_t * newNodePoint (coor_t x, coor_t y, nodePoint_t *point)
{
    subnode_t **cons, *psn, *sn, *newsn;
    nodePoint_t *newPoint;
    int i;

    cons = currTile -> cons;
    newPoint = createNodePoint (x, y);

    for (i = 0; i < nrOfCondStd; ++i) {
	if ((sn = cons[i])) {
	    newPoint -> cons[i] = newsn = CONS (newPoint, i);
	    newsn -> cond = sn -> cond;
	    psn = point -> cons[i];
	    if (!(sn -> highres & 1)) {
		subnodeCopy (psn, newsn); /* lowres conductor */
	    }
	    else {
		subnodeNew2 (newsn, Grp (psn -> node));
		if (psn -> pn) polnodeCopy (psn, newsn);
		newsn -> node -> mask = conductorMask[i];
		newsn -> node -> node_x = x;
		newsn -> node -> node_y = y;
	    }
	}
	else newPoint -> cons[i] = NULL;
    }
    return newPoint;
}

Private void triangular_core (nodePoint_t *rbP)
{
    nodePoint_t *tlP = rbP;

    /* Now we will perform the triangularization
     * rbP follows next, tlP follows prev
     */
    if (edgeLength (tlP -> prev, rbP) < edgeLength (tlP, rbP -> next))
	tlP = tlP -> prev;
    else
	rbP = rbP -> next;

    /* tlP and rbP create triangles (and boxes) until they meet */

    while (tlP -> prev != rbP) {

	if (onSameLine (rbP -> next, rbP, tlP)

	    || (onSameLine (rbP -> next -> next, rbP -> next, tlP)
	        && onSameLine (rbP -> next, tlP, tlP -> prev))

	    || (!(onSameLine (tlP -> prev, rbP, tlP)
		  || (onSameLine (tlP -> prev -> prev, rbP, tlP -> prev)
		      && onSameLine (tlP -> prev, rbP -> next, rbP)))
		 && (edgeLength (tlP -> prev, rbP) < edgeLength (rbP -> next, tlP)))) {

	    tryTriangle (tlP -> prev, rbP, tlP);
	    tlP = tlP -> prev;
	}
	else {
	    tryTriangle (rbP -> next, tlP, rbP);
	    rbP = rbP -> next;
	}
    }

    if (pbS) { /* flush tryTriangle */
	doTriangle (pf1S, pf2S, pbS);
	pbS = NULL;
    }
}

Private void e_triangular_core (nodePoint_t *rb, nodePoint_t *rbn, nodePoint_t *tl, nodePoint_t *tlp)
{
    nodePoint_t *p, *pn;

    pn = (p = rb) -> prev;
	//
	//   tl +---------+ p = rb
	//      |         |
	//      |         + pn
    for (;;) {
	doTriangle (pn, tl, p);
	pn = (p = pn) -> prev;
	if (pn == rbn) break;
	//
	//   tl +---------+ rb
	//      |      |  |
	//      |    e1|  + p ----
	//      |      |  |     |
	//      |     ----+ pn  |e2
	//      |         |     |
	//   tlp+---------+ rbn---
	//
	if (edgeLength (rbn, p) < edgeLength (pn, rb)) break;
    }
    doTriangle (p, tlp, tl);
    for (;;) {
	doTriangle (pn, tlp, p);
	if (pn == rbn) break;
	pn = (p = pn) -> prev;
    }
}

Private int onSameLine (nodePoint_t *p1, nodePoint_t *p2, nodePoint_t *p3)
{
    return ((p2 -> x - p1 -> x) * (p3 -> y - p1 -> y)
	  - (p2 -> y - p1 -> y) * (p3 -> x - p1 -> x) == 0);
}

Private double edgeLength (nodePoint_t *p1, nodePoint_t *p2)
{
    double x = p2 -> x - p1 -> x;
    double y = p2 -> y - p1 -> y;
    if (x == 0) return (y < 0 ? -y : y);
    if (y == 0) return (x < 0 ? -x : x);
    return sqrt (x * x + y * y);
}

Private void tryTriangle (nodePoint_t *pf1, nodePoint_t *pf2, nodePoint_t *pb)
{
    /* Tries a triangle but tries to detect a rectangle first.
     * This is achieved by keeping the latest triangle in the buffer
     * and combining this triangle with the subsequent triangle
     * if possible.
     */
    nodePoint_t * p1, * p2, * p3, * p4;
    nodePoint_t * pl1, * pl2, * pr1, * pr2;
    nodePoint_t * tlP, * trP, * blP, * brP;

    if (!pbS) {
	pf1S = pf1; pf2S = pf2; pbS = pb;
	return;
    }

    p1 = pbS;
    p2 = pf1S;
    p3 = pf2S;
    if ((pf1 -> x == pf1S -> x && pf1 -> y == pf1S -> y)
     || (pf1 -> x == pf2S -> x && pf1 -> y == pf2S -> y)) {
	p4 = pf2;
    }
    else p4 = pf1;

    if (p1 -> x <= p2 -> x && p1 -> x <= p3 -> x) {
	pl1 = p1;
	if (p2 -> x == pl1 -> x) {
	    pl2 = p2; pr1 = p3; pr2 = p4;
	}
	else if (p3 -> x == pl1 -> x) {
	    pl2 = p3; pr1 = p2; pr2 = p4;
	}
	else {
	    pl2 = p4; pr1 = p2; pr2 = p3;
	}
    }
    else {
	pr1 = p1;
	if (p2 -> x == pr1 -> x) {
	    pr2 = p2; pl1 = p3; pl2 = p4;
	}
	else if (p3 -> x == pr1 -> x) {
	    pr2 = p3; pl1 = p2; pl2 = p4;
	}
	else {
	    pr2 = p4; pl1 = p2; pl2 = p3;
	}
    }

    if (pl1 -> y > pl2 -> y) { tlP = pl1; blP = pl2; }
    else { tlP = pl2; blP = pl1; }

    if (pr1 -> y > pr2 -> y) { trP = pr1; brP = pr2; }
    else { trP = pr2; brP = pr1; }

    /* Is it a rectangle? */
    if (brP -> y == blP -> y && trP -> y == tlP -> y &&
	brP -> x == trP -> x && blP -> x == tlP -> x) {

	doRectangle (blP, brP, tlP, trP);
	pbS = NULL;
    }
    else {
	doTriangle (pf1S, pf2S, pbS);
	pf1S = pf1; pf2S = pf2; pbS = pb;
    }
}

Private void doRectangle (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP)
{
    /* Processes a rectangle (adds resistances and parallel plate capacitances). */
    int i;
    double recArea, sheetCon, ratio1, ratio2, area, dx, dy;
    elemDef_t *el;

    dx = brP -> x - blP -> x;
    dy = trP -> y - brP -> y;

#ifdef DISPLAY
    if (optDisplay && !equiMode) {
	drawTriangleEdge (blP -> x, blP -> y, brP -> x, brP -> y);
	drawTriangleEdge (blP -> x, blP -> y, tlP -> x, tlP -> y);
	drawTriangleEdge (trP -> x, trP -> y, tlP -> x, tlP -> y);
	drawTriangleEdge (trP -> x, trP -> y, brP -> x, brP -> y);
    }
#endif
#ifdef PLOT_CIR_MODE
    if (optPlotCir && !equiMode) {
	plotLine (SOLID, blP -> x, blP -> y, brP -> x, brP -> y);
	plotLine (SOLID, blP -> x, blP -> y, tlP -> x, tlP -> y);
	plotLine (SOLID, trP -> x, trP -> y, tlP -> x, tlP -> y);
	plotLine (SOLID, trP -> x, trP -> y, brP -> x, brP -> y);
    }
#endif

    /* Add resistances. */

    ratio1 = dy / dx;
    ratio2 = dx / dy;

    if (equiMode & 3) {
	if (equiMode == 1) // conTB
	    ratio1 *= 2;
	else
	    ratio2 *= 2;
    }

    for (i = firstA; i < lastA; ++i) {
	joiningCon = conNums[i];
	sheetCon = 0.5 * conVal[joiningCon];
	conAdd (tlP -> cons[joiningCon], trP -> cons[joiningCon], sheetCon * ratio1, conSort[joiningCon]);
	conAdd (blP -> cons[joiningCon], brP -> cons[joiningCon], sheetCon * ratio1, conSort[joiningCon]);
	conAdd (tlP -> cons[joiningCon], blP -> cons[joiningCon], sheetCon * ratio2, conSort[joiningCon]);
	conAdd (trP -> cons[joiningCon], brP -> cons[joiningCon], sheetCon * ratio2, conSort[joiningCon]);
    }

    if (doSurfCap && !equiMode) { /* Add parallel plate capacitances. */
	recArea = meters * meters * dx * dy;
	for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
	    if (!el -> s.cap.done) {
		if (cap_assign_type & 1) {
		    area = recArea / 8;
		    parPlateCap (tlP, tlP, el, area);
		    parPlateCap (trP, trP, el, area);
		    parPlateCap (brP, brP, el, area);
		    parPlateCap (blP, blP, el, area);

		    area = recArea / 24;
		    parPlateCap (tlP, trP, el, area);
		    parPlateCap (tlP, blP, el, area);
		    parPlateCap (tlP, brP, el, area);

		    parPlateCap (trP, tlP, el, area);
		    parPlateCap (trP, brP, el, area);
		    parPlateCap (trP, blP, el, area);

		    parPlateCap (brP, blP, el, area);
		    parPlateCap (brP, trP, el, area);
		    parPlateCap (brP, tlP, el, area);

		    parPlateCap (blP, brP, el, area);
		    parPlateCap (blP, tlP, el, area);
		    parPlateCap (blP, trP, el, area);
		}
		else {
		    area = recArea / 4;
		    parPlateCap (tlP, tlP, el, area);
		    parPlateCap (trP, trP, el, area);
		    parPlateCap (brP, brP, el, area);
		    parPlateCap (blP, blP, el, area);
		}
	    }
	}
    }
}

Private void doTriangle (nodePoint_t *p1, nodePoint_t *p2, nodePoint_t *p3)
{
    /* Processes a triangle (adds resistances and parallel plate capacitances). */
    int i;
    double A4, triSurf, sheetCon, area, v1, v2, v3;
    double dx1, dx2, dx3, dy1, dy2, dy3;
    elemDef_t *el;
    coor_t x1 = p1 -> x;
    coor_t x2 = p2 -> x;
    coor_t x3 = p3 -> x;
    coor_t y1 = p1 -> y;
    coor_t y2 = p2 -> y;
    coor_t y3 = p3 -> y;

    dy1 = y2 - y3;
    dy2 = y3 - y1;
    dy3 = y1 - y2;

    /* A4 is area of triangle * 4 */
    A4 = (x1 * dy1 + x2 * dy2 + x3 * dy3) * 2;
    if (A4 < 0) A4 = -A4; /* Absolute! */

#ifdef DISPLAY
    if (optDisplay && !equiMode) {
	drawTriangleEdge (x1, y1, x2, y2);
	drawTriangleEdge (x1, y1, x3, y3);
	drawTriangleEdge (x2, y2, x3, y3);
    }
#endif
#ifdef PLOT_CIR_MODE
    if (optPlotCir && !equiMode) {
	plotLine (SOLID, x1, y1, x2, y2);
	plotLine (SOLID, x1, y1, x3, y3);
	plotLine (SOLID, x2, y2, x3, y3);
    }
#endif

    /* Add resistances. */

    dx1 = x2 - x3;
    dx2 = x3 - x1;
    dx3 = x1 - x2;

    v1 = (dy1 * -dy2 + dx2 * -dx1) / A4;
    v2 = (dy2 * -dy3 + dx3 * -dx2) / A4;
    v3 = (dy3 * -dy1 + dx1 * -dx3) / A4;

    if (equiMode & 3) {
	if (equiMode == 1) { // conTB
	    if (dy1 == 0) v2 *= 2;
	    if (dy2 == 0) v3 *= 2;
	    if (dy3 == 0) v1 *= 2;
	}
	else { // conLR
	    if (dx1 == 0) v2 *= 2;
	    if (dx2 == 0) v3 *= 2;
	    if (dx3 == 0) v1 *= 2;
	}
    }

    for (i = firstA; i < lastA; ++i) {
	joiningCon = conNums[i];
	sheetCon = conVal[joiningCon];
	if (v1) conAdd (p1 -> cons[joiningCon], p2 -> cons[joiningCon], v1 * sheetCon, conSort[joiningCon]);
	if (v2) conAdd (p2 -> cons[joiningCon], p3 -> cons[joiningCon], v2 * sheetCon, conSort[joiningCon]);
	if (v3) conAdd (p3 -> cons[joiningCon], p1 -> cons[joiningCon], v3 * sheetCon, conSort[joiningCon]);
    }

    if (doSurfCap && !equiMode) { /* Add parallel plate capacitances. */
	triSurf = meters * meters * A4 / 4;
	for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
	    if (!el -> s.cap.done) {
		if (cap_assign_type & 1) {
		    area = 4 * triSurf / 21;
		    parPlateCap (p1, p1, el, area);
		    parPlateCap (p2, p2, el, area);
		    parPlateCap (p3, p3, el, area);
		    area = triSurf / 14;
		    parPlateCap (p1, p2, el, area);
		    parPlateCap (p1, p3, el, area);
		    parPlateCap (p2, p1, el, area);
		    parPlateCap (p2, p3, el, area);
		    parPlateCap (p3, p2, el, area);
		    parPlateCap (p3, p1, el, area);
		}
		else {
		    area = triSurf / 3;
		    parPlateCap (p1, p1, el, area);
		    parPlateCap (p2, p2, el, area);
		    parPlateCap (p3, p3, el, area);
		}
	    }
	}
    }
}

Private void doEquiRectangle (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP)
{
    /* Processes a rectangle (adds resistances and parallel plate capacitances).
     */
    int i;
    double sheetCon, ratio, area, val, dx, dy;
    coor_t mx, my, xl, xr, br, tr;
    subnode_t snspace, *sn;

    xl = blP -> x;
    xr = brP -> x;
    br = brP -> y;
    tr = trP -> y;

    dx = xr - xl;
    dy = tr - br;

    mx = (xr + xl) / 2;
    my = (tr + br) / 2;

#ifdef DISPLAY
    if (optDisplay) {
	nodePoint_t *p;
	drawTriangleEdge (xl, br, xr, tr);
	drawTriangleEdge (xl, tr, xr, br);
	if (equiMode == 1) { // conTB
	    drawTriangleEdge (xl, tr, xr, tr);
	    for (p = trP -> prev; p != tlP; p = p -> prev) drawTriangleEdge (mx, my, p -> x, tr);
	    for (p = brP -> next; p != blP; p = p -> next) drawTriangleEdge (p -> x, br, mx, my);
	    drawTriangleEdge (xl, br, xr, br);
	    drawEquiEdge (xl, my, xr, my);
	}
	else {
	    drawTriangleEdge (xr, br, xr, tr);
	    for (p = trP -> next; p != brP; p = p -> next) drawTriangleEdge (mx, my, xr, p -> y);
	    for (p = blP -> next; p != tlP; p = p -> next) drawTriangleEdge (xl, p -> y, mx, my);
	    drawTriangleEdge (xl, br, xl, tr);
	    drawEquiEdge (mx, br, mx, tr);
	}
    }
#endif
#ifdef PLOT_CIR_MODE
    if (optPlotCir) {
	nodePoint_t *p;
	plotLine (SOLID, xl, br, xr, tr);
	plotLine (SOLID, xl, tr, xr, br);
	if (equiMode == 1) { // conTB
	    plotLine (SOLID, xl, tr, xr, tr);
	    for (p = trP -> prev; p != tlP; p = p -> prev) plotLine (SOLID, mx, my, p -> x, tr);
	    for (p = brP -> next; p != blP; p = p -> next) plotLine (SOLID, p -> x, br, mx, my);
	    plotLine (SOLID, xl, br, xr, br);
	}
	else {
	    plotLine (SOLID, xr, br, xr, tr);
	    for (p = trP -> next; p != brP; p = p -> next) plotLine (SOLID, mx, my, xr, p -> y);
	    for (p = blP -> next; p != tlP; p = p -> next) plotLine (SOLID, xl, p -> y, mx, my);
	    plotLine (SOLID, xl, br, xl, tr);
	}
    }
#endif

    if (firstB > 0) { lastB = lastA; lastA = firstB; }

    if (extraPoints) { /* Do a separate triangulation for each tile half. */
	nodePoint_t nodePoint, *newP1, *newP2;

	newP1 = createNodePoint (mx, my);
	newP2 = &nodePoint;
	newP2 -> cons = newP1 -> cons;

	for (i = 0; i < lastA; ++i) {
	    joiningCon = conNums[i];
	    newP1 -> cons[joiningCon] = sn = CONS (newP1, joiningCon);
	    subnodeNew2 (sn, Grp (trP -> cons[joiningCon] -> node));
	    sn -> node -> mask = conductorMask[joiningCon];
	    sn -> node -> node_x = mx;
	    sn -> node -> node_y = my;
	    if (equi_line_area) {
		if (corner_line_node) sn -> node -> node_w = equiMode == 1 ? dx : dy;
		makeLineNode (sn);
	    }
	}

	if (equiMode == 1) { // conTB
	   /*  tlP x-----x trP
	    *       \   /
	    *        \ /
	    *     P1 -x- P2    !vPoint
	    *        / \
	    *       /   \
	    *  blP x-----x brP
	    */
	    newP1 -> x = xl; newP2 -> x = xr; newP2 -> y = my;
	    if (trP -> prev == tlP) doRectangle (newP1, newP2, tlP, trP);
	    else e_triangular_core (trP, tlP, newP2, newP1);
	    if (brP -> next == blP) doRectangle (blP, brP, newP1, newP2);
	    else e_triangular_core (blP, brP, newP1, newP2);
	}
	else {
	   /*  tlP x-   P2  -x trP
	    *      | \  |  / |
	    *      |  --x--  |   !hPoint
	    *      | /  |  \ |
	    *  blP x-   P1  -x brP
	    */
	    ASSERT (equiMode == 2); // conLR
	    newP1 -> y = br; newP2 -> x = mx; newP2 -> y = tr;
	    if (trP -> next == brP) doRectangle (newP1, brP, newP2, trP);
	    else e_triangular_core (brP, trP, newP1, newP2);
	    if (blP -> next == tlP) doRectangle (blP, newP1, tlP, newP2);
	    else e_triangular_core (tlP, blP, newP2, newP1);
	}

	for (i = 0; i < lastA; ++i) subnodeDel (newP1 -> cons[conNums[i]]);
	disposeNodePoint (newP1);

	if (firstB > 0) {
	    firstA = firstB; lastA = lastB;
	    i = equiMode;
	    equiMode = 4; /* no cap */
	    triangular_core (trP);
	    equiMode = i;
	    firstA = 0;
	}
	goto ret;
    }

    /* Add resistances. */

    ratio = dy / dx;
    sn = &snspace;

    for (i = 0; i < lastA; ++i) {
	joiningCon = conNums[i];
	sheetCon = conVal[joiningCon];
	subnodeNew2 (sn, Grp (trP -> cons[joiningCon] -> node));
	sn -> node -> node_x = mx;
	sn -> node -> node_y = my;
	sn -> node -> mask = conductorMask[joiningCon];

	if (equi_line_area) {
	    if (corner_line_node) sn -> node -> node_w = equiMode == 1 ? dx : dy;
	    makeLineNode (sn);
	}

	if (equiMode == 1) { // conTB
	    val = 0.5 * sheetCon * ratio;
	    conAdd (tlP -> cons[joiningCon], trP -> cons[joiningCon], val, conSort[joiningCon]);
	    conAdd (blP -> cons[joiningCon], brP -> cons[joiningCon], val, conSort[joiningCon]);
	    val = sheetCon / ratio;
	}
	else {
	    val = 0.5 * sheetCon / ratio;
	    conAdd (tlP -> cons[joiningCon], blP -> cons[joiningCon], val, conSort[joiningCon]);
	    conAdd (trP -> cons[joiningCon], brP -> cons[joiningCon], val, conSort[joiningCon]);
	    val = sheetCon * ratio;
	}
	conAdd (tlP -> cons[joiningCon], sn, val, conSort[joiningCon]);
	conAdd (blP -> cons[joiningCon], sn, val, conSort[joiningCon]);
	conAdd (trP -> cons[joiningCon], sn, val, conSort[joiningCon]);
	conAdd (brP -> cons[joiningCon], sn, val, conSort[joiningCon]);
	subnodeDel (sn);
    }
if (firstB > 0) {
    lastA = lastB;
    for (i = firstB; i < lastB; ++i) {
	joiningCon = conNums[i];
	val = 0.5 * conVal[joiningCon] * ratio;
	conAdd (tlP -> cons[joiningCon], trP -> cons[joiningCon], val, conSort[joiningCon]);
	conAdd (blP -> cons[joiningCon], brP -> cons[joiningCon], val, conSort[joiningCon]);
	val = 0.5 * conVal[joiningCon] / ratio;
	conAdd (tlP -> cons[joiningCon], blP -> cons[joiningCon], val, conSort[joiningCon]);
	conAdd (trP -> cons[joiningCon], brP -> cons[joiningCon], val, conSort[joiningCon]);
    }
}

ret:
    if (doSurfCap) { /* Add parallel plate capacitances. */
	elemDef_t *el;
	double recArea = meters * meters * dx * dy;

	for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
	    if (!el -> s.cap.done) {
		if (cap_assign_type & 1) {
		    area = recArea / 8;
		    parPlateCap (tlP, tlP, el, area);
		    parPlateCap (trP, trP, el, area);
		    parPlateCap (brP, brP, el, area);
		    parPlateCap (blP, blP, el, area);

		    area = recArea / 24;
		    parPlateCap (tlP, trP, el, area);
		    parPlateCap (tlP, blP, el, area);
		    parPlateCap (tlP, brP, el, area);

		    parPlateCap (trP, tlP, el, area);
		    parPlateCap (trP, brP, el, area);
		    parPlateCap (trP, blP, el, area);

		    parPlateCap (brP, blP, el, area);
		    parPlateCap (brP, trP, el, area);
		    parPlateCap (brP, tlP, el, area);

		    parPlateCap (blP, brP, el, area);
		    parPlateCap (blP, tlP, el, area);
		    parPlateCap (blP, trP, el, area);
		}
		else {
		    area = recArea / 4;
		    parPlateCap (tlP, tlP, el, area);
		    parPlateCap (trP, trP, el, area);
		    parPlateCap (brP, brP, el, area);
		    parPlateCap (blP, blP, el, area);
		}
	    }
	}
    }
}

void missingSubArea (int mask, nodePoint_t *p, elemDef_t *el)
{
    char buf[512], *msk;
    int i;

    msk = mask < 0? (mask < -1? "@sub" : "@gnd") : masktable[mask].name;

    sprintf (buf, "warning: value of element '%s' ignored because substrate area\n", el -> name);
    sprintf (buf + strlen (buf), "  for pin '%s' is missing at position %s\n", msk, strCoorBrackets (p -> x, p -> y));
    sprintf (buf + strlen (buf), "  masks present :");
    for (i = 0; i < nrOfCondStd; ++i) {
	if (p -> cons[i]) sprintf (buf + strlen (buf), " %s", conNr2Name (i));
    }
    say (buf);
}

Private void parPlateCap (nodePoint_t *pA, nodePoint_t *pB, elemDef_t *el, double surf)
{
    /* add the parallel plate capacitance between the relevant subnodes of a point */
    subnode_t *subnA, *subnB;
    capElemDef_t *ced = &el -> s.cap;
    int cx, cy, cz;

    cx = ced -> pCon;
    ASSERT (cx >= 0); /* see gettech.cc */
    cy = ced -> nCon;

    subnA = pA -> cons[cx];
    if (!subnA) subnA = pTR -> cons[cx];
    if (!subnA) missingCon (ced -> pMask, ced -> pOccurrence, /* die */
	    currTile, tNull, tNull, el, currTile -> xl, currTile -> bl);
    if (ced -> pKeep) subnA -> node -> term = 2;

    if (cy <= -2) {
	if (substrRes) {
	    if (currTile -> subcont) {
		subnB = currTile -> subcont -> subn;
		goto snB;
	    }
	    missingSubarea (ced -> nMask, currTile, tNull, el, currTile -> xl, currTile -> bl);
	    return; /* don't create parplatecap in this tile
			if substrate contact is smaller */
	}
	subnB = subnSUB;
    }
    else if (cy < 0)
	subnB = subnGND;
    else {
	subnB = pB -> cons[cy];
	if (!subnB) subnB = pTR -> cons[cy];
snB:
	if (!subnB) missingCon (ced -> nMask, ced -> nOccurrence, /* die */
		currTile, tNull, tNull, el, currTile -> xl, currTile -> bl);
	if (ced -> nKeep) subnB -> node -> term = 2;
    }

    capAdd (subnA, subnB, ced -> val * surf, ced -> sortNr);
}

Private int makeContact (nodePoint_t *p, int position, elemDef_t *el, double surf, double contval)
{
    extern bool_t join_contacts;
    subnode_t *sn1, *sn2;
    contElemDef_t *cont = &el -> s.cont;
    int i, j, distr_subcont = 0;

    if ((i = cont -> con1) < 0) { /* see gettech.cc */
	ASSERT (cont -> con1 >= 0);
	return -1;
    }
    ASSERT (i < nrOfCondStd);

    sn1 = p -> cons[joiningCon = i];

    if ((j = cont -> con2) >= 0) {
	sn2 = p -> cons[joiningCon = j];
    }
    else {
	if (currTile -> subcont) {
	    /* We do not connect the substrate contacts in
	       the first pass because we want to number the
	       contacts through the nodes (see subcont.c) */
	    sn2 = currTile -> subcont -> subn;
	    if (currTile -> subcont -> distributed) distr_subcont = 1;
		 /* This substrate terminal is modeled as "distributed".
		    It will have a interconnect resistance mesh above it.
		    We are going to reconnect the substrate resistances
		    from the substrate terminal to the nodes of the
		    interconnect mesh (possibly via an extra node
		    when contact resistance is defined). */
	}
	else if (substrRes) sn2 = NULL;
	else sn2 = subnSUB;
    }

    /* commentSUBAREA:
       When (cont -> con2 == -4) && (sn2 == NULL), or when no substrate
       contact area is present here, we cannot make a substrate connection
       for element 'el' and a return follows in the next if-statement.
       It will probably connect somewhere else then (substrate contact
       area is smaller than the device), or it will not connect all.
    */

    if (!sn1 || !sn2) { /* This is possible by incorrect technology file! */
	missingContact (!sn1 ? 1 : 2, el, currTile -> xl, currTile -> bl);
	return -1;
    }

    if (cont -> keep1) sn1 -> node -> term = 2;
    if (cont -> keep2) sn2 -> node -> term = 2;

    if (distr_subcont) { /* distributed subcont */
	if (!position) {
	    if (sn2 -> node -> term < 2) sn2 -> node -> term = 0;
	}
	else {
	    /* create subnode for a corner of the substrate terminal */
	    subnode_t *subcontSubn = sn2;
	    sn2 = NEW (subnode_t, 1);
	    subnodeNew2 (sn2, Grp (sn1 -> node));
	    sn2 -> node -> node_x = p -> x;
	    sn2 -> node -> node_y = p -> y;
	    sn2 -> node -> mask = -1;
	    sn2 -> node -> substr = 2;
	    switch (position) {
		case POS_TL: currTile -> subcont -> subcontInfo -> subnTL = sn2; break;
		case POS_TR: currTile -> subcont -> subcontInfo -> subnTR = sn2; break;
		case POS_BR: currTile -> subcont -> subcontInfo -> subnBR = sn2; break;
		case POS_BL: currTile -> subcont -> subcontInfo -> subnBL = sn2; break;
		default: ASSERT (position == POS_BL);
	    }
	    sn2 -> node -> term = 2; /* don't eliminate */

	/* Attach 0.25 part of the substrate resistances that are connected to
	   subcontSubn to the subnode at the corner of the substrate terminal.
	*/
	    subnodeSubcontReconnect (subcontSubn, sn2, 0.25);
	    if (position == POS_BL) {
		/* The substrate resistances connected to the
		   substrate terminal were reconnected.
		   Except for one resistance - which will prevent
		   that nodes of one group will become separated -
		   - we delete all substrate resistances that are
		   connected to the substrate terminal node subcontSubn
		*/
		subnodeSubcontEmpty (subcontSubn);
		subnodeJoin (subcontSubn, sn2);
	    }
	}
    }

    /* default: contacts with res <= 0.1 ohm / um ^ 2 are neglected */
    /* see: low_contact_res parameter */

    if (contval > 0) { /* optIntRes */
	if (p == pTR && join_contacts) {
	    if (p -> next) {
		while ((p = p -> next) != pTRb) {
		    subnodeJoin (sn1, p -> cons[i]);
		    if (j >= 0) subnodeJoin (sn2, p -> cons[j]);
		}
	    }
	    sn1 -> node -> area = 2; // makeAreaNode (sn1);
	    if (j >= 0) sn2 -> node -> area = 2; // makeAreaNode (sn2);
	}
	conAdd (sn1, sn2, surf / contval, cont -> sortNr);
    }
    else {
	if (sn2 == subnSUB) { /* !substrRes */
	    if (join_contacts) {
		if (p == pTR) {
		    conAdd (sn1, sn2, surface / 1e-200, cont -> sortNr); /* add dummy */
		    if (p -> next) {
			while ((p = p -> next) != pTRb) subnodeJoin (sn1, p -> cons[i]);
		    }
		    sn1 -> node -> area = 2; // makeAreaNode (sn1);
		}
		return 0;
	    }
	    else if (p == pTR || (cont -> keep1 && cont -> keep2)) {
		conAdd (sn1, sn2, 1e+300, cont -> sortNr); /* add dummy */
		if (min_res <= 0) min_res = 1e+299; /* delete dummy's */
	    }
	    else if (position == POS_BL) { /* join all points */
		sn1 = pTR -> cons[i];
		for (p = pTR; (p = p -> next) != pTRb;) subnodeJoin (sn1, p -> cons[i]);
	    }
	}
	else {
	    if (join_contacts) {
		if (p == pTR) {
		    conAdd (sn1, sn2, surface / 1e-200, cont -> sortNr); /* add dummy */
		    if (p -> next) {
			while ((p = p -> next) != pTRb) {
			    subnodeJoin (sn1, p -> cons[i]);
			    if (j >= 0) subnodeJoin (sn2, p -> cons[j]);
			}
		    }
		    sn1 -> node -> area = 2; // makeAreaNode (sn1);
		    if (j >= 0) sn2 -> node -> area = 2; // makeAreaNode (sn2);
		}
		return 0;
	    }
	    else if (cont -> keep1 && cont -> keep2) {
		conAdd (sn1, sn2, 1e+300, cont -> sortNr); /* add dummy */
		return 0;
	    }
	    else {
		subnodeJoin (sn1, sn2);
	    }
	    if (position == POS_BL && !distr_subcont && extraPoints) { /* join extra points */
		p = pTR;
		while ((p = p -> next)) {
		    if (p -> fixed != 2) { /* extra point */
			sn1 = p -> cons[i];
			subnodeJoin (sn1, (j < 0 ? sn2 : p -> cons[j]));
		    }
		}
	    }
	}
    }
    return 0;
}
