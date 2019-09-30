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

static int det_gaplength (int x_sr, struct sr_field *c_sr);

void check_xgap (int x_sr, struct sr_field *c_sr)
/* x_sr - state_ruler position */
/* c_sr - pointer to sr_field */
{
/* This procedure checks the gap between two areas in
 * the x_direction.
 */
    if (((group_check == ON) && (c_sr -> group == c_sr -> group_old)) ||
	    ((c_sr -> chk_type == c_sr -> chk_type_old) &&
		(c_sr -> chk_type != 0)))
	return;
    if ((x_sr - c_sr -> xstart) >= gap)
	return;
    if ((exgapflag == 0) ||
	    ((x_sr - c_sr -> xstart) < exgap) ||
	    (det_gaplength (x_sr, c_sr) > exlength)) {
	if (nr_samples == 0) {
	    ERROR ("gap", c_sr -> xstart, c_sr -> yb, x_sr, c_sr -> yb);
	}
	else
	    add_err ("gap", c_sr -> xstart, c_sr -> yb, x_sr, c_sr -> yb, 0);
	return;
    }
}

static int det_gaplength (int x_sr, struct sr_field *c_sr)
/* x_sr - state_ruler position */
/* c_sr - pointer to sr_field */
{
/* This procedure determines the length of a gap which
 * is smaller then the admitted value gap.
 */
    struct sr_field *c_sr_down;
    struct sr_field *c_sr_up;

    c_sr_down = c_sr -> prev;
    c_sr_up = c_sr -> next;
    while (((c_sr_down -> lay_status & PRESENT) != 0) &&
	    ((x_sr - c_sr_down -> xstart) < gap)) {
	c_sr_down = c_sr_down -> prev;
    }
    while (((c_sr_up -> lay_status & PRESENT) != 0) &&
	    ((x_sr - c_sr_up -> xstart) < gap)) {
	c_sr_up = c_sr_up -> next;
    }
    return (c_sr_up -> yb - c_sr_down -> yt);
}

void check_g_circle (int x_sr, struct sr_field *c_sr, int direction)
/* c_sr - pointer to sr_field */
/* x_sr - state_ruler position */
/* direction - UP or DOWN */
{
/* This procedure checks for gap_errors at the corners of
 * the sr_field. If direction is UP the area left_above
 * the sr_field is checked; if direction is DOWN the area
 * left beneth the sr_field is checked.
 */
    int     y_centre;
    int     group;
    int     chk_type;

    group = c_sr -> group;
    chk_type = c_sr -> chk_type;

    if (direction == UP) {
	y_centre = c_sr -> yt;
	while (c_sr != h_sr -> prev) {
	    c_sr = c_sr -> next;
	    if (((c_sr -> yb - y_centre) > exgap) ||
		    ((group_check == ON) && (c_sr -> group_old == group)) ||
		    ((chk_type != 0) && (c_sr -> chk_type == chk_type)))
		return;
	    if(((c_sr -> lay_status == PRESENT) ||
	        (c_sr -> lay_status == CHG_TO_NOTPRESENT) ||
	        ((c_sr -> lay_status == CHG_TO_PRESENT) &&
                 ((group_check == OFF) || (c_sr -> group != group)))) &&
	        ((c_sr -> yb - y_centre) < exgap)) {
		if (nr_samples == 0) {
		    ERROR ("gap", x_sr, y_centre, x_sr, c_sr -> yb);
		}
		else
		    add_err ("gap", x_sr, y_centre, x_sr, c_sr -> yb, 0);
		return;
	    }
	    if (dig_circle (exgap, (c_sr -> yb - y_centre)) >
		    (x_sr - c_sr -> xstart)) {
		if (nr_samples == 0) {
		    ERROR ("gap", c_sr -> xstart, c_sr -> yb, x_sr, y_centre);
		}
		else
		    add_err ("gap", c_sr -> xstart, c_sr -> yb, x_sr, y_centre, 0);
		return;
	    }
	}
    }
    else {
	y_centre = c_sr -> yb;
	while (c_sr != h_sr) {
	    c_sr = c_sr -> prev;
	    if (((y_centre - c_sr -> yt) > exgap) ||
		    ((group_check == ON) && (c_sr -> group_old == group)) ||
		    ((chk_type != 0) && (c_sr -> chk_type == chk_type)))
		return;
	    if(((c_sr -> lay_status == PRESENT) ||
	        (c_sr -> lay_status == CHG_TO_NOTPRESENT) ||
	        ((c_sr -> lay_status == CHG_TO_PRESENT) &&
                 ((group_check == OFF) || (c_sr -> group != group)))) &&
	        ((y_centre - c_sr -> yt) < exgap)) {
		if (nr_samples == 0) {
		    ERROR ("gap", x_sr, c_sr -> yt, x_sr, y_centre);
		}
		else
		    add_err ("gap", x_sr, c_sr -> yt, x_sr, y_centre, 0);
		return;
	    }
	    if (dig_circle (exgap, (y_centre - c_sr -> yt)) >
		    (x_sr - c_sr -> xstart)) {
		if (nr_samples == 0) {
		    ERROR ("gap", c_sr -> xstart, c_sr -> yt, x_sr, y_centre);
		}
		else
		    add_err ("gap", c_sr -> xstart, c_sr -> yt, x_sr, y_centre, 0);
		return;
	    }
	}
    }
}

void check_w_circle (int x_sr, struct sr_field *c_sr, int direction)
/* c_sr - pointer to sr_field */
/* x_sr - state_ruler position */
/* direction - UP or DOWN */
{
/* This procedure checks for width_errors at the corners
 * of the sr_field.If direction is UP the area left_above
 * the sr_field is checked; if direction is DOWN the area
 * left beneth the sr_field is checked.
 */
    int     y_centre;
    int     x_ref;

    if (c_sr -> chk_type != 0)
	return;
    x_ref = c_sr -> xstart;
    if (direction == UP) {
	y_centre = c_sr -> yt;
	if (c_sr != h_sr -> prev) {
	    c_sr = c_sr -> next;
	    if ((c_sr -> xstart > x_ref) &&
		    ((x_sr - c_sr -> xstart) < width)) {
		if (nr_samples == 0) {
		    ERROR ("width", c_sr -> xstart, c_sr -> yb, x_sr, y_centre);
		}
		else
		    add_err ("width", c_sr -> xstart, c_sr -> yb, x_sr, y_centre, 0);
		return;
	    }
	}
	while (c_sr != h_sr -> prev) {
	    c_sr = c_sr -> next;
	    if ((c_sr -> lay_status != PRESENT) ||
		    ((c_sr -> yb - y_centre) > width))
		return;
	    if ((c_sr -> xstart > x_ref) &&
		    (dig_circle (width, (c_sr -> yb - y_centre)) >
			(x_sr - c_sr -> xstart))) {
		if (nr_samples == 0) {
		    ERROR ("width", c_sr -> xstart, c_sr -> yb, x_sr, y_centre);
		}
		else
		    add_err ("width", c_sr -> xstart, c_sr -> yb, x_sr, y_centre, 0);
		return;
	    }
	}
    }
    else {
	y_centre = c_sr -> yb;
	if (c_sr != h_sr) {
	    c_sr = c_sr -> prev;
	    if ((c_sr -> xstart > x_ref) &&
		    ((x_sr - c_sr -> xstart) < width)) {
		if (nr_samples == 0) {
		    ERROR ("width", c_sr -> xstart, c_sr -> yt, x_sr, y_centre);
		}
		else
		    add_err ("width", c_sr -> xstart, c_sr -> yt, x_sr, y_centre, 0);
		return;
	    }
	}
	while (c_sr != h_sr) {
	    c_sr = c_sr -> prev;
	    if ((c_sr -> lay_status != PRESENT) ||
		    ((y_centre - c_sr -> yt) > width))
		return;
	    if ((c_sr -> xstart > x_ref) &&
		    (dig_circle (width, (y_centre - c_sr -> yt)) >
			(x_sr - c_sr -> xstart))) {
		if (nr_samples == 0) {
		    ERROR ("width", c_sr -> xstart, c_sr -> yt, x_sr, y_centre);
		}
		else
		    add_err ("width", c_sr -> xstart, c_sr -> yt, x_sr, y_centre, 0);
		return;
	    }
	}
    }
}

void check_ygap (int x_sr, struct sr_field *c_sr, int direction)
/* c_sr - pointer to sr_field */
/* x_sr - state_ruler position */
/* direction - UP or DOWN */
{
/* This procedure checks a gap between the sr_field and
 * the first non_empty field laying above (direction is UP)
 * or laying beneth it (direction = DOWN).
 */
    struct sr_field *c_sr_tmp;

    c_sr_tmp = c_sr;
    if (direction == UP) {
	while (((c_sr_tmp -> lay_status & PRESENT) == 0) &&
		(c_sr_tmp != h_sr)) {
	    c_sr_tmp = c_sr_tmp -> next;
	}
	if (((c_sr_tmp -> prev -> yt - c_sr -> yb) >= gap) ||
		((group_check == ON) &&
		 (c_sr_tmp -> group == c_sr -> prev -> group)) ||
		((c_sr -> prev -> chk_type_old != 0) &&
		    (c_sr_tmp -> chk_type_old ==
			c_sr -> prev -> chk_type_old))) {
	    return;
	}
	if ((exgapflag == 0) ||
		((c_sr_tmp -> prev -> yt - c_sr -> yb) < exgap) ||
		(((x_sr - c_sr -> prev -> xstart) > exlength) &&
		    ((x_sr - c_sr_tmp -> xstart) > exlength))) {
	    if (nr_samples == 0) {
		ERROR ("gap", x_sr, c_sr -> yb, x_sr, c_sr_tmp -> yb);
	    }
	    else
		add_err ("gap", x_sr, c_sr -> yb, x_sr, c_sr_tmp -> yb, 0);
	    return;
	}
    }
    else {
	while (((c_sr_tmp -> lay_status & PRESENT) == 0) &&
		(c_sr_tmp != h_sr -> prev)) {
	    c_sr_tmp = c_sr_tmp -> prev;
	}
	if ((c_sr_tmp -> lay_status == CHG_TO_NOTPRESENT) ||
		((c_sr -> yt - c_sr_tmp -> next -> yb) >= gap) ||
		((group_check == ON) &&
		 (c_sr_tmp -> group == c_sr -> next -> group)) ||
		((c_sr -> next -> chk_type_old != 0) &&
		    (c_sr_tmp -> chk_type_old ==
			c_sr -> next -> chk_type_old))) {
	    return;
	}
	if ((exgapflag == 0) ||
		((c_sr -> yt - c_sr_tmp -> next -> yb) < exgap) ||
		(((x_sr - c_sr -> next -> xstart) > exlength) &&
		    ((x_sr - c_sr_tmp -> xstart) > exlength))) {
	    if (nr_samples == 0) {
		ERROR ("gap", x_sr, c_sr_tmp -> yt, x_sr, c_sr -> yt);
	    }
	    else
		add_err ("gap", x_sr, c_sr_tmp -> yt, x_sr, c_sr -> yt, 0);
	    return;
	}
    }
}

void check_xwidth (int x_sr, struct sr_field *c_sr)
/* c_sr - pointer to sr_field */
/* x_sr - state_ruler position */
{
/* This procedure checks the width of a sr_field in the
 * x_direction.
 */
    if (((x_sr - c_sr -> xstart) >= width) ||
	    (c_sr -> chk_type != 0)) {
	return;
    }
    if (c_sr -> next -> lay_status == NOT_PRESENT) {
	if (nr_samples == 0) {
	    ERROR ("width", c_sr -> xstart, c_sr -> yb, x_sr, c_sr -> yb);
	}
	else {
	    add_err ("width", c_sr -> xstart, c_sr -> yb, x_sr, c_sr -> yb, 0);
	    if (((c_sr -> yt - c_sr -> yb) < width) &&
		    (c_sr -> next -> xstart < c_sr -> xstart)) {
		add_err ("top", c_sr -> xstart, c_sr -> yt, x_sr, c_sr -> yt, 0);
	    }
	}
	return;
    }
    if (c_sr -> prev -> lay_status == NOT_PRESENT) {
	if (nr_samples == 0) {
	    ERROR ("width", c_sr -> xstart, c_sr -> yt, x_sr, c_sr -> yt);
	}
	else {
	    add_err ("width", c_sr -> xstart, c_sr -> yt, x_sr, c_sr -> yt, 0);
	    if (((c_sr -> yt - c_sr -> yb) < width) &&
		    (c_sr -> prev -> xstart < c_sr -> xstart)) {
		add_err ("bottom", c_sr -> xstart, c_sr -> yb, x_sr, c_sr -> yb, 0);
	    }
	}
	return;
    }
    if (nr_samples == 0) {
	ERROR ("width", c_sr -> xstart, c_sr -> yb, x_sr, c_sr -> yb);
    }
    else
	add_err ("width", c_sr -> xstart, c_sr -> yt, x_sr, c_sr -> yt, 0);
}

void check_ywidth (int x_sr, struct sr_field *c_sr, int chk_status, int direction)
/* c_sr - pointer to sr_field */
/* x_sr - state_ruler position */
/* chk_status - help status */
/* direction - UP or DOWN */
{
/* This procedure checks the width of an area starting
 * at the sr_field given. If direction is UP the field
 * given is taken as the bottom of the area; if direction
 * is DOWN it is taken as the top.
 */
    struct sr_field *c_sr_tmp;

    c_sr_tmp = c_sr;
    if (direction == UP) {
	while ((c_sr_tmp -> lay_status & chk_status) == chk_status) {
	    c_sr_tmp = c_sr_tmp -> next;
	}
	c_sr_tmp = c_sr_tmp -> prev;
	if (((c_sr_tmp -> yt - c_sr -> yb) >= width) ||
		(c_sr_tmp -> chk_type != 0)) {
	    return;
	}
	if (nr_samples == 0) {
	    ERROR ("width", x_sr, c_sr -> yb, x_sr, c_sr_tmp -> yt);
	}
	else {
	    add_err ("width", x_sr, c_sr -> yb, x_sr, c_sr_tmp -> yt, 0);
	}
	return;
    }
    else {
	while ((c_sr_tmp -> lay_status & chk_status) == chk_status) {
	    c_sr_tmp = c_sr_tmp -> prev;
	}
	c_sr_tmp = c_sr_tmp -> next;
	if (((c_sr -> yt - c_sr_tmp -> yb) >= width) ||
		(c_sr_tmp -> chk_type != 0)) {
	    return;
	}
	if (nr_samples == 0) {
	    ERROR ("width", x_sr, c_sr_tmp -> yb, x_sr, c_sr -> yt);
	}
	else {
	    add_err ("width", x_sr, c_sr_tmp -> yb, x_sr, c_sr -> yt, 0);
	}
	return;
    }
}

void check_max_xwidth (int x_sr, struct sr_field *c_sr)
/* c_sr - pointer to sr_field */
/* x_sr - stateruler position */
{
/* This procedure checks if the width of an item in the
 * x_direction is not too large.
 */
    if (((x_sr - c_sr -> xstart) <= maxwidth) ||
	    (c_sr -> chk_type != 0)) {
	return;
    }
    if (nr_samples == 0) {
	ERROR ("max_width", c_sr -> xstart, c_sr -> yb, x_sr, c_sr -> yb);
    }
    else
	add_err ("max_width", c_sr -> xstart, c_sr -> yb, x_sr, c_sr -> yb, c_sr -> group);
}

void check_max_ywidth (int x_sr, struct sr_field *c_sr)
/* c_sr - pointer to sr_field */
/* x_sr - stateruler position */
{
/* This procedure checks if the width of an item in the
 * y_direction is not too large.
 */
    struct sr_field *bottom, *top;
    int     dummy_ct;		/* tempory checktype */

    dummy_ct = c_sr -> chk_type;
    bottom = c_sr -> prev;
    top = c_sr -> next;
    while (bottom -> lay_status == PRESENT) {
	if (bottom -> chk_type != dummy_ct)
	    dummy_ct = 0;
	bottom = bottom -> prev;
    }
    if (bottom -> lay_status == CHG_TO_PRESENT)
	return;
    while ((top -> lay_status == PRESENT) ||
	    (top -> lay_status == CHG_TO_PRESENT)) {
	if (top -> chk_type != dummy_ct)
	    dummy_ct = 0;
	top = top -> next;
    }
    if (((top -> yb - bottom -> yt) <= maxwidth) ||
	    (dummy_ct != 0)) {
	return;
    }
    if (nr_samples == 0) {
	ERROR ("max_width", x_sr, bottom -> yt, x_sr, top -> yb);
    }
    else
	add_err ("max_width", x_sr, bottom -> yt, x_sr, top -> yb, c_sr -> group);
}

void check_right_stop (int sr_pos, struct sr_field *c_sr)
{
    struct sr_field *c_sr_tmp;

    if (c_sr -> prev -> lay_status != NOT_PRESENT)
	return;
    c_sr_tmp = c_sr;
    while (c_sr_tmp -> lay_status == CHG_TO_NOTPRESENT) {
	c_sr_tmp = c_sr_tmp -> next;
    }
    if ((c_sr_tmp -> lay_status == NOT_PRESENT) &&
	    ((c_sr_tmp -> yb - c_sr -> yb) < width)) {
	add_err ("right", sr_pos, c_sr -> yb, sr_pos, c_sr_tmp -> yb, 0);
    }
}

void check_left_start (int sr_pos, struct sr_field *c_sr)
{
    struct sr_field *c_sr_tmp;

    if (c_sr -> prev -> lay_status != NOT_PRESENT)
	return;
    c_sr_tmp = c_sr;
    while (c_sr_tmp -> lay_status == CHG_TO_PRESENT) {
	c_sr_tmp = c_sr_tmp -> next;
    }
    if ((c_sr_tmp -> lay_status == NOT_PRESENT) &&
	    ((c_sr_tmp -> yb - c_sr -> yb) < width)) {
	add_err ("left", sr_pos, c_sr -> yb, sr_pos, c_sr_tmp -> yb, 0);
    }
}

void check_start (int sr_pos, struct sr_field *c_sr)
{
    struct sr_field *c_sr_tmp;

    if (c_sr -> prev -> lay_status != NOT_PRESENT)
	return;
    c_sr_tmp = c_sr;
    while (c_sr_tmp -> lay_status == CHG_TO_PRESENT) {
	c_sr_tmp = c_sr_tmp -> next;
    }
    if (c_sr_tmp -> lay_status == NOT_PRESENT) {
	add_err ("max left", sr_pos, c_sr -> yb, sr_pos, c_sr_tmp -> yb, c_sr -> group);
    }
}

void check_stop (int sr_pos, struct sr_field *c_sr)
{
    struct sr_field *c_sr_tmp;

    if (c_sr -> prev -> lay_status != NOT_PRESENT)
	return;
    c_sr_tmp = c_sr;
    while (c_sr_tmp -> lay_status == CHG_TO_NOTPRESENT) {
	c_sr_tmp = c_sr_tmp -> next;
    }
    if (c_sr_tmp -> lay_status == NOT_PRESENT) {
	add_err ("max right", sr_pos, c_sr -> yb, sr_pos, c_sr_tmp -> yb, c_sr -> group);
    }
}
