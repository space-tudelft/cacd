static char *rcsid = "$Id: access.c,v 1.1 2018/04/30 12:17:22 simon Exp $";
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
 * This module contains the set of primitive functions to
 * build, manipulate and access the abstract datatype 'network'.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

Import long n_objt_alloc;
Import hash *nlist, *slist;
Import network *cur_network;

/*
 * This function creates a single object of the
 * specified type an with the specified name
 * and returns a pointer to it.
 * The structure is properly initialised.
 */

Private object * empty_objects = NULL;

Public object *mk_object (string name, int type)
{
    Private int ThiS_instance = 1;
    object * ThiS;

    Assert (type == NET || type == TERMINAL || type == DEVICE);

    if (empty_objects) {
	ThiS = empty_objects;
	empty_objects = ThiS -> next_obj;
    }
    else {
	Malloc (ThiS, 1, object);
    }

    ThiS -> instance = ThiS_instance++;
    ThiS -> color = 0;
    ThiS -> edges = 1;
    ThiS -> name = mk_symbol (name);
    ThiS -> netw = NULL;
    ThiS -> call = NULL;
    ThiS -> head = NULL;
    ThiS -> tail = NULL;
    ThiS -> next_obj = NULL;
    ThiS -> next_elm = NULL;
    ThiS -> equiv = NULL;
    ThiS -> block = NULL;
    ThiS -> par_list = NULL;
    ThiS -> flags = 0;
    Set_field (ThiS, TYPE, type);

    n_objt_alloc++;

    return (ThiS);
}

/*
 * The difference with mk_object() is that the
 * field "call" is filled with argument 'netw'.
 */
Public object *mk_device (string instname, network *netw)
{
    object *ThiS = mk_object (instname, DEVICE);

    ThiS -> call = netw;

    return (ThiS);
}

/*
 * This function removes an object.
 * If the object is of type DEVICE then the associated
 * link list is removed as well.
 */
Public void rm_object (object *ThiS)
{
    link_type *lnk, *next;

    Assert (ThiS);

    rm_symbol (ThiS -> name); /* free the name */

    if (Get_field (ThiS, TYPE) == DEVICE) {
	for (lnk = ThiS -> head; lnk; lnk = next) { /* free link list */
	    next = lnk -> next_down;
	    del_link (lnk);
	}
    }

    n_objt_alloc--;

    ThiS -> next_obj = empty_objects;
    empty_objects = ThiS;
}

/*
 * Adds the specified object(s) to the network datastructure.
 * The new object(s) is(are) appended at the end of the
 * appropriate (type) object list of the network.
 */
Public void add_object (network *netw, object *ThiS)
{
    object **head, **tail, *that, *next;

    head = tail = NULL; /* suppres uninitialized warning */

    Assert (netw && ThiS);

    for (that = ThiS; that; that = next) {

	next = that -> next_obj;

    /* set 'Home' pointer */
	that -> netw = netw;

    /* add object to end of appropriate list */
	switch (Get_field (that, TYPE)) {

	    case TERMINAL:
		head = &netw -> thead;
		tail = &netw -> ttail;
		if (nlist && netw == cur_network) h_link (nlist, that -> name, that);
		netw -> n_terms++;
		break;

	    case NET:
		head = &netw -> nhead;
		tail = &netw -> ntail;
		if (nlist && netw == cur_network) h_link (nlist, that -> name, that);
		break;

	    case DEVICE:
		head = &netw -> dhead;
		tail = &netw -> dtail;
		break;
	}

	if (*tail == NULL) *head = that;
	else (*tail) -> next_obj = that;
	that -> next_obj = NULL;
	*tail = that;
    }
}

/*
 * This function checks if a terminal with name 'name'
 * exists for the given network.
 * A pointer to the terminal structure is returned.
 * When no such terminal is present, NULL is returned.
 */
Public object *is_term (network *netw, string name)
{
    object *term;

    Assert (netw && name);

    for (term = netw -> thead; term; term = term -> next_obj) {
	if (strcmp (term -> name, name) == 0) return (term);
    }
    return (NULL);
}

/*
 * This function is only used in readNet and replaces
 * the separate calls to functions is_term and is_net.
 */
Public object *is_tnet (network *netw, string name)
{
    object *net;

    Assert (name);

    for (net = netw -> thead; net; net = net -> next_obj) {
	if (strcmp (net -> name, name) == 0) return (net);
    }
    for (net = netw -> nhead; net; net = net -> next_obj) {
	if (strcmp (net -> name, name) == 0) return (net);
    }
    return (NULL);
}

/*
 * This function checks if a net with name 'name'
 * exists for the given network.
 * A pointer to the net structure is returned.
 * When no such net is present, NULL is returned.
 */
Public object *is_net (network *netw, string name)
{
    object *net;

    if (nlist && netw == cur_network) return h_get (nlist, name);

    for (net = netw -> nhead; net; net = net -> next_obj) {
	if (strcmp (net -> name, name) == 0) return (net);
    }
    return (NULL);
}

/*
 * This function checks if a subterm with name 'name'
 * exists for the given network.
 * A pointer to the net structure is returned.
 * When no such net is present, NULL is returned.
 */
Public object *is_subterm (string name)
{
    return h_unlink (slist, name, 1);
}

/*
 * Basic constructor function.
 * The function 'array' multiplicates the specified
 * objects a predefined number of times.
 * Each instance is given a unique instance name.
 * Single objects created by 'mk_object' as well as
 * array's may be 'array-ed'.
 * An array is implemented as a single linked list.
 */
Public void array (object *thing, long first, long last)
{
    object *head, *tail, *ThiS, *that;
    string  name;
    long    index;

    Assert (thing);
    Assert (first >= 0 && last >= 0);

    head = tail = NULL;

 /* create additional instances */
    for (index = first; ;) {
	if (last > first) {
	    if (++index > last) break;
	} else {
	    if (--index < last) break;
	}
	for (ThiS = thing; ThiS; ThiS = ThiS -> next_obj) {

	    that = mk_object (NULL, Get_field (ThiS, TYPE));
	    that -> call = ThiS -> call;
	    that -> par_list = ThiS -> par_list;
	    that -> name = add_index (ThiS -> name, index);

	    if (head == NULL) head = that;
	    else  tail -> next_obj = that;
	    tail = that;
	}
    }

 /* rename first, existing instance(s) */
    for (ThiS = thing; ; ThiS = ThiS -> next_obj) {
	name = ThiS -> name;
	ThiS -> name = add_index (name, first);
	rm_symbol (name);
	if (!ThiS -> next_obj) break;
    }

    /* merge the lot into one list */
    ThiS -> next_obj = head;
}

/*
 * Connects a device to a set of nets.
 * A single device as well as arrays of devices may be specified.
 * A list of nets (the connect list) must be specified
 * as well as the connect mode (INST_MAJOR or PARM_MAJOR).
 */
Public void netw_connect (object *dev_list, list *conn_list, int cnt, int conn_mode)
{
    list   *lp;
    object *net, *dev;
    int i;

    Assert (dev_list && conn_list && cnt > 0);

    if (conn_mode == INST_MAJOR) {
	lp = conn_list;
	for (dev = dev_list; dev; dev = dev -> next_obj) {
	    for (i = 0; i < cnt; i++) {
		if (lp) {
		    net = (object *) lp -> car;
		    lp = lp -> cdr;
		}
		else net = NULL;
		add_link (net, dev);
	    }
	}
    }
    else {
	Assert (conn_mode == PARM_MAJOR);
	lp = conn_list;
	for (i = 0; i < cnt; i++) {
	    for (dev = dev_list; dev; dev = dev -> next_obj) {
		if (lp) {
		    net = (object *) lp -> car;
		    lp = lp -> cdr;
		}
		else net = NULL;
		add_link (net, dev);
	    }
	}
    }

    while ((lp = conn_list)) { conn_list = lp -> cdr; rm_list (lp); }
}

/*
 * Connects a device to a set of nets of the network.
 * A single device as well as arrays of devices may be specified.
 * Also the device terminal list needs to be used.
 * For each device terminal a link must be made.
 */
Public void do_connect (object *dev_list)
{
    char netname[DM_MAXNAME+DM_MAXNAME+2];
    object *dev, *term, *net;

    for (dev = dev_list; dev; dev = dev -> next_obj) {
        for (term = dev -> call -> thead; term; term = term -> next_obj) {
	    sprintf (netname, "%s.%s", dev -> name, term -> name);
            net = is_subterm (netname);
	    add_link (net, dev);
        }
    }
}

/*
 * Merges the specified nets from the net_list.
 * The net_list must contain at least TWO nets.
 * Finally, the net_list is removed.
 */
Public void merge (network *netw, list *net_list)
{
    object *n, *t, *main_obj, *net;
    list   *lp;
    link_type *lnk, *hd, *tl;
    static object **st;
    static int st_size;
    int i, sp = 0;

    Assert (netw && net_list);

    if (!net_list -> cdr) goto ret; /* only one net */

    if (st_size == 0) {
	st_size = 64;
	st = (object **) malloc (st_size * sizeof (object *));
	if (!st) Fatal (6);
    }

    /* gather all terminals and nets on the stack */

    for (t = netw -> thead; t; t = t -> next_obj) {
	for (lp = net_list; lp; lp = lp -> cdr) {
	    net = (object *) lp -> car;
	    if (net == t || net == t -> equiv || (net -> equiv && (net -> equiv == t || net -> equiv == t -> equiv))) {
		if (sp >= st_size) {
		    st_size *= 2;
		    st = (object **) realloc ((void *) st, st_size * sizeof (object *));
		    if (!st) Fatal (6);
		}
		st[sp++] = t;
		break;
	    }
	}
    }
    for (n = netw -> nhead; n; n = n -> next_obj) {
	for (lp = net_list; lp; lp = lp -> cdr) {
	    net = (object *) lp -> car;
	    if (net == n || net == n -> equiv || (net -> equiv && (net -> equiv == n || net -> equiv == n -> equiv))) {
		if (sp >= st_size) {
		    st_size *= 2;
		    st = (object **) realloc ((void *) st, st_size * sizeof (object *));
		    if (!st) Fatal (6);
		}
		st[sp++] = n;
		break;
	    }
	}
    }
    Assert (sp >= 2);

    main_obj = st[0]; /* new main object */

    /* next, merge all net chains */
    hd = tl = NULL;
    for (i = 0; i < sp; i++) {
	n = st[i];
	if (!n -> equiv || n -> equiv == n) {
	    if (n -> head) {
		if (hd == NULL) hd = n -> head;
		else tl -> next_up = n -> head;
		tl = n -> tail;
		Assert (tl && tl -> next_up == NULL);
	    }
	}
	else Assert (n -> head == NULL);
    }

    /* update home net pointers */
    for (lnk = hd; lnk; lnk = lnk -> next_up) lnk -> net = main_obj;

    /* install merged link object list and set equivalent pointers */
    for (i = 0; i < sp; i++) {
	st[i] -> head = hd;
	st[i] -> tail = tl;
	st[i] -> equiv = main_obj;
	hd = tl = NULL; /** NEW **/
    }
ret:
    /* finally, remove input list */
    while ((lp = net_list)) { net_list = lp -> cdr; rm_list (lp); }
}

/*
 * Returns a pointer to the name of the port
 * on the specified position of 'device'.
 */
Public string port_name (object *device, long pos)
{
    object *term;
    long    cnt;

    Assert (device && device -> call);

    cnt = 0;
    for (term = device -> call -> thead; term; term = term -> next_obj) {
	if (++cnt == pos) return (term -> name);
    }
    return (NULL);
}
