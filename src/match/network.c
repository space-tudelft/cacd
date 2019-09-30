static char *rcsid = "$Id: network.c,v 1.1 2018/04/30 12:17:40 simon Exp $";
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
/*
 * This module contains a number of routines which
 * implement some 'high'-level functions on a
 * (hierarchical) network datastructure.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

Import boolean D_opt;

/*
 * Makes a copy of the given network.
 * A pointer to the new network is returned.
 * All names remain the same (terminals, nets, etc.).
 */
Private network *cp_network (network *netw)
{
    object *t_p, *n_p, *d_p, *t1, *t2, *t3, *t4;
    object *d_new, *tmp;
    network * new_netw;
    link_type *l_p, *n_l;

    if (!netw -> nhead && !netw -> dhead) {
       prnt_mesg ("Warning: Network '%s' is empty, but not declared primitive\n", netw -> name);
    }

    /* copy network body */

    new_netw = mk_network (netw -> name, COMPOUND);

    new_netw -> color	= netw -> color;
    new_netw -> flags   = netw -> flags;
    new_netw -> n_terms = netw -> n_terms;

    /* copy terminal list */
    for (t_p = netw -> thead; t_p; t_p = t_p -> next_obj) {
	tmp = mk_object (t_p -> name, TERMINAL);
	tmp -> netw = new_netw;
	tmp -> color = t_p -> color;
	if (new_netw -> thead == NULL)
	    new_netw -> thead = tmp;
	else
	    new_netw -> ttail -> next_obj = tmp;
	new_netw -> ttail = tmp;
    }

    /* copy net list */
    for (n_p = netw -> nhead; n_p; n_p = n_p -> next_obj) {
	tmp = mk_object (n_p -> name, NET);
	tmp -> netw = new_netw;
	if (new_netw -> nhead == NULL)
	    new_netw -> nhead = tmp;
	else
	    new_netw -> ntail -> next_obj = tmp;
	new_netw -> ntail = tmp;
    }

    /* copy device list */
    for (d_p = netw -> dhead; d_p; d_p = d_p -> next_obj) {
	d_new = mk_device (d_p -> name, d_p -> call);
	d_new -> netw = new_netw;
	d_new -> par_list = d_p -> par_list;
	if (new_netw -> dhead == NULL)
	    new_netw -> dhead = d_new;
	else
	    new_netw -> dtail -> next_obj = d_new;
	new_netw -> dtail = d_new;

	/* copy link objects */
	for (l_p = d_p -> head; l_p; l_p = l_p -> next_down) {
	    n_l = mk_link ();
	    n_l -> dev = d_new;
	    if (!d_new -> head) d_new -> head = n_l;
	    else d_new -> tail -> next_down = n_l;
	    d_new -> tail = n_l;

	    if (l_p -> net) {
		t2 = new_netw -> thead;
		for (t1 = netw -> thead; t1; t1 = t1 -> next_obj, t2 = t2 -> next_obj) {
		    if (t1 == l_p -> net) { /* found */
			Assert (!t1 -> equiv || t1 -> equiv == t1);
			n_l -> net = t2;
			if (!t2 -> head) t2 -> head = n_l;
			else t2 -> tail -> next_up = n_l;
			t2 -> tail = n_l;
			break;
		    }
		}
	      if (!n_l -> net) {
		t2 = new_netw -> nhead;
		for (t1 = netw -> nhead; t1; t1 = t1 -> next_obj, t2 = t2 -> next_obj) {
		    if (t1 == l_p -> net) { /* found */
			Assert (!t1 -> equiv || t1 -> equiv == t1);
			n_l -> net = t2;
			if (!t2 -> head) t2 -> head = n_l;
			else t2 -> tail -> next_up = n_l;
			t2 -> tail = n_l;
			break;
		    }
		}
		Assert (n_l -> net);
	      }
	    }
	}
    }

    /* set equivalence pointers for terminal list */
    for (t1 = netw -> thead, t2 = new_netw -> thead; t1; t1 = t1 -> next_obj, t2 = t2 -> next_obj) {
	if (t1 -> equiv == t1) {
	    Assert (t2 -> equiv == NULL);
	    for (t3 = netw -> thead; t3 != t1; t3 = t3 -> next_obj) Assert (t3 -> equiv != t1);
	    t2 -> equiv = t2;
	    for (t3 = t1 -> next_obj, t4 = t2 -> next_obj; t3; t3 = t3 -> next_obj, t4 = t4 -> next_obj) {
		if (t3 -> equiv == t1) {
		    Assert (t3 -> head == NULL);
		    Assert (t4 -> head == NULL);
		    Assert (t4 -> equiv == NULL);
		    t4 -> equiv = t2;
		}
	    }
	    for (t3 = netw -> nhead, t4 = new_netw -> nhead; t3; t3 = t3 -> next_obj, t4 = t4 -> next_obj) {
		if (t3 -> equiv == t1) {
		    Assert (t3 -> head == NULL);
		    Assert (t4 -> head == NULL);
		    Assert (t4 -> equiv == NULL);
		    t4 -> equiv = t2;
		}
	    }
	}
    }

    /* set equivalence pointers for net list */
    for (t1 = netw -> nhead, t2 = new_netw -> nhead; t1; t1 = t1 -> next_obj, t2 = t2 -> next_obj) {
	if (t1 -> equiv == t1) {
	    Assert (t2 -> equiv == NULL);
	    for (t3 = netw -> thead; t3; t3 = t3 -> next_obj) Assert (t3 -> equiv != t1);
	    for (t3 = netw -> nhead; t3 != t1; t3 = t3 -> next_obj) Assert (t3 -> equiv != t1);
	    t2 -> equiv = t2;
	    for (t3 = t1 -> next_obj, t4 = t2 -> next_obj; t3; t3 = t3 -> next_obj, t4 = t4 -> next_obj) {
		if (t3 -> equiv == t1) {
		    Assert (t3 -> head == NULL);
		    Assert (t4 -> head == NULL);
		    Assert (t4 -> equiv == NULL);
		    t4 -> equiv = t2;
		}
	    }
	}
    }

    return (new_netw);
}

/*
 * This recursive function fully instantiates the given
 * acyclic network graph into a tree structure.
 * The graph is scanned depth-first.
 * Only a copy of a visited network is made.
 * Also connect dev links to the network terminals.
 */
Public void instantiate (network *netw)
{
    object *dev, *t;
    link_type * lnk;

    Set_flag (netw, VISITED);

    for (dev = netw -> dhead; dev; dev = dev -> next_obj) {

	Assert (dev -> call);

	if (Get_field (dev -> call, TYPE) != PRIMITIVE) {
	    if (Get_flag (dev -> call, VISITED)) {
		dev -> call = cp_network (dev -> call);
	    }
	    instantiate (dev -> call);
	}

	if ((lnk = dev -> head) && !lnk -> port) {
	    for (t = dev -> call -> thead; t; t = t -> next_obj) {
		lnk -> port = t;
		if (!(lnk = lnk -> next_down)) break;
	    }
	}
    }
}

/*
 * Expands the devices of a hierarchical network created by 'instantiate'.
 */
Public void expand_netw (network *netw)
{
    object *Device;
    object *next, *net, *t_p, *n_p, *d_p;
    link_type *l_p, *l;
    char name[512];
    network *sub_netw;

    Device = netw -> dhead;

    netw -> dhead = NULL;
    netw -> dtail = NULL;

    while (Device) {

	sub_netw = Device -> call;
	Assert (sub_netw);

	if (Get_field (sub_netw, TYPE) != PRIMITIVE) {

	    /* wire child terminals into father network */

	    for (l_p = Device -> head; l_p; l_p = l_p -> next_down) {

		if (!(net = l_p -> net)) continue;

		t_p = l_p -> port;
		Assert (t_p);
		if (!t_p -> equiv || t_p -> equiv == t_p) {
		    if ((l = t_p -> head)) {
			/* merge net chains */
			Assert (net -> head);
			net -> tail -> next_up = l;
			net -> tail = t_p -> tail;
			/* update home net pointers */
			do { l -> net = net; } while ((l = l -> next_up));
		    }
		}
		t_p -> head = NULL;
		t_p -> tail = NULL;
	    }

	    /* Add 'main' nets to netlist of father network */
	    /* and update home pointers of those nets */

	    for (n_p = sub_netw -> nhead; n_p; n_p = next) {
		next = n_p -> next_obj;
		if (!n_p -> equiv || n_p -> equiv == n_p) {
		    if (netw -> nhead == NULL)
			netw -> nhead = n_p;
		    else
			netw -> ntail -> next_obj = n_p;
		    netw -> ntail = n_p;
		    n_p -> next_obj = NULL;
		    n_p -> netw = netw;
		    n_p -> equiv = NULL;

		    /* rename net n_p */
		    Assert (n_p -> name);
		    sprintf (name, "%s.%s", Device -> name, n_p -> name);
		    rm_symbol (n_p -> name);
		    Assert (strlen (name) < 512);
		    n_p -> name = mk_symbol (name);
		}
		else {
		    Assert (!n_p -> head);
		    rm_object (n_p);
		}
	    }
	    sub_netw -> nhead = NULL;

	    /* Update pointers to 'home' network in devicelist */
	    /* and update device names (AJH) */

	    for (d_p = sub_netw -> dhead; d_p; d_p = d_p -> next_obj) {
		d_p -> netw = netw;
		sprintf (name, "%s.%s", Device -> name, d_p -> name);
		rm_symbol (d_p -> name);
		Assert (strlen (name) < 512);
		d_p -> name = mk_symbol (name);
	    }

	    Device = (d_p = Device) -> next_obj;

	    if (sub_netw -> dhead) {
		sub_netw -> dtail -> next_obj = Device;
		Device = sub_netw -> dhead;
		sub_netw -> dhead = NULL;
	    }

	    rm_network (sub_netw); /* remove device network */
	    rm_object (d_p);	   /* remove device instance */
	}
	else { /* primitive device */
	    if (!netw -> dhead) netw -> dhead = Device;
	    else    netw -> dtail -> next_obj = Device;
	    netw -> dtail = Device;
	    Device = Device -> next_obj;
	}
    }

    if (netw -> dtail) netw -> dtail -> next_obj = NULL;
}

/*
 * Propagates a connection between two or
 * more nets in a sub network to the father network.
 */
Private void propagate_conn (link_type *lnk)
{
    link_type *chn, *hd, *tl, *next;
    object *main_net, *lnkport, *lnk_net, *net, *t;
    object *netlist[64];
    int index, nr_of_equiv_nets = 0;

    if (!(lnk_net = lnk -> net)) return;

 /* merge all net chains of equivalent links into work chain */

    hd = lnk_net -> head;
    tl = lnk_net -> tail;
    Assert (hd && tl);

    lnkport = lnk -> port;
    Assert (lnkport -> equiv == lnkport);

    netlist[nr_of_equiv_nets++] = lnk_net;

    for (chn = lnk -> next_down; chn; chn = chn -> next_down) {
	net = chn -> net;
	if (chn -> port -> equiv == lnkport && net != lnk_net && net) {
	    for (index = 0; index < nr_of_equiv_nets; index++) {
		if (net == netlist[index]) break;
	    }
	    if (index < nr_of_equiv_nets) {
	        if (D_opt) fprintf (stderr, "propagate_conn: net %s already processed\n", net -> name);
		continue;
	    }
	    Assert (nr_of_equiv_nets < 64);
	    netlist[nr_of_equiv_nets++] = net;

	    /* add net chain to work chain */
	    Assert (net -> head);
	    tl -> next_up = net -> head;
	    tl = net -> tail;
	    Assert (tl && tl -> next_up == NULL);
	}
    }

    if (nr_of_equiv_nets > 1) { /* a merge occured */

	/* try to find best main_net */
	main_net = NULL;
	for (index = 0; index < nr_of_equiv_nets; index++) {
	    net = netlist[index];
	    if (Get_field (net, TYPE) == TERMINAL) {
		if (!main_net) main_net = net;
		else { /* take 1st terminal */
		    for (t = lnk -> dev -> netw -> thead; t; t = t -> next_obj) {
			if (t == main_net) break;
			if (t == net) { main_net = net; break; }
		    }
		}
	    }
	}
	if (!main_net) {
	    for (net = lnk -> dev -> netw -> nhead; net; net = net -> next_obj) {
		for (index = 0; index < nr_of_equiv_nets; index++)
		    if (net == netlist[index]) goto found;
	    }
found:
	    Assert (net);
	    main_net = net;
	}

	/* set equivalence pointers */
	for (index = 0; index < nr_of_equiv_nets; index++) {
	    net = netlist[index];
	    net -> head = NULL;
	    net -> tail = NULL;
	    net -> equiv = main_net;
	}

	/* set all hd links to the main_net */
	for (chn = hd; chn; chn = chn -> next_up) chn -> net = main_net;
	main_net -> head = hd;
	main_net -> tail = tl;
    }

    /* remove all equivalent link objects */
    for (lnk = lnk -> next_down; lnk; lnk = next) {
	next = lnk -> next_down;
	if (lnk -> port -> equiv == lnkport) del_link (lnk);
    }
}

/*
 * Recursively descends the (instantiated) hierarchical
 * network datastructure and eliminates connected nets.
 */
Public void reduce_netw (network *netw)
{
    object *dev;
    link_type *lnk;

    for (dev = netw -> dhead; dev; dev = dev -> next_obj) {

	reduce_netw (dev -> call); /* depth first */

	for (lnk = dev -> head; lnk; lnk = lnk -> next_down) {
	    if (lnk -> port -> equiv == lnk -> port) propagate_conn (lnk);
	}
    }
}
