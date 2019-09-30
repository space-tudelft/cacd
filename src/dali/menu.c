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

#define MAX_MENU  30	/* maximum size of a menu */

extern struct Disp_wdw *c_wdw;
extern float XL, XR, YB, YT;
extern float c_cW, c_cH;
extern int  Backgr;
extern int  Gridnr;
extern int  Yellow;
extern int  ask_again;
extern char *yes_no[];

int    ask_yes_no = 0;
int    Curr_nr_alt = 0;	    /* init to zero */
static char *Curr_alt_arr[MAX_MENU];
static int   Curr_lamps[MAX_MENU]; /* init to zero */
static int   Rflag = 0;

static char *Prev_alt_arr[MAX_MENU];
static int   Prev_lamps[MAX_MENU]; /* init to zero */
static int   Prev_nr_alt = 0;      /* init to zero */

void Rmenu () /* redraw current menu */
{
    register int nbr;

    Rflag = 1;
    menu (Curr_nr_alt, Curr_alt_arr);
    for (nbr = 0; nbr < Curr_nr_alt; ++nbr) {
	if (Curr_lamps[nbr]) turn_lamp (nbr, 1);
    }
    Rflag = 0;
}

void prev_menu () /* restore previous (saved) menu */
{
    register int nbr;
    menu (Prev_nr_alt, Prev_alt_arr);
    for (nbr = 0; nbr < Prev_nr_alt; ++nbr) {
	if (Prev_lamps[nbr]) turn_lamp (nbr, 1);
    }
}

void save_menu () /* save current menu */
{
    register int nbr;
    for (nbr = 0; nbr < Curr_nr_alt; ++nbr) {
	Prev_lamps[nbr] = Curr_lamps[nbr];
	Prev_alt_arr[nbr] = Curr_alt_arr[nbr];
    }
    Prev_nr_alt = Curr_nr_alt;
}

void menu (int nr_alt, char *alt_arr[])
{
    float xc, yb;
    register int nbr, strLen;

    if (nr_alt <= 0) return;

    if (Rflag) goto draw_menu; /* A forced redraw of old menu */

    if (nr_alt > MAX_MENU) {
	ptext ("warning: Too many commands! (number reduced)");
	nr_alt = MAX_MENU;
    }

    /*
    ** Check whether this menu is already on screen.
    */
    if (nr_alt == Curr_nr_alt) {
	for (nbr = 0; nbr < nr_alt; ++nbr)
	    if (strcmp (alt_arr[nbr], Curr_alt_arr[nbr])) break; /* diff */
	if (nbr == nr_alt) return; /* all strings equal */
    }

    /* init new menu */
    for (nbr = 0; nbr < nr_alt; ++nbr) {
	Curr_lamps[nbr] = 0;
	Curr_alt_arr[nbr] = alt_arr[nbr];
    }
    Curr_nr_alt = nr_alt;

draw_menu:
    def_world_win (MENU, 0.0, 1.0, 0.0, (float) nr_alt);
    ggClearWindow ();

    ggSetColor (Gridnr);
    yb = YB;
    for (nbr = 0; nbr < nr_alt; ++nbr) {
	if (nbr) d_line (XL, yb, XR, yb);
	strLen = strlen (alt_arr[nbr]);
	if ((xc = XL + 0.5 - 0.5 * c_cW * strLen) < XL) xc = XL;
	d_text (xc, yb + 0.5 - 0.5 * c_cH, alt_arr[nbr], strLen);
	++yb;
    }
}

int ask (int nr_alt, char *alt_arr[], int old)
{
    int nbr;
    menu (nr_alt, alt_arr);
    turn_lamp (old, 1);
    if (alt_arr == yes_no) ask_yes_no = 1;
    else if (old > 0 && ask_again == 2) ask_yes_no = old;
    if ((nbr = get_com ()) != old) {
	turn_lamp (old, 0);
	turn_lamp (nbr, 1);
    }
    ask_yes_no = 0;
    return (nbr);
}

void turn_lamp (int nbr, int enable)
{
    float xc, yb;
    struct Disp_wdw *old_wdw = c_wdw;
    int    old_col = ggGetColor ();
    int    old_mode = get_disp_mode ();
    int strLen;

    if (nbr < 0 || nbr >= Curr_nr_alt) return;
    if (enable) {
	if (Curr_lamps[nbr] && !Rflag) return; /* already ON */
	disp_mode (TRANSPARENT);
	set_c_wdw (MENU);
	ggSetColor (Yellow);
	yb = YB + nbr;
	paint_box (XL + 0.05, XL + 0.10, yb + 0.1, yb + 0.9);
	paint_box (XR - 0.10, XR - 0.05, yb + 0.1, yb + 0.9);
	paint_box (XL + 0.05, XR - 0.05, yb + 0.1, yb + 0.2);
	paint_box (XL + 0.05, XR - 0.05, yb + 0.8, yb + 0.9);
    }
    else {
	if (!Curr_lamps[nbr]) return; /* already OFF */
	disp_mode (ERASE);
	set_c_wdw (MENU);
	yb = YB + nbr;
	paint_box (XL, XR, yb + 0.1, yb + 0.9);
	disp_mode (TRANSPARENT);
	ggSetColor (Gridnr);
	strLen = strlen (Curr_alt_arr[nbr]);
	if ((xc = XL + 0.5 - 0.5 * c_cW * strLen) < XL) xc = XL;
	d_text (xc, yb + 0.5 - 0.5 * c_cH, Curr_alt_arr[nbr], strLen);
    }
    Curr_lamps[nbr] = enable;
    if (old_wdw) set_c_wdw (old_wdw -> w_nr);
    ggSetColor (old_col);
    disp_mode (old_mode);
    flush_pict ();
}

void pre_cmd_proc (int nbr)
{
    turn_lamp (nbr, 1); /* Turn lamp of menu item ON */
}

void post_cmd_proc (int nbr)
{
    turn_lamp (nbr, 0); /* Turn lamp of menu item OFF */
}
