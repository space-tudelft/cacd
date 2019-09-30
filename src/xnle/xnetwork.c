
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

long netCounter;
long nodeCounter;

#ifdef __cplusplus
  extern "C" {
#endif
static void readNet (struct model_info *m, struct net_ref **anets);
static void cpCnet2Netel (int subnet, struct net_el *nel);
static void delNetel (struct net_ref *nets);
static void resetStringSpace (void);
static void resetIndexSpace (void);
#ifdef __cplusplus
  }
#endif

void xnetwork (char *ntwname, DM_PROJECT *proj, int imported, char *orig_name, int submod)
{
    struct model_info *ntw;
    struct net_ref *nets;

    outPos ();

    ntw = newNetw (ntwname, proj, imported, orig_name, submod);

    prHead ();

    readNet (ntw, &nets);
    findNetInit (ntw, nets);

    prNets (ntw, nets);

    prExt ();

    prInst (ntw, nets);

    prFoot ();

    if (submod) {
	delNetel (nets);
	nameNbrReset ();
	resetStringSpace ();
	resetIndexSpace ();
    }
}

/* read the net file of model m */
static void readNet (struct model_info *m, struct net_ref **anets)
{
    DM_STREAM *dsp;
    struct net_ref *nref, *nreflast;
    struct net_el *n1, *n;
    char *val;
    char attribute_string[256];
    long lower[10], lower1[10], lower2[10];
    long upper[10], upper1[10], upper2[10];
    long i, Neqv;

    cnet.net_attribute = attribute_string;
    cnet.net_lower  = lower;
    cnet.net_upper  = upper;
    cnet.inst_lower = lower1;
    cnet.inst_upper = upper1;
    cnet.ref_lower  = lower2;
    cnet.ref_upper  = upper2;

    dsp = dmOpenStream (m -> dkey, "net", "r");

    netCounter = 0;
    nodeCounter = 1;

    *anets = NULL;
    nreflast = NULL;

    while (dmGetDesignData (dsp, CIR_NET_ATOM) > 0) {

	netCounter++;

	PALLOC (nref, 1, struct net_ref);
	PALLOC (n1, 1, struct net_el);
	nref -> n = n1;
	cpCnet2Netel (0, n1);

	if ((Neqv = n1 -> net_neqv) > 0) {

	    PALLOC (n, Neqv, struct net_el);
	    n1 -> net_eqv = n;
	    n1 -> nx = nodeCounter++;

	    for (i = 0; i < Neqv; ++i) {
		if (dmGetDesignData (dsp, CIR_NET_ATOM) <= 0) fatalErr ("net read error:", m -> name);
		cpCnet2Netel (1, n);
		++n;
	    }
	}

	nref -> next = NULL;
	if (!*anets)   *anets = nref;
	else nreflast -> next = nref;
	nreflast = nref;
    }

    dmCloseStream (dsp, COMPLETE);
}

static void cpCnet2Netel (int subnet, struct net_el *nel)
{
    long i;

    nel -> net_name = newStringSpace (cnet.net_name);
    if (!nel -> net_name) fatalErr ("no net_name:", "zero length");

    if ((nel -> net_dim = cnet.net_dim) > 0) {
	int net_decl = !subnet && !cnet.net_neqv;
	nel -> net_lower = newIndexSpace (cnet.net_dim);
	if (net_decl) nel -> inst_lower = newIndexSpace (cnet.net_dim);
	for (i = 0; i < cnet.net_dim; i++) {
	    nel -> net_lower[i] = cnet.net_lower[i];
	    if (net_decl) nel -> inst_lower[i] = cnet.net_upper[i];
	    else
	    if (cnet.net_upper[i] != cnet.net_lower[i]) fatalErr ("net_upper != net_lower in net:", cnet.net_name);
	}
    }

    if (subnet) {
	nel -> inst_name = newStringSpace (cnet.inst_name);
	if ((nel -> inst_dim = cnet.inst_dim) > 0) {
	    nel -> inst_lower = newIndexSpace (cnet.inst_dim);
	    for (i = 0; i < cnet.inst_dim; i++) {
		nel -> inst_lower[i] = cnet.inst_lower[i];
		if (cnet.inst_upper[i] != cnet.inst_lower[i]) fatalErr ("inst_upper != inst_lower in net:", cnet.net_name);
	    }
	}
    }
    else nel -> net_neqv = cnet.net_neqv;
}

static void delNetel (struct net_ref *nets)
{
    struct net_ref *nref, *help;
    struct net_el *n;

    nref = nets;
    while (nref) {
	help = nref;
	nref = nref -> next;

	n = help -> n;
	if (n -> net_neqv > 0) Free (n -> net_eqv);
	Free (n);
	Free (help);
    }
}

static char **SS;
static long **IS;
static int SS_y_size, SS_y_cnt, SS_x_cnt;
static int IS_y_size, IS_y_cnt, IS_x_cnt;

static void resetStringSpace ()
{
    SS_y_cnt = SS_x_cnt = 0;
}

static void resetIndexSpace ()
{
    IS_y_cnt = IS_x_cnt = 0;
}

char *newStringSpace (char *s)
{
    int i, cnt;

    cnt = 1;
    if (s) cnt += strlen (s);
    if (cnt == 1) return (NULL);

    if (SS_x_cnt + cnt > 4096) { ++SS_y_cnt; SS_x_cnt = 0; }
    SS_x_cnt += cnt;

    if (SS_y_cnt >= SS_y_size) {
	i = SS_y_size; /* old_size */
	SS_y_size += 3;
	REPALLOC (SS, SS_y_size, char *);
	for (; i < SS_y_size; ++i) PALLOC (SS[i], 4096, char);
    }
    return (strcpy (&SS[SS_y_cnt][SS_x_cnt - cnt], s));
}

long *newIndexSpace (long dim)
{
    int i, cnt;

    if ((cnt = (int)dim) <= 0) return (NULL);

    if (IS_x_cnt + cnt > 4096) { ++IS_y_cnt; IS_x_cnt = 0; }
    IS_x_cnt += cnt;

    if (IS_y_cnt >= IS_y_size) {
	i = IS_y_size; /* old_size */
	IS_y_size += 2;
	REPALLOC (IS, IS_y_size, long *);
	for (; i < IS_y_size; ++i) PALLOC (IS[i], 4096, long);
    }
    return (&IS[IS_y_cnt][IS_x_cnt - cnt]);
}
