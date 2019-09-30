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

#define MINGRDLNS 25
#define MAXGRDLNS 75
#define E_NR      3

extern struct Disp_wdw *p_wdw;
extern float XL, XR, YB, YT;
extern int *pict_arr;
extern int Gridnr;
extern int cmd_nbr, grid_cmd;
extern int *vis_arr;
extern int v_grid, v_sngrid;

int gridflag = 0; /* draw-flag for display and snap grid */
int not_disp_mess = 1;
int not_snap_mess = 1;

static Coor   grid_sp = 0;
static Coor  *grd_sp = NULL;
static char **ask_grid_str = NULL;
static int    no_grid_values = 0;
static int    adjust_grid = 1; /* default: YES */

static int default_grid_sp[] = {
	1, 2, 4, 8, 10, 20, 50, 100, 1000, 10000,
};

static int  adjust_grid_sp (void);
static void disp_grid (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
static void message_grid (void);

void no_grid_adjust ()
{
    adjust_grid = 0; /* NO */
}

void set_grid_values (int no_grd_sp, int *spc_list)
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
    ask_grid_str[1] = "auto-adjust";
    ask_grid_str[2] = "visible";

    for (i = 0; i < no_grd_sp; ++i) {
	grd_sp[i] = (Coor) spc_list[i] * QUAD_LAMBDA;
	MALLOCN (ask_grid_str[i + E_NR], char, 10);
	sprintf (ask_grid_str[i + E_NR], "%d", spc_list[i]);
    }
}

void set_grid_width (int grid_width)
{
    grid_sp = (Coor) grid_width * QUAD_LAMBDA;
    adjust_grid = 0; /* NO */
}

void display_grids (Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    if (!gridflag) return;
    if (vis_arr[v_grid])   disp_grid (wxl, wxr, wyb, wyt);
    if (vis_arr[v_sngrid]) disp_sn_grid (wxl, wxr, wyb, wyt);
}

static void disp_grid (Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    void d_grid ();
    char  infostr[80];
    int   nr;
    float dx, dy;

    if (adjust_grid) adjust_grid_sp ();

    dx = XR - XL;
    dy = YT - YB;
    if (dy > dx) dx = dy;
    nr = grid_sp ? dx / grid_sp : 0;
    if (nr < 1 || nr > MAXGRDLNS) {
	if (not_disp_mess) {
	    not_disp_mess = 0;
	    sprintf (infostr, "Grid not displayed! (spacing = %ld, #lines = %d)",
		grid_sp / QUAD_LAMBDA, nr);
	    notify (infostr);
	}
	return;
    }
    ggSetColor (Gridnr);
    d_grid (wxl, wxr, wyb, wyt, grid_sp);

    if (adjust_grid &&
	(float) wxl <= XL + 1.0 && (float) wxr >= XR - 1.0 &&
	(float) wyb <= YB + 1.0 && (float) wyt >= YT - 1.0) {
	message_grid ();
    }
}

int grid_on ()
{
    return (gridflag && (vis_arr[v_grid] || vis_arr[v_sngrid]));
}

void toggle_grid ()
{
    gridflag = !gridflag;
    if (vis_arr[v_grid] || vis_arr[v_sngrid]) {
	if (gridflag) {
	    if (vis_arr[v_grid])   not_disp_mess = 1;
	    if (vis_arr[v_sngrid]) not_snap_mess = 1;
	    pict_arr[Gridnr] = DRAW;
	}
	else
	    pict_arr[Gridnr] = ERAS_DR;
    }
    else if (gridflag)
	btext ("display- and snap-grid not visible!");
    if (grid_cmd) {
	if (gridflag) pre_cmd_proc (grid_cmd);
	else         post_cmd_proc (grid_cmd);
    }
}

void set_gr_spac ()
{
    register int old;

    if (!grd_sp) { /* no grid: set default grid */
	set_grid_values (sizeof (default_grid_sp) / sizeof (int), default_grid_sp);
    }
    if (adjust_grid) adjust_grid_sp ();
    if (!grid_sp) grid_sp = grd_sp[0]; /* self initialising */

    for (old = 0; old < no_grid_values; ++old) {
	if (grd_sp[old] == grid_sp) break;
    }
    old += E_NR;

do_menu:
    menu (no_grid_values + E_NR, ask_grid_str);
    if (adjust_grid) pre_cmd_proc (1);
    if (vis_arr[v_grid]) pre_cmd_proc (2);
    pre_cmd_proc (old);

    while (TRUE) {
	message_grid ();
	get_cmd ();
	switch (cmd_nbr) {
	    case 0:
		return;
	    case 1:
		adjust_grid = !adjust_grid; /* toggle */
		if (adjust_grid) {
		    pre_cmd_proc (cmd_nbr);
		    cmd_nbr = adjust_grid_sp () + E_NR;
		    if (cmd_nbr != old) {
			post_cmd_proc (old);
			pre_cmd_proc (old = cmd_nbr);
			if (vis_arr[v_grid] && gridflag) {
			    not_disp_mess = 1;
			    pict_arr[Gridnr] = ERAS_DR;
			}
		    }
		}
		else
		    post_cmd_proc (cmd_nbr);
		break;
	    case 2:
		vis_arr[v_grid] = !vis_arr[v_grid]; /* toggle */
		if (vis_arr[v_grid]) {
		    pre_cmd_proc (cmd_nbr);
		    if (gridflag) {
			not_disp_mess = 1;
			pict_arr[Gridnr] = DRAW;
		    }
		}
		else {
		    post_cmd_proc (cmd_nbr);
		    if (gridflag) pict_arr[Gridnr] = ERAS_DR;
		}
		break;
	    default:
		if (cmd_nbr < 0) { /* special key pressed! */
		    sleep (1);
		    goto do_menu; /* redraw menu */
		}
		if (!adjust_grid && cmd_nbr != old) {
		    post_cmd_proc (old);
		    grid_sp = grd_sp[cmd_nbr - E_NR];
		    pre_cmd_proc (old = cmd_nbr);
		    if (vis_arr[v_grid] && gridflag) {
			not_disp_mess = 1;
			pict_arr[Gridnr] = ERAS_DR;
		    }
		}
	}
	picture ();
    }
}

static int adjust_grid_sp ()
{
    int    nr, max_j;
    static int j = 0;
    float  dx, dy;

    if (!grd_sp) { /* no grid: set default grid */
	set_grid_values (sizeof (default_grid_sp) / sizeof (int), default_grid_sp);
    }
    max_j = no_grid_values - 1;
    dx = p_wdw -> wxmax - p_wdw -> wxmin;
    dy = p_wdw -> wymax - p_wdw -> wymin;
    if (dy > dx) dx = dy;
    nr = dx / grd_sp[j];
    while (nr > MAXGRDLNS && j < max_j) nr = dx / grd_sp[++j];
    while (nr < MINGRDLNS && j > 0) nr = dx / grd_sp[--j];
    if (nr > MAXGRDLNS && j < max_j) ++j;
    grid_sp = grd_sp[j];
    return (j);
}

void disp_axis ()
{
    int m = 0;
    if (YB < 0.0 && YT > 0.0) m = 1;
    if (XL < 0.0 && XR > 0.0) m += 2;
    if (m) {
	ggSetColor (Gridnr);
	d_ltype (LINE_DOTTED);
	if (m & 1) d_line (XL, 0.0, XR, 0.0);
	if (m & 2) d_line (0.0, YB, 0.0, YT);
	d_ltype (LINE_SOLID);
    }
}

static void message_grid ()
{
    char infostr[80];
    sprintf (infostr, "display-grid: spacing = %ld", grid_sp / QUAD_LAMBDA);
    ptext (infostr);
}
