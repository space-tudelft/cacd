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

extern int adjConnLoss;
extern int debug_ready;
extern int sub_caps_entry;
extern bool_t lowest_min_res;
extern double MIN_CONDUCTANCE;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private double min_R_path (node_t *ns, node_t *nt, int n_cnt, int doContinue);
Private int  reducArtDegree (node_t **qn, int n_cnt);
Private int  reducMinRes    (node_t **qn, int n_cnt);
Private int  reducMinSepRes (node_t **qn, int n_cnt);
Private int  reducMaxParRes (node_t **qn, int n_cnt);
Private int  reducRmNegRes  (node_t **qn, int n_cnt);
#ifdef __cplusplus
  }
#endif
int do_min_res = 0;

int reducGroupI (node_t **qn, int n_cnt)
{
    if (!optFineNtw) {
	n_cnt = reducArtDegree (qn, n_cnt);
	if (!n_cnt) return (n_cnt);
	n_cnt = reducMinRes    (qn, n_cnt);
	n_cnt = reducMinSepRes (qn, n_cnt);
    }
    n_cnt = reducRmNegRes  (qn, n_cnt);
    n_cnt = reducMaxParRes (qn, n_cnt);
    return (n_cnt);
}

Private int reducArtDegree (node_t **qn, int n_cnt)
{
    int i, j, cx, rx, degree;
    register element_r *con, *el;
    node_t *n, *on, *obn;

    if (artReduc) {
	if (debug_ready) fprintf (stderr, "ready_grp: n_cnt=%d reducArtDeg: total(cap=%d res=%d)\n", n_cnt, currIntCap, currIntRes);
	if (min_art_degree > 0) n_cnt = nqEliminateAreaNodes (qn, n_cnt);
    }

    /* Eliminate non-terminal nodes that are connected to only a few conductors.
     */
    for (i = 0; i < n_cnt; i++) {
	n = qn[i];
try_again:
	n -> degree = i;

	if (n -> term > 1) n -> keep = 1;
	else {
            /* Test whether the node should be eliminated, and, if so,
             * use the sort <cx> of resistance connected to the node.
             */
	    n -> keep = 0;
	    cx = testRCelim (n);
	    if (n -> keep) continue;

	    if ((rx = cx) < 0) rx = 0;

	    if (artReduc) {
		if (n -> area) {
		    degree = n -> res_cnt;

		    if (n -> area == 1) {
			/* An equi-potential line introduces an art.degree = 2.
			Actually, it can also be 1 if a conductor loop occurs
			or if it is connected to a dead end.
			It is not necessary to compute the articulation degree
			exactly since with art.degree 2, the node may still be
			connected to a dead end (with an equi-potential line
			node on it). */
			if (degree < 2) { if (degree < min_art_degree) goto nokeep; }
			else if (degree < min_degree && min_art_degree > 2) goto nokeep;
		    }
		    else if (min_art_degree < 2) {
			if (degree < min_art_degree) goto nokeep;
		    }
		    else {
			/* Compute the articulation degree.
			Actually, the number of cliques connected to the node. */

			j = degree < min_degree ? min_art_degree : 2;
			if (degree < j) goto nokeep;

			/* first set all branches of node n */
			for (con = n -> con[rx]; con; con = NEXT (con, n)) {
			    on = OTHER (con, n);
			    on -> flag = 1; // setBranch
			}
			degree = 0;
			for (con = n -> con[rx]; con; con = NEXT (con, n)) {
			    on = OTHER (con, n);
			    if (on -> flag) {
				on -> flag = 0; // unsetBranch
				++degree;
				for (el = on -> con[rx]; el; el = NEXT (el, on)) {
				    obn = OTHER (el, on);
				    if (obn -> flag) obn -> flag = 0;
				}
			    }
			}
			if (degree < j) goto nokeep;
		    }
		    n -> keep = 1;
		    continue;
		}
		else if (n -> term) { /* articulation degree = 1 */
		    if (min_art_degree <= 1) n -> keep = 1;
		    continue;
		}
	    }
nokeep:
	    if (!n -> term) {
		elim (n, cx); // After testRCelim
		if (i == --n_cnt) return (n_cnt);
		qn[i] = n = qn[n_cnt];
		goto try_again;
	    }
	}
    }
    return (n_cnt);
}

Private int reducMinRes (node_t **qn, int n_cnt)
{
    register element_r *con;
    register int i, k;
    node_t *n, *nA, *node[2];
    double aval, maxaval;

    if (min_res > 0) {
	if (debug_ready) fprintf (stderr, "ready_grp: n_cnt=%d reducMinRes: total(cap=%d res=%d)\n", n_cnt, currIntCap, currIntRes);
	/* Join nodes that are connected by a small resistance
	 * (at least one of the nodes must have keep == 0).
	 */
	/* In "help" we store the total value of the eliminated
	   resistances that were connected to the node.
	   If both nodes that are connected to a resistor having keep == 0,
	   then the node that has the smallest value of "help" is eliminated.
	   This will guarantee a certain balancing when eliminating nodes.
	   Note that function createNode has inited "help" to zero.
	   Thus, we don't need to init the nodes here.
	*/
	nA = NULL; /* prevent compiler uninitialized warning */
	for (i = 0; i < n_cnt; i++) {
do_again:
	    n = qn[i];

	    if (!n -> keep)
	    for (k = 0; k < resSortTabSize; k++) {

		if (n -> substrCon[k] != 0) {
		    if ((aval = n -> substrCon[k]) < 0) aval = -aval;
		    if (aval > min_res) { // too much conductivity
			nA = nSUB; goto do_elim;
		    }
		}

		maxaval = min_res;
		for (con = n -> con[k]; con; con = NEXT (con, n)) {
		    if (con -> type == 'S') continue;
		    if ((aval = con -> val) < 0) aval = -aval;
		    if (aval > maxaval) { // too much conductivity
			maxaval = aval;
			nA = OTHER (con, n);
			if (!lowest_min_res) break;
		    }
		}

		if (maxaval > min_res) { // too much conductivity
		    if (!nA -> keep && n -> help > nA -> help) { // elim nA
			i = nA -> degree;
			////for (i = 0; i < n_cnt && qn[i] != nA; ++i);
			ASSERT (i < n_cnt && qn[i] == nA); // nA must be in qn
			nA = n;
			n = qn[i];
		    }
		    nA -> help += (1 / maxaval) + n -> help;
do_elim:
		    nodeRelJoin (nA, n);
		    if (nA -> term < n -> term) nA -> term = n -> term;
		    n -> term = 0;
		    do_min_res = i;
		    elim (n, k); // elimRES
		    qn[i] = qn[--n_cnt];
		    qn[i] -> degree = i;
		    if ((i = do_min_res) < n_cnt) goto do_again;
		    goto ret;
		}
	    }
	}
ret:
	do_min_res = 0;
    }
    return (n_cnt);
}

Private int reducMinSepRes (node_t **qn, int n_cnt)
{
    register element_r *con;
    register int i, j, k;
    node_t *n, *nA;

    if (min_sep_res > 0 && n_cnt > 1) {
	if (debug_ready) fprintf (stderr, "ready_grp: n_cnt=%d reducSepRes: total(cap=%d res=%d)\n", n_cnt, currIntCap, currIntRes);
    /*
     * join nodes that are connected by a small resistance
     */
	for (i = 0; i < n_cnt; i++) {
	    n = qn[i];
	    if (n -> term < 2)
	    for (k = 0; k < resSortTabSize; k++) {
		for (con = n -> con[k]; con; con = NEXT (con, n)) {
		    if (con -> type == 'S') continue;
		    if (Abs (con -> val) > min_sep_res) { // too much conductivity
			nA = OTHER (con, n);
			if ((n = nodeJoin (nA, n)) == nA) j = i; // n deleted
			else { /* find nA */
			    for (j = 0; j < n_cnt && qn[j] != nA; j++);
			    ASSERT (j < n_cnt); // nA must be in qn
			}
			qn[j] = qn[--n_cnt];
			if (n_cnt == 1) return (n_cnt);

			/* If node n still exists, start with it again
			   since it has been joined with an other node.
			   Otherwise, continue with a new node. */
			i--;
			goto end_of_node_loop2;
		    }
		}
	    }
end_of_node_loop2: ;
	}
    }
    return (n_cnt);
}

static int QC_cnt;
static node_t **QC;

Private void putQueueClique (node_t *n)
{
    static int QC_size = 0;

    if (QC_cnt >= QC_size) {
	int old_size = QC_size;
	QC_size = !QC_size ? 50 : (int)(QC_size * 1.5);
	QC = RESIZE (QC, node_t *, QC_size, old_size);
    }
    QC[ QC_cnt++ ] = n;
}

Private int reducMaxParRes (node_t **qn, int n_cnt)
{
    int i, j, k, l, r_flag, area_nodes;
    node_t *n, *c_n;
    register element_r *con;
    double min, res;

    if (max_par_res >= 1 && n_cnt > 1) {
    /*
     * remove resistances that are
     * short-circuited by a much lower resistance path
     */
	if (debug_ready) fprintf (stderr, "ready_grp: n_cnt=%d reducParRes: total(cap=%d res=%d)\n", n_cnt, currIntCap, currIntRes);

	area_nodes = 0;
	for (i = 0; i < n_cnt; i++) {
	    n = qn[i];
	    if (n -> area) { ++area_nodes; continue; }
	    if (n -> flag == 4) continue;

	    /* First, search clique of n */

	    QC_cnt = 0;
	    n -> flag = 1;
	    putQueueClique (n);

	    for (k = 0; k < resSortTabSize; k++) {
		for (con = n -> con[k]; con; con = NEXT (con, n)) {
		    if (con -> type == 'S') continue;
		    c_n = OTHER (con, n);
		    if (c_n -> flag != 1) {
			c_n -> flag = 1;
			putQueueClique (c_n);
		    }
		}
	    }

	    /* Then, evaluate resistances between all pairs of nodes in the clique */

	    for (l = 0; l < QC_cnt-1; l++) {
		n = QC[l];

		if (l) { /* re-init */
		    for (j = 0; j < l; j++) QC[j] -> flag = 3;
		    for ( ; ++j < QC_cnt; ) QC[j] -> flag = 1;
		}
		n -> flag = 3;

		/* r_flag is used to speed-up the searching with min_R_path().
		 * As long as we start searching from the same node, we can use
		 * the previous results obtained so far.
		 */
		r_flag = 0;
		for (k = 0; k < resSortTabSize; k++) {
		    for (con = n -> con[k]; con; con = NEXT (con, n)) {
			c_n = OTHER (con, n);
			if ((c_n -> flag & 3) == 1) {
			    if ((res = con -> val) < 0) res = -res;
			    if (res < MIN_CONDUCTANCE) goto try_del1;
			    res = 1 / res;
			    min = min_R_path (n, c_n, QC_cnt, r_flag);
			    r_flag = 1;
			    if (min > 0 && min * max_par_res + 1e-9 < res) {
try_del1:
				elemDelRes (con);
			    }
			}
		    }
		}
	    }

	    for (l = 0; l < QC_cnt; l++) QC[l] -> flag = 4;
	}

	/* Cliques with only area nodes are still uninvestigated now.
	   Therefore, evaluate the resistances between the area node pairs.

	   Now, min_R_path() will not find the minimum parallel path if
	   this path is via a non-area node.  However, in that case,
	   the resistance between the two nodes will already have
	   been computed correctly in the previous part.
	*/
	if (area_nodes < 2) return (n_cnt);

	QC_cnt = 0;
	for (i = 0; i < n_cnt; i++) {
	    if (qn[i] -> area) {
		qn[i] -> flag = 1;
		putQueueClique (qn[i]);
	    }
	}

	for (l = 0; l < QC_cnt-1; l++) {
	    n = QC[l];

	    if (l) { /* re-init */
		for (j = 0; j < l; j++) QC[j] -> flag = 3;
		for ( ; ++j < QC_cnt; ) QC[j] -> flag = 1;
	    }
	    n -> flag = 3;

	    r_flag = 0;
	    for (k = 0; k < resSortTabSize; k++) {
		for (con = n -> con[k]; con; con = NEXT (con, n)) {
		    c_n = OTHER (con, n);
		    if ((c_n -> flag & 3) == 1) {
			if ((res = con -> val) < 0) res = -res;
			if (res < MIN_CONDUCTANCE) goto try_del2;
			res = 1 / res;
			min = min_R_path (n, c_n, QC_cnt, r_flag);
			r_flag = 1;
			if (min > 0 && min * max_par_res + 1e-9 < res) {
try_del2:
			    elemDelRes (con);
			}
		    }
		}
	    }
	}

	for (l = 0; l < QC_cnt; l++) QC[l] -> flag = 0;
    }
    return (n_cnt);
}

Private int only_neg_res (node_t *n, element_r *CON, int kk)
{
    register element_r *con;
    double val;
    int k, cx = -1;

    for (k = 0; k < resSortTabSize; k++) {
	for (con = n -> con[k]; con; con = NEXT (con, n)) {
	    if (con == CON) continue; /* skip this resistor */
	    if (con -> val >= MIN_CONDUCTANCE) { /* positive resistor */
		return (-1); /* no, too many */
	    }
	    else if (Abs (con -> val) < MIN_CONDUCTANCE) {
		if (con -> type == 'S') adjConnLoss = 1;
		elemDelRes (con); /* delete */
		continue;
	    }
	    if (cx < 0) cx = k; else if (cx != k) return (-1); /* no */
	}
	if ((val = n -> substrCon[k]) != 0) {
	    if (val >= MIN_CONDUCTANCE) { /* positive resistor */
		return (-1); /* no, too many */
	    }
	    else if (Abs (val) < MIN_CONDUCTANCE) {
		n -> substrCon[k] = 0; /* delete */
		continue;
	    }
	    if (cx < 0) cx = k; else if (cx != k) return (-1); /* no */
	}
    }
    if (cx >= 0 && cx != kk) { kk = cx; elemDelRes (CON); } /* first delete CON */
    return (kk); /* yes, elim node */
}

Private int reducRmNegRes (node_t **qn, int n_cnt)
{
    register element_r *con;
    register int i, j, k;
    node_t *n, *c_n;

    if (no_neg_res) { /* remove negative (or too large) resistances */
	if (debug_ready) fprintf (stderr, "ready_grp: n_cnt=%d reducNegRes: total(cap=%d res=%d)\n", n_cnt, currIntCap, currIntRes);
try_again:
	for (i = 0; i < n_cnt; i++) {
	    n = qn[i];
	    for (k = 0; k < resSortTabSize; k++) {

		if (n -> substrCon[k] != 0)
		    if (n -> substrCon[k] < MIN_CONDUCTANCE) n -> substrCon[k] = 0;

		for (con = n -> con[k]; con; con = NEXT (con, n)) {
		    if (con -> val < MIN_CONDUCTANCE) {
			if (con -> type == 'S') { /* connect with other group */
			    adjConnLoss = 1;
			    elemDelRes (con);
			    continue;
			}
                        /* First check if one of the nodes has ONLY negative
                           (or too large) resistances connected to it OR
			   one dangling resistor in case the node is no term.
			   Because then the node would become isolated when we
			   remove all resistances.  Therefore, in that case,
                           we eliminate the node.
			 */
			c_n = OTHER (con, n);

			if (n -> term < 2 && (j = only_neg_res (n, con, k)) >= 0) {
			    nodeRelJoin (c_n, n);
			    if (c_n -> term < n -> term) c_n -> term = n -> term;
			    n -> term = 0;
			    elim (n, j); // elimRES
			    qn[i] = qn[--n_cnt];
			    goto try_again;
			}
			if (c_n -> term < 2 && (j = only_neg_res (c_n, con, k)) >= 0) {
			    nodeRelJoin (n, c_n);
			    if (n -> term < c_n -> term) n -> term = c_n -> term;
			    c_n -> term = 0;
			    elim (c_n, j); // elimRES
			    n_cnt--;
			    for (i = 0; i < n_cnt && qn[i] != c_n; i++);
			    ASSERT (qn[i] == c_n);
			    qn[i] = qn[n_cnt];
			    goto try_again;
			}
			elemDelRes (con);
                    }
                }
	    }
	}
    }
    return (n_cnt);
}

void reducRmNegCap (node_t **qn, int n_cnt)
{
    register int i, k;
    register element_c *cap;
    node_t *n, *on;

    if (debug_ready) fprintf (stderr, "ready_grp: n_cnt=%d reducNegCap: total(cap=%d res=%d)\n", n_cnt, currIntCap, currIntRes);

    /* remove negative capacitances */
	for (i = 0; i < n_cnt; i++) {
	    n = qn[i];
	    for (k = 0; k < capSortTabSize; k++) {

		if (n -> gndCap[k] < 0) n -> gndCap[k] = 0;
		if (n -> substrCap[k] < 0) n -> substrCap[k] = 0;

		for (cap = n -> cap[k]; cap; cap = NEXT (cap, n)) {
		    if (cap -> val <= 0) {
			on = OTHER (cap, n);
			if (Grp(on) != Grp(n)) adjConnLoss = 1;
			elemDelCap (cap);
		    }
		}
	    }
	}
}

/* The second part of the reduction can only be done if all adjacent
 * groups are ready.
 * This is because, otherwise, nodes in adjacent groups may be collapsed.
 */
void reducGroupII (node_t **qn, int n_cnt)
{
    int i, j;
    node_t *n, *on;
    register element_c *cap;
    double gCap, ogCap, val, min_coup;

    /* Remove coupling capacitances that are connected
     * (on both sides) to a much larger ground capacitance.
     * Small ground capacitors are removed with parameter min_ground_cap
     * in routine outNode.
     */
    for (i = 0; i < n_cnt; i++) {
	n = qn[i];
	if (n -> cap_cnt > 0)
	for (j = 0; j < capSortTabSize; j++) {
	    min_coup = capAreaPerimEnable[j] ? min_coup_area : min_coup_cap;
	    if (min_coup <= 0) continue;

	    gCap = n -> gndCap[j] + n -> substrCap[j];
	    if (gCap <= 0) continue;

	    gCap *= min_coup;
	    for (cap = n -> cap[j]; cap; cap = NEXT (cap, n)) {

		val = Abs (cap -> val);
		if (val >= gCap) continue;

		on = OTHER (cap, n);
		ogCap = on -> gndCap[j] + on -> substrCap[j];
		if (val >= ogCap * min_coup) continue;

		/* remove couple cap ONLY between 2 different conductor groups */
		if (Grp(on) == Grp(n)) continue;

		if ((n -> gndCap[j] == 0 && on -> gndCap[j] == 0) || (sub_caps_entry && j == sub_caps_entry)) {
		    n -> substrCap[j] += cap -> val;
		    on -> substrCap[j] += cap -> val;
		}
		else {
		    n -> gndCap[j] += cap -> val;
		    on -> gndCap[j] += cap -> val;
		}
		elemDelCap (cap);
	    }
	}
    }
}

static int min_R_tabsize = 0;
static node_t ** frontNodes;

Private double min_R_path (node_t *ns, node_t *nt, int n_cnt, int doContinue)
{
    static node_t *n;
    static int f_cnt;
    int i, f, k;
    double min;
    node_t * c_n;
    register element_r *con;

    if (n_cnt > min_R_tabsize) { /* n_cnt > 0 */
	ASSERT (!doContinue);
	if (!min_R_tabsize) min_R_tabsize = Max (100, n_cnt);
	else {
	    DISPOSE (frontNodes, sizeof(node_t *) * min_R_tabsize);
	    min_R_tabsize = n_cnt;
	}
	frontNodes = NEW (node_t *, min_R_tabsize);
    }

    if (!doContinue) {
	n = ns; n -> help = 0; f_cnt = 0; n -> flag |= 8;
    }
    else { /* use previous results (ns must be unchanged) */
	if (nt -> flag & 8) return (nt -> help);
    }

    while (n && n != nt) {

	for (k = 0; k < resSortTabSize; k++) {
	    for (con = n -> con[k]; con; con = NEXT (con, n)) {
		c_n = OTHER (con, n);
		if ((c_n -> flag & 9) == 1 && con -> val > 0) {
		    min = 1 / con -> val + n -> help; /* min > 0 */
		    if (c_n -> flag < 4) {
			frontNodes[ f_cnt++ ] = c_n;
			c_n -> help = min;
			c_n -> flag |= 4;
		    }
		    else if (min < c_n -> help) {
			c_n -> help = min;
		    }
		}
	    }
	}

	if (f_cnt > 0) {
	    min = frontNodes[f = 0] -> help;
	    for (i = 0; ++i < f_cnt;) {
		if (frontNodes[i] -> help < min) min = frontNodes[f = i] -> help;
	    }
	    n = frontNodes[f];
	    frontNodes[f] = frontNodes[--f_cnt];
	    n -> flag |= 8; /* minimum resistance path has been determined */
	}
	else
	    n = NULL;
    }
    if (n) return (nt -> help);
    return (-1); /* no path (along positive R's) */
}
