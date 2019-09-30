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
#include <math.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/lump/define.h"
#include "src/space/lump/extern.h"

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif

Private void elimCAP (node_t *n);
Private void elimRES (node_t *n, int rx);
#ifdef MOMENTS
Private void momentsElimCAP (node_t *nodeK);
Private void momentsElimRES (node_t *nodeK, int rx);
#ifdef SNE
Private double momentsWeight (node_t *nodeK);
Private double setWeightNorm3 (node_t *n);
Private double calculateBranchWeight (double *Mold, double *Mshunt, double oldWeight);
#endif
#endif /* MOMENTS */

#ifdef HISTOGRAM
Private void addHisto (int R_cnt, int C_cnt);
#endif

#ifdef __cplusplus
  }
#endif

#ifdef MOMENTS
extern bool_t sneResetMoments;
#ifdef SNE
extern double sneOmega2;
#endif
#endif /* MOMENTS */
extern double MIN_CONDUCTANCE;
extern int do_min_res;

static int cx_testRC;

/*============================================================================*/
/*! @brief Eliminate a node from the network.

    This function eliminates a given node from the network. In all cases is
    the node deleted from the network.

    Note: the function simply calls momentsElim if "moment extraction"
    is turned on (when the SNE method is active, for example).

    @param n The node to be eliminated.
*//*==========================================================================*/

void elim (node_t *n, int rx)
{
    ASSERT (!n -> netEq);
    ASSERT (!n -> names);
    ASSERT (!n -> term);
 /* ASSERT (!n -> keep); SdeG: checked before */
 /* ASSERT (n != nSUB); SdeG: is term node */

#ifdef SNE
    Debug (fprintf (stderr, "elim %d w=%d n=%d r=%d c=%d x=%d y=%d DC=%d SC=%d\n",
	n -> id, n -> weight, n -> degree, n -> res_cnt, n -> cap_cnt,
	n -> node_x, n -> node_y, delayCount, storeCount));

    if (printElimCount) fprintf (stderr, "cnt=%d w=%d elim node x,y=%d,%d\n",
				++elimCount, n -> weight, n->node_x/4, n->node_y/4);
#endif
    if (rx >= 0)
	elimRES (n, rx);
    else
	elimCAP (n); /* SdeG: After testRCelim */

    ASSERT (n -> res_cnt == 0 && n -> cap_cnt == 0);
    eliNod++;
    nodeDel (n);
}

Private void elimRES (node_t *n, int rx)
{
    node_t **nodes, *on;
    element_c *cap;
    element_r *con;
    double subconval, totconval, maxabscon, usedtotconval, frag_con;
    double *value, v;
    int i, j, N, N_cnt, R_cnt;

    R_cnt = n -> res_cnt;
    if ((subconval = n -> substrCon[rx]) != 0) ++R_cnt;
    ASSERT (R_cnt > 0);

#ifdef HISTOGRAM
    if (optHisto) addHisto (R_cnt, n -> cap_cnt);
#endif

#ifdef MOMENTS
    if (maxMoments > 0) { momentsElimRES (n, rx); return; }
#endif

    N_cnt = R_cnt + n -> cap_cnt;
    nodes = NEW (node_t*, N_cnt);
    value = NEW (double, R_cnt);

    totconval = subconval;
    if ((maxabscon = subconval) < 0) maxabscon = -subconval;
    N = 0;
    for (con = n -> con[rx]; con; con = NEXT (con, n)) {
	nodes[N] = OTHER (con, n);
	if (nodes[N] -> delayed) nodes[N] -> delayed = 2;
	value[N++] = v = con -> val;
	totconval += v;
	if (v < 0) v = -v;
	if (v > maxabscon) maxabscon = v;
	elemDelRes (con);
    }
    if (subconval != 0) {
	nodes[N] = nSUB;
	value[N++] = subconval;
    }
    ASSERT (N == R_cnt);
    ASSERT (totconval != 0); // RES

    /* redistribute GND, SUBSTR and coupling capacitances */
    for (i = 0; i < capSortTabSize; i++) {
	if ((cap = n -> cap[i])) {
	    /*	Since a subdivision of cap -> val into too many fragments
		will significantly slow down the program, we only use
		conductances >= frag_con * max. conductance to
		redistribute the coupling capacitance cap -> val.
	    */
	    frag_con = capAreaPerimEnable[i] ? frag_coup_area : frag_coup_cap;
	    frag_con *= maxabscon;
	    if (frag_con < MIN_CONDUCTANCE) frag_con = MIN_CONDUCTANCE;

	    usedtotconval = 0;
	    if ((v = n -> gndCap[i]) != 0) v /= totconval;
	    for (j = 0; j < R_cnt; ++j) {
		if (Abs (value[j]) >= frag_con) { usedtotconval += value[j]; nodes[j] -> flag2 = 1; }
		else nodes[j] -> flag2 = 0;
		if (v != 0) nodes[j] -> gndCap[i] += v * value[j];
	    }

	    do {
		on = OTHER (cap, n);
		if (on -> delayed == 1) { on -> delayed = 2; nodes[N++] = on; }
		if (usedtotconval != 0) {
		    v = cap -> val / usedtotconval;
		    for (j = 0; j < R_cnt; ++j) {
			if (nodes[j] != on && nodes[j] -> flag2) {
			    elemAddCap (nodes[j], on, v * value[j], i, NULL);
			}
		    }
		}
		elemDelCap (cap);
	    } while ((cap = NEXT (cap, n)));
	}
	else if ((v = n -> gndCap[i]) != 0) {
	    v /= totconval;
	    for (j = 0; j < R_cnt; ++j) nodes[j] -> gndCap[i] += v * value[j];
	}
	if ((v = n -> substrCap[i]) != 0) {
	    v /= totconval;
	    for (j = 0; j < R_cnt; ++j) nodes[j] -> substrCap[i] += v * value[j];
	}
    }

    for (i = 0; i < R_cnt; ++i) {
	on = nodes[i];
	v = value[i] / totconval;
	for (j = i; ++j < R_cnt;) {
	    /* The use of type 'S' is save also for type 'G'.
	     * Function elemNew shall changed the type to 'G'
	     * when both nodes are in the same group.
	     */
	    elemAddRes ('S', on, nodes[j], v * value[j], rx);
	}
	if (on -> delayed) nqSetDegree (on); // and delayed = 1
	else if (do_min_res) {
	    if (!on -> keep && Grp(on) == elim_grp) {
		if (on -> degree < do_min_res) do_min_res = on -> degree;
	    }
	}
    }
    for (; i < N; ++i) nqSetDegree (nodes[i]); // and delayed = 1

    DISPOSE (value, R_cnt);
    DISPOSE (nodes, N_cnt);
}

void elimCAP (node_t *n)
{
    node_t *on;
    element_c *cap, *next_cap, *cap2;
    double totcapval, v, g, s;
    int cx;

#ifdef HISTOGRAM
    if (optHisto) addHisto (0, n -> cap_cnt);
#endif

    /*	No resistances are connected to this node.
	So we perform a star-triangle transformation for the node
	where the elements are capacitors.
	Before elimination we already tested that only ONE type of
	capacitance is connected to this node (with testRCelim).
    */
    cx = cx_testRC;

    if (cx >= 0) {
#ifdef MOMENTS
	if (maxMoments > 0 && cx == 0) { momentsElimCAP (n); return; }
#endif
	totcapval = g = n -> gndCap[cx];
	for (cap = n -> cap[cx]; cap; cap = NEXT (cap, n)) {
	    totcapval += cap -> val;
	    on = OTHER (cap, n);
	    if (on -> delayed == 1) on -> delayed = 2;
	}
	if ((s = n -> substrCap[cx]) != 0) totcapval += s;

	for (cap = n -> cap[cx]; cap; cap = next_cap) {
	    on = OTHER (cap, n);
	    next_cap = NEXT (cap, n);
	    if ((v = cap -> val / totcapval) != 0) {
		for (cap2 = next_cap; cap2; cap2 = NEXT (cap2, n)) {
		    elemAddCap (on, OTHER (cap2, n), v * cap2 -> val, cx, NULL);
		}
		if (s != 0) on -> substrCap[cx] += v * s;
		if (g != 0) on -> gndCap[cx]    += v * g;
	    }
	    elemDelCap (cap);
	    if (on -> delayed) nqSetDegree (on); // and delayed = 1
	}
	if (s != 0) {
	    if (g != 0) nSUB -> gndCap[cx] += s / totcapval * g;
	}
    }
}

#ifdef HISTOGRAM
Private void addHisto (int R_cnt, int C_cnt)
{
    int histoVal, i;
    if (optCoupCap)
	histoVal = R_cnt * R_cnt + (C_cnt + 1) * R_cnt;
    else
	histoVal = R_cnt;
    if (histoVal > histoMaxVal) histoMaxVal = histoVal;
    i = histoVal / histoBucket;
    if (i >= histoSize) i = histoSize - 1;
    histogram[i]++;
}
#endif

/*============================================================================*/
/*! @brief Test whether a node should (or can) be eliminated.
 *
 *  This function sets the field `keep' of the given node to 1 if the
 *  node should NOT be eliminated.
 *
 *  The logic exercised by this function is different for the Elmore Preserving
 *  Reduction (EPR) method as opposed to moment-based elimination (used when
 *  SNE is active, for example).
 *
 *  For the EPR:
 *      A node should NOT be eliminated when it has more than one unique
 *      "sort" of resistance connected to it, or more than one unique sort of
 *      capacitance AND no resistors (actually, the logic is slightly more
 *      complicated, see the code below). Having one sort of resistance, and
 *      different sorts of capacitance is allowed, since, during EPR reduction,
 *      capacitances are distributed over neighboring nodes first.
 *
 *  For moment-based elimination:
 *      A node should NOT be eliminated when it has more than one unique
 *      "sort" of resistance or more than one "sort" of capacitance.
 *
 *  @return The (lowest) index of the "sort" of resistance connected to the
 *  node, or -1 if there are no resistances connected.
 *//*=========================================================================*/

int testRCelim (node_t *n)
{
    int k, cx;

    if (optRes) {
      if (resSortTabSize == 1) {
        if (n -> con[0] || n -> substrCon[0] != 0) return (0);
      }
      else {
	cx = -1;
	for (k = 0; k < resSortTabSize; k++) {
	    if (n -> con[k] || n -> substrCon[k] != 0) {
		if (cx < 0) cx = k;
		else { /* two or more different types of resistors */
		    n -> keep = 1; /* are connected to this node */
		    return (cx);
		}
	    }
	}
	if (cx >= 0) return (cx); /* a resistor found */
      }
    }

    /* no resistors */
    cx_testRC = k = -1;
    while (++k < capSortTabSize) {
	if (n -> cap[k] || n -> gndCap[k] != 0 || n -> substrCap[k] != 0) {
	    if (cx_testRC >= 0 || capPolarityTab[k] != 'x') {
		    /* two different types of capacitors */
		    /* or 'polar' capacitors */
		n -> keep = 1; /* are connected to this node */
		return (-1);
	    }
	    cx_testRC = k;
	}
    }
    return (-1);
}

#ifdef MOMENTS

#ifdef SNE
int updateWeight (node_t *n) /* sneOmega > 0 */
{
    int posW;
    double weight = sneNorm == 3 ? setWeightNorm3 (n) : momentsWeight (n);

    if (weight >= sneTolerance) posW = sneResolution;
    else if (sneResolution > 1) {
	posW = (int) (weight * sneNormedResolution);
	ASSERT (posW >= 0 && posW < sneResolution);
    }
    else posW = 0;
    return posW;
}

Private double setWeightNorm3 (node_t *n)
{
    element_c *cap;
    element_r *con;
    double sumCap, sumCon;
    int rx, i;

    rx = -1;
    for (i = 0; i < resSortTabSize; i++) {
	if (n -> con[i] || n -> substrCon[i] != 0) {
	    ASSERT (rx < 0);
	    rx = i;
	}
    }
    if (rx < 0) return (0);

    sumCap = 0;
    for (i = 0; i < capSortTabSize; ++i) {
	if (n -> gndCap[i] != 0) sumCap += n -> gndCap[i];
	if (n -> substrCap[i] != 0) sumCap += n -> substrCap[i];
	for (cap = n -> cap[i]; cap; cap = NEXT (cap, n)) sumCap += cap -> val;
    }
    if (sumCap == 0) return (0);

    sumCon = n -> substrCon[rx];
    for (con = n -> con[rx]; con; con = NEXT (con, n)) sumCon += con -> val;
    ASSERT (sumCon != 0);
    return (sneOmega * sumCap / sumCon);
}
#endif

/*============================================================================*/
/*! @brief Eliminate a node using higher order moments, or update a node's weight.

    This function has two modes, depending on the flag <code>doWeight</code>.

    If <code>doWeight</code> is <code>0</code>, then this function eliminates
    the node given by the argument <code>nodeK</code>. The suffix "k" refers
    to the notation in the paper [EliasDac96].

    This function should never be called with <code>doWeight == 1</code> from
    the "outside". This function calls itself with <code>doWeight == 1</code>
    to update the node weight of node k, where the "weight" is understood as
    the relative error associated with the (imaginary) elimination of node k.
    The concept of relative error is described in the paper.

    @param nodeK    The node to be eliminated.
*//*==========================================================================*/

Private void momentsElimRES (node_t *nodeK, int rx)
{
    element_c *cap;
    element_r *con;
    node_t *n, *on, **nodes;
    double **Mk;
    double Mij_[MAXMOMENT+1];
    double sumMkn[MAXMOMENT+1];
    double v, subcap, subcon, *moments;
    int cx, cx2, i, j, l, m, p, N, ND;
    int max, N_cnt;

 // ASSERT (maxMoments > 0 && maxMoments <= MAXMOMENT);

    /* sumMkn[0] is denominator of eq 9,
     * sumMkn[1] is denominator of eq 19,
     * sumMkn[i] are used in last part of numerator
     */

    /* Find the capacitance type cx of this node. (It must be ZERO.)
     * The resistance type rx >= 0 is guaranteed by the calling routine.
     */
    cx = cx2 = -1;
    for (i = 0; i < capSortTabSize; i++)
	if (nodeK -> gndCap[i] != 0 || nodeK -> cap[i] || nodeK -> substrCap[i] != 0) {
	    if (i == 0) cx = i;
	    else { cx2 = i; break; }
	}

    N_cnt = 2; /* nGND + nSUB */
    N_cnt += nodeK -> cap_cnt;
    N_cnt += nodeK -> res_cnt;
    nodes = NEW (node_t*, N_cnt);

    max = maxMoments + 1;
    Mk    = NEW (double*, N_cnt);
    Mk[0] = NEW (double, N_cnt * max);

 // nodes[0] = NULL; /* The ground node. */

    /*  Note: In the following, the suffix "k" refers to the number of the
     *  node that is to be eliminated. Note also that "k" is used only
     *  symbolically here: the symbol "k" does NOT denote an index into the
     *  array nodes[].
     *
     *  For each element, we create a new index "n" into the nodes[] array,
     *  and we then fill Mk[n][..] with the moments of the elements between
     *  nodes[n] and "node k". We also sum these moments into the array sumMkn[..].
     *
     *  The variable `N' will hold the number of nodes connected to this node.
     *
     *  Note that the actual code for doing this is somewhat bulky, mainly
     *  because of the different ways in which elements can be connected to a node.
     */

    /* First, we consider the admittance to ground.
       Note: In this implementation no ground resistances are possible, so
       the 0-th order moment of the ground-admittance will be 0.
    */
    i = 1;
    sumMkn[0] = Mk[0][0] = 0;
    if (cx < 0) Mk[0][1] = 0;
    else   if ((Mk[0][1] = nodeK -> gndCap[cx]) != 0) i = 0;
    sumMkn[1] = Mk[0][1];

    /* Capacitances may have higher order moments (extra moments)
       associated with them.
     */
    if (nodeK -> moments) {
	for (l = 2; l <= maxMoments; ++l) {
	    sumMkn[l] = Mk[0][l] = nodeK -> moments[l-2];
	    if (sumMkn[l] != 0) i = 0;
	}
    }
    else for (l = 2; l <= maxMoments; ++l) sumMkn[l] = Mk[0][l] = 0;

    N = ND = 0;

    if (cx >= 0) {
	/* Next, consider capacitances between "node k" and other nodes.
	*/
	for (cap = nodeK -> cap[cx]; cap; cap = NEXT (cap, nodeK)) {
	    N++;
	    nodes[N] = OTHER (cap, nodeK);
	    if (nodes[N] -> delayed) nodes[ND = N] -> delayed = 2;
	    Mk[N] = Mk[0] + N * max;
	    /* Capacitances may have higher order moments (extra moments) */
	    if (cap -> moments) {
		for (l = 2; l <= maxMoments; ++l) sumMkn[l] += Mk[N][l] = cap -> moments[l-2];
	    }
	    else for (l = 2; l <= maxMoments; ++l) Mk[N][l] = 0;
	    Mk[N][0] = 0;
	    sumMkn[1] += Mk[N][1] = cap -> val;
	}
	subcap = nodeK -> substrCap[cx];
    }
    else subcap = 0;

    /* Next, consider resistances between "node k" and other nodes.
    */
    for (con = nodeK -> con[rx]; con; con = NEXT (con, nodeK)) {
	on = OTHER (con, nodeK);
	if (do_min_res) {
	    if (!on -> keep && Grp(on) == elim_grp) {
		if (on -> degree < do_min_res) do_min_res = on -> degree;
	    }
	}
	for (j = 1; j <= N; ++j) if (on == nodes[j]) break;
	if (j > N) { /* new node */
	    N++;
	    nodes[N] = on;
	    Mk[N] = Mk[0] + N * max;
	    for (l = 1; l <= maxMoments; ++l) Mk[N][l] = 0;
	    if (on -> delayed) { on -> delayed = 2; ND = N; }
	}
	sumMkn[0] += Mk[j][0] = con -> val;
	elemDelRes (con);
    }
    subcon = nodeK -> substrCon[rx];

    /* Next, consider res & cap between "node k" and the substrate node.
    */
    if (subcap != 0 || subcon != 0 || nodeK -> moments2) {
	N++;
	nodes[N] = nSUB;
	Mk[N] = Mk[0] + N * max;
	sumMkn[0] += Mk[N][0] = subcon;
	sumMkn[1] += Mk[N][1] = subcap;
	if (nodeK -> moments2) {
	    for (l = 2; l <= maxMoments; ++l) sumMkn[l] += Mk[N][l] = nodeK -> moments2[l-2];
	}
	else for (l = 2; l <= maxMoments; ++l) Mk[N][l] = 0;
    }
    ASSERT (N < N_cnt);

    /*  Note: The variable `N' now holds the number of nodes connected to this
     *  node (the degree). `N' will not change hereafter.
     */
    /*
     *  In the following, Mij[l] denotes "Mij (l)" in equation (4) in the
     *  paper [EliasDac96]. The value of Mij_[l] refers to "Mij tilde (l)".
     *  The variable l0 refers to "alpha k" in the paper. (Not more used, is 0).
     *
     *  Note that Mij[...] are the old admittance moments between node i and
     *  node j, i.e., before elimination. Mij_[...] are the new moments.
     */

    ASSERT (sumMkn[0] != 0); /* RES */
    if (doElmore) sumMkn[1] = 0; /* maxMoments == 1 */
    if (cx < 0) if (i == 0 || nodeK -> moments2) cx = 0; /* noCAP && (gndMOM || subMOM) */

    moments = Mij_+2;
    Mij_[0] = 0;

    for (; i < N; i++) {
	n = nodes[i];
	for (j = i; ++j <= N;) {
	    on = nodes[j];

	    if (i) { /* RES */
		Mij_[0] = Mk[j][0] * Mk[i][0];
		if (Mij_[0] != 0) {
		    Mij_[0] /= sumMkn[0];
		    /* Add the 0-th order moment Mij_[0] to the network.
		    */
		    /* The use of type 'S' is save also for type 'G'.
		     * Function elemNew shall changed the type to 'G'
		     * when both nodes are in the same group.
		     */
		    elemAddRes ('S', n, on, Mij_[0], rx);
		}
		if (cx < 0) continue; /* noCAP && noMOM */
	    }

	    m = 0;
	    for (l = 1; l <= maxMoments; l++) {
		Mij_[l] = Mk[j][l] * Mk[i][0];
		for (p = 0; p < l; p++) {
		    Mij_[l] += Mk[j][p] * Mk[i][l-p];
		    Mij_[l] -= Mij_[p] * sumMkn[l-p];
		}
		if (Mij_[l] != 0) {
		    Mij_[l] /= sumMkn[0];
		    if ((m = l) == 1 && sneResetMoments) break;
		}
	    }
	    if (m) {
		/* Add the 1-st order moment Mij_[1] to the network.
		*/
		if (i == 0) { /* nGND cx=0 */
		    on -> gndCap[cx] += Mij_[1];
		    if (m > 1) {
			if (!on -> moments) on -> moments = newMoments (moments);
			else for (l = 0; l < extraMoments; ++l) on -> moments[l] += moments[l];
		    }
		}
		else elemAddCap (n, on, Mij_[1], cx, m > 1 ? moments : NULL);
	    }
        }
    }

    /* Delete elements connected to "node k" from the network.
     */
    for (cap = nodeK -> cap[cx]; cap; cap = NEXT (cap, nodeK)) {
	elemDelCap (cap);
    }

    if (cx2 > 0) {
	j = N;
	for (cx = cx2; cx < capSortTabSize; ++cx) {
	    if ((v = nodeK -> gndCap[cx]) != 0) {
		v /= sumMkn[0];
		for (i = 1; i <= N; ++i)
		    if (Mk[i][0] != 0) nodes[i] -> gndCap[cx] += v * Mk[i][0];
	    }
	    if ((v = nodeK -> substrCap[cx]) != 0) {
		v /= sumMkn[0];
		for (i = 1; i <= N; ++i)
		    if (Mk[i][0] != 0) nodes[i] -> substrCap[cx] += v * Mk[i][0];
	    }
	    for (cap = nodeK -> cap[cx]; cap; cap = NEXT (cap, nodeK)) {
		if ((v = cap -> val / sumMkn[0]) != 0) {
		    on = OTHER (cap, nodeK);
		    if (on -> delayed == 1) { on -> delayed = 2; nodes[++j] = on; }
		    for (i = 1; i <= N; ++i)
			if (Mk[i][0] != 0) elemAddCap (nodes[i], on, v * Mk[i][0], cx, NULL);
		}
		elemDelCap (cap);
	    }
	}
	if (j > N) ND = j;
    }

    /* Update the surrounding nodes
     */
    for (j = 1; j <= ND; ++j) {
	if (nodes[j] -> delayed) nqSetDegree (nodes[j]); // and delayed = 1
    }
    DISPOSE (Mk[0], N_cnt * max);
    DISPOSE (Mk   , N_cnt);
    DISPOSE (nodes, N_cnt);
}

Private void momentsElimCAP (node_t *nodeK)
{
    element_c *cap, *cap2;
    node_t *n, *on, **nodes;
    double **Mk;
    double Mij_[MAXMOMENT+1];
    double sumMkn[MAXMOMENT+1];
    double v, subcap, *moments;
    int cx, i, j, l, m, p, N, ND;
    int max, N_cnt;

    cx = 0; /* capacitance type (guaranteed by calling routine) */

    N_cnt = 2; /* nGND + nSUB */
    N_cnt += nodeK -> cap_cnt;
    nodes = NEW (node_t*, N_cnt);

    max = maxMoments + 1;
    Mk    = NEW (double*, N_cnt);
    Mk[0] = NEW (double, N_cnt * max);

 // nodes[0] = NULL; /* The ground node. */
    i = 1;
    if ((Mk[0][1] = nodeK -> gndCap[cx]) != 0) i = 0;
    sumMkn[1] = Mk[0][1];

    /* Capacitances may have higher order moments (extra moments)
       associated with them.
     */
    if (nodeK -> moments) {
	for (l = 2; l <= maxMoments; ++l) {
	    sumMkn[l] = Mk[0][l] = nodeK -> moments[l-2];
	    if (sumMkn[l] != 0) i = 0;
	}
    }
    else for (l = 2; l <= maxMoments; ++l) sumMkn[l] = Mk[0][l] = 0;

    N = ND = 0;

    /* Next, consider capacitances between "node k" and other nodes.
    */
    for (cap = nodeK -> cap[cx]; cap; cap = NEXT (cap, nodeK)) {
	N++;
	nodes[N] = OTHER (cap, nodeK);
	if (nodes[N] -> delayed) nodes[ND = N] -> delayed = 2;
	Mk[N] = Mk[0] + N * max;

	/* Capacitances may have higher order moments (extra moments) */
	if (cap -> moments) {
	    for (l = 2; l <= maxMoments; ++l) sumMkn[l] += Mk[N][l] = cap -> moments[l-2];
	}
	else for (l = 2; l <= maxMoments; ++l) Mk[N][l] = 0;
	sumMkn[1] += Mk[N][1] = cap -> val;
    }
    subcap = nodeK -> substrCap[cx];

    /* Next, consider cap between "node k" and the substrate node.
    */
    if (subcap != 0 || nodeK -> moments2) {
	N++;
	nodes[N] = nSUB;
	Mk[N] = Mk[0] + N * max;
	sumMkn[1] += Mk[N][1] = subcap;
	if (nodeK -> moments2) {
	    for (l = 2; l <= maxMoments; ++l) sumMkn[l] += Mk[N][l] = nodeK -> moments2[l-2];
	}
	else for (l = 2; l <= maxMoments; ++l) Mk[N][l] = 0;
    }
    ASSERT (N < N_cnt);

    ASSERT (sumMkn[1] != 0); /* only CAP */

    moments = Mij_+2;

    for (; i < N; i++) {
	n = nodes[i];
	for (j = i; ++j <= N;) {
	    on = nodes[j];
	    m = 0;
	    for (l = 1; l <= maxMoments; l++) {
		Mij_[l] = Mk[j][l] * Mk[i][1];
		for (p = 1; p < l; p++) {
		    Mij_[l] += Mk[j][p] * Mk[i][l-p+1];
		    Mij_[l] -= Mij_[p] * sumMkn[l-p+1];
		}
		if (Mij_[l] != 0) {
		    Mij_[l] /= sumMkn[1];
		    if ((m = l) == 1 && sneResetMoments) break;
		}
	    }
	    if (m) {
		/* Add the 1-st order moment Mij_[1] to the network.
		*/
		if (i == 0) { /* nGND */
		    on -> gndCap[cx] += Mij_[1];
		    if (m > 1) {
			if (!on -> moments) on -> moments = newMoments (moments);
			else for (l = 0; l < extraMoments; ++l) on -> moments[l] += moments[l];
		    }
		}
		else elemAddCap (n, on, Mij_[1], cx, m > 1 ? moments : NULL);
	    }
        }
    }

    /* Delete elements connected to "node k" from the network.
     */
    for (cap = nodeK -> cap[cx]; cap; cap = NEXT (cap, nodeK)) {
	elemDelCap (cap);
    }

    /* Update the surrounding nodes
     */
    for (j = 1; j <= ND; ++j) {
	if (nodes[j] -> delayed) nqSetDegree (nodes[j]); // and delayed = 1
    }
    DISPOSE (Mk[0], N_cnt * max);
    DISPOSE (Mk   , N_cnt);
    DISPOSE (nodes, N_cnt);
}

#ifdef SNE
Private double momentsWeight (node_t *nodeK)
{
    element_c *cap;
    element_r *con;
    node_t *n, *on, **nodes;
    double **Mk;
    double Mij_[MAXMOMENT+1];
    double sumMkn[MAXMOMENT+1];
    double weight, subcap, subcon, *Mold;
    int cx, rx, i, j, l, m, p, N, l0;
    int max, N_cnt;

 // ASSERT (maxMoments >= 2 && maxMoments <= MAXMOMENT);

    weight = 0;
    rx = -1;
    for (i = 0; i < resSortTabSize; i++)
	if (nodeK -> con[i] || nodeK -> substrCon[i] != 0) {
	    ASSERT (rx < 0);
	    rx = i;
	}

    cx = -1;
    if (nodeK -> gndCap[0] != 0 || nodeK -> cap[0] || nodeK -> substrCap[0] != 0) cx = 0;

    if (rx < 0) {
	if (cx < 0) return (weight);
    } else {
	if (cx < 0 && !nodeK -> moments && !nodeK -> moments2) return (weight);
    }

    N_cnt = 2; /* nGND + nSUB */
    N_cnt += nodeK -> cap_cnt;
    N_cnt += nodeK -> res_cnt;
    nodes = NEW (node_t*, N_cnt);

    max = maxMoments + 1;
    Mk    = NEW (double*, N_cnt);
    Mk[0] = NEW (double, N_cnt * max);

 // nodes[0] = NULL; /* The ground node. */
    i = 1;
    sumMkn[0] = Mk[0][0] = 0;
    if (cx < 0) Mk[0][1] = 0;
    else   if ((Mk[0][1] = nodeK -> gndCap[cx]) != 0) i = 0;
    sumMkn[1] = Mk[0][1];

    m = 0;
    if (nodeK -> moments) { ++m;
	for (l = 2; l <= maxMoments; ++l) {
	    sumMkn[l] = Mk[0][l] = nodeK -> moments[l-2];
	    if (sumMkn[l] != 0) i = 0;
	}
    }
    else for (l = 2; l <= maxMoments; ++l) sumMkn[l] = Mk[0][l] = 0;

    N = 0;
    if (cx >= 0) {
	for (cap = nodeK -> cap[cx]; cap; cap = NEXT (cap, nodeK)) {
	    N++;
	    nodes[N] = OTHER (cap, nodeK);
	    Mk[N] = Mk[0] + N * max;
	    if (cap -> moments) { ++m;
		for (l = 2; l <= maxMoments; ++l) sumMkn[l] += Mk[N][l] = cap -> moments[l-2];
	    }
	    else for (l = 2; l <= maxMoments; ++l) Mk[N][l] = 0;
	    Mk[N][0] = 0;
	    sumMkn[1] += Mk[N][1] = cap -> val;
	}
	sumMkn[1] += subcap = nodeK -> substrCap[cx];
    }
    else { subcap = 0; cx = 0; }

    if (rx >= 0) {
	l0 = N;
	for (con = nodeK -> con[rx]; con; con = NEXT (con, nodeK)) {
	    on = OTHER (con, nodeK);
	    for (j = 1; j <= l0; ++j) if (on == nodes[j]) break;
	    if (j > l0) { /* new node */
		j = ++N;
		Mk[N] = Mk[0] + N * max;
		for (l = 1; l <= maxMoments; ++l) Mk[N][l] = 0;
		nodes[N] = on;
	    }
	    sumMkn[0] += Mk[j][0] = con -> val;
	}
	sumMkn[0] += subcon = nodeK -> substrCon[rx];
	ASSERT (sumMkn[0] != 0);
	l0 = 0; /* RES */
    }
    else {
	if (!m && !nodeK -> moments2) goto ret; /* no moments */
	subcon = 0; rx = 0;
	ASSERT (sumMkn[1] != 0);
	l0 = 1; /* noRES */
    }

    if (subcap != 0 || subcon != 0 || nodeK -> moments2) {
	N++;
	Mk[N] = Mk[0] + N * max;
	Mk[N][0] = subcon;
	Mk[N][1] = subcap;
	if (nodeK -> moments2) {
	    for (l = 2; l <= maxMoments; ++l) sumMkn[l] += Mk[N][l] = nodeK -> moments2[l-2];
	}
	else for (l = 2; l <= maxMoments; ++l) Mk[N][l] = 0;
	nodes[N] = nSUB;
    }
    ASSERT (N < N_cnt);

    if (i == 0) { /* gndCAP || gndMOM */
	Mij_[0] = 0;
	for (j = 0; ++j <= N;) {
	    on = nodes[j];
	    for (l = 1; l <= maxMoments; l++) {
		Mij_[l] = Mk[j][l] * Mk[0][l0];
		for (p = l0; p < l; p++) {
		    Mij_[l] += Mk[j][p] * Mk[0][l-p+l0];
		    Mij_[l] -= Mij_[p] * sumMkn[l-p+l0];
		}
		Mij_[l] /= sumMkn[l0];
	    }
	    Mij_[1] += on -> gndCap[cx];
	    Mold = on -> moments;
	    weight = calculateBranchWeight (Mold, Mij_, weight);
        }
    }

    if (sneFullGraph)
    for (i = 1; i < N; i++) {
	n = nodes[i];
	for (j = i; ++j <= N;) {
	    on = nodes[j];

	    if (l0) Mij_[0] = 0;
	    else Mij_[0] = (Mk[j][0] * Mk[i][0]) / sumMkn[0];

	    for (l = 1; l <= maxMoments; l++) {
		Mij_[l] = Mk[j][l] * Mk[i][l0];
		for (p = l0; p < l; p++) {
		    Mij_[l] += Mk[j][p] * Mk[i][l-p+l0];
		    Mij_[l] -= Mij_[p] * sumMkn[l-p+l0];
		}
		Mij_[l] /= sumMkn[l0];
	    }

	    if (on == nSUB) {
		Mij_[0] += n -> substrCon[rx];
		Mij_[1] += n -> substrCap[cx];
		Mold = n -> moments2;
	    }
	    else {
		for (con = n -> con[rx]; con; con = NEXT (con, n)) {
		    if (on == OTHER (con, n)) {
			Mij_[0] += con -> val;
			break;
		    }
		}
		Mold = NULL;
		for (cap = n -> cap[cx]; cap; cap = NEXT (cap, n)) {
		    if (on == OTHER (cap, n)) {
			Mij_[1] += cap -> val;
			Mold = cap -> moments;
			break;
		    }
		}
	    }
		/* All branches || Only capacitive branches */
	    if (sneFullGraph == 2 || Mij_[0] == 0) {
		weight = calculateBranchWeight (Mold, Mij_, weight);
	    }
        }
    }

    /* For some norm-schemes, we have to take the square root of the weight.
     * See calculateBranchWeight().
     */
    if (sneNorm == 0 || sneNorm == 2) weight = sqrt (weight);
ret:
    DISPOSE (Mk[0], N_cnt * max);
    DISPOSE (Mk   , N_cnt);
    DISPOSE (nodes, N_cnt);

    return (weight);
}

Private double calculateBranchWeight (double *Mold, double *Mshunt, double oldWeight)
{
    int i;
    double sm0, sm1, dw, nw, smIm, smRe, Omega;

    sm0 = Mshunt[0];            /* M0 */
    sm1 = Mshunt[1] * sneOmega; /* M1 * O */

/*  In this routine the squared weight of a given branch is calculated
 *
 *  Different versions of weight calculation (sneErrorFunc):
 *  0. w = | - m2*O^2 | / | M0 + j M1*O |
 *  1. w = | - m2*O^2 | / | M0 + j M1*O - M2*O^2 |
 *  2. w = | Sum mi*O^i | / | M0 + j M1*O |
 *  3. w = | Sum mi*O^i | / | M0 + j M1*O + Sum Mi*O^i |
 *
 *  O is Omega
 *  mi is the i-th moment of the generated shunt
 *  Mi is the i-th moment of the shunt added to the already present admittance
 */

    if (sneErrorFunc < 2) {
	smRe = Mshunt[2] * sneOmega2;    /* m2 * O^2 */
	nw = smRe * smRe;
	if (sneErrorFunc == 1 && !sneResetMoments) {
	    if (Mold) smRe += Mold[0] * sneOmega2; /* M2 * O^2 */
	    sm0 -= smRe;
	}
	dw = sm0 * sm0 + sm1 * sm1;
    }
    else {
	smRe = smIm = 0;
	Omega = sneOmega;
	for (i = 2; i <= maxMoments; i++) {
	    Omega *= sneOmega;
	    if ((i%2) == 0) { // even
		Omega = -Omega; // change sign
		smRe += Mshunt[i] * Omega; /* mi * O^i */
	    }
	    else
		smIm += Mshunt[i] * Omega; /* mi * O^i */
	}
	nw = smRe * smRe + smIm * smIm;
	if (sneErrorFunc == 3 && !sneResetMoments) {
	    if (Mold) {
		Omega = sneOmega;
		for (i = 2; i <= maxMoments; i++) {
		    Omega *= sneOmega;
		    if ((i%2) == 0) { // even
			Omega = -Omega; // change sign
			smRe += Mold[i-2] * Omega; /* Mi * O^i */
		    }
		    else
			smIm += Mold[i-2] * Omega; /* Mi * O^i */
		}
	    }
	    sm0 += smRe;
	    sm1 += smIm;
	}
	dw = sm0 * sm0 + sm1 * sm1;
    }

    if (dw > 0 && nw > 0) {

	nw /= dw; /* new weight */

	/* Now, the weight of a given node is updated
	*  Different versions of norm taking (sneNorm):
	*  0. L-infinity norm
	*    (For economy reasons the squared weight is calculated.
	*     Later the square root must be taken.)
	*  1. L-1 norm
	*  2. L-2 norm
	*    (For economy reasons the squared weight is calculated.
	*     Later the square root must be taken.)
	*/
	switch (sneNorm) {
	default:
	    ASSERT (sneNorm >= 0 && sneNorm <= 2);
	case 0: /* L-infinity norm */
	    return (Max (nw, oldWeight));
	case 1: /* L-1 norm */
	    return (oldWeight + sqrt (nw));
	case 2: /* L-2 norm */
	    return (oldWeight + nw);
	}
    }
    return (oldWeight);
}
#endif /* SNE */
#endif /* MOMENTS */
