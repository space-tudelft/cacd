/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	P. van der Wolf
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

#define GRIDNR 1

extern int  tracker_mode;
extern int  allow_keypress;
extern int  ask_iname;
extern int  cmd_nbr, in_cmd, new_cmd, grid_cmd;
extern int  erase_text;
extern int  exp_level;
extern int  exp_ask_coord;
extern int  gridflag;
extern int  use_new_mode;
extern int *vis_arr;
extern int  v_label, v_comment;
extern int  zoom_mode;
extern int  rmode;
extern int  Draw_dominant;
extern int  Draw_hashed;
extern Coor xlc, xrc, ybc, ytc; /* cursor parameters */

extern char *yes_no[];

int dom_order_cmd = 0;

static char *cmd[] = {
     /* 0 */ "-quit-",
     /* 1 */ "visible",
     /* 2 */ "DB_menu",
     /* 3 */ "box_menu",
     /* 4 */ "term_menu",
     /* 5 */ "inst_menu",
     /* 6 */ "DRC_menu",
     /* 7 */ "annotate",
     /* 8 */ "info_menu",
     /* 9 */ "settings",
};

static char *dm_c[] = {
     /* 0 */ "-return-",
     /* 1 */ "read_cell",
     /* 2 */ "write_cell",
     /* 3 */ "erase_all",
     /* 4 */ "indiv_exp",
     /* 5 */ "all_exp",
};

static char *inst_c[] = {
     /* 0 */ "-return-",
     /* 1 */ "grid",
     /* 2 */ "[bbox]",
     /* 3 */ "[prev]",
     /* 4 */ "[center]",
     /* 5 */ "[zoom]",
     /* 6 */ "[dezoom]",
     /* 7 */ "add_inst",
     /* 8 */ "del_inst",
     /* 9 */ "mov_inst",
     /*10 */ "mir_inst",
     /*11 */ "rot_inst",
     /*12 */ "array_inst",
     /*13 */ "name_inst",
     /*14 */ "show_inst",
     /*15 */ "show_tree",
     /*16 */ "upd_bbox",
};

static char *term_c[] = {
     /* 0 */ "-return-",
     /* 1 */ "grid",
     /* 2 */ "[bbox]",
     /* 3 */ "[prev]",
     /* 4 */ "[center]",
     /* 5 */ "[zoom]",
     /* 6 */ "[dezoom]",
     /* 7 */ "add_term",
     /* 8 */ "del_term",
     /* 9 */ "del_area",
     /*10 */ "yank_area",
     /*11 */ "put_buf",
     /*12 */ "mov_term",
     /*13 */ "array_term",
     /*14 */ "lift_tms",
};

static char *box_c[] = {
     /* 0 */ "-return-",
     /* 1 */ "grid",
     /* 2 */ "[bbox]",
     /* 3 */ "[prev]",
     /* 4 */ "[center]",
     /* 5 */ "[zoom]",
     /* 6 */ "[dezoom]",
     /* 7 */ "x,y,mskEXP",
     /* 8 */ "x,y,mskTOP",
     /* 9 */ "add_box",
     /*10 */ "del_box",
     /*11 */ "add_poly",
     /*12 */ "del_poly",
     /*13 */ "add_wire",
     /*14 */ "yank",
     /*15 */ "put",
     /*16 */ "check",
};

static char *info_c[] = {
     /* 0 */ "-return-",
     /* 1 */ "window",
     /* 2 */ "process",
     /* 3 */ "cell",
     /* 4 */ "S-o-Gates",
};

static char *set_c[] = {
     /* 0 */ "-return-",
     /* 1 */ "grid",
     /* 2 */ "disp_grid",
     /* 3 */ "snap_grid",
     /* 4 */ "zoom mode",
     /* 5 */ "new mode",
     /* 6 */ "hash mode",
     /* 7 */ "set_fillst",
     /* 8 */ "set_color",
     /* 9 */ "unset_order",
     /*10 */ "set_order",
     /*11 */ "draw mode",
     /*12 */ "def_level",
     /*13 */ "ask_inst",
     /*14 */ "tracker",
     /*15 */ "[load]",
     /*16 */ "[save]",
};

static char *chk_c[] = {
     /* 0 */ "-return-",
     /* 1 */ "grid",
     /* 2 */ "[bbox]",
     /* 3 */ "[prev]",
     /* 4 */ "[center]",
     /* 5 */ "[zoom]",
     /* 6 */ "[dezoom]",
     /* 7 */ "x,y,masks",
     /* 8 */ "ind_err",
     /* 9 */ "nxt_in_w",
     /*10 */ "nxt_err",
     /*11 */ "error",
     /*12 */ "chk_file",
     /*13 */ "do_check",
};

static char *comment_c[] = {
     /* 0 */ "-return-",
     /* 1 */ "grid",
     /* 2 */ "[bbox]",
     /* 3 */ "[prev]",
     /* 4 */ "[center]",
     /* 5 */ "[zoom]",
     /* 6 */ "[dezoom]",
     /* 7 */ "--------",
     /* 8 */ "------->",
     /* 9 */ "<-------",
     /*10 */ "<------>",
     /*11 */ "....    ",
     /*12 */ "    ....",
     /*13 */ "  ....  ",
     /*14 */ "label",
     /*15 */ "DELETE",
};

static void ask_center_w (void);
static void box_cmd (void);
static void chk_cmd (void);
static void comment_cmd (void);
static void dm_cmd (void);
static void info_cmd (void);
static void inst_cmd (void);
static void set_cmd (void);
static void term_cmd (void);

void main_menu ()
{
    int nbr_cmds = sizeof (cmd) / sizeof (char *);
    menu (nbr_cmds, cmd);
}

void command ()
{
    int nbr_cmds = sizeof (cmd) / sizeof (char *);

 // pict_all (SKIP); /* SKIP is default mode */
    menu (nbr_cmds, cmd);
    picture ();	/* flush graphics */

    animate ();
    toggle_tracker ();
    ptext ("the Delft Advanced Layout Interface is ready!");

    while (TRUE) {
	menu (nbr_cmds, cmd);
	get_cmd ();
	pre_cmd_proc (cmd_nbr);
	switch (cmd_nbr) {
	    case -1: /* special key pressed! */
		continue;
	    case 0:
		ask_quit ();
		break;
	    case 1:
		Visible ();
		break;
	    case 2:
		ptext ("<= Database Menu");
		dm_cmd ();
		break;
	    case 3:
		ptext ("<= Box Menu");
		box_cmd ();
		break;
	    case 4:
		ptext ("<= Terminal Menu");
		term_cmd ();
		break;
	    case 5:
		ptext ("<= Instance Menu");
		inst_cmd ();
		break;
	    case 6:
		ptext ("<= Designrule Check Menu");
		chk_cmd ();
		break;
	    case 7:
		ptext ("<= Annotate Menu");
		comment_cmd ();
		break;
	    case 8:
		ptext ("<= Information Menu");
		info_cmd ();
		break;
	    case 9:
		ptext ("<= Settings Menu");
		set_cmd ();
		break;
	}
	ptext ("<= Main Menu");
    }
}

static void chk_cmd ()
{
    while (TRUE) {
	menu (sizeof (chk_c) / sizeof (char *), chk_c);
	grid_cmd = GRIDNR;
	if (gridflag) pre_cmd_proc (GRIDNR);
	get_cmd ();
	pre_cmd_proc (cmd_nbr);
	switch (cmd_nbr) {
	    case 0:
		grid_cmd = 0;
		return;
	    case 1:
		toggle_grid ();
		break;
	    case 2:
		bound_w ();
		break;
	    case 3:
		prev_w ();
		break;
	    case 4:
		ask_center_w ();
		break;
	    case 5:
		ask_curs_w ();
		break;
	    case 6:
		ask_de_zoom ();
		break;
	    case 7:
		exp_ask_coord = 0;
		ask_coordnts ();
		break;
	    case 8:
		ptext ("Select coordinate point of error!");
		erase_text = 1;
		if ((new_cmd = get_cursor (1, 1, NO_SNAP)) == -1) {
		    ind_err (xlc, ybc);
		}
		break;
	    case 9:
		/* next error in current window */
		disp_next (1); /* YES: inside_window */
		break;
	    case 10:
		/* next error in possibly different window */
		disp_next (0); /* NO: !inside_window */
		break;
	    case 11:
		toggle_drc_err ();
		break;
	    case 12:
		chk_file ();
		break;
	    case 13:
		check ();
		break;
	}
	picture ();
	if (cmd_nbr != GRIDNR) post_cmd_proc (cmd_nbr);
    }
}

static void dm_cmd ()
{
    while (TRUE) {
	menu (sizeof (dm_c) / sizeof (char *), dm_c);
	get_cmd ();
	pre_cmd_proc (cmd_nbr);
	switch (cmd_nbr) {
	    case 0:
		return;
	    case 1:
		inp_mod (NULL);
		break;
	    case 2:
		wrte_cell ();
		set_titlebar (NULL);
		break;
	    case 3:
		eras_worksp (NULL);
		set_titlebar (NULL);
		break;
	    case 4:
		indiv_exp ();
		break;
	    case 5:
		if (expansion (-1)) {
		    upd_boundb ();
		    inform_cell ();
		    set_titlebar (NULL);
		}
		break;
	}
	picture ();
	post_cmd_proc (cmd_nbr);
    }
}

static void inst_cmd ()
{
    while (TRUE) {
	menu (sizeof (inst_c) / sizeof (char *), inst_c);
	grid_cmd = GRIDNR;
	if (gridflag) pre_cmd_proc (GRIDNR);
	get_cmd ();
	pre_cmd_proc (cmd_nbr);
	switch (cmd_nbr) {
	    case 0:
		grid_cmd = 0;
		return;
	    case 1:
		toggle_grid ();
		break;
	    case 2:
		bound_w ();
		break;
	    case 3:
		prev_w ();
		break;
	    case 4:
		ask_center_w ();
		break;
	    case 5:
		ask_curs_w ();
		break;
	    case 6:
		ask_de_zoom ();
		break;
	    case 7:
		grid_cmd = 0;
		add_inst ();
		break;
	    case 8:
		del_inst ();
		break;
	    case 9:
		mov_inst ();
		break;
	    case 10:
		mir_inst ();
		break;
	    case 11:
		rot_inst ();
		break;
	    case 12:
		arr_par ();
		break;
	    case 13:
		set_inst_name ();
		break;
	    case 14:
		set_actinst (0);
		break;
	    case 15:
		set_actinst (1);
		break;
	    case 16:
		upd_inst_bbox ();
		break;
	}
	picture ();
	if (cmd_nbr != GRIDNR) post_cmd_proc (cmd_nbr);
    }
}

static void term_cmd ()
{
    while (TRUE) {
	menu (sizeof (term_c) / sizeof (char *), term_c);
	grid_cmd = GRIDNR;
	if (gridflag) pre_cmd_proc (GRIDNR);
	get_cmd ();
	pre_cmd_proc (cmd_nbr);
	switch (cmd_nbr) {
	    case 0:
		grid_cmd = 0;
		return;
	    case 1:
		toggle_grid ();
		break;
	    case 2:
		bound_w ();
		break;
	    case 3:
		prev_w ();
		break;
	    case 4:
		ask_center_w ();
		break;
	    case 5:
		ask_curs_w ();
		break;
	    case 6:
		ask_de_zoom ();
		break;
	    case 7:
		add_term ();
		break;
	    case 8:
		del_term ();
		break;
	    case 9:
		fill_tbuf ();
		break;
	    case 10:
		yank_tbuf ();
		break;
	    case 11:
		grid_cmd = 0;
		put_buffer (TRUE);
		break;
	    case 12:
		mov_term ();
		break;
	    case 13:
		t_arr_par ();
		break;
	    case 14:
		lift_terms ();
		break;
	}
	picture ();
	if (cmd_nbr != GRIDNR) post_cmd_proc (cmd_nbr);
    }
}

static void box_cmd ()
{
    while (TRUE) {
	menu (sizeof (box_c) / sizeof (char *), box_c);
	grid_cmd = GRIDNR;
	if (gridflag) pre_cmd_proc (GRIDNR);
	get_cmd ();
	pre_cmd_proc (cmd_nbr);
	switch (cmd_nbr) {
	    case 0:
		grid_cmd = 0;
		return;
	    case 1:
		toggle_grid ();
		break;
	    case 2:
		bound_w ();
		break;
	    case 3:
		prev_w ();
		break;
	    case 4:
		ask_center_w ();
		break;
	    case 5:
		ask_curs_w ();
		break;
	    case 6:
		ask_de_zoom ();
		break;
	    case 7:
		exp_ask_coord = 1;
		ask_coordnts ();
		break;
	    case 8:
		exp_ask_coord = 0;
		ask_coordnts ();
		break;
	    case 9:
		ptext ("Enter two points to add box!");
		erase_text = 1;
		in_cmd = cmd_nbr;
		if ((new_cmd = get_cursor (5, 2, SNAP)) == -1) {
		    addel_cur (xlc, xrc, ybc, ytc, ADD);
		    new_cmd = cmd_nbr;
		}
		in_cmd = 0;
		break;
	    case 10:
		ptext ("Enter two points to delete area!");
		erase_text = 1;
		in_cmd = cmd_nbr;
		if ((new_cmd = get_cursor (5, 2, SNAP)) == -1) {
		    addel_cur (xlc, xrc, ybc, ytc, DELETE);
		    new_cmd = cmd_nbr;
		}
		in_cmd = 0;
		break;
	    case 11:
		add_del_poly (ADD);
		break;
	    case 12:
		add_del_poly (DELETE);
		break;
	    case 13:
		wire_points ();
		break;
	    case 14:
		ptext ("Select area to yank!");
		erase_text = 1;
		if ((new_cmd = get_cursor (5, 2, SNAP)) == -1)
		    fill_buffer (xlc, xrc, ybc, ytc);
		break;
	    case 15:
		grid_cmd = 0;
		put_buffer (FALSE);
		break;
	    case 16:
		area_check ();
		break;
	}
	picture ();
	if (cmd_nbr != GRIDNR) post_cmd_proc (cmd_nbr);
    }
}

static void info_cmd ()
{
    while (TRUE) {
	menu (sizeof (info_c) / sizeof (char *), info_c);
	get_cmd ();
	pre_cmd_proc (cmd_nbr);
	switch (cmd_nbr) {
	    case 0:
		return;
	    case 1:
		inform_window ();
		break;
	    case 2:
		inform_process ();
		break;
	    case 3:
		upd_boundb ();
		inform_cell ();
		break;
	    case 4:
		inform_SofG ();
		break;
	}
	post_cmd_proc (cmd_nbr);
    }
}

void set_dominant ()
{
    Draw_dominant = !Draw_dominant;
    if (Draw_dominant)
	ptext ("Drawing layers 'dominant'");
    else
	ptext ("Drawing layers 'transparent'");
    pict_all (ERAS_DR);
    init_colmenu ();
}

void set_hashed ()
{
    Draw_hashed = !Draw_hashed;
    if (present_inst ()) {
	if (exp_level > 1) pict_all (ERAS_DR);
    }
    else
	sleep (1);
    if (Draw_hashed)
	ptext ("Drawing instances 'hashed'");
    else
	ptext ("Drawing instances 'normal'");
}

#define UNSET_ORDER  9
#define SET_ORDER   10

static void set_cmd ()
{
    static char *zoom_c[] = {
	 /* 0 */ "fixed",
	 /* 1 */ "point",
	 /* 2 */ "area",
    };
    static char *track_c[] = {
	 /* 0 */ "off",
	 /* 1 */ "on",
	 /* 2 */ "auto",
    };
    char mess_str[MAXCHAR];
    int  a_old, rv;

    while (TRUE) {
	menu (sizeof (set_c) / sizeof (char *), set_c);
	grid_cmd = GRIDNR;
	if (gridflag) pre_cmd_proc (GRIDNR);
	get_cmd ();
	if (dom_order_cmd) {
	    if (dom_order_cmd < UNSET_ORDER && cmd_nbr == -7) {
		if (dom_order_cmd == 7)
		    fillst_menu ();
		else
		    color_menu ();
		cmd_nbr = -1;
	    }
	    else {
		if (dom_order_cmd > SET_ORDER) dom_order_cmd = SET_ORDER;
		post_cmd_proc (dom_order_cmd);
	    }
	    dom_order_cmd = 0;
	}
	pre_cmd_proc (cmd_nbr);
	switch (cmd_nbr) {
	    case 0:
		grid_cmd = 0;
		return;
	    case 1:
		toggle_grid ();
		break;
	    case 2:
		grid_cmd = 0;
		set_gr_spac ();
		break;
	    case 3:
		grid_cmd = 0;
		set_sn_gr_spac ();
		break;
	    case 4:
		a_old = allow_keypress;
		allow_keypress = 0;
		do {
		    ptext ("Select zooming mode!");
		    rv = ask (sizeof (zoom_c) / sizeof (char *), zoom_c, zoom_mode);
		} while (rv < 0);
		sprintf (mess_str, "Selected '%s' zooming mode", zoom_c[zoom_mode = rv]);
		ptext (mess_str);
		allow_keypress = a_old;
		break;
	    case 5:
		use_new_mode = !use_new_mode;
		if (use_new_mode)
		    ptext ("Use new cell name after write");
		else
		    ptext ("Use old cell name after write");
		break;
	    case 6:
		set_hashed ();
		break;
	    case 7:
	    case 8:
		dom_order_cmd = cmd_nbr;
		if (nr_of_selected_lays () != 1) {
		    if (dom_order_cmd == 7)
			ptext ("Toggle lay to set fill style!");
		    else
			ptext ("Toggle lay to set color!");
		}
		else {
		    if (dom_order_cmd == 7)
			fillst_menu ();
		    else
			color_menu ();
		}
		break;
	    case UNSET_ORDER:
	    case SET_ORDER:
		if (cmd_nbr == SET_ORDER)
		    ptext ("Toggle lay to set in dominant order position!");
		else
		    ptext ("Toggle lay to unset dominant order position!");
		dom_order_cmd = cmd_nbr;
		if (!Draw_dominant) {
		    Draw_dominant = 1;
		    pict_all (ERAS_DR);
		    init_colmenu ();
		}
		break;
	    case 11:
		set_dominant ();
		break;
	    case 12:
		def_level ();
		break;
	    case 13:
		ask_iname = !ask_iname;
		if (ask_iname)
		    ptext ("Ask instance name by add_inst");
		else
		    ptext ("Don't ask instance name by add_inst");
		break;
	    case 14:
		a_old = allow_keypress;
		allow_keypress = 0;
		do {
		    ptext ("Select tracker mode!");
		    rv = ask (sizeof (track_c) / sizeof (char *), track_c, tracker_mode);
		} while (rv < 0);
		sprintf (mess_str, "Selected '%s' tracker mode", track_c[tracker_mode = rv]);
		ptext (mess_str);
		toggle_tracker ();
		allow_keypress = a_old;
		break;
	    case 15: /* load */
		load_settings (".dalisave");
		break;
	    case 16: /* save */
		save_settings ();
		break;
	}
	picture ();
	if (cmd_nbr != GRIDNR && cmd_nbr != dom_order_cmd) post_cmd_proc (cmd_nbr);
    }
}

static void comment_cmd ()
{
    while (TRUE) {
	menu (sizeof (comment_c) / sizeof (char *), comment_c);
	grid_cmd = GRIDNR;
	if (gridflag) pre_cmd_proc (GRIDNR);
	get_cmd ();
	pre_cmd_proc (cmd_nbr);
	switch (cmd_nbr) {
	    case 0:
		grid_cmd = 0;
		return;
	    case 1:
		toggle_grid ();
		break;
	    case 2:
		bound_w ();
		break;
	    case 3:
		prev_w ();
		break;
	    case 4:
		ask_center_w ();
		break;
	    case 5:
		ask_curs_w ();
		break;
	    case 6:
		ask_de_zoom ();
		break;
	    case 7: 		/* ----- */
	    case 8: 		/* -----> */
	    case 9: 		/* <----- */
	    case 10: 		/* <-----> */
		if (!vis_arr[v_comment]) {
		    btext ("Comments not visible!");
		    break;
		}
		ptext ("Enter points to add line!");
	        if ((new_cmd = get_cursor (4, 2, SNAP)) == -1) {
		    if (cmd_nbr == 7)
			c_line (xlc, ybc, xrc, ytc, GA_NO_ARROW);
		    else if (cmd_nbr == 8)
			c_line (xlc, ybc, xrc, ytc, GA_FW_ARROW);
		    else if (cmd_nbr == 9)
			c_line (xlc, ybc, xrc, ytc, GA_BW_ARROW);
		    else
			c_line (xlc, ybc, xrc, ytc, GA_DB_ARROW);
		    new_cmd = cmd_nbr;
	        }
		break;
	    case 11: 		/* right_adjusted_text  */
	    case 12: 		/* left_adjusted_text */
	    case 13: 		/* center_text */
		if (!vis_arr[v_comment]) {
		    btext ("Comments not visible!");
		    break;
		}
		ptext ("Enter point to add text!");
		if ((new_cmd = get_cursor (1, 1, SNAP)) == -1) {
		    if (cmd_nbr == 11)
			c_text (xlc, ybc, GA_RIGHT);
		    else if (cmd_nbr == 12)
			c_text (xlc, ybc, GA_LEFT);
		    else
			c_text (xlc, ybc, GA_CENTER);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 14: 		/* LABEL */
		if (!vis_arr[v_label]) {
		    btext ("Labels not visible!");
		    break;
		}
		ptext ("Enter point to add label!");
		if ((new_cmd = get_cursor (1, 1, SNAP)) == -1) {
		    c_label (xlc, ybc, GA_LEFT);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 15: 		/* DELETE */
		ptext ("Enter point to delete annotation!");
		if ((new_cmd = get_cursor (1, 1, SNAP)) == -1) {
		    del_comment (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	}
	picture ();
	if (cmd_nbr != GRIDNR) post_cmd_proc (cmd_nbr);
    }
}

static void ask_center_w ()
{
    ptext ("Select point to center!");
    erase_text = 1;
    in_cmd = cmd_nbr;
    if ((new_cmd = get_cursor (1, 1, NO_SNAP)) == -1) {
	center_w (xlc, ybc);
	new_cmd = cmd_nbr;
    }
    in_cmd = 0;
}

void ask_curs_w ()
{
    if (zoom_mode == 2)
	ptext ("Select area to zoom!");
    else if (zoom_mode == 1)
	ptext ("Select point to zoom!");
    else {
	curs_w (xlc, xrc, ybc, ytc);
	return;
    }
    erase_text = 1;
    in_cmd = cmd_nbr;
    if ((new_cmd = get_cursor (5, zoom_mode, NO_SNAP)) == -1) {
	curs_w (xlc, xrc, ybc, ytc);
	new_cmd = cmd_nbr;
    }
    in_cmd = 0;
}

void ask_de_zoom ()
{
    if (zoom_mode == 2)
	ptext ("Select area to dezoom!");
    else if (zoom_mode == 1)
	ptext ("Select point to dezoom!");
    else {
	de_zoom (xlc, xrc, ybc, ytc);
	return;
    }
    erase_text = 1;
    in_cmd = cmd_nbr;
    if ((new_cmd = get_cursor (5, zoom_mode, NO_SNAP)) == -1) {
	de_zoom (xlc, xrc, ybc, ytc);
	new_cmd = cmd_nbr;
    }
    in_cmd = 0;
}

void ask_quit ()
{
    if (!rmode) {
	ptext ("Quit, are you sure? (data lost if not written)");
	if (ask (2, yes_no, -1) > 0) stop_show (0);
    }
    else {
	stop_show (0);
    }
}
