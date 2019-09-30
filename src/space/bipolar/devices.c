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
#include "src/space/lump/define.h"
#include "src/space/lump/extern.h"

#include "src/space/bipolar/define.h"
#include "src/space/bipolar/extern.h"

#ifndef CONFIG_SPACE2
#define SET_BJT_TERM(node) node -> term = 1
#else
#define SET_BJT_TERM(node) node -> term |= 1+8
#endif

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private node_t *nearestNode (polnode_t *pn, node_t *n);
#ifdef __cplusplus
  }
#endif

Private BJT_t * findBJT (pnTorLink_t *tl, polnode_t *pnC,
	subnode_t *snB, subnode_t *snE, char *name, int type)
{
    BJT_t *tor;

    for (; tl; tl = tl -> next) if (tl -> type == type) {
	tor = tl -> tor;
	if (strsame (tor -> type -> name, name) && tor -> pn[BA] == snB -> pn) {
	    if (type == VBJTELEM) {
		if (tor -> pn[CO] == pnC && tor -> pn[EM] == snE -> pn) {
		    return (tor);
		}
	    } else { /* type == LBJTELEM */
		if ((tor -> pn[CO] == pnC && tor -> pn[EM] == snE -> pn) ||
		    (tor -> pn[EM] == pnC && tor -> pn[CO] == snE -> pn)) {
		    return (tor);
		}
	    }
	}
    }
    return (NULL);
}

void pnTorLinkAdd (polnode_t *pn, BJT_t *tor, int type)
{
    pnTorLink_t *tl = NEW (pnTorLink_t, 1);
    tl -> type = type;
    tl -> tor = tor;
    tl -> next = pn -> tors;
    pn -> tors = tl;
}

BJT_t * newVBJT (subnode_t *snC, subnode_t *snB, subnode_t *snE, subnode_t *snS,
	struct elemDef *type, double dim, int sidewall)
{
    int i;
    BJT_t *vT;

    ASSERT (snC -> pn && snB -> pn && snE -> pn);

    if ((vT = findBJT (snE -> pn -> tors, snC -> pn, snB, snE, type -> name, VBJTELEM))) {
	if (!sidewall)
	    vT -> area += dim;
	else
	    vT -> length += dim;

        if (snS && vT -> n[SU] == NULL) {
            vT -> n[SU] = snS -> node;
	    SET_BJT_TERM (snS -> node);
	    if (snS -> pn) {
		ASSERT (vT -> pn[SU] == NULL);
		vT -> pn[SU] = snS -> pn;
		pnTorLinkAdd (snS -> pn, vT, VBJTELEM);
	    }
        }
	return (vT);
    }

    inVBJT++;
    currIntVBJT++;

    vT = NEW (BJT_t, 1);
    vT -> type = type;

    if (!sidewall) {
	vT -> area = dim;
	vT -> length = 0;
    }
    else {
	vT -> area = 0;
	vT -> length = dim;
    }
    vT -> scalef = 1;

    vT -> pn[CO] = snC -> pn;
    vT -> pn[BA] = snB -> pn;
    vT -> pn[EM] = snE -> pn;
    if (snS)
        vT -> pn[SU] = snS -> pn;
    else
        vT -> pn[SU] = NULL;

    vT -> n[CO] = snC -> node;
    vT -> n[BA] = snB -> node;
    vT -> n[EM] = snE -> node;
    if (snS)
        vT -> n[SU] = snS -> node;
    else
        vT -> n[SU] = NULL;

    for (i = 0; i < 4; i++) {
        if (vT -> pn[i]) {
	    pnTorLinkAdd (vT -> pn[i], vT, VBJTELEM);
	} else ASSERT (i == 3);		/* EM: only allowed for SU node */
    }

#ifndef CONFIG_SPACE2
    vT -> instName = NULL;
    vT -> tiles = NULL;
#endif

    SET_BJT_TERM (snC -> node);
    SET_BJT_TERM (snB -> node);
    SET_BJT_TERM (snE -> node);
    if (snS) SET_BJT_TERM (snS -> node);
    return (vT);
}

BJT_t * newLBJT (polnode_t *pnC, subnode_t *snB, subnode_t *snE, subnode_t *snS,
	struct elemDef *type, int econ, coor_t basew)
{
    int i;
    node_t *nC;
    BJT_t *lT;

    if ((lT = findBJT (snE -> pn -> tors, pnC, snB, snE, type -> name, LBJTELEM))) {

	if (basew < lT -> basew) lT -> basew = basew;

        if (snS && lT -> n[SU] == NULL) {
            lT -> n[SU] = snS -> node;
	    SET_BJT_TERM (snS -> node);
	    if (snS -> pn) {
		ASSERT (lT -> pn[SU] == NULL);
		lT -> pn[SU] = snS -> pn;
		pnTorLinkAdd (snS -> pn, lT, LBJTELEM);
	    }
        }
	return (lT);
    }

    inLBJT++;
    currIntLBJT++;

    lT = NEW (BJT_t, 1);
    lT -> type = type;

    lT -> area = 0;
    lT -> length = 0;
    lT -> scalef = 1;
    lT -> basew = basew;

    nC = nearestNode (pnC, snE -> node);
    ASSERT (nC);

    if (snE -> pn -> conNr == econ) {
        lT -> n[CO] = nC;
        lT -> pn[CO] = pnC;
        lT -> n[EM] = snE -> node;
        lT -> pn[EM] = snE -> pn;
    }
    else {
        ASSERT (pnC -> conNr == econ);
        lT -> n[EM] = nC;
        lT -> pn[EM] = pnC;
        lT -> n[CO] = snE -> node;
        lT -> pn[CO] = snE -> pn;
    }
    lT -> pn[BA] = snB -> pn;
    lT -> n[BA] = snB -> node;

    if (snS) {
	lT -> n[SU] = snS -> node;
	lT -> pn[SU] = snS -> pn;
    } else {
    	lT -> n[SU] = NULL;
    	lT -> pn[SU] = NULL;
    }

    for (i = 0; i < 4; i++) {
        if (lT -> pn[i]) {
	    pnTorLinkAdd (lT -> pn[i], lT, LBJTELEM);
	} else ASSERT (i == 3);		/* EM: only allowed for SU node */
    }

#ifndef CONFIG_SPACE2
    lT -> instName = NULL;
    lT -> tiles = NULL;
#endif

    SET_BJT_TERM (nC);
    SET_BJT_TERM (snB -> node);
    SET_BJT_TERM (snE -> node);
    if (snS)  SET_BJT_TERM (snS -> node);
    return (lT);
}

/* return the node that is linked to <pn> and is
 * closest (in lateral width) to node <n>.
 */
Private node_t * nearestNode (polnode_t *pn, node_t *n)
{
    nodeLink_t * nL;
    double r, x, y, fr = 0;
    node_t * node = 0;

    for (nL = pn -> nodes; nL; nL = nL -> next_n) {

	x = nL -> n -> node_x - n -> node_x;
	y = nL -> n -> node_y - n -> node_y;

	r = sqrt (x * x + y * y);

	if (!node || r < fr) {
	    node = nL -> n;
	    fr = r;
	}
    }
    return (node);
}

pnTorLink_t * parBJT (pnTorLink_t *tp, pnTorLink_t *tl)
{
    BJT_t *bjt, *tor;
    node_t *n;

    tor = tl -> tor;
    for (; tp; tp = tp -> next) {
	if (tp != tl && tp -> type == tl -> type) {
	    bjt = tp -> tor;
	    if (bjt != tor
		&& strsame (bjt -> type -> name, tor -> type -> name)
		&& bjt -> pn[BA] == tor -> pn[BA]
		&& bjt -> pn[SU] == tor -> pn[SU]) {

		if (bjt -> pn[CO] != tor -> pn[CO]) {
		    if (tl -> type != LBJTELEM || bjt -> pn[EM] != tor -> pn[CO]) continue;
		    n = bjt -> n[EM];
		    bjt -> n[EM] = bjt -> n[CO];
		    bjt -> n[CO] = n;
		    bjt -> pn[EM] = bjt -> pn[CO];
		    bjt -> pn[CO] = tor -> pn[CO];
		}

		/* Check if transistors with different emitter polnodes,
		 * but the same emitter conductors, may be joined.
		 */
		if (parallelMerge) {
		    if (Grp (bjt -> n[EM]) == Grp (tor -> n[EM])) return (tp);
		}

		/* Otherwise the emitter polnodes must be the same and
		 * if the transistor is outputted as a distributed devices
		 * we must check on having the same node connections.
		 */
		if (bjt -> pn[EM] == tor -> pn[EM]) return (tp);
	    }
	}
    }
    return (tp);
}

void pnTorReLink (polnode_t *pnB, polnode_t *pn)
{
    int i;
    pnTorLink_t *tl, *tp;
    dsBoundary_t *b;
    BJT_t *tor;

    while ((tl = pnB -> tors)) {
	tor = tl -> tor;
	if (tl -> type == TORELEM) {
	    for (b = ((transistor_t*)tor) -> boundaries; b; b = b -> next) {
		if (b -> pn == pnB) b -> pn = pn;
	    }
	}
	else {
	    for (i = 0; i < 4; i++) {
		if (tor -> pn[i] == pnB) tor -> pn[i] = pn;
	    }
	}

	pnB -> tors = tl -> next;

	for (tp = pn -> tors; tp; tp = tp -> next) {
	    if (tp -> tor == tor) break;
	}
	if (!tp) { /* tor not found */
	    tl -> next = pn -> tors;
	    pn -> tors = tl;
	}
	else {
	    DISPOSE (tl, sizeof(pnTorLink_t));
	}
    }
}

void deviceDel (polnode_t *pnR, pnTorLink_t *tl)
{
    int i;
    BJT_t *tor;
    pnTorLink_t *tp;
    polnode_t *pn;

    if (tl -> type == VBJTELEM)
	currIntVBJT--;
    else
	currIntLBJT--;

    tor = tl -> tor;

    for (i = 0; i < 4; i++) {
	if ((pn = tor -> pn[i])) {
	    tp = tl = pn -> tors;
	    while (tp -> tor != tor) tp = tp -> next;
	    if (tp == tl)
		pn -> tors = tp -> next;
	    else {
		while (tl -> next != tp) tl = tl -> next;
		tl -> next = tp -> next;
	    }
	    DISPOSE (tp, sizeof(pnTorLink_t));

	    if (pn != pnR && !pn -> tors && !pn -> juncs && !pn -> subs) polnodeDispose (pn);
	}
	else ASSERT (i == 3); /* EM: only allowed for SU node */
    }

#ifndef CONFIG_SPACE2
    if (tor -> tiles) {
	tileRef_t *tR;
	while ((tR = tor -> tiles)) {
	    tor -> tiles = tR -> next;
	    DISPOSE (tR, sizeof(tileRef_t));
	}
    }
#endif
    DISPOSE (tor, sizeof(BJT_t));
}
