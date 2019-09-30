static char *rcsid = "$Id: map.c,v 1.1 2018/04/30 12:17:37 simon Exp $";
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
 * Contains basic mapping function with some
 * specialised service routines.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

Import network * nom_netw;
Import network * act_netw;
Import boolean d_opt;
Import boolean map_opt;
Import boolean N_opt;

/*
 * Matches the two specified networks.
 * Either True or False is returned.
 */
Public boolean match (network *netw1, network *netw2)
{
    partition * p;
    stack * st;
    long     old_n_blocks;
    long     old_n_bound;

    st = mk_a_stack ();
    p = init_partition (netw1, netw2);
    if (N_opt) {
	match_by_name (p);
    }
    validate_list (&(p -> active));

    refine (p, st, NOSAVE);

    if (p -> invalid == NULL && p -> n_blcks != p -> n_bound) {
	map (p, SAVE); /* Try to find an isomorphism via backtracking */
    }

    if (p -> invalid == NULL && p -> n_blcks == p -> n_bound) {
	verb_mesg ("Refine succeeds: Mapping found !\n");
	return (True);
    }
    else {
       /*
	* No isomorphism exists, try to come as close to one as is
	* humanly possible.
	*/
	do {
	    old_n_bound = p -> n_bound;
	    old_n_blocks = p -> n_blcks;

	    reconfigure (p);
	    if (N_opt) {
		match_by_name (p);
	    }
	    refine (p, st, NOSAVE);


	} while ((old_n_bound < p -> n_bound) ||
		(old_n_bound == p -> n_bound && old_n_blocks < p -> n_blcks) ||
		map (p, NOSAVE) == True);

	if (d_opt) {
	    refine (p, st, NOSAVE);
	    reconfigure (p);
	    if (N_opt) {
		match_by_name (p);
	    }
	    refine (p, st, NOSAVE);
	}
	return False;
    }
}

/*
 * Tries to map the elements of a partition.
 * Calls itself recursively (via refine () and only if mode == SAVE)
 * until all possible mappings are investigated
 * or a correct mapping is found.
 * If mode equals SAVE, the refinement history is administrated.
 */
Public boolean map (partition *p, short mode)
{
    object * e1, *e2;
    block *b, *ThiS, *ref;
    bucket st;
    bucket to_be_mapped;

 /* select an unbound valid block */
    if (mode == NOSAVE && map_opt == False) {
	return False;
    }

    /*
    verb_mesg ("doing a map with invalid blocks\n");
    */

    ThiS = p -> passive;
    ref = ThiS;
    do {
	if (ThiS == NULL) {

	    return (False);
	}

	if (!Get_flag (ThiS, BOUNDED))
	    break;

	ThiS = ThiS -> next;

    } while (ThiS != ref);

    if (Get_flag (ThiS, BOUNDED)) {
	return (False);
    }

    Assert (Get_field (ThiS, STATE) == PASSIVE);
    Assert (!Get_flag (ThiS, BOUNDED));
#ifdef VOGEL
    Assert (ThiS -> n > 2);
#endif

 /* Assign two elements to a new block.
  * In case the network does not match, ThiS probably results in a block
  * containing two devices that should not actually match */

    for (e1 = ThiS -> head; e1 != NULL; e1 = e1 -> next_elm) {
	if (e1 -> netw == nom_netw) break;
    }
    if (e1 == NULL) return False;

    st.next = NULL;
    to_be_mapped.next = NULL;

    for (e2 = ThiS -> head; e2 != NULL; e2 = e2 -> next_elm) {
	if (e2 -> netw == act_netw && e1 -> flavor == e2 -> flavor) {
	    push ((stack *)&to_be_mapped, e2);
	}
    }

    while ((e2 = (object *) pop ((stack *)& to_be_mapped)) != NULL) {

	verb_mesg ("Selected %s (Nom) with ", e1 -> name);
	verb_mesg ("%s (Act)\n", e2 -> name);

	e1 -> color = 1;
	e2 -> color = 1;

	split (ThiS, 0);
	if (refine (p, (stack * ) &st, mode) == False) {
	    if (mode == SAVE) {
		verb_mesg ("Backtracking from %s (Nom) and ", e1 -> name);
		verb_mesg ("%s (Act)\n", e2 -> name);
		while ((b = (block *) pop ((stack *)& st)) != NULL) {
		    merge_block (b);
		}
		merge_block (ThiS);
	    }
	    else {
		while ((object *) pop ((stack *)& to_be_mapped) != NULL);
		return False;
	    }
	}
	else {
	    while ((object *) pop ((stack *)& to_be_mapped) != NULL);
	    return (True);
	}
    }
    return False;
}
