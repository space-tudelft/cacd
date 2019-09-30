/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
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

extern Coor xlc, ybc;
extern struct Disp_wdw *c_wdw;
extern int *pict_arr;
extern int Gridnr;
extern int new_cmd;
extern int cmd_nbr;

int draw_sngrid = FALSE;

Coor sn_grid_sp = QUAD_LAMBDA;	/* In QUAD_LAMBDA */
Coor sn_grid_x = 0;     	/* In QUAD_LAMBDA */
Coor sn_grid_y = 0;     	/* In QUAD_LAMBDA */

static Coor * grd_sp = NULL;
static char ** ask_grid_str = NULL;
static int no_grid_values = 0;

static int default_grid_sp[] = {
	1, 2, 4, 8, 10, 20, 50, 100, 1000, 10000,
};

static void message_snap_grid (void);

void set_sn_grid_offset (int x_offset, int y_offset)
{
    sn_grid_x = (Coor) x_offset * QUAD_LAMBDA;
    sn_grid_y = (Coor) y_offset * QUAD_LAMBDA;

    draw_sngrid = TRUE;
}

void set_snap_grid_width (int grid_width)
{
    if (sn_grid_sp <= 0) {
	sn_grid_sp = grd_sp[0];
    }
    else {
	sn_grid_sp = (Coor) grid_width * QUAD_LAMBDA;
    }
    draw_sngrid = TRUE;
}

void set_sn_grid_values (int no_grd_sp, int *spc_list)
{
    int i;
    static char * returnStr = "-ready-";
    static char * drawGridStr = "on / off";
    static char * offsetStr = "offset";

    no_grid_values = no_grd_sp;

    grd_sp = (Coor *) calloc (no_grd_sp, sizeof (Coor));
    ask_grid_str = (char **) calloc (no_grd_sp + 3, sizeof (char *));
    ask_grid_str[0] = returnStr;
    ask_grid_str[1] = drawGridStr;
    ask_grid_str[2] = offsetStr;

    for (i = 0; i < no_grd_sp; i++) {
	int value;
	ask_grid_str[i+3] = (char *) calloc (10, sizeof (char));

	value = spc_list[i];
	if (value <= 0) value = 1;

	grd_sp[i] = (Coor) value * QUAD_LAMBDA;
	sprintf (ask_grid_str[i + 3], "%d", value);
    }
}

#define MAXGRDLNS 50	/* Displayable for default snap-settings. */

void disp_sn_grid (Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    char  infostr[80];
    int nr;
    float dx, dy;

    if (!draw_sngrid) return;

    dx = XR - XL;
    dy = YT - YB;
    if (dy > dx) dx = dy;
    nr = dx / sn_grid_sp;
    if (nr < 1 || nr > MAXGRDLNS) {
        sprintf (infostr, "Snap-grid not displayed! (spacing = %ld, #lines = %d)", sn_grid_sp / QUAD_LAMBDA, nr);
        ptext (infostr);
        return;
    }

    ggSetColor (Gridnr);

    d_snapgrid (wxl, wxr, wyb, wyt, sn_grid_sp, sn_grid_x, sn_grid_y);
}

void set_sn_gr_spac ()
{
    register int j;

    if (!grd_sp) {
	set_sn_grid_values (sizeof (default_grid_sp) / sizeof (int), default_grid_sp);
    }
    if (sn_grid_sp == 0) sn_grid_sp = grd_sp[0]; /* self initialising */
    for (j = 0; j < no_grid_values; ++j) {
	if (grd_sp[j] == sn_grid_sp) break;
    }

start_menu:
    menu (no_grid_values + 3, ask_grid_str);

    if (draw_sngrid) pre_cmd_proc (1, ask_grid_str);

    if (j < no_grid_values) pre_cmd_proc (j + 3, ask_grid_str);

    message_snap_grid ();

    while (TRUE) {
	get_cmd ();
	switch (cmd_nbr) {
	case -2:
	    goto start_menu;
	case 0:
	    return;
	case 1:
	    if (draw_sngrid) {
		draw_sngrid = FALSE;
		post_cmd_proc (cmd_nbr, ask_grid_str);
	    }
	    else {
		draw_sngrid = TRUE;
		pre_cmd_proc (cmd_nbr, ask_grid_str);
	    }
	    pict_arr[Gridnr] = (draw_sngrid == FALSE) ? ERAS_DR : DRAW;
	    break;
	case 2:
	    pre_cmd_proc (cmd_nbr, ask_grid_str);

	    if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		sn_grid_x = xlc;
		sn_grid_y = ybc;
		new_cmd = cmd_nbr;
		if (draw_sngrid == TRUE) pict_arr[Gridnr] = ERAS_DR;
	    }
	    if (new_cmd == -2) goto start_menu;
	    break;
	default:
	    if (j < no_grid_values) post_cmd_proc (j+3, ask_grid_str);
	    sn_grid_sp = grd_sp[cmd_nbr - 3];
	    j = cmd_nbr - 3;
	    pre_cmd_proc (j + 3, ask_grid_str);
	    if (draw_sngrid == TRUE) pict_arr[Gridnr] = ERAS_DR;
	}
	picture ();
	if (cmd_nbr == 2) post_cmd_proc (cmd_nbr, ask_grid_str);
	message_snap_grid ();
    }
}

static void message_snap_grid ()
{
    char infostr[80];
    sprintf (infostr, "snap-grid: spacing = %ld,  offset = (%ld, %ld)",
	sn_grid_sp / QUAD_LAMBDA, (sn_grid_x % sn_grid_sp) / QUAD_LAMBDA, (sn_grid_y % sn_grid_sp) / QUAD_LAMBDA);
    ptext (infostr);
}
