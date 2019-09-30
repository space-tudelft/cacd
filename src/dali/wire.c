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

#define E_NR       2
#define MAX_W  10000

extern int *def_arr;
extern int  act_sngrid;
extern int  allow_keypress;
extern int  erase_text;
extern int  gridflag;
extern int  cmd_nbr, new_cmd, grid_cmd;
extern int  Cur_nr;
extern int  NR_lay, Nr_p;
extern Coor piwl, piwr, piwb, piwt; /* window to be drawn */
extern Coor xlc, xrc, ybc, ytc;     /* cursor coordinates */
extern Coor *Line_x, *Line_y;

static Coor cur_wire_width = 0;	/* Current wire-width in QUAD_LAMBDA */
static int  wire_ext = 0;	/* Wire-extension flag, default: OFF */

void set_wire_width (int aWidth)
{
    cur_wire_width = aWidth * QUAD_LAMBDA;
}

void set_wire_ext (int aVal)
{
    wire_ext = aVal;
}

#define READY    0
#define NEXT_W   1
#define CANCEL   2
#define GRIDNR   3
#define BACK     9
#define W_SET   10
#define W_EXT   11
#define W_NOR   12

static int ask_wire_width (void);

void wire_points ()
{
    static char *ask_str[] = {
    /* 0 */ "-ready-",
    /* 1 */ "-next-",
    /* 2 */ "-cancel-",
    /* 3 */ "grid",
    /* 4 */ "[bbox]",
    /* 5 */ "[prev]",
    /* 6 */ "[center]",
    /* 7 */ "[zoom]",
    /* 8 */ "[dezoom]",
    /* 9 */ "walk_back",
    /*10 */ "set_width",
    /*11 */ "extension",
    /*12 */ "45 degree",
    };
    char    width_str[MAXSTR];
    Coor    line_x[MaxPoints], line_y[MaxPoints];
    Coor    coor_x, coor_y;
    int     nbr, lay, non_orth = 0;

    Line_x = line_x;
    Line_y = line_y;

    if (!cur_wire_width || (act_sngrid && (cur_wire_width / QUAD_LAMBDA) % 2 == 1)) {
	cur_wire_width = ask_wire_width ();
    }

    Nr_p = 0;

do_menu:
    menu (sizeof (ask_str) / sizeof (char *), ask_str);
    grid_cmd = GRIDNR;
    if (gridflag) pre_cmd_proc (GRIDNR);
    if (wire_ext) pre_cmd_proc (W_EXT);
    if (non_orth) pre_cmd_proc (W_NOR);

    sprintf (width_str, "Enter points to add wire (width = %ld)!", cur_wire_width / QUAD_LAMBDA);
    ptext (width_str);

    set_c_wdw (PICT);

    while (TRUE) {
	if (new_cmd < 0) {
	    while ((nbr = get_wire_point (Nr_p ? 4 : 1,
			&coor_x, &coor_y, cur_wire_width, non_orth)) < 0) {
		if (nbr != -1) goto do_menu;
		if (Nr_p == MaxPoints) goto auto_fini;
		line_x[Nr_p] = coor_x;
		line_y[Nr_p] = coor_y;
		if (Nr_p) { /* if not first point */
		    if (test_point (0, non_orth, line_x, line_y)) {
			/* If (new_coord == prev_coord)
			**    Finish this wire and start with a new one!
			*/
			nbr = NEXT_W;
			break;
		    }
		}
		if (++Nr_p == MaxPoints) { /* Auto finish this wire! */
auto_fini:
		    ptext ("Too many wire points!");
		    nbr = NEXT_W;
		    break;
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
	switch (nbr) {
	    case BACK:
		pre_cmd_proc (nbr);
		if (Nr_p > 1) {
		    --Nr_p;
		    set_c_wdw (PICT);
		    draw_lines (line_x + Nr_p - 1, line_y + Nr_p - 1, 2);
		    coor_x = line_x[Nr_p - 1];
		    coor_y = line_y[Nr_p - 1];
		}
		break;

	    case READY:
	    case NEXT_W:
	    case CANCEL:
		pre_cmd_proc (nbr);
		if (Nr_p > 1) { /* erase lines */
		    if (nbr != CANCEL) {
			/*
			** Test for defined layers.
			*/
			for (lay = 0; lay < NR_lay; ++lay) {
			    if (def_arr[lay]) break;
			}
			if (lay == NR_lay) {
			    btext ("Set your layer first!");
			    sleep (1);
			    post_cmd_proc (nbr);
			    goto do_menu;
			}
		    }
		    set_c_wdw (PICT);
		    draw_lines (line_x, line_y, Nr_p); /* erase */
		    if (nbr != CANCEL) {
			add_wire (line_x, line_y,
			    Nr_p, cur_wire_width, non_orth, wire_ext);
			if (nbr == NEXT_W) picture ();
		    }
		}
		Nr_p = 0;
		if (nbr == READY) {
		    erase_text = 1;
		    return;
		}
		post_cmd_proc (nbr);
		goto do_menu; /* start all over again */

	    case GRIDNR:
		toggle_grid ();
		picture ();
		break;

	    case 4: /* w_bbx */
		pre_cmd_proc (nbr);
		bound_w ();
		picture ();
		draw_lines (line_x, line_y, Nr_p);
		break;

	    case 5: /* w_prev */
		pre_cmd_proc (nbr);
		prev_w ();
		picture ();
		draw_lines (line_x, line_y, Nr_p);
		break;

	    case 6: /* w_center */
		pre_cmd_proc (nbr);
		ptext ("Select point to center!");
		new_cmd = get_one (Nr_p ? 4 : 1, &coor_x, &coor_y);
		if (new_cmd == -1) { /* coordinate selected */
		    center_w (coor_x, coor_y);
		    picture ();
		    if (Nr_p) {
			draw_lines (line_x, line_y, Nr_p);
			coor_x = line_x[Nr_p - 1];
			coor_y = line_y[Nr_p - 1];
		    }
		}
		else if (new_cmd == -2) goto do_menu;
		break;

	    case 7: /* zoom */
		pre_cmd_proc (nbr);
		cmd_nbr = -1;
		ask_curs_w ();
		if (new_cmd == -1) { /* coordinate selected */
		    picture ();
		    draw_lines (line_x, line_y, Nr_p);
		}
		else if (new_cmd == -2) goto do_menu;
		break;

	    case 8: /* dezoom */
		pre_cmd_proc (nbr);
		cmd_nbr = -1;
		ask_de_zoom ();
		if (new_cmd == -1) { /* coordinate selected */
		    picture ();
		    draw_lines (line_x, line_y, Nr_p);
		}
		else if (new_cmd == -2) goto do_menu;
		break;

	    case W_SET: /* wire_width */
		pre_cmd_proc (nbr);
		if (Nr_p) {
		    btext ("Width cannot be changed now!");
		    sleep (1);
		}
		else {
		    cur_wire_width = ask_wire_width ();
		}
		post_cmd_proc (nbr);
		goto do_menu;

	    case W_EXT: /* tgle_ext */
		wire_ext = !wire_ext;
		if (wire_ext)
		    pre_cmd_proc (nbr);
		else
		    post_cmd_proc (nbr);
		break;

	    case W_NOR: /* nor_toggle */
		if (!Nr_p) { /* only at begin of wire! */
		    non_orth = !non_orth;
		    if (non_orth)
			pre_cmd_proc (nbr);
		    else
			post_cmd_proc (nbr);
		}
		break;
	}

	if (nbr != W_EXT && nbr != W_NOR && nbr != GRIDNR) {
	    post_cmd_proc (nbr);
	    /*
	    ** switch directly back to wire mode if any of
	    ** the other commands has been executed, and
	    ** another command has not been reselected.
	    */
	}
    }
}

static Coor  *wire_width = NULL;
static char **ask_wire_str = NULL;
static int    no_wire_values = 0;

int default_wire_widths[] = {
	2, 4, 6, 8, 10, 12,
};

void set_wire_values (int no_wire_widths, int *wire_list)
{
    register int i;

    if (no_wire_values) {
	for (i = 0; i < no_wire_values; ++i) FREE (ask_wire_str[i + E_NR]);
	FREE (ask_wire_str);
	FREE (wire_width);
    }

    no_wire_values = no_wire_widths;
    MALLOCN (wire_width, Coor, no_wire_widths);
    MALLOCN (ask_wire_str, char *, no_wire_widths + E_NR);

    ask_wire_str[0] = "keyboard";
    ask_wire_str[1] = "interact";

    for (i = 0; i < no_wire_widths; ++i) {
	wire_width[i] = (Coor) wire_list[i] * QUAD_LAMBDA;
	MALLOCN (ask_wire_str[i + E_NR], char, 10);
	sprintf (ask_wire_str[i + E_NR], "%d", wire_list[i]);
    }
}

static int ask_wire_width ()
{
    char    tmp_str[MAXSTR];
    Coor    x1, x2, y1, y2, sel_width;
    int     a_old, value;
    register int old, nbr, new = -1;

    if (!ask_wire_str) {
	set_wire_values (sizeof (default_wire_widths) / sizeof (int), default_wire_widths);
    }

    for (old = 0; old < no_wire_values; ++old) {
	if (wire_width[old] == cur_wire_width) break;
    }
    old += E_NR;

    a_old = allow_keypress;
    allow_keypress = 0;
    menu (no_wire_values + E_NR, ask_wire_str);
    pre_cmd_proc (old);

    while (TRUE) {
	if (act_sngrid)
	    ptext ("Please specify even width!");
	else
	    ptext ("Please specify width!");
	if (new < 0)
	    nbr = get_com ();
	else {
	    nbr = new;
	    new = -1;
	}
	switch (nbr) {
	case 0: 		/* keyboard */
	    pre_cmd_proc (nbr);
	    while (!ask_string ("width: ", tmp_str)) ;
	    sscanf (tmp_str, "%d", &value);
	    sel_width = value * QUAD_LAMBDA;
	    break;
	case 1: 		/* interactive */
	    pre_cmd_proc (nbr);
	    ptext ("Please enter two points!");
	    if ((new = get_one (1, &x1, &y1)) >= 0) {
		/* (other) menu item selected */
		post_cmd_proc (nbr);
		continue;
	    }
	    x2 = x1;
	    y2 = y1;
	    if ((new = get_one (4, &x2, &y2)) >= 0) {
		/* (other) menu item selected */
		post_cmd_proc (nbr);
		continue;
	    }
	    /* two points have been entered */
	    if ((x2 -= x1) < 0) x2 = -x2;
	    if ((y2 -= y1) < 0) y2 = -y2;
	    sel_width = x2 > y2 ? x2 : y2;
	    break;
	default:
	    post_cmd_proc (old);
	    sel_width = wire_width[nbr - E_NR];
	    pre_cmd_proc (old = nbr);
	    break;
	}

	if (sel_width <= 0 || sel_width > MAX_W || (act_sngrid && (sel_width / QUAD_LAMBDA) % 2 == 1)) {
	    if (sel_width <= 0)
		ptext ("Positive width required!");
	    else if (sel_width > MAX_W)
		ptext ("Width too large!");
	    else
		ptext ("Snap-grid is active: even width required!");
	    sleep (1);
	    if (nbr < E_NR) new = nbr;
	}
	else break;
    }
    allow_keypress = a_old;
    return (sel_width);
}
