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

/*============================================================================*/
/*! @file
    @brief Node allocation/deletion and node priority queue functions.

    This module implements
    <ul>
    <li>a fast node allocation and deletion module</li>
    <li>a priority queue for elimination of nodes based on
    combined frontal-markowitz ordering as discussed in my (NvdM)
    notebook 940802 page 34-36</li>
    <li>a node statistics module</li>
    </ul>
*//*==========================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/export.h"
#include "src/space/lump/define.h"
#include "src/space/lump/extern.h"
#include "src/space/bipolar/define.h"
#include "src/space/bipolar/export.h"
#include <limits.h>

extern double minRes;
extern double MIN_CONDUCTANCE;
extern int elim_check_again;
int pr_it = 0;

int pre_elim_mode = 0;
int pre_elim_degree = -2;
bool_t pre_elim_area = 0;

static cluster_t *Cluster;
static node_t **nqBase = NULL;
static int max_elim_degree = 0;
static int Max_degree = 0;
static int Min_degree = 0;

static unsigned int allocs = 0;
static unsigned int frees  = 0;
static unsigned int max    = 0;
static unsigned int n_size = 0;

static node_t * free_list = NULL;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif

extern void joinNodes (node_t *nA, node_t *nB);
extern void removeNodeFromCluster (node_t *n, int dispose);
extern void preElim (node_t *n, int k);
extern long lrand48 (void);
extern void srand48 (long int);
extern void nqElimGrp (group_t *grp);
extern void nqElimCluster (node_t *n, int type);

Private node_t *allocNode (void);
Private double minR_path (node_t *ns, node_t *nt);
Private void queueNode (node_t *n);
Private void reducMaxParRes (void);
Private int  reducMinSepRes (int nr);
Private void checkMaxParRes (node_t *n);

#ifdef __cplusplus
  }
#endif

static node_t **frontNodes;
static node_t **QN;
static int QN_cnt, QN_size;

Private void queueNode (node_t *n)
{
    if (QN_cnt == QN_size) { QN_size += 20; QN = RESIZE (QN, node_t *, QN_size, QN_cnt); }
//fprintf (stderr, "queueNode: a=%d t=%d m=%d x,y=%d,%d\n", n->area, n->term, n->mask, n->node_x/4, n->node_y/4);
    QN[ QN_cnt++ ] = n; n -> flag = 1;
}

/* Init the node module, also for each new cell.
 */
void nqInit ()
{
    int i, max;

    srand48 (1L); /* init random node id's */

    if (pre_elim_degree == -2 && optIntRes) {

	if (optFineNtw) pre_elim_degree = 0;
	else {
	    pre_elim_degree = paramLookupI ("pre_elim_degree", "6");
	    if (pre_elim_degree > 12) {
		paramError ("pre_elim_degree", "too big value (using 12)");
		pre_elim_degree = 12;
	    }
	    if (++pre_elim_degree < 0) pre_elim_degree = 0;
	}

	if (pre_elim_degree) pre_elim_area = paramLookupB ("pre_elim_area", "on");

	if (optFineNtw) max_elim_degree = 0;
	else {
	    max_elim_degree = paramLookupI ("max_elim_degree", "40");
	    if (max_elim_degree > 500) {
		paramError ("max_elim_degree", "too big value (using 500)");
		max_elim_degree = 500;
	    }
	    if (++max_elim_degree < 0) max_elim_degree = 0;
	}

	max = max_elim_degree;
	if (max < pre_elim_degree) max = pre_elim_degree;
	if (max < min_art_degree) max = min_art_degree;
	if (max > 0) {
	    nqBase = NEW (node_t *, max);
	    for (i = 0; i < max; ++i) nqBase[i] = NULL;
	}
    }
}

/*  Create a fresh node
 */
node_t * createNode ()
{
    int i;
    node_t *n;

    if (free_list) {
	n = free_list;
	free_list = free_list -> next;
    }
    else
	n = allocNode ();

    ++allocs;

    n -> substr = 0;
    n -> flag = 0;
    n -> names = NULL;
    n -> netEq = NULL;
    n -> netEq2 = NULL;
    n -> prev = NULL; /* not in pq */
    n -> subs = NULL;
    n -> area = 0;
    n -> term = 0;
    n -> delayed = 0;
    n -> res_cnt = 0;
    n -> cap_cnt = 0;
    n -> n_n_cnt = 0;
    n -> pols = NULL;
 /* n -> grp = NULL; ** grp not known (is set outside createNode) */

    n -> node_h = 0;
    n -> node_w = 0;

    /* lrand48 returns a long but, if the sequence is truly random,
     * it is safe to truncate if an int is of lower precision.
     */
    n -> id = (int) lrand48 ();

    for (i = 0; i < resSortTabSize; i++) {
	n -> con[i] = NULL;
	n -> substrCon[i] = 0;
    }

    for (i = 0; i < capSortTabSize; i++) {
	n -> cap[i] = NULL;
	n -> gndCap[i] = 0;
	n -> substrCap[i] = 0;
    }

    inNod++;
    if (++currIntNod > maxIntNod) maxIntNod = currIntNod;

    return (n);
}

/*  Dispose a node
 */
void disposeNode (node_t *n)
{
    ASSERT (!n -> pols);
    currIntNod--;

    frees++;
    n -> next = free_list;
    free_list = n;
}

void nodeStatistics (FILE *fp)
{
    extern int maxIntClr;
    fprintf (fp, "overall node statistics:\n");
    fprintf (fp, "\tnodes created      : %u\n", allocs);
    fprintf (fp, "\tnodes disposed     : %u\n", frees);
    fprintf (fp, "\tnodes allocated    : %u of %u byte\n", max, n_size);
    fprintf (fp, "\tclusters allocated : %u of %u byte\n", maxIntClr, (unsigned)sizeof(cluster_t));
}

/* Allocate a node, add element arrays as approriate.
 * Allignment is a tricky issue;
 * This routine works on all hardware we have tried
 * thanks to the fact that all items in the allocated structs
 * have a size that is a multiple of 4 and can be allocated
 * on a 4-byte boundary.
 * However, allignment is easily testable with the test driver
 * included in this file, compile with -DDriver.
 */
Private node_t * allocNode ()
{
    static unsigned int rsize1 = 0;
    static unsigned int csize1 = 0;
    static unsigned int csize2 = 0;
    node_t * node;
    char * p;

    ++max;

    /* It is difficult to do this in
     * a type-clean manner, e.g. using unions as is done in malloc.
     * The problem is that i need to attach several pieces of memory
     * of a size *unknown at compile time*.
     *
     * These data are contiguous in memory for improved VM performance
     * and lower malloc overhead (both time and space).
     */

    /* First, compute the size of the memory to be allocated.
     */
    if (!n_size) {
	n_size = sizeof (node_t);

	rsize1 = resSortTabSize * sizeof (double);
	csize1 = capSortTabSize * sizeof (double);
	csize2 = capSortTabSize * sizeof (element_c *);
	n_size += rsize1;
	n_size += csize1;
	n_size += csize1;
	n_size += csize2;
	n_size += resSortTabSize * sizeof (element_r *);
    }

    node = (node_t *) MALLOC (n_size);
    p = (char *) (node + 1);

    /* Warning: For alignment, doubles must first be allocated */

    node -> substrCon = (double *) p; p += rsize1;
    node -> substrCap = (double *) p; p += csize1;
    node -> gndCap    = (double *) p; p += csize1;
    node -> cap   = (element_c **) p; p += csize2;
    node -> con   = (element_r **) p;

    return (node);
}

#define Degree(n) n -> res_cnt
#define Ready(n) !n -> subs
#define Keep(n)   n -> term
#define ReadyArea1(n) (n -> area == 1 && Ready(n) && !Keep(n))

void nqChange (node_t *n)
{
    int newdegree = Degree(n);

    if (n -> prev) { /* n in pq */
	if (newdegree == n -> degree) return;
	/* delete from pq */
	if (nqBase[n -> degree] == n) nqBase[n -> degree] = n -> next;
	else {
	    ASSERT (n -> prev -> next == n);
	    if (n -> next) n -> next -> prev = n -> prev;
	    n -> prev -> next = n -> next;
	}
	n -> prev = NULL; /* n not in pq */
    }

    if (newdegree < Max_degree) { /* insert into pq */
	if (pre_elim_mode) {
	    if (!Ready(n) || Keep(n) || n -> clr != Cluster) return;
	    ASSERT (!n -> area);
	}
	if (newdegree < Min_degree) Min_degree = newdegree;
	n -> next = nqBase[newdegree];
	if (n -> next) n -> next -> prev = n;
	nqBase[newdegree] = n;
	n -> degree = newdegree;
	n -> prev = n; /* n in pq */
    }
}

void prepareElim (node_t *nA, node_t *nB)
{
    nA -> term |= nB -> term;
    if (nB -> area > nA -> area) nA -> area = nB -> area;
    nB -> term = 0;

    if (nB -> node_x < nA -> node_x
            || (nB -> node_x == nA -> node_x && nB -> node_y < nA -> node_y)) {
        nA -> mask = nB -> mask;
	nA -> node_x = nB -> node_x;
	nA -> node_y = nB -> node_y;
    }

    if (nB -> names) {
	if (!nA -> names) nA -> names = nB -> names;
	else {
	    terminal_t *ta, *tb;
	    ta = nA -> names;
	    tb = nB -> names;
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
	netEquiv_t *ne;
	while ((ne = nB -> netEq) && ne -> instType != RES) {
	    nB -> netEq = ne -> next;
	    ne -> next = nA -> netEq;
	    nA -> netEq = ne;
	}
    }
}

#define KEEP(n) (n -> term & 2)
static int elimGrp_check;

void nqElimGrp (group_t *grp)
{
    element_r *con;
    node_t *n, *on;
    int i;
//#define EXTRA_REDUC_MAXRES
//#define CNT_ELIM
#ifdef CNT_ELIM
    int k=0, e_cnt=0, a1=0, a2=0, a3=0;
#endif

    if (min_art_degree < 1) return;

    Min_degree = Max_degree = min_art_degree;

    if (minRes > 0) {
	for (n = grp -> nodes; n; n = n -> gnext) {
again_n:
	    for (con = n -> con[0]; con; con = NEXT (con, n)) {
		if (con -> val > minRes) { // too much conductivity
		    on = OTHER (con, n);
		    if (!KEEP(on)) {
			if (min_sep_res > 0 && con -> val > min_sep_res) {
			    joinNodes (n, on);
			} else {
			    prepareElim (n, on);
			    elim (on, 0);
			}
			goto again_n;
		    }
		}
	    }
	}
    }

    if (grp -> nod_cnt > 2 && max_par_res >= 2) {
	QN_cnt = grp -> nod_cnt;
	checkMaxParRes (grp -> nodes);
    }

    for (n = grp -> nodes; n; n = n -> gnext) {
	if (Keep(n)) {
	    if (n -> res_cnt == 1 && (con = n -> con[0])) { on = OTHER (con, n);
		if (min_sep_res > 0 && con -> val > min_sep_res) {
		    joinNodes (n, on);
		}
		else if (!Keep(on)) {
		    prepareElim (n, on);
#ifdef EXTRA_REDUC_MAXRES
		    QN_cnt = 0;
		    if (on->res_cnt >= 3) {
			for (con = on -> con[0]; con; con = NEXT (con, on)) queueNode (OTHER (con, on));
		    }
#endif
		    elim (on, 0);
#ifdef EXTRA_REDUC_MAXRES
		    if (QN_cnt) {
			if (QN_cnt >= 3) reducMaxParRes ();
			for (i = 0; i < QN_cnt; ++i) QN[i] -> flag = 0;
		    }
#endif
		}
	    }
	}
    }

    pr_it = 0;
    elimGrp_check = 1;
again:
    //++pr_it;
    for (n = grp -> nodes; n; n = n -> gnext) {
	if (!Keep(n)) {
	    if (n -> area == 1) {
		nqChange (n); n -> delayed = 1;
#ifdef CNT_ELIM
		++a1;
#endif
	    }
	    else if (n -> area == 2) {
		nqChange (n); n -> delayed = 1;
#ifdef CNT_ELIM
		++a2;
#endif
	    }
	    else {
		nqChange (n); n -> delayed = 1;
#ifdef CNT_ELIM
		++a3;
#endif
	    }
	}
	else {
	    n -> delayed = 0;
#ifdef CNT_ELIM
	    ++k;
#endif
	}
    }

    while (Min_degree < Max_degree) {
	while ((n = nqBase[Min_degree])) {
	    nqBase[Min_degree] = n -> next;
	    i = testRCelim (n);
	    if (!Keep(n)) {
#ifdef EXTRA_REDUC_MAXRES
		    QN_cnt = 0;
		    if (n->res_cnt >= 3) {
			for (con = n -> con[0]; con; con = NEXT (con, n)) queueNode (OTHER (con, n));
		    }
#endif
		elim (n, i); elimGrp_check = 1;
#ifdef EXTRA_REDUC_MAXRES
		    if (QN_cnt) {
			if (QN_cnt >= 3) reducMaxParRes ();
			for (i = 0; i < QN_cnt; ++i) QN[i] -> flag = 0;
		    }
#endif
#ifdef CNT_ELIM
		++e_cnt;
#endif
	    }
	    else n -> delayed = 0;
	}
	Min_degree++;
    }
#ifdef CNT_ELIM
    fprintf (stderr, "elimGrp: total=%d (a1=%d a2=%d kept=%d other=%d) elim=%d\n", a1+a2+k, a1, a2, k, a3, e_cnt);
#endif
//-----------------------------------------------------------------------------
    if (elimGrp_check && grp -> nod_cnt > 2 && max_par_res >= 2) {
	//QN_cnt = 0;
	//for (n = grp -> nodes; n; n = n -> gnext) queueNode (n);
	//ASSERT (QN_cnt == grp -> nod_cnt);
	QN_cnt = grp -> nod_cnt;

	checkMaxParRes (grp -> nodes);

	if (!elimGrp_check) goto again;
    }
//-----------------------------------------------------------------------------
    pr_it = 0;
}

static node_t *nf;
static int fn_cnt;

Private double minR_path2 (node_t *nt)
{
    int i, j;
    double res;
    node_t *on;
    element_r *con;

    if (nt -> flag & 8) return (nt -> help); /* ready */
    if (!nf) return (-1);

    while (nf != nt) {
	for (con = nf -> con[0]; con; con = NEXT (con, nf)) {
	    on = OTHER (con, nf);
	    if ((on -> flag & (8+5)) == 5 && con -> val > 0) { /* not ready frontnode */
		res = 1 / con -> val + nf -> help; /* res > 0 */
		if (res < on -> help) on -> help = res;
	    }
	}
        if (fn_cnt) {
            res = frontNodes[j = 0] -> help;
            for (i = 0; ++i < fn_cnt;) {
                if (frontNodes[i] -> help < res) res = frontNodes[j = i] -> help;
            }
            nf = frontNodes[j];
            nf -> flag |= 8; /* ready */
	    if (--fn_cnt != j) {
		frontNodes[j] = frontNodes[fn_cnt];
		frontNodes[fn_cnt] = nf;
	    }
        }
        else { nf = NULL; return (-1); }
    }
    return (nt -> help);
}

Private void checkMaxParRes (node_t *n)
{
    double res;
    node_t *on; element_r *con;
    int i, j, n_cnt, cnt;

for (j = 1; j < QN_cnt; ++j) {

    cnt = n -> res_cnt;
    frontNodes = NEW (node_t *, cnt);
    n -> flag = 9; /* done */

    res = 0;
    i = n_cnt = 0;
    for (con = n -> con[0]; con; con = NEXT (con, n)) {
	on = OTHER (con, n);
	if (con->val > 0) {
	    on -> help = 1 / con -> val;
	    on -> flag |= 5;
	    if (!n_cnt || on -> help < res) { res = on -> help; i = n_cnt; }
	    frontNodes[ n_cnt++ ] = on;
	}
	else fprintf (stderr, "checkMaxParRes: con->val=%e <= 0\n", con->val);
    }

    if ((fn_cnt = n_cnt)) {
	nf = frontNodes[i];
	nf -> flag |= 8; /* ready */
	if (--fn_cnt != i) {
	    frontNodes[i] = frontNodes[fn_cnt];
	    frontNodes[fn_cnt] = nf;
	}
    }
    else nf = NULL;

    for (con = n -> con[0]; con; con = NEXT (con, n)) {
	on = OTHER (con, n);
	if ((on -> flag & 5)) { /* on is frontnode */
	    if (con->val < MIN_CONDUCTANCE) {
if (pr_it) fprintf (stderr, "checkMaxParRes%d: con->val=%e < MIN_CON\n", pr_it, con->val);
		elemDelRes (con);
		if (elim_check_again && Degree(on) < Max_degree) elimGrp_check = 0;
	    }
	    else if ((res = minR_path2 (on)) > 0) {
		if (1/con->val > res * max_par_res) {
if (pr_it) fprintf (stderr, "checkMaxParRes%d: res=%e > %e (ratio=%g)\n", pr_it, 1/con->val, res, (1/con->val)/res);
		    elemDelRes (con);
		    if (elim_check_again && Degree(on) < Max_degree) elimGrp_check = 0;
		}
	    }
	}
    }
    if (elim_check_again && Degree(n) < Max_degree) elimGrp_check = 0;

    n -> flag = 0;
    for (i = 0; i < n_cnt; ++i) frontNodes[i] -> flag = 0;
    DISPOSE (frontNodes, sizeof(node_t *) * cnt);
    n = n -> gnext;
}
}

void nqElimCluster (node_t *n, int type) /* n is first cluster node */
{
    element_r *con;
    node_t *on, *on2;
    int i, t_cnt, e_cnt;

    if (!max_elim_degree) return;

    Cluster = n -> clr;
    ASSERT (Cluster);
    Min_degree = Max_degree = max_elim_degree;

    e_cnt = t_cnt = 0;
    QN_cnt = 0;

    do {
	++t_cnt;
	if (!Keep(n)) {
	    ASSERT (Ready(n));
	    ASSERT (!n -> area);
	    nqChange (n);
	    n -> delayed = 1;
	}
    } while ((n = n -> cl_next));

    while (Min_degree < Max_degree) {
	while ((n = nqBase[Min_degree])) {
	    nqBase[Min_degree] = n -> next;
	    i = testRCelim (n);
	    if (!Keep(n)) {
		if (++e_cnt == t_cnt) { /* last node */
		    /* get the area nodes */
		    for (con = n -> con[0]; con; con = NEXT (con, n)) { on = OTHER (con, n);
			if (on -> area) { /* else 'on' is from other cluster */
			    ASSERT (!on -> flag);
			    queueNode (on);
			}
		    }
		}
		ASSERT (n -> clr == Cluster);
		removeNodeFromCluster (n, 0);
		elim (n, i);
	    }
	    else n -> delayed = 0;
	}
	Min_degree++;
    }

    i = 0; /* cluster nodes left */

    if (!Cluster -> nodes) { /* last cluster node eliminated */
	ASSERT (e_cnt == t_cnt);
    }
    else { /* not all cluster nodes eliminated */
	for (n = Cluster -> nodes; n; n = n -> cl_next) {
	    for (con = n -> con[0]; con; con = NEXT (con, n)) { on = OTHER (con, n);
		if (on -> area && !on -> flag) queueNode (on);
	    }
	}
	for (n = Cluster -> nodes; n; n = n -> cl_next) {
	    n -> delayed = 0;
	    ASSERT (!n -> flag);
	    queueNode (n); ++i;
	}
	ASSERT (i); /* cluster nodes left */
	ASSERT (e_cnt + i == t_cnt);

	if (minRes > 0) i = reducMinSepRes (i);
    }

    if (QN_cnt <= 2) {
	if (i == 1) { /* one node left in cluster */
	    if (QN_cnt != 1) {
		ASSERT (QN_cnt == 2);
		Cluster -> nodes = NULL; /* make cluster empty, do not output */
		n = QN[1];
		n -> clr = NULL;
//fprintf (stderr, "-- node (%d,%d) in elimClr a=%d t=%d\n", n->node_x/4, n->node_y/4, n->area, n->term);
	    }
#if 0
	    else {
		n = QN[0];
		if (!(n -> term & 1))
		    fprintf (stderr, "elimCluster: unconn. node (%d,%d)? msk=%d\n", n->node_x/4, n->node_y/4, n->mask);
	    }
#endif
	}
#ifdef ELIM_AREA
	if (QN_cnt == 2) { /* is on2 a ready area1 node? */
	    on2 = QN[1];
	    if (!ReadyArea1(on2)) on2 = NULL; /* no */
	    else if (i) elim (on2, 0); /* one area1 node */
	}
	else on2 = NULL;

	if (!i) { /* no nodes left, on is area node */
	    on = QN[0];
	    if (!ReadyArea1(on)) {
		if (!on2 && QN_cnt == 1 && Ready(on) && !Keep(on) && on -> res_cnt < 2) {
#if 0
		    if (on -> res_cnt == 2 && (con = on -> con[0])) {
			n = OTHER (con, on);
			if (n -> clr && (con = NEXT (con, on))) {
			    on2 = OTHER (con, on);
			    if (on2 -> clr && on2 -> clr != n -> clr)
				fprintf (stderr, "warning: on2 -> clr != n -> clr\n");
			}
		    }
#endif
		}
		else on = on2;
	    }
	    else if (on2 && Degree(on2) < Degree(on)) on = on2;
#if 1
	    else if (!on2 && Degree(on) > 2) {
		if (Degree(on) == 10) {
		    fprintf (stderr, "elimCluster: degree=%d area=%d (%d,%d)\n", Degree(on), on->area, on->node_x/4, on->node_y/4);
		    for (con = on -> con[0]; con; con = NEXT (con, on)) { n = OTHER (con, on);
			fprintf (stderr, "-- area=%d R=%d cl=%p (%d,%d)\n", n->area, Ready(n), n->clr, n->node_x/4, n->node_y/4);
		    }
		}
	    }
#endif
	    if (on) {
		ASSERT (Degree(on) > 0);
		elim (on, 0);
	    }
	}
#endif // ELIM_AREA
    }
    else if (max_par_res >= 2) {
	reducMaxParRes ();
    }
    for (i = 0; i < QN_cnt; ++i) QN[i] -> flag = 0;
}

Private int reducMinSepRes (int nr)
{
    int i;
    node_t *n, *on;
    element_r *con;

    /* join nodes that are connected by a too small resistor */
    i = QN_cnt - nr;
again:
    for (; i < QN_cnt; ++i) {
	n = QN[i];
	if (KEEP(n)) continue;
	ASSERT (!n -> area);
	for (con = n -> con[0]; con; con = NEXT (con, n)) {
	    if (con -> val > minRes) { // too much conductivity
		on = OTHER (con, n);
		ASSERT (on -> flag);
		if (on -> area) continue;
		--nr; // delete n
		if (min_sep_res > 0 && con -> val > min_sep_res) {
		    joinNodes (on, n);
		} else {
		    prepareElim (on, n);
		    if (n -> clr) removeNodeFromCluster (n, 0);
		    elim (n, 0);
		}
		QN[i] = QN[--QN_cnt];
		goto again;
	    }
	}
    }
    return (nr);
}

Private void reducMaxParRes ()
{
    int i, j;
    double minR;
    node_t *n, *on;
    element_r *con;

    frontNodes = NEW (node_t *, QN_cnt);

    for (i = 0; i < QN_cnt-1; ++i) {
	n = QN[i];
	if (i) { /* re-init */
	    for (j = 0; j < i; ++j) QN[j] -> flag = 3; /* done */
	    while (++j < QN_cnt) QN[j] -> flag = 1; /* not done */
	}
	n -> flag = 3; /* done */

	for (con = n -> con[0]; con; con = NEXT (con, n)) {
	    on = OTHER (con, n);
	    if ((on -> flag & 3) == 1) { /* path (n, on) not done */
		if (con->val < MIN_CONDUCTANCE) {
if (pr_it) fprintf (stderr, "reducMaxParRes%d: con->val=%e < MIN_CON (%d of %d)\n", pr_it, con->val, i, QN_cnt);
		    elemDelRes (con);
		}
		else if ((minR = minR_path (n, on)) > 0) {
		    if (1/con->val > minR * max_par_res) {
if (pr_it) fprintf (stderr, "reducMaxParRes%d: res=%e > %e (ratio=%g) (%d of %d)\n", pr_it, 1/con->val, minR, (1/con->val)/minR, i, QN_cnt);
			elemDelRes (con);
		    }
		}
	    }
	}
    }

    DISPOSE (frontNodes, sizeof(node_t *) * QN_cnt);
}

Private double minR_path (node_t *ns, node_t *nt)
{
    static node_t *n;
    static int f_cnt;
    int i, j, k;
    double res;
    node_t *on;
    element_r *con;

    if (ns -> flag == 3) { ns -> flag |= 8; ns -> help = 0; f_cnt = 0; n = ns; }
    else {
	if (nt -> flag & 8) return (nt -> help); /* use previous result */
	if (!n) return (-1); /* no path (along positive R's) */
    }

    while (n != nt) {
        k = 0; do {
	for (con = n -> con[k]; con; con = NEXT (con, n)) {
	    on = OTHER (con, n);
	    if ((on -> flag & 9) == 1 && con -> val > 0) {
		res = 1 / con -> val + n -> help; /* res > 0 */
		if (on -> flag < 4) {
		    frontNodes[ f_cnt++ ] = on;
		    on -> help = res;
		    on -> flag |= 4;
		}
		else if (res < on -> help) on -> help = res;
	    }
	} } while (++k < resSortTabSize);

        if (f_cnt) {
            res = frontNodes[j = 0] -> help;
            for (i = 0; ++i < f_cnt;) if (frontNodes[i] -> help < res) res = frontNodes[j = i] -> help;
            n = frontNodes[j]; frontNodes[j] = frontNodes[--f_cnt];
            n -> flag |= 8; /* min R path has been determined */
        }
        else { n = NULL; return (-1); } /* no path */
    }
    return (nt -> help);
}

void preElim (node_t *n, int k)
{
//#define PRE_ELIM_AREA
#ifdef PRE_ELIM_AREA
    node_t *on, *on2;
    element_r *con;

    if (pre_elim_area && n -> res_cnt <= 2 && (con = n -> con[k])) {
	on = OTHER (con, n);
	if ((con = NEXT (con, n))) {
	    on2 = OTHER (con, n);
	    if (ReadyArea1 (on)) {
		if (ReadyArea1 (on2)) {
		    if (on2 -> res_cnt < on -> res_cnt) on = on2;
		}
		else if (!on2 -> area) {
		    on2 -> area = 1;
		    if (on2 -> clr) removeNodeFromCluster (on2, 0);
		}
	    }
	    else if (ReadyArea1 (on2)) {
		if (!on -> area) {
		    on -> area = 1;
		    if (on -> clr) removeNodeFromCluster (on, 0);
		}
		on = on2;
	    }
	    else on = NULL;
	    con = NEXT (con, n); ASSERT (!con);
	}
	else if (!ReadyArea1 (on)) on = NULL;

	if (on) {
	    removeNodeFromCluster (n, 0);
	    elim (n, k);
	    ASSERT (Degree(on) > 0);
	    elim (on, k);
	    return;
	}
    }
#endif

    Cluster = n -> clr;

    Min_degree = Max_degree = pre_elim_degree;
    pre_elim_mode = 1;
    removeNodeFromCluster (n, 0);
    elim (n, k);

    while (Min_degree < Max_degree) {
	while ((n = nqBase[Min_degree])) { /* n is a ready node */
	    nqBase[Min_degree] = n -> next;
	    ASSERT (Min_degree > 0);
	    removeNodeFromCluster (n, 0);
//fprintf (stderr, "preElim: elim (min=%d max=%d)\n", Min_degree, Max_degree);
	    elim (n, k);
	}
	Min_degree++;
    }

    pre_elim_mode = 0;
}
