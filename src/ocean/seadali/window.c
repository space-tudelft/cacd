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

extern Window cwin; /* current window */
extern Window win_arr[]; /* window array */
extern float  c_sX, c_sY, c_wX, c_wY;
extern int    mW, mH, pH, tW, tH;
extern int    cW, cH; /* char Width, Height */

extern short  CLIP_FLAG;
extern struct Disp_wdw *wdw_arr[];
extern struct Disp_wdw *c_wdw;
extern struct Disp_wdw *p_wdw;

float c_cW, c_cH; /* current char. width/height */

void center_w (Coor xx, Coor yy)
{
    float  xlw, xrw, ybw, ytw, ratio;
    Coor   i;

    /* do not clip trapezoids while painting */
    CLIP_FLAG = FALSE;

    save_oldw ();
    i = (Coor) ((p_wdw -> wxmax - p_wdw -> wxmin) / 2.0);
    xlw = (float) (xx - i);
    xrw = (float) (xx + i + 1);
    i = (Coor) ((p_wdw -> wymax - p_wdw -> wymin) / 2.0);
    ybw = (float) (yy - i);
    /*
    ** now adjust the maximal y-value to the ratio of the viewport
    */
    ratio = p_wdw -> vymax / p_wdw -> vxmax;
    ytw = ratio * (xrw - xlw) + ybw;

    def_world_win (PICT, xlw, xrw, ybw, ytw);
    pict_all (ERAS_DR);
    pic_max ();
    inform_window ();
}

void curs_w (Coor xl, Coor xr, Coor yb, Coor yt) /* zoom */
{
    float   xlw, xrw, ybw, ytw, ratio;

    /* do not clip trapezoids while painting */
    CLIP_FLAG = FALSE;

    if (xr == xl || yb == yt) return;

    /* Patrick: to prevent core dump: window never smaller than 3 x 3 lamba */
    if (Abs(xr - xl) < 16 || Abs(yt - yb) < 16) return;

    save_oldw ();

    ratio = p_wdw -> vymax / p_wdw -> vxmax;
    if ((float) (xr - xl) * ratio > (float) (yt - yb)) {
	xlw = (float) xl;
	xrw = (float) xr;
	/* Bottom is 'middle' y-coordinate minus half height. */
	ybw = (yb + yt) / 2 - (((xrw - xlw) * ratio) / 2);
	ytw = (xrw - xlw) * ratio + ybw;
    }
    else {
	ybw = (float) yb;
	ytw = (float) yt;
	/* Left is 'middle' x-coordinate minus half width. */
	xlw = (xl + xr) / 2 - (((ytw - ybw) / ratio) / 2);
	xrw = (ytw - ybw) / ratio + xlw;
    }
    def_world_win (PICT, xlw, xrw, ybw, ytw);
    pict_all (ERAS_DR);
    pic_max ();
    inform_window ();
}

void de_zoom (Coor xl, Coor xr, Coor yb, Coor yt)
{
    float  ratio;	/* the ratio of the virtual window */
    float  factor;	/* zoom-factor */
    float  xlw, xrw, ybw, ytw;
    Coor   int_xl, int_yb;

    /* do not clip trapezoids while painting */
    CLIP_FLAG = FALSE;

    if (xr == xl || yb == yt) return;

    save_oldw ();

    ratio = p_wdw -> vymax / p_wdw -> vxmax;
    if ((float) (xr - xl) * ratio > (float) (yt - yb))
	factor = (p_wdw -> wxmax - p_wdw -> wxmin) / (float) (xr - xl);
    else
	factor = (p_wdw -> wxmax - p_wdw -> wxmin) / (float) (yt - yb);
    factor = Min (4.0, factor);
    int_xl = (Coor) (p_wdw -> wxmin - factor * ((float) xl - p_wdw -> wxmin));
    int_yb = (Coor) (p_wdw -> wymin - factor * ((float) yb - p_wdw -> wymin));
    /*
    ** via integers to adjust left and bottom to grid
    */
    xlw = (float) int_xl;
    ybw = (float) int_yb;
    if ((float) (xr - xl) * ratio > (float) (yt - yb)) {
	xrw = xlw + factor * (p_wdw -> wxmax - p_wdw -> wxmin);
	ytw = (xrw - xlw) * ratio + ybw;
    }
    else {
	ytw = ybw + factor * (p_wdw -> wymax - p_wdw -> wymin);
	xrw = (ytw - ybw) / ratio + xlw;
    }

    def_world_win (PICT, xlw, xrw, ybw, ytw);
    pict_all (ERAS_DR);
    pic_max ();
    inform_window ();
}

static float old_xlw, old_xrw, old_ybw, old_ytw;

void prev_w ()
{
    float  xlw, xrw, ybw, ytw;

    /* do not clip trapezoids while painting */
    CLIP_FLAG = FALSE;

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

void save_oldw ()
{
    old_xlw = p_wdw -> wxmin;
    old_xrw = p_wdw -> wxmax;
    old_ybw = p_wdw -> wymin;
    old_ytw = p_wdw -> wymax;
}

extern Coor xltb, xrtb, ybtb, yttb;

#define	FACTOR	0.05

void bound_w ()
{
    float  xlw, xrw, ybw, ytw, ratio, w_width, w_height;
    Coor   xrb, ytb;

    /* do not clip trapezoids while painting */
    CLIP_FLAG = FALSE;

    save_oldw ();
    upd_boundb ();

    xrb = Max (xrtb, xltb + 4 * QUAD_LAMBDA);
    ytb = Max (yttb, ybtb + 4 * QUAD_LAMBDA);
    w_width = (float) (xrb - xltb);
    w_height = (float) (ytb - ybtb);

    ratio = p_wdw -> vymax / p_wdw -> vxmax;

    if (w_width * ratio > w_height) {
	xlw = (float) xltb - FACTOR * w_width;
	xrw = (float) xrb + FACTOR * w_width;
	ybw = (float) ybtb - FACTOR * ratio * w_width;
	ytw = (xrw - xlw) * ratio + ybw;
    }
    else {
	ybw = (float) ybtb - FACTOR * w_height;
	ytw = (float) ytb + FACTOR * w_height;
	xlw = (float) xltb - FACTOR * w_height / ratio;
	xrw = (ytw - ybw) / ratio + xlw;
    }
    def_world_win (PICT, xlw, xrw, ybw, ytw);
    pict_all (ERAS_DR);
    pic_max ();
    inform_window ();
}

void initwindow ()
{
    float  xlw, xrw, ybw, ytw, ratio;

    CLIP_FLAG = TRUE;

    ratio = p_wdw -> vymax / p_wdw -> vxmax;
    xlw = 0.0;
    ybw = 0.0;
    xrw = xlw + 45.0 * QUAD_LAMBDA;
    ytw = ratio * (xrw - xlw) + ybw;
    def_world_win (PICT, xlw, xrw, ybw, ytw);
    save_oldw ();
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
	if (XR == XL) fatal (0002, "set_c_wdw");
	c_wX = c_wdw -> wxmin;
	c_wY = c_wdw -> wymax;
	c_sX = c_wdw -> vxmax / (c_wdw -> wxmax - c_wX);
	c_sY = c_wdw -> vymax / (c_wY - c_wdw -> wymin);
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
    struct Disp_wdw *wdw;

    wdw = wdw_arr[w_nr];
    if (!wdw) fatal (0001, "def_world_win");
    wdw -> wxmin = Wxmin;
    wdw -> wymin = Wymin;
    wdw -> wxmax = Wxmax;
    wdw -> wymax = Wymax;
    if (c_wdw == wdw) {
	c_wdw = NULL; /* recalculate transformation */
	set_c_wdw (w_nr);
    }
}
