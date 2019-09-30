/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
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

#define MAXGRDLNS 50	/* Displayable for default snap-settings */
#define E_NR       4

extern float XL, XR, YB, YT;
extern Coor xlc, ybc;
extern int *pict_arr;
extern int  Gridnr;
extern int  act_sngrid;
extern int  cmd_nbr, new_cmd;
extern int  erase_text;
extern int  gridflag; /* draw-flag for display and snap grid */
extern int  not_snap_mess;
extern int *vis_arr;
extern int  v_sngrid;

Coor sn_grid_sp = 0; /* In QUAD_LAMBDA */
Coor sn_grid_x = 0;  /* In QUAD_LAMBDA */
Coor sn_grid_y = 0;  /* In QUAD_LAMBDA */

static Coor  *grd_sp = NULL;
static char **ask_grid_str = NULL;
static int    no_grid_values = 0;

static int default_grid_sp[] = {
	1, 2, 4, 8, 10, 20, 50, 100, 1000, 10000,
};

void set_sn_grid_offset (int x_offset, int y_offset)
{
    sn_grid_x = (Coor) x_offset * QUAD_LAMBDA;
    sn_grid_y = (Coor) y_offset * QUAD_LAMBDA;
    vis_arr[v_sngrid] = 1; /* ON */
    act_sngrid = 1; /* activated */
}

void set_snap_grid_width (int grid_width)
{
    sn_grid_sp = (Coor) grid_width * QUAD_LAMBDA;
    vis_arr[v_sngrid] = 1; /* ON */
    act_sngrid = 1; /* activated */
}

void set_sn_grid_values (int no_grd_sp, int *spc_list)
{
    register int i;

    if (no_grid_values) {
	for (i = 0; i < no_grid_values; ++i) FREE (ask_grid_str[i + E_NR]);
	FREE (ask_grid_str);
	FREE (grd_sp);
    }

    no_grid_values = no_grd_sp;
    MALLOCN (grd_sp, Coor, no_grd_sp);
    MALLOCN (ask_grid_str, char *, no_grd_sp + E_NR);

    ask_grid_str[0] = "-ready-";
    ask_grid_str[1] = "active";
    ask_grid_str[2] = "visible";
    ask_grid_str[3] = "offset";

    for (i = 0; i < no_grd_sp; ++i) {
	grd_sp[i] = (Coor) spc_list[i] * QUAD_LAMBDA;
	MALLOCN (ask_grid_str[i + E_NR], char, 10);
	sprintf (ask_grid_str[i + E_NR], "%d", spc_list[i]);
    }
}

void disp_sn_grid (Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    char infostr[80];
    int  nr;
    float dx, dy;

    dx = XR - XL;
    dy = YT - YB;
    if (dy > dx) dx = dy;
    if (!sn_grid_sp) {
	if (grd_sp) sn_grid_sp = grd_sp[0];
	else sn_grid_sp = (Coor) default_grid_sp[0] * QUAD_LAMBDA;
    }
    nr = sn_grid_sp ? dx / sn_grid_sp : 0;
    if (nr < 1 || nr > MAXGRDLNS) {
	if (not_snap_mess) {
	    not_snap_mess = 0;
	    sprintf (infostr, "Snap-grid not displayed! (spacing = %ld, #lines = %d)", sn_grid_sp / QUAD_LAMBDA, nr);
	    notify (infostr);
	}
        return;
    }
    ggSetColor (Gridnr);
    d_snapgrid (wxl, wxr, wyb, wyt, sn_grid_sp, sn_grid_x, sn_grid_y);
}

void set_sn_gr_spac ()
{
    char infostr[80];
    register int old;

    if (!grd_sp) { /* no snap_grid: set default snap_grid */
	set_sn_grid_values (sizeof (default_grid_sp) / sizeof (int), default_grid_sp);
    }
    if (!sn_grid_sp) sn_grid_sp = grd_sp[0]; /* self initialising */

    for (old = 0; old < no_grid_values; ++old) {
	if (grd_sp[old] == sn_grid_sp) break;
    }
    old += E_NR;

do_menu:
    menu (no_grid_values + E_NR, ask_grid_str);
    if (act_sngrid) pre_cmd_proc (1);
    if (vis_arr[v_sngrid]) pre_cmd_proc (2);
    pre_cmd_proc (old);

    while (TRUE) {
	sprintf (infostr, "snap-grid: spacing = %ld,  offset = (%ld, %ld)",
	    sn_grid_sp / QUAD_LAMBDA,
	    (sn_grid_x % sn_grid_sp) / QUAD_LAMBDA,
	    (sn_grid_y % sn_grid_sp) / QUAD_LAMBDA);
	ptext (infostr);
	get_cmd ();
	switch (cmd_nbr) {
	case 0:
	    return;
	case 1:
	    act_sngrid = !act_sngrid; /* toggle */
	    if (act_sngrid)
		pre_cmd_proc (cmd_nbr);
	    else
		post_cmd_proc (cmd_nbr);
	    break;
	case 2:
	    vis_arr[v_sngrid] = !vis_arr[v_sngrid]; /* toggle */
	    if (vis_arr[v_sngrid]) {
		pre_cmd_proc (cmd_nbr);
		if (gridflag) {
		    not_snap_mess = 1;
		    pict_arr[Gridnr] = DRAW;
		}
	    }
	    else {
		post_cmd_proc (cmd_nbr);
		if (gridflag) pict_arr[Gridnr] = ERAS_DR;
	    }
	    break;
	case 3:
	    pre_cmd_proc (cmd_nbr);
	    ptext ("Enter new offset position!");
	    erase_text = 1;
	    new_cmd = get_cursor (1, 1, NO_SNAP);
	    if (new_cmd == -1 && sn_grid_x != xlc && sn_grid_y != ybc) {
		sn_grid_x = xlc;
		sn_grid_y = ybc;
		if (vis_arr[v_sngrid] && gridflag) pict_arr[Gridnr] = ERAS_DR;
	    }
	    post_cmd_proc (cmd_nbr);
	    break;
	default:
	    if (cmd_nbr < 0) { /* special key pressed! */
		sleep (1);
		goto do_menu; /* redraw menu */
	    }
	    if (cmd_nbr != old) {
		post_cmd_proc (old);
		sn_grid_sp = grd_sp[cmd_nbr - E_NR];
		pre_cmd_proc (old = cmd_nbr);
		if (vis_arr[v_sngrid] && gridflag) {
		    not_snap_mess = 1;
		    pict_arr[Gridnr] = ERAS_DR;
		}
	    }
	}
	picture ();
    }
}
