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

extern void do_outGroup (group_t*);
Private int strmatch (char *sb, char *tb);

extern contRef_t *selResTiles, *selResTilesLast;
extern char **prickName;
extern int   *prickUsed;
extern int prickNameCnt;

extern bool_t termIsLabel;
extern int sub_caps_entry;
extern int QAG_inuse;
extern int artReduc1;
extern int inGrp;

extern int corner_line_node;
extern int joiningCon;
extern coor_t joiningX;
extern coor_t joiningY;

bool_t outputNoDangling = FALSE; /* used to prevent output */
double *el_moments = NULL;

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

    if (subnB == subnGND) {
	subnA -> node -> gndCap[sort] += val;
    }
    else if (!optCoupCap && sort == 0) { /* capPolarityTab == 'x' */
	if (subnB == subnSUB) {
	    subnA -> node -> gndCap[sort] += val;
	    return;
	}
	/* no gndCap for nodes in the same group */
	if (Grp (subnA -> node) == Grp (subnB -> node)) return;
	/* wait till readyGroup */
	elemAddCap (subnA -> node, subnB -> node, val, sort, NULL);
	inCap++;
    }
    else {
	elemAddCap (subnA -> node, subnB -> node, val, sort, NULL);
	inCap++;
    }
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

    inSubnod++;
    if (++currIntSubnod > maxIntSubnod) maxIntSubnod = currIntSubnod;

    subn -> next_pn = NULL;
    subn -> pn = NULL;

    n -> subs = subn;
    n -> grp = grp = NEW (group_t, 1);
    grp -> nodes = n;
    n -> gprev = n;
    n -> gnext = NULL;

    grp -> nod_cnt = 1;
    grp -> notReady = 1;
    grp -> supply = 0;
    grp -> prick = 0;
    grp -> flagQG = 0;

    if (optBackInfo > 1) {
	groupTileInfo_t * info;
	grp -> tileInfo = info = NEW (groupTileInfo_t, 1);
	info -> tiles = NULL;
    }
    else
	grp -> tileInfo = NULL;
    grp -> name = NULL;

    ++inGrp;
    if (++currIntGrp > maxIntGrp) maxIntGrp = currIntGrp;
}

void subnodeNew2 (subnode_t *subn, group_t *grp)
{
    node_t *n;

    subn -> next_node = NULL;
    subn -> prev_node = NULL;
    subn -> node = n = createNode ();

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
    int i, j;
    node_t *on;
    register element_c *cap;
    register element_r *con;
    subnode_t *last, *sub;
    group_t *grA, *grB;

    if (nA == nB) return (nA);

    grA = Grp (nA);
    grB = Grp (nB);

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
    else if ((nB -> term == 2 && nA -> term < 2) || ((nA -> term < 2 || nB -> term == 2) &&
       (10 * nA -> res_cnt + 10 * nA -> cap_cnt + grA -> notReady + nA -> n_n_cnt <
	10 * nB -> res_cnt + 10 * nB -> cap_cnt + grB -> notReady + nB -> n_n_cnt))) {
	/* swap nodes nA and nB for efficiency reasons */
	Swap (node_t *, nA, nB);
    }

    if (grA != grB) grA = mergeGrps (grA, grB);

    if (nB -> substr > nA -> substr) nA -> substr = nB -> substr;
    if (nB -> term > nA -> term) nA -> term = nB -> term;
    if (nB -> keep > nA -> keep) nA -> keep = nB -> keep;
    if (nB -> area > nA -> area) nA -> area = nB -> area;

    for (i = 0; i < resSortTabSize; i++) {
	nA -> substrCon[i] += nB -> substrCon[i];
	for (con = nB -> con[i]; con; con = NEXT (con, nB)) {
	    on = OTHER (con, nB);
	    if (on -> delayed) on -> delayed = 2;
	    elemAddRes (con -> type, nA, on, con -> val, i);
	    elemDelRes (con);
	    if (on -> delayed) { on -> delayed = 1;
		if (on -> res_cnt != on -> degree) nqChange (on);
	    }
	}
    }

    for (i = 0; i < capSortTabSize; i++) {
	nA -> gndCap[i] += nB -> gndCap[i];
	nA -> substrCap[i] += nB -> substrCap[i];
	for (cap = nB -> cap[i]; cap; cap = NEXT (cap, nB)) {
#ifdef MOMENTS
	    el_moments = cap -> moments;
#endif
	    on = OTHER (cap, nB);
	    elemAddCap (nA, on, cap -> val, i, el_moments);
	    elemDelCap (cap);
	    /* When all couple caps go to node nSUB and Grp(on) is ready,
	       then the group must be outputted, else it is lost. (SdeG) */
	    if (nA == nSUB && Grp(on) -> notReady == 0) do_outGroup (Grp(on));
	}
    }

    if (nA == nSUB) do_outGroup (NULL);

#ifdef MOMENTS
    if (nB -> moments) {
	if (!nA -> moments) nA -> moments = newMoments (nB -> moments);
	else for (i = 0; i < extraMoments; i++) nA -> moments[i] += nB -> moments[i];
    }
    if (nB -> moments2) {
	if (!nA -> moments2) nA -> moments2 = newMoments (nB -> moments2);
	else for (i = 0; i < extraMoments; i++) nA -> moments2[i] += nB -> moments2[i];
    }
#endif

    nodeRelJoin (nA, nB);

    last = NULL;
    for (sub = nB -> subs; sub; sub = sub -> next_node) {
	sub -> node = nA;
	last = sub;
    }
    if (last) {
	last -> next_node = nA -> subs;
	if ((sub = nA -> subs)) sub -> prev_node = last;
	nA -> subs = nB -> subs;
    }
    else ASSERT (QAG_inuse); /* called via readyGroup */

    if ((nB -> mask >= 0 || nA -> mask == nB -> mask)
        && (nB -> node_x < nA -> node_x
            || (nB -> node_x == nA -> node_x && nB -> node_y < nA -> node_y)
            || (nA -> mask < 0 && nB -> mask >= 0))) {
#ifdef DISPLAY
	if ((goptDrawResistor || goptUnDrawResistor) && nA -> res_cnt > 0) {
            /* Since the coordinates of nA change, we have to redraw the
               resistor that are connected to it. */
	    for (i = 0; i < resSortTabSize; i++) {
		for (con = nA -> con[i]; con; con = NEXT (con, nA)) {
		    on = OTHER (con, nA);
		    if (goptUnDrawResistor) undrawResistor (nA -> node_x, nA -> node_y, on -> node_x, on -> node_y);
		    if (goptDrawResistor)   drawResistor   (nB -> node_x, nB -> node_y, on -> node_x, on -> node_y);
		}
	    }
        }
        /* Also redraw capacitors. */
	if ((goptDrawCapacitor || goptUnDrawCapacitor) && nA -> cap_cnt > 0) {
	    for (i = 0; i < capSortTabSize; i++) {
		for (cap = nA -> cap[i]; cap; cap = NEXT (cap, nA)) {
		    on = OTHER (cap, nA);
		    if (goptUnDrawCapacitor) undrawCapacitor (nA -> node_x, nA -> node_y, on -> node_x, on -> node_y);
		    if (goptDrawCapacitor)   drawCapacitor   (nB -> node_x, nB -> node_y, on -> node_x, on -> node_y);
		}
	    }
        }
#endif
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

    nodeDel (nB);

    if (!QAG_inuse) { /* if not called via readyGroup */
	ASSERT (grA -> notReady >= 2 || nA -> substr == 2);
	if (sub) grA -> notReady--;
	/* Note: When nA -> subs was already NULL, don't decrement notReady
	    for nB -> subs, because readyNode is called again! */
    }

    return (nA);
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
	netEquiv_t *ne = nB -> netEq;
	while (ne -> next) ne = ne -> next;
	ne -> next = nA -> netEq;
	nA -> netEq = nB -> netEq;
	nB -> netEq = NULL;
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

void subnodeCopy2 (subnode_t *subnA, subnode_t *subnB)
{
    node_t *n;
    group_t *grp;

    if (!subnA) {
	n = createNode ();
	n -> grp = grp = NEW (group_t, 1);
	grp -> prick = 0;
	grp -> tileInfo = NEW (groupTileInfo_t, 1);
	grp -> tileInfo -> conts = NULL;
	subnB -> next_node = NULL;
    }
    else {
	n = subnA -> node;
	subnB -> next_node = n -> subs;
	n -> subs -> prev_node = subnB;
    }
    subnB -> node = n;
    n -> subs = subnB;
}

void subnodeJoin2 (subnode_t *subnA, subnode_t *subnB)
{
    contRef_t *cA, *cB, *cL;
    subnode_t *last;
    group_t *grA, *grB;
    node_t *nA, *nB;

    if ((nA = subnA -> node) == (nB = subnB -> node)) return;

    grA = Grp (nA);
    grB = Grp (nB);
    ASSERT (grA != grB);

    if (grB -> prick) grA -> prick = 1;
    if ((cB = grB -> tileInfo -> conts)) {
	if ((cA = grA -> tileInfo -> conts)) {
	    cL = cB; while (cL -> cnext) cL = cL -> cnext;
	    cL -> cnext = cA;
	}
	grA -> tileInfo -> conts = cB;
    }

    ASSERT (nA -> subs);
    ASSERT (nB -> subs);
    last = nB -> subs; last -> node = nA;
    while (last -> next_node) { last = last -> next_node; last -> node = nA; }
    last -> next_node = nA -> subs;
    nA -> subs -> prev_node = last;
    nA -> subs = nB -> subs;

    DISPOSE (grB -> tileInfo, sizeof(groupTileInfo_t));
    DISPOSE (grB, sizeof(group_t));
    disposeNode (nB);
}

void subnodeDel2 (subnode_t *subn)
{
    node_t *n = subn -> node;

    if (n -> subs == subn) n -> subs = subn -> next_node;
    else {
	subn -> prev_node -> next_node = subn -> next_node;
	if (subn -> next_node) subn -> next_node -> prev_node = subn -> prev_node;
    }

    if (!n -> subs) { /* no subnodes, ready group */
	group_t *grp = Grp (n);
	outGrp++;
	if (grp -> prick) outPrePassGrp++; /* selected */
	else {
	    contRef_t *c, *cn = grp -> tileInfo -> conts;
	    while ((c = cn)) { /* remove unselected conts */
		cn = c -> cnext;
		if (c -> prev) c -> prev -> next = c -> next;
		else {
		    ASSERT (c == selResTiles);
		    selResTiles = c -> next;
		}
		if (c -> next) c -> next -> prev = c -> prev;
		else {
		    ASSERT (c == selResTilesLast);
		    selResTilesLast = c -> prev;
		}
		DISPOSE (c, sizeof(contRef_t));
	    }
	}
	DISPOSE (grp -> tileInfo, sizeof(groupTileInfo_t));
	DISPOSE (grp, sizeof(group_t));
	disposeNode (n);
    }
}

void nameAdd (subnode_t *sn, terminal_t *t)
{
    terminal_t *t1;
    node_t * n;
    group_t * grp;
    int i;
    char *tname = t -> termName;

    if (prePass == 2) {
	for (i = 0; i < prickNameCnt; ++i) {
	    if (strmatch (prickName[i], tname)) {
		Grp (sn -> node) -> prick = 1;
		++prickUsed[i];
		return;
	    }
	}
	return;
    }

    n = sn -> node;
    if (!n -> term) n -> term = 1;
    grp = Grp (n);

    t -> done = 0; /* see enumtile.c */

    if (t -> type != tTerminal || (termIsLabel && !t -> instName)) /* groupNameAdd */
	if (!grp -> name || (grp -> name -> type == tLabel2 && t -> type != tLabel2))
	    grp -> name = t;

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

Private int strmatch (char *sb, char *tb)
{
    /* Try to match string 'sb' with (a part of) termName 'tb'.
     * The character '*' may be used at the beginning and at the
     * end of string 'sb' to denote an arbitrarily string part.
     */
    if (*sb == '*') {
	char *s, *t;
	++sb;
	while (*tb) {
	    s = sb; t = tb++;
	    while (*s && *s != '*' && *s == *t) ++s, ++t;
	    if (*s == '*' || *s == *t) return 1;
	}
    }
    else {
	while (*sb && *sb != '*' && *sb == *tb) ++sb, ++tb;
	if (*sb == '*' || *sb == *tb) return 1;
    }
    return 0;
}

void supplyShort ()
{
#ifdef MES_SUPPLYSHORT
    if (optRes)
        say ("Vdd connected to Vss/Gnd (possibly via one or more resistors).");
    else
        say ("Vdd connected to Vss/Gnd.");
    say ("\tA point on this net is mask '%s' at position %s.",
	conNr2Name (joiningCon), strCoorBrackets (joiningX, joiningY));
#endif
}

void groupNameAdd (group_t *grp, terminal_t *t)
{
    terminal_t *gt = grp -> name;

    if (!gt || (gt -> type == tLabel2 && t -> type != tLabel2))
	grp -> name = t;
    else if ( (gt -> type == tLabel2 || t -> type != tLabel2) &&
	    (t -> x < gt -> x || (t -> y < gt -> y && t -> x == gt -> x)))
	grp -> name = t;
}

void subnodeReconnect (subnode_t *subn, subnode_t *subn1, subnode_t *subn2, double val1, double val2)
{
    register int i, j;
    register element_c *cap;
    node_t *n, *on, *n1, *n2;
#ifdef MOMENTS
    double moments1[MAXMOMENT + 1];
    double moments2[MAXMOMENT + 1];
#endif
    double *m1, *m2;
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

#ifndef MOMENTS
    m1 = m2 = NULL;
#else
    if (n -> moments) {
	for (j = 0; j < extraMoments; j++) {
	    moments1[j] = n -> moments[j] * frac1;
	    moments2[j] = n -> moments[j] * frac2;
	}
	if (!n1 -> moments) n1 -> moments = newMoments (moments2);
	else for (j = 0; j < extraMoments; j++) n1 -> moments[j] += moments2[j];
	if (!n2 -> moments) n2 -> moments = newMoments (moments1);
	else for (j = 0; j < extraMoments; j++) n2 -> moments[j] += moments1[j];
    }
    if (n -> moments2) {
	for (j = 0; j < extraMoments; j++) {
	    moments1[j] = n -> moments2[j] * frac1;
	    moments2[j] = n -> moments2[j] * frac2;
	}
	if (!n1 -> moments2) n1 -> moments2 = newMoments (moments2);
	else for (j = 0; j < extraMoments; j++) n1 -> moments2[j] += moments2[j];
	if (!n2 -> moments2) n2 -> moments2 = newMoments (moments1);
	else for (j = 0; j < extraMoments; j++) n2 -> moments2[j] += moments1[j];
    }
#endif

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
#ifdef MOMENTS
	    if (cap -> moments) {
		m1 = moments1;
		m2 = moments2;
		for (j = 0; j < extraMoments; j++) {
		    val = cap -> moments[j];
		    m1[j] = val * frac1;
		    m2[j] = val * frac2;
		}
	    }
	    else m1 = m2 = NULL;
#endif
	    elemAddCap (n1, on, cap -> val * frac2, i, m2);
	    elemAddCap (n2, on, cap -> val * frac1, i, m1);
	    elemDelCap (cap);
	}
    }
}

void makeAreaNode (subnode_t *subn)
{
    areaNodes++;
    subn -> node -> area = 2;
}

void makeLineNode (subnode_t *subn)
{
    if (subn -> node -> term) return;
    equiLines++;
    subn -> node -> area = 1;
    if (corner_line_node) subn -> node -> term = 2;
}

void subnodeDel (subnode_t *subn)
{
    node_t    * n = subn -> node;
    polnode_t *pn = subn -> pn;

    /* remove the subnode outof the subnode-list linked to its
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

    /* Remove subnode from the list of subnodes
     * for its node.
     */
    if (n -> subs == subn) {
	n -> subs = subn -> next_node;
    }
    else {
	ASSERT (n -> subs);
	subn -> prev_node -> next_node = subn -> next_node;
	if (subn -> next_node) subn -> next_node -> prev_node = subn -> prev_node;
    }

    /* set subn -> node to NULL to ensure that that a reference via
     * subn to node can no longer occur; if it does occur it's a bug
     */
    subn -> node = NULL;

    currIntSubnod--;

    /* If there are no subnodes left,
     * this node is ready and can possibly be eliminated.
     * Otherwise, we are done.
     */
    if (!n -> subs) {
	if (outputNoDangling) {
	    group_t *grp = Grp (n);

	    while (n -> pols) nodeLinkDel (n -> pols);

	    nodeDel (n);
	    grp -> notReady--;
	    ASSERT (grp -> notReady > 0);
	    return;
	}
	readyNode (n);
    }
}

int subnodesPrint (node_t *n)
{
    int nr = 0;
    subnode_t *subn = n -> subs;

    while (subn) {
	fprintf (stderr, "%p ", subn);
	nr++;
	subn = subn -> next_node;
    }
    fprintf (stderr, "\n");
    return nr;
}

/*============================================================================*/
/*! @brief Function called when a node is "ready", meaning that it can be eliminated.

    This function is called when a node is "ready" to be examined for
    elimination.
*//*==========================================================================*/

void readyNode (node_t *n)
{
    int i;
    group_t *grp = Grp (n);

    /* Decrease the node ref count of the group:
	(a) for each node that is ready (!n -> subs),
	(b) for each node tor link (port).
     */
    grp -> notReady--;

    ASSERT (!n -> subs);

    ASSERT (!n -> delayed);

    /* I wonder why this test is not done before entering this routine.
       But this method already exists for some time, and always worked so
       far.  Moreover, maybe it should be exactly in this way (AvG).
    */
 // if (n -> pols) return; /* BIPOLAR */

    if (n -> area) areaNodesTotal++;

    /* If the group is ready, we are done,
     * node <n> is possibly eliminated in readyGroup.
     */
    if (grp -> notReady == 0) { readyGroup (n); return; }

    if (n -> term) return;

    i = testRCelim (n);

    /* Don't delay nodes with the 'term' or 'keep' flag.
     */
    if (n -> keep || n -> area == 2) return;

    if (n -> area && artReduc) {
	if (!(n -> area == 1 && n -> res_cnt < 3 && artReduc1)) return;
    }
    nqDelayElim (n, i);
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
    t -> instName = NULL;
    t -> gate = NULL;
    t -> bulk = NULL;
    t -> totPerimeter = 0;
    t -> dsPerimeter = 0;
    t -> surface = 0;
    t -> tor_nr = 0;
    t -> boundaries = NULL;
    if (optBackInfo > 1) {
	t -> tileInfo = NEW (transTileInfo_t, 1);
	t -> tileInfo -> tiles = NULL;
    }
    else
	t -> tileInfo = NULL;

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

			    if (snA -> pn && tA -> type -> s.tor.dsCap) { /* BIPOLAR */
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
	if (snB -> pn && tA -> type -> s.tor.dsCap) { /* BIPOLAR */
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

    if (optBackInfo > 1) {
	if ((tileR = tA -> tileInfo -> tiles)) {
	    while (tileR -> next) tileR = tileR -> next;
	    tileR -> next = tB -> tileInfo -> tiles;
	}
	else
	    tA -> tileInfo -> tiles = tB -> tileInfo -> tiles;

	DISPOSE (tB -> tileInfo, sizeof(transTileInfo_t));
    }

    if (tB -> instName) {
	if (!tA -> instName) tA -> instName = tB -> instName;
	else {
	    say ("%s\n   %s and %s, second name not used",
		"warning: two instance names specified for transistor:",
		tA -> instName, tB -> instName);
	}
    }

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

void subnodeTile (subnode_t *subn, tile_t *tile, int cx)
{
    group_t * grp = Grp (subn -> node);
    tileRef_t * tileref;

    tileref = NEW (tileRef_t, 1);
    tileref -> tile = tile -> cnt;
    tileref -> cx = cx;
    tileref -> color = tile -> color;
    tileref -> xl = tile -> xl;
    tileref -> xr = tile -> xr;
    tileref -> bl = tile -> bl;
    tileref -> br = tile -> br;
    tileref -> tl = tile -> tl;
    tileref -> tr = tile -> tr;
    tileref -> next = grp -> tileInfo -> tiles;
    grp -> tileInfo -> tiles = tileref;
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

    if (!subn -> node -> term) subn -> node -> term = 1;

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
	    if (sn1 -> node != subn -> node) {
		subnodeJoin (sn1, subn);
	    }
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

	if (tor -> type -> s.tor.dsCap) { /* BIPOLAR */
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

void subtorTile (tile_t *tile, int cx)
{
    tileRef_t *tileref = NEW (tileRef_t, 1);
    tileref -> tile = tile -> cnt;
    tileref -> cx = cx;
    tileref -> color = tile -> color;
    tileref -> xl = tile -> xl;
    tileref -> xr = tile -> xr;
    tileref -> bl = tile -> bl;
    tileref -> br = tile -> br;
    tileref -> tl = tile -> tl;
    tileref -> tr = tile -> tr;
    tileref -> next = tile -> tor -> tileInfo -> tiles;
    tile -> tor -> tileInfo -> tiles = tileref;
}
