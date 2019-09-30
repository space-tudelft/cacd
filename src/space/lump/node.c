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

extern int equi_line_new;
extern int debug_readyGrp;
extern int debug_ready;
extern bool_t elim_sub_con;

static unsigned int allocs = 0;
static unsigned int frees  = 0;
static unsigned int max    = 0;
static unsigned int n_size = 0;
static int maxdeg = 0;
static int max_degree = 0;
static int useDelayList = 0;
static int debug_max_degree = 0;

static node_t * free_list = NULL;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif

Private node_t *allocNode (void);

extern long int lrand48 (void);
extern void     srand48 (long int);

#ifdef __cplusplus
  }
#endif

/*============================================================================*/
/*! @brief The number of nodes that are delayed.
 *//*=========================================================================*/

int delayCount = 0;
int delayCountMax = 0;

/*============================================================================*/
/*! @brief The number of nodes that are "stored".
 *
 *  @@: keesjan: The concept of "stored" nodes is not really clear.
 *
 *  Nodes which are delayed and have a weight larger than sneTolerance are put
 *  in a special "weight" class (the highest weight class) in the priority
 *  queue. Those nodes are said to be "stored".
 *//*=========================================================================*/

int storeCount = 0;

/*============================================================================*/
/*! @brief The number of nodes that have been eliminated.
 *
 *  This variable is only used for printing statistics.
 *//*=========================================================================*/

int elimCount = 0;

/*============================================================================*/
/*! @brief The group of the node which is eliminated.
 *
 *  This variable must be set before calling function elim.
 *//*=========================================================================*/

group_t *elim_grp;
node_t *nqDelayList = NULL;

/*============================================================================*/
/*! @brief Maximum number of nodes allowed in the priority queue.
 *
 *  This variable is one of the primary parameters of the delayed frontal
 *  elimination scheme.
 *//*=========================================================================*/

static int maxDelayed = 500;

/*============================================================================*/
/*! @brief The priority queue data structure.
 *
 *  The priority queue data structure is, physically, an array of
 *  <code>nqSize</code> pointers to arrays of <code>nqSizeW</code> pointers to
 *  nodes. The variable <code>nqBase</code> is a pointer to this data structure.
 *
 *  @see nqInit
 *//*=========================================================================*/

static node_t *** nqBase = NULL;
static int nqSize = 100, nqSizeW = 1, maxPosW = 0;
static int nqSizeN;

static int cnt_elim;
static int pre_elim_degree = 3;
int artReduc1 = 0;
int artReduc2 = 0;

/* Init the node module, also for each new cell.
 */
void nqInit ()
{
    int i;

    srand48 (1L); /* init random node id's */

    delayCount = 0;

    if (nqBase) { /* not the first nqInit */
	for (i = 0; i < nqSizeN; i++) if (nqBase[0][i]) ASSERT (0);
    }
    else {
	maxDelayed = paramLookupI ("max_delayed", "100000");
	if (maxDelayed > 10000000) {
	    paramError ("max_delayed", "too big value (using 10000000)");
	    maxDelayed = 10000000;
	}
	if (maxDelayed < 0) maxDelayed = 0;

	pre_elim_degree = paramLookupI ("pre_elim_degree", "7");
	if (pre_elim_degree > 20) {
	    paramError ("pre_elim_degree", "too big value (using 20)");
	    pre_elim_degree = 20;
	}

	debug_max_degree = paramLookupB ("debug.max_degree", "off");
	max_degree = paramLookupI ("max_degree", "500");
	if (max_degree > 2000) {
	    paramError ("max_degree", "too big value (using 2000)");
	    max_degree = 2000;
	}
	else if (max_degree < pre_elim_degree) max_degree = pre_elim_degree;
	if (max_degree < 0) max_degree = 0;
	if (debug_max_degree) fprintf (stderr, "ready_grp: max_degree=%d maxDelayed=%d\n", max_degree, maxDelayed);

	if (artReduc && min_art_degree > 1) artReduc1 = 1;
	if (artReduc && min_art_degree > 2 && !equi_line_new) artReduc2 = 1;
#ifdef SNE
	if (sneOmega > 0) {
	    nqSizeW = sneResolution + 1;
	    maxPosW = sneResolution;
	    ASSERT (maxPosW > 0);
	    useDelayList = 1;
	}
#endif
	if (elim_sub_con) useDelayList = 1;
	nqSize = max_degree + 2;
	nqSizeN = nqSizeW * nqSize;
	ASSERT (nqSizeN > 0);
	nqBase    = NEW (node_t **, nqSizeW);
	nqBase[0] = NEW (node_t *, nqSizeN);
	for (i = 1; i < nqSizeW; i++) nqBase[i] = nqBase[0] + i * nqSize;
	for (i = 0; i < nqSizeN; i++) nqBase[0][i] = NULL;
    }
}

static int i_iter;

/* nqFirst and nqNext iterate over all nodes that are in the node queue.
 * If the iteration is destructive (i.e. if there are elements deleted
 * from the list, as in nqInit () above), the iteration must be as in
 *     while (n = nqFirst ()) printNode (n);
 *
 * Otherwise it must be used as in:
 *    for (n = nqFirst (); n; n = nqNext ()) printNode (n);
 *
 * But nqFirst and nqNext return NULL upon end of the iteration.
 */
node_t * nqFirst ()
{
    i_iter = 0;
    return nqNext ();
}

/* See the comment on nqFirst
 */
node_t * nqNext ()
{
    node_t * n;
    static node_t * next = 0;

    if (i_iter == nqSizeN) return (NULL);

    if (i_iter == 0 || next == NULL) {
	while (nqBase[0][i_iter] == NULL) {
	    i_iter++;
	    /* also return the nodes that are not delayed */
	    if (i_iter == nqSizeN) return (NULL);
	}
	next = nqBase[0][i_iter];
    }

    ASSERT (next);
    n = next;
    next = n -> next;
    if (next == NULL) i_iter++;
    return (n);
}

/*============================================================================*/
/*! @brief Delay the elimination of a node.

    This function places the given node, <code>n</code> in the priority queue,
    so that it can be eliminated later. If, after insertion, the queue
    contains more than <code>maxDelayed</code> delayed nodes, then the node
    with the smallest weight will be removed from the queue, and eliminated.
*//*==========================================================================*/

/* Delay the elimination of a node.
 */
void nqDelayNode (node_t *n) /* used in readyGroup */
{
#ifdef SNE
    if (sneOmega > 0) n -> weight = updateWeight (n);
#endif
    n -> delayed = 1;
    nqInsert (n);
    if (++delayCount > delayCountMax) delayCountMax = delayCount;
}

void nqDelayNodes ()
{
    register node_t *n;

    for (n = nqDelayList; n; n = n -> next) {
#ifdef SNE
	if (sneOmega > 0) n -> weight = updateWeight (n);
#endif
	n -> delayed = 1;
	nqInsert (n);
    }
    nqDelayList = NULL;
}

void nqDelayElim (node_t *n, int i) /* used in readyNode */
{
    static node_t *on, *on2;
    register int j;
    element_r *con;

    nqDelayNode (n);

    /* Try to pre-eliminate ready nodes.
     */
    for (j = 0; j <= pre_elim_degree; ++j) {
	while ((n = nqBase[0][j])) {
	    i = testRCelim (n);
	    nqDelete (n); n -> delayed = 0; delayCount--;
	    if (!n -> keep) {
		elim_grp = Grp(n);
		on = NULL;
		if (n -> res_cnt == 2) {
		    if (artReduc2 && (con = n -> con[i])) {
			on = OTHER (con, n);
			if (on -> area == 1 && Grp(on) == elim_grp && (con = NEXT (con, n))) {
			    on2 = OTHER (con, n);
			    if (on2 -> area != 1 || Grp(on2) != elim_grp) on = NULL;
			}
			else on = NULL;
		    }
		}
//#if 0
		else if (n -> res_cnt == 1 && artReduc1 && (con = n -> con[i])) {
		    on = OTHER (con, n);
		    if (on -> area != 1 || Grp(on) != elim_grp) on = NULL; else on2 = NULL;
		}
//#endif
		elim (n, i); j = 0;
		if (on) {
#define ReadyNode(n) !n -> subs
		    if (!on2) {
			if (ReadyNode(on) && !on -> term && !on -> delayed) {
			    on -> area = 0; nqDelayNode (on);
			}
		    }
		    else {
			if (ReadyNode(on) && !on -> term && !on -> delayed) {
			    on -> keep = 0;
			    i = testRCelim (on);
			    if (!on -> keep) { elim (on, i); on = NULL; }
			}
			if (on) { on = on2;
			if (ReadyNode(on) && !on -> term && !on -> delayed) {
			    on -> keep = 0;
			    i = testRCelim (on);
			    if (!on -> keep) elim (on, i);
			} }
		    }
		}
	    }
	}
    }

    while (delayCount > maxDelayed + storeCount) {
	/* Too many delayed nodes.
	 * Get a delayed node with lowest degree from priority queue
	 * and eliminate this node.
	 */
	i = 0;
	do {
	    for (j = 0; j <= max_degree; ++j) {
		if ((n = nqBase[i][j])) goto found;
	    }
	} while (++i < maxPosW);
	break;
found:
	nqDelete (n); n -> delayed = 0; delayCount--;
	i = testRCelim (n);
	if (!n -> keep) {
	    if (debug_readyGrp && (++cnt_elim % 10) == 0) {
		fprintf(stderr, ".");
		if (cnt_elim == 1000) { cnt_elim = 0; fprintf(stderr, "\n"); }
	    }
	    elim_grp = Grp(n);
	    elim (n, i);
	}
    }

    if (nqDelayList) nqDelayNodes ();
}

/* Eliminate all delayed nodes of a group.
 * For each of these nodes, nqChange() must have been done.
 */
int nqEliminateGroup (node_t **qn, int n_cnt)
{
    node_t *n;
    register int j, i, d_cnt;
    int dc;

    cnt_elim = 0;
    dc = delay_cnt;
    d_cnt = debug_ready ? delay_cnt : 0;
    for (;;) {
	/* Get a delayed node with the lowest degree of group <elim_grp>.
	 */
	i = 0;
	do {
	    for (j = 0; j <= max_degree; ++j) {
		if ((n = nqBase[i][j]) && Grp(n) == elim_grp) goto found;
	    }
	} while (++i < maxPosW);
	ASSERT (j == max_degree + 1);

	if (debug_max_degree)
	    fprintf (stderr, "ready_grp: eliminateGroup: n_cnt=%d delay_cnt=%d DC=%d (elim_cnt=%d) no_elim\n",
		n_cnt, delay_cnt, delayCount, dc - delay_cnt);
	i = 0;
	do {
	    for (n = nqBase[i][j]; n; n = n -> next) {
		if (Grp(n) == elim_grp) {
		    nqDelete (n); n -> delayed = 0; delayCount--;
		    n -> term = 1; qn[n_cnt++] = n; /* no_elim */
		    if (--delay_cnt == 0) return (n_cnt);
		}
		else break;
	    }
	} while (++i < maxPosW);
#ifdef SNE
	/* If (sneOmega > 0) i == maxPosW contains nodes with
	 * weight >= sneTolerance, which may not be eliminated.
	 */
	if (sneOmega > 0) {
	    for (j = 0; j < nqSize; ++j) {
		for (n = nqBase[i][j]; n; n = n -> next) {
		    if (Grp(n) == elim_grp) {
			nqDelete (n); n -> delayed = 0; delayCount--;
			Debug (fprintf (stderr, "node id=%d w=%d d=%d x=%d y=%d DC=%d SC=%d no_elim\n",
			    n -> id, i, n -> res_cnt, n -> node_x, n -> node_y, delayCount, storeCount));
			n -> term = 1; qn[n_cnt++] = n; /* no_elim */
			if (printElimCount)
			    fprintf (stderr, "ready_grp: n_cnt=%d elim_cnt=%d no_elim of node x,y=%d,%d w=%d\n",
				n_cnt, ++elimCount, n -> node_x/4, n -> node_y/4, i);
			if (--delay_cnt == 0) return (n_cnt);
		    }
		    else break;
		}
	    }
	}
#endif
	break;
found:
	if (delay_cnt == d_cnt) {
	    if (maxPosW > 1)
		fprintf (stderr, "ready_grp: n_cnt=%d eliminateGroup: delay_cnt=%d degree=%d i=%d\n", n_cnt, d_cnt, j, i);
	    else
		fprintf (stderr, "ready_grp: n_cnt=%d eliminateGroup: delay_cnt=%d degree=%d\n", n_cnt, d_cnt, j);
	    if (d_cnt > 1000) d_cnt -= 1000;
	    else if (d_cnt > 100) d_cnt -= 100;
	}
	nqDelete (n); n -> delayed = 0; delayCount--;
	i = testRCelim (n);
	if (!n -> keep) elim (n, i);
	else qn[n_cnt++] = n; /* no_elim */
	if (--delay_cnt == 0) return (n_cnt);
    }
    ASSERT (delay_cnt == 0);
    return (n_cnt);
}

Private int art_degree (node_t *n)
{
    register element_r *con, *el;
    node_t *on, *obn;
    int i, j, k;

    if (min_art_degree < 2) return 2;

    i = 0;
    while (!n -> con[i] && i+1 < resSortTabSize) ++i;
    j = 0;
    for (con = n -> con[i]; con; con = NEXT (con, n)) {
	on = OTHER (con, n); on -> flag = 1; ++j;
    }
    k = j < n -> res_cnt ? 2 : 0;
    for (con = n -> con[i]; con; con = NEXT (con, n)) {
	on = OTHER (con, n);
	if (on -> flag) { on -> flag = 0;
	    if (k < 2) { ++k;
		for (el = on -> con[i]; el; el = NEXT (el, on)) {
		    obn = OTHER (el, on);
		    if (obn -> flag) obn -> flag = 0;
		}
	    }
	}
    }
    return k;
}

Private void fix_nodes (node_t *n)
{
    register element_r *con;
    node_t *on;
    int i;

    n -> term = 2;
    i = 0;
    while (!n -> con[i] && i+1 < resSortTabSize) ++i;
    for (con = n -> con[i]; con; con = NEXT (con, n)) {
	on = OTHER (con, n);
	if (on -> term) on -> term = 2;
    }
}

Private int node_eliminated (node_t *n)
{
    register element_r *con;
    node_t *on1, *on2;
    int i;

    if (min_art_degree < 2) return 0;

    n -> keep = 0;
    i = testRCelim (n);
    if (n -> keep) return 0;

    on1 = on2 = NULL;
    for (con = n -> con[i]; con; con = NEXT (con, n)) {
	     if (!on1) on1 = OTHER (con, n);
	else if (!on2) on2 = OTHER (con, n);
	else return 0;
    }
    if (on2) {
	if ((on1 -> node_x == n -> node_x && on2 -> node_x == n -> node_x)
	 || (on1 -> node_y == n -> node_y && on2 -> node_y == n -> node_y)) {
	    n -> term = 0;
	    elim (n, i);
	    return 1;
	}
    }
    return 0;
}

int nqEliminateAreaNodes (node_t **qn, int n_cnt)
{
    node_t *n;
    int i, j;

    delay_cnt = n_cnt;
    for (i = 0; i < n_cnt; i++) {
	n = qn[i];
	if (n -> area == 1) {
	    n -> area = 3;
	    n -> delayed = 1; nqInsert (n);
	    qn[i--] = qn[--n_cnt];
	}
    }
    delay_cnt -= n_cnt;
    if (!delay_cnt) return (n_cnt);

again:
    for (j = 0; j <= max_degree; ++j) {
	for (n = nqBase[0][j]; n; n = n -> next) {
	    if (n -> area == 3) {
		if (n -> term) { /* special area=1 node */
		    if (j > 1 && art_degree(n) > 1) continue;
		    n -> term = 0;
		}
		nqDelete (n); n -> delayed = 0;

		if (j < 2) { if (j >= min_art_degree) goto keep; }
		else if (j >= min_degree || min_art_degree <= 2) goto keep;

		n -> keep = 0;
		i = testRCelim (n);
		if (!n -> keep) {
		    elim (n, i);
		    if (--delay_cnt == 0) return (n_cnt);
		    goto again;
		}
keep:
		n -> area = 1;
		qn[n_cnt++] = n; /* no_elim */
		if (--delay_cnt == 0) return (n_cnt);
	    }
	}
    }
    if (delay_cnt > 0) {
	for (j = 2; j < nqSize; ++j) {
again2:
	    for (n = nqBase[0][j]; n; n = n -> next) {
		if (n -> area == 3) {
		    nqDelete (n); n -> delayed = 0;
		    if (j == 2 && node_eliminated (n)) {
			if (--delay_cnt == 0) return (n_cnt);
			goto again2;
		    }
		    n -> area = 1;
#if 0
		    fix_nodes (n);
#else
		    n -> term = 2;
#endif
		    qn[n_cnt++] = n; /* no_elim */
		    if (--delay_cnt == 0) return (n_cnt);
		}
	    }
	}
    }
    ASSERT (delay_cnt == 0);
    return (n_cnt);
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
    n -> keep = 0;
    n -> help = 0;
    n -> delayed = 0;
    n -> names = NULL;
    n -> netEq = NULL;
    n -> subs = NULL;
    n -> area = 0;
    n -> res_cnt = 0;
    n -> cap_cnt = 0;
    n -> n_n_cnt = 0;
    n -> degree = 0;
    n -> pols = NULL;
 /* n -> grp = NULL; ** grp not known (is set outside createNode) */
#ifdef MOMENTS
    n -> moments = NULL;
    n -> moments2 = NULL;
#endif
    n -> weight = 0;

    n -> node_h = 0;
    n -> node_w = 0;

    /* lrand48 returns a long but, if the sequence is truly random,
     * it is safe to truncate if an int is of lower precision.
     */
    n -> id = (int) lrand48 ();

    n -> prev = NULL; /* see nqInsert */

    n -> term = optFineNtw ? 2 : 0;

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
    ASSERT (!n -> prev); // node must not be in queue

    currIntNod--;

#ifdef MOMENTS
    if (n -> moments ) DISPOSE (n -> moments , sizeof(double) * extraMoments);
    if (n -> moments2) DISPOSE (n -> moments2, sizeof(double) * extraMoments);
#endif
    frees++;
    n -> next = free_list;
    free_list = n;
}

void nodeStatistics (FILE *fp)
{
    fprintf (fp, "overall node statistics:\n");
    fprintf (fp, "\tnodes created      : %u\n", allocs);
    fprintf (fp, "\tnodes disposed     : %u\n", frees);
    fprintf (fp, "\tnodes allocated    : %u of %u byte\n", max, n_size);
 // fprintf (fp, "\tmax nodes in core  : %u\n", max);
    fprintf (fp, "\tmax nodes delayed  : %d\n", delayCountMax);
    fprintf (fp, "\tmax node degree    : %d\n", maxdeg);
}

/* Node must be set delayed on new degree position.
 */
void nqSetDegree (node_t *n)
{
    n -> delayed = 1;
    if (Grp(n) != elim_grp) {
	if (useDelayList) {
	    nqDelete (n); n -> delayed = 0;
	    n -> next = nqDelayList; nqDelayList = n;
	}
    }
    else {
#ifdef SNE
	if (sneOmega > 0 && n -> area != 3) {
	    int weight = updateWeight (n);
	    if (weight != n -> weight) {
		nqDelete (n);
		n -> weight = weight;
		nqInsert (n);
		return;
	    }
	}
#endif
	if (n -> res_cnt != n -> degree) nqChange (n);
    }
}

/* Insert a delayed node in the priority queue.
 */
void nqInsert (node_t *n)
{
    node_t *b;
    int pos, posW = 0;

    pos = n -> res_cnt;
    if (pos > maxdeg) maxdeg = pos;
    if (pos > max_degree) pos = max_degree + 1;
    n -> degree = pos;
    ASSERT (!n -> prev); // node must not be in queue
#ifdef SNE
    if (sneOmega > 0) if ((posW = n -> weight) == maxPosW) ++storeCount;
#endif
    /* link it into new proper list */
    if ((b = nqBase[posW][pos])) b -> prev = n;
    nqBase[posW][pos] = n;
    n -> next = b;
    n -> prev = n;
}

/* Delete a delayed node from the priority queue.
 */
void nqDelete (node_t *n)
{
    node_t *b;
    int pos = n -> degree;
    int posW = 0;

#ifdef SNE
    if (sneOmega > 0) if ((posW = n -> weight) == maxPosW) --storeCount;
#endif
    /* unlink from present position */
    if (nqBase[posW][pos] == n) nqBase[posW][pos] = n -> next;
    else {
	b = n -> prev;
	ASSERT (b && b -> next == n);
	if (n -> next) n -> next -> prev = b;
	b -> next = n -> next;
    }
    n -> prev = NULL; // node not more in queue
}

/* This function must only be called for delayed nodes.
 * Only delayed nodes are in the node priority queue (nq).
 *
 * Needs to be called when the number of resistances of the node change.
 * In that case the node degree change and it must get another position.
 *
 * Also called to improve the position of delayed nodes in a ready group.
 * This is implemented by putting the nodes in the front of each list.
 * Function nqEliminateGroup then can easy find the lowest degree nodes.
 */
void nqChange (node_t *n)
{
    node_t *b;
    int pos = n -> degree;
    int posW = 0;

    /* nqDelete (n) */
#ifdef SNE
    if (sneOmega > 0) posW = n -> weight;
#endif
    /* unlink n from present position */
    if (nqBase[posW][pos] == n) nqBase[posW][pos] = n -> next;
    else {
	b = n -> prev;
	ASSERT (b && b -> next == n);
	if (n -> next) n -> next -> prev = b;
	b -> next = n -> next;
    }

    /* nqInsert (n) */
    if (pos != n -> res_cnt) {
	pos = n -> res_cnt;
	if (pos > maxdeg) maxdeg = pos;
	if (pos > max_degree) pos = max_degree + 1;
	n -> degree = pos;
    }
    /* link n into the head position */
    if ((b = nqBase[posW][pos])) b -> prev = n;
    nqBase[posW][pos] = n;
    n -> next = b;
    n -> prev = n;
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

void nqDumpQueue (void) /* only delayed nodes */
{
    int i, j;
    node_t *n;

    fprintf (stderr, "debug: queue:\n");
    for (i = 0; i < nqSizeW; i++) {
        for (j = 0; j < nqSize; j++) {
            for (n = nqBase[i][j]; n; n = n -> next) {
                fprintf (stderr, "Queue (%d,%d) Node id=%d w=%d\n", i, j, n -> id, n -> weight);
            }
	}
    }
}
