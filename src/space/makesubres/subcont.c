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

extern int makesubcap;
static DM_STREAM *dmsSubres = NULL;
static FILE *fp_subres;
static subcont_t *subcont_list_begin;
static subcont_t *subcont_list_end;
static int scGroupCnt;

#ifdef __cplusplus
  extern "C" {
#endif
Private void add2subcont_list   (subcont_t *sci);
Private void rmFromSubcont_list (subcont_t *sci);
Private void subContGroupDel (subcont_t *scInfo);
Private void subContGroupNew (subcont_t *scInfo);
#ifdef __cplusplus
  }
#endif

int inSubTerm;

void initSubstr (DM_CELL *lkey)
{
    dmsSubres = dmOpenStream (lkey, makesubcap ? "subcap" : "subres", "w");

    fp_subres = dmsSubres -> dmfp;

    inSubTerm = 0;
    scGroupCnt = 0;

    subcont_list_begin = NEW (subcont_t, 1);    /* sentinel */
    subcont_list_end   = NEW (subcont_t, 1);    /* sentinel */

    subcont_list_begin -> xl = -INF;
    subcont_list_begin -> yb = -INF;
    subcont_list_begin -> node = 0;
    subcont_list_begin -> prev = NULL;
    subcont_list_begin -> next = subcont_list_end;

    subcont_list_end -> xl = INF;
    subcont_list_end -> yb = INF;
    subcont_list_end -> node = 0;
    subcont_list_end -> prev = subcont_list_begin;
    subcont_list_end -> next = NULL;
}

void endSubstr ()
{
    ASSERT (subcont_list_begin -> next == subcont_list_end);
    dmCloseStream (dmsSubres, COMPLETE);
}

subcontRef_t * subContNew (tile_t *tile)
{
    subcontRef_t * subcR;
    subcont_t * info;

    subcR = NEW (subcontRef_t, 1);
    subcR -> node = createNode ();
    subcR -> causing_con = -1;
    subcR -> distributed = 0;
    subcR -> nextRef = NULL;
    subcR -> subcontInfo = info = NEW (subcont_t, 1);

    info -> subcontRefs = subcR;
    info -> xl = tile -> xl;
    info -> yb = tile -> bl;
    info -> nr = 1;
    info -> node = NULL;
    subContGroupNew (info);

    add2subcont_list (info);

    return (subcR);
}

void subContJoin (tile_t *tileA, tile_t *tileB)
{
    element_t *el, *el2;
    subcontRef_t *subcR, *lastR;
    subcont_t *iA, *iB;
    node_t *nA, *nB, *nC;

    iA = tileA -> subcont -> subcontInfo;
    iB = tileB -> subcont -> subcontInfo;
    if (iA == iB) return;

    if (iB -> xl < iA -> xl || (iB -> xl == iA -> xl && iB -> yb < iA -> yb)) {
	Swap (subcont_t *, iA, iB);
	nA = tileB -> subcont -> node;
	nB = tileA -> subcont -> node;
    }
    else {
	nA = tileA -> subcont -> node;
	nB = tileB -> subcont -> node;
    }

    subContGroupJoin (iA, iB);
    subContGroupDel (iB);
    rmFromSubcont_list (iB);

    ASSERT (iA -> node == NULL);
    ASSERT (iB -> node == NULL);

    nA -> substrCon += nB -> substrCon;
    for (el = nB -> con; el; el = el2) {
	nC = OTHER (el, nB);
	if (nC != nA) elemAdd (nA, nC, el -> val);
	el2 = NEXT (el, nB);
	elemDel (el);
    }
    disposeNode (nB);

    lastR = NULL;
    for (subcR = iB -> subcontRefs; subcR; subcR = subcR -> nextRef) {
	subcR -> subcontInfo = iA;
	subcR -> node        = nA;
	lastR = subcR;
    }
    ASSERT (lastR);
    lastR -> nextRef  = iA -> subcontRefs;
    iA -> subcontRefs = iB -> subcontRefs;

    DISPOSE (iB, sizeof(subcont_t));
}

void subContDel (tile_t *tile)
{
    subcontRef_t *sR;
    subcont_t *sc, *next_sc;
    element_t *con, *nxt;
    node_t *n, *on;
    int cnt;

    sc = tile -> subcont -> subcontInfo;
    sR = sc -> subcontRefs;

    /* remove subcontRef of tile from list */

    if (sR == tile -> subcont) {
	sc -> subcontRefs = sR -> nextRef;
    }
    else {
	while (sR -> nextRef != tile -> subcont) sR = sR -> nextRef;
	sR -> nextRef = sR -> nextRef -> nextRef;
    }

    if (!sc -> subcontRefs) { // last tile of substrate contact

	sc -> node = tile -> subcont -> node;
	ASSERT (sc -> node);

	sc = subcont_list_begin -> next;
	while ((n = sc -> node)) { // ready subcont

	    ASSERT (n -> sc_nr == 0);
	    n -> sc_nr = ++inSubTerm;
	    /* now n->sc_nr > 0 also denotes that the contact was outputted */
	  if (makesubcap)
	    fprintf (fp_subres, "c %d %d xl %d yb %d c %le\n",
		inSubTerm, sc -> group, sc -> xl, sc -> yb, n -> substrCon);
	  else
	    fprintf (fp_subres, "c %d %d xl %d yb %d g %le\n",
		inSubTerm, sc -> group, sc -> xl, sc -> yb, n -> substrCon);

	    cnt = 0;
	    for (con = n -> con; con; con = nxt) {
		nxt = NEXT (con, n);
		on = OTHER (con, n);
		if (on -> sc_nr > 0) { /* The neighbor contact was already outputted,
					reference that contact and output the resistor. */
		  if (makesubcap)
		    fprintf (fp_subres, "nc %d c %le\n", on -> sc_nr, con -> val);
		  else
		    fprintf (fp_subres, "nc %d g %le\n", on -> sc_nr, con -> val);

		    elemDel (con);
		    if (!on -> con) disposeNode (on);
		}
		cnt++;
	    }
	    fprintf (fp_subres, "nr_neigh %d\n", cnt);

	    if (!n -> con) disposeNode (n);

	    next_sc = sc -> next;
	    subContGroupDel (sc);
	    rmFromSubcont_list (sc);

	    DISPOSE (sc, sizeof(subcont_t));
	    sc = next_sc;
	}
    }

    DISPOSE (tile -> subcont, sizeof(subcontRef_t));
}

Private void add2subcont_list (subcont_t* sc)
{
    subcont_t * scl = subcont_list_end -> prev;
    while (scl -> xl > sc -> xl || (scl -> xl == sc -> xl && scl -> yb > sc -> yb)) {
	scl = scl -> prev;
    }
    sc -> next = scl -> next;
    sc -> prev = scl;
    scl -> next -> prev = sc;
    scl -> next = sc;
}

Private void rmFromSubcont_list (subcont_t *sc)
{
    sc -> next -> prev = sc -> prev;
    sc -> prev -> next = sc -> next;
}

Private void subContGroupDel (subcont_t *sc)
{
    if (sc -> prevGroup) sc -> prevGroup -> nextGroup = sc -> nextGroup;
    if (sc -> nextGroup) sc -> nextGroup -> prevGroup = sc -> prevGroup;
}

Private void subContGroupNew (subcont_t *sc)
{
    sc -> group = ++scGroupCnt;
    sc -> prevGroup = NULL;
    sc -> nextGroup = NULL;
}

void subContGroupJoin (subcont_t *scA, subcont_t *scB)
{
    subcont_t *sc, *scN;
    int gA = scA -> group;
    int gB = scB -> group;

    if (gA == gB) return;
    if (gB < gA) { sc = scB; scB = scA; scA = sc; gA = gB; }
    scN = scA -> nextGroup;
    sc = scB; sc -> group = gA;
    while (sc -> prevGroup) { sc = sc -> prevGroup; sc -> group = gA; }
    sc -> prevGroup = scA; scA -> nextGroup = sc;
    sc = scB;
    while (sc -> nextGroup) { sc = sc -> nextGroup; sc -> group = gA; }
    if (scN) { sc -> nextGroup = scN; scN -> prevGroup = sc; }
}
