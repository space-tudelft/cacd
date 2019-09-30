/*
 * ISC License
 *
 * Copyright (C) 1989-2018 by
 *	A.J. van Genderen
 *	S. de Graaf
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

#include "src/csls/sys_incl.h"
#include "src/csls/class.h"
#include "src/csls/mkdbdefs.h"
#include "src/csls/mkdbincl.h"

extern Netelem *notconnected;
extern DM_STREAM *dsp_net;
extern Network *curr_ntw;

class simpleNet * snetQueue = NULL;
int intNetCnt = 0;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
static class netMember *newNetMems (int nr, class ntwinst *inst, Netelem *net);
static void initCi (Stack *xs, int *ci);
static int  incrCi (Stack *xs, int *ci);
static int  findOffset (Stack *xsdef, int *ci);
static void fillArrNet (struct cir_net *pcnet, class netMember *nmem,
			    struct cir_net *prefcnet);
static int findCi (Stack *xs, int offset);
#ifdef __cplusplus
  }
#endif

void joinSimple (
Netelem *dnet,
Stack *dnet_xs,
class ntwinst *inst,
Stack *inst_xs,
Netelem *net,
Stack *net_xs)
{
    int j;
    int nbr;
    Netelem *pterm;
    Queue *termq;
    int termq_len;
    int dnet_off, inst_off, net_off;
    class netMember *net1mem, *net2mem;
    class netMember *net1memlast;
    class simpleNet *snet1, *snet2;
    int dnet_ci[XSTACK_SIZE];
    int inst_ci[XSTACK_SIZE];
    int net_ci[XSTACK_SIZE];

    if (dnet -> nmem == NULL) {
	dnet -> nmem = newNetMems (-1, (class ntwinst *)NULL, dnet);
    };

    if (inst != NULL && inst -> inst_struct -> nmem == NULL) {

	termq = inst -> ntw -> termq;
	termq_len = termq -> length ();

	inst -> inst_struct -> termCnt = 0;

	for (j = 0, pterm = (Netelem *) termq -> first_elem ();
	     j < termq_len;
	     j++, pterm = (Netelem *) termq -> next_elem ((Link *)pterm)) {

            /* use nmem to store an integer value */
	    pterm -> nmem = (class netMember *)(long)(inst -> inst_struct -> termCnt);

	    inst -> inst_struct -> termCnt += getxslength (pterm -> xs);
	}

	nbr = (inst -> inst_struct -> termCnt)
	        * getxslength (inst -> inst_struct -> inst_construct);

	inst -> inst_struct -> nmem = newNetMems (nbr, inst, NULL);
    }

    if (inst == NULL && net -> nmem == NULL) {
	net -> nmem = newNetMems (-1, (class ntwinst *)NULL, net);
    }

    /* now, perform the joining */

    initCi (dnet_xs, dnet_ci);
    initCi (inst_xs, inst_ci);
    initCi (net_xs, net_ci);

    do {
        dnet_off = findOffset (dnet -> xs, dnet_ci);
	net_off = findOffset (net -> xs, net_ci);
        net1mem = (dnet -> nmem) + dnet_off;
	if (inst) {
	    inst_off = findOffset (inst -> inst_struct -> inst_construct, inst_ci);
	    net2mem = (inst -> inst_struct -> nmem)
		      + inst_off * inst -> inst_struct -> termCnt
#if __WORDSIZE == 64
		      + (long long)(net -> nmem) + net_off;
#else
		      + (int)(net -> nmem) + net_off;
#endif
	}
	else
	    net2mem = (net -> nmem) + net_off;

	snet1 = net1mem -> snet;
	snet2 = net2mem -> snet;

        if (snet1 == snet2)
	    continue;       /* already joined */

	net1memlast = net1mem;
	while (net1memlast -> next)
	    net1memlast = net1memlast -> next;

        if (snet2) {
	    net1memlast -> next = snet2 -> nmem;   /* join lists */
	    net1memlast = net1memlast -> next;
	    while (net1memlast) {
		net1memlast -> snet = snet1;
		net1memlast = net1memlast -> next;
	    }

	    if (snet2 -> prev)
		snet2 -> prev -> next = snet2 -> next;
	    else
		snetQueue = snet2 -> next;
	    if (snet2 -> next)
		snet2 -> next -> prev = snet2 -> prev;
	}
	else {
	    net1memlast -> next = net2mem;
	    net2mem -> snet = snet1;
	}

	if (snet2)
	    delete snet2;
    }
    while (incrCi (dnet_xs, dnet_ci)
	   && (incrCi (net_xs, net_ci) || incrCi (inst_xs, inst_ci)));
}

static class netMember *newNetMems (int nr, class ntwinst *inst, Netelem *net)
{
    int i;
    class netMember *ret, *r;

    if (inst) {
	int ilen, i_termNext;
	int tlen, termq_len;
	Netelem *pterm, *ptermNext;
	Queue *termq;

	termq = inst -> ntw -> termq;
	termq_len = termq -> length ();
	if (termq_len > 0) {
	    pterm = (Netelem *) termq -> first_elem ();
	}
	else {
	    pterm = NULL;
	}
	tlen = 1;
	if (termq_len > 1) {
	    ptermNext = (Netelem *) termq -> next_elem ((Link *)pterm);
#if __WORDSIZE == 64
	    i_termNext = (long long)(ptermNext -> nmem);
#else
	    i_termNext = (int)(ptermNext -> nmem);
#endif
	    tlen++;
	}
	else {
	    ptermNext = NULL;
	    i_termNext = inst -> inst_struct -> termCnt;
	}
	ilen = 0;
	ret = new netMember [nr];
	for (i = 0; i < nr; i++) {
	    r = ret + i;
	    r -> inst = inst;
	    r -> next = NULL;
	    r -> snet = new simpleNet;
	    r -> snet -> nmem = r;
	    r -> snet -> next = snetQueue;
	    if (snetQueue) {
		snetQueue -> prev = r -> snet;
	    }
	    snetQueue = r -> snet;
	    r -> snet -> prev = NULL;

	    if (i >= i_termNext) {
		pterm = ptermNext;
		if (tlen >= termq_len) {
		    ptermNext = (Netelem *) termq -> first_elem ();
		    tlen = 1;
		    ilen++;
		    i_termNext = ilen * inst -> inst_struct -> termCnt;
		}
		else {
		    ptermNext = (Netelem *) termq -> next_elem ((Link *)ptermNext);
		    i_termNext = ilen * inst -> inst_struct -> termCnt
#if __WORDSIZE == 64
				 + (long long)(ptermNext -> nmem);
#else
				 + (int)(ptermNext -> nmem);
#endif
		    tlen++;
		}
	    }
	    r -> net = pterm;
	}
    }
    else {
	nr = getxslength (net -> xs);
	ret = new netMember [nr];
	for (i = 0; i < nr; i++) {
	    r = ret + i;
	    r -> inst = inst;
	    r -> next = NULL;
	    r -> snet = new simpleNet;
	    r -> snet -> nmem = r;
	    r -> snet -> next = snetQueue;
	    if (snetQueue) {
		snetQueue -> prev = r -> snet;
	    }
	    snetQueue = r -> snet;
	    r -> snet -> prev = NULL;
	    r -> net = net;
	}
    }

    return (ret);
}

static void initCi (Stack *xs, int *ci)
{
    int i;
    char **pxs;

    if (xs) {
	i = xs -> length ();
	pxs = xs -> base () + i;

	while (--i >= 0) {
	    --pxs;
	    ci[i] = ((Xelem *) * pxs) -> left_bound;
	}
    }
}

static int incrCi (Stack *xs, int *ci)
{
    int i;
    char **pxs;
    int lb, rb;

    if (xs) {
	i = xs -> length ();
	pxs = xs -> base () + i;

	while (--i >= 0) {
	    --pxs;
	    lb = ((Xelem *) * pxs) -> left_bound;
	    rb = ((Xelem *) * pxs) -> right_bound;
	    if (rb >= lb) {
		if (++ci[i] <= rb) return (1);
	    }
	    else {
		if (--ci[i] >= rb) return (1);
	    }
	    ci[i] = lb;
	}
    }
    return (0);  /* all values have been reset */
}

static int findOffset (Stack *xsdef, int *ci)
{
    int i;
    int offset;
    int weight;
    char **pxs;
    int lb, rb;

    offset = 0;

    if (xsdef) {
	i = xsdef -> length ();
	pxs = xsdef -> base () + i;

	weight = 1;
	while (--i >= 0) {
	    --pxs;
	    lb = ((Xelem *) * pxs) -> left_bound;
	    rb = ((Xelem *) * pxs) -> right_bound;
	    if (rb >= lb) {
		offset += (ci[i] - lb) * weight;
		weight *= rb - lb + 1;
	    }
	    else {
		offset += (lb - ci[i]) * weight;
		weight *= lb - rb + 1;
	    }
	}
    }

    return (offset);
}

void outSimple (Queue *q)
{
    int i, j;
    int nr;
    int k;
    class simpleNet * snet;
    class simpleNet * snetOld;
    class netMember * otherMem;
    class netMember * eqvMem;
    Netelem *pnet;
    char **pxs;

    if (!q -> empty ()) {
	for (i = 0, pnet = (Netelem *) q -> first_elem ();
	     i < q -> length ();
	     i++, pnet = (Netelem *) q -> next_elem ((Link *) pnet)) {

	    if (!noWarnings && pnet != notconnected && pnet -> nmem == NULL
		&& (pnet -> type & N_TERMINAL) && !(pnet -> type & N_GLOBADDED))
		fprintf (stderr,
			 "Warning: network %s, unconnected terminal %s\n",
			 curr_ntw -> ntw_name, pnet -> name);

	    if (pnet == notconnected || pnet -> xs == NULL)
		continue;

	    strcpy (cnet.net_name, pnet -> name);

	    if (cnet.net_dim > 0) {
		delete cnet.net_lower;
		delete cnet.net_upper;
	    }

	    cnet.net_dim = pnet -> xs -> length ();
	    cnet.net_lower = new long [cnet.net_dim];
	    cnet.net_upper = new long [cnet.net_dim];

	    pxs = pnet -> xs -> base ();
	    for (j = 0; j < cnet.net_dim; j++) {
		cnet.net_lower[j] = ((Xelem *) * pxs) -> left_bound;
		cnet.net_upper[j] = ((Xelem *) * pxs) -> right_bound;
		++pxs;
	    }

	    cnet.net_neqv = 0;

	    dmPutDesignData (dsp_net, CIR_NET);
	}
    }

    snet = snetQueue;
    while (snet != NULL) {

	if (snet -> nmem -> net != notconnected) {

	    if (genIntNet && snet -> nmem -> net -> xs) {
		sprintf (cnet.net_name, "_N%d", ++intNetCnt);
		if (cnet.net_dim > 0) {
		    delete cnet.net_lower;
		    delete cnet.net_upper;
		}
		cnet.net_dim = 0;
		eqvMem = snet -> nmem;
	    }
	    else {
		strcpy (cnet.net_name, snet -> nmem -> net -> name);
		fillArrNet (&cnet, snet -> nmem, NULL);
		eqvMem = snet -> nmem -> next;
	    }

	    otherMem = eqvMem;
	    nr = 0;
	    while (otherMem) {
		if (otherMem -> net != notconnected)
		    nr++;
		otherMem = otherMem -> next;
	    }

	    if (cnet.net_neqv > 0)
		delete cnet.net_eqv;

	    cnet.net_neqv = nr;
	    cnet.net_eqv = new struct cir_net [cnet.net_neqv];

	    otherMem = eqvMem;
	    i = 0;
	    while (otherMem) {

		if (otherMem -> net == notconnected) {
		    otherMem = otherMem -> next;
		    continue;
		}

		cnet.net_eqv[i].net_attribute = NULL;
		cnet.net_eqv[i].net_neqv = 0;
		cnet.net_eqv[i].net_dim = 0;
		cnet.net_eqv[i].inst_dim = 0;
		cnet.net_eqv[i].ref_dim = 0;

		strcpy (cnet.net_eqv[i].net_name, otherMem -> net -> name);
		if (otherMem -> inst) {
		    strcpy (cnet.net_eqv[i].inst_name,
			    otherMem -> inst -> inst_struct -> inst_name);
		}
		else {
		    strcpy (cnet.net_eqv[i].inst_name, "");
		}
		fillArrNet (&cnet.net_eqv[i], otherMem, &cnet);

		otherMem = otherMem -> next;

		i++;
	    }

	    if (!noWarnings && i < 1) {
		fprintf (stderr,
			 "Warning: network %s, unconnected terminal %s",
			 curr_ntw -> ntw_name, cnet.net_name);
		if (cnet.net_dim > 0) {
		    fprintf (stderr, "[");
		    for (k = 0; k < cnet.net_dim; k++) {
			if (k != 0)
			    fprintf (stderr, ", ");
			fprintf (stderr, "%ld", cnet.net_lower[k]);
		    }
		    fprintf (stderr, "]");
		}
		fprintf (stderr, "\n");
	    }

	    dmPutDesignData (dsp_net, CIR_NET);

	}

        snetOld = snet;

	snet = snet -> next;

	delete snetOld;
    }

    snetQueue = NULL;
    intNetCnt = 0;

    while(!q -> empty()) {
	pnet = (Netelem *) q -> get();
	if (pnet -> nmem)
	    delete pnet -> nmem;
	delete pnet;
    }
    delete q;
}

int cis[XSTACK_SIZE];

static void fillArrNet (struct cir_net *pcnet, class netMember *nmem,
						struct cir_net *prefcnet)
{
    int i, offs, i_offs;
    class netMember *help;

    if (nmem -> inst) {

	if (pcnet -> inst_dim > 0) {
	    delete pcnet -> inst_lower;
	    delete pcnet -> inst_upper;
	}

        i_offs = nmem - nmem -> inst -> inst_struct -> nmem;
	i_offs = i_offs / nmem -> inst -> inst_struct -> termCnt;

	pcnet -> inst_dim = findCi
	   (nmem -> inst -> inst_struct -> inst_construct, i_offs);

	if (pcnet -> inst_dim > 0) {
	    pcnet -> inst_lower = new long [ pcnet -> inst_dim ];
	    pcnet -> inst_upper = new long [ pcnet -> inst_dim ];
	}

	for (i = 0; i < pcnet -> inst_dim; i++) {
	    pcnet -> inst_lower[i] = cis[i];
	    pcnet -> inst_upper[i] = cis[i];
	}

        help = nmem -> inst -> inst_struct -> nmem
		       + (i_offs * nmem -> inst -> inst_struct -> termCnt)
#if __WORDSIZE == 64
		       + (long long)(nmem -> net -> nmem);
#else
		       + (int)(nmem -> net -> nmem);
#endif
        offs = nmem - help;
    }
    else {
        offs = nmem - nmem -> net -> nmem;
    }

    if (pcnet -> net_dim > 0) {
	delete pcnet -> net_lower;
	delete pcnet -> net_upper;
    }

    pcnet -> net_dim = findCi (nmem -> net -> xs, offs);

    if (pcnet -> net_dim > 0) {
	pcnet -> net_lower = new long [ pcnet -> net_dim ];
	pcnet -> net_upper = new long [ pcnet -> net_dim ];
    }

    for (i = 0; i < pcnet -> net_dim; i++) {
	pcnet -> net_lower[i] = cis[i];
	pcnet -> net_upper[i] = cis[i];
    }

    if (prefcnet) {
	if (pcnet -> ref_dim > 0) {
	    delete pcnet -> ref_lower;
	    delete pcnet -> ref_upper;
	}

        pcnet -> ref_dim = prefcnet -> net_dim;

	if (pcnet -> ref_dim > 0) {
	    pcnet -> ref_lower = new long [ pcnet -> ref_dim ];
	    pcnet -> ref_upper = new long [ pcnet -> ref_dim ];
	}

	for (i = 0; i < pcnet -> ref_dim; i++) {
	    pcnet -> ref_lower[i] = prefcnet -> net_lower[i];
	    pcnet -> ref_upper[i] = prefcnet -> net_upper[i];
	}
    }
}

static int findCi (Stack *xs, int offset)
{
    int i;
    int dim;
    int v;
    int weight;
    int lb, rb;
    char **pxs;

    int w[XSTACK_SIZE];

    if (xs) {
	dim = i = xs -> length ();
	pxs = xs -> base () + i;

	weight = 1;
	while (--i >= 0) {
	    --pxs;
	    rb = ((Xelem *) * pxs) -> right_bound;
	    lb = ((Xelem *) * pxs) -> left_bound;
	    w[i] = weight;
	    if (rb >= lb)
		weight *= rb - lb + 1;
	    else
		weight *= lb - rb + 1;
	}

	while (++i < dim) {
	    v = offset / w[i];
	    offset %= w[i];
	    rb = ((Xelem *) * pxs) -> right_bound;
	    lb = ((Xelem *) * pxs) -> left_bound;
	    if (rb >= lb)
		cis[i] = lb + v;
	    else
		cis[i] = lb - v;
	    pxs++;
	}
    }
    else
	dim = 0;

    return (dim);
}
