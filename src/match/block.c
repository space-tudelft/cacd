static char *rcsid = "$Id: block.c,v 1.1 2018/04/30 12:17:24 simon Exp $";
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
 * This file contains a collection of functions which
 * perform elementary operations on a partition.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

/*	FUNCTIONS:
 */
Private boolean check_name (char *name);

/*	VARIABLES:
 */
Import long  n_part_alloc;
Import long  n_blck_alloc;
Import long  n_arr_alloc;
Import long  n_arr_mem;
Import boolean D_opt;
Import boolean T_opt;
Import boolean d_opt;
Import boolean i_opt;
Import boolean n_or_d;

Import range * range_checks;

Import network * nom_netw;
Import network * act_netw;

Private long network_size = 0;
Private double next_size_report = 0.0;

Import double progress_size;

/*
 * Allocates storage for a structure of type partition.
 * All members of the structure are initialised to their default values.
 * A pointer to the structure is returned.
 */
Public partition *mk_partition ()
{
    partition * ThiS_p;

    Malloc (ThiS_p, 1, partition);

    ThiS_p -> n_blcks = 0;
    ThiS_p -> n_elemts = 0;
    ThiS_p -> n_active = 0;
    ThiS_p -> n_passive = 0;
    ThiS_p -> n_touched = 0;
    ThiS_p -> n_invalid = 0;
    ThiS_p -> n_bound = 0;
    ThiS_p -> n_iter = 0;
    ThiS_p -> active = NULL;
    ThiS_p -> passive = NULL;
    ThiS_p -> invalid = NULL;
    ThiS_p -> touched = NULL;
    ThiS_p -> hist = NULL;

    n_part_alloc++;

    return (ThiS_p);
}

/*
 * Removes a structure of type partition.
 */
Public void rm_partition (partition *p)
{
    Free (p);
    n_part_alloc--;
}

/*
 * Allocates storage for a structure of type block.
 * A pointer to the structure is returned.
 */

Private block * empty_blocks = NULL;

Private block *mk_block ()
{
    block *ThiS;

    if (empty_blocks) {
        ThiS = empty_blocks;
        empty_blocks = ThiS -> next;
    }
    else {
        Malloc (ThiS, 1, block);
    }

    ThiS -> n_el = 0;
    ThiS -> head = NULL;
    ThiS -> part = NULL;
    ThiS -> next = NULL;
    ThiS -> prev = NULL;
    ThiS -> t_nxt = NULL;
    ThiS -> childs = NULL;
    ThiS -> n_childs = 0;
    ThiS -> flags = 0;
    ThiS -> parent = 0;
    ThiS -> level = 0;

    n_blck_alloc++;

    return (ThiS);
}

/*
 * Deletes a structure of type block.
 */
Public void rm_block (block *ThiS_block)
{
    ThiS_block -> next = empty_blocks;
    empty_blocks = ThiS_block;

    n_blck_alloc--;
}

/*
 * Links a block into a blocklist of the appropriate partition.
 * The block is inserted at the beginning of the list or appended
 * at the end of the list, depending on the value of mode (HEAD or TAIL).
 * The type of the block is taken into account.
 * All the necessary administration is done.
 */
Public void add_block (block *ThiS_block, short mode)
{
    partition * p = ThiS_block -> part;

    if (ThiS_block -> n_el == 0) {
	rm_block (ThiS_block);
	return;
    }

    switch (Get_field (ThiS_block, STATE)) {

	case ACTIVE:
	    if (p -> active == NULL) {
		p -> active = ThiS_block;
		ThiS_block -> next = ThiS_block;
		ThiS_block -> prev = ThiS_block;
	    }
	    else {
		ThiS_block -> next = p -> active;
		ThiS_block -> prev = p -> active -> prev;
		p -> active -> prev -> next = ThiS_block;
		p -> active -> prev = ThiS_block;
	    }
	    p -> n_active++;

	    /*
	     * If you want to find incorrect nets rather than incorrect
	     * devices and terms, let the first elements of the
	     * list of active blocks be devices and terms and the last be
	     * nets (AJH).
	     */

	    /*
	    if (Get_field (ThiS_block -> head, TYPE) != NET &&
		n_or_d == False) {
		static long firstDev = 1;
		if (firstDev) {
		    verb_mesg ("focusing on faulty nets\n");
		    firstDev = 0;
		}
		p -> active = p -> active -> prev;
	    }

	    else if (Get_field (ThiS_block -> head, TYPE) == NET &&
		n_or_d == True) {
		static long firstNet = 1;
		if (firstNet) {
		    verb_mesg ("focusing on faulty devices\n");
		    firstNet = 0;
		}
		p -> active = p -> active -> prev;
	    }
	    */

	    if (mode == HEAD)
		p -> active = p -> active -> prev;
	    break;

	case PASSIVE:
	    if (p -> passive == NULL) {
		p -> passive = ThiS_block;
		ThiS_block -> next = ThiS_block;
		ThiS_block -> prev = ThiS_block;
	    }
	    else {
		ThiS_block -> next = p -> passive;
		ThiS_block -> prev = p -> passive -> prev;
		p -> passive -> prev -> next = ThiS_block;
		p -> passive -> prev = ThiS_block;
	    }
	    p -> n_passive++;
	    if (mode == HEAD)
		p -> passive = p -> passive -> prev;
	    break;

	case INVALID:
	    if (p -> invalid == NULL) {
		p -> invalid = ThiS_block;
		ThiS_block -> next = ThiS_block;
		ThiS_block -> prev = ThiS_block;
	    }
	    else {
		ThiS_block -> next = p -> invalid;
		ThiS_block -> prev = p -> invalid -> prev;
		p -> invalid -> prev -> next = ThiS_block;
		p -> invalid -> prev = ThiS_block;
	    }
	    p -> n_invalid++;
	    if (mode == HEAD)
		p -> invalid = p -> invalid -> prev;
	    break;
    }

    if (Get_flag (ThiS_block, BOUNDED)) p -> n_bound++;

    p -> n_blcks++;
    p -> n_elemts += ThiS_block -> n_el;
}

/*
 * Unlinks a block from a blocklist of the specified partition.
 * The type of the block is taken into account.
 * All the necessary administration is done.
 */
Public void del_block (block *ThiS_block)
{
    partition * p = ThiS_block -> part;

    ThiS_block -> next -> prev = ThiS_block -> prev;
    ThiS_block -> prev -> next = ThiS_block -> next;

    switch (Get_field (ThiS_block, STATE)) {

	case ACTIVE:
	    if (p -> active == ThiS_block)
		p -> active = p -> active -> next;
	    if (p -> active == ThiS_block)
		p -> active = NULL;
	    p -> n_active--;
	    break;

	case PASSIVE:
	    if (p -> passive == ThiS_block)
		p -> passive = p -> passive -> next;
	    if (p -> passive == ThiS_block)
		p -> passive = NULL;
	    p -> n_passive--;
	    break;

	case INVALID:
	    if (p -> invalid == ThiS_block)
		p -> invalid = p -> invalid -> next;
	    if (p -> invalid == ThiS_block)
		p -> invalid = NULL;
	    p -> n_invalid--;
	    break;
    }

    if (Get_flag (ThiS_block, BOUNDED)) p -> n_bound--;

    p -> n_blcks--;
    p -> n_elemts -= ThiS_block -> n_el;
    ThiS_block -> next = NULL;
    ThiS_block -> prev = NULL;
}

/*
 * Links an object into the element list of the block b.
 * All the necessary administration is done.
 */
Public void add_elem (block *b, object *ThiS)
{
    ThiS -> next_elm = b -> head;
    b -> head = ThiS;
    ThiS -> block = b;
    b -> n_el++;
}

/*
 * Creates an initial partition on the network graph
 * consisting of the two components netw1 and netw2.
 * The initial partition consists of several blocks.
 * This partition can then be refined using
 * the neighbour relations with the function refine().
 * A pointer to the partition is returned.
 */
Public partition *init_partition (network *netw1, network *netw2)
{
    partition * p;
    block *b;
 /* block **b_a; */
    block *r;
    object * ThiS;

    p = mk_partition ();

    r = mk_block ();		/* root block of history */
    r -> n_childs = 0;
    Malloc (r -> childs, 3, block *);
    n_arr_alloc++;
    n_arr_mem += 3 * sizeof (block *);
    r -> part = p;
    p -> hist = r;

    b = mk_block ();
    for (ThiS = netw1 -> thead; ThiS != NULL; ThiS = ThiS -> next_obj)
	if (ThiS -> equiv == NULL || ThiS -> equiv == ThiS) {
	    ThiS -> flavor = 0;
	    if (ThiS -> head == NULL) {
		user_mesg ("Unconnected (Nom) terminal: %s\n", ThiS -> name);
	    }
	    else {
		add_elem (b, ThiS);
		network_size++;
	    }
	}
    for (ThiS = netw2 -> thead; ThiS != NULL; ThiS = ThiS -> next_obj)
	if (ThiS -> equiv == NULL || ThiS -> equiv == ThiS) {
	    ThiS -> flavor = 0;
	    if (ThiS -> head == NULL) {
		user_mesg ("Unconnected (Act) terminal: %s\n", ThiS -> name);
	    }
	    else {
		add_elem (b, ThiS);
		network_size++;
	    }
	}

    Set_field (b, STATE, ACTIVE);
    b -> part = p;

    if (b -> head) {
	/*
	validate (b);
	*/
	add_block (b, HEAD);
	*(r -> childs + (r -> n_childs)) = b;
	r -> n_childs++;
    }

    b = mk_block ();

    for (ThiS = netw1 -> nhead; ThiS != NULL; ThiS = ThiS -> next_obj)
	if (ThiS -> equiv == NULL || ThiS -> equiv == ThiS) {
	    ThiS -> flavor = 1;
	    if (ThiS -> head == NULL) {
		user_mesg ("Unconnected (Nom) net: %s\n", ThiS -> name);
	    }
	    else {
		add_elem (b, ThiS);
		network_size++;
	    }
	}
    for (ThiS = netw2 -> nhead; ThiS != NULL; ThiS = ThiS -> next_obj)
	if (ThiS -> equiv == NULL || ThiS -> equiv == ThiS) {
	    ThiS -> flavor = 1;
	    if (ThiS -> head == NULL) {
		user_mesg ("Unconnected (Act) net: %s\n", ThiS -> name);
	    }
	    else {
		add_elem (b, ThiS);
		network_size++;
	    }
	}
    Set_field (b, STATE, ACTIVE);
    b -> part = p;

    if (b -> head) {
	/*
	validate (b);
	*/
	add_block (b, HEAD);
	*(r -> childs + (r -> n_childs)) = b;
	r -> n_childs++;
    }

    b = mk_block ();

    for (ThiS = netw1 -> dhead; ThiS != NULL; ThiS = ThiS -> next_obj) {
	if (Get_field (ThiS -> call, TYPE) == PRIMITIVE) {
	    ThiS -> flavor = 3 + find_prim_index (ThiS -> call -> name);
	    /*
	    add_elem (b_a[find_prim_index (ThiS -> call -> name)], ThiS);
	    */
	}
	else {
	    ThiS -> flavor = 2;
	    /*
	    add_elem (b, ThiS);
	    */
	}
	add_elem (b, ThiS);
	network_size++;
    }
    for (ThiS = netw2 -> dhead; ThiS != NULL; ThiS = ThiS -> next_obj) {
	if (Get_field (ThiS -> call, TYPE) == PRIMITIVE) {
	    ThiS -> flavor = 3 + find_prim_index (ThiS -> call -> name);
	    /*
	    add_elem (b_a[find_prim_index (ThiS -> call -> name)], ThiS);
	    */
	}
	else {
	    ThiS -> flavor = 2;
	    /*
	    add_elem (b, ThiS);
	    */
	}
	add_elem (b, ThiS);
	network_size++;
    }
    Set_field (b, STATE, ACTIVE);
    b -> part = p;

    if (b -> head) {
	/*
	validate (b);
	*/
	add_block (b, HEAD);
	*(r -> childs + (r -> n_childs)) = b;
	r -> n_childs++;
    }

    netw1 -> part = p;
    netw2 -> part = p;

    network_size /= 2;

    verb_mesg ("Initial partitioning made.\n");

    return (p);
}

/*
 * Deletes the partition datastructure of the specified network.
 */
Public void del_partition (network *netw)
{
    block *b, *n_b;
    object *e, *n_e;
    partition *p;

    Assert (netw);

    if (!(p = netw -> part)) return;

    for (b = p -> active; b != NULL; b = n_b) {
	n_b = b -> next;
	for (e = b -> head; e != NULL; e = n_e) {
	    n_e = e -> next_elm;
	    e -> next_elm = NULL;
	    e -> block = NULL;
	}
	rm_block (b);
    }
    for (b = p -> passive; b != NULL; b = n_b) {
	n_b = b -> next;
	for (e = b -> head; e != NULL; e = n_e) {
	    n_e = e -> next_elm;
	    e -> next_elm = NULL;
	    e -> block = NULL;
	}
	rm_block (b);
    }
    for (b = p -> invalid; b != NULL; b = n_b) {
	n_b = b -> next;
	for (e = b -> head; e != NULL; e = n_e) {
	    n_e = e -> next_elm;
	    e -> next_elm = NULL;
	    e -> block = NULL;
	}
	rm_block (b);
    }

    rm_partition (p);
    netw -> part = NULL;
}

/*
 * Reconfigures the specified partition.
 * All invalid blocks of p are merged into
 * two new blocks: a net error block and a device error block.
 * All blocks of p are set in the ACTIVE state.
 */
Public void reconfigure (partition *p)
{
    block *ThiS_b;
    block *dev_err_b, *net_err_b, *term_err_b;
    object * obj;
    object * next;

    Assert (p);

    if (p -> invalid == NULL) return;

 /* merge all invalid blocks into three new blocks */
 /* A net block, a terminal block and a device block    */

    verb_mesg ("Reconfiguring.\n");
    if (p -> invalid) {
	net_err_b = mk_block ();
	term_err_b = mk_block ();
	dev_err_b = mk_block ();
	net_err_b -> part = p;
	term_err_b -> part = p;
	dev_err_b -> part = p;

	while ((ThiS_b = p -> invalid)) {
	    del_block (ThiS_b);	/* unlink */
	    for (obj = ThiS_b -> head; obj; obj = next) {
		next = obj -> next_elm;
		if (Get_field (obj, TYPE) == DEVICE) {
		    /*
		    if (Get_field (obj -> call, TYPE) == PRIMITIVE) {
			block **prim_bb;
			int prim_no = find_prim_index (obj -> call -> name);
			obj -> next_elm = prim_bb[prim_no] -> head;
			prim_bb[prim_no] -> head = obj;
			obj -> block = prim_bb[prim_no];
			(prim_bb[prim_no] -> n)++;
		    }
		    else {
			obj -> next_elm = dev_err_b -> head;
			dev_err_b -> head = obj;
			obj -> block = dev_err_b;
			(dev_err_b -> n)++;
		    }
		    */
		    obj -> next_elm = dev_err_b -> head;
		    dev_err_b -> head = obj;
		    obj -> block = dev_err_b;
		    dev_err_b -> n_el++;
		}
		else if (Get_field (obj, TYPE) == NET) {
		    obj -> next_elm = net_err_b -> head;
		    net_err_b -> head = obj;
		    obj -> block = net_err_b;
		    net_err_b -> n_el++;
		}
		else if (Get_field (obj, TYPE) == TERMINAL) {
		    obj -> next_elm = term_err_b -> head;
		    term_err_b -> head = obj;
		    obj -> block = term_err_b;
		    term_err_b -> n_el++;
		}
	    }
	    rm_block (ThiS_b);
	}
	if (dev_err_b -> head) {
	    Set_field (dev_err_b, STATE, ACTIVE);
	    add_block (dev_err_b, TAIL);
	}
	else {
	    rm_block (dev_err_b);
	}

	if (net_err_b -> head) {
	    Set_field (net_err_b, STATE, ACTIVE);
	    add_block (net_err_b, TAIL);
	}
	else {
	    rm_block (net_err_b);
	}

	if (term_err_b -> head) {
	    Set_field (term_err_b, STATE, ACTIVE);
	    add_block (term_err_b, TAIL);
	}
	else {
	    rm_block (term_err_b);
	}
    }

 /* set all passive blocks to active */
    while ((ThiS_b = p -> passive)) {
	del_block (ThiS_b);
	Set_field (ThiS_b, STATE, ACTIVE);
	add_block (ThiS_b, TAIL);
    }

    verb_mesg ("Reconfigured.\n");
}

/*
 * Refines a partition possibly containing invalid blocks.
 * If error blocks should be created, the current partition
 * is reconfigured, and the process is continued.
 * This cycle is repeated untill the number of blocks no longer increases.
 * True is returned if a complete map could be constructed.
 * See also the function refine().
 */
Public boolean refine_err (partition *p, stack *st, short mode)
{
    long     old_n_blcks = 0;

    if (refine (p, st, mode) && p -> invalid == NULL)
	return (True);

    if (p -> invalid != NULL && p -> n_blcks > old_n_blcks) {
	old_n_blcks = p -> n_blcks;
	reconfigure (p);
	if (refine (p, st, mode))
	    return (True);
	if (refine (p, st, mode)) /* ThiS time, use an invalid block to  */
            return (True);        /* continue the partitioning process.  */
				  /* ThiS will prevent the creation of a */
				  /* a block with 2 non-matching devices */
				  /* in most cases.                      */
	reconfigure (p);
	if (refine (p, st, mode))
	    return (True);
    }
    return (False);
}

/*
 * Refines the specified partition.
 * The process continues until no further refinements can be made.
 * When all blocks are valid and of size two, True is returned.
 * If mode equals SAVE, all blocks which are split are pushed on a stack
 * (In order to facilitate the backtracking procedure.)
 * If mode equals SAVE, partition will call map() if necessary, to continue the refinement.
 */
Public boolean refine (partition *p, stack *st, int mode)
{
    block *ThiS;
    block *next_b;
    block *ref;
/*
    boolean done = True;
    while (done) {
	done = False;
	ThiS = p -> active;
	while (ThiS != NULL) {
	    if (ThiS -> n == 2) {
		refine_block (ThiS, st, mode);

		if (p -> invalid != NULL && mode == SAVE) {
		    return False;
		}
		done = True;
		break;
	    }
	    ThiS = ThiS -> next;
	    if (ThiS == p -> active) break;
	}
    }
    */

    while (p -> active) {
	ThiS = p -> active;
	ref = p -> active;
	while (ThiS != NULL) {
	    next_b = ThiS -> next;

	    if (Get_flag (ThiS, BOUNDED)) {
	    /* Bounded blocks have priority over other active blocks */
		refine_block (ThiS, st, mode);
		if (p -> invalid != NULL && mode == SAVE) {
		    return False;
		}
		break;
	    }
	    else if (next_b != ref && next_b != NULL) {
		ThiS = next_b;
	    }
	    else {
		ThiS = NULL;
		break;
	    }
	}
	if (ThiS != NULL) { /* a bounded block was processed */
	    continue;
	}
	else { /* we must process a non-bounded block */
	    Assert (p -> active);
	    refine_block (p -> active, st, mode);
	    if (p -> invalid != NULL && mode == SAVE) {
		return False;
	    }
	}
    }

    (void) cmp_devsizes (p, st, mode);

    if (p -> invalid != NULL) {
	return False;
    }
    else if (p -> n_blcks != p -> n_bound && mode == SAVE) {
       return map (p, mode);
    }
    else {
	return (p -> n_blcks == p -> n_bound ? True : False);
    }
}

#define NOM 0
#define ACT 1

Public void cmp_devsizes (partition *p, stack *st, int mode)
{
    block * that = NULL;
    block * next = NULL;
    block * ref = NULL;
    char  * val_str[2];
    double par_val[2];
    block * tmp[2];
    int i, isSplit;
    range * cur_r;

    if (!range_checks) return;

    for (that = p -> passive; that != NULL && that != ref; that = next) {
	next = that -> next;
	ref = p -> passive;
	if (!Get_flag (that, BOUNDED)) continue;

	if (Get_field (that -> head, TYPE) != DEVICE) continue;

	isSplit = False;

	for (cur_r = range_checks; cur_r != NULL && isSplit == False; cur_r = cur_r -> next) {

	    if (strcmp (that -> head -> call -> name, cur_r -> device) == 0) {

		val_str[NOM] = get_par (that -> head, cur_r -> parameter);
		val_str[ACT] = get_par (that -> head -> next_elm, cur_r -> parameter);

		if (val_str[NOM] == NULL || val_str[ACT] == NULL) continue;

		for (i = 0; i < 2; i++) {
		    par_val[i] = atof (val_str[i]);
		}

		if (!(par_val[NOM] <= (1 + cur_r -> value) * par_val[ACT] &&
		      par_val[ACT] <= (1 + cur_r -> value) * par_val[NOM])) {

		    del_block (that);
		    Malloc (that -> childs, 2, block *);
		    n_arr_alloc++;
		    n_arr_mem += 2 * sizeof (block *);
		    that -> n_childs = (short) 2;

		    for (i = 0; i < 2; i++) {
			tmp[i] = mk_block ();
			tmp[i] -> part = p;
			tmp[i] -> head = NULL;
			tmp[i] -> n_el = 0;
			*(that -> childs + i) = tmp[i];
			tmp[i] -> parent = that;
		    }

		    /* Assign 1st dev. to tmp[NOM] and 2nd to tmp[ACT] */

		    tmp[NOM] -> head = that -> head;
		    tmp[ACT] -> head = that -> head -> next_elm;

		    for (i = 0; i < 2; i++) {
			tmp[i] -> head -> block = tmp[i];
			tmp[i] -> head -> next_elm = NULL;
			tmp[i] -> n_el = 1;

			validate (tmp[i]);
			add_block (tmp[i], HEAD);
		    }
		    if (mode == SAVE) push (st, that);

		    isSplit = True;
		}
	    }
	}
    }
}

/*
 * Partitions the set of elements with respect to the specified block.
 * See also the function refine().
 */
Public void refine_block (block *ThiS, stack *st, short mode)
{
    object * e;
    block *b;
    block *next;
    partition * p;
    long     cnt;
    long     tch;
    int      split_mode;
    static int first = 1;

    double current_progress;

    p = ThiS -> part;

    if (T_opt && first) {
	fprintf (stderr, "[%ld]\tTot: %-4ld Act: %-4ld Pas: %-4ld Inv: %-4ld",
		p -> n_iter, p -> n_blcks, p -> n_active, p -> n_passive, p -> n_invalid);
	fprintf (stderr, " Bnd: %-4ld Tch: %-4d Spt: %-4d\n", p -> n_bound, 0, 0);
	first = 0;
    }

 /* propagate color to all neighbours of the elements of */
 /* the selected block					 */

    if (Get_flag (ThiS, ISMBN)) {
	split_mode = BYMBN;
    }
    else {
	split_mode = 0;
    }

    p -> touched = NULL;
    for (e = ThiS -> head; e != NULL; e = e -> next_elm)
	propagate (e);

    p -> n_iter++;

    del_block (ThiS);
    Set_field (ThiS, STATE, PASSIVE);
    add_block (ThiS, HEAD);

 /* split all the blocks of the touched block list	 */
 /* according to their colors.				 */

    cnt = 0;
    tch = p -> n_touched;
    for (b = p -> touched; b != NULL; b = next) {

	next = b -> t_nxt;
	b -> t_nxt = NULL;
	Clr_flag (b, TOUCHED);
	p -> n_touched--;

	if (split_mode == BYMBN && b == ThiS) {
	    continue;
	}

	if (split (b, split_mode) > 1) {
	    if (mode == SAVE) push (st, b);
	    cnt++;
	}
    }

    p -> touched = NULL;
    p -> n_touched = 0;

    if (T_opt) {
	fprintf (stderr, "[%ld]\tTot: %-4ld Act: %-4ld Pas: %-4ld Inv: %-4ld",
		p -> n_iter, p -> n_blcks, p -> n_active, p -> n_passive, p -> n_invalid);
	fprintf (stderr, " Bnd: %-4ld Tch: %-4ld Spt: %-4ld\n", p -> n_bound, tch, cnt);
    }

    current_progress = (double) p -> n_bound / (double) network_size * 100.0;
    if (progress_size && current_progress > next_size_report) {
	fprintf (stderr, "Matched %2.2lf %%\n", current_progress);

	while (current_progress > next_size_report) {
	    next_size_report += progress_size;
	}
    }
}

/*
 * Splits a block into several disjunct fractions
 * according to the colors of the elements.
 * The number of fractions into which the block is split is returned.
 * If mode equals SAVE, the history of the refinement
 * process is maintained in an hierarchical data structure.
 */

#define SMALLPRIME	23

Public long split (block *b, short mode)
{
    Private boolean initialised = False;
    Private block *table[SMALLPRIME];	/* internal hash table */
 /* for partitioning    */
    register object * ThiS;
    register object * next;
    register object * pnt;
    register object * e, * e1, * e2;
    register block *that;
    register block *tmp;
    boolean found;
    block *max;
    long     cnt;
    short   index;
    partition * p;
    boolean invalid_blcks;

    Assert (b);
    Assert (!Get_flag (b, TOUCHED));

 /* Splitting an invalid blok may well result in a number of valid blocks (AJH)
    Assert (Get_field (b, STATE) != INVALID);
 */

 /*
  Assert(b -> n > 1);
  */

    if (!initialised) {
	for (index = 0; index < SMALLPRIME; index++)
	    table[index] = NULL;
	initialised = True;
    }

    if (mode != BYMBN && Get_flag (b, ISMBN)) {
	del_block (b);
	Set_field (b, STATE, ACTIVE);
	add_block (b, HEAD);
	return 1;
    }

    p = b -> part;

    del_block (b);

 /* Classify all elements of b according to their color	    */
 /* and link them at the appropriate place in the hash table */

    for (ThiS = b -> head; ThiS != NULL; ThiS = next) {
	/*
	if (strcmp (ThiS -> name, "xxxx") == 0) {
	    fprintf (stdout, "Splitting xxxx\n");
	}
	*/
	next = ThiS -> next_elm;
	ThiS -> next_elm = NULL;
	ThiS -> block = NULL;
	b -> n_el--;
	found = False;
	index = (short) (ThiS -> color) % SMALLPRIME;
	for (that = table[index]; that != NULL; that = that -> next) {
	    pnt = that -> head;
	    if (pnt != NULL) {
		if (pnt -> color == ThiS -> color &&
			pnt -> edges == ThiS -> edges &&
			pnt -> flavor == ThiS -> flavor) {
		    ThiS -> next_elm = that -> head;
		    that -> head = ThiS;
		    ThiS -> block = that;
		    that -> n_el++;
		    found = True;
		    break;
		}
	    }
	}
	if (!found) {
	    tmp = mk_block ();
	    tmp -> part = p;
	    tmp -> next = table[index];
	    table[index] = tmp;
	    ThiS -> next_elm = tmp -> head;
	    tmp -> head = ThiS;
	    ThiS -> block = tmp;
	    tmp -> n_el++;
	}
    }

 /* search for largest fraction and set states */

    cnt = 0;
    max = NULL;
    for (index = 0; index < SMALLPRIME; index++) {
	for (that = table[index]; that != NULL; that = that -> next) {

	    /* Why ? (AJH)
	    if (!max || that -> n > max -> n)
	    */
	    max = that;

	    Set_field (that, STATE, ACTIVE);
	    cnt++;
	}
    }

    Assert (cnt > 0);

    if (cnt == 1) {
	b -> head = max -> head;
	b -> n_el = max -> n_el;
	table[(max -> head -> color) % SMALLPRIME] = NULL;
	for (e = max -> head; e != NULL; e = e -> next_elm) {
	    e -> block = b;
	    e -> color = 0;
	    e -> edges = 1;
	}
	validate (b);
	if (mode == MKMBN && Get_flag (b, BOUNDED)) {
	    e1 = b -> head;
	    e2 = e1 -> next_elm;
	    if (check_name (e1 -> name) &&
		strcmp (e1 -> name, e2 -> name) == 0) {
		verb_mesg ("Matched %s by name\n", e1 -> name);
		Set_flag (b, ISMBN);
	    }
	}
	add_block (b, HEAD);
	rm_block (max);
	return (cnt);
    }

    if (Get_flag (b, ISMBN)) {
	verb_mesg ("Splitting matched_by_name block\n");
    }

    Malloc (b -> childs, cnt, block *);
    n_arr_alloc++;
    n_arr_mem += cnt * sizeof (block *);
    b -> n_childs = (short) cnt;

 /* add all fractions to their corresponding block lists */

    invalid_blcks = False;
    cnt = 0;
    for (index = 0; index < SMALLPRIME; index++) {
	for (that = table[index]; that != NULL; that = tmp) {
	    tmp = that -> next;

	/* clear colors of all elements */

	    for (ThiS = that -> head; ThiS != NULL; ThiS = ThiS -> next_elm) {
		ThiS -> color = 0;
		ThiS -> edges = 1;
	    }

	    validate (that);

	    if (mode == MKMBN && Get_flag (that, BOUNDED)) {
		e1 = that -> head;
		e2 = e1 -> next_elm;
		if (check_name (e1 -> name) &&
		    strcmp (e1 -> name, e2 -> name) == 0) {
		    verb_mesg ("Matched %s by name\n", e1 -> name);
		    Set_flag (that, ISMBN);
		}
	    }

	    add_block (that, HEAD);

	    *(b -> childs + cnt) = that;
	    that -> parent = b;
	    cnt++;
	}
	table[index] = NULL;
    }

#ifdef DEBUG
    if (D_opt)
	fprintf (stderr, "Split into %ld fractions.\n", cnt);
#endif

    return (cnt);
}

/*
 * Merges the childs of the specified block into the single block b.
 * This function is designed to be used in conjunction with the function split.
 */
Public void merge_block (block *b)
{
    long     i;
    object * ThiS;
    object * next;
    block *ThiS_child;

    Assert (b -> n_childs > 0);
    Assert (b -> childs);

    b -> head = NULL;
    for (i = 0; i < b -> n_childs; i++) {

	ThiS_child = *(b -> childs + i);

	del_block (ThiS_child);	/* unlink */

	for (ThiS = ThiS_child -> head; ThiS != NULL; ThiS = next) {
	    next = ThiS -> next_elm;
	    ThiS -> next_elm = b -> head;
	    b -> head = ThiS;
	    ThiS -> block = b;
	}
	b -> n_el += ThiS_child -> n_el;

	rm_block (ThiS_child);
    }

    Free (b -> childs);
    b -> childs = NULL;
    b -> n_childs = 0;
    add_block (b, HEAD);	/* link */
}

/*
 * Propagates the properties of the specified object to all its neighbours.
 */
Public void propagate (object *obj)
{
    object *ThiS;
    link_type *lnk, *next_lnk;
    int device = (Get_field (obj, TYPE) == DEVICE);

    for (lnk = obj -> head; lnk; lnk = next_lnk) {
	if (device) {
	    next_lnk = lnk -> next_down;
	    if (!(ThiS = lnk -> net)) continue;
	}
	else {
	    next_lnk = lnk -> next_up;
	    ThiS = lnk -> dev;
	}
	ThiS -> color += 1;
	ThiS -> edges *= lnk -> color;
	touch_b (ThiS -> block);
    }
}

/*
 * Validates the specified block.
 * The criteria wich must be satisfied are:
 *
 * 1) even number of elements.
 * 2) number of elements > 1.
 * 3) equal number of elements from both networks.
 *
 * The state of the block is set accordingly.
 */
Public void validate (block *ThiS)
{
    long     cnt;
    network * netw;
    object * e;

    Assert (ThiS);
    Assert (ThiS -> head);

    Clr_flag (ThiS, BOUNDED);

    if (ThiS -> n_el <= 1 || ThiS -> n_el % 2 == 1) {
	if (Get_field (ThiS -> head, TYPE) == DEVICE && !d_opt) {
	    Set_field (ThiS, STATE, INVALID);
	    return;
	}
	else if (Get_field (ThiS -> head, TYPE) == NET && !i_opt) {
	    Set_field (ThiS, STATE, INVALID);
	    return;
	}
	else if (Get_field (ThiS -> head, TYPE) == TERMINAL) {
	    Set_field (ThiS, STATE, INVALID);
	    return;
	}
    }

    cnt = 0;
    netw = ThiS -> head -> netw;
    Assert (ThiS -> head -> netw);
    for (e = ThiS -> head; e != NULL; e = e -> next_elm) {
	if (e -> netw == netw) cnt++;
    }

    if (cnt != ThiS -> n_el - cnt) {
	if (Get_field (ThiS -> head, TYPE) == DEVICE && !d_opt) {
	    Set_field (ThiS, STATE, INVALID);
	    return;
	}
	else
	    if (Get_field (ThiS -> head, TYPE) == NET && !i_opt) {
		Set_field (ThiS, STATE, INVALID);
		return;
	    }
	    else
		if (Get_field (ThiS -> head, TYPE) == TERMINAL) {
		    Set_field (ThiS, STATE, INVALID);
		    return;
		}
    }
    else {
	if (ThiS -> n_el == 2) Set_flag (ThiS, BOUNDED);
    }
}

Public void validate_list (block **aList)
{
    block * that, * next;
    block * ref;
    int found = 0;

    if (*aList == NULL) return;

    ref = NULL;
    for (that = *aList; that != NULL && that != ref; that = next) {
        next = that -> next;
	if (next == that) { /* only remaining item in aList */
	    next = NULL;
	}

	del_block (that);
	validate (that);
	add_block (that, HEAD);
	if (!found && that == *aList) {
	    ref = *aList;
	    found = 1;
	}
    }
}

/*
 * Adds the specified block to the touch list of the corresponding partition.
 */
Public void touch_b (block *blk)
{
    partition * p;

    if (blk && !Get_flag (blk, TOUCHED)) {
	p = blk -> part;
	blk -> t_nxt = p -> touched;
	p -> touched = blk;
	Set_flag (blk, TOUCHED);
	p -> n_touched++;
    }
}

/*
 * Returns a pointer to an exact copy of the specified block.
 */
Public block *cp_block (block *ThiS)
{
    block *copy;

    copy = mk_block ();

    copy -> part = ThiS -> part;
    copy -> next = ThiS -> next;
    copy -> prev = ThiS -> prev;
    copy -> head = ThiS -> head;
    copy -> t_nxt = ThiS -> t_nxt;
    copy -> n_el = ThiS -> n_el;
    copy -> flags = ThiS -> flags;

    return (copy);
}

/*
 * Tries to map the elements of a partition by name.
 * This routine is called after 'init_partition ()',
 * which links all blocks into the 'active' list of the partition.
 * Hence, only blocks inthis list are traversed.
 */
Public boolean match_by_name (partition *p)
{
    object * e1, *e2;
    block *ThiS, *next_obj;
    int i, n_active;
    static long uniq_color = 0;

    verb_mesg ("match network elements by name\n");

    if (uniq_color == 0) uniq_color = gen_prime ();

    n_active = p -> n_active;

    for (i = 0, ThiS = p -> active; i < n_active; i++, ThiS = next_obj) {
        next_obj = ThiS -> next;
	/* After splitting, 'ThiS' is no longer part of a list */

	if (Get_flag (ThiS, BOUNDED)) continue;

	for (e1 = ThiS -> head; e1 != NULL; e1 = e1 -> next_elm) {
	    if (e1 -> netw == nom_netw && check_name (e1 -> name)) {

		/* find an 'actual' object matching 'e1' by name */
		for (e2 = ThiS -> head; e2 != NULL; e2 = e2 -> next_elm) {
		    if (e2 -> netw == act_netw &&
			    e1 -> flavor == e2 -> flavor &&
			    strcmp (e1 -> name, e2 -> name) == 0) {
			/* give them a unique color */
			e1 -> color = e2 -> color = uniq_color++;
			break;
		    }
		}
	    }
	}
	split (ThiS, MKMBN);
    }
    return True;
}

/*
 * check_name () checks if 'name' does not contain elements starting
 * with a number or an '_'.
 */
Private boolean check_name (char *name)
{
    char *c_ptr;

    Assert (name);

    for (c_ptr = name - 1; c_ptr != NULL; c_ptr = strchr (c_ptr, '.')) {
	++c_ptr;
        if (*c_ptr == '_' || (*c_ptr >= '0' && *c_ptr <= '9')) return False;
    }
    return True;
}
