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
#include <string.h>
#include <stdlib.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/export.h"
#include "src/space/extract/define.h"
#include "src/space/extract/export.h"
#include "src/space/lump/define.h"
#include "src/space/lump/extern.h"
#include "src/space/X11/export.h"
#include "src/space/bipolar/define.h"
#include "src/space/bipolar/export.h"

#define KEEPNODE(node) node -> term |= 2
#define SET_TERM(node) node -> term |= 1+4

#define Degree(n) n -> res_cnt
#define Ready(n) !n -> subs
#define Keep(n)   n -> term
#define X strCoor

#ifdef __cplusplus
extern "C" {
#endif
extern void joinNodes (node_t *nA, node_t *nB);
extern void joinClusters (cluster_t *cA, cluster_t *cB);
extern void clusterIt (nodePoint_t *p1, nodePoint_t *p2);
extern void outCapacitor (node_t *n, netEquiv_t *ne, int nr, int sort, double val);
extern void outNode (node_t *n);
extern void removeNodeFromCluster (node_t *n, int dispose);
extern void subnodeNetAreaCoor (subnode_t *subn, double area, tile_t *tile);
extern void addNetEqCAP (node_t *n, int nr, int sort, double val);
extern netEquiv_t *putNetEqCAP (node_t *n, netEquiv_t *ne, netEquiv_t *p);
extern netEquiv_t *putNetEqRES (node_t *n, netEquiv_t *ne, netEquiv_t *p);
extern void preElim (node_t *n, int k);
extern void readyGrp (group_t *grp);
extern void nqElimCluster (node_t *n, int type);
extern void addNodeToCluster (node_t *nA, cluster_t *cB);
Private void disposeCluster (cluster_t *cB);
Private void joinNodeClusters (node_t *nA, node_t *nB);
Private void readyCluster (cluster_t *cluster, int type);
#ifdef __cplusplus
}
#endif

extern bool_t termIsLabel;
extern int currIntAln, currIntClr, maxIntClr;
extern int sub_caps_entry;
extern int inGrp;

extern int pre_elim_degree;
extern int corner_line_node;
extern int joiningCon;

extern coor_t joiningX;
extern coor_t joiningY;

bool_t outputNoDangling = FALSE; /* used to prevent output */
#ifdef CL_LIST
cluster_t * cl_list = NULL;
#endif

void conAdd (subnode_t *subnA, subnode_t *subnB, double val, int sort)
{
    inRes++;
    elemAddRes ('G', subnA -> node, subnB -> node, val, sort);
}

void conAddS (subnode_t *subnA, subnode_t *subnB, double val)
{
    inRes++;
 /* ASSERT (substrRes); */
    elemAddRes ('S', subnA -> node, subnB -> node, val, 0);
}

void conAddSUB (subnode_t *sn, double val)
{
    inRes++;
    ASSERT (sn -> node -> substr);

    /* The value may not become zero because otherwise
       it will be eliminated (too soon). */
    if ((sn -> node -> substrCon[0] += val) <= 0) {
	if (sn -> node -> substrCon[0] == 0)
	    say ("warning: resistance to substrate node becomes zero");
	else if (!optSubRes)
	    say ("warning: resistance to substrate node becomes < 0");
    }
}

void capAddS (subnode_t *snA, subnode_t *snB, double val)
{
    elemAddCap (snA -> node, snB -> node, val, sub_caps_entry, NULL);
    inCap++;
}

void capAddSUB (subnode_t *sn, double val)
{
    inCap++;
    /* The value may not become zero because otherwise
       it will be eliminated (too soon). */
    if ((sn -> node -> substrCap[sub_caps_entry] += val) == 0) {
	say ("warning: capacitance to substrate node becomes zero");
    }
}

void capAdd (subnode_t *subnA, subnode_t *subnB, double val, int sort)
{
    if (val == 0) return;

    ASSERT (subnA != subnGND);
 // ASSERT (subnA != subnSUB);

    if (subnB == subnGND) { subnA -> node -> gndCap[sort] += val; return; }
 // if (subnB == subnSUB) { subnA -> node -> gndCap[sort] += val; return; }
    elemAddCap (subnA -> node, subnB -> node, val, sort, NULL);
    inCap++;
}

void capAddNEG (subnode_t *subnA, subnode_t *subnB, double v, int s)
{
    node_t *nA = subnA -> node;

    if (subnB == subnGND) {
	if (nA -> gndCap[s] - v >= 0 || nA -> gndCap[s] < 0) nA -> gndCap[s] -= v;
    }
    else if (subnB == subnSUB) {
	if (nA -> substrCap[s] - v >= 0 || nA -> substrCap[s] < 0) nA -> substrCap[s] -= v;
    }
    else {
	element_c *cap;
	if ((cap = findCapElement (nA, subnB -> node, s))) {
	    if (cap -> val - v > 0 || cap -> val < 0) cap -> val -= v;
	    else elemDelCap (cap); /* don't make zero or negative cap */
	}
    }
}

void subnodeNew (subnode_t *subn)
{
    node_t *n;
    group_t *grp;

    subn -> next_node = NULL;
    subn -> prev_node = NULL;
    subn -> node = n = createNode ();
    n -> clr = NULL; n -> cl_next = NULL;

    inSubnod++;
    if (++currIntSubnod > maxIntSubnod) maxIntSubnod = currIntSubnod;

    subn -> next_pn = NULL;
    subn -> pn = NULL;

    n -> subs = subn;
    n -> grp = grp = NEW (group_t, 1);
    grp -> nodes = n;
    n -> gprev = n;
    n -> gnext = NULL;

    grp -> grp_nr = 0;
    grp -> nod_cnt = 1;
    grp -> notReady = 1;
    grp -> supply = 0;

    ++inGrp;
    if (++currIntGrp > maxIntGrp) maxIntGrp = currIntGrp;
}

void subnodeNew2 (subnode_t *subn, group_t *grp)
{
    node_t *n;

    subn -> next_node = NULL;
    subn -> prev_node = NULL;
    subn -> node = n = createNode ();
    n -> clr = NULL; n -> cl_next = NULL;

    inSubnod++;
    if (++currIntSubnod > maxIntSubnod) maxIntSubnod = currIntSubnod;

    subn -> next_pn = NULL;
    subn -> pn = NULL;

    n -> subs = subn;
    n -> grp = grp;
    n -> gprev = grp -> nodes -> gprev;
    n -> gnext = NULL;
    n -> gprev -> gnext = n;
    grp -> nodes -> gprev = n;
    grp -> nod_cnt++;
    grp -> notReady++;
}

void warnSubnodeJoin (int cx, coor_t x, coor_t y, int mode)
{
    if (!(warnSubnJoin & mode)) { warnSubnJoin |= mode;
	say ("warning: %snode join of subnodes with different conductor-type", mode == 1 ? "no " : "");
	say ("\tfor conductor mask '%s' at position %s.", conNr2Name (cx), strCoorBrackets (x, y));
    }
}

void subnodeJoin (subnode_t *subnA, subnode_t *subnB)
{
    polnode_t *pnA, *pnB;

    if ((pnA = subnA -> pn) && (pnB = subnB -> pn) && pnA != pnB)
	if (pnA -> type == pnB -> type)
	    if (pnA -> conNr == pnB -> conNr) /* LUMPED_MODEL */
		polnodeJoin (pnA, pnB);

    nodeJoin (subnA -> node, subnB -> node);
}

node_t * nodeJoin (node_t *nA, node_t *nB)
{
    int i;
    node_t *on;
    register element_c *cap;
    register element_r *con;
    subnode_t *last, *sub;
    group_t *grA, *grB;

    if (nA == nB) return (nA);

    grA = Grp (nA);
    grB = Grp (nB);

#define KEEP(n) (n -> term & 2)

    /* Make sure that the connections from the smallest node
     * (fewest number of connections, but resistances are more
     * important than capacitances) are added to those of
     * the largest.
     */
    if (nA == nSUB || nB == nSUB) {
	ASSERT (extrPass && !optRes);
	/* eliminate nB, but don't eliminate nSUB */
	if (nB == nSUB) { nB = nA; nA = nSUB; }
    }
    else if (nB -> clr && !nA -> clr && !nA -> area) {
	Swap (node_t *, nA, nB);
    }
    else if ((KEEP(nB) && !KEEP(nA)) || ((!KEEP(nA) || KEEP(nB)) &&
       (10 * nA -> res_cnt + 10 * nA -> cap_cnt + grA -> notReady + nA -> n_n_cnt <
	10 * nB -> res_cnt + 10 * nB -> cap_cnt + grB -> notReady + nB -> n_n_cnt))) {
	/* swap nodes nA and nB for efficiency reasons */
	Swap (node_t *, nA, nB);
    }

    if (grA != grB) grA = mergeGrps (grA, grB);

    if (nB -> substr > nA -> substr) nA -> substr = nB -> substr;
    nA -> term |= nB -> term;
    if (nB -> area > nA -> area) nA -> area = nB -> area;

    for (i = 0; i < resSortTabSize; i++) {
	nA -> substrCon[i] += nB -> substrCon[i];
	for (con = nB -> con[i]; con; con = NEXT (con, nB)) {
	    on = OTHER (con, nB);
	    elemAddRes (con -> type, nA, on, con -> val, i);
	    elemDelRes (con);
	}
    }

    for (i = 0; i < capSortTabSize; i++) {
	nA -> gndCap[i] += nB -> gndCap[i];
	nA -> substrCap[i] += nB -> substrCap[i];
	for (cap = nB -> cap[i]; cap; cap = NEXT (cap, nB)) {
	    on = OTHER (cap, nB);
	    elemAddCap (nA, on, cap -> val, i, NULL);
	    elemDelCap (cap);
	}
    }

    nodeRelJoin (nA, nB);

    last = NULL;
    for (sub = nB -> subs; sub; sub = sub -> next_node) {
	sub -> node = nA;
	last = sub;
    }
    ASSERT (last);
    ASSERT (nA -> subs);
    last -> next_node = nA -> subs;
    nA -> subs -> prev_node = last;
    nA -> subs = nB -> subs;

    if ((nB -> mask >= 0 || nA -> mask == nB -> mask)
        && (nB -> node_x < nA -> node_x
            || (nB -> node_x == nA -> node_x && nB -> node_y < nA -> node_y)
            || (nA -> mask < 0 && nB -> mask >= 0))) {
        nA -> mask = nB -> mask;
	nA -> node_x = nB -> node_x;
	nA -> node_y = nB -> node_y;
    }

    if (nB -> pols) {
	node_t **n;
	nodeLink_t *nL;
	pnTorLink_t *dL;

	/* If <nB> refers to polnodes, goto all polnodes and set their
	 * possible junction node-references to <nA>.
	 */
	while ((nL = nB -> pols)) {
	    for (dL = nL -> pn -> tors; dL; dL = dL -> next){
		if (dL -> type != TORELEM) {
		    n = dL -> tor -> n;
		    for (i = 0; i < 4; i++) if (n[i] == nB) n[i] = nA;
		}
	    }
	    /* Relink polnode nodeLink of <nB> to <nA> (or delete).
	     */
	    relinkNodeLink (nA, nL);
	}
    }

    if (nB -> clr) {
	if (nA -> area) removeNodeFromCluster (nB, 3);
	else joinNodeClusters (nA, nB);
    }
    ASSERT (!nB -> clr);

    grA -> notReady--;
    nodeDel (nB);
    ASSERT (grA -> notReady > 0);

    return (nA);
}

void joinNodes (node_t *nA, node_t *nB)
{
    int i;
    node_t *on;
    register element_c *cap;
    register element_r *con;

    ASSERT (nA != nB);
    ASSERT (nA && nA != nSUB);
    ASSERT (nB && nB != nSUB);
    ASSERT (Grp (nA) == Grp (nB)); /* in dezelfde groep */

    ASSERT (!nA -> subs); /* ready */
    ASSERT (!nB -> subs); /* ready */

    if (nB -> clr) {
	ASSERT (nA -> area || nA -> clr == nB -> clr);
    }
    else ASSERT (!nA -> clr);

    if (nB -> substr > nA -> substr) nA -> substr = nB -> substr;
    nA -> term |= nB -> term;
    if (nB -> area > nA -> area) nA -> area = nB -> area;

    for (i = 0; i < resSortTabSize; i++) {
	nA -> substrCon[i] += nB -> substrCon[i];
	for (con = nB -> con[i]; con; con = NEXT (con, nB)) {
	    on = OTHER (con, nB);
	    elemAddRes (con -> type, nA, on, con -> val, i);
	    elemDelRes (con);
	}
    }

    for (i = 0; i < capSortTabSize; i++) {
	nA -> gndCap[i] += nB -> gndCap[i];
	nA -> substrCap[i] += nB -> substrCap[i];
	for (cap = nB -> cap[i]; cap; cap = NEXT (cap, nB)) {
	    on = OTHER (cap, nB);
	    elemAddCap (nA, on, cap -> val, i, NULL);
	    elemDelCap (cap);
	}
    }

    nodeRelJoin (nA, nB);

    if ((nB -> mask >= 0 || nA -> mask == nB -> mask)
        && (nB -> node_x < nA -> node_x
            || (nB -> node_x == nA -> node_x && nB -> node_y < nA -> node_y)
            || (nA -> mask < 0 && nB -> mask >= 0))) {
        nA -> mask = nB -> mask;
	nA -> node_x = nB -> node_x;
	nA -> node_y = nB -> node_y;
    }

    if (nB -> clr) removeNodeFromCluster (nB, 0);
    nodeDel (nB);
}

void addNodeToCluster (node_t *nA, cluster_t *cB)
{
    ASSERT (!Ready(nA)); /* nA is a not ready node */
    ASSERT (!nA -> clr);
    nA -> clr = cB;
    nA -> cl_next = cB -> nodes;
    cB -> nodes = nA;
    cB -> notReadyNodes++;
}

#ifdef ADD_ALINKS
Private void addAreaNodeToCluster (node_t *nA, cluster_t *cB)
{
    alnk_t *al;

    for (al = cB -> Anodes; al; al = al -> Anext) {
	if (al -> Anode == nA) return; /* don't add twice */
    }
    al = NEW (alnk_t, 1); ++currIntAln;
    al -> Anode = nA;
    al -> Anext = cB -> Anodes;
    cB -> Anodes = al;
}

Private void removeAreaNodeFromCluster (node_t *nA, cluster_t *cB)
{
    alnk_t *al, *p = NULL;

    for (al = cB -> Anodes; al; al = (p = al) -> Anext) {
	if (al -> Anode == nA) { /* found */
	    if (p) p -> Anext = al -> Anext;
	    else cB -> Anodes = al -> Anext;
	    DISPOSE (al, sizeof(alnk_t)); --currIntAln;
	    return;
	}
    }
}
#endif /* ADD_ALINKS */

void clusterIt (nodePoint_t *p1, nodePoint_t *p2)
{
    cluster_t *cl;
    nodePoint_t *p, *ok;
    node_t *node;

    cl = NULL;
    ok = NULL;
    for (p = p1;; p = p -> next) {
	node = p -> cons[joiningCon] -> node;
	if (node -> clr) {
	    if (!cl) cl = node -> clr;
	    else if (node -> clr != cl) joinClusters (cl, node -> clr);
	}
	else if (!node -> area && !ok) ok = p; /* cluster this node */
	if (p == p2) break;
    }
    if (!ok) return; /* no node needs to be clustered */
    if (!cl) { /* no cluster found */
	cl = NEW (cluster_t, 1); if (++currIntClr > maxIntClr) maxIntClr = currIntClr;
#ifdef ADD_ALINKS
	cl -> Anodes = NULL;
#endif
	cl -> nodes = NULL;
	cl -> notReadyNodes = 0;
#ifdef CL_LIST
	cl -> x = p1 -> x;
	cl -> y = p1 -> y;
	cl -> next = cl_list;
	cl_list = cl;
#endif
    }
    for (p = ok;; p = p -> next) {
	node = p -> cons[joiningCon] -> node;
	if (!node -> clr && !node -> area) addNodeToCluster (node, cl);
	if (p == p2) break;
    }
}

Private void disposeCluster (cluster_t *cB)
{
#ifdef CL_LIST
    if (cl_list == cB) cl_list = cB -> next;
    else { cluster_t *cl;
	for (cl = cl_list; cl && cl -> next != cB;) cl = cl -> next;
	ASSERT (cl);
	cl -> next = cB -> next;
    }
#endif
    DISPOSE (cB, sizeof(cluster_t)); --currIntClr;
}

void joinClusters (cluster_t *cA, cluster_t *cB)
{
    node_t *n = cB -> nodes;
    n -> clr = cA;
    while (n -> cl_next) { n = n -> cl_next; n -> clr = cA; }
    n -> cl_next = cA -> nodes;
    cA -> nodes = cB -> nodes;
    cA -> notReadyNodes += cB -> notReadyNodes;
    disposeCluster (cB);
}

Private void joinNodeClusters (node_t *nA, node_t *nB)
{
    cluster_t *cA = nA -> clr;
    cluster_t *cB = nB -> clr;

    if (cA) { if (cB != cA) joinClusters (cA, cB); }
    else { cA = cB; addNodeToCluster (nA, cA); }
    removeNodeFromCluster (nB, 0);
    ASSERT (cA -> nodes);
}

void removeNodeFromCluster (node_t *node, int dispose)
{
    cluster_t *cl = node -> clr;

    node -> clr = NULL;
    if (!Ready (node)) cl -> notReadyNodes--;

    if (cl -> nodes == node) {
	cl -> nodes = node -> cl_next;
	if (!cl -> nodes) {
	    ASSERT (cl -> notReadyNodes == 0);
	    if (dispose) disposeCluster (cl);
	    return;
	}
    }
    else {
	node_t *n = cl -> nodes;
	while (n && n -> cl_next != node) n = n -> cl_next;
	ASSERT (n);
	n -> cl_next = node -> cl_next;
    }
    if (dispose >= 2) {
	if (cl -> notReadyNodes == 0) readyCluster (cl, dispose);
	else ASSERT (cl -> notReadyNodes > 0);
    }
}

#define NOT_BJT_TERM(n) n -> term < 8

Private void readyCluster (cluster_t *cl, int type)
{
    node_t *n;
    if ((n = cl -> nodes)) {
	nqElimCluster (n, type);
	for (n = cl -> nodes; n; n = n -> cl_next) {
	    ASSERT (n -> clr == cl);
	    n -> clr = NULL;
	    //if (NOT_BJT_TERM (n)) outNode (n);
	}
    }
    disposeCluster (cl);
}

void nodeRelJoin (node_t *nA, node_t *nB) /* join nB to nA */
{
    terminal_t *ta, *tb;

    ASSERT (nB != nSUB);
    /* nA and nB are in the same group */

    if ((tb = nB -> names)) {
	if (!(ta = nA -> names)) nA -> names = tb;
	else {
	    if (!tb -> instName && (ta -> instName ||
			tb -> x < ta -> x || (tb -> y < ta -> y && tb -> x == ta -> x))) {
		nA -> names = tb; tb = ta; ta = nB -> names;
	    }
	    while (ta -> next) ta = ta -> next;
	    ta -> next = tb;
	}
	nB -> names = NULL;
    }

    nA -> n_n_cnt += nB -> n_n_cnt;  /* for names and netEq */

    if (nB -> netEq) {
	if (nA -> netEq) {
	    netEquiv_t *e, *p, *ne = nB -> netEq;
	    p = NULL;
	    while ((e = ne)) { ne = e -> next; p = putNetEqRES (nA, e, p); }
	}
	else {
	    nA -> netEq = nB -> netEq;
	    nA -> res_cnt += nB -> res_cnt;
	}
	nB -> netEq = NULL;
    }
    if (nB -> netEq2) {
	if (nA -> netEq2) {
	    netEquiv_t *e, *p, *ne = nB -> netEq2;
	    p = NULL;
	    while ((e = ne)) { ne = e -> next; p = putNetEqCAP (nA, e, p); }
	} else {
	    if (nA == nSUB) {
		netEquiv_t *e, *ne = nB -> netEq2;
		while ((e = ne)) {
		    ne = e -> next;
		    outCapacitor (nA, NULL, -e -> number, e -> sort, e -> val);
		    DISPOSE (e, sizeof(netEquiv_t)); currNeqv--;
		}
	    }
	    else {
		nA -> netEq2 = nB -> netEq2;
		nA -> cap_cnt += nB -> cap_cnt;
	    }
	}
	nB -> netEq2 = NULL;
    }
}

/* subnodeCopy (subnA, subnB) is a short-hand for
 *    subnodeNew (subnB);
 *    subnodeJoin (subnA, subnB);
 */
void subnodeCopy (subnode_t *subnA, subnode_t *subnB)
{
    node_t *nA = subnA -> node;

    inSubnod++;
    if (++currIntSubnod > maxIntSubnod) maxIntSubnod = currIntSubnod;

    ASSERT (nA -> subs);
    subnB -> node = nA;
    subnB -> next_node = nA -> subs;
    nA -> subs -> prev_node = subnB;
    nA -> subs = subnB;

    /* In case the subnodeCopy is not followed by a polnodeCopy,
     * initialize all polnode-references.
     */
    subnB -> pn = NULL;
    subnB -> next_pn = NULL;

    if (subnA -> pn) {
	if (subnA -> pn -> type == subnB -> cond -> type)
	    polnodeCopy (subnA, subnB);
	else
	    polnodeAdd (subnB, subnB -> cond -> con, subnB -> cond -> type);
    }
}

void nameAdd (subnode_t *sn, terminal_t *t)
{
    terminal_t *t1;
    node_t *n;
    group_t *grp;
    int i;
    char *tname = t -> termName;

    n = sn -> node;
    SET_TERM (n);
    grp = Grp (n);

    if (!(t1 = n -> names) || t1 -> instName) {
	t -> next = t1;
	n -> names = t;
    }
    else {
	t -> next = t1 -> next;
	t1 -> next = t;
    }

    n -> n_n_cnt++;

if (grp -> supply != 3) {
    for (i = 0; i < no_pos_supply; i++) {
	if (strsame (pos_supply[i], tname)) break;
    }
    if (i < no_pos_supply || (
	(tname[0] == 'v' || tname[0] == 'V') &&
	(tname[1] == 'd' || tname[1] == 'D') &&
	(tname[2] == 'd' || tname[2] == 'D'))) {
	grp -> supply |= 2;
    }

    for (i = 0; i < no_neg_supply; i++) {
	if (strsame (neg_supply[i], tname)) break;
    }
    if (i < no_neg_supply || (
	(tname[0] == 'v' || tname[0] == 'V') &&
	(tname[1] == 's' || tname[1] == 'S') &&
	(tname[2] == 's' || tname[2] == 'S')) || (
	(tname[0] == 'g' || tname[0] == 'G') &&
	(tname[1] == 'n' || tname[1] == 'N') &&
	(tname[2] == 'd' || tname[2] == 'D'))) {
	grp -> supply |= 1;
    }
    if (grp -> supply == 3) {
	joiningCon = t -> conductor;
	joiningX = t -> x;
	joiningY = t -> y;
	supplyShort ();
    }
}

    if (!t -> instName) {
      if (strsame (tname, nameGND))
	say ("WARNING: %s '%s' is already predefined as ground net name",
		t -> type == tTerminal ? "terminal" : "label", tname);
      if (strsame (tname, nameSUBSTR))
	say ("WARNING: %s '%s' is already predefined as substrate net name",
		t -> type == tTerminal ? "terminal" : "label", tname);
    }
}

void supplyShort ()
{
    say ("Vdd connected to Vss/Gnd%s.", optRes ? " (possibly via one or more resistors)" : "");
    say ("\tA point on this net is mask '%s' at position %s.",
	conNr2Name (joiningCon), strCoorBrackets (joiningX, joiningY));
}

void subnodeReconnect (subnode_t *subn, subnode_t *subn1, subnode_t *subn2, double val1, double val2)
{
    register int i, j;
    register element_c *cap;
    node_t *n, *on, *n1, *n2;
    double frac1, frac2, val;

    n = subn -> node;
    n1 = subn1 -> node;
    n2 = subn2 -> node;

    if (n == n1 || n == n2) return;

    frac1 = val1 / (val1 + val2);
    frac2 = val2 / (val1 + val2);

    for (j = 0; j < resSortTabSize; j++) {
	ASSERT (!n -> con[j]);
	ASSERT (n -> substrCon[j] == 0);
    }
    ASSERT (n -> netEq == NULL);

    for (i = 0; i < capSortTabSize; i++) {
	if ((val = n -> gndCap[i]) != 0) {
	    n2 -> gndCap[i] += val * frac1;
	    n1 -> gndCap[i] += val * frac2;
	    n -> gndCap[i] = 0;
	}
	if ((val = n -> substrCap[i]) != 0) {
	    n2 -> substrCap[i] += val * frac1;
	    n1 -> substrCap[i] += val * frac2;
	    n -> substrCap[i] = 0;
	}

	for (cap = n -> cap[i]; cap; cap = NEXT (cap, n)) {
	    on = OTHER (cap, n);
	    elemAddCap (n1, on, cap -> val * frac2, i, NULL);
	    elemAddCap (n2, on, cap -> val * frac1, i, NULL);
	    elemDelCap (cap);
	}
    }
    if (n -> netEq2) {
	netEquiv_t *e, *p, *ne = n -> netEq2;
	p = NULL;
	while ((e = ne)) {
	    ne = e -> next;
	    addNetEqCAP (n1, e -> number, e -> sort, e -> val * frac2);
	    e -> val *= frac1;
	    p = putNetEqCAP (n2, e, p);
	}
    }
}

void makeAreaNode (subnode_t *subn)
{
    node_t *node = subn -> node;
    if (node -> area != 2) {
	ASSERT (!node -> area);
	node -> area  = 2; areaNodes++;
    }
    if (node -> clr) {
#ifdef ADD_ALINKS
	addAreaNodeToCluster (node, node -> clr);
#endif
	removeNodeFromCluster (node, 2);
    }
}

void makeLineNode (subnode_t *subn)
{
    extern int equi_line_area;
    node_t *node = subn -> node;
    ASSERT (!node -> area);
    if (equi_line_area) {
	node -> area = 1; equiLines++;
	if (node -> clr) {
#ifdef ADD_ALINKS
	    addAreaNodeToCluster (node, node -> clr);
#endif
	    removeNodeFromCluster (node, 4);
	}
	if (corner_line_node) KEEPNODE (node);
    }
}

void subnodeDel (subnode_t *subn)
{
    node_t *n = subn -> node;
    polnode_t *pn = subn -> pn;

    /* Remove the subnode outof the subnode-list linked to its
     * polnode and if it was the last subnode, perform a
     * polnodeDel for the polnode of subnode.
     */
    if (pn) {
	if (pn -> subs == subn)
	    pn -> subs = subn -> next_pn;
	else {
	    subnode_t * sn = pn -> subs;
	    while (sn -> next_pn != subn) sn = sn -> next_pn;
	    sn -> next_pn = subn -> next_pn;
	}
	if (!pn -> subs) polnodeDel (pn);
    }

    ASSERT (!Ready (n));

    /* Remove subnode from the list of subnodes for its node.
     */
    if (n -> subs == subn) n -> subs = subn -> next_node;
    else {
	subn -> prev_node -> next_node = subn -> next_node;
	if (subn -> next_node) subn -> next_node -> prev_node = subn -> prev_node;
    }

    /* Set subn -> node to NULL to ensure that that a reference via
     * subn to node can no longer occur; if it does occur it's a bug.
     */
    subn -> node = NULL;

    currIntSubnod--;

    if (Ready (n)) { /* there are no subnodes left */
	if (outputNoDangling) {
	    group_t *grp = Grp (n);
	    grp -> notReady--;
	    ASSERT (!n -> clr);
	    nodeDel (n);
	    ASSERT (grp -> notReady > 0);
	    return;
	}
	readyNode (n);
    }
}

void readyNode (node_t *n)
{
    cluster_t *cluster;
    int i, k;
    group_t *grp = Grp (n);

    grp -> notReady--;
    ASSERT (grp -> notReady >= 0);

//fprintf (stderr, "\nreadyNode0: memory allocation : %.3f Mbyte\n", allocatedMbyte ());
//fprintf (stderr, "readyNode: x,y=%s,%s area=%d term=%d res=%d\n", X(n->node_x), X(n->node_y), n->area, n->term, n->res_cnt);

    cluster = n -> clr;

    if (n -> area) {
	ASSERT (!cluster);
	areaNodesTotal++;
    }
    else if (cluster) {
	/* Not all nodes need to be in a cluster (for example nSUB). */
	if (--cluster -> notReadyNodes > 0 && !n -> term) { /* try pre-elim (not last node) */
	    k = testRCelim (n);
	    if (!Keep(n) && k >= 0 && Degree(n) < pre_elim_degree) preElim (n, k);
	}
	if (cluster -> notReadyNodes == 0) readyCluster (cluster, 1);
	else {
	    ASSERT (cluster -> notReadyNodes > 0);
	    ASSERT (grp -> notReady > 0);
	}
    }

    /* A ready group does not have clusters. */
    if (grp -> notReady == 0) readyGrp (grp);
}

void subtorNew (tile_t *tile, elemDef_t *el)
{
    transistor_t *t;

    tile -> tor = t = NEW (transistor_t, 1);
    tile -> next_tor = NULL;

    t -> subs = tile;
    t -> type = el;
    t -> xl = tile -> xl;
    t -> yb = tile -> bl;
    t -> gate = NULL;
    t -> bulk = NULL;
    t -> totPerimeter = 0;
    t -> dsPerimeter = 0;
    t -> surface = 0;
    t -> tor_nr = 0;
    t -> boundaries = NULL;

    inSubtor++;
    if (++currIntSubtor > maxIntSubtor) maxIntSubtor = currIntSubtor;

    inTor++;
    if (++currIntTor > maxIntTor) maxIntTor = currIntTor;
}

Private void warnTouchingDs (transistor_t *tor, coor_t x, coor_t y)
{
    static int dsWarn = 0;
    if (!dsWarn) { dsWarn = 1;
	say ("warning: transistor '%s' with touching drain and source area at position %s",
	    tor -> type -> name, strCoorBrackets (x, y));
    }
}

void subtorJoin (tile_t *tileA, tile_t *tileB)
{
    transistor_t *tA, *tB;
    tile_t *sub, *lasttile;
    dsBoundary_t *dsBdrA, *dsBdrB, *nextDsBdrB;
    dsCoor_t     *pointA, *pointB;
    subnode_t *snA, *snB;
    tileRef_t * tileR;

    tA = tileA -> tor;
    tB = tileB -> tor;

    if (tA == tB) return;

    lasttile = tB -> subs;
    ASSERT (lasttile);

    for (sub = lasttile; sub; sub = sub -> next_tor) {
	sub -> tor = tA;
	lasttile = sub;
    }
    lasttile -> next_tor = tA -> subs;
    tA -> subs = tB -> subs;

    /* update torBoundary structure */

    dsBdrB = tB -> boundaries;
    while (dsBdrB) {
	nextDsBdrB = dsBdrB -> next;
	for (pointB = dsBdrB -> points; pointB; pointB = pointB -> next) {
	    for (dsBdrA = tA -> boundaries; dsBdrA; dsBdrA = dsBdrA -> next) {
		for (pointA = dsBdrA -> points; pointA; pointA = pointA -> next) {
		    if (pointA -> x == pointB -> x && pointA -> y == pointB -> y) {
			if (dsBdrA -> type == dsBdrB -> type) {
			    snA = dsBdrA -> dsCond;
			    snB = dsBdrB -> dsCond;
			    if (snA -> cond -> type != snB -> cond -> type) {
				warnSubnodeJoin (snA -> cond -> con, pointA -> x, pointA -> y, 2);
			    }
			    subnodeJoin (snA, snB);
			    if (snA -> pn && tA -> type -> s.tor.dsCap) {
				pnTorLink_t *tl = snA -> pn -> tors;
				pnTorLink_t *tlp = NULL;
				while (tl && tl -> tor != (BJT_t*)tB) tl = (tlp = tl) -> next;
				if (tl) {
				    if (tlp) tlp -> next = tl -> next;
				    else snA -> pn -> tors = tl -> next;
				    DISPOSE (tl, sizeof(pnTorLink_t));
				}
			    }
			    subnodeDel (snB);
			    if (dsBdrA -> points == pointA) dsBdrA -> points = pointA -> next;
			    if (dsBdrB -> points == pointB) dsBdrA -> points -> next = pointB -> next;
			    else {
				dsBdrA -> points -> next = dsBdrB -> points;
				dsBdrB -> points -> next = NULL;
			    }
			    DISPOSE (snB, sizeof(subnode_t));
			    DISPOSE (dsBdrB, sizeof(dsBoundary_t));
			    DISPOSE (pointB, sizeof(dsCoor_t));
			    DISPOSE (pointA, sizeof(dsCoor_t));
			    goto added;
			}
			else warnTouchingDs (tA, pointA -> x, pointA -> y);
		    }
		}
	    }
	}
	/* SdeG: only in very special occasions this can happen */
	snB = dsBdrB -> dsCond;
	if (snB -> pn && tA -> type -> s.tor.dsCap) {
	    pnTorLink_t *tl = snB -> pn -> tors;
	    pnTorLink_t *tlA = NULL;
	    pnTorLink_t *tlp = NULL;
	    while (tl && tl -> tor != (BJT_t*)tB) {
		if (tl -> tor == (BJT_t*)tA) tlA = tl;
		tl = (tlp = tl) -> next;
	    }
	    if (tl) {
		if (!tlA) {
		    tlA = tl -> next;
		    while (tlA && tlA -> tor != (BJT_t*)tA) tlA = tlA -> next;
		}
		if (tlA) { /* polnode contains already a link to tA */
		    if (tlp) tlp -> next = tl -> next;
		    else snB -> pn -> tors = tl -> next;
		    DISPOSE (tl, sizeof(pnTorLink_t));
		}
		else tl -> tor = (BJT_t*)tA;
	    }
	    else fprintf (stderr, "subtorJoin: missing pnTorLink\n");
	}
	dsBdrB -> next = tA -> boundaries; /* add dsBdrB to tA as separate boundary */
	tA -> boundaries = dsBdrB;
added:
	dsBdrB = nextDsBdrB;
    }
    tB -> boundaries = NULL;

    /* update perimeter and surface values */

    tA -> totPerimeter += tB -> totPerimeter;
    tA -> dsPerimeter  += tB -> dsPerimeter;
    tA -> surface      += tB -> surface;

    if (tB -> xl < tA -> xl) {
	tA -> xl = tB -> xl;
	tA -> yb = tB -> yb;
    }
    else if (tB -> xl == tA -> xl && tB -> yb < tA -> yb) {
	tA -> yb = tB -> yb;
    }

    /* The gate of the tile that will stay longest in the stateruler
       is used as the gate node of the joined transistor */

    ASSERT (!tB -> gate);
    ASSERT (!tB -> bulk);

    torDel (tB);
}

void subtorCopy (tile_t *tile, tile_t *newTile)
{
    transistor_t *t;

    newTile -> tor = t = tile -> tor;
    newTile -> next_tor = t -> subs;
    t -> subs = newTile;

    inSubtor++;
    if (++currIntSubtor > maxIntSubtor) maxIntSubtor = currIntSubtor;
}

void subtorDel (tile_t *tile)
{
    transistor_t *t = tile -> tor;

    if (t -> subs == tile) {
	if (!(t -> subs = tile -> next_tor)) outTransistor (t);
    }
    else {
	tile_t *sub = t -> subs;
	while (sub -> next_tor != tile) sub = sub -> next_tor;
	sub -> next_tor = tile -> next_tor;
    }

    currIntSubtor--;
}

void portAdd (subnode_t *subn, tile_t *tile, int port)
{
    if (!subn) { ASSERT (0); return; }

    if (port == 'g')
	tile -> tor -> gate = subn;
    else
	tile -> tor -> bulk = subn;
}

void subnodeSubcontReconnect (subnode_t *subnSrc, subnode_t *subnDst, double frac)
{
    int i;
    register element_c *cap;
    register element_r *con;
    node_t *nD, *nS, *nJ, *n;

    nD = subnDst -> node;
    nS = subnSrc -> node;
    nJ = 0;

    nD -> substrCon[0] += nS -> substrCon[0] * frac;
    for (i = 0; i < resSortTabSize; i++) {
	for (con = nS -> con[i]; con; con = NEXT (con, nS)) {
	    ASSERT  (con -> type == 'S');
	    n = OTHER (con, nS);
	    if (n -> substr == 2 && n -> node_x == nD -> node_x && n -> node_y == nD -> node_y)
		nJ = n;
	    else
		elemAddRes (con -> type, nD, n, con -> val * frac, i);
	}
    }

    if (nS -> substrCap) {
	if (nS -> substrCap[0]) nD -> substrCap[0] += nS -> substrCap[0] * frac;
	for (i = 0; i < capSortTabSize; i++)
	    for (cap = nS -> cap[i]; cap; cap = NEXT (cap, nS))
		elemAddCap (nD, OTHER (cap, nS), cap -> val * frac, i, NULL);
    }

    if (nJ) nodeJoin (nD, nJ);
}

void subnodeSubcontEmpty (subnode_t *subn)
{
    int i;
    register element_c *cap;
    register element_r *con;
    node_t *node = subn -> node;

    /* The resistance to SUBSTR must be 0, because the node
       will be joined later on.
    */
    node -> substrCon[0] = 0;
    for (i = 0; i < resSortTabSize; ++i)
	for (con = node -> con[i]; con; con = NEXT (con, node)) elemDelRes (con);

    if (node -> substrCap) {
	node -> substrCap[0] = 0;
	for (i = 0; i < capSortTabSize; ++i)
	    for (cap = node -> cap[i]; cap; cap = NEXT (cap, node)) elemDelCap (cap);
    }
}

void torBoundary (transistor_t *tor, coor_t x1, coor_t y1, coor_t x2, coor_t y2, subnode_t *subn, int type)
{
    dsBoundary_t *dsBdr, *dsBdr1, *dsBdr2, *newDsBdr;
    dsCoor_t *point, *point1, *point2;

    SET_TERM (subn -> node);

    point1 = point2 = NULL;
    dsBdr1 = dsBdr2 = NULL;
    for (dsBdr = tor -> boundaries; dsBdr; dsBdr = dsBdr -> next) {
	for (point = dsBdr -> points; point; point = point -> next) {
	    if ((point -> x == x1 && point -> y == y1)
	     || (point -> x == x2 && point -> y == y2)) {
		if (dsBdr -> type == type) {
		    if (!point1) {
			point1 = point; dsBdr1 = dsBdr;
		    } else {
			point2 = point; dsBdr2 = dsBdr; break;
		    }
		}
		else warnTouchingDs (tor, point -> x, point -> y);
	    }
	}
    }

    if (point1) {
	if (point2) {
            if (dsBdr1 != dsBdr2) { /* add points from dsBdr1 to dsBdr2 */
		subnode_t *sn1, *sn2;
		sn1 = dsBdr1 -> dsCond;
		sn2 = dsBdr2 -> dsCond;
		if (sn1 -> cond -> type != sn2 -> cond -> type) {
		    warnSubnodeJoin (sn1 -> cond -> con, x1, y1, 2);
		}
		subnodeJoin (sn2, sn1);
		if (sn1 -> node != subn -> node) subnodeJoin (sn1, subn);
		subnodeDel (sn1);

		if (dsBdr2 -> points == point2) dsBdr2 -> points = point2 -> next;
		if (dsBdr1 -> points == point1) dsBdr2 -> points -> next = point1 -> next;
		else {
		    dsBdr2 -> points -> next = dsBdr1 -> points;
		    dsBdr1 -> points -> next = NULL;
		}

		/* remove dsBdr1 from tor */
		if ((dsBdr = tor -> boundaries) == dsBdr1) tor -> boundaries = dsBdr1 -> next;
		else {
		    while (dsBdr -> next != dsBdr1) dsBdr = dsBdr -> next;
		    dsBdr -> next = dsBdr1 -> next;
		}
		DISPOSE (sn1, sizeof(subnode_t));
		DISPOSE (dsBdr1, sizeof(dsBoundary_t));
	    }
	    else dsBdr1 -> points = NULL;
	    DISPOSE (point1, sizeof(dsCoor_t));
	    DISPOSE (point2, sizeof(dsCoor_t));
	}
	else { /* add new point to dsBdr1 */
	    subnode_t *sn1 = dsBdr1 -> dsCond;
	    if (sn1 -> node != subn -> node) subnodeJoin (sn1, subn);
	    if (point1 -> x == x1 && point1 -> y == y1) {
		point1 -> x = x2; point1 -> y = y2;
	    } else {
		point1 -> x = x1; point1 -> y = y1;
	    }
	}
    }
    else { /* add new boundary and new points */
	subnode_t *sn;
	newDsBdr = NEW (dsBoundary_t, 1);
	newDsBdr -> type = type;
	newDsBdr -> next = tor -> boundaries;
	tor -> boundaries = newDsBdr;
	newDsBdr -> dsCond = sn = NEW (subnode_t, 1);
	sn -> cond = subn -> cond;
	subnodeCopy (subn, sn);

	if (tor -> type -> s.tor.dsCap) {
	    if ((newDsBdr -> pn = sn -> pn)) {
		pnTorLink_t *tl = sn -> pn -> tors;
		while (tl && tl -> tor != (BJT_t*)tor) tl = tl -> next;
		if (!tl) pnTorLinkAdd (sn -> pn, (BJT_t*)tor, TORELEM);
	    }
	    else fprintf (stderr, "torBoundary: missing polnode\n");
	}
	else newDsBdr -> pn = NULL;

	newDsBdr -> points = point1 = NEW (dsCoor_t, 1);
	point1 -> x = x1;
	point1 -> y = y1;
	point1 -> next = point2 = NEW (dsCoor_t, 1);
	point2 -> x = x2;
	point2 -> y = y2;
	point2 -> next = NULL;
    }
}
