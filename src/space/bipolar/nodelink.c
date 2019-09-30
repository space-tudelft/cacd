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
#include "src/space/lump/define.h"
#include "src/space/lump/extern.h"

#include "src/space/bipolar/define.h"
#include "src/space/bipolar/extern.h"

/* Relink first polnode nodeLink of <nB> to node <nA>.
 */
void relinkNodeLink (node_t *nA, nodeLink_t *nLB)
{
    nodeLink_t *nL;

    for (nL = nA -> pols; nL; nL = nL -> next_pn) {
	if (nL -> pn == nLB -> pn) break;
    }
    if (!nL) { /* not found, add nLB to <nA> */
	nLB -> n -> pols = nLB -> next_pn;
	nLB -> n = nA;
	nLB -> next_pn = nA -> pols;
	nA -> pols = nLB;
    }
    else nodeLinkDel (nLB);
}

/* Create new nodelink between <n> and <pn>
 */
void nodeLinkAdd (node_t *n, polnode_t *pn)
{
    nodeLink_t *nL;

    nL = NEW (nodeLink_t, 1);
    nL -> n = n;
    nL -> pn = pn;
    nL -> next_n = pn -> nodes;
    nL -> next_pn = n -> pols;
    n -> pols = nL;
    pn -> nodes = nL;
    Grp (n) -> notReady++;
}

/* Remove <nL> from both the node- and polnode-list
 */
void nodeLinkDel (nodeLink_t *nL)
{
    nodeLink_t *pnL;
    node_t *n = nL -> n;

    /* Relink the polnode-links of <nL>
     */
    if ((pnL = n -> pols) == nL) n -> pols = nL -> next_pn;
    else {
	while (pnL -> next_pn != nL) pnL = pnL -> next_pn;
	pnL -> next_pn = nL -> next_pn;
    }

    /* Relink the node-links of <nL>
     */
    if ((pnL = nL -> pn -> nodes) == nL) nL -> pn -> nodes = nL -> next_n;
    else {
	while (pnL -> next_n != nL) pnL = pnL -> next_n;
	pnL -> next_n = nL -> next_n;
    }
    Grp (n) -> notReady--;
    DISPOSE (nL, sizeof(nodeLink_t));
}

/* Remove all nodeLinks from polnode
 */
void nodeLinksDel (polnode_t *pn)
{
    group_t *g;
    node_t *n;
    nodeLink_t *pnL, *nL;

    if (!(nL = pn -> nodes)) return;
    g = Grp (nL -> n);

    do {
	n = nL -> n;
	if (n -> pols == nL) {
	    n -> pols = nL -> next_pn;
	}
	else {
	    pnL = n -> pols;
	    while (pnL -> next_pn != nL) pnL = pnL -> next_pn;
	    pnL -> next_pn = nL -> next_pn;
	}
	pnL = nL -> next_n;
	g -> notReady--;
	DISPOSE (nL, sizeof(nodeLink_t));
    } while ((nL = pnL));

    if (g -> notReady == 0) readyGroup (n);
    else ASSERT (g -> notReady > 0);
}
