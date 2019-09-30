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

static void copy_ct (struct check *pntr, unsigned int mask, int chk_type);
static int  mask_dec (unsigned int mask);
static void split_fld  (struct sr_field *c_sr, int ysplit);
static void update_fld (struct sr_field *c_sr, char edge_type, int chk_type, unsigned int mask);

/* The procedure insert_edge inserts an edge in the
 * state_ruler. It uses two functions: split_edge to
 * split an edge in the stateruler and update_fld to
 * update a field in the stateruler.
 */

void insert_edge (struct sr_field **r_srp, int yb, int yt, char edge_type, int chk_type, unsigned int mask)
/* r_srp  - ptr to first field of rsr */
/* yb, yt - bottom, top of the edge */
/* chk_type - check_type of the edge */
/* mask  - layer of the edge */
{
    struct sr_field *c_sr; /* current rsr-pointer */

    c_sr = (*r_srp);
    while (yb >= c_sr -> yt) {
	c_sr = c_sr -> next;
    }
    (*r_srp) = c_sr;
    if (yb > c_sr -> yb) {
	split_fld (c_sr, yb);
	c_sr = c_sr -> next;
    }
    while (yt >= c_sr -> yt) {
	update_fld (c_sr, edge_type, chk_type, mask);
	c_sr = c_sr -> next;
    }
    if (yt > c_sr -> yb) {
	split_fld (c_sr, yt);
	update_fld (c_sr, edge_type, chk_type, mask);
    }
}

static void split_fld (struct sr_field *c_sr, int ysplit)
/* c_sr - current sr_pointer */
/* ysplit - split_value */
{
/* this function splits a stateruler_field in two parts,
 * ysplit being the value where the split is carried out.
 * The contents of the newly formed field in the
 * stateruler are copied from the already existing part.
 */
    int     i;			/* loop variable	 */
    struct sr_field *p_nf;	/* ptr to new sr_field	 */
    struct check   *p_chk;	/* ptr to the chk_types	 */
    struct checktype   *pn_ct;	/* ptr to chged chk_types */
    struct checktype   *p_chg_ct;/* ptr to chged chk_types */

    ALLOC (p_nf, sr_field);
    p_nf -> yb = ysplit;
    p_nf -> yt = c_sr -> yt;
    p_nf -> chk_type = c_sr -> chk_type;
    if (c_sr -> p_chk) {
	ALLOC (p_chk, check);
	p_nf -> p_chk = p_chk;
	for (i = 0; i < MAX_NOMASKS; i++)
	    p_chk -> chk_arr[i] = c_sr -> p_chk -> chk_arr[i];
    }
    p_chg_ct = c_sr -> p_chg_ct;
    while (p_chg_ct) {
	ALLOC (pn_ct, checktype);
	pn_ct -> mask = p_chg_ct -> mask;
	pn_ct -> chk_type = p_chg_ct -> chk_type;
	pn_ct -> next = p_nf -> p_chg_ct;
	p_nf -> p_chg_ct = pn_ct;
	p_chg_ct = p_chg_ct -> next;
    }
    p_nf -> mask_past = c_sr -> mask_past;
    p_nf -> mask_fut = c_sr -> mask_fut;
    p_nf -> ov_mask = c_sr -> ov_mask;
    p_nf -> next = c_sr -> next;
    p_nf -> prev = c_sr;
    p_nf -> next -> prev = p_nf;
    p_nf -> prev -> next = p_nf;
    c_sr -> yt = ysplit;
}

static void update_fld (struct sr_field *c_sr, char edge_type, int chk_type, unsigned int mask)
/* c_sr - current sr_pointer */
/* edge_type - type of the edge to insert */
/* chk_type - check_type of the edge */
/* mask - layer the edge is part of */
{
/* This function updates an exsisting stateruler_field
 * with the new data of the edge.
 */
    int     i;			/* loop_variable	 */
    struct check   *p_chk;	/* ptr to the chk_types  */
    struct checktype   *pn_ct;	/* ptr to chged chk_types */

/* if an overlap starts or stops in a certain mask this	 */
/* is indicated in the state_ruler by setting up the	 */
/* appropriate bit in the ov_mask.			 */

    if ((edge_type & (START_OV + STOP_OV)) != 0) {
	c_sr -> ov_mask = c_sr -> ov_mask | mask;
    }

/* in case of a start_edge the check_type structure is	 */
/* updated and the layer of the edge added to  mask_fut. */

    if ((edge_type & START_EDGE) != 0) {
	i = mask_dec (mask);
	if (i < 0) {
	    fprintf (stderr, "error in masktypes");
	    die (1);
	}
	if (edges[i].mask_type != TERM_MASK) {
	    if (c_sr -> chk_type == INITIAL)
		c_sr -> chk_type = chk_type;
	    else
		if (c_sr -> chk_type != chk_type) {
		    if (c_sr -> p_chk == NULL) {
			ALLOC (p_chk, check);
			c_sr -> p_chk = p_chk;
			copy_ct (p_chk, c_sr -> mask_fut | c_sr -> mask_past,
				c_sr -> chk_type);
		    }
		    c_sr -> p_chk -> chk_arr[i] = chk_type;
		    c_sr -> chk_type = DIFF_CT;
		}
	}
	c_sr -> mask_fut = c_sr -> mask_fut | mask;
    }
    else {

/* in case of a stop_edge the layer of the edge is	 */
/* removed from mask_fut. The check_type of the edge is	 */
/* not yet removed from the check_type structures because*/
/* we still need the check_type in the functions	 */
/* check_hrchy and extr_prof.  The updating of the	 */
/* check_type structure is done in the function		 */
/* update_sr in this case.				 */

	if ((edge_type & STOP_EDGE) != 0) {
	    c_sr -> mask_fut = c_sr -> mask_fut & ~mask;
	}

/* in case of a change_edge the check_type is updated	 */
/* and the appropriate bit in the ov_mask is set to	 */
/* indicate that a change of check_type has taken place. */

	if ((edge_type & CHANGE_EDGE) != 0) {
	    c_sr -> ov_mask = c_sr -> ov_mask | mask;
	    i = mask_dec (mask);
	    if (i < 0) {
		fprintf (stderr, "error in masktypes");
		die (1);
	    }
	    if (edges[i].mask_type != TERM_MASK) {

/* in case of a change of check_type, store the old	 */
/* value(s) per mask.					 */

		ALLOC (pn_ct, checktype);
		pn_ct -> mask = mask;
		if (c_sr -> p_chk == NULL)
		    pn_ct -> chk_type = c_sr -> chk_type;
		else
		    pn_ct -> chk_type = c_sr -> p_chk -> chk_arr[i];
		pn_ct -> next = c_sr -> p_chg_ct;
		c_sr -> p_chg_ct = pn_ct;

		if (mask != ((c_sr -> mask_fut | c_sr -> mask_past) &
			    ~mask_terms)) {
		    if (c_sr -> p_chk == NULL) {
			ALLOC (p_chk, check);
			c_sr -> p_chk = p_chk;
			copy_ct (p_chk, c_sr -> mask_fut | c_sr -> mask_past,
				c_sr -> chk_type);
			c_sr -> p_chk -> chk_arr[i] = chk_type;
			c_sr -> chk_type = DIFF_CT;
		    }
		    else {
			c_sr -> p_chk -> chk_arr[i] = chk_type;
			for (i = 1; i < nomasks; i++) {
			    if ((c_sr -> p_chk -> chk_arr[i] != INITIAL) &&
				    (c_sr -> p_chk -> chk_arr[i] != chk_type))
				break;
			}
			if (i >= nomasks) {
			    FREE (c_sr -> p_chk);
			    c_sr -> p_chk = NULL;
			    c_sr -> chk_type = chk_type;
			}
		    }
		}
		else
		    c_sr -> chk_type = chk_type;
	    }
	}
    }
}

static void copy_ct (struct check *pntr, unsigned int mask, int chk_type)
/* pntr - ptr to the chk_types */
/* mask - layers to be used */
/* chk_type - chk_type to be copied */
{
/* this function copies the check_types of the layers
 * given in mask from the variable chk_type The.
 * check_types of the other layers are set to INITIAL.
 */
    int i;
    for (i = 0; i < nomasks; i++) {
	if (((mask & edges[i].mask) != 0) && (edges[i].mask_type != TERM_MASK))
	    pntr -> chk_arr[i] = chk_type;
	else
	    pntr -> chk_arr[i] = INITIAL;
    }
}

static int mask_dec (unsigned int mask)
/* mask - mask to be converted */
{
/* this function determines the decimal code from the
 * bit_code of a mask.
 */
    int i;
    for (i = 0; i < nomasks; i++) {
	if (edges[i].mask == mask) return (i);
    }
    return (-1);
}
