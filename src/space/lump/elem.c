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
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/lump/define.h"
#include "src/space/lump/extern.h"
#ifdef DISPLAY
#include "src/space/X11/export.h"
#endif

#define ELEMHASH(n1,n2,size)  ((n2 -> id - n1 -> id) % size)

extern int contact_join;
static element_c * elemList = NULL;
static element_c ** capHashTab;
static element_r ** resHashTab;

static int capHT_size = 0;
static int resHT_size = 0;
static unsigned int max = 0;

void elementStatistics (FILE *fp)
{
    fprintf (fp, "overall element statistics:\n");
    fprintf (fp, "\telements allocated : %u of %d byte\n", max, (int)sizeof(element_c));
    fprintf (fp, "\tcapHT size         : %d of %d byte\n", capHT_size, (int)sizeof(element_c*));
    fprintf (fp, "\tresHT size         : %d of %d byte\n", resHT_size, (int)sizeof(element_r*));
}

#ifdef MOMENTS
double * newMoments (double *m)
{
    register int i;
    double * moments;

    moments = NEW (double, extraMoments);
    for (i = 0; i < extraMoments; i++) moments[i] = m[i];
    return moments;
}
#endif /* MOMENTS */

#define elemDisposeCap(el) el->nextHash = elemList; elemList = el
#define elemDisposeRes(el) el->nextHash = (element_r *)elemList; elemList = (element_c *)el

element_c * newElem ()
{
    register element_c *el;

    if (!(el = elemList)) {
#define FITSIZE 100
	elemList = NEW (element_c, FITSIZE);
	el = elemList + FITSIZE - 1;
	el -> nextHash = NULL;
	while (el != elemList) { --el; el -> nextHash = el + 1; }
	max += FITSIZE;
    }
    elemList = el -> nextHash;
    return el;
}

Private void enlargeElemHT (element_c ***elemHT, int *HT_size)
{
    register int i;
    register element_c *el, *el_next, **newHT, **oldHT;
    int new_size, old_size, hashval;

    old_size = *HT_size;
    new_size = (old_size == 0)? 1024*8 : old_size * 2;
    oldHT = *elemHT;
    *elemHT = newHT = NEW (element_c *, new_size);
    *HT_size = new_size;

    /* initialize new hash table */
    for (i = 0; i < new_size; i++) newHT[i] = NULL;

    /* move elements from old hash table to new hash table */
    for (i = 0; i < old_size; i++) {
	for (el = oldHT[i]; el; el = el_next) {
	    el_next = el -> nextHash;
	    hashval = ELEMHASH (el -> parentA, el -> parentB, new_size);
	    el -> nextHash = newHT[hashval];
	    newHT[hashval] = el;
	}
    }

    /* delete old hash table */
    if (old_size > 0) DISPOSE (oldHT, sizeof(element_c *) * old_size);
}

/*============================================================================*/
/*! @brief Add a circuit element between two nodes.

    This function adds a circuit element between the two nodes @c nA and
    @c nB. The type of circuit element is specified by the parameter @type,
    and can be 'C' (capacitor), 'G' (conductor) or 'S' (substrate conductor).

    The @c sort parameter designates the sort of the element at node A. Note
    that if the element is a junction capacitance (a fact which can be derived
    from the @sort parameter), then @c nA and @c nB will have different
    polarities, and the algorithm will automatically detect this case.
*//*==========================================================================*/

element_c * elemAddCap (node_t *nA, node_t *nB, double val, int sortA, double *moments)
{
    register element_c *el;
    element_c *el_nextA, *el_nextB;
    int hashval, i, sortB;

    if (nA == nB) return NULL;
 // ASSERT (nA && nB);

    if (nB == nSUB) {
	nA -> substrCap[sortA] += val;
#ifdef MOMENTS
	if (moments) {
	    if (!nA -> moments2) nA -> moments2 = newMoments (moments);
	    else for (i = 0; i < extraMoments; ++i) nA -> moments2[i] += moments[i];
	}
#endif
	return NULL;
    }

    sortB = sortA;
    if (capPolarityTab[sortA] != 'x') { /* set sort for nB */
	if (capPolarityTab[sortA] == 'p') ++sortB; else --sortB;
    }

    if (nA == nSUB) {
	nB -> substrCap[sortB] += val;
#ifdef MOMENTS
	if (moments) {
	    if (!nB -> moments2) nB -> moments2 = newMoments (moments);
	    else for (i = 0; i < extraMoments; ++i) nB -> moments2[i] += moments[i];
	}
#endif
	return NULL;
    }

    if (nA -> id > nB -> id) {
	Swap (node_t *, nA, nB);
	Swap (int, sortA, sortB);
    }

    if (nA -> cap_cnt && nB -> cap_cnt) {
	hashval = ELEMHASH (nA, nB, capHT_size);
	for (el = capHashTab[hashval]; el; el = el -> nextHash) {
	    if (el -> parentA == nA && el -> parentB == nB && el -> sort == sortA) {
		el -> val += val;
#ifdef MOMENTS
		if (moments) {
		    if (!el -> moments) el -> moments = newMoments (moments);
		    else for (i = 0; i < extraMoments; ++i) el -> moments[i] += moments[i];
		}
#endif
		return el;
	    }
	}
    }

    /* cap element not present between nA and nB */

    el = (element_c *) newElem ();
    el -> val = val;
    el -> type = 'C';
    el -> flagEl = 0;
    el -> sort = sortA;
    el -> parentA = nA;
    el -> parentB = nB;
#ifdef MOMENTS
    if (moments) el -> moments = newMoments (moments);
    else el -> moments = NULL;
#endif
    el -> prevA = NULL;
    el -> prevB = NULL;
    el -> nextA = el_nextA = nA -> cap[sortA];
    el -> nextB = el_nextB = nB -> cap[sortB];
    nA -> cap[sortA] = el;
    nB -> cap[sortB] = el;
    if (el_nextA) { if (el_nextA->parentA == nA) el_nextA->prevA = el; else el_nextA->prevB = el; }
    if (el_nextB) { if (el_nextB->parentA == nB) el_nextB->prevA = el; else el_nextB->prevB = el; }
#ifdef DISPLAY
    if (goptDrawCapacitor) drawCapacitor (nA -> node_x, nA -> node_y, nB -> node_x, nB -> node_y);
#endif
    nA -> cap_cnt++; //ASSERT (nA -> delayed != 1); // nqChange (nA);
    nB -> cap_cnt++; //ASSERT (nB -> delayed != 1); // nqChange (nB);

    if (++currIntCap > maxIntCap) { maxIntCap = currIntCap;
	if (currIntCap > capHT_size) enlargeElemHT (&capHashTab, &capHT_size);
    }
    hashval = ELEMHASH (nA, nB, capHT_size);
    el -> nextHash = capHashTab[hashval];
    capHashTab[hashval] = el;
    return el;
}

element_r * elemAddRes (int type, node_t *nA, node_t *nB, double val, int sortA)
{
    register element_r *el;
    element_r *el_nextA, *el_nextB;
    int hashval;

    if (nA == nB) return NULL;
 // ASSERT (nA && nB);
 // ASSERT (type == 'G' || type == 'S');

    if (nB == nSUB || nA == nSUB) {
	if (nA == nSUB) Swap (node_t *, nA, nB);
	ASSERT (nA != nSUB);
	nA -> substrCon[sortA] += val;
	return NULL;
    }

    if (nA -> id > nB -> id) Swap (node_t *, nA, nB);

    if (nA -> res_cnt && nB -> res_cnt) {
	hashval = ELEMHASH (nA, nB, resHT_size);
	for (el = resHashTab[hashval]; el; el = el -> nextHash) {
	    if (el -> parentA == nA && el -> parentB == nB && el -> sort == sortA) {
		if (type == 'G' && el -> type == 'S') {
		    el -> type = 'G';
		    if (Grp(nB) != Grp(nA)) (void) mergeGrps (Grp(nA), Grp(nB));
		}
		el -> val += val;
		return el;
	    }
	}
    }

    /* res element not present between nA and nB */

    el = (element_r *) newElem ();
    el -> val = val;
    el -> sort = sortA;
    el -> parentA = nA;
    el -> parentB = nB;
    el -> prevA = NULL;
    el -> prevB = NULL;
    el -> nextA = el_nextA = nA -> con[sortA];
    el -> nextB = el_nextB = nB -> con[sortA];
    nA -> con[sortA] = el;
    nB -> con[sortA] = el;
    if (el_nextA) { if (el_nextA->parentA == nA) el_nextA->prevA = el; else el_nextA->prevB = el; }
    if (el_nextB) { if (el_nextB->parentA == nB) el_nextB->prevA = el; else el_nextB->prevB = el; }

#ifdef DISPLAY
    if (goptDrawResistor) drawResistor (nA -> node_x, nA -> node_y, nB -> node_x, nB -> node_y);
#endif
    nA -> res_cnt++;
    nB -> res_cnt++;
    if (contact_join) {
	if (nA -> delayed == 1) nqChange (nA);
	if (nB -> delayed == 1) nqChange (nB);
    }
    else {
	ASSERT (nA -> delayed != 1); // nqChange (nA);
	ASSERT (nB -> delayed != 1); // nqChange (nB);
    }

    if (Grp(nB) != Grp(nA)) {
	if (type == 'G') (void) mergeGrps (Grp(nA), Grp(nB));
	el -> type = type;
    }
    else el -> type = 'G';

    if (++currIntRes > maxIntRes) { maxIntRes = currIntRes;
	if (currIntRes > resHT_size) enlargeElemHT ((element_c ***)&resHashTab, &resHT_size);
    }
    hashval = ELEMHASH (nA, nB, resHT_size);
    el -> nextHash = resHashTab[hashval];
    resHashTab[hashval] = el;
    return el;
}

element_c * findCapElement (node_t *nA, node_t *nB, int sortA)
{
    int hashval;
    register element_c *el;

    ASSERT (nA && nB);
    if (nA -> cap_cnt == 0 || nB -> cap_cnt == 0) return NULL;

    if (nA -> id > nB -> id) {
	Swap (node_t *, nA, nB);
	if (capPolarityTab[sortA] == 'p') ++sortA;
	else if (capPolarityTab[sortA] == 'n') --sortA;
    }
    hashval = ELEMHASH (nA, nB, capHT_size);
    for (el = capHashTab[hashval]; el; el = el -> nextHash) {
	if (el -> parentA == nA && el -> parentB == nB && el -> sort == sortA) return el;
    }
    return el;
}

void elemDelCap (element_c *el)
{
    register element_c *e_p, *e_n;
    int hashval, sortA;
    node_t *paA, *paB;

    sortA = el -> sort;
    paA = el -> parentA;
    paB = el -> parentB;

    e_n = el -> nextA;
    e_p = el -> prevA;
    if (e_p) { if (e_p->parentA == paA) e_p->nextA = e_n; else e_p->nextB = e_n; }
    else paA -> cap[sortA] = e_n;
    if (e_n) { if (e_n->parentA == paA) e_n->prevA = e_p; else e_n->prevB = e_p; }

    e_n = el -> nextB;
    e_p = el -> prevB;
    if (e_p) { if (e_p->parentA == paB) e_p->nextA = e_n; else e_p->nextB = e_n; }
    else {
	if (capPolarityTab[sortA] == 'p') ++sortA;
	else if (capPolarityTab[sortA] == 'n') --sortA;
	paB -> cap[sortA] = e_n;
    }
    if (e_n) { if (e_n->parentA == paB) e_n->prevA = e_p; else e_n->prevB = e_p; }

    currIntCap--;
    paA -> cap_cnt--; //ASSERT (paA -> delayed != 1); // nqChange (paA);
    paB -> cap_cnt--; //ASSERT (paB -> delayed != 1); // nqChange (paB);

#ifdef DISPLAY
    if (goptUnDrawCapacitor) undrawCapacitor (paA -> node_x, paA -> node_y, paB -> node_x, paB -> node_y);
#endif
#ifdef MOMENTS
    if (el -> moments) DISPOSE (el -> moments, sizeof(double) * extraMoments);
#endif

    hashval = ELEMHASH (paA, paB, capHT_size);
    e_n = capHashTab[hashval];
    if (e_n == el) capHashTab[hashval] = el -> nextHash;
    else {
	while (e_n && e_n -> nextHash != el) e_n = e_n -> nextHash;
	ASSERT (e_n);
	e_n -> nextHash = el -> nextHash;
    }
    elemDisposeCap (el);
}

void elemDelRes (element_r *el)
{
    register element_r *e_p, *e_n;
    int hashval;
    node_t *paA, *paB;

    paA = el -> parentA;
    paB = el -> parentB;

    e_n = el -> nextA;
    e_p = el -> prevA;
    if (e_p) { if (e_p->parentA == paA) e_p->nextA = e_n; else e_p->nextB = e_n; }
    else paA -> con[el->sort] = e_n;
    if (e_n) { if (e_n->parentA == paA) e_n->prevA = e_p; else e_n->prevB = e_p; }

    e_n = el -> nextB;
    e_p = el -> prevB;
    if (e_p) { if (e_p->parentA == paB) e_p->nextA = e_n; else e_p->nextB = e_n; }
    else paB -> con[el->sort] = e_n;
    if (e_n) { if (e_n->parentA == paB) e_n->prevA = e_p; else e_n->prevB = e_p; }

    currIntRes--;
    paA -> res_cnt--; ASSERT (paA -> delayed != 1); // nqChange (paA);
    paB -> res_cnt--; ASSERT (paB -> delayed != 1); // nqChange (paB);

#ifdef DISPLAY
    if (goptUnDrawResistor) undrawResistor (paA -> node_x, paA -> node_y, paB -> node_x, paB -> node_y);
#endif

    hashval = ELEMHASH (paA, paB, resHT_size);
    e_n = resHashTab[hashval];
    if (e_n == el) resHashTab[hashval] = el -> nextHash;
    else {
	while (e_n && e_n -> nextHash != el) e_n = e_n -> nextHash;
	ASSERT (e_n);
	e_n -> nextHash = el -> nextHash;
    }
    elemDisposeRes (el);
}

group_t * mergeGrps (group_t *grA, group_t *grB)
{
    node_t *nA, *nB, *nAlast;
    groupTileInfo_t *infoA, *infoB;
    register tileRef_t *tileR;

    if (grA -> nod_cnt < grB -> nod_cnt) Swap (group_t *, grA, grB); /* swap for efficiency reasons */

    if ((nB = grB -> nodes)) {
	nA = grA -> nodes;
	nAlast = nA -> gprev;
	nAlast -> gnext = nB;
	nA -> gprev = nB -> gprev;
	nB -> gprev = nAlast;
	do { nB -> grp = grA; } while ((nB = nB -> gnext));
    }

    grA -> nod_cnt  += grB -> nod_cnt;
    grA -> notReady += grB -> notReady;

    if (grA -> supply != grB -> supply) {
	if ((grA -> supply == 1 && grB -> supply == 2)
	    || (grA -> supply == 2 && grB -> supply == 1)) {
	    supplyShort ();
	}
	grA -> supply |= grB -> supply;
    }

    grA -> prick |= grB -> prick;

    if (optBackInfo > 1) {
	infoA = grA -> tileInfo;
	infoB = grB -> tileInfo;

	if ((tileR = infoA -> tiles)) {
	    while (tileR -> next) tileR = tileR -> next;
	    tileR -> next = infoB -> tiles;
	}
	else
	    infoA -> tiles = infoB -> tiles;
	infoB -> tiles = NULL;
    }

    if (grB -> name) groupNameAdd (grA, grB -> name);

    groupDel (grB);
    return (grA);
}
