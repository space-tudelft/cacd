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

#include "X11/Xlib.h"
#include "src/dali/header.h"

extern void set_text_cursor (int text_mode);

extern struct Disp_wdw *c_wdw;
extern float XL, XR, YB, YT;
extern int  Cur_nr;
extern int  NOdm_msg;
extern int  Textnr;
extern int  erase_text;
extern int  cmd_nbr, in_cmd, new_cmd;
extern Coor xlc, ybc; /* cursor parameters */

#ifdef MAX_LEN
#undef MAX_LEN
#endif
#define MAX_LEN MAXCHAR + 40
static char Curr_txt[MAX_LEN + 2];
static int  Curr_txt_len = 0;
static int  Rflag = 0;
static int  enter_txt_mode = 0; /* OFF */

/*
** Redraw the error message.
*/
void Rtext ()
{
    Rflag = 1;
    if (*Curr_txt) err_meas (Curr_txt);
    if (enter_txt_mode) Prompt ();
    Rflag = 0;
}

/*
** If (chk_name == TRUE) a valid name will be returned!
** If (chk_name == 2) name == "." is also allowed!
*/
int ask_name (char *disp_str, char *namep, int chk_name)
{
    char  answer_str[MAXCHAR];
    char  err_str[MAXCHAR];
    char *head, *tail;
    int   len, rv;

    while (TRUE) {
	len = ask_string (disp_str, answer_str);

	/* strip white space */
	head = answer_str;
	while (*head == ' ' || *head == '\t') ++head;
	if (!*head) return (0); /* no char's */

	tail = answer_str + len - 1;
	while (*tail == ' ' || *tail == '\t') --tail;
	*++tail = 0;

	len = tail - head; /* length of string */
	if (!chk_name) goto ret;
	if (chk_name == 2 && len == 1 && *head == '.') goto ret;
	NOdm_msg = 1;
	rv = dmTestname (head);
	NOdm_msg = 0;
	if (rv >= 0) goto ret;
	sprintf (err_str, "Bad name '%s', try again!", head);
	btext (err_str);
	sleep (2);
    }
ret:
    strcpy (namep, head);
    return (len);
}

char * strsave (char *str, int len)
{
    char *p;
    if (len <= 0) len = strlen (str);
    MALLOCN (p, char, len + 1);
    if (p) strcpy (p, str);
    return (p);
}

void pr_nomem ()
{
    ptext ("No memory available! (command aborted)");
}

static int  cross_present = 0; /* NO */
static Coor oldxx, oldyy;

void ask_coordnts ()
{
    char mess_str[MAXCHAR];

    set_c_wdw (PICT);
    ggSetColor (Cur_nr);
    disp_mode (TRANSPARENT);

    ptext ("Select coordinate point!");
    init_set_coord ();

    in_cmd = cmd_nbr;
    while (TRUE) {
	new_cmd = get_cursor (1, 1, SNAP);

	if (cross_present) {
	    draw_cross (oldxx, oldyy); /* erase old */
	    cross_present = 0; /* NO */
	}

	if (new_cmd != -1) break;

	draw_cross (oldxx = xlc, oldyy = ybc);
	cross_present = 1; /* YES */
	sprintf (mess_str, "Coordinate (%ld,%ld), select another point?",
	    (long) xlc / QUAD_LAMBDA, (long) ybc / QUAD_LAMBDA);
	ptext (mess_str);
	give_lay_stack (xlc, ybc);
    }
    erase_text = 1;
    in_cmd = 0;
    exit_set_coord ();
}

int get_coordnts ()
{
    char mess_str[MAXCHAR], *s;
    Coor x, y;
    int g = ask_name ("Enter coordinate <x,y>: ", mess_str, FALSE);

    if (g == 0) {
	if (!cross_present) { oldxx = oldyy = 0; }
    }
    else if (*mess_str != ',') {
	if (sscanf (mess_str, "%ld", &x) > 0) oldxx = x * QUAD_LAMBDA;
	else if (!cross_present) oldxx = 0;
	s = mess_str;
	while (*s && *s != ' ' && *s != ',') ++s;
	while (*s == ' ' || *s == ',') ++s;
	if (*s && sscanf (s, "%ld", &y) > 0) oldyy = y * QUAD_LAMBDA;
	else if (!cross_present) oldyy = 0;
    }
    else {
	if (sscanf (mess_str+1, "%ld", &y) > 0) oldyy = y * QUAD_LAMBDA;
	else if (!cross_present) oldyy = 0;
    }
    cross_present = 1; /* YES */
    center_w (oldxx, oldyy);
    sprintf (mess_str, "Center coordinate (%ld,%ld)", oldxx / QUAD_LAMBDA, oldyy / QUAD_LAMBDA);
    ptext (mess_str);
    return g;
}

void redraw_cross ()
{
    if (cross_present) draw_cross (oldxx, oldyy); /* redraw */
}

void notify (char *str)
{
    char  mess_str[MAXCHAR + 1];
    if (Curr_txt_len > MAXCHAR) {
	Curr_txt_len = MAXCHAR;
	Curr_txt[MAXCHAR] = 0;
    }
    strcpy (mess_str, Curr_txt);
    _ggBell (0);
    err_meas (str);
    sleep (1);
    err_meas (mess_str);
}

void btext (char *str)
{
    _ggBell (0);
    ptext (str);
}

void ptext (char *str)
{
    err_meas (str);
    erase_text = 0; /* another ptext does erase text! */
}

void err_meas (char *str)
{
    int color_id_prev;
    register int  len;
    register char *cp;
    struct Disp_wdw *old_wdw = c_wdw;

    color_id_prev = ggGetColor ();

    if (str && *str) {
	set_c_wdw (TEXT);
	if (!Rflag) { /* No redraw mode */
	    cp = Curr_txt;
	    for (len = 0; *str && len < MAXCHAR - 1; ++len) *cp++ = *str++;
	    *cp = 0;
	    Curr_txt_len = len;
	    ggClearWindow ();
	}
	ggSetColor (Textnr);
	d_text (1.0, 0.3, Curr_txt, Curr_txt_len);
    }
    else if (*Curr_txt) { /* Erase text and text window */
	*Curr_txt = 0;
	set_c_wdw (TEXT);
	ggClearWindow ();
    }

    flush_pict ();
    if (old_wdw) set_c_wdw (old_wdw -> w_nr);
    ggSetColor (color_id_prev);
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
int ask_string (char *disp_str, char *ret_str)
{
    int color_id_prev;
    register int  len;
    register char *cp;
    struct Disp_wdw *old_wdw = c_wdw;

    set_text_cursor (1);
    color_id_prev = ggGetColor ();
    set_c_wdw (TEXT);
    ggClearWindow ();
    ggSetColor (Textnr);

    cp = Curr_txt;
    for (len = 0; *disp_str && len < MAXCHAR - 1; ++len) *cp++ = *disp_str++;
    *cp = 0;

    enter_txt_mode = 1; /* ON */
    GetString (Curr_txt, len, MAX_LEN);
    enter_txt_mode = 0;

    for (len = 0; *cp && len < MAXCHAR - 1; ++len) *ret_str++ = *cp++;
    *ret_str = 0;

    if (old_wdw) set_c_wdw (old_wdw -> w_nr);
    ggSetColor (color_id_prev);
    set_text_cursor (0);
    return (len); /* ret_str length */
}
