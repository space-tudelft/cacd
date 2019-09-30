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
#include <math.h>
#include <string.h>
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/export.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"
#include "src/space/substr/export.h"
#include "src/space/lump/define.h"
#include "src/space/lump/export.h"
#include "src/space/bipolar/define.h"
#include "src/space/bipolar/export.h"

// #define PR_EDGE_INFO /* for debugging (SdeG) */

#define tNull (tile_t *)0
#define pNull (nodePoint_t *)0

#define KEEPNODE(node) node -> term |= 2

#define SET_conLR1(tile) tile -> known |= 1
#define SET_conLR2(tile) tile -> known |= 2
#define SET_conLR5(tile) tile -> known |= 512
#define SET_conTB1(tile) tile -> known |= 4
#define SET_conTB2(tile) tile -> known |= 8
#define conTB(tile) (tile -> known & (4+8))
#define SET_cVBJT(tile) tile -> known |= 16
#define SET_KNOW1(tile) tile -> known |= 128

extern elemDef_t ** otherElemSpace;
extern int condWarnCnt;
extern bool_t *has_polnode;
extern int *helpArray;

coor_t lastY = INF;

extern tileBoundary_t *bdr;
double bdr_length;
static int tHasConduc, nHasConduc, newtile;
static int both_same_highres;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void pp1EnumPair (tile_t *tile, tile_t *newerTile, int edgeOrien);
Private void pp2EnumPair (tile_t *tile, tile_t *newerTile, int edgeOrien);
Private void resEnumPair (tile_t *tile, tile_t *newerTile, int edgeOrien);
Private void setBoundary (tile_t *tile, tile_t *newerTile, int edgeOrien);
Private terminal_t *findNextTerm (tile_t *tileA, tile_t *tileB, int *fromB);
Private void placePoints (tile_t *tile, tile_t *newerTile, int edgeOrien, coor_t x, coor_t y);
Private void connectPoints (tile_t *tile, tile_t *otherTile, int edgeOrien, coor_t x, coor_t y);
Private void joinPoints (tile_t *tile, tile_t *otherTile, int edgeOrien);
Private void updateTorEdge (tile_t *tile, tile_t *adjTile,
    subnode_t **adjCons, nodePoint_t *adjPointStart, int edgeOrien, int adjIsNewer);
Private int updateEdgeCap (elemDef_t **elem, tile_t *tile, tile_t *adjTile);
Private int updateResEdgeCap (elemDef_t **elem, tile_t *tile,
    tile_t *adjTile, nodePoint_t *pointStart, nodePoint_t *adjPointStart, int use_ttl);
void missingSubArea (int mask, nodePoint_t *p, elemDef_t *el);
Private void twoSubContacts (elemDef_t *el_old, elemDef_t *el_new, coor_t x, coor_t y);
void missingTermCon (terminal_t *term);
#ifdef __cplusplus
  }
#endif

/* enumPair (tile, newerTile, edgeOrien)
 *
 * will update the network description for the influence
 * of the boundary between 'tile' and 'newerTile'.
 *
 * actions performed :
 *
 * - For both 'tile' and 'newerTile' a list of recognized
 *   elements is fetched.
 *
 * - If 'newerTile' is passed to enumPair for the first time
 *   resistance and transistor elements are investigated to
 *   allocate appropriate subnodes and subtors for newerTile.
 *   When resistance extraction is performed, a point
 *   will also be allocated for 'newerTile'.
 *
 * - Joins are made between subnodes and subtors of the
 *   same type that are present in both 'tile' and
 *   'newerTile'.
 *
 * - Terminal names are assigned to the appropriate subnodes.
 *   When resistances are extracted a point is placed for
 *   each terminal position.
 *
 * - When a transistor is present in one tile and a
 *   transistor of the same type is not present in the
 *   other tile, the transistor edge is updated.
 *
 * - Then the edge capacitances are calculated.
 *   For both 'tile' and 'newerTile' the list of recognized
 *   elements is searched for capacitor elements that have
 *   one pin connection layer that is an edge layer and one
 *   pin connection layer that is a a surface layer.
 *   Capacitances are calculated and assigned between the
 *   subnodes of the two pin connection layers.
 *
 */
void enumPair (tile_t *tile, tile_t *newerTile, int edgeOrien)
{
    int cx, i, resPresent;
    elemDef_t *el, **elem, **elem2;
    resElemDef_t *res;
    subnode_t *sn, *st;
    terminal_t *term;

    if ((newtile = !newerTile -> cnt)) newerTile -> cnt = ++tileCnt;

    nHasConduc = HasConduc (newerTile);
    tHasConduc = HasConduc (tile);
    cx = nHasConduc || tHasConduc;

    setBoundary (tile, newerTile, edgeOrien);
    if (!cx) return;

    joiningX = bdr -> x1;
    joiningY = bdr -> y1;

    if (!extrPass) {
	if (prePass1) { pp1EnumPair (tile, newerTile, edgeOrien); return; }
	if (prePass) { pp2EnumPair (tile, newerTile, edgeOrien); return; }
	ASSERT (extrPass);
    }
    if (optRes) {
	resEnumPair (tile, newerTile, edgeOrien);
	return;
    }

    if (nHasConduc) {
	if (newtile) {
	    resPresent = 0;
	    elem = recogSurface (newerTile);

	    ASSERT (edgeOrien == 'v');

	    for (i = 0; (el = elem[i]); i++) {
		switch (el -> type) {
		    case RESELEM:
			res = &el -> s.res;
			cx = res -> con;

			++el -> el_recog_cnt;
			if (!resPresent) resPresent = 1, tileConCnt++;

			if (newerTile -> cons[cx]) {
			    if (++condWarnCnt <= 10) {
				say ("warning: two conductors for '%s' at position %s",
				    masktable[res -> mask].name, strCoorBrackets (newerTile -> xl, newerTile -> bl));
			    }
			    break;
			}
			newerTile -> cons[cx] = sn = CONS (newerTile, cx);
			sn -> cond = res;

			/* Note: polnodes of adjacent masks are not connected if
			   they have a different polarity type (see subnodeCopy).
			*/
			if ((bdr_length && (st = tile -> cons[cx])) || (st = newerTile -> stb -> cons[cx])) {
			    if (st -> cond -> type != sn -> cond -> type) {
				st = newerTile -> stb -> cons[cx];
				if (!st || st -> cond -> type != sn -> cond -> type) {
				    warnSubnodeJoin (cx, joiningX, joiningY, 1);
				    goto new_subnode;
				}
			    }
			    subnodeCopy (st, sn);
			}
			else {
new_subnode:
			    subnodeNew (sn);
			    if (has_polnode[cx]) polnodeAdd (sn, cx, res -> type);
			    sn -> node -> mask = conductorMask[cx];
			    sn -> node -> node_x = newerTile -> xl;
			    sn -> node -> node_y = newerTile -> bl;
			}
			break;

		    case VBJTELEM:
			SET_cVBJT (newerTile);
			break;

		    case TORELEM:
			++el -> el_recog_cnt;
			if (newerTile -> tor) {
			    say ("warning: two different transistors '%s' and '%s'",
				 newerTile -> tor -> type -> name, el -> name);
			    say ("\tat position %s (transistor '%s' skipped).",
				 strCoorBrackets (newerTile -> xl, newerTile -> bl), el -> name);
			    break;
			}
			if (tile -> tor && tile -> tor -> type == el && bdr_length) {
			    subtorCopy (tile, newerTile);
			} else if (newerTile -> stb -> tor && newerTile -> stb -> tor -> type == el) {
			    subtorCopy (newerTile -> stb, newerTile);
			} else {
			    subtorNew (newerTile, el);
			}
		}
	    }
	}
	else if (tHasConduc && bdr_length) {
            /* Make joins between subnodes in 'tile' and subnodes
	     * in 'newerTile'.
	     */
	    for (joiningCon = 0; joiningCon < nrOfCondStd; ++joiningCon) {
		if ((st = tile -> cons[joiningCon]) && (sn = newerTile -> cons[joiningCon])) {
		    if (st -> cond -> type != sn -> cond -> type) {
			warnSubnodeJoin (joiningCon, joiningX, joiningY, 1);
		    }
		    else subnodeJoin (st, sn);
		}
	    }

            /* If 'tile' and 'newerTile' both have a transistor of
	     * the same type, join them.
	     */
	    if (tile -> tor && newerTile -> tor && tile -> tor -> type == newerTile -> tor -> type) {
		subtorJoin (tile, newerTile);
	    }
	}
    }

    /* Assign terminal names to the appropriate subnodes. */

    if (edgeOrien == 'v' && newLookTerm (bdr -> x1, bdr -> y1)) {
	while ((term = findNextTerm (tile, newerTile, &i))) {
	    if (i) sn = newerTile -> cons[term -> conductor];
	    else   sn = tile -> cons[term -> conductor];
	    nameAdd (sn, term);
	}
    }

    if (!bdr_length) return;

    elem = elem2 = NULL;
    if ((hasBipoElem || hasEdgeConnect) && nHasConduc && tHasConduc) {
	/* First enumerate boundary from new to old */
	elem = recogEdge (newerTile, tile, NULL);
	bipoPair (newerTile, tile, elem, edgeOrien);
	elem2 = recogEdge (tile, newerTile, otherElemSpace);
	bipoPair (tile, newerTile, elem2, edgeOrien);
    }

    /* Update transistor edges */

    if (newerTile -> tor) updateTorEdge (newerTile, tile, tile -> cons, pNull, edgeOrien, 0);
    if (tile -> tor) updateTorEdge (tile, newerTile, newerTile -> cons, pNull, edgeOrien, 1);

    /* Update edge capacitances */

    if (extrEdgeCaps && !COLOR_EQ_COLOR (&tile -> color, &newerTile -> color)) {

	if (tHasConduc) {
	    if (!elem) elem = recogEdge (newerTile, tile, NULL);
	    i = updateEdgeCap (elem, newerTile, tile);
	    if (i >= 0 && edgeOrien == 'h') { /* optLatCap */
		lastY = bdr -> y2;
		if (bdr -> y2 > bdr -> y1) {
		    updateLateralCap (tile, newerTile, elem, 'v', i);
		}
	    }
	}

	if (nHasConduc) {
	    if (!elem2) elem2 = recogEdge (tile, newerTile, otherElemSpace);
	    i = updateEdgeCap (elem2, tile, newerTile);
	    if (i >= 0) { /* optLatCap */
		/* lateral coup. cap. are only searched backwards */
		/* WARNING: elem2 must be in otherElemSpace, because
		   function updateLatEdgeCap does recogEdge */

		updateLateralCap (newerTile, tile, elem2, edgeOrien, i);
		if (edgeOrien == 'h' && bdr -> y2 < bdr -> y1) {
		    updateLateralCap (newerTile, tile, elem2, 'v', i);
		}
	    }
	}
    }
}

Private void pp1EnumPair (tile_t *tile, tile_t *newerTile, int edgeOrien)
{
    int cx, i;
    elemDef_t *el, **elem;
    resElemDef_t *res;
    subnode_t *sn;
    subcontRef_t *tsc = tile -> subcont;
    subcontRef_t *nsc = newerTile -> subcont;

    if (nHasConduc) {
	if (newtile) {
	    elemDef_t *newsub_elem = NULL;

	    elem = recogSurface (newerTile);

	    for (i = 0; (el = elem[i]); i++) {
		switch (el -> type) {
		    case RESELEM:
			if (optIntRes) {
			    res = &el -> s.res;
			    cx = res -> con;
			    if (newerTile -> cons[cx]) break;
			    newerTile -> cons[cx] = sn = CONS (newerTile, cx);
			    sn -> highres = (res -> val > 0);
			}
			break;

		    case TORELEM:
			if (el -> s.tor.bCon <= -2) {
			    cx = -1;
			    if (el -> s.tor.bCon == -2) goto do_it;
			    goto do_it2;
			}
			break;

		    case CONTELEM:
			if (el -> s.cont.con2 <= -2) {
			    cx = el -> s.cont.con1;
			    if (el -> s.cont.con2 == -2) goto do_it;
			    goto do_it2;
			}
			break;

		    case SUBCONTELEM:
			cx = el -> s.subc.ccnr;
do_it:
			if (newsub_elem) {
			    twoSubContacts (newsub_elem, el, newerTile -> xl, newerTile -> bl);
			    break;
			}
do_it2:
			newsub_elem = el;
			if (!nsc) newerTile -> subcont = nsc = subContNew (newerTile);
			nsc -> causing_con = cx;
		}
	    }

	    if (nsc && optSubResSave && (cx = nsc -> causing_con) >= 0) {
		if ((sn = newerTile -> cons[cx]) && sn -> highres) nsc -> distributed = sep_on_res[cx];
		if (mergeNeighborSubContacts && !nsc -> distributed) nsc -> causing_con = -1;
	    }
	}
    }

    if ((tsc || nsc) && bdr_length) {
	if (tsc && nsc) {
	    /* For optSimpleSubRes we always join adjacent substrate terminals
	       since makedela can not (yet) handle these. */
	    if (optSimpleSubRes
		|| (!tsc -> distributed && !nsc -> distributed
		    && tsc -> causing_con == nsc -> causing_con)) {
		subContJoin (tile, newerTile);
	    }
	    else subContGroupJoin (tsc -> subcontInfo, nsc -> subcontInfo);

	    if (optSubResSave && edgeOrien != 'v') {
		if (tsc -> distributed || nsc -> distributed
			|| tsc -> causing_con != nsc -> causing_con) {
		    contEdge (tile, newerTile);
		}
	    }
	}
	else if (optSimpleSubRes) {
	    if (tsc) tsc -> subcontInfo -> perimeter += bdr_length;
	    else     nsc -> subcontInfo -> perimeter += bdr_length;
	    if (edgeOrien != 'v') contEdge (tile, newerTile);
	}
    }
}

Private void newpoint (tile_t *tile, coor_t x, coor_t y, int mode)
{
    nodePoint_t *newPoint = createNodePoint (x, y);
    if (mode) { newPoint -> next = tile -> rbPoints; tile -> rbPoints = newPoint; }
    else      { newPoint -> next = tile -> tlPoints; tile -> tlPoints = newPoint; }
}

Private void pp0EnumPair (tile_t *tile, tile_t *newerTile, int edgeOrien) /* prePass == 1 */
{
    coor_t y1, y2;
    int cx, i, resPresent;
    elemDef_t  *el, **elem;
    resElemDef_t *res;
    subnode_t  *sn;
    terminal_t *term;

    if (nHasConduc && newtile) {
	elem = recogSurface (newerTile);
	el = elem[i = 0];
	for (; el && el -> type != RESELEM; el = elem[++i]) ;
	for (; el && el -> type == RESELEM; el = elem[++i]) {
	    res = &el -> s.res;
	    cx = res -> con;
	    if (!newerTile -> cons[cx]) {
		newerTile -> cons[cx] = sn = CONS (newerTile, cx);
		sn -> cond = res; // ??
		sn -> highres = 0;
		if (res -> val > 0) {
		    sn -> highres = 1;
		    SET_KNOW1 (newerTile);
		    if (!newerTile -> tlPoints)
			newerTile -> tlPoints =
			newerTile -> rbPoints = createNodePoint (newerTile -> xl, newerTile -> bl);
		}
	    }
	}
    }

    resPresent = 0;

    if (bdr_length) {
	if (HasHighRes (tile)) ++resPresent;
	if (HasHighRes (newerTile)) ++resPresent;
	if (resPresent == 2) {
	    for (i = 0; i < nrOfCondStd; ++i) {
		if ((sn = newerTile -> cons[i]) && sn -> highres)
		     if ((sn = tile -> cons[i]) && sn -> highres) { resPresent = 3; break; }
	    }
	}
    }
    if (!resPresent) {
	if (edgeOrien == 'v' && newLookTerm (bdr -> x1, bdr -> y1)) {
	    while (findNextTerm (tile, newerTile, &i)) ;
	}
	return;
    }

    if (edgeOrien == 'v') {
	int f = 0;
	if (resPresent == 3) { SET_conLR1 (tile); SET_conLR2 (newerTile); }
	if (newLookTerm (bdr -> x1, bdr -> y1)) {
	    y2 = y1 = bdr -> y1;
	    while ((term = findNextTerm (tile, newerTile, &i))) {
		if (term -> y < bdr -> y2) {
		    i = term -> conductor;
		    if ((sn = tile -> cons[i]) && sn -> highres) {
			if (term -> y > y1) newpoint (tile, bdr -> x2, (y1 = term -> y), 1);
			tile -> rbPoints -> fixed = 1;
		    }
		    if ((sn = newerTile -> cons[i]) && sn -> highres) {
			if (term -> y > y2) newpoint (newerTile, bdr -> x2, (y2 = term -> y), 0);
			newerTile -> tlPoints -> fixed = 1;
		    }
		}
		else f = 1;
	    }
	}
	if (tile -> tlPoints) {
	    newpoint (tile, bdr -> x2, bdr -> y2, 1);
	    if (f) tile -> rbPoints -> fixed = 1;
	}
	if (newerTile -> tlPoints) {
	    newpoint (newerTile, bdr -> x2, bdr -> y2, 0);
	    if (f) newerTile -> tlPoints -> fixed = 1;
	}
    }
    else {
	if (resPresent == 3) { SET_conTB1 (tile); SET_conTB2 (newerTile); }
	if (tile -> tlPoints)      newpoint (tile, bdr -> x2, bdr -> y2, 0);
	if (newerTile -> tlPoints) newpoint (newerTile, bdr -> x2, bdr -> y2, 1);
    }
}

Private void pp2EnumPair (tile_t *tile, tile_t *newerTile, int edgeOrien) /* prePass */
{
    int cx, i, resPresent;
    elemDef_t *el, **elem;
    resElemDef_t *res;
    subnode_t *sn;
    terminal_t *term;

    if (bandWidth2) { pp0EnumPair (tile, newerTile, edgeOrien); return; }

    /* prePass0 */
    if (nHasConduc && newtile) {
	elem = recogSurface (newerTile);
	el = elem[i = 0];
	while (el && el -> type != RESELEM) el = elem[++i];
	while (el && el -> type == RESELEM) {
	    res = &el -> s.res;
	    cx = res -> con;
	    if (!newerTile -> cons[cx]) {
		newerTile -> cons[cx] = sn = CONS (newerTile, cx);
		if (res -> val > 0) {
		    if (++cx > newerTile -> known) newerTile -> known = cx;
		    sn -> highres = 1;
		}
		else sn -> highres = 0;
	    }
	    el = elem[++i];
	}
    }

    resPresent = 0;
    if (tile -> known) ++resPresent;
    if (newerTile -> known) ++resPresent;
    if (resPresent == 2) {
	if ((cx = newerTile -> known) > tile -> known) cx = tile -> known;
	while (--cx >= 0) {
	    if ((sn = newerTile -> cons[cx]) && sn -> highres)
		 if ((sn = tile -> cons[cx]) && sn -> highres) { ++resPresent; break; }
	}
    }

    if (edgeOrien == 'v') {
	if (newLookTerm (bdr -> x1, bdr -> y1)) {
	    coor_t Y2 = bdr -> y2;
	    while ((term = findNextTerm (tile, newerTile, &i))) {
		if (resPresent && term -> y > bdr -> y1) {
		    bdr -> y2 = term -> y;
		    meshEdge (tile, newerTile, resPresent);
		    bdr -> y1 = term -> y;
		    bdr -> y2 = Y2;
		}
	    }
	}
	if (resPresent && bdr -> y2 > bdr -> y1) {
	    meshEdge (tile, newerTile, resPresent);
	}
    }
    else if (resPresent && resPresent != 2) {
	meshEdge (tile, newerTile, resPresent);
    }
}

Private void resEnumPair (tile_t *tile, tile_t *newerTile, int edgeOrien)
{
    int i, cx, fromNewer;
    nodePoint_t *pointStart, *newerPointStart, *point;
    resElemDef_t *res;
    subnode_t **cons, *sn;
    terminal_t *term;
    elemDef_t *el, **elem, **elem2, *newsub_elem = NULL;
    subcontRef_t *tsc = tile -> subcont;
    subcontRef_t *nsc = newerTile -> subcont;

    both_same_highres = 0;

    if (nHasConduc) {
	if (newtile) {
	    res = NULL;

	    elem = recogSurface (newerTile);

	    ASSERT (edgeOrien == 'v');

	    for (i = 0; (el = elem[i]); i++) {
		switch (el -> type) {
		    case RESELEM:
			res = &el -> s.res;
			cx = res -> con;
			++el -> el_recog_cnt;
			if (newerTile -> cons[cx]) {
			    if (++condWarnCnt <= 10) {
				say ("warning: two conductors for '%s' at position %s",
				    masktable[res -> mask].name, strCoorBrackets (newerTile -> xl, newerTile -> bl));
			    }
			    break;
			}
			newerTile -> cons[cx] = sn = CONS (newerTile, cx);
			sn -> cond = res;
			sn -> highres = 0;
			break;

		    case VBJTELEM:
			SET_cVBJT (newerTile);
			break;

		    case TORELEM:
			++el -> el_recog_cnt;

			if (newerTile -> tor) {
			    say ("warning: two different transistors '%s' and '%s'",
				 newerTile -> tor -> type -> name, el -> name);
			    say ("\tat position %s (transistor '%s' skipped).",
				 strCoorBrackets (newerTile -> xl, newerTile -> bl), el -> name);
			    break;
			}

			if (tile -> tor && tile -> tor -> type == el && bdr_length) {
			    subtorCopy (tile, newerTile);
			} else if (newerTile -> stb -> tor && newerTile -> stb -> tor -> type == el) {
			    subtorCopy (newerTile -> stb, newerTile);
			} else {
			    subtorNew (newerTile, el);
			}

			if (substrRes && el -> s.tor.bCon <= -2) {
			    cx = -1;
			    if (el -> s.tor.bCon == -2) goto do_it;
			    goto do_it2;
			}
			break;

		    case CONTELEM:
			if (substrRes && el -> s.cont.con2 <= -2) {
			    cx = el -> s.cont.con1;
			    if (el -> s.cont.con2 == -2) goto do_it;
			    goto do_it2;
			}
			break;

		    case SUBCONTELEM:
			if (substrRes) {
			    cx = el -> s.subc.ccnr;
			    ++el -> el_recog_cnt;
do_it:
			    if (newsub_elem) {
				twoSubContacts (newsub_elem, el, newerTile -> xl, newerTile -> bl);
				break;
			    }
do_it2:
			    newsub_elem = el;
			    if (!nsc) newerTile -> subcont = nsc = subContNew (newerTile);
			    else if (nsc -> causing_con != cx) {
				say ("more than one causing conductor at position %s\n",
				    strCoorBrackets (bdr -> x1, bdr -> y1));
			    }
			    nsc -> causing_con = cx;
			}
		}
	    }

	    if (res) { /* conductor found */
		tileConCnt++;
		connectPoints (tile, newerTile, 0, newerTile -> xl, newerTile -> bl);
	    }

	    if (nsc && optSubRes && (cx = nsc -> causing_con) >= 0) {
		if ((sn = newerTile -> cons[cx])) {
		    if (sn -> highres & 1) nsc -> distributed = sep_on_res[cx];
		}
	    }
	}
	else if (tile -> tlPoints && newerTile -> tlPoints && bdr_length) {
	    joinPoints (tile, newerTile, edgeOrien);

	    if (tile -> tor && newerTile -> tor && tile -> tor -> type == newerTile -> tor -> type) {
		subtorJoin (tile, newerTile);
	    }
	}
    }

    if ((tsc || nsc) && bdr_length) {
	if (tsc && nsc) {
	    /* For optSimpleSubRes we always join adjacent substrate terminals
	       since makedela can not (yet) handle these. */
	    if (optSimpleSubRes || (!tsc -> distributed && !nsc -> distributed
		    && (mergeNeighborSubContacts || tsc -> causing_con == nsc -> causing_con))) {
		subContJoin (tile, newerTile);
	    }
	}
    }

#define TOR_AND_DSCON(t1,t2) t1->tor && (sn = t2->cons[t1->tor->type->s.tor.dsCon]) && (sn->highres & 1)

    /* Place points for the cell terminals and assign the terminal
     * names to the subnodes of those points.
     */
    if (edgeOrien == 'v') {
	coor_t y = bdr -> y1;

	if (bdr_length) {
	    cx = both_same_highres ? 2 : (HasHighRes(tile) || HasHighRes(newerTile));
	    if (cx == 2) { SET_conLR1 (tile); SET_conLR2 (newerTile);
		if (conTB (tile)) SET_conLR5 (newerTile);
		if (TOR_AND_DSCON (tile, newerTile)) sn -> highres |= 0x20; /* LR2 */
		if (TOR_AND_DSCON (newerTile, tile)) sn -> highres |= 0x10; /* LR1 */
	    }
	    else if (cx) {
		if (TOR_AND_DSCON (tile, newerTile)) { SET_conLR2 (newerTile); sn -> highres |= 0x20; }
		if (TOR_AND_DSCON (newerTile, tile)) { SET_conLR1 (tile);      sn -> highres |= 0x10; }
	    }
	}
	else cx = 0;
	pointStart = tile -> rbPoints;
	newerPointStart = newerTile -> tlPoints;

	if (newLookTerm (bdr -> x1, y))
	while ((term = findNextTerm (tile, newerTile, &fromNewer))) {
	    if (term -> y > y) { y = term -> y;
		if (cx) placePoints (tile, newerTile, edgeOrien, term -> x, y);
	    }
	    point = fromNewer ? newerTile -> tlPoints : tile -> rbPoints;
	    point -> fixed = 1;
	    nameAdd (point -> cons[term -> conductor], term);
	}
	if (cx && y < bdr -> y2) /* Place final points. */
	    placePoints (tile, newerTile, edgeOrien, bdr -> x2, bdr -> y2);
	else if (!bdr_length) return;
    }
    else { /* 'h' */
	cx = both_same_highres ? 2 : (HasHighRes(tile) || HasHighRes(newerTile));
	if (cx == 2) { SET_conTB1 (tile); SET_conTB2 (newerTile);
	    if (TOR_AND_DSCON (tile, newerTile)) sn -> highres |= 0x80; /* TB2 */
	    if (TOR_AND_DSCON (newerTile, tile)) sn -> highres |= 0x40; /* TB1 */
	}
	else if (cx) {
	    if (TOR_AND_DSCON (tile, newerTile)) { SET_conTB2 (newerTile); sn -> highres |= 0x80; }
	    if (TOR_AND_DSCON (newerTile, tile)) { SET_conTB1 (tile);      sn -> highres |= 0x40; }
	}
	pointStart = tile -> tlPoints;
	newerPointStart = newerTile -> rbPoints;
	/* Place final points. */
	if (cx) placePoints (tile, newerTile, edgeOrien, bdr -> x2, bdr -> y2);
    }

    /* Update transistor edges. */
    if (newerTile -> tor) {
	cons = pointStart ? pointStart -> cons : NULL;
	updateTorEdge (newerTile, tile, cons, pointStart, edgeOrien, 0);
    }
    if (tile -> tor) {
	cons = newerPointStart ? newerPointStart -> cons : NULL;
	updateTorEdge (tile, newerTile, cons, newerPointStart, edgeOrien, 1);
    }

    /* Update edge capacitances. */
    elem = elem2 = NULL;
    if (extrEdgeCaps) {
	if (pointStart) {
	    elem = recogEdge (newerTile, tile, NULL);
	    i = updateResEdgeCap (elem, newerTile, tile, newerPointStart, pointStart, edgeOrien == 'v');
	    if (i >= 0 && edgeOrien == 'h') { /* optLatCap */
		lastY = bdr -> y2;
		if (bdr -> y2 > bdr -> y1) {
		    updateLateralCap (tile, newerTile, elem, 'v', i);
		}
	    }
	}
	if (newerPointStart) {
	    elem2 = recogEdge (tile, newerTile, otherElemSpace);
	    i = updateResEdgeCap (elem2, tile, newerTile, pointStart, newerPointStart, edgeOrien != 'v');
	    if (i >= 0) { /* optLatCap */
		/* lateral coup. cap. are only searched backwards */
		/* WARNING: elem2 must be in otherElemSpace, because
		   function updateLatEdgeCap does recogEdge */
		updateLateralCap (newerTile, tile, elem2, edgeOrien, i);
		if (edgeOrien == 'h' && bdr -> y2 < bdr -> y1) {
		    updateLateralCap (newerTile, tile, elem2, 'v', i);
		}
	    }
	}
    }

    if ((hasBipoElem || hasEdgeConnect) && pointStart && newerPointStart) {
	/* First enumerate boundary from new to old */
	if (!elem) elem = recogEdge (newerTile, tile, NULL);
	resBipoPair (newerTile, tile, elem, edgeOrien, newerPointStart, pointStart);
	if (!elem2) elem2 = recogEdge (tile, newerTile, NULL);
	resBipoPair (tile, newerTile, elem2, edgeOrien, pointStart, newerPointStart);
    }
}

Private void setBoundary (tile_t *tile, tile_t *newerTile, int edgeOrien)
{
    if (edgeOrien == 'v') {
	bdr -> x1 = bdr -> x2 = tile -> xr;
	bdr -> y2 = Min (tile -> tr, newerTile -> tl);
	bdr -> y1 = Max (tile -> br, newerTile -> bl);
	bdr_length = (double)bdr -> y2 - bdr -> y1;
	if (bdr_length < 0) {
	    if (tHasConduc || nHasConduc) fprintf (stderr, "-- ver.edge: incorrect boundary length (dy = %g)\n", bdr_length);
	    bdr_length = 0;
	}
#ifdef PR_EDGE_INFO
	if (bdr -> y1 == -INF)
	    fprintf (stderr, "(v) x1=%4d y1=-INF y2=%4d\n", bdr -> x1, bdr -> y2);
	else if (bdr -> y2 == INF)
	    fprintf (stderr, "(v) x1=%4d y1=%4d y2=+INF\n", bdr -> x1, bdr -> y1);
	else
	    fprintf (stderr, "(v) x1=%4d y1=%4d y2=%4d\n", bdr -> x1, bdr -> y1, bdr -> y2);
#endif
    }
    else {
	if (tile -> xr < newerTile -> xr) {
	    bdr -> x2 = tile -> xr;
	    bdr -> y2 = tile -> tr;
	} else {
	    bdr -> x2 = newerTile -> xr;
	    bdr -> y2 = newerTile -> br;
	}

	if (tile -> xl > newerTile -> xl) {
	    bdr -> x1 = tile -> xl;
	    bdr -> y1 = tile -> tl;
	} else {
	    bdr -> x1 = newerTile -> xl;
	    bdr -> y1 = newerTile -> bl;
	}

	bdr_length = (double)bdr -> x2 - bdr -> x1;
	if (bdr_length <= 0) {
	    if (tHasConduc || nHasConduc) fprintf (stderr, "-- hor.edge: incorrect boundary length (dx = %g)\n", bdr_length);
	    bdr_length = 0;
	}
	else if (bdr -> y1 != bdr -> y2) {
	    double dy = (double)bdr -> y2 - bdr -> y1;
	    bdr_length = sqrt (bdr_length * bdr_length + dy * dy);
	}
#ifdef PR_EDGE_INFO
	if (bdr -> x1 == -INF)
	     fprintf (stderr, "(h) x1=-INF");
	else fprintf (stderr, "(h) x1=%4d", bdr -> x1);
	if (bdr -> y1 == -INF)
	    fprintf (stderr, " x2=%4d y1=-INF\n", bdr -> x2);
	else if (bdr -> y1 == INF)
	    fprintf (stderr, " x2=%4d y1=+INF\n", bdr -> x2);
	else
	    fprintf (stderr, " x2=%4d y1=%4d\n", bdr -> x2, bdr -> y1);
#endif
    }

    bdr_length *= meters;
}

Private terminal_t * findNextTerm (tile_t *tileA, tile_t *tileB, int *fromB)
{
    terminal_t *term;
    int cx;

    while ((term = lookTerm ())) {
	ASSERT (term -> x == bdr -> x1 && term -> y <= bdr -> y2);
	cx = term -> conductor;
	if (tileA -> cons[cx]) { useLastLookTerm (); *fromB = 0; return (term); }
	if (tileB -> cons[cx]) { useLastLookTerm (); *fromB = 1; return (term); }
	if (term -> y < bdr -> y2) { useLastLookTerm (); missingTermCon (term); }
    }
    return (NULL);
}

Private void
placePoints (tile_t *tile, tile_t *newerTile, int edgeOrien, coor_t x, coor_t y)
{
    nodePoint_t *point, *newPoint;
    subnode_t **cons, *psn, *sn;
    int i, newer;

    if (tile -> tlPoints) newer = 0;
    else {
	tile = newerTile; newer = 1;
    }

    if (HasHighRes (tile)) {
	cons = tile -> cons;
	newPoint = createNodePoint (x, y);

	if ((!newer && edgeOrien == 'v') || (newer && edgeOrien == 'h')) {
	    newPoint -> next = point = tile -> rbPoints; tile -> rbPoints = newPoint;
	} else {
	    newPoint -> next = point = tile -> tlPoints; tile -> tlPoints = newPoint;
	}

	for (i = 0; i < nrOfCondStd; i++) {
	    if (cons[i]) {
		newPoint -> cons[i] = sn = CONS (newPoint, i);
		sn -> cond = cons[i] -> cond;
		psn = point -> cons[i];

		if (!(cons[i] -> highres & 1)) {
		    subnodeCopy (psn, sn);
		    continue;
		}
		subnodeNew2 (sn, Grp (psn -> node));
		if (psn -> pn) polnodeCopy (psn, sn);
		sn -> node -> mask = conductorMask[i];
		sn -> node -> node_x = x;
		sn -> node -> node_y = y;
	    }
	    else newPoint -> cons[i] = NULL;
	}
    }

    if (!newer && newerTile -> tlPoints) {
	if (HasHighRes (newerTile))
	  connectPoints (tile, newerTile, edgeOrien, x, y);
	else joinPoints (tile, newerTile, edgeOrien);
    }
}

Private void
connectPoints (tile_t *tile, tile_t *otherTile, int edgeOrien, coor_t x, coor_t y)
{
    nodePoint_t *point, *otherPoint, *prevPoint;
    subnode_t *sn, *othersn, **ocons, **tcons;
    int i, both_eq_conductor;

    otherPoint = createNodePoint (x, y);

    if (!edgeOrien) {
	prevPoint = otherTile -> stb -> tlPoints;
	otherTile -> rbPoints = otherTile -> tlPoints = otherPoint;
	point = bdr_length ? tile -> rbPoints : NULL;
    }
    else if (edgeOrien == 'v') {
	prevPoint = otherTile -> tlPoints;
	otherPoint -> next = prevPoint; otherTile -> tlPoints = otherPoint;
	point = tile -> rbPoints;
    } else {
	prevPoint = otherTile -> rbPoints;
	otherPoint -> next = prevPoint; otherTile -> rbPoints = otherPoint;
	point = tile -> tlPoints;
    }

    ocons = otherTile -> cons;
    tcons = tile -> cons;
    for (i = 0; i < nrOfCondStd; i++) {
	if (ocons[i]) {
	    otherPoint -> cons[i] = othersn = CONS (otherPoint, i);
	    othersn -> cond = ocons[i] -> cond;
	    both_eq_conductor = 0;
	    if (point && (sn = point -> cons[i])) { /* both has a conductor */
		if (othersn -> cond -> type == sn -> cond -> type) {
		    both_eq_conductor = 1;
		    subnodeCopy (sn, othersn); goto ready1;
		}
		warnSubnodeJoin (i, otherPoint -> x, otherPoint -> y, 1);
	    }
	    if (edgeOrien) { /* otherTile has prevPoint */
		sn = prevPoint -> cons[i];
		if (!(ocons[i] -> highres & 1)) {
		    subnodeCopy (sn, othersn); goto ready1;
		}
		subnodeNew2 (othersn, Grp (sn -> node));
		if (sn -> pn) polnodeCopy (sn, othersn);
		goto ready0;
	    }
	    if (prevPoint && (sn = prevPoint -> cons[i])) {
		if (othersn -> cond -> type == sn -> cond -> type) {
		    subnodeCopy (sn, othersn); goto ready1;
		}
		warnSubnodeJoin (i, otherPoint -> x, otherPoint -> y, 1);
	    }
	    subnodeNew (othersn);
	    if (has_polnode[i]) polnodeAdd (othersn, i, othersn -> cond -> type);
ready0:
	    othersn -> node -> mask = conductorMask[i];
	    othersn -> node -> node_x = otherPoint -> x;
	    othersn -> node -> node_y = otherPoint -> y;
ready1:
	    if (!edgeOrien) { /* otherTile is newtile */
		if (ocons[i] -> cond -> val > 0) { /* high res */
		    ocons[i] -> highres = 1;
		    SET_KNOW1(otherTile);
		}
	    }
	    if (both_eq_conductor) {
		if (edgeOrien != 'h') { /* v */
		    tcons[i] -> highres |= 0x10; /* LR1 */
		    ocons[i] -> highres |= 0x20; /* LR2 */
		} else {
		    tcons[i] -> highres |= 0x40; /* TB1 */
		    ocons[i] -> highres |= 0x80; /* TB2 */
		}
		if (ocons[i] -> highres & 1) { otherPoint -> fixed = 1;
		     if (tcons[i] -> highres & 1) point -> fixed = both_same_highres = 1; }
		else if (tcons[i] -> highres & 1) point -> fixed = 1;
	    }
	}
	else {
	    otherPoint -> cons[i] = NULL;
	}
    }
}

Private void joinPoints (tile_t *tile, tile_t *oTile, int edgeOrien)
{
    nodePoint_t *point, *oPoint;
    subnode_t *sn, *osn;

    if (edgeOrien == 'v') {
	point = tile -> rbPoints; oPoint = oTile -> tlPoints;
    } else {
	point = tile -> tlPoints; oPoint = oTile -> rbPoints;
    }

    for (joiningCon = 0; joiningCon < nrOfCondStd; joiningCon++) {
	if ((sn = point -> cons[joiningCon]) && (osn = oPoint -> cons[joiningCon])) {
	    if (osn -> cond -> type == sn -> cond -> type) {
		subnodeJoin (sn, osn);
		sn  = tile  -> cons[joiningCon];
		osn = oTile -> cons[joiningCon];
		if (edgeOrien == 'v') {
		    sn  -> highres |= 0x10; /* LR1 */
		    osn -> highres |= 0x20; /* LR2 */
		} else {
		    sn  -> highres |= 0x40; /* TB1 */
		    osn -> highres |= 0x80; /* TB2 */
		}
		if (osn -> highres & 1) { oPoint -> fixed = 1;
		     if (sn -> highres & 1) point -> fixed = both_same_highres = 1; }
		else if (sn -> highres & 1) point -> fixed = 1;
	    }
	    else warnSubnodeJoin (joiningCon, oPoint -> x, oPoint -> y, 1);
	}
    }
}

Private void
dsNodeJoin (tile_t *adjTile, nodePoint_t *adjPointStart, int edgeOrien, int adjIsNewer, int cx)
{
    nodePoint_t *point;

    /* join nodes along drain/source */

    if ((edgeOrien == 'v' && !adjIsNewer) || (edgeOrien == 'h' && adjIsNewer))
	point = adjTile -> rbPoints;
    else
	point = adjTile -> tlPoints;

    joiningCon = cx;

    while (point != adjPointStart) {
	subnodeJoin (point -> next -> cons[cx], point -> cons[cx]);
	point -> fixed = 1;
	point = point -> next;
    }
    point -> fixed = 1;
}

Private void
updateTorEdge (tile_t *tile, tile_t *adjTile, subnode_t **adjCons,
	nodePoint_t *adjPointStart, int edgeOrien, int adjIsNewer)
{
    int cx, cy;
    subnode_t *sn;

    if (!adjTile -> tor || adjTile -> tor -> type != tile -> tor -> type) {

	tile -> tor -> totPerimeter += bdr_length;

	cx = tile -> tor -> type -> s.tor.dsCon;
	cy = tile -> tor -> type -> s.tor.sCon;

	if (adjCons && (adjCons[cx] || adjCons[cy])) {

	    tile -> tor -> dsPerimeter += bdr_length;

	    if ((sn = adjCons[cx])) {
		torBoundary (tile -> tor, bdr -> x1, bdr -> y1, bdr -> x2, bdr -> y2, sn, cy != cx ? 'd' : 'x');
		if (optRes) dsNodeJoin (adjTile, adjPointStart, edgeOrien, adjIsNewer, cx);
	    }
	    if (cy != cx && (sn = adjCons[cy])) {
		torBoundary (tile -> tor, bdr -> x1, bdr -> y1, bdr -> x2, bdr -> y2, sn, 's');
		if (optRes) dsNodeJoin (adjTile, adjPointStart, edgeOrien, adjIsNewer, cy);
	    }
	}
    }
}

Private int
updateEdgeCap (elemDef_t **elem, tile_t *tile, tile_t *adjTile)
{
    elemDef_t *jun_el = NULL;
    capElemDef_t *cap;
    subnode_t *subnA, *subnB;
    double val;
    int i, cx;
    int j, h = 0;

 /* ASSERT (extrPass && !optRes && !substrRes); */

    for (i = 0; elem[i]; i++) {
	if (elem[i] -> type == EDGECAPELEM) {
	    ++elem[i] -> el_recog_cnt;

	    cap = &elem[i] -> s.cap;

	    subnA = adjTile -> cons[cap -> pCon];
	    if (!subnA) missingCon (cap -> pMask, cap -> pOccurrence, /* die */
				tile, adjTile, tNull, elem[i], bdr -> x1, bdr -> y1);
	    if (cap -> dsCap) {
		for (j = 0; j < h; ++j) if (helpArray[j] == cap -> pCon) break;
		if (j < h) continue; /* skip, double dscap for pCon found */
		helpArray[h++] = cap -> pCon;
		ASSERT (subnA -> pn);
		subnA -> pn -> ds_peri += bdr_length;
		continue;
	    }

	    if (cap -> pKeep) KEEPNODE (subnA -> node);

	    if (capPolarityTab[cap -> sortNr] != 'x') {
		if (jun_el) checkDoubleJuncCaps (jun_el, elem + i, adjTile -> xl, adjTile -> bl);
		jun_el = elem[i];
	    }

	    if ((cx = cap -> nCon) < 0) subnB = cx < -1 ? subnSUB : subnGND;
	    else {
		subnB = (cap -> nOccurrence == EDGE)? adjTile -> cons[cx] : tile -> cons[cx];
		if (!subnB) missingCon (cap -> nMask, cap -> nOccurrence, /* die */
				tile, adjTile, tNull, elem[i], bdr -> x1, bdr -> y1);
		if (cap -> nKeep) KEEPNODE (subnB -> node);
	    }

	    val = cap -> val * bdr_length;

	    capAdd (subnA, subnB, val, cap -> sortNr);
	}
	else if (elem[i] -> type == LATCAPELEM) return i;
    }

    return -1;
}

Private int
updateResEdgeCap (elemDef_t **elem, tile_t *tile, tile_t *adjTile,
	nodePoint_t *pointStart, nodePoint_t *adjPointStart, int use_ttl)
{
    capElemDef_t *ced;
    nodePoint_t *p1, *q1, *p2, *q2, *pf;
    subnode_t *subnA1, *subnB1, *subnA2, *subnB2;
    double cap;
    int i, cx, cy, typeA, typeB;
    int j, h = 0;

 /* ASSERT (extrPass && optRes); */

    for (i = 0; elem[i]; i++) {
	if (elem[i] -> type == EDGECAPELEM) {
	    ++elem[i] -> el_recog_cnt;

	    ced = &elem[i] -> s.cap;

	    /* subdivide capacitance over the two edgepoints */

            /* see also commentSUBAREA is enumtile.c */

	    cx = ced -> pCon;
	    cy = ced -> nCon;

	    ASSERT (ced -> pOccurrence == EDGE); /* see gettech.cc */
	    p1 = adjPointStart;
	    q1 = use_ttl ? adjTile -> rbPoints : adjTile -> tlPoints;

	    if (ced -> nOccurrence == EDGE) { p2 = p1; q2 = q1; }
	    else { /* SURFACE */
		p2 = pointStart;
		q2 = use_ttl ? tile -> tlPoints : tile -> rbPoints;
	    }

	    subnA1 = p1 -> cons[cx];
	    if (!subnA1) missingCon (ced -> pMask, ced -> pOccurrence, /* die */
		    tile, adjTile, tNull, elem[i], bdr -> x1, bdr -> y1);
	    if (ced -> dsCap) {
		for (j = 0; j < h; ++j) if (helpArray[j] == cx) break;
		if (j < h) continue; /* skip, double dscap for cx found */
		helpArray[h++] = cx;
		ASSERT (subnA1 -> pn);
		subnA1 -> pn -> ds_peri += bdr_length;
		continue;
	    }

snA:
	    subnA2 = q1 -> cons[cx];
	    if (ced -> pKeep) {
		KEEPNODE (subnA1 -> node);
		KEEPNODE (subnA2 -> node);
	    }

            /* see also commentSUBAREA is enumtile.c */

	    if (cy <= -2) {
		subnB1 = subnSUB;
		if (substrRes) {
		    /* For edge capacitances we first try a substrate terminal
			directly below the edge mask in adjTile. */
		    if (adjTile -> subcont)
			subnB1 = adjTile -> subcont -> subn;
		    else if (tile -> subcont)
			subnB1 = tile -> subcont -> subn;
		    else {
			missingSubarea (ced -> nMask, tile, adjTile, elem[i], bdr -> x1, bdr -> y1);
			continue;
		    }
		    if (ced -> nKeep) KEEPNODE (subnB1 -> node);
		}
		subnB2 = subnB1;
	    }
	    else if (cy < 0) {
		subnB2 = subnB1 = subnGND;
	    }
	    else {
		subnB1 = p2 ? p2 -> cons[cy] : 0;
		if (!subnB1) missingCon (ced -> nMask, ced -> nOccurrence, /* die */
			tile, adjTile, tNull, elem[i], bdr -> x1, bdr -> y1);
		subnB2 = q2 -> cons[cy];
		if (ced -> nKeep) {
		    KEEPNODE (subnB1 -> node);
		    KEEPNODE (subnB2 -> node);
		}
	    }

	    cap = ced -> val * bdr_length / 2;
	    capAdd (subnA1, subnB1, cap, ced -> sortNr);
	    capAdd (subnA2, subnB2, cap, ced -> sortNr);
	}
	else if (elem[i] -> type == LATCAPELEM) return i;
    }

    return -1;
}

Private void missingConPresent (tile_t *tile, char *type)
{
    int i;
    elemDef_t ** elem = recogSurface (tile);

    fprintf (stderr, "  %s masks present:", type);
    for (i = 0; i < nrOfMasks; ++i) {
	if (masktable[i].gln && !COLOR_ABSENT (&tile -> color, &masktable[i].color))
	    fprintf (stderr, " %s", masktable[i].name);
    }
    fprintf (stderr, "\n");

    fprintf (stderr, "     conductors present:");
    for (i = 0; elem[i]; ++i) if (elem[i] -> type == RESELEM) {
	fprintf (stderr, " %s", elem[i] -> name);
    }
    fprintf (stderr, "\n");
}

void missingCon (int mask, int occurrence, tile_t *tile, tile_t *eTile, tile_t *oeTile,
	elemDef_t *el, coor_t x, coor_t y)
{
    char buf[256];

    sprintf (buf, "missing conductor for pin ");

    if (occurrence == SURFACE)
	sprintf (buf + strlen (buf), "'%s'", masktable[mask].name);
    else if (occurrence == EDGE)
	sprintf (buf + strlen (buf), "'-%s'", masktable[mask].name);
    else /* OTHEREDGE */
	sprintf (buf + strlen (buf), "'=%s'", masktable[mask].name);

    if (el -> name[0] == DSCAP_PREFIX && strstr (el -> name, DSCAP_SUFFIX))
	sprintf (buf + strlen (buf), " of fet drain/source region");
    else
	sprintf (buf + strlen (buf), " of element '%s'", el -> name);

    say ("%s at position %s\n", buf, strCoorBrackets (x, y));

    if (tile) missingConPresent (tile, "surface");
    if (eTile) missingConPresent (eTile, "edge");
    if (oeTile) missingConPresent (oeTile, "other edge");

    die ();
}

void missingTermCon (terminal_t *term)
{
    char *s;

    /* If it is a terminal of an instance, just return.
     * Otherwise, give a warning.
     */
    if (!lastPass || term -> instName) return;

    s = (term -> type == tTerminal)? "terminal" : "label";

    if (term -> conductor < 0)
	say ("warning: no conductor defined for %s '%s'", s, term -> termName);
    else
	say ("warning: conductor '%s' missing for %s '%s'\n   at position %s",
	    masktable[conductorMask[term -> conductor]].name, s, term -> termName,
	    strCoorBrackets (term -> x, term -> y));
}

Private void twoSubContacts (elemDef_t *el_old, elemDef_t *el_new, coor_t x, coor_t y)
{
  if (!mergeNeighborSubContacts)
    say ("warning: %s, '%s' and '%s',\n   present at position %s",
	"two substrate terminal elements", el_old -> name, el_new -> name, strCoorBrackets (x, y));
}

void missingSubarea (int mask, tile_t *tile, tile_t *eTile, elemDef_t *el, coor_t x, coor_t y)
{
    char buf[512], *msk;
    int i;

    msk = mask < 0? (mask < -1? "@sub" : "@gnd") : masktable[mask].name;

    sprintf (buf, "warning: value of element '%s' ignored because substrate area\n", el -> name);
    sprintf (buf + strlen (buf), "  for pin '%s' is missing at position %s\n", msk, strCoorBrackets (x, y));
    sprintf (buf + strlen (buf), "  masks present :");
    if (tile) {
	for (i = 0; i < nrOfMasks; i++) {
	    if (masktable[i].gln && !COLOR_ABSENT (&tile -> color, &masktable[i].color))
		sprintf (buf + strlen (buf), " %s", masktable[i].name);
	}
    }
    if (eTile) {
	for (i = 0; i < nrOfMasks; i++) {
	    if (masktable[i].gln && !COLOR_ABSENT (&eTile -> color, &masktable[i].color))
		sprintf (buf + strlen (buf), " -%s", masktable[i].name);
	}
    }
    say (buf);
}
