/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	Pieter van der Wolf
 *	Patrick Groeneveld
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

#define COMM_CMD  15
#define NBR_CMDS  9
#define DM_CMD    16
#define INST_CMD  16
#define PATRICK_INST_CMD 19
#define TERM_CMD  15
#define BOX_CMD   16
#define TOOLS_CMD 14
#define AUX_CMD   13
#define CHK_CMD   15

extern int Draw_dominant, Draw_hashed, ImageMode, BackingStore;
extern int Default_expansion_level;
extern INST *act_inst, *inst_root;
extern char ImageInstName[];
extern char *ThisImage;
extern int  NR_lay;
extern int  Sub_terms; /* TRUE to indicate sub terminals */
extern char cirname[];

extern int *def_arr;
extern int *edit_arr;
extern int *pict_arr;
extern int  Textnr;
extern int  new_cmd;
extern int  cmd_nbr;
extern int  dirty;
extern Coor xlc, xrc, ybc, ytc; /* cursor parameters */
extern char *yes_no[];

static int aux_menu = 0;

char *no_yes[] = {
/* 0 */ "&no",
/* 1 */ "&yes",
};

static char * cmd[] = {
        /* 0 */ "-&quit-",
	/* 1 */ "&?&? help &?&?",
	/* 2 */ "visible\nmasks",
	/* 3 */ "settings",
	/* 4 */ "terminals",
	/* 5 */ "boxes",
	/* 6 */ "instances",
        /* 7 */ "automatic\ntools",
	/* 8 */ "database",
	/* 9 */ "annotate",
};

static char * dm_c[] = {
	/* 0 */ "-return-",
	/* 1 */ "$1show &grid",
	/* 2 */ "$1&bounding box\nwindow",
	/* 3 */ "$1&previous\nwindow",
	/* 4 */ "$1&center\nwindow",
	/* 5 */ "$1zoom &in",
	/* 6 */ "$1zoom &out",
	/* 7 */ "show &Sub\nterminals",
	/* 8 */ "Expand\ninstance",
	/* 9 */ "Expand all\n&1,&2,&3,..&9,&0",
	/*10 */ "$2>>$9 Fish -i $2>>$9\nno image",
	/*11 */ "$2>>$9  Fish  $2>>",
	/*12 */ "Place and\n$2O>$9  Route  $2)>",
	/*13 */ " NEW ---> ",
	/*14 */ "<--- WRITE",
	/*15 */ "---> READ ",
};

static char * inst_c[] = {
	/* 0 */ "-return-",
	/* 1 */ "$1show &grid",
	/* 2 */ "$1&bounding box\nwindow",
	/* 3 */ "$1&previous\nwindow",
	/* 4 */ "$1&center\nwindow",
	/* 5 */ "$1zoom &in",
	/* 6 */ "$1zoom &out",
	/* 7 */ "add_imp",
	/* 8 */ "add_inst",
	/* 9 */ "del_inst",
	/*10 */ "mov_inst",
	/*11 */ "mir_inst",
	/*12 */ "rot_inst",
	/*13 */ "arr_par",
	/*14 */ "set_inst",
	/*15 */ "upd_bbox",
};

static char *patrick_inst_c[] = {
	/* 0 */ "-return-",
	/* 1 */ "$1old menu",
	/* 2 */ "$1&bounding box\nwindow",
	/* 3 */ "$1&previous\nwindow",
	/* 4 */ "$1&center\nwindow",
	/* 5 */ "$1zoom &in",
	/* 6 */ "$1zoom &out",
	/* 7 */ "ADD imported\ninstance",
	/* 8 */ "ADD\ninstance",
	/* 9 */ "DELETE\ninstance",
	/*10 */ "move",
	/*11 */ "mirror",
	/*12 */ "rotate",
	/*13 */ "set array\nparameters",
	/*14 */ "set instance\nname",
	/*15 */ "multiply xl",
	/*16 */ "update\nbounding box",
	/*17 */ "$2>>$9 Fish  $2>>",
	/*18 */ "$2O>$9 Madonna $2O>",
};

static char *tools_c[] = {
        /* 0 */ "-return-",
	/* 1 */ "$1&bounding box\nwindow",
	/* 2 */ "$1&previous\nwindow",
	/* 3 */ "$1&center\nwindow",
	/* 4 */ "$1zoom &in",
	/* 5 */ "$1zoom &out",
	/* 6 */ "DRC menu",
	/* 7 */ "$2#$9 Print $2#$9\nhardcopy",
	/* 8 */ "$2>>$9 Fish  $2>>",
	/* 9 */ "$2>>$9 Check $2>>\n$9nets",
	/*10 */ "$2)>$9 Trout $2)>",
	/*11 */ "$2O>$9 Madonna $2O>",
	/*12 */ "Place and\n$2O>$9  Route  $2)>",
        /*13 */ "Highlight\n$2!!$9  Net  $2!!",
};

static char *term_c[] = {
	/* 0 */ "-return-",
	/* 1 */ "$1show &grid",
	/* 2 */ "$1&bounding box\nwindow",
	/* 3 */ "$1&previous\nwindow",
	/* 4 */ "$1&center\nwindow",
	/* 5 */ "$1zoom &in",
	/* 6 */ "$1zoom &out",
	/* 7 */ "ADD\nterminal",
	/* 8 */ "DELETE\nterminal",
	/* 9 */ "DELETE terms\nin box",
	/*10 */ "put terminal\nbuffer",
	/*11 */ "move\nterminal",
	/*12 */ "set array\nparameters",
	/*13 */ "set \nterminal",
	/*14 */ "lift\nterminals",
};

static char * box_c[] = {
	/* 0 */ "-return-",
	/* 1 */ "$1show &grid",
	/* 2 */ "$1&bounding box\nwindow",
	/* 3 */ "$1&previous\nwindow",
	/* 4 */ "$1&center\nwindow",
	/* 5 */ "$1zoom &in",
	/* 6 */ "$1zoom &out",
	/* 7 */ "45 degree\nstyle",
	/* 8    "coordi-\nnates", */
	/* 8 */ "APPEND",
	/* 9 */ "DELETE",
	/*10 */ "yank",
	/*11 */ "put",
	/*12 */ "wire",
	/*13 */ "$2>>$9 Check $2>>\n$9nets",
	/*14 */ "$2)>$9 Trout $2)>",
	/*15 */ "$2>>$9 Fish $2>>",
};

static char * image_box_c[] = {
        /* 0 */ "-return-",
	/* 1 */ "$1show &grid",
	/* 2 */ "$1&bounding box\nwindow",
	/* 3 */ "$1&previous\nwindow",
	/* 4 */ "$1&center\nwindow",
	/* 5 */ "$1zoom &in",
	/* 6 */ "$1zoom &out",
	/* 7 */ "$1&redraw\nscreen",
	/* 8    "coordi-\nnates", */
	/* 8 */ "APPEND",
	/* 9 */ "DELETE",
	/*10 */ "yank",
	/*11 */ "put",
	/*12 */ "wire",
	/*13 */ "$2>>$9 Check $2>>\n$9nets",
	/*14 */ "$2)>$9 Trout $2)>",
	/*15 */ "$2>>$9 Fish $2>>",
};

static char *aux_c[] = {
	/* 0 */ "-return-",
	/* 1 */ "$1show &grid",
	/* 2 */ "disp_grid",
	/* 3 */ "snap_grid",
	/* 4 */ "window",
	/* 5 */ "process",
	/* 6 */ "act term",
	/* 7 */ "backing\nstore",
	/* 7    "edit cell", */
	/* 8 */ "image mode",
	/* 9 */ "&Draw\ndominant",
	/*10 */ "&draw\nhashed",
	/*11 */ "check menu",
	/*12 */ "&tracker\nwindow"
};

static char *chk_c[] = {
	/* 0 */ "-return-",
	/* 1 */ "$1show &grid",
	/* 2 */ "$1&bounding box\nwindow",
	/* 3 */ "$1&previous\nwindow",
	/* 4 */ "$1&center\nwindow",
	/* 5 */ "$1zoom &in",
	/* 6 */ "$1zoom &out",
	/* 7 */ "coordnts",
	/* 8 */ "ind_err",
	/* 9 */ "nxt_in_w",
	/*10 */ "nxt_err",
	/*11 */ "tgle_err",
	/*12 */ "chk_file",
	/*13 */ "do_check",
	/*14 */ "simple_chk", /* PATRICK: moved from box menu to here */
};

static void aux_cmd (void);
static void box_cmd (void);
static void chk_cmd (void);
static void dm_cmd  (void);
static void inst_cmd (void);
static void patrick_inst_cmd (void);
static void term_cmd (void);
static void tools_cmd (void);
static void comment_cmd (void);

static char *comment_c[] = {
	/* 0 */ "-return-",
	/* 1 */ "$1show &grid",
	/* 2 */ "$1&bounding box\nwindow",
	/* 3 */ "$1&previous\nwindow",
	/* 4 */ "$1&center\nwindow",
	/* 5 */ "$1zoom &in",
	/* 6 */ "$1zoom &out",
	/* 7 */ "--------",
	/* 8 */ "------->",
	/* 9 */ "<-------",
	/*10 */ "<------>",
	/*11 */ "....    ",
	/*12 */ "    ....",
	/*13 */ "  ....  ",
	/*14 */ "DELETE",
};

void command ()
{
    register int j;

    /*
    ** Set the arrays, which determine the layers that
    ** are involved in an operation, to zero.
    */
    for (j = 0; j < NR_lay; ++j) {
	def_arr[j] = FALSE;
	edit_arr[j] = TRUE;
    }
    pict_all (SKIP);

    Sub_terms = FALSE;

    picture ();			/* draws a rectangle, put flags right */

    /*
    * load all the rc files: .dalirc and image.seadif
    */
    load_rc_files ();

    dirty = FALSE;

    /*
    ** In the next section of the program the commands
    ** will be decoded from the command_areas pointed to
    ** at the screen.
    */
    while (TRUE) {
	menu (NBR_CMDS + 1, cmd);
	get_cmd ();
	switch (cmd_nbr) {
	case -2:
	    break;
	case 0:
	    ask_quit (1);
	    break;
	case 1:
	    pre_cmd_proc (cmd_nbr, cmd);
	    ptext ("Help is on its way to you, starting browser...");
	    start_browser ("help");
	    sleep (2);
	    post_cmd_proc (cmd_nbr, cmd);
	    break;
	case 2:
	    Visible ();
	    break;
	case 8:
	    dm_cmd ();
	    break;
	case 5:
	    box_cmd ();
	    break;
	case 4:
	    term_cmd ();
	    break;
	case 7:
	    tools_cmd ();
	    break;
	case 6:
	    patrick_inst_cmd ();
	    break;
	case 3:
	    aux_cmd ();
	    break;
	case 9:
	    comment_cmd ();
	    break;
	default:
	    ptext ("Command not implemented!");
	    sleep (2);
	}
	ptext ("This is the main menu: please select one of the sub menu's");
    }
}

static void chk_cmd ()
{
chk_menu_txt:
    ptext ("This is the check menu: please select a command");

    while (TRUE) {
	menu (CHK_CMD, chk_c);
	get_cmd ();
	switch (cmd_nbr) {
	    case -2:
		goto chk_menu_txt;
	    case 0:
		return;
	    case 1:
		pre_cmd_proc (cmd_nbr, chk_c);
		toggle_grid ();
		break;
	    case 2:
		pre_cmd_proc (cmd_nbr, chk_c);
		bound_w ();
		break;
	    case 3:
		pre_cmd_proc (cmd_nbr, chk_c);
		prev_w ();
		break;
	    case 4:
		pre_cmd_proc (cmd_nbr, chk_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    center_w (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 5:
		pre_cmd_proc (cmd_nbr, chk_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    curs_w (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 6:
		pre_cmd_proc (cmd_nbr, chk_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    de_zoom (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 7:
		pre_cmd_proc (cmd_nbr, chk_c);
		if ((new_cmd = set_tbltcur (1, SNAP)) == -1) {
		    track_coord (xlc, ybc, FALSE);
		    new_cmd = cmd_nbr;
		}
		else { /* other command selected: erase old cross */
		    track_coord (0, 0, TRUE);
		}
		break;
	    case 8:
		pre_cmd_proc (cmd_nbr, chk_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    ind_err (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 9:
		pre_cmd_proc (cmd_nbr, chk_c);
		disp_next (TRUE); /* next error in current window */
		break;
	    case 10:
		pre_cmd_proc (cmd_nbr, chk_c);
		disp_next (FALSE); /* next error in possibly different window */
		break;
	    case 11:
		pre_cmd_proc (cmd_nbr, chk_c);
		toggle_drc_err ();
		break;
	    case 12:
		pre_cmd_proc (cmd_nbr, chk_c);
		chk_file ();
		break;
	    case 13:
		pre_cmd_proc (cmd_nbr, chk_c);
		check ();
		break;
	     case 14: /* is one is the simple check */
		pre_cmd_proc (cmd_nbr, chk_c);
		area_check ();
		break;
	    default:
		ptext ("Command not implemented!");
	}
	picture ();
	post_cmd_proc (cmd_nbr, chk_c);
	if (new_cmd == -2) goto chk_menu_txt;
    }
}

static void dm_cmd ()
{
dm_menu_txt:
    ptext ("This is the database menu: please select a command");

    while (TRUE) {
	menu (DM_CMD, dm_c);
	if (Sub_terms == TRUE) pre_cmd_proc ( 7, dm_c);

	get_cmd ();
	switch (cmd_nbr) {
	    case -2:
		goto dm_menu_txt;
	    case 0:
		return;
	    case 1:
		pre_cmd_proc (cmd_nbr, dm_c);
		toggle_grid ();
		break;
	    case 2:
		pre_cmd_proc (cmd_nbr, dm_c);
		bound_w ();
		break;
	    case 3:
		pre_cmd_proc (cmd_nbr, dm_c);
		prev_w ();
		break;
	    case 4:
		pre_cmd_proc (cmd_nbr, dm_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    center_w (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 5:
		pre_cmd_proc (cmd_nbr, dm_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    curs_w (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 6:
		pre_cmd_proc (cmd_nbr, dm_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    de_zoom (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 14: /* write */
		pre_cmd_proc (cmd_nbr, dm_c);
		wrte_cell ();
		set_titlebar (NULL);
		break;
	    case 15: /* read */
		pre_cmd_proc (cmd_nbr, dm_c);
		ptext ("Select the cell to be read");
		inp_mod (NULL);
		if (Default_expansion_level != 1) {
		    picture ();
		    expansion (Default_expansion_level);
		}
		set_titlebar (NULL);
		break;
	    case 11:  /* fish the cell */
		pre_cmd_proc (cmd_nbr, dm_c);
                do_fish ("");
		break;
	    case 10:  /* fish the cell (fish ai) */
		pre_cmd_proc (cmd_nbr, dm_c);
                do_fish ("ai");
		break;
	    case 12: /* Place and route */
		pre_cmd_proc (cmd_nbr, dm_c);
		do_madonna (TRUE);
		set_titlebar (NULL);
		break;
	    case 13:
		if (dirty == TRUE) {
		    ptext ("Are you sure? The cell was modified but not yet saved.");
		    if (ask (2, yes_no, -1) == 0) {
			ptext ("OK, You asked for it, don't blame me!");
			strcpy (cirname, ""); /* default cirname off */
			eras_worksp ();
			initwindow ();
			set_titlebar (NULL);
			Sub_terms = FALSE;
			dirty = FALSE;
		    }
		}
		else {
		    strcpy (cirname, ""); /* default cirname off */
		    eras_worksp ();
		    initwindow ();
		    set_titlebar (NULL);
		    Sub_terms = FALSE;
		}
		break;
	    case 7: /* sub_term */
		pre_cmd_proc (cmd_nbr, dm_c);
		all_term ();
		Sub_terms = TRUE;
		break;
	    case 8: /* ind_exp */
		pre_cmd_proc (cmd_nbr, dm_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1)
		    indiv_exp (xlc, ybc);
		break;
	    case 9: /* expand */
		expansion (-1);
		set_titlebar (NULL);
		break;
	    default:
		ptext ("Command not implemented!");
	}
	picture ();
	post_cmd_proc (cmd_nbr, dm_c);
	if (new_cmd == -2) goto dm_menu_txt;
    }
}

static void inst_cmd ()
{
io_menu_txt:
    ptext ("The old instance menu (that is, with set instance)");

    while (TRUE) {
	menu (INST_CMD, inst_c);
	get_cmd ();
	switch (cmd_nbr) {
	    case -2:
		goto io_menu_txt;
	    case 0:
		return;
	    case 1:
		pre_cmd_proc (cmd_nbr, inst_c);
		toggle_grid ();
		break;
	    case 2:
		pre_cmd_proc (cmd_nbr, inst_c);
		bound_w ();
		break;
	    case 3:
		pre_cmd_proc (cmd_nbr, inst_c);
		prev_w ();
		break;
	    case 4:
		pre_cmd_proc (cmd_nbr, inst_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    center_w (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 5:
		pre_cmd_proc (cmd_nbr, inst_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    curs_w (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 6:
		pre_cmd_proc (cmd_nbr, inst_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    de_zoom (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 7:
		add_inst (IMPORTED);
		break;
	    case 8:
		add_inst (LOCAL);
		break;
	    case 9:
		pre_cmd_proc (cmd_nbr, inst_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    del_inst (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 10:
		pre_cmd_proc (cmd_nbr, inst_c);
		if ((new_cmd = set_tbltcur (1, SNAP)) == -1) {
		    mov_inst (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 11:
		pre_cmd_proc (cmd_nbr, inst_c);
		mir_inst ((int) FALSE);
		break;
	    case 12:
		pre_cmd_proc (cmd_nbr, inst_c);
		rot_inst ();
		break;
	    case 13:
		arr_par ();
		break;
	    case 14:
		pre_cmd_proc (cmd_nbr, inst_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1)
		    set_actinst (xlc, ybc);
		break;
	    case 15:
		pre_cmd_proc (cmd_nbr, inst_c);
		upd_inst_bbox ();
		break;
	    default:
		ptext ("Command not implemented!");
	}
	picture ();
	post_cmd_proc (cmd_nbr, inst_c);
	if (new_cmd == -2) goto io_menu_txt;
    }
}

/*
 * PATRICK: automatic tools menu
 */
static void tools_cmd ()
{
    char mess_str[200];

tls_menu_txt:
    ptext ("This is the menu for calling automatic tools");

    while (TRUE) {
	menu (TOOLS_CMD, tools_c);
	get_cmd ();
	switch (cmd_nbr) {
	    case -2:
		goto tls_menu_txt;
	    case 0:
		return;
	    case 1:
		pre_cmd_proc (cmd_nbr, tools_c);
		bound_w ();
		break;
	    case 2:
		pre_cmd_proc (cmd_nbr, tools_c);
		prev_w ();
		break;
	    case 3:
		pre_cmd_proc (cmd_nbr, tools_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    center_w (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 4:
		pre_cmd_proc (cmd_nbr, tools_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    curs_w (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 5:
		pre_cmd_proc (cmd_nbr, tools_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    de_zoom (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	     case 6:  /* DRC menu */
		chk_cmd ();
		goto tls_menu_txt;
	     case 7: /* print */
		pre_cmd_proc (cmd_nbr, tools_c);
		do_print ();
		break;
	     case 8: /* fish */
		pre_cmd_proc (cmd_nbr, tools_c);
                do_fish ("");
		break;
	     case 9: /* check nets */
		pre_cmd_proc (cmd_nbr, tools_c);
		do_autoroute (TRUE);
		break;
	     case 10: /* trout */
		pre_cmd_proc (cmd_nbr, tools_c);
		do_autoroute (FALSE);
		break;
	     case 11: /* madonna */
		pre_cmd_proc (cmd_nbr, tools_c);
		do_madonna (FALSE);
		break;
	     case 12: /* place & route */
		pre_cmd_proc (cmd_nbr, tools_c);
		do_madonna (TRUE);
		set_titlebar (NULL);
		break;
	     case 13: /* Highlight net */
	        pre_cmd_proc (cmd_nbr, tools_c);
	        do_autoroute (TRUE+1);
		break;
	     default:
		ptext ("Command not implemented!");
	}
	picture ();
	post_cmd_proc (cmd_nbr, tools_c);
	if (new_cmd == -2) goto tls_menu_txt;
    }
}

/*
 * PATRICK: new instance command
 */
static void patrick_inst_cmd ()
{
    char mess_str[200];
    char ret_str[200];
    double xlmul;

i_menu_txt:
    ptext ("This is the menu for instance manipulation; please select a command");

    while (TRUE) {
	menu (PATRICK_INST_CMD, patrick_inst_c);
	get_cmd ();
	switch (cmd_nbr) {
	    case -2:
		goto i_menu_txt;
	    case 0:
		return;
	    case 1: /* OLD INSTANCE COMMAND MENU */
		inst_cmd ();
		goto i_menu_txt;
	    case 2:
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
		bound_w ();
		break;
	    case 3:
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
		prev_w ();
		break;
	    case 4:
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    center_w (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 5:
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    curs_w (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 6:
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    de_zoom (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 7:
		ptext ("Select imported instance");
		add_inst (IMPORTED);
		dirty = TRUE;
		break;
	    case 8:
		ptext ("Select instance");
		add_inst (LOCAL);
		dirty = TRUE;
		break;
	    case 9:
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
		ptext ("Click instance to be deleted");
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    dirty = TRUE;
		    del_inst (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 10:
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
		ptext ("Click instance to be moved");
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    set_actinst (xlc, ybc);
		    new_cmd = cmd_nbr;
		    if (!act_inst || (ImageMode && strcmp (act_inst->inst_name, ImageInstName) == 0)) {
			/* the image cannot be moved */
			ptext ("Nothing selected for move, try again!");
			sleep (1);
		    }
		    else {
			sprintf (mess_str, "The instance is: %s (%s), click new left bottom position",
			    act_inst->inst_name, act_inst->templ->cell_name);
			ptext (mess_str);
			if ((new_cmd = set_tbltcur (1, SNAP)) == -1) {
			    dirty = TRUE;
			    mov_inst (xlc, ybc);
			    new_cmd = cmd_nbr;
			}
		    }
		}
		break;
	    case 11: /* MIRROR */
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
		ptext ("Click instance to be mirrored");
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    set_actinst (xlc, ybc);
		    new_cmd = cmd_nbr;
		    if (!act_inst || (ImageMode && strcmp (act_inst->inst_name, ImageInstName) == 0)) {
			/* the image cannot be mirror */
			ptext ("Nothing selected for mirror, try again!");
			sleep (1);
		    }
		    else {
			sprintf (mess_str, "The instance is: %s (%s)", act_inst->inst_name, act_inst->templ->cell_name);
			ptext (mess_str);
			dirty = TRUE;
			/* implicit x-mirror if image mode */
			mir_inst ((int) ImageMode);
		    }
		}
		break;
	    case 12:  /* ROTATE */
		if (ImageMode && strcmp (ThisImage, "octagon") != 0) {
		    ptext ("Hey! Do you REALLY want to rotate on Sea of Gates? Switch ImageMode off if so.");
		}
		else {
		    pre_cmd_proc (cmd_nbr, patrick_inst_c);
		    ptext ("Click instance to be rotated");
		    if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
			set_actinst (xlc, ybc);
			new_cmd = cmd_nbr;
			if (!act_inst || (ImageMode && strcmp (act_inst->inst_name, ImageInstName) == 0)) {
			    /* the image cannot be mirror */
			    ptext ("Nothing selected for rotate, try again!");
			    sleep (1);
			}
			else {
			    sprintf (mess_str, "The rotated instance is: %s (%s)",
			    act_inst->inst_name, act_inst->templ->cell_name);
			    ptext (mess_str);
			    dirty = TRUE;
			    rot_inst ();
			    ptext (mess_str);
			}
		    }
		}
		break;
	    case 13: /* ARR PARR */
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
		ptext ("Click instance to be indexed");
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    set_actinst (xlc, ybc);
		    new_cmd = cmd_nbr;
		    if (!act_inst) {
			ptext ("Nothing selected for array parameters, try again!");
			sleep (1);
		    }
		    else {
			sprintf (mess_str, "The selected instance is: %s (%s), click parameter",
			    act_inst->inst_name, act_inst->templ->cell_name);
			ptext (mess_str);
			dirty = TRUE;
			arr_par ();
		    }
		}
		break;

	    case 14: /* SET INSTANCE NAME */
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
		ptext ("Click instance to be renamed");
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    set_actinst (xlc, ybc);
		    new_cmd = cmd_nbr;
		    if (!act_inst || (ImageMode && strcmp (act_inst->inst_name, ImageInstName) == 0)) {
			ptext ("No instance selected, try again!");
			sleep (1);
		    }
		    else {
			char new_name[DM_MAXNAME +10];
			sprintf (mess_str, "Instance: %s (%s), New instance name: ",
			    act_inst->inst_name, act_inst->templ->cell_name);
			if (ask_name (mess_str, new_name, TRUE) == -1) {
			    ptext ("Something wrong with the name, try again");
			    sleep (1);
			}
			else {
			    INST *ip;
			    for (ip = inst_root; ip; ip = ip -> next)
				if (strcmp (ip->inst_name, new_name) == 0) break;

			    if (ip && ip != act_inst && strcmp (ip->inst_name,".") != 0) {
				sprintf (mess_str, "WARNING: Instance name '%s' is already used for cell %s  ********",
				    new_name, ip->templ->cell_name);
				ptext (mess_str);
				sleep (2);
			    }
			    dirty = TRUE;
			    strcpy (act_inst->inst_name, new_name);
			    pict_arr[Textnr] = DRAW;
			}
		    }
		}
		break;
	    case 15:  /* multiply xl */
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
	        sprintf (mess_str, "Enter multiplier for lower left coordinate of instances: ");
	        if (ask_name (mess_str, ret_str, FALSE) == -1 || sscanf (ret_str, "%le", &xlmul) == 0 || xlmul <= 0)
		    ptext ("Invalid specification ...");
	        else {
		    ptext ("Changing xl values ...");
                    mov_insts (xlmul, 1.0000000000);
                }
		break;
	    case 16:
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
		ptext ("Click instance of which the bbx is to be updated");
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    set_actinst (xlc, ybc);
		    new_cmd = cmd_nbr;
		    if (!act_inst || (ImageMode && strcmp (act_inst->inst_name, ImageInstName) == 0)) {
			ptext ("No instance selected, try again!");
		    }
		    else {
			sprintf (mess_str, "The selected instance is: %s (%s)", act_inst->inst_name, act_inst->templ->cell_name);
			ptext (mess_str);
			dirty = TRUE;
			upd_inst_bbox ();
		    }
		}
		break;
            case 17:  /* fish the cell */
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
                do_fish ("");
		break;
            case 18:  /* auto place the cell */
		pre_cmd_proc (cmd_nbr, patrick_inst_c);
                do_madonna (FALSE);
		break;
	    default:
		ptext ("Command not implemented!");
	}
	picture ();
	post_cmd_proc (cmd_nbr, patrick_inst_c);
	if (new_cmd == -2) goto i_menu_txt;
    }
}

static void term_cmd ()
{
te_menu_txt:
    ptext ("This is the terminal menu: please select a command");

    while (TRUE) {
	menu (TERM_CMD, term_c);
	get_cmd ();
	switch (cmd_nbr) {
	    case -2:
		goto te_menu_txt;
	    case 0:
		return;
	    case 1:
		pre_cmd_proc (cmd_nbr, term_c);
		toggle_grid ();
		break;
	    case 2:
		pre_cmd_proc (cmd_nbr, term_c);
		bound_w ();
		break;
	    case 3:
		pre_cmd_proc (cmd_nbr, term_c);
		prev_w ();
		break;
	    case 4:
		pre_cmd_proc (cmd_nbr, term_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    center_w (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 5:
		pre_cmd_proc (cmd_nbr, term_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    curs_w (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 6:
		pre_cmd_proc (cmd_nbr, term_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    de_zoom (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 7:
		pre_cmd_proc (cmd_nbr, term_c);
		if ((new_cmd = set_tbltcur (2, SNAP)) == -1) {
		    dirty = TRUE;
		    add_term (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 8:
		pre_cmd_proc (cmd_nbr, term_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    dirty = TRUE;
		    del_term (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 9:
		pre_cmd_proc (cmd_nbr, term_c);
		if ((new_cmd = set_tbltcur (2, SNAP)) == -1) {
		    dirty = TRUE;
		    fill_tbuf (xlc, xrc, ybc, ytc);
		    del_t_area (xlc, xrc, ybc, ytc);
		}
		break;
	    case 10:
		pre_cmd_proc (cmd_nbr, term_c);
		dirty = TRUE;
		put_buffer (TRUE);
		break;
	    case 11:
		pre_cmd_proc (cmd_nbr, term_c);
		if ((new_cmd = set_tbltcur (1, SNAP)) == -1) {
		    dirty = TRUE;
		    mov_term (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 12:
		dirty = TRUE;
		t_arr_par ();
		break;
	    case 13:
		pre_cmd_proc (cmd_nbr, term_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1)
		    set_act_term (xlc, ybc);
		break;
	    case 14:
		pre_cmd_proc (cmd_nbr, term_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    dirty = TRUE;
		    lift_terms (xlc, ybc);
		}
		break;
	    default:
		ptext ("Command not implemented!");
	}
	picture ();
	post_cmd_proc (cmd_nbr, term_c);
	if (new_cmd == -2) goto te_menu_txt;
    }
}

#define NOR_TOGGLE  7
#define APP_CMD     8
#define DEL_CMD     9

static void box_cmd ()
{
    static int non_orth_flag = 0;
    int     j, lay;
    int     old_cmd = 0;
    char ** box_cmd_arr;

    if (ImageMode)
	box_cmd_arr = image_box_c;
    else
	box_cmd_arr = box_c;

box_menu_txt:
    ptext ("This is the box manipulation menu; please select a command");

    while (TRUE) {
        menu (BOX_CMD, box_cmd_arr);
	if (non_orth_flag && !ImageMode) pre_cmd_proc (NOR_TOGGLE, box_cmd_arr);
	get_cmd ();
	switch (cmd_nbr) {
	    case -2:
		if (old_cmd) new_cmd = old_cmd;
		goto box_menu_txt;
	    case 0:
		return;
	    case 1:
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
		toggle_grid ();
		if (old_cmd) new_cmd = old_cmd;
		break;
	    case 2:
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
		bound_w ();
		if (old_cmd) new_cmd = old_cmd;
		break;
	    case 3:
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
		prev_w ();
		if (old_cmd) new_cmd = old_cmd;
		break;
	    case 4:
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    center_w (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 5:
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    curs_w (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 6:
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    de_zoom (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
/*	    case 8:  coordinates
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
		if ((new_cmd = set_tbltcur (1, SNAP)) == -1) {
		    track_coord (xlc, ybc, FALSE);
		    new_cmd = cmd_nbr;
		}
		else {
		   other command selected: erase old cross
		    track_coord (0, 0, TRUE);
		}
		break; */
	    case NOR_TOGGLE:
		if (old_cmd) new_cmd = old_cmd;
		if (ImageMode) { /* redraw */
		    for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
		    break;
		}
		if (!non_orth_flag) non_orth_flag = 1;
		else {
		    non_orth_flag = 0;
		    post_cmd_proc (cmd_nbr, box_cmd_arr);
		}
		continue;
	    case APP_CMD:
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
		if (!non_orth_flag) {
		    if ((new_cmd = set_tbltcur (2, SNAP)) == -1) {
			dirty = TRUE;
			addel_cur (xlc, xrc, ybc, ytc, ADD);
			new_cmd = cmd_nbr;
		    }
		}
		else if ((new_cmd = add_del_poly (ADD)) == -1) {
		    dirty = TRUE;
		    new_cmd = cmd_nbr;
		}
		old_cmd = cmd_nbr;
		break;
	    case DEL_CMD:
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
		if (!non_orth_flag) {
		    if ((new_cmd = set_tbltcur (2, SNAP)) == -1) {
			dirty = TRUE;
			addel_cur (xlc, xrc, ybc, ytc, DELETE);
			new_cmd = cmd_nbr;
		    }
		}
		else if ((new_cmd = add_del_poly (DELETE)) == -1) {
		    dirty = TRUE;
		    new_cmd = cmd_nbr;
		}
		old_cmd = cmd_nbr;
		break;
	    case 10: /* yank */
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
		if ((new_cmd = set_tbltcur (2, SNAP)) == -1) fill_buffer (xlc, xrc, ybc, ytc, TRUE);
		break;
	    case 11:  /* put */
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
		dirty = TRUE;
		put_buffer (FALSE);
		break;
	    case 12:  /* wire */
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
		dirty = TRUE;
		wire_points (non_orth_flag);
		break;
	     case 13: /* check the nets */
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
		do_autoroute (TRUE);
		break;
	     case 14: /* autoroute */
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
                do_autoroute (FALSE);
		break;
	     case 15: /* go fishing this cell */
		pre_cmd_proc (cmd_nbr, box_cmd_arr);
                do_fish ("");
		break;
	    default:
		ptext ("Command not implemented!");
	}
	picture ();
        post_cmd_proc (cmd_nbr, box_cmd_arr);

	if (cmd_nbr != APP_CMD && cmd_nbr != DEL_CMD) old_cmd = 0;
	if (new_cmd == -2) goto box_menu_txt;
    }
}

void bulb_dominant ()
{
    if (!aux_menu) return;
    if (Draw_dominant) {
	pre_cmd_proc (9, aux_c);
	ptext ("Drawing mode: dominant");
    }
    else {
	post_cmd_proc (9, aux_c);
	ptext ("Drawing mode: transparent");
    }
}

void bulb_hashed ()
{
    if (!aux_menu) return;
    if (Draw_hashed) {
	pre_cmd_proc (10, aux_c);
	ptext ("Instances will be drawn hashed");
    }
    else {
	post_cmd_proc (10, aux_c);
	ptext ("Instances will be drawn normal");
    }
}

static void aux_cmd ()
{
    register int lay;

    aux_menu = 1;
aux_menu_txt:
    ptext ("This is the settings menu");

    while (TRUE) {
	menu (AUX_CMD, aux_c);

	if (BackingStore  == TRUE) pre_cmd_proc (7, aux_c);
	if (ImageMode     == TRUE) pre_cmd_proc (8, aux_c);
	if (Draw_dominant == TRUE) pre_cmd_proc (9, aux_c);
	if (Draw_hashed   == TRUE) pre_cmd_proc (10, aux_c);

	get_cmd ();
	switch (cmd_nbr) {
	    case -2:
		goto aux_menu_txt;
	    case 0:
		aux_menu = 0;
		return;
	    case 1:
		pre_cmd_proc (cmd_nbr, aux_c);
		toggle_grid ();
		break;
	    case 2:
		set_gr_spac ();
		break;
	    case 3:
		set_sn_gr_spac ();
		break;
	    case 4:
		inform_window ();
		break;
	    case 5:
		inform_process ();
		break;
	    case 6:
		inform_act_term ();
		break;
/*	    case 7:
		upd_boundb ();
		inform_cell ();
		break; */
	     case 7: /* backing store */
		if (BackingStore == FALSE) {
		    BackingStore = TRUE;
		    set_backing_store (BackingStore);
		    pre_cmd_proc (cmd_nbr, aux_c);
		    ptext ("Backing store switched on (if possible)");
		}
		else {
		    BackingStore = FALSE;
		    set_backing_store (BackingStore);
		    post_cmd_proc (cmd_nbr, aux_c);
		    ptext ("Backing store switched off");
		}
		break;
	     case 8: /* snap inst on grid */
		if (!ImageMode) {
		    if (!ThisImage)
			ptext ("SORRY: No Sea-of-Gates image mode possible on non-Sea-of-Gates image");
		    else {
			ImageMode = TRUE;
			pre_cmd_proc (cmd_nbr, aux_c);
			ptext ("Snapping instances and boxes on the Sea-of-Gates image");
		    }
		}
		else {
		    ImageMode = FALSE;
		    post_cmd_proc (cmd_nbr, aux_c);
		    ptext ("No snapping of instances or boxes on the Sea-of-Gates image");
		}
		break;
	    case 9:
		if (Draw_dominant == FALSE)
		    Draw_dominant = TRUE;
		else
		    Draw_dominant = FALSE;
		bulb_dominant ();
		for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
		break;
	    case 10:
		if (Draw_hashed == FALSE)
		    Draw_hashed = TRUE;
		else
		    Draw_hashed = FALSE;
		bulb_hashed ();
		for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
		break;
	    case 11: /* check menu */
		chk_cmd ();
		goto aux_menu_txt;
	    case 12:
		toggle_tracker ();
		break;
	    default:
		ptext ("Command not implemented!");
	}
	picture ();
	post_cmd_proc (cmd_nbr, aux_c);
    }
}

static void comment_cmd ()
{
an_menu_txt:
    ptext ("This is the annotate menu: please select a command");

    while (TRUE) {
	menu (COMM_CMD, comment_c);
	get_cmd ();
	switch (cmd_nbr) {
	    case -2:
		goto an_menu_txt;
	    case 0: 		/* return */
		return;
	    case 1:
		pre_cmd_proc (cmd_nbr, box_c);
		toggle_grid ();
		break;
	    case 2:
		pre_cmd_proc (cmd_nbr, box_c);
		bound_w ();
		break;
	    case 3:
		pre_cmd_proc (cmd_nbr, box_c);
		prev_w ();
		break;
	    case 4:
		pre_cmd_proc (cmd_nbr, box_c);
		if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		    center_w (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 5:
		pre_cmd_proc (cmd_nbr, box_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    curs_w (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 6:
		pre_cmd_proc (cmd_nbr, box_c);
		if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		    de_zoom (xlc, xrc, ybc, ytc);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 7: 		/* ----- */
		pre_cmd_proc (cmd_nbr, comment_c);
	        if ((new_cmd = get_line_points (2, SNAP)) == -1) {
		    c_line (xlc, ybc, xrc, ytc, GA_NO_ARROW);
		    new_cmd = cmd_nbr;
	        }
		break;
	    case 8: 		/* -----> */
		pre_cmd_proc (cmd_nbr, comment_c);
	        if ((new_cmd = get_line_points (2, SNAP)) == -1) {
		    c_line (xlc, ybc, xrc, ytc, GA_FW_ARROW);
		    new_cmd = cmd_nbr;
	        }
		break;
	    case 9: 		/* <----- */
		pre_cmd_proc (cmd_nbr, comment_c);
	        if ((new_cmd = get_line_points (2, SNAP)) == -1) {
		    c_line (xlc, ybc, xrc, ytc, GA_BW_ARROW);
		    new_cmd = cmd_nbr;
	        }
		break;
	    case 10: 		/* <-----> */
		pre_cmd_proc (cmd_nbr, comment_c);
	        if ((new_cmd = get_line_points (2, SNAP)) == -1) {
		    c_line (xlc, ybc, xrc, ytc, GA_DB_ARROW);
		    new_cmd = cmd_nbr;
	        }
		break;
	    case 11: 		/* left_text  */
		pre_cmd_proc (cmd_nbr, comment_c);
		if ((new_cmd = set_tbltcur (1, SNAP)) == -1) {
		    c_text (xlc, ybc, GA_RIGHT);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 12: 		/* right_text */
		pre_cmd_proc (cmd_nbr, comment_c);
		if ((new_cmd = set_tbltcur (1, SNAP)) == -1) {
		    c_text (xlc, ybc, GA_LEFT);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 13: 		/* center_text */
		pre_cmd_proc (cmd_nbr, comment_c);
		if ((new_cmd = set_tbltcur (1, SNAP)) == -1) {
		    c_text (xlc, ybc, GA_CENTER);
		    new_cmd = cmd_nbr;
		}
		break;
	    case 14: 		/* DELETE */
		pre_cmd_proc (cmd_nbr, comment_c);
		if ((new_cmd = set_tbltcur (1, SNAP)) == -1) {
		    del_comment (xlc, ybc);
		    new_cmd = cmd_nbr;
		}
		break;
	    default:
		ptext ("Command not implemented!");
	}
	picture ();
        post_cmd_proc (cmd_nbr, comment_c);
	if (new_cmd == -2) goto an_menu_txt;
    }
}

void ask_quit (int sure)
{
    if (dirty == TRUE || sure) {
	if (dirty == TRUE)
	    ptext ("Sure? (Cell was modified, but not yet written)");
	else
	    ptext ("Sure, do you want to quit?");
	if (ask (2, no_yes, -1) > 0) stop_show (0);
    }
    else stop_show (0);
    ptext ("");
}
