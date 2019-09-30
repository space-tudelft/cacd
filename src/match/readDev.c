static char *rcsid = "$Id: readDev.c,v 1.1 2018/04/30 12:17:48 simon Exp $";
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

/*	VARIABLES:
 */
Import struct model_info *impNetws;
Import network * cur_network;
Import hash *def_h;
Import boolean c_opt, r_opt;
Import string string_n, string_p;
Import string string_x, string_y;
Import string string_v, string_w, string_l;
Import string string_cap, string_res;
Import string string_inv, string_nand, string_nor;
Import string string_exor, string_and, string_or;

Private void merge_res (void);

/*
 * Attaches sub-networks to the current network by
 * reading the model calls (again). Terminals which were
 * read earlier will be attached to this device.
 * Devices (e.g. nenh, ndep, etc.), resistors, capacitors,
 * and functions (e.g. nand, or, etc) are created here.
 */
void readDev (DM_CELL *dkey)
{
    DM_PROJECT *proj;
    DM_STREAM * dsp;
    struct model_info *mod;
    network * n_network;
    object *tmp, *cur_device;
    char   *nme, nme1[DM_MAXNAME+20];
    long    ninputs, prime;
    int  i, ok;

    dsp = dmOpenStream (dkey, "mc", "r");
    nme = cmc.cell_name;
    ok = 0;
    while (dmGetDesignData (dsp, CIR_MC) > 0) {

	if ((i = is_dev ())) {
	    cur_device = NULL;
	    switch (i) {
	    case 'c': /* capasitor */
		if (c_opt) continue; /* skip */
		if (!(n_network = h_get (def_h, nme))) {
		    n_network = mk_network (nme, PRIMITIVE);
		    tmp = mk_object (string_p, TERMINAL); add_object (n_network, tmp); tmp -> color = 2;
		    tmp = mk_object (string_n, TERMINAL); add_object (n_network, tmp); tmp -> color = 2;
		    h_link (def_h, n_network -> name, n_network);
		}
		cur_device = mk_device (cmc.inst_name, n_network);
		set_parpair (cur_device, cmc.inst_attribute, string_v);
		set_parpair (cur_device, cmc.inst_attribute, string_x);
		set_parpair (cur_device, cmc.inst_attribute, string_y);
		break;
	    case 'r': /* resistor */
		if (r_opt) { merge_res (); continue; }
		if (!(n_network = h_get (def_h, nme))) {
		    n_network = mk_network (nme, PRIMITIVE);
		    tmp = mk_object (string_p, TERMINAL); add_object (n_network, tmp); tmp -> color = 2;
		    tmp = mk_object (string_n, TERMINAL); add_object (n_network, tmp); tmp -> color = 2;
		    h_link (def_h, n_network -> name, n_network);
		}
		cur_device = mk_device (cmc.inst_name, n_network);
		set_parpair (cur_device, cmc.inst_attribute, string_v);
		set_parpair (cur_device, cmc.inst_attribute, string_x);
		set_parpair (cur_device, cmc.inst_attribute, string_y);
		break;
	    case 'd': /* transistor */
		if (!(n_network = h_get (def_h, nme))) {
		    n_network = mk_network (nme, PRIMITIVE);
		    rewind_gen ();
		    prime = gen_prime ();
		    tmp = mk_object ("g", TERMINAL); add_object (n_network, tmp); tmp -> color = prime;
		    prime = gen_prime ();
		    tmp = mk_object ("d", TERMINAL); add_object (n_network, tmp); tmp -> color = prime;
		    tmp = mk_object ("s", TERMINAL); add_object (n_network, tmp); tmp -> color = prime;
		    h_link (def_h, n_network -> name, n_network);
		}
		cur_device = mk_device (cmc.inst_name, n_network);
		set_parpair (cur_device, cmc.inst_attribute, string_w);
		set_parpair (cur_device, cmc.inst_attribute, string_l);
		set_parpair (cur_device, cmc.inst_attribute, string_x);
		set_parpair (cur_device, cmc.inst_attribute, string_y);
	    }
	}
	else if (is_func ()) { /* function */

		if (strcmp (nme, string_inv) == 0
		 || strcmp (nme, string_nand) == 0
		 || strcmp (nme, string_nor) == 0
		 || strcmp (nme, string_and) == 0
		 || strcmp (nme, string_or) == 0
		 || strcmp (nme, string_exor) == 0) {

		    if (get_ival (cmc.inst_attribute, string_n, &ninputs) == 0) {
			Assert (ninputs > 0 && ninputs < 10);
		    }
		    else ninputs = 1;
		}
		else ninputs = 0;

		if (ninputs > 1) { /* standard func */
		    sprintf (nme1, "@%s%ld", nme, ninputs);
		} else {
		    sprintf (nme1, "@%s", nme);
		}

		/* create function network if not yet existing */
		if (!(n_network = h_get (def_h, nme1))) {
		    if (ninputs > 0) { /* standard func */
			rewind_gen ();
			prime = gen_prime ();
			n_network = mk_network (nme1, PRIMITIVE);
			for (i = 0; i < ninputs; ++i) {
			    sprintf (nme1, "i[%d]", i);
			    tmp = mk_object (nme1, TERMINAL); add_object (n_network, tmp);
			    tmp -> color = prime;
			}
			tmp = mk_object ("o", TERMINAL); add_object (n_network, tmp);
			tmp -> color = gen_prime ();
		    }
		    else {
			DM_CELL *ckey;
			proj = dkey -> dmproject;
			ckey = dmCheckOut (proj, nme, WORKING, DONTCARE, CIRCUIT, READONLY);
			if (proj -> projectno != 0) {
			    sprintf (nme1, "@p%d:%s", proj -> projectno, nme);
			}
			if (!ckey) err_mesg ("Function: '%s' not extracted!\n", nme1);
			n_network = mk_network (nme1, PRIMITIVE);
			readTerm (n_network, ckey, 1);
			dmCheckIn (ckey, COMPLETE);
		    }
		    h_link (def_h, n_network -> name, n_network);
		}
		cur_device = mk_device (cmc.inst_name, n_network);
		set_parpair (cur_device, cmc.inst_attribute, "tr");
		set_parpair (cur_device, cmc.inst_attribute, "tf");
		set_parpair (cur_device, cmc.inst_attribute, string_x);
		set_parpair (cur_device, cmc.inst_attribute, string_y);
	}
	else { /* network */
		proj = dkey -> dmproject;

		if (cmc.imported == IMPORTED) {
		    for (mod = impNetws; mod; mod = mod -> next) {
			if (mod -> proj != proj) continue;
			if (strcmp (mod -> name, nme) == 0) {
			    proj = mod -> impproj;
			    nme  = mod -> orig_name;
			    break;
			}
		    }
		}
		if (proj -> projectno != 0) {
		    /* expand name with foreign project number */
		    sprintf (nme1, "p%d:%s", proj -> projectno, nme); nme = nme1;
		}
		if (!(n_network = h_get (def_h, nme))) goto ret;
		cur_device = mk_device (cmc.inst_name, n_network);
		set_parpair (cur_device, cmc.inst_attribute, string_x);
		set_parpair (cur_device, cmc.inst_attribute, string_y);
		nme = cmc.cell_name;
	}

	if (cur_device) {
	    for (i = cmc.inst_dim; --i >= 0;) {
		array (cur_device, cmc.inst_lower[i], cmc.inst_upper[i]);
	    }
	    do_connect (cur_device);
	    add_object (cur_network, cur_device);
	}
    }
    ok = 1;
ret:
    dmCloseStream (dsp, COMPLETE);
    if (!ok) {
	err_mesg ("Cannot find netw '%s' while reading MC of '%s'\n", nme, cur_network -> name);
    }
}

Private void merge_res ()
{
    pair *cur_pair, *head_pairs;
    char nme[DM_MAXNAME+33];
    object *net1, *net2;
    int i;

    head_pairs = NULL;
    if (cmc.inst_dim > 0) {
	for (i = 0; i < cmc.inst_dim; ++i) {
	    cur_pair = mk_pair ();
	    cur_pair -> first = cmc.inst_lower[i];
	    cur_pair -> index = cmc.inst_lower[i];
	    cur_pair -> last  = cmc.inst_upper[i];
	    cur_pair -> next = head_pairs;
	    head_pairs = cur_pair;
	}
	make_index (cmc.inst_name, head_pairs, nme);
    }
    else {
	strcpy (nme, cmc.inst_name);
    }

beg:
    i = strlen (nme);
    nme[i] = '.';
    nme[i+1] = 'p';
    nme[i+2] = '\0';
    net1 = is_subterm (nme);

    if (net1) {
	nme[i+1] = 'n';
	net2 = is_subterm (nme);
	if (net2) {
	    list *head_list = NULL;
	    head_list = add_list (head_list, net2);
	    head_list = add_list (head_list, net1);
	    merge (cur_network, head_list);
	}
    }

    if (head_pairs) {
	if (incr_index (head_pairs)) {
	    make_index (cmc.inst_name, head_pairs, nme);
	    goto beg;
	}
    }
}
