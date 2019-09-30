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

#include "X11/Xlib.h"

#include "src/ocean/seadali/header.h"

DM_PROJECT *dmproject;
DM_CELL *ckey;
char *cellstr;
char cirname[DM_MAXNAME + 10];
char *argv0;

INST *inst_root;
INST *act_inst;

TEMPL *first_templ;

TERM **term_root;
TERM *act_term;

int     act_t_lay;
int     act_mask_lay;
int    *MaskLink;

struct drc **drc_data; /* structure containing design rules of used process */
qtree_t **quad_root; /* pointer to root of the quad trees */

int     rmode = 0; /* read only mode (if true) */
int     exp_level; /* expansion level of overall expansions */
int     sub_term_flag;
int     checked;
int     dirty;      /* TRUE if the cell was edited */
int     NR_lay;
int     NR_all;
int     Sub_terms; /* TRUE to indicate sub terminals */
int    *term_bool;
int    *def_arr;
int    *edit_arr;
int    *dom_arr;
int    *fillst;
int    *vis_arr; /* for all layers + terminals + subterms */

int    *eIds; /* erased Ids of pict_arr */
int    *pict_arr; /* for all layers + text/b-boxes + grid + design-rule-errors */

Coor piwl, piwr, piwb, piwt;	/* picture drawing window */
Coor xlbb, xrbb, ybbb, ytbb;	/* bound box: boxes + terminals */
Coor xlmb, xrmb, ybmb, ytmb;	/* bound box: instances */
Coor xlim, xrim, ybim, ytim;    /* bounding box of instances and boxes without the image */
Coor xltb, xrtb, ybtb, yttb;	/* bound box: total */
Coor xlc, xrc, ybc, ytc;	/* cursor parameters */

char *yes_no[] = {
/* 0 */ "&yes",
/* 1 */ "&no",
};

char *ready_cancel[] = {
/* 0 */ "-ready-",
/* 1 */ "-cancel-",
};

int    new_cmd = -1;
int    cmd_nbr;

struct Disp_wdw *wdw_arr[10];
struct Disp_wdw *c_wdw;
struct Disp_wdw *p_wdw;

short   CHECK_FLAG;
short   CLIP_FLAG;
int     nr_planes;
int     Backgr;
int     Cur_nr;
int     DRC_nr;
int     Gridnr;
int     Textnr;
int     Yellow;

int     d_apollo = 0; /* Apollo mode (if true) */
char   *DisplayName;
char   *geometry;

long Clk_tck;			  /* either CLK_TCK or from sysconf() */

