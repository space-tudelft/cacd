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

#define LEFT	1
#define RIGHT	2
#define BOTTOM	3
#define TOP	4

#define INITIAL		 0
#define ERR_FOUND	-1
#define NO_ERR		 1

#define UNDETERMINED	-1

#define ROOT2	1.4142135623
#define PI	3.1415926535
extern int  test;

static int check_spec_err (struct chk_err *spec_err, int side);
static int det_err_status (int diff, int min_val, int max_val, int *bound1, int *bound2);
static void remove_from_list (struct chk_err *err);
static int same_err (struct chk_err *err1, struct chk_err *err2);
static void print_errlist (struct chk_err *head_err);

void filter_err ()
{
/* This routine filters the errors of the linked	 */
/* list made if nr_samples is not zero.			 */
/* Three filter actions are carried out:		 */
/* 1. Width_errors that are related to a special	 */
/*    error_edge (left, right, bottom or top) are 	 */
/*    suppressed.					 */
/* 2. Width_ and gap_errors that stemm from the same	 */
/*    slanted edges are taken together.			 */
/* 3. Max_Width errors are only reported if the errors	 */
/*    belonging to the same group do not all fit in a	 */
/*    circle with a radius of maxwidth * ROOT2/2.	 */

    struct chk_err *c_err, *prev_err, *ref_err, *clust_err,
                   *clust_add_p, *head_clust;
    int     err_status, group, nbr_errs_clust;
    int     x1_err, x2_err, y1_err, y2_err, x_ref, y_ref;
    int     xl_err, xr_err, yb_err, yt_err, xref_err, yref_err;
    double  cos_alfa, dx, dy, dist;
    float   alfa, phi;
    float   alfa_range_min, alfa_range_max;
    float   range_min, range_max;

/* first remove the width_errors that belong to the	 */
/* special error_edges and the special edges themselves	 */

    y_ref = 0; /* suppres uninitialized warning */

    if (test == ON) print_errlist (head_err);
    ref_err = head_err;
    prev_err = NULL;
 /* first look for special error_edges */

    while (ref_err) {
	if (strcmp (ref_err -> err_type, "left") == 0) {
	    if (check_spec_err (ref_err, LEFT) != NO_ERR) {
		ERROR ("width", ref_err -> x1, ref_err -> y1,
			ref_err -> x2, ref_err -> y2);
	    }
	    remove_from_list (ref_err);
	    ref_err = ref_err -> next;
	}
	else
	    if (strcmp (ref_err -> err_type, "right") == 0) {
		if (check_spec_err (ref_err, RIGHT) != NO_ERR) {
		    ERROR ("width", ref_err -> x1, ref_err -> y1,
			    ref_err -> x2, ref_err -> y2);
		}
		remove_from_list (ref_err);
		ref_err = ref_err -> next;
	    }
	    else
		if (strcmp (ref_err -> err_type, "bottom") == 0) {
		    if (check_spec_err (ref_err, BOTTOM) != NO_ERR) {
			ERROR ("width", ref_err -> x1, ref_err -> y1,
				ref_err -> x2, ref_err -> y2);
		    }
		    remove_from_list (ref_err);
		    ref_err = ref_err -> next;
		}
		else
		    if (strcmp (ref_err -> err_type, "top") == 0) {
			if (check_spec_err (ref_err, TOP) != NO_ERR) {
			    ERROR ("width", ref_err -> x1, ref_err -> y1,
				    ref_err -> x2, ref_err -> y2);
			}
			remove_from_list (ref_err);
			ref_err = ref_err -> next;
		    }
		    else {
			prev_err = ref_err;
			ref_err = ref_err -> next;
		    }
    }

 /* now cluster the maxwidth errors with respect to	 */
 /* their group and only report an error if the 	 */
 /* errors do not fit in a circle with radius	 */
 /* maxwidth * ROOT2/2.				 */

    err_status = INITIAL;
    head_clust = NULL;
    group = UNDETERMINED;
    range_min = -PI / 2.0;
    range_max = PI / 2.0;
    while (err_status != NO_ERR) {
	c_err = head_err;
	prev_err = NULL;
	while (c_err != NULL) {
	    if ((strncmp (c_err -> err_type, "max", 3) == 0) &&
		    ((group == UNDETERMINED) ||
			(c_err -> group == group))) {
		group = c_err -> group;
		if (prev_err != NULL) {
		    prev_err -> next = c_err -> next;
		    c_err -> next = head_clust;
		    head_clust = c_err;
		    c_err = prev_err -> next;
		}
		else {
		    head_err = head_err -> next;
		    c_err -> next = head_clust;
		    head_clust = c_err;
		    c_err = head_err;
		}
	    }
	    else {
		prev_err = c_err;
		c_err = c_err -> next;
	    }
	}
	if (head_clust == NULL) {
	    err_status = NO_ERR;
	}
	else {
	    x_ref = MAXINT;
	    clust_err = head_clust;
	    while (clust_err) {
		if (clust_err -> x1 < x_ref) {
		    x_ref = clust_err -> x1;
		    y_ref = clust_err -> y1;
		}
		if (clust_err -> x2 < x_ref) {
		    x_ref = clust_err -> x2;
		    y_ref = clust_err -> y2;
		}
		clust_err = clust_err -> next;
	    }
	    clust_err = head_clust;
	    while (clust_err) {
		if ((clust_err -> x1 != x_ref) ||
			(clust_err -> y1 != y_ref)) {
		    dx = (double) clust_err -> x1 - (double) x_ref;
		    dy = (double) clust_err -> y1 - (double) y_ref;
		    dist = sqrt (dx * dx + dy * dy);
		    if (dist > (double) (ROOT2 * maxwidth)) {
			err_status = ERR_FOUND;
			break;
		    }
		    else {
			phi = atan2 (dy, dx);
			cos_alfa = 0.5 * ROOT2 * dist / (double) maxwidth;
			alfa = acos (cos_alfa);
			alfa_range_min = phi - alfa;
			alfa_range_max = phi + alfa;
			if (((alfa_range_min - range_max) > 0.0001) ||
				((alfa_range_max - range_min) < -0.0001)) {
			    err_status = ERR_FOUND;
			    break;
			}
			else {
			    if (alfa_range_min > range_min)
				range_min = alfa_range_min;
			    if (alfa_range_max < range_max)
				range_max = alfa_range_max;
			}
		    }
		}
		if ((clust_err -> x2 != x_ref) ||
			(clust_err -> y2 != y_ref)) {
		    dx = (double) clust_err -> x2 - (double) x_ref;
		    dy = (double) clust_err -> y2 - (double) y_ref;
		    dist = sqrt (dx * dx + dy * dy);
		    if (dist > (double) (ROOT2 * maxwidth)) {
			err_status = ERR_FOUND;
			break;
		    }
		    else {
			phi = atan2 (dy, dx);
			cos_alfa = 0.5 * ROOT2 * dist / (double) maxwidth;
			alfa = acos (cos_alfa);
			alfa_range_min = phi - alfa;
			alfa_range_max = phi + alfa;
			if (((alfa_range_min - range_max) > 0.0001) ||
				((alfa_range_max - range_min) < -0.0001)) {
			    err_status = ERR_FOUND;
			    break;
			}
			else {
			    if (alfa_range_min > range_min)
				range_min = alfa_range_min;
			    if (alfa_range_max < range_max)
				range_max = alfa_range_max;
			}
		    }
		}
		clust_err = clust_err -> next;
	    }
	    err_status = ERR_FOUND;
	    if (err_status == ERR_FOUND) {
		clust_err = head_clust;
		xl_err = xr_err = yref_err = 0;
		yb_err = yt_err = xref_err = 0;
		while (clust_err) {
		    if (strcmp (clust_err -> err_type, "max_width") == 0) {
			if (ABS (clust_err -> x2 - clust_err -> x1) >
				(xr_err - xl_err)) {
			    xl_err = MIN (clust_err -> x1, clust_err -> x2);
			    xr_err = MAX (clust_err -> x1, clust_err -> x2);
			    yref_err = clust_err -> y1;
			}
			if (ABS (clust_err -> y2 - clust_err -> y1) >
				(yt_err - yb_err)) {
			    yb_err = MIN (clust_err -> y1, clust_err -> y2);
			    yt_err = MAX (clust_err -> y1, clust_err -> y2);
			    xref_err = clust_err -> x1;
			}
		    }
		    clust_err = clust_err -> next;
		}
		if (xl_err || xr_err) {
		    ERROR ("m_width", xl_err, yref_err, xr_err, yref_err);
		}
		if (yb_err || yt_err) {
		    ERROR ("m_width", xref_err, yb_err, xref_err, yt_err);
		}
	    }
	    free_errs (head_clust);
	    group = UNDETERMINED;
	    head_clust = NULL;
	}
    }
 /* at last cluster the remaining errors and report	 */
 /* the average of the clusters.			 */

    if (test == ON)
	print_errlist (head_err);
    while (head_err != NULL) {
	head_clust = head_err;
	head_err = head_err -> next;
	head_clust -> next = NULL;
	clust_err = head_clust;
	while (clust_err != NULL) {
	    c_err = head_err;
	    prev_err = NULL;
	    clust_add_p = clust_err;
	    while (c_err != NULL) {
		if (same_err (c_err, clust_err) == TRUE) {
		    if (prev_err == NULL) {
			head_err = c_err -> next;
			c_err -> next = NULL;
			clust_add_p -> next = c_err;
			clust_add_p = c_err;
			c_err = head_err;
		    }
		    else {
			prev_err -> next = c_err -> next;
			c_err -> next = NULL;
			clust_add_p -> next = c_err;
			clust_add_p = c_err;
			c_err = prev_err -> next;
		    }
		}
		else {
		    prev_err = c_err;
		    c_err = c_err -> next;
		}
	    }
	    clust_err = clust_err -> next;
	}
	clust_err = head_clust;
	nbr_errs_clust = 0;
	x1_err = 0;
	x2_err = 0;
	y1_err = 0;
	y2_err = 0;
	while (clust_err != NULL) {
	    x1_err = x1_err + clust_err -> x1;
	    x2_err = x2_err + clust_err -> x2;
	    y1_err = y1_err + clust_err -> y1;
	    y2_err = y2_err + clust_err -> y2;
	    nbr_errs_clust++;
	    clust_err = clust_err -> next;
	}
	x1_err = x1_err / nbr_errs_clust;
	x2_err = x2_err / nbr_errs_clust;
	y1_err = y1_err / nbr_errs_clust;
	y2_err = y2_err / nbr_errs_clust;
	ERROR (head_clust -> err_type, x1_err, y1_err, x2_err, y2_err);
	free_errs (head_clust);
    }
}

static void remove_from_list (struct chk_err *err)
{
    struct chk_err *prev_err;

    if (err == head_err) {
	head_err = err -> next;
	free (err);
    }
    else {
	prev_err = head_err;
	while (prev_err -> next != err) {
	    prev_err = prev_err -> next;
	}
	prev_err -> next = err -> next;
	free (err);
    }
}

static int check_spec_err (struct chk_err *spec_err, int side)
{
/* This procedure searches the errors that belong to
 * a special error and checks if the errors are real.
 */
    struct chk_err *c_err;
    int     xl_wdw, xr_wdw, yb_wdw, yt_wdw;
    int     err_status;
    int     min_dist;
    int     diff, min_val, max_val;
    int     bound1, bound2;

    switch (side) {
    case LEFT:
	xl_wdw = spec_err -> x1;
	xr_wdw = spec_err -> x1 + width / 2 + 1;
	bound1 = MIN (spec_err -> y1, spec_err -> y2);
	bound2 = MAX (spec_err -> y1, spec_err -> y2);
	yb_wdw = bound1 - width;
	yt_wdw = bound2 + width;
	break;
    case RIGHT:
	xl_wdw = spec_err -> x1 - width / 2 - 1;
	xr_wdw = spec_err -> x1;
	bound1 = MIN (spec_err -> y1, spec_err -> y2);
	bound2 = MAX (spec_err -> y1, spec_err -> y2);
	yb_wdw = bound1 - width;
	yt_wdw = bound2 + width;
	break;
    case BOTTOM:
	bound1 = MIN (spec_err -> x1, spec_err -> x2);
	bound2 = MAX (spec_err -> x1, spec_err -> x2);
	xl_wdw = bound1 - width;
	xr_wdw = bound2 + width;
	yb_wdw = spec_err -> y1;
	yt_wdw = spec_err -> y1 + width / 2 + 1;
	break;
    case TOP:
	bound1 = MIN (spec_err -> x1, spec_err -> x2);
	bound2 = MAX (spec_err -> x1, spec_err -> x2);
	xl_wdw = bound1 - width;
	xr_wdw = bound2 + width;
	yb_wdw = spec_err -> y1 - width / 2 - 1;
	yt_wdw = spec_err -> y1;
	break;
    default:
	bound1 = bound2 = 0;
	xl_wdw = xr_wdw = yb_wdw = yt_wdw = 0;
	fprintf (stderr, "check_spec_err: error: unexpected side\n");
	die (1);
    }
    err_status = INITIAL;
    min_dist = nr_samples;
    c_err = head_err;
    while (c_err) {
	if ((strcmp (c_err -> err_type, "width") == 0) &&
		(c_err -> x1 >= xl_wdw) &&
		(c_err -> x1 <= xr_wdw) &&
		(c_err -> x2 >= xl_wdw) &&
		(c_err -> x2 <= xr_wdw) &&
		(c_err -> y1 >= yb_wdw) &&
		(c_err -> y1 <= yt_wdw) &&
		(c_err -> y2 >= yb_wdw) &&
		(c_err -> y2 <= yt_wdw)) {
	    if ((side == LEFT) &&
	       (c_err -> x1 != spec_err -> x1)){
		min_dist = MIN(min_dist, (c_err -> x1 - spec_err -> x1));
	    }
	    if ((side == RIGHT) &&
	       (c_err -> x1 != spec_err -> x1)){
		min_dist = MIN(min_dist, (spec_err -> x1 - c_err -> x1));
	    }
	    if ((side == BOTTOM) &&
	       (c_err -> y1 != spec_err -> y1)){
		min_dist = MIN(min_dist, (c_err -> y1 - spec_err -> y1));
	    }
	    if ((side == TOP) &&
	       (c_err -> y1 != spec_err -> y1)){
		min_dist = MIN(min_dist, (spec_err -> y1 - c_err -> y1));
	    }
	    if (err_status != ERR_FOUND) {
		if (side == LEFT) {
		    diff = c_err -> x1 - spec_err -> x1;
		    min_val = MIN (c_err -> y1, c_err -> y2);
		    max_val = MAX (c_err -> y1, c_err -> y2);
		    err_status = det_err_status (diff, min_val, max_val, &bound1, &bound2);
		}
		if (side == RIGHT) {
		    diff = spec_err -> x1 - c_err -> x1;
		    min_val = MIN (c_err -> y1, c_err -> y2);
		    max_val = MAX (c_err -> y1, c_err -> y2);
		    err_status = det_err_status (diff, min_val, max_val, &bound1, &bound2);
		}
		if (side == BOTTOM) {
		    diff = c_err -> y1 - spec_err -> y1;
		    min_val = MIN (c_err -> x1, c_err -> x2);
		    max_val = MAX (c_err -> x1, c_err -> x2);
		    err_status = det_err_status (diff, min_val, max_val, &bound1, &bound2);
		}
		if (side == TOP) {
		    diff = spec_err -> y1 - c_err -> y1;
		    min_val = MIN (c_err -> x1, c_err -> x2);
		    max_val = MAX (c_err -> x1, c_err -> x2);
		    err_status = det_err_status (diff, min_val, max_val, &bound1, &bound2);
		}
	    }
	    remove_from_list (c_err);
	    c_err = c_err -> next;
	}
	else {
	    c_err = c_err -> next;
	}
    }
    if (min_dist == nr_samples) {
	err_status = ERR_FOUND;
    }
    return (err_status);
}

static int det_err_status (int diff, int min_val, int max_val, int *bound1, int *bound2)
{
    int err_status;

    if ((min_val > (*bound2 - diff + 1)) ||
	    (max_val < (*bound1 + diff - 1))) {
	err_status = ERR_FOUND;
    }
    else {
	err_status = NO_ERR;
	*bound1 = MAX (*bound1, (min_val + diff - 1));
	*bound2 = MIN (*bound2, (max_val - diff + 1));
    }
    return (err_status);
}

void add_err (char *str, int x1_err, int y1_err, int x2_err, int y2_err, int group)
{
    struct chk_err *err_next;

    err_next = head_err;
    ALLOC (head_err, chk_err);
    strcpy (head_err -> err_type, str);
    head_err -> x1 = x1_err;
    head_err -> x2 = x2_err;
    head_err -> y1 = y1_err;
    head_err -> y2 = y2_err;
    head_err -> group = group;
    head_err -> next = err_next;
}

static int same_err (struct chk_err *err1, struct chk_err *err2)
{
    if ((ABS (err1 -> x1 - err2 -> x1) < nr_samples) &&
	    (ABS (err1 -> y1 - err2 -> y1) < nr_samples) &&
	    (ABS (err1 -> x2 - err2 -> x2) < nr_samples) &&
	    (ABS (err1 -> y2 - err2 -> y2) < nr_samples))
	return (TRUE);
    else
	return (FALSE);
}

static void print_errlist (struct chk_err *head_err)
{
    struct chk_err *c_err;
    c_err = head_err;
    fprintf (pout, "\nerror_list:\n");
    while (c_err) {
	fprintf (pout, "\n%s\t%d\t%d\t%d\t%d\t%d",
		c_err -> err_type,
		c_err -> x1, c_err -> y1,
		c_err -> x2, c_err -> y2,
		c_err -> group);
	c_err = c_err -> next;
    }
    fprintf (pout, "\n");
}
