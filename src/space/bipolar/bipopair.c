/*
 * ISC License
 *
 * Copyright (C) 1994-2018 by
 *	Frederik Beeftink
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
#include "src/space/include/type.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"

#include "src/space/bipolar/define.h"
#include "src/space/bipolar/extern.h"

extern coor_t baseWindow;
extern double bdr_length;
extern tileBoundary_t *bdr;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private BJT_t *torPresent (char *name, subnode_t *snB, subnode_t *snE, coor_t *w_return);
Private polnode_t *pnFind (int con, polnode_t *pnB, polnode_t *pnE, int orientation, coor_t *w_return);
#ifdef __cplusplus
  }
#endif

/* Will update the network description for the
 * influence of the boundary between tile_r and
 * tile_n. Actions performed :
 *
 * - If tile_n is passed to bipoPair for the
 *   first time junction and bipolar transistor
 *   elements are investigated to allocate
 *   appropriate subnodes for tile_n.
 *
 * - Joins are made between junctions of the
 *   same type that are present in both tile_r
 *   and tile_n.
 *
 * - When a junction or bipolar transistor is
 *   present in one tile and a junction or
 *   bipolar transistor of the same type is
 *   not present in the other tile, the edges
 *   are updated.
 */
void bipoPair (tile_t *tile_v, tile_t *tile_l, elemDef_t **elem, int orientation)
{
    BJT_t *lT;
    coor_t w;
    int i, cx, cr, sCon;
    tile_t *tl, *tv;
    elemDef_t *el;
    pnconElemDef_t *pnc;
    bjtorElemDef_t *ted;
    subnode_t *snSub, *snB, *snC, *snE;
    polnode_t *pnC;

    for (i = 0; (el = elem[i]); i++) {
	switch (el -> type) {
	case PNCONELEM:
	    pnc = &el -> s.pnc;
	    if ((joiningCon = pnc -> con1) >= nrOfCondStd) break;
	    if (extrPass) ++el -> el_recog_cnt;

	    ASSERT (pnc -> occurrence1 == EDGE);
	    mkConnect (tile_l -> cons[joiningCon], tile_v -> cons[pnc -> con2], el);
	    break;

	case LBJTELEM:
	    if (!extrPass) break;
	    ++el -> el_recog_cnt;

	    ted = &el -> s.bjt;

	    if ((orientation == 'v' && tile_l -> xl < tile_v -> xl)
	     || (orientation == 'h' && tile_l -> bl < tile_v -> bl)) {
		tv = tile_v;
		tl = tile_l;
	    }
	    else {
		tv = tile_l;
		tl = tile_v;
	    }
            if (tv -> cons[ted -> eCon] || tl -> cons[ted -> eCon]) {
                cx = ted -> eCon;
                cr = ted -> cCon;
            }
            else {
                cx = ted -> cCon;
                cr = ted -> eCon;
            }

	    snSub = NULL;
	    if ((sCon = ted -> sCon) >= 0) {
		if (tv -> cons[sCon]) {
		    snSub = tv -> cons[sCon];
		}
		else if (tl -> cons[sCon]) {
		    snSub = tl -> cons[sCon];
		}
	    }
	    else if (sCon <= -2) {
		if (substrRes) {
		    if (tv -> subcont) {
			snSub = tv -> subcont -> subn;
		    }
		    else if (tl -> subcont) {
			snSub = tl -> subcont -> subn;
		    }
		}
		else
		    snSub = subnSUB;
	    }

	    snB = tl -> cons[ted -> bCon];
	    snE = tv -> cons[cx];

	    if (snB && snE) {
		pnC = pnFind (cr, snB -> pn, snE -> pn, orientation, &w);
	    }
	    else {
		snB = tv -> cons[ted -> bCon];
		snE = tl -> cons[cx];
		if (!snB) missingCon (ted -> bMask, SURFACE, tv, tl, (tile_t *) NULL, el, bdr -> x1, bdr -> y1);
		if (!snE) missingCon (cx == ted -> eCon ? ted -> eMask : ted -> cMask, EDGE, tv, tl, (tile_t *) NULL, el, bdr -> x1, bdr -> y1);
		junAdd (snE -> pn, snB -> pn);
		pnEdgeAdd (snE -> pn, bdr, orientation);
		pnC = NULL;
	    }
	    snE -> pn -> length += bdr_length;

	    if (!pnC && snSub) {
		lT = torPresent (el -> name, snB, snE, &w);
		if (lT && !lT -> n[SU]) pnC = lT -> pn[CO];
	    }
	    if (pnC) newLBJT (pnC, snB, snE, snSub, el, ted -> eCon, w);
	}
    }

    if (!HasVBJT (tile_v)) return;

    elem = recogSurface (tile_v);

    for (i = 0; (el = elem[i]); i++) {
	if (el -> type != VBJTELEM) continue;

	ted = &el -> s.bjt;

	/* Only if it is an emitter sidewall
	 * add extra area or new BJT.
	 */
	if (!tile_l -> cons[ted -> eCon]) {

	    if (!(snC = tile_l -> cons[ted -> cCon]))
		missingCon (ted -> cMask, EDGE, tile_v, tile_l, (tile_t *) NULL, el, bdr -> x1, bdr -> y1);
	    if (!(snB = tile_l -> cons[ted -> bCon]))
		missingCon (ted -> bMask, EDGE, tile_v, tile_l, (tile_t *) NULL, el, bdr -> x1, bdr -> y1);
	    if (!(snE = tile_v -> cons[ted -> eCon]))
		missingCon (ted -> eMask, SURFACE, tile_v, tile_l, (tile_t *) NULL, el, bdr -> x1, bdr -> y1);

	    snSub = NULL;
	    if ((sCon = ted -> sCon) >= 0) {
		if (tile_v -> cons[sCon]) {
		    snSub = tile_v -> cons[sCon];
		}
		else if (tile_l -> cons[sCon]) {
		    snSub = tile_l -> cons[sCon];
		}
	    }
	    else if (sCon <= -2) {
		if (substrRes) {
		    if (tile_v -> subcont) {
			snSub = tile_v -> subcont -> subn;
		    }
		    else if (tile_l -> subcont) {
			snSub = tile_l -> subcont -> subn;
		    }
		}
		else
		    snSub = subnSUB;
	    }

	    newVBJT (snC, snB, snE, snSub, el, bdr_length, 1);
	}
    }
}

/*
 * Performs the same actions as 'bipoPair' only with
 * resistance-extraction active.
 */
void resBipoPair (tile_t *tile_v, tile_t *tile_l, elemDef_t **elem, int orientation, nodePoint_t *np_v, nodePoint_t *np_l)
{
    BJT_t *lT;
    coor_t w;
    int i, cx, cr, sCon;
    elemDef_t *el;
    pnconElemDef_t *pnc;
    bjtorElemDef_t *ted;
    subnode_t *snSub, *snB, *snC, *snE;
    polnode_t *pnC;
    nodePoint_t *pA1, *pA2, *pB1, *pB2, *nA1, *nB1;

    if ((orientation == 'v' && tile_v -> xl > tile_l -> xl)
     || (orientation == 'h' && tile_v -> br < tile_l -> br)) {
	pA1 = np_v; pA2 = tile_v -> tlPoints;
	pB1 = np_l; pB2 = tile_l -> rbPoints;
    }
    else {
	pA1 = np_v; pA2 = tile_v -> rbPoints;
	pB1 = np_l; pB2 = tile_l -> tlPoints;
    }

    for (i = 0; (el = elem[i]); i++) {
	switch (el -> type) {
	case PNCONELEM:
	    pnc = &el -> s.pnc;
	    if ((joiningCon = pnc -> con1) >= nrOfCondStd) break;
	    ++el -> el_recog_cnt;

	    ASSERT (pnc -> occurrence1 == EDGE);
	    nA1 = pA2; nB1 = pB2;
	    for (;;) {
		mkConnect (nB1 -> cons[joiningCon], nA1 -> cons[pnc -> con2], el);
		nA1 -> fixed = 1; nB1 -> fixed = 1;
		if (nA1 != pA1) nA1 = nA1 -> next;
		else if (nB1 == pB1) break;
		if (nB1 != pB1) nB1 = nB1 -> next;
	    }
	    break;

	case LBJTELEM:
	    ++el -> el_recog_cnt;

	    ted = &el -> s.bjt;

	    if ((orientation == 'v' && tile_l -> xl < tile_v -> xl)
	    ||  (orientation == 'h' && tile_l -> bl < tile_v -> bl)) {
		nA1 = pA1; nB1 = pB1;
	    }
	    else {
		nA1 = pB1; nB1 = pA1;
	    }

	    if (nA1 -> cons[ted -> eCon] || nB1 -> cons[ted -> eCon]) {
		cx = ted -> eCon;
		cr = ted -> cCon;
	    }
	    else {
		cx = ted -> cCon;
		cr = ted -> eCon;
	    }

	    snSub = NULL;
	    if ((sCon = ted -> sCon) >= 0) {
		if (!(snSub = nA1 -> cons[sCon])) snSub = nB1 -> cons[sCon];
	    }
	    else if (sCon <= -2) {
		if (substrRes) {
		    if (tile_v -> subcont) {
			snSub = tile_v -> subcont -> subn;
		    }
		    else if (tile_l -> subcont) {
			snSub = tile_l -> subcont -> subn;
		    }
		}
		else
		    snSub = subnSUB;
	    }

	    snB = nB1 -> cons[ted -> bCon];
	    snE = nA1 -> cons[cx];

	    if (snB && snE) {
		pnC = pnFind (cr, snB -> pn, snE -> pn, orientation, &w);
	    }
	    else {
		snB = nA1 -> cons[ted -> bCon];
		snE = nB1 -> cons[cx];
		if (!snB) missingCon (ted -> bMask, SURFACE, tile_v, tile_l, (tile_t *) NULL, el, bdr -> x1, bdr -> y1);
		if (!snE) missingCon (cx == ted -> eCon ? ted -> eMask : ted -> cMask, EDGE, tile_v, tile_l, (tile_t *) NULL, el, bdr -> x1, bdr -> y1);
		junAdd (snE -> pn, snB -> pn);
		pnEdgeAdd (snE -> pn, bdr, orientation);
		pnC = NULL;
	    }
	    snE -> pn -> length += bdr_length;

	    if (!pnC && snSub) {
		lT = torPresent (el -> name, snB, snE, &w);
		if (lT && !lT -> n[SU]) pnC = lT -> pn[CO];
	    }
	    if (pnC) newLBJT (pnC, snB, snE, snSub, el, ted -> eCon, w);
	}
    }

    if (!HasVBJT (tile_v)) return;

    elem = recogSurface (tile_v);

    for (i = 0; (el = elem[i]); i++) {
	if (el -> type != VBJTELEM) continue;

	ted = &el -> s.bjt;

	if (!pB1 -> cons[ted -> eCon]) {

	    if (!(snC = pB1 -> cons[ted -> cCon]))
		missingCon (ted -> cMask, EDGE, tile_v, tile_l, (tile_t *) NULL, el, bdr -> x1, bdr -> y1);
	    if (!(snB = pB1 -> cons[ted -> bCon]))
		missingCon (ted -> bMask, EDGE, tile_v, tile_l, (tile_t *) NULL, el, bdr -> x1, bdr -> y1);
	    if (!(snE = pA1 -> cons[ted -> eCon]))
		missingCon (ted -> eMask, SURFACE, tile_v, tile_l, (tile_t *) NULL, el, bdr -> x1, bdr -> y1);

	    snSub = NULL;
	    if ((sCon = ted -> sCon) >= 0) {
		if (!(snSub = pA1 -> cons[sCon])) snSub = pB1 -> cons[sCon];
	    }
	    else if (sCon <= -2) {
		if (substrRes) {
		    if (tile_v -> subcont) {
			snSub = tile_v -> subcont -> subn;
		    }
		    else if (tile_l -> subcont) {
			snSub = tile_l -> subcont -> subn;
		    }
		}
		else
		    snSub = subnSUB;
	    }

	    newVBJT (snC, snB, snE, snSub, el, bdr_length, 1);
	}
    }
}

Private BJT_t * torPresent (char *name, subnode_t *snB, subnode_t *snE, coor_t *w_return)
{
    BJT_t *lT;
    pnTorLink_t *tl;

    for (tl = snE -> pn -> tors; tl; tl = tl -> next) {
	if (tl -> type == LBJTELEM) {
	    lT = tl -> tor;
	    if (strsame (lT -> type -> name, name)
                && lT -> pn[BA] == snB -> pn
                && lT -> pn[EM] == snE -> pn) {
		*w_return = lT -> basew;
		return (lT);
	    }
	}
    }
    return ((BJT_t *) NULL);
}

/* Compute the lateral width between <pnC> and <bdr>. <min> indicates
 * whether the width has to be minimal or just small enough.
 */
Private coor_t computeBaseW (polnode_t *pnC, int orientation, int min)
{
    register pnEdge_t *e2;
    coor_t l, w, wf = baseWindow + 1;

    for (e2 = pnC -> edges; e2; e2 = e2 -> next) {
	if (e2 -> edgeOrien == orientation) {
	    if (orientation == 'v') {
		l = (bdr -> y2 < e2 -> y2)? bdr -> y2 : e2 -> y2;
		if (bdr -> y1 > e2 -> y1) l -= bdr -> y1;
		else l -= e2 -> y1;
		if (l >= 0) {
		    if ((w = bdr -> x1 - e2 -> x1) < 0) w = -w;
		    if (w < wf) { if (min) wf = w; else return w; }
		}
	    }
	    else {
		l = (bdr -> x2 < e2 -> x2)? bdr -> x2 : e2 -> x2;
		if (bdr -> x1 > e2 -> x1) l -= bdr -> x1;
		else l -= e2 -> x1;
		if (l >= 0) {
		    if ((w = bdr -> y1 - e2 -> y1) < 0) w = -w;
		    if (w < wf) { if (min) wf = w; else return w; }
		}
	    }
	}
    }
    return wf;
}

/* Return the collector-polnode that represents
 * the mask-layer <con>, start the search from
 * <pnB> and <pnE>. (subgraph-isomorphism tests).
 */
Private polnode_t * pnFind (int con, polnode_t *pnB, polnode_t *pnE, int orientation, coor_t *w_return)
{
    polnode_t *pnC;
    junction_t *j;
    int opn, spn;
    coor_t wb = baseWindow + 1;

    ASSERT (pnB && pnE);

    opn = OPN (pnB);
    spn = SPN (pnB);
    for (j = pnB -> juncs; j; j = j -> next[spn]) {
	pnC = j -> pn[opn];
	if (pnC != pnE && pnC -> conNr == con) {
	    *w_return = computeBaseW (pnC, orientation, 0);
	    if (*w_return < wb && *w_return > 0) return (pnC);
	}
    }
    return ((polnode_t *) NULL);
}
