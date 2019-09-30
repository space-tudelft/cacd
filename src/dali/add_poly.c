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

extern struct obj_node **PutBuf;
extern Coor piwl, piwb, piwr, piwt;
extern Coor *Line_x, *Line_y;
extern int  NR_lay, Nr_p;
extern int  Cur_nr;
extern int *def_arr;
extern int  erase_text;
extern int  gridflag;
extern int *pict_arr;
extern int  cmd_nbr, new_cmd, grid_cmd;
static int  cross = 0;

#define RETURN   0
#define NEXT     1
#define CANCEL   2
#define GRIDNR   3
#define BACK     9
#define BIG     10
#define CROSS   11
#define NOR     12

static void do_poly (struct obj_node *list, int mode);

/*
** Get a polygon, split it into trapezoids and add these
** to the quad tree.
*/
void add_del_poly (int mode)
{
    static char *ask_str[] = {
    /* 0 */ "-return-",
    /* 1 */ "-next-",
    /* 2 */ "-cancel-",
    /* 3 */ "grid",
    /* 4 */ "[bbox]",
    /* 5 */ "[prev]",
    /* 6 */ "[center]",
    /* 7 */ "[zoom]",
    /* 8 */ "[dezoom]",
    /* 9 */ "walk_back",
    /*10 */ "use_big",
    /*11 */ "set_cross",
    /*12 */ "45 degree",
    };
    struct obj_node *list;
    Coor line_x[MaxPoints], line_y[MaxPoints], xp, yp;
    Coor coor_x, coor_y;
    char txt[MAXSTR];
    int  big = 0, lay, nbr, ready, non_orth = 1;

    Line_x = line_x;
    Line_y = line_y;

    sprintf (txt, "Enter points to %s!",
	(mode == ADD) ? "add polygon" : "delete poly area");

    ready = Nr_p = 0;

do_menu:
    menu (sizeof (ask_str) / sizeof (char *), ask_str);
    grid_cmd = GRIDNR;
    if (gridflag) pre_cmd_proc (GRIDNR);
    if (big) pre_cmd_proc (BIG);
    if (non_orth) pre_cmd_proc (NOR);
    ptext (txt);

    set_c_wdw (PICT);

    while (TRUE) {
	if (new_cmd < 0) {
	    while ((nbr = get_all ((Nr_p == 0) ? 1 : 4, &coor_x, &coor_y)) < 0) {
		if (nbr != -1) goto do_menu;

		line_x[Nr_p] = coor_x;
		line_y[Nr_p] = coor_y;
		if (Nr_p) { /* if not first point */
		    if (test_point (big, non_orth, line_x, line_y)) continue;
		    if (Nr_p > 2 && line_x[Nr_p] == line_x[0]
				 && line_y[Nr_p] == line_y[0]) {
			++Nr_p;
			nbr = NEXT;
			ready = 1;
			break; /* polygon is closed */
		    }
		}
maxtest:
		if (++Nr_p >= MaxPoints - 1) {
		    notify ("Max. points reached, please enter last point!");
		    if (Nr_p == MaxPoints) { /* skip point */
			--Nr_p;
			draw_lines (line_x + Nr_p - 1, line_y + Nr_p - 1, 2);
		    }
		}
		coor_x = line_x[Nr_p - 1];
		coor_y = line_y[Nr_p - 1];
	    }
	}
	else {
	    nbr = new_cmd;
	    new_cmd = -1;
	}

	/* menu value selected */
	if (nbr != CROSS) pre_cmd_proc (nbr);
	switch (nbr) {
	    case BACK:
		if (Nr_p > 1) {
		    --Nr_p;
		    set_c_wdw (PICT);
		    draw_lines (line_x + Nr_p - 1, line_y + Nr_p - 1, 2);
		    coor_x = line_x[Nr_p - 1];
		    coor_y = line_y[Nr_p - 1];
		}
		break;

	    case NEXT:
		if (!ready) { /* try auto finish! */
		    if (Nr_p > 2) {
			line_x[Nr_p] = line_x[0];
			line_y[Nr_p] = line_y[0];
			set_c_wdw (PICT);
			test_point (big, non_orth, line_x, line_y);
			if (line_x[Nr_p] != line_x[0] ||
			    line_y[Nr_p] != line_y[0]) {
			    ptext ("Polygon not yet closed!");
			    post_cmd_proc (nbr);
			    set_c_wdw (PICT);
			    goto maxtest;
			}
			++Nr_p;
			ready = 1;
		    }
		    else {
			ptext ("Too less polygon points!");
			break;
		    }
		}
		/*
		** Test for defined layers.
		*/
		do {
		    for (lay = 0; lay < NR_lay; ++lay) {
			if (def_arr[lay]) break;
		    }
		    if (lay < NR_lay) break;
		    btext ("Set your layer first!");
		    do {
			nbr = get_one (1, &xp, &yp);
		    } while (nbr != BACK && nbr != RETURN
				&& nbr != NEXT && nbr != CANCEL);
		} while (nbr == NEXT || nbr == RETURN);
		post_cmd_proc (NEXT);
		if (nbr == BACK) {
		    ready = 0;
		    new_cmd = nbr;
		    break;
		}

	    case RETURN:
		if (nbr == RETURN && Nr_p > 0 && !ready) {
		    btext ("Polygon not yet closed, give cancel/next first!");
		    post_cmd_proc (nbr);
		    break;
		}

	    case CANCEL:
		set_c_wdw (PICT);
		Rpoly (); /* erase */
		if (cross) {
		    cross = 0;
		    post_cmd_proc (CROSS);
		}
		if (ready && nbr != CANCEL) {
		    list = poly_trap (line_x, line_y, Nr_p);
		    if (!list) {
			btext ("Illegal polygon!");
			sleep (1);
		    }
		    else {
			do_poly (list, mode);
			picture ();
		    }
		}
		ready = Nr_p = 0;
		if (nbr == RETURN) {
		    erase_text = 1;
		    return;
		}
		post_cmd_proc (nbr);
		goto do_menu; /* start all over again */

	    case GRIDNR:
		toggle_grid ();
		picture ();
		break;

	    case 4: /* bbox-window */
		bound_w ();
		picture ();
		Rpoly ();
		break;

	    case 5: /* prev-window */
		prev_w ();
		picture ();
		Rpoly ();
		break;

	    case 6: /* center-window */
		ptext ("Select point to center!");
		xp = coor_x;
		yp = coor_y;
		new_cmd = get_one (Nr_p ? 4 : 1, &xp, &yp);
		if (new_cmd == -1) { /* coordinate selected */
		    center_w (xp, yp);
		    picture ();
		    Rpoly ();
		}
		else if (new_cmd == -2) goto do_menu;
		break;

	    case 7: /* zoom */
		cmd_nbr = -1;
		ask_curs_w ();
		if (new_cmd == -1) { /* coordinate selected */
		    picture ();
		    Rpoly ();
		}
		else if (new_cmd == -2) goto do_menu;
		break;

	    case 8: /* dezoom */
		cmd_nbr = -1;
		ask_de_zoom ();
		if (new_cmd == -1) { /* coordinate selected */
		    picture ();
		    Rpoly ();
		}
		else if (new_cmd == -2) goto do_menu;
		break;

	    case BIG: /* use big_angle */
		if (!(big = !big)) post_cmd_proc (nbr);
		break;

	    case CROSS: /* set cross */
		if (Nr_p) {
		    if (!(cross = !cross)) post_cmd_proc (nbr);
		    else pre_cmd_proc (nbr);
		    set_c_wdw (PICT);
		    ggSetColor (Cur_nr);
		    draw_Cross (line_x[0], line_y[0]);
		}
		break;

	    case NOR: /* nor_toggle */
		if (!(non_orth = !non_orth)) post_cmd_proc (nbr);
		break;
	}
	if (nbr != BIG && nbr != CROSS &&
	    nbr != NOR && nbr != GRIDNR) post_cmd_proc (nbr);
    }
}

static void do_poly (struct obj_node *list, int mode)
{
    register struct obj_node *p;
    register int lay;
    int first = 1;

    /*
    ** For each layer add the trapezoids to the quad tree.
    */
    for (lay = 0; lay < NR_lay; ++lay) {

	if (mode == DELETE) clearPutBuf (lay);

	if (def_arr[lay]) {
	    if (mode == ADD) {
		add_new_traps (lay, list);
		pict_arr[lay] = DRAW;
	    }
	    else { /* delete trapezoids from the data base */
		del_traps (list, lay);
		if (PutBuf[lay]) {
		    pict_arr[lay] = ERAS_DR;
		    calc_bbox (first, PutBuf[lay]);
		    if (first) first = 0;
		}
	    }
	}
    }

    if (mode == ADD) calc_bbox (first, list);

    while ((p = list)) {
	list = p -> next;
	FREE (p);
    }
}

/*
** Calculate bounding box for picture routine.
*/
void calc_bbox (int first, struct obj_node *list)
{
    register struct obj_node *p = list;

    if (first) {
	piwl = p -> ll_x1;
	piwb = p -> ll_y1;
	piwr = p -> ll_x2;
	piwt = p -> ll_y2;
	p = p -> next;
    }
    for (; p; p = p -> next) {
	if (p -> ll_x1 < piwl) piwl = p -> ll_x1;
	if (p -> ll_y1 < piwb) piwb = p -> ll_y1;
	if (p -> ll_x2 > piwr) piwr = p -> ll_x2;
	if (p -> ll_y2 > piwt) piwt = p -> ll_y2;
    }
}

void draw_lines (Coor line_x[], Coor line_y[], int npoints)
{
    register int i;

    if (npoints > 1) { /* two or more points */
	disp_mode (COMPLEMENT);
	ggSetColor (Cur_nr);
	d_ltype (LINE_DOUBLE);
	for (i = 1; i < npoints; ++i) {
	    d_line ((float) line_x[i - 1], (float) line_y[i - 1],
		    (float) line_x[i], (float) line_y[i]);
	}
	d_ltype (LINE_SOLID);
	disp_mode (TRANSPARENT);
    }
}

void Rpoly () /* Redraw add_poly OR add_wire */
{
    if (Nr_p) {
	float xp, yp;
	xp = Line_x[Nr_p - 1];
	yp = Line_y[Nr_p - 1];
	fix_loc (xp, yp, 4);
	draw_lines (Line_x, Line_y, Nr_p);
	if (cross) draw_Cross (*Line_x, *Line_y);
    }
}

/*
** Test if the new point is a valid one.
*/
int test_point (int big, int non_orth, Coor line_x[], Coor line_y[])
{
    Coor dx, dy;
    int  rvA, rvB, paintflag, nr_p;

    if (!(nr_p = Nr_p)) return (0); /* first point! */
    dx = line_x[nr_p] - line_x[nr_p - 1];
    dy = line_y[nr_p] - line_y[nr_p - 1];
    if (!dx && !dy) return (1); /* skip point! */

    if (!non_orth) { /* Align for orthogonal lines */
	if (Abs (dx) < Abs (dy)) {
	    if (big && dx) goto doHor;
	    goto doVer;
	}
	if (big && dy) goto doVer;
	goto doHor;
    }
    else { /* Align for orthogonal or 45 degree lines */
	if (Abs (dy) > 2 * Abs (dx)) { /* vertical */
	    if (big && dx) goto do45;
doVer:
	    if (dx) line_x[nr_p] = line_x[nr_p - 1];
	    rvB = 2;
	}
	else if (Abs (dx) > 2 * Abs (dy)) { /* horizontal */
	    if (big && dy) goto do45;
doHor:
	    if (dy) line_y[nr_p] = line_y[nr_p - 1];
	    rvB = 0;
	}
	else { /* 45 degree */
	    if (big) {
		if (Abs (dy) > Abs (dx)) goto doVer;
		if (Abs (dy) < Abs (dx)) goto doHor;
	    }
do45:
	    if (Abs (dy) > Abs (dx)) {
		dy = (dy < 0) ? -Abs (dx) : Abs (dx);
		line_y[nr_p] = line_y[nr_p - 1] + dy;
	    }
	    else {
		dx = (dx < 0) ? -Abs (dy) : Abs (dy);
		line_x[nr_p] = line_x[nr_p - 1] + dx;
	    }
	    if (dy == dx) rvB = 1;
	    else rvB = -1;
	}
    }

    paintflag = 1;
    /*
    ** Must we walk back over our own center line?
    */
    if (nr_p > 1) { /* more than one line */
	dx = line_x[nr_p - 1] - line_x[nr_p - 2];
	dy = line_y[nr_p - 1] - line_y[nr_p - 2];

	if (dy == -dx) rvA = -1;
	else if (dy == dx) rvA = 1;
	else if (!dx) rvA = 2; /* vertical */
	else rvA = 0; /* horizontal */

	if (rvA == rvB) { /* walk back */
	    draw_lines (line_x + nr_p - 2, line_y + nr_p - 2, 2); /* erase */
	    --nr_p;
	    line_x[nr_p] = line_x[nr_p + 1];
	    line_y[nr_p] = line_y[nr_p + 1];

	    if (line_x[nr_p] == line_x[nr_p - 1] &&
		line_y[nr_p] == line_y[nr_p - 1]) {
		--nr_p;
		paintflag = 0;
	    }
	}
    }
    if (paintflag) {
	draw_lines (line_x + nr_p - 1, line_y + nr_p - 1, 2);
    }
    Nr_p = nr_p;
    return (0); /* OK */
}
