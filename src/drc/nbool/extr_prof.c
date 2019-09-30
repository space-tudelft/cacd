/*
 * ISC License
 *
 * Copyright (C) 1982-2018 by
 *	T.G.R. van Leuken
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

static void add_edge    (struct sr_field *c_sr, struct form *frm_pntr,
			char edge_type, int chk_type, int sr_pos);
static void buff_edge   (struct sr_field *c_sr, int sr_pos);
static int  det_chktype (struct sr_field *c_sr, unsigned int mask);
static int  det_old_ct  (struct sr_field *c_sr, unsigned int mask);
static void update_edge (struct sr_field *c_sr, struct form *frm_pntr);

void extr_profile (int sr_pos)
/* sr_pos - x_position of the sr */
{
/* This function examines the state_ruler and determines
 * according to the difference in the mask_past and mask_fut
 * layers which edges must be added to the bolean files.
 */
    struct sr_field *c_sr; /* ptr to the current sr */

    c_sr = h_sr -> next;
    while (c_sr != h_sr -> prev) {
	if ((c_sr -> mask_fut != c_sr -> mask_past) ||
		(c_sr -> p_chg_ct != NULL)) {
	    buff_edge (c_sr, sr_pos);
	}
	c_sr = c_sr -> next;
    }
}

static void buff_edge (struct sr_field *c_sr, int sr_pos)
/* c_sr  - ptr to the current sr */
/* sr_pos - state_ruler position */
{
/* This function determines in which bolean file the
 * edge must be placed and if it concerns a start, stop
 * or change_edge. It uses two subroutines: add_edge
 * to add a new edge to the buffers of a file and
 * update_edge to update an edge in the file buffers in
 * the case of adjecent edges.
 */
    int     pres_flag;		/* form status in sr_field */
    unsigned int    mask_past;	/* sr_layers of the past */
    unsigned int    mask_fut;	/* sr_layers of the fut. */
    struct min_term *mt_pntr;	/* current min_term ptr	 */
    struct form *frm_pntr;	/* current form pointer	 */
    char    edge_type;		/* edge_type to insert	 */
    int     chk_type;		/* check_type of new edge */
    int     chk_type_old;	/* previous check_type	 */

    edge_type    = 0; /* suppres uninitialized warning */
    chk_type     = 0; /* suppres uninitialized warning */
    chk_type_old = 0; /* suppres uninitialized warning */

    mask_past = c_sr -> mask_past;
    mask_fut = c_sr -> mask_fut;
    frm_pntr = fp_head;
    while (frm_pntr) {
	pres_flag = 0;
	if ((((mask_past ^ mask_fut) | c_sr -> ov_mask) &
		    frm_pntr -> vuln_mask) == 0) {
	    frm_pntr = frm_pntr -> next;
	    continue;
	}
	mt_pntr = frm_pntr -> mt_pntr;
	while (mt_pntr) {
	    if (((mask_past & mt_pntr -> mask) == mt_pntr -> mask) &&
		    ((~mask_past & mt_pntr -> not_mask) ==
			mt_pntr -> not_mask)) {
		pres_flag = pres_flag | PAST_PRESENT;
		if (c_sr -> chk_type != DIFF_CT)
		    chk_type = c_sr -> chk_type;
		else
		    chk_type = det_chktype (c_sr, mt_pntr -> mask);
		if ((c_sr -> p_chg_ct != NULL) &&
			((c_sr -> ov_mask & mt_pntr -> mask) != 0))
		    chk_type_old = det_old_ct (c_sr, mt_pntr -> mask);
		else
		    chk_type_old = chk_type;
		break;
	    }
	    mt_pntr = mt_pntr -> next;
	}
	mt_pntr = frm_pntr -> mt_pntr;
	while (mt_pntr) {
	    if (((mask_fut & mt_pntr -> mask) == mt_pntr -> mask) &&
		    ((~mask_fut & mt_pntr -> not_mask) ==
			mt_pntr -> not_mask)) {
		pres_flag = pres_flag | FUT_PRESENT;
		if (c_sr -> chk_type != DIFF_CT)
		    chk_type = c_sr -> chk_type;
		else
		    chk_type = det_chktype (c_sr, mt_pntr -> mask);
		if ((c_sr -> p_chg_ct != NULL) &&
			((c_sr -> ov_mask & mt_pntr -> mask) != 0))
		    chk_type_old = det_old_ct (c_sr, mt_pntr -> mask);
		else
		    chk_type_old = chk_type;
		break;
	    }
	    mt_pntr = mt_pntr -> next;
	}
	switch (pres_flag) {
	    case NOT_PRESENT:
		edge_type = '@';
		break;
	    case PAST_PRESENT:
		if (chk_type_old == chk_type)
		    edge_type = 'B';
		else
		    edge_type = 'F';
		break;
	    case FUT_PRESENT:
		if (chk_type_old == chk_type)
		    edge_type = 'A';
		else
		    edge_type = 'E';
		break;
	    case PRESENT:
		if (chk_type_old == chk_type)
		    edge_type = '@';
		else
		    edge_type = 'D';
		break;
	}
	if (edge_type != '@') {
	    if ((frm_pntr -> curr_place != -1) &&
		    (edge_type ==
			frm_pntr -> b_pntr[frm_pntr -> curr_place] -> edge) &&
		    (chk_type ==
			frm_pntr -> b_pntr[frm_pntr -> curr_place] -> ct) &&
		    (c_sr -> yb ==
			frm_pntr -> b_pntr[frm_pntr -> curr_place] -> yt) &&
		    (sr_pos == frm_pntr -> b_pntr[frm_pntr -> curr_place] -> x))
		update_edge (c_sr, frm_pntr);
	    else
		add_edge (c_sr, frm_pntr, edge_type, chk_type, sr_pos);
	}
	frm_pntr = frm_pntr -> next;
    }
}

static void update_edge (struct sr_field *c_sr, struct form *frm_pntr)
/* c_sr    - pointer to current sr */
/* frm_pntr - current form pointer */
{
/* This function updates an edge in the buffer of the
 * bolean files in the case of adjecent edge segments.
 */
    struct min_term *mt_pntr;	/* current min_term ptr	 */

    b_pntr = frm_pntr -> b_pntr[frm_pntr -> curr_place];

/* The top_value of the edge and the type of connection	 */
/* of the upperside of the edge are updated.		 */

    b_pntr -> yt = c_sr -> yt;
    mt_pntr = frm_pntr -> mt_pntr;
    while (mt_pntr) {
	if (((c_sr -> next -> mask_fut & mt_pntr -> mask) ==
		    mt_pntr -> mask) &&
		((~c_sr -> next -> mask_fut & mt_pntr -> not_mask) ==
		    mt_pntr -> not_mask)) {
	    b_pntr -> conn = b_pntr -> conn | 'B';
	    break;
	}
	mt_pntr = mt_pntr -> next;
    }
    if (mt_pntr == NULL) {
	b_pntr -> conn = b_pntr -> conn & ~('B' - '@');
    }
}

static void add_edge (struct sr_field *c_sr, struct form *frm_pntr, char edge_type, int chk_type, int sr_pos)
/* c_sr      - pointer to current sr */
/* frm_pntr  - current form pointer */
/* edge_type - type of the edge */
/* chk_type  - check_type of the edge */
/* sr_pos    - current sr position */
{
/* This function adds an edge to the buffers of a bolean file.
*/
    struct min_term *mt_pntr;	/* current min_term ptr	 */

    frm_pntr -> curr_place++;
    if (frm_pntr -> curr_place >= BUFLEN) {
	write_buf (frm_pntr, BUFLEN - 1);
	frm_pntr -> curr_place = 0;
    }
    b_pntr = frm_pntr -> b_pntr[frm_pntr -> curr_place];
    b_pntr -> x = sr_pos;
    b_pntr -> yb = c_sr -> yb;
    b_pntr -> yt = c_sr -> yt;
    b_pntr -> grp = NULL;
    b_pntr -> conn = '@';
    mt_pntr = frm_pntr -> mt_pntr;
    while (mt_pntr) {
	if (((c_sr -> prev -> mask_fut & mt_pntr -> mask) ==
		    mt_pntr -> mask) &&
		((~c_sr -> prev -> mask_fut & mt_pntr -> not_mask) ==
		    mt_pntr -> not_mask)) {
	    b_pntr -> conn = b_pntr -> conn | 'A';
	    break;
	}
	mt_pntr = mt_pntr -> next;
    }
    mt_pntr = frm_pntr -> mt_pntr;
    while (mt_pntr) {
	if (((c_sr -> next -> mask_fut & mt_pntr -> mask) ==
		    mt_pntr -> mask) &&
		((~c_sr -> next -> mask_fut & mt_pntr -> not_mask) ==
		    mt_pntr -> not_mask)) {
	    b_pntr -> conn = b_pntr -> conn | 'B';
	    break;
	}
	mt_pntr = mt_pntr -> next;
    }
    b_pntr -> ct = chk_type;
    b_pntr -> edge = edge_type;
}

void write_buf (struct form *frm_pntr, int nbr)
/* frm_pntr - current form pointer */
/* nbr      - last buffer to write */
{
/* this function adds the contents of the buffers up
 * till nbr to a temporarily file in a binairy form.
 * filename: TEMP_ONE.
 * the variable frm_pntr->curr_place is set to -1 to indicate
 * that the buffers belonging to the file are empty again.
 */
    int     i;			/* loop variable	 */
    FILE   *f_out;		/* file pointer		 */

    sprintf (fr_name, TEMP_ONE, frm_pntr -> f_nbr, pid);
    OPEN (f_out, fr_name, "a");
    for (i = 0; i <= nbr; i++) {
	b_pntr = frm_pntr -> b_pntr[i];
	fwrite ((char *) b_pntr, sizeof (*b_pntr), 1, f_out);
    }
    frm_pntr -> curr_place = -1;
    CLOSE (f_out);
}

static int det_chktype (struct sr_field *c_sr, unsigned int mask)
/* c_sr - current sr position */
/* mask - layers to treat */
{
/* This procedure determines the checktype of the layers given
 * in mask in the sr_field given by c_sr and returns its value.
 * If the checktypes are different zero is returned.
 */
    int     i;			/* loop variable	 */
    int     chk_type;		/* checktype returned	 */

    chk_type = INITIAL;
    for (i = 0; i < nomasks; i++) {
	if ((edges[i].mask & mask) != 0) {
	    if (chk_type == INITIAL)
		chk_type = c_sr -> p_chk -> chk_arr[i];
	    else
		if (c_sr -> p_chk -> chk_arr[i] != chk_type)
		    return (0);
	}
    }
    return (chk_type);
}

static int det_old_ct (struct sr_field *c_sr, unsigned int mask)
/* c_sr - current sr position */
/* mask - layers to treat */
{
/* This procedure determines, if a change of checktype
 * has taken place in one of the layers of mask, the old
 * checktype and returns its value.
 * If the checktypes of the layers in mask are different
 * zero is returned.
 */
    int     i;			/* loop variable	 */
    int     chk_type_old;	/* checktype returned	 */
    struct checktype   *p_chg_ct;/* ptr to chged chk_types */

    chk_type_old = INITIAL;
    p_chg_ct = c_sr -> p_chg_ct;
    while (p_chg_ct) {
	if ((p_chg_ct -> mask & mask) != 0) {
	    if (chk_type_old == INITIAL)
		chk_type_old = p_chg_ct -> chk_type;
	    else
		if (p_chg_ct -> chk_type != chk_type_old)
		    return (0);
	}
	p_chg_ct = p_chg_ct -> next;
    }
    if (c_sr -> chk_type != DIFF_CT) {
	if (c_sr -> chk_type != chk_type_old)
	    return (0);
    }
    else {
	mask = mask & c_sr -> ov_mask;
	for (i = 0; i < nomasks; i++) {
	    if ((edges[i].mask & mask) != 0) {
		if (c_sr -> p_chk -> chk_arr[i] != chk_type_old)
		    return (0);
	    }
	}
    }
    return (chk_type_old);
}
