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

#include "src/drc/dubcheck/dubcheck.h"

static void split_fld  (struct sr_field *c_sr, int ysplit);
static void update_fld (struct sr_field *c_sr, char edge_type, int group_no, int chk_type, int nbr_file);

void insert_edge (struct sr_field **r_srp, int yb, int yt,
	char edge_type, int group_no, int chk_type, int nbr_file)
/* r_srp    - ptr to first field of SR */
/* yb, yt   - bottom,top value of edge */
/* group_no - group_no no. of edge */
/* chk_type - check_type of the edge */
/* nbr_file - first or second file */
{
/* This procedure inserts an edge in the state_ruler.
 * It starts its search where to insert the edge in the
 * state_ruler at the first field of the reduced state_ruler.
 */
    register struct sr_field   *c_sr;/* curr.rsr-pointer */

    c_sr = (*r_srp);
    while (yb < c_sr -> yb) {
	c_sr = c_sr -> prev;
    }
    while (yb >= c_sr -> yt) {	/* while no overlap go   */
	c_sr = c_sr -> next;	/* the next field	 */
    }
    if (yb > c_sr -> yb) {	/* overlap: split field  */
	split_fld (c_sr, yb);	/* in the stateruler	 */
	c_sr = c_sr -> next;
    }
    while (yt >= c_sr -> yt) {	/* while overlap upd. fld */
	update_fld (c_sr, edge_type, group_no, chk_type, nbr_file);
	c_sr = c_sr -> next;
    }
    if (yt > c_sr -> yb) {	/* partly overlap: split */
	split_fld (c_sr, yt);	/* fld;upd.the lower part */
	update_fld (c_sr, edge_type, group_no, chk_type, nbr_file);
	c_sr = c_sr -> next;
    }
    (*r_srp) = c_sr;
}

static void split_fld (struct sr_field *c_sr, int ysplit)
/* ysplit - y_val of the split */
{
/* This procedure splits the state_ruler field pointed
 * to by c_sr into two fields: a bottom field reaching
 * from the bottom of the old field to ysplit, and a
 * top field reaching from ysplit to the top of the old
 * field. The contents of the two newly created fields
 * is the same as that of the old field.
 */
    struct sr_field *nf; /* ptr to new SR_field */

    ALLOC (nf, sr_field);
    nf -> xstart[0] = c_sr -> xstart[0];
    nf -> xstart[1] = c_sr -> xstart[1];
    nf -> yb = ysplit;
    nf -> yt = c_sr -> yt;
    nf -> lay_status[0] = c_sr -> lay_status[0];
    nf -> lay_status[1] = c_sr -> lay_status[1];
    nf -> helplay_status = c_sr -> helplay_status;
    nf -> group[0] = c_sr -> group[0];
    nf -> group[1] = c_sr -> group[1];
    nf -> chk_type[0] = c_sr -> chk_type[0];
    nf -> chk_type[1] = c_sr -> chk_type[1];
    nf -> next = c_sr -> next;
    nf -> prev = c_sr;
    nf -> next -> prev = nf;
    nf -> prev -> next = nf;
    c_sr -> yt = ysplit;
}

static void update_fld (struct sr_field *c_sr, char edge_type, int group_no, int chk_type, int nbr_file)
{
/* This procedure updates the field in the state_ruler
 * given by the pointer c_sr. According to the data of
 * the edge to add the data of the sr_field will be updated.
 */
    switch (edge_type) {
	case STOP_EDGE:
	    if (nbr_file == THIRD)
		c_sr -> helplay_status = CHG_TO_NOTPRESENT;
	    else
	        c_sr -> lay_status[nbr_file] = CHG_TO_NOTPRESENT;
	    break;
	case START_EDGE:
	case START_CHG_EDGE:
	    if (nbr_file == THIRD)
		c_sr -> helplay_status = CHG_TO_PRESENT;
	    else {
	        c_sr -> lay_status[nbr_file] = CHG_TO_PRESENT;
	        c_sr -> group[nbr_file] = group_no;
	        c_sr -> chk_type[nbr_file] = chk_type;
	    }
	    break;
	case STOP_CHG_EDGE:
	    if (nbr_file == THIRD)
		c_sr -> helplay_status = CHG_TO_NOTPRESENT;
	    else {
	        c_sr -> lay_status[nbr_file] = CHG_TO_NOTPRESENT;
	        c_sr -> chk_type[nbr_file] = chk_type;
	    }
	    break;
	case CHANGE_EDGE:
	    if (nbr_file != THIRD)
	        c_sr -> chk_type[nbr_file] = chk_type;
	    break;
	default:
	    error ("wrong edge_type");
	    break;
    }
}
