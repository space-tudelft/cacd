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

/*
** Add a trapezoid to "list".
** INPUT: the trapezoid's attributes.
*/
static struct obj_node * out (Coor x1, Coor x2, Coor x3, Coor x4, Coor ymin, Coor ymax, struct obj_node *list)
{
    register struct obj_node *p;
    Coor xmin, xmax, dx, dy;
    short ls, rs, sides;

    xmin = Min (x1, x2);
    xmax = Max (x3, x4);

    /* test_trap */
    if ((dx = xmax - xmin) <= 0 || (dy = ymax - ymin) <= 0) return (list);
    if (dx == dy && x1 != x2 && x3 != x4) return (list);

    if (x1 == x2) ls = 0;
    else ls = (x1 < x2) ? 1 : 2;
    if (x3 == x4) rs = 0;
    else rs = (x3 < x4) ? 1 : 2;
    sides = rs + (ls << 2);

    /*
    ** Merge trapezoid vertically with elements in list if possible.
    */
    for (p = list; p; p = p -> next) {

	if (p -> sides != sides ||
	   (p -> ll_y2 != ymin && p -> ll_y1 != ymax)) continue;

	dx = p -> ll_y2 - p -> ll_y1;

	switch (ls) {
	case 0:
	    if (p -> ll_x1 != xmin) continue;
	    break;
	case 1:
	    if (p -> ll_y2 == ymin) {
		if (p -> ll_x1 + dx != xmin) continue;
	    }
	    else
		if (p -> ll_x1 != xmin + dy) continue;
	    break;
	case 2:
	    if (p -> ll_y2 == ymin) {
		if (p -> ll_x1 != xmin + dy) continue;
	    }
	    else
		if (p -> ll_x1 + dx != xmin) continue;
	}
	switch (rs) {
	case 0:
	    if (p -> ll_x2 != xmax) continue;
	    break;
	case 1:
	    if (p -> ll_y2 == ymin) {
		if (p -> ll_x2 != xmax - dy) continue;
	    }
	    else
		if (p -> ll_x2 - dx != xmax) continue;
	    break;
	case 2:
	    if (p -> ll_y2 == ymin) {
		if (p -> ll_x2 - dx != xmax) continue;
	    }
	    else
		if (p -> ll_x2 != xmax - dy) continue;
	}
	/*
	** trapezoid can be merged
	*/
	if (p -> ll_y2 == ymin) {
	    p -> ll_y2 = ymax;
	    if (ls == 2) p -> ll_x1 = xmin;
	    if (rs == 1) p -> ll_x2 = xmax;
	}
	else {
	    p -> ll_y1 = ymin;
	    if (ls == 1) p -> ll_x1 = xmin;
	    if (rs == 2) p -> ll_x2 = xmax;
	}
	return (list);
    }
    /*
    ** Insert trapezoid into the list:
    */
    MALLOC (p, struct obj_node);
    p -> ll_x1 = xmin;
    p -> ll_x2 = xmax;
    p -> ll_y1 = ymin;
    p -> ll_y2 = ymax;
    p -> sides = sides;
    p -> next = list;
    return (list = p);
}

/*
** Sort a[l] to a[r].
*/
static void quicksort (Coor a[], int l, int r)
{
    register int i = l;
    register int j = r;
    Coor x = a[(l + r) / 2], w;

    do {
	while (a[i] < x) ++i;
	while (a[j] > x) --j;
	if (i > j) break;
	w = a[i];
	a[i] = a[j];
	a[j] = w;
    } while (++i <= --j);

    if (l < j) quicksort (a, l, j);
    if (i < r) quicksort (a, i, r);
}

/*
** Find those parts of p outside q.
** INPUT: pointers to trapezoids p and q and  pointer to
** a list of trapezoids to which the result must be added.
** OUTPUT: a pointer to the list of resulting trapezoids.
*/
struct obj_node * clip (struct obj_node *p, struct obj_node *q, struct obj_node *clip_head)
{
    Coor a[8], crossy, max_y1, min_y2;
    Coor pl, pr, ql, qr, y, yold;
    Coor lxmin, lxmax, loldxmin, loldxmax;
    Coor rxmin, rxmax, roldxmin, roldxmax;
    register int count, n;
    int  pls, prs, qls, qrs;

    pls = p -> sides;
    if ((prs = pls & 3) > 1) prs = -1;
    if ((pls >>= 2)     > 1) pls = -1;

    if (q -> ll_y1 >= p -> ll_y2 || q -> ll_y2 <= p -> ll_y1 ||
	q -> ll_x1 >= p -> ll_x2 || q -> ll_x2 <= p -> ll_x1) {
	/*
	** no intersection: p lies completely outside q: return p
	*/
	y = p -> ll_y2 - p -> ll_y1;
	loldxmin = lxmin = p -> ll_x1;
	if (pls < 0)   loldxmin += y;
	else if (pls > 0) lxmin += y;
	roldxmax = rxmax = p -> ll_x2;
	if (prs > 0)   roldxmax -= y;
	else if (prs < 0) rxmax -= y;
	return (out (loldxmin, lxmin, roldxmax, rxmax, p -> ll_y1, p -> ll_y2, clip_head));
    }

    /*
    ** find y scanline values
    */
    a[0] = max_y1 = p -> ll_y1;
    a[1] = min_y2 = p -> ll_y2;
    count = 1;

    if (q -> ll_y1 > p -> ll_y1) a[++count] = max_y1 = q -> ll_y1;
    if (q -> ll_y2 < p -> ll_y2) a[++count] = min_y2 = q -> ll_y2;

    qls = q -> sides;
    if ((qrs = qls & 3) > 1) qrs = -1;
    if ((qls >>= 2)     > 1) qls = -1;

    pl = (pls == 1) ? p -> ll_y1 : p -> ll_y2;
    pr = (prs == 1) ? p -> ll_y2 : p -> ll_y1;
    ql = (qls == 1) ? q -> ll_y1 : q -> ll_y2;
    qr = (qrs == 1) ? q -> ll_y2 : q -> ll_y1;

    /* find intersection between p->leftside and q->rightside */
    if ((n = pls - qrs)) {
	crossy = qr + (q -> ll_x2 - p -> ll_x1 + pls * (pl - qr)) / n;
	if (crossy > max_y1 && crossy < min_y2) a[++count] = crossy;
    }

    /* find intersection between p->leftside and q->leftside */
    if ((n = pls - qls)) {
	crossy = ql + (q -> ll_x1 - p -> ll_x1 + pls * (pl - ql)) / n;
	if (crossy > max_y1 && crossy < min_y2) a[++count] = crossy;
    }

    /* find intersection between p->rightside and q->leftside */
    if ((n = prs - qls)) {
	crossy = ql + (q -> ll_x1 - p -> ll_x2 + prs * (pr - ql)) / n;
	if (crossy > max_y1 && crossy < min_y2) a[++count] = crossy;
    }

    /* find intersection between p->rightside and q->rigthside */
    if ((n = prs - qrs)) {
	crossy = qr + (q -> ll_x2 - p -> ll_x2 + prs * (pr - qr)) / n;
	if (crossy > max_y1 && crossy < min_y2) a[++count] = crossy;
    }

    quicksort (a, 0, count); /* sort the y-values in a[] */

    y = a[0];
    /* intersection between current y and p->leftside */
    lxmin = p -> ll_x1;
    if (pls) lxmin += pls * (y - pl);
    /* intersection between current y and q->leftside */
    lxmax = q -> ll_x1;
    if (qls) lxmax += qls * (y - ql);
    /* intersection between current y and p->rightside */
    rxmax = p -> ll_x2;
    if (prs) rxmax += prs * (y - pr);
    /* intersection between current y and q->rightside */
    rxmin = q -> ll_x2;
    if (qrs) rxmin += qrs * (y - qr);
    if (rxmax < lxmax) lxmax = rxmax;
    if (lxmin > rxmin) rxmin = lxmin;

    for (n = 1; n <= count; ++n) { /* scan in the y-direction */

	yold = y;
	loldxmin = lxmin;
	loldxmax = lxmax;
	roldxmin = rxmin;
	roldxmax = rxmax;

	y = a[n];
	/* intersection between current y and p->leftside */
	lxmin = p -> ll_x1;
	if (pls) lxmin += pls * (y - pl);
	/* intersection between current y and q->leftside */
	lxmax = q -> ll_x1;
	if (qls) lxmax += qls * (y - ql);
	/* intersection between current y and p->rightside */
	rxmax = p -> ll_x2;
	if (prs) rxmax += prs * (y - pr);
	/* intersection between current y and q->rightside */
	rxmin = q -> ll_x2;
	if (qrs) rxmin += qrs * (y - qr);
	if (rxmax < lxmax) lxmax = rxmax;
	if (lxmin > rxmin) rxmin = lxmin;

	if ((y == q -> ll_y1 && yold == p -> ll_y1) ||
	    (y == p -> ll_y2 && yold == q -> ll_y2)) {
		clip_head = out (loldxmin, lxmin, roldxmax, rxmax, yold, y, clip_head);
	}
	else {
	    if (lxmin <= lxmax)
		clip_head = out (loldxmin, lxmin, loldxmax, lxmax, yold, y, clip_head);
	    if (rxmax >= rxmin)
		clip_head = out (roldxmin, rxmin, roldxmax, rxmax, yold, y, clip_head);
	}
    }
    return (clip_head);
}
