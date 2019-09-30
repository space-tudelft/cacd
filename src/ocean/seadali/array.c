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

#define MAX_NX_NY   10000

extern INST *act_inst;
extern int  *term_bool;
extern int  *pict_arr;
extern int  *vis_arr;
extern int   NR_lay;
extern int   Textnr;
extern Coor  piwl, piwr, piwb, piwt; /* window to be drawn */

int ask_par (char ***arr_p_p)
{
    static char *par_str[] = { /* static for redraw in multi-window */
	"-cancel-", "nx", "ny", "dx", "dy"
    };
    *arr_p_p = par_str;
    return (ask (sizeof (par_str) / sizeof (char *), par_str, -1));
}

int nx_ny_par (int *nx_ny, Coor *dx_dy, Coor dd, char *obj_name)
{
    char par_str[MAXCHAR];
    char input_str[MAXCHAR];
    int  par;

    par = *nx_ny;
    sprintf (input_str, "number of repetitions of %s (old value = %d): ", obj_name, par);
    ask_string (input_str, par_str);
    sscanf (par_str, "%d", &par);

    if (par < 0) {
	ptext ("No negative repetition allowed!");
	return (0);
    }
    if (par > MAX_NX_NY) {
	ptext ("Array parameters too large!");
	return (0);
    }
    if (par == *nx_ny) {
	ptext ("Same as old parameter value, no change!");
	return (0);
    }

    *nx_ny = par;
    if (*dx_dy == (Coor) 0) *dx_dy = dd;
    if (par == 0) *dx_dy = (Coor) 0;
    return (-1);
}

/* argument comm_par: 3 if dx, 4 if dy (see calling routines)
*/
int dx_dy_par (Coor *d_p, int n_p, Coor margin, int comm_par)
{
    int  par;
    Coor xx, yy, sel_val;

    if (n_p == 0) {
	ptext ("There is no repetition in this direction!");
	return (0);
    }
    if ((par = get_one (1, &xx, &yy)) == -1) {
	sel_val = (comm_par == 3) ? xx : yy;
	if (sel_val == margin) {
	    ptext ("Not allowed to make distance equal to 0!");
	    return (0);
	}
	else if (sel_val - margin == *d_p) {
	    ptext ("Same as old parameter value, no change!");
	    return (comm_par);
	}
	else
	    *d_p = sel_val - margin;
    }
    return (par);
}

void arr_par ()
{
    register int lay;
    int  par = 0;
    Coor ll, rr, bb, tt;
    char **text_arr;

    if (!act_inst) {
	ptext ("Set your actual instance first!");
	return;
    }
    inst_window (act_inst, &piwl, &piwr, &piwb, &piwt);

    while (par != -1) {
	par = ask_par (&text_arr);
	pre_cmd_proc (par, text_arr);
	switch (par) {
	case 0:
	    return;
	case 1:
	    par = nx_ny_par (&act_inst -> nx, &act_inst -> dx,
		    act_inst -> bbxr - act_inst -> bbxl, act_inst -> templ -> cell_name);
	    /* if par == 0 -> cancel -> return, no picture */
	    break;
	case 2:
	    par = nx_ny_par (&act_inst -> ny, &act_inst -> dy,
		    act_inst -> bbyt - act_inst -> bbyb, act_inst -> templ -> cell_name);
	    /* if par == 0 -> cancel -> return, no picture */
	    break;
	case 3:
	    par = dx_dy_par (&act_inst -> dx, act_inst -> nx, act_inst -> bbxl, par);
	    post_cmd_proc (3, text_arr);
	    break;
	case 4:
	    par = dx_dy_par (&act_inst -> dy, act_inst -> ny, act_inst -> bbyb, par);
	    post_cmd_proc (4, text_arr);
	    break;
	default:
	    ptext ("Wrong choice!");
	}
    }

    pict_arr[Textnr] = ERAS_DR; /* redraw instances */

    if (vis_arr[NR_lay + 2]) {
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
    }
    else if (vis_arr[NR_lay + 1]) {
	for (lay = 0; lay < NR_lay; ++lay)
	    if (term_bool[lay]) pict_arr[lay] = ERAS_DR;
    }

    inst_window (act_inst, &ll, &rr, &bb, &tt);

    piwl = Min (piwl, ll);
    piwr = Max (piwr, rr);
    piwb = Min (piwb, bb);
    piwt = Max (piwt, tt);
}
