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

#include "X11/Xlib.h"

#include "src/dali/header.h"

FILE *rcfile = NULL;
DM_PROJECT *dmproject;
DM_CELL *ckey = NULL;
char  *argv0;
char  *cellstr = NULL;
INST  *inst_root;
TEMPL *first_templ;
TERM **term_root;
TERM **TBuf;
struct obj_node **PutBuf;

struct drc **drc_data = NULL;
 /* structure containing design rules of used process */
qtree_t **quad_root; /* pointer to root of the quad trees */

int     act_mask_lay;
int     act_sngrid = 0; /* if true: snap grid activated */
int     ask_iname = 0; /* ask instance name by add_inst (if true) */
int     checked;
int     exp_level = 1; /* expansion level of overall expansions */
int     zoom_mode = 2; /* default zoom/dezoom mode used */
int     fmode = 0; /* read dali_drc from CWD (if true) */
int     rmode = 0; /* read only mode (if true) */
int     omode = 0; /* animate output mode (if true) */
int     use_new_mode = 0; /* if true: use new name after write */
int     Default_expansion_level = 1; /* initial exp. level */
int     Draw_dominant = 0; /* OFF */
int     Draw_hashed = 0; /* OFF */
int     NR_dom = 0; /* number of dominant lays in drawing order */
int     NR_lay = 0; /* number of layers in used process */
int     NR_all = 0; /* number of lays + whites */
int     Nr_p = 0; /* Number of points in line buffer for redraw */
int    *term_lay;
int    *def_arr;
int    *drawing_order;
int    *ggeIds;
int    *non_edit;
int    *black_arr;
int    *dom_arr;
int    *pict_arr; /* for all layers + text/b-boxes + grid + design-rule-errors */
int    *fillshift;
int    *fillst;
int    *vis_arr; /* for all layers + terminals + subterms */
int     v_grid, v_sngrid, v_term, v_sterm, v_inst, v_bbox;
int     v_tname, v_stname, v_iname;
int     v_label, v_comment;

float  XL, XR, YB, YT;		/* current world window */
float  c_cW, c_cH;		/* current char. width/height */
Coor piwl, piwr, piwb, piwt;	/* picture drawing window */
Coor xlbb, xrbb, ybbb, ytbb;	/* bound box: boxes + terminals */
Coor xlmb, xrmb, ybmb, ytmb;	/* bound box: instances */
Coor xltb, xrtb, ybtb, yttb;	/* bound box: total */
Coor xlc, xrc, ybc, ytc;	/* cursor parameters */
Coor *Line_x, *Line_y;

char *yes_no[] = {
/* 0 */ "no",
/* 1 */ "yes",
};

char *ready_cancel[] = {
/* 0 */ "-ready-",
/* 1 */ "-cancel-",
};

int    new_cmd = -1;
int    cmd_nbr;
int    in_cmd;
int    grid_cmd;

struct Disp_wdw *wdw_arr[10];
struct Disp_wdw *c_wdw;
struct Disp_wdw *p_wdw;

int     CHECK_FLAG;
int     CLIP_FLAG = 0; /* do not clip */
int     nr_planes;
int     Backgr;
int     Cur_nr;
int     DRC_nr;
int     Gridnr;
int     Textnr;
int     Yellow;

int     colormap = 1; /* Colormap mode (if true) */
int     d_apollo = 0; /* Apollo mode (if true) */
char   *DisplayName;
char   *geometry;

/*
 * These are default settings which are convenient for Sea-of-Gates:
 */
char *ImageName; /* inst_name of the image */
char *ViaName;   /* inst_name of via's (1st three char's) */
int  MaxDrawImage = 120; /* draw at most 120 image cells on the screen */
int  ImageMode = -1; /* init imagemode to no set */

