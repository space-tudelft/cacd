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
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/lump/define.h"
#include "src/space/lump/extern.h"

extern int debug_readygrp;
extern int debug_readyGrp;
extern bool_t eliminateSubstrNode;

node_t **QN = NULL; /* not necessary to do explicity, but it MUST be NULL */
int QN_cnt;

static group_t **QAG;
static int QAG_cnt;
static int adjNotReady;
static int cntR;

int debug_ready = 0;
int adjConnLoss;
int QAG_inuse = 0;
int delay_cnt = 0;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void putQueueNodes (node_t *n);
Private void putQueueGroups (group_t *grp);
Private void checkOutGroup (group_t *grp);
#ifdef __cplusplus
  }
#endif

/* pre: group to which n belongs has become ready */
/* actions: RC model reduction for group is completed */
/*          group is outputted if adjacent groups are ready */
/*          adjacent groups are checked for outputting */

void readyGroup (node_t *n)
{
    int i, j, keep_cnt, term_cnt;
    node_t *on;
    register element_c *cap;
    register element_r *con;
    group_t *grp, *ogr;

    ASSERT (QAG_inuse == 0);
    QAG_inuse = 1; /* readyGroup may not be called recursively */

    if (!n -> term) testRCelim (n);

    grp = Grp (n);
    ASSERT (grp -> notReady == 0);

#ifdef DEBUG_NODES
    fprintf (fp_dbn, "\nReady group: %p node: %p\n", grp, n);
    debug_nodes_check (1);
#endif

    adjNotReady = 0;
    QAG_cnt = 0;

    QN_cnt = 0; /* init findGroup(n) */
    putQueueNodes (n);

    if (n -> res_cnt) { /* Find other nodes of the group */
	n -> flag = 1;
	for (i = 0; i < QN_cnt; ++i) {
	    n = QN[i];
	    for (j = 0; j < resSortTabSize; ++j) {
		for (con = n -> con[j]; con; con = NEXT (con, n)) {
		    on = OTHER (con, n);
		    if (con -> type == 'S') {
			if ((ogr = Grp (on)) != grp) {
			    if (!ogr -> flagQG) putQueueGroups (ogr);
			    continue;
			}
			con -> type = 'G';
		    }
		    if (!on -> flag) { on -> flag = 1; putQueueNodes (on); }
		}
	    }
	}
    }

    if (currIntCap > 0) { /* Find the adjacent groups */
	for (i = 0; i < QN_cnt; ++i) {
	    n = QN[i];
	    for (j = 0; j < capSortTabSize; ++j) {
		for (cap = n -> cap[j]; cap; cap = NEXT (cap, n)) {
		    on = OTHER (cap, n);
		    ogr = Grp (on);
		    if (!optCoupCap && j == 0) {
			/* remove couple cap */
			if (ogr != grp) {
			    /* make ground caps */
			    n -> gndCap[j] += cap -> val;
			    on -> gndCap[j] += cap -> val;
			}
			elemDelCap (cap);
		    }
		    else if (ogr != grp && !ogr -> flagQG) putQueueGroups (ogr);
		}
	    }
	}
    }

    /* First, put the appropriate nodes on the elimination queue.
     * Remove all delayed nodes from QN and increase priority.
     */
    j = QN_cnt;
    keep_cnt = term_cnt = 0;
    for (i = 0; i < QN_cnt;) {
	n = QN[i];
	n -> flag = 0;
	if (n -> delayed) nqChange (n); /* increase priority */
	else if (n -> term) { ++term_cnt; ++i; continue; }
	else if (n -> keep) { ++keep_cnt; ++i; continue; }
	else if (n -> area && artReduc) { ++i; continue; }
	else nqDelayNode (n);
	QN[i] = QN[--QN_cnt]; /* remove delayed node from QN */
    }
    delay_cnt = j - QN_cnt;

    if (debug_readygrp) { ++cntR;
    if (j >= debug_readygrp) {
	if (debug_readyGrp && j >= debug_readyGrp) debug_ready = 1;
	fprintf (stderr, "ready_grp %d: nodes=%d (delayed=%d term=%d keep=%d area=%d) adjgrps=%d total(nodes=%d delayed=%d cap=%d res=%d mem=%.3f)\n",
		cntR, j, delay_cnt, term_cnt, keep_cnt, QN_cnt - term_cnt - keep_cnt, QAG_cnt,
		currIntNod, delayCount, currIntCap, currIntRes, allocatedMbyte());
    }}

    elim_grp = grp;

    if (delay_cnt) {
	/* Now eliminate (delete) all delayed nodes of the group,
	 * use Markowitz scheme. Get delayed nodes from priority queue.
	 */
	QN_cnt = nqEliminateGroup (QN, QN_cnt);
    }

    /* QN_cnt is 0 if all nodes are eliminated */

    adjConnLoss = 0;
    if (QN_cnt && optReduc) { /* extrPass */
#ifdef DEBUG_NODES
	fprintf (fp_dbn, "\nReducing group %p\n", grp);
#endif
	if (optRes) QN_cnt = reducGroupI (QN, QN_cnt);
	if (no_neg_cap) reducRmNegCap (QN, QN_cnt);
    }

    if (nqDelayList) nqDelayNodes ();

    if (debug_ready) { debug_ready = 0;
	fprintf (stderr, "ready_grp: n_cnt=%d do outGroup: total(cap=%d res=%d)\n", QN_cnt, currIntCap, currIntRes);
    }

    if (!QN_cnt) groupDel (grp);
    else if (adjNotReady == 0) outGroup (QN, QN_cnt);
    else if (adjConnLoss) checkOutGroup (grp);

    for (i = 0; i < QAG_cnt; i++) {
	ogr = QAG[i];
	ogr -> flagQG = 0; /* reset flag */
	if (ogr -> notReady == 0) checkOutGroup (ogr);
    }

#ifdef DEBUG_NODES
    debug_nodes_check (2);
#endif
    QAG_inuse = 0;
}

Private void putQueueNodes (node_t *n)
{
    static int QN_size = 0;

    if (QN_cnt >= QN_size) {
	int old_size = QN_size;
	QN_size = !QN_size ? 50 : (int)(QN_size * 1.5);
	QN = RESIZE (QN, node_t *, QN_size, old_size);
    }
    QN[ QN_cnt++ ] = n;
}

Private void putQueueGroups (group_t *grp)
{
    static int QAG_size = 0;

    if (QAG_cnt >= QAG_size) {
	int old_size = QAG_size;
	QAG_size = !QAG_size ? 50 : (int)(QAG_size * 1.5);
	QAG = RESIZE (QAG, group_t *, QAG_size, old_size);
    }
    /* QAG will contain the adjacent group */
    QAG[ QAG_cnt++ ] = grp;
    grp -> flagQG = 1;
    if (grp -> notReady > 0) adjNotReady++;
#ifdef DEBUG_NODES
    fprintf (fp_dbn, "Adjacent group %p notReady=%d\n", grp, grp -> notReady);
#endif
}

Private void checkOutGroup (group_t *grp)
{
    register element_c *cap;
    register element_r *con;
    group_t *ogr;
    node_t *n, *on;
    int i, cnt = QAG_cnt;

    QN_cnt = 0;

    for (n = grp -> nodes; n; n = n -> gnext) {
	if (substrRes)
	for (i = 0; i < resSortTabSize; ++i) {
	    for (con = n -> con[i]; con; con = NEXT (con, n)) {
		if (con -> type == 'S') {
		    on = OTHER (con, n);
		    ogr = Grp (on);
		    if (ogr -> notReady > 0) goto ret;
		    /* add ready group to QAG, else possible left alone (SdeG) */
		    if (ogr != grp && !ogr -> flagQG) putQueueGroups (ogr);
		}
	    }
	}
	for (i = 0; i < capSortTabSize; ++i) {
	    for (cap = n -> cap[i]; cap; cap = NEXT (cap, n)) {
		on = OTHER (cap, n);
		ogr = Grp (on);
		if (ogr -> notReady > 0) goto ret;
		    /* add ready group to QAG, else possible left alone (SdeG) */
		if (ogr != grp && !ogr -> flagQG) putQueueGroups (ogr);
	    }
	}
	putQueueNodes (n);
    }

    outGroup (QN, QN_cnt);
    return;
ret:
    while (QAG_cnt > cnt) { /* remove the added groups (SdeG) */
	ogr = QAG[ --QAG_cnt ];
	ogr -> flagQG = 0;
    }
}

void do_outGroup (group_t *grp)
{
    static int cnt = 0;

    if (grp) {
	if (!grp -> flagQG) {
	    if (!cnt++) QAG_cnt = 0;
	    putQueueGroups (grp);
	}
    }
    else if (cnt) {
	for (cnt = 0; cnt < QAG_cnt; ++cnt) {
	    grp = QAG[cnt];
	    checkOutGroup (grp);
	    grp -> flagQG = 0; /* reset flag */
	}
	cnt = 0;
    }
}
