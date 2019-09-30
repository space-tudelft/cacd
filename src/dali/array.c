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

#define MAX_NX_NY   10000

extern int  *term_lay;
extern int  *pict_arr;
extern int  *vis_arr;
extern int   v_sterm, v_inst;
extern int   NR_lay;
extern int   Textnr;
extern int   cmd_nbr, new_cmd;
extern int   erase_text;
extern Coor  xlc, ybc;
extern Coor piwl, piwr, piwb, piwt; /* window to be drawn */

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
    char str[MAXCHAR];
    int  par;

    par = *nx_ny;
    sprintf (str, "number of repetitions of '%s' (old value = %d): ",
	obj_name, par);

    if (!ask_string (str, par_str)) return (0);
    sscanf (par_str, "%d", &par);

    if (par < 0) {
	ptext ("No negative repetition allowed!");
	return (-2);
    }
    if (par > MAX_NX_NY) {
	ptext ("Array parameters too large!");
	return (-2);
    }
    if (par == *nx_ny) {
	ptext ("Same as old parameter value, no change!");
	return (-2);
    }
    *nx_ny = par;
    if (*dx_dy == (Coor) 0) *dx_dy = dd;
    if (par == 0) *dx_dy = (Coor) 0;
    return (-1);
}

int dx_dy_par (Coor *d_p, int n_p, Coor bbxl, Coor bbyb, int comm_par)
/* comm_par: 3 if dx, 4 if dy: see calling routines */
{
    int  par;
    Coor margin, sel_val;

    if (n_p == 0) {
	ptext ("There is no repetition in this direction!");
	sleep (2);
	return (comm_par - 2);
    }

    margin = (comm_par == 3) ? bbxl : bbyb;
    if (comm_par == 3)
	ptext ("Enter dx value with cursor!");
    else
	ptext ("Enter dy value with cursor!");
    if ((par = get_one (comm_par - 1, &bbxl, &bbyb)) == -1) {
	sel_val = (comm_par == 3) ? bbxl : bbyb;
	if (sel_val == margin) {
	    ptext ("Not allowed to make distance equal to 0!");
	    return (-2);
	}
	else if (sel_val - margin == *d_p) {
	    ptext ("Same as old parameter value, no change!");
	    sleep (2);
	    return (comm_par);
	}
	else
	    *d_p = sel_val - margin;
    }
    return (par);
}

void arr_par ()
{
    INST *ip;
    register int lay;
    int  par;
    Coor ll, rr, bb, tt;
    char **text_arr;
    char mess_str[MAXCHAR];

    if (!present_inst ()) return;

    do {
	ptext ("Select an instance to be indexed!");
	if ((new_cmd = get_cursor (1, 1, NO_SNAP)) != -1) {
	    erase_text = 1;
	    return;
	}
	ip = search_inst (xlc, ybc);
    } while (!ip);

    sprintf (mess_str, "Instance %s(%s), select array parameter to change!",
	ip -> inst_name, ip -> templ -> cell_name);
    ptext (mess_str);

    inst_window (ip, &ll, &rr, &bb, &tt);

    par = ask_par (&text_arr);
    while (par != -1) {
	pre_cmd_proc (par);
	switch (par) {
	case -2:
	case 0:  /* cancel */
	    goto ret;
	case 1:  /* nx */
	    par = nx_ny_par (&ip -> nx, &ip -> dx, ip -> bbxr - ip -> bbxl, ip -> templ -> cell_name);
	    break;
	case 2:  /* ny */
	    par = nx_ny_par (&ip -> ny, &ip -> dy, ip -> bbyt - ip -> bbyb, ip -> templ -> cell_name);
	    break;
	case 3:  /* dx */
	    par = dx_dy_par (&ip -> dx, ip -> nx, ip -> bbxl, ip -> bbyb, par);
	    post_cmd_proc (3);
	    break;
	case 4:  /* dy */
	    par = dx_dy_par (&ip -> dy, ip -> ny, ip -> bbxl, ip -> bbyb, par);
	    post_cmd_proc (4);
	    break;
	default:
	    par = -2;
	    ptext ("Wrong choice!");
	}
    }

    pict_arr[Textnr] = ERAS_DR; /* redraw instances */
    if (ip -> level > 1 && vis_arr[v_inst]) { /* inst. visible */
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
    }
    else if (vis_arr[v_sterm]) { /* sub-terminals visible */
	for (lay = 0; lay < NR_lay; ++lay)
	    if (term_lay[lay]) pict_arr[lay] = ERAS_DR;
    }
    inst_window (ip, &piwl, &piwr, &piwb, &piwt);
    if (ll < piwl) piwl = ll;
    if (rr > piwr) piwr = rr;
    if (bb < piwb) piwb = bb;
    if (tt > piwt) piwt = tt;
ret:
    erase_text = 1;
    new_cmd = cmd_nbr;
}
