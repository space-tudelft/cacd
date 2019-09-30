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

static int do_distance (struct f_edge *p, struct f_edge *q, struct f_edge *short_d);

/*
** Find minimum distance between two line pieces.
** INPUT: pointers to the edges.
** OUTPUT: the minimum distance between the edges.
*/
int distance (struct f_edge *p, struct f_edge *q, struct f_edge *sh_d)
{
    struct f_edge s1;
    int result1, result2;

    result1 = do_distance (p, q, sh_d);

    s1.xs = sh_d -> xs;
    s1.ys = sh_d -> ys;
    s1.xe = sh_d -> xe;
    s1.ye = sh_d -> ye;

    result2 = do_distance (q, p, sh_d);

    if (result1 < result2) {
	sh_d -> xs = s1.xs;
	sh_d -> ys = s1.ys;
	sh_d -> xe = s1.xe;
	sh_d -> ye = s1.ye;
	return (result1);
    }
    return (result2);
}

/*
** Calculate distance between two points.
** INPUT: the coordinats of the points.
** OUTPUT: the distance between the points.
*/
float dis (float x1, float y1, float x2, float y2)
{
    return (sqrt ((y2 - y1) * (y2 - y1) + (x2 - x1) * (x2 - x1)));
}

/*
** Calculate minimum distance from start and stop coordinats of p to edge q.
** INPUT: pointers to the edges p and q.
*/
static int do_distance (struct f_edge *p, struct f_edge *q, struct f_edge *short_d)
{
    float crossy, crossx, d1, d2, tmp1, tmp2;
    short legal;

    /*
    ** Find distance from ps to line q.
    */
    switch (q -> dir) {
    case -1:
	crossy = p -> ys - (p -> xs - q -> xs - q -> ys + p -> ys) / 2;
	break;
    case 0:
	crossy = p -> ys;
	break;
    case 1:
	crossy = p -> ys + (p -> xs - q -> xs + q -> ys - p -> ys) / 2;
	break;
    case 2:
    default:
	crossy = q -> ys;
	break;
    }

    /*
    ** Check if intersection crossy on line q.
    */
    legal = TRUE;
    if ((crossy < Min (q -> ys, q -> ye)) || (crossy > Max (q -> ys, q -> ye)))
	legal = FALSE;
    else
	if ((q -> dir == 2) && ((p -> xs < Min (q -> xs, q -> xe)) || (p -> xs > Max (q -> xs, q -> xe))))
	    legal = FALSE;

    if (!legal) {
	tmp1 = dis (p -> xs, p -> ys, q -> xs, q -> ys);
	tmp2 = dis (p -> xs, p -> ys, q -> xe, q -> ye);
	d1 = Min (tmp1, tmp2);

	short_d -> xs = p -> xs;
	short_d -> ys = p -> ys;

	if (tmp1 < tmp2) {
	    short_d -> xe = q -> xs;
	    short_d -> ye = q -> ys;
	}
	else {
	    short_d -> xe = q -> xe;
	    short_d -> ye = q -> ye;
	}
    }
    else {
	switch (q -> dir) {
	case -1:
	    crossx = q -> xs - crossy + q -> ys;
	    break;
	case 0:
	    crossx = q -> xs;
	    break;
	case 1:
	    crossx = q -> xs + crossy - q -> ys;
	    break;
	case 2:
	default:
	    crossx = p -> xs;
	    break;
	}
	d1 = dis (p -> xs, p -> ys, crossx, crossy);

	short_d -> xs = p -> xs;
	short_d -> ys = p -> ys;
	short_d -> xe = crossx;
	short_d -> ye = crossy;
    }

    /*
    ** Find distance from pe to line q.
    */
    switch (q -> dir) {
    case -1:
	crossy = p -> ye - (p -> xe - q -> xs - q -> ys + p -> ye) / 2;
	break;
    case 0:
	crossy = p -> ye;
	break;
    case 1:
	crossy = p -> ye + (p -> xe - q -> xs + q -> ys - p -> ye) / 2;
	break;
    case 2:
    default:
	crossy = q -> ys;
	break;
    }

    /*
    ** Check if intersection crossy on line q.
    */
    legal = TRUE;
    if ((crossy < Min (q -> ys, q -> ye)) || (crossy > Max (q -> ys, q -> ye)))
	legal = FALSE;
    else
	if ((q -> dir == 2) && ((p -> xe < Min (q -> xs, q -> xe)) || (p -> xe > Max (q -> xs, q -> xe))))
	    legal = FALSE;

    if (!legal) {
	tmp1 = dis (p -> xe, p -> ye, q -> xs, q -> ys);
	tmp2 = dis (p -> xe, p -> ye, q -> xe, q -> ye);
	d2 = Min (tmp1, tmp2);

	if (d2 < d1) {
	    short_d -> xs = p -> xe;
	    short_d -> ys = p -> ye;

	    if (tmp1 < tmp2) {
		short_d -> xe = q -> xs;
		short_d -> ye = q -> ys;
	    }
	    else {
		short_d -> xe = q -> xe;
		short_d -> ye = q -> ye;
	    }
	}
    }
    else {
	switch (q -> dir) {
	case -1:
	    crossx = q -> xs - crossy + q -> ys;
	    break;
	case 0:
	    crossx = q -> xs;
	    break;
	case 1:
	    crossx = q -> xs + crossy - q -> ys;
	    break;
	case 2:
	default:
	    crossx = p -> xe;
	    break;
	}
	d2 = dis (p -> xe, p -> ye, crossx, crossy);

	if (d2 < d1) {
	    short_d -> xs = p -> xe;
	    short_d -> ys = p -> ye;
	    short_d -> xe = crossx;
	    short_d -> ye = crossy;
	}
    }
    return ((int) (Min (d1, d2) + 0.01));
}
