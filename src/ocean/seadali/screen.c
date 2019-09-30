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
#include "X11/Xutil.h"
#include "X11/cursorfont.h"
#include "X11/keysym.h"
#include "src/ocean/seadali/header.h"

// #define DEBUG_EVENTS /* Print X11 Events */
// #define DEBUG_MOTION_EVENTS  /* Print MotionEvents */

#ifndef CACDCMAP	/* use CACD colormap */
#define SET_COLORS	/* set colormap and colors */
#endif
			/* last two color entries are reserved */
#define N_MENU 12 /* max. number of chars in menu */
#define N_CMDS 20 /* max. number of cmds  in menu */

#define TRANSF_X(wx) (((wx) - c_wX) * c_sX)
#define TRANSF_Y(wy) ((c_wY - (wy)) * c_sY)
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
extern int looks_like_the_sea_child_died (int);

extern int ImageMode;
extern int Draw_dominant, Draw_hashed;

static XClassHint theXclasshint;

extern DM_PROJECT *dmproject;
extern char *argv0;
extern char *cellstr;
extern struct Disp_wdw *wdw_arr[];
extern struct Disp_wdw *c_wdw;
extern struct Disp_wdw *p_wdw;

extern int   exp_level;
extern int   interrupt_flag;
extern int   in_flight_flag;
extern int   maxDisplayStrLen;

extern int  Sub_terms; /* TRUE to indicate sub terminals */

extern int   nr_planes;
extern int   Backgr;
extern int   Cur_nr;
extern int   DRC_nr;
extern int   Gridnr;
extern int   Textnr;
extern int   Yellow;
extern int   NR_lay;
extern int   NR_all;
extern int  *dom_arr;

static XColor c_fg = { 0, 65535, 65535, 65535 };
static XColor c_bg = { 0, 0, 0, 0 };

extern int   d_apollo;
extern char *DisplayName;
extern char *geometry;
extern int *pict_arr;

static char *CWD;
static char  HOST[100];
Bool	tracker_enabled = FALSE;
static	GC trgc; /* tracker gc */
int	trW = 250; /* tracker window Width */
Coor	sx = 0, sy = 0, /* cursor pos'n's */
	endx = 0, endy = 0,
	dx = 0, dy = 0,
	old_sx = 0, old_sy = 0,
	old_endx = 0, old_endy = 0;
static	char mystr[100]  = "--------------------------------";

Display *dpy;
Window   rwin; /* root window */
Window   mwin; /* menu window */
Window   twin; /* text window */
Window	trwin; /* tracker window */
Window   pwin; /* pict window */
Window   lwin; /* layer window */
Window   ewin; /* event window */
Window   cwin; /* current window */
Window   win_arr[10]; /* window array */
static XEvent event;
static XFontStruct *font;
GC pgc; /* picture gc */
static GC tgc; /* text    gc */
static GC cgc; /* cursor  gc */
static XGCValues gcv;
static Colormap cmap;
static unsigned long std_mask;
static unsigned long Black, White;
static int BW = 1; /* borderwidth */
static int DW, DH; /* DisplayWidth, Height */
static int maxW, maxH; /* maxWidth, maxHeight */
static int DMAX;   /* Display Max */
static int rW, rH; /* root Width, Height */
int mW, mH; /* menu Width, Height */
int tW, tH; /* text Width, Height */
/* AvG */
int cW, cH; /* char Width, Height */
static int cA, cD; /* char Ascent, Descent */
int pH; /* picture Height */
float c_sX, c_sY, c_wX, c_wY;
static float cfixx, cfixy;
static int cxs, cys, cxe, cye;
static int crx, cry;
static unsigned int crw, crh;
static int cecho, ccur;
static int ret_c_x, ret_c_y;
int RGBids[3];
int ask_mode = 0;

#define SEA_CHILD_DIES 20

/* PATRICK: new for fillstyles */
static int Filld = 4;       /* fill density */
static int Filld2 = 8;      /* half fill density */

static int c_dmode = -1;
static int c_ltype = -1;
static int c_style = -1;
static int maxindex;
static int ggcurid = -1;
static int ggidnr;
static char kc[4]; /* ASCII key code */

#define GGMAXCOLORS 32

static int  ggMaxIds;
static int *ggcindex;
#ifdef CACDCMAP
static int *aIds;
#endif
static char **ggconame;
static int ggmaxcolors = GGMAXCOLORS;
static char  *ggcolors[GGMAXCOLORS] = {
/*  0 */ "black",
/*  1 */ "red",
/*  2 */ "green",
/*  3 */ "yellow",
/*  4 */ "blue",
/*  5 */ "magenta",
/*  6 */ "cyan",
/*  7 */ "white",
/*  8 */ "gold",
/*  9 */ "coral",
/* 10 */ "sienna",
/* 11 */ "goldenrod",
/* 12 */ "orange",
/* 13 */ "violet red",
/* 14 */ "pink",
/* 15 */ "green yellow",
/* 16 */ "spring green",
/* 17 */ "aquamarine",
/* 18 */ "navy",
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
/* 31 */ "dark turquoise"
};

static int ggFindIdForColor (char *color);
static int ggGetColorIndex (int color_id);
static int ggGetColorsIndex (int col_ids[], int nr_ids);
static int ggGetIdForColor (char *color);
static void ggInit (int planes);
static void remove_cursor (void);
static void WaitForEvent (int etype);
static void Xerror (int errno, char *s);

static void zdcol (int ci, float R, float G, float B)
{
    XColor X11_col;

    if (ci < 0 || ci >= maxindex) return;

    ci <<= 1;
    if (ci >= maxindex) ci = ci - maxindex + 1;
    X11_col.pixel = (unsigned long) ci;
    X11_col.red   = (unsigned short) (R * 65535);
    X11_col.green = (unsigned short) (G * 65535);
    X11_col.blue  = (unsigned short) (B * 65535);
    X11_col.flags = (char) (DoRed | DoGreen | DoBlue);
    XStoreColor (dpy, cmap, &X11_col);
}

void pic_cur (Coor x1, Coor x2, Coor y1, Coor y2)
{
    ggSetColor (Cur_nr);
    set_c_wdw (PICT);
    pict_rect ((float) x1, (float) x2, (float) y1, (float) y2);
    flush_pict ();
}

void clear_curs ()
{
    d_fillst (FILL_SOLID);
    ggSetColor (Cur_nr);
    disp_mode (ERASE);

    set_c_wdw (PICT);
    paint_box (XL, XR, YB, YT);
    disp_mode (TRANSPARENT);
}

void paint_box (float x1, float x2, float y1, float y2)
{
    int dx1, dy1, dx2, dy2, dw, dh, dm, x, y;

    if (stop_drawing () == TRUE) return;

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
	}
	else dw = DMAX;
    }
    if (dh > DMAX) {
	if (dy1 < 0) {
	    dh += dy1 + 1; dy1 = -1;
	    if (dh > DMAX) dh = DMAX;
	}
	else dh = DMAX;
    }

    if (c_style == FILL_HOLLOW) {
	XDrawRectangle (dpy, cwin, pgc, dx1, dy1, (unsigned int) dw, (unsigned int) dh);
    }
    else if (c_style == FILL_HASHED ||
	     c_style == FILL_HASHED12B ||
	     c_style == FILL_HASHED25 ||
	     c_style == FILL_HASHED25B ||
	     c_style == FILL_HASHED50 ||
	     c_style == FILL_HASHED50B) {

	if (c_style == FILL_HASHED12B ||
	    c_style == FILL_HASHED25B ||
	    c_style == FILL_HASHED50B) { /* PATRICK: draw a box around it */
	    XDrawRectangle (dpy, cwin, pgc, dx1, dy1, (unsigned int) dw, (unsigned int) dh);
	}

	if (--dw < 0) dw = 0;
	if (--dh < 0) dh = 0;
	dx2 = dx1 + dw;
	dy2 = dy1 + dh;
	dm = dy2 % Filld - dx1 % Filld;
/*
 * Patrick: this is the special case where the instances are
 * drawn in a lower intensity. To obtain _much_ better transparency
 * of the design, we give the fill styles in certain layers a specific offset.
 * Notice that this hack is specific for the c3tu process.
 * It won't crash on other processes, but the results are unclear.
 */
	if (c_style == FILL_HASHED25) {
	    switch (ggcurid) {
	    case 4:  /* contacts in same mask as in, and draw box.. */
	    case 5:
	    case 6:
		XDrawRectangle (dpy, cwin, pgc, dx1, dy1, (unsigned int) dw, (unsigned int) dh);
	    case 2:  /* in */
		dm -= 1;
		break;
	    case 3:  /* ins */
	    case 7:  /* cos */
		break;  /* nothing */
	    default: /* ps, od */
		dm -= 3;
	    }
	}
	else {
	    if (ggcurid == 1) dm--; /* shift fill pattern of od to make it visible through ins */
	}

	while (dm < 0) dm += Filld;
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
    else {
	if (dw < 1) dw = 1;
	if (dh < 1) dh = 1;
	XFillRectangle (dpy, cwin, pgc, dx1, dy1, (unsigned int) dw, (unsigned int) dh);
    }
}

void pict_rect (float x1, float x2, float y1, float y2)
{
    int dx1, dx2, dy1, dy2, dw, dh;

    if (stop_drawing () == TRUE) return;

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
	}
	else dw = DMAX;
    }
    if (dh > DMAX) {
	if (dy1 < 0) {
	    dh += dy1 + 1; dy1 = -1;
	    if (dh > DMAX) dh = DMAX;
	}
	else dh = DMAX;
    }

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
    int x, y, xs, ys, dm, dh, h1, h2;
    int dx0, dx1, dx2, dx3, dy0, dy1, dy2, dy3;
    Coor tmp = 0;
    register int i, n, np;

    if (c_style == FILL_HASHED || c_style == FILL_HASHED12B ||
	     c_style == FILL_HASHED25 || c_style == FILL_HASHED25B ||
	     c_style == FILL_HASHED50 || c_style == FILL_HASHED50B)
       {
    /* Test for rotate/mirror */
    if (line_y[0] == line_y[3] && line_y[1] == line_y[2]) { /* R0 || R180 */
	if (line_y[2] < line_y[0]) { /* MX */
	    tmp = line_x[0]; line_x[0] = line_x[1]; line_x[1] = tmp;
	    tmp = line_y[0]; line_y[0] = line_y[1]; line_y[1] = tmp;
	    tmp = line_x[3]; line_x[3] = line_x[2]; line_x[2] = tmp;
	    tmp = line_y[3]; line_y[3] = line_y[2]; line_y[2] = tmp;
	}
	if (line_x[3] < line_x[0] || line_x[2] < line_x[1]) { /* MY */
	    tmp = line_x[0]; line_x[0] = line_x[3]; line_x[3] = tmp;
	    tmp = line_y[0]; line_y[0] = line_y[3]; line_y[3] = tmp;
	    tmp = line_x[1]; line_x[1] = line_x[2]; line_x[2] = tmp;
	    tmp = line_y[1]; line_y[1] = line_y[2]; line_y[2] = tmp;
	}
	tmp = 0;
    }
    else if (line_y[0] == line_y[1] && line_y[3] == line_y[2]) { /* R0 || R180 */
	if (line_y[2] < line_y[0]) { /* MX */
	    tmp = line_x[0]; line_x[0] = line_x[3]; line_x[3] = tmp;
	    tmp = line_y[0]; line_y[0] = line_y[3]; line_y[3] = tmp;
	    tmp = line_x[1]; line_x[1] = line_x[2]; line_x[2] = tmp;
	    tmp = line_y[1]; line_y[1] = line_y[2]; line_y[2] = tmp;
	}
	if (line_x[1] < line_x[0] || line_x[2] < line_x[3]) { /* MY */
	    tmp = line_x[0]; line_x[0] = line_x[1]; line_x[1] = tmp;
	    tmp = line_y[0]; line_y[0] = line_y[1]; line_y[1] = tmp;
	    tmp = line_x[3]; line_x[3] = line_x[2]; line_x[2] = tmp;
	    tmp = line_y[3]; line_y[3] = line_y[2]; line_y[2] = tmp;
	}
	tmp = 1;
    }
    else if (line_x[0] == line_x[3] && line_x[1] == line_x[2]) { /* R90 || R270 */
	if (line_y[2] < line_y[1] || line_y[3] < line_y[0]) { /* MX */
	    tmp = line_x[0]; line_x[0] = line_x[3]; line_x[3] = tmp;
	    tmp = line_y[0]; line_y[0] = line_y[3]; line_y[3] = tmp;
	    tmp = line_x[1]; line_x[1] = line_x[2]; line_x[2] = tmp;
	    tmp = line_y[1]; line_y[1] = line_y[2]; line_y[2] = tmp;
	}
	if (line_x[3] < line_x[2] || line_x[0] < line_x[1]) { /* MY */
	    tmp = line_x[0]; line_x[0] = line_x[1]; line_x[1] = tmp;
	    tmp = line_y[0]; line_y[0] = line_y[1]; line_y[1] = tmp;
	    tmp = line_x[3]; line_x[3] = line_x[2]; line_x[2] = tmp;
	    tmp = line_y[3]; line_y[3] = line_y[2]; line_y[2] = tmp;
	}
	tmp = 2;
    }
    else {
	if (line_y[2] < line_y[3] || line_y[1] < line_y[0]) { /* MX */
	    tmp = line_x[0]; line_x[0] = line_x[1]; line_x[1] = tmp;
	    tmp = line_y[0]; line_y[0] = line_y[1]; line_y[1] = tmp;
	    tmp = line_x[3]; line_x[3] = line_x[2]; line_x[2] = tmp;
	    tmp = line_y[3]; line_y[3] = line_y[2]; line_y[2] = tmp;
	}
	if (line_x[1] < line_x[2] || line_x[0] < line_x[3]) { /* MY */
	    tmp = line_x[0]; line_x[0] = line_x[3]; line_x[3] = tmp;
	    tmp = line_y[0]; line_y[0] = line_y[3]; line_y[3] = tmp;
	    tmp = line_x[1]; line_x[1] = line_x[2]; line_x[2] = tmp;
	    tmp = line_y[1]; line_y[1] = line_y[2]; line_y[2] = tmp;
	}
	tmp = 3;
    }
    }

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
    if (c_style == FILL_HOLLOW) {
	point[np].x = point[0].x;
	point[np].y = point[0].y;
	++np;
	XDrawLines (dpy, cwin, pgc, point, np, CoordModeOrigin);
    }
    else if (c_style == FILL_HASHED || c_style == FILL_HASHED12B ||
	     c_style == FILL_HASHED25 || c_style == FILL_HASHED25B ||
	     c_style == FILL_HASHED50 || c_style == FILL_HASHED50B) {

	if (c_style == FILL_HASHED12B || c_style == FILL_HASHED25B ||
		c_style == FILL_HASHED50B) { /* PATRICK: draw box around it */
	    h1 = point[np].x;
	    h2 = point[np].y;
	    point[np].x = point[0].x;
	    point[np].y = point[0].y;
	    XDrawLines (dpy, cwin, pgc, point, (np+1), CoordModeOrigin);
	    point[np].x = h1;
	    point[np].y = h2;
	}

	dx0 = point[0].x;
	dy0 = point[0].y;
	if (tmp % 2) { /* swap points */
	    i = np - 1;
	    dx1 = point[i].x; point[i].x = point[1].x; point[1].x = dx1;
	    dy1 = point[i].y; point[i].y = point[1].y; point[1].y = dy1;
	}
	else {
	    dx1 = point[1].x;
	    dy1 = point[1].y;
	}
	if (tmp > 1) { /* R90 || R270 */
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
		dm = dy1 % Filld - dx1 % Filld;
		if (c_style == FILL_HASHED25) {
		    switch (ggcurid) {
		    case 2:  /* in */
		    case 4:
		    case 5:
		    case 6:
			dm -= 1;
			break;
		    case 3:  /* ins */
		    case 7:  /* cos */
			break;  /* nothing */
		    default: /* ps, od */
			dm -= 3;
		    }
		}
		while (dm < 0) dm += Filld;
		x = dx1 + dm;
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
		dm = dy1 % Filld - dx1 % Filld;
		if (c_style == FILL_HASHED25) {
		    switch (ggcurid) {
		    case 2:  /* in */
		    case 4:
		    case 5:
		    case 6:
			dm -= 1;
			break;
		    case 3:  /* ins */
		    case 7:  /* cos */
			break;  /* nothing */
		    default: /* ps, od */
			dm -= 3;
		    }
		}
		while (dm < 0) dm += Filld;
		y = dy1 - dm;
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
		dm = dy0 % Filld - dx0 % Filld;
		if (c_style == FILL_HASHED25) {
		    switch (ggcurid) {
		    case 2:  /* in */
		    case 4:
		    case 5:
		    case 6:
			dm -= 1;
			break;
		    case 3:  /* ins */
		    case 7:  /* cos */
			break;  /* nothing */
		    default: /* ps, od */
			dm -= 3;
		    }
		}
		while (dm < 0) dm += Filld;
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
	else {
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
		dm = dy0 % Filld - dx1 % Filld;
		if (c_style == FILL_HASHED25) {
		    switch (ggcurid) {
		    case 2:  /* in */
		    case 4:
		    case 5:
		    case 6:
			dm -= 1;
			break;
		    case 3:  /* ins */
		    case 7:  /* cos */
			break;  /* nothing */
		    default: /* ps, od */
			dm -= 3;
		    }
		}
		while (dm < 0) dm += Filld;
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
		dm = dy0 % Filld - dx1 % Filld;
		if (c_style == FILL_HASHED25) {
		    switch (ggcurid) {
		    case 2:  /* in */
		    case 4:
		    case 5:
		    case 6:
			dm -= 1;
			break;
		    case 3:  /* ins */
		    case 7:  /* cos */
			break;  /* nothing */
		    default: /* ps, od */
			dm -= 3;
		    }
		}
		while (dm < 0) dm += Filld;
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
		dm = dy0 % Filld - dx0 % Filld;
		if (c_style == FILL_HASHED25) {
		    switch (ggcurid) {
		    case 2:  /* in */
		    case 4:
		    case 5:
		    case 6:
			dm -= 1;
			break;
		    case 3:  /* ins */
		    case 7:  /* cos */
			break;  /* nothing */
		    default: /* ps, od */
			dm -= 3;
		    }
		}
		while (dm < 0) dm += Filld;
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
    else {
	XFillPolygon (dpy, cwin, pgc, point, np, Convex, CoordModeOrigin);
    }
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

void toggle_tracker ()
{
    tracker_enabled = !tracker_enabled;
    if (tracker_enabled) {
    	XMapWindow (dpy, trwin);
    }
    else {
	XUnmapWindow (dpy, trwin);
    }
}

void switch_tracker (int show_it)
{
    if (tracker_enabled) {
	if (show_it == FALSE) toggle_tracker ();
    }
    else {
	if (show_it == TRUE) toggle_tracker ();
    }
}

static void transf (int x, int y, Coor *x_grid, Coor *y_grid)
{
    float wx = p_wdw -> wxmin;
    float wy = p_wdw -> wymax;
    float x_p, y_p;

    x_p = wx + (float) x / (p_wdw -> vxmax / (p_wdw -> wxmax - wx));
    y_p = wy - (float) y / (p_wdw -> vymax / (wy - p_wdw -> wymin));
    *x_grid = (Coor)(Round (x_p / (float)QUAD_LAMBDA));
    *y_grid = (Coor)(Round (y_p / (float)QUAD_LAMBDA));
}

static Cursor pcur; /* pict cursor */
static Cursor mcur; /* menu cursor */
static Cursor tcur; /* text cursor */

void set_text_cursor (int text_mode)
{
    if (text_mode) {
	XDefineCursor (dpy, pwin, tcur);
	XDefineCursor (dpy, mwin, tcur);
	XDefineCursor (dpy, lwin, tcur);
	XDefineCursor (dpy, twin, tcur);
    }
    else {
	XDefineCursor (dpy, pwin, pcur);
	XDefineCursor (dpy, mwin, mcur);
	XDefineCursor (dpy, lwin, mcur);
	XDefineCursor (dpy, twin, pcur);
    }
}

void init_graph ()
{
    char *fontname;
    char *cp;
    char *option;
    XSizeHints hints;
    XWMHints wmhints;
    int      s_nr; /* screen number */
    register int i;
    char B_str[200], I_str[200];

    if (!(dpy = XOpenDisplay (DisplayName))) Xerror (1, DisplayName);

    if ((option = XGetDefault (dpy, argv0, "BorderWidth")))
	BW = atoi (option);
    if (BW <= 0) BW = 1;

    if (!(fontname = XGetDefault (dpy, argv0, "FontName")))
	fontname = "fixed";

    if (!geometry) geometry = XGetDefault (dpy, argv0, "GeoMetry");
    s_nr = DefaultScreen (dpy);

    pcur = XCreateFontCursor (dpy, XC_dotbox);
    mcur = XCreateFontCursor (dpy, XC_hand2);
    tcur = XCreateFontCursor (dpy, XC_xterm);

#ifndef CACDCMAP
    if (BlackPixel (dpy, s_nr)) { /* inverse fg cursor */
	XRecolorCursor (dpy, pcur, &c_bg, &c_fg);
	XRecolorCursor (dpy, mcur, &c_bg, &c_fg);
	XRecolorCursor (dpy, tcur, &c_bg, &c_fg);
    }
    else {
#endif
	XRecolorCursor (dpy, pcur, &c_fg, &c_bg);
	XRecolorCursor (dpy, mcur, &c_fg, &c_bg);
	XRecolorCursor (dpy, tcur, &c_fg, &c_bg);
#ifndef CACDCMAP
    }
#endif

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
#ifdef CACDCMAP
    MALLOCN (aIds, int, ggMaxIds);
#endif

    init_mskcol ();

    Textnr = ggGetIdForColor ("white");
    Gridnr = ggGetIdForColor ("white");
    DRC_nr = ggGetIdForColor ("white");
    Cur_nr = DRC_nr;

    /* NR_all is set in init_mskcol() */
    if (NR_all != DRC_nr + 1) Xerror (5, "");

    Backgr = ggFindIdForColor ("black");
    Yellow = ggFindIdForColor ("yellow");
    RGBids[0] = ggFindIdForColor ("red");
    RGBids[1] = ggFindIdForColor ("green");
    RGBids[2] = ggFindIdForColor ("blue");
    Black = ggGetColorIndex (Backgr);
    White = ggGetColorIndex (Gridnr);

    mW = N_MENU * cW + 2 * cD;
    tW = 7 * mW;
    i = maxW - 2*mW - 2*BW;
    if (tW > i) tW = i;

    mH = N_CMDS * tH;
    i = (float) (mW + tW) * (float) DH / (float) DW;
    if (mH < i) mH = i;
    i = maxH;
    if (mH > i) mH = i;

    hints.x = hints.y = 0;
    hints.width  = 2*mW + 2*BW + tW;
    hints.height = mH;
    hints.min_width  = 7*mW;
    hints.min_height = 6*mW;
    hints.flags = PPosition | PSize | PMinSize;

    /* PATRICK: default window bigger... */
    hints.width *= 1.3;
    hints.height *= 1.3;
    if (geometry) {
	int rv = XParseGeometry (geometry,
	    &hints.x, &hints.y, (unsigned*)&hints.width, (unsigned*)&hints.height);
	if (rv & (WidthValue | HeightValue)) {
	    hints.flags |= USSize;
	    if (hints.width < hints.min_width)
		hints.width = hints.min_width;
	    if (hints.height < hints.min_height)
		hints.height = hints.min_height;
	}
	if (rv & (XValue | YValue)) {
	    hints.flags |= USPosition;
	    if (rv & XNegative) hints.x += maxW - hints.width;
	    if (rv & YNegative) hints.y += maxH - hints.height;
	}
	if (hints.x < 0) hints.x = 0;
	if (hints.y < 0) hints.y = 0;
    }

    rwin = RootWindow (dpy, s_nr);
    rwin = XCreateSimpleWindow (dpy, rwin, hints.x, hints.y, hints.width, hints.height, BW, White, Black);
    if (!rwin) Xerror (3, DisplayName);

    /*
     * Make a nice title in bar:
     */
    if (gethostname (HOST, (size_t) 100) == -1) strcpy (HOST, "???");
    /* remove domain extension from hostname */
    for (cp = HOST; *cp && *cp != '.'; ++cp); *cp = 0;
    CWD = dmproject -> dmpath;
    sprintf (B_str, "Welcome to %s in project %s, running on %s", argv0, CWD, HOST);
    sprintf (I_str, "%s  %s on %s", argv0, CWD, HOST);
 /* XSetStandardProperties (dpy, rwin, B_str, I_str, None, NULL, 0, &hints); */
    XSetStandardProperties (dpy, rwin, B_str, I_str, None, &argv0, 1, &hints);

    /* strip leading path from cwd name (for later usage) */
    if ((cp = strrchr (CWD, '/'))) CWD = ++cp;

    wmhints.input = TRUE;
    wmhints.flags = InputHint;
    XSetWMHints (dpy, rwin, &wmhints);

    theXclasshint.res_name = "seadali";
    theXclasshint.res_class = "Seadali";
    XSetClassHint (dpy, rwin, &theXclasshint);

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
    std_mask = ExposureMask | KeyPressMask;
    XSelectInput (dpy, pwin, std_mask);
    XSelectInput (dpy, mwin, std_mask);
    XSelectInput (dpy, lwin, std_mask);
    XSelectInput (dpy, twin, std_mask);
    XSelectInput (dpy, trwin, ExposureMask);

    set_text_cursor (0);

#ifdef SET_COLORS
    if (nr_planes > 1) {
	cmap = XCreateColormap (dpy, rwin, DefaultVisual (dpy, s_nr), AllocAll);
	XSetWindowColormap (dpy, rwin, cmap);
	StoreColors ();
    }
#endif

#ifdef CACDCMAP
    if (!CreateGCforCmap (rwin, &pgc)) CacdCmapError (argv0, 1);
#else
    pgc = XCreateGC (dpy, rwin, 0, 0);
#endif
    XSetFont (dpy, pgc, font -> fid);

    if (d_apollo) {
#ifdef CACDCMAP
	if (!CreateGCforCmap (rwin, &tgc)) CacdCmapError (argv0, 1);
#else
	tgc = XCreateGC (dpy, rwin, 0, 0);
#endif
	XSetFont (dpy, tgc, font -> fid);
    }
    else tgc = pgc;

#ifdef CACDCMAP
    if (!CreateGCforCmap (rwin, &cgc)) CacdCmapError (argv0, 1);
    SetForegroundFromIds (cgc, RGBids, 3);
#else
    cgc = XCreateGC (dpy, rwin, 0, 0);
    White = ggGetColorsIndex (RGBids, 3);
    XSetForeground (dpy, cgc, White);
#endif
    XSetFunction (dpy, cgc, GXxor);

    /* tracker graphic content */
#ifdef CACDCMAP
    if (!CreateGCforCmap (rwin, &trgc)) CacdCmapError (argv0, 1);
#else
    trgc = XCreateGC (dpy, rwin, 0, 0);
#endif
    XSetFont (dpy, trgc, font -> fid);
    XSetForeground (dpy, trgc, White);
    XSetBackground (dpy, trgc, Black);
    XSetFunction   (dpy, trgc, GXcopy);

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

void set_backing_store (int doit)
{
    XSetWindowAttributes attributes;

    if (doit == TRUE)
	attributes.backing_store = Always;
    else
	attributes.backing_store = NotUseful;
    XChangeWindowAttributes (dpy, rwin, CWBackingStore, &attributes);
    XChangeWindowAttributes (dpy, pwin, CWBackingStore, &attributes);
}

/*
 * Set title bar string of window.
 */
void set_titlebar (char *ownstring)
{
    char B_str[256], I_str[200], *cp;
    XSizeHints hints;
    int i;

    /*
    * dunno what this is for...
    */
    hints.x = hints.y = 0;
    hints.width  = 600;
    hints.height = 300;
    hints.min_width  = 200;
    hints.min_height = 200;
    hints.flags = PPosition | PSize | PMinSize;

    if (!ownstring || !*ownstring) {
	if (!(cp = cellstr) || !*cp) cp = "---";
	sprintf (I_str, "%s  %s in %s on %s", argv0, cp, CWD, HOST);
	sprintf (B_str, ">> %s >>   Layout: %s  Expansion: %d  Project: %s  Host: %s",
		argv0, cp, exp_level, CWD, HOST);
    }
    else {
	sprintf (I_str, "%s  %s", argv0, ownstring);
	sprintf (B_str, ">> %s >>   %s", argv0, ownstring);
    }

    XSetStandardProperties (dpy, rwin, B_str, I_str, None, NULL, 0, &hints);
}

void exit_graph ()
{
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

void ch_siz (float *awidth, float *aheight)
{
    if (!c_wdw) fatal (1, "ch_siz");
    *awidth  = (float) cW / c_sX;
    *aheight = (float) cH / c_sY;
}

void d_text (float x, float y, char *str)
{
    int dx1, dy1;
    int strLen;

    if (stop_drawing () == TRUE) return;

    strLen = strlen (str);

    maxDisplayStrLen = Max (maxDisplayStrLen, strLen);

    if (cwin == twin) {
	dx1 = cD;
	dy1 = cH;
    }
    else {
	dx1 = TRANSF_X (x);
	dy1 = TRANSF_Y (y);
    }
    if (cwin == pwin) { dx1 += cD; dy1 -= cD; }
    else if (cwin == mwin) dy1 -= cD;
    XDrawString (dpy, cwin, tgc, dx1, dy1, str, strLen);
}

/*
 * Patrick: draw text vertically
 */
void v_text (float x, float y, char *str)
{
    int dx1, dy1;
    int strLen;

    if (stop_drawing () == TRUE) return;

    dx1 = TRANSF_X (x);
    dy1 = TRANSF_Y (y);
    if (cwin == pwin) { dx1 += cD; dy1 -= cD; }
    else if (cwin == mwin) dy1 -= cD;

    for (strLen = strlen (str); strLen > 0; strLen--) {
	XDrawString (dpy, cwin, tgc, dx1, dy1, &str[strLen-1], 1);
	dy1 -= (0.8 * cH);
    }
}

void d_fillst (int fs) /* set fill style */
{
    if (fs == c_style) return;

    switch (fs) {
    case FILL_HOLLOW:
	break;
    case FILL_SOLID:
	/* XSetFillStyle (dpy, pgc, FillSolid); */
	break;
    case FILL_HASHED:
    case FILL_HASHED12B:  /* 12.5 % fill */
	Filld = 8;
	Filld2 = 4;
	break;
    case FILL_HASHED25:
    case FILL_HASHED25B:  /* 25 % fill */
	Filld = 4;
	Filld2 = 2;
	break;
    case FILL_HASHED50:
    case FILL_HASHED50B:  /* 50 % fill */
	Filld = 2;
	Filld2 = 1;
	break;
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

int draw_cross (Coor xCoor, Coor yCoor)
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
	return (TRUE);
    }
    return (FALSE);
}

/*
** Get cursor location.
** Returned after the mouse button is pressed.
*/
int get_loc (float *x_p, float *y_p, int echo)
{
    unsigned long emask;
    emask = std_mask | ButtonPressMask;
    if (echo >= 4) {
	if (echo > 5) echo = 5;
	cecho = echo;
	XSelectInput (dpy, pwin, emask | PointerMotionMask);
    }
    else XSelectInput (dpy, pwin, emask | PointerMotionMask);
    XSelectInput (dpy, mwin, emask);
    XSelectInput (dpy, lwin, emask);
    XSelectInput (dpy, twin, emask);
#ifdef DEBUG_EVENTS
    PE "-- WaitForEvent ButtonPress\n");
#endif
    WaitForEvent (ButtonPress);
    XSelectInput (dpy, pwin, std_mask | PointerMotionMask);
    XSelectInput (dpy, mwin, std_mask);
    XSelectInput (dpy, lwin, std_mask);
    XSelectInput (dpy, twin, std_mask);
    cecho = 0;

    if (event.type == KeyPress && !in_flight_flag) { /* quit */
	switch (*kc) {
	case 'Q':
	case 'q':
	    ask_quit (1);
	    *y_p = -2; /* redraw menu */
	    return (MENU);
	case 'N':
	case 'n':
	    *y_p = 0; /* return "no" */
	    return (MENU);
	case 'Y':
	case 'y':
	    *y_p = 1; /* return "yes" */
	    return (MENU);
	}
    }

    if (looks_like_the_sea_child_died (FALSE) == TRUE) return (SEA_CHILD_DIES);

    if (ewin == twin) return (TEXT);

    if (cwin == ewin) {
	*x_p = c_wX + (float) ret_c_x / c_sX;
	*y_p = c_wY - (float) ret_c_y / c_sY;
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
    }

    if (ewin == lwin) return (LAYS);

    if (ccur) remove_cursor ();

    if (ewin == mwin) return (MENU);

    return (PICT);
}

void fix_loc (float xx, float yy)
{

    set_c_wdw (PICT);

    cfixx = xx;
    cfixy = yy;
    cxs = TRANSF_X (xx);
    cys = TRANSF_Y (yy);
}

static void ggInit (int planes)
{
    int max;
    if (planes > 5) planes = 5;
    if (planes < 3) planes = 3;
    nr_planes = DisplayPlanes (dpy, DefaultScreen (dpy));
    if (nr_planes > 8) nr_planes = 8;
#ifdef CACDCMAP
    if (nr_planes == 8) {
	/* nr_planes is used by various commands to decide on
	 * drawing in COMPLEMENT mode.  This by itself is wrong.
	 * This should depend on whether 'Cur_nr' has a bitplane
	 * for itself.  A more advanced solution has to be
	 * constructed to do proper checking on bitplanes and redrawing
	 * in case of (1) no bitplane for itself, and (2) window operations.
	 * For now we force using COMPLEMENT mode, thereby
	 * prohibiting improper (rigurous) erase operations.
	 */
	nr_planes = 7;
    }
    if (!InitCacdCmap (dpy, DefaultScreen (dpy))) CacdCmapError (argv0, 1);
#endif
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

static int ggGetIdForColor (char *color)
{
#ifndef CACDCMAP
    static int first = 1;
    static int w_index;
#endif
    register int i;

    for (i = 0; i < ggmaxcolors; ++i) {
	if (!strcmp (color, ggcolors[i])) goto ok;
    }
    PE "%s: ggGetIdForColor: unknown color '%s'\n", argv0, color);
    exit (1);
ok:
    if (ggidnr >= ggMaxIds) {
	PE "%s: ggGetIdForColor: too many ids\n", argv0);
	exit (1);
    }
    ggconame[ggidnr] = ggcolors[i];

#ifdef CACDCMAP
    if (!CoupleIdToColor (ggidnr, color)) CacdCmapError (argv0, 1);
#else /*  NOT CACDCMAP */
    if (i == 7) { /* white */
	if (first) { w_index = ggmaxcolors; first = 0; }
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
#endif /* NOT CACDCMAP */
    return (ggidnr++);
}

static int ggFindIdForColor (char *color)
{
    register int i;
    for (i = 0; i < NR_lay; ++i) {
	if (ggTestColor (i, color)) return (i);
    }
    return (ggGetIdForColor (color));
}

static int ggGetColorIndex (int color_id)
{
#ifdef CACDCMAP
    unsigned long ci;
#endif

    if (color_id < 0 || color_id >= ggidnr) return (0);
#ifdef CACDCMAP
    GetPixelFromId (color_id, &ci);
    return ((int) ci);
#else
    return (ggcindex[color_id]);
#endif
}

static int ggGetColorsIndex (int col_ids[], int nr_ids)
{
#ifdef CACDCMAP
    unsigned long ci;
#else
    register int ci, i;
#endif

#ifdef CACDCMAP
    GetPixelFromIds (col_ids, nr_ids, &ci);
    return ((int) ci);
#else
    for (ci = i = 0; i < nr_ids; ++i) ci |= ggGetColorIndex (col_ids[i]);
    return (ci);
#endif
}

int ggSetErasedAffected (int eIds[], int p_arr[], int nr_all)
{
#ifdef CACDCMAP
    int nr_aIds = ggMaxIds;
#else
    register int ci;
#endif
    register int i, j, nr_eIds;

    nr_eIds = 0;
    for (i = 0; i < NR_lay; ++i) {
	if (p_arr[i] == ERAS_DR) {
	    if (dom_arr[i] && (Draw_dominant || dom_arr[i] != 2)) {
		pict_all (DRAW);
		return (nr_all);
	    }
	    eIds[nr_eIds++] = i;
	}
    }
    for (; i < nr_all; ++i) {
	if (p_arr[i] == ERAS_DR) eIds[nr_eIds++] = i;
    }

    if (!nr_eIds) return (0);

#ifdef CACDCMAP
    GetAffectedIds (eIds, nr_eIds, aIds, &nr_aIds);
    for (j = i = 0; i < nr_aIds; ++i)
	if (aIds[i] < nr_all) { ++j; p_arr[aIds[i]] = DRAW; }
#else
    ci = ggGetColorsIndex (eIds, nr_eIds);
    for (j = i = 0; i < nr_all; ++i)
	if (ci & ggGetColorIndex (i)) { ++j; p_arr[i] = DRAW; }
#endif
    if (j == nr_all) return (nr_all);
    return (nr_eIds);
}

int ggGetColor ()
{
    return (ggcurid);
}

#ifndef CACDCMAP
static void _ggSetColorByIndex (int ci)
{
    XSetForeground (dpy, pgc, (unsigned long) ci);
    if (d_apollo) {
	XSetForeground (dpy, tgc, (unsigned long) ci);
	if (ci > 0)
	    XSetPlaneMask (dpy, tgc, (unsigned long) ci);
	else
	    XSetPlaneMask (dpy, tgc, (unsigned long) (maxindex - 1));
    }
}
#endif /* NOT CACDCMAP */

void ggSetColor (int color_id)
{
    if (color_id < 0 || color_id == ggcurid || color_id >= ggidnr) return;
#ifdef CACDCMAP
    SetForegroundFromId (pgc, color_id);
    if (d_apollo) SetForegroundFromId (tgc, color_id);
#else
    _ggSetColorByIndex (ggGetColorIndex (color_id));
#endif
    ggcurid = color_id;
}

void ggSetColors (int col_ids[], int nr_ids)
{
    ggcurid = -1;
#ifdef CACDCMAP
    SetForegroundFromIds (pgc, col_ids, nr_ids);
    if (d_apollo) SetForegroundFromIds (tgc, col_ids, nr_ids);
#else
    _ggSetColorByIndex (ggGetColorsIndex (col_ids, nr_ids));
#endif
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

void ggClearArea (float xl, float xr, float yb, float yt)
{
    int dx1, dy1, dx2, dy2;
    unsigned int dw, dh;
    dx1 = (int) TRANSF_X (xl);
    dx2 = (int) TRANSF_X (xr);
    dy1 = (int) TRANSF_Y (yb);
    dy2 = (int) TRANSF_Y (yt);
    if (dx1 > dx2) { dw = dx1 - dx2; dx1 = dx2; }
    else dw = dx2 - dx1;
    if (dy1 > dy2) { dh = dy1 - dy2; dy1 = dy2; }
    else dh = dy2 - dy1;
    XClearArea (dpy, cwin, dx1, dy1, dw, dh, False);
}

void ggEraseArea (float xl, float xr, float yb, float yt)
{
    disp_mode (ERASE);
    paint_box (xl, xr, yb, yt);
}

void d_grid (Coor wxl, Coor wxr, Coor wyb, Coor wyt, Coor sp)
{
    int dx1, dy1;
    Coor x, y;

    /* Patrick: added drawing of the image grid.. */
    if (ImageMode == TRUE) {
	d_grid_image (wxl, wxr, wyb, wyt);
	return;
    }

    for (x = wxl - wxl % sp; x <= wxr; x += sp) {
	dx1 = TRANSF_X (x);
	for (y = wyb - wyb % sp; y <= wyt; y += sp) {
	    dy1 = TRANSF_Y (y);
	    XDrawPoint (dpy, cwin, pgc, dx1, dy1);
	}
    }
}

void d_snapgrid (Coor wxl, Coor wxr, Coor wyb, Coor wyt, Coor sp, Coor xoff, Coor yoff)
{
    Coor x, y;
    double xst, yst, cr_size, x1, x2, y1, y2;
    int dx1, dy1, dx2, dy2;

    for (x = wxl - wxl % sp + xoff % sp - sp; x <= wxr; x += sp) {
	for (y = wyb - wyb % sp + yoff % sp - sp; y <= wyt; y += sp) {

	    xst = (double) x;
	    yst = (double) y;
	    cr_size = (XR - XL) / 240.0;

	    if (xst + cr_size >= XL && xst - cr_size <= XR && yst + cr_size >= YB && yst - cr_size <= YT) {
		/* inside display window */
		x1 = xst - cr_size;
		x2 = xst + cr_size;
		y1 = yst - cr_size;
		y2 = yst + cr_size;
		dx1 = TRANSF_X (x1);
		dx2 = TRANSF_X (x2);
		dy1 = TRANSF_Y (y1);
		dy2 = TRANSF_Y (y2);
		XDrawLine (dpy, cwin, pgc, dx1, dy1, dx2, dy2);
		XDrawLine (dpy, cwin, pgc, dx1, dy2, dx2, dy1);
	    }
	}
    }
}

void d_line (float x1, float y1, float x2, float y2)
{
    int dx1, dy1, dx2, dy2, x, y, n;

    if (stop_drawing () == TRUE) return;

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

/*
 * called to redraw picture after a key event
 */
static void kpicture ()
{
    picture ();
    ccur = 0; /* do not erase cursor */
}

static int do_keypress ()
{
    Coor piwl, piwr, piwb, piwt; /* picture drawing window */
    KeySym ks;
    int lay;

    /* flush all keypresses, only keep last */
    while (XCheckTypedEvent (dpy, KeyPress, &event) == TRUE);
    ks = XKeycodeToKeysym (dpy, event.xkey.keycode, 0);

    /* perform windowing actions attached to keypress */
    piwl = p_wdw -> wxmin; piwr = p_wdw -> wxmax;
    piwb = p_wdw -> wymin; piwt = p_wdw -> wymax;

    /* panning */
    if (ks == XK_Up) {
	center_w ((Coor) (piwr + piwl)/2, (Coor) piwt);
	kpicture ();
	return (0);
    }
    if (ks == XK_Down) {
	center_w ((Coor) (piwr + piwl)/2, (Coor) piwb);
	kpicture ();
	return (0);
    }
    if (ks == XK_Right) {
	center_w ((Coor) piwr, (Coor) (piwt + piwb)/2);
	kpicture ();
	return (0);
    }
    if (ks == XK_Left) {
	center_w ((Coor) piwl, (Coor) (piwt + piwb)/2);
	kpicture ();
	return (0);
    }
    if (ks == XK_Prior) { /* previous */
	prev_w ();
	kpicture ();
	return (0);
    }
    if (ks == XK_Next) { /* redraw */
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
	kpicture ();
	return (0);
    }
    if (ks == XK_Home) { /* bbx */
	bound_w ();
	kpicture ();
	return (0);
    }
    if (ks == XK_Select) { /* center at cursor position */
	old_sx = sx;
	old_sy = sy;
	transf (event.xmotion.x, event.xmotion.y, &sx, &sy);
	center_w ((Coor) sx*4, (Coor) sy*4);
	kpicture ();
	return (0);
    }

    if (ks == (KeySym)NULL || ks == NoSymbol ||
	ks == XK_Shift_L || ks == XK_Shift_R ||
	ks == XK_Control_L || ks == XK_Control_R ||
	ks == XK_Caps_Lock || ks == XK_Shift_Lock ||
	ks == XK_Meta_L || ks == XK_Meta_R ||
	ks == XK_Alt_L || ks == XK_Alt_R) return (0);

    XLookupString (&event.xkey, kc, 1, NULL, NULL);
    lay = *kc;

    if (isdigit (lay)) { /* expansion level */
	lay -= '0';
	if (lay == 0) lay = 16; /* maximum */
	expansion (lay);
	set_titlebar (NULL);
	kpicture ();
	return (0);
    }

    switch (lay) { /* process key code */
    case 'i':
    case 'I':
    case '+': /* zoom in factor 2 */
	curs_w ((Coor) (piwl + ((piwr - piwl)/4)),
		(Coor) (piwr - ((piwr - piwl)/4)),
		(Coor) (piwb + ((piwt - piwb)/4)),
		(Coor) (piwt - ((piwt - piwb)/4)));
	kpicture ();
	break;
    case 'o':
    case 'O':
    case '-': /* zoom out factor 2 */
	curs_w ((Coor) (piwl - ((piwr - piwl)/4)),
		(Coor) (piwr + ((piwr - piwl)/4)),
		(Coor) (piwb - ((piwt - piwb)/4)),
		(Coor) (piwt + ((piwt - piwb)/4)));
	kpicture ();
	break;
    case 'b':
    case 'B':
	/* bbx */
	bound_w ();
	kpicture ();
	break;
    case 'h': /* vi h = left */
	center_w ((Coor) piwl, (Coor) (piwt + piwb)/2);
	kpicture ();
	break;
    case 'l': /* vi l = right */
	center_w ((Coor) piwr, (Coor) (piwt + piwb)/2);
	kpicture ();
	break;
    case 'j': /* vi j = down */
	center_w ((Coor) (piwr + piwl)/2, (Coor) piwb);
	kpicture ();
	break;
    case 'k': /* vi k = up */
	center_w ((Coor) (piwr + piwl)/2, (Coor) piwt);
	kpicture ();
	break;
    case 'c':
    case 'C': /* center at current cursor position */
	old_sx = sx;
	old_sy = sy;
	transf (event.xmotion.x, event.xmotion.y, &sx, &sy);
	center_w ((Coor) sx*4, (Coor) sy*4);
	kpicture ();
	break;
    case 'p': /* previous */
    case 'P':
	prev_w ();
	kpicture ();
	break;
    case 'n': /* no */
    case 'N':
    case 'y': /* yes */
    case 'Y':
	if (ask_mode) return (1);
	break;
    case 'q': /* quit? */
    case 'Q':
	if (!ask_mode) return (1);
	break;
    case 'r':
    case 'R': /* redraw */
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
	kpicture ();
	break;
    case 'g' : /* toggle grid */
    case 'G' : /* toggle grid */
	toggle_grid ();
	kpicture ();
	break;
    case 's' : /* sub terms */
    case 'S' :
	all_term ();
	Sub_terms = TRUE;
	kpicture ();
	break;
    case 'd':
	Draw_hashed = Draw_hashed == FALSE ? TRUE : FALSE;
	bulb_hashed ();
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
	kpicture ();
	break;
    case 'D':
	Draw_dominant = Draw_dominant == FALSE ? TRUE : FALSE;
	bulb_dominant ();
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
	kpicture ();
	break;
    case 't':
    case 'T':
	toggle_tracker ();
	break;
    }
    return (0);
}

static void WaitForEvent (int etype)
{
    Window ret_r, ret_c;
    int    ret_r_x, ret_r_y;
    int    ex, ey, ew, eh;
    unsigned int ret_mask;

    for (;;) {
	/* for the case we are waiting for a sub-process to die: check whether it died.... */
        if (looks_like_the_sea_child_died (FALSE) == TRUE) return; /* it died! */

        /* get event, if no last one was stored */
	if (looks_like_the_sea_child_died (-1) != -1) { /* sea child exists */
	    /* periodically scan event queue, instead of using xnextevent to get the event */
	    while (XCheckMaskEvent (dpy, (std_mask | ButtonPressMask), &event) != TRUE) { /* no interesting event found */
		if (looks_like_the_sea_child_died (FALSE) == TRUE) return; /* it died! */
		sleep (1);
	    }
	}
	else
	    XNextEvent (dpy, &event);

	ewin = event.xany.window;

	switch (event.type) {

	case MotionNotify:
#ifdef DEBUG_MOTION_EVENTS
	    PE "-- MotionNotify\n");
#endif
	    if (cecho == 4) {
		/*
		** Draw rubber line cursor
		*/
		if (ccur) remove_cursor ();
		ccur = cecho;
		cxe = event.xmotion.x;
		cye = event.xmotion.y;
		XDrawLine (dpy, pwin, cgc, cxs, cys, cxe, cye);
		if (tracker_enabled) {
		    old_endx = endx;
		    old_endy = endy;
		    transf (cxe, cye, &endx, &endy);
		    dx = Abs(sx - endx);
		    dy = Abs(sy - endy);
		}
	    }
	    else if (cecho == 5) {
		/*
		** Draw rubber box cursor
		*/
		if (ccur) remove_cursor ();
		ccur = cecho;
		cxe = event.xmotion.x;
		cye = event.xmotion.y;
		if (cxs < cxe) { crx = cxs; crw = cxe - cxs; }
		else           { crx = cxe; crw = cxs - cxe; }
		if (cys < cye) { cry = cys; crh = cye - cys; }
		else           { cry = cye; crh = cys - cye; }
		XDrawRectangle (dpy, pwin, cgc, crx, cry, crw, crh);
		if (tracker_enabled) {
		    old_endx = endx;
		    old_endy = endy;
		    transf (cxe, cye, &endx, &endy);
		    dx = Abs(sx - endx);
		    dy = Abs(sy - endy);
		}
	    }
	    else if (tracker_enabled) {
	    	old_sx = sx;
	    	old_sy = sy;
		old_endx = endx = dx = 0;
		old_endy = endy = dy = 0;
	    	transf (event.xmotion.x, event.xmotion.y, &sx, &sy);
	    }

	    if (tracker_enabled && (old_endx != endx || old_endy != endy || old_sx != sx || old_sy != sy)) {
		if (!ImageMode || !tracker_to_image (mystr, sx, sy, endx, endy)) {
		    if (endx == 0 && endy == 0)
			sprintf (mystr, "cursor: (%ld,%ld) lambda                        ", sx, sy);
		    else
			sprintf (mystr, "s(%ld,%ld) e(%ld,%ld) d(%ld,%ld) lambda         ", sx, sy, endx, endy, dx, dy);
		}
		XDrawImageString (dpy, trwin, trgc, cD, cH, mystr, strlen (mystr));
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
#ifdef DEBUG_EVENTS
		PE "-- Expose: ewin = pwin\n");
#endif
		while (XCheckTypedWindowEvent (dpy, pwin, Expose, &event) == TRUE);
		pict_all (DRAW);
		pic_max ();
		picture (); /* redraw picture, this may take TIME!! */
		if (cecho >= 4) {
		    ccur = 0; /* do not redraw cursor */
		    fix_loc (cfixx, cfixy);
		}
	    }
	    else if (ewin == mwin) {
#ifdef DEBUG_EVENTS
		PE "-- Expose: ewin = mwin\n");
#endif
		Rmenu (); /* redraw menu */
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
		XDrawImageString (dpy, trwin, trgc, cD, cH, mystr, strlen (mystr));
	    }
	    else if (ewin == lwin) {
#ifdef DEBUG_EVENTS
		PE "-- Expose: ewin = lwin\n");
#endif
		Rmsk (); /* redraw lay menu */
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
	    if (etype == KeyPress) return; /* char input: see GetString */
	    if (cwin && do_keypress ()) return; /* see get_loc */
	    break;

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
	    XResizeWindow (dpy, pwin, tW, pH);
	    XSync (dpy, 0); /* Needed to get all Expose events! */
	    Set_window (LAYS);
	    Set_window (PICT);
	    break;
	}

	if (event.type == etype) break;
    }
}

/*
 * returns TRUE if a something happened during drawing
 */
int event_exists ()
{
    /* due to an expose: start all over again... */
    if (XCheckTypedEvent (dpy, Expose, &event) == TRUE) {
	XPutBackEvent (dpy, &event);
	/* printf ("intr: expose\n"); */
	return (TRUE);
    }
    /* due to a key press */
    if (XCheckTypedEvent (dpy, KeyPress, &event) == TRUE) {
	XPutBackEvent (dpy, &event);
	/* printf ("intr: key\n"); */
	return (TRUE);
    }
    /* due to a button press
    if (XCheckTypedEvent (dpy, ButtonPress, &event) == TRUE) {
	XPutBackEvent (dpy, &event);
	return (TRUE);
    }  */
    return (FALSE);
}

void print_reason ()
{
    if (XCheckTypedEvent (dpy, Expose, &event) == TRUE) { /* the reason was expose... */
	XPutBackEvent (dpy, &event);
	ptext (NULL); /* get back previous text */
    }
    else
	ptext ("Drawing was interrupted! The picture is incomplete");
}

static void remove_cursor ()
{
    if (ccur == 4) {
	/*
	** Remove old rubber line cursor:
	*/
	XDrawLine (dpy, pwin, cgc, cxs, cys, cxe, cye);
    }
    else if (ccur == 5) {
	/*
	** Remove old rubber box cursor:
	*/
	XDrawRectangle (dpy, pwin, cgc, crx, cry, crw, crh);
    }
    ccur = 0;
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
	if (ks == NoSymbol || ks == XK_Shift_L || ks == XK_Shift_R) continue;
	XLookupString (&event.xkey, kc, 1, NULL, NULL);

	if (*kc == '\r' || *kc == '\n') { /* return */
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
