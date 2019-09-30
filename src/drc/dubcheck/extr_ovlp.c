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

static void check1_bottom (struct sr_field *c_sr, int sr_pos, int total);
static void check1_top (struct sr_field *c_sr, int sr_pos, int total);
static void check2_bottom (struct sr_field *c_sr, int sr_pos, int total);
static void check2_top (struct sr_field *c_sr, int sr_pos, int total);
#if 0
static void check3_bottom (struct sr_field *c_sr, int sr_pos, int total);
static void check3_top (struct sr_field *c_sr, int sr_pos, int total);
#endif
static void check4_bottom (struct sr_field *c_sr, int sr_pos, int total);
static void check4_top (struct sr_field *c_sr, int sr_pos, int total);

void extr_overlap (int sr_pos)
/* sr_pos - x_pos of the SR */
{
/* This is the routine for analysing the stateruler for
 * overlap errors of kind = 0.
 * (Kind = 0 means a total overlap).
 * It scans the SR and determines what checks have to be
 * carried out, according to the status of the layers in
 * the stateruler fields.
 */
    struct sr_field *c_sr;

    c_sr = h_sr;
    do {
	if (c_sr -> lay_status[0] == CHG_TO_PRESENT) {
	    if (c_sr -> chk_type[0] != 0) {
		c_sr = c_sr -> next;
		continue;
	    }
	    if (c_sr -> lay_status[1] != PRESENT) {
		error_meas ("overlap", sr_pos, c_sr -> yb, sr_pos, c_sr -> yt);
	    }
	    else
		if ((sr_pos - c_sr -> xstart[1]) < overlap) {
		    error_meas ("overlap", c_sr -> xstart[1],
			    c_sr -> yb, sr_pos, c_sr -> yb);
		}
	    if ((c_sr -> prev -> lay_status[0] == NOT_PRESENT) ||
		    (c_sr -> prev -> lay_status[0] == CHG_TO_NOTPRESENT))
		check1_bottom (c_sr, sr_pos, TRUE);
	    if ((c_sr -> next -> lay_status[0] == NOT_PRESENT) ||
		    (c_sr -> next -> lay_status[0] == CHG_TO_NOTPRESENT))
		check1_top (c_sr, sr_pos, TRUE);
	}
	if (c_sr -> lay_status[1] == CHG_TO_NOTPRESENT) {
	    if ((c_sr -> lay_status[0] != NOT_PRESENT) &&
		    (c_sr -> chk_type[0] == 0)) {
		error_meas ("overlap", sr_pos, c_sr -> yb, sr_pos, c_sr -> yt);
	    }
	    else
		if (((sr_pos - c_sr -> xstart[0]) < overlap)
			&&
			(c_sr -> chk_type[0] == 0)) {
		    error_meas ("overlap", c_sr -> xstart[0],
			    c_sr -> yb, sr_pos, c_sr -> yb);
		}
	    if (c_sr -> prev -> lay_status[1] == PRESENT)
		check2_bottom (c_sr, sr_pos, TRUE);
	    if (c_sr -> next -> lay_status[1] == PRESENT)
		check2_top (c_sr, sr_pos, TRUE);
	}
	if (c_sr -> lay_status[0] == CHG_TO_NOTPRESENT) {
	    if ((c_sr -> chk_type[0] != 0) || (nr_samples == 0)) {
		c_sr = c_sr -> next;
		continue;
	    }
	    if ((c_sr -> prev -> lay_status[0] == NOT_PRESENT) ||
		    (c_sr -> prev -> lay_status[0] == CHG_TO_NOTPRESENT))
		check1_bottom (c_sr, sr_pos, FALSE);
	    if ((c_sr -> next -> lay_status[0] == NOT_PRESENT) ||
		    (c_sr -> next -> lay_status[0] == CHG_TO_NOTPRESENT))
		check1_top (c_sr, sr_pos, FALSE);
	}
	c_sr = c_sr -> next;
    }
    while (c_sr != h_sr);
}

void extr_overlap1 (int sr_pos)
/* sr_pos - stateruler position */
{
/* This is the routine for analysing the stateruler for
 * overlap errors of kind = 1.
 * (Kind = 1 means an overlap to two opposite sides)
 * It scans te SR and determines what checks have to be
 * carried out, according to the status of the layers
 * in the SR_fields.
 */
    struct sr_field *c_sr;
    struct sr_field *c_sr_tmp;

    c_sr = h_sr;
    do {
	if (c_sr -> lay_status[0] == CHG_TO_PRESENT) {
	    if (c_sr -> chk_type[0] != 0) {
		c_sr = c_sr -> next;
		continue;
	    }
	    if (c_sr -> lay_status[1] == PRESENT) {
		if (sr_pos - c_sr -> xstart[1] < overlap) {
		    error_meas ("overlap", c_sr -> xstart[1],
			    c_sr -> yb, sr_pos, c_sr -> yb);
		}
	    }
	    if (c_sr -> lay_status[1] == CHG_TO_PRESENT) {
		if ((((c_sr -> prev -> lay_status[0] == PRESENT) ||
			(c_sr -> prev -> lay_status[0] == CHG_TO_PRESENT)) &&
			    (c_sr -> prev -> lay_status[1] == PRESENT)) ||
			(((c_sr -> next -> lay_status[0] == PRESENT) ||
			(c_sr -> next -> lay_status[0] == CHG_TO_PRESENT)) &&
			    (c_sr -> next -> lay_status[1] == PRESENT))) {
		    if (c_sr -> chk_type[0] == 0)
			error_meas ("overlap", sr_pos,
				c_sr -> yb, sr_pos, c_sr -> yt);
		}
		if (c_sr -> prev -> lay_status[0] == NOT_PRESENT) {
		    c_sr_tmp = c_sr -> prev;
		    while ((c_sr -> yb - c_sr_tmp -> yt) < overlap) {
			if ((c_sr_tmp -> lay_status[1] == NOT_PRESENT) ||
			    (c_sr_tmp -> lay_status[1] == CHG_TO_NOTPRESENT)) {
			    error_meas ("overlap", sr_pos,
				    c_sr_tmp -> yt, sr_pos, c_sr -> yb);
			    break;
			}
			c_sr_tmp = c_sr_tmp -> prev;
		    }
		}
		if (c_sr -> next -> lay_status[0] == NOT_PRESENT) {
		    c_sr_tmp = c_sr -> next;
		    while ((c_sr_tmp -> yb - c_sr -> yt) < overlap) {
			if ((c_sr_tmp -> lay_status[1] == NOT_PRESENT) ||
			    (c_sr_tmp -> lay_status[1] == CHG_TO_NOTPRESENT)) {
			    error_meas ("overlap", sr_pos,
				    c_sr -> yt, sr_pos, c_sr_tmp -> yb);
			    break;
			}
			c_sr_tmp = c_sr_tmp -> next;
		    }
		}
	    }
	}
	if (c_sr -> lay_status[1] == CHG_TO_NOTPRESENT) {
	    if (c_sr -> lay_status[0] == NOT_PRESENT) {
		if ((sr_pos - c_sr -> xstart[0]) < overlap) {
		    if (c_sr -> chk_type[0] == 0)
			error_meas ("overlap", c_sr -> xstart[0],
				c_sr -> yb, sr_pos, c_sr -> yb);
		}
		c_sr_tmp = c_sr -> prev;
		while (((c_sr_tmp -> yb != -MAXINT) &&
			    (c_sr -> yb - c_sr_tmp -> yt) < overlap)) {
		    if (c_sr_tmp -> lay_status[0] == PRESENT) {
			if (c_sr_tmp -> chk_type[0] == 0)
			    error_meas ("overlap", sr_pos,
				    c_sr_tmp -> yt, sr_pos, c_sr -> yb);
			break;
		    }
		    c_sr_tmp = c_sr_tmp -> prev;
		}
		c_sr_tmp = c_sr -> next;
		while (((c_sr_tmp -> yt != MAXINT) &&
			    (c_sr_tmp -> yb - c_sr -> yt) < overlap)) {
		    if (c_sr_tmp -> lay_status[0] == PRESENT) {
			if (c_sr_tmp -> chk_type[0] == 0)
			    error_meas ("overlap", sr_pos,
				    c_sr -> yt, sr_pos, c_sr_tmp -> yb);
			break;
		    }
		    c_sr_tmp = c_sr_tmp -> next;
		}
	    }
	    if (c_sr -> lay_status[0] == CHG_TO_NOTPRESENT) {
		if ((((c_sr -> prev -> lay_status[0] == PRESENT) ||
		    (c_sr -> prev -> lay_status[0] == CHG_TO_NOTPRESENT)) &&
			    (c_sr -> prev -> lay_status[1] == PRESENT)) ||
			(((c_sr -> next -> lay_status[0] == PRESENT) ||
			(c_sr -> next -> lay_status[0] == CHG_TO_NOTPRESENT)) &&
			    (c_sr -> next -> lay_status[1] == PRESENT))) {
		    if (c_sr -> chk_type[0] == 0)
			error_meas ("overlap", sr_pos,
				c_sr -> yb, sr_pos, c_sr -> yt);
		}
		if (c_sr -> prev -> lay_status[0] == NOT_PRESENT) {
		    c_sr_tmp = c_sr -> prev;
		    while ((c_sr -> yb - c_sr_tmp -> yt) < overlap) {
			if ((c_sr_tmp -> lay_status[1] == NOT_PRESENT) ||
				(c_sr_tmp -> lay_status[1] == CHG_TO_PRESENT)) {
			    if (c_sr -> chk_type[0] == 0)
				error_meas ("overlap", sr_pos,
					c_sr_tmp -> yt, sr_pos, c_sr -> yb);
			    break;
			}
			c_sr_tmp = c_sr_tmp -> prev;
		    }
		}
		if (c_sr -> next -> lay_status[0] == NOT_PRESENT) {
		    c_sr_tmp = c_sr -> next;
		    while ((c_sr_tmp -> yb - c_sr -> yt) < overlap) {
			if ((c_sr_tmp -> lay_status[1] == NOT_PRESENT) ||
				(c_sr_tmp -> lay_status[1] == CHG_TO_PRESENT)) {
			    if (c_sr -> chk_type[0] == 0)
				error_meas ("overlap", sr_pos,
					c_sr -> yt, sr_pos, c_sr_tmp -> yb);
			    break;
			}
			c_sr_tmp = c_sr_tmp -> next;
		    }
		}
	    }
	}
	c_sr = c_sr -> next;
    }
    while (c_sr != h_sr);
}

void extr_overlap2 (int sr_pos)
/* sr_pos - x_pos of the SR */
{
/* This is the routine for analysing the stateruler for
 * overlap errors of kind = 2.
 * Kind = 2 checks the overlap of the edges of the layer
 * to be overlapped and the overlapping layer, but only
 * in places where the helplay is not present.
 * At corners where at both sides the helplay is not
 * present, there is a test for a full_overlap; if the
 * helplay is present at one of the sides only the
 * overlap in the perpendicular direction is checked.
 */
    struct sr_field *c_sr;

    c_sr = h_sr;
    do {
	if (c_sr -> lay_status[0] == CHG_TO_PRESENT) {
	    if (c_sr -> chk_type[0] != 0) {
                c_sr = c_sr -> next;
                continue;
            }
	    if(c_sr -> helplay_status != PRESENT) {
	        if (c_sr -> lay_status[1] != PRESENT) {
		    error_meas ("overlap", sr_pos, c_sr -> yb,
			        sr_pos, c_sr -> yt);
	        }
		if (sr_pos - c_sr -> xstart[1] < overlap) {
		    error_meas ("overlap", c_sr -> xstart[1],
			    c_sr -> yb, sr_pos, c_sr -> yb);
		}
	    }
            if ((c_sr -> prev -> lay_status[0] == NOT_PRESENT) &&
		((c_sr -> prev -> helplay_status == NOT_PRESENT) ||
		 (c_sr -> prev -> helplay_status == CHG_TO_NOTPRESENT))) {
	        if(c_sr -> helplay_status == PRESENT)
                    check1_bottom(c_sr, sr_pos, FALSE);
		else
                    check1_bottom(c_sr, sr_pos, TRUE);
	    }
            if ((c_sr -> next -> lay_status[0] == NOT_PRESENT) &&
		((c_sr -> next -> helplay_status == NOT_PRESENT) ||
		 (c_sr -> next -> helplay_status == CHG_TO_NOTPRESENT))) {
	        if(c_sr -> helplay_status == PRESENT)
                    check1_top(c_sr, sr_pos, FALSE);
		else
                    check1_top(c_sr, sr_pos, TRUE);
	    }
        }
	if (c_sr -> lay_status[1] == CHG_TO_NOTPRESENT) {
	    if (c_sr -> chk_type[0] != 0) {
                c_sr = c_sr -> next;
                continue;
            }
	    if(c_sr -> helplay_status != PRESENT) {
	        if ((sr_pos - c_sr -> xstart[0]) < overlap) {
		    error_meas ("overlap", c_sr -> xstart[0],
			        c_sr -> yb, sr_pos, c_sr -> yb);
	        }
                if (c_sr -> prev -> lay_status[1] == PRESENT)
                    check2_bottom(c_sr, sr_pos, TRUE);
                if (c_sr -> next -> lay_status[1] == PRESENT)
                    check2_top(c_sr, sr_pos, TRUE);
            }
	    if((c_sr ->lay_status[0] == PRESENT) ||
	       (c_sr -> lay_status[0] == CHG_TO_NOTPRESENT)) {
		if((c_sr -> prev -> lay_status[0] == NOT_PRESENT) &&
		   (c_sr -> prev -> helplay_status != PRESENT)) {
                    check4_bottom(c_sr, sr_pos, FALSE);
		}
		if((c_sr -> next -> lay_status[0] == NOT_PRESENT) &&
		   (c_sr -> next -> helplay_status != PRESENT)) {
                    check4_top(c_sr, sr_pos, FALSE);
		}
	    }
	}
	c_sr = c_sr -> next;
    }
    while (c_sr != h_sr);
}

void extr_overlap3 (int sr_pos)
/* sr_pos - stateruler position */
{
/* This is the routine for analysing the stateruler for
 * overlap errors of kind = 3.
 * Kind = 3 checks the width of an overlap in accordance
 * with the direction given in the conn_dir array.
 */
    struct sr_field *c_sr, *c_sr_tmp;

    c_sr = h_sr;
    do {
	if (c_sr -> lay_status[0] == CHG_TO_PRESENT) {
	    if (c_sr -> chk_type[0] != 0) {
		c_sr = c_sr -> next;
		continue;
	    }
	    if(c_sr -> group[0] >= MAX_GROUP) {
		fprintf(stderr, "\ngroup_nbr too large; NOT CHECKED\n");
		c_sr = c_sr -> next;
		continue;
	    }
	    if((conn_dir[c_sr -> group[0]] & (BOTTOM + TOP)) ==
						  (BOTTOM + TOP)) {
		if(sr_pos - c_sr -> xstart[1] < overlap) {
		    error_meas("overlap", c_sr -> xstart[1], c_sr -> yb,
				sr_pos, c_sr -> yb);
		}
	    }
	    if((conn_dir[c_sr -> group[0]] & (LEFT + RIGHT)) ==
						   (LEFT + RIGHT)) {
		c_sr_tmp = c_sr -> prev;
		while(1) {
		    if(c_sr_tmp -> lay_status[1] != PRESENT) {
			error_meas("overlap", sr_pos, c_sr_tmp -> yt,
				    sr_pos, c_sr -> yb);
			break;
		    }
		    if((c_sr -> yb - c_sr_tmp -> yb) >= overlap) {
			break;
		    }
		    c_sr_tmp = c_sr_tmp -> prev;
		}
		c_sr_tmp = c_sr -> next;
		while(1) {
		    if(c_sr_tmp -> lay_status[1] != PRESENT) {
			error_meas("overlap", sr_pos, c_sr -> yt,
				    sr_pos, c_sr_tmp -> yb);
			break;
		    }
		    if((c_sr_tmp -> yt - c_sr -> yt) >= overlap) {
			break;
		    }
		    c_sr_tmp = c_sr_tmp -> next;
		}
	    }
	}
	if (c_sr -> lay_status[1] == CHG_TO_NOTPRESENT) {
	    if(sr_pos - c_sr -> xstart[0] < overlap) {
		if(c_sr -> group[0] >= MAX_GROUP) {
		    fprintf(stderr, "\ngroup_nbr too large; NOT CHECKED\n");
		    c_sr = c_sr -> next;
		    continue;
		}
		if((conn_dir[c_sr -> group[0]] & (BOTTOM + TOP)) ==
			                               (BOTTOM + TOP)) {
		    error_meas("overlap", c_sr -> xstart[0], c_sr -> yb,
				sr_pos, c_sr -> yb);
		}
	    }
	    c_sr_tmp = c_sr -> prev;
	    while(1) {
	        if(c_sr_tmp -> lay_status[0] != NOT_PRESENT) {
		    if(c_sr_tmp -> group[0] >= MAX_GROUP) {
		        fprintf(stderr, "\ngroup_nbr too large; NOT CHECKED\n");
			c_sr = c_sr -> next;
			continue;
		    }
		    if((conn_dir[c_sr_tmp -> group[0]] & (LEFT + RIGHT)) ==
			                               (LEFT + RIGHT)) {
			error_meas("overlap", sr_pos, c_sr_tmp -> yt,
				    sr_pos, c_sr -> yb);
			break;
		    }
		}
	        if((c_sr -> yb - c_sr_tmp -> yb) >= overlap) {
		    break;
		}
		c_sr_tmp = c_sr_tmp -> prev;
	    }
	    c_sr_tmp = c_sr -> next;
	    while(1) {
	        if(c_sr_tmp -> lay_status[0] != NOT_PRESENT) {
		    if(c_sr_tmp -> group[0] >= MAX_GROUP) {
		        fprintf(stderr, "\ngroup_nbr too large; NOT CHECKED\n");
			c_sr = c_sr -> next;
			continue;
		    }
		    if((conn_dir[c_sr_tmp -> group[0]] & (LEFT + RIGHT)) ==
			                               (LEFT + RIGHT)) {
			error_meas("overlap", sr_pos, c_sr -> yt,
				    sr_pos, c_sr_tmp -> yb);
			break;
		    }
		}
	        if((c_sr_tmp -> yt - c_sr -> yt) >= overlap) {
		    break;
		}
		c_sr_tmp = c_sr_tmp -> next;
	    }
	}
	c_sr = c_sr -> next;
    }
    while (c_sr != h_sr);
}

void extr_overlap6 (int sr_pos)
/* sr_pos - x_pos of the SR */
{
/* This is the routine for analysing the stateruler for
 * overlap errors of kind = 6.
 * Kind = 6 checks the overlap of the edges of the layer
 * to be overlapped and the overlapping layer, but only
 * in places where the helplay is not present.
 * It dffers from kind = 2 in that that in this test the
 * helplay does not need to overlap the layer to be overlapped
 * but nearly needs to touch it.
 */
    struct sr_field *c_sr;

    c_sr = h_sr;
    do {
	if (c_sr -> lay_status[0] == CHG_TO_PRESENT) {
	    if (c_sr -> chk_type[0] != 0) {
                c_sr = c_sr -> next;
                continue;
            }
	    if(c_sr -> helplay_status == NOT_PRESENT) {
	        if (c_sr -> lay_status[1] != PRESENT) {
		    error_meas ("overlap", sr_pos, c_sr -> yb,
			        sr_pos, c_sr -> yt);
	        }
		if (sr_pos - c_sr -> xstart[1] < overlap) {
		    error_meas ("overlap", c_sr -> xstart[1],
			    c_sr -> yb, sr_pos, c_sr -> yb);
		}
	    }
	    if((c_sr -> helplay_status == NOT_PRESENT) ||
	       (c_sr -> helplay_status == CHG_TO_NOTPRESENT)) {
                if ((c_sr -> prev -> lay_status[0] == NOT_PRESENT) &&
		    ((c_sr -> prev -> helplay_status == NOT_PRESENT) ||
		     (c_sr -> prev -> helplay_status == CHG_TO_NOTPRESENT))) {
                        check1_bottom(c_sr, sr_pos, TRUE);
	        }
                if ((c_sr -> next -> lay_status[0] == NOT_PRESENT) &&
		    ((c_sr -> next -> helplay_status == NOT_PRESENT) ||
		     (c_sr -> next -> helplay_status == CHG_TO_NOTPRESENT))) {
                        check1_top(c_sr, sr_pos, TRUE);
	        }
	    }
        }
	if (c_sr -> lay_status[0] == CHG_TO_NOTPRESENT) {
	    if((c_sr -> helplay_status == NOT_PRESENT) &&
	       (c_sr -> lay_status[1] != PRESENT)) {
		    error_meas ("overlap", sr_pos, c_sr -> yb,
			        sr_pos, c_sr -> yt);
	    }
	}
	if (c_sr -> lay_status[1] == CHG_TO_NOTPRESENT) {
	    if (c_sr -> chk_type[0] != 0) {
                c_sr = c_sr -> next;
                continue;
            }
	    if(c_sr -> lay_status[0] != NOT_PRESENT) {
	        if(c_sr -> helplay_status != NOT_PRESENT) {
                    c_sr = c_sr -> next;
		    continue;
		}
		else
		    error_meas ("overlap", sr_pos,
			    c_sr -> yb, sr_pos, c_sr -> yt);
	    }
	    else {
	        if(((c_sr -> helplay_status == NOT_PRESENT) ||
	          (c_sr -> helplay_status == CHG_TO_PRESENT)) &&
	          (c_sr -> helplay_xstart >= c_sr -> xstart[0])) {
                    c_sr = c_sr -> next;
		    continue;
		}
	        if(((c_sr -> helplay_status == PRESENT) ||
	          (c_sr -> helplay_status == CHG_TO_NOTPRESENT)) &&
	          (c_sr -> helplay_xstart <= c_sr -> xstart[0])) {
                    c_sr = c_sr -> next;
		    continue;
		}
	        if ((sr_pos - c_sr -> xstart[0]) < overlap) {
		    error_meas ("overlap", c_sr -> xstart[0],
			        c_sr -> yb, sr_pos, c_sr -> yb);
	        }
	    }
            if (c_sr -> prev -> lay_status[1] == PRESENT)
                check2_bottom(c_sr, sr_pos, TRUE);
            if (c_sr -> next -> lay_status[1] == PRESENT)
                check2_top(c_sr, sr_pos, TRUE);
        }
	if (c_sr -> lay_status[1] == CHG_TO_PRESENT) {
	    if((c_sr -> helplay_status == NOT_PRESENT) &&
	       (c_sr -> lay_status[0] == PRESENT)) {
		    error_meas ("overlap", sr_pos, c_sr -> yb,
			        sr_pos, c_sr -> yt);
	    }
	}
	if ((c_sr -> helplay_status == CHG_TO_NOTPRESENT) &&
	    (c_sr -> lay_status[0] == NOT_PRESENT)) {
	    if(c_sr -> prev -> lay_status[0] == PRESENT)
                check1_top(c_sr -> prev, sr_pos, FALSE);
	    if(c_sr -> next -> lay_status[0] == PRESENT)
                check1_bottom(c_sr -> next, sr_pos, FALSE);
	}
	c_sr = c_sr -> next;
    }
    while (c_sr != h_sr);
}

static void check1_bottom (struct sr_field *c_sr, int sr_pos, int total)
{
    int     bottom;
    bottom = c_sr -> yb;
    c_sr = c_sr -> prev;
    while (1) {
	if (total == FALSE) {
	    if ((c_sr -> lay_status[1] != PRESENT) &&
		    (c_sr -> lay_status[1] != CHG_TO_PRESENT)) {
		error_meas ("overlap", sr_pos, c_sr -> yt, sr_pos, bottom);
		return;
	    }
	}
	if (total == TRUE) {
	    if (c_sr -> lay_status[1] != PRESENT) {
		error_meas ("overlap", sr_pos, c_sr -> yt, sr_pos, bottom);
		return;
	    }
	    if (dig_circle (overlap, (bottom - c_sr -> yt)) >
		    (sr_pos - c_sr -> xstart[1])) {
		error_meas ("overlap", c_sr -> xstart[1],
			c_sr -> yt, sr_pos, bottom);
		return;
	    }
	}
	if ((bottom - c_sr -> yb) >= overlap)
	    return;
	c_sr = c_sr -> prev;
    }
}

static void check1_top (struct sr_field *c_sr, int sr_pos, int total)
{
    int     top;

    top = c_sr -> yt;
    c_sr = c_sr -> next;
    while (1) {
	if (total == FALSE) {
	    if ((c_sr -> lay_status[1] != PRESENT) &&
		    (c_sr -> lay_status[1] != CHG_TO_PRESENT)) {
		error_meas ("overlap", sr_pos, top, sr_pos, c_sr -> yb);
		return;
	    }
	}
	if (total == TRUE) {
	    if (c_sr -> lay_status[1] != PRESENT) {
		error_meas ("overlap", sr_pos, top, sr_pos, c_sr -> yb);
		return;
	    }
	    if (dig_circle (overlap, (c_sr -> yb - top)) >
		    (sr_pos - c_sr -> xstart[1])) {
		error_meas ("overlap", c_sr -> xstart[1], c_sr -> yb,
			sr_pos, top);
		return;
	    }
	}
	if ((c_sr -> yt - top) >= overlap)
	    return;
	c_sr = c_sr -> next;
    }
}

static void check2_bottom (struct sr_field *c_sr, int sr_pos, int total)
{
    int     bottom;

    bottom = c_sr -> yb;
    c_sr = c_sr -> prev;
    while (1) {
	if (total == FALSE) {
	    if ((c_sr -> lay_status[0] != NOT_PRESENT) &&
		    (c_sr -> lay_status[0] != CHG_TO_NOTPRESENT) &&
		    (c_sr -> chk_type[0] == 0)) {
		error_meas ("overlap", sr_pos, c_sr -> yt, sr_pos, bottom);
		return;
	    }
	}
	if (total == TRUE) {
	    if (c_sr -> lay_status[0] != NOT_PRESENT) {
		error_meas ("overlap", sr_pos, c_sr -> yt, sr_pos, bottom);
		return;
	    }
	    if ((dig_circle (overlap, (bottom - c_sr -> yt)) >
			(sr_pos - c_sr -> xstart[0])) &&
		    (c_sr -> chk_type[0] == 0)) {
		error_meas ("overlap",
			c_sr -> xstart[0], c_sr -> yt, sr_pos, bottom);
		return;
	    }
	}
	if ((bottom - c_sr -> yb) >= overlap)
	    return;
	c_sr = c_sr -> prev;
    }
}

static void check2_top (struct sr_field *c_sr, int sr_pos, int total)
{
    int     top;

    top = c_sr -> yt;
    c_sr = c_sr -> next;
    while (1) {
	if (total == FALSE) {
	    if ((c_sr -> lay_status[0] != NOT_PRESENT) &&
		    (c_sr -> lay_status[0] != CHG_TO_NOTPRESENT) &&
		    (c_sr -> chk_type[0] == 0)) {
		error_meas ("overlap", sr_pos, top, sr_pos, c_sr -> yb);
		return;
	    }
	}
	if (total == TRUE) {
	    if (c_sr -> lay_status[0] != NOT_PRESENT) {
		error_meas ("overlap", sr_pos, top, sr_pos, c_sr -> yb);
		return;
	    }
	    if ((dig_circle (overlap, (c_sr -> yb - top)) >
			(sr_pos - c_sr -> xstart[0])) &&
		    (c_sr -> chk_type[0] == 0)) {
		error_meas ("overlap", c_sr -> xstart[0],
			c_sr -> yb, sr_pos, top);
		return;
	    }
	}
	if ((c_sr -> yt - top) >= overlap)
	    return;
	c_sr = c_sr -> next;
    }
}

#if 0
static void check3_bottom (struct sr_field *c_sr, int sr_pos, int total)
{
    int     bottom;
    bottom = c_sr -> yb;
    c_sr = c_sr -> prev;
    while (1) {
	if (total == FALSE) {
	    if ((c_sr -> lay_status[1] != PRESENT) ||
		    (c_sr -> lay_status[0] != PRESENT)) {
		error_meas ("overlap", sr_pos, c_sr -> yt, sr_pos, bottom);
		return;
	    }
	}
	if (total == TRUE) {
	    if ((c_sr -> lay_status[1] != PRESENT) ||
		(c_sr -> lay_status[0] != PRESENT)) {
		error_meas ("overlap", sr_pos, c_sr -> yt, sr_pos, bottom);
		return;
	    }
	    if (dig_circle (overlap, (bottom - c_sr -> yt)) >
		    (sr_pos - c_sr -> xstart[1])) {
		error_meas ("overlap", c_sr -> xstart[1],
			c_sr -> yt, sr_pos, bottom);
		return;
	    }
	}
	if ((bottom - c_sr -> yb) >= overlap)
	    return;
	c_sr = c_sr -> prev;
    }
}

static void check3_top (struct sr_field *c_sr, int sr_pos, int total)
{
    int     top;

    top = c_sr -> yt;
    c_sr = c_sr -> next;
    while (1) {
	if (total == FALSE) {
	    if ((c_sr -> lay_status[1] != PRESENT) ||
		    (c_sr -> lay_status[0] != PRESENT)) {
		error_meas ("overlap", sr_pos, top, sr_pos, c_sr -> yb);
		return;
	    }
	}
	if (total == TRUE) {
	    if ((c_sr -> lay_status[1] != PRESENT) ||
	        (c_sr -> lay_status[0] != PRESENT)) {
		error_meas ("overlap", sr_pos, top, sr_pos, c_sr -> yb);
		return;
	    }
	    if (dig_circle (overlap, (c_sr -> yb - top)) >
		    (sr_pos - c_sr -> xstart[1])) {
		error_meas ("overlap", c_sr -> xstart[1], c_sr -> yb,
			sr_pos, top);
		return;
	    }
	}
	if ((c_sr -> yt - top) >= overlap)
	    return;
	c_sr = c_sr -> next;
    }
}
#endif

static void check4_bottom (struct sr_field *c_sr, int sr_pos, int total)
{
    int     bottom;

    bottom = c_sr -> yb;
    c_sr = c_sr -> prev;
    while (1) {
	if (total == FALSE) {
	    if ((c_sr -> lay_status[1] != PRESENT) &&
		    (c_sr -> lay_status[1] != CHG_TO_NOTPRESENT) &&
		    (c_sr -> chk_type[0] == 0)) {
		error_meas ("overlap", sr_pos, c_sr -> yt, sr_pos, bottom);
		return;
	    }
	}
	if (total == TRUE) {
	    if (c_sr -> lay_status[1] != NOT_PRESENT) {
		error_meas ("overlap", sr_pos, c_sr -> yt, sr_pos, bottom);
		return;
	    }
	    if ((dig_circle (overlap, (bottom - c_sr -> yt)) >
			(sr_pos - c_sr -> xstart[0])) &&
		    (c_sr -> chk_type[0] == 0)) {
		error_meas ("overlap", c_sr -> xstart[0], c_sr -> yt, sr_pos, bottom);
		return;
	    }
	}
	if ((bottom - c_sr -> yb) >= overlap)
	    return;
	c_sr = c_sr -> prev;
    }
}

static void check4_top (struct sr_field *c_sr, int sr_pos, int total)
{
    int     top;

    top = c_sr -> yt;
    c_sr = c_sr -> next;
    while (1) {
	if (total == FALSE) {
	    if ((c_sr -> lay_status[1] != PRESENT) &&
		    (c_sr -> lay_status[1] != CHG_TO_NOTPRESENT) &&
		    (c_sr -> chk_type[0] == 0)) {
		error_meas ("overlap", sr_pos, top, sr_pos, c_sr -> yb);
		return;
	    }
	}
	if (total == TRUE) {
	    if (c_sr -> lay_status[1] != PRESENT) {
		error_meas ("overlap", sr_pos, top, sr_pos, c_sr -> yb);
		return;
	    }
	    if (dig_circle (overlap, (c_sr -> yb - top)) >
		    (sr_pos - c_sr -> xstart[1]) &&
		    (c_sr -> chk_type[0] == 0)) {
		error_meas ("overlap", c_sr -> xstart[1], c_sr -> yb,
			sr_pos, top);
		return;
	    }
	}
	if ((c_sr -> yt - top) >= overlap)
	    return;
	c_sr = c_sr -> next;
    }
}
