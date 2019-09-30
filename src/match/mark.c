static char *rcsid = "$Id: mark.c,v 1.1 2018/04/30 12:17:38 simon Exp $";
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
 * Contains a set of functions to mark individual
 * nodes of the hierarchical network graph.
 * Marked nodes cannot be expanded by the function 'expand'.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

/*
 * Searches the specified hierarchical network
 * data structure in a breadth-first fashion for
 * subnetworks (nodes) with the specified color and marks them.
 * The root node is ignored.
 * Nodes (trees) which are already marked are ignored.
 * When one or more nodes could be marked, True is returned.
 */
Private boolean search_bfs (network *netw, long color)
{
    queue * ThiS_queue;
    network * ThiS_netw;
    object * ThiS;
    boolean marked;

    ThiS_queue = mk_a_queue ();	/* create a queue */
    marked = False;

    if (netw == NULL) return (marked);

    ThiS = netw -> dhead;
    while (ThiS != NULL) {
	add_queue (ThiS_queue, ThiS -> call, TAIL);
	ThiS = ThiS -> next_obj;
    }

    while ((ThiS_netw = (network *) del_queue (ThiS_queue, HEAD)) != NULL) {

	if (Get_flag (ThiS_netw, MARKED)) continue;

	if (ThiS_netw -> color == (unsigned long) color) {
	    marked = True;
	    Set_flag (ThiS_netw, MARKED);
	}

	ThiS = ThiS_netw -> dhead;
	while (ThiS != NULL) {
	    add_queue (ThiS_queue, ThiS -> call, TAIL);
	    ThiS = ThiS -> next_obj;
	}
    }

    return (marked);
}

/*
 * Marks the (sub) networks of the two specified networks
 * based on their (corresponding) group colors.
 */
Public void mark (network *netw1, network *netw2)
{
    queue * ThiS_queue;
    network * ThiS_netw;
    object * ThiS;

    ThiS_queue = mk_a_queue ();	/* create a queue */

    if (netw1 == NULL) return;

    ThiS = netw1 -> dhead;
    while (ThiS != NULL) {
	add_queue (ThiS_queue, ThiS -> call, TAIL);
	ThiS = ThiS -> next_obj;
    }

    while ((ThiS_netw = (network *) del_queue (ThiS_queue, HEAD)) != NULL) {

	if (Get_flag (ThiS_netw, MARKED)) continue;

	if (search_bfs (netw2, ThiS_netw -> color) == True) {
	    (void) search_bfs (netw1, ThiS_netw -> color);
	    continue;
	}

	ThiS = ThiS_netw -> dhead;
	while (ThiS != NULL) {
	    add_queue (ThiS_queue, ThiS -> call, TAIL);
	    ThiS = ThiS -> next_obj;
	}
    }
}
