/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
 *	J. Liedorp
 *	T.G.R. van Leuken
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

#include "src/drc/dimcheck/dimcheck.h"

void update_sr (int x_sr, int xnew)
/* x_sr - state_ruler position */
/* xnew - next state_ruler position */
{
/* This procedure updates the state_ruler after it has been
 * analysed by extr_profile, and merges fields if possible.
 */
    register int    dx;
    register int    dx_prev;
    register struct sr_field   *c_sr;
    struct sr_field *rm_pntr;

/* update fields */

    dx_prev = MAXINFLUENCE;
    c_sr = h_sr -> next;
    while (c_sr != h_sr) {
	if (c_sr -> lay_status == CHG_TO_PRESENT) {
	    c_sr -> lay_status = PRESENT;
	    c_sr -> xstart = x_sr;
	    c_sr -> group_old = c_sr -> group;
	}
	else
	    if (c_sr -> lay_status == CHG_TO_NOTPRESENT) {
		c_sr -> lay_status = NOT_PRESENT;
	        c_sr -> group_old = c_sr -> group;
	        c_sr -> group = 0;
		c_sr -> xstart = x_sr;
	    }
	c_sr -> chk_type_old = c_sr -> chk_type;
	if (c_sr -> helplay_status == CHG_TO_PRESENT)
	    c_sr -> helplay_status = PRESENT;
	if (c_sr -> helplay_status == CHG_TO_NOTPRESENT)
	    c_sr -> helplay_status = NOT_PRESENT;

/* merge fields */

	dx = MIN (xnew - c_sr -> xstart, MAXINFLUENCE);
	if ((dx_prev == dx) &&
		(c_sr -> lay_status == c_sr -> prev -> lay_status) &&
		(c_sr -> helplay_status == c_sr -> prev -> helplay_status) &&
		(c_sr -> chk_type == c_sr -> prev -> chk_type)) {
	    c_sr -> prev -> yt = c_sr -> yt;
	    rm_pntr = c_sr;
	    c_sr -> prev -> next = c_sr -> next;
	    c_sr -> next -> prev = c_sr -> prev;
	    c_sr = c_sr -> prev;
	    free (rm_pntr);
	}
	dx_prev = dx;
	c_sr = c_sr -> next;
    }
}
