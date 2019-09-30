static char *rcsid = "$Id: color.c,v 1.1 2018/04/30 12:17:26 simon Exp $";
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
 * Contains a number of functions to handle color issues
 * with respect to the network partitioning.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

/*
 * Set the terminal colors of a primitive network
 * to prime numbers corresponding with their p_expr.
 * If no p_expr is specified, all terminals obtain unique numbers.
 */
Public void set_termcol (network *netw, list *p_expr)
{
    list *class, *lp;
    object *term, *t;

    rewind_gen ();

    for (term = netw -> thead; term; term = term -> next_obj) {
	if (term -> color == 0) {
	    term -> color = gen_prime ();
	    for (class = p_expr; class; class = class -> cdr) {
		for (lp = class -> car; lp; lp = lp -> cdr) {
		    if (term == (object *) lp -> car) break;
		}
		if (lp) break; /* term found */
	    }
	    if (class) {
		for (lp = class -> car; lp; lp = lp -> cdr) {
		    if ((t = (object *) lp -> car)) t -> color = term -> color;
		}
	    }
	}
    }
}

/*
 * Gives the specified network a group color.
 * Recursively descends the (acyclic) graph
 * corresponding with the hierarchy of the network.
 * Used as a heuristic function when matching two hierarchical networks.
 */
Public void color_grp (network *netw)
{
    object *dev, *net;
    link_type *lnk;

    netw -> color = 0;

    if (Get_field (netw, TYPE) == PRIMITIVE) {
	set_termcol (netw, NULL);
	for (net = netw -> thead; net; net = net -> next_obj) {
	    netw -> color += net -> color;
	}
    }
    else {
	/* initially color the devices, nets and terminals */

	for (dev = netw -> dhead; dev; dev = dev -> next_obj) {

	    /* depth first */
	    if (dev -> call -> color == 0) color_grp (dev -> call);

	    for (lnk = dev -> head; lnk; lnk = lnk -> next_down) {
		if (lnk -> port) lnk -> color = lnk -> port -> color;
	    }

	    dev -> color = dev -> call -> color;

	    netw -> color += dev -> color;
	}

	/* initialize net/terminal colors */

	for (net = netw -> thead; net; net = net -> next_obj) {
	    net -> color = 2;
	}
	for (net = netw -> nhead; net; net = net -> next_obj) {
	    net -> color = 2;
	}
    }
}
