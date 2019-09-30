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

extern Coor piwl, piwr, piwb, piwt; /* window to be drawn */
extern Coor xlc, ybc; /* cursor parameters */
extern int *pict_arr;
extern int  Textnr;
extern int  cmd_nbr, new_cmd;
extern int  erase_text;

void mov_term ()
{
    TERM *tp;
    Coor ll, rr, bb, tt;
    char mess_str[MAXCHAR];
    int  lay;

    if (!present_term ()) goto ret;

    erase_text = 1;
    do {
	ptext ("Select a terminal to move!");
	if ((new_cmd = get_cursor (1, 1, NO_SNAP)) != -1) goto ret;
	tp = search_term (xlc, ybc, &lay);
    } while (!tp);

    term_win (tp, &ll, &rr, &bb, &tt, 1);
    set_bbox_loc (ll, rr, bb, tt);

    sprintf (mess_str, "Terminal '%s', enter new position!", tp -> tmname);
    ptext (mess_str);
    if ((new_cmd = get_cursor (6, 1, SNAP)) != -1) goto ret;
    new_cmd = cmd_nbr;

    if (tp -> dx < 0) xlc -= tp -> nx * tp -> dx;
    if (tp -> dy < 0) ybc -= tp -> ny * tp -> dy;
    tp -> xr = xlc + (tp -> xr - tp -> xl);
    tp -> xl = xlc;
    tp -> yt = ybc + (tp -> yt - tp -> yb);
    tp -> yb = ybc;
    term_win (tp, &piwl, &piwr, &piwb, &piwt, 1);
    if (ll == piwl && bb == piwb) { /* same position */
ret:
	piwr = piwl; /* no picture() */
	return;
    }
    pict_arr[Textnr] = ERAS_DR; /* redraw characters */
    pict_arr[lay] = ERAS_DR;
    if (ll < piwl) piwl = ll;
    if (rr > piwr) piwr = rr;
    if (bb < piwb) piwb = bb;
    if (tt > piwt) piwt = tt;
}

void t_arr_par ()
{
    TERM *tp;
    int  lay, par;
    Coor ll, rr, bb, tt;
    char **text_arr;
    char mess_str[MAXCHAR];

    if (!present_term ()) return;

    do {
	ptext ("Select a terminal to be indexed!");
	if ((new_cmd = get_cursor (1, 1, NO_SNAP)) != -1) {
	    erase_text = 1;
	    return;
	}
	tp = search_term (xlc, ybc, &lay);
    } while (!tp);

    sprintf (mess_str, "Terminal '%s', select array parameter to change!", tp -> tmname);
    ptext (mess_str);

    term_win (tp, &ll, &rr, &bb, &tt, 1);

    par = ask_par (&text_arr);
    while (par != -1) {
	pre_cmd_proc (par);
	switch (par) {
	case -2:
	case 0:  /* cancel */
	    goto ret;
	case 1:  /* nx */
	    par = nx_ny_par (&tp -> nx, &tp -> dx, tp -> xr - tp -> xl, tp -> tmname);
	    break;
	case 2:  /* ny */
	    par = nx_ny_par (&tp -> ny, &tp -> dy, tp -> yt - tp -> yb, tp -> tmname);
	    break;
	case 3:  /* dx */
	    par = dx_dy_par (&tp -> dx, tp -> nx, tp -> xl, tp -> yb, par);
	    post_cmd_proc (3);
	    break;
	case 4:  /* dy */
	    par = dx_dy_par (&tp -> dy, tp -> ny, tp -> xl, tp -> yb, par);
	    post_cmd_proc (4);
	    break;
	default:
	    par = -2;
	    ptext ("Wrong choice!");
	}
    }

    pict_arr[Textnr] = ERAS_DR; /* characters */
    pict_arr[lay] = ERAS_DR;
    term_win (tp, &piwl, &piwr, &piwb, &piwt, 1);
    if (ll < piwl) piwl = ll;
    if (rr > piwr) piwr = rr;
    if (bb < piwb) piwb = bb;
    if (tt > piwt) piwt = tt;
ret:
    new_cmd = cmd_nbr;
    erase_text = 1;
}
