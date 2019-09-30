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
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/lump/define.h"
#include "src/space/lump/extern.h"

#define ELEMHASH(n1,n2,size)  ((n2 -> id - n1 -> id) % size)

static element_c * elemList = NULL;
static element_c ** capHashTab;
static element_r ** resHashTab;

static int do_cap_enlarge = 1;
static int do_res_enlarge = 1;
static int capHT_size = 0;
static int resHT_size = 0;
static unsigned int max = 0;

extern DM_STREAM *dmsNetCD;
extern int debug_hashtab, debug_hashtab2;
extern int cap_hashsize, res_hashsize;
extern int progress_mem;

#ifdef __cplusplus
  extern "C" {
#endif

extern element_r * findResElement (node_t *nA, node_t *nB, int sortA);
extern void catchAlarm ();

#ifdef __cplusplus
  }
#endif

void elementStatistics (FILE *fp)
{
    extern int maxNeqv;
    fprintf (fp, "overall element statistics:\n");
    fprintf (fp, "\telements allocated : %u of %d byte\n", max, (int)sizeof(element_c));
    fprintf (fp, "\tnetEq's  allocated : %u of %d byte\n", maxNeqv, (int)sizeof(netEquiv_t));
    fprintf (fp, "\tcapHT size         : %d of %d byte\n", capHT_size, (int)sizeof(element_c*));
    fprintf (fp, "\tresHT size         : %d of %d byte\n", resHT_size, (int)sizeof(element_r*));
}

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

Private void enlargeElemHT (int type)
{
    register int i;
    register element_c *el, *el_next, **newHT, **oldHT;
    int new_size, old_size, hashval;

    if (type == CAP) {
	oldHT = capHashTab;
	old_size = capHT_size;
	if (cap_hashsize > 0) {
	    new_size = cap_hashsize;
	    do_cap_enlarge = 0;
	}
	else new_size = (old_size == 0)? 1024*8 : old_size * 2;
	capHT_size = new_size;
	capHashTab = newHT = NEW (element_c *, new_size);
    }
    else {
	oldHT = (element_c **)resHashTab;
	old_size = resHT_size;
	if (res_hashsize > 0) {
	    new_size = res_hashsize;
	    do_res_enlarge = 0;
	}
	else new_size = (old_size == 0)? 1024*8 : old_size * 2;
	resHT_size = new_size;
	newHT = NEW (element_c *, new_size);
	resHashTab = (element_r **)newHT;
    }
    if (debug_hashtab) {
	fprintf (stderr, "enlargeElemHT: %sHashTab: new_size=%d\n", type == CAP ? "cap" : "res", new_size);
    }
    if (progress_mem > 0) catchAlarm ();

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

static int prev_cnt = 10;
static int prev_mcnt = 10;

element_c * elemAddCap (node_t *nA, node_t *nB, double val, int sortA, double *moments)
{
    register element_c *el;
    element_c *el_nextA, *el_nextB;
    int hashval, sortB, cnt;

    if (nA == nB) return NULL;
 // ASSERT (nA && nB);

    if (nB == nSUB) {
	nA -> substrCap[sortA] += val;
	return NULL;
    }

    sortB = sortA;
    if (capPolarityTab[sortA] != 'x') { /* set sort for nB */
	if (capPolarityTab[sortA] == 'p') ++sortB; else --sortB;
    }

    if (nA == nSUB) {
	nB -> substrCap[sortB] += val;
	return NULL;
    }

    if (nA -> id > nB -> id) {
	Swap (node_t *, nA, nB);
	Swap (int, sortA, sortB);
    }

    cnt = 0;
    if (nA -> cap_cnt && nB -> cap_cnt) {
	hashval = ELEMHASH (nA, nB, capHT_size);
	for (el = capHashTab[hashval]; el; el = el -> nextHash) {
	    ++cnt;
	    if (el -> parentA == nA && el -> parentB == nB && el -> sort == sortA) {
		el -> val += val;
		if (debug_hashtab2 && cnt > prev_mcnt) { prev_mcnt = cnt;
		    fprintf (stderr, "elemAddCap: match at %d\n", cnt);
		}
		return el;
	    }
	}
    }
    if (debug_hashtab2 && cnt > prev_cnt) { prev_cnt = cnt;
	fprintf (stderr, "elemAddCap: no match at %d\n", cnt);
    }

    /* cap element not present between nA and nB */

    el = (element_c *) newElem ();
    el -> val = val;
    el -> type = 'C';
    el -> sort = sortA;
    el -> parentA = nA;
    el -> parentB = nB;
    el -> prevA = NULL;
    el -> prevB = NULL;
    el -> nextA = el_nextA = nA -> cap[sortA];
    el -> nextB = el_nextB = nB -> cap[sortB];
    nA -> cap[sortA] = el;
    nB -> cap[sortB] = el;
    if (el_nextA) { if (el_nextA->parentA == nA) el_nextA->prevA = el; else el_nextA->prevB = el; }
    if (el_nextB) { if (el_nextB->parentA == nB) el_nextB->prevA = el; else el_nextB->prevB = el; }
    nA -> cap_cnt++;
    nB -> cap_cnt++;

    if (++currIntCap > maxIntCap) { maxIntCap = currIntCap;
	if (do_cap_enlarge && currIntCap > capHT_size) enlargeElemHT (CAP);
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

    if (nA -> con[sortA] && nB -> con[sortA]) {
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

    nA -> res_cnt++;
    nB -> res_cnt++;

    if (Grp(nB) != Grp(nA)) {
	if (type == 'G') (void) mergeGrps (Grp(nA), Grp(nB));
	el -> type = type;
    }
    else {
	el -> type = 'G';
    }

    if (++currIntRes > maxIntRes) { maxIntRes = currIntRes;
	if (do_res_enlarge && currIntRes > resHT_size) enlargeElemHT (RES);
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

element_r * findResElement (node_t *nA, node_t *nB, int sortA)
{
    register element_r *el;

    if (nA -> id > nB -> id) Swap (node_t *, nA, nB);

    for (el = resHashTab[ ELEMHASH (nA, nB, resHT_size) ]; el; el = el -> nextHash) {
	if (el -> parentA == nA && el -> parentB == nB && el -> sort == sortA) break;
    }
    return el;
}

void elemDelCap (element_c *el)
{
    register element_c *e_p, *e_n;
    int hashval, sortA;
    node_t *nA, *nB;

    sortA = el -> sort;
    nA = el -> parentA;
    nB = el -> parentB;

    e_n = el -> nextA;
    e_p = el -> prevA;
    if (e_p) { if (e_p->parentA == nA) e_p->nextA = e_n; else e_p->nextB = e_n; }
    else nA -> cap[sortA] = e_n;
    if (e_n) { if (e_n->parentA == nA) e_n->prevA = e_p; else e_n->prevB = e_p; }

    e_n = el -> nextB;
    e_p = el -> prevB;
    if (e_p) { if (e_p->parentA == nB) e_p->nextA = e_n; else e_p->nextB = e_n; }
    else {
	if (capPolarityTab[sortA] == 'p') ++sortA;
	else if (capPolarityTab[sortA] == 'n') --sortA;
	nB -> cap[sortA] = e_n;
    }
    if (e_n) { if (e_n->parentA == nB) e_n->prevA = e_p; else e_n->prevB = e_p; }

    currIntCap--;
    nA -> cap_cnt--;
    nB -> cap_cnt--;

    hashval = ELEMHASH (nA, nB, capHT_size);
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
    node_t *nA, *nB;

    nA = el -> parentA;
    nB = el -> parentB;

    e_n = el -> nextA;
    e_p = el -> prevA;
    if (e_p) { if (e_p->parentA == nA) e_p->nextA = e_n; else e_p->nextB = e_n; }
    else nA -> con[el->sort] = e_n;
    if (e_n) { if (e_n->parentA == nA) e_n->prevA = e_p; else e_n->prevB = e_p; }

    e_n = el -> nextB;
    e_p = el -> prevB;
    if (e_p) { if (e_p->parentA == nB) e_p->nextA = e_n; else e_p->nextB = e_n; }
    else nB -> con[el->sort] = e_n;
    if (e_n) { if (e_n->parentA == nB) e_n->prevA = e_p; else e_n->prevB = e_p; }

    currIntRes--;
    nA -> res_cnt--;
    nB -> res_cnt--;

    hashval = ELEMHASH (nA, nB, resHT_size);
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

    if (!grA -> grp_nr) grA -> grp_nr = grB -> grp_nr;
    else if (grB -> grp_nr) {
	ASSERT (grB -> grp_nr != grA -> grp_nr);
	if (grB -> grp_nr < grA -> grp_nr) {
	    if (dmsNetCD) fprintf (dmsNetCD -> dmfp, "%d %d\n", grB -> grp_nr, grA -> grp_nr);
	    grA -> grp_nr = grB -> grp_nr;
	}
	else
	    if (dmsNetCD) fprintf (dmsNetCD -> dmfp, "%d %d\n", grA -> grp_nr, grB -> grp_nr);
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

    groupDel (grB);
    return (grA);
}
