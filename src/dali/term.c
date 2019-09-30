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

extern TERM **term_root;
extern TERMREF *termlist, *termlistlast;
extern char **lay_names;
extern int   NR_lay;
extern int  *term_lay;
extern int  *def_arr;
extern int  *non_edit;
extern int  *pict_arr;
extern int  *vis_arr;
extern int   allow_keypress;
extern int   v_term;
extern int   ADD_mode;
extern int   Cur_nr;
extern int   Textnr;
extern int   new_cmd, cmd_nbr;
extern int   erase_text;
extern Coor  piwl, piwr, piwb, piwt;
extern Coor  xlc, xrc, ybc, ytc; /* cursor parameters */

static int  in_term (Coor x, Coor y, TERM *tp);

void add_term ()
{
    char  name[DM_MAXNAME + 1];
    char  str[MAXCHAR];
    register int lay;
    int   len, lflag;
    float x1, x2, y1, y2;

    if (!vis_arr[v_term]) {
	btext ("Terminals not visible!");
	return;
    }

    for (lay = 0; lay < NR_lay; ++lay) {
	if (!term_lay[lay] && def_arr[lay]) toggle_lay (lay);
    }
    ADD_mode = 1;

again:
    ptext ("Enter position to add terminal!");
    do {
	if ((new_cmd = get_cursor (5, 2, SNAP)) != -1) {
	    erase_text = 1;
	    goto ret;
	}
	if (xlc == xrc && ybc == ytc) break; /* point terminal */
    } while (xlc >= xrc || ybc >= ytc);

    x1 = (float) xlc;
    x2 = (float) xrc;
    y1 = (float) ybc;
    y2 = (float) ytc;
    if (xlc == xrc) { ++x2; ++y2; } /* point terminal */

    /* new_cmd = cmd_nbr; */
    set_c_wdw (PICT);

    lflag = 0;
    for (lay = 0; lay < NR_lay; ++lay) {
	if (def_arr[lay] && term_lay[lay]) {

	    disp_mode (COMPLEMENT);
	    ggSetColor (Cur_nr);
	    pict_rect (x1, x2, y1, y2);
	    flush_pict ();

	    ++lflag;
	    sprintf (str, "terminal_name (layer = %s): ", lay_names[lay]);
	    if (!(len = ask_name (str, name, TRUE))) continue;

	    pict_rect (x1, x2, y1, y2);
	    flush_pict ();
	    disp_mode (TRANSPARENT);

	    if (new_term (xlc, xrc, ybc, ytc, name, len, 0, 0, 0, 0, lay)) {
		pict_arr[lay] = DRAW;
		pict_arr[Textnr] = DRAW;
		piwl = xlc; piwr = xrc;
		piwb = ybc; piwt = ytc;
		if (xlc == xrc) { ++piwr; ++piwt; } /* point terminal */
		picture ();
	    }
	}
    }

    if (!lflag) {
	btext ("No terminal layer set!");
	sleep (1);
    }
    goto again;

ret:
    ADD_mode = 0;
}

TERM * new_term (Coor xl, Coor xr, Coor yb, Coor yt, char *name, int len, Coor dx, int nx, Coor dy, int ny, int lay)
{
    char new_name[DM_MAXNAME + 1];
    char meas[MAXCHAR];
    register TERM *tp;
    register TERMREF *t;

    /* find_term */
    for (t = termlist; t; t = t -> next) {
	if ((tp = t -> tp) && len == tp -> tmlen && !strcmp (name, tp -> tmname)) {
	    sprintf (meas, "terminal_name '%s' used; new name: ", name);
	    name = new_name;
	    if (!(len = ask_name (meas, name, TRUE))) return (NULL);
	}
    }
    tp = create_term (&term_root[lay], xl, xr, yb, yt, name, len, dx, nx, dy, ny);
    if (tp) {
	MALLOC (t, TERMREF);
	t -> tp = tp;
	t -> lay = lay;
	t -> next = NULL;
	if (!termlist) termlist = t;
	else termlistlast -> next = t;
	termlistlast = t;
    }
    return tp;
}

TERM * create_term (TERM **terml_pp, Coor xl, Coor xr, Coor yb, Coor yt, char *name, int len, Coor dx, int nx, Coor dy, int ny)
{
    register TERM *tp;

    MALLOC (tp, TERM);
    if (tp) {
	tp -> xl = xl;
	tp -> xr = xr;
	tp -> yb = yb;
	tp -> yt = yt;
	tp -> dx = dx;
	tp -> nx = nx;
	tp -> dy = dy;
	tp -> ny = ny;
	tp -> tmname = strsave (name, len);
	tp -> tmlen = len;
	tp -> nxttm = *terml_pp;
	*terml_pp = tp;
    }
    return (tp);
}

void del_term ()
{
    char mess_str[MAXCHAR];
    int lay;
    register TERM *prev, *tp;
    register TERMREF *t, *pt;

    if (!present_term ()) return;

    do {
	ptext ("Select a terminal to delete!");
	if ((new_cmd = get_cursor (1, 1, NO_SNAP)) != -1) {
	    erase_text = 1;
	    return;
	}
	tp = search_term (xlc, ybc, &lay);
    } while (!tp);

    new_cmd = cmd_nbr;

    term_win (tp, &piwl, &piwr, &piwb, &piwt, 1);

    if (tp == term_root[lay])
	term_root[lay] = tp -> nxttm;
    else {
	for (prev = term_root[lay]; prev -> nxttm != tp; prev = prev -> nxttm);
	prev -> nxttm = tp -> nxttm;
    }
    sprintf (mess_str, "Terminal '%s' deleted!", tp -> tmname);
    ptext (mess_str);
    sleep (1);
    pt = 0;
    for (t = termlist; t && t -> tp != tp; t = t -> next) pt = t;
    if (t) {
	if (pt) pt -> next = t -> next;
	else termlist = t -> next;
	if (t == termlistlast) termlistlast = pt;
	FREE (t);
    }
    FREE (tp -> tmname);
    FREE (tp);
    pict_arr[Textnr] = ERAS_DR;
    pict_arr[lay] = ERAS_DR;
}

TERM * search_term (Coor x_c, Coor y_c, int *layP)
{
    static char *ask_str[LIST_LENGTH]; /* static for redraw */
    TERM        *st_pntr[LIST_LENGTH];
    int          lay_lst[LIST_LENGTH];
    char mess_str[MAXCHAR];
    register TERM *tp;
    register int found, lay;

    found = 0;
    for (lay = 0; lay < NR_lay; ++lay) {
	for (tp = term_root[lay]; tp; tp = tp -> nxttm) {
	    if (in_term (x_c, y_c, tp)) {
		if (non_edit[lay]) {
		    sprintf (mess_str,
			"Terminal '%s' non-editable, skipped!", tp -> tmname);
		    notify (mess_str);
		    continue;
		}
		if (found < LIST_LENGTH) {
		    lay_lst[found] = lay;
		    st_pntr[found] = tp;
		    ask_str[found] = tp -> tmname;
		}
		++found;
	    }
	}
    }

    if (found == 0)
	return ((TERM *) 0);
    else if (found == 1)
	lay = 0;
    else { /* select by menu */
	int a_old;
	if (found > LIST_LENGTH) {
	    sprintf (mess_str,
		"List full, %d terminals skipped!", found - LIST_LENGTH);
	    notify (mess_str);
	    found = LIST_LENGTH;
	}
	ptext ("Select one of the found terminals!");
	a_old = allow_keypress;
	allow_keypress = 0;
	lay = ask (found, ask_str, -1);
	allow_keypress = a_old;
    }

    *layP = lay_lst[lay];
    return (st_pntr[lay]);
}

int present_term ()
{
    if (termlist) {
	if (!vis_arr[v_term]) {
	    ptext ("Terminals not visible!");
	    return (0);
	}
	return (1);
    }
    ptext ("No terminals present!");
    return (0);
}

static int in_term (Coor x, Coor y, TERM *tp)
{
    register int it, jt;

    for (it = 0; it <= tp -> ny; ++it) {
	for (jt = 0; jt <= tp -> nx; ++jt) {
	    if (x >= tp -> xl + jt * tp -> dx &&
		x <= tp -> xr + jt * tp -> dx &&
		y >= tp -> yb + it * tp -> dy &&
		y <= tp -> yt + it * tp -> dy) return (1); /* YES */
	}
    }
    return (0); /* NO */
}

void term_win (TERM *tp, Coor *ll, Coor *rr, Coor *bb, Coor *tt, int point_mode)
{
    if (tp -> dx >= (Coor) 0) {
	*ll = tp -> xl;
	*rr = tp -> xr + tp -> nx * tp -> dx;
    }
    else {
	*ll = tp -> xl + tp -> nx * tp -> dx;
	*rr = tp -> xr;
    }
    if (tp -> dy >= (Coor) 0) {
	*bb = tp -> yb;
	*tt = tp -> yt + tp -> ny * tp -> dy;
    }
    else {
	*bb = tp -> yb + tp -> ny * tp -> dy;
	*tt = tp -> yt;
    }
    if (point_mode) {
	if (tp -> xr == tp -> xl) ++(*rr);
	if (tp -> yt == tp -> yb) ++(*tt);
    }
}
