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

#define	FACTOR	0.05

extern Window cwin; /* current window */
extern Window win_arr[]; /* window array */
extern float  c_sX, c_sY;
extern int    mW, mH, pH, tW, tH;
extern int    cW, cH; /* char Width, Height */

extern float  XL, XR, YB, YT;
extern float  c_cW, c_cH; /* current char. width/height */
extern int    not_disp_mess;
extern int    not_snap_mess;
extern int    zoom_mode;
extern struct Disp_wdw *wdw_arr[];
extern struct Disp_wdw *c_wdw;
extern struct Disp_wdw *p_wdw;
extern Coor   xltb, xrtb, ybtb, yttb;

int    retain_oldw = 0;
int    initwindow_flag = 0;
Coor   initwindow_xl, initwindow_yb, initwindow_dx;

void center_w (Coor xc, Coor yc)
{
    float ratio, dx, dy;

    ratio = p_wdw -> vymax / p_wdw -> vxmax;
    dx = (p_wdw -> wxmax - p_wdw -> wxmin) / 2;
    dy = dx * ratio;
    save_oldw ();
    def_world_win (PICT, xc - dx, xc + dx, yc - dy, yc + dy);
    pict_all (ERAS_DR);
    pic_max ();
    inform_window ();
}

void curs_w (Coor xl, Coor xr, Coor yb, Coor yt) /* zoom */
{
    float ratio, dx, dy, xc, yc, min;

    if (zoom_mode < 2) {
	xc = p_wdw -> wxmin;
	yc = p_wdw -> wymin;
	dx = (p_wdw -> wxmax - xc) / 4;
	dy = (p_wdw -> wymax - yc) / 4;
	if (zoom_mode == 0) {
	    xc += 2 * dx;
	    yc += 2 * dy;
	}
	else {
	    xc = xl;
	    yc = yb;
	}
    }
    else {
	dx = (float)(xr - xl) / 2;
	dy = (float)(yt - yb) / 2;
	xc = xl + dx;
	yc = yb + dy;
    }
    min = QUAD_LAMBDA * (2 + 4 * FACTOR);
    if (dx < min) dx = min;
    if (dy < min) dy = min;

    ratio = p_wdw -> vymax / p_wdw -> vxmax;

    if (dx * ratio > dy) dy = dx * ratio;
    else dx = dy / ratio;
    save_oldw ();
    def_world_win (PICT, xc - dx, xc + dx, yc - dy, yc + dy);
    pict_all (ERAS_DR);
    pic_max ();
    inform_window ();
}

void de_zoom (Coor xl, Coor xr, Coor yb, Coor yt)
{
    float ratio;	/* the ratio of the virtual window */
    float factor;	/* zoom-factor */
    float dx, dy, xc, yc;
    int   mode;

    xc = p_wdw -> wxmin;
    yc = p_wdw -> wymin;
    dx = p_wdw -> wxmax - xc;
    dy = p_wdw -> wymax - yc;

    ratio = p_wdw -> vymax / p_wdw -> vxmax;

    if ((mode = zoom_mode) < 2) {
	if (zoom_mode == 0) {
	    xc += dx / 2;
	    yc += dy / 2;
	}
	else {
	    xc = xl;
	    yc = yb;
	}
    }
    else {
	factor = 4;
	if ((float) (xr - xl) * ratio > (float) (yt - yb)) {
	    if (xr > xl) factor = dx / (xr - xl);
	    mode = 0;
	}
	else if (yt > yb) factor = dy / (yt - yb);
	if (factor > 4) factor = 4;
	xc = (xl + xr) / 2;
	yc = (yb + yt) / 2;
	dx *= factor / 2;
	dy *= factor / 2;
    }

    if (mode < 2) dy = dx * ratio;
    else dx = dy / ratio;

    save_oldw ();
    def_world_win (PICT, xc - dx, xc + dx, yc - dy, yc + dy);
    pict_all (ERAS_DR);
    pic_max ();
    inform_window ();
    not_disp_mess = not_snap_mess = 1;
}

static float old_xlw, old_xrw, old_ybw, old_ytw;

void prev_w ()
{
    float xlw, xrw, ybw, ytw;

    xlw = old_xlw;
    xrw = old_xrw;
    ybw = old_ybw;
    ytw = old_ytw;
    save_oldw ();
    def_world_win (PICT, xlw, xrw, ybw, ytw);
    pict_all (ERAS_DR);
    pic_max ();
    inform_window ();
}

void save_oldw () /* save old picture window for "prev_w" */
{
    if (retain_oldw) return;
    old_xlw = p_wdw -> wxmin;
    old_xrw = p_wdw -> wxmax;
    old_ybw = p_wdw -> wymin;
    old_ytw = p_wdw -> wymax;
}

void bound_w ()
{
    float xlw, xrw, ybw, ytw, ratio, w_width, w_height;
    Coor  xrb, ytb;

    save_oldw ();
    upd_boundb ();

    if (initwindow_flag == 2) {
	initwindow ();
	goto set_dr;
    }
    xrb = Max (xrtb, xltb + 4 * QUAD_LAMBDA);
    ytb = Max (yttb, ybtb + 4 * QUAD_LAMBDA);
    w_width  = xrb - xltb;
    w_height = ytb - ybtb;

    ratio = p_wdw -> vymax / p_wdw -> vxmax;
    if (w_width * ratio > w_height) {
	w_width *= FACTOR;
	xlw = xltb - w_width;
	xrw = xrb  + w_width;
	ybw = ybtb - w_width;
	ytw = ybw + (xrw - xlw) * ratio;
    }
    else {
	w_height *= FACTOR;
	ybw = ybtb - w_height;
	ytw = ytb  + w_height;
	xlw = xltb - w_height;
	xrw = xlw + (ytw - ybw) / ratio;
    }
    def_world_win (PICT, xlw, xrw, ybw, ytw);
    pic_max ();
set_dr:
    pict_all (ERAS_DR);
    inform_window ();
    not_disp_mess = not_snap_mess = 1;
}

void initwindow ()
{
    float xlw, xrw, ybw, ytw, ratio;

    ratio = p_wdw -> vymax / p_wdw -> vxmax;
    if (!initwindow_flag) {
	xlw = ybw = 0.0;
	xrw = 45.0 * QUAD_LAMBDA;
	ytw = ratio * xrw;
    }
    else {
	xlw = initwindow_xl;
	ybw = initwindow_yb;
	xrw = initwindow_dx;
	if (xrw < 40) xrw = 40;
	ytw = ybw + ratio * xrw;
	xrw += xlw;
    }
    def_world_win (PICT, xlw, xrw, ybw, ytw);
    pic_max ();
}

/*
** Define window (Only ONE time called)
*/
void Def_window (int w_nr)
{
    struct Disp_wdw *wdw;

    if ((wdw = wdw_arr[w_nr])) fatal (0001, "Def_window");
    MALLOC (wdw, struct Disp_wdw);
    if (!wdw) fatal (0002, "Def_window");
    wdw_arr[w_nr] = wdw;
    wdw -> w_nr = w_nr;
    wdw -> wxmin = 0;
    wdw -> wxmax = 0;
    wdw -> wymin = 0;
    wdw -> wymax = 0;

    switch (w_nr) {
    case MENU:
	wdw -> vxmax = (float) mW;
	wdw -> vymax = (float) mH;
	break;
    case PICT:
	p_wdw = wdw;
	wdw -> vxmax = (float) tW;
	wdw -> vymax = (float) pH;
	break;
    case TEXT:
	wdw -> vxmax = (float) tW;
	wdw -> vymax = (float) tH;
	break;
    case LAYS:
	wdw -> vxmax = (float) mW;
	wdw -> vymax = (float) mH;
	break;
    }
}

void Set_window (int w_nr)
{
    float xlw, xrw, ybw, ytw, ratio;
    float old_vxmax, old_vymax;
    struct Disp_wdw *wdw;

    if (!(wdw = wdw_arr[w_nr])) fatal (0001, "Set_window");

    old_vxmax = wdw -> vxmax;
    old_vymax = wdw -> vymax;

    switch (w_nr) {
    case MENU:
	wdw -> vxmax = (float) mW;
	wdw -> vymax = (float) mH;
	break;
    case PICT:
	wdw -> vxmax = (float) tW;
	wdw -> vymax = (float) pH;
	break;
    case TEXT:
	wdw -> vxmax = (float) tW;
	wdw -> vymax = (float) tH;
	break;
    case LAYS:
	wdw -> vxmax = (float) mW;
	wdw -> vymax = (float) mH;
	break;
    }
    if (cwin) {
	if (w_nr == PICT) { /* rescale window co-ordinates */
	    if (old_vxmax != wdw -> vxmax) {
		ratio = wdw -> vxmax / old_vxmax;
		xlw = wdw -> wxmin;
		xrw = wdw -> wxmax;
		wdw -> wxmax = xlw + (xrw - xlw) * ratio;
		old_xrw = old_xlw + (old_xrw - old_xlw) * ratio;
	    }
	    if (old_vymax != wdw -> vymax) {
		ratio = wdw -> vymax / old_vymax;
		ybw = wdw -> wymin;
		ytw = wdw -> wymax;
		wdw -> wymax = ybw + (ytw - ybw) * ratio;
		old_ytw = old_ybw + (old_ytw - old_ybw) * ratio;
	    }
	}
	if (wdw == c_wdw) { /* recalculate transformation */
	    c_wdw = NULL;
	    set_c_wdw (w_nr);
	}
    }
}

/*
** Set current window and normalization transformation
** with	the specified name.
*/
void set_c_wdw (int w_nr)
{
    if (!c_wdw || c_wdw != wdw_arr[w_nr]) {
	/*
	** A new current window and transformation has to be set.
	*/
	c_wdw = wdw_arr[w_nr];
	if (!c_wdw) fatal (0001, "set_c_wdw");
	XL = c_wdw -> wxmin;
	XR = c_wdw -> wxmax;
	if (XR == XL) fatal (0001, "set_c_wdw");
	YB = c_wdw -> wymin;
	YT = c_wdw -> wymax;
	c_sX = c_wdw -> vxmax / (XR - XL);
	c_sY = c_wdw -> vymax / (YT - YB);
	c_cW = (float) cW / c_sX;
	c_cH = (float) cH / c_sY;
	cwin = win_arr[w_nr];
    }
}

/*
** Define new world window.
*/
void def_world_win (int w_nr, float Wxmin, float Wxmax, float Wymin, float Wymax)
{
    c_wdw = wdw_arr[w_nr];
    if (!c_wdw) fatal (0001, "def_world_win");
    c_wdw -> wxmin = Wxmin;
    c_wdw -> wymin = Wymin;
    c_wdw -> wxmax = Wxmax;
    c_wdw -> wymax = Wymax;
    c_wdw = NULL; /* recalculate transformation */
    set_c_wdw (w_nr);
}
