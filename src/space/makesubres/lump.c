/*
 * ISC License
 *
 * Copyright (C) 2004-2018 by
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

#include "src/space/makesubres/define.h"
#include "src/space/makesubres/extern.h"

static node_t * free_node = NULL;
static int elemCnt, freedElemCnt, maxElems;
static int nodeCnt, freedNodeCnt, maxNodes;
static int currIntRes;

static element_t * elemList = NULL;
static element_t ** resHashTab;
static int resHT_size = 0;

int tileCnt, tileConCnt;

void initLump ()
{
    elemCnt = freedElemCnt = maxElems = 0;
    nodeCnt = freedNodeCnt = maxNodes = 0;
    tileCnt = tileConCnt = 0;
}

void endLump (char *cellname)
{
    ASSERT (!currIntRes);

    if (optVerbose) {
	FILE * fp = stdout;

	fprintf (fp, "\noverall substrate statistics:\n");
	fprintf (fp, "\ttotal num. of tiles : %d\n", tileCnt);
	fprintf (fp, "\tsubstrate tiles     : %d\n", tileConCnt);
	fprintf (fp, "\tsubstrate terminals : %d\n", inSubTerm);

	if (!optInfo) return;
	fprintf (fp, "overall node statistics:\n");
	fprintf (fp, "\tnodes allocated     : %d\n", nodeCnt);
	fprintf (fp, "\tnodes freed         : %d\n", freedNodeCnt);
	fprintf (fp, "\tmax nodes in core   : %d\n", maxNodes);

	fprintf (fp, "overall element statistics:\n");
	fprintf (fp, "\telements allocated  : %d\n", elemCnt);
	fprintf (fp, "\telements freed      : %d\n", freedElemCnt);
	fprintf (fp, "\tmax elements in core: %d\n", maxElems);
	fprintf (fp, "\telem hashtable size : %d\n", resHT_size);

	scanPrintInfo (fp);
	sub3dStatistics (fp);
    }
}

node_t * createNode ()
{
    node_t * n;

    if (free_node) { n = free_node; free_node = free_node -> next_n; }
    else n = NEW (node_t, 1);

    n -> id = nodeCnt++;
    n -> sc_nr = 0;
    n -> con = 0;
    n -> substrCon = 0.0;

    if (nodeCnt - freedNodeCnt > maxNodes) maxNodes = nodeCnt - freedNodeCnt;

    return (n);
}

void disposeNode (node_t *n)
{
    ++freedNodeCnt;
    n -> next_n = free_node;
    free_node = n;
}

#define ELEMHASH(n1,n2) ((n2 -> id - n1 -> id) % resHT_size)

Private void enlargeElemHT ()
{
    register int i;
    register element_t *el;
    element_t *next;
    element_t **oldHT = resHashTab;
    int old_size = resHT_size;
    int hashval;

    if (resHT_size == 0) {
	resHT_size = 100;
	resHashTab = NEW (element_t *, resHT_size);
    }
    else {
	resHT_size *= 2;
	resHashTab = NEW (element_t *, resHT_size);
    }

    /* initialize new hash table */
    for (i = 0; i < resHT_size; i++) resHashTab[i] = NULL;

    /* move elements from old hash table to new hash table */
    for (i = 0; i < old_size; i++) {
	for (el = oldHT[i]; el; el = next) {
	    next = el -> nextHash;
	    hashval = ELEMHASH (el -> parentA, el -> parentB);
	    el -> nextHash = resHashTab[hashval];
	    resHashTab[hashval] = el;
	}
    }

    if (old_size > 0) DISPOSE (oldHT, sizeof(element_t *) * old_size);
}

void elemAdd (node_t *nA, node_t *nB, double val)
{
    element_t *el, *next;
    int hashval;

    if (nA -> id > nB -> id) Swap (node_t *, nA, nB);

    if (currIntRes > 0) {
	hashval = ELEMHASH (nA, nB);
	for (el = resHashTab[hashval]; el; el = el -> nextHash) {
	    if (el -> parentA == nA && el -> parentB == nB) {
		el -> val += val;
		return;
	    }
	}
    }

    if (!(el = elemList)) {
#define FITSIZE 100
	elemList = NEW (element_t, FITSIZE);
	el = elemList + FITSIZE - 1;
	el -> nextA = NULL;
	while (el != elemList) {
	    --el;
	    el -> nextA = el + 1;
	}
    }
    elemList = el -> nextA;

    el -> val = val;
    el -> parentA = nA;
    el -> parentB = nB;

    next = nA -> con;
    nA -> con = el;
    el -> nextA = next;
    el -> prevA = NULL;
    if (next) *APREV (next, nA) = el;

    next = nB -> con;
    nB -> con = el;
    el -> nextB = next;
    el -> prevB = NULL;
    if (next) *APREV (next, nB) = el;

    if (++elemCnt - freedElemCnt > maxElems) maxElems = elemCnt - freedElemCnt;
    ++currIntRes;
    if (2 * currIntRes > resHT_size) enlargeElemHT ();
    hashval = ELEMHASH (nA, nB);
    el -> nextHash = resHashTab[hashval];
    resHashTab[hashval] = el;
}

void elemDel (element_t *el)
{
    element_t *e_p, *e_n;
    int hashval;
    node_t *nA, *nB;

    nA = el -> parentA;
    nB = el -> parentB;

    e_n = el -> nextA;
    e_p = el -> prevA;
    if (e_p) *ANEXT (e_p, nA) = e_n;
    else nA -> con = e_n;
    if (e_n) *APREV (e_n, nA) = e_p;

    e_n = el -> nextB;
    e_p = el -> prevB;
    if (e_p) *ANEXT (e_p, nB) = e_n;
    else nB -> con = e_n;
    if (e_n) *APREV (e_n, nB) = e_p;

    ++freedElemCnt;
    currIntRes--;
    hashval = ELEMHASH (nA, nB);

    e_p = NULL;
    for (e_n = resHashTab[hashval]; e_n && e_n != el; e_n = e_n -> nextHash) e_p = e_n;
    ASSERT (e_n == el);

    if (e_p) e_p -> nextHash = el -> nextHash;
    else resHashTab[hashval] = el -> nextHash;

    // elemDispose
    el -> nextA = elemList;
    elemList = el;
}
