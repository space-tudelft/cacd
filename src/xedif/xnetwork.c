/*
 * ISC License
 *
 * Copyright (C) 1987-2011 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
 *	Bastiaan Sneeuw
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

#include "src/xedif/incl.h"
#include "src/xedif/search.h"

long netCounter;

#ifdef __cplusplus
  extern "C" {
#endif
static void readNet (struct model_info *m, struct net_ref **anets);
static void cpCnet2Netel (int subnet, struct net_el *nel);
static void delNetel (struct net_ref *nets);
#ifdef __cplusplus
  }
#endif

void xnetwork (char *ntwname, DM_PROJECT *proj, int imported, char *orig_name, int submod)
{
    struct model_info *ntw;
    struct net_ref *nets;

    outPos ();

    ntw = newNetw (ntwname, proj, imported, orig_name, submod);

    prHead (ntw, submod);

    readNet (ntw, &nets);
    findNetInit (ntw, nets);

    /* Added by B.Sneeuw : Introducing of hash table for instance names
       with temporary instance numbers (e.g. nmos <->&_T1.) */
    hcreate ();

    prInst (ntw, nets);

    prNets (ntw, nets);

    prFoot (ntw, submod);

    if (submod) {
	delNetel (nets);
	resetStringSpace (0);
	resetIndexSpace (0);
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

    nel -> net_name = newStringSpace (cnet.net_name, 0);
    if (!nel -> net_name) fatalErr ("no net_name:", "zero length");

    if ((nel -> net_dim = cnet.net_dim) > 0) {
	int net_decl = !subnet && !cnet.net_neqv;
	nel -> net_lower = newIndexSpace (cnet.net_dim, 0);
	if (net_decl) nel -> inst_lower = newIndexSpace (cnet.net_dim, 0);
	for (i = 0; i < cnet.net_dim; i++) {
	    nel -> net_lower[i] = cnet.net_lower[i];
	    if (net_decl) nel -> inst_lower[i] = cnet.net_upper[i];
	    else
	    if (cnet.net_upper[i] != cnet.net_lower[i]) fatalErr ("net_upper != net_lower in net:", cnet.net_name);
	}
    }

    if (subnet) {
	nel -> inst_name = newStringSpace (cnet.inst_name, 0);
	if ((nel -> inst_dim = cnet.inst_dim) > 0) {
	    nel -> inst_lower = newIndexSpace (cnet.inst_dim, 0);
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

#define NRS 2

static char **SS[NRS];
static long **IS[NRS];
static int SS_y_size[NRS], SS_x_cnt[NRS], SS_y_cnt[NRS];
static int IS_y_size[NRS], IS_x_cnt[NRS], IS_y_cnt[NRS];

void resetStringSpace (int stock)
{
    SS_y_cnt[stock] = 0;
    SS_x_cnt[stock] = 0;
}

void resetIndexSpace (int stock)
{
    IS_y_cnt[stock] = 0;
    IS_x_cnt[stock] = 0;
}

char *newStringSpace (char *s, int stock)
{
    int i, cnt;

    cnt = 1;
    if (s) cnt += strlen (s);
    if (cnt == 1) return (NULL);

    if (SS_x_cnt[stock] + cnt > 4096) {
	SS_y_cnt[stock]++;
	SS_x_cnt[stock] = 0;
    }
    SS_x_cnt[stock] += cnt;

    if (SS_y_cnt[stock] >= SS_y_size[stock]) {
	i = SS_y_size[stock]; /* old_size */
	SS_y_size[stock] += 3;
	REPALLOC (SS[stock], SS_y_size[stock], char *);
	for (; i < SS_y_size[stock]; i++) PALLOC (SS[stock][i], 4096, char);
    }
    return (strcpy (&SS[stock][SS_y_cnt[stock]][SS_x_cnt[stock] - cnt], s));
}

long *newIndexSpace (long dim, int stock)
{
    int i, cnt;

    if ((cnt = (int)dim) <= 0) return (NULL);

    if (IS_x_cnt[stock] + cnt > 4096) {
	IS_y_cnt[stock]++;
	IS_x_cnt[stock] = 0;
    }
    IS_x_cnt[stock] += cnt;

    if (IS_y_cnt[stock] >= IS_y_size[stock]) {
	i = IS_y_size[stock]; /* old_size */
	IS_y_size[stock] += 2;
	REPALLOC (IS[stock], IS_y_size[stock], long *);
	for (; i < IS_y_size[stock]; i++) PALLOC (IS[stock][i], 4096, long);
    }
    return (&IS[stock][IS_y_cnt[stock]][IS_x_cnt[stock] - cnt]);
}
