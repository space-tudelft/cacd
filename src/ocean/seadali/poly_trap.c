/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	Pieter van der Wolf
 *	Simon de Graaf
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

#include "src/ocean/seadali/header.h"

struct xl {
    struct xl *next;
    Coor   xval;
    short  rv;
};
static int  rv_l[MaxPolyPoints * 2];

static struct xl *insertx (struct xl *root, Coor val, int rv);
static struct ylist *make_ylist (struct ylist *root, Coor line[], int npoints);

/*
** Input parameters: array of polygon points,
** number of polygon points
** output: pointer to a linked list of trapezoids
**
** This program converts a polygon into a minimum set of
** non-overlapping trapezoids.  It scans the polygon in
** the y direction to find the y values where an event takes place.
** For each y value the polygon is scanned in x direction
** and trapezoids are reconstructed.
**
** INPUT: an array with polygon edge points and the number of edge points.
** OUTPUT: a list of non-overlapping trapezoids.
*/
struct obj_node * poly_trap (Coor line[], int npoints)
{
    struct obj_node *result, *trap;
    struct ylist *ycur, *yroot;
    struct xl *xroot, *p;
    Coor xm, xmin, xmax, x1, x2, interx, d;
    short lside, rside;
    int  i;

    xroot = NULL;
    yroot = NULL;
    result = NULL;

 /* make a list of all interesting y-values	 */
    yroot = make_ylist (yroot, line, npoints);

 /* scan in y-direction	 */
    for (ycur = yroot; ycur -> next; ycur = ycur -> next) {

	xroot = NULL;

    /* build the x-list	 */
	for (i = 0; i < npoints * 2 - 2; i = i + 2) {

	    if (rv_l[i / 2] != 2) {
		if ((ycur -> yval >= Min (line[i + 1], line[i + 3])) &&
			(ycur -> yval < Max (line[i + 1], line[i + 3]))) {

		    if (line[i + 1] < line[i + 3])
			xm = line[i];
		    else
			xm = line[i + 2];

		    interx = xm + rv_l[i / 2] * (ycur -> yval - Min (line[i + 1], line[i + 3]));
		    xroot = insertx (xroot, interx, rv_l[i / 2]);
		}
	    }
	}
	p = xroot;

    /* scan in x-direction	 */
	while (p) {
	    d = ycur -> next -> yval - ycur -> yval;

	    xmin = p -> xval;
	    x1 = xmin + p -> rv * d;
	    lside = p -> rv;

	    p = p -> next;
	    if (!p) {
		PE "error in poly_trap routine\n");
		for (trap = result; trap; trap = result) {
		    result = trap -> next;
		    FREE (trap);
		}
		return (NULL);
	    }
	    xmax = p -> xval;
	    x2 = xmax + p -> rv * d;
	    rside = p -> rv;

	    MALLOC (trap, struct obj_node);
	    trap -> ll_x1 = Min (xmin, x1);
	    trap -> ll_x2 = Max (xmax, x2);
	    trap -> ll_y1 = ycur -> yval;
	    trap -> ll_y2 = ycur -> next -> yval;
	    trap -> leftside = lside;
	    trap -> rightside = rside;
	    trap -> mark = 0;

	    if (!test_trap (trap)) {
		trap -> next = result;
		result = trap;
	    }
	    else {
		FREE (trap);
	    }
	    p = p -> next;
	}
	for (p = xroot; p; p = xroot) {
	    xroot = p -> next;
	    FREE (p);
	}
    }

    for (ycur = yroot; ycur; ycur = yroot) {
	yroot = ycur -> next;
	FREE (ycur);
    }
    return (result);
}

/*
** Insert an x value in x list.
** INPUT: x value and direction.
*/
static struct xl * insertx (struct xl *root, Coor val, int rv)
{
    struct xl *p, *prev, *new;

    prev = NULL;
    p = root;
    while (p && val > p -> xval) {
	prev = p;
	p = p -> next;
    }
    MALLOC (new, struct xl);
    new -> xval = val;
    new -> rv = rv;

    if (p) {
	if ((val == p -> xval) && (rv > p -> rv)) {
	    new -> next = p -> next;
	    p -> next = new;
	    return (root);
	}
    }
    new -> next = p;
    if (!prev)
	root = new;
    else
	prev -> next = new;
    return (root);
}

/*
** Make a list of y scan line values.
** We must calculate intersection points between line pieces.
*/
static struct ylist * make_ylist (struct ylist *root, Coor line[], int npoints)
{
    int  i, j;
    Coor px, py, qx, qy, crossy;

    for (i = 0; i < 2 * npoints - 2; i = i + 2) {
	root = yinsert (root, line[i + 1]);

	if (line[i] == line[i + 2])
	    rv_l[i / 2] = 0; /* vertical line   */
	else if (line[i + 1] == line[i + 3])
	    rv_l[i / 2] = 2; /* horizontal line */
	else if (line[i] > line[i + 2] && line[i + 1] < line[i + 3])
	    rv_l[i / 2] = -1;
	else if (line[i] < line[i + 2] && line[i + 1] > line[i + 3])
	    rv_l[i / 2] = -1;
	else
	    rv_l[i / 2] = 1;

	/* find intersection points between current line piece and all previous ones */

	for (j = 0; j < i; j = j + 2) {
	    if (rv_l[j / 2] != 2 && rv_l[i / 2] != 2 && rv_l[i / 2] != rv_l[j / 2]) {
		px = line[i];
		py = line[i + 1];
		qx = line[j];
		qy = line[j + 1];

		crossy = qy + (qx - px + rv_l[i / 2] * (py - qy)) / (rv_l[i / 2] - rv_l[j / 2]);
		if (crossy < Max (line[i + 1], line[i + 3]) &&
		    crossy > Min (line[i + 1], line[i + 3])) root = yinsert (root, crossy);
	    }
	}
    }
    return (root);
}

/*
** Convert trapezoid 'p' to a polygon. The result is
** stored in the array 'line'. The order of the corner-
** points in the resulting array is: ll, lr, ur, ul.
*/
int trap_to_poly (Coor line[], struct obj_node *p)
{
    /*
    ** Calculate the corner points of the trapezoid.
    */
    line[1] = p -> ll_y1;
    line[7] = p -> ll_y2;

    switch (p -> leftside) {
	case 0:
	    line[0] = line[6] = p -> ll_x1;
	    break;
	case 1:
	    line[0] = p -> ll_x1;
	    line[6] = p -> ll_x1 + (p -> ll_y2 - p -> ll_y1);
	    break;
	case -1:
	    line[0] = p -> ll_x1 + (p -> ll_y2 - p -> ll_y1);
	    line[6] = p -> ll_x1;
	    break;
	default:
	    /* illegal trapezoid. */
	    return (-1);
    }

    line[3] = p -> ll_y1;
    line[5] = p -> ll_y2;

    switch (p -> rightside) {
	case 0:
	    line[2] = line[4] = p -> ll_x2;
	    break;
	case 1:
	    line[2] = p -> ll_x2 - (p -> ll_y2 - p -> ll_y1);
	    line[4] = p -> ll_x2;
	    break;
	case -1:
	    line[2] = p -> ll_x2;
	    line[4] = p -> ll_x2 - (p -> ll_y2 - p -> ll_y1);
	    break;
	default:
	    /* illegal trapezoid. */
	    return (-1);
    }
    return (0);
}

#ifdef DRIVER
/*
** Testdriver for poly_trap().
** Also requires some routines from insert.c -->
** compilation requires some additional efforts.
*/
/*
Coor poly_line[] = {
    -55, 81,
    -55, 52,
    -20, 52,
    -20, 62,
    -46, 88,
   -115, 88,
   -115, 89,
   -125, 89,
   -125, 81,
    -55, 81
};

int numb_points = 10;
*/

Coor poly_line[] = {
     0, 0,
    10, 10,
     0, 10,
    10, 0,
     0, 0
};

int numb_points = 5;

/*
Coor poly_line[] = {
     0, 0,
    10, 10,
    10, 0,
     0, 10,
     0, 0
};

int numb_points = 5;
*/

main ()
{
    struct obj_node *p, *result_p_list;

    result_p_list = poly_trap (poly_line, numb_points);

    for (p = result_p_list; p; p = p -> next) {
	PE "x1 = %ld, x2 = %ld, y1 = %ld, y2 = %ld\n",
	    (long) p -> ll_x1, (long) p -> ll_x2, (long) p -> ll_y1, (long) p -> ll_y2);
	PE "\tlefts = %d, rights = %d, mark = %d, next = %s\n",
	    p -> leftside, p -> rightside, p -> mark, (p -> next) ? "no-null" : "null");
    }
}

err_meas (char *str, int num)
{
    PE "%s\n", str);
}

print_assert (char *filen, char *linen)
{
    PE "assertion failed: %s, %s\n", filen, linen);
}
#endif /* DRIVER */
