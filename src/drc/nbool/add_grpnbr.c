/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
 *	J. Liedorp
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

#include "src/drc/nbool/nbool.h"

static int  fdgrp_name (struct group_tree *group);
static void free_groups (void);
static void free_grp_sr (void);
static struct group_tree *fdgrp_ptr (struct group_tree *group);
static void ins_sr (struct grp_sr **r_srp, int yb, int yt, int edge_type);
static struct group_tree *merge_groups (struct group_tree *group1, struct group_tree *group2);
static int  number_groups (void);
static void split_grp_fld (struct grp_sr *c_sr, int ysplit);
static void update_group (struct grp_sr *gsr_pntr);
static void update_grp_fld (struct grp_sr *c_sr, int edge_type);
static void update_grp_sr (struct grp_sr *g_sr);

/* This file contains the procedures to add the group_numbers to the edges.
 * Most of them have been copied from the program makevln.
 */

void add_grpnbr (int form_nbr)
/* form_nbr - bool_form_number */
{
/* main procedure for the addition of the group_numbers
 * A state_ruler is built containing only the group_pntrs
 * As its input it uses the binary files made by nbool
 * so far. (the TEMP_ONE files).
 * First a tempory file (TEMP_TWO) is made containing the
 * group_pointers, and the TEMP_ONE file is removed.
 * Then the TEMP_TWO file is read and the group_pointers
 * replaced by the group_numbers. At last the vln_file
 * in the 'normal' format is made and the TEMP_TWO file
 * is also deleted.
 */
    char    fw_name[10];	/* bool_ filename	 */
    struct grp_sr  *g_sr;	/* ptr to groupstruct.	 */
    FILE   *f_in;		/* input_file		 */
    FILE   *f_int;		/* intermediate_file	 */
    DM_STREAM   *f_out;		/* output_file		 */
    int     x_sr;		/* state_ruler position	 */

    sprintf (fr_name, TEMP_ONE, form_nbr, pid);
    sprintf (fi_name, TEMP_TWO, form_nbr, pid);

    OPEN (f_in, fr_name, "r");
    OPEN (f_int, fi_name, "w");

    x_sr = -MAXINT;
    group_ptr = NULL;

    /* initialize state ruler */
    g_sr = &grp_sr_head;
    g_sr -> yt = MAXINT;
    g_sr -> yb = -MAXINT;
    g_sr -> group = NULL;
    g_sr -> next = g_sr -> prev = &grp_sr_head;

    while (fread ((char *) b_pntr, sizeof (*b_pntr), 1, f_in) != 0) {
	if (b_pntr -> x != x_sr) {
	    x_sr = b_pntr -> x;
	    g_sr = &grp_sr_head;
	    update_grp_sr (g_sr);
	}
	if ((b_pntr -> edge & START_EDGE) != 0) {
	    ins_sr (&g_sr, b_pntr -> yb, b_pntr -> yt, START_EDGE);
	    update_group (g_sr);
	}
	else
	    if ((b_pntr -> edge & STOP_EDGE) != 0) {
		update_group (g_sr);
		ins_sr (&g_sr, b_pntr -> yb, b_pntr -> yt, STOP_EDGE);
	    }
	    else
		update_group (g_sr);

	fwrite ((char *) b_pntr, sizeof (*b_pntr), 1, f_int);
    }
    free_grp_sr ();
    CLOSE (f_in);
    CLOSE (f_int);
    unlink (fr_name);

    number_groups ();

    sprintf (fw_name, "bool_%d", form_nbr);
    OPEN (f_int, fi_name, "r");
    f_out = dmOpenStream (mod_key, fw_name, "w");

    while (fread ((char *) b_pntr, sizeof (*b_pntr), 1, f_int)) {
	gvlnlay.x  = b_pntr -> x;
	gvlnlay.yb = b_pntr -> yb;
	gvlnlay.yt = b_pntr -> yt;
	gvlnlay.occ_type = b_pntr -> edge;
	gvlnlay.con_type = b_pntr -> conn;
	gvlnlay.chk_type = b_pntr -> ct;
	gvlnlay.grp_number = fdgrp_name (b_pntr -> grp);
	dmPutDesignData (f_out, GEO_VLNLAY);
    }
    CLOSE (f_int);
    dmCloseStream (f_out, COMPLETE);
    unlink (fi_name);
    free_groups ();
}

static void ins_sr (struct grp_sr **r_srp, int yb, int yt, int edge_type)
/* r_srp  - ptr to first field of rsr */
/* yb, yt - bottom, top value of edge_type */
{
    struct grp_sr *c_sr; /* current rsr-pointer */

    c_sr = (*r_srp);
    while (yb >= c_sr -> yt) {
	c_sr = c_sr -> next;
    }
    (*r_srp) = c_sr;
    if (yb > c_sr -> yb) {
	split_grp_fld (c_sr, yb);
	c_sr = c_sr -> next;
    }
    while (yt >= c_sr -> yt) {
	update_grp_fld (c_sr, edge_type);
	c_sr = c_sr -> next;
    }
    if (yt > c_sr -> yb) {
	split_grp_fld (c_sr, yt);
	update_grp_fld (c_sr, edge_type);
    }
}

static void split_grp_fld (struct grp_sr *c_sr, int ysplit)
{
    struct grp_sr *p_nf; /* ptr to new state ruler field */

    ALLOC (p_nf, grp_sr);
    p_nf -> yb = ysplit;
    p_nf -> yt = c_sr -> yt;
    p_nf -> group = c_sr -> group;
    p_nf -> next = c_sr -> next;
    p_nf -> prev = c_sr;
    p_nf -> next -> prev = p_nf;
    p_nf -> prev -> next = p_nf;
    c_sr -> yt = ysplit;
}

static void update_grp_fld (struct grp_sr *c_sr, int edge_type)
{
    struct group_tree  *grp_prev;
    struct group_tree  *grp_next;
    struct group_tree  *group1;

    if (edge_type == START_EDGE) {
	grp_prev = c_sr -> prev -> group;
	grp_next = c_sr -> next -> group;
	if ((grp_prev != NULL) && (grp_next != NULL)) {
	    c_sr -> group = merge_groups (c_sr -> group, grp_prev);
	    c_sr -> group = merge_groups (c_sr -> group, grp_next);
	}
	if ((grp_prev != NULL) && (grp_next == NULL)) {
	    c_sr -> group = merge_groups (c_sr -> group, grp_prev);
	}
	if ((grp_prev == NULL) && (grp_next != NULL)) {
	    c_sr -> group = merge_groups (c_sr -> group, grp_next);
	}
	if ((grp_prev == NULL) && (grp_next == NULL)) {
	    ALLOC (group1, group_tree);
	    c_sr -> group = group1;
	    group1 -> tree.count = 1;
	    group1 -> parent = NULL;
	    group1 -> next = group_ptr;
	    group_ptr = group1;
	}
    }
    if (edge_type == STOP_EDGE) {
	c_sr -> group = NULL;
    }
}

static void update_grp_sr (struct grp_sr *g_sr)
{
    struct grp_sr  *gsr_pntr;
    struct grp_sr  *gsr_pntr_next;

    gsr_pntr = g_sr -> next;
    while (gsr_pntr != g_sr) {
	gsr_pntr_next = gsr_pntr -> next;
	if (gsr_pntr -> group == gsr_pntr -> prev -> group) {
	    gsr_pntr -> prev -> yt = gsr_pntr -> yt;
	    gsr_pntr -> next -> prev = gsr_pntr -> prev;
	    gsr_pntr -> prev -> next = gsr_pntr -> next;
	    FREE (gsr_pntr);
	}
	gsr_pntr = gsr_pntr_next;
    }
}

static void update_group (struct grp_sr *gsr_pntr)
{
    while (gsr_pntr -> yb > b_pntr -> yb) {
	gsr_pntr = gsr_pntr -> prev;
    }
    while (gsr_pntr -> yt <= b_pntr -> yb) {
	gsr_pntr = gsr_pntr -> next;
    }
    b_pntr -> grp = gsr_pntr -> group;
}

static void free_groups () /* free all groups */
{
    struct group_tree  *group;

    while (group_ptr) {
	group = group_ptr;
	group_ptr = group -> next;
	FREE (group);
    }
}

static struct group_tree *merge_groups (struct group_tree *group1, struct group_tree *group2)
{
/* Merge two groups, i.e. make one the parent of the other */

    struct group_tree *large, *small;

    if (group1 == NULL && group2 == NULL) {
	fprintf (stderr, " Function : merge_groups \n");
	fprintf (stderr, " ??? Unidentified situation\n");
	return (NULL);
    }
    else
	if (group1 == NULL)
	    return (group2);
	else
	    if (group2 == NULL)
		return (group1);
	    else {
		group1 = fdgrp_ptr (group1);
		group2 = fdgrp_ptr (group2);
		if (group1 == group2)
		    return (group1);
		if (group1 -> tree.count <= group2 -> tree.count) {
		    large = group2;
		    small = group1;
		}
		else {
		    large = group1;
		    small = group2;
		}
		small -> parent = large;
		large -> tree.count += small -> tree.count;
		return (large);
	    }
}

static int number_groups ()
{
/* Number the groups in the group_tree */

    struct group_tree  *group;
    int     group_number = 0;

    group = group_ptr;
    while (group) {
	if (group -> parent == NULL)
	    group -> tree.name = ++group_number;
	group = group -> next;
    }

    return (group_number);
}

static int fdgrp_name (struct group_tree *group)
{
/* Find the root node of the group_tree from which the
** group is part. Collaps path to root node, and return
** name of root node.
*/
    struct group_list  *head = NULL;
    struct group_list  *tmp;

    while (group -> parent && group -> parent -> parent) {
	ALLOC (tmp, group_list);
	tmp -> group = group;
	tmp -> next = head;
	head = tmp;
	group = group -> parent;
    }

    if (group -> parent)
	group = group -> parent;

    while (head) {
	head -> group -> parent = group;
	tmp = head;
	head = head -> next;
	FREE (tmp);
    }

    return (group -> tree.name);
}

static struct group_tree *fdgrp_ptr (struct group_tree *group)
{
/* Find the root node of the group_tree from which the
** group is part. Collaps path to root node, and return
** pointer to root node.
*/
    struct group_list  *head = NULL;
    struct group_list  *tmp;

    while (group -> parent && group -> parent -> parent) {
	ALLOC (tmp, group_list);
	tmp -> group = group;
	tmp -> next = head;
	head = tmp;
	group = group -> parent;
    }

    if (group -> parent)
	group = group -> parent;

    while (head) {
	head -> group -> parent = group;
	tmp = head;
	head = head -> next;
	FREE (tmp);
    }

    return (group);
}

static void free_grp_sr ()
{
    struct grp_sr  *gsr_pntr;
    struct grp_sr  *gsr_pntr_next;

    gsr_pntr = (&grp_sr_head) -> next;
    while (gsr_pntr != &grp_sr_head) {
	gsr_pntr_next = gsr_pntr -> next;
	FREE (gsr_pntr);
	gsr_pntr = gsr_pntr_next;
    }
}
