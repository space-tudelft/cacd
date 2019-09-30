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

static int det_gaplength (int x_sr, struct sr_field *c_sr, int i);

void check_xgap (int x_sr, struct sr_field *c_sr, int i)
/* x_sr - state_ruler position */
/* c_sr - pointer to sr_field */
/* i    - mask 1 or 2 */
{
/* This procedure checks the gap between two areas in
 * the x_direction.
 */
    int     j;
    j = (i + 1) & 1;
    if ((c_sr -> chk_type[0] == c_sr -> chk_type[1]) &&
	    (c_sr -> chk_type[0] != 0))
	return;
    if ((x_sr - c_sr -> xstart[j]) >= gap)
	return;
    if ((exgapflag == 0) ||
	    ((x_sr - c_sr -> xstart[j]) < exgap) ||
	    (det_gaplength (x_sr, c_sr, i) > exlength)) {
	add_err (c_sr -> xstart[j], c_sr -> yb, x_sr,
		c_sr -> yb, c_sr -> group[0], c_sr -> group[1]);
	return;
    }
}

static int det_gaplength (int x_sr, struct sr_field *c_sr, int i)
/* x_sr - state_ruler position */
/* c_sr - pointer to sr_field */
/* i    - mask 1 or 2 */
{
/* This procedure determines the length of a gap which
 * is smaller then the admitted value gap.
 */
    int     j;
    struct sr_field *c_sr_down;
    struct sr_field *c_sr_up;

    j = (i + 1) & 1;
    c_sr_down = c_sr -> prev;
    c_sr_up = c_sr -> next;
    while (((c_sr_down -> lay_status[i] & PRESENT) != 0) &&
	    ((x_sr - c_sr_down -> xstart[j]) < gap)) {
	c_sr_down = c_sr_down -> prev;
    }
    while (((c_sr_up -> lay_status[i] & PRESENT) != 0) &&
	    ((x_sr - c_sr_up -> xstart[j]) < gap)) {
	c_sr_up = c_sr_up -> next;
    }
    return (c_sr_up -> yb - c_sr_down -> yt);
}

void check_g_circle (int x_sr, struct sr_field *c_sr, int direction, int i)
/* c_sr      - pointer to sr_field */
/* direction - UP or DOWN */
/* x_sr      - state_ruler position */
/* i         - mask 1 or 2 */
{
/* This procedure checks for gap_errors at the corners of
 * the sr_field. If direction is UP the area left_above
 * the sr_field is checked; if direction is DOWN the area
 * left beneth the sr_field is checked.
 */
    int     y_centre;
    int     chk_type;
    int     group;
    int     j;

    j = (i + 1) & 1;
    chk_type = c_sr -> chk_type[i];
    group = c_sr -> group[i];

    if (direction == UP) {
	y_centre = c_sr -> yt;
	while (c_sr != h_sr -> prev) {
	    c_sr = c_sr -> next;
	    if (((c_sr -> yb - y_centre) > exgap) ||
		    ((chk_type != 0) && (c_sr -> chk_type[j] == chk_type)))
		return;
	    if (dig_circle (exgap, (c_sr -> yb - y_centre)) >
		    (x_sr - c_sr -> xstart[j])) {
                if(i == FIRST)
		    add_err (x_sr, y_centre, c_sr -> xstart[j],
			    c_sr -> yb, group, c_sr -> group[j]);
                else
		    add_err (x_sr, y_centre, c_sr -> xstart[j],
			    c_sr -> yb, c_sr -> group[j], group);
		return;
	    }
	}
    }
    else {
	y_centre = c_sr -> yb;
	while (c_sr != h_sr) {
	    c_sr = c_sr -> prev;
	    if (((y_centre - c_sr -> yt) > exgap) ||
		    ((chk_type != 0) && (c_sr -> chk_type[j] == chk_type)))
		return;
	    if (dig_circle (exgap, (y_centre - c_sr -> yt)) >
		    (x_sr - c_sr -> xstart[j])) {
                if(i == FIRST)
		    add_err (x_sr, y_centre, c_sr -> xstart[j],
			    c_sr -> yt, group, c_sr -> group[j]);
                else
		    add_err (x_sr, y_centre, c_sr -> xstart[j],
			    c_sr -> yt, c_sr -> group[j], group);
		return;
	    }
	}
    }
}

void check_ygap (int x_sr, struct sr_field *c_sr, int direction, int i)
/* c_sr      - pointer to sr_field */
/* direction - UP or DOWN */
/* x_sr      - state_ruler position */
/* i         - mask 1 or 2 */
{
/* This procedure checks a gap between the sr_field and
 * the first non_empty field laying above (direction = UP)
 * or laying beneth it (direction = DOWN).
 */
    struct sr_field *c_sr_tmp;
    int     j;

    j = (i + 1) & 1;
    c_sr_tmp = c_sr;
    if (direction == UP) {
	while ((c_sr_tmp -> lay_status[j] == 0) &&
		(c_sr_tmp != h_sr)) {
	    c_sr_tmp = c_sr_tmp -> next;
	}
	if (((c_sr_tmp -> prev -> yt - c_sr -> yb) >= gap) ||
		((c_sr -> prev -> chk_type[i] != 0) &&
		    (c_sr_tmp -> chk_type[j] ==
			c_sr -> prev -> chk_type[i]))) {
	    return;
	}
	if ((exgapflag == 0) ||
		((c_sr_tmp -> prev -> yt - c_sr -> yb) < exgap) ||
		(((x_sr - c_sr -> prev -> xstart[j]) > exlength) &&
		    ((x_sr - c_sr_tmp -> xstart[j]) > exlength))) {
	    if (i == 0)
		add_err (x_sr, c_sr -> yb, x_sr, c_sr_tmp -> yb,
			c_sr -> prev -> group[0],
			c_sr_tmp -> group[1]);
	    else
		add_err (x_sr, c_sr -> yb, x_sr, c_sr_tmp -> yb,
			c_sr_tmp -> group[0],
			c_sr -> prev -> group[1]);
	    return;
	}
    }
    else {
	while ((c_sr_tmp -> lay_status[j] == 0) &&
		(c_sr_tmp != h_sr -> prev)) {
	    c_sr_tmp = c_sr_tmp -> prev;
	}
	if (((c_sr -> yt - c_sr_tmp -> next -> yb) >= gap) ||
		((c_sr -> next -> chk_type[i] != 0) &&
		    (c_sr_tmp -> chk_type[j] ==
			c_sr -> next -> chk_type[i]))) {
	    return;
	}
	if ((exgapflag == 0) ||
		((c_sr -> yt - c_sr_tmp -> next -> yb) < exgap) ||
		(((x_sr - c_sr -> next -> xstart[j]) > exlength) &&
		    ((x_sr - c_sr_tmp -> xstart[j]) > exlength))) {
	    if (i == 0)
		add_err (x_sr, c_sr -> yt, x_sr, c_sr_tmp -> yt,
			c_sr -> next -> group[0],
			c_sr_tmp -> group[1]);
	    else
		add_err (x_sr, c_sr -> yt, x_sr, c_sr_tmp -> yt,
			c_sr_tmp -> group[0],
			c_sr -> next -> group[1]);
	    return;
	}
    }
}

void add_err (int x1, int y1, int x2, int y2, int group1, int group2)
/* x1, y1, x2, y2 - error coordinates */
/* group1, group2 - group in mask1, mask2 */
{
/* This procedure adds the errors found to a tempory file bufil.
*/
    buff.x1 = x1;
    buff.y1 = y1;
    buff.x2 = x2;
    buff.y2 = y2;
    buff.group1 = group1;
    buff.group2 = group2;
    fwrite ((char *) & buff, sizeof (buff), 1, bufil);
}
