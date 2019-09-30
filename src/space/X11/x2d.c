/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include <stdio.h>
#include <math.h>

#include "src/space/include/config.h"

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "src/space/auxil/auxil.h"
#include "src/space/X11/pane.h"
#include "src/space/include/tile.h"
#include "src/space/X11/extern.h"

extern Display *display;
extern int goptSaveImageEP;

canvas_t MY_CANVAS;
canvas_t * my_canvas = 0;

static int displayW, displayH;
extern GC gcBlack;
extern GC gcDraw;
extern GC gcUnDraw;
static GC gcXCopy;
static Pixmap pixmap; /* double buffer */
static Window window; /* canvas drawable */
static Widget canvas_widget;

#define fillPolygon(gc, pts, n) \
    XFillPolygon (display, window, gc, pts, n, Convex, CoordModeOrigin); \
    XFillPolygon (display, pixmap, gc, pts, n, Convex, CoordModeOrigin)

#define drawLine(gc, x1, y1, x2, y2) \
    XDrawLine (display, window, gc, x1, y1, x2, y2); \
    XDrawLine (display, pixmap, gc, x1, y1, x2, y2)

#define drawPoint(gc, x, y) \
    XDrawPoint (display, window, gc, x, y); \
    XDrawPoint (display, pixmap, gc, x, y)

#define drawString(gc, x, y, str, len) \
    XDrawString (display, window, gc, x, y, str, len); \
    XDrawString (display, pixmap, gc, x, y, str, len)

#define CvtX(c,x) c->deltaX + boundForInt (c->scale * ((x) - c->Wx))
#define CvtY(c,y) c->deltaY + boundForInt (c->scale * (c->Wy - (y)))

int boundForInt (double a)
{
    if (a < 0) {
	if (a < -INT_MAX / 2) a = -INT_MAX / 2;
	return (int) (a - 0.5);
    }
    if (a > INT_MAX / 2) a = INT_MAX / 2;
    return (int) (a + 0.5);
}

void X2dFillTile (GC gc, wcoor xl, wcoor bl, wcoor tl, wcoor xr, wcoor br, wcoor tr)
{
    static XPoint points[4];

#if 0
    points[0].x = CvtX (my_canvas, xl);
    points[0].y = CvtY (my_canvas, bl);
    points[1].x = CvtX (my_canvas, xr);
    if (br == bl) points[1].y = points[0].y;
    else points[1].y = CvtY (my_canvas, br);
    points[2].x = points[1].x;
    points[2].y = CvtY (my_canvas, tr);
    points[3].x = points[0].x;
    if (tl == tr) points[3].y = points[2].y;
    else points[3].y = CvtY (my_canvas, tl);
#else
    int eq_bot, eq_top;

    xl = my_canvas->deltaX + (my_canvas->scale * (xl - my_canvas->Wx));
    if (xl >= displayW) return;
    xr = my_canvas->deltaX + (my_canvas->scale * (xr - my_canvas->Wx));
    if (xr < 0) return;

    eq_bot = (br == bl);
    eq_top = (tr == tl);
    bl = my_canvas->deltaY + (my_canvas->scale * (my_canvas->Wy - bl));
    tl = my_canvas->deltaY + (my_canvas->scale * (my_canvas->Wy - tl));
    br = eq_bot ? bl : my_canvas->deltaY + (my_canvas->scale * (my_canvas->Wy - br));
    tr = eq_top ? tl : my_canvas->deltaY + (my_canvas->scale * (my_canvas->Wy - tr));

    if (xl < 0) {
	if (!eq_bot) bl = bl - xl * (br - bl) / (xr - xl);
	if (!eq_top) tl = tl - xl * (tr - tl) / (xr - xl);
	xl = 0;
    }
    if (xr > displayW) {
	if (!eq_bot) br = bl + (displayW - xl) * (br - bl) / (xr - xl);
	if (!eq_top) tr = tl + (displayW - xl) * (tr - tl) / (xr - xl);
	xr = displayW;
    }
    if (bl < 0 && br < 0) return;
    if (tl >= displayH && tr >= displayH) return;
    if (tl < 0 && tr < 0) tr = tl = 0;
    if (bl > displayH && br > displayH) br = bl = displayH;

    points[3].x = points[0].x = (int)(xl + 0.5);
    points[2].x = points[1].x = (int)(xr + 0.5);
    points[0].y = (int)(bl + 0.5);
    points[1].y = (int)(br + 0.5);
    points[2].y = (int)(tr + 0.5);
    points[3].y = (int)(tl + 0.5);
#endif

    fillPolygon (gc, points, 4);
}

static struct draw_object *firstobj;
static struct draw_object *freeobjs;
static int obj_count;

static void saveobj (wcoor xl, wcoor yl, wcoor xr, wcoor yr)
{
    struct draw_object *obj;
    if (freeobjs) { obj = freeobjs; freeobjs = obj->next; }
    else obj = NEW (struct draw_object, 1);
    obj->xl = xl;
    obj->xr = xr;
    obj->yl = yl;
    obj->yr = yr;
    obj->next = firstobj;
    firstobj = obj;
    ++obj_count;
}

static void removeobj (wcoor xl, wcoor yl, wcoor xr, wcoor yr)
{
    struct draw_object *obj, *prev;
    prev = NULL;
    for (obj = firstobj; obj; obj = obj->next) {
	if (obj->xl == xl && obj->xr == xr && obj->yl == yl && obj->yr == yr) {
	    if (!prev) firstobj = obj->next;
	    else prev->next = obj->next;
	    obj->next = freeobjs; freeobjs = obj;
	    --obj_count;
	    return;
	}
	prev = obj;
    }
}

void removeobjs ()
{
    struct draw_object *obj;
    if ((obj = firstobj)) {
	while (obj->next) obj = obj->next;
	obj->next = freeobjs;
	freeobjs = firstobj;
	firstobj = NULL;
	obj_count = 0;
    }
}

void print_objs ()
{
    fprintf (stderr, "number of objects = %d\n", obj_count);
}

static void redrawobjs (int xl, int xr, int yb, int yt)
{
    double dx1, dx2, dy1, dy2;
    int x1, x2, y1, y2;
    struct draw_object *obj;
    for (obj = firstobj; obj; obj = obj -> next) {
#if 0
	x1 = CvtX (my_canvas, obj->xl);
	x2 = CvtX (my_canvas, obj->xr);
	if (x2 < xl || x1 > xr) continue;
	y1 = CvtY (my_canvas, obj->yl);
	y2 = CvtY (my_canvas, obj->yr);
	if (y2 > y1) { if (y2 < yb || y1 > yt) continue; }
	else if (y1 < yb || y2 > yt) continue;
#else
	dx1 = my_canvas->deltaX + (my_canvas->scale * (obj->xl - my_canvas->Wx));
	dx2 = my_canvas->deltaX + (my_canvas->scale * (obj->xr - my_canvas->Wx));
	if (dx2 < xl || dx1 > xr) continue;
	dy1 = my_canvas->deltaY + (my_canvas->scale * (my_canvas->Wy - obj->yl));
	dy2 = my_canvas->deltaY + (my_canvas->scale * (my_canvas->Wy - obj->yr));
	if (dy2 > dy1) {
	    if (dy2 < yb || dy1 > yt) continue;
	    if (dy2 > displayH) {
		dx2 = dx1 + (displayH - dy1) * (dx2 - dx1) / (dy2 - dy1);
		dy2 = displayH;
	    }
	    if (dy1 < 0) {
		dx1 = dx1 - dy1 * (dx2 - dx1) / (dy2 - dy1);
		dy1 = 0;
	    }
	}
	else {
	    if (dy1 < yb || dy2 > yt) continue;
	    if (dy1 > displayH) {
		dx1 = dx1 + (dy1 - displayH) * (dx2 - dx1) / (dy1 - dy2);
		dy1 = displayH;
	    }
	    if (dy2 < 0) {
		dx2 = dx1 + dy1 * (dx2 - dx1) / (dy1 - dy2);
		dy2 = 0;
	    }
	}
	if (dx2 > displayW) {
	    dy2 = dy1 + (displayW - dx1) * (dy2 - dy1) / (dx2 - dx1);
	    dx2 = displayW;
	}
	if (dx1 < 0) {
	    dy1 = dy1 - dx1 * (dy2 - dy1) / (dx2 - dx1);
	    dx1 = 0;
	}
	x1 = (int)(dx1 + 0.5);
	x2 = (int)(dx2 + 0.5);
	y1 = (int)(dy1 + 0.5);
	y2 = (int)(dy2 + 0.5);
#endif
	XDrawLine (display, window, gcDraw, x1, y1, x2, y2);
    }
}

void draw_objects ()
{
    redrawobjs (0, displayW, 0, displayH);
}

void X2dDrawLine (GC gc, wcoor xl, wcoor yl, wcoor xr, wcoor yr)
{
    double dx1, dx2, dy1, dy2;
    int x1, x2, y1, y2;

    if (xl > xr || (xl == xr && yl > yr)) { // swap
	wcoor x, y;
	x = xl; xl = xr; xr = x;
	y = yl; yl = yr; yr = y;
    }

    if (gc == gcDraw) saveobj (xl, yl, xr, yr);
    else if (gc == gcUnDraw) removeobj (xl, yl, xr, yr);

#if 0
    x1 = CvtX (my_canvas, xl);
    x2 = CvtX (my_canvas, xr);
    y1 = CvtY (my_canvas, yl);
    y2 = CvtY (my_canvas, yr);

    // note: x2 >= x1
    if (x2 < 0 || x1 >= displayW) return;
    if (y2 > y1) { if (y2 < 0 || y1 >= displayH) return; }
    else if (y1 < 0 || y2 >= displayH) return;
#else
    dx1 = my_canvas->deltaX + (my_canvas->scale * (xl - my_canvas->Wx));
    dx2 = my_canvas->deltaX + (my_canvas->scale * (xr - my_canvas->Wx));
    // note: dx2 >= dx1
    if (dx2 < 0 || dx1 >= displayW) return;

    dy1 = my_canvas->deltaY + (my_canvas->scale * (my_canvas->Wy - yl));
    dy2 = my_canvas->deltaY + (my_canvas->scale * (my_canvas->Wy - yr));
    if (dy2 > dy1) {
	if (dy2 < 0 || dy1 >= displayH) return;
	if (dy2 > displayH) {
	    dx2 = dx1 + (displayH - dy1) * (dx2 - dx1) / (dy2 - dy1);
	    dy2 = displayH;
	}
	if (dy1 < 0) {
	    dx1 = dx1 - dy1 * (dx2 - dx1) / (dy2 - dy1);
	    dy1 = 0;
	}
    }
    else {
	if (dy1 < 0 || dy2 >= displayH) return;
	if (dy1 > displayH) {
	    dx1 = dx1 + (dy1 - displayH) * (dx2 - dx1) / (dy1 - dy2);
	    dy1 = displayH;
	}
	if (dy2 < 0) {
	    dx2 = dx1 + dy1 * (dx2 - dx1) / (dy1 - dy2);
	    dy2 = 0;
	}
    }
    if (dx2 > displayW) {
	dy2 = dy1 + (displayW - dx1) * (dy2 - dy1) / (dx2 - dx1);
	dx2 = displayW;
    }
    if (dx1 < 0) {
	dy1 = dy1 - dx1 * (dy2 - dy1) / (dx2 - dx1);
	dx1 = 0;
    }
    x1 = (int)(dx1 + 0.5);
    x2 = (int)(dx2 + 0.5);
    y1 = (int)(dy1 + 0.5);
    y2 = (int)(dy2 + 0.5);
#endif

    if (gc == gcDraw) {
	XDrawLine (display, window, gc, x1, y1, x2, y2);
    }
    else if (gc == gcUnDraw) {
	unsigned int w, h;
	--x1;
	w = x2 - x1;
	if (y1 > y2) { int y = y1; y1 = y2; y2 = y; }
	--y1;
	h = y2 - y1;
	XCopyArea (display, pixmap, window, gcXCopy, x1, y1, w+1, h+1, x1, y1);
	redrawobjs (x1, x2, y1, y2);
    }
    else {
	drawLine (gc, x1, y1, x2, y2);
    }
}

void X2dDrawDot (GC gc, wcoor xp, wcoor yp, int m, int w, char *s)
{
    static Font font;
    int x = CvtX (my_canvas, xp);
    int y = CvtY (my_canvas, yp);

    if (x < 0 || y < 0 || x >= displayW || y >= displayH) return;

    if (w > 0) {
	if (m == 'x') {
	    drawLine (gc, x-w, y-w, x+w, y+w);
	    drawLine (gc, x-w, y+w, x+w, y-w);
	}
	else if (m == 'p') {
	    drawLine (gc, x-w, y, x+w, y);
	    drawLine (gc, x, y+w, x, y-w);
	}
	else if (m == 'h') {
	    drawLine (gc, x-w, y, x+w, y);
	}
	else if (m == 'v') {
	    drawLine (gc, x, y+w, x, y-w);
	}
	else if (m == 'r') {
	    drawLine (gc, x-w, y, x, y+w);
	    drawLine (gc, x, y+w, x+w, y);
	    drawLine (gc, x+w, y, x, y-w);
	    drawLine (gc, x, y-w, x-w, y);
	}
    }
    else {
	drawPoint (gc, x, y);
    }
    if (!s || font < 0) return;
    if (!font) {
	font = XLoadFont (display, "fixed");
	if (!font) { say ("font fixed not loaded"); font = -1; }
	else XSetFont (display, gc, font);
    }
    w = strlen (s);
    drawString (gc, x+2, y+2, s, w);
}

void X2dSetPointer (wcoor wx, wcoor wy)
{
    int x = CvtX (my_canvas, wx);
    int y = CvtY (my_canvas, wy);
    if (x < 0) x = 0; else if (x > my_canvas -> Cw) x = my_canvas -> Cw;
    if (y < 0) y = 0; else if (y > my_canvas -> Ch) y = my_canvas -> Ch;
    XWarpPointer (display, None, window, 0, 0, 0, 0, x, y);
}

void X2dSetWorldCoordinates (wcoor xl, wcoor xr, wcoor yb, wcoor yt)
{
    Arg arg[4];
    double scale2, Ch, Cw, Wh, Ww;

    XtSetArg (arg[0], XtNheight, &(my_canvas -> Ch));
    XtSetArg (arg[1], XtNwidth,  &(my_canvas -> Cw));
    XtGetValues (canvas_widget, arg, 2);

    my_canvas -> Wx = xl;
    my_canvas -> Wy = yt;
    my_canvas -> Ww = xr - xl;
    my_canvas -> Wh = yt - yb;

    Wh = my_canvas -> Wh;
    Ww = my_canvas -> Ww;
    Ch = my_canvas -> Ch - 2 * FromBorder;
    Cw = my_canvas -> Cw - 2 * FromBorder;

    if (Ch <= 0 || Cw <= 0 || Wh <= 0 || Ww <= 0) {
	say ("Invalid dimensions");
	return;
    }

    my_canvas -> deltaX = FromBorder;
    my_canvas -> deltaY = FromBorder;
    my_canvas -> scale = Cw / Ww;
    scale2 = Ch / Wh;

    if (scale2 < my_canvas -> scale) {
	my_canvas -> scale = scale2;
	my_canvas -> deltaX += (Cw - my_canvas -> scale * Ww) / 2;
    }
    else {
	my_canvas -> deltaY += (Ch - my_canvas -> scale * Wh) / 2;
    }
 /* fprintf (stderr, "X2dSetWorldCoordinates: scale=%g deltaX=%d deltaY=%d\n",
	my_canvas -> scale, my_canvas -> deltaX, my_canvas -> deltaY); */
}

/* ARGSUSED */
static void expose (Widget w, XtPointer client_data, XEvent *ev, Boolean *b)
{
    XExposeEvent *event = (XExposeEvent *) ev;
    /* copy part of the double buffering pixmap into the window */
    if (event->x == 0 && event->y == 0) { // resize
	Arg arg[4];
	XtSetArg (arg[0], XtNheight, &(my_canvas -> Ch));
	XtSetArg (arg[1], XtNwidth,  &(my_canvas -> Cw));
	XtGetValues (canvas_widget, arg, 2);
    }
    XCopyArea (display, pixmap, window, gcXCopy,
	event->x, event->y, event->width, event->height, event->x, event->y);
    redrawobjs (event->x, event->x+event->width, event->y, event->y+event->height);
    *b = False;
}

void initCanvas (Widget widget)
{
    XGCValues values;
    XSetWindowAttributes wa;
    int depth, screen_number;

    canvas_widget = widget;
    window = XtWindow (canvas_widget);

    my_canvas = &MY_CANVAS;
    canvas_widget = canvas_widget;
    screen_number = DefaultScreen (display);
    depth = DefaultDepth (display, screen_number);
    my_canvas -> deltaX = 0;
    my_canvas -> deltaY = 0;
    my_canvas -> scale = 1.0;

    displayW = DisplayWidth  (display, screen_number);
    displayH = DisplayHeight (display, screen_number);

    gcXCopy = XCreateGC (display, window, 0, &values);

    pixmap = XCreatePixmap (display, window, displayW, displayH, depth);
    ASSERT (pixmap);

    X3dInit (my_canvas);
    clearDisplay ();

    wa.backing_store = WhenMapped;
    XChangeWindowAttributes (display, window, CWBackingStore, &wa);

    XtAddEventHandler (canvas_widget, ExposureMask, 0, expose, (XtPointer)0);
}

void clearDisplay ()
{
    /* fill window and pixmap with background color */
    XFillRectangle (display, window, gcBlack, 0, 0, displayW, displayH);
    XFillRectangle (display, pixmap, gcBlack, 0, 0, displayW, displayH);
}

void X2dFlush ()
{
    XFlush (display);
}

void queryPointer (int *x, int *y)
{
    Window root, child;
    int x_root, y_root;
    unsigned int mask = 0;
    XQueryPointer (display, window, &root, &child, &x_root, &y_root, x, y, &mask);
 // fprintf (stderr, "queryPointer: x_root=%d y_root=%d x=%d y=%d\n", x_root, y_root, *x, *y);
}

#ifdef STATIC
/* libX11.a fix for static linking */
void *dlopen (const char *filename, int flag)
{
    return NULL;
}
#endif
