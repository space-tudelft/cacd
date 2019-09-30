static char *rcsid = "$Id: link.c,v 1.1 2018/04/30 12:17:34 simon Exp $";
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
 * Contains a number of low level access functions
 * to update and query the link_type structures between the
 * terminals/nets and devices of a network graph.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

/*	VARIABLES:
 */
Import long n_link_alloc;

/*
 * Allocates storage for a link object which represent
 * a link between a net and a device.
 * A pointer to the link object is returned.
 * The link object is properly initialized.
 */

static link_type * empty_links = NULL;

Public link_type * mk_link ()
{
    link_type * new_link;

    if  (empty_links) {
	new_link = empty_links;
	empty_links = new_link -> next_up;
    }
    else {
	Malloc (new_link, 1, link_type);
    }
    Assert (new_link);

    new_link -> next_up = NULL;
    new_link -> next_down = NULL;
    new_link -> net = NULL;
    new_link -> dev = NULL;
    new_link -> port = NULL;
    new_link -> color = 0;

    n_link_alloc++;

    return (new_link);
}

/*
 * Releases the allocated space for a link object.
 */
Public void rm_link (link_type *ThiS_link)
{
    Assert (ThiS_link);

    ThiS_link -> next_up = empty_links;
    empty_links = ThiS_link;

    n_link_alloc--;
}

/*
 * Adds to the set of links in a network graph
 * a link between a net (terminal) and a device.
 */
Public void add_link (object *net, object *dev)
{
    link_type * ThiS_link;

    Assert (dev && Get_field (dev, TYPE) == DEVICE);

    ThiS_link = mk_link ();	/* Create a link object	 */

if (net) {
    Assert (Get_field (net, TYPE) == NET || Get_field (net, TYPE) == TERMINAL);

    if (net -> equiv) {
	net = net -> equiv;
	Assert (net -> equiv == net);
    }

    /* Add ThiS_link to tail of net chain */
    if (net -> tail == NULL)
	net -> head = ThiS_link;
    else
	net -> tail -> next_up = ThiS_link;
    net -> tail = ThiS_link;
}

    /* Add ThiS_link to tail of device chain */
    if (dev -> tail == NULL)
	dev -> head = ThiS_link;
    else
	dev -> tail -> next_down = ThiS_link;
    dev -> tail = ThiS_link;

    ThiS_link -> dev = dev;	/* Set 'Home' pointers */
    ThiS_link -> net = net;
}

/*
 * Deletes the specified link object from the set of
 * links of a network graph.
 * The object is unlinked from both the neigbour chain
 * of a net and the neighbour chain of a device.
 */
Public void del_link (link_type *lnk)
{
    object *home_dev, *home_net;
    link_type *ThiS, *prev;

 /* Determine home pointers */
    home_dev = lnk -> dev;
    home_net = lnk -> net;

    Assert (home_dev);
    Assert (home_dev -> tail -> next_down == NULL);
    Assert (Get_field (home_dev, TYPE) == DEVICE);

    /* search lnk and previous link object in device chain */
    prev = NULL;
    for (ThiS = home_dev -> head; ThiS && ThiS != lnk; ThiS = ThiS -> next_down) prev = ThiS;
    if (ThiS) {
	if (prev == NULL)
	    home_dev -> head = ThiS -> next_down;
	else
	    prev -> next_down = ThiS -> next_down;

	if (home_dev -> tail == ThiS) {
	    home_dev -> tail = prev;
	    if (prev) prev -> next_down = NULL;
	}
    }
    else Assert (ThiS); /* Must be found */

if (home_net) {
    Assert (home_net -> tail);
    Assert (home_net -> tail -> next_up == NULL);
    Assert (Get_field (home_net, TYPE) == NET || Get_field (home_net, TYPE) == TERMINAL);

    /* search lnk and previous link object in net chain */
    prev = NULL;
    for (ThiS = home_net -> head; ThiS && ThiS != lnk; ThiS = ThiS -> next_up) prev = ThiS;
    if (ThiS) {
	if (prev == NULL)
	    home_net -> head = ThiS -> next_up;
	else
	    prev -> next_up = ThiS -> next_up;

	if (home_net -> tail == ThiS) {
	    home_net -> tail = prev;
	    if (prev) prev -> next_up = NULL;
	}
    }
}

    rm_link (lnk);
}

/*
 * This function unlinks the specified link object
 * from the neighbour chain of some net and links it
 * into the neighbour chain of the specified net.
 */
Public void chn_link (link_type *lnk, object *net)
{
    link_type * prev;
    link_type * ThiS;

    Assert (lnk);
    Assert (net);
    Assert (Get_field (net, TYPE) == NET || Get_field (net, TYPE) == TERMINAL);

 /* unlink object from old neighbour chain */

    prev = NULL;
    ThiS = lnk -> net -> head;
    while (ThiS != NULL) {
	if (ThiS == lnk) {
	    if (prev == NULL)
		lnk -> net -> head = lnk -> next_up;
	    else
		prev -> next_up = lnk -> next_up;

	    if (lnk -> net -> tail == lnk)
		lnk -> net -> tail = prev;
	    break;
	}
	prev = ThiS;
	ThiS = ThiS -> next_up;
    }

    Assert (ThiS);	/* Must be found */

 /* link object in neighbour chain of new net */

    if (net -> head == NULL) {
	net -> head = lnk;
	net -> tail = lnk;
    }
    else {
	net -> tail -> next_up = lnk;
	net -> tail = lnk;
    }

    lnk -> next_up = NULL;
    lnk -> net = net;

    Assert (net -> tail -> next_up == NULL);
}

/*
 * Checks whether there exists a link between the
 * specified net and device.
 * Either True or False is returned.
 */
Public boolean is_link (object *net, object *dev)
{
    link_type * ThiS;

    Assert (net);
    Assert (dev);
    Assert (Get_field (dev, TYPE) == DEVICE);
    Assert (Get_field (net, TYPE) == NET || Get_field (net, TYPE) == TERMINAL);

    ThiS = net -> head;
    while (ThiS != NULL) {
	if (ThiS -> dev == dev) return (True);
	ThiS = ThiS -> next_up;
    }
    return (False);
}
