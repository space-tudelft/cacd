/*
 * ISC License
 *
 * Copyright (C) 1995-2018 by
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
#include <math.h>
#include <string.h>
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/lump/define.h"
#include "src/space/lump/export.h"
#include "src/space/extract/export.h"
#include "src/space/substr/export.h"
#include "src/space/X11/export.h"
#include "src/space/bipolar/export.h"

extern double sub_rc_const;
extern int add_sub_caps;
extern bool_t prePass1;
extern bool_t optCoupCap;
extern bool_t optSubRes;
extern bool_t optEstimate3D;
extern bool_t elim_sub_con;
extern bool_t eliminateSubstrNode;
extern bool_t omit_self_sub_res; /* default 'off' */
extern int hasBipoElem;
extern int inSubTerm;

static DM_STREAM *dmsContPos = NULL;
static DM_STREAM *dmsSubRes = NULL;
static DM_STREAM *dmsSubCap = NULL;
static int readcap;
static int next_nr, next_grp;
static int Next_nr, Next_grp;
static coor_t next_xl, next_yb;
static coor_t Next_xl, Next_yb;
static const coor_t Inf = INF;
static double val1;
static subcont_t *subcont_list_begin;
static subcont_t *subcont_list_end;
static int scGroupCnt;
static int sub_param_message;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void add2subcont_list (subcont_t *sci);
Private void rmFromSubcont_list (subcont_t *sci);
Private void subContAssignRes (subcont_t *sc, subnode_t *sn);
Private void subContGroupDel (subcont_t *scInfo);
Private void subContGroupNew (subcont_t *scInfo);
#ifdef __cplusplus
  }
#endif

void initSubstr (DM_CELL *lkey)
{
    int i;

    if (prePass1) {
	if (optSubRes)
	    dmsSubRes = dmOpenStream (lkey, "subres", "w");
	else
	    dmsContPos = dmOpenStream (lkey, "cont_pos", "w");
    }
    else {
	dmsSubRes = dmOpenStream (lkey, "subres", "r");
	readcap = 0;
	if ((i = fscanf (dmsSubRes -> dmfp, "%*s %d %d %*s %d %*s %d %*s %le",
		&next_nr, &next_grp, &next_xl, &next_yb, &val1)) < 1) next_xl = Inf;
	else {
	    ASSERT (i == 5);
	    if (getc (dmsSubRes -> dmfp) != '\n') readcap = 1;
	}

	if (add_sub_caps > 1) {
	    if (!readcap) dmsSubCap = dmOpenStream (lkey, "subcap", "r");
	}
	else if (add_sub_caps) {
	    ASSERT (sub_rc_const > 0);
	}

	if (dmsSubCap) {
	    if (fscanf (dmsSubCap -> dmfp, "%*s %d %d %*s %d %*s %d",
		&Next_nr, &Next_grp, &Next_xl, &Next_yb) < 1) Next_xl = Inf;
	    else ASSERT (Next_nr == next_nr && Next_yb == next_yb);
	    ASSERT (Next_xl == next_xl);
	}

	sub_param_message = 0;
    }

    inSubTerm = 0;
    scGroupCnt = 0;

    subcont_list_begin = NEW (subcont_t, 1);    /* sentinel */
    subcont_list_end   = NEW (subcont_t, 1);    /* sentinel */

    subcont_list_begin -> xl = -Inf;
    subcont_list_begin -> yb = -Inf;
    subcont_list_begin -> ready = 0;
    subcont_list_begin -> prev = NULL;
    subcont_list_begin -> next = subcont_list_end;

    subcont_list_end -> xl = Inf;
    subcont_list_end -> yb = Inf;
    subcont_list_end -> ready = 0;
    subcont_list_end -> prev = subcont_list_begin;
    subcont_list_end -> next = NULL;
}

void endSubstr ()
{
    if (subcont_list_begin -> next != subcont_list_end) {
        if (!prePass1)
	    say ("%s\n%s\n",
		"Inconsistent substrate terminal subdivision during different",
		"         extraction passes; expecting wrong results!");
	else ASSERT (0);
    }

    if (!prePass1) ASSERT (next_xl == Inf);

#define dmCLOSE(x) if (x) dmCloseStream (x, COMPLETE), x = NULL
    dmCLOSE (dmsContPos);
    dmCLOSE (dmsSubRes);
    dmCLOSE (dmsSubCap);
}

subcontRef_t * subContNew (tile_t *tile)
{
    subcontRef_t *subcR;
    subcont_t *info;
    subnode_t *sn = NULL;

    subcR = NEW (subcontRef_t, 1);

    if (optSubRes || !prePass1) {
	sn = NEW (subnode_t, 1);
	subnodeNew (sn);
	sn -> node -> mask = -1;
	sn -> node -> node_x = tile -> xl;
	sn -> node -> node_y = tile -> bl;
	sn -> node -> substr = 1;
	if (prePass1 || !elim_sub_con) sn -> node -> term = 2;
	if (hasBipoElem) polnodeAdd (sn, -2, 'a');
    }

    subcR -> subn = sn;
    subcR -> causing_con = -1;
    subcR -> distributed = 0;
    subcR -> nextRef = NULL;
    subcR -> subcontInfo = info = NEW (subcont_t, 1);
    info -> subcontRefs = subcR;
    info -> xl = tile -> xl;
    info -> yb = tile -> bl;

    info -> nr = 0;     /* undefined */
    info -> ready = 0;

    info -> subn = NULL;
    info -> subnTL = NULL;
    info -> subnTR = NULL;
    info -> subnBR = NULL;
    info -> subnBL = NULL;

    add2subcont_list (info);

    if (prePass1) {
	info -> area = 0;
	info -> perimeter = 0;
	subContGroupNew (info);
    }
    else if (tile -> xl == next_xl && tile -> bl == next_yb) {
	ASSERT (next_nr != 0);
	info -> nr = next_nr;
	info -> group = (subcontGroup_t *)(long)next_grp;
	subContAssignRes (info, sn);
    }

    return (subcR);
}

Private void subContAssignRes (subcont_t *sc, subnode_t *sn)
{
    double val2;
    int i, j, cnt;
    char buf[132];
    FILE *dmfp;
    subcont_t *nc;

    ASSERT (sn);

    dmfp = dmsSubRes -> dmfp;

    if (readcap && fscanf (dmfp, "%*s %le", &val2) != 1) ASSERT (0);
    if (dmsSubCap && fscanf (dmsSubCap -> dmfp, "%*s %le", &val2) != 1) ASSERT (0);

    if (!eliminateSubstrNode) {
	if (val1) conAddSUB (sn, val1);
	else {
	    if (optEstimate3D && optSubRes) conAddSUB (sn, 1e99);
	    else if (!(sub_param_message & 1)) {
		sub_param_message |= 1;
		say ("found zero conductance to substrate node in subres file");
	    }
	}
	if (add_sub_caps > 1) {
	    if (val2) capAddSUB (sn, val2);
	    else if (!(sub_param_message & 4)) {
		sub_param_message |= 4;
		say ("found zero capacitance to substrate node in %s file", dmsSubCap ? "subcap" : "subres");
	    }
	}
	else if (add_sub_caps && val1) {
	    capAddSUB (sn, val1 * sub_rc_const);
	}
    }

    inSubTerm++;

    cnt = 0;
    while (fscanf (dmfp, "%s", buf) > 0 && strsame (buf, "nc")) {

	if (fscanf (dmfp, "%d %*s %le", &i, &val1) != 2) ASSERT (0);
	if (readcap && fscanf (dmfp, "%*s %le", &val2) != 1) ASSERT (0);
	if (dmsSubCap) {
	    if (fscanf (dmsSubCap -> dmfp, "%s %d %*s %le", buf, &j, &val2) != 3) ASSERT (0);
	    ASSERT (strsame (buf, "nc"));
	    ASSERT (i == j);
	}

	/* search neighbor substrate contact terminal */
	nc = subcont_list_begin -> next;
	while (nc != subcont_list_end && nc -> nr != i) nc = nc -> next;
	ASSERT (nc -> nr == i);

	if (val1 == 0) {
	    if (!(sub_param_message & 2)) {
		sub_param_message |= 2;
		say ("found zero conductance to neighbor node in subres file");
	    }
	}
	else if (!omit_self_sub_res || nc -> group != sc -> group) {
	    if (add_sub_caps > 1 && !val2 && !(sub_param_message & 8)) {
		sub_param_message |= 8;
		say ("found zero capacitance to neighbor node in %s file", dmsSubCap ? "subcap" : "subres");
	    }
	    if (nc -> subnTL) {
		val1 /= 4;
		conAddS (sn, nc -> subnTL, val1);
		conAddS (sn, nc -> subnTR, val1);
		conAddS (sn, nc -> subnBL, val1);
		conAddS (sn, nc -> subnBR, val1);

		if (add_sub_caps > 1) val2 /= 4;
		else if (add_sub_caps) val2 = val1 * sub_rc_const;
		else val2 = 0;
		if (val2) {
		    capAddS (sn, nc -> subnTL, val2);
		    capAddS (sn, nc -> subnTR, val2);
		    capAddS (sn, nc -> subnBL, val2);
		    capAddS (sn, nc -> subnBR, val2);
		}
	    }
	    else {
		subnode_t *nc_subn;
		if (nc -> subcontRefs)
		    nc_subn = nc -> subcontRefs -> subn;
		else
		    nc_subn = nc -> subn;
		ASSERT (nc_subn);
		conAddS (sn, nc_subn, val1);

		if (add_sub_caps > 1) ;
		else if (add_sub_caps) val2 = val1 * sub_rc_const;
		else val2 = 0;
		if (val2) capAddS (sn, nc_subn, val2);
	    }
	}
	nc -> rest_neigh_cnt--;
	cnt++;

#ifdef DISPLAY
	drawSubResistor (sc -> xl, sc -> yb, nc -> xl, nc -> yb);
#endif
    }

    ASSERT (strsame (buf, "nr_neigh"));
    if (fscanf (dmfp, "%d", &sc -> rest_neigh_cnt) != 1) ASSERT (0);
    sc -> rest_neigh_cnt -= cnt;

    if ((i = fscanf (dmfp, "%*s %d %d %*s %d %*s %d %*s %le",
	&next_nr, &next_grp, &next_xl, &next_yb, &val1)) < 1) next_xl = Inf;
    else ASSERT (i == 5);

    if (dmsSubCap) {
	if (fscanf (dmsSubCap -> dmfp, "%s %d", buf, &j) != 2) ASSERT (0);
	ASSERT (strsame (buf, "nr_neigh"));
	if (fscanf (dmsSubCap -> dmfp, "%*s %d %d %*s %d %*s %d",
	    &Next_nr, &Next_grp, &Next_xl, &Next_yb) < 1) Next_xl = Inf;
	else ASSERT (Next_nr == next_nr && Next_yb == next_yb);
	ASSERT (Next_xl == next_xl);
    }
}

void subContJoin (tile_t *tileA, tile_t *tileB)
{
    subcontRef_t *subcR, *lastR;
    subcont_t *iA, *iB, *iC;

    iA = tileA -> subcont -> subcontInfo;
    iB = tileB -> subcont -> subcontInfo;
    if (iA == iB) return;

    if (iB -> xl < iA -> xl
	|| (iB -> xl == iA -> xl && iB -> yb < iA -> yb)) { /* swap */
	iC = iA; iA = iB; iB = iC;
    }

    if (prePass1) {
	iA -> area      += iB -> area;
	iA -> perimeter += iB -> perimeter;
	subContGroupJoin (iA, iB);
	subContGroupDel (iB);
    }

    /* remove tileB -> subcont -> subcontInfo from subcont_list */

    rmFromSubcont_list (iB);

    /* Add subcontInfo of tileB to subcontInfo of tileA and let the
       members subcontInfo of tileB -> subcont -> subcontInfo -> subcontRefs
       point to subcontInfo of tileA.
    */

    ASSERT (iA -> subn == NULL);
    ASSERT (iB -> subn == NULL);
    ASSERT (!iB -> subnTL);

    if (tileB -> subcont -> subn) {
	subnodeJoin (tileA -> subcont -> subn, tileB -> subcont -> subn);
    }

    lastR = NULL;
    for (subcR = iB -> subcontRefs; subcR; subcR = subcR -> nextRef) {
	subcR -> subcontInfo = iA;
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
    subnode_t *sn;

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

    if (!sc -> subcontRefs) { /* last tile of substrate terminal */

        sc -> ready = 1;
	sc -> subn = tile -> subcont -> subn;

	/* Delete all the substrate terminals that are ready.
	 * During prePass1, the substrate terminal information
	 * will then be written to disk.
	 * During all passes, connected subnodes will be deleted,
	 * resulting in the elimination or outputting of attached nodes.
	 * The group information may not be completely ready (the group
	 * of the substrate terminal may still be merged with another
	 * group), but this doesn't matter much for our purpose.
	 */

	sc = subcont_list_begin -> next;
	while (sc -> ready && (prePass1 || sc -> rest_neigh_cnt == 0)) {

	    if (prePass1) { /* write to disk */

		++inSubTerm;

		if (optSubRes) {
		    element_r *con;
		    node_t *n;
		    int cnt, nr;

		    ASSERT (sc -> subn);
		    n = sc -> subn -> node;
		    ASSERT (n -> degree == 0);
		    n -> degree = inSubTerm;
		    /* now n->degree > 0 also denotes that the contact was
		       outputted. */
		    fprintf (dmsSubRes -> dmfp, "c %d %d xl %d yb %d g %le\n",
			inSubTerm, sc -> group -> id, sc -> xl, sc -> yb, n -> substrCon[0]);
		    cnt = 0;
		    for (con = n -> con[0]; con; con = NEXT (con, n)) {
			nr = OTHER (con, n) -> degree;
			if (nr > 0) /* The neighbor contact was already outputted,
				reference that contact and output the resistor. */
			    fprintf (dmsSubRes -> dmfp, "nc %d g %le\n", nr, con -> val);
			cnt++;
		    }
		    fprintf (dmsSubRes -> dmfp, "nr_neigh %d\n", cnt);
		}
		else {
		    /* Makedela will number the contacts according to
		       the ordering predescribed by xl, yb.
		       So we follow that ordering scheme.
		    */
		    fprintf (dmsContPos -> dmfp, "%d %d %d %d %le %le\n",
			inSubTerm, sc -> group -> id,
			sc -> xl, sc -> yb, sc -> area, sc -> perimeter);
		}
	    }

	    if ((sn = sc -> subn))   { subnodeDel (sn); DISPOSE (sn, sizeof(subnode_t)); }
	    if ((sn = sc -> subnTL)) { subnodeDel (sn); DISPOSE (sn, sizeof(subnode_t)); }
	    if ((sn = sc -> subnTR)) { subnodeDel (sn); DISPOSE (sn, sizeof(subnode_t)); }
	    if ((sn = sc -> subnBL)) { subnodeDel (sn); DISPOSE (sn, sizeof(subnode_t)); }
	    if ((sn = sc -> subnBR)) { subnodeDel (sn); DISPOSE (sn, sizeof(subnode_t)); }

	    next_sc = sc -> next;
	    if (prePass1) subContGroupDel (sc);
	    rmFromSubcont_list (sc);

	    DISPOSE (sc, sizeof(subcont_t));
	    sc = next_sc;
	}
    }
    else {
	if ((sn = tile -> subcont -> subn)) { subnodeDel (sn); DISPOSE (sn, sizeof(subnode_t)); }
    }

    DISPOSE (tile -> subcont, sizeof(subcontRef_t));
}

Private void add2subcont_list (subcont_t* sc)
{
    subcont_t *scl = subcont_list_end -> prev;
    while (scl -> xl > sc -> xl
	   || (scl -> xl == sc -> xl && scl -> yb > sc -> yb)) {
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
    subcontGroup_t *g = sc -> group;

    if (!sc -> nextGroup && !sc -> prevGroup) { /* group is finished */
	DISPOSE (sc -> group, sizeof(subcontGroup_t));
    }
    else {
	if (g -> sc_begin == sc) g -> sc_begin = sc -> nextGroup;
	if (g -> sc_end   == sc) g -> sc_end   = sc -> prevGroup;
	if (sc -> nextGroup) sc -> nextGroup -> prevGroup = sc -> prevGroup;
	if (sc -> prevGroup) sc -> prevGroup -> nextGroup = sc -> nextGroup;
	ASSERT (g -> sc_end -> nextGroup == NULL);
    }
}

Private void subContGroupNew (subcont_t *sc)
{
    subcontGroup_t *g;

    sc -> group = g = NEW (subcontGroup_t, 1);
    g -> id = ++scGroupCnt;
    g -> sc_begin = sc;
    g -> sc_end   = sc;
    sc -> nextGroup = NULL;
    sc -> prevGroup = NULL;
}

void subContGroupJoin (subcont_t *scInfoA, subcont_t *scInfoB)
{
    subcont_t *sc;
    subcontGroup_t *gA, *gB;

    if ((gA = scInfoA -> group) == (gB = scInfoB -> group)) return;

    ASSERT (gA -> sc_end != gB -> sc_end);

    /* join subcont_groupB to subcont_groupA */

    ASSERT (gA -> sc_end -> nextGroup == NULL);
    gA -> sc_end -> nextGroup = gB -> sc_begin;

    ASSERT (gB -> sc_begin -> prevGroup == NULL);
    gB -> sc_begin -> prevGroup = gA -> sc_end;

    ASSERT (gB -> sc_end -> nextGroup == NULL);
    gA -> sc_end = gB -> sc_end;

    for (sc = gB -> sc_begin; sc; sc = sc -> nextGroup)
	sc -> group = gA;
    DISPOSE (gB, sizeof(subcontGroup_t));
}
