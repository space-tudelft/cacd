static char *rcsid = "$Id: xnetw.c,v 1.1 2018/04/30 12:17:55 simon Exp $";
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	T. Vogel
 *	A.J. van Genderen
 *	S. de Graaf
 *	A.J. van der Hoeven
 *	N.P. van der Meijs
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

#include "src/match/head.h"
#include "src/match/incl.h"
#include "src/match/proto.h"

#define PRIME	3029

Import network * cur_network;
Import stack * net_st;
Import string string_x, string_y;
Public hash *slist; /* hash list of subterminals */

Public void xnetwork (DM_PROJECT *projkey, char *ntwname, DM_CELL *cell_key)
{
    char nme1[DM_MAXNAME+10];
    DM_CELL *dkey;

    if (!(dkey = cell_key)) {
	dkey = dmCheckOut (projkey, ntwname, WORKING, DONTCARE, CIRCUIT, READONLY);
    }
    if (dkey) {
	if (projkey -> projectno != 0) {
	    sprintf (nme1, "p%d:%s", projkey -> projectno, ntwname);
	    ntwname = nme1;
	}
	cur_network = mk_network (ntwname, COMPOUND);
	slist = mk_hash (PRIME);

	readTerm (cur_network, dkey, 0);
	readNet  (dkey);
	readDev  (dkey);

	rm_hash (slist); slist = NULL;

	time_progress ("read data of network", cur_network -> name);

	if (!cell_key) dmCheckIn (dkey, COMPLETE);
    }
    else
	cur_network = NULL;
}

/* Reads the terminals of the model.
   If the 'funf' argument is 1, the model is
   considered to be a function (not needed here).
*/
void readTerm (network *c_network, DM_CELL *dkey, int funf)
{
    DM_STREAM * dsp;
    object * cur_object;
    int i;

    if (funf) {
	dsp = dmOpenStream (dkey, "fterm", "r");
    } else {
	dsp = dmOpenStream (dkey, "term", "r");
    }

    while (dmGetDesignData (dsp, CIR_TERM) > 0) {

	cur_object = mk_object (cterm.term_name, TERMINAL);
	set_parpair (cur_object, cterm.term_attribute, string_x);
	set_parpair (cur_object, cterm.term_attribute, string_y);

	for (i = cterm.term_dim; --i >= 0;) {
	    array (cur_object, cterm.term_lower[i], cterm.term_upper[i]);
	}
	add_object (c_network, cur_object);
    }

    dmCloseStream (dsp, COMPLETE);
}

void make_indices (char *s, int dim, long lower[])
{
    int i = 0;
    s += strlen (s);
    sprintf (s, "[%ld]", lower[i]);
    while (++i < dim) {
	s += strlen (s) - 1;
	sprintf (s, ",%ld]", lower[i]);
    }
}

static object ** nets;
static int nr_of_nets;

object *find_ref (char *name)
{
    int i;
    for (i = 0; i < nr_of_nets; ++i) {
	if (strcmp (name, nets[i] -> name) == 0) return nets[i];
    }
    err_mesg ("Incorrect net_ref '%s' in NET of '%s'\n", name, cur_network -> name);
    return NULL;
}

void readNet (DM_CELL *dkey)
{
    DM_STREAM *dsp;
    struct cir_net *cur_subnet, *cur_net;
    string nme, nmi;
    char   nme1[DM_MAXNAME+DM_MAXNAME+2];
    char   nme2[DM_MAXNAME+DM_MAXNAME+20];
    char   cur_net_name[DM_MAXNAME+1];
    pair  *cur_pair, *head_pairs, *sub_pairs, *ref_pairs, *ins_pairs;
    list  *sub_list;
    object *tmp, *head_net;
    int i, j, dim, net_neqv;

    dsp = dmOpenStream (dkey, "net", "r");

    while (dmGetDesignData (dsp, CIR_NET_ATOM) > 0) {

	cur_net = &cnet;

	/* process head_net first */
	head_pairs = NULL;
	nme = cur_net -> net_name;

	if ((dim = cur_net -> net_dim) > 0) { /* check, if it is a single net */
	    for (i = 0; i < dim; i++) {
		if (cur_net -> net_lower[i] != cur_net -> net_upper[i]) break;
	    }
	    if (i < dim) { /* net array */
		nr_of_nets = 1;
		for (i = 0; i < dim; i++) {
		    cur_pair = mk_pair ();
		    cur_pair -> first = cur_net -> net_lower[i];
		    cur_pair -> index = cur_net -> net_lower[i];
		    cur_pair -> last  = cur_net -> net_upper[i];
		    cur_pair -> next = head_pairs;
		    head_pairs = cur_pair;

		    j = cur_net -> net_upper[i] - cur_net -> net_lower[i];
		    if (j < 0) j = -j;
		    nr_of_nets *= j + 1;
		}
	    }
	    else { /* single net */
		nme = strcpy (nme1, nme);
		make_indices (nme, dim, cur_net -> net_lower);
	    }
	}

	net_neqv = cur_net -> net_neqv;

	if (head_pairs == NULL) { /* simple (not arrayed) net */
	    if (!(head_net = is_tnet (cur_network, nme))) {
		head_net = mk_object (nme, NET); add_object (cur_network, head_net);
		set_parpair (head_net, cur_net -> net_attribute, string_x);
		set_parpair (head_net, cur_net -> net_attribute, string_y);
	    }

	    /* process all equivalent simple sub_nets */
	    sub_list = NULL;
	    for (j = 0; j < net_neqv; j++) {
		if (dmGetDesignData (dsp, CIR_NET_ATOM) <= 0) err_mesg ("subnet read error\n");
		cur_subnet = &cnet;

		nme = cur_subnet -> net_name;
		nmi = cur_subnet -> inst_name;
		if (*nmi) {
		    if ((i = cur_subnet -> inst_dim) > 0) {
			nmi = strcpy (nme1, nmi);
			make_indices (nmi, i, cur_subnet -> inst_lower);
		    }
		    sprintf (nme2, "%s.%s", nmi, nme);
		    if ((i = cur_subnet -> net_dim) > 0) {
			make_indices (nme2, i, cur_subnet -> net_lower);
		    }
		    /* net object is not made for hash-key 'nme2' */
		    h_link (slist, mk_symbol (nme2), head_net);
		}
		else {
		    if ((i = cur_subnet -> net_dim) > 0) {
			nme = strcpy (nme2, nme);
			make_indices (nme, i, cur_subnet -> net_lower);
		    }
		    if (!(tmp = is_tnet (cur_network, nme))) {
			tmp = mk_object (nme, NET); add_object (cur_network, tmp);
		    }
		    sub_list = add_list (sub_list, tmp);
		}
	    }
	    if (sub_list) {
		sub_list = add_list (sub_list, head_net);
		push (net_st, sub_list);
	    }
	}
	else { /* arrayed net */
	    /* normally only used to define terminal/local-net arrays */
	    /* thus, normally there are no subnets and net_neqv = 0 */

	    /* we need 'nets' for find_ref */
	    MALLOC (nets, nr_of_nets, object*);
	    j = 0;
	    strcpy (cur_net_name, cur_net -> net_name);
	    do { /* expand this net */
		make_index (cur_net_name, head_pairs, nme2);
		if (!(tmp = is_tnet (cur_network, nme2))) {
		    tmp = mk_object (nme2, NET); add_object (cur_network, tmp);
		}
		Assert (j < nr_of_nets);
		nets[j++] = tmp;
	    } while (incr_index (head_pairs));

	    Assert (j == nr_of_nets);

	    /* process each equivalent subnet individually */
	    for (j = 0; j < net_neqv; j++) {
		if (dmGetDesignData (dsp, CIR_NET_ATOM) <= 0) err_mesg ("subnet read error\n");
		cur_subnet = &cnet;
		ref_pairs = NULL;
		sub_pairs = NULL;
		ins_pairs = NULL;

		Assert (cur_subnet -> ref_dim == dim);
		/* head_pairs is re-used for ref_pairs */
		for (i = 0; i < cur_subnet -> ref_dim; i++) {
		    cur_pair = head_pairs; head_pairs = head_pairs -> next;
		    cur_pair -> first = cur_subnet -> ref_lower[i];
		    cur_pair -> index = cur_subnet -> ref_lower[i];
		    cur_pair -> last  = cur_subnet -> ref_upper[i];
		    cur_pair -> next = ref_pairs;
		    ref_pairs = cur_pair;
		}
		head_pairs = ref_pairs;
		for (i = 0; i < cur_subnet -> net_dim; i++) {
		    cur_pair = mk_pair ();
		    cur_pair -> first = cur_subnet -> net_lower[i];
		    cur_pair -> index = cur_subnet -> net_lower[i];
		    cur_pair -> last  = cur_subnet -> net_upper[i];
		    cur_pair -> next = sub_pairs;
		    sub_pairs = cur_pair;
		}
		nmi = cur_subnet -> inst_name;
		if (*nmi)
		for (i = 0; i < cur_subnet -> inst_dim; i++) {
		    cur_pair = mk_pair ();
		    cur_pair -> first = cur_subnet -> inst_lower[i];
		    cur_pair -> index = cur_subnet -> inst_lower[i];
		    cur_pair -> last  = cur_subnet -> inst_upper[i];
		    cur_pair -> next = ins_pairs;
		    ins_pairs = cur_pair;
		}

		i = 0;
		do {
		    nme = cur_subnet -> net_name;
		    if (*nmi) { /* inst_name */
			if (ins_pairs) {
			    make_index (nmi, ins_pairs, nme2);
			    sprintf (nme1, "%s.%s", nme2, nme);
			}
			else sprintf (nme1, "%s.%s", nmi, nme);
			nme = nme1;
		    }
		    do {
			if (!i) i = 1;
			else if (!incr_index (ref_pairs) && i++ == 1) verb_mesg ("WARNING: ref used twice\n");
			if (sub_pairs) { make_index (nme, sub_pairs, nme2); nme = nme2; }

			if (*nmi) { /* inst_name */
			    nme = mk_symbol (nme); /* do this first before re-using nme2 */
			    make_index (cur_net_name, ref_pairs, nme2); tmp = find_ref (nme2);
			    /* net object is not made for hash-key 'nme' */
			    h_link (slist, nme, tmp);
			}
			else {
			    if (!(tmp = is_tnet (cur_network, nme))) {
				tmp = mk_object (nme, NET); add_object (cur_network, tmp);
			    }
			    sub_list = add_list (NULL, tmp);
			    make_index (cur_net_name, ref_pairs, nme2); tmp = find_ref (nme2);
			    sub_list = add_list (sub_list, tmp);
			    push (net_st, sub_list);
			}
		    } while (sub_pairs && incr_index (sub_pairs));
		} while (ins_pairs && incr_index (ins_pairs));

		/* clean up */
		while ((cur_pair = ins_pairs)) {
		    ins_pairs = ins_pairs -> next;
		    rm_pair (cur_pair);
		}
		while ((cur_pair = sub_pairs)) {
		    sub_pairs = sub_pairs -> next;
		    rm_pair (cur_pair);
		}
	    }

	    FREE (nets);

	    while ((cur_pair = head_pairs)) {
		head_pairs = head_pairs -> next;
		rm_pair (cur_pair);
	    }
	}
    }
    dmCloseStream (dsp, COMPLETE);
}
