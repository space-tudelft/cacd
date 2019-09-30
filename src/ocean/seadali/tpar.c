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

extern TERM *act_term;
extern Coor piwl, piwr, piwb, piwt; /* window to be drawn */
extern int  act_t_lay;
extern int *edit_arr;
extern int *pict_arr;
extern int  Textnr;

void mov_term (Coor x_cur, Coor y_cur)
{
    Coor ddx, ddy;
    Coor ll, rr, bb, tt;

    if (!act_term) {
	ptext ("Set your actual terminal first!");
	return;
    }
    if (!edit_arr[act_t_lay]) {
	ptext ("Actual terminal is non-editable");
	return;
    }
    term_win (act_term, &piwl, &piwr, &piwb, &piwt);
    ddx = act_term -> xr - act_term -> xl;
    ddy = act_term -> yt - act_term -> yb;
    act_term -> xl = x_cur;
    act_term -> xr = x_cur + ddx;
    act_term -> yb = y_cur;
    act_term -> yt = y_cur + ddy;

    pict_arr[Textnr] = ERAS_DR; /* redraw characters */
    pict_arr[act_t_lay] = ERAS_DR;
    term_win (act_term, &ll, &rr, &bb, &tt);
    piwl = Min (piwl, ll);
    piwr = Max (piwr, rr);
    piwb = Min (piwb, bb);
    piwt = Max (piwt, tt);
}

void set_act_term (Coor x_c, Coor y_c)
{
    act_term = search_term (x_c, y_c, &act_t_lay, FALSE);
}

void t_arr_par ()
{
    int  par = 0;
    Coor ll, rr, bb, tt;
    char **text_arr;

    if (!act_term) {
	ptext ("Set your actual terminal first!");
	return;
    }
    if (!edit_arr[act_t_lay]) {
	ptext ("Actual terminal is non-editable");
	return;
    }

    term_win (act_term, &piwl, &piwr, &piwb, &piwt);

    while (par != -1) {
	par = ask_par (&text_arr);
	pre_cmd_proc (par, text_arr);
	switch (par) {
	case 0:
	    return;
	case 1:
	    par = nx_ny_par (&act_term -> nx, &act_term -> dx,
		    act_term -> xr - act_term -> xl, act_term -> tmname);
	    /* par == 0 -> cancel -> return, no picture */
	    break;
	case 2:
	    par = nx_ny_par (&act_term -> ny, &act_term -> dy,
		    act_term -> yt - act_term -> yb, act_term -> tmname);
	    /* par == 0 -> cancel -> return, no picture */
	    break;
	case 3:
	    par = dx_dy_par (&act_term -> dx, act_term -> nx, act_term -> xl, par);
	    post_cmd_proc (3, text_arr);
	    break;
	case 4:
	    par = dx_dy_par (&act_term -> dy, act_term -> ny, act_term -> yb, par);
	    post_cmd_proc (4, text_arr);
	    break;
	}
    }
    pict_arr[Textnr] = ERAS_DR; /* characters */
    pict_arr[act_t_lay] = ERAS_DR;

    term_win (act_term, &ll, &rr, &bb, &tt);
    piwl = Min (piwl, ll);
    piwr = Max (piwr, rr);
    piwb = Min (piwb, bb);
    piwt = Max (piwt, tt);
}
