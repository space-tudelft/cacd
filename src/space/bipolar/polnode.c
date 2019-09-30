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
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"
#include "src/space/lump/define.h"
#include "src/space/lump/extern.h"

#include "src/space/bipolar/define.h"
#include "src/space/bipolar/extern.h"

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void tryOutputVBJT (pnTorLink_t *tp);
Private void tryOutputLBJT (pnTorLink_t *tp);
Private void termNodeClear (pnTorLink_t *tp);
Private bool_t otherDevices (node_t *n, pnTorLink_t *tp);
#ifdef __cplusplus
  }
#endif

/* Create a new polnode and link it to <sn> and back.
 * Set type of polnode to <type> and junctions to NULL.
 */
void polnodeAdd (subnode_t *sn, int con, int type)
{
    polnode_t * pn;

    if (sn -> pn) return;

    sn -> pn = pn = NEW (polnode_t, 1);
    pn -> type = type;
    pn -> conNr = con;
    pn -> subs = sn;

    pn -> xl = bigbxr;
    pn -> yb = bigbyt;
    pn -> area = 0;
    pn -> length = 0;
    pn -> ds_tcnt = 0;
    pn -> ds_wtot = 0;
    pn -> ds_area = 0;
    pn -> ds_peri = 0;

    pn -> edges = NULL;
    pn -> nodes = NULL;
#ifndef CONFIG_SPACE2
    pn -> tiles = NULL;
#endif
    pn -> juncs = NULL;
    pn -> tors = NULL;

    inPolnode++;
    currIntPolnode++;

    nodeLinkAdd (sn -> node, pn);
}

/* Link <snB> to the same polnode as <snA> is linked to. Therefore,
 * <snB> is put at the beginning of the subnode-list of the polnode.
 *
 * Note that this operation is actually a short-cut for:
 * - polnodeNew (snB);
 * - polnodeJoin (snA -> pn, snB -> pn);
 */
void polnodeCopy (subnode_t *snA, subnode_t *snB)
{
    if (snB -> pn) return;

    snB -> pn = snA -> pn;
    snB -> next_pn = snA -> pn -> subs;
    snA -> pn -> subs = snB;

    if (snA -> node != snB -> node)
	nodeLinkAdd (snB -> node, snB -> pn);
}

/* throw away <pn> and remove all pnEdges
 * and nodeLinks that are still there.
 */
void polnodeDispose (polnode_t *pn)
{
    ASSERT (pn -> subs == NULL);
    ASSERT (pn -> juncs == NULL);
    ASSERT (pn -> tors == NULL);

    while (pn -> edges) pnEdgeDel (pn, pn -> edges);
    nodeLinksDel (pn);
#ifndef CONFIG_SPACE2
    if (pn -> tiles) {
	tileRef_t *tR;
	while ((tR = pn -> tiles)) {
	    pn -> tiles = tR -> next;
	    DISPOSE (tR, sizeof(tileRef_t));
	}
    }
#endif
    currIntPolnode--;

    DISPOSE (pn, sizeof(polnode_t));
}

/* Join two polnodes, update <pnA> and delete <pnB>.
 */
void polnodeJoin (polnode_t *pnA, polnode_t *pnB)
{
    junction_t *j;
    nodeLink_t *nL;
    pnEdge_t *e;
    subnode_t *sn;
    int opn;

 /* ASSERT (pnA != pnB && pnA && pnB); */
 /* ASSERT (pnA -> type == pnB -> type); */

    pnTorReLink (pnB, pnA);

    opn = OPN (pnB);
    while ((j = pnB -> juncs)) {
	junAdd (pnA, j -> pn[opn]);
	junctionDel (j, pnB);
    }

    if (!(sn = pnA -> subs)) pnA -> subs = pnB -> subs;
    else {
	while (sn -> next_pn) sn = sn -> next_pn;
	sn -> next_pn = pnB -> subs;
    }

    for (sn = pnB -> subs; sn; sn = sn -> next_pn) sn -> pn = pnA;

    pnB -> subs = NULL;

    pnA -> area   += pnB -> area;
    pnA -> length += pnB -> length;
    pnA -> ds_tcnt += pnB -> ds_tcnt;
    pnA -> ds_wtot += pnB -> ds_wtot;
    pnA -> ds_area += pnB -> ds_area;
    pnA -> ds_peri += pnB -> ds_peri;

    while ((e = pnB -> edges)) {
	pnEdgeAdd (pnA, e, e -> edgeOrien);
	pnEdgeDel (pnB, e);
    }

    if ((nL = pnB -> nodes)) {
	nL -> pn = pnA;
	while (nL -> next_n) { nL = nL -> next_n; nL -> pn = pnA; }
	nL -> next_n = pnA -> nodes;
	pnA -> nodes = pnB -> nodes;
	pnB -> nodes = NULL;
    }

    polnodeDispose (pnB);
}

/* Check if a polnode and its junctions are ready for output.
 * Eventually, dispose the polnode.
 */
void polnodeDel (polnode_t *pn)
{
    junction_t *j, *jp;
    pnTorLink_t *tl, *tp;
    polnode_t **p;

    ASSERT (!pn -> subs);

#define Ready(pn) (!pn || !pn -> subs)

    /* Check if the junctions connected to <pn> can be outputted:
     * if the other polnode is also ready (has no subnode-references).
     */
    if ((tl = pn -> tors)) {
	while (tl) {
	    tp = tl;
	    tl = tl -> next;
	    if (tp -> type == TORELEM) {
		outputReadyTransistor (pn, tp);
	    }
	    else {
		p = tp -> tor -> pn;
		if (Ready (p[CO]) && Ready (p[EM]) && Ready (p[BA])) {
		    if (tp -> type == VBJTELEM)
			tryOutputVBJT (tp);
		    else
			tryOutputLBJT (tp);
		    deviceDel (pn, tp);
		}
	    }
	}
    }

    if (!pn -> tors && (j = pn -> juncs)) {
	int spn = SPN (pn);
	int opn = OPN (pn);
	while (j) {
	    jp = j;
	    j = j -> next[spn];
	    if (Ready (jp -> pn[opn])) {
		outJun++;
		junctionDel (jp, pn);
	    }
	}
    }

    outPolnode++;

    if (!pn -> juncs && !pn -> tors) polnodeDispose (pn);
}

#define UpdateX(pnA,pnB) if (pnA -> xl > pnB -> xl) pnA -> xl = pnB -> xl;
#define UpdateY(pnA,pnB) if (pnA -> yb > pnB -> yb) pnA -> yb = pnB -> yb;

Private void tryOutputVBJT (pnTorLink_t *tp)
{
    pnTorLink_t * th;
    BJT_t * vT = tp -> tor;
    polnode_t *pnA, *pnB;

    if ((th = parBJT (vT -> pn[CO] -> tors, tp))) {
	BJT_t * hT = th -> tor;

	ASSERT (vT != hT);

	pnA = hT -> pn[EM];
	pnB = vT -> pn[EM];

	if (pnA != pnB) {
	    ASSERT (parallelMerge);
	    hT -> scalef += vT -> scalef;
	    UpdateX (pnA, pnB);
	    UpdateY (pnA, pnB);
	}
	else {
	    hT -> scalef += vT -> scalef - 1;
	    hT -> area   += vT -> area;
	    hT -> length += vT -> length;
	}

	termNodeClear (tp);

#ifndef CONFIG_SPACE2
	if (optBackInfo > 1) {
	    tileRef_t *tR;
	    if ((tR = vT -> tiles)) {
		while (tR -> next) tR = tR -> next;
		tR -> next = hT -> tiles;
		hT -> tiles = vT -> tiles;
		vT -> tiles = NULL;
	    }
	}
#endif
    }
    else {
	outVBJT (vT);
    }
}

Private void tryOutputLBJT (pnTorLink_t *tp)
{
    pnTorLink_t *th;
    polnode_t *pn, *pnEM, *pnCO;
    BJT_t *hT, *lT = tp -> tor;

    pnEM = lT -> pn[EM];
    pnCO = lT -> pn[CO];

    /* Remove the not really transistors: when
     * emitter and collector are the same polnode.
     */
    if (pnEM == pnCO) {
	termNodeClear (tp);
	return;
    }

    /* Check also if largest polnode is assigned to
     * collector and not emitter, otherwise exchange them.
     */
    if (pnEM -> conNr == pnCO -> conNr && pnEM -> area > pnCO -> area) {
	node_t *n;
	for (th = lT -> pn[BA] -> tors; th; th = th -> next) {
	    if (th -> type == tp -> type) {
		hT = th -> tor;
		if (hT == lT || (hT -> pn[CO] == pnCO && hT -> pn[EM] == pnEM
			    && strsame (hT -> type -> name, lT -> type -> name))) {
		    n = hT -> n[EM]; hT -> n[EM] = hT -> n[CO]; hT -> n[CO] = n;
		    hT -> pn[EM] = pnCO;
		    hT -> pn[CO] = pnEM;
		}
	    }
	}
	pnEM = pnCO;
    }

    if ((th = parBJT (lT -> pn[BA] -> tors, tp))) {
	hT = th -> tor;

	ASSERT (lT != hT);

	if ((pn = hT -> pn[EM]) != pnEM) {
	    ASSERT (parallelMerge);
	    hT -> scalef += lT -> scalef;
	    UpdateX (pn, pnEM);
	    UpdateY (pn, pnEM);
#ifndef CONFIG_SPACE2
	    if (optBackInfo > 1) {
		tileRef_t *tR;
		if ((tR = pnEM -> tiles)) {
		    while (tR -> next) tR = tR -> next;
		    tR -> next = pn -> tiles;
		    pn -> tiles = pnEM -> tiles;
		    pnEM -> tiles = NULL;
		}
	    }
#endif
	}
	else {
	    hT -> scalef += lT -> scalef - 1;
	    if (lT -> basew < hT -> basew) hT -> basew = lT -> basew;
	}

	termNodeClear (tp);
    }
    else {
	outLBJT (lT);
    }
}

#define ONLY_BJT_TERM(n) (n -> term == 1+8)

Private void termNodeClear (pnTorLink_t *tp)
{
#ifdef CONFIG_SPACE2
    static int warn;
#endif
    int i;
    node_t * n;

    for (i = 0; i < 4; i++) {
	n = tp -> tor -> n[i];
        /* EM: 4th node can be NULL, substrate terminal is optional */
        if (n) {
#ifdef CONFIG_SPACE2
	    if (ONLY_BJT_TERM (n) && !otherDevices (n, tp)) {
		if (warn++ < 10) fprintf (stderr, "warning: termNodeClear of BJT node\n");
		n -> term = 0;
	    }
#else
	    if (n -> term < 2 && !n -> names && !otherDevices (n, tp)) n -> term = 0;
#endif
        }
	else ASSERT (i == 3);
    }
}

Private bool_t otherDevices (node_t *n, pnTorLink_t *tp)
{
    int i;
    node_t ** t_n;
    nodeLink_t * nL;
    pnTorLink_t * th;

    for (nL = n -> pols; nL; nL = nL -> next_pn) {
	for (th = nL -> pn -> tors; th; th = th -> next) {
	    if (th -> type != TORELEM && th -> tor != tp -> tor) {
		t_n = th -> tor -> n;
		for (i = 0; i < 4; i++) if (t_n[i] == n) return TRUE;
	    }
        }
    }
    return FALSE;
}
