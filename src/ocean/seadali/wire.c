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

#define MAX_P    128
#define MAX_W  10000

extern long *LayerWidth;
extern int  ImageMode;
extern int  draw_sngrid;
extern int  nr_planes;
extern int  Cur_nr;
extern int *pict_arr;
extern Coor piwl, piwr, piwb, piwt; /* window to be drawn */
extern Coor xlc, xrc, ybc, ytc;     /* cursor coordinates */

static Coor cur_wire_width = 0;	/* Current wire-width in QUAD_LAMBDA. */
static int  wire_ext = FALSE;	/* Wire-extension flag. */

static int  ask_wire_width (void);
static void draw_wire_lines (Coor x_arr[], Coor y_arr[], int nr_points);

void set_wire_width (int aWidth)
{
    if (aWidth <= 0) cur_wire_width = 0;
    else cur_wire_width = aWidth * QUAD_LAMBDA;
}

void set_wire_ext (int aVal)
{
    wire_ext = aVal;
}

void wire_points (int non_orth)
{
    Coor    stack_x[MAX_P], stack_y[MAX_P], line[4];
    Coor    dx, dy, coor_x, coor_y;
    short   rvA, rvB, paintflag;
    int     nr_p;
    int     choice, new_choice, ask_nr;

    static char *ask_str[] = {
    /* 0 */ "-ready-",
    /* 1 */ "-next-",
    /* 2 */ "-restart-",
    /* 3 */ "tgle_grid",
    /* 4 */ "w_bbx",
    /* 5 */ "w_prev",
    /* 6 */ "w_center",
    /* 7 */ "zoom",
    /* 8 */ "dezoom",
    /* 9 */ "wire_width",
    /* 10 */ "tgle_ext",
    };
    char    width_str[MAXSTR];

    if (cur_wire_width <= 0 ||
	    (draw_sngrid == TRUE && (cur_wire_width / QUAD_LAMBDA) % 2 == 1)) {
// #ifdef IMAGE
	cur_wire_width = QUAD_LAMBDA * LayerWidth[0];
// #else
//	cur_wire_width = ask_wire_width ();
// #endif
    }

    new_choice = 1;
    nr_p = 0;

start_menu:
    sprintf (width_str, "width = %ld", cur_wire_width / QUAD_LAMBDA);
    ptext (width_str);

    ask_nr = sizeof (ask_str) / sizeof (char *);
    menu (ask_nr, ask_str);

    if (wire_ext) pre_cmd_proc (10, ask_str);

    set_c_wdw (PICT);
    while (TRUE) {
	if (new_choice == -1) {
	    if (nr_p > 0) {
		coor_x = stack_x[nr_p - 1];
		coor_y = stack_y[nr_p - 1];
	    }
	    choice = get_wire_point ((nr_p == 0) ? 1 : 4, &coor_x, &coor_y, cur_wire_width, non_orth);
	    if (choice == -2) goto start_menu;
	    if (ImageMode == TRUE && nr_p > 0) snap_point_to_grid (&coor_x, &coor_y);
	    choice = Min (choice, ask_nr);
	    /* limit menu value to max value (security) */
	}
	else {
	    choice = new_choice;
	    new_choice = -1;
	}

	if (choice == -1) {
	    paintflag = TRUE;
	    stack_x[nr_p] = coor_x;
	    stack_y[nr_p] = coor_y;
	    if (nr_p > 0) {
		/* if new coordinate == old coordinate */
		if (stack_x[nr_p] == stack_x[nr_p - 1] && stack_y[nr_p] == stack_y[nr_p - 1]) {
		    if (nr_planes == 8)
			clear_curs ();
		    else {
			/* draw to erase */
			draw_wire_lines (stack_x, stack_y, nr_p);
		    }
		    add_wire (stack_x, stack_y, nr_p, cur_wire_width, non_orth, wire_ext);
		    picture ();
		    nr_p = 0; /* start with a new wire */
		    continue;
		}

		dx = stack_x[nr_p] - stack_x[nr_p - 1];
		dy = stack_y[nr_p] - stack_y[nr_p - 1];

		if (non_orth) {
		    /* allign for orthogonal or 45 degr wires */

		    if (Abs (dy) > 2 * Abs (dx)) {
			stack_x[nr_p] = stack_x[nr_p - 1];
		    }
		    else if (Abs (dx) > 2 * Abs (dy)) {
			stack_y[nr_p] = stack_y[nr_p - 1];
		    }
		    else {
			if (dx < 0) dx = -Abs (dy);
			else        dx =  Abs (dy);
			stack_x[nr_p] = stack_x[nr_p - 1] + dx;
			stack_y[nr_p] = stack_y[nr_p - 1] + dy;
		    }
		}
		else {		/* allign for orthogonal wires */
		    if (Abs (dx) < Abs (dy))
			stack_x[nr_p] = stack_x[nr_p - 1];
		    else
			stack_y[nr_p] = stack_y[nr_p - 1];
		}

		/* must we walk back over our own center wire? */

		if (nr_p >= 2) {

		    dx = stack_x[nr_p] - stack_x[nr_p - 1];
		    dy = stack_y[nr_p] - stack_y[nr_p - 1];

		 /* if (!dy) */ rvB = 0;
		    if (!dx) rvB = 2;
		    if (dy == dx) rvB = 1;
		    if (dy == -dx) rvB = -1;

		    dx = stack_x[nr_p - 1] - stack_x[nr_p - 2];
		    dy = stack_y[nr_p - 1] - stack_y[nr_p - 2];

		 /* if (!dy) */ rvA = 0;
		    if (!dx) rvA = 2;
		    if (dy == dx) rvA = 1;
		    if (dy == -dx) rvA = -1;

		    if (rvA == rvB) {		/* walk back */

			line[0] = stack_x[nr_p - 2];
			line[1] = stack_y[nr_p - 2];
			line[2] = stack_x[nr_p - 1];
			line[3] = stack_y[nr_p - 1];

			/* delete part of the polygon */
			draw_poly_line (line, 0, 4, DELETE);

			line[2] = stack_x[nr_p - 1] = stack_x[nr_p];
			line[3] = stack_y[nr_p - 1] = stack_y[nr_p];

			/* redraw part of polygon */
			draw_poly_line (line, 0, 4, ADD);
			paintflag = FALSE;

			nr_p--;
			if (stack_x[nr_p] == stack_x[nr_p - 1] &&
				stack_y[nr_p] == stack_y[nr_p - 1])
			    nr_p--;
		    }
		}
		if (paintflag) {
		    draw_wire_lines (stack_x + nr_p - 1, stack_y + nr_p - 1, 2);
		}
	    }
	    nr_p++;

	    if (nr_p == MAX_P) {
		ptext ("Too many wire points!");
		choice = 1;
		goto sw_c;
	    }
	}
	else { /* menu value selected */
	    post_cmd_proc (1, ask_str);
sw_c:
	    switch (choice) {
	    case 0:
	    case 1:
	    case 2:
		pre_cmd_proc (choice, ask_str);
		if (nr_p > 0) {
		    /* erase lines */
		    if (nr_planes == 8)
			clear_curs ();
		    else {
			set_c_wdw (PICT);
			/* draw to erase */
			draw_wire_lines (stack_x, stack_y, nr_p);
		    }
		    if (choice != 2) { /* if no restart (0 or 1) */
			add_wire (stack_x, stack_y, nr_p, cur_wire_width, non_orth, wire_ext);
			if (choice == 1) /* next wire -> picture first */
			    picture ();
		    }
		    nr_p = 0; /* start with a new wire */
		}
		if (choice == 0) {
		    /* ready -> return to calling command loop */
		    return;
		}
		break;

	    case 3: /* tgle_grid */
		pre_cmd_proc (choice, ask_str);
		toggle_grid ();
		picture ();
		if (!grid_on ())
		    draw_wire_lines (stack_x, stack_y, nr_p);
		break;

	    case 4: /* w_bbx */
		pre_cmd_proc (choice, ask_str);
		bound_w ();
		picture ();
		draw_wire_lines (stack_x, stack_y, nr_p);
		break;

	    case 5: /* w_prev */
		pre_cmd_proc (choice, ask_str);
		prev_w ();
		picture ();
		draw_wire_lines (stack_x, stack_y, nr_p);
		break;

	    case 6: /* w_center */
		pre_cmd_proc (choice, ask_str);
		if (nr_p > 0) {
		    /* old wire point for fixing of rubber line */
		    coor_x = stack_x[nr_p - 1];
		    coor_y = stack_y[nr_p - 1];
		}
		new_choice = get_one ((nr_p == 0) ? 1 : 4, &coor_x, &coor_y);
		if (new_choice == -2) goto start_menu;
		new_choice = Min (new_choice, ask_nr);
		if (new_choice == -1) { /* coordinate selected */
		    center_w (coor_x, coor_y);
		    picture ();
		    draw_wire_lines (stack_x, stack_y, nr_p);
		}
		break;

	    case 7: /* zoom */
		pre_cmd_proc (choice, ask_str);
		new_choice = set_tbltcur (2, NO_SNAP);
		if (new_choice == -2) goto start_menu;
		new_choice = Min (new_choice, ask_nr);
		if (new_choice == -1) { /* coordinates selected */
		    curs_w (xlc, xrc, ybc, ytc);
		    picture ();
		    draw_wire_lines (stack_x, stack_y, nr_p);
		}
		break;

	    case 8: /* dezoom */
		pre_cmd_proc (choice, ask_str);
		new_choice = set_tbltcur (2, NO_SNAP);
		if (new_choice == -2) goto start_menu;
		new_choice = Min (new_choice, ask_nr);
		if (new_choice == -1) { /* coordinates selected */
		    de_zoom (xlc, xrc, ybc, ytc);
		    picture ();
		    draw_wire_lines (stack_x, stack_y, nr_p);
		}
		break;
	    case 9: /* wire_width */
		pre_cmd_proc (choice, ask_str);
		if (nr_p > 0) {
		    ptext ("Width cannot be changed now!");
		    sleep (1);
		}
		else {
		    cur_wire_width = ask_wire_width ();

		    menu (ask_nr, ask_str);
		    if (wire_ext) {
			pre_cmd_proc (10, ask_str);
		    }
		}
		sprintf (width_str, "width = %ld", cur_wire_width / QUAD_LAMBDA);
		ptext (width_str);
		break;
	    case 10: /* tgle_ext */
		if (!wire_ext) {
		    pre_cmd_proc (choice, ask_str);
		    wire_ext = TRUE;
		}
		else {
		    post_cmd_proc (choice, ask_str);
		    wire_ext = FALSE;
		}
		new_choice = -1;
		break;
	    }

	    if (choice != 1 && choice != 10) {
		post_cmd_proc (choice, ask_str);
		/*
		** switch directly back to wire mode if any of
		** the other commands has been executed, and
		** another command has not been reselected.
		*/
	    }
	    if (new_choice == -1) pre_cmd_proc (1, ask_str);
	}
    }
}

static void draw_wire_lines (Coor x_arr[], Coor y_arr[], int nr_points)
{
    register int i;

    if (nr_planes < 8) disp_mode (COMPLEMENT);

    ggSetColor (Cur_nr);

    d_ltype (LINE_DOUBLE);
    for (i = 0; i < (nr_points - 1); ++i)
	d_line ((float) x_arr[i], (float) y_arr[i], (float) x_arr[i + 1], (float) y_arr[i + 1]);
    d_ltype (LINE_SOLID);

    if (nr_planes < 8) disp_mode (TRANSPARENT);
}

static Coor * wire_width = NULL;
static char ** ask_wire_str = NULL;
static int no_wire_values = 0;

int default_wire_widths[] = {
	2, 4, 6, 8, 10, 12,
};

void set_wire_values (int no_wire_widths, int *wire_list)
{
    int i;
    static char * kbd = "keyboard";
    static char * intct = "interact";

    no_wire_values = no_wire_widths;

    wire_width = (Coor *) calloc (no_wire_widths, sizeof (Coor));
    ask_wire_str = (char **) calloc (no_wire_widths + 2, sizeof (char *));

    ask_wire_str[0] = kbd;
    ask_wire_str[1] = intct;

    for (i = 0; i < no_wire_widths; i++) {
	int value;
	ask_wire_str[i + 2] = (char *) calloc (10, sizeof (char));

	value = wire_list[i];
	if (value <= 0) value = 2;

	wire_width[i] = value * QUAD_LAMBDA;
	sprintf (ask_wire_str[i + 2], "%d", value);
    }
}

static int ask_wire_width ()
{
    int	    index;
    int     choice;
    int     new_choice;
    Coor    x1, x2, y1, y2;
    Coor    sel_width = 0;
    int     value;
    char    tmp_str[MAXSTR];

    if (!ask_wire_str) {
	set_wire_values (sizeof (default_wire_widths) / sizeof (int), default_wire_widths);
    }

    for (index = 0; index < no_wire_values; ++index) {
	if (wire_width[index] == cur_wire_width) break;
    }

start_menu:
    if (draw_sngrid == TRUE) {
	ptext ("Please specify even width!");
    }
    else {
	ptext ("Please specify width!");
    }

    choice = ask (no_wire_values + 2, ask_wire_str, (index >= no_wire_values) ? -1 : index + 2);
    if (choice == -2) goto start_menu;

    while (1) {

	switch (choice) {
	case 0: 		/* keyboard */
	    ask_string ("width: ", tmp_str);
	    sscanf (tmp_str, "%d", &value);
	    sel_width = value * QUAD_LAMBDA;
	    break;
	case 1: 		/* interactive */
	    ptext ("Please enter two points!");
	    if ((new_choice = get_one (1, &x1, &y1)) != -1) {
		/* (other) menu item selected */
		if (new_choice == -2) goto start_menu;
		post_cmd_proc (choice, ask_wire_str);
		choice = new_choice;
		pre_cmd_proc (choice, ask_wire_str);
		continue;
	    }
	    x2 = x1;
	    y2 = y1;
	    if ((new_choice = get_one (4, &x2, &y2)) != -1) {
		/* (other) menu item selected */
		if (new_choice == -2) goto start_menu;
		post_cmd_proc (choice, ask_wire_str);
		choice = new_choice;
		pre_cmd_proc (choice, ask_wire_str);
		continue;
	    }
	    /* two points have been entered */
	    sel_width = (Coor) Max (2, (Max (Abs (x2 - x1), Abs (y2 - y1))));
	    break;
	default:
	    sel_width = wire_width[choice - 2];
	    break;
	}

	if (sel_width <= 0 || sel_width > MAX_W
	      || (draw_sngrid == TRUE && (sel_width / QUAD_LAMBDA) % 2 == 1)) {
	    if (sel_width <= 0) {
		ptext ("Positive width required!");
	    }
	    else if (sel_width > MAX_W) {
		ptext ("Width too large!");
	    }
	    else {
		ptext ("Snap-grid is on: even width required!");
	    }
	    sleep (1);
	}
	else break;
    }

    return (sel_width);
}
