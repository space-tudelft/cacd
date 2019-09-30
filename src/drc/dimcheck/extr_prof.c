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

#include "src/drc/dimcheck/dimcheck.h"

void extr_profile (int sr_pos)
/* sr_pos - x_position of the state_ruler */
{
/* This routine scans the state_ruler and determines
 * what checks have to be carried out, according to the
 * status of the layers in the stateruler fields and the
 * status of the width- and gap_flags.
 */
    struct sr_field *c_sr;

    c_sr = h_sr;
    do {
	if (c_sr -> lay_status == CHG_TO_PRESENT) {
	    if (maxwidthflag) {
		check_max_ywidth (sr_pos, c_sr);
		if (nr_samples != 0)
		    check_start (sr_pos, c_sr);
	    }
	    if (gapflag && (c_sr -> helplay_status == NOT_PRESENT))
		check_xgap (sr_pos, c_sr);
	    if ((c_sr -> prev -> lay_status & PRESENT) == 0) {
		if (gapflag && (c_sr -> prev -> helplay_status == NOT_PRESENT))
		    check_g_circle (sr_pos, c_sr, DOWN);
	    }
	    if ((c_sr -> next -> lay_status & PRESENT) == 0) {
		if (gapflag && (c_sr -> next -> helplay_status == NOT_PRESENT))
		    check_g_circle (sr_pos, c_sr, UP);
	    }
	    if (widthflag &&
		    ((c_sr -> prev -> lay_status & CHG_TO_PRESENT) != CHG_TO_PRESENT) &&
		    (c_sr -> next -> lay_status != PRESENT) &&
		    ((pvln[1] == NULL) ||
		    (((c_sr -> prev -> helplay_status == PRESENT) ||
			(c_sr -> prev -> helplay_status == CHG_TO_PRESENT)) &&
		    ((c_sr -> next -> helplay_status == PRESENT) ||
			(c_sr -> next -> helplay_status == CHG_TO_PRESENT))))) {
		check_ywidth (sr_pos, c_sr, CHG_TO_PRESENT, UP);
		if ((nr_samples != 0) &&
		    (c_sr ->helplay_status != PRESENT))
		    check_left_start (sr_pos, c_sr);
	    }
	    if (widthflag && (nr_samples != 0)) {
		if ((c_sr -> next -> lay_status == PRESENT) &&
			((pvln[1] == NULL) ||
			(c_sr -> next -> helplay_status == PRESENT) ||
			    (c_sr -> next -> helplay_status == CHG_TO_PRESENT)))
		    check_ywidth (sr_pos, c_sr -> next, PRESENT, UP);
		if ((c_sr -> prev -> lay_status == PRESENT) &&
			((pvln[1] == NULL) ||
			(c_sr -> prev -> helplay_status == PRESENT) ||
			    (c_sr -> prev -> helplay_status == CHG_TO_PRESENT)))
		    check_ywidth (sr_pos, c_sr -> prev, PRESENT, DOWN);
	    }

	}
	if (c_sr -> lay_status == CHG_TO_NOTPRESENT) {
	    if (maxwidthflag) {
		check_max_xwidth (sr_pos, c_sr);
		if (nr_samples != 0)
		    check_stop (sr_pos, c_sr);
	    }
	    if (widthflag &&
		    ((pvln[1] == NULL) ||
		    (c_sr -> helplay_status == PRESENT))) {
		check_xwidth (sr_pos, c_sr);
		if ((nr_samples != 0) &&
		    (c_sr -> helplay_status != PRESENT))
		    check_right_stop (sr_pos, c_sr);
	    }
	    if (c_sr -> prev -> lay_status != CHG_TO_NOTPRESENT) {
		if ((c_sr -> prev -> lay_status & PRESENT) != 0){
		    if (widthflag &&
		       ((pvln[1] == NULL) ||
			(c_sr -> helplay_status == PRESENT))) {
			check_ywidth (sr_pos, c_sr -> prev, PRESENT, DOWN);
			check_w_circle (sr_pos, c_sr, DOWN);
		    }
		}
		else {
		    if (gapflag &&
			    (c_sr -> prev -> helplay_status == NOT_PRESENT))
			check_ygap (sr_pos, c_sr -> prev, DOWN);
		}
	    }
	    if (c_sr -> next -> lay_status != CHG_TO_NOTPRESENT) {
		if ((c_sr -> next -> lay_status & PRESENT) != 0) {
		    if (widthflag &&
		       ((pvln[1] == NULL) ||
			(c_sr -> helplay_status == PRESENT))) {
			check_ywidth (sr_pos, c_sr -> next, PRESENT, UP);
			check_w_circle (sr_pos, c_sr, UP);
		    }
		}
		else {
		    if (gapflag &&
			    (c_sr -> next -> helplay_status == NOT_PRESENT))
			check_ygap (sr_pos, c_sr -> next, UP);
		}
	    }
	}
	c_sr = c_sr -> next;
    }
    while (c_sr != h_sr);
}
