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

#define SEA_CHILD_DIES 20

extern struct Disp_wdw *c_wdw;
extern int  new_cmd, cmd_nbr;
extern int *def_arr;
extern int *edit_arr;
extern int *pict_arr;
extern int *vis_arr;
extern Coor xlc, xrc, ybc, ytc;
extern int  NR_lay;
extern int  nbr_max;
extern int  nbr_offset;
extern Coor sn_grid_sp;		/* In QUAD_LAMBDA */
extern Coor sn_grid_x;		/* In QUAD_LAMBDA */
extern Coor sn_grid_y;		/* In QUAD_LAMBDA */
extern int draw_sngrid;

/* Coordinate variables for tracker. */
extern Coor sx;
extern Coor sy;
extern Coor endx;
extern Coor endy;
extern Coor dx;
extern Coor dy;
extern int *MaskLink;
extern int vismenu_mode;

static void toggle_lay (int nbr);

/*
** Get next menu command number.
*/
void get_cmd ()
{
    cmd_nbr = new_cmd < 0 ? get_com () : new_cmd;
    new_cmd = -1;
}

/*
** Get menu command number.
*/
int get_com ()
{
    float f_wx, f_wy;
    int nr;

    while (TRUE) {
	switch (get_loc (&f_wx, &f_wy, 1)) {
	case MENU:
	    return ((int) f_wy);
	case LAYS:
	    nr = (int) f_wy;
	    if (nr < nbr_max) toggle_lay (nr);
	    else if (f_wx < 0.5) next_colmenu ();
	    else if (f_wx > 0.5) prev_colmenu ();
	    break;
	case SEA_CHILD_DIES:
	    return ((int) 0);
	default:
	    ptext ("No command pointed at, please select a menu item.");
	}
    }
}

/*
** Get picture cursor position or menu command number.
*/
static int get_pos (int echo, float *p_x, float *p_y)
{
    float nw_x, nw_y;
    int nr;

    nw_x = nw_y = 0.0;

    while (TRUE) {
	if (echo >= 4) {
	    fix_loc (*p_x, *p_y);
	}
	switch (get_loc (&nw_x, &nw_y, echo)) {
	case MENU:
	    return ((int) nw_y);
	case LAYS:
	    nr = (int) nw_y;
	    if (nr < nbr_max) toggle_lay (nr);
	    else if (nw_x < 0.5) next_colmenu ();
	    else if (nw_x > 0.5) prev_colmenu ();
	    break;
	case PICT:
	    *p_x = nw_x;
	    *p_y = nw_y;
	    return (-1);
	default:
	    ptext ("Indicated spot outside window, try again!");
	}
    }
}

/*
** Get one wire point.
** This point must be on LAMBDA grid for even widths
** and on half LAMBDA grid for odd widths.
** Only if non-orthogonal flag is true, the point may
** always be on half LAMBDA grid (if NOT snapping!).
*/
int get_wire_point (int echo, Coor *x, Coor *y, int width, int non_orth)
{
    float f_x, f_y;
    int   ret_value;

    if (non_orth && !draw_sngrid) { /* and NOT snapping! */
	ret_value = get_all (echo, x, y); /* get half LAMBDA grid point */
    }
    else if ((width / QUAD_LAMBDA) % 2 == 0) {
	ret_value = get_one (echo, x, y); /* get LAMBDA grid point */
    }
    else { /* odd width */
	f_x = (float) (*x);
	f_y = (float) (*y);
	if ((ret_value = get_pos (echo, &f_x, &f_y)) == -1) {
	    /* in picture viewport */
	    /* get half LAMBDA shifted LAMBDA grid point */
	    *x = ((Coor) Round ((f_x + (float) (QUAD_LAMBDA / 2)) / (float) QUAD_LAMBDA))
		    * (Coor) QUAD_LAMBDA - (Coor) (QUAD_LAMBDA / 2);
	    *y = ((Coor) Round ((f_y + (float) (QUAD_LAMBDA / 2)) / (float) QUAD_LAMBDA))
		    * (Coor) QUAD_LAMBDA - (Coor) (QUAD_LAMBDA / 2);
	}
    }
    return (ret_value);
}

/*
** Get one LAMBDA grid point.
*/
int get_one (int echo, Coor *x, Coor *y)
{
    float f_x, f_y;
    int   ret_value;
    Coor x_off = 0;
    Coor y_off = 0;
    Coor spac = QUAD_LAMBDA;

    if (draw_sngrid) {
	x_off = sn_grid_x % sn_grid_sp;
	y_off = sn_grid_y % sn_grid_sp;
	spac = sn_grid_sp;
    }

    f_x = (float) (*x);
    f_y = (float) (*y);
    if ((ret_value = get_pos (echo, &f_x, &f_y)) == -1) {
	/* in picture viewport */
	*x = ((Coor) Round ((f_x - (float) x_off) / (float) spac)) * spac + x_off;
	*y = ((Coor) Round ((f_y - (float) y_off) / (float) spac)) * spac + y_off;
    }
    if (draw_sngrid) {
	/* We have performed snapping.  Correct tracker info. */
	if (echo >= 4) {
	    endx = *x / QUAD_LAMBDA;
	    endy = *y / QUAD_LAMBDA;
	    dx = Abs (endx - sx);
	    dy = Abs (endy - sy);
	}
	else {
	    sx = *x / QUAD_LAMBDA;
	    sy = *y / QUAD_LAMBDA;
	}
    }
    return (ret_value);
}

/*
** Get one half LAMBDA grid point.
*/
int get_all (int echo, Coor *x, Coor *y)
{
    float f_x, f_y;
    int   ret_value;
    Coor  half_grid = QUAD_LAMBDA / 2;

    f_x = (float) (*x);
    f_y = (float) (*y);
    if ((ret_value = get_pos (echo, &f_x, &f_y)) == -1) {
	/* in picture viewport */
	*x = ((Coor) Round (f_x / (float) half_grid)) * (Coor) half_grid;
	*y = ((Coor) Round (f_y / (float) half_grid)) * (Coor) half_grid;
    }
    return (ret_value);
}

/*
** Requesting one or two cursor positions:
** If (return_value == -1) {
**    One or two picture cursor positions are returned.
**    Positions are returned in global variables:
**      Cursor position 1 = (xlc, ybc)
**      Cursor position 2 = (xrc, ytc) (if num == 2)
**      Note that: xlc <= xrc  and  ybc <= ytc.
** }
** Else {
**    A menu command number is returned.
** }
*/
int set_tbltcur (int num, int snap)
{
    Coor swap;
    int tmp;
    int save_snap = draw_sngrid;

    if (snap == NO_SNAP) draw_sngrid = FALSE; /* switch snapping temporary off */

    if ((tmp = get_one (1, &xlc, &ybc)) != -1) goto ready; /* command selected */

    if (num == 2) {
	xrc = xlc;
	ytc = ybc;
	if ((tmp = get_one (5, &xrc, &ytc)) != -1) goto ready; /* command selected */
	if (xrc < xlc) { swap = xrc; xrc = xlc; xlc = swap; }
	if (ytc < ybc) { swap = ytc; ytc = ybc; ybc = swap; }
    }
    /* If tmp == -1: returning picture cursor position */
ready:
    if (snap == NO_SNAP) draw_sngrid = save_snap;
    return (tmp);
}

int get_line_points (int num, int snap)
{
    Coor swap;
    int tmp;
    int save_snap = draw_sngrid;

    if (snap == NO_SNAP) draw_sngrid = FALSE; /* switch snapping temporary off */

    if ((tmp = get_one (1, &xlc, &ybc)) != -1) goto ready; /* command selected */

    if (num == 2) {
	xrc = xlc;
	ytc = ybc;
	/* use rubber line cursor */
	if ((tmp = get_one (4, &xrc, &ytc)) != -1) goto ready; /* command selected */
	/* no sorting, (xlc, ybc) is first point; (xrc, ytc) is second point */
    }
    /* If tmp == -1: returning picture cursor position */
ready:
    if (snap == NO_SNAP) draw_sngrid = save_snap;
    return (tmp);
}

/*
** Set/Reset color layer viewport layer number.
*/
static void toggle_lay (int nbr)
{
    int i, lay;

    lay = nbr + nbr_offset;
    if (lay < 0 || lay >= NR_lay) return;

    if (vismenu_mode) {
	if (vis_arr[lay]) {
	    def_arr[lay] = 0;
	    vis_arr[lay] = 0; /* OFF */
	    pict_arr[lay] = ERAS_DR;
	}
	else {
	    vis_arr[lay] = 1; /* ON */
	    pict_arr[lay] = DRAW;
	}
	paint_lay (nbr);
	picture ();
	return;
    }

    if (!edit_arr[lay]) return;
    if (!vis_arr[lay]) return;

    def_arr[lay] = !def_arr[lay];
    bulb (lay, def_arr[lay]);

   /*
    * Patrick: if the masks are linked: switch them off together
    */
    if (!MaskLink || MaskLink[lay] == 0) return;

    for (i = 0; i < NR_lay; ++i) {
	if (i != lay && MaskLink[i] == MaskLink[lay]) {
	    def_arr[i] = def_arr[lay];
	    bulb (i, def_arr[i]);
	}
    }
}
