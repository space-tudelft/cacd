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

extern Coor piwl, piwr, piwb, piwt;
extern Coor xlc, ybc; /* cursor parameters */
extern int  Cur_nr;
extern int  NR_lay;
extern int  Textnr;
extern int *def_arr;
extern int *pict_arr;
extern int *vis_arr;
extern int  v_term;
extern int  erase_text;
extern qtree_t **quad_root;
extern TERM **TBuf;
extern struct obj_node **PutBuf;

static Coor b_x1, b_y1, b_x2, b_y2;
static Coor b_line[100][6];
static Coor s_line[100][6];
static Coor xlc_old, ybc_old; /* old cursor position */
static Coor x_off, y_off;
static int  ih, iv, isv, isw;
static int  m1, m2, m3, m4;

int first_pict_buf = 1;

static void build_set (Coor line[]);
static void place_buffer (Coor xl, Coor yb);
static void place_tbuf (Coor xl, Coor yb);
static void yank_lay_area (int lay, Coor xl, Coor xr, Coor yb, Coor yt);

void clearPutBuf (int lay)
{
    register struct obj_node *p, *pn;

    if ((p = PutBuf[lay])) { /* clear put buffer */
	do {
	    pn = p -> next;
	    FREE (p);
	} while ((p = pn));
	PutBuf[lay] = NULL;
    }
}

void fill_buffer (Coor xl, Coor xr, Coor yb, Coor yt)
{
    register int lay;

    for (lay = 0; lay < NR_lay; ++lay) {
	clearPutBuf (lay);
	if (def_arr[lay]) { /* fill buffer for current layer */
	    yank_lay_area (lay, xl, xr, yb, yt);
	}
    }
}

#define PUT_BUF 1
#define SET_CUR 2
#define ROT_CW  3
#define ROT_CCW 4
#define MIR_Y   5
#define MIR_X   6

/*
** Put_buffer is used to put either the box-buffer or
** the terminal-buffer.  The argument termflag is used
** to switch between these alternatives when necessary.
** Most of the code, however, is unaware of this.
** Note: pict_cur() does set Color and COMPLEMENT mode.
*/
void put_buffer (int termflag)
{
    static char *ask_str[] = {
	 /* 0 */ "-return-",
	 /* 1 */ "PUT_BUF",
	 /* 2 */ "set_cursor",
	 /* 3 */ "rotate_CW",
	 /* 4 */ "rotate_CCW",
	 /* 5 */ "mirror_Y",
	 /* 6 */ "mirror_X",
    };
    char mess_str[MAXCHAR];
    Coor ll, rr, bb, tt, line[8];
    register struct obj_node *p;
    register TERM *tp;
    register int lay;
    int  cmd, i, j;
    int  empty = 1;
    int  first = 1;

    m1 = m4 = 1; m2 = m3 = 0;
    ih = iv = isv = isw = 0; /* reset indices of line buffer */
    if (!termflag) {
	for (lay = 0; lay < NR_lay; ++lay) {
	    if (!(p = PutBuf[lay])) continue;
	    empty = 0;
	    if (vis_arr[lay])
	    do {
		if (first) {
		    first = 0;
		    b_x1 = p -> ll_x1;
		    b_y1 = p -> ll_y1;
		    b_x2 = p -> ll_x2;
		    b_y2 = p -> ll_y2;
		}
		else {
		    if (p -> ll_x1 < b_x1) b_x1 = p -> ll_x1;
		    if (p -> ll_y1 < b_y1) b_y1 = p -> ll_y1;
		    if (p -> ll_x2 > b_x2) b_x2 = p -> ll_x2;
		    if (p -> ll_y2 > b_y2) b_y2 = p -> ll_y2;
		}
		if (trap_to_poly (line, p))
		    build_set (line);
		else
		    notify ("Illegal trapezoid!");
	    } while ((p = p -> next));
	}
    }
    else {
	for (lay = 0; lay < NR_lay; ++lay) {
	    if (!(tp = TBuf[lay])) continue;
	    empty = 0;
	    if (vis_arr[v_term])
	    do {
		term_win (tp, &ll, &rr, &bb, &tt, 1);
		if (first) {
		    first = 0;
		    b_x1 = ll;
		    b_y1 = bb;
		    b_x2 = rr;
		    b_y2 = tt;
		}
		else {
		    if (ll < b_x1) b_x1 = ll;
		    if (bb < b_y1) b_y1 = bb;
		    if (rr > b_x2) b_x2 = rr;
		    if (tt > b_y2) b_y2 = tt;
		}
		ll = tp -> xl;
		rr = tp -> xr;
		bb = tp -> yb;
		tt = tp -> yt;
		if (tp -> xr == tp -> xl) ++rr;
		if (tp -> yt == tp -> yb) ++tt;
		for (i = 0; i <= tp -> nx; ++i) {
		    for (j = 0; j <= tp -> ny; ++j) {
			line[6] = line[0] = ll + i * tp -> dx;
			line[4] = line[2] = rr + i * tp -> dx;
			line[3] = line[1] = bb + j * tp -> dy;
			line[7] = line[5] = tt + j * tp -> dy;
			build_set (line);
		    }
		}
	    } while ((tp = tp -> nxttm));
	}
    }

    if (empty) {
	ptext ("Buffer is empty!");
	return;
    }
    if (first) {
	if (termflag)
	    ptext ("Terminals not visible!");
	else
	    ptext ("Layers not visible!");
	return;
    }

    x_off = (b_x1 / QUAD_LAMBDA) * QUAD_LAMBDA;
    y_off = (b_y1 / QUAD_LAMBDA) * QUAD_LAMBDA;
do_menu:
    menu (sizeof (ask_str) / sizeof (char *), ask_str);
    ptext ("Enter point to put buffer!");
    set_c_wdw (PICT);
again:
    while ((cmd = get_cursor (1, 1, SNAP)) == -1) { /* a position given */
	if (!first_pict_buf) {
	    if (xlc == xlc_old && ybc == ybc_old) {
		cmd = PUT_BUF;
		break;
	    }
	    pict_buf (); /* clear cursor rectangles */
	}
	else first_pict_buf = 0;
	xlc_old = xlc; ybc_old = ybc;
	pict_buf (); /* draw cursor rectangles */
	sprintf (mess_str,
	    "Buffer at (%ld,%ld), select another position?", xlc_old, ybc_old);
	ptext (mess_str);
    }
    if (first_pict_buf && cmd) {
	if (cmd == -2) goto do_menu;
	goto again;
    }
    if (cmd == SET_CUR) {
	pre_cmd_proc (SET_CUR);
	ptext ("Select point to set cursor!");
	cmd = get_cursor (1, 1, NO_SNAP);
	post_cmd_proc (SET_CUR);
	if (cmd == -1) {
	    pict_cur (xlc_old, ybc_old); /* clear cursor spot */
	    x_off += m1 * (xlc - xlc_old) + m3 * (ybc - ybc_old);
	    y_off += m2 * (xlc - xlc_old) + m4 * (ybc - ybc_old);
	    pict_cur (xlc_old = xlc, ybc_old = ybc); /* draw cursor spot */
	}
	sprintf (mess_str,
	    "Buffer at (%ld,%ld), select another position?", xlc_old, ybc_old);
	ptext (mess_str);
	if (cmd < 0 || cmd == SET_CUR) goto again;
    }
    if (cmd == ROT_CW) { /* rotate clockwise */
	pict_buf (); /* clear cursor rectangles */
	if (m1) { m2 = m4; m3 = -m1; m1 = m4 = 0; }
	else    { m1 = m3; m4 = -m2; m2 = m3 = 0; }
	pict_buf (); /* draw cursor rectangles */
	goto again;
    }
    if (cmd == ROT_CCW) { /* rotate counter-clockwise */
	pict_buf (); /* clear cursor rectangles */
	if (m1) { m2 = -m4; m3 = m1; m1 = m4 = 0; }
	else    { m1 = -m3; m4 = m2; m2 = m3 = 0; }
	pict_buf (); /* draw cursor rectangles */
	goto again;
    }
    if (cmd == MIR_Y) { /* mirror around y-axis */
	pict_buf (); /* clear cursor rectangles */
	m1 = -m1; m2 = -m2;
	pict_buf (); /* draw cursor rectangles */
	goto again;
    }
    if (cmd == MIR_X) { /* mirror around x-axis */
	pict_buf (); /* clear cursor rectangles */
	m3 = -m3; m4 = -m4;
	pict_buf (); /* draw cursor rectangles */
	goto again;
    }
    if (!first_pict_buf) {
	pict_buf (); /* clear cursor rectangles */
	if (cmd == PUT_BUF) {
	    if (termflag)
		place_tbuf (xlc_old, ybc_old);
	    else
		place_buffer (xlc_old, ybc_old);
	    picture ();
	    ggSetColor (Cur_nr);
	    disp_mode (COMPLEMENT);
	    pict_buf (); /* draw cursor rectangles */
	    goto again;
	}
	first_pict_buf = 1;
	if (cmd == -2) goto do_menu;
    }
    disp_mode (TRANSPARENT);
    erase_text = 1;
}

#define xL(i) b_line[i][0]
#define xR(i) b_line[i][1]
#define yH(i) b_line[i][2]
#define xV(i) b_line[i][3]
#define yB(i) b_line[i][4]
#define yT(i) b_line[i][5]

#define vx0(i) s_line[i][0]
#define vy1(i) s_line[i][1]
#define vy2(i) s_line[i][2]
#define wx0(i) s_line[i][3]
#define wy1(i) s_line[i][4]
#define wy2(i) s_line[i][5]

void pict_buf ()
{
    float xl, xr, yb, yt;
    register Coor x_c, y_c;
    register int i;

    pict_cur (x_c = xlc_old, y_c = ybc_old); /* clear cursor spot */

    x_c -= m1 * x_off + m2 * y_off;
    y_c -= m3 * x_off + m4 * y_off;

    /* clear/draw cursor rectangles of buffer */
    for (i = 0; i < ih; ++i) { /* hor. lines */
	xl = xL(i); xr = xR(i); yb = yH(i);
	if (m1) { /* R0 or R180 */
	    xl = m1 * xl + x_c;
	    xr = m1 * xr + x_c;
	    yb = m4 * yb + y_c;
	    d_line (xl, yb, xr, yb);
	}
	else {
	    yb = m2 * yb + x_c;
	    xl = m3 * xl + y_c;
	    xr = m3 * xr + y_c;
	    d_line (yb, xl, yb, xr);
	}
    }
    for (i = 0; i < iv; ++i) { /* ver. lines */
	xl = xV(i); yb = yB(i); yt = yT(i);
	if (m1) { /* R0 or R180 */
	    xl = m1 * xl + x_c;
	    yb = m4 * yb + y_c;
	    yt = m4 * yt + y_c;
	    d_line (xl, yb, xl, yt);
	}
	else {
	    yb = m2 * yb + x_c;
	    yt = m2 * yt + x_c;
	    xl = m3 * xl + y_c;
	    d_line (yb, xl, yt, xl);
	}
    }
    for (i = 0; i < isv; ++i) { /* slanting v-lines */
	yb = vy1(i);
	yt = vy2(i);
	xr = xl = vx0(i);
	xl += yb;
	xr += yt;
	if (m1) {
	    xl = m1 * xl + x_c; yb = m4 * yb + y_c;
	    xr = m1 * xr + x_c; yt = m4 * yt + y_c;
	    d_line (xl, yb, xr, yt);
	}
	else {
	    yb = m2 * yb + x_c; xl = m3 * xl + y_c;
	    yt = m2 * yt + x_c; xr = m3 * xr + y_c;
	    d_line (yb, xl, yt, xr);
	}
    }
    for (i = 0; i < isw; ++i) { /* slanting w-lines */
	yb = wy1(i);
	yt = wy2(i);
	xr = xl = wx0(i);
	xl -= yb;
	xr -= yt;
	if (m1) {
	    xl = m1 * xl + x_c; yb = m4 * yb + y_c;
	    xr = m1 * xr + x_c; yt = m4 * yt + y_c;
	    d_line (xl, yb, xr, yt);
	}
	else {
	    yb = m2 * yb + x_c; xl = m3 * xl + y_c;
	    yt = m2 * yt + x_c; xr = m3 * xr + y_c;
	    d_line (yb, xl, yt, xr);
	}
    }
    flush_pict ();
}

static void build_set (Coor line[])
{
    register int i, j, k, m;
    Coor exl, exr, yh;
    Coor eyb, eyt, xv;
    int  first;

    if (ih == 0) { /* empty set of lines */
	xL(ih) = line[0]; xR(ih) = line[2]; yH(ih) = line[3]; ++ih;
	xL(ih) = line[6]; xR(ih) = line[4]; yH(ih) = line[5]; ++ih;
	if (line[0] == line[6]) {
	    xV(iv) = line[0]; yB(iv) = line[1]; yT(iv) = line[7]; ++iv;
	}
	else if (line[0] < line[6]) { /* slanting v-line */
	    vx0(isv) = line[0] - line[1];
	    vy1(isv) = line[1]; vy2(isv) = line[7]; ++isv;
	}
	else { /* slanting w-line */
	    wx0(isw) = line[0] + line[1];
	    wy1(isw) = line[1]; wy2(isw) = line[7]; ++isw;
	}
	if (line[2] == line[4]) {
	    xV(iv) = line[2]; yB(iv) = line[3]; yT(iv) = line[5]; ++iv;
	}
	else if (line[2] < line[4]) { /* slanting v-line */
	    vx0(isv) = line[2] - line[3];
	    vy1(isv) = line[3]; vy2(isv) = line[5]; ++isv;
	}
	else { /* slanting w-line */
	    wx0(isw) = line[2] + line[3];
	    wy1(isw) = line[3]; wy2(isw) = line[5]; ++isw;
	}
    }
    else {
	first = 1;
	m = -1;
	/* for hor. lines: */
	exl = line[0]; exr = line[2]; yh = line[3];
h_beg:
	for (i = 0; i < ih; ++i) {
	    if (i == m) continue;
	    if (yh == yH(i) && exr >= xL(i) && exl <= xR(i)) {
		if (m == -1) {
		    if (exl < xL(i)) m = i;
		    else if (exr <= xR(i)) goto skip1;
		    if (exr > xR(i)) {
			if (m != i) { m = i; exl = xL(i); }
			goto h_beg;
		    }
		    else if (m == i) {
			exr = xR(i);
			goto h_beg;
		    }
		}
		else { /* lines i & m connected! */
		    if (xL(i) < exl) exl = xL(i);
		    if (xR(i) > exr) exr = xR(i);
		    /* remove line i */
		    if (m > i) { k = m; m = i; }
		    else { k = i; --i; }
		    for (j = k + 1; j < ih; ++j) {
			xL(k) = xL(j); xR(k) = xR(j); yH(k) = yH(j); ++k;
		    }
		    --ih;
		}
	    }
	}
	if (m != -1) { /* new line connected with line m */
	    /* make line m equal to expanded new line */
	    xL(m) = exl; xR(m) = exr; m = -1;
	}
	else if (ih < 100) { /* add new line to list */
	    xL(ih) = exl; xR(ih) = exr; yH(ih) = yh; ++ih;
	}
skip1:
	if (first) {
	    first = 0;
	    exl = line[6]; exr = line[4]; yh = line[5];
	    goto h_beg; /* do second hor. line */
	}

	/* for ver. lines: */
	first = 1;
	m = -1;
	if (line[0] != line[6]) goto skip2;
	xv = line[0]; eyb = line[1]; eyt = line[7];
v_beg:
	for (i = 0; i < iv; ++i) {
	    if (i == m) continue;
	    if (xv == xV(i) && eyt >= yB(i) && eyb <= yT(i)) {
		if (m == -1) {
		    if (eyb < yB(i)) m = i;
		    else if (eyt <= yT(i)) goto skip2;
		    if (eyt > yT(i)) {
			if (m != i) { m = i; eyb = yB(i); }
			goto v_beg;
		    }
		    else if (m == i) {
			eyt = yT(i);
			goto v_beg;
		    }
		}
		else { /* lines i & m connected! */
		    if (yB(i) < eyb) eyb = yB(i);
		    if (yT(i) > eyt) eyt = yT(i);
		    /* remove line i */
		    if (m > i) { k = m; m = i; }
		    else { k = i; --i; }
		    for (j = k + 1; j < iv; ++j) {
			yB(k) = yB(j); yT(k) = yT(j); xV(k) = xV(j); ++k;
		    }
		    --iv;
		}
	    }
	}
	if (m != -1) { /* new line connected with line m */
	    /* make line m equal to expanded new line */
	    yB(m) = eyb; yT(m) = eyt; m = -1;
	}
	else if (iv < 100) { /* add new line to list */
	    yB(iv) = eyb; yT(iv) = eyt; xV(iv) = xv; ++iv;
	}
skip2:
	if (first && line[2] == line[4]) {
	    first = 0;
	    xv = line[2]; eyb = line[1]; eyt = line[7];
	    goto v_beg; /* do second vert. line */
	}

	/* for slanting v-lines: */
	first = 1;
	m = -1;
	if (line[0] >= line[6]) goto skip3;
	eyb = line[1]; eyt = line[7]; xv = line[0] - eyb;
sv_beg:
	for (i = 0; i < isv; ++i) {
	    if (i == m) continue;
	    if (xv == vx0(i) && eyt >= vy1(i) && eyb <= vy2(i)) {
		if (m == -1) {
		    if (eyb < vy1(i)) m = i;
		    else if (eyt <= vy2(i)) goto skip3;
		    if (eyt > vy2(i)) {
			if (m != i) { m = i; eyb = vy1(i); }
			goto sv_beg;
		    }
		    else if (m == i) {
			eyt = vy2(i);
			goto sv_beg;
		    }
		}
		else { /* lines i & m connected! */
		    if (vy1(i) < eyb) eyb = vy1(i);
		    if (vy2(i) > eyt) eyt = vy2(i);
		    /* remove line i */
		    if (m > i) { k = m; m = i; }
		    else { k = i; --i; }
		    for (j = k + 1; j < isv; ++j) {
			vy1(k) = vy1(j); vy2(k) = vy2(j); vx0(k) = vx0(j); ++k;
		    }
		    --isv;
		}
	    }
	}
	if (m != -1) { /* new line connected with line m */
	    /* make line m equal to expanded new line */
	    vy1(m) = eyb; vy2(m) = eyt; m = -1;
	}
	else if (isv < 100) { /* add new line to list */
	    vy1(isv) = eyb; vy2(isv) = eyt; vx0(isv) = xv; ++isv;
	}
skip3:
	if (first && line[2] < line[4]) {
	    first = 0;
	    eyb = line[3]; eyt = line[5]; xv = line[2] - eyb;
	    goto sv_beg; /* do second line */
	}

	/* for slanting w-lines: */
	first = 1;
	m = -1;
	if (line[0] <= line[6]) goto skip4;
	eyb = line[1]; eyt = line[7]; xv = line[0] + eyb;
sw_beg:
	for (i = 0; i < isw; ++i) {
	    if (i == m) continue;
	    if (xv == wx0(i) && eyt >= wy1(i) && eyb <= wy2(i)) {
		if (m == -1) {
		    if (eyb < wy1(i)) m = i;
		    else if (eyt <= wy2(i)) goto skip4;
		    if (eyt > wy2(i)) {
			if (m != i) { m = i; eyb = wy1(i); }
			goto sw_beg;
		    }
		    else if (m == i) {
			eyt = wy2(i);
			goto sw_beg;
		    }
		}
		else { /* lines i & m connected! */
		    if (wy1(i) < eyb) eyb = wy1(i);
		    if (wy2(i) > eyt) eyt = wy2(i);
		    /* remove line i */
		    if (m > i) { k = m; m = i; }
		    else { k = i; --i; }
		    for (j = k + 1; j < isw; ++j) {
			wy1(k) = wy1(j); wy2(k) = wy2(j); wx0(k) = wx0(j); ++k;
		    }
		    --isw;
		}
	    }
	}
	if (m != -1) { /* new line connected with line m */
	    /* make line m equal to expanded new line */
	    wy1(m) = eyb; wy2(m) = eyt; m = -1;
	}
	else if (isw < 100) { /* add new line to list */
	    wy1(isw) = eyb; wy2(isw) = eyt; wx0(isw) = xv; ++isw;
	}
skip4:
	if (first && line[2] > line[4]) {
	    first = 0;
	    eyb = line[3]; eyt = line[5]; xv = line[2] + eyb;
	    goto sw_beg; /* do second line */
	}
    }
}

static void place_buffer (Coor xl, Coor yb)
{
    register int lay;
    register struct obj_node *p, *list, *new;
    Coor x1, x2, y1, y2, dx, dy, z;
    int  ls, rs, first = 1;

    xl -= m1 * x_off + m2 * y_off;
    yb -= m3 * x_off + m4 * y_off;

    for (lay = 0; lay < NR_lay; ++lay) {
	if (vis_arr[lay] && (p = PutBuf[lay])) {
	    list = NULL;
	    do {
		rs = p -> sides;
		if (m1) { /* R0 or R180 */
		    if (rs) {
			if (m1 > 0) {
			    if (m4 < 0) {
				if ((ls = rs & 12)) ls = ~ls & 12;
				if ((rs = rs & 03)) rs = ~rs & 03;
				rs += ls;
			    }
			}
			else {
			    ls = (rs & 3) << 2;
			    rs >>= 2;
			    if (m4 > 0) {
				if (ls) ls = ~ls & 12;
				if (rs) rs = ~rs & 03;
			    }
			    rs += ls;
			}
		    }
		    x1 = xl + m1 * p -> ll_x1;
		    x2 = xl + m1 * p -> ll_x2;
		    y1 = yb + m4 * p -> ll_y1;
		    y2 = yb + m4 * p -> ll_y2;
		}
		else { /* R90 or R270 */
		    x1 = xl + m2 * p -> ll_y1;
		    x2 = xl + m2 * p -> ll_y2;
		    y1 = yb + m3 * p -> ll_x1;
		    y2 = yb + m3 * p -> ll_x2;
		}
		if (x2 < x1) { z = x1; x1 = x2; x2 = z; }
		if (y2 < y1) { z = y1; y1 = y2; y2 = z; }

		if (first) { first = 0;
		    piwl = x1;
		    piwr = x2;
		    piwb = y1;
		    piwt = y2;
		}
		else {
		    if (x1 < piwl) piwl = x1;
		    if (x2 > piwr) piwr = x2;
		    if (y1 < piwb) piwb = y1;
		    if (y2 > piwt) piwt = y2;
		}

		if (m2 && rs) { /* R90 or R270 */
		    dx = x2 - x1;
		    if (m3 > 0) { ls = rs >> 2; rs &= 3; }
		    else        { ls = rs & 3; rs >>= 2; }
		    if (m2 != m3) { /* R90 or R270 */
			if (rs) rs = ~rs & 3;
			if (ls) ls = ~ls & 3;
		    }
		    if (ls && rs) {
			if (ls == 2) ls <<= 2;
			if (rs == 1) rs <<= 2;
			if (y2 >= y1 + 2 * dx) {
			    MALLOC (new, struct obj_node);
			    new -> ll_x1 = x1;
			    new -> ll_x2 = x2;
			    new -> ll_y1 = y1; y1 += dx;
			    new -> ll_y2 = y1;
			    new -> sides = ls;
			    new -> next = list;
			    list = new;
			    if (y1 < y2 - dx) {
				MALLOC (new, struct obj_node);
				new -> ll_x1 = x1;
				new -> ll_x2 = x2;
				new -> ll_y1 = y1; y1 = y2 - dx;
				new -> ll_y2 = y1;
				new -> sides = 0;
				new -> next = list;
				list = new;
			    }
			}
			else { /* y2 - dx < y1 + dx */
			    dy = (y1 + dx) - (y2 - dx);
			    MALLOC (new, struct obj_node);
			    if (ls == 1) {
				new -> ll_x1 = x1;
				new -> ll_x2 = x2 - dy;
			    }
			    else {
				new -> ll_x1 = x1 + dy;
				new -> ll_x2 = x2;
			    }
			    new -> ll_y1 = y1;
			    new -> ll_y2 = y2 - dx;
			    new -> sides = ls;
			    new -> next = list;
			    list = new;
			    MALLOC (new, struct obj_node);
			    new -> ll_x1 = x1;
			    new -> ll_x2 = x2;
			    new -> ll_y1 = y2 - dx; y1 += dx;
			    new -> ll_y2 = y1;
			    new -> sides = ls + rs;
			    new -> next = list;
			    list = new;
			    if (ls == 1) x1 += dy;
			    else x2 -= dy;
			}
		    }
		    else if (ls) {
			if (ls == 1) rs = ls;
			else rs = ls << 2;
			if (y2 > y1 + dx) {
			    MALLOC (new, struct obj_node);
			    new -> ll_x1 = x1;
			    new -> ll_x2 = x2;
			    new -> ll_y1 = y1; y1 += dx;
			    new -> ll_y2 = y1;
			    new -> sides = rs; rs = 0;
			    new -> next = list;
			    list = new;
			}
		    }
		    else { /* rs */
			if (rs == 1) rs <<= 2;
			if (y1 < y2 - dx) {
			    MALLOC (new, struct obj_node);
			    new -> ll_x1 = x1;
			    new -> ll_x2 = x2;
			    new -> ll_y1 = y1; y1 = y2 - dx;
			    new -> ll_y2 = y1;
			    new -> sides = 0;
			    new -> next = list;
			    list = new;
			}
		    }
		}
		MALLOC (new, struct obj_node);
		new -> ll_x1 = x1;
		new -> ll_x2 = x2;
		new -> ll_y1 = y1;
		new -> ll_y2 = y2;
		new -> sides = rs;
		new -> next = list;
		list = new;
	    } while ((p = p -> next));
	    pict_arr[lay] = DRAW;
	    add_new_traps (lay, list);
	    do {
		p = list -> next;
		FREE (list);
	    } while ((list = p));
	}
    }
}

static void place_tbuf (Coor xl, Coor yb)
{
    Coor  dx, dy, x1, x2, y1, y2, z;
    int   nx, ny;
    register TERM *tp, *tn;
    register int lay, first = 1;

    xl -= m1 * x_off + m2 * y_off;
    yb -= m3 * x_off + m4 * y_off;

    for (lay = 0; lay < NR_lay; ++lay) {
	if ((tp = TBuf[lay]))
	do {
	    if (m1) { /* R0 or R180 */
		dx = m1 * tp -> dx;
		dy = m4 * tp -> dy;
		nx = tp -> nx;
		ny = tp -> ny;
		x1 = xl + m1 * tp -> xl;
		x2 = xl + m1 * tp -> xr;
		y1 = yb + m4 * tp -> yb;
		y2 = yb + m4 * tp -> yt;
	    }
	    else { /* R90 or R270 */
		dx = m2 * tp -> dy;
		dy = m3 * tp -> dx;
		nx = tp -> ny;
		ny = tp -> nx;
		x1 = xl + m2 * tp -> yb;
		x2 = xl + m2 * tp -> yt;
		y1 = yb + m3 * tp -> xl;
		y2 = yb + m3 * tp -> xr;
	    }
	    if (x2 < x1) { z = x1; x1 = x2; x2 = z; }
	    if (y2 < y1) { z = y1; y1 = y2; y2 = z; }

	    if ((tn = new_term (x1, x2, y1, y2, tp -> tmname, tp -> tmlen, dx, nx, dy, ny, lay))) {
		term_win (tn, &x1, &x2, &y1, &y2, 1);
		if (first) { first = 0;
		    piwl = x1;
		    piwr = x2;
		    piwb = y1;
		    piwt = y2;
		}
		else {
		    if (x1 < piwl) piwl = x1;
		    if (x2 > piwr) piwr = x2;
		    if (y1 < piwb) piwb = y1;
		    if (y2 > piwt) piwt = y2;
		}
		pict_arr[lay] = DRAW;
	    }
	} while ((tp = tp -> nxttm));
    }
    if (!first) pict_arr[Textnr] = DRAW;
}

/*
** Copy objects from a user specified area in the yank buffer.
*/
static void yank_lay_area (int lay, Coor xl, Coor xr, Coor yb, Coor yt)
{
    struct obj_node  Area;
    struct obj_node *ilist;
    register struct obj_node *itrap;

    Area.ll_x1 = xl;
    Area.ll_y1 = yb;
    Area.ll_x2 = xr;
    Area.ll_y2 = yt;
    Area.sides = 0;
    Area.next  = 0;
    /*
    ** Search for intersecting trapezoids into the quad tree
    ** and put these in maximal hor. strip representation.
    */
    ilist = insert (&Area, lay, DELETE);

    if ((itrap = ilist))
    do {	   /* yank two trapezoids, add to PutBuf */
	PutBuf[lay] = yank_traps (itrap, &Area, PutBuf[lay]);
    } while ((itrap = itrap -> next));

    /*
    ** Add intersecting traps back into the quad tree.
    */
    add_quad (quad_root, ilist, lay);
}

/*
** Find those parts of p inside q.
** INPUT: pointers to trapezoids p and q.
** OUTPUT: a list of resulting trapezoids.
*/
struct obj_node * yank_traps (struct obj_node *p, struct obj_node *q, struct obj_node *yank_head)
{
    Coor a[4], crossy, interx2, interx4, pl, pr, ql, qr, dx, dy;
    Coor px1, px2, xmin, xmax, oldxmin, oldxmax, ymin, ymax;
    register int count, n;
    int pls, prs, qls, qrs;
    short sides;

    ymin = Max (p -> ll_y1, q -> ll_y1);
    ymax = Min (p -> ll_y2, q -> ll_y2);
    if (ymax <= ymin) return (yank_head);

    pls = p -> sides;
    if ((prs = pls & 3) > 1) prs = -1;
    if ((pls >>= 2)     > 1) pls = -1;
    qls = q -> sides;
    if ((qrs = qls & 3) > 1) qrs = -1;
    if ((qls >>= 2)     > 1) qls = -1;

    pl = (pls == 1) ? p -> ll_y1 : p -> ll_y2;
    ql = (qls == 1) ? q -> ll_y1 : q -> ll_y2;
    pr = (prs == 1) ? p -> ll_y2 : p -> ll_y1;
    qr = (qrs == 1) ? q -> ll_y2 : q -> ll_y1;

    px1 = p -> ll_x1;
    px2 = p -> ll_x2;

    /* find intersection between p->leftside and q->rightside */
    if ((n = pls - qrs)) {
	crossy = qr + (q -> ll_x2 - px1 + pls * (pl - qr)) / n;
	if (crossy > ymin && crossy < ymax) {
	    if (pls >= 0 && qrs <= 0) ymax = crossy;
	    else ymin = crossy;
	}
    }

    /* find intersection between p->rightside and q->leftside */
    if ((n = prs - qls)) {
	crossy = ql + (q -> ll_x1 - px2 + prs * (pr - ql)) / n;
	if (crossy > ymin && crossy < ymax) {
	    if (qls >= 0 && prs <= 0) ymax = crossy;
	    else ymin = crossy;
	}
    }

    count = 0;

    /* find intersection between p->leftside and q->leftside */
    if ((n = pls - qls)) {
	crossy = ql + (q -> ll_x1 - px1 + pls * (pl - ql)) / n;
	if (crossy > ymin && crossy < ymax) a[++count] = crossy;
    }

    /* find intersection between p->rightside and q->rigthside */
    if ((n = prs - qrs)) {
	crossy = qr + (q -> ll_x2 - px2 + prs * (pr - qr)) / n;
	if (crossy > ymin && crossy < ymax) {
	    if (count && crossy <= a[1]) {
		if (crossy < a[1]) { /* swap */
		    a[++count] = a[1]; a[1] = crossy;
		}
	    }
	    else a[++count] = crossy;
	}
    }

    a[++count] = ymax;
    ymax = ymin;

    /* intersection between current ymax and p->leftside */
    xmin = px1;
    if (pls) xmin += pls * (ymax - pl);
    /* intersection between current ymax and q->leftside */
    interx2 = q -> ll_x1;
    if (qls) interx2 += qls * (ymax - ql);
    /* intersection between current ymax and p->rightside */
    xmax = px2;
    if (prs) xmax += prs * (ymax - pr);
    /* intersection between current ymax and q->rightside */
    interx4 = q -> ll_x2;
    if (qrs) interx4 += qrs * (ymax - qr);
    if (interx2 > xmin) xmin = interx2;
    if (interx4 < xmax) xmax = interx4;

    for (n = 1; n <= count; ++n) {
	oldxmin = xmin;
	oldxmax = xmax;
	ymin = ymax;
	ymax = a[n];

	/* intersection between current ymax and p->leftside */
	xmin = px1;
	if (pls) xmin += pls * (ymax - pl);
	/* intersection between current ymax and q->leftside */
	interx2 = q -> ll_x1;
	if (qls) interx2 += qls * (ymax - ql);
	/* intersection between current ymax and p->rightside */
	xmax = px2;
	if (prs) xmax += prs * (ymax - pr);
	/* intersection between current ymax and q->rightside */
	interx4 = q -> ll_x2;
	if (qrs) interx4 += qrs * (ymax - qr);
	if (interx2 > xmin) xmin = interx2;
	if (interx4 < xmax) xmax = interx4;

	sides = 0;
	if (xmin < oldxmin) { oldxmin = xmin; sides = 8; }
	else if (xmin > oldxmin) sides = 4;
	if (xmax > oldxmax) { oldxmax = xmax; sides += 1; }
	else if (xmax < oldxmax) sides += 2;
	if (oldxmin < oldxmax) {
	    /*
	    ** merge trapezoid with other trapezoids?
	    */
	    dy = ymax - ymin;
	    for (p = yank_head; p; p = p -> next) {

		if (p -> sides != sides ||
		   (p -> ll_y2 != ymin && p -> ll_y1 != ymax)) continue;

		dx = p -> ll_y2 - p -> ll_y1;

		switch (sides >> 2) { /* leftside */
		case 0:
		    if (p -> ll_x1 != oldxmin) continue;
		    break;
		case 1:
		    if (p -> ll_y2 == ymin) {
			if (p -> ll_x1 + dx != oldxmin) continue;
		    }
		    else
			if (p -> ll_x1 != oldxmin + dy) continue;
		    break;
		case 2:
		    if (p -> ll_y2 == ymin) {
			if (p -> ll_x1 != oldxmin + dy) continue;
		    }
		    else
			if (p -> ll_x1 + dx != oldxmin) continue;
		}
		switch (sides & 3) { /* rightside */
		case 0:
		    if (p -> ll_x2 != oldxmax) continue;
		    break;
		case 1:
		    if (p -> ll_y2 == ymin) {
			if (p -> ll_x2 != oldxmax - dy) continue;
		    }
		    else
			if (p -> ll_x2 - dx != oldxmax) continue;
		    break;
		case 2:
		    if (p -> ll_y2 == ymin) {
			if (p -> ll_x2 - dx != oldxmax) continue;
		    }
		    else
			if (p -> ll_x2 != oldxmax - dy) continue;
		}
		/*
		** trapezoid can be merged
		*/
		if (p -> ll_y2 == ymin) {
		    p -> ll_y2 = ymax;
		    if (sides & 8) p -> ll_x1 = oldxmin;
		    if (sides & 1) p -> ll_x2 = oldxmax;
		}
		else {
		    p -> ll_y1 = ymin;
		    if (sides & 4) p -> ll_x1 = oldxmin;
		    if (sides & 2) p -> ll_x2 = oldxmax;
		}
		break;
	    }
	    if (!p) { /* not merged */
		MALLOC (p, struct obj_node);
		p -> ll_x1 = oldxmin;
		p -> ll_x2 = oldxmax;
		p -> ll_y1 = ymin;
		p -> ll_y2 = ymax;
		p -> sides = sides;
		p -> next = yank_head;
		yank_head = p;
	    }
	}
    }
    return (yank_head);
}
