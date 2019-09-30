/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
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

extern DM_PROJECT *dmproject;
extern Coor piwl, piwr, piwb, piwt; /* window to be drawn */
extern Coor xlc, ybc; /* cursor parameters */
extern int *term_lay;
extern int *non_edit;
extern int *pict_arr;
extern int  NR_lay;
extern int  Textnr;
extern int  new_cmd;
extern int  erase_text;
extern int  allow_keypress;

#define	MAX_EXT 10

#define S_LE 1
#define S_RI 2
#define S_BO 3
#define S_TO 4

static char *side_str[] = { /* static for redraw in multi-window */
/* 0 */  "-ready-",
/* 1 */  "left",
/* 2 */  "right",
/* 3 */  "bottom",
/* 4 */  "top"
};

static int  ask_sides (int side[]);
static void do_new_term (int lay, int side_nr, Coor pretrn[], TERM *tp, INST *ip, int newflag, char *ext);
static void do_tlift (INST *ip, int side[], int newflag, char *ext);
static int  name_choice (char *ext, int *newflag);

void lift_terms ()
{
    INST *ip;
    char  ext[DM_MAXNAME + 1];
    char  mess_str[MAXCHAR];
    int   newflag, side[5];

    if (!present_inst ()) return;

    ptext ("Select an instance to lift terminals!");
    erase_text = 1;
    if ((new_cmd = get_cursor (1, 1, NO_SNAP)) != -1) return;

    if (!(ip = search_inst (xlc, ybc))) {
	ptext ("No instance selected!");
	return;
    }

    sprintf (mess_str, "Instance '%s', select side(s) to lift!", ip -> templ -> cell_name);
    ptext (mess_str);

    allow_keypress = 0;
    if (ask_sides (side) && name_choice (ext, &newflag))
	do_tlift (ip, side, newflag, ext);
    else
	ptext ("No terminals lifted!");
    allow_keypress = 1;
}

static int ask_sides (int side[])
{
    register int nbr;
    menu (sizeof (side_str) / sizeof (char *), side_str);
    for (nbr = 1; nbr <= 4; ++nbr) side[nbr] = 0;
    while ((nbr = get_com ())) {
	if (nbr > 4) nbr = 4;
	side[nbr] = !side[nbr]; /* toggle */
	if (side[nbr])
	    pre_cmd_proc (nbr);
	else
	    post_cmd_proc (nbr);
    }
    for (nbr = 1; nbr <= 4; ++nbr) if (side[nbr]) return (1);
    return (0);
}

static int name_choice (char *ext, int *newflag)
{
    static char *ask_str[] = { /* static for redraw */
	"-cancel-",
	"new name",
	"extension",
	"old name"
    };
    int len, nbr;

    menu (sizeof (ask_str) / sizeof (char *), ask_str);

    *newflag = 0;

    while (TRUE) {
	ptext ("Select name assignment for new terminals!");
	nbr = get_com ();
	pre_cmd_proc (nbr);
	switch (nbr) {
	case 0: /* cancel */
	    goto ret;
	case 1:
	    *newflag = 1;
	    return (1);
	case 2:
	    if ((len = ask_string ("name_extension: ", ext))) {
		if (len > MAX_EXT) ext[MAX_EXT] = 0;
		return (1);
	    }
	    break;
	case 3:
	    *ext = 0;
	    return (1);
	}
	post_cmd_proc (nbr);
    }
ret:
    return (0); /* cancel */
}

static int t_cnt;

static void do_tlift (INST *ip, int side[], int newflag, char *ext)
{
    char mess_str[MAXCHAR];
    Coor trans0[4], pretrn[4];
    register int lay, it, jt;
    register TERM *tp;
    int  cnt, ne_skip;

    if (!ip -> templ -> t_flag) {
	if (exp_templ (ip -> templ, dmproject, ip -> imported, READ_TERM)) return; /* read error */
    }

    t_cnt = 0;
    ne_skip = 0;

    for (lay = 0; lay < NR_lay; ++lay) {
	if (!term_lay[lay]) continue;
	tp = ip -> templ -> term_p[lay];
	if (non_edit[lay]) {
	    if (tp) ne_skip = 1;
	    continue;
	}
	cnt = t_cnt;
	for (; tp; tp = tp -> nxttm) {
	    for (it = 0; it <= tp -> ny; ++it)
	    for (jt = 0; jt <= tp -> nx; ++jt) {
		pretrn[0] = tp -> xl + jt * tp -> dx;
		pretrn[1] = tp -> xr + jt * tp -> dx;
		pretrn[2] = tp -> yb + it * tp -> dy;
		pretrn[3] = tp -> yt + it * tp -> dy;
		trans_box (pretrn, trans0, ip, 0, 0);

		if (side[S_LE] && trans0[0] == ip -> bbxl) {
		    do_new_term (lay, S_LE, pretrn, tp, ip, newflag, ext);
		}
		else if (side[S_BO] && trans0[2] == ip -> bbyb) {
		    do_new_term (lay, S_BO, pretrn, tp, ip, newflag, ext);
		}
		else if (side[S_RI] && trans0[1] == ip -> bbxr) {
		    do_new_term (lay, S_RI, pretrn, tp, ip, newflag, ext);
		}
		else if (side[S_TO] && trans0[3] == ip -> bbyt) {
		    do_new_term (lay, S_TO, pretrn, tp, ip, newflag, ext);
		}
	    }
	}
	if (cnt != t_cnt) pict_arr[lay] = DRAW;
    }
    sprintf (mess_str, "Lifted %d terminals!%s", t_cnt, ne_skip ? " Skipped non-edit lays terminals!" : "");
    ptext (mess_str);
    if (t_cnt) pict_arr[Textnr] = DRAW;
}

static void do_new_term (int lay, int side_nr, Coor pretrn[], TERM *tp, INST *ip, int newflag, char *ext)
{
    Coor  trans[4];
    Coor  ll, rr, bb, tt, dx, dy;
    char  str[MAXCHAR];
    char  name[DM_MAXNAME + 1];
    int   nx, ny;
    register int len;
    register char *cp;

    if (newflag) {
	sprintf (str, "terminal_name (oldname = %s): ", tp -> tmname);
	len = ask_name (str, name, TRUE);
    }
    else {
	len = tp -> tmlen;
	strcpy (name, tp -> tmname);
	for (cp = name + len; *ext && len < DM_MAXNAME; ++len) *cp++ = *ext++;
	*cp = 0;
    }
    if (!len) return;

    dx = ip -> dx; nx = ip -> nx;
    dy = ip -> dy; ny = ip -> ny;

    switch (side_nr) {
    case S_LE:
	trans_box (pretrn, trans, ip, (dx >= 0) ? 0 : nx, 0);
	dx = 0; nx = 0;
	break;
    case S_RI:
	trans_box (pretrn, trans, ip, (dx >= 0) ? nx : 0, 0);
	dx = 0; nx = 0;
	break;
    case S_BO:
	trans_box (pretrn, trans, ip, 0, (dy >= 0) ? 0 : ny);
	dy = 0; ny = 0;
	break;
    case S_TO:
	trans_box (pretrn, trans, ip, 0, (dy >= 0) ? ny : 0);
	dy = 0; ny = 0;
	break;
    }

    ll = trans[0]; rr = trans[1]; bb = trans[2]; tt = trans[3];

    if (!new_term (ll, rr, bb, tt, name, len, dx, nx, dy, ny, lay)) return;

    if (dx > (Coor) 0) rr += nx * dx;
    else if (dx < (Coor) 0) ll += nx * dx;

    if (dy > (Coor) 0) tt += ny * dy;
    else if (dy < (Coor) 0) bb += ny * dy;

    if (++t_cnt == 1) {
	piwl = ll;
	piwr = rr;
	piwb = bb;
	piwt = tt;
    }
    else {
	if (ll < piwl) piwl = ll;
	if (rr > piwr) piwr = rr;
	if (bb < piwb) piwb = bb;
	if (tt > piwt) piwt = tt;
    }
}
