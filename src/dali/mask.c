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

extern Coor Q_xl, Q_xr, Q_yb, Q_yt; /* quad search window */
extern INST *inst_root;
extern TERM **term_root;
extern TERM **TBuf;
extern qtree_t **quad_root;
extern struct obj_node **PutBuf;
extern int *term_lay;
extern int *def_arr;
extern int *drawing_order;
extern int *non_edit;
extern int *fillshift;
extern int *fillst;
extern int *vis_arr;
extern int *black_arr;
extern int *dom_arr;
extern int *ggeIds;
extern int *pict_arr;

extern FILE *rcfile;
extern char *argv0;
extern char *cellstr;
extern char *ggcolors[];
extern DM_PROJECT *dmproject;
extern int  Default_expansion_level;
extern int  Draw_dominant;
extern int  Draw_hashed;
extern int  RGBids[3];
extern int  Yellow;
extern int  mH, tH;
extern int  NR_dom;
extern int  NR_lay;
extern int  NR_all;
extern int  ANI_mode;
extern int  Backgr;
extern int  Gridnr;
extern int  Textnr;
extern int  cmd_nbr;
extern int  dom_order_cmd;
extern int  Erase_hollow;
extern int  gridflag, not_disp_mess, not_snap_mess;
extern int  ask_iname;
extern int  initwindow_flag;
extern Coor initwindow_xl, initwindow_yb, initwindow_dx;
extern int  tracker_mode;
extern int  use_new_mode;
extern int  zoom_mode;
extern int  v_grid, v_sngrid, v_term, v_sterm, v_inst;
extern int  v_bbox, v_tname, v_stname, v_iname;
extern int  v_label, v_comment;
extern struct Disp_wdw *c_wdw;
extern struct Disp_wdw *p_wdw;
extern float XL, XR, YB, YT;
extern float c_cW, c_cH;
extern Coor piwl, piwr, piwb, piwt;

#define E_NR 5 /* extra entries (visuals offset) */

char *vis_str[] = {
/* 0 */ "-return-",
/* 1 */ "-restore-",
/* 2 */ "-save-",
/* 3 */ "-all_on-",
/* 4 */ "-all_off-",
/* 5 */ "disp_grid",	/* E_NR */
/* 6 */ "snap_grid",
/* 7 */ "terminals",
/* 8 */ "sub_terms",
/* 9 */ "instances",
/*10 */ "bboxes",
/*11 */ "term_name",
/*12 */ "subt_name",
/*13 */ "inst_name",
/*14 */ "labels",
/*15 */ "comments",
};

static int *old_arr;
static int *new_arr;
static int lay_present;

int exp_ask_coord = 0;
int nbr_max;
int nbr_offset = 0;
int selected_lay;
int VIS_mode = 0;
int ADD_mode = 0;
char **lay_names;
char **visuals;

static void bulb (int lay, int enable);

void Rmsk ()
{
    init_colmenu ();
    flush_pict ();
}

void init_mskcol ()
{
    register int lay, nbr;
    char line[200], token[200], a1[200], a2[200];
    char *cp, *fn = ".dalirc";
    DM_PROCDATA *process = dmproject -> maskdata;
 /*
  ** The mask_numbers in the process structure are guaranteed
  ** to go from 0 to no_masks-1 (in that order). If this editor
  ** makes all these masks editable we can use the mask_numbers
  ** of the boxes and terminals directly as the indices in the
  ** data-structures: simple and efficient.
  */
    lay_names = process -> mask_name;
    NR_dom = 0;
    NR_lay = process -> nomasks;
    NR_all = NR_lay + 3;

    MALLOCN (term_root, TERM *, NR_lay);
    MALLOCN (quad_root, qtree_t *, NR_lay);
    MALLOCN (term_lay, int, NR_lay);
    MALLOCN (def_arr, int, NR_lay);
    MALLOCN (drawing_order, int, NR_lay);
    MALLOCN (non_edit, int, NR_lay);
    MALLOCN (fillshift, int, NR_lay);
    MALLOCN (fillst, int, NR_lay);
    MALLOCN (vis_arr, int, NR_lay + NR_VIS);
    MALLOCN (PutBuf, struct obj_node *, NR_lay);
    MALLOCN (TBuf, TERM *, NR_lay);

    MALLOCN (black_arr, int, NR_all);
    MALLOCN (dom_arr  , int, NR_all);
    MALLOCN (ggeIds   , int, NR_all);
    MALLOCN (pict_arr , int, NR_all);

    for (lay = 0; lay < NR_all; ++lay) {
	black_arr[lay] = 0;
	dom_arr[lay]   = 0;
	pict_arr[lay]  = 0;
    }

    if (rcfile) {
	while (fgets (line, 200, rcfile)) {
	    cp = line;
	    while (*cp == ' ' || *cp == '\t') ++cp; /* skip white space */
	    if (*cp == '#' || *cp == '\n' || !*cp) continue; /* comment line */
	    nbr = sscanf (cp, "%s%s%s", token, a1, a2);

	    if (strcmp (token, "set_colornr") == 0) {
		if (nbr < 3) goto ill1;
		cp = a1;
		if (!isdigit ((int)*cp)) goto ill2;
		if ((nbr = atoi (cp)) < 8 || nbr > 15) goto ill2;
		set_color_entry (nbr, a2);
		continue;
	    }

	    if (strcmp (token, "set_maskcolornr") == 0) {
		if (nbr < 3) goto ill1;
		cp = a2;
		if (!isdigit ((int)*cp)) goto ill2;
		if ((nbr = atoi (cp)) < 0 || nbr > 15) goto ill2;
		for (lay = 0; lay < NR_lay; ++lay)
		    if (strcmp (lay_names[lay], a1) == 0) break;
		if (lay >= NR_lay) goto ill3;
		process -> RT[lay] -> code = nbr;
		continue;
	    }
	    break;
ill1:
	    PE "%s: %s: %s: Ill. number of arguments\n", argv0, fn, token);
	    continue;
ill2:
	    PE "%s: %s: %s: Ill. colornr '%s'\n", argv0, fn, token, cp);
	    continue;
ill3:
	    PE "%s: %s: %s: Unknown mask '%s'\n", argv0, fn, token, a1);
	}
    }

    for (lay = 0; lay < NR_lay; ++lay) {
	term_lay[lay] = (process -> mask_type[lay] == DM_INTCON_MASK);
	ggSetIdForColorCode (nbr = process -> RT[lay] -> code);
	if (nbr == 0 || nbr == 15) { /* dominant layer (BLACK) */
	    black_arr[lay] = 1;
	    dom_arr[lay] = 1;
	    drawing_order[NR_dom++] = lay;
	}
	set_fill_style (lay, process -> RT[lay] -> fill, 0);
	vis_arr[lay] = 1; /* ON */
	def_arr[lay]  = 0;
	non_edit[lay] = 0;
	quad_root[lay] = NULL;
	term_root[lay] = NULL;
	PutBuf[lay] = NULL;
	TBuf[lay] = NULL;
    }
    visuals = &vis_str[E_NR];
    vis_arr[v_grid   = lay++] = 1;
    vis_arr[v_sngrid = lay++] = 0;
    vis_arr[v_term   = lay++] = 1;
    vis_arr[v_sterm  = lay++] = 0;
    vis_arr[v_inst   = lay++] = 1;
    vis_arr[v_bbox   = lay++] = 1;
    vis_arr[v_tname  = lay++] = 1;
    vis_arr[v_stname = lay++] = 1;
    vis_arr[v_iname  = lay++] = 1;
    vis_arr[v_label  = lay++] = 1;
    vis_arr[v_comment= lay++] = 1;

    MALLOCN (new_arr, int, lay);
    MALLOCN (old_arr, int, lay);
    while (--lay >= 0) old_arr[lay] = vis_arr[lay];
}

void set_fill_style (int lay, int style, int shift)
{
    if (style >= 10 && style <= 18) {
	shift %= 8;
	if (style >= 15)
	    style = FILL_STIPPLE5 + (style - 15);
	else if (style >= 11)
	    style = FILL_STIPPLE1 + (style - 11);
	else {
	    style = FILL_CROSS;
	    shift = 0;
	}
    }
    else if (style >= 20 && style <= 28) {
	shift %= 8;
	if (style >= 25)
	    style = FILL_STIPPLE5 + (style - 25) + FILL_HOLLOW;
	else if (style >= 21)
	    style = FILL_STIPPLE1 + (style - 21) + FILL_HOLLOW;
	else {
	    style = FILL_CROSS + FILL_HOLLOW;
	    shift = 0;
	}
    }
    else
    switch (style) {
	case 2:
	    shift = 0;
	    style = FILL_HOLLOW;
	    break;
	case 3:
	    shift %= 8;
	    style = FILL_HASHED12B; /* 12% fill + outline */
	    break;
	case 4:
	    shift %= 4;
	    style = FILL_HASHED25B; /* 25% fill + outline */
	    break;
	case 5:
	    shift %= 2;
	    style = FILL_HASHED50B; /* 50% fill + outline */
	    break;
	case 0:
	case 6:
	    shift %= 8;
	    style = FILL_HASHED;    /* 12% fill */
	    break;
	case 7:
	    shift %= 4;
	    style = FILL_HASHED25;  /* 25% fill */
	    break;
	case 8:
	    shift %= 2;
	    style = FILL_HASHED50;  /* 50% fill */
	    break;
	case 1:
	default: /* this is also case 1 */
	    shift = 0;
	    style = FILL_SOLID;
    }
    fillst[lay] = style;
    fillshift[lay] = shift;
}

void save_settings ()
{
    FILE *fp;
    register int lay, pos;
    int style, xl, xr, yb;
    DM_PROCDATA *process = dmproject -> maskdata;

    if ((fp = fopen (".dalisave", "w"))) {
	fprintf (fp, "ask_inst_name %s\n", ask_iname ? "on" : "off");
	if (Default_expansion_level == -1)
	    fprintf (fp, "default_expan no\n");
	else
	    fprintf (fp, "default_expan %d\n", Default_expansion_level);
	fprintf (fp, "drawing_order");
	for (pos = 0; pos < NR_dom; ++pos)
	    fprintf (fp, " %s", lay_names[drawing_order[pos]]);
	fprintf (fp, "\ndominant %s\n", Draw_dominant ? "on" : "off");
	for (lay = 0; lay < NR_lay; ++lay) {
	    style = fillst[lay];
	    if (style >= FILL_CROSS) {
		if (style >= FILL_STIPPLE5)
		    style = 15 + (style - FILL_STIPPLE5);
		else if (style >= FILL_STIPPLE1)
		    style = 11 + (style - FILL_STIPPLE1);
		else
		    style = 10;
		if (fillst[lay] & FILL_HOLLOW) style += 10;
	    }
	    else {
		switch (style) {
		case FILL_SOLID:     style = 1; break;
		case FILL_HOLLOW:    style = 2; break;
		case FILL_HASHED12B: style = 3; break;
		case FILL_HASHED25B: style = 4; break;
		case FILL_HASHED50B: style = 5; break;
		case FILL_HASHED:    style = 6; break;
		case FILL_HASHED25:  style = 7; break;
		case FILL_HASHED50:  style = 8; break;
		}
	    }
	    fprintf (fp, "fill_style %s %d+%d\n",
		lay_names[lay], style, fillshift[lay]);
	}
	fprintf (fp, "grid %s\n", gridflag ? "on" : "off");
	fprintf (fp, "hashed %s\n", Draw_hashed ? "on" : "off");
	for (lay = 0; lay < NR_lay; ++lay) {
	    fprintf (fp, "set_maskcolornr %s %d\n",
		lay_names[lay], process -> RT[lay] -> code);
	}
	fprintf (fp, "tracker %s\n",
		tracker_mode == 0 ? "off" : (
		tracker_mode == 1 ? "on" : "auto"));
	fprintf (fp, "use_new_name %s\n", use_new_mode ? "on" : "off");
	fprintf (fp, "zoom_mode %s\n",
		zoom_mode == 0 ? "fixed" : (
		zoom_mode == 1 ? "point" : "area"));
	for (lay = 0; lay < NR_lay; ++lay) {
	    fprintf (fp, "visible %s %s\n",
		lay_names[lay], vis_arr[lay] ? "on" : "off");
	}
	for (pos = 0; pos < NR_VIS; ++pos, ++lay) {
	    fprintf (fp, "visible %s %s\n",
		visuals[pos], vis_arr[lay] ? "on" : "off");
	}
	xl = (int) p_wdw -> wxmin;
	xr = (int) p_wdw -> wxmax;
	yb = (int) p_wdw -> wymin;
	fprintf (fp, "#init_window %d %d %d\n", xl, yb, xr - xl);
	if (cellstr) fprintf (fp, "#read %s\n", cellstr);

	fclose (fp);
	ptext ("Settings saved in .dalisave file!");
    }
}

void load_settings (char *file)
{
    FILE *fp;
    register int lay, pos;
    DM_PROCDATA *process = dmproject -> maskdata;
    char scanline[200], *token, arg1[100], arg2[100];
    register char *s;

    if (!file || !*file) return;
    if ((fp = fopen (file, "r"))) {
	while (fgets (scanline, 200, fp)) {
	    s = scanline;
	    while (*s == ' ' || *s == '\t') ++s;
	    if (!isalpha ((int)*s)) continue;
	    token = s++;
	    while (isalnum ((int)*s) || *s == '_') ++s;
	    if (!*s || *s == '\n') continue;
	    *s++ = 0;

	    if (strcmp (token, "ask_inst_name") == 0) {
		if (sscanf (s, "%s", arg1) == 1)
		    ask_iname = (strcmp (arg1, "on") == 0);
		continue;
	    }
	    if (strcmp (token, "default_expan") == 0) {
		if (sscanf (s, "%s", arg1) == 1) {
		    if (strcmp (arg1, "no") == 0)
			Default_expansion_level = -1;
		    else if (isdigit ((int)*arg1))
			Default_expansion_level = *arg1 - '0';
		}
		continue;
	    }
	    if (strcmp (token, "drawing_order") == 0) {
		NR_dom = 0;
		for (lay = 0; lay < NR_lay; ++lay) dom_arr[lay] = 0;
		while (sscanf (s, "%s", arg1) == 1) {
		    for (lay = 0; lay < NR_lay; ++lay)
			if (strcmp (lay_names[lay], arg1) == 0) {
			    drawing_order[NR_dom++] = lay;
			    dom_arr[lay] = 1;
			    break;
			}
		    while (isspace ((int)*s)) ++s;
		    while (*s && !isspace ((int)*s)) ++s;
		}
		continue;
	    }
	    if (strcmp (token, "dominant") == 0) {
		if (sscanf (s, "%s", arg1) == 1)
		    Draw_dominant = (strcmp (arg1, "on") == 0);
		continue;
	    }
	    if (strcmp (token, "fill_style") == 0) {
		if (sscanf (s, "%s %s", arg1, arg2) == 2) {
		    for (lay = 0; lay < NR_lay; ++lay)
			if (strcmp (lay_names[lay], arg1) == 0) {
			    for (s = arg2; isdigit ((int)*s); ++s);
			    set_fill_style (lay, atoi (arg2), atoi (s));
			    break;
			}
		}
		continue;
	    }
	    if (strcmp (token, "grid") == 0) {
		if (sscanf (s, "%s", arg1) == 1) {
		    pos = (strcmp (arg1, "on") == 0);
		    if (pos != gridflag) toggle_grid ();
		}
		continue;
	    }
	    if (strcmp (token, "hashed") == 0) {
		if (sscanf (s, "%s", arg1) == 1)
		    Draw_hashed = (strcmp (arg1, "on") == 0);
		continue;
	    }
	    if (strcmp (token, "init_window") == 0) {
		int xl, yb, dx;
		if (sscanf (s, "%d %d %d", &xl, &yb, &dx) == 3) {
		    initwindow_flag = 1;
		    initwindow_xl = xl;
		    initwindow_yb = yb;
		    initwindow_dx = dx;
		}
		continue;
	    }
	    if (strcmp (token, "set_maskcolornr") == 0) {
		if (sscanf (s, "%s %s", arg1, arg2) == 2) {
		    for (lay = 0; lay < NR_lay; ++lay)
			if (strcmp (lay_names[lay], arg1) == 0) {
			    if (isdigit ((int)*arg2)) {
				pos = atoi (arg2);
				if (pos <= 15)
				    process -> RT[lay] -> code = pos;
			    }
			    break;
			}
		}
		continue;
	    }
	    if (strcmp (token, "tracker") == 0) {
		if (sscanf (s, "%s", arg1) == 1) {
		    if (strcmp (arg1, "off") == 0)
			tracker_mode = 0;
		    else if (strcmp (arg1, "on") == 0)
			tracker_mode = 1;
		    else
			tracker_mode = 2;
		    toggle_tracker ();
		}
		continue;
	    }
	    if (strcmp (token, "use_new_name") == 0) {
		if (sscanf (s, "%s", arg1) == 1)
		    use_new_mode = (strcmp (arg1, "on") == 0);
		continue;
	    }
	    if (strcmp (token, "zoom_mode") == 0) {
		if (sscanf (s, "%s", arg1) == 1) {
		    if (strcmp (arg1, "fixed") == 0)
			zoom_mode = 0;
		    else if (strcmp (arg1, "point") == 0)
			zoom_mode = 1;
		    else
			zoom_mode = 2;
		}
		continue;
	    }
	    if (strcmp (token, "visible") == 0) {
		if (sscanf (s, "%s %s", arg1, arg2) == 2) {
		    for (lay = 0; lay < NR_lay; ++lay)
			if (strcmp (lay_names[lay], arg1) == 0)
			    goto set_vis;
		    for (pos = 0; pos < NR_VIS; ++pos, ++lay)
			if (strcmp (visuals[pos], arg1) == 0)
			    goto set_vis;
		}
		continue;
set_vis:
		vis_arr[lay] = (strcmp (arg2, "on") == 0);
	    }
	}
	fclose (fp);
	ptext ("Settings loaded from .dalisave file!");
	init_colmenu ();
	pict_all (ERAS_DR);
	picture ();
    }
}

void rebulb (int lay)
{
    int old;

    if (vis_arr[lay]) {
	old = VIS_mode;
	VIS_mode = 1;
	bulb (lay, 1);
	flush_pict ();
	VIS_mode = old;
    }
}

void disable_masks (int no_masks, int *mask_array)
{
    register int lay;
    for (lay = 0; lay < NR_lay; ++lay) non_edit[lay] = 0; /* reset */
    while (--no_masks >= 0) {
	non_edit[mask_array[no_masks]] = 1; /* set */
	def_arr[mask_array[no_masks]] = 0; /* reset */
    }
    init_colmenu ();
    flush_pict ();
}

void init_set_coord ()
{
    int lay = NR_lay;
    VIS_mode = 2;
    while (--lay >= 0) new_arr[lay] = 0;
    init_colmenu ();
}

void is_lay_present (struct obj_node *q)
{
    lay_present = 1; /* YES */
}

static int present_in_inst (Coor x, Coor y, int lay, INST *p, int level, Trans mtx[], DM_PROJECT *pkey)
{
    int     nx1, nx2, ny1, ny2;
    register int nx, ny;
    float   f_n1, f_n2, s_x, s_y, z;
    Coor    ll, rr, bb, tt;
    Trans   new_mtx[6], t_x, t_y;
    register INST *ip;
    qtree_t *qt_lay;
/*
fprintf(stderr, "present_in_inst: clevel=%d level=%d in=%s cn=%s\n",
level, p -> level, p -> inst_name, p -> templ -> cell_name);
*/
    if (!p -> templ -> quad_trees[0]) { /* template is empty: read it */
	if (exp_templ (p -> templ, pkey, p -> imported, READ_ALL))
	    return 0; /* read error */
    }

    nx1 = 0; nx2 = 0; ny1 = 0; ny2 = 0;

    if (p -> nx || p -> ny) { /* repetition */
	/* calculate search window at this instance level */
	if (mtx[0]) { /* r0 or r180 */
	    s_x = x - mtx[2]; s_y = y - mtx[5];
	    s_x /= mtx[0]; s_y /= mtx[4];
	}
	else {
	    s_y = x - mtx[2]; s_x = y - mtx[5];
	    s_x /= mtx[3]; s_y /= mtx[1];
	}

	if (p -> nx) {
	    f_n1 = (s_x - (float) p -> bbxr) / (float) p -> dx;
	    f_n2 = (s_x - (float) p -> bbxl) / (float) p -> dx;
	    if (f_n2 < f_n1) { z = f_n1; f_n1 = f_n2; f_n2 = z; }
	    if ((nx1 = UpperRound (f_n1)) < 0) nx1 = 0;
	    if ((nx2 = LowerRound (f_n2)) > p -> nx) nx2 = p -> nx;
	    if (nx1 > nx2) return 0;
	}
	if (p -> ny) {
	    f_n1 = (s_y - (float) p -> bbyt) / (float) p -> dy;
	    f_n2 = (s_y - (float) p -> bbyb) / (float) p -> dy;
	    if (f_n2 < f_n1) { z = f_n1; f_n1 = f_n2; f_n2 = z; }
	    if ((ny1 = UpperRound (f_n1)) < 0) ny1 = 0;
	    if ((ny2 = LowerRound (f_n2)) > p -> ny) ny2 = p -> ny;
	    if (ny1 > ny2) return 0;
	}
    }

    new_mtx[0] = mtx[0] * p -> tr[0] + mtx[1] * p -> tr[3];
    new_mtx[1] = mtx[0] * p -> tr[1] + mtx[1] * p -> tr[4];
    new_mtx[3] = mtx[3] * p -> tr[0] + mtx[4] * p -> tr[3];
    new_mtx[4] = mtx[3] * p -> tr[1] + mtx[4] * p -> tr[4];

    pkey = p -> templ -> projkey;
    qt_lay = p -> templ -> quad_trees[lay];

    t_x = p -> tr[2] + nx1 * p -> dx;
    for (nx = nx1;;) {
	t_y = p -> tr[5] + ny1 * p -> dy;
	for (ny = ny1;;) {
	    /* calculate search window at this level */
	    new_mtx[2] = mtx[2] + t_x * mtx[0] + t_y * mtx[1];
	    new_mtx[5] = mtx[5] + t_x * mtx[3] + t_y * mtx[4];

	    if (new_mtx[0]) { /* R0 or R180 */
		s_x = (float) (x - new_mtx[2]) / new_mtx[0];
		s_y = (float) (y - new_mtx[5]) / new_mtx[4];
	    }
	    else { /* R90 or R270 */
		s_y = (float) (x - new_mtx[2]) / new_mtx[1];
		s_x = (float) (y - new_mtx[5]) / new_mtx[3];
	    }

	    Q_xl = ll = LowerRound (s_x);
	    Q_xr = rr = UpperRound (s_x);
	    Q_yb = bb = LowerRound (s_y);
	    Q_yt = tt = UpperRound (s_y);

	    quad_search (qt_lay, is_lay_present);
	    if (lay_present) return 1;

	    for (ip = p -> templ -> inst; ip; ip = ip -> next) {
		if (level > 2 && !inst_outside_window (ip, ll, rr, bb, tt)) {
		    if (present_in_inst (x, y, lay, ip, level - 1, new_mtx, pkey)) return 1;
		}
	    }
	    if (++ny > ny2) break;
	    t_y += p -> dy;
	}
	if (++nx > nx2) break;
	t_x += p -> dx;
    }
    return 0;
}

static void search_instances (Coor x, Coor y, int lay)
{
    Trans matrix[6];
    register INST *ip;

 // if (!vis_arr[v_inst]) return; /* instances not visible */

    matrix[0] = 1; matrix[1] = 0; matrix[2] = 0;
    matrix[3] = 0; matrix[4] = 1; matrix[5] = 0;

    for (ip = inst_root; ip; ip = ip -> next) {
	if (ip -> level > 1 && !inst_outside_window (ip, x, x, y, y)) {
	    if (present_in_inst (x, y, lay, ip, ip -> level, matrix, dmproject)) return;
	}
    }
}

void give_lay_stack (Coor x, Coor y)
{
    int lay = NR_lay;

    while (--lay >= 0) {
	lay_present = 0; /* NO */
	Q_xl = Q_xr = x;
	Q_yb = Q_yt = y;
	quad_search (quad_root[lay], is_lay_present);
	if (!lay_present && exp_ask_coord) /* search in expanded subcell instances */
	    search_instances (x, y, lay);
	if (lay_present != new_arr[lay])
	    bulb (lay, new_arr[lay] = lay_present);
    }
}

void exit_set_coord ()
{
    VIS_mode = 0;
    init_colmenu ();
}

void Visible ()
{
    register int nbr, lay, mode;

    VIS_mode = 1;

do_menu:
    menu (NR_VIS + E_NR, vis_str);
    lay = NR_lay;
    for (nbr = E_NR; nbr < NR_VIS + E_NR; ++nbr)
	if (vis_arr[lay++]) pre_cmd_proc (nbr);

    ptext ("Set visible items or layers!");

    do {
	get_cmd ();
	if (cmd_nbr < E_NR) pre_cmd_proc (cmd_nbr);
	switch (cmd_nbr) {
	case 0: /* return */
	    break;
	case 2: /* save */
	    lay = NR_lay + NR_VIS;
	    while (--lay >= 0) old_arr[lay] = vis_arr[lay];
	    break;
	case 1: /* restore */
	case 3: /* all_on */
	case 4: /* all_off */
	    for (lay = 0; lay < NR_lay; ++lay) {
		if (cmd_nbr == 1) {
		    mode = old_arr[lay];
		    old_arr[lay] = vis_arr[lay];
		}
		else mode = cmd_nbr == 3 ? 1 : 0;
		if (vis_arr[lay] != mode) bulb (lay, mode);
		new_arr[lay] = mode;
	    }
	    for (nbr = E_NR; nbr < NR_VIS + E_NR; ++nbr, ++lay) {
		if (cmd_nbr == 1) {
		    mode = old_arr[lay];
		    old_arr[lay] = vis_arr[lay];
		}
		else mode = cmd_nbr == 3 ? 1 : 0;
		if (vis_arr[lay] != mode) turn_lamp (nbr, mode);
		new_arr[lay] = mode;
	    }
	    break;
	default:
	    if (cmd_nbr < 0) { /* special key press */
		sleep (1);
		goto do_menu; /* redraw menu */
	    }
	    lay = NR_lay + NR_VIS;
	    while (--lay >= 0) new_arr[lay] = vis_arr[lay];
	    lay = NR_lay + cmd_nbr - E_NR;
	    turn_lamp (cmd_nbr, new_arr[lay] = !new_arr[lay]);
	}
	if (cmd_nbr < E_NR) post_cmd_proc (cmd_nbr);
	if (cmd_nbr > 0 && cmd_nbr != 2) {
	    set_pict_arr (new_arr);
	    picture ();
	}
    } while (cmd_nbr);

    ptext ("");
    VIS_mode = 0;
    for (lay = 0; lay < NR_lay; ++lay) {
	if (!vis_arr[lay] && def_arr[lay]) bulb (lay, def_arr[lay] = 0);
    }
}

void set_pict_arr (int tmp_arr[])
{
    register int nbr, lay, erase;

    for (lay = 0; lay < NR_lay; ++lay) {
	if (tmp_arr[lay] == vis_arr[lay]) continue;
	vis_arr[lay] = tmp_arr[lay];
	pict_arr[lay] = !vis_arr[lay] ? ERAS_DR : DRAW;
    }

    for (nbr = 0; nbr < NR_VIS; ++nbr) {
	lay = nbr + NR_lay;
	if (tmp_arr[lay] == vis_arr[lay]) continue;
	vis_arr[lay] = tmp_arr[lay];
	erase = !vis_arr[lay];

	if (lay == v_grid || lay == v_sngrid) {
	    if (gridflag) {
		if (!erase) {
		    if (lay == v_grid) not_disp_mess = 1;
		    if (lay == v_sngrid) not_snap_mess = 1;
		    pict_arr[Gridnr] = DRAW;
		}
		else
		    pict_arr[Gridnr] = ERAS_DR;
	    }
	}
	else {
	    pict_arr[Textnr] = erase ? ERAS_DR : DRAW;

	    if (lay == v_term || lay == v_sterm) {
		for (lay = 0; lay < NR_lay; ++lay)
		    if (term_lay[lay])
			pict_arr[lay] = erase ? ERAS_DR : DRAW;
	    }
	    else if (lay == v_inst) {
		for (lay = 0; lay < NR_lay; ++lay)
		    pict_arr[lay] = erase ? ERAS_DR : DRAW;
	    }
	    else if (lay == v_label) {
		for (lay = 0; lay < NR_lay; ++lay)
		    pict_arr[lay] = erase ? ERAS_DR : DRAW;
	    }
	}
    }
}

void toggle_subterm ()
{
    register int lay, erase;

    if ((erase = vis_arr[v_sterm])) {
	vis_arr[v_sterm] = 0;
    }
    else {
	vis_arr[v_sterm] = 1;
	vis_arr[v_stname] = 1;
    }
    pict_arr[Textnr] = erase ? ERAS_DR : DRAW;
    for (lay = 0; lay < NR_lay; ++lay)
	if (term_lay[lay])
	    pict_arr[lay] = erase ? ERAS_DR : DRAW;
}

static void paint_lay (int lay, float xl, float xr, float yt, int mode)
/* mode: 0 = off; 1 = on; 2 = text only */
{
    float yb = yt - 1.0;

    if (mode == 2) goto txt;

    /* write: bg */
    Erase_hollow = 1;
    ggEraseArea (xl+6, xr, yb, yt, 1);
    Erase_hollow = 0;
    if (!mode) goto txt;

    /* write: fg */
    ggSetColor (lay);
    d_fillst (fillst[lay]); /* warning: set first the color! */
    paint_box (xl+6, xr, yb+0.05, yt-0.05);

txt:
    /* write: text */
    d_fillst (FILL_SOLID);
    if (non_edit[lay])
	ggSetColor (RGBids[0]); /* red */
    else
	ggSetColor (Gridnr);
    d_text (xl + 1, yb + 0.25, lay_names[lay], strlen (lay_names[lay]));
    if (mode != 1) d_text (xl + 6, yb + 0.25, "-off-", 5);
}

void init_colmenu ()
{
    float y;
    register int i, j, lay;
    int old_mode, ip;

    nbr_max = mH / tH;
    def_world_win (LAYS, 0.0, 10.0, 0.0, (float) nbr_max);
    --nbr_max;

    ggClearWindow ();

    old_mode = get_disp_mode ();
    disp_mode (DOMINANT);

    y = YB;

    ip = i = 0;
    if (Draw_dominant)
    while (i + nbr_offset < NR_dom) {
	lay = drawing_order[i + nbr_offset];

	paint_lay (lay, XL, XR, y+1, vis_arr[lay] ? 1 : 2);

	if ((def_arr[lay] && !non_edit[lay] && VIS_mode != 2) || (VIS_mode == 2 && new_arr[lay])) { /* bulb */
	    ggSetColor (Yellow);
	    paint_box (XL, XL+0.9, y, y+0.9);
	}
	if (i) {
	    ggSetColor (Gridnr);
	    d_line (XL, y, XR, y);
	}
	++y;
	if (++i == nbr_max) break;
    }
    ip = i;

    if (Draw_dominant) { lay = 0; j = NR_dom; }
    else { lay = nbr_offset; j = 0; }

    if (i < nbr_max)
    for (; lay < NR_lay; ++lay) {
	if (Draw_dominant) {
	    if (dom_arr[lay]) continue;
	    if (j++ < nbr_offset) continue;
	}

	paint_lay (lay, XL, XR, y+1, vis_arr[lay] ? 1 : 2);

	if ((def_arr[lay] && !non_edit[lay] && VIS_mode != 2) || (VIS_mode == 2 && new_arr[lay])) { /* bulb */
	    ggSetColor (Yellow);
	    paint_box (XL, XL+0.9, y, y+0.9);
	}
	if (i) {
	    ggSetColor (Gridnr);
	    d_line (XL, y, XR, y);
	}
	++y;
	if (++i == nbr_max) break;
    }

    ggSetColor (Gridnr);

    /* draw empty menu items? */
    while (i++ < nbr_max) { d_line (XL, y, XR, y); ++y; }

    /* draw last menu item */
    d_line (XL, y, XR, y);
    d_line (XL+5, y, XL+5, YT);
    ggSetColor (Yellow);
    d_text (XL+1, y + 0.25, "next", 4);
    d_text (XL+6, y + 0.25, "prev", 4);

    /* draw dominant marker? */
    if (ip && ip + nbr_offset == NR_dom) {
	ggSetColor (RGBids[0]); /* red */
	y = YB + ip;
	d_line (XL, y, XR, y);
    }

    disp_mode (old_mode);
}

void next_colmenu ()
{
    int lay = nbr_max + nbr_offset;
    if (lay < NR_lay) {
	nbr_offset = lay;
	init_colmenu ();
    }
}

void prev_colmenu ()
{
    if (nbr_offset > 0) {
	if ((nbr_offset -= nbr_max) < 0) nbr_offset = 0;
	init_colmenu ();
    }
}

static void bulb (int lay, int enable)
{
    float  y;
    struct Disp_wdw *old_wdw;
    int i, pos, old_mode;

    if (!VIS_mode && non_edit[lay]) return;

    i = 0;
    if (Draw_dominant && NR_dom) {
        for (pos = 0; pos < NR_dom; ++pos) {
            if (drawing_order[pos] == lay) break;
        }
        if (pos == NR_dom)
	for (i = 0; i < NR_lay; ++i) {
	    if (dom_arr[i]) continue;
	    if (i == lay) break;
	    ++pos;
	}
	i = NR_dom - nbr_offset;
    }
    else pos = lay;

    if ((pos -= nbr_offset) < 0 || pos >= nbr_max) return;

    old_wdw = c_wdw;
    set_c_wdw (LAYS);
    old_mode = get_disp_mode ();
    disp_mode (DOMINANT);

    y = pos;
    if (VIS_mode == 1) { /* color layer */
	paint_lay (lay, XL, XR, y+1, enable ? 1 : 0);
    }
    if (!VIS_mode || VIS_mode == 2) { /* (de)activate layer */
	ggSetColor (enable ? Yellow : Backgr);
	paint_box (XL, XL+0.9, y, y+0.9);
    }
    if (pos) {
	ggSetColor (pos == i ? RGBids[0] : Gridnr);
	d_line (XL, y, XR, y);
    }
    ggSetColor (pos+1 == i ? RGBids[0] : Gridnr);
    d_line (XL, y+1, XR, y+1);

    disp_mode (old_mode);
    if (old_wdw) set_c_wdw (old_wdw -> w_nr);
}

/*
** Set/Reset color layer viewport layer number.
*/
void toggle_lay (int lay)
{
    int t;
    if (!ANI_mode && !VIS_mode) {
	if (non_edit[lay]) {
	    notify ("layer disabled (non-editable)!");
	    return;
	}
	if (!vis_arr[lay]) {
	    notify ("layer not visible (non-editable)!");
	    return;
	}
    }
    if (ADD_mode && !term_lay[lay]) {
	notify ("layer disabled (not a terminal lay)!");
	return;
    }
    if (VIS_mode) {
	vis_arr[lay] = t = !vis_arr[lay];
	pict_arr[lay] = t ? DRAW : ERAS_DR;
	picture ();
    }
    else
	def_arr[lay] = t = !def_arr[lay];
    bulb (lay, t);
    flush_pict ();
}

void fillst_menu ()
{
    static char *style_str[] = {
    /* 0 */ "-return-",
    /* 1 */ "[shift>>]",
    /* 2 */ "Solid",
    /* 3 */ "Hashed",
    /* 4 */ "Hashed25",
    /* 5 */ "Hashed50",
    /* 6 */ "Outline",
    /* 7 */ "Cross",
    /* 8 */ "Stipple1",
    /* 9 */ "Stipple2",
    /*10 */ "Stipple3",
    /*11 */ "Stipple4",
    /*12 */ "Stipple5",
    /*13 */ "Stipple6",
    /*14 */ "Stipple7",
    /*15 */ "Stipple8",
    };
    int lay, old, oldfs;

again:
    lay = selected_lay;
    menu (16, style_str);
    ptext ("Select menu button to set fill_style!");

    old = oldfs = fillst[lay];
    if ((oldfs & 4) && oldfs != 4) {
	pre_cmd_proc (6);
	old -= 4;
    }
    if (old < 6) old += 2;
    else if (old >= 32) old -= 20;
    else if (old >= 16) old -= 8;
    else if (old == 8) old = 7;
    pre_cmd_proc (old);
    get_cmd ();
    while (cmd_nbr > 0) {
	if (cmd_nbr == 6) { /* Outline on/off */
	    if (oldfs & 4) {
		if (oldfs != 4) {
		    post_cmd_proc (6);
		    fillst[lay] -= 4;
		}
	    }
	    else {
		if (old == 2) { post_cmd_proc (2); old = 6; }
		pre_cmd_proc (6);
		fillst[lay] += 4;
	    }
	}
	else if (cmd_nbr == 1) { /* shift>> */
	    if (oldfs != 0 && oldfs != 4 && !(oldfs & 8)) {
		pre_cmd_proc (1);
		fillshift[lay] = (fillshift[lay] + 1) % 8;
		oldfs = 0;
	    }
	}
	else if (cmd_nbr != old) {
	    if (cmd_nbr == 2) {
		post_cmd_proc (old);
		if (old != 6 && (oldfs & 4))
		    post_cmd_proc (6);
	    }
	    else if (old != 6)
		post_cmd_proc (old);
	    pre_cmd_proc (old = cmd_nbr);
	    if (cmd_nbr < 6)
		fillst[lay] = cmd_nbr - 2;
	    else if (cmd_nbr == 7)
		fillst[lay] = 8;
	    else if (cmd_nbr >= 12)
		fillst[lay] = cmd_nbr + 20;
	    else if (cmd_nbr >= 8)
		fillst[lay] = cmd_nbr + 8;
	    if (cmd_nbr != 2 && (oldfs & 4)) fillst[lay] += 4;
	}
	else if (oldfs != 4 && (oldfs & 4)) {
	    post_cmd_proc (old);
	    pre_cmd_proc (old = 6);
	    fillst[lay] = 4;
	}
	if (fillst[lay] != oldfs) {
	    oldfs = fillst[lay];
	    VIS_mode = 1;
	    bulb (lay, 1);
	    VIS_mode = 0;
	    pict_all (ERAS_DR);
	    picture ();
	    if (cmd_nbr == 1) post_cmd_proc (1); /* shift */
	}
	get_cmd ();
    }
    if (cmd_nbr == -7) {
	post_cmd_proc (old);
	if (old != 6 && (oldfs & 4)) post_cmd_proc (6);
	goto again;
    }
    ptext ("");
}

void color_menu ()
{
    static int first_time = 1;
    static char *color_str[] = {
    /* 0 */ "-return-",
    /* 1 */ "0",
    /* 2 */ "1",
    /* 3 */ "2",
    /* 4 */ "3",
    /* 5 */ "4",
    /* 6 */ "5",
    /* 7 */ "6",
    /* 8 */ "7",
    /* 9 */ "8",
    /*10 */ "9",
    /*11 */ "10",
    /*12 */ "11",
    /*13 */ "12",
    /*14 */ "13",
    /*15 */ "14",
    /*16 */ "15",
    /*17 */ "[black_lay]",
    };
    int lay, old;
    DM_PROCDATA *process = dmproject -> maskdata;

    if (first_time) {
	first_time = 0;
	for (lay = 0; lay < 16; ++lay)
	    color_str[lay+1] = ggcolors[lay];
    }
again:
    lay = selected_lay;
    menu (18, color_str);
    ptext ("Select menu button to set color!");

    old = process -> RT[lay] -> code;
    pre_cmd_proc (++old);
    if (black_arr[lay]) pre_cmd_proc (17);
    get_cmd ();
    while (cmd_nbr > 0) {
	VIS_mode = 0;
	if (cmd_nbr == 17) { /* toggle black */
	    if (!Draw_dominant) ++VIS_mode;
	    if (black_arr[lay]) {
		black_arr[lay] = 0;
		post_cmd_proc (17);
		if (drawing_order[NR_dom-1] == lay) {
		    dom_arr[lay] = 0;
		    --NR_dom;
		    if (Draw_dominant) {
			++VIS_mode;
			init_colmenu ();
		    }
		}
	    }
	    else
		goto set_black;
	}
	else if (cmd_nbr != old) {
	    post_cmd_proc (old);
	    pre_cmd_proc (old = cmd_nbr);

	    process -> RT[lay] -> code = old - 1;
	    ggChangeColorCode (lay, old - 1);

	    if (old == 1 || old == 16) { /* black layer */
		if (!black_arr[lay]) {
set_black:
		    black_arr[lay] = 1;
		    pre_cmd_proc (17);
		    if (!dom_arr[lay]) {
			dom_arr[lay] = 1;
			drawing_order[NR_dom++] = lay;
			if (Draw_dominant) {
			    ++VIS_mode;
			    init_colmenu ();
			}
		    }
		    else if (Draw_dominant && cmd_nbr == 17)
			VIS_mode = -1; /* no bulb & redraw */
		}
	    }
	    else if (black_arr[lay]) {
		black_arr[lay] = 0;
		post_cmd_proc (17);
	    }
	    if (++VIS_mode == 1) bulb (lay, 1);
	}
	if (VIS_mode) {
	    VIS_mode = 0;
	    pict_all (ERAS_DR);
	    picture ();
	}
	get_cmd ();
    }
    if (cmd_nbr == -7) {
	post_cmd_proc (old);
	if (black_arr[lay]) post_cmd_proc (17);
	goto again;
    }
    ptext ("");
}

#define UNSET_ORDER  9
#define SET_ORDER   10

int toggle_pos (int nbr)
{
    int i, lay, old_nbr;

    /* set_coord command */
    if (VIS_mode == 2) return (0);

    if ((nbr += nbr_offset) >= NR_lay) return (0);

    if (dom_order_cmd) {
	if (dom_order_cmd >= SET_ORDER) {
	    if (dom_order_cmd == SET_ORDER) {
		ptext ("Toggle 2nd lay to put lay in front of!");
		dom_order_cmd = nbr + (SET_ORDER + 1);
	    }
	    else { /* nbr is new position */
		old_nbr = dom_order_cmd - (SET_ORDER + 1);
		if (old_nbr >= NR_dom && nbr >= NR_dom) { /* add */
		    if (old_nbr < nbr) {
			i = NR_dom;
			for (lay = 0; lay < NR_lay; ++lay) {
			    if (dom_arr[lay]) continue;
			    if (i != old_nbr)
				drawing_order[NR_dom++] = lay;
			    else
				drawing_order[nbr-1] = lay;
			    dom_arr[lay] = 1;
			    if (++i >= nbr) break;
			}
			NR_dom = nbr;
		    }
		    else {
			i = NR_dom;
			for (lay = 0; lay < NR_lay; ++lay) {
			    if (dom_arr[lay]) continue;
			    if (i < nbr) {
				drawing_order[NR_dom++] = lay;
				dom_arr[lay] = 1;
			    }
			    else if (i == old_nbr) {
				drawing_order[NR_dom++] = lay;
				dom_arr[lay] = 1;
				break;
			    }
			    ++i;
			}
		    }
		    init_colmenu ();
		    pict_all (ERAS_DR);
		    picture ();
		}
		else if (nbr != old_nbr && nbr - 1 != old_nbr &&
			nbr <= NR_dom) {
		    if (old_nbr >= NR_dom) { /* add old */
			i = NR_dom;
			for (lay = 0; lay < NR_lay; ++lay) {
			    if (dom_arr[lay]) continue;
			    if (i++ == old_nbr) break;
			}
			dom_arr[lay] = 1;
			i = NR_dom++;
		    }
		    else { /* old already in drawing_order */
			lay = drawing_order[i = old_nbr];
		    }
		    if (old_nbr > nbr) {
		      ++i;
		      while (--i > nbr)
			drawing_order[i] = drawing_order[i-1];
		    }
		    else {
		      while (++i < nbr)
			drawing_order[i-1] = drawing_order[i];
		      --i;
		    }
		    drawing_order[i] = lay;
		    init_colmenu ();
		    pict_all (ERAS_DR);
		    picture ();
		}
		ptext ("Toggle lay to set in dominant order position!");
		dom_order_cmd = SET_ORDER;
	    }
	}
	else if (dom_order_cmd == UNSET_ORDER && nbr < NR_dom) {
	    lay = drawing_order[nbr];
	    if (!black_arr[lay]) {
		dom_arr[lay] = 0;
		while (++nbr < NR_dom)
		    drawing_order[nbr-1] = drawing_order[nbr];
		--NR_dom;
		init_colmenu ();
		pict_all (ERAS_DR);
		picture ();
	    }
	    else
		notify ("Black lay cannot be unset!");
	}
	else if (dom_order_cmd < UNSET_ORDER) { /* set_color, set_fillst */
	    goto nxt;
	}
	return (0);
    }
nxt:
    if (Draw_dominant) {
        if (nbr < NR_dom) lay = drawing_order[nbr];
        else {
            i = NR_dom;
            for (lay = 0; lay < NR_lay; ++lay) {
                if (dom_arr[lay]) continue;
                if (i++ == nbr) break;
            }
	}
    }
    else lay = nbr;

    if (dom_order_cmd) { /* set_color, set_fillst */
	if (!vis_arr[lay]) {
	    notify ("layer not visible!");
	    return (0);
	}
	if (lay == selected_lay) return (0);

	for (i = 0; i < NR_lay; ++i)
	    if (def_arr[i] && i != lay) bulb (i, def_arr[i] = 0);
	if (!def_arr[lay]) bulb (lay, def_arr[lay] = 1);
	flush_pict ();
	selected_lay = lay;
	return (-7); /* back to menu */
    }
    toggle_lay (lay);
    return (0);
}

int nr_of_selected_lays ()
{
    int i, lay;

    for (i = lay = 0; lay < NR_lay; ++lay)
	if (def_arr[lay]) {
	    if (++i > 1) break;
	    selected_lay = lay;
	}
    if (i != 1) selected_lay = -1;
    return (i);
}
