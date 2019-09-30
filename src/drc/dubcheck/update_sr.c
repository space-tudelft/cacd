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

void update_sr (int x_sr, int xnew)
/* x_sr - state_ruler position */
/* xnew - next state_ruler position */
{
/* This procedure updates the state_ruler after it has
 * been analysed by extr_profile or extr_overlap, and
 * merges fields if possible.
 */
    register int dx1, dx2, dx3;
    register int dx_prev1, dx_prev2, dx_prev3;
    register struct sr_field *c_sr;
    struct sr_field *rm_pntr;

/* update fields */

    dx_prev1 = MAXINFLUENCE;
    dx_prev2 = MAXINFLUENCE;
    dx_prev3 = MAXINFLUENCE;
    c_sr = h_sr -> next;
    while (c_sr != h_sr) {
	if (c_sr -> lay_status[0] == CHG_TO_PRESENT) {
	    c_sr -> lay_status[0] = PRESENT;
	    c_sr -> xstart[0] = x_sr;
	}
	if (c_sr -> lay_status[0] == CHG_TO_NOTPRESENT) {
	    c_sr -> lay_status[0] = NOT_PRESENT;
	    c_sr -> xstart[0] = x_sr;
	}
	if (c_sr -> lay_status[1] == CHG_TO_PRESENT) {
	    c_sr -> lay_status[1] = PRESENT;
	    c_sr -> xstart[1] = x_sr;
	}
	if (c_sr -> lay_status[1] == CHG_TO_NOTPRESENT) {
	    c_sr -> lay_status[1] = NOT_PRESENT;
	    c_sr -> xstart[1] = x_sr;
	}
	if (c_sr -> helplay_status == CHG_TO_PRESENT) {
	    c_sr -> helplay_status = PRESENT;
	    c_sr -> helplay_xstart = x_sr;
	}
	if (c_sr -> helplay_status == CHG_TO_NOTPRESENT) {
	    c_sr -> helplay_status = NOT_PRESENT;
	    c_sr -> helplay_xstart = x_sr;
	}

/* merge fields */

	dx1 = MIN (xnew - c_sr -> xstart[0], MAXINFLUENCE);
	dx2 = MIN (xnew - c_sr -> xstart[1], MAXINFLUENCE);
	dx3 = MIN (xnew - c_sr -> helplay_xstart, MAXINFLUENCE);
	if ((dx_prev1 == dx1) && (dx_prev2 == dx2) && (dx_prev3 == dx3) &&
		(c_sr -> lay_status[0] == c_sr -> prev -> lay_status[0]) &&
		(c_sr -> lay_status[1] == c_sr -> prev -> lay_status[1]) &&
		(c_sr -> chk_type[0] == c_sr -> prev -> chk_type[0]) &&
		(c_sr -> chk_type[1] == c_sr -> prev -> chk_type[1]) &&
		(c_sr -> helplay_status == c_sr -> prev -> helplay_status)) {
	    c_sr -> prev -> yt = c_sr -> yt;
	    rm_pntr = c_sr;
	    c_sr -> prev -> next = c_sr -> next;
	    c_sr -> next -> prev = c_sr -> prev;
	    FREE (rm_pntr);
	    c_sr = c_sr -> prev;
	}
	dx_prev1 = dx1;
	dx_prev2 = dx2;
	dx_prev3 = dx3;
	c_sr = c_sr -> next;
    }
}
