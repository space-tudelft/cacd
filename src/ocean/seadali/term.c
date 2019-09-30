/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	Pieter van der Wolf
 *	Patrick Groeneveld
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

extern char **lay_names;
extern INST *inst_root;
extern TERM **term_root;
extern TERM *act_term;
extern int   act_t_lay;
extern int   NR_lay;
extern int  *term_bool;
extern int  *def_arr;
extern int  *edit_arr;
extern int  *pict_arr;
extern int   Textnr;
extern int   ImageMode;
extern int   Cur_nr;
extern int   nr_planes;
extern int   sub_term_flag;
extern Coor  piwl, piwr, piwb, piwt;

static TERM *find_term (char *name_p);
static void t_unlink (TERM *freepntr, int lay);

void all_term ()
{
    register int lay;
    int   repaint;
    INST *inst_p;

    repaint = FALSE;
    sub_term_flag = TRUE;

    for (inst_p = inst_root; inst_p; inst_p = inst_p -> next) {
	inst_p -> t_draw = TRUE;
	repaint = TRUE;
    }

    if (repaint == TRUE) {
	pict_arr[Textnr] = DRAW;
	for (lay = 0; lay < NR_lay; ++lay) {
	    if (term_bool[lay]) pict_arr[lay] = DRAW;
	}
    }
}

void add_term (Coor x1, Coor x2, Coor y1, Coor y2)
{
    TERM *tpntr;
    char  namestr[DM_MAXNAME + 1];
    char  hulp_str[MAXCHAR];
    register int lay;
    int   lflag = 0;

    if (ImageMode == TRUE) snap_box_to_grid (&x1, &x2, &y1, &y2);

    if (x1 == x2 || y1 == y2) return;

    ggSetColor (Cur_nr);
    set_c_wdw (PICT);

    if (nr_planes == 8) pic_cur (x1, x2, y1, y2);
    else {
	disp_mode (COMPLEMENT);
	pict_rect ((float) x1, (float) x2, (float) y1, (float) y2);
	flush_pict ();
	disp_mode (TRANSPARENT);
    }

    for (lay = 0; lay < NR_lay; ++lay) {
	if (def_arr[lay] && term_bool[lay]) {
	    ++lflag;
	    sprintf (hulp_str, "terminal_name (layer = %s): ", lay_names[lay]);
	    if (ask_name (hulp_str, namestr, TRUE) == -1) continue;

	    if ((tpntr = new_term (x1, x2, y1, y2, namestr, (Coor) 0, 0, (Coor) 0, 0, lay))) {
		act_term = tpntr;
		act_t_lay = lay;
		pict_arr[lay] = DRAW;
		pict_arr[Textnr] = DRAW;
	    }
	}
    }
    if (!lflag) ptext ("No terminal layer set!");
    piwl = x1;
    piwr = x2;
    piwb = y1;
    piwt = y2;

    if (nr_planes == 8) clear_curs ();
    else {
	disp_mode (COMPLEMENT);
	pict_rect ((float) x1, (float) x2, (float) y1, (float) y2);
	flush_pict ();
	disp_mode (TRANSPARENT);
    }
}

TERM * new_term (Coor xl, Coor xr, Coor yb, Coor yt, char *name, Coor dx, int nx, Coor dy, int ny, int lay)
{
    char  new_name[DM_MAXNAME + 1];
    char  helpname[DM_MAXNAME + 1];
    char  meas[MAXCHAR];
    char  indexname[100];

    if (xl >= xr || yb >= yt) return (NULL);

    strcpy (new_name, name);
    while (find_term (new_name)) {
	sprintf (meas, "terminal_name '%s' used; new name: ", new_name);
	if (ask_name (meas, new_name, TRUE) == -1) return (NULL); /* empty string: cancel */
    }
    return (create_term (&(term_root[lay]), xl, xr, yb, yt, new_name, dx, nx, dy, ny));
}

/*
 * PATRICK: changed that shitty indexed terminals, only causes troubles!
 * we expand the terminals...
 *
 * nx, ny : instance index
 * it, jt : terminal index
 */
void lnew_term (Coor xl, Coor xr, Coor yb, Coor yt, char *name, Coor dx, int nx, Coor dy, int ny, int lay, int it, int jt)
{
    char  new_name[100];
    char  helpname[DM_MAXNAME + 1];
    char  meas[MAXCHAR];
    char  indexname[100];
    int nnx, nny;

    if (xl >= xr || yb >= yt) return;

    if (it == 0 && jt == 0) strcpy (new_name, name);
    else {
	sprintf (indexname, "_%d_%d", it, jt);
	/* printf ("Terminal %s : %s\n", name, indexname); */
	if (strlen (name) + strlen (indexname) > DM_MAXNAME) { /* must truncate... */
	    strncpy (helpname, name, DM_MAXNAME - strlen (indexname));
	    helpname[DM_MAXNAME - strlen (indexname)] = '\0';
	    sprintf (new_name, "%s%s", helpname, indexname);
	}
	else {
	    sprintf (new_name, "%s%s", name, indexname);
	}
    }

    while (find_term (new_name)) {
	sprintf (meas, "terminal_name '%s' used; new name: ", new_name);
	if (ask_name (meas, new_name, TRUE) == -1) return; /* empty string: cancel */
    }

   /* PATRICK: changed that shitty indexed terminals, only causes troubles!
    * we expand the terminals...
    */
    if (nx != 0 || ny != 0) { /* nasty index exists!! */
	for (nnx = 0; nnx <= nx; nnx++)
	for (nny = 0; nny <= ny; nny++) {
	    sprintf (indexname, "_%d_%d", nnx, nny);
	    /* printf ("Terminal %s : %s\n", name, indexname); */
	    if (strlen (new_name) + strlen (indexname) > DM_MAXNAME) { /* must truncate... */
		strncpy (helpname, new_name, DM_MAXNAME - strlen (indexname));
		helpname[DM_MAXNAME - strlen (indexname)] = '\0';
		sprintf (new_name, "%s%s", helpname, indexname);
	    }
	    else {
		strcpy (helpname, new_name);
		sprintf (new_name, "%s%s", helpname, indexname);
	    }
	    create_term (&(term_root[lay]),
		xl + (nnx * dx), xr + (nnx * dx),
		yb + (nny * dy), yt + (nny * dy), new_name, 0, 0, 0, 0);
	}
    }
    else {
	create_term (&(term_root[lay]), xl, xr, yb, yt, new_name, dx, nx, dy, ny);
    }
}

TERM * create_term (TERM **terml_pp, Coor xl, Coor xr, Coor yb, Coor yt, char *name, Coor dx, int nx, Coor dy, int ny)
{
    TERM *tpntr;

    MALLOC (tpntr, TERM);
    if (tpntr) {
	tpntr -> xl = xl;
	tpntr -> xr = xr;
	tpntr -> yb = yb;
	tpntr -> yt = yt;
	tpntr -> dx = dx;
	tpntr -> nx = nx;
	tpntr -> dy = dy;
	tpntr -> ny = ny;
	tpntr -> tmname = strsave (name);
	tpntr -> nxttm = *terml_pp;
	*terml_pp = tpntr;
    }
    return (tpntr);
}

void term_win (TERM *termp, Coor *ll, Coor *rr, Coor *bb, Coor *tt)
{
    *ll = (termp -> dx >= 0) ? termp -> xl : termp -> xl + termp -> nx * termp -> dx;
    *rr = (termp -> dx <= 0) ? termp -> xr : termp -> xr + termp -> nx * termp -> dx;
    *bb = (termp -> dy >= 0) ? termp -> yb : termp -> yb + termp -> ny * termp -> dy;
    *tt = (termp -> dy <= 0) ? termp -> yt : termp -> yt + termp -> ny * termp -> dy;
}

void del_term (Coor x_cur, Coor y_cur)
{
    int lay;
    TERM *freepntr;

    if (!(freepntr = search_term (x_cur, y_cur, &lay, TRUE))) return;
    term_win (freepntr, &piwl, &piwr, &piwb, &piwt);
    t_unlink (freepntr, lay);
    pict_arr[Textnr] = ERAS_DR;
    pict_arr[lay] = ERAS_DR;
}

void del_t_area (Coor x1, Coor x2, Coor y1, Coor y2)
{
    register int lay;
    Coor ll, rr, bb, tt;
    register TERM *tp;
    register TERM *tp_next;

    for (lay = 0; lay < NR_lay; ++lay) {
	if (!edit_arr[lay]) continue;
	for (tp = term_root[lay]; tp; tp = tp_next) {
            tp_next = tp -> nxttm;
	    term_win (tp, &ll, &rr, &bb, &tt);
	    if (ll >= x1 && rr <= x2 && bb >= y1 && tt <= y2) {
		t_unlink (tp, lay);
		pict_arr[lay] = ERAS_DR;
		pict_arr[Textnr] = ERAS_DR;
	    }
	}
    }
    piwl = x1;
    piwr = x2;
    piwb = y1;
    piwt = y2;
}

static TERM * prev_term (TERM *term_p, int lay)
{
    register TERM *tpntr;
    for (tpntr = term_root[lay]; tpntr -> nxttm != term_p; tpntr = tpntr -> nxttm) ;
    return (tpntr);
}

static void t_unlink (TERM *freepntr, int lay)
{
    TERM *prev;

    if (freepntr == term_root[lay])
	term_root[lay] = term_root[lay] -> nxttm;
    else {
	prev = prev_term (freepntr, lay);
	prev -> nxttm = freepntr -> nxttm;
    }
    if (freepntr == act_term) act_term = NULL;
    FREE (freepntr -> tmname);
    FREE (freepntr);
}

static TERM * find_term (char *name_p)
{
    register int lay;
    register TERM *tpntr;

    for (lay = 0; lay < NR_lay; ++lay) {
	for (tpntr = term_root[lay]; tpntr; tpntr = tpntr -> nxttm) {
	    if (!strcmp (name_p, tpntr -> tmname)) return (tpntr);
	}
    }
    return (NULL);
}

static int in_term (Coor x, Coor y, TERM *tp)
{
    register int it, jt;

    for (it = 0; it <= tp -> ny; ++it)
    for (jt = 0; jt <= tp -> nx; ++jt) {
	if (x >= tp -> xl + jt * tp -> dx && x <= tp -> xr + jt * tp -> dx &&
	    y >= tp -> yb + it * tp -> dy && y <= tp -> yt + it * tp -> dy) return (TRUE);
    }
    return (FALSE);
}

TERM * search_term (Coor x_c, Coor y_c, int *p_lay, int edit_only)
{
    static char *ask_str[LIST_LENGTH]; /* static for redraw */
    register TERM *tp;
    TERM *st_pntr[LIST_LENGTH];
    int    lay_l[LIST_LENGTH];
    register int count, lay;
    char   mess_str[MAXCHAR];

    count = 0;
    for (lay = 0; lay < NR_lay; ++lay) {
	if (edit_only && !edit_arr[lay]) continue;
	for (tp = term_root[lay]; tp; tp = tp -> nxttm) {
	    if (in_term (x_c, y_c, tp) && count < LIST_LENGTH) {
		lay_l[count] = lay;
		st_pntr[count] = tp;
		ask_str[count] = tp -> tmname;
		++count;
	    }
	}
    }

    if (count == 0)
	return ((TERM *) NULL);
    else if (count == 1)
	lay = 0;
    else { /* select by menu */
	lay = ask (count, ask_str, -1);
	if (lay < 0) return ((TERM *) NULL);
    }

    *p_lay = lay_l[lay];
    tp = st_pntr[lay];
    sprintf (mess_str, "the selected terminal is: %s", tp -> tmname);
    ptext (mess_str);
    return (tp);
}
