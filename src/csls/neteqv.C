/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
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

extern Dictionary * sym_dict;
extern Network * ntw;
extern Netelem *notconnected;

extern int n_net_cnt;
extern int n_inst_cnt;
int n_term_cnt;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
static Stack **flatten_xs (Stack * xs);
static int flatten (NetReference * netref, int flat_inst, int flat_net, Queue * que);
static int match_net (Queue ** pdstq, Queue ** psrcq, int nettype);
static int getnetlength (NetReference * net);
static int pm_net_eqv (NetworkInstance * inst, Queue * netq);
static int im_net_eqv (NetworkInstance * inst, Queue * netq);
static void net_eqv (Queue * dst, Queue * src);
#ifdef __cplusplus
  }
#endif

static int newxs_len;

static Stack **flatten_xs (Stack * xs)
{
    int     i, j, k, n;
    int    *xv, *lb, *rb;
    char  **q;
    Xelem * pxelem;
    Stack **newxs, *pxs;

    if ((n = xs -> length ()) <= 0) {
	fprintf (stderr, "flatten_xs: Invalid stack length!\n");
	newxs_len = 0;
	return (0);
    }

    lb = new int [n];
    rb = new int [n];
    xv = new int [n];

    newxs_len = 1;
    q = xs -> base ();
    i = 0;
    do {
	lb[i] = ((Xelem *) *q) -> left_bound;
	rb[i] = ((Xelem *) *q) -> right_bound;
	xv[i] = lb[i];
	k = rb[i] - lb[i];
	if (k < 0) newxs_len *= 1 - k;
	else       newxs_len *= 1 + k;
	++q;
    } while (++i < n);

    newxs = new Stack * [newxs_len];

    i = 0;
    do {
	newxs[i] = pxs = new Stack (n);

	j = 0;
	do {
	    pxelem = new Xelem (xv[j], xv[j]);
	    pxs -> push ((char *) pxelem);
	} while (++j < n);

	while (--j >= 0) {
	    if (lb[j] <= rb[j]) {
		if (++xv[j] <= rb[j]) break;
	    }
	    else {
		if (--xv[j] >= rb[j]) break;
	    }
	    xv[j] = lb[j];
	}
    } while (++i < newxs_len);

    delete lb;
    delete rb;
    delete xv;

    return (newxs);
}

static int flatten (NetReference * netref, int flat_inst, int flat_net,
							    Queue * que)
{
    int i, j;
    int inst_xs_len;
    int net_xs_len;
    NetReference * newnref;
    Stack **inst_xs;
    Stack **net_xs;

    if (flat_inst && netref -> inst_xs) {
        inst_xs = flatten_xs (netref -> inst_xs);
        inst_xs_len = newxs_len;
    }
    else {
        inst_xs = 0; // init, to eliminate compiler warning
        inst_xs_len = 0;
    }

    if (flat_net && netref -> net_xs) {
        net_xs = flatten_xs (netref -> net_xs);
        net_xs_len = newxs_len;
    }
    else {
        net_xs = 0; // init, to eliminate compiler warning
        net_xs_len = 0;
    }

    if (inst_xs_len > 0) {
        for (i = 0; i < inst_xs_len; i++) {
            if (net_xs_len > 0) {
                for (j = 0; j < net_xs_len; j++) {
		    newnref = new NetReference (netref -> net, net_xs[j]);
		    newnref -> inst = netref -> inst;
		    newnref -> inst_xs = inst_xs[i];
		    que -> put ((Link *) newnref);
                }
            }
            else {
		newnref = new NetReference (netref -> net, netref -> net_xs);
		newnref -> inst = netref -> inst;
		newnref -> inst_xs = inst_xs[i];
		que -> put ((Link *) newnref);
            }
        }
	delete inst_xs;
    }
    else {
        if (net_xs_len > 0) {
            for (j = 0; j < net_xs_len; j++) {
		newnref = new NetReference (netref -> net, net_xs[j]);
	        newnref -> inst = netref -> inst;
	        newnref -> inst_xs = netref -> inst_xs;
		que -> put ((Link *) newnref);
            }
        }
        else {
            newnref = new NetReference (netref -> net, netref -> net_xs);
	    newnref -> inst = netref -> inst;
	    newnref -> inst_xs = netref -> inst_xs;
	    que -> put ((Link *) newnref);
        }
        inst_xs_len = 1;
    }

    if (net_xs_len == 0) net_xs_len = 1;
    else delete net_xs;

    delete netref;

    return (inst_xs_len * net_xs_len);
}

static int match_net (Queue ** pdstq, Queue ** psrcq, int nettype)
{
    int j;
    int j_max;
    Netelem *pnet;
    NetReference *pnref;
    NetReference *psrc;
    NetReference *pdst;
    Queue * srcq;
    Queue * dstq;
    Queue * new_srcq;
    Queue * new_dstq;

    int src_len;
    int dst_len;
    int newsrc_cnt = 0;
    int newdst_cnt = 0;
    int src_action;
    int dst_action;
    int flag;

    srcq = *psrcq;
    dstq = *pdstq;

    new_srcq = new Queue (QueueType);
    new_dstq = new Queue (QueueType);
    *psrcq = new_srcq;
    *pdstq = new_dstq;

    psrc = (NetReference *) srcq -> get ();
    pdst = (NetReference *) dstq -> get ();

    if ((psrc == NULL || isGlobalNet (psrc -> net -> name))
	&& pdst -> net == notconnected && dstq -> length () == 0) {
	pdst = NULL;
	n_net_cnt = 0;
    }

    src_len = getnetlength(psrc);
    dst_len = getnetlength(pdst);

    if (psrc == NULL && pdst)
	return (1);
    else if (psrc && !isGlobalNet (psrc -> net -> name) && pdst == NULL)
	return (-1);

    while (psrc || pdst) {

	if (n_inst_cnt > 0) {

	    flag = 0;

	    if (newdst_cnt >= newsrc_cnt)
	    while ((nettype == 'i'
		    && ((newsrc_cnt % n_term_cnt) >= (n_net_cnt / n_inst_cnt)))
	           || ((nettype == 'p'
		       && (newsrc_cnt + 1) > n_net_cnt))) {

		if (psrc && psrc -> net_xs == NULL
		    && isGlobalNet (psrc -> net -> name)) {

		    /* add global net as extra to new dst queue */
		    /* ONLY when newdst_cnt is NOT lower newsrc_cnt */

		    flag = 1;

		    if (nettype == 'p' && pdst) {
			/* flush dst queue (happens if instance array) */
			new_dstq -> put ((Link *) pdst);
			newdst_cnt += dst_len;
			pdst = (NetReference *) dstq -> get ();
			dst_len = getnetlength(pdst);
		    }

		    pnet = (Netelem *) sym_dict -> fetch (psrc -> net -> name);
		    if (!pnet) {
			pnet = new Netelem (psrc -> net -> name, NULL, NetType);
			sym_dict->store (pnet->name, (char *) pnet);
			ntw -> netq -> put ((Link *) pnet);
		    }
		    else if (pnet -> xs) {
			sls_errno = ILLRANGE;
			return (-999);
		    }
		    if (psrc -> inst_xs)
			j_max = getxslength (psrc -> inst_xs);
		    else
			j_max = 1;
		    for (j = 0; j < j_max; j++) {
			pnref = new NetReference (pnet, NULL);
			new_dstq -> put ((Link *) pnref);
			newdst_cnt++;
		    }

		    if (j_max > 1) {
			src_len = flatten (psrc, 1, 1, new_srcq);
			newsrc_cnt += src_len;
		    }
		    else {
			new_srcq -> put ((Link *) psrc);
			newsrc_cnt += src_len;
		    }
		    psrc = (NetReference *) srcq -> get ();
		    src_len = getnetlength(psrc);
		}
		else
		    break;
	    }

	    if (flag == 0
	        && (nettype == 'i' || nettype == 'p')
		&& newdst_cnt == newsrc_cnt
		&& psrc && psrc -> net_xs == NULL
		 && isGlobalNet (psrc -> net -> name)
		&& pdst
		 && !isGlobalNet (pdst -> net -> name)
		 && pdst -> net != notconnected) {

		/* this terminal is not connected to an actual parameter
		   that is a global net; so add (a) connection(s) between
		   the actual parameter(s) and the global net */

		pnet = (Netelem *) sym_dict -> fetch (psrc -> net -> name);
		if (!pnet) {
		    pnet = new Netelem (psrc -> net -> name, NULL, NetType);
		    sym_dict->store (pnet->name, (char *) pnet);
		    ntw -> netq -> put ((Link *) pnet);
		}
		else if (pnet -> xs) {
		    sls_errno = ILLRANGE;
		    return (-999);
		}

		if (psrc -> inst_xs)
		    j_max = getxslength (psrc -> inst_xs);
		else
		    j_max = 1;
		for (j = 0; j < j_max; j++) {
		    pnref = new NetReference (pnet, NULL);
		    new_dstq -> put ((Link *) pnref);

                    if (!pdst -> net_xs) {
			pnref = new NetReference (pdst -> net, NULL);
			new_srcq -> put ((Link *) pnref);
		    }
		}
		if (pdst -> net_xs) {
		    pnref = new NetReference (pdst -> net, pdst -> net_xs);
		    j = flatten (pnref, 1, 1, new_srcq);
		}
	    }
	}

	if ((psrc == NULL && pdst == NULL))
	    break;

        if (newsrc_cnt > newdst_cnt) {
            src_action = 0;
        }
        else if (newsrc_cnt == newdst_cnt && src_len == dst_len) {
            src_action = 1;
        }
        else {
            src_action = 2;
        }

        if (newdst_cnt > newsrc_cnt) {
            dst_action = 0;
        }
        else if (newdst_cnt == newsrc_cnt && dst_len == src_len) {
            dst_action = 1;
        }
        else {
            dst_action = 2;
        }

        if (src_action == 1) {
            if (psrc == NULL)
                return (1);
   	    new_srcq -> put ((Link *) psrc);
            newsrc_cnt += src_len;
            psrc = (NetReference *) srcq -> get ();
            src_len = getnetlength(psrc);
        }
        else if (src_action == 2) {
            if (psrc == NULL)
                return (1);

            src_len = flatten (psrc, 1, 1, new_srcq);

            newsrc_cnt += src_len;
            psrc = (NetReference *) srcq -> get ();
            src_len = getnetlength (psrc);
        }

        if (dst_action == 1) {
            if (pdst == NULL)
                return (-1);
   	    new_dstq -> put ((Link *) pdst);
            newdst_cnt += dst_len;
            pdst = (NetReference *) dstq -> get ();
            dst_len = getnetlength(pdst);
        }
        else if (dst_action == 2) {
            if (pdst == NULL)
                return (-1);

            dst_len = flatten (pdst, 1, 1, new_dstq);

            newdst_cnt += dst_len;
            pdst = (NetReference *) dstq -> get ();
            dst_len = getnetlength (pdst);
        }
    }

    delete srcq;
    delete dstq;

    if(newsrc_cnt < newdst_cnt)
	return (1);
    else if(newsrc_cnt > newdst_cnt)
	return (-1);
    else
        return (0);
}

int chk_bounds (Stack * xs1, Stack *xs2)
{
    int     i, xs1_dim, xs2_dim;
    int     lb1, lb2, rb1, rb2, tmp;
    char  **px1, **px2;

    if (xs1 && xs2) {
	/* both net definition and net reference have array structure */
	xs1_dim = xs1 -> length ();
	xs2_dim = xs2 -> length ();
	if (xs1_dim != xs2_dim) { /* the arrays have different dimension */
	    return (1);
	}
	px1 = xs1 -> base ();
	px2 = xs2 -> base ();
	for (i = 0; i < xs1_dim; i++) {
	    lb1 = ((Xelem *) * px1) -> left_bound;
	    rb1 = ((Xelem *) * px1) -> right_bound;
	    if (rb1 < lb1) { tmp = lb1; lb1 = rb1; rb1 = tmp; }
	    lb2 = ((Xelem *) * px2) -> left_bound;
	    rb2 = ((Xelem *) * px2) -> right_bound;
	    if (rb2 < lb2) { tmp = lb2; lb2 = rb2; rb2 = tmp; }
	    /* bounds of array2 must be in range of array1 */
	    if (lb1 > lb2 || rb1 < rb2)
		/* the arrays have inconsistent bounds */
	        return (1);
	    px1++;
	    px2++;
	}
    }
    else if (xs1 || xs2) return (1);

    return (0); /* bounds OK */
}

int inst_net_eqv (NetworkInstance * inst, Queue * cons)
{
    switch (cons -> type) {
	case PmQueue:
	    return(pm_net_eqv (inst, cons));
	case ImQueue:
	    return(im_net_eqv (inst, cons));
	default:
	    fprintf (stderr, "inst_net_eqv: Illegal Queue type %x\n",
			     cons -> type);
	    die();
    }

    return (0);
}

static int pm_net_eqv (NetworkInstance * inst, Queue * netq)
{
    int     i, j, k;
    Stack **net_xs;
    Netelem * pterm;
    NetReference *term_ref;

    Queue * term_refq =  new Queue (QueueType);
    Stack * inst_xs = stackcpy (inst -> inst_struct -> inst_construct);
    Queue * termq = inst -> ntw -> termq;
    int     termq_len = termq -> length ();

    for (j = 0, pterm = (Netelem *) termq -> first_elem ();
	    j < termq_len;
	    j++, pterm = (Netelem *) termq -> next_elem ((Link *) pterm)) {

        if (pterm -> xs) {
	    net_xs = flatten_xs (pterm -> xs);
	    for (i = 0; i < newxs_len; i++) {
		term_ref = new NetReference (pterm, net_xs[i]);
		term_ref -> inst = inst;
		term_ref -> inst_xs = inst_xs;
		term_refq -> put ((Link *) term_ref);
	    }
	    delete net_xs;
        }
        else {
	    term_ref = new NetReference (pterm, 0);
	    term_ref -> inst = inst;
	    term_ref -> inst_xs = inst_xs;
	    term_refq -> put ((Link *) term_ref);
        }
    }

    k = match_net (&netq, &term_refq, 'p');

    if (k < -900) return (1);

    if (k > 0) {
	sls_errno = ATERM_GT_FTERM;
	return (1);
    }
    else if (k < 0) {
	sls_errno = ATERM_LT_FTERM;
	return (1);
    }
    else
        net_eqv (netq, term_refq);

    return (0);
}

static int im_net_eqv (NetworkInstance * inst, Queue * netq)
{
    int     i, j, k;
    Stack * pxs;
    Stack **inst_xs;
    Netelem * pterm;
    NetReference *term_ref;
    Queue * termq = inst -> ntw -> termq;
    Queue * term_refq = new Queue (QueueType);
    int	    termq_len =  termq -> length ();

    if (inst -> inst_struct -> inst_construct) {
	inst_xs = flatten_xs (inst -> inst_struct -> inst_construct);
	k = newxs_len;
    }
    else {
	inst_xs = 0;
	k = 1;
    }

    for (i = 0; i < k; i++) {
	pxs = inst_xs ? inst_xs[i] : 0;
	for (j = 0, pterm = (Netelem *) termq -> first_elem ();
		j < termq_len;
		j++, pterm = (Netelem *) termq -> next_elem ((Link *) pterm)) {

	    term_ref = new NetReference (pterm, stackcpy(pterm -> xs));
	    term_ref -> inst = inst;
	    term_ref -> inst_xs = pxs;
	    term_refq->put ((Link *) term_ref);
	}
    }

    if (inst_xs) delete inst_xs;

    k = match_net (&netq, &term_refq, 'i');

    if (k < -900) return (1);

    if (k > 0) {
	sls_errno = ATERM_GT_FTERM;
	return (1);
    }
    else if (k < 0) {
	sls_errno = ATERM_LT_FTERM;
	return (1);
    }
    else
        net_eqv (netq, term_refq);

    return (0);
}

static void net_eqv (Queue * dst, Queue * src)
{
    Netelem * pnet;
    NetReference * dst_ref, *src_ref;

    while(! dst -> empty())
    {
	dst_ref = (NetReference *) dst -> get ();
	src_ref = (NetReference *) src -> get ();
	src_ref -> ref_xs = stackcpy (dst_ref -> net_xs);

	pnet = dst_ref -> net;
	if (doSimpleNet) {
	    joinSimple (pnet, src_ref -> ref_xs,
			src_ref -> inst, src_ref -> inst_xs,
			src_ref -> net, src_ref -> net_xs);
	    delete src_ref;
	}
	else {
	    if (pnet -> eqv == NULL)
		pnet -> eqv = new Queue (QueueType);

	    pnet -> eqv -> put (src_ref);
	}
	delete dst_ref;
    }
    delete dst;
    delete src;
}

int    neteqv (Queue * netq)
{
     int i, k;
     Link * psrc;
     Stack **dst_net_xs;
     Stack **src_net_xs;
     Netelem * dst_net;
     NetReference * pnref;
     NetReference * src_ref, *dst_ref;

     Link * pdst = netq -> get();

     switch(pdst->type)
     {
	 case QueueType:
             while(! netq->empty()) {
	        psrc = netq -> get ();

	        k = match_net ( ((Queue **) &pdst),  ((Queue **) &psrc), '\0');

		if (k < -900) return (1);

	        if (k > 0) {
		    sls_errno = DNET_GT_SNET;
		    return(1);
    		}
	        else if (k < 0) {
		    sls_errno = DNET_LT_SNET;
		    return(1);
    		}
	        else
		    net_eqv ((Queue *) pdst, (Queue *) psrc);
             }
	     delete netq;
	     break;

	 case NetRefType:
	     dst_ref = (NetReference *) pdst;
	     dst_net = dst_ref -> net;
	     if (dst_net -> eqv == NULL && !doSimpleNet)
	         dst_net -> eqv = new Queue (QueueType);

	     dst_net_xs = 0;
	     if (dst_ref -> net_xs) {
		 dst_net_xs = flatten_xs (dst_ref -> net_xs);

		 for (i = 0; i < newxs_len; i++) {
		     pnref = new NetReference (dst_ref->net, dst_net_xs[i]);
		     pnref -> ref_xs = stackcpy (dst_net_xs[0]);
		     pnref -> inst = dst_ref -> inst;
		     pnref -> inst_xs = dst_ref -> inst_xs;
		     if (doSimpleNet) {
			 joinSimple (dst_net, pnref -> ref_xs,
				     pnref -> inst, pnref -> inst_xs,
				     pnref -> net, pnref -> net_xs);
			 delete pnref;
		     }
		     else
			 dst_net -> eqv -> put (pnref);
		 }
	     }

             while(! netq->empty()) {
	         psrc = netq -> get ();
		 src_ref = (NetReference *) psrc;

		 if (src_ref -> net_xs) {
		     src_net_xs = flatten_xs (src_ref -> net_xs);

		     for (i = 0; i < newxs_len; i++) {
		         pnref = new NetReference (src_ref->net, src_net_xs[i]);
	     		 if (dst_net_xs)
		             pnref -> ref_xs = stackcpy (dst_net_xs[0]);
		         pnref -> inst = src_ref -> inst;
		         pnref -> inst_xs = src_ref -> inst_xs;
			 if (doSimpleNet) {
			     joinSimple (dst_net, pnref -> ref_xs,
					 pnref -> inst, pnref -> inst_xs,
					 pnref -> net, pnref -> net_xs);
			     delete pnref;
			 }
			 else
			     dst_net -> eqv -> put (pnref);
		     }
		     if (src_net_xs) delete src_net_xs;
		     delete src_ref;
		 }
		 else {
		     if (dst_net_xs)
		         src_ref -> ref_xs = stackcpy (dst_net_xs[0]);
		     if (doSimpleNet) {
			 joinSimple (dst_net, src_ref -> ref_xs,
				     src_ref -> inst, src_ref -> inst_xs,
				     src_ref -> net, src_ref -> net_xs);
			 delete src_ref;
		     }
		     else
			 dst_net -> eqv -> put (src_ref);
		 }
	     }

	     if (dst_net_xs) delete dst_net_xs;
	     delete dst_ref;
	     delete netq;
	     break;
     }
     return(0);
}

int getxslength (Stack * xs)
{
    int len = 1;

    if (xs) {
	int k, n;
	char **q = xs -> base ();

	for (n = xs -> length (); n-- > 0; ++q) {
	    k = ((Xelem *) *q) -> right_bound - ((Xelem *) *q) -> left_bound;
	    if (k < 0) len *= 1 - k;
	    else       len *= 1 + k;
	}
    }

    return (len);
}

static int getnetlength (NetReference * net)
{
    if (!net) return (0);

    return (getxslength (net -> net_xs) * getxslength (net -> inst_xs));
}

int getnetcnt (Queue * netq)
{
    int i;
    int net_cnt = 0;
    int netq_len = netq -> length();
    NetReference * pq;

    for(i=0, pq = (NetReference *) netq -> first_elem();
        i < netq_len;
        i++, pq = (NetReference *) netq -> next_elem((Link *) pq))
    {
	net_cnt += getnetlength(pq);
    }

    return (net_cnt);
}

int gettermcnt (Network *netw)
{
    Queue *termq;
    Netelem *pterm;
    int i, termq_len;
    int term_cnt = 0;

    termq = netw -> termq;
    termq_len = termq -> length ();

    for (i = 0, pterm = (Netelem *) termq -> first_elem ();
	i < termq_len;
	i++, pterm = (Netelem *) termq -> next_elem ((Link *) pterm))
    {
	term_cnt += getxslength (pterm -> xs);
    }

    return (term_cnt);
}

Netelem *findterm (NetworkInstance *inst, char *term_name)
{
    Queue *termq;
    Netelem *pterm;
    int i, termq_len;

    if (inst)
	termq = inst -> ntw -> termq;
    else
	termq = ntw -> termq;

    termq_len = termq -> length ();

    for (i = 0, pterm = (Netelem *) termq -> first_elem ();
	i < termq_len;
	i++, pterm = (Netelem *) termq -> next_elem ((Link *) pterm))
    {
	if (strcmp (pterm -> name, term_name) == 0) return (pterm);
    }

    return (0);
}
