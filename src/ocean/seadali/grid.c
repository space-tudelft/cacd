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

#define NBR_GRDSP 11
#define MINGRDLNS 25
#define MAXGRDLNS 75

extern struct Disp_wdw *c_wdw;
extern int *pict_arr;
extern int Gridnr;
extern int ImageMode;
extern char * ThisImage;

static Coor grid_sp = QUAD_LAMBDA;
static int adjust_grid = TRUE;
static int draw_grid = TRUE;	/* draw-flag for display grid. */
static int gridflag = FALSE;	/* draw-flag for display and snap grid. */

static Coor * grd_sp = NULL;
static char ** ask_grid_str = NULL;
static int no_grid_values = 0;

static int default_grid_sp[] = {
	1, 2, 4, 8, 10, 20, 50, 100, 1000, 10000,
};

static void adjust_grid_sp (void);
static void disp_grid (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
static void message_grid (void);

void no_grid_adjust ()
{
    adjust_grid = FALSE;
}

void set_grid_values (int no_grd_sp, int *spc_list)
{
    int i;
    static char * returnStr = "-ready-";
    static char * adjustStr = "auto-adjust";
    static char * drawGridStr = "on / off";

    no_grid_values = no_grd_sp;

    grd_sp = (Coor *) calloc (no_grd_sp, sizeof (Coor));
    ask_grid_str = (char **) calloc (no_grd_sp + 3, sizeof (char *));
    ask_grid_str[0] = returnStr;
    ask_grid_str[1] = adjustStr;
    ask_grid_str[2] = drawGridStr;

    for (i = 0; i < no_grd_sp; i++) {
	int value;
	ask_grid_str[i+3] = (char *) calloc (10, sizeof (char));

	value = spc_list[i];
	if (value <= 0) value = 1;

	grd_sp[i] = (Coor) value * QUAD_LAMBDA;
	sprintf (ask_grid_str[i + 3], "%d", value);
    }
}

void set_grid_width (int grid_width)
{
    if (grid_sp <= 0) grid_sp = grd_sp[0];
    else grid_sp = (Coor) grid_width * QUAD_LAMBDA;
}

void display_grids (Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    if (gridflag == FALSE) return;

    disp_grid (wxl, wxr, wyb, wyt);
    disp_sn_grid (wxl, wxr, wyb, wyt);
}

static void disp_grid (Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    char  infostr[80];
    int   nr;
    float dx, dy;

    if (!draw_grid) return;
    if (adjust_grid) adjust_grid_sp ();

    dx = XR - XL;
    dy = YT - YB;
    if (dy > dx) dx = dy;
    nr = dx / grid_sp;
    if (nr < 1 || nr > MAXGRDLNS) {
	sprintf (infostr, "Grid not displayed! (#lines = %d)", nr);
	ptext (infostr);
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
    return (gridflag);
}

void toggle_grid ()
{
    if (gridflag == TRUE) {
	gridflag = FALSE;
	pict_arr[Gridnr] = ERAS_DR;
    }
    else {
	gridflag = TRUE;
	pict_arr[Gridnr] = DRAW;
    }
}

void switch_grid (int show_it)
{
    if (show_it == TRUE) {
	gridflag = TRUE;
	pict_arr[Gridnr] = DRAW;
    }
    else {
	gridflag = FALSE;
	pict_arr[Gridnr] = ERAS_DR;
    }
}

void set_gr_spac ()
{
    register int j;
    int nbr;

    if (!grd_sp) {
	set_grid_values (sizeof (default_grid_sp) / sizeof (int), default_grid_sp);
    }
    if (grid_sp == 0) grid_sp = grd_sp[0]; /* self initialising */

start_menu:
    menu (no_grid_values + 3, ask_grid_str);
    if (adjust_grid) pre_cmd_proc (1, ask_grid_str);
    if (draw_grid) pre_cmd_proc (2, ask_grid_str);

    message_grid ();

    while (1) {

	for (j = 0; j < no_grid_values; ++j) {
	    if (grd_sp[j] == grid_sp) break;
	}
	if (j < no_grid_values) pre_cmd_proc (j + 3, ask_grid_str);

	if ((nbr = get_com ()) == 0) break;
	if (nbr == -2) goto start_menu;

	switch (nbr) {
	    case 1:
		if (adjust_grid) {
		    adjust_grid = FALSE;
		    post_cmd_proc (1, ask_grid_str);
		}
		else {
		    adjust_grid = TRUE;
		    adjust_grid_sp ();
		    pre_cmd_proc (1, ask_grid_str);
		    pict_arr[Gridnr] = (gridflag == TRUE) ? ERAS_DR : DRAW;
		}
		break;
	    case 2:
		if (draw_grid) {
		    draw_grid = FALSE;
		    post_cmd_proc (2, ask_grid_str);
		}
		else {
		    draw_grid = TRUE;
		    pre_cmd_proc (2, ask_grid_str);
		}
		pict_arr[Gridnr] = (gridflag == TRUE) ? ERAS_DR : DRAW;
		break;
	    default:
		if (!adjust_grid) {
		    grid_sp = grd_sp[nbr - 3];
		    pict_arr[Gridnr] = (gridflag == TRUE) ? ERAS_DR : DRAW;
		}
	}

	if (j < no_grid_values) post_cmd_proc (j+3, ask_grid_str);
	picture ();
	message_grid ();
    }
}

static void adjust_grid_sp ()
{
    int    nr;
    static int j = 0;
    float  dx, dy;

    if (!adjust_grid) return;

    if (!grd_sp) {
        set_grid_values (sizeof (default_grid_sp) / sizeof (int), default_grid_sp);
    }

    dx = XR - XL;
    dy = YT - YB;
    if (dy > dx) dx = dy;
    nr = dx / grd_sp[j];
    while (nr > MAXGRDLNS && j < NBR_GRDSP - 1) nr = dx / grd_sp[++j];
    while (nr < MINGRDLNS && j > 0) nr = dx / grd_sp[--j];
    if (nr > MAXGRDLNS && j < NBR_GRDSP - 1) ++j;

    grid_sp = grd_sp[j];
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
    char infostr [80];

    if (ImageMode == TRUE && ThisImage) {
	return; /* its a useless message... */
    /*	sprintf (infostr, "display-grid: The grid points of the image '%s' are displayed", ThisImage); */
    }
    else
	sprintf (infostr, "display-grid: spacing = %ld", grid_sp / QUAD_LAMBDA);
    ptext (infostr);
}
