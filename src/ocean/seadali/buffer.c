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

extern char *ready_cancel[];
extern Coor piwl, piwr, piwb, piwt;
extern int  NR_lay;
extern int *def_arr;
extern int *edit_arr;
extern int *pict_arr;
extern int  nr_planes;
extern qtree_t **quad_root;

static struct found_list *result; /* elements found while searching */
static struct obj_node **PutBuf;
static Coor Buf_width, Buf_height;

static int  empty_pbuf (void);
static void pict_buf (int termflag, Coor xl, Coor yb);
static void place_buffer (Coor xl, Coor yb);
static struct obj_node *yank_lay_area (int lay, Coor lx, Coor ux, Coor ly, Coor uy);
static struct obj_node *yank_traps (struct obj_node *p, struct obj_node *q);

void fill_buffer (Coor x1, Coor x2, Coor y1, Coor y2, short all_mode)
{
    struct obj_node *pt, *next_p;
    register int lay;
    int new = !PutBuf;

    if (new) MALLOCN (PutBuf, struct obj_node *, NR_lay);

    for (lay = 0; lay < NR_lay; ++lay) {
	/*
	** Clear buffer for current layer.
	*/
	if (!new)
	for (pt = PutBuf[lay]; pt; pt = next_p) {
	    next_p = pt -> next;
	    FREE (pt);
	}
	PutBuf[lay] = NULL;
	if ((all_mode && edit_arr[lay]) || def_arr[lay]) {
	    /*
	    ** Fill buffer for current layer.
	    ** Make coordinates relative to
	    ** lower left corner of buffer.
	    */
	    PutBuf[lay] = yank_lay_area (lay, x1, x2, y1, y2);
	    for (pt = PutBuf[lay]; pt; pt = pt -> next) {
		pt -> ll_x1 -= x1;
		pt -> ll_y1 -= y1;
		pt -> ll_x2 -= x1;
		pt -> ll_y2 -= y1;
	    }
	}
    }
    Buf_width = x2 - x1;
    Buf_height = y2 - y1;
}

/*
** Put_buffer is used to put either the box-buffer or
** the terminal-buffer.  The argument termflag is used
** to switch between these alternatives when necessary.
** Most of the code, however, is unaware of this.
*/
void put_buffer (int termflag)
{
    int  first_time, choice;
    Coor xc_old, yc_old, x_cur, y_cur;

    if ((!termflag && empty_pbuf ()) || (termflag && empty_tbuf ())) {
	ptext ("Buffer is empty!");
	return;
    }
    ptext ("");

    menu (2, ready_cancel);
    first_time = TRUE;
    xc_old = 0; yc_old = 0; /* to eliminate compiler warning */

    set_c_wdw (PICT);
    while ((choice = get_one (1, &x_cur, &y_cur)) == -1) {
	if (nr_planes == 8) clear_curs ();
	else {
	    disp_mode (COMPLEMENT);
	    if (first_time == FALSE) pict_buf (termflag, xc_old, yc_old);
	    xc_old = x_cur;
	    yc_old = y_cur;
	}
	pict_buf (termflag, x_cur, y_cur);
	first_time = FALSE;
	if (nr_planes < 8) disp_mode (TRANSPARENT);
    }

    if (choice == 0 && first_time == FALSE) {
	if (termflag)
	    place_tbuf (x_cur, y_cur);
	else
	    place_buffer (x_cur, y_cur);
    }

    if (nr_planes == 8) clear_curs ();
    else {
	if (first_time == FALSE) {
	    disp_mode (COMPLEMENT);
	    pict_buf (termflag, x_cur, y_cur);
	    disp_mode (TRANSPARENT);
	}
    }
}

static void pict_buf (int termflag, Coor xl, Coor yb)
{
    Coor line[8];
    register struct obj_node *pt;
    register int lay, i;

    if (termflag) {
	pict_tbuf (xl, yb); /* draw terminal buffer */
    }
    else {
	/*
	** Draw primitives buffer.
	*/
	pict_rect ((float) xl, (float) (xl + Buf_width), (float) yb, (float) (yb + Buf_height));

	/*
	** Also draw the contents of the buffer.
	*/
	for (lay = 0; lay < NR_lay; ++lay) {
	    for (pt = PutBuf[lay]; pt; pt = pt -> next) {
		if (pt -> leftside == 0 && pt -> rightside == 0) {
		    /* Draw rectangular object. */
		    pict_rect ((float) pt -> ll_x1 + xl, (float) pt -> ll_x2 + xl,
			(float) pt -> ll_y1 + yb, (float) pt -> ll_y2 + yb);
		}
		else {
		    if (trap_to_poly (line, pt) != -1) {
			for (i = 0; i < 4; ++i) {
			    line[2 * i] = line[2 * i] + xl;
			    line[(2 * i) + 1] = line[(2 * i) + 1] + yb;
			}
			pict_poly (line, 4);
		    }
		}
	    }
	}
    }
    flush_pict ();
}

static int empty_pbuf ()
{
    register int lay;
    if (PutBuf)
    for (lay = 0; lay < NR_lay; ++lay) {
	if (PutBuf[lay]) return (FALSE);
    }
    return (TRUE);
}

static void place_buffer (Coor xl, Coor yb)
{
    register int lay;
    register struct obj_node *pt;

    for (lay = 0; lay < NR_lay; ++lay) {
	for (pt = PutBuf[lay]; pt; pt = pt -> next) {
	    add_new_trap (lay, pt -> ll_x1 + xl, pt -> ll_y1 + yb,
			       pt -> ll_x2 + xl, pt -> ll_y2 + yb, pt -> leftside, pt -> rightside);
	}
	if (PutBuf[lay]) pict_arr[lay] = DRAW;
    }
    piwl = xl;
    piwr = xl + Buf_width;
    piwb = yb;
    piwt = yb + Buf_height;
}

/*
** Copy objects from a user specified area in the yank buffer.
*/
static struct obj_node * yank_lay_area (int lay, Coor lx, Coor ux, Coor ly, Coor uy)
{
    struct obj_node *buffer, *tmp, *yank_area, *yank_buffer;
    struct found_list *free_p, *res_tmp;

    MALLOC (yank_area, struct obj_node);
    yank_area -> ll_x1 = Min (lx, ux);
    yank_area -> ll_y1 = Min (ly, uy);
    yank_area -> ll_x2 = Max (lx, ux);
    yank_area -> ll_y2 = Max (ly, uy);
    yank_area -> leftside = yank_area -> rightside = 0;

    yank_buffer = NULL;
    result = NULL;

    /*
    ** Find those parts of the trapezoids inside the YANK-area.
    */
    quad_search (quad_root[lay], yank_area, q_found);

    for (res_tmp = result; res_tmp; res_tmp = res_tmp -> next) {

	buffer = yank_traps (res_tmp -> ptrap, yank_area);

	if (buffer) {
	    /*
	    ** Attach partial list to previous ones
	    **
	    ** Search end of partial list:
	    */
	    for (tmp = buffer; tmp -> next; tmp = tmp -> next);
	    tmp -> next = yank_buffer;
	    yank_buffer = buffer;
	}
    }

    FREE (yank_area);
    for (res_tmp = result; res_tmp; res_tmp = free_p) {
	free_p = res_tmp -> next;
	FREE (res_tmp);
    }

    result = NULL;
    return (yank_buffer);
}

/*
** Find those parts of p inside q.
** INPUT: pointers to trapezoids p and q.
** OUTPUT: a list of resulting trapezoids.
*/
static struct obj_node * yank_traps (struct obj_node *p, struct obj_node *q)
{
    struct obj_node *yank_head;
    Coor a[8];
    Coor crossy;
    Coor interx1, interx2, interx3, interx4;
    Coor px, py, qx, qy;
    Coor xmin, xmax, oldxmin, oldxmax, yold;
    register int count, teller;

    oldxmin = 0; oldxmax = 0; yold = 0; /* to eliminate compiler warning */

    yank_head = NULL;

    ASSERT (p);
    ASSERT (q);

    count = 0;
    a[count++] = Max (p -> ll_y1, q -> ll_y1);
    a[count] = Min (p -> ll_y2, q -> ll_y2);

    /* find intersection between p->leftside and q->rightside */
    px = p -> ll_x1;
    py = (p -> leftside  == 1)? p -> ll_y1 : p -> ll_y2;
    qx = q -> ll_x2;
    qy = (q -> rightside == 1)? q -> ll_y2 : q -> ll_y1;

    if (p -> leftside != q -> rightside) {
	crossy = qy + (qx - px + p -> leftside * (py - qy)) / (p -> leftside - q -> rightside);
	if (crossy > Max (p -> ll_y1, q -> ll_y1) && crossy < Min (p -> ll_y2, q -> ll_y2))
	    a[++count] = crossy;
    }

    /* find intersection between p->leftside and q->leftside */
    qx = q -> ll_x1;
    qy = (q -> leftside == 1)? q -> ll_y1 : q -> ll_y2;

    if (p -> leftside != q -> leftside) {
	crossy = qy + (qx - px + p -> leftside * (py - qy)) / (p -> leftside - q -> leftside);
	if (crossy > Max (p -> ll_y1, q -> ll_y1) && crossy < Min (p -> ll_y2, q -> ll_y2))
	    a[++count] = crossy;
    }

    /* find intersection between p->rightside and q->leftside */
    px = p -> ll_x2;
    py = (p -> rightside == 1)? p -> ll_y2 : p -> ll_y1;

    if (p -> rightside != q -> leftside) {
	crossy = qy + (qx - px + p -> rightside * (py - qy)) / (p -> rightside - q -> leftside);
	if (crossy > Max (p -> ll_y1, q -> ll_y1) && crossy < Min (p -> ll_y2, q -> ll_y2))
	    a[++count] = crossy;
    }

    /* find intersection between p->rightside and q->rigthside */
    qx = q -> ll_x2;
    qy = (q -> rightside == 1)? q -> ll_y2 : q -> ll_y1;

    if (p -> rightside != q -> rightside) {
	crossy = qy + (qx - px + p -> rightside * (py - qy)) / (p -> rightside - q -> rightside);
	if (crossy > Max (p -> ll_y1, q -> ll_y1) && crossy < Min (p -> ll_y2, q -> ll_y2))
	    a[++count] = crossy;
    }

    quicksort (a, 0, count); /* sort the y-values */

    for (teller = 0; teller <= count; ++teller) {

	/* intersection between current y and p->leftside */
	px = p -> ll_x1;
	py = (p -> leftside == 1)? p -> ll_y1 : p -> ll_y2;
	interx1 = px + p -> leftside * (a[teller] - py);

	/* intersection between current y and q->leftside */
	qx = q -> ll_x1;
	qy = (q -> leftside == 1)? q -> ll_y1 : q -> ll_y2;
	interx2 = qx + q -> leftside * (a[teller] - qy);

	/* intersection between current y and p->rightside */
	px = p -> ll_x2;
	py = (p -> rightside == 1)? p -> ll_y2 : p -> ll_y1;
	interx3 = px + p -> rightside * (a[teller] - py);

	/* intersection between current y and q->rightside */
	qx = q -> ll_x2;
	qy = (q -> rightside == 1)? q -> ll_y2 : q -> ll_y1;
	interx4 = qx + q -> rightside * (a[teller] - qy);

	xmin = Max (interx1, interx2);
	xmax = Min (interx3, interx4);

	if (teller) yank_head = out (oldxmin, xmin, oldxmax, xmax, yold, a[teller], yank_head);
	yold = a[teller];
	oldxmin = xmin;
	oldxmax = xmax;
    }

    return (yank_head);
}

/*
** Add a trapezoid to "list".
** INPUT: the trapezoid's attributes.
*/
struct obj_node * out (Coor x1, Coor x2, Coor x3, Coor x4, Coor ymin, Coor ymax, struct obj_node *list)
{
    struct obj_node *r;
    struct obj_node *tmp;
    Coor xmin, xmax;

    xmin = Min (x1, x2);
    xmax = Max (x3, x4);

    if (xmin >= xmax) return (list);
    if (ymin >= ymax) return (list);

    /*
    ** Insert object into a linked list.
    */

    /* allocate memory for object */
    MALLOC (r, struct obj_node);

    r -> ll_x1 = xmin;
    r -> ll_y1 = ymin;
    r -> ll_x2 = xmax;
    r -> ll_y2 = ymax;

    if (x1 > x2)
	r -> leftside = -1;
    else if (x1 < x2)
	r -> leftside = 1;
    else
	r -> leftside = 0;

    if (x3 < x4)
	r -> rightside = 1;
    else if (x3 > x4)
	r -> rightside = -1;
    else
	r -> rightside = 0;

    if (test_trap (r)) {
	FREE (r);
	return (list);
    }

    /*
    ** Merge r vertically with elements in list if possible.
    */
    for (tmp = list; tmp; tmp = tmp -> next)
	if (merge (tmp, r)) {
	    FREE (r);
	    return (list);
	}
    r -> next = list;
    r -> mark = 0;
    list = r;

    return (list);
}

/*
** Sort a[l] to a[r].
*/
void quicksort (Coor a[], int l, int r)
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
** Add a pointer to the trapezoid to the result list.
** INPUT: a pointer to the trapezoid.
*/
void q_found (struct obj_node *p)
{
    struct found_list *flist;

    MALLOC (flist, struct found_list);
    flist -> ptrap = p;
    flist -> next = result;
    result = flist;
}
