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

extern Netelem *notconnected;

/*local operations */

#ifdef __cplusplus
  extern "C" {
#endif

static void fill_lub_cnet (struct cir_net *cnet, Stack *xs);
static void fill_lub_ref  (struct cir_net *cnet, Stack *xs);
static void fill_lub_inst (struct cir_net *cnet, Stack *xs);

#ifdef __cplusplus
  }
#endif

extern DM_STREAM * dsp_net;

void    term_to_db (Queue * q)
{
    int     i;
    Link * pq;

    if (!q) return;

    for (i = 0, pq = q -> first_elem ();
	    i < q -> length ();
	    i++, pq = q -> next_elem (pq)) {
	switch (pq -> type) {
	    case QueueType:
		term_to_db ((Queue *) pq);
		break;

	    case TermType:
		((Netelem *) pq) -> to_db ();
		break;
	}
    }
}

void    inst_to_db (NetworkInstance * inst)
{
    inst -> to_db ();
}

void    net_to_db (Queue * q)
{
    int     i,
            j;
    Queue * pq;
    Netelem * pnet;
    NetReference * peqv;

    if (q -> empty ()) {
	delete q;
	return;
    }

    if (doSimpleNet) {
	outSimple (q);
    }
    else {
	for (i = 0, pnet = (Netelem *) q -> first_elem ();
		i < q -> length ();
		i++, pnet = (Netelem *) q -> next_elem ((Link *) pnet)) {

	    if (pnet == notconnected)
	        continue;             /* this net is not stored in the db */

	    strcpy (cnet.net_name, pnet -> name);
	    fill_lub_cnet (&cnet, pnet -> xs);

	    pq = pnet -> eqv;
	    if (pq) {

		cnet.net_neqv = pq -> length ();
		cnet.net_eqv = new struct cir_net   [cnet.net_neqv];

		for (j=0; ! pq->empty(); j++) {
		    peqv = (NetReference *) pq -> get();
                    if (peqv -> net == notconnected) {
                        delete peqv;
                        cnet.net_neqv--;
			j--;
                        continue;       /* this one is not stored in the db */
                    }
		    strcpy (cnet.net_eqv[j].net_name, peqv -> net -> name);
		    fill_lub_cnet (&(cnet.net_eqv[j]), peqv -> net_xs);
		    fill_lub_ref (&(cnet.net_eqv[j]), peqv -> ref_xs);

		    if (peqv -> inst)
		    {
			strcpy (cnet.net_eqv[j].inst_name,
				peqv -> inst -> inst_struct -> inst_name);
			if(peqv -> inst_xs)
			    fill_lub_inst(&cnet.net_eqv[j], peqv->inst_xs);
		    }

		    delete peqv;
		}
		delete pq;
	    }

	    dmPutDesignData (dsp_net, CIR_NET);

	    if(cnet.net_dim)
	    {
#ifdef DMEM
		int_nbyte -= 2 * cnet.net_dim * sizeof(int);
#endif
		delete cnet.net_lower;
		delete cnet.net_upper;
		cnet.net_dim = 0;
	    }

	    if(cnet.inst_dim)
	    {
#ifdef DMEM
		int_nbyte -= 2 * cnet.inst_dim * sizeof(int);
#endif
		delete cnet.inst_lower;
		delete cnet.inst_upper;
		cnet.inst_dim = 0;
	    }

	    if(cnet.ref_dim)
	    {
#ifdef DMEM
		int_nbyte -= 2 * cnet.net_dim * sizeof(int);
#endif
		delete cnet.ref_lower;
		delete cnet.ref_upper;
		cnet.ref_dim = 0;
	    }

	    if(cnet.net_neqv)
	    {
		delete cnet.net_eqv;
		cnet.net_neqv = 0;
	    }
	}

	while(!q -> empty()) {
	    pnet = (Netelem *) q -> get();
	    delete pnet;
	}
	delete q;
    }
}

static void fill_lub_cnet (struct cir_net *cnet, Stack * xs)
{
    int j, size;
    char  **pxs;

    if (xs) {
	cnet -> net_dim = size = xs -> length ();
#ifdef DMEM
    int_nbyte += 2 * size * sizeof(int);
    if (int_nbyte > int_maxnbyte) int_maxnbyte = int_nbyte;
#endif
	cnet -> net_lower = new long [size];
	cnet -> net_upper = new long [size];

	pxs = xs -> base ();
	for (j = 0; j < size; j++) {
	    cnet -> net_lower[j] = ((Xelem *) * pxs) -> left_bound;
	    cnet -> net_upper[j] = ((Xelem *) * pxs) -> right_bound;
	    ++pxs;
	}
    }
    else
	cnet -> net_dim = 0;
}

static void fill_lub_ref (struct cir_net *cnet, Stack * xs)
{
    int j, size;
    char  **pxs;

    if (xs) {
	cnet -> ref_dim = size = xs -> length ();
#ifdef DMEM
    int_nbyte += 2 * size * sizeof(int);
    if (int_nbyte > int_maxnbyte) int_maxnbyte = int_nbyte;
#endif
	cnet -> ref_lower = new long [size];
	cnet -> ref_upper = new long [size];
	pxs = xs -> base ();
	for (j = 0; j < size; j++) {
	    cnet -> ref_lower[j] = ((Xelem *) * pxs) -> left_bound;
	    cnet -> ref_upper[j] = ((Xelem *) * pxs) -> right_bound;
	    ++pxs;
	}
    }
    else
	cnet -> ref_dim = 0;
}

static void fill_lub_inst (struct cir_net *cnet, Stack * xs)
{
    int j, size;
    char  **pxs;

    if (xs) {
	cnet -> inst_dim = size = xs -> length ();
#ifdef DMEM
    int_nbyte += 2 * size * sizeof(int);
    if (int_nbyte > int_maxnbyte) int_maxnbyte = int_nbyte;
#endif
	cnet -> inst_lower = new long [size];
	cnet -> inst_upper = new long [size];
	pxs = xs -> base ();
	for (j = 0; j < size; j++) {
	    cnet -> inst_lower[j] = ((Xelem *) * pxs) -> left_bound;
	    cnet -> inst_upper[j] = ((Xelem *) * pxs) -> right_bound;
	    ++pxs;
	}
    }
    else
	cnet -> inst_dim = 0;
}
