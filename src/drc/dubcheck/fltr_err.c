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

extern int  test;

static void print_errlist (struct chk_err *head_errlist);
static int same_err (struct chk_err *err1, struct chk_err *err2);

void filter_err ()
{
/* This routine filters the errors of the linked
 * list made if nr_samples is not zero.
 * The filter action carried out is:
 * Overlap_ and gap_errors that stemm from the same
 * slanted edges are taken together.
 */
    struct chk_err *c_err,
                   *prev_err,
                   *clust_err,
                   *clust_add_p,
                   *head_clust;
    int     nbr_errs_clust;
    int     x1_err, x2_err, y1_err, y2_err;

    if (test == ON)
	print_errlist (head_errlist);

 /* cluster the errors and report the average of the clusters */

    while (head_errlist != NULL) {
	head_clust = head_errlist;
	head_errlist = head_errlist -> next;
	head_clust -> next = NULL;
	clust_err = head_clust;
	while (clust_err != NULL) {
	    c_err = head_errlist;
	    prev_err = NULL;
	    clust_add_p = clust_err;
	    while (c_err != NULL) {
		if (same_err (c_err, clust_err) == TRUE) {
		    if (prev_err == NULL) {
			head_errlist = c_err -> next;
			c_err -> next = NULL;
			clust_add_p -> next = c_err;
			clust_add_p = c_err;
			c_err = head_errlist;
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

static void add_errlist (char *str, int x1_err, int y1_err, int x2_err, int y2_err)
{
    struct chk_err *err_next;

    err_next = head_errlist;
    ALLOC (head_errlist, chk_err);
    strcpy (head_errlist -> err_type, str);
    head_errlist -> x1 = x1_err;
    head_errlist -> x2 = x2_err;
    head_errlist -> y1 = y1_err;
    head_errlist -> y2 = y2_err;
    head_errlist -> next = err_next;
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

static void print_errlist (struct chk_err *head_errlist)
{
    struct chk_err *c_err;
    c_err = head_errlist;
    while (c_err != NULL) {
	fprintf (pout, "\n%s\t%d\t%d\t%d\t%d",
		c_err -> err_type,
		c_err -> x1, c_err -> y1,
		c_err -> x2, c_err -> y2);
	c_err = c_err -> next;
    }
    fprintf (pout, "\n");
}

void error_meas (char *str, int x1, int y1, int x2, int y2)
{
    if (nr_samples == 0) {
	ERROR (str, x1, y1, x2, y2);
    }
    else
	add_errlist (str, x1, y1, x2, y2);
}
