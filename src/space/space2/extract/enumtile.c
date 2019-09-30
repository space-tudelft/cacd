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
#include "src/space/scan/export.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"
#include "src/space/lump/define.h"
#include "src/space/lump/export.h"
#include "src/space/bipolar/define.h"
#include "src/space/bipolar/export.h"

#define conAdd(snA, snB, val, sort) elemAddRes('G', snA -> node, snB -> node, val, sort); ++inRes

#define POS_TL   1
#define POS_TR   2
#define POS_BL   3
#define POS_BR   4

#define tNull (tile_t *)0
#define D(x) (double)(x)
#define L(x) (long)(x)

#define conLR(tile)  (tile -> known & (1+2))
#define conLR1(tile) (tile -> known & 1)
#define conLR2(tile) (tile -> known & 2)
#define conLR5(tile) (tile -> known & 514) == 514
#define conTB(tile)  (tile -> known & (4+8))
#define conTB1(tile) (tile -> known & 4)
#define conTB2(tile) (tile -> known & 8)

#define KEEPNODE(node) node -> term |= 2

extern FILE *xout;
extern mask_t resBitmask;
#define HasResC(color) !COLOR_ABSENT (&color, &resBitmask)

extern int   *helpArray;
extern int    equi_line_new;
extern int    equi_line_old;
extern int    inRes;
extern int    area_contacts;
extern int    join_contacts;
extern int    keep_contacts;
extern double use_corner_ratio;

static elemDef_t **elem;
static tile_t * currTile;
static int doSurfCap;
static int equiMode;
static int cap_i1, cap_i2;
static int cnt_i1, cnt_i2;
static double surface;

static int extraPoints, hPoint, vPoint;
static int firstA, firstB, lastA;
nodePoint_t *pTR, *pBR, *pBL, *pTL;
static nodePoint_t *pTRb;
static nodePoint_t *pf1S, *pf2S, *pbS;
static coor_t doit_m1, doit_m2;
int corner_line_node = 0;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
element_r * elemAddRes (int type, node_t *nA, node_t *nB, double val, int sort);
extern  void addNodeToCluster (node_t *nA, cluster_t *cB);
extern  void clusterIt (nodePoint_t *p1, nodePoint_t *p2);
extern  void joinClusters (cluster_t *cA, cluster_t *cB);
extern  void contTile (tile_t *tile);
extern  void subnodeNetAreaCoor (subnode_t *subn, double area, tile_t *tile);
Private void pp0EnumTile (tile_t *tile);
Private void resEnumTile (tile_t *tile);
Private void triangular (tile_t *tile);
Private void triangular_core (nodePoint_t *rbP);
Private int onSameLine (nodePoint_t *p1, nodePoint_t *p2, nodePoint_t *p3);
Private double edgeLength (nodePoint_t *p1, nodePoint_t *p2);
Private void tryTriangle (nodePoint_t *pf1, nodePoint_t *pf2, nodePoint_t *pb);
Private void doRectangle (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP);
Private void doEquiRectangle (tile_t *tile, int doit);
Private void doTriangle (nodePoint_t *p1, nodePoint_t *p2, nodePoint_t *p3);
Private int RsideOK (coor_t w, mask_t co);
Private int LsideOK (coor_t w, mask_t co);
void missingSubArea (int mask, nodePoint_t *p, elemDef_t *el);
Private void parPlateCap (nodePoint_t *pA, nodePoint_t *pB, elemDef_t *el, double surf);
Private int makeContact (nodePoint_t *p, int position, elemDef_t *el, double surf, double contval);
Private nodePoint_t * newPoint (coor_t x, coor_t y);
Private void newPointDispose (nodePoint_t *p);
Private void doExtraRectangle (coor_t dx, coor_t dy);
Private void doRectangleExtraH (void);
Private void doRectangleExtraV (void);
Private void doRectangleR (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP);
Private void doRectangleL (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP);
Private void doRectangleT (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP);
Private void doRectangleB (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP);
Private void doExtraTriangular (void);
Private void doSimpleTriangle2 (nodePoint_t *p1, nodePoint_t *p2, nodePoint_t *p3);
Private void doSimpleTriangle4 (nodePoint_t *p1, nodePoint_t *p2, nodePoint_t *p3);
#ifdef __cplusplus
  }
#endif

Private void drawCross (coor_t x, coor_t y, coor_t d)
{
    d /= 8;
    fprintf (xout, "f %ld %ld %ld %ld\n", L(x - d), L(y + d), L(x + d), L(y - d));
    fprintf (xout, "f %ld %ld %ld %ld\n", L(x - d), L(y - d), L(x + d), L(y + d));
}

Private void drawTriangleEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
    if (xr < xl) fprintf (xout, "f %ld %ld %ld %ld\n", L(xr), L(yr), L(xl), L(yl));
    else         fprintf (xout, "f %ld %ld %ld %ld\n", L(xl), L(yl), L(xr), L(yr));
}

Private void drawTile (tile_t *tile, int hascond)
{
    char *hexstr = colorHexStr (&tile -> color);
    while (*hexstr == '0') ++hexstr;
    if (*hexstr)
    fprintf (xout, "t%s %s %ld %ld %ld %ld %ld %ld\n",
	tile -> subcont ? "2" : (hascond ? "1" : "0"), hexstr,
	L(tile -> xl), L(tile -> bl), L(tile -> tl), L(tile -> xr), L(tile -> br), L(tile -> tr));
}

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
    int i, j, h, cx;
    capElemDef_t *cap;

    cx = HasConduc (tile);
    if (xout) drawTile (tile, cx);
    if (!cx) return;

    if (prePass) {
	if (bandWidth2) pp0EnumTile (tile);
	return;
    }

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

    h = join_contacts ? 4+2 : 4;
    cnt_i1 = -1;
    for (i = 0; (el = elem[i]); i++) {
	if (el -> type == PNCONELEM) {
fprintf (stderr, "found PNCONELEM: hasBipoElem=%d, hasSurfConnect=%d\n", hasBipoElem, hasSurfConnect);
	}
	else if (el -> type == CONTELEM) {
	    cnt_i2 = i; if (cnt_i1 < 0) cnt_i1 = i;

	    j = el -> s.cont.con1;
	    if ((sn1 = tile -> cons[j])) sn1 -> highres |= h;
	    cx = el -> s.cont.con2;
	    if (cx >= 0) {
		if ((sn2 = tile -> cons[cx])) sn2 -> highres |= h;
	    }
	    else if (!join_contacts && sn1 && !substrRes && optRes && !el -> s.cont.keep1) {
		double v = el -> s.cont.val;
		if (v == 0) sn1 -> highres |= 2; /* join all points */
	    }
	}
	else if (el -> type == SURFCAPELEM || el -> type == SURFCAP3DELEM) break;
    }

if (doSurfCap) {
    if (!el) doSurfCap = 0;
    else {
	h = 0;
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

		joiningCon = el -> s.cont.con1;
		cx = el -> s.cont.con2;
		sn1 = tile -> cons[joiningCon];
		sn2 = cx >= 0 ? tile -> cons[cx] : subnSUB;

		++el -> el_recog_cnt;

		if (!sn1 || !sn2) {
		    missingContact (!sn1 ? 1 : 2, el, tile -> xl, tile -> bl);
		    continue;
		}
		subnodeJoin (sn1, sn2);
	}
    }

    if (doSurfCap) {
	elemDef_t *jun_el = NULL;

	for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
	    cap = &el -> s.cap;
	    if (!cap -> done) {
		if (!(sn1 = tile -> cons[cap -> pCon])) {
		    missingCon (cap -> pMask, cap -> pOccurrence, /* die */
			tile, tNull, tNull, el, tile -> xl, tile -> bl);
		    continue;
		}

		if ((cx = cap -> nCon) < 0) sn2 = (cx <= -2)? subnSUB : subnGND;
		else if (!(sn2 = tile -> cons[cx])) {
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
    register int i;
    register elemDef_t *el;
    double cval;

    for (i = cnt_i1; i <= cnt_i2; i++) { el = elem[i];

	cval = el -> s.cont.val;
	++el -> el_recog_cnt;
	if (split && !join_contacts) {
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
    subnode_t *sn, *sn1;

    if (!(pTR = tile -> rbPoints)) return; /* no conductor */
    pTRb = tile -> tlPoints;
    currTile = tile;

    joiningX = pTR -> x;
    joiningY = pTR -> y;

    extraPoints = 0;
    if ((split = (pTRb != pTR))) { /* high res tile */

	/* Find the conductance values for the different conductor layers.
         * And join subnodes of the two points in the upper-right corner.
	 */
	lastA = 0;
	for (joiningCon = 0; joiningCon < nrOfCondStd; ++joiningCon) {
	    if ((sn = tile -> cons[joiningCon])) {
		sn1 = pTR  -> cons[joiningCon];
		subnodeJoin (sn1, pTRb -> cons[joiningCon]);

		if ((sn -> highres & 3) == 1) { /* highres && !join */
		    conNums[lastA++] = joiningCon;
		    conSort[joiningCon] = sn -> cond -> sortNr;
		    conVal [joiningCon] = sn -> cond -> val;
		    continue;
		}
		/* lowres */
		for (p = pTRb -> next; p; p = p -> next) subnodeJoin (sn1, p -> cons[joiningCon]);
		for (p = pTR  -> next; p; p = p -> next) subnodeJoin (sn1, p -> cons[joiningCon]);
		if (!(sn -> highres & 1) || area_contacts) makeAreaNode (sn1);
	    }
	}
	if (lastA == 0) split = 0;
    }

    if (split) { /* high res tile, lastA > 0 */
        /* perform triangularization and update the surface capacitances */
	triangular (tile);
    }
    else {
	if (pTRb != pTR) {
	    while (pTRb -> next) { pTRb = (p = pTRb) -> next;
		for (i = 0; i < nrOfCondStd; ++i) if ((sn = p -> cons[i])) subnodeDel (sn);
		disposeNodePoint (p);
	    }
	    while (pTR -> next) { pTR = (p = pTR) -> next;
		for (i = 0; i < nrOfCondStd; ++i) if ((sn = p -> cons[i])) subnodeDel (sn);
		disposeNodePoint (p);
	    }
	    ASSERT (pTRb == pTR);
	    tile -> rbPoints = tile -> tlPoints = pTR;
	}
	else {
	    for (i = 0; i < nrOfCondStd; ++i) if ((sn = pTR -> cons[i])) makeAreaNode (sn);
	}
	if (hasBipoElem || hasSurfConnect) resBipoTile (tile, elem, surface);
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
	subtorDel (tile);
    }
}

Private int rmPoint (nodePoint_t *p, nodePoint_t *pPrev, nodePoint_t *pNext)
{
    extern bool_t outputNoDangling;
    register int cx;
    double dl1, dl2;

    ASSERT (pPrev && pNext);

    dl1 = edgeLength (pPrev, p);
    dl2 = edgeLength (p, pNext);

    outputNoDangling = TRUE;
    for (cx = 0; cx < nrOfCondStd; ++cx) {
	if (p -> cons[cx]) {
	    subnodeReconnect (p -> cons[cx], pPrev -> cons[cx], pNext -> cons[cx], dl1, dl2);
	    subnodeDel (p -> cons[cx]);
	}
    }
    outputNoDangling = FALSE;

    disposeNodePoint (p);
    return (1); /* ok */
}

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
    if (conLR5 (tile)) rv |= 2;
    return rv;
}

Private void triangular (tile_t *tile)
{
    static int warn;
    nodePoint_t *point, *pPrev, *pNext;
    coor_t xl, xr, br, tr;
    int cx, i;

    hPoint = vPoint = 0;

    xl = tile -> xl;
    xr = tile -> xr;
    br = tile -> br;
    tr = tile -> tr;

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
		if (!point -> fixed && rmPoint (point, pPrev, pNext)) {
		    pPrev -> next = pNext;
		    continue;
		}
		vPoint = 1;
	    }
	    else pBR = point;
	}
	else if (point -> x != xl) {
	    if (!point -> fixed && rmPoint (point, pPrev, pNext)) {
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
		if (!point -> fixed && rmPoint (point, pPrev, pNext)) {
		    continue;
		}
		vPoint |= 2;
	    }
	    else if (!pTL) pTL = point;
	}
	else if (point -> x != xr) {
	    if (!point -> fixed && rmPoint (point, pPrev, pNext)) {
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

    extraPoints = (hPoint | vPoint);

    /* Here we check if the tile can be covered by an equi-potential
       line to introduce a node with an articulation degree > 1. */

    pTR -> prev = pTRb -> prev;
    firstA = 0;

    if (tile -> bl == br && tile -> tl == tr) { /* Rectangle */
	coor_t dx, dy, w, h;
	int equi_old;
	node_t *nBL;
	subnode_t *sn;

#define HAS_CONTACT(j) (tile -> cons[j] -> highres & 4)

#define HAS_CON_R(j)  (tile -> cons[j] -> highres & 0x10)
#define HAS_CON_L(j)  (tile -> cons[j] -> highres & 0x20)
#define HAS_CON_LR(j) (tile -> cons[j] -> highres & 0x30)

#define HAS_CON_T(j)  (tile -> cons[j] -> highres & 0x40)
#define HAS_CON_B(j)  (tile -> cons[j] -> highres & 0x80)
#define HAS_CON_BT(j) (tile -> cons[j] -> highres & 0xc0)

	dx = xr - xl;
	dy = tr - br;

	firstB = 0;

	equi_old = (equi_line_old && (!vPoint || !hPoint));

	if (equi_old || equi_line_new) {
	    int new2, ok = 0;
	    for (i = 0; i < lastA; ++i) {
		joiningCon = conNums[i];

		if (HAS_CONTACT (joiningCon)) { /* skip */
		    if (equi_line_new) {
			nBL = pBL -> cons[joiningCon] -> node;
			nBL -> node_h = 0;
			nBL -> node_w = 0;
		    }
		    continue;
		}
		new2 = 0;

		if (equi_line_new) {
		    sn = pBL -> cons[joiningCon];
		    nBL = sn -> node;
		if (!HAS_CON_LR(joiningCon)) { /* not left or right same conductor */
		    if ((h = nBL -> node_h)) {
			if (!(hPoint & 1) && !nBL -> area) { /* down no extra point && !lowres */
			    if (D(h + dy) / dx >= equi_line_ratio) { ok |= 1;
				subnodeJoin (sn, pBR -> cons[joiningCon]);
				makeLineNode (sn);
				if (equi_line_new == 2) new2 = 1;
			    }
			}
			else h = 0;
		    }
		    if (HAS_CON_T(joiningCon) && hPoint < 2) pTL -> cons[joiningCon] -> node -> node_h = h + dy;
		}
		else {
		    nBL -> node_h = 0;

		    if (!HAS_CON_BT(joiningCon)) { /* not down or up same conductor */
			if ((w = nBL -> node_w)) {
			    if (vPoint < 2 && !nBL -> area) { /* left no extra point && !lowres */
				if (D(w + dx) / dy >= equi_line_ratio) { ok |= 2;
				    subnodeJoin (sn, pTL -> cons[joiningCon]);
				    makeLineNode (sn);
				    if (equi_line_new == 2) new2 = 1;
				}
			    }
			    else w = 0;
			}
			if (HAS_CON_R(joiningCon) && !(vPoint & 1)) pBR -> cons[joiningCon] -> node -> node_w = w + dx;
		    }
		    else
			nBL -> node_w = 0;
		}}

		if (equi_old && !new2) {
		    if (HAS_CON_BT (joiningCon)) {
			if (!vPoint && !HAS_CON_LR (joiningCon)) {
			    if ((D(dy)/dx >= equi_line_ratio) ||
				    (optResMesh == 2 && dy >= dx/2 && conTB_lr (tile))) {
				if (i != firstB) { conNums[i] = conNums[firstB]; conNums[firstB] = joiningCon; }
				++firstB; equiMode |= 1;
			    }
			}
		    }
		    else {
			if (!hPoint && HAS_CON_LR (joiningCon)) {
			    if ((D(dx)/dy >= equi_line_ratio) ||
				    (optResMesh == 2 && dx >= dy/2 && conLR_tb (tile))) {
				if (i != firstB) { conNums[i] = conNums[firstB]; conNums[firstB] = joiningCon; }
				++firstB; equiMode |= 2;
			    }
			}
		    }
		}
	    }

	    if (ok && xout) {
		if (ok & 1) drawCross (xl + dx/2, br, dx);
		if (ok & 2) drawCross (xl, br + dy/2, dy);
	    }
	}

	if (equiMode) {
	    int doit = 0;

	    ASSERT (firstB > 0);

	    if (equiMode == 3) {
		if (!warn++) say ("warning: can't handle equiMode=3 at position %s", strCoorBrackets (xl, br));
		equiMode = 0;
		goto no_equi;
	    }

	    if (use_corner_ratio > 0) {
		if (equiMode == 1) { /* conTB */
		    if (D(dy)/dx >= 2 * use_corner_ratio) {
			if ((doit = conTB_lr (tile))) {
			    coor_t d = use_corner_ratio * dx;
			    if (d < 2) d = 2;
			    if (doit != 3) {
				if (doit & 1) doit_m1 = tr - d; else doit_m1 = br + d;
			    } else {
				if (dy < 3*d) doit_m2 = doit_m1 = br + dy/2;
				else { doit_m1 = tr - d; doit_m2 = br + d; }
			    }
			}
		    }
		    else corner_line_node = 1;
		} else { /* conLR */
		    if (D(dx)/dy >= 2 * use_corner_ratio) {
			if ((doit = conLR_tb (tile))) {
			    coor_t d = use_corner_ratio * dy;
			    if (d < 2) d = 2;
			    if (doit != 3) {
				if (doit & 1) doit_m1 = xr - d; else doit_m1 = xl + d;
			    } else {
				if (dx < 3*d) doit_m2 = doit_m1 = xl + dx/2;
				else { doit_m1 = xr - d; doit_m2 = xl + d; }
			    }
			}
		    }
		    else corner_line_node = 1;
		}
	    }

	    doEquiRectangle (tile, doit);
	    corner_line_node = 0;
	    equiMode = 0;
	    goto ret;
        }
no_equi:
	if (!extraPoints) {
	    doRectangle (pBL, pBR, pTL, pTR);
	    goto ret;
	}
	/* add extra points to make rectangles */
	doExtraRectangle (dx, dy);
	pTR -> prev = NULL;
	return;
    }

    doExtraTriangular ();
    pTR -> prev = NULL;
    return;
    //triangular_core (pTR);
ret:
    pTR -> prev = NULL;

    for (i = firstA; i < lastA; ++i) {
	joiningCon = conNums[i];
	clusterIt (pTR, pTRb);
    }
}

Private void doExtraRectangle (coor_t dx, coor_t dy)
{
    int i;
    for (i = firstA; i < lastA; ++i) {
	joiningCon = conNums[i];
	clusterIt (pTR, pTRb);
    }
    if (hPoint && (dx > dy || !vPoint)) {
	doRectangleExtraH ();
    }
    else { /* (vPoint && (dy > dx || !hPoint)) */
	doRectangleExtraV ();
    }
}

Private void doRectangleExtraH ()
{
    nodePoint_t *blPnext, *brPprev, *first, *last;
    nodePoint_t *blP, *brP, *pPrev, *pNext;

    blP = NULL;
    first = last = NULL;
    blPnext = pTR;
    brPprev = pBR;
    pPrev = pTR -> prev;
    pNext = pBR -> next;
    while (pPrev != pTL || pNext != pBL) { /* extra point */
	if (pPrev -> x > pNext -> x) {
	    blP = pPrev;
	    brP = newPoint (pPrev -> x, pBL -> y);
	    if (first) last -> prev = brP; else first = brP;
	    last = brP;
	}
	else if (pPrev -> x < pNext -> x) {
	    brP = pNext;
	    blP = newPoint (pNext -> x, pTL -> y);
	    if (first) last -> prev = blP; else first = blP;
	    last = blP;
	}
	else {
	    blP = pPrev;
	    brP = pNext;
	}
	if ((vPoint & 1) && brPprev == pBR) {
	    doRectangleR (brP, brPprev, blP, pTR);
	} else {
	    doRectangle (brP, brPprev, blP, blPnext);
	}
	blPnext = blP;
	brPprev = brP;
	if (blP == pPrev) pPrev = pPrev -> prev;
	if (brP == pNext) pNext = pNext -> next;
    }
    ASSERT (blP);
    if (vPoint & 2) {
	doRectangleL (pBL, brPprev, pTL, blPnext);
    } else {
	doRectangle (pBL, brPprev, pTL, blPnext);
    }

    if (first) { last -> prev = NULL; newPointDispose (first); }
}

Private void doRectangleExtraV ()
{
    nodePoint_t *blPnext, *brPprev, *first, *last;
    nodePoint_t *blP, *brP, *pPrev, *pNext;

    blP = NULL;
    first = last = NULL;
    blPnext = pTL;
    brPprev = pTR;
    pPrev = pTL -> prev;
    pNext = pTR -> next;
    while (pPrev != pBL || pNext != pBR) { /* extra point */
	if (pPrev -> y > pNext -> y) {
	    blP = pPrev;
	    brP = newPoint (pBR -> x, pPrev -> y);
	    if (first) last -> prev = brP; else first = brP;
	    last = brP;
	}
	else if (pPrev -> y < pNext -> y) {
	    brP = pNext;
	    blP = newPoint (pBL -> x, pNext -> y);
	    if (first) last -> prev = blP; else first = blP;
	    last = blP;
	}
	else {
	    blP = pPrev;
	    brP = pNext;
	}
	if ((hPoint & 2) && brPprev == pTR) {
	    doRectangleT (blP, brP, blPnext, brPprev);
	} else {
	    doRectangle (blP, brP, blPnext, brPprev);
	}
	blPnext = blP;
	brPprev = brP;
	if (blP == pPrev) pPrev = pPrev -> prev;
	if (brP == pNext) pNext = pNext -> next;
    }
    ASSERT (blP);
    if (hPoint & 1) {
	doRectangleB (pBL, pBR, blPnext, brPprev);
    } else {
	doRectangle (pBL, pBR, blPnext, brPprev);
    }

    if (first) { last -> prev = NULL; newPointDispose (first); }
}

Private void newPointDispose (nodePoint_t *p)
{
    nodePoint_t *q;
    int i;

    while (p) {
	for (i = firstA; i < lastA; ++i) subnodeDel (p -> cons[conNums[i]]);
	p = (q = p) -> prev;
	disposeNodePoint (q);
    }
}

Private nodePoint_t * newPoint (coor_t x, coor_t y)
{
    int i, j;
    node_t *node;
    subnode_t *sn;
    nodePoint_t *p = createNodePoint (x, y);

    /* init of new point is necessary for parPlateCap */
    for (i = 0; i < nrOfCondStd; ++i) p -> cons[i] = NULL;

    for (i = firstA; i < lastA; ++i) {
	j = conNums[i];
	p -> cons[j] = sn = CONS (p, j);
	node = pTR -> cons[j] -> node;
	subnodeNew2 (sn, Grp (node));
	ASSERT (node -> clr);
	addNodeToCluster (sn -> node, node -> clr);
	sn -> node -> mask = conductorMask[j];
	sn -> node -> node_x = x;
	sn -> node -> node_y = y;
    }
    return (p);
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
    double sheetCon, ratio1, ratio2, area, dx, dy;
    elemDef_t *el;

    dx = brP -> x - blP -> x;
    dy = trP -> y - brP -> y;

    ratio1 = dy / dx;
    ratio2 = dx / dy;

    for (i = firstA; i < lastA; ++i) {
	joiningCon = conNums[i];
	sheetCon = 0.5 * conVal[joiningCon];
	conAdd (tlP -> cons[joiningCon], trP -> cons[joiningCon], sheetCon * ratio1, conSort[joiningCon]);
	conAdd (blP -> cons[joiningCon], brP -> cons[joiningCon], sheetCon * ratio1, conSort[joiningCon]);
	conAdd (tlP -> cons[joiningCon], blP -> cons[joiningCon], sheetCon * ratio2, conSort[joiningCon]);
	conAdd (trP -> cons[joiningCon], brP -> cons[joiningCon], sheetCon * ratio2, conSort[joiningCon]);
    }

    if (!equiMode) {
	if (xout) {
	    drawTriangleEdge (blP -> x, blP -> y, brP -> x, brP -> y);
	    drawTriangleEdge (blP -> x, blP -> y, tlP -> x, tlP -> y);
	    drawTriangleEdge (trP -> x, trP -> y, tlP -> x, tlP -> y);
	    drawTriangleEdge (trP -> x, trP -> y, brP -> x, brP -> y);
	}
	if (doSurfCap) { /* Add parallel plate capacitances. */
	    area = meters * meters * dx * dy / 4;
	    for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
		if (!el -> s.cap.done) {
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
    double A4, sheetCon, area, v1, v2, v3;
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

    A4 = (x1 * dy1 + x2 * dy2 + x3 * dy3) * 2; /* triangle_area * 4 */
    if (A4 < 0) A4 = -A4; /* positive! */

    dx1 = x2 - x3;
    dx2 = x3 - x1;
    dx3 = x1 - x2;

    v1 = (dy1 * -dy2 + dx2 * -dx1) / A4;
    v2 = (dy2 * -dy3 + dx3 * -dx2) / A4;
    v3 = (dy3 * -dy1 + dx1 * -dx3) / A4;

    for (i = firstA; i < lastA; ++i) {
	joiningCon = conNums[i];
	sheetCon = conVal[joiningCon];
	if (v1) { conAdd (p1 -> cons[joiningCon], p2 -> cons[joiningCon], v1 * sheetCon, conSort[joiningCon]); }
	if (v2) { conAdd (p2 -> cons[joiningCon], p3 -> cons[joiningCon], v2 * sheetCon, conSort[joiningCon]); }
	if (v3) { conAdd (p3 -> cons[joiningCon], p1 -> cons[joiningCon], v3 * sheetCon, conSort[joiningCon]); }
    }

    if (!equiMode) {
	if (xout) {
	    drawTriangleEdge (x1, y1, x2, y2);
	    drawTriangleEdge (x1, y1, x3, y3);
	    drawTriangleEdge (x2, y2, x3, y3);
	}
	if (doSurfCap) { /* Add parallel plate capacitances. */
	    area = meters * meters * (A4 / 4) / 3;
	    for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
		if (!el -> s.cap.done) {
		    parPlateCap (p1, p1, el, area);
		    parPlateCap (p2, p2, el, area);
		    parPlateCap (p3, p3, el, area);
		}
	    }
	}
    }
}

Private void doEquiRectangle (tile_t *tile, int doit)
{
    coor_t xl, xr, br, tr;
    coor_t mx, my, mx1, my1, mx2, my2;
    double sheetCon, sheetCon2, ratio, area, val, oldval, dx, dy;
    subnode_t snspace, snspace2, *sn1, *sn2;
    nodePoint_t *pl, *pr;
    int i, rx;

    xl = pBL -> x;
    xr = pBR -> x;
    br = pBR -> y;
    tr = pTR -> y;
    mx2 = mx1 = mx = xl + (xr - xl) / 2;
    my2 = my1 = my = br + (tr - br) / 2;

    sn2 = sn1 = &snspace;

    if (doit) {
	corner_line_node = 1;
	if (doit == 3) {
	    sn2 = &snspace2;
	    mx1 = xr - (xr - doit_m1) / 2;
	    my1 = tr - (tr - doit_m1) / 2;
	    mx2 = xl + (doit_m2 - xl) / 2;
	    my2 = br + (doit_m2 - br) / 2;
	}
	else if (doit == 2) {
	    mx2 = mx1 = xl + (doit_m1 - xl) / 2;
	    my2 = my1 = br + (doit_m1 - br) / 2;
	}
	else {/* doit == 1 */
	    mx2 = mx1 = xr - (xr - doit_m1) / 2;
	    my2 = my1 = tr - (tr - doit_m1) / 2;
	}
    }

    for (i = 0; i < firstB; ++i) {
	joiningCon = conNums[i];
	rx = conSort[joiningCon];
	sheetCon2 = conVal[joiningCon];
	sheetCon = sheetCon2 / 2;

	subnodeNew2 (sn1, Grp (pTR -> cons[joiningCon] -> node));
	sn1 -> node -> mask = conductorMask[joiningCon];
	makeLineNode (sn1);

	if (HAS_CON_BT (joiningCon)) { /* conTB */
	    sn1 -> node -> node_x = mx;
	    sn1 -> node -> node_y = my1;
	    if (doit == 3) {
		subnodeNew2 (sn2, Grp (sn1 -> node));
		sn2 -> node -> node_x = mx;
		sn2 -> node -> node_y = my2;
		sn2 -> node -> mask = conductorMask[joiningCon];
		makeLineNode (sn2);
	    }
	    //----------------------------------------------------------
	    dy = tr - my1;
	    oldval = 0;
	    pr = pTR;
	    do {
		pl = pr -> prev;
		ratio = dy / (pr -> x - pl -> x);
		conAdd (pl -> cons[joiningCon], pr -> cons[joiningCon], sheetCon2 * ratio, rx);
		val = sheetCon / ratio;
		conAdd (pr -> cons[joiningCon], sn1, oldval + val, rx);
		oldval = val;
	    } while ((pr = pl) != pTL);
	    conAdd (pl -> cons[joiningCon], sn1, val, rx);
	    //----------------------------------------------------------
	    dy = my2 - br;
	    oldval = 0;
	    pr = pBR;
	    do {
		pl = pr -> next;
		ratio = dy / (pr -> x - pl -> x);
		conAdd (pl -> cons[joiningCon], pr -> cons[joiningCon], sheetCon2 * ratio, rx);
		val = sheetCon / ratio;
		conAdd (pr -> cons[joiningCon], sn2, oldval + val, rx);
		oldval = val;
	    } while ((pr = pl) != pBL);
	    conAdd (pl -> cons[joiningCon], sn2, val, rx);
	    //----------------------------------------------------------
	    clusterIt (pTL, pTRb);
	    clusterIt (pBR, pBL);
	    if (doit == 3) {
		ratio = (my1 - my2) / (double)(xr - xl);
		conAdd (sn1, sn2, sheetCon2 * ratio, rx);
		subnodeDel (sn2);
	    }
	}
	else { /* conLR */
	    sn1 -> node -> node_x = mx1;
	    sn1 -> node -> node_y = my;
	    if (doit == 3) {
		subnodeNew2 (sn2, Grp (sn1 -> node));
		sn2 -> node -> node_x = mx2;
		sn2 -> node -> node_y = my;
		sn2 -> node -> mask = conductorMask[joiningCon];
		makeLineNode (sn2);
	    }
	    //----------------------------------------------------------
	    dx = xr - mx1;
	    oldval = 0;
	    pr = pTR;
	    do {
		pl = pr -> next;
		ratio = (pr -> y - pl -> y) / dx;
		conAdd (pr -> cons[joiningCon], pl -> cons[joiningCon], sheetCon2 / ratio, rx);
		val = sheetCon * ratio;
		conAdd (pr -> cons[joiningCon], sn1, oldval + val, rx);
		oldval = val;
	    } while ((pr = pl) != pBR);
	    conAdd (pl -> cons[joiningCon], sn1, val, rx);
	    //----------------------------------------------------------
	    dx = mx2 - xl;
	    oldval = 0;
	    pr = pTL;
	    do {
		pl = pr -> prev;
		ratio = (pr -> y - pl -> y) / dx;
		conAdd (pr -> cons[joiningCon], pl -> cons[joiningCon], sheetCon2 / ratio, rx);
		val = sheetCon * ratio;
		conAdd (pr -> cons[joiningCon], sn2, oldval + val, rx);
		oldval = val;
	    } while ((pr = pl) != pBL);
	    conAdd (pl -> cons[joiningCon], sn2, val, rx);
	    //----------------------------------------------------------
	    clusterIt (pTR, pBR);
	    clusterIt (pBL, pTL);
	    if (doit == 3) {
		ratio = (mx1 - mx2) / (double)(tr - br);
		conAdd (sn1, sn2, sheetCon2 * ratio, rx);
		subnodeDel (sn2);
	    }
	}
	subnodeDel (sn1);
    }

    if (xout) {
	nodePoint_t *p;
	if (equiMode & 1) { /* conTB */
	    drawTriangleEdge (xl, tr, xr, tr);
	    for (p = pTR;; p = p -> prev) { drawTriangleEdge (mx, my1, p -> x, tr); if (p == pTL) break; }
	    for (p = pBR;; p = p -> next) { drawTriangleEdge (p -> x, br, mx, my2); if (p == pBL) break; }
	    if (my2 < my1) drawTriangleEdge (mx, my2, mx, my1);
	    drawTriangleEdge (xl, br, xr, br);
	}
	else { /* conLR */
	    drawTriangleEdge (xr, br, xr, tr);
	    for (p = pTR;; p = p -> next) { drawTriangleEdge (mx1, my, xr, p -> y); if (p == pBR) break; }
	    for (p = pBL;; p = p -> next) { drawTriangleEdge (xl, p -> y, mx2, my); if (p == pTL) break; }
	    if (mx2 < mx1) drawTriangleEdge (mx2, my, mx1, my);
	    drawTriangleEdge (xl, br, xl, tr);
	}
    }

    if ((firstA = firstB) < lastA) {
	ASSERT (equiMode);
	if (extraPoints) {
	    doExtraRectangle (xr - xl, tr - br);
	    firstA = lastA; /* don't cluster */
	}
	else {
	    doRectangle (pBL, pBR, pTL, pTR);
	}
    }

    if (doSurfCap) { /* Add parallel plate capacitances. */
	elemDef_t *el;
	area = surface / 4;
	for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
	    if (!el -> s.cap.done) {
		parPlateCap (pTL, pTL, el, area);
		parPlateCap (pTR, pTR, el, area);
		parPlateCap (pBR, pBR, el, area);
		parPlateCap (pBL, pBL, el, area);
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
    for (i = 0; i < nrOfCondStd; i++) {
	if (p -> cons[i]) sprintf (buf + strlen (buf), " %s", conNr2Name (i));
    }
    say (buf);
}

Private void parPlateCap (nodePoint_t *pA, nodePoint_t *pB, elemDef_t *el, double surf)
{
    /* add the parallel plate capacitance between the relevant subnodes of a point */
    subnode_t *subnA, *subnB;
    capElemDef_t *ced = &el -> s.cap;
    int cx, cy;

    cx = ced -> pCon;
    cy = ced -> nCon;

    subnA = pA -> cons[cx];
    if (!subnA) subnA = pTR -> cons[cx];
    if (!subnA) missingCon (ced -> pMask, ced -> pOccurrence, /* die */
	    currTile, tNull, tNull, el, currTile -> xl, currTile -> bl);
    if (ced -> pKeep) KEEPNODE (subnA -> node);

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
	if (ced -> nKeep) KEEPNODE (subnB -> node);
    }

    capAdd (subnA, subnB, ced -> val * surf, ced -> sortNr);
}

Private int makeContact (nodePoint_t *p, int position, elemDef_t *el, double surf, double contval)
{
    subnode_t *sn1, *sn2;
    contElemDef_t *cont = &el -> s.cont;
    int j, keep_contact = 0, distr_subcont = 0;

    sn1 = p -> cons[joiningCon = cont -> con1];

    if ((j = cont -> con2) >= 0) sn2 = p -> cons[j];
    else if (currTile -> subcont) {
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

    if (!sn1 || !sn2) { /* This is possible by incorrect technology file! */
	missingContact (!sn1 ? 1 : 2, el, currTile -> xl, currTile -> bl);
	return -1;
    }

    if (keep_contacts || cont -> keep1) { KEEPNODE (sn1 -> node); ++keep_contact; }
    if (j < 0) keep_contact = 2;
    else if (keep_contacts || cont -> keep2) { KEEPNODE (sn2 -> node); ++keep_contact; }

    if (distr_subcont && position) { /* distributed subcont */
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
	KEEPNODE (sn2 -> node); /* don't eliminate */

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

    if (contval > 0) { /* optIntRes */
	conAdd (sn1, sn2, surf / contval, cont -> sortNr);
    }
    else if (keep_contact == 2) { /* by j < 0 (substrate) always */
	contval = 1e-200; /* add dummy */
	conAdd (sn1, sn2, surf / contval, cont -> sortNr);
    }
    else {
	subnodeJoin (sn1, sn2);
	if (position == POS_BL && extraPoints) { /* join extra points */
	    p = pTR;
	    while ((p = p -> next)) if (p -> fixed != 2) { /* extra point */
		subnodeJoin (p -> cons[joiningCon], p -> cons[j]);
	    }
	}
    }

    if (position == POS_BL || !position) { /* last makeContact */
	if (j >= 0 && sn1 -> node -> clr != sn2 -> node -> clr) {
	    if (sn1 -> node -> clr && sn2 -> node -> clr)
		joinClusters (sn1 -> node -> clr, sn2 -> node -> clr);
	}
    }
    return 0;
}

Private void doRectangleR (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP)
{
    nodePoint_t *pL, *pR, *prev = NULL;

    while ((pR = trP -> next) != brP) { /* extra point */
	pL = newPoint (tlP -> x, pR -> y);
	pL -> prev = prev;
	doRectangle (pL, pR, tlP, trP);
	prev = tlP = pL; trP = pR;
    }
    doRectangle (blP, brP, tlP, trP);

    if (prev) newPointDispose (prev);
}

Private void doRectangleL (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP)
{
    nodePoint_t *pL, *pR, *prev = NULL;

    while ((pL = tlP -> prev) != blP) { /* extra point */
	pR = newPoint (trP -> x, pL -> y);
	pR -> prev = prev;
	doRectangle (pL, pR, tlP, trP);
	tlP = pL; prev = trP = pR;
    }
    doRectangle (blP, brP, tlP, trP);

    if (prev) newPointDispose (prev);
}

Private void doRectangleT (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP)
{
    nodePoint_t *pT, *pB, *prev = NULL;

    while ((pT = trP -> prev) != tlP) { /* extra point */
	pB = newPoint (pT -> x, brP -> y);
	pB -> prev = prev;
	doRectangle (pB, brP, pT, trP);
	trP = pT; prev = brP = pB;
    }
    doRectangle (blP, brP, tlP, trP);

    if (prev) newPointDispose (prev);
}

Private void doRectangleB (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP)
{
    nodePoint_t *pT, *pB, *prev = NULL;

    while ((pB = brP -> next) != blP) { /* extra point */
	pT = newPoint (pB -> x, trP -> y);
	pT -> prev = prev;
	doRectangle (pB, brP, pT, trP);
	prev = trP = pT; brP = pB;
    }
    doRectangle (blP, brP, tlP, trP);

    if (prev) newPointDispose (prev);
}

Private void doSimpleRectangleTB (nodePoint_t *blP, nodePoint_t *brP, nodePoint_t *tlP, nodePoint_t *trP)
{
    nodePoint_t *tP, *bP, *otP;
    double sheetCon, ratioB, ratioT, ratioM, dx, dy, conLT, conRT;
    double conM, conT, conB, conL, conR, conX, conY, conZ, contot;
    int i;

    dx = brP -> x - blP -> x;

    dy = trP -> y - tlP -> y;
    if (dy > 0) { tP = tlP; ratioT =  dy / dx; } else
    if (dy < 0) { tP = trP; ratioT = -dy / dx; } else { tP = NULL; ratioT = 0; }

    dy = brP -> y - blP -> y;
    if (dy > 0) { bP = brP; ratioB =  dy / dx; } else
    if (dy < 0) { bP = blP; ratioB = -dy / dx; } else { bP = NULL; ratioB = 0; }

    otP = NULL;
    if (tP) {
	if (!bP)
	    bP = (tP == tlP)? brP : blP;
	else if (bP -> x == tP -> x)
	    otP = (tP == tlP)? trP : tlP;
    }
    else if (bP == blP) tP = trP;
    else { bP = brP; tP = tlP; }

    dy = tP -> y - bP -> y;
    ASSERT (dy > 0);
    ratioM = dy / dx;

    for (i = firstA; i < lastA; ++i) {
	joiningCon = conNums[i];
	sheetCon = 0.5 * conVal[joiningCon];

	conT = conB = sheetCon * ratioM;
	conZ = sheetCon / ratioM;
	conL = conR = conLT = conRT = conZ;

	if (ratioB) {
	    conX = sheetCon * ratioB + conB;
	    conY = sheetCon / ratioB;
	    contot = conX + conY + conZ;

	    conB = (conX * conY) / contot;
	    conM = (conX * conZ) / contot;
	    if (bP == blP) {
		conR = (conY * conZ) / contot;
	    }
	    else {
		conL = (conY * conZ) / contot;
	    }
	    if (otP) conZ = bP == blP ? conR : conL;
	}
	else conM = 0;

	if (ratioT) {
	    conX = sheetCon * ratioT + conT;
	    conY = sheetCon / ratioT;
	    contot = conX + conY + conZ;

	    if (otP) {
		contot += conM;
		conT = (conX * conY) / contot;
		if (bP == blP) {
		    conL += (conX * conM) / contot;
		    conR = (conY * conZ) / contot;
		}
		else {
		    conR += (conX * conM) / contot;
		    conL = (conY * conZ) / contot;
		}
		conB += (conZ * conM) / contot;
		conM = (conY * conM) / contot;
		conZ = (conX * conZ) / contot;
	    }
	    else {
		conT = (conX * conY) / contot;
		conX = (conX * conZ) / contot;
		if (tP == tlP) {
		    conRT = (conY * conZ) / contot;
		    conR = conRT;
		}
		else {
		    conLT = (conY * conZ) / contot;
		    conL = conLT;
		}
		conM += conX;
	    }
	}

	conAdd (tlP -> cons[joiningCon], trP -> cons[joiningCon], conT, conSort[joiningCon]);
	conAdd (blP -> cons[joiningCon], brP -> cons[joiningCon], conB, conSort[joiningCon]);
	conAdd (tlP -> cons[joiningCon], blP -> cons[joiningCon], conL, conSort[joiningCon]);
	conAdd (trP -> cons[joiningCon], brP -> cons[joiningCon], conR, conSort[joiningCon]);
	if (otP) {
	    if (tP == tlP) {
		   conAdd (tP -> cons[joiningCon], brP -> cons[joiningCon], conZ, conSort[joiningCon]); }
	    else { conAdd (tP -> cons[joiningCon], blP -> cons[joiningCon], conZ, conSort[joiningCon]); }
	    conAdd (otP -> cons[joiningCon], bP -> cons[joiningCon], conM, conSort[joiningCon]);
	}
	else if (conM) {
	    conAdd (tP -> cons[joiningCon], bP -> cons[joiningCon], conM, conSort[joiningCon]); }
    }
    if (xout) {
	drawTriangleEdge (tlP -> x, tlP -> y, trP -> x, trP -> y);
	drawTriangleEdge (blP -> x, blP -> y, brP -> x, brP -> y);
	drawTriangleEdge (blP -> x, blP -> y, tlP -> x, tlP -> y);
	drawTriangleEdge (brP -> x, brP -> y, trP -> x, trP -> y);
    }
}

Private void doSimpleTriangle4 (nodePoint_t *p1, nodePoint_t *p2, nodePoint_t *p3)
{
    double A2, con1, con2, con3, sheetCon;
    double ratio1, ratio2, ratio3, contot, con12, con13, con23;
    double dx1, dx2, dx3, dy1, dy2, dy3, h, w1, w2, w3;
    int i;

    dx1 = p2 -> x - p3 -> x;
    dx2 = p3 -> x - p1 -> x;
    dx3 = p1 -> x - p2 -> x;

    dy1 = p2 -> y - p3 -> y;
    dy2 = p3 -> y - p1 -> y;
    dy3 = p1 -> y - p2 -> y;

    A2 = (p1 -> x * dy1 + p2 -> x * dy2 + p3 -> x * dy3); /* triangle_area * 2 */
    if (A2 < 0) A2 = -A2; /* positive! */

    w1 = sqrt (dy1 * dy1 + dx1 * dx1);
    w2 = sqrt (dy2 * dy2 + dx2 * dx2);
    w3 = sqrt (dy3 * dy3 + dx3 * dx3);

    if (w3 >= w1 && w3 >= w2) ;
    else if (w2 >= w1 && w2 >= w3)
	 { Swap (double, w2, w3); Swap (nodePoint_t *, p2, p3); }
    else { Swap (double, w1, w3); Swap (nodePoint_t *, p1, p3); }

    if (w2 < w1) { w1 = w2; Swap (nodePoint_t *, p1, p2); }

    h = A2 / w3;
    w1 = sqrt (w1*w1 - h*h);

    ratio1 = (h / (w3 - w1));
    ratio2 = (h / w1);
    ratio3 = (w3 / h);

    for (i = firstA; i < lastA; ++i) {
	joiningCon = conNums[i];
	sheetCon = 0.5 * conVal[joiningCon];

	con1 = sheetCon * ratio1;
	con2 = sheetCon * ratio2;
	con3 = sheetCon * ratio3;
	contot = con1 + con2 + con3;
	con12 = (con1 * con2) / contot;
	con13 = (con1 * con3) / contot;
	con23 = (con2 * con3) / contot;
//fprintf (stderr, "doSimpleTriangle4: con12=%g con13=%g con23=%g\n", con12, con13, con23);

	conAdd (p1 -> cons[joiningCon], p2 -> cons[joiningCon], con12, conSort[joiningCon]);
	conAdd (p2 -> cons[joiningCon], p3 -> cons[joiningCon], con23, conSort[joiningCon]);
	conAdd (p3 -> cons[joiningCon], p1 -> cons[joiningCon], con13, conSort[joiningCon]);
    }
    if (xout) {
	drawTriangleEdge (p1 -> x, p1 -> y, p2 -> x, p2 -> y);
	drawTriangleEdge (p1 -> x, p1 -> y, p3 -> x, p3 -> y);
	drawTriangleEdge (p2 -> x, p2 -> y, p3 -> x, p3 -> y);
    }
}

Private void doSimpleTriangle2 (nodePoint_t *p1, nodePoint_t *p2, nodePoint_t *p3)
{
    double sheetCon, ratio, dx, dy;
    int i; /* p1 is corner of 90 degree */

    if (p2 -> x == p1 -> x) Swap (nodePoint_t *, p2, p3);
    dx = p2 -> x - p1 -> x;
    dy = p3 -> y - p1 -> y;
    ratio = dy / dx;
    if (ratio < 0) ratio = -ratio;

    for (i = firstA; i < lastA; ++i) {
	joiningCon = conNums[i];
	sheetCon = 0.5 * conVal[joiningCon];
	conAdd (p1 -> cons[joiningCon], p2 -> cons[joiningCon], sheetCon * ratio, conSort[joiningCon]);
	conAdd (p1 -> cons[joiningCon], p3 -> cons[joiningCon], sheetCon / ratio, conSort[joiningCon]);
    }
    if (xout) {
	drawTriangleEdge (p1 -> x, p1 -> y, p2 -> x, p2 -> y);
	drawTriangleEdge (p1 -> x, p1 -> y, p3 -> x, p3 -> y);
	drawTriangleEdge (p2 -> x, p2 -> y, p3 -> x, p3 -> y);
    }
}

Private void doSimpleTriangle (nodePoint_t *p1, nodePoint_t *p2, nodePoint_t *p3)
{
    double sheetCon;
    int i; /* p1 is corner of 90 degree */

    for (i = firstA; i < lastA; ++i) {
	joiningCon = conNums[i];
	sheetCon = conVal[joiningCon];
	conAdd (p1 -> cons[joiningCon], p2 -> cons[joiningCon], sheetCon / 4, conSort[joiningCon]);
	conAdd (p1 -> cons[joiningCon], p3 -> cons[joiningCon], sheetCon / 4, conSort[joiningCon]);
	conAdd (p2 -> cons[joiningCon], p3 -> cons[joiningCon], sheetCon / 8, conSort[joiningCon]);
    }
    if (xout) {
	drawTriangleEdge (p1 -> x, p1 -> y, p2 -> x, p2 -> y);
	drawTriangleEdge (p1 -> x, p1 -> y, p3 -> x, p3 -> y);
	drawTriangleEdge (p2 -> x, p2 -> y, p3 -> x, p3 -> y);
    }
}

Private void doTriangleTB ()
{
    nodePoint_t *blP, *brP, *p;

    if (pTL == pBL) {
	brP = pTR -> next;
	if (brP == pBR) { /* no extra vert. points */
	    brP = pTL;
	    blP = brP -> prev;
	    doSimpleTriangle4 (brP -> next, blP, brP); brP = brP -> next;
	    while (blP != pBR || brP != pTRb) {
		if (brP == pTRb ||
		   (blP != pBR && edgeLength (blP -> prev, brP) < edgeLength (brP -> next, blP))) {
		    doSimpleTriangle4 (blP -> prev, brP, blP); blP = blP -> prev;
		} else {
		    doSimpleTriangle4 (brP -> next, blP, brP); brP = brP -> next;
		}
	    }
	}
	else if (pBL -> y == pBR -> y) { /* TOP(1) BOT(3) */
	    p = pTR; do { doSimpleTriangle4 (p -> prev, p, brP); p = p -> prev; } while (p != pTL);
	    while (brP -> next != pBR) { doSimpleTriangle4 (pBL, brP, brP -> next); brP = brP -> next; }
	    p = pBL; do { doSimpleTriangle4 (p, brP, p -> prev); p = p -> prev; } while (p != pBR);
	}
	else { /* TOP(3) BOT(2) */
	    ASSERT (pTL -> y == pTR -> y);
	    p = pTR; do { doSimpleTriangle4 (brP, p -> prev, p); p = p -> prev; } while (p != pTL);
	    while (brP -> next != pBR) { doSimpleTriangle4 (brP -> next, pBL, brP); brP = brP -> next; }
	    p = pBL; do { doSimpleTriangle4 (p, p -> prev, brP); p = p -> prev; } while (p != pBR);
	}
    }
    else {
	ASSERT (pTR == pBR);
	blP = pTL -> prev;
	if (blP == pBL) { /* no extra vert. points */
	    brP = pTR;
	    blP = brP -> prev;
	    doSimpleTriangle4 (brP -> next, blP, brP); brP = brP -> next;
	    while (blP != pTL || brP != pBL) {
		if (brP == pBL ||
		   (blP != pTL && edgeLength (blP -> prev, brP) < edgeLength (brP -> next, blP))) {
		    doSimpleTriangle4 (blP -> prev, brP, blP); blP = blP -> prev;
		} else {
		    doSimpleTriangle4 (brP -> next, blP, brP); brP = brP -> next;
		}
	    }
	}
	else if (pBL -> y == pBR -> y) { /* TOP(2) BOT(3) */
	    p = pTL; do { doSimpleTriangle4 (p -> next, p, blP); p = p -> next; } while (p != pTRb);
	    while (blP -> prev != pBL) { doSimpleTriangle4 (blP, pBR, blP -> prev); blP = blP -> prev; }
	    p = pBR; do { doSimpleTriangle4 (blP, p, p -> next); p = p -> next; } while (p != pBL);
	}
	else { /* TOP(3) BOT(1) */
	    ASSERT (pTL -> y == pTR -> y);
	    p = pTL; do { doSimpleTriangle4 (blP, p -> next, p); p = p -> next; } while (p != pTRb);
	    while (blP -> prev != pBL) { doSimpleTriangle4 (blP -> prev, pBR, blP); blP = blP -> prev; }
	    p = pBR; do { doSimpleTriangle4 (p -> next, p, blP); p = p -> next; } while (p != pBL);
	}
    }
}

Private void doExtraTriangular ()
{
    nodePoint_t *blP, *brP, *tlP, *trP, *p, *prev;
    elemDef_t *el;
    coor_t d;
    int i;

    for (i = firstA; i < lastA; ++i) {
	joiningCon = conNums[i];
	clusterIt (pTR, pTRb);
    }

    prev = NULL;

    if (pTL -> y < pTR -> y) { /* TOP(1) */
	if (pBL -> y < pBR -> y) { /* BOT(1) */
	    if (pBR -> y > pTL -> y) { /* special case */
		d = (pTL -> y - pBL -> y) / 2;
		blP = newPoint (pBL -> x + d, pBL -> y + d);
		trP = newPoint (pTR -> x - d, pTR -> y - d);
		p = pBL; do { doSimpleTriangle4 (blP, p, p -> next); p = p -> next; } while (p != pTL);
		doRectangle (blP, pBR, pTL, trP);
		p = pBR; do { doSimpleTriangle4 (trP, p, p -> prev); p = p -> prev; } while (p != pTR);
		trP -> prev = blP;
		newPointDispose (trP);
		goto ret;
	    }
	    tlP = pTL;
	    trP = pTR -> next;
	    blP = tlP -> prev;
	    brP = pBR;
	    if (trP == pBR && pTL -> y > pBR -> y) prev = trP = newPoint (pTR -> x, pTL -> y);
	    p = pTR; do { doSimpleTriangle4 (p -> prev, p, trP); p = p -> prev; } while (p != pTL);
	    if (!prev) {
		while (trP != pBR && trP -> next -> y >= tlP -> y) {
		    doSimpleTriangle4 (tlP, trP, trP -> next); trP = trP -> next;
		}
		while (blP -> y >= trP -> y) {
		    doSimpleTriangle4 (tlP, trP, blP); blP = (tlP = blP) -> prev;
		}
	    }
	    goto bot1;
	}
	else if (pBL -> y > pBR -> y) { /* BOT(2) */
	    tlP = pTL;
	    trP = pTR -> next;
	    blP = pBL;
	    if (trP == pBR) prev = trP = newPoint (pTR -> x, pTL -> y);
	    p = pTR; do { doSimpleTriangle4 (p -> prev, p, trP); p = p -> prev; } while (p != pTL);
	    if (!prev) {
		brP = trP -> next;
		while (brP -> y >= tlP -> y) {
		    doSimpleTriangle4 (tlP, trP, brP); brP = (trP = brP) -> next;
		}
		while (tlP != pBL && tlP -> prev -> y >= trP -> y) {
		    doSimpleTriangle4 (tlP, trP, tlP -> prev); tlP = tlP -> prev;
		}
	    }
	    else brP = pBR;
	    goto bot2;
	}
	else { /* BOT(3) */
	    if (pTL != pBL) { /* MID */
		tlP = pTL;
		trP = pTR -> next;

		if ((hPoint & 1)) { /* extra point on bottom edge */
		    /* there must be a trP, because we do the bottom separate */
		    if (trP == pBR) prev = trP = newPoint (pTR -> x, pTL -> y);
		    p = pTR; do { doSimpleTriangle4 (p -> prev, p, trP); p = p -> prev; } while (p != pTL);
		    while (trP -> y > tlP -> y) {
			doSimpleTriangle4 (tlP, trP, trP -> next);
			trP = trP -> next;
		    }
		    if (trP -> y < tlP -> y) {
			prev = newPoint (tlP -> x, trP -> y);
			doSimpleTriangle4 (tlP, trP, prev);
			tlP = prev;
		    }
		    if (trP == pBR) {
			p = pBL; do { doSimpleTriangle4 (tlP, p -> prev, p); p = p -> prev; } while (p != pBR);
		    }
		    else doRectangleB (pBL, pBR, tlP, trP);
		}
		else {
		    if ((hPoint & 2)) { /* extra point on top edge */
			/* there must be a trP, because we do the top separate */
			if (trP == pBR) { prev = trP = newPoint (pTR -> x, pTL -> y); brP = pBR; }
			else brP = trP -> next;
			p = pTR; do { doSimpleTriangle4 (p -> prev, p, trP); p = p -> prev; } while (p != pTL);
		    }
		    else brP = (trP = pTR) -> next;

		    while (brP -> y >= tlP -> y) {
			doSimpleTriangle4 (tlP, trP, brP);
			brP = (trP = brP) -> next;
		    }
		    blP = tlP -> prev;

		    while (blP != pBL || brP != pBR) {
			if (blP -> y > brP -> y) {
			    p = newPoint (pBR -> x, blP -> y); p -> prev = prev; prev = p;
			    doSimpleRectangleTB (blP, p, tlP, trP);
			    blP = (tlP = blP) -> prev;
			    trP = p;
			}
			else if (blP -> y < brP -> y) {
			    p = newPoint (pBL -> x, brP -> y); p -> prev = prev; prev = p;
			    doSimpleRectangleTB (p, brP, tlP, trP);
			    tlP = p;
			    brP = (trP = brP) -> next;
			}
			else {
			    doSimpleRectangleTB (blP, brP, tlP, trP);
			    blP = (tlP = blP) -> prev;
			    brP = (trP = brP) -> next;
			}
		    }
		    doSimpleRectangleTB (pBL, pBR, tlP, trP);
		}
		if (prev) newPointDispose (prev);
	    }
	    else {
		doTriangleTB ();
	    }
	}
    }
    else if (pTL -> y > pTR -> y) { /* TOP(2) */
	if (pBL -> y < pBR -> y) { /* BOT(1) */
	    tlP = pTL -> prev;
	    trP = pTR;
	    brP = pBR;
	    if (tlP == pBL) prev = tlP = newPoint (pTL -> x, pTR -> y);
	    p = pTL; do { doSimpleTriangle4 (p, p -> next, tlP); p = p -> next; } while (p != pTRb);
	    if (!prev) { /* tlP != pBL */
		blP = tlP -> prev;
		while (blP -> y >= trP -> y) {
		    doSimpleTriangle4 (trP, tlP, blP); blP = (tlP = blP) -> prev;
		}
		while (trP != pBR && trP -> next -> y >= tlP -> y) {
		    doSimpleTriangle4 (tlP, trP, trP -> next); trP = trP -> next;
		}
	    }
	    else blP = pBL;
	    goto bot1;
	}
	else if (pBL -> y > pBR -> y) { /* BOT(2) */
	    if (pBL -> y > pTR -> y) { /* special case */
		d = (pTL -> y - pBL -> y) / 2;
		tlP = newPoint (pBL -> x + d, pBL -> y + d);
		brP = newPoint (pTR -> x - d, pTR -> y - d);
		p = pBL; do { doSimpleTriangle4 (tlP, p, p -> next); p = p -> next; } while (p != pTL);
		doRectangle (pBL, brP, tlP, pTR);
		p = pBR; do { doSimpleTriangle4 (brP, p, p -> prev); p = p -> prev; } while (p != pTR);
		brP -> prev = tlP;
		newPointDispose (brP);
		goto ret;
	    }
	    tlP = pTL -> prev;
	    trP = pTR;
	    blP = pBL;
	    brP = trP -> next;
	    if (tlP == pBL && pBL -> y < pTR -> y) prev = tlP = newPoint (pTL -> x, pTR -> y);
	    p = pTL; do { doSimpleTriangle4 (p, p -> next, tlP); p = p -> next; } while (p != pTRb);
	    if (!prev) {
		while (tlP != pBL && tlP -> prev -> y >= trP -> y) {
		    doSimpleTriangle4 (trP, tlP, tlP -> prev); tlP = tlP -> prev;
		}
		while (brP -> y >= tlP -> y) {
		    doSimpleTriangle4 (tlP, trP, brP); brP = (trP = brP) -> next;
		}
	    }
	    goto bot2;
	}
	else { /* BOT(3) */
	    if (pTR != pBR) { /* MID */
		tlP = pTL -> prev;
		trP = pTR;

		if ((hPoint & 1)) { /* extra point on bottom edge */
		    /* there must be a tlP, because we do the bottom separate */
		    if (tlP == pBL) prev = tlP = newPoint (pTL -> x, pTR -> y);
		    p = pTL; do { doSimpleTriangle4 (p, p -> next, tlP); p = p -> next; } while (p != pTRb);
		    while (tlP -> y > trP -> y) {
			doSimpleTriangle4 (tlP, trP, tlP -> prev);
			tlP = tlP -> prev;
		    }
		    if (tlP -> y < trP -> y) {
			prev = newPoint (trP -> x, tlP -> y);
			doSimpleTriangle4 (tlP, trP, prev);
			trP = prev;
		    }
		    if (tlP == pBL) {
			p = pBR; do { doSimpleTriangle4 (p -> next, trP, p); p = p -> next; } while (p != pBL);
		    }
		    else doRectangleB (pBL, pBR, tlP, trP);
		}
		else {
		    if ((hPoint & 2)) { /* extra point on top edge */
			/* there must be a tlP, because we do the top separate */
			if (tlP == pBL) { prev = tlP = newPoint (pTL -> x, pTR -> y); blP = pBL; }
			else blP = tlP -> prev;
			p = pTL; do { doSimpleTriangle4 (p, p -> next, tlP); p = p -> next; } while (p != pTRb);
		    }
		    else blP = (tlP = pTL) -> prev;

		    while (blP -> y >= trP -> y) {
			doSimpleTriangle4 (tlP, trP, blP);
			blP = (tlP = blP) -> prev;
		    }
		    brP = trP -> next;

		    while (blP != pBL || brP != pBR) {
			if (blP -> y > brP -> y) {
			    p = newPoint (pBR -> x, blP -> y); p -> prev = prev; prev = p;
			    doSimpleRectangleTB (blP, p, tlP, trP);
			    blP = (tlP = blP) -> prev;
			    trP = p;
			}
			else if (blP -> y < brP -> y) {
			    p = newPoint (pBL -> x, brP -> y); p -> prev = prev; prev = p;
			    doSimpleRectangleTB (p, brP, tlP, trP);
			    tlP = p;
			    brP = (trP = brP) -> next;
			}
			else {
			    doSimpleRectangleTB (blP, brP, tlP, trP);
			    blP = (tlP = blP) -> prev;
			    brP = (trP = brP) -> next;
			}
		    }
		    doSimpleRectangleTB (pBL, pBR, tlP, trP);
		}
		if (prev) newPointDispose (prev);
	    }
	    else {
		doTriangleTB ();
	    }
	}
    }
    else if (pBL -> y < pBR -> y) { /* T3-B1 */
	tlP = pTL;
	trP = pTR;
	blP = tlP -> prev;
	brP = pBR;

	if ((hPoint & 2)) { /* extra point on top edge */
	    if (trP != pBR) { /* MID */
		brP = trP -> next;
		if (blP -> y > brP -> y) {
		    prev = newPoint (pBR -> x, blP -> y);
		    doRectangleT (blP, prev, tlP, trP);
		    blP = (tlP = blP) -> prev;
		    trP = prev;
		}
		else if (blP -> y < brP -> y) {
		    prev = newPoint (pBL -> x, brP -> y);
		    doRectangleT (prev, brP, tlP, trP);
		    tlP = prev;
		    trP = brP;
		}
		else {
		    doRectangleT (blP, brP, tlP, trP);
		    blP = (tlP = blP) -> prev;
		    trP = brP;
		}
	    }
	    else {
		doTriangleTB ();
		goto ret;
	    }
	}
bot1:
	if (trP != pBR) { /* MID */
	    if (trP -> next) brP = trP -> next;
	    if (blP != pBL) {
		while (brP != pBR) {
		    if (blP -> y < brP -> y) {
			p = newPoint (pBL -> x, brP -> y);
			p -> prev = prev; prev = p;
			doSimpleRectangleTB (p, brP, tlP, trP);
			tlP = p; brP = (trP = brP) -> next;
		    }
		    else if (blP -> y > brP -> y) {
			p = newPoint (pBR -> x, blP -> y);
			p -> prev = prev; prev = p;
			doSimpleRectangleTB (blP, p, tlP, trP);
			blP = (tlP = blP) -> prev; trP = p;
		    }
		    else {
			doSimpleRectangleTB (blP, brP, tlP, trP);
			blP = (tlP = blP) -> prev;
			brP = (trP = brP) -> next;
		    }
		}
		while (blP != pBL) {
		    if (blP -> y > pBR -> y
#if 1
			    && blP -> prev != pBL
#endif
		    ) {
			p = newPoint (pBR -> x, blP -> y);
			p -> prev = prev; prev = p;
			doSimpleRectangleTB (blP, p, tlP, trP);
			trP = p;
		    }
		    else if (trP != brP) {
			doSimpleRectangleTB (blP, brP, tlP, trP);
			trP = brP;
		    }
		    else doSimpleTriangle4 (blP, brP, tlP);
		    blP = (tlP = blP) -> prev;
		}
	    }
	    else {
		while (brP != pBR) {
		    p = newPoint (pBL -> x, brP -> y);
		    p -> prev = prev; prev = p;
		    doSimpleRectangleTB (p, brP, tlP, trP);
		    tlP = p;
		    brP = (trP = brP) -> next;
		}
	    }
	    if (trP != brP) {
		if (tlP -> y > brP -> y) { /* MID */
		    if ((hPoint & 1)) { /* extra point on bottom edge */
			p = newPoint (pBL -> x, brP -> y);
			p -> prev = prev; prev = p;
			doSimpleRectangleTB (p, brP, tlP, trP);
			tlP = p;
		    }
		    else {
			doSimpleRectangleTB (blP, brP, tlP, trP);
			tlP = NULL;
		    }
		}
		else doSimpleTriangle4 (tlP, trP, brP);
	    }
	}
	if (tlP) {
	    while (blP != pBL) {
		doSimpleTriangle4 (blP, pBR, tlP);
		blP = (tlP = blP) -> prev;
	    }
	    p = pBR; do { doSimpleTriangle4 (p -> next, p, tlP); p = p -> next; } while (p != pBL);
	}
	if (prev) newPointDispose (prev);
    }
    else { /* T3-B2 */
	ASSERT (pBL -> y > pBR -> y);
	tlP = pTL;
	trP = pTR;
	blP = pBL;
	brP = trP -> next;

	if ((hPoint & 2)) { /* extra point on top edge */
	    if (tlP != pBL) { /* MID */
		blP = tlP -> prev;
		if (blP -> y > brP -> y) {
		    prev = newPoint (pBR -> x, blP -> y);
		    doRectangleT (blP, prev, tlP, trP);
		    tlP = blP;
		    trP = prev;
		}
		else if (blP -> y < brP -> y) {
		    prev = newPoint (pBL -> x, brP -> y);
		    doRectangleT (prev, brP, tlP, trP);
		    tlP = prev;
		    brP = (trP = brP) -> next;
		}
		else {
		    doRectangleT (blP, brP, tlP, trP);
		    tlP = blP;
		    brP = (trP = brP) -> next;
		}
	    }
	    else {
		doTriangleTB ();
		goto ret;
	    }
	}
bot2:
	if (tlP != pBL) { /* MID */
	    if (tlP -> prev) blP = tlP -> prev;
	    if (brP != pBR) {
		while (blP != pBL) {
		    if (blP -> y < brP -> y) {
			p = newPoint (pBL -> x, brP -> y);
			p -> prev = prev; prev = p;
			doSimpleRectangleTB (p, brP, tlP, trP);
			tlP = p; brP = (trP = brP) -> next;
		    }
		    else if (blP -> y > brP -> y) {
			p = newPoint (pBR -> x, blP -> y);
			p -> prev = prev; prev = p;
			doSimpleRectangleTB (blP, p, tlP, trP);
			blP = (tlP = blP) -> prev; trP = p;
		    }
		    else {
			doSimpleRectangleTB (blP, brP, tlP, trP);
			blP = (tlP = blP) -> prev;
			brP = (trP = brP) -> next;
		    }
		}
		while (brP != pBR) {
		    if (brP -> y > pBL -> y
#if 1
			    && brP -> next != pBR
#endif
		    ) {
			p = newPoint (pBL -> x, brP -> y);
			p -> prev = prev; prev = p;
			doSimpleRectangleTB (p, brP, tlP, trP);
			tlP = p;
		    }
		    else if (tlP != blP) {
			doSimpleRectangleTB (blP, brP, tlP, trP);
			tlP = blP;
		    }
		    else doSimpleTriangle4 (blP, brP, trP);
		    brP = (trP = brP) -> next;
		}
	    }
	    else { /* brP == pBR */
		while (blP != pBL) {
		    p = newPoint (pBR -> x, blP -> y);
		    p -> prev = prev; prev = p;
		    doSimpleRectangleTB (blP, p, tlP, trP);
		    blP = (tlP = blP) -> prev; trP = p;
		}
	    }
	    if (tlP != blP) {
		if (trP -> y > blP -> y) { /* MID */
		    if ((hPoint & 1)) { /* extra point on bottom edge */
			p = newPoint (pBR -> x, blP -> y);
			p -> prev = prev; prev = p;
			doSimpleRectangleTB (blP, p, tlP, trP);
			trP = p;
		    }
		    else {
			doSimpleRectangleTB (blP, brP, tlP, trP);
			trP = NULL;
		    }
		}
		else doSimpleTriangle4 (tlP, trP, blP);
	    }
	}
	if (trP) {
	    while (brP != pBR) {
		doSimpleTriangle4 (pBL, brP, trP);
		brP = (trP = brP) -> next;
	    }
	    p = pBL; do { doSimpleTriangle4 (p, p -> prev, trP); p = p -> prev; } while (p != pBR);
	}
	if (prev) newPointDispose (prev);
    }
ret:
    if (doSurfCap) {
	if (pTL == pBL) { /* triangle? */
	    if (pBL -> y == pBR -> y) {
		for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
		    if (!el -> s.cap.done) {
			parPlateCap (pTL, pTL, el, surface * 0.25);
			parPlateCap (pTR, pTR, el, surface * 0.25);
			parPlateCap (pBR, pBR, el, surface * 0.5);
		    }
		}
		return;
	    }
	    else if (pTL -> y == pTR -> y) {
		for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
		    if (!el -> s.cap.done) {
			parPlateCap (pTR, pTR, el, surface * 0.5);
			parPlateCap (pBL, pBL, el, surface * 0.25);
			parPlateCap (pBR, pBR, el, surface * 0.25);
		    }
		}
		return;
	    }
	}
	else if (pTR == pBR) { /* triangle? */
	    if (pBL -> y == pBR -> y) {
		for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
		    if (!el -> s.cap.done) {
			parPlateCap (pTL, pTL, el, surface * 0.25);
			parPlateCap (pTR, pTR, el, surface * 0.25);
			parPlateCap (pBL, pBL, el, surface * 0.5);
		    }
		}
		return;
	    }
	    else if (pTL -> y == pTR -> y) {
		for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
		    if (!el -> s.cap.done) {
			parPlateCap (pTL, pTL, el, surface * 0.5);
			parPlateCap (pBR, pBR, el, surface * 0.25);
			parPlateCap (pBL, pBL, el, surface * 0.25);
		    }
		}
		return;
	    }
	}
	for (i = cap_i1; i < cap_i2; i++) { el = elem[i];
	    if (!el -> s.cap.done) {
		parPlateCap (pTL, pTL, el, surface * 0.25);
		parPlateCap (pTR, pTR, el, surface * 0.25);
		parPlateCap (pBL, pBL, el, surface * 0.25);
		parPlateCap (pBR, pBR, el, surface * 0.25);
	    }
	}
    }
}
