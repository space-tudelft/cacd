/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
 *	J. Annevelink
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

void main_bool ()
{
/* Initialize the nbool data structures.
 * Make the stateruler_profiles by repeatedly calling
 * select_edge and insert_edge; extract each
 * profile by calling extract_profile.
 */
    int     next_edge;		/* status of edge heap	 */
    struct sr_field *r_sr;	/* ptr to first fld of rsr */
    int     sr_pos;		/* state_ruler position	 */
    int     edge_pos,		/* x_value of the edge	 */
            yb,			/* bottom of the edge	 */
            yt,			/* top of the edge	 */
            chk_type;		/* check_type of the edge */
    unsigned    mask;		/* edge mask_code	 */
    char    edge_type;		/* type of the edge	 */
    struct form *frm_pntr;	/* pointer to the forms	 */

    /* initialize state ruler */
    h_sr = &head_sr;
    h_sr -> chk_type = INITIAL;
    h_sr -> yt = MAXINT;
    h_sr -> yb = -MAXINT;
    h_sr -> next = h_sr -> prev = &head_sr;

    next_edge = select_edge (&edge_pos, &yb, &yt, &edge_type, &chk_type, &mask);
    while (next_edge) {
	sr_pos = edge_pos;
	r_sr = h_sr;
	do {
	    insert_edge (&r_sr, yb, yt, edge_type, chk_type, mask);
	    next_edge = select_edge (&edge_pos, &yb, &yt, &edge_type, &chk_type, &mask);
	} while (next_edge && edge_pos == sr_pos);

	if (chk_flag == CHK_HRCHY) check_hierarchy (sr_pos);
	extr_profile (sr_pos);
	update_sr ();
    }

    /* now the last remaining edges are put on the files */

    for (frm_pntr = fp_head; frm_pntr; frm_pntr = frm_pntr -> next) {
	if (frm_pntr -> curr_place != -1) {
	    write_buf (frm_pntr, frm_pntr -> curr_place);
	}
    }
}
