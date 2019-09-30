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

#include "X11/Xlib.h"
#include "src/ocean/seadali/header.h"

extern void set_text_cursor (int text_mode);

extern struct Disp_wdw *c_wdw;
extern int  nr_planes;
extern int  Cur_nr;
extern int  Textnr;

#ifdef MAX_LEN
#undef MAX_LEN
#endif
#define MAX_LEN MAXCHAR + 40
static char Curr_txt[MAX_LEN + 2];
static int  Curr_txt_color_id;
static int  enter_txt_mode;

/*
** Redraw the error message.
*/
void Rtext ()
{
    err_meas (Curr_txt, Curr_txt_color_id);
    if (enter_txt_mode) Prompt ();
}

int ask_name (char *string, char *namep, unsigned chk_name)
{
    char answer_str[MAXCHAR], *head_p, *hp1;

    while (TRUE) {
	ask_string (string, answer_str);

	/* strip white space */
	head_p = answer_str;
	while (*head_p == ' ' || *head_p == '\t') ++head_p;
	if (*head_p == '\0') return (-1); /* no legal chars */

	hp1 = head_p + strlen (head_p) - 1;
	while (*hp1 == ' ' || *hp1 == '\t') *hp1-- = '\0';

	if (chk_name == FALSE || dmTestname (head_p) != -1) {
	    /* if chk_name == TRUE a valid name will be returned */
	    strcpy (namep, head_p);
	    return (0);
	}
	else {
	    ptext ("Bad name, try again!");
	    sleep (2);
	}
    }
}

char * strsave (char *s)
{
    char *p;
    MALLOCN (p, char, strlen (s) + 1);
    if (p) strcpy (p, s);
    return (p);
}

void pr_nomem ()
{
    ptext ("No memory available! (command aborted)");
}

void track_coord (Coor xx, Coor yy, int erase_only)
{
    static int track_flag = FALSE;
    static Coor oldxx, oldyy;
    char coord[MAXCHAR];

    set_c_wdw (PICT);

    if (track_flag) {
	/* old cross present: erase */
	(void) draw_cross (oldxx, oldyy);
	track_flag = FALSE;
    }

    if (erase_only == TRUE) return;

    /*
    ** If not erase-only: We have to draw a new cross.
    */
    if (draw_cross (xx, yy)) {
/*
	sprintf (coord, "cursor coordinates: %ld,%ld", (long) xx / QUAD_LAMBDA, (long) yy / QUAD_LAMBDA);
	ptext (coord);
*/
	print_image_crd (xx, yy);
    }
    else {
	ptext ("Coordinate outside window!");
	sleep (2);
	ptext ("");
    }

    /*
    ** Save displayed coordinates and flag presence of little cross.
    */
    oldxx = xx;
    oldyy = yy;
    track_flag = TRUE;
}

void ptext (char *str)
{
    static char prev_str[100];

    if (str) {
	err_meas (str, Textnr);
	/* store for next time .. */
	if (strncmp (str, "####", 3) != 0) strcpy (prev_str, str);
    }
    else
	err_meas (prev_str, Textnr);
}

void err_meas (char *str, int color_id)
{
    struct Disp_wdw *old_wdw;
    int color_id_prev, len;

    if (str && *str) {
	color_id_prev = ggGetColor ();
	old_wdw = c_wdw;
	set_c_wdw (TEXT);
	if (Curr_txt[0]) ggClearWindow ();
	if (Curr_txt != str) {
	    len = strlen (str);
	    if (len >= MAXCHAR) len = MAXCHAR - 1;
	    strncpy (Curr_txt, str, len);
	    Curr_txt[len] = '\0';
	}
	Curr_txt_color_id = color_id;
	ggSetColor (Curr_txt_color_id);
	d_text (1.0, 0.3, Curr_txt);
	if (old_wdw) set_c_wdw (old_wdw -> w_nr);
	ggSetColor (color_id_prev);
    }
    else if (Curr_txt[0]) {
	Curr_txt[0] = '\0';
	old_wdw = c_wdw;
	set_c_wdw (TEXT);
	ggClearWindow ();
	if (old_wdw) set_c_wdw (old_wdw -> w_nr);
    }
    flush_pict ();
}

void init_txtwdw (char *str)
{
    def_world_win (TEXT, 0.0, 100.0, 0.0, 1.0);
    ptext (str);
}

/*
** Ask_string can be used to obtain a string from
** the user via the text area.  Its first argument
** is a string which should be displayed prior to
** the request.  The second argument will contain
** the requested string.
** Ask_string sets the current 'window' to TEXT.
*/
void ask_string (char *disp_str, char *ret_str)
{
    struct Disp_wdw *old_wdw;
    int len;

    set_text_cursor (1);
    old_wdw = c_wdw;
    set_c_wdw (TEXT);
    ggClearWindow ();
    Curr_txt_color_id = Textnr;
    ggSetColor (Curr_txt_color_id);

    len = strlen (disp_str);
    if (len >= MAXCHAR) len = MAXCHAR - 1;
    strncpy (Curr_txt, disp_str, len);
    Curr_txt[len] = '\0';
    enter_txt_mode = 1;
    GetString (Curr_txt, len, MAX_LEN);
    enter_txt_mode = 0;

    strncpy (ret_str, Curr_txt + len, MAXCHAR - 1);
    if (old_wdw) set_c_wdw (old_wdw -> w_nr);
    set_text_cursor (0);
}
