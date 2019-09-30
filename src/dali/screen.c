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

#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "X11/cursorfont.h"
#include "X11/keysym.h"
#include "src/dali/header.h"
#include <math.h>

// #define DEBUG_EVENTS	/* Print X11 Events */
// #define DEBUG_MOTION_EVENTS	/* Print MotionEvents */

// #define APOLLO_OLD	/* old Apollo driver */
			/* last two color entries are reserved */
#define N_MENU 12 /* max. number of chars in menu */
#define N_CMDS 20 /* max. number of cmds  in menu */

#define TRANSF_X(wx) (((wx) - XL) * c_sX)
#define TRANSF_Y(wy) ((YT - (wy)) * c_sY)
#define SCALE_DX(wx) ((wx) * c_sX)

extern int CacdCmapError (char *string, int fatal);
extern int CoupleIdToColor (int Id, char *color);
extern int CreateGCforCmap (Drawable d, GC *pgc);
extern int GetAffectedIds (int *Ids, int nids, int *AfIds, int *pnaffs);
extern int GetPixelFromId (int Id, unsigned long *pixel);
extern int GetPixelFromIds (int *Ids, int n, unsigned long *pixel);
extern int InitCacdCmap (Display *dpy, int s_nr);
extern int SetForegroundFromId (GC gc, int Id);
extern int SetForegroundFromIds (GC gc, int *Ids, int n);
extern int get_coordnts (void);
extern int upd_mod (void);

extern DM_PROJECT *dmproject;
extern char *argv0;
extern char *cellstr;
extern struct Disp_wdw *wdw_arr[];
extern struct Disp_wdw *c_wdw;
extern struct Disp_wdw *p_wdw;
extern float XL, XR, YB, YT;
extern int   Default_expansion_level;
extern int   exp_level;
extern int   maxDisplayStrLen;
extern int   cmd_nbr, in_cmd, new_cmd, grid_cmd;
extern int   dom_order_cmd;
extern int   nr_planes;
extern int   Backgr;
extern int   Cur_nr;
extern int   DRC_nr;
extern int   Gridnr;
extern int   Textnr;
extern int   VIS_mode;
extern int   Yellow;
extern int   NR_lay;
extern int   NR_all;
extern int  *dom_arr;
extern int   Draw_dominant;
extern int  *black_arr;
extern int   first_pict_buf;
extern int   zoom_mode;
extern int   ask_yes_no;

int allow_keypress = 1; /* YES */
// int allow_keypress = 0; /* NO */
int tracker_mode = 0; /* OFF */

static XColor c_fg = { 0, 65535, 65535, 65535 };
static XColor c_bg = { 0, 0, 0, 0 };

extern int   colormap;
extern int   d_apollo;
extern int  *fillshift;
extern int  *fillst;
extern char *DisplayName;
extern char *geometry;
extern Coor  piwl, piwr, piwb, piwt;

static char *CWD;
static char  HOST[100];
static XSizeHints hints;
static int   tracker_enabled = 0;
static int   trW; /* tracker window Width */
static Coor  txs = 0, tys = 0; /* tracker start position */
static Coor  txe = 0, tye = 0; /* tracker end position */
static Coor  txe_old = 0, tye_old = 0;
static char  trstr[50];
static int   trstrLen = 24;

Window cwin; /* current window */
Window win_arr[10]; /* window array */

static Display *dpy;
static Window rwin; /* root window */
static Window mwin; /* menu window */
static Window twin; /* text window */
static Window trwin;/* tracker window */
static Window pwin; /* pict window */
static Window lwin; /* layr window */
static Window ewin; /* event window */
static XEvent event;
static XFontStruct *font;
static GC pgc; /* picture/menu gc */
static GC tgc; /* text/tracker gc */
static GC cgc; /* cursor  gc */
static XGCValues gcv;
static Colormap cmap;
static unsigned long std_mask;
static int BW = 1; /* borderwidth */
static int DW, DH; /* DisplayWidth, Height */
static int maxW, maxH; /* maxWidth, maxHeight */
static int DMAX;   /* Display Max */
static int rW, rH; /* root Width, Height */
static int e_area_x, e_area_y;
int mW, mH; /* menu Width, Height */
int tW, tH; /* text Width, Height */
int cW, cH; /* char Width, Height */
static int cA, cD; /* char Ascent, Descent */
int pH; /* picture Height */
int Erase_hollow = 0;
float c_sX, c_sY;
static float cfixx, cfixy;
static int cxs, cys, cxe, cye;
static unsigned int crw, crh;
static int cecho, ccur;
static int ret_c_x, ret_c_y;
int RGBids[3];

static int Filld  = 8;      /* fill density */
static int Filld2 = 4;      /* half fill density */

static int c_dmode = -1;
static int c_ltype = -1;
static int c_style = -1;
static int maxindex;
static int ggcurid = -1;
static int ggidnr = 0; /* first color_id number */

static char bmap1_bits[] = { 0x02, 0x01};
static char bmap2_bits[] = { 0x01, 0x00};
static char bmap3_bits[] = { 0x01, 0x00, 0x00};
static char bmap4_bits[] = { 0x01, 0x00, 0x00, 0x00};
static char bmap5_bits[] = { 0x08, 0x04, 0x02, 0x01};
static char bmap6_bits[] = { 0x01, 0x0a, 0x04, 0x0a};
static char bmap7_bits[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
static char bmap8_bits[] = { 0x01, 0x82, 0x44, 0x28, 0x10, 0x28, 0x44, 0x82};

Pixmap stipple1 = 0;
Pixmap stipple2 = 0;
Pixmap stipple3 = 0;
Pixmap stipple4 = 0;
Pixmap stipple5 = 0;
Pixmap stipple6 = 0;
Pixmap stipple7 = 0;
Pixmap stipple8 = 0;

#define GGMAXCOLORS 32

static int ggmaxcolors = GGMAXCOLORS;
static int ggMaxIds;
static int *aIds;
static int *ggcindex;
static char **ggconame;

char *ggcolors[GGMAXCOLORS] = {
/*  0 */ "black",
/*  1 */ "red",
/*  2 */ "green",
/*  3 */ "yellow",
/*  4 */ "blue",
/*  5 */ "magenta",
/*  6 */ "cyan",
/*  7 */ "white",
/*  8 */ "khaki",
/*  9 */ "coral",
/* 10 */ "sienna",
/* 11 */ "firebrick",
/* 12 */ "orange",
/* 13 */ "violet red",
/* 14 */ "navy",
/* 15 */ "grey",
/* 16 */ "spring green",
/* 17 */ "aquamarine",
/* 18 */ "pink",
/* 19 */ "pale green",
/* 20 */ "sea green",
/* 21 */ "yellow green",
/* 22 */ "medium spring green",
/* 23 */ "lime green",
/* 24 */ "cadet blue",
/* 25 */ "dark orchid",
/* 26 */ "sky blue",
/* 27 */ "light grey",
/* 28 */ "steel blue",
/* 29 */ "orchid",
/* 30 */ "medium aquamarine",
/* 31 */ "dark turquoise",
};

static int  calc_dm (int x, int y);
static void DrawCursor (void);
static void DrawTracker (void);
static int  ggGetColorIndex (int color_id);
static int  ggGetColorsIndex (int col_ids[], int nr_ids);
static int  ggGetIdForColor  (char *color);
static void ggInit (int planes);
static void WaitForEvent (int etype);
static void Xerror (int errno, char *s);

static void zdcol (int ci, float R, float G, float B)
{
    XColor X11_col;

    if (ci < 0 || ci >= maxindex) return;

#ifdef APOLLO_OLD
    /* last two entries are reserved */
    if (ci >= maxindex - 2) return;
#endif /* APOLLO_OLD */
    ci <<= 1;
    if (ci >= maxindex) ci = ci - maxindex + 1;
    X11_col.pixel = (unsigned long) ci;
    X11_col.red   = (unsigned short) (R * 65535);
    X11_col.green = (unsigned short) (G * 65535);
    X11_col.blue  = (unsigned short) (B * 65535);
    X11_col.flags = (char) (DoRed | DoGreen | DoBlue);
    XStoreColor (dpy, cmap, &X11_col);
}

void paint_box (float x1, float x2, float y1, float y2)
{
    int dx1, dy1, dx2, dy2, dw, dh, dm, x, y;

    dx1 = (int) TRANSF_X (x1);
    dx2 = (int) TRANSF_X (x2);
    dy1 = (int) TRANSF_Y (y1);
    dy2 = (int) TRANSF_Y (y2);

    if (dx1 > dx2) { dw = dx1 - dx2; dx1 = dx2; }
    else dw = dx2 - dx1;
    if (dy1 > dy2) { dh = dy1 - dy2; dy1 = dy2; }
    else dh = dy2 - dy1;

    if (dw > DMAX) {
	if (dx1 < 0) {
	    dw += dx1 + 1; dx1 = -1;
	    if (dw > DMAX) dw = DMAX;
	    else if (dw <= 0) return;
	}
	else dw = DMAX;
    }
    if (dh > DMAX) {
	if (dy1 < 0) {
	    dh += dy1 + 1; dy1 = -1;
	    if (dh > DMAX) dh = DMAX;
	    else if (dh <= 0) return;
	}
	else dh = DMAX;
    }
/*if (cwin == pwin) PE "paint_box: x=%d, y=%d, w=%d, h=%d\n",dx1,dy1,dw,dh);*/

    if (c_style == FILL_SOLID) {
	if (Erase_hollow) {
	    if (dx1 + dw == e_area_x) ++dw;
	    if (dy1 + dh == e_area_y) ++dh;
	}
	if (!dh || !dw) return;
	XFillRectangle (dpy, cwin, pgc,
	    dx1, dy1, (unsigned int) dw, (unsigned int) dh);
    }
    else {
	if (c_style & FILL_HOLLOW) { /* draw outline */
	    if (!dw) {
		XDrawLine (dpy, cwin, pgc, dx1, dy1, dx1, dy1 + dh);
		return;
	    }
	    if (!dh) {
		XDrawLine (dpy, cwin, pgc, dx1, dy1, dx1 + dw, dy1);
		return;
	    }
	    XDrawRectangle (dpy, cwin, pgc,
		dx1, dy1, (unsigned int) dw, (unsigned int) dh);
	    if (c_style == FILL_HOLLOW || dh == 1 || dw == 1) return;
	}
	if (c_style & FILL_CROSS) { /* draw cross */
	    XDrawLine (dpy, cwin, pgc, dx1, dy1, dx1 + dw, dy1 + dh);
	    if (dw && dh)
		XDrawLine (dpy, cwin, pgc, dx1 + dw, dy1, dx1, dy1 + dh);
	    return;
	}
	if (c_style >= FILL_STIPPLE1) {
	    XSetFillStyle (dpy, pgc, FillStippled);
	    XFillRectangle (dpy, cwin, pgc,
		dx1, dy1, (unsigned int) dw, (unsigned int) dh);
	    XSetFillStyle (dpy, pgc, FillSolid);
	    return;
	}
	/* FILL_HASHED */
	if (dw) --dw;
	if (dh) --dh;
	dx2 = dx1 + dw;
	dy2 = dy1 + dh;
	dm = calc_dm (dx1, dy2);
	x = dx1 + dm;
	y = dy2 - dm;
	while (y > dy1) {
	    if (x > dx2) {
		x = dx2;
		dy2 = y + dw;
	    }
	    XDrawLine (dpy, cwin, pgc, dx1, y, x, dy2);
	    x += Filld; y -= Filld;
	}
	x = dx1 + (dy1 - y) + dh;
	while (x <= dx2) {
	    XDrawLine (dpy, cwin, pgc, x - dh, dy1, x, dy2);
	    x += Filld;
	}
	x = x - dh;
	y = dy1 + (dx2 - x);
	while (x <= dx2) {
	    XDrawLine (dpy, cwin, pgc, x, dy1, dx2, y);
	    x += Filld; y -= Filld;
	}
    }
}

void pict_rect (float x1, float x2, float y1, float y2)
{
    int dx1, dx2, dy1, dy2, dw, dh;

    dx1 = (int) TRANSF_X (x1);
    dx2 = (int) TRANSF_X (x2);
    dy1 = (int) TRANSF_Y (y1);
    dy2 = (int) TRANSF_Y (y2);

    if (dx1 > dx2) { dw = dx1 - dx2; dx1 = dx2; }
    else dw = dx2 - dx1;
    if (dy1 > dy2) { dh = dy1 - dy2; dy1 = dy2; }
    else dh = dy2 - dy1;

    if (dw > DMAX) {
	if (dx1 < 0) {
	    dw += dx1 + 1; dx1 = -1;
	    if (dw > DMAX) dw = DMAX;
	    else if (dw <= 0) return;
	}
	else dw = DMAX;
    }
    if (dh > DMAX) {
	if (dy1 < 0) {
	    dh += dy1 + 1; dy1 = -1;
	    if (dh > DMAX) dh = DMAX;
	    else if (dh <= 0) return;
	}
	else dh = DMAX;
    }
/*if (cwin == pwin) PE "pict_rect(): x=%d, y=%d, w=%d, h=%d\n",dx1,dy1,dw,dh);*/

    if (!dw)
	XDrawLine (dpy, cwin, pgc, dx1, dy1, dx1, dy1 + dh);
    else if (!dh)
	XDrawLine (dpy, cwin, pgc, dx1, dy1, dx1 + dw, dy1);
    else
	XDrawRectangle (dpy, cwin, pgc, dx1, dy1, (unsigned int) dw, (unsigned int) dh);
}

/*
** Draw the given polygon.
*/
void pict_poly (Coor coors[], int nr_c)
{
    XPoint p1[16];
    register int i;

    if (nr_c > 15) return;

    for (i = 0; i < nr_c; ++i) {
	p1[i].x = TRANSF_X (coors[2 * i]);
	p1[i].y = TRANSF_Y (coors[(2 * i) + 1]);
    }
    p1[nr_c].x = p1[0].x;
    p1[nr_c].y = p1[0].y;
    XDrawLines (dpy, cwin, pgc, p1, nr_c + 1, CoordModeOrigin);
}

void draw_trap (Coor line_x[], Coor line_y[])
{
    XPoint point[5];
    int x, y, xs, ys, dm, dh, r90;
    int dx0, dx1, dx2, dx3, dy0, dy1, dy2, dy3;
    register int i, n, np;

    if (line_y[3] == line_y[0] && line_y[2] == line_y[1]) r90 = 0;
    else r90 = 1;

    /* Is last point equal to first point? */
    if (line_x[3] == line_x[0] && line_y[3] == line_y[0]) n = 3;
    else n = 4;

    point[0].x = TRANSF_X (line_x[0]);
    point[0].y = TRANSF_Y (line_y[0]);
    for (np = i = 1; i < n; ++i) {
	if (line_x[i] == line_x[i-1] &&
	    line_y[i] == line_y[i-1]) continue;
	point[np].x = TRANSF_X (line_x[i]);
	point[np].y = TRANSF_Y (line_y[i]);
	++np;
    }

    if (c_style == FILL_SOLID) {
	XFillPolygon (dpy, cwin, pgc, point, np, Convex, CoordModeOrigin);
    }
    else {
	dx0 = dx1 = dx2 = dy0 = dy1 = dy2 = 0; // init, to suppress compiler warning
	if (c_style & FILL_HOLLOW) { /* draw outline */
	    point[np].x = point[0].x;
	    point[np].y = point[0].y;
	    XDrawLines (dpy, cwin, pgc, point, np + 1, CoordModeOrigin);
	    if (c_style == FILL_HOLLOW /*|| dh == 1 || dw == 1*/) return;
	}
	if (c_style & FILL_CROSS) { /* draw cross */
	    if (np == 4) {
		XDrawLine (dpy, cwin, pgc, point[0].x, point[0].y, point[2].x, point[2].y);
		XDrawLine (dpy, cwin, pgc, point[1].x, point[1].y, point[3].x, point[3].y);
	    }
	    else if (np == 3) {
		for (i = 0; i < 3; ++i) {
		    n = i < 2 ? i + 1 : 0;
		    if (point[i].x == point[n].x) { /* vert */
			dy0 = point[i].y;
			dy1 = point[n].y;
			if (dy0 > dy1) { y = dy0; dy0 = dy1; dy1 = y; }
			dx2 = point[n].x;
		    }
		    else if (point[i].y == point[n].y) { /* hor */
			dx0 = point[i].x;
			dx1 = point[n].x;
			if (dx0 > dx1) { x = dx0; dx0 = dx1; dx1 = x; }
			dy2 = point[n].y;
		    }
		}
		if (dx2 == dx1)
		    dx0 = dx1 - (dx1 - dx0) / 2;
		else
		    dx1 = dx0 + (dx1 - dx0) / 2;
		if (dy2 == dy1)
		    dy0 = dy1 - (dy1 - dy0) / 2;
		else
		    dy1 = dy0 + (dy1 - dy0) / 2;
		XDrawLine (dpy, cwin, pgc, dx0, dy0, dx1, dy1);
		XDrawLine (dpy, cwin, pgc, dx0, dy1, dx1, dy0);
	    }
	    return;
	}
	if (c_style >= FILL_STIPPLE1) {
	    XSetFillStyle (dpy, pgc, FillStippled);
	    XFillPolygon (dpy, cwin, pgc, point, np, Convex, CoordModeOrigin);
	    XSetFillStyle (dpy, pgc, FillSolid);
	    return;
	}
	/* FILL_HASHED */
	dx0 = point[0].x;
	dy0 = point[0].y;
	dx1 = point[1].x;
	dy1 = point[1].y;
	if (r90) {
	    dh = dx0 - dx1;
	    if (dy0 == dy1) { /* S1 */
		if (np == 4) {
		    dy2 = point[2].y;
		    dy3 = point[3].y;
		}
		else {
		    if (point[2].x < dx0) {
			dy2 = point[2].y;
			dy3 = dy0;
		    }
		    else {
			dy2 = dy1;
			dy3 = point[2].y;
		    }
		}
		dm = calc_dm (dx1, dy1);
		x  = dx1 + dm;
		ys = dy1 - dm;
		y = dy0;
		while (ys >= dy2) {
		    if (x > dx0) { x = dx0; y = ys + dh; }
		    XDrawLine (dpy, cwin, pgc, dx1, ys, x, y);
		    x += Filld; ys -= Filld;
		}
		if (dy3 == dy2) { /* box */
		    xs = dx1 + (dy2 - ys);
		    while (x <= dx0) {
			XDrawLine (dpy, cwin, pgc, xs, dy2, x, dy0);
			xs += Filld; x += Filld;
		    }
		    y = dy0 - (x - dx0);
		    while (xs < dx0) {
			XDrawLine (dpy, cwin, pgc, xs, dy2, dx0, y);
			xs += Filld; y -= Filld;
		    }
		}
		else {
		    if (x > dx0) {
			y = ys + dh;
			ys = dy2 - (dy2 - ys) / 2;
			xs = dx0 - (y - ys);
		    }
		    else {
			ys = dy2 - (dy2 - ys) / 2;
			xs = x - (y - ys);
			while (x <= dx0) {
			    XDrawLine (dpy, cwin, pgc, xs, ys, x, dy0);
			    xs += Filld2; ys -= Filld2; x += Filld;
			}
			y = ys + (dx0 - xs);
		    }
		    while (y >= dy3) {
			XDrawLine (dpy, cwin, pgc, xs, ys, dx0, y);
			xs += Filld2; ys -= Filld2; y -= Filld;
		    }
		}
	    }
	    else if (dy1 > dy0) { /* S2 */
		dy2 = point[2].y;
		dy3 = (np == 4) ? point[3].y : dy0;
		dm = calc_dm (dx1, dy1);
		y  = dy1 - dm;
		ys = (dy1 - y) / 2;
		xs = dx1 + ys;
		ys = y + ys;
		while (y >= dy2) {
		    if (xs > dx0) { xs = dx0; ys = y + dh; }
		    XDrawLine (dpy, cwin, pgc, dx1, y, xs, ys);
		    y -= Filld; xs += Filld2; ys -= Filld2;
		}
		if (dy2 == dy3) {
		    x = dx1 + (dy2 - y);
		    while (xs <= dx0) {
			XDrawLine (dpy, cwin, pgc, x, dy2, xs, ys);
			x += Filld; xs += Filld2; ys -= Filld2;
		    }
		    ys = dy2 + (dx0 - x);
		    while (ys >= dy2) {
			XDrawLine (dpy, cwin, pgc, x, dy2, dx0, ys);
			x += Filld; xs += Filld; ys -= Filld;
		    }
		}
		else if (dy2 > dy3) {
		    if (xs > dx0) {
			ys = y + dh;
			y = dy2 - (dy2 - y) / 2;
			x = dx0 - (ys - y);
		    }
		    else {
			y = dy2 - (dy2 - y) / 2;
			x = xs - (ys - y);
			while (xs <= dx0) {
			    XDrawLine (dpy, cwin, pgc, x, y, xs, ys);
			    x += Filld2; y -= Filld2; xs += Filld2; ys -= Filld2;
			}
			ys = y + (dx0 - x);
		    }
		    while (ys >= dy3) {
			XDrawLine (dpy, cwin, pgc, x, y, dx0, ys);
			x += Filld2; y -= Filld2; ys -= Filld;
		    }
		}
	    }
	    else { /* dy1 < dy0 */ /* S3 */
		dm = calc_dm (dx0, dy0);
		if (np == 4) {
		    dy2 = point[2].y;
		    dy3 = point[3].y;
		}
		else {
		    dy2 = dy1;
		    dy3 = point[2].y;
		}
		y = dy0 - dm;
		ys = y - dh;
		while (ys >= dy2) {
		    XDrawLine (dpy, cwin, pgc, dx1, ys, dx0, y);
		    ys -= Filld; y -= Filld;
		}
		if (dy2 == dy3) {
		    x = dx0 - (y - dy3);
		    while (y >= dy3) {
			XDrawLine (dpy, cwin, pgc, x, dy3, dx0, y);
			x += Filld; y -= Filld;
		    }
		}
		else {
		    ys = dy2 - (dy2 - ys) / 2;
		    xs = dx0 - (y - ys);
		    while (y >= dy3) {
			XDrawLine (dpy, cwin, pgc, xs, ys, dx0, y);
			xs += Filld2; ys -= Filld2; y -= Filld;
		    }
		}
	    }
	}
	else { /* R0 */
	    dh = dy0 - dy1;
	    if (dx0 == dx1) { /* S1 */
		if (np == 4) {
		    if (point[2].x < point[3].x) {
			dx2 = point[2].x;
			dx3 = point[3].x;
		    }
		    else {
			dx2 = point[3].x;
			dx3 = point[2].x;
		    }
		}
		else {
		    dx2 = dx1;
		    dx3 = point[2].x;
		}
		dm = calc_dm (dx1, dy0);
		x = dx1 + dm;
		y = dy0 - dm;
		if (point[np - 2].x < point[np - 1].x
		    && point[np - 2].y < point[np - 1].y) {
		    while (x <= dx3) {
			if (y < dy1) { y = dy1; dx1 = x - dh; }
			XDrawLine (dpy, cwin, pgc, dx1, y, x, dy0);
			x += Filld; y -= Filld;
		    }
		}
		else {
		    while (x <= dx2) {
			if (y < dy1) { y = dy1; dx0 = x - dh; }
			XDrawLine (dpy, cwin, pgc, dx0, y, x, dy0);
			x += Filld; y -= Filld;
		    }
		    if (dx3 == dx2) { /* box */
			ys = dy0 - (x - dx2);
			while (y >= dy1) {
			    XDrawLine (dpy, cwin, pgc, dx1, y, dx2, ys);
			    y -= Filld; ys -= Filld;
			}
			x = dx2 - (ys - dy1);
			while (ys > dy1) {
			    XDrawLine (dpy, cwin, pgc, x, dy1, dx2, ys);
			    x += Filld; ys -= Filld;
			}
		    }
		    else {
			ys = dy0 - (x - dx2) / 2;
			if (y >= dy1) {
			    xs = dx1 + (ys - y);
			    while (y >= dy1) {
				XDrawLine (dpy, cwin, pgc, dx1, y, xs, ys);
				y -= Filld; xs += Filld2; ys -= Filld2;
			    }
			    x = xs - (ys - dy1);
			}
			else {
			    x -= dh;
			    xs = x + (ys - dy1);
			}
			while (x <= dx3) {
			    XDrawLine (dpy, cwin, pgc, x, dy1, xs, ys);
			    x += Filld; xs += Filld2; ys -= Filld2;
			}
		    }
		}
	    }
	    else if (dx0 > dx1) { /* S2 */
		dx0 = point[1].x;
		dx1 = point[0].x;
		dm = calc_dm (dx1, dy0);
		x = dx1 + dm;
		if (np == 4) {
		    dx2 = point[2].x;
		    dx3 = point[3].x;
		}
		else {
		    dx2 = point[2].x;
		    dx3 = dx1;
		}
		if (dx2 == dx3) {
		    while (x <= dx2) {
			XDrawLine (dpy, cwin, pgc, x - dh, dy1, x, dy0);
			x += Filld;
		    }
		    y = dy0 - (x - dx2);
		    x -= dh;
		    while (x <= dx2) {
			XDrawLine (dpy, cwin, pgc, x, dy1, dx2, y);
			x += Filld; y -= Filld;
		    }
		}
		else if (dx2 > dx3) {
		    while (x <= dx3) {
			XDrawLine (dpy, cwin, pgc, x - dh, dy1, x, dy0);
			x += Filld;
		    }
		    ys = dy0 - (x - dx3) / 2;
		    x -= dh;
		    xs = x + (ys - dy1);
		    while (x <= dx2) {
			XDrawLine (dpy, cwin, pgc, x, dy1, xs, ys);
			x += Filld; xs += Filld2; ys -= Filld2;
		    }
		}
		else { /* dx2 < dx3 */
		    while (x <= dx3) {
			XDrawLine (dpy, cwin, pgc, x - dh, dy1, x, dy0);
			x += Filld;
		    }
		}
	    }
	    else { /* dx0 < dx1 */ /* S3 */
		dm = calc_dm (dx0, dy0);
		x = dx0 + dm;
		ys = dy0 - (x - dx0) / 2;
		xs = x - (dy0 - ys);
		if (np == 4) {
		    dx2 = point[2].x;
		    dx3 = point[3].x;
		}
		else {
		    dx2 = point[1].x;
		    dx3 = point[2].x;
		}
		while (x <= dx3) {
		    if (ys < dy1) { ys = dy1; xs = x - dh; }
		    XDrawLine (dpy, cwin, pgc, xs, ys, x, dy0);
		    xs += Filld2; ys -= Filld2; x += Filld;
		}
		if (dx2 == dx3) {
		    y = dy0 - (x - dx3);
		    while (ys >= dy1) {
			XDrawLine (dpy, cwin, pgc, xs, ys, dx3, y);
			xs += Filld2; ys -= Filld2; y -= Filld;
		    }
		    xs = dx2 - (y - dy1);
		    while (y > dy1) {
			XDrawLine (dpy, cwin, pgc, xs, dy1, dx3, y);
			xs += Filld; y -= Filld;
		    }
		}
		else if (dx2 > dx3) {
		    y = dy0 - (x - dx3) / 2;
		    if (ys >= dy1) {
			x = xs + (y - ys);
			while (ys >= dy1) {
			    XDrawLine (dpy, cwin, pgc, xs, ys, x, y);
			    xs += Filld2; ys -= Filld2; x += Filld2; y -= Filld2;
			}
			xs = x - (y - dy1);
		    }
		    else {
			xs = x - dh;
			x = xs + (y - dy1);
		    }
		    while (xs <= dx2) {
			XDrawLine (dpy, cwin, pgc, xs, dy1, x, y);
			xs += Filld; x += Filld2; y -= Filld2;
		    }
		}
	    }
	}
    }
}

static int calc_dm (int x, int y)
{
    int dm = y % Filld - x % Filld;
    /*
    ** Shift fill pattern? (to make layer visible through other)
    */
    if (ggcurid >= 0 && ggcurid < NR_lay) dm += fillshift[ggcurid];
    if (dm < 0) dm = Filld - (-dm) % Filld;
    return (dm % Filld);
}

void get_gterm ()
{
    if (!DisplayName) {
	DisplayName = getenv ("DISPLAY");
	if (!DisplayName) {
	    PE "%s: no graphics display specified\n", argv0);
	    Xerror (0, "");
	}
    }
}

/*
** Setting the colors:
** Color 32 (bitplane 6) will be used for the instances and text.
** Color 64 (bitplane 7) will be used for making a menu and grid.
** Color 128 (bitplane 8) will be used by the cursor.
*/
static void StoreColors ()
{
    register int color;
    register int i;

    color = 0;
    zdcol (color++, 0.000, 0.000, 0.000); /* 0 = black */
    zdcol (color++, 1.000, 0.000, 0.000); /* 1 = red */
    zdcol (color++, 0.000, 1.000, 0.000); /* 2 = green */
    if (nr_planes == 2) goto set_whites;
    zdcol (color++, 1.000, 1.000, 0.000); /* 3 = yellow */
    zdcol (color++, 0.000, 0.000, 1.000); /* 4 = blue */
    zdcol (color++, 1.000, 0.000, 1.000); /* 5 = magenta */
    zdcol (color++, 0.000, 1.000, 1.000); /* 6 = cyan */
    if (nr_planes == 3) goto set_whites;
    zdcol (color++, 1.000, 1.000, 1.000); /* 7 = white */

    if (nr_planes == 4) goto set_whites;
    zdcol (color++, 0.940, 0.627, 0.000); /* 8 = gold */
    zdcol (color++, 0.940, 0.627, 0.000); /* gold */
    zdcol (color++, 0.500, 0.500, 0.000);
    zdcol (color++, 0.700, 0.700, 0.400);
    zdcol (color++, 0.933, 0.333, 0.000); /* toasted orange */
    zdcol (color++, 0.700, 0.400, 0.600);
    zdcol (color++, 0.700, 0.500, 0.500);
    zdcol (color++, 0.700, 0.700, 0.400); /* 15 */

    if (nr_planes == 5) goto set_whites;
    zdcol (color++, 0.000, 0.940, 0.627); /* 16 = aqua */
    zdcol (color++, 0.000, 0.940, 0.627); /* aqua */
    zdcol (color++, 0.000, 0.300, 0.627);
    zdcol (color++, 0.000, 0.940, 0.627); /* aqua */
    zdcol (color++, 0.000, 0.940, 0.627); /* aqua */
    zdcol (color++, 0.600, 0.700, 0.300);
    zdcol (color++, 0.313, 0.940, 0.000);
    zdcol (color++, 0.000, 0.940, 0.627); /* aqua */
    zdcol (color++, 0.000, 0.533, 0.600); /* turquoise */
    zdcol (color++, 0.600, 0.133, 1.000); /* violet */
    zdcol (color++, 0.000, 0.533, 0.600); /* turquoise */
    zdcol (color++, 0.600, 0.600, 0.600);
    zdcol (color++, 0.200, 0.533, 0.600);
    zdcol (color++, 0.700, 0.400, 0.700);
    zdcol (color++, 0.000, 0.700, 0.500);
    zdcol (color++, 0.300, 0.533, 0.600); /* 31 */

set_whites:
    for (i = color; i < maxindex; ++i)
	zdcol (i, 1.000, 1.000, 1.000); /* white */

    for (i = color + 2; i < maxindex; i += 32) {
	zdcol (i, 0.000, 0.000, 0.000);   /* green -> black */
	zdcol (i+1, 0.000, 0.000, 0.000); /* yellow-> black */
	zdcol (i+4, 0.000, 0.000, 0.000); /* cyan  -> black */
	zdcol (i+5, 0.000, 0.000, 0.000); /* white -> black */
    }
}

void set_color_entry (int nbr, char *color)
{
    ggcolors[nbr] = strsave (color, 0);
}

static Cursor pcur; /* pict cursor */
static Cursor mcur; /* menu cursor */
static Cursor tcur; /* text cursor */

void set_text_cursor (int text_mode)
{
    if (text_mode) {
	XDefineCursor (dpy, pwin,  tcur);
	XDefineCursor (dpy, mwin,  tcur);
	XDefineCursor (dpy, lwin,  tcur);
	XDefineCursor (dpy, twin,  tcur);
	XDefineCursor (dpy, trwin, tcur);
    }
    else {
	XDefineCursor (dpy, pwin,  pcur);
	XDefineCursor (dpy, mwin,  mcur);
	XDefineCursor (dpy, lwin,  mcur);
	XDefineCursor (dpy, twin,  pcur);
	XDefineCursor (dpy, trwin, pcur);
    }
}

void init_graph ()
{
    char *fontname;
    char *cp;
    XWMHints wmhints;
    unsigned long Black, White;
    int      s_nr; /* screen number */
    register int i;
    char B_str[200]; /* title bar string */
    char I_str[200]; /* icon text string */

    if (!(dpy = XOpenDisplay (DisplayName))) Xerror (1, DisplayName);

    if ((cp = XGetDefault (dpy, argv0, "BorderWidth"))) BW = atoi (cp);
    if (BW <= 0) BW = 1;

    if (!(fontname = XGetDefault (dpy, argv0, "FontName")))
	fontname = "fixed";

    if (!geometry) geometry = XGetDefault (dpy, argv0, "GeoMetry");
    s_nr = DefaultScreen (dpy);

    pcur = XCreateFontCursor (dpy, XC_dotbox);
    mcur = XCreateFontCursor (dpy, XC_hand2);
    tcur = XCreateFontCursor (dpy, XC_xterm);

    if (!colormap && BlackPixel (dpy, s_nr)) { /* inverse fg cursor */
	XRecolorCursor (dpy, pcur, &c_bg, &c_fg);
	XRecolorCursor (dpy, mcur, &c_bg, &c_fg);
	XRecolorCursor (dpy, tcur, &c_bg, &c_fg);
    }
    else {
	XRecolorCursor (dpy, pcur, &c_fg, &c_bg);
	XRecolorCursor (dpy, mcur, &c_fg, &c_bg);
	XRecolorCursor (dpy, tcur, &c_fg, &c_bg);
    }

    if (!(font = XLoadQueryFont (dpy, fontname))) Xerror (2, fontname);
    cW = font -> max_bounds.width;
    cA = font -> ascent;
    cD = font -> descent;
    cH = cA + cD;
    tH = cH + 2 * cD;

    DW = DisplayWidth  (dpy, s_nr);
    DH = DisplayHeight (dpy, s_nr);
    DMAX = DW > DH ? DW : DH;
    maxW = DW - 2 * BW;
    maxH = DH - 2 * BW;

    ggInit (4);

    ggMaxIds = dmproject -> maskdata -> nomasks + 8;
    MALLOCN (ggcindex, int, ggMaxIds);
    MALLOCN (ggconame, char*, ggMaxIds);
    MALLOCN (aIds, int, ggMaxIds);

    init_mskcol ();

    Textnr = ggGetIdForColor ("white");
    Gridnr = ggGetIdForColor ("white");
    DRC_nr = ggGetIdForColor ("white");
    Cur_nr = DRC_nr;

    /* NR_all is set in init_mskcol() */
    if (NR_all != DRC_nr + 1) Xerror (5, "");

    Backgr = ggGetIdForColor ("black");
    Yellow = ggGetIdForColor ("yellow");
    RGBids[0] = ggGetIdForColor ("red");
    RGBids[1] = ggGetIdForColor ("green");
    RGBids[2] = ggGetIdForColor ("blue");
    Black = ggGetColorIndex (Backgr);
    White = ggGetColorIndex (Gridnr);

    mW = N_MENU * cW + 2 * cD;
    tW = 7 * mW;
    if (tW > (i = maxW - 2*mW - 2*BW)) tW = i; /* reduce tW */
    trW = trstrLen * cW + 2 * cD;
    if (trW > (i = 2 * tW / 3)) trW = i; /* reduce trW */

    if ((i = 2 * trW) > tW) i = tW;
    hints.min_width  = 2*mW + 2*BW + i;
    hints.min_height = mH = N_CMDS * tH;
    i = (float) (mW + tW) * (float) DH / (float) DW;
    if (mH < i) mH = i;
    if (mH > maxH) mH = maxH; /* reduce mH */

    hints.flags = PPosition | PSize | PMinSize;
    hints.x = hints.y = 0;
    hints.width  = 2*mW + 2*BW + tW;
    hints.height = mH;
    if (hints.min_width  > maxW) hints.min_width = maxW;
    if (hints.min_height > maxH) hints.min_height= maxH;

    if (geometry) {
	int rv = XParseGeometry (geometry,
	    &hints.x, &hints.y, (unsigned*)&hints.width, (unsigned*)&hints.height);
	if (rv & (WidthValue | HeightValue)) {
	    hints.flags |= USSize;
	    if (hints.width < hints.min_width)   hints.width = hints.min_width;
	    if (hints.height < hints.min_height) hints.height= hints.min_height;
	}
	if (rv & (XValue | YValue)) {
	    hints.flags |= USPosition;
	    if (rv & XNegative) hints.x += maxW - hints.width;
	    if (rv & YNegative) hints.y += maxH - hints.height;
	}
	if (hints.x < 0) hints.x = 0;
	if (hints.y < 0) hints.y = 0;
    }

    rwin = XCreateSimpleWindow (dpy, RootWindow (dpy, s_nr),
	    hints.x, hints.y, hints.width, hints.height, BW, White, Black);
    if (!rwin) Xerror (3, DisplayName);
    /*
     * Make a nice title in bar:
     */
    if (gethostname (HOST, (size_t) 100) == -1) strcpy (HOST, "???");
    /* remove domain extension from hostname */
    for (cp = HOST; *cp && *cp != '.'; ++cp); *cp = 0;
    CWD = dmproject -> dmpath;
    sprintf (B_str, "Welcome to %s in project %s on %s", argv0, CWD, HOST);
    sprintf (I_str, "%s  %s on %s", argv0, CWD, HOST);
    XSetStandardProperties (dpy, rwin, B_str, I_str, None, NULL, 0, &hints);
/** XSetStandardProperties (dpy, rwin, argv0, argv0, None, NULL, 0, &hints); **/

    /* strip leading path from cwd name (for later usage) */
    if ((cp = strrchr (CWD, '/'))) CWD = ++cp;

    wmhints.input = TRUE;
    wmhints.flags = InputHint;
    XSetWMHints (dpy, rwin, &wmhints);

    mwin = XCreateSimpleWindow (dpy, rwin, -BW, -BW, 1, 1, BW, White, Black);
    pwin = XCreateSimpleWindow (dpy, rwin,  mW,  tH, 1, 1, BW, White, Black);
    i = mW + tW + BW; /* new layer window x-position */
    lwin = XCreateSimpleWindow (dpy, rwin,   i, -BW, 1, 1, BW, White, Black);
    twin = XCreateSimpleWindow (dpy, rwin,  mW, -BW, 1, 1, BW, White, Black);
    i = mW + tW - trW; /* tracker window x-position */
    trwin = XCreateSimpleWindow (dpy, rwin,  i, -BW, 1, 1, BW, White, Black);

    if (!mwin || !pwin || !lwin || !twin || !trwin) Xerror (3, DisplayName);
    XStoreName (dpy, pwin, "daliscreen");

    XSelectInput (dpy, rwin, StructureNotifyMask);
    std_mask = ExposureMask;
    XSelectInput (dpy, pwin, std_mask);
    XSelectInput (dpy, mwin, std_mask);
    XSelectInput (dpy, lwin, std_mask);
    XSelectInput (dpy, twin, std_mask);
    XSelectInput (dpy, trwin, std_mask);

    set_text_cursor (0);

    if (!colormap && nr_planes > 1) {
	cmap = XCreateColormap (dpy, rwin, DefaultVisual (dpy, s_nr), AllocAll);
	XSetWindowColormap (dpy, rwin, cmap);
	StoreColors ();
    }

    if (!colormap) pgc = XCreateGC (dpy, rwin, 0, 0);
    else if (!CreateGCforCmap (rwin, &pgc)) CacdCmapError (argv0, 1);
    XSetFont (dpy, pgc, font -> fid);

    stipple1 = XCreateBitmapFromData (dpy, rwin, bmap1_bits, 2, 2);
    stipple2 = XCreateBitmapFromData (dpy, rwin, bmap2_bits, 2, 2);
    stipple3 = XCreateBitmapFromData (dpy, rwin, bmap3_bits, 3, 3);
    stipple4 = XCreateBitmapFromData (dpy, rwin, bmap4_bits, 4, 4);
    stipple5 = XCreateBitmapFromData (dpy, rwin, bmap5_bits, 4, 4);
    stipple6 = XCreateBitmapFromData (dpy, rwin, bmap6_bits, 4, 4);
    stipple7 = XCreateBitmapFromData (dpy, rwin, bmap7_bits, 8, 8);
    stipple8 = XCreateBitmapFromData (dpy, rwin, bmap8_bits, 8, 8);

    if (!colormap) tgc = XCreateGC (dpy, rwin, 0, 0);
    else if (!CreateGCforCmap (rwin, &tgc)) CacdCmapError (argv0, 1);

    XSetBackground (dpy, tgc, Black);
    XSetForeground (dpy, tgc, White);
    XSetFont (dpy, tgc, font -> fid);
    if (d_apollo) {
	if (White > 0)
	    XSetPlaneMask (dpy, tgc, White);
	else
	    XSetPlaneMask (dpy, tgc, (unsigned long) (maxindex - 1));
    }

    if (colormap) {
	if (!CreateGCforCmap (rwin, &cgc)) CacdCmapError (argv0, 1);
	SetForegroundFromIds (cgc, RGBids, 3);
    }
    else {
	cgc = XCreateGC (dpy, rwin, 0, 0);
	White = ggGetColorsIndex (RGBids, 3);
	XSetForeground (dpy, cgc, White);
    }
    XSetFunction (dpy, cgc, GXxor);

    win_arr[MENU] = mwin;
    win_arr[PICT] = pwin;
    win_arr[TEXT] = twin;
    win_arr[LAYS] = lwin;
    Def_window (MENU);
    Def_window (PICT);
    Def_window (TEXT);
    Def_window (LAYS);

    XMapWindow (dpy, rwin);
    /*
    ** The above XMapWindow will generate
    ** a ConfigureNotify event telling us the actual size
    ** of the window when it is mapped.
    ** We wait for this event before mapping the subwindows.
    */
#ifdef DEBUG_EVENTS
    PE "-- WaitForEvent ConfigureNotify\n");
#endif
    WaitForEvent (ConfigureNotify);

    XMapWindow (dpy, pwin);
    XMapWindow (dpy, mwin);
    XMapWindow (dpy, lwin);
    XMapWindow (dpy, twin);
#ifdef DEBUG_EVENTS
    PE "-- WaitForEvent Expose\n");
#endif
    WaitForEvent (Expose);

    disp_mode (TRANSPARENT);
    ggSetColor (Gridnr);
    d_fillst (FILL_SOLID);
    d_ltype (LINE_SOLID);
}

void set_new_bitmap (int nr, int w, int h, char bm[])
{
    Pixmap old, new;
    int lay;

    if (nr < 1 || nr > 8) return;
    new = XCreateBitmapFromData (dpy, rwin, bm, (unsigned)w, (unsigned)h);
    if (!new) return;
    old = 0;
    switch (nr) {
    case 1: old = stipple1; stipple1 = new;
	nr = FILL_STIPPLE1; break;
    case 2: old = stipple2; stipple2 = new;
	nr = FILL_STIPPLE1+1; break;
    case 3: old = stipple3; stipple3 = new;
	nr = FILL_STIPPLE1+2; break;
    case 4: old = stipple4; stipple4 = new;
	nr = FILL_STIPPLE1+3; break;
    case 5: old = stipple5; stipple5 = new;
	nr = FILL_STIPPLE5; break;
    case 6: old = stipple6; stipple6 = new;
	nr = FILL_STIPPLE5+1; break;
    case 7: old = stipple7; stipple7 = new;
	nr = FILL_STIPPLE5+2; break;
    case 8: old = stipple8; stipple8 = new;
	nr = FILL_STIPPLE5+3; break;
    }
    if (old) XFreePixmap (dpy, old);
    /* Is old one in use by some layer? */
    for (lay = 0; lay < NR_lay; ++lay) {
	if ((fillst[lay] & (0xff - FILL_HOLLOW)) == nr) rebulb (lay);
    }
}

/*
 * Set title bar string of window.
 */
void set_titlebar (char *ownstring)
{
    char B_str[256], I_str[200], *cp;

    if (!ownstring || !*ownstring) {
	if (!(cp = cellstr) || !*cp) {
	    cp = "---";
	    sprintf (I_str, "%s  %s on %s", argv0, CWD, HOST);
	}
	else
	    sprintf (I_str, "%s  %s in %s on %s", argv0, cp, CWD, HOST);
	sprintf (B_str, ">> %s >> Cell: %s  Exp_level: %d  Project: %s on %s", argv0, cp, exp_level, CWD, HOST);
    }
    else {
	sprintf (I_str, "%s  %s", argv0, ownstring);
	sprintf (B_str, ">> %s >> %s", argv0, ownstring);
    }

    XSetStandardProperties (dpy, rwin, B_str, I_str, None, NULL, 0, &hints);
}

int get_disp_mode ()
{
    return (c_dmode);
}

void disp_mode (int dmode)
{
    if (dmode == c_dmode) return;

    switch (dmode) {
    case DOMINANT:
	XSetFunction (dpy, pgc, GXcopy);
	break;
    case TRANSPARENT:
	XSetFunction (dpy, pgc, GXor);
	break;
    case ERASE:
	XSetFunction (dpy, pgc, GXandInverted);
	break;
    case COMPLEMENT:
	XSetFunction (dpy, pgc, GXxor);
	break;
    default:
	dmode = c_dmode;
    }
    c_dmode = dmode;
}

void d_text (float x, float y, char *str, int strLen)
{
    int dx1, dy1;

    if (cwin == twin) {
	XDrawString (dpy, twin, tgc, cD, cH, str, strLen);
    }
    else {
	dx1 = TRANSF_X (x);
	dy1 = TRANSF_Y (y);
	if (cwin == pwin) { /* picture window */
	    dx1 += cD; dy1 -= cD;
	    if (strLen > maxDisplayStrLen) maxDisplayStrLen = strLen;
	}
	else if (cwin == mwin) dy1 -= cD;
	XDrawString (dpy, cwin, pgc, dx1, dy1, str, strLen);
    }
}

void d_fillst (int fs) /* set fill style */
{
    if (fs == c_style) return;

    if (fs >= FILL_STIPPLE1) {
	Pixmap stipple = 0;
	c_style = fs;
	if (fs & FILL_HOLLOW) fs -= FILL_HOLLOW;
	switch (fs) {
	case FILL_STIPPLE1: stipple = stipple1; break;
	case FILL_STIPPLE1+1: stipple = stipple2; break;
	case FILL_STIPPLE1+2: stipple = stipple3; break;
	case FILL_STIPPLE1+3: stipple = stipple4; break;
	case FILL_STIPPLE5: stipple = stipple5; break;
	case FILL_STIPPLE5+1: stipple = stipple6; break;
	case FILL_STIPPLE5+2: stipple = stipple7; break;
	case FILL_STIPPLE5+3: stipple = stipple8; break;
	}
	if (stipple) {
	    XSetStipple (dpy, pgc, stipple);
	    XSetTSOrigin (dpy, pgc, fillshift[ggcurid], 0);
	    /* this is only correct possible if you set first the color */
	}
	else {
	    if (c_style & FILL_HOLLOW) c_style = FILL_HOLLOW;
	    else c_style = FILL_SOLID;
	}
	return;
    }

    switch (fs) {
    case FILL_CROSS:
    case FILL_CROSS+FILL_HOLLOW:
    case FILL_HOLLOW:
	break;
    case FILL_SOLID:
	/* XSetFillStyle (dpy, pgc, FillSolid); */
	break;
    case FILL_HASHED:
    case FILL_HASHED12B: /* 12.5% fill */
	Filld  = 8;
	Filld2 = 4;
	break;
    case FILL_HASHED25:
    case FILL_HASHED25B: /* 25% fill */
	Filld  = 4;
	Filld2 = 2;
	break;
    case FILL_HASHED50:
    case FILL_HASHED50B: /* 50% fill */
	Filld  = 2;
	Filld2 = 1;
	break;
    default:
	fs = c_style;
    }
    c_style = fs;
}

void d_ltype (int lt) /* set line type */
{
    if (lt == c_ltype) return;

    switch (lt) {
    case LINE_SOLID:
	gcv.line_style = LineSolid;
	XChangeGC (dpy, pgc, GCLineStyle, &gcv);
	break;
    case LINE_DOTTED:
#define NOT_LD
#ifndef NOT_LD
	gcv.line_style = LineOnOffDash;
	XChangeGC (dpy, pgc, GCLineStyle, &gcv);
#endif /* NOT_LD */
	break;
    case LINE_DOUBLE:
	gcv.line_style = LineSolid;
	XChangeGC (dpy, pgc, GCLineStyle, &gcv);
	break;
    default:
	lt = c_ltype;
    }
    c_ltype = lt;
}

void draw_Cross (Coor xCoor, Coor yCoor)
{
    double xst, yst;
    int dx1, dy1, dx2, dy2;

    yst = (double) yCoor;
    if (yst > YB && yst < YT) {
	/* inside display window */
	dx1 = TRANSF_X (XL);
	dx2 = TRANSF_X (XR);
	dy1 = TRANSF_Y (yst);
	XDrawLine (dpy, cwin, cgc, dx1, dy1, dx2, dy1); /* hor */
    }
    xst = (double) xCoor;
    if (xst > XL && xst < XR) {
	dx1 = TRANSF_X (xst);
	dy1 = TRANSF_Y (YB);
	dy2 = TRANSF_Y (YT);
	XDrawLine (dpy, cwin, cgc, dx1, dy1, dx1, dy2); /* ver */
    }
}

void draw_cross (Coor xCoor, Coor yCoor)
{
    double xst, yst, cr_size, x1, x2, y1, y2;
    int dx1, dy1, dx2, dy2;

    xst = (double) xCoor;
    yst = (double) yCoor;

    cr_size = (XR - XL) / 40.0;

    if (xst >= XL && xst <= XR && yst >= YB && yst <= YT) {
	/* inside display window */
	x1 = Max (XL, xst - cr_size);
	x2 = Min (XR, xst + cr_size);
	y1 = Max (YB, yst - cr_size);
	y2 = Min (YT, yst + cr_size);
	dx1 = TRANSF_X (x1);
	dx2 = TRANSF_X (x2);
	dy1 = TRANSF_Y (yst);
	XDrawLine (dpy, cwin, cgc, dx1, dy1, dx2, dy1);
	dx1 = TRANSF_X (xst);
	dy1 = TRANSF_Y (y1);
	dy2 = TRANSF_Y (y2);
	XDrawLine (dpy, cwin, cgc, dx1, dy1, dx1, dy2);
    }
}

/*
** Get cursor location.
** Returned after the mouse button is pressed.
** Note: current window is PICT.
*/
int get_loc (float *x_p, float *y_p, int echo)
{
    static int nexttime = 0;
    unsigned long emask = std_mask | ButtonPressMask | KeyPressMask;
    Coor xc, yc, dx, dy;
    int level, rv;
    int a_old, g_old, z_old;
    int ill_key = 1;

    if (nexttime) { /* previous button press of locator was in picture
		       window and the tracker was enabled, the effect
		       of this button press was possible a new def_window,
		       use old return values to calculate new actual
		       tracker position (before locator is moving) */
	float x, y;
	nexttime = 0;
	x = (XL + (float) ret_c_x / c_sX) / QUAD_LAMBDA;
	y = (YT - (float) ret_c_y / c_sY) / QUAD_LAMBDA;
	txe = (Coor) Round (x);
	tye = (Coor) Round (y);
	cxe = ret_c_x; /* for cecho == 6 */
	cye = ret_c_y;
    }

again:
    if (echo > 1) {
	if ((cecho = echo) > 6) {
	    cecho = 5;
	    notify ("get_loc: internal error (cecho > 6)!");
	}
	XSelectInput (dpy, pwin, emask | PointerMotionMask);
	if (tracker_mode) {
	    if (tracker_mode == 2) { /* AUTO ENABLE */
		tracker_enabled = 1;
		XMapWindow (dpy, trwin);
	    }
	    else DrawTracker ();
	}

	if (!ill_key) {
	    /* After some keys (like 'c' and 'i') the start position
	       of the rubber-box/line cursor must be fixed!
	       By cecho == 6 the size of the rubber-bbox cursor
	       must be fixed by resize of the window.
	    */
	    fix_loc (cfixx, cfixy, cecho);

	    if (cecho == 6) {
		ccur = cecho;
		DrawCursor ();
	    }
	}
    }
    else if (tracker_enabled) {
	XSelectInput (dpy, pwin, emask | PointerMotionMask);
	DrawTracker ();
    }
    else
	XSelectInput (dpy, pwin, emask);
    XSelectInput (dpy, mwin, emask);
    XSelectInput (dpy, lwin, emask);
    XSelectInput (dpy, twin, emask);
#ifdef DEBUG_EVENTS
    PE "-- WaitForEvent ButtonPress\n");
#endif
    WaitForEvent (ButtonPress);
    XSelectInput (dpy, pwin, std_mask);
    XSelectInput (dpy, mwin, std_mask);
    XSelectInput (dpy, lwin, std_mask);
    XSelectInput (dpy, twin, std_mask);
    if (cecho) {
	cecho = 0;
	if (tracker_mode == 2) { /* AUTO DISABLE */
	    tracker_enabled = 0;
	    XUnmapWindow (dpy, trwin);
	}
    }

    if (event.type == KeyPress) {
	    char kc[4]; /* ASCII key code */
	    KeySym ks;

	    *kc = 0;
	    ill_key = 0;

	    ks = XKeycodeToKeysym (dpy, event.xkey.keycode, 0);
	    switch (ks) {
		case XK_Up:	*kc = 'k'; break;
		case XK_Down:	*kc = 'j'; break;
		case XK_Right:	*kc = 'l'; break;
		case XK_Left:	*kc = 'h'; break;
		case XK_Prior:	*kc = 'p'; break;
		case XK_Next:	*kc = 'r'; break;
		case XK_Home:	*kc = 'b'; break;
		case XK_Select:	*kc = 'c'; break;

		case NoSymbol:
		case XK_Shift_L:
		case XK_Shift_R:
		case XK_Control_L:
		case XK_Control_R:
		case XK_Caps_Lock:
		case XK_Shift_Lock:
		case XK_Meta_L:
		case XK_Meta_R:
		case XK_Alt_L:
		case XK_Alt_R:
		    ill_key = 1;
		    goto again;

		default:
		    XLookupString (&event.xkey, kc, 1, NULL, NULL);
	    }

	if (ask_yes_no) {
	    if (*kc == 'n') { *y_p = 0; return (MENU); }
	    if (*kc == 'y') { *y_p = ask_yes_no; return (MENU); }
	}
	else if (allow_keypress) {
	    switch (*kc) { /* begin key matching */
		case '+':
		case '-':
		case '=':
		    z_old = zoom_mode;
		    zoom_mode = 0;
		    if (*kc == '-')
			de_zoom (piwl, piwr, piwb, piwt);
		    else
			curs_w (piwl, piwr, piwb, piwt);
		    zoom_mode = z_old;
		    break;
		case 'b':
		    bound_w ();
		    break;
		case 'p':
		    prev_w ();
		    break;
		case 'c':
		case 'i':
		case 'o':
		    if (!tracker_enabled) {
			Window ret_r, ret_c;
			float  x, y;
			int    ret_r_x, ret_r_y;
			unsigned int ret_mask;

			XQueryPointer (dpy, pwin, &ret_r, &ret_c,
			    &ret_r_x, &ret_r_y, &ret_c_x, &ret_c_y, &ret_mask);

			x = (XL + (float) ret_c_x / c_sX) / QUAD_LAMBDA;
			y = (YT - (float) ret_c_y / c_sY) / QUAD_LAMBDA;
			txe = (Coor) Round (x);
			tye = (Coor) Round (y);
		    }
		    xc = txe * QUAD_LAMBDA;
		    yc = tye * QUAD_LAMBDA;
		    if (*kc == 'c') {
			center_w (xc, yc);
		    }
		    else {
			z_old = zoom_mode;
			zoom_mode = 1;
			if (*kc == 'i')
			    curs_w (xc, piwr, yc, piwt);
			else
			    de_zoom (xc, piwr, yc, piwt);
			zoom_mode = z_old;
		    }
		    break;
		case 'x': /* enter coordinate */
		    get_coordnts ();
		    break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'e':
		    if (*kc != 'e') level = *kc - '0';
		    else level = Default_expansion_level;
		    a_old = allow_keypress;
		    allow_keypress = 0;
		    if (expansion (level)) {
			upd_boundb ();
			inform_cell ();
			set_titlebar (NULL);
			if (ccur) { /* Remove current cursor */
			    DrawCursor ();
			    ccur = 0;
			}
			set_c_wdw (PICT);
			Rpoly ();
			redraw_cross ();
			if (!first_pict_buf) pict_buf ();
			picture ();
			Rpoly ();
			redraw_cross ();
			if (!first_pict_buf) pict_buf ();
		    }
		    allow_keypress = a_old;
		    if (level == -1) { /* asking for level */
			if (in_cmd)
			    *y_p = in_cmd;
			else
			    *y_p = -1; /* redraw of menu needed! */
			return (MENU);
		    }
		    goto again;
		case 'N':
		case 'U':
		case 'R':
		case 'W':
		    a_old = allow_keypress;
		    allow_keypress = 0;
		    if (*kc == 'U') {
			rv = upd_mod ();
		    }
		    else if (*kc == 'R') {
			rv = inp_mod (NULL);
		    }
		    else {
			rv = 0;
			if (*kc == 'N')
			    rv = eras_worksp (NULL);
			else
			    wrte_cell ();
			set_titlebar (NULL);
		    }
		    if (rv) {
			picture ();
			Rpoly ();
			redraw_cross ();
			if (!first_pict_buf) pict_buf ();
		    }
		    allow_keypress = a_old;
		    if (in_cmd) {
			cmd_nbr = 0;
			*y_p = in_cmd;
		    }
		    else
			*y_p = -1; /* redraw of menu needed! */
		    return (MENU);
		case 'D':
		    set_dominant ();
		    break;
		case 'd':
		    set_hashed ();
		    break;
		case 'g':
		    toggle_grid ();
		    if (ccur) { /* Remove current cursor */
			DrawCursor ();
			ccur = 0;
		    }
		    picture ();
		    goto again;
		case 'H':
		case 'J':
		case 'K':
		case 'L':
		case 'h':
		case 'j':
		case 'k':
		case 'l':
		    dx = piwr - piwl;
		    dy = piwt - piwb;
		    xc = piwl + dx/2;
		    yc = piwb + dy/2;
		    switch (*kc) {
			case 'H': dx = -dx; dy = 0; break;
			case 'L': dx =  dx; dy = 0; break;
			case 'J': dx = 0; dy = -dy; break;
			case 'K': dx = 0; dy =  dy; break;
			case 'h': dx = -dx/4; dy = 0; break;
			case 'l': dx =  dx/4; dy = 0; break;
			case 'j': dx = 0; dy = -dy/4; break;
			case 'k': dx = 0; dy =  dy/4; break;
		    }
		    center_w (xc + dx, yc + dy);
		    break;
		case 'E':
		    a_old = allow_keypress;
		    allow_keypress = 0;
		    indiv_exp ();
		    picture ();
		    allow_keypress = a_old;
		    if (in_cmd)
			*y_p = in_cmd;
		    else
			*y_p = new_cmd;
		    return (MENU);
		case 's':
		    if (VIS_mode) goto dis_mess;
		    toggle_subterm ();
		    break;
		case 'v':
		    if (VIS_mode) {
			if (VIS_mode == 2) break;
			*y_p = 0; /* return */
			return (MENU);
		    }
		    a_old = allow_keypress;
		    g_old = grid_cmd;
		    dom_order_cmd = grid_cmd = 0;
		    Visible ();
		    grid_cmd = g_old;
		    allow_keypress = a_old;
		    if (in_cmd)
			*y_p = in_cmd;
		    else
			*y_p = -1; /* redraw of menu needed! */
		    return (MENU);
		case 'q': /* quit */
		    ask_quit ();
		    if (in_cmd)
			*y_p = in_cmd;
		    else
			*y_p = -1; /* redraw of menu needed! */
		    ptext ("");
		    return (MENU);
		case 'r': /* redraw screen */
		case '\014':
		    pict_all (ERAS_DR);
		    pic_max ();
		    break;
		case '\033': /* escape */
		    *y_p = 0; /* back to previous menu! */
		    if (ccur) { /* Remove current cursor */
			DrawCursor ();
			ccur = 0;
		    }
		    return (MENU);
		case 't':
		    tracker_mode = (tracker_mode != 1);
		    toggle_tracker ();
		default:
		    ill_key = 1;
		    goto again;
	    }
	    picture ();
	    ccur = 0;
	    if (!first_pict_buf) pict_buf ();
	    Rpoly ();
	    redraw_cross ();
	    if (tracker_enabled) {
		Window ret_r, ret_c;
		float  x, y;
		int    ret_r_x, ret_r_y;
		unsigned int ret_mask;

		XQueryPointer (dpy, pwin, &ret_r, &ret_c,
		    &ret_r_x, &ret_r_y, &ret_c_x, &ret_c_y, &ret_mask);

		x = (XL + (float) ret_c_x / c_sX) / QUAD_LAMBDA;
		y = (YT - (float) ret_c_y / c_sY) / QUAD_LAMBDA;
		txe = (Coor) Round (x);
		tye = (Coor) Round (y);
	    }
	    goto again;
	}
	ill_key = 1;
	notify ("KeyPress not allowed!");
	goto again;
dis_mess:
	ill_key = 1;
	notify ("This key is disabled!");
	goto again;
    } /* end key pressed */

    /*
    ** OK, ButtonPress event seen!
    ** In which window did it occur?
    ** "ewin" is the window of the event!
    */
    if (ewin == twin) return (TEXT);

    if (ewin == cwin) { /* event window is current window */
	*x_p = XL + (float) ret_c_x / c_sX;
	*y_p = YT - (float) ret_c_y / c_sY;
	/* if (tracker_enabled) */ nexttime = 1;
	if (ewin != pwin) notify ("get_loc: internal error (cwin != pwin)!");
    }
    else {
	float wx, wy;
	struct Disp_wdw *wdw;
	     if (ewin == mwin) wdw = wdw_arr[MENU];
	else if (ewin == lwin) wdw = wdw_arr[LAYS];
	else wdw = p_wdw;
	wx = wdw -> wxmin;
	wy = wdw -> wymax;
	*x_p = wx + (float) ret_c_x / (wdw -> vxmax / (wdw -> wxmax - wx));
	*y_p = wy - (float) ret_c_y / (wdw -> vymax / (wy - wdw -> wymin));
	if (ewin == pwin) notify ("get_loc: internal error!");
    }

    if (ewin == lwin) return (LAYS);

    if (ccur) { /* Remove current cursor */
	DrawCursor ();
	ccur = 0;
    }

    if (ewin == mwin) return (MENU);

    return (PICT);
}

static float loc_ll, loc_rr, loc_bb, loc_tt;

void set_bbox_loc (Coor ll, Coor rr, Coor bb, Coor tt) /* Set bbox of picture locator */
{
    loc_ll = ll; loc_rr = rr; loc_bb = bb; loc_tt = tt;
}

/*
** If (echo_mode > 0)
** Set or fix start point of picture locator
** Set start and end points of tracker window
*/
void fix_loc (float xx, float yy, int echo)
{
    int x1, x2, y1, y2;
    /* Note: current window is PICT */
    if (cwin != pwin) notify ("fix_loc: internal error (cwin != pwin)!");
    if (echo == 6) {
	xx = loc_ll;
	yy = loc_bb;
	x1 = TRANSF_X (loc_ll);
	x2 = TRANSF_X (loc_rr);
	y2 = TRANSF_Y (loc_bb);
	y1 = TRANSF_Y (loc_tt);
	if ((crw = x2 - x1) > DMAX) crw = DMAX;
	if ((crh = y2 - y1) > DMAX) crh = DMAX;
    }
    else {
	cfixx = xx; cfixy = yy;
	cxs = TRANSF_X (xx);
	cys = TRANSF_Y (yy);
    }
    txs = txe = xx / QUAD_LAMBDA;
    tys = tye = yy / QUAD_LAMBDA;
}

void ggBell (int percent)
{
    if (percent <= -100) return;
    if (percent > 100) percent = 100;
    _ggBell (percent);
    XFlush (dpy);
}

void _ggBell (int percent)
{
    XBell (dpy, percent);
}

static void ggInit (int planes)
{
    int max;
    if (planes > 5) planes = 5;
    if (planes < 3) planes = 3;
    nr_planes = DisplayPlanes (dpy, DefaultScreen (dpy));
    if (nr_planes > 8) nr_planes = 8;
    if (colormap) {
	if (!InitCacdCmap (dpy, DefaultScreen (dpy))) CacdCmapError (argv0, 1);
    }
    if (nr_planes < 1) {
	PE "%s: ggInit: not enough bit planes\n", argv0);
	exit (1);
    }
    maxindex = 1 << nr_planes;
    max = 1 << planes;
    if (max <= GGMAXCOLORS) ggmaxcolors = max;
}

int ggTestColor (int color_id, char *color)
{
    if (!strcmp (ggconame[color_id], color)) return (1);
    return (0);
}

void ggSetIdForColorCode (int code)
{
    if (code < 0 || code >= ggmaxcolors) {
	PE "%s: Illegal color code '%d' in maskdata\n", argv0, code);
	exit (1);
    }
    (void) ggGetIdForColor (ggcolors[code]);
}

void ggChangeColorCode (int id, int code)
{
    ggconame[id] = ggcolors[code];
    if (!CoupleIdToColor (id, ggcolors[code])) CacdCmapError (argv0, 1);
}

static int ggGetIdForColor (char *color)
{
    static int Firsttime = 1;
    static int w_index;
    register int i;

    for (i = 0; i < ggmaxcolors; ++i) {
	if (strcmp (color, ggcolors[i]) == 0) goto ok;
    }
    PE "%s: ggGetIdForColor: unknown color '%s'\n", argv0, color);
    exit (1);
ok:
    if (ggidnr >= ggMaxIds) {
	PE "%s: ggGetIdForColor: too many ids\n", argv0);
	exit (1);
    }
    ggconame[ggidnr] = ggcolors[i];

    if (!colormap) {
	if (i == 7) { /* white */
	    if (Firsttime) { w_index = ggmaxcolors; Firsttime = 0; }
	    i = w_index;
	    w_index <<= 1;
	}
	if (i >= maxindex) {
	    i %= maxindex;
	    if (i == 0) {
		if (nr_planes > 3) { /* last plane is a white plane */
		    /* use last plane */
		    i = maxindex >> 1;
		}
		else {
		    i = maxindex - 1; /* white */
		}
	    }
	}
	if ((i <<= 1) >= maxindex) i += 1 - maxindex;
	ggcindex[ggidnr] = i;
    }
    else if (!CoupleIdToColor (ggidnr, color)) CacdCmapError (argv0, 1);
    return (ggidnr++);
}

static int ggGetColorIndex (int color_id)
{
    unsigned long ci;

    if (color_id < 0 || color_id >= ggidnr) return (0);
    if (colormap) {
	GetPixelFromId (color_id, &ci);
	return ((int) ci);
    }
    return (ggcindex[color_id]);
}

static int ggGetColorsIndex (int col_ids[], int nr_ids)
{
    unsigned long ci;
    register int cn, i;

    if (colormap) {
	GetPixelFromIds (col_ids, nr_ids, &ci);
	return ((int) ci);
    }
    for (cn = i = 0; i < nr_ids; ++i) {
	cn |= ggGetColorIndex (col_ids[i]);
    }
    return (cn);
}

int ggSetErasedAffected (int eIds[], int p_arr[], int nr_all)
{
    int nr_aIds = ggMaxIds;
    register int ci, i, j, nr_eIds;

    nr_eIds = 0;
    for (i = 0; i < nr_all; ++i) {
	if (p_arr[i] == ERAS_DR) {
	    if (fillst[i] & FILL_HOLLOW) Erase_hollow = 1;
	    if (black_arr[i] || (Draw_dominant && dom_arr[i])) { /* dominant */
		pict_all (DRAW);
		goto ret_all;
	    }
	    eIds[nr_eIds++] = i;
	}
    }
    if (!nr_eIds) return (0);

    if (colormap) {
	GetAffectedIds (eIds, nr_eIds, aIds, &nr_aIds);
	for (j = i = 0; i < nr_aIds; ++i)
	    if (aIds[i] < nr_all) { ++j; p_arr[aIds[i]] = DRAW; }
    }
    else {
	ci = ggGetColorsIndex (eIds, nr_eIds);
	for (j = i = 0; i < nr_all; ++i)
	    if (ci & ggGetColorIndex (i)) { ++j; p_arr[i] = DRAW; }
    }
    if (j == nr_all) {
ret_all:
	Erase_hollow = 1;
	return (nr_all);
    }
    return (nr_eIds);
}

static void SetForegroundByIndex (ci)
{
    XSetForeground (dpy, pgc, (unsigned long) ci);
}

int ggGetColor ()
{
    return (ggcurid);
}

void ggSetColor (int color_id)
{
    if (cwin == twin) return;
    if (color_id < 0 || color_id == ggcurid || color_id >= ggidnr) return;
    if (colormap)
	SetForegroundFromId (pgc, color_id);
    else
	SetForegroundByIndex (ggGetColorIndex (color_id));
    ggcurid = color_id;
}

void ggSetColors (int col_ids[], int nr_ids)
{
    if (cwin == twin) return;
    ggcurid = -1;
    if (colormap)
	SetForegroundFromIds (pgc, col_ids, nr_ids);
    else
	SetForegroundByIndex (ggGetColorsIndex (col_ids, nr_ids));
}

void ggEraseWindow ()
{
    disp_mode (ERASE);
    paint_box (XL, XR, YB, YT);
    disp_mode (TRANSPARENT);
    ggSetColor (Gridnr);
}

void ggClearWindow ()
{
    XClearWindow (dpy, cwin);
}

void ggEraseArea (float xl, float xr, float yb, float yt, int mode)
{
    int dx1, dy1, dx2, dy2;
    unsigned int dw, dh;
    if (!mode) disp_mode (ERASE);
    if (yb - YB < 0.01) yb = YB - 1.0; /* be sure everything is erased! */
    dx1 = (int) TRANSF_X (xl);
    dx2 = (int) TRANSF_X (xr);
    dy1 = (int) TRANSF_Y (yb);
    dy2 = (int) TRANSF_Y (yt);
    if (dx1 > dx2) { dw = dx1 - dx2; dx1 = dx2; }
    else dw = dx2 - dx1;
    if (dy1 > dy2) { dh = dy1 - dy2; dy1 = dy2; }
    else dh = dy2 - dy1;
    if (Erase_hollow) {
	if (Erase_hollow++ == 1) { /* first time */
	    e_area_x = dx1 + dw;
	    e_area_y = dy1 + dh;
	}
	++dw; ++dh;
    }
    if (mode) /* erase everything */
	XClearArea (dpy, cwin, dx1, dy1, dw, dh, False);
    else
	XFillRectangle (dpy, cwin, pgc, dx1, dy1, dw, dh);
}

void d_grid (Coor wxl, Coor wxr, Coor wyb, Coor wyt, Coor sp)
{
    int dx1, dy1;
    Coor x, y, xs, ys;

    if ((xs = wxl - wxl % sp) < wxl) xs += sp;
    if ((ys = wyb - wyb % sp) < wyb) ys += sp;
    for (x = xs; x <= wxr; x += sp) {
	dx1 = TRANSF_X (x);
	for (y = ys; y <= wyt; y += sp) {
	    dy1 = TRANSF_Y (y);
	    XDrawPoint (dpy, cwin, pgc, dx1, dy1);
	}
    }
}

void d_snapgrid (Coor wxl, Coor wxr, Coor wyb, Coor wyt, Coor sp, Coor xoff, Coor yoff)
{
    int xp, yp;
    Coor x, y, xs, ys;

    xs = wxl - wxl % sp + xoff % sp - sp;
    while (xs < wxl) xs += sp;
    ys = wyb - wyb % sp + yoff % sp - sp;
    while (ys < wyb) ys += sp;

    for (x = xs; x <= wxr; x += sp) {
	xp = TRANSF_X (x);
	for (y = ys; y <= wyt; y += sp) {
	    yp = TRANSF_Y (y);
	    XDrawLine (dpy, cwin, pgc, xp, yp - 2, xp, yp + 2);
	    XDrawLine (dpy, cwin, pgc, xp - 2, yp, xp + 2, yp);
	}
    }
}

void pict_cur (Coor x_c, Coor y_c)
{
    int x = TRANSF_X ((float) x_c);
    int y = TRANSF_Y ((float) y_c);
    ggSetColor (Cur_nr);
    disp_mode (COMPLEMENT);
    XDrawLine (dpy, cwin, pgc, x - 4, y - 4, x + 4, y + 4);
    XDrawLine (dpy, cwin, pgc, x - 4, y + 4, x + 4, y - 4);
}

void d_line (float x1, float y1, float x2, float y2)
{
    int dx1, dy1, dx2, dy2, x, y;

    dx1 = TRANSF_X (x1);
    dy1 = TRANSF_Y (y1);
    dx2 = TRANSF_X (x2);
    dy2 = TRANSF_Y (y2);
    if (c_ltype == LINE_DOUBLE) {
	if (dy1 == dy2) { /* hor. line */
	    XDrawLine (dpy, cwin, pgc, dx1, dy1-1, dx2, dy1-1);
	    XDrawLine (dpy, cwin, pgc, dx1, dy1+1, dx2, dy1+1);
	}
	else if (dx1 == dx2) { /* vert. line */
	    XDrawLine (dpy, cwin, pgc, dx1-1, dy1, dx1-1, dy2);
	    XDrawLine (dpy, cwin, pgc, dx1+1, dy1, dx1+1, dy2);
	}
	else { /* 45 degrees line */
	    XDrawLine (dpy, cwin, pgc, dx1+1, dy1, dx2+1, dy2);
	    XDrawLine (dpy, cwin, pgc, dx1-1, dy1, dx2-1, dy2);
	}
    }
#ifdef NOT_LD
    else if (c_ltype == LINE_DOTTED) {
	if (dy1 == dy2) { /* hor. line */
	    if (dx2 < dx1) { x = dx1; dx1 = dx2; dx2 = x; }
	    for (x = dx1; x <= dx2; x += 10)
		/*
		XDrawPoint (dpy, cwin, pgc, x, dy1);
		*/
		XDrawLine (dpy, cwin, pgc, x, dy1, x+4, dy1);
	}
	else if (dx1 == dx2) { /* vert. line */
	    if (dy2 < dy1) { y = dy1; dy1 = dy2; dy2 = y; }
	    for (y = dy1; y <= dy2; y += 10)
		/*
		XDrawPoint (dpy, cwin, pgc, dx1, y);
		*/
		XDrawLine (dpy, cwin, pgc, dx1, y, dx1, y+4);
	}
	else
	    XDrawLine (dpy, cwin, pgc, dx1, dy1, dx2, dy2);
    }
#endif /* NOT_LD */
    else {
	XDrawLine (dpy, cwin, pgc, dx1, dy1, dx2, dy2);
    }
}

void d_circle (float x1, float y1, float x2, float y2)
{
    double a, b;
    float  w;
    int dx1, dy1;
    unsigned int dw;

    a = x2 - x1;
    b = y2 - y1;
    w = (float) sqrt (a * a + b * b);
    dw  = SCALE_DX (w);
    dx1 = TRANSF_X ((x2 + x1 - w) / 2);
    dy1 = TRANSF_Y ((y2 + y1 + w) / 2);
    if (c_style == FILL_HOLLOW)
	XDrawArc (dpy, cwin, pgc, dx1, dy1, dw, dw, 0, 360 * 64);
    else
	XFillArc (dpy, cwin, pgc, dx1, dy1, dw, dw, 0, 360 * 64);
}

void flush_pict ()
{
    XFlush (dpy);
}

void toggle_tracker ()
{
    if (tracker_mode == 1) {
	if (!tracker_enabled) {
	    tracker_enabled = 1;
	    XMapWindow (dpy, trwin);
	}
    }
    else if (tracker_enabled) {
	tracker_enabled = 0;
	XUnmapWindow (dpy, trwin);
    }
}

static void DrawCursor ()
{
    switch (ccur) {
    case 2: /* rubber line cursor */
    case 3: /* rubber line cursor */
    case 4: /* rubber line cursor */
	XDrawLine (dpy, pwin, cgc, cxs, cys, cxe, cye);
	break;
    case 5: /* rubber box cursor */
	XDrawRectangle (dpy, pwin, cgc, cxe, cye, crw, crh);
	break;
    case 6: /* bbox cursor */
	XDrawRectangle (dpy, pwin, cgc, cxe, cye - crh, crw, crh);
	break;
    }
}

static void DrawTracker ()
{
    if (cecho > 1) {
	if (cecho > 3)
	    sprintf (trstr, "%5ld,%5ld (%4ld,%4ld) ", txe, tye, txe - txs, tye - tys);
	else if (cecho > 2)
	    sprintf (trstr, "%5ld,%5ld (dy = %4ld) ", txe, tye, tye - tys);
	else
	    sprintf (trstr, "%5ld,%5ld (dx = %4ld) ", txe, tye, txe - txs);
    }
    else
	sprintf (trstr, "%5ld,%5ld %12s", txe, tye, " ");
    XDrawImageString (dpy, trwin, tgc, cD, cH, trstr, trstrLen);
    txe_old = txe;
    tye_old = tye;
}

static void WaitForEvent (int etype)
{
    Window ret_r, ret_c;
    float  x_p, y_p;
    int    ret_r_x, ret_r_y;
    int    ex, ey, ew, eh;
    int    pwin_resize = 0;
    unsigned int ret_mask;

    for (;;) {
	XNextEvent (dpy, &event);

	ewin = event.xany.window;

	switch (event.type) {

	case MotionNotify:
#ifdef DEBUG_MOTION_EVENTS
	    PE "-- MotionNotify\n");
#endif
	    while (XCheckTypedEvent (dpy, MotionNotify, &event)) {
#ifdef DEBUG_MOTION_EVENTS
		PE "-- One More MotionNotify!\n");
#endif
	    }
	    if (cwin != pwin) {
		if (etype == KeyPress) break;
		notify ("WaitForEvent: internal error (cwin != pwin)!");
	    }

	    ex = event.xmotion.x;
	    ey = event.xmotion.y;

	    if (cecho > 1) {
		DrawCursor (); /* Remove old cursor */
		if (cecho == 5) { /* rubber box cursor */
		    if (ex < cxs) { crw = cxs - ex; cxe = ex; }
		    else          { crw = ex - cxs; cxe = cxs; }
		    if (ey < cys) { crh = cys - ey; cye = ey; }
		    else          { crh = ey - cys; cye = cys; }
		}
		else { /* rubber line or bbox cursor */
		    cxe = ex; cye = ey;
		}
		ccur = cecho;
		DrawCursor (); /* Draw new cursor */
	    }
	    if (tracker_enabled) {
		x_p = (XL + (float) ex / c_sX) / QUAD_LAMBDA;
		y_p = (YT - (float) ey / c_sY) / QUAD_LAMBDA;
		txe = (Coor) Round (x_p);
		tye = (Coor) Round (y_p);
		if (txe != txe_old || tye != tye_old) DrawTracker ();
	    }
	    break;

	case Expose:

	    if (etype == Expose) { /* first time */
#ifdef DEBUG_EVENTS
		PE "-- Expose: first time!\n");
#endif
		/* flush all exposes */
		while (XCheckTypedEvent (dpy, Expose, &event) == TRUE);
		return;
	    }
	    if (!cwin) {
#ifdef DEBUG_EVENTS
		PE "-- Expose: !cwin\n");
#endif
		break;
	    }

	    if (ewin == pwin) { /* redraw picture */
		XRectangle R[1];
beg_pwin:
#ifdef DEBUG_EVENTS
		PE "-- Expose: ewin = pwin\n");
#endif
		R[0].x = event.xexpose.x;
		R[0].y = event.xexpose.y;
		R[0].width  = event.xexpose.width;
		R[0].height = event.xexpose.height;
		if (pwin_resize) {
		    pwin_resize = 0;
		    while (XCheckTypedWindowEvent (dpy, pwin, Expose, &event)) {
#ifdef DEBUG_EVENTS
			PE "-- Expose(pwin): SKIP THIS ONE!\n");
#endif
			if (R[0].width  < event.xexpose.width ||
				R[0].height < event.xexpose.height) {
			    R[0].x = event.xexpose.x;
			    R[0].y = event.xexpose.y;
			    R[0].width  = event.xexpose.width;
			    R[0].height = event.xexpose.height;
			}
		    }
		}
#ifdef DEBUG_EVENTS
		PE "-- Rpicture: x,y = %d,%d  width,height = %d,%d\n",
		    R[0].x, R[0].y, R[0].width, R[0].height);
#endif
		Rpicture (R[0].x, R[0].y, R[0].width, R[0].height);
		XSetClipRectangles (dpy, cgc, 0, 0, R, 1, Unsorted);
		XSetClipRectangles (dpy, pgc, 0, 0, R, 1, Unsorted);
		if (cecho) {
		    if (ccur) DrawCursor (); /* Redraw current cursor */
		    else fix_loc (cfixx, cfixy, cecho); /* resize of window! */
		}
		if (!first_pict_buf) {
		    pict_buf (); /* redraw cursor rectangles */
		}
		Rpoly ();
		redraw_cross ();
		XSetClipMask (dpy, cgc, None);
		XSetClipMask (dpy, pgc, None);
	    }
	    else if (ewin == mwin) {
#ifdef DEBUG_EVENTS
		PE "-- Expose: ewin = mwin\n");
#endif
		Rmenu (); /* redraw menu */
		set_c_wdw (PICT);
	    }
	    else if (ewin == twin) {
#ifdef DEBUG_EVENTS
		PE "-- Expose: ewin = twin\n");
#endif
		Rtext (); /* redraw text */
	    }
	    else if (ewin == trwin) {
#ifdef DEBUG_EVENTS
		PE "-- Expose: ewin = trwin\n");
#endif
		if (tracker_enabled) DrawTracker ();
	    }
	    else if (ewin == lwin) {
#ifdef DEBUG_EVENTS
		PE "-- Expose: ewin = lwin\n");
#endif
		Rmsk (); /* redraw lay menu */
		set_c_wdw (PICT);
	    }

	    if (pwin_resize) {
		if (XCheckTypedWindowEvent (dpy, pwin, Expose, &event))
		    goto beg_pwin;
		pwin_resize = 0;
	    }
	    break;

	case ButtonPress:
#ifdef DEBUG_EVENTS
	    PE "-- ButtonPress\n");
#endif
	    XQueryPointer (dpy, ewin, &ret_r, &ret_c,
		&ret_r_x, &ret_r_y, &ret_c_x, &ret_c_y, &ret_mask);
	    break;

	case KeyPress:
#ifdef DEBUG_EVENTS
	    PE "-- KeyPress\n");
#endif
	    return;

	case ConfigureNotify:
#ifdef DEBUG_EVENTS
	    PE "-- ConfigureNotify\n");
#endif
	    ew = event.xconfigure.width;
	    eh = event.xconfigure.height;

#ifdef NOWINDOWMANAGER
	    ex = event.xconfigure.x;
	    ey = event.xconfigure.y;
	    if (ex + ew > maxW) ex = maxW - ew;
	    if (ex < 0) ex = 0;
	    if (ey + eh > maxH) ey = maxH - eh;
	    if (ey < 0) ey = 0;
	    if (ex != event.xconfigure.x || ey != event.xconfigure.y) {
		XMoveWindow (dpy, rwin, ex, ey); /* incorrect root window position */
		continue;
	    }
#endif /* NOWINDOWMANAGER */

	    if (rH != eh) {
		rH = mH = eh;
		pH = mH - tH - BW;
		XResizeWindow (dpy, mwin, mW, mH);
		XResizeWindow (dpy, lwin, mW, mH);
		Set_window (MENU);
	    }
	    else if (rW == ew) break; /* not a resize, do nothing */

	    if (rW != ew) {
		rW = ew;
		tW = rW - 2*mW - 2*BW;
		XResizeWindow (dpy, twin, tW, tH);
		XResizeWindow (dpy, trwin, trW, tH);
		XMoveWindow (dpy, lwin, mW + tW + BW, -BW);
		XMoveWindow (dpy, trwin, mW + tW - trW, -BW);
		Set_window (TEXT);
	    }
	    ccur = 0; /* Do not redraw cursor! */
	    pwin_resize = 1;
#ifdef DEBUG_EVENTS
	    PE "-- XResizeWindow(pwin): width = %d, height = %d\n", tW, pH);
#endif
	    XResizeWindow (dpy, pwin, tW, pH);
	    XSync (dpy, 0); /* Needed to get all Expose events! */
	    Set_window (LAYS);
	    Set_window (PICT);
	    break;
	}

	if (event.type == etype) break;
    }
}

static int t_x;

void GetString (char sbuf[], int len, int max)
{
    char   kc[4]; /* ASCII key code */
    KeySym ks;
    unsigned long emask;
    int ibuf = len;

    XDrawString (dpy, twin, tgc, cD, cH, sbuf, len);
    t_x = cD + cW * len;
    Prompt ();

    emask = std_mask | KeyPressMask;
    XSelectInput (dpy, pwin, emask);
    XSelectInput (dpy, mwin, emask);
    XSelectInput (dpy, lwin, emask);
    XSelectInput (dpy, twin, emask);
    for (;;) {
	WaitForEvent (KeyPress);

	ks = XKeycodeToKeysym (dpy, event.xkey.keycode, 0);
	if (ks == NoSymbol ||
	    ks == XK_Shift_L || ks == XK_Shift_R) continue;
	XLookupString (&event.xkey, kc, 1, NULL, NULL);

	if (*kc == '\r' || *kc == '\n' || *kc == '\033') { /* return */
	    XClearArea (dpy, twin, t_x, cH - cA, cW+1, cH+1, False);
	    break;
	}
	if (*kc == '\b' || *kc == '\177') { /* delete */
	    if (ibuf == len) { /* do nothing: sbuf is empty */
		continue;
	    }
	    XClearArea (dpy, twin, t_x, cH - cA, cW+1, cH+1, False);
	    --ibuf;
	    t_x -= cW;
	}
	else {
	    if (ibuf == max) { /* do nothing: sbuf is full */
		continue;
	    }
	    if (t_x + cW > tW) { /* do nothing: text window is full */
		continue;
	    }
	    if (*kc < ' ' || *kc >= '\177') { /* ill. char */
		if (*kc != '\t') continue;
		*kc = ' ';
	    }
	    XClearArea (dpy, twin, t_x, cH - cA, cW+1, cH+1, False);
	    XDrawString (dpy, twin, tgc, t_x, cH, kc, 1);
	    sbuf[ibuf++] = *kc;
	    t_x += cW;
	}
	sbuf[ibuf] = '\0';
	Prompt ();
    }
    XSelectInput (dpy, pwin, std_mask);
    XSelectInput (dpy, mwin, std_mask);
    XSelectInput (dpy, lwin, std_mask);
    XSelectInput (dpy, twin, std_mask);
}

void Prompt ()
{
    XFillRectangle (dpy, twin, tgc, t_x, cH - cA, cW, cH);
}

char *Xerrlist[] = {
/* 0 */ "set the DISPLAY environment variable or give option -h",
/* 1 */ "cannot open display on %s",
/* 2 */ "cannot load font %s",
/* 3 */ "cannot create window on %s",
/* 4 */ "unknown window event",
/* 5 */ "internal colorid mismatch",
/* 6 */ "unknown Xerror"
};

static void Xerror (int errno, char *s)
{
    if (errno < 0 || errno > 6) errno = 6;
    PE "%s: ", argv0);
    PE Xerrlist[errno], s);
    PE "\n");
    exit (1);
}
