
/*
 * ISC License
 *
 * Copyright (C) 1987-2011 by
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

#include "src/xnle/incl.h"

extern long *Nil;
extern long nodeCounter;
extern int tog_longlist;

extern char *snbulk, *spbulk;
extern char **globNets;
extern int globNets_cnt;

extern int NLEnbulkdefined;
extern int NLEpbulkdefined;
long NLENodeIndex = 0;
int  NLENodeType = 0;
int  currTermType;

static void prNetNbr (char *name, long  dim, long *lower)
{
    struct net_el netspace;
    struct net_el *n;
    char *s;

    n = &netspace;
    n -> net_name = name;
    n -> net_dim = dim;
    n -> net_lower = lower;
    n -> inst_dim = 0;
    n -> inst_name = NULL;

    n = findNet (n);
    NLENodeIndex = n ? n -> nx : 0;
    if (!NLENodeIndex || !testNbr (NLENodeIndex)) {
	s = makeArrayName (1, name, dim, lower);
	if (!NLENodeIndex) {
	    if (NLENodeType == 'X')
		P_E "Warning: terminal '%s' not connected!\n", s);
	    NLENodeIndex = nodeCounter++;
	}
	assignNameNbr (s, NLENodeIndex);
    }
    nmprint (0, name, dim, lower, Nil, 0);
}

void prNets (struct model_info *ntw, struct net_ref *nets)
{
    struct cir_term *t;
    struct term_ref *tref;
    struct net_ref *nref;
    struct net_el *n;
    char buf[DM_MAXNAME + DM_MAXNAME + 52];
    char *iname, *nname, *sname, *s;
    long *lower, *upper, xvector[10];
    long i, k;

    NLENodeType = 'X';
    for (tref = ntw -> terms; tref; tref = tref -> next) {
	currTermType = tref -> type;
	t = tref -> t;
	s = t -> term_name;
	k = t -> term_dim;
	if (k > 0) {
	    lower = t -> term_lower;
	    upper = t -> term_upper;
	    for (i = 0; i < k; ++i) xvector[i] = lower[i];
	    while (1) {
		prNetNbr (s, k, xvector);

		for (i = k - 1; i >= 0; --i) {
		    if (lower[i] <= upper[i]) {
			if (++xvector[i] <= upper[i]) break;
		    }
		    else {
			if (--xvector[i] >= upper[i]) break;
		    }
		    if (i == 0) goto endwhile;
		    xvector[i] = lower[i];
		}
	    }
endwhile:	;
	}
	else {
	    prNetNbr (s, 0L, xvector);
	}
    }

    NLENodeType = 'G';
    currTermType = 0;
    for (i = 0; i < globNets_cnt; ++i) {
	prNetNbr (globNets[i], 0L, xvector);
    }
    if (NLEnbulkdefined || NLEpbulkdefined) { /* add bulk? */
	struct net_el netspace;

	n = &netspace;
	n -> net_dim = 0;
	n -> inst_dim = 0;
	n -> inst_name = NULL;

	read_tox ();

	if (NLEnbulkdefined) {
	    if (snbulk) {
		n -> net_name = snbulk;
		if (findNet (n)) goto n1;
	    }
	    else {
		n -> net_name = snbulk = "vss";
		if (findNet (n) || testNameNbr (snbulk) >= 0) goto n1;
		n -> net_name = snbulk = "VSS";
		if (findNet (n)) goto n1;
	    }
	    if (testNameNbr (snbulk) < 0)
		prNetNbr (snbulk, 0L, xvector);
	}
n1:
	if (NLEpbulkdefined) {
	    if (spbulk) {
		n -> net_name = spbulk;
		if (findNet (n)) goto n2;
	    }
	    else {
		n -> net_name = spbulk = "vdd";
		if (findNet (n) || testNameNbr (spbulk) >= 0) goto n2;
		n -> net_name = spbulk = "VDD";
		if (findNet (n)) goto n2;
	    }
	    if (testNameNbr (spbulk) < 0)
		prNetNbr (spbulk, 0L, xvector);
	}
    }
n2:
    NLENodeType = 'N';

    for (nref = nets; nref; nref = nref -> next) {
	n = nref -> n;
	if (!n -> net_neqv) continue; /* net decl. of array */

	nname = n -> net_name;

	/* set node-index first */
	if (!testNbr (NLENodeIndex = n -> nx)) {
	    s = makeArrayName (1, nname, n -> net_dim, n -> net_lower);
	    assignNameNbr (s, NLENodeIndex);
	}
	else if (!tog_longlist) continue;
	nmprint (0, nname, n -> net_dim, n -> net_lower, Nil, 0);
	if (!tog_longlist) continue;

	i = n -> net_neqv;
	n = n -> net_eqv;
	do { /* for all subnets */

	    sname = n -> net_name;
	    iname = n -> inst_name;

	    if (iname) {
		iname = makeArrayName (0, iname, n -> inst_dim, n -> inst_lower);
		iname = makeArrayName (0, iname, n -> inst_dim, n -> inst_lower);
		sprintf (buf, "%s.%s", iname, sname);
		sname = buf;
	    }
	    nmprint (0, sname, n -> net_dim, n -> net_lower, Nil, 0);

	    ++n;
	} while (--i);
    }

    NLENodeIndex = 0;
    NLENodeType = 0;
}
