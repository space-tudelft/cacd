/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	P. van der Wolf
 *	H.T. Fassotte
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

#include "src/dali/header.h"

struct xl {
    struct xl *next;
    Coor   xval;
    short  rv;
};
static int rv_l[MaxPoints * 2];

/*
** Insert an x value in x list.
** INPUT: x value and direction.
*/
static struct xl * insertx (struct xl *root, Coor val, int rv)
{
    register struct xl *p, *prev, *new;

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
	if (val == p -> xval && rv > p -> rv) {
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
static struct ylist * make_ylist (struct ylist *root, Coor line_x[], Coor line_y[], int npoints)
{
    register int i, j;
    Coor a, yc, px, py, pymin, pymax;

    for (i = 0; i < npoints; ++i) {
	py = line_y[i];
	root = yinsert (root, py);

	pymax = line_y[i + 1];
	if (py == pymax) {
	    rv_l[i] = 2; /* horizontal line */
	    continue;
	}

	px = line_x[i];
	if (px == line_x[i + 1])
	    rv_l[i] = 0; /* vertical line */
	else if (px > line_x[i + 1] && py < pymax)
	    rv_l[i] = -1;
	else if (px < line_x[i + 1] && py > pymax)
	    rv_l[i] = -1;
	else
	    rv_l[i] = 1;

	/* If (line(i,i+1) is not a horizontal line)
	** find intersection points between current line piece
	** and all previous ones:
	*/
	if (pymax < py) {
	    pymin = pymax; pymax = py;
	}
	else pymin = py;
	for (j = 0; j < i; ++j) {
	    if (rv_l[j] != 2 && (a = rv_l[i] - rv_l[j])) {
		yc = line_y[j];
		yc += (line_x[j] - px + rv_l[i] * (py - yc)) / a;
		if (yc < pymax && yc > pymin) root = yinsert (root, yc);
	    }
	}
    }
    return (root);
}

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
struct obj_node * poly_trap (Coor line_x[], Coor line_y[], int npoints)
{
    struct obj_node *outlist;
    register struct obj_node *p;
    register struct xl    *xroot, *xcur;
    register struct ylist *yroot, *ynext;
    Coor xmin, xmax, interx, yc, ymin, ymax, yn, dx, dy;
    short ls, rs, sides;
    register int i;

    outlist = NULL;
    /*
    ** make a list of all interesting y-values
    */
    yroot = make_ylist (NULL, line_x, line_y, --npoints);
    if (!yroot) return (outlist);

    xroot = NULL;
    /*
    ** scan in y-direction
    */
    yn = yroot -> yval;
    ynext = yroot -> next;
    FREE (yroot);
    while ((yroot = ynext)) {
	yc = yn;
	yn = yroot -> yval;
	ynext = yroot -> next;
	FREE (yroot);
	dy = yn - yc;

	/*
	** build the x-list
	*/
	for (i = 0; i < npoints; ++i) {
	    if (rv_l[i] != 2) {
		ymin = line_y[i];
		ymax = line_y[i + 1];
		if (ymax < ymin) {
		    dx = ymin; ymin = ymax; ymax = dx;
		}
		if (yc >= ymin && yc < ymax) {
		    if (line_y[i] < line_y[i + 1])
			interx = line_x[i];
		    else
			interx = line_x[i + 1];
		    interx += rv_l[i] * (yc - ymin);
		    xroot = insertx (xroot, interx, rv_l[i]);
		}
	    }
	}

	/*
	** scan in x-direction
	*/
	while ((xcur = xroot)) {

	    xmin = xcur -> xval;
	    if ((ls = xcur -> rv) < 0) xmin += ls * dy;
	    xroot = xcur -> next;
	    FREE (xcur);

	    if (!(xcur = xroot)) {
		btext ("error in poly_trap routine!");
		while ((p = outlist)) {
		    outlist = p -> next;
		    FREE (p);
		}
		goto ret;
	    }

	    xmax = xcur -> xval;
	    if ((rs = xcur -> rv) > 0) xmax += rs * dy;
	    xroot = xcur -> next;
	    FREE (xcur);

	    /* test_trap */
	    if ((dx = xmax - xmin) <= 0) continue;
	    if (dx == dy && ls && rs) continue;

	    if (rs < 0) rs = 2;
	    if (ls < 0) ls = 2;
	    sides = rs + (ls << 2);

	    /*
	    ** merge trapezoid with other trapezoids in outlist vertically
	    */
	    for (p = outlist; p; p = p -> next) {

		if (p -> ll_y2 != yc || p -> sides != sides) continue;

		dx = p -> ll_y2 - p -> ll_y1;

		switch (ls) {
		case 0:
		    if (p -> ll_x1 != xmin) continue;
		    break;
		case 1:
		    if (p -> ll_x1 + dx != xmin) continue;
		    break;
		case 2:
		    if (p -> ll_x1 != xmin + dy) continue;
		}
		switch (rs) {
		case 0:
		    if (p -> ll_x2 != xmax) continue;
		    break;
		case 1:
		    if (p -> ll_x2 != xmax - dy) continue;
		    break;
		case 2:
		    if (p -> ll_x2 - dx != xmax) continue;
		}
		/*
		** trapezoid can be merged
		*/
		p -> ll_y2 = yn;
		if (ls == 2) p -> ll_x1 = xmin;
		if (rs == 1) p -> ll_x2 = xmax;
		break;
	    }
	    if (!p) { /* not merged */
		MALLOC (p, struct obj_node);
		p -> ll_x1 = xmin;
		p -> ll_x2 = xmax;
		p -> ll_y1 = yc;
		p -> ll_y2 = yn;
		p -> sides = sides;
		p -> next = outlist;
		outlist = p;
	    }
	}
    }
ret:
    while ((yroot = ynext)) {
	ynext = yroot -> next;
	FREE (yroot);
    }
    return (outlist);
}

/*
** Convert trapezoid 'p' to a polygon. The result is
** stored in the array 'line'. The order of the corner-
** points in the resulting array is: ll, lr, ur, ul.
*/
int trap_to_poly (Coor line[], struct obj_node *p)
{
    Coor dy;
    /*
    ** Calculate the corner points of the trapezoid.
    */
    line[3] = line[1] = p -> ll_y1;
    line[7] = line[5] = p -> ll_y2;
    dy = line[5] - line[1];

    switch (p -> sides >> 2) { /* leftside */
    case 0:
	line[0] = line[6] = p -> ll_x1;
	break;
    case 1:
	line[0] = p -> ll_x1;
	line[6] = p -> ll_x1 + dy;
	break;
    case 2:
	line[0] = p -> ll_x1 + dy;
	line[6] = p -> ll_x1;
	break;
    default:
	return (0); /* illegal trapezoid */
    }

    switch (p -> sides & 3) { /* rightside */
    case 0:
	line[2] = line[4] = p -> ll_x2;
	break;
    case 1:
	line[2] = p -> ll_x2 - dy;
	line[4] = p -> ll_x2;
	break;
    case 2:
	line[2] = p -> ll_x2;
	line[4] = p -> ll_x2 - dy;
	break;
    default:
	return (0); /* illegal trapezoid */
    }
    return (1); /* OK */
}

#ifdef DRIVER
/*
** Testdriver for poly_trap ().
** Also requires some routines from insert.c -->
** compilation requires some additional efforts.
*/
/*
Coor poly_x[] = { -55, -55, -20, -20, -46, -115, -115, -125, -125, -55 };
Coor poly_y[] = {  81,  52,  52,  62,  88,   88,   89,   89,   81,  81 };
int numb_points = 10;
*/

Coor poly_x[] = { 0, 10,  0, 10, 0 };
Coor poly_y[] = { 0, 10, 10,  0, 0 };
int numb_points = 5;

/*
Coor poly_x[] = { 0, 10, 10,  0, 0 };
Coor poly_y[] = { 0, 10,  0, 10, 0 };
int numb_points = 5;
*/

main ()
{
    struct obj_node *p, *result_p_list;

    result_p_list = poly_trap (poly_x, poly_y, numb_points);

    for (p = result_p_list; p; p = p -> next) {
	PE "x1 = %ld, x2 = %ld, y1 = %ld, y2 = %ld\n",
	    (long) p -> ll_x1, (long) p -> ll_x2, (long) p -> ll_y1, (long) p -> ll_y2);
	PE "\tls = %d, rs = %d, mark = %d, next = %s\n",
	    p -> sides >> 2, p -> sides & 3, p -> mark, (p -> next) ? "no-null" : "null");
    }
}

void err_meas (char *str)
{
    PE "%s\n", str);
}

void print_assert (char *filen, char *linen)
{
    PE "assertion failed: %s, %s\n", filen, linen);
}
#endif /* DRIVER */
