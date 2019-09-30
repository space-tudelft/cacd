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

#include "src/drc/dubcheck/dubcheck.h"

static void set_grp_eq (struct sr_field *c_sr);

void extr_profile (int sr_pos)
/* sr_pos - x_pos of the SR */
{
/* This is the routine that analysis the SR for the
 * presence of gap_errors.
 * It scans the state_ruler and determines
 * what checks have to be carried out, according to
 * the status of the layers in the stateruler fields.
 */
    struct sr_field *c_sr;

    c_sr = h_sr;
    do {
	if (c_sr -> lay_status[0] == CHG_TO_PRESENT) {
	    if ((c_sr -> lay_status[1] == PRESENT) ||
		    (c_sr -> lay_status[1] == CHG_TO_PRESENT)) {
		set_grp_eq (c_sr);
		c_sr = c_sr -> next;
		continue;
	    }
	    if (c_sr -> lay_status[1] == CHG_TO_NOTPRESENT) {
                if((c_sr ->chk_type[0] == 0) ||
                   (c_sr ->chk_type[0] != c_sr -> chk_type[1]))
		    add_err (sr_pos, c_sr -> yb, sr_pos, c_sr -> yt,
			          c_sr -> group[0], c_sr -> group[1]);
		c_sr = c_sr -> next;
		continue;
	    }
	    check_xgap (sr_pos, c_sr, FIRST);
	    if (c_sr -> prev -> lay_status[0] == NOT_PRESENT) {
		check_ygap (sr_pos, c_sr -> prev, DOWN, FIRST);
		check_g_circle (sr_pos, c_sr, DOWN, FIRST);
	    }
	    if (c_sr -> next -> lay_status[0] == NOT_PRESENT) {
		check_ygap (sr_pos, c_sr -> next, UP, FIRST);
		check_g_circle (sr_pos, c_sr, UP, FIRST);
	    }
	}
	if (c_sr -> lay_status[1] == CHG_TO_PRESENT) {
	    if ((c_sr -> lay_status[0] == PRESENT) ||
		    (c_sr -> lay_status[0] == CHG_TO_PRESENT)) {
		set_grp_eq (c_sr);
		c_sr = c_sr -> next;
		continue;
	    }
	    if (c_sr -> lay_status[0] == CHG_TO_NOTPRESENT) {
                if((c_sr ->chk_type[0] == 0) ||
                   (c_sr ->chk_type[0] != c_sr -> chk_type[1]))
		    add_err (sr_pos, c_sr -> yb, sr_pos, c_sr -> yt,
			          c_sr -> group[0], c_sr -> group[1]);
		c_sr = c_sr -> next;
		continue;
	    }
	    check_xgap (sr_pos, c_sr, SECOND);
	    if (c_sr -> prev -> lay_status[1] == NOT_PRESENT) {
		check_ygap (sr_pos, c_sr -> prev, DOWN, SECOND);
		check_g_circle (sr_pos, c_sr, DOWN, SECOND);
	    }
	    if (c_sr -> next -> lay_status[1] == NOT_PRESENT) {
		check_ygap (sr_pos, c_sr -> next, UP, SECOND);
		check_g_circle (sr_pos, c_sr, UP, SECOND);
	    }
	}
	if (c_sr -> lay_status[0] == CHG_TO_NOTPRESENT) {
	    if ((c_sr -> lay_status[1] == PRESENT) ||
		    (c_sr -> lay_status[1] == CHG_TO_NOTPRESENT)) {
		set_grp_eq (c_sr);
		c_sr = c_sr -> next;
		continue;
	    }
	    if (c_sr -> prev -> lay_status[0] == NOT_PRESENT) {
		check_ygap (sr_pos, c_sr -> prev, DOWN, FIRST);
	    }
	    if (c_sr -> next -> lay_status[0] == NOT_PRESENT) {
		check_ygap (sr_pos, c_sr -> next, UP, FIRST);
	    }
	}
	if (c_sr -> lay_status[1] == CHG_TO_NOTPRESENT) {
	    if ((c_sr -> lay_status[0] == PRESENT) ||
		    (c_sr -> lay_status[0] == CHG_TO_NOTPRESENT)) {
		set_grp_eq (c_sr);
		c_sr = c_sr -> next;
		continue;
	    }
	    if (c_sr -> prev -> lay_status[1] == NOT_PRESENT) {
		check_ygap (sr_pos, c_sr -> prev, DOWN, SECOND);
	    }
	    if (c_sr -> next -> lay_status[1] == NOT_PRESENT) {
		check_ygap (sr_pos, c_sr -> next, UP, SECOND);
	    }
	}
	c_sr = c_sr -> next;
    }
    while (c_sr != h_sr);
}

static void set_grp_eq (struct sr_field *c_sr)
{
    unsigned int i, j, mask = GRP_BUFLEN - 1;
    struct group   *g_pntr;
    struct eq  *eq_pntr;
    struct eq  *e_pntr;

    i = c_sr -> group[0] & mask;
    j = c_sr -> group[0] / GRP_BUFLEN;
    if (j >= GRP_BUFLEN) {
	error ("too many groups");
    }
    if (group_arr[j] == NULL) {
	NALLOC (g_pntr, GRP_BUFLEN, struct group);
	group_arr[j] = g_pntr;
	ALLOC (eq_pntr, eq);
	eq_pntr -> eq_group = c_sr -> group[1];
	g_pntr[i].eq_pntr = eq_pntr;
    }
    else {
	g_pntr = group_arr[j];
	e_pntr = g_pntr[i].eq_pntr;
	while (e_pntr != NULL) {
	    if (c_sr -> group[1] == e_pntr -> eq_group)
		return;
	    e_pntr = e_pntr -> next;
	}
	ALLOC (eq_pntr, eq);
	eq_pntr -> eq_group = c_sr -> group[1];
	eq_pntr -> next = g_pntr[i].eq_pntr;
	g_pntr[i].eq_pntr = eq_pntr;
    }
}

void det_conn_hor (int sr_pos)
/* sr_pos - x_pos of the SR */
{
/* This procedure detects if an area is connected to an
 * area in another layer in a layer to the left and/or
 * the right. In doing so it fills the conn_dir array.
 */
    struct sr_field *c_sr;

    c_sr = h_sr;
    do {
	if((c_sr -> lay_status[1] == CHG_TO_NOTPRESENT) &&
	   (c_sr -> helplay_status == PRESENT)) {
	    c_sr -> lay_status[1] = PRESENT;
	}
	if(c_sr -> helplay_status == CHG_TO_NOTPRESENT) {
	    c_sr -> lay_status[0] = NOT_PRESENT;
	    c_sr -> lay_status[1] = NOT_PRESENT;
        }
	if((c_sr -> lay_status[0] == CHG_TO_PRESENT) &&
	   (c_sr-> lay_status[1] == PRESENT)) {
	    c_sr -> lay_status[1] = NOT_PRESENT;
	    if(c_sr -> group[0] < MAX_GROUP) {
		conn_dir[c_sr -> group[0]] = conn_dir[c_sr -> group[0]] | LEFT;
	    }
	}
	if(c_sr -> lay_status[0] == CHG_TO_NOTPRESENT) {
	    c_sr -> lay_status[0] = PRESENT;
	}
	if((c_sr -> lay_status[1] == CHG_TO_PRESENT) &&
	   (c_sr -> lay_status[0] == PRESENT)) {
	    c_sr -> lay_status[0] = NOT_PRESENT;
	    if(c_sr -> group[0] < MAX_GROUP) {
		conn_dir[c_sr -> group[0]] = conn_dir[c_sr -> group[0]] | RIGHT;
	    }
	}
	c_sr = c_sr -> next;
    }
    while (c_sr != h_sr);
}

void det_conn_ver (int sr_pos)
/* sr_pos - x_pos of the SR */
{
/* This procedure detects if an area is connected to an
 * area in another layer in a layer to the bottom and/or
 * the top. In doing so it fills the conn_dir array.
 */
    struct sr_field *c_sr, *c_sr_tmp;

    c_sr = h_sr;
    do {
	if((c_sr -> lay_status[0] == CHG_TO_PRESENT) ||
	   (c_sr -> lay_status[0] == CHG_TO_NOTPRESENT)) {
	    c_sr_tmp = c_sr -> prev;
	    while(c_sr_tmp -> helplay_status != NOT_PRESENT) {
		if(c_sr_tmp ->lay_status[1] != NOT_PRESENT) {
		    if(c_sr -> group[0] < MAX_GROUP) {
		        conn_dir[c_sr -> group[0]] =
			conn_dir[c_sr -> group[0]] | BOTTOM;
		    }
		    break;
		}
		c_sr_tmp = c_sr_tmp -> prev;
	    }
	    c_sr_tmp = c_sr -> next;
	    while(c_sr_tmp -> helplay_status != NOT_PRESENT) {
		if(c_sr_tmp ->lay_status[1] != NOT_PRESENT) {
		    if(c_sr -> group[0] < MAX_GROUP) {
		        conn_dir[c_sr -> group[0]] =
			conn_dir[c_sr -> group[0]] | TOP;
		    }
		    break;
		}
		c_sr_tmp = c_sr_tmp -> next;
	    }
	}
	if((c_sr -> lay_status[1] == CHG_TO_PRESENT) ||
	   (c_sr -> lay_status[1] == CHG_TO_NOTPRESENT)) {
	    c_sr_tmp = c_sr -> prev;
	    while(c_sr_tmp -> helplay_status != NOT_PRESENT) {
		if(c_sr_tmp ->lay_status[0] != NOT_PRESENT) {
		    if(c_sr_tmp -> group[0] < MAX_GROUP) {
		        conn_dir[c_sr_tmp -> group[0]] =
			conn_dir[c_sr_tmp -> group[0]] | TOP;
		    }
		    break;
		}
		c_sr_tmp = c_sr_tmp -> prev;
	    }
	    c_sr_tmp = c_sr -> next;
	    while(c_sr_tmp -> helplay_status != NOT_PRESENT) {
		if(c_sr_tmp ->lay_status[0] != NOT_PRESENT) {
		    if(c_sr_tmp -> group[0] < MAX_GROUP) {
		        conn_dir[c_sr_tmp -> group[0]] =
			conn_dir[c_sr_tmp -> group[0]] | BOTTOM;
		    }
		    break;
		}
		c_sr_tmp = c_sr_tmp -> next;
	    }
	}
	c_sr = c_sr -> next;
    }
    while (c_sr != h_sr);
}
