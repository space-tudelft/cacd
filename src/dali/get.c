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

extern struct Disp_wdw *c_wdw;
extern int  Curr_nr_alt;
extern int  nbr_max;
extern int  act_sngrid;
extern int  cmd_nbr, in_cmd, new_cmd;
extern Coor xlc, xrc, ybc, ytc;
extern Coor sn_grid_sp;		/* In QUAD_LAMBDA */
extern Coor sn_grid_x;		/* In QUAD_LAMBDA */
extern Coor sn_grid_y;		/* In QUAD_LAMBDA */

static int  menu_command = 0;

static int get_pos (int echo, float *p_x, float *p_y);

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
    float f_x;
    menu_command = 1;
    return (get_pos (1, &f_x, &f_x));
}

/*
** Get picture cursor position or menu command number.
*/
static int get_pos (int echo, float *p_x, float *p_y)
{
    float f_x, f_y;
    int   nbr;
    struct Disp_wdw *old_wdw = c_wdw;

    set_c_wdw (PICT);
    if (echo > 1) fix_loc (*p_x, *p_y, echo);

    while (TRUE) {
	switch (get_loc (&f_x, &f_y, echo)) {
	case MENU:
	    nbr = f_y;
	    menu_command = 0;
	    if ((nbr = f_y) < 0) {
		if (nbr == -1) --nbr;
	    }
	    else if (nbr >= Curr_nr_alt && !in_cmd) {
		nbr = Curr_nr_alt - 1;
	    }
	    set_c_wdw (old_wdw -> w_nr);
	    return (nbr);
	case LAYS:
	    nbr = f_y;
	    if (nbr >= 0 && nbr < nbr_max) {
		nbr = toggle_pos (nbr);
		if (nbr) { /* go back to menu */
		    set_c_wdw (old_wdw -> w_nr);
		    return (nbr);
		}
	    }
	    else if (nbr == nbr_max) {
		if (f_x < 5) next_colmenu ();
		if (f_x > 5) prev_colmenu ();
	    }
	    set_c_wdw (PICT);
	    break;
	case PICT:
	    if (!menu_command) {
		*p_x = f_x;
		*p_y = f_y;
		return (-1);
	    }
	default:
	    if (menu_command)
		notify ("No command pointed at!");
	    else
		notify ("Indicated spot outside window, try again!");
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
    int   rv;

    if (non_orth && !act_sngrid) { /* and NOT snapping! */
	rv = get_all (echo, x, y); /* get half LAMBDA grid point */
    }
    else if ((width / QUAD_LAMBDA) % 2 == 0) {
	rv = get_one (echo, x, y); /* get LAMBDA grid point */
    }
    else { /* odd width */
	f_x = (float) (*x);
	f_y = (float) (*y);
	menu_command = 0;
	if ((rv = get_pos (echo, &f_x, &f_y)) == -1) {
	    /* in picture viewport */
	    /* get half LAMBDA shifted LAMBDA grid point */
	    *x = ((Coor) Round ((f_x + (float) (QUAD_LAMBDA / 2)) / (float) QUAD_LAMBDA))
		    * (Coor) QUAD_LAMBDA - (Coor) (QUAD_LAMBDA / 2);
	    *y = ((Coor) Round ((f_y + (float) (QUAD_LAMBDA / 2)) / (float) QUAD_LAMBDA))
		    * (Coor) QUAD_LAMBDA - (Coor) (QUAD_LAMBDA / 2);
	}
    }
    return (rv);
}

/*
** Get one LAMBDA grid point.
*/
int get_one (int echo, Coor *x, Coor *y)
{
    float f_x, f_y;
    int   rv;
    Coor x_off = 0;
    Coor y_off = 0;
    Coor spac = QUAD_LAMBDA;

    if (act_sngrid) {
	x_off = sn_grid_x % sn_grid_sp;
	y_off = sn_grid_y % sn_grid_sp;
	spac = sn_grid_sp;
    }

    f_x = (float) *x;
    f_y = (float) *y;
    menu_command = 0;
    if ((rv = get_pos (echo, &f_x, &f_y)) < 0) {
	/* in picture viewport */
	*x = ((Coor) Round ((f_x - (float) x_off) / (float) spac)) * spac + x_off;
	*y = ((Coor) Round ((f_y - (float) y_off) / (float) spac)) * spac + y_off;
    }
    return (rv);
}

/*
** Get one half LAMBDA grid point.
*/
int get_all (int echo, Coor *x, Coor *y)
{
    float f_x, f_y;
    int   rv;
    Coor  half_grid = QUAD_LAMBDA / 2;

    f_x = (float) (*x);
    f_y = (float) (*y);
    menu_command = 0;
    if ((rv = get_pos (echo, &f_x, &f_y)) < 0) {
	/* in picture viewport */
	*x = ((Coor) Round (f_x / (float) half_grid)) * (Coor) half_grid;
	*y = ((Coor) Round (f_y / (float) half_grid)) * (Coor) half_grid;
    }
    return (rv);
}

/*
** Requesting one or two cursor positions:
** By echo == 5, rubberbox cursor is used.
** By echo == 4, rubberline cursor.
** By echo == 3, bbox cursor is used.
** If (return_value < 0) {
**    One or two picture cursor positions are returned.
**    Positions are returned in global variables:
**      Cursor position 1 = (xlc, ybc)
**      Cursor position 2 = (xrc, ytc) (if num == 2)
**      Note that: xlc <= xrc  and  ybc <= ytc (rubberbox).
** }
** Else {
**    A menu command number is returned.
** }
*/
int get_cursor (int echo, int num, int snap)
{
    Coor swap;
    int  rv, save_snap = act_sngrid;

    if (snap == NO_SNAP) act_sngrid = 0; /* switch snapping temporary off */

    if ((rv = get_one (echo != 6 ? 1 : 6, &xlc, &ybc)) == -1 && num == 2) {
	xrc = xlc;
	ytc = ybc;
	if ((rv = get_one (echo, &xrc, &ytc)) == -1 && echo == 5) {
	    if (xrc < xlc) { swap = xrc; xrc = xlc; xlc = swap; }
	    if (ytc < ybc) { swap = ytc; ytc = ybc; ybc = swap; }
	}
    }

    if (snap == NO_SNAP) act_sngrid = save_snap;
    return (rv);
}
