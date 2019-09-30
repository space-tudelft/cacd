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

#define MAX_MENU  30	/* maximum size of a menu */

extern struct Disp_wdw *c_wdw;
extern int  Backgr;
extern int  Gridnr;
extern int  Yellow;
extern int  RGBids[3];
extern int  ask_mode;
extern char *no_yes[];
extern char *yes_no[];

static int    Curr_nr_alt;	    /* init to zero */
static short  Curr_lamps[MAX_MENU]; /* init to zero */
static char  *Curr_alt_arr[MAX_MENU];

static void printtext (float ddy, char *tstr, float ch_w);

void Rmenu () /* redraw current menu */
{
    register int i;

    menu (Curr_nr_alt, Curr_alt_arr);

    for (i = 0; i < Curr_nr_alt; ++i)
	if (Curr_lamps[i]) pre_cmd_proc (i, Curr_alt_arr);
}

void menu (int nr_alt, char *alt_arr[])
{
    float ddx, ddy, ch_w, ch_h;
    register int i;

    if (nr_alt <= 0) return;

    if (Curr_alt_arr == alt_arr) {
	/* A forced redraw of old menu. */
	goto draw_menu;
    }

    if (nr_alt > MAX_MENU) {
	ptext ("warning: Too many commands! (number reduced)");
	nr_alt = MAX_MENU;
    }

    /*
    ** Check whether this menu is already on screen.
    */
    if (nr_alt == Curr_nr_alt) {
	for (i = 0; i < nr_alt; ++i)
	    if (strcmp (alt_arr[i], Curr_alt_arr[i])) break;
	if (i == nr_alt) return;
    }

    for (i = 0; i < nr_alt; ++i) {
	Curr_lamps[i] = 0;
	Curr_alt_arr[i] = alt_arr[i];
    }
    Curr_nr_alt = nr_alt;

draw_menu:
    def_world_win (MENU, 0.0, 1.0, 0.0, (float) nr_alt);
    set_c_wdw (MENU);
    ggClearWindow ();

    ggSetColor (Gridnr);
    ch_siz (&ch_w, &ch_h);
    for (i = 0; i < nr_alt; ++i) {
	ddy = YB + i;
	if (i) d_line (XL, ddy, XR, ddy);
	printtext (ddy - 0.5 * ch_h, alt_arr[i], ch_w);
    }
}

static void printtext (float ddy, char *tstr, float ch_w)
{
    float ddx;
    char hstr[20];
    register int i, j, k;
    int c, len, color, ncolor;
    int first = 1;

    ncolor = color = Gridnr;

again:
    k = -1;
    len = 0;
    /* correct for non-printable control character */
    for (i = j = 0; tstr[j] >= ' '; j++) {
	if (tstr[j] != '&') {
	    ++len;
	    if (tstr[j] == '$') { /* color code */
		++j;
		if (isdigit (tstr[j])) { /* color code */
		    --len;
		    if (k < 0) k = j-1;
		}
		else {
		    if (tstr[j] < ' ') break;
		    if (k < 0) hstr[i++] = tstr[j];
		}
	    }
	    else if (k < 0) hstr[i++] = tstr[j];
	}
	else if (k < 0) k = j;
    }
    ddx = XL + 0.5 - 0.5 * len * ch_w;
    if (first) {
	if (tstr[j]) ddy += 0.68;
	else ddy += 0.5;
    }

    hstr[i] = 0;

    if (k >= 0) j = k; /* j is first color code position */
    do {
	if (k == 0) {
	    c = tstr[j++];
	    if (c == '$') {
		switch (tstr[j++]) {
		    case '0': ncolor = RGBids[0]; break; /* red */
		    case '1': ncolor = RGBids[1]; break; /* green */
		    case '2': ncolor = RGBids[2]; break; /* blue */
		    case '3': ncolor = Yellow; break;
		    default : ncolor = Gridnr; /* white */
		}
	    }
	    i = 0;
	    while (tstr[j] >= ' ' && tstr[j] != '&') {
		if (tstr[j] == '$') { /* color code */
		    if (isdigit (tstr[j+1])) break;
		    if (tstr[++j] < ' ') break;
		}
		hstr[i++] = tstr[j++];
	    }
	    if (i > 0) {
		hstr[i] = 0;
		if (c == '&') {
		    if (color != Yellow) ggSetColor (color = Yellow);
		    c = hstr[1];
		    hstr[1] = 0;
		    d_text (ddx, ddy, hstr); ddx += ch_w; --i; k = 1;
		    hstr[1] = c;
		}
	    }
	}
	else k = 0;
	if (i > 0) {
	    if (color != ncolor) ggSetColor (color = ncolor);
	    d_text (ddx, ddy, hstr+k); ddx += i * ch_w;
	}
	k = 0;
    } while (tstr[j] >= ' ');

    if (tstr[j] && first) {
	first = 0;
	ddy -= 0.35;
	tstr += j+1;
	goto again;
    }

    if (color != Gridnr) ggSetColor (Gridnr);
}

int ask (int nr_alt, char *alt_arr[], int old)
{
    int choice;

    if (alt_arr == no_yes) ask_mode = 1;
    else if (alt_arr == yes_no) ask_mode = 2;
    menu (nr_alt, alt_arr);
    if (old != -1) pre_cmd_proc (old, alt_arr);
    choice = get_com ();
    if (old != -1) post_cmd_proc (old, alt_arr);
    pre_cmd_proc (choice = Min (choice, nr_alt - 1), alt_arr);
    ask_mode = 0;
    return (choice);
}

/* #define OLD_STYLE */
#ifdef OLD_STYLE
/*
** Turn lamp of menu item ON and set Curr_lamp flag.
*/
void pre_cmd_proc (int nbr, char *names[])
{
    float ddx, ddy, ch_w, ch_h;

    if (nbr < 0 || nbr >= Curr_nr_alt) return;
    if (names && strcmp (names[nbr], Curr_alt_arr[nbr])) return;

    if (Curr_lamps[nbr] && names != Curr_alt_arr) return;
    Curr_lamps[nbr] = 1; /* store for redraw in window system */

    set_c_wdw (MENU);

    ddx = XL;
    ddy = YB + nbr;
    ggSetColor (Yellow);
    paint_box (ddx, XR, ddy, ddy + 1.0);

    if (names) {
	ggSetColor (Backgr);
	ch_siz (&ch_w, &ch_h);
	d_text (ddx + 0.5 - 0.5 * strlen (names[nbr]) * ch_w, ddy + 0.5 - 0.5 * ch_h, names[nbr]);
    }

    disp_mode (DOMINANT);
    ggSetColor (Gridnr);
    if (nbr) d_line (ddx, ddy, XR, ddy);
    if (ddy + 1.1 < YT) d_line (ddx, ddy + 1.0, XR, ddy + 1.0);
    disp_mode (TRANSPARENT);
    flush_pict ();
}

/*
** Turn lamp of menu item OFF and reset Curr_lamp flag.
*/
void post_cmd_proc (int nbr, char *names[])
{
    float ddx, ddy, ch_w, ch_h;

    if (nbr < 0 || nbr >= Curr_nr_alt) return;
    if (names && strcmp (names[nbr], Curr_alt_arr[nbr])) return;

    set_c_wdw (MENU);

    ddx = XL;
    ddy = YB + nbr;
    ggClearArea (ddx, XR, ddy, ddy + 1.0);

    disp_mode (DOMINANT);
    ggSetColor (Gridnr);
    if (nbr) d_line (ddx, ddy, XR, ddy);
    if (ddy + 1.1 < YT) d_line (ddx, ddy + 1.0, XR, ddy + 1.0);
    if (names) {
	ch_siz (&ch_w, &ch_h);
	d_text (ddx + 0.5 - 0.5 * strlen (names[nbr]) * ch_w, ddy + 0.5 - 0.5 * ch_h, names[nbr]);
    }
    Curr_lamps[nbr] = 0;
    disp_mode (TRANSPARENT);
    flush_pict ();
}
#else /*  NOT OLD_STYLE */
/*
** Turn lamp of menu item ON and set Curr_lamp flag.
*/
void pre_cmd_proc (int nbr, char *names[])
{
    double ddy;

    if (nbr < 0 || nbr >= Curr_nr_alt) return;
    if (names && strcmp (names[nbr], Curr_alt_arr[nbr])) return;

    if (Curr_lamps[nbr] && names != Curr_alt_arr) return;
    Curr_lamps[nbr] = 1; /* store for redraw in window system */

    set_c_wdw (MENU);
    ggSetColor (Yellow);
    ddy = YB + nbr;
    paint_box (XL + 0.05, XL + 0.10, ddy + 0.1, ddy + 0.9);
    paint_box (XR - 0.10, XR - 0.05, ddy + 0.1, ddy + 0.9);
    paint_box (XL + 0.05, XR - 0.05, ddy + 0.1, ddy + 0.2);
    paint_box (XL + 0.05, XR - 0.05, ddy + 0.8, ddy + 0.9);
    flush_pict ();
}

/*
** Turn lamp of menu item OFF and reset Curr_lamp flag.
*/
void post_cmd_proc (int nbr, char *names[])
{
#if 0
    double ddy;
#else
    float ddy, ch_w, ch_h;
#endif

    if (nbr < 0 || nbr >= Curr_nr_alt) return;
    if (names && strcmp (names[nbr], Curr_alt_arr[nbr])) return;

    set_c_wdw (MENU);
    ddy = YB + nbr;
#if 0
    ggSetColor (Yellow);
    disp_mode (ERASE);
    paint_box (XL + 0.05, XL + 0.10, ddy + 0.1, ddy + 0.9);
    paint_box (XR - 0.10, XR - 0.05, ddy + 0.1, ddy + 0.9);
    paint_box (XL + 0.05, XR - 0.05, ddy + 0.1, ddy + 0.2);
    paint_box (XL + 0.05, XR - 0.05, ddy + 0.8, ddy + 0.9);
#else
    ggClearArea (XL, XR, ddy, ddy + 1.0);
    ggSetColor (Gridnr);
    disp_mode (DOMINANT);
    if (ddy + 1.1 < YT) d_line (XL, ddy + 1.0, XR, ddy + 1.0);
    if (names) {
	ch_siz (&ch_w, &ch_h);
	printtext (ddy - 0.5 * ch_h, names[nbr], ch_w);
    }
#endif
    disp_mode (TRANSPARENT);
    flush_pict ();
    Curr_lamps[nbr] = 0;
}
#endif /* NOT OLD_STYLE */
