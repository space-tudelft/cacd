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

#define KEEPNODE(node) node -> term |= 2

#ifdef __cplusplus
  extern "C" {
#endif

extern char *strCoorBrackets (coor_t x, coor_t y);
extern void addNetEqCAP (node_t *n, int nr, int sort, double val);
extern void addNetEqRES (node_t *n, int nr, int sort, double val);
extern void outResistor (int ne_nr, int nr, int sort, double val);
extern void outCapacitor(node_t *n, netEquiv_t *ne, int nr, int sort, double val);
Private void elimCAP (node_t *n);
Private void elimRES (node_t *n, int rx);

#ifdef __cplusplus
  }
#endif

extern double MIN_CONDUCTANCE;
extern int pre_elim_mode;
static int cx_testRC;

void elim (node_t *n, int rx)
{
 // ASSERT (!n -> netEq);
    ASSERT (!n -> names);
    ASSERT (!n -> term);
#if 0
    if (n -> netEq || n -> area == 2) {
fprintf (stderr, "\n-- elim:");
if (n -> netEq) fprintf (stderr, " n->netEq:");
fprintf (stderr, " area=%d lay=%d x,y=%g,%g\n", n->area, n->mask, (double)n->node_x/4, (double)n->node_y/4);
    }
#endif

    if (rx >= 0) {
//fprintf (stderr, "elimRES:");
	elimRES (n, rx);
    } else {
//fprintf (stderr, "elimCAP:");
	elimCAP (n); /* SdeG: After testRCelim */
    }
//fprintf (stderr, " DONE\n");

    ASSERT (n -> res_cnt == 0 && n -> cap_cnt == 0);
    eliNod++;
    nodeDel (n);
}

Private void elimRES (node_t *n, int rx)
{
    static int warn = 1;
    static int R_size = 0;
    static node_t **nodes = NULL;
    static double  *value = NULL;
    element_c *cap;
    element_r *con;
    node_t *on;
    double v, val, subconval, totconval;
    int i, j, number, P_cnt, R_cnt, N_cnt;
    netEquiv_t *e, *e_next;

    R_cnt = n -> res_cnt;
    if ((subconval = n -> substrCon[rx]) != 0) ++R_cnt;
    ASSERT (R_cnt > 0);

    if (R_cnt > R_size) {
	int old_size = R_size;
	R_size = ((R_cnt / 50) + 1) * 50;
	nodes = RESIZE (nodes, node_t *, R_size, old_size);
	value = RESIZE (value, double  , R_size, old_size);
    }

    totconval = subconval;
    i = 0;
    for (e = n -> netEq; e; e = e -> next) {
	ASSERT (e -> instType == RES);
	nodes[i] = (node_t*)e;
	value[i++] = e -> val;
	totconval += e -> val;
    }
    N_cnt = i;
    for (con = n -> con[rx]; con; con = NEXT (con, n)) {
	nodes[i] = OTHER (con, n);
	value[i++] = v = con -> val;
	totconval += v;
	elemDelRes (con);
    }
    P_cnt = i;
    if (subconval != 0) {
	nodes[i] = nSUB;
	value[i++] = subconval;
	if (!warn++) say ("warning: elimRES of node at position %s has res to nSUB", strCoorBrackets (n->node_x, n->node_y));
    }
    ASSERT (i == R_cnt);
    ASSERT (totconval != 0);

    /* redistribute GND, SUBSTR and coupling capacitances */
    for (i = 0; i < capSortTabSize; i++) {
	for (cap = n -> cap[i]; cap; cap = NEXT (cap, n)) {
	    on = OTHER (cap, n);
	    v = cap -> val / totconval;
	    for (j = 0; j < N_cnt; ++j) {
		e = (netEquiv_t*) nodes[j];
		addNetEqCAP (on, e -> number, i, v * value[j]);
	    }
	    for (; j < R_cnt; ++j) {
		elemAddCap (nodes[j], on, v * value[j], i, NULL);
	    }
	    elemDelCap (cap);
	}
	if ((v = n -> gndCap[i]) != 0) {
	    v /= totconval;
	    for (j = 0; j < N_cnt; ++j) {
		e = (netEquiv_t*) nodes[j];
		outCapacitor (n, NULL, e -> number, i, v * value[j]);
	    }
	    for (; j < R_cnt; ++j) {
		nodes[j] -> gndCap[i] += v * value[j];
	    }
	}
	if ((v = n -> substrCap[i]) != 0) {
	    v /= totconval;
	    for (j = 0; j < N_cnt; ++j) {
		e = (netEquiv_t*) nodes[j];
		outCapacitor (n, NULL, -e -> number, i, v * value[j]);
	    }
	    for (; j < R_cnt; ++j) {
		nodes[j] -> substrCap[i] += v * value[j];
	    }
	}
    }

    for (e = n -> netEq2; e; e = e_next) {
	e_next = e -> next;
	v = e -> val / totconval;
	i = e -> sort;
	number = e -> number;
	DISPOSE (e, sizeof(netEquiv_t)); currNeqv--;
	n -> cap_cnt--;
	for (j = 0; j < N_cnt; ++j) {
	    e = (netEquiv_t*) nodes[j];
	    if (e -> number != number) outCapacitor (n, e, number, i, v * value[j]);
	}
	for (; j < P_cnt; ++j) {
	    addNetEqCAP (nodes[j], number, i, v * value[j]);
	}
	if (j < R_cnt) {
	    ASSERT (nodes[j] == nSUB);
	    outCapacitor (nSUB, NULL, -number, i, v * value[j]);
	}
    }

    for (i = 0; i < N_cnt; ++i) {
	e = (netEquiv_t*) nodes[i];
	number = e -> number;
	DISPOSE (e, sizeof(netEquiv_t)); currNeqv--;
	n -> res_cnt--;
	v = value[i] / totconval;
	for (j = i; ++j < N_cnt;) {
	    e = (netEquiv_t*) nodes[j];
	    outResistor (e -> number, number, rx, v * value[j]);
	}
	for (; j < P_cnt; ++j) {
	    addNetEqRES (nodes[j], number, rx, v * value[j]);
	}
	if (j < R_cnt) {
	    ASSERT (nodes[j] == nSUB);
	    outResistor (0, number, rx, v * value[j]);
	}
    }

    for (; i < P_cnt; ++i) {
	n = nodes[i];
	v = value[i] / totconval;
	for (j = i; ++j < R_cnt;) {
	    val = v * value[j];
	    if (val < MIN_CONDUCTANCE) continue;
	    elemAddRes ('S', n, nodes[j], val, rx);
	}
	if (pre_elim_mode || n -> delayed) nqChange (n);
    }
}

void elimCAP (node_t *n)
{
    static int warn = 1;
    netEquiv_t *e, *ne;
    node_t *on;
    element_c *cap, *next_cap, *cap2;
    double totcapval, v, g, s;
    int number, cx = cx_testRC;

    /* no resistances are connected to this node */

    if (cx >= 0) {
	if (!warn++) say ("warning: elimCAP of node at position %s lay=%d cap_cnt=%d", strCoorBrackets (n->node_x, n->node_y), n->mask, n->cap_cnt);
	totcapval = g = n -> gndCap[cx];
	for (cap = n -> cap[cx]; cap; cap = NEXT (cap, n)) {
	    totcapval += cap -> val;
	    on = OTHER (cap, n);
	}
	if ((s = n -> substrCap[cx]) != 0) totcapval += s;

	ASSERT (n -> netEq == NULL);

	ne = n -> netEq2;
	for (e = ne; e; e = e -> next) {
	    ASSERT (e -> sort == cx);
	    totcapval += e -> val;
	}
	if (ne) fprintf (stderr, "-- elimCAP!!!  netEq2!!!\n");

	for (cap = n -> cap[cx]; cap; cap = next_cap) {
	    on = OTHER (cap, n);
	    next_cap = NEXT (cap, n);
	    if ((v = cap -> val / totcapval) != 0) {
		for (cap2 = next_cap; cap2; cap2 = NEXT (cap2, n)) {
		    elemAddCap (on, OTHER (cap2, n), v * cap2 -> val, cx, NULL);
		}
		for (e = ne; e; e = e -> next) {
		    addNetEqCAP (on, e -> number, cx, v * e -> val);
		}
		if (g) on -> gndCap[cx]    += v * g;
		if (s) on -> substrCap[cx] += v * s;
	    }
	    elemDelCap (cap);
	}
	if (s && g) nSUB -> gndCap[cx] += s / totcapval * g;

	while ((e = ne)) {
	    ne = e -> next;
	    v = e -> val / totcapval;
	    number = e -> number;
	    DISPOSE (e, sizeof(netEquiv_t)); currNeqv--;
	    n -> cap_cnt--;

	    for (e = ne; e; e = e -> next) {
		outCapacitor (n, e, number, cx, v * e -> val);
	    }
	    if (g) outCapacitor (n, NULL,  number, cx, v * g);
	    if (s) outCapacitor (n, NULL, -number, cx, v * s);
	}
    }
}

int testRCelim (node_t *n)
{
    int k, cx;
    netEquiv_t *e;

    if (resSortTabSize == 1) { /* ok, no different res sorts */
        if (n -> con[0] || n -> substrCon[0] != 0) return (0);
	for (e = n -> netEq; e && e -> instType != RES; e = e -> next) ;
	if (e) return (0);
    }
    else {
	cx = -1;
	for (k = 0; k < resSortTabSize; k++) {
	    if (n -> con[k] || n -> substrCon[k] != 0) {
		if (cx < 0) cx = k;
		else { /* stop, different res sorts found */
		    KEEPNODE (n);
		    return (cx);
		}
	    }
	}
	for (e = n -> netEq; e && e -> instType != RES; e = e -> next) ;
	if (cx < 0 && e) { cx = e -> sort; e = e -> next; }
	if (cx >= 0) {
	    for (; e; e = e -> next) if (e -> sort != cx) { KEEPNODE (n); return (cx); }
	    return (cx); /* a resistor found */
	}
    }

    /* no resistors */
    cx_testRC = k = -1;
    while (++k < capSortTabSize) {
	if (n -> cap[k] || n -> gndCap[k] != 0 || n -> substrCap[k] != 0) {
	    if (cx_testRC >= 0 || capPolarityTab[k] != 'x') {
		/* stop, different cap sorts (or polar sort) found */
		KEEPNODE (n);
		return (-1);
	    }
	    cx_testRC = k;
	}
    }
    e = n -> netEq2;
    while (e) {
	k = e -> sort;
	if ((cx_testRC >= 0 && cx_testRC != k) || capPolarityTab[k] != 'x') {
	    KEEPNODE (n);
	    return (-1);
	}
	cx_testRC = k;
	e = e -> next;
    }
    return (-1);
}
