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
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"

#ifdef CONFIG_XSPACE
#include <X11/Intrinsic.h>
#endif

#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/extract/define.h"
#ifdef CONFIG_XSPACE
#include "src/space/X11/pane.h"
#include "src/space/X11/extern.h"
#endif
#include "src/space/scan/export.h"

#define D(v) (double)(v)
#define F(v) (float)(v)
#define W(v) (wcoor)(v)
#define L(v) (long)(v)

extern int goptPairBoundary;
extern int goptDrawTile;
extern int goptDrawEdge;
extern int goptDrawResistor;
extern int goptUnDrawResistor;
extern int goptOutResistor;
extern int goptDrawCapacitor;
extern int goptUnDrawCapacitor;
extern int goptOutCapacitor;
extern int goptDrawTriangle;
extern int goptDrawEquiEdge;
extern int goptDrawGreen;
extern int goptDrawDelaunay;
extern int goptDrawSubContact;
extern int goptFillSubContact;
extern int goptDrawSubResistor;

#ifdef CONFIG_SPACE3D
extern FILE *xout;
#else
extern char cellname[];
extern int extracting;
extern int interrupt;
extern int goptPauseAfterPP;
extern int goptSaveImageEP;
extern int goptSaveImagePP;
extern int goptCoordInDbUnits;
extern int goptCoordInMicrons;
extern int goptDraw3D;
extern int goptPairConductOnly;
extern int goptPairToInfinity;
extern int goptRobot;
extern int goptXl, goptXr, goptYb, goptYt;
extern int space_ready;
extern int use_pixel_copy_mode;

extern canvas_t * my_canvas;
extern double camDistance, camLatitude, camLongitude;
extern GC gcWhite;
extern GC gcDraw;
extern GC gcDraw2;
extern GC gcUnDraw;

int prepass_1b;
int drawGreenLine = 0;
#endif

#ifdef CONFIG_XSPACE
static int worldCoordinatesDone = 0;
static double Z_bot = 0;	/* lowest  z-coordinate of spiders */
static double Z_top = 0;	/* highest z-coordinate of spiders */
static int inscale  = 4;	/* default cell input scale factor */
static int outscale = 4;	/* default cell output scale factor */

#ifdef __cplusplus
  extern "C" {
#endif
void draw_objects (void);
void print_objs (void);
void removeobjs (void);
void X2dDrawDot (GC gc, wcoor xp, wcoor yp, int mode, int size, char *s);
int  doEvent (void);
int  setPause (void);
void setText (char *);
void queryPointer (int *x, int *y);
#ifdef __cplusplus
  }
#endif

/* Clipping area.
   X drawing commands will only be given for objects within clipping area. */
static float xl_clip, xr_clip, yb_clip, yt_clip;
float bb_xl, bb_xr, bb_yb, bb_yt;
static float c2d_xl, c2d_xr, c2d_yb, c2d_yt;
static float old_xc, old_yc;
static double old_sc;
static int opt3D = 0;

#endif // CONFIG_XSPACE

void worldCoordinates (float xl, float xr, float yb, float yt)
{
#ifdef CONFIG_SPACE3D
    if (xout) {
	fprintf (xout, "w %ld %ld %ld %ld\n", L(xl), L(yb), L(xr), L(yt));
	fflush (xout);
    }
#else
    float extension;

    old_xc = my_canvas -> Wx + my_canvas -> Ww / 2;
    old_yc = my_canvas -> Wy - my_canvas -> Wh / 2;
    old_sc = my_canvas -> scale;

    /* The clipping area is made larger so that the whole image
       will also be filled if the W/L ratio of the specified
       layout window is not equal to the W/L ratio of the X window.
    */
    extension = 4 * Max ((yt - yb), (xr - xl));
    xl_clip = xl - extension; xr_clip = xr + extension;
    yb_clip = yb - extension; yt_clip = yt + extension;

#ifdef DEBUG
if(DEBUG) fprintf (stderr, "worldCoordinates: 3d=%d clr=%d clip:xl=%g xr=%g yb=%g yt=%g\n",
		goptDraw3D, !goptSaveImageEP, xl_clip, xr_clip, yb_clip, yt_clip);
#endif

    if (goptDraw3D && !opt3D) {
	/* Decrease Z_bot and increase Z_top with (yt - yb) / 50
	   to obtain a small bottom and top margin in the picture */
	X3dSetViewReferencePoint (W((xl + xr)/2), W((yb + yt)/2), W(Z_bot - (yt - yb)/50));
	X3dSetSimplePerspective (W(camLongitude), W(camLatitude), W((yt - yb) + (xr - xl)));
	X3dSetWorldCoordinates (W(xl), W(xr), W(yb), W(yt), W(Z_bot - (yt - yb)/50), W(Z_top + (yt - yb)/50));
    }
    else {
	X2dSetWorldCoordinates (W(xl), W(xr), W(yb), W(yt));
    }

    if (!goptDraw3D || !opt3D) { c2d_xl = xl; c2d_xr = xr; c2d_yb = yb; c2d_yt = yt; }
    opt3D = goptDraw3D;

    if (!worldCoordinatesDone) { worldCoordinatesDone = 1;
	old_xc = my_canvas -> Wx + my_canvas -> Ww / 2;
	old_yc = my_canvas -> Wy - my_canvas -> Wh / 2;
	old_sc = my_canvas -> scale;
    }
    else clearDisplay ();
#endif
}

#ifdef CONFIG_XSPACE
int getCoorPoint (coor_t *rx, coor_t *ry, int cz)
{
    wcoor wx, wy, wz;
    int x, y;

    if (!worldCoordinatesDone) return 0;

    queryPointer (&x, &y);
    wx = my_canvas -> Wx + D(x - my_canvas -> deltaX) / my_canvas -> scale;
    wy = my_canvas -> Wy - D(y - my_canvas -> deltaY) / my_canvas -> scale;
    wz = cz;

    if (goptDraw3D) X3dProjectReverse (wx, wy, wz, &wx, &wy);

    if (goptCoordInMicrons) { // display coordinates in microns
	*rx = (meters * 1e9 * wx + 0.5);
	*ry = (meters * 1e9 * wy + 0.5);
    }
    else if (goptCoordInDbUnits) { // display coordinates in db units
	*rx = (wx / 4 + 0.5);
	*ry = (wy / 4 + 0.5);
    }
    else { // display coordinates as is
	*rx = (wx + 0.5);
	*ry = (wy + 0.5);
    }
    return 1;
}

void setwPosition (int mode) // 'x' && 'y'
{
    float xl, xr, yb, yt;
    wcoor wx, wy, w, h;
    int x, y;

    if (!worldCoordinatesDone) return;

    queryPointer (&x, &y);
    wx = my_canvas -> Wx + D(x - my_canvas -> deltaX) / my_canvas -> scale;
    wy = my_canvas -> Wy - D(y - my_canvas -> deltaY) / my_canvas -> scale;

    xl = my_canvas -> Wx;
    yt = my_canvas -> Wy;
    if ((x = my_canvas -> deltaX - FromBorder) != 0) xl -= x / my_canvas -> scale;
    if ((y = my_canvas -> deltaY - FromBorder) != 0) yt += y / my_canvas -> scale;
    w = my_canvas -> Cw - 2 * FromBorder; w /= my_canvas -> scale;
    h = my_canvas -> Ch - 2 * FromBorder; h /= my_canvas -> scale;
    xr = xl + w;
    yb = yt - h;

    if (mode == 1) { /* LeftBottom */
	if (wx >= xr) xr = wx; else xl = wx;
	if (wy >= yt) yt = wy; else yb = wy;
    }
    else {
	if (wx <= xl) xl = wx; else xr = wx;
	if (wy <= yb) yb = wy; else yt = wy;
    }
    worldCoordinates (xl, xr, yb, yt);
    redrawPicture (0);
}

void zoomPosition (int mode) // 'i' && 'o'
{
    wcoor wx, wy, w, h;
    int x, y;

    if (!worldCoordinatesDone) return;

    if (mode <= 2) queryPointer (&x, &y);
    else { x = my_canvas -> Cw / 2; y = my_canvas -> Ch / 2; }
    wx = my_canvas -> Wx + D(x - my_canvas -> deltaX) / my_canvas -> scale;
    wy = my_canvas -> Wy - D(y - my_canvas -> deltaY) / my_canvas -> scale;

    w = my_canvas -> Cw - 2 * FromBorder; w /= my_canvas -> scale * 2;
    h = my_canvas -> Ch - 2 * FromBorder; h /= my_canvas -> scale * 2;
    if (mode == 1 || mode == 3) { w /= 2; h /= 2; } /* ZoomIn */
    if (mode == 2 || mode == 4) { w *= 2; h *= 2; } /* ZoomOut */

    worldCoordinates (wx - w, wx + w, wy - h, wy + h);
    redrawPicture (0);
}

void zoomBbox (int mode) // 'b' && toggle_3d
{
    if (!worldCoordinatesDone) return;

    if (drawGreenLine) { /* clear green line */
	double d = 0;
	drawGreenLine = 2;
	if (mode) goptDraw3D = !goptDraw3D;
	drawGreen (d, d, d, d, d, d);
	if (mode) goptDraw3D = !goptDraw3D;
	drawGreenLine = 3; // mark for redraw
    }

    if (mode) { // toggle_3d
	worldCoordinates (c2d_xl, c2d_xr, c2d_yb, c2d_yt);
    }
    else {
	opt3D = 0;
	worldCoordinates (bb_xl, bb_xr, bb_yb, bb_yt);
    }
    redrawPicture (0);
}

void zoomUndo () // 'u'
{
    float w, h;

    if (!worldCoordinatesDone) return;
    w = my_canvas -> Cw - 2 * FromBorder; w /= old_sc * 2;
    h = my_canvas -> Ch - 2 * FromBorder; h /= old_sc * 2;
    worldCoordinates (old_xc - w, old_xc + w, old_yc - h, old_yc + h);
    redrawPicture (0);
}

void gotoCoord (int cx, int cy, int cz) // 'Enter'
{
    wcoor x, y, w, h, wx, wy, wz;

    if (!worldCoordinatesDone) return;

    wx = cx; wy = cy; wz = cz;
    if (goptCoordInMicrons) { // coordinates are in microns
	w = meters * 1e9;
	wx /= w; wy /= w; wz /= w;
    }
    else if (goptCoordInDbUnits) { // coordinates are in db units
	wx *= 4; wy *= 4; wz *= 4;
    }
    if (goptDraw3D) X3dProject (wx, wy, wz, &wx, &wy);

    w = my_canvas -> Cw - 2 * FromBorder; w /= my_canvas -> scale * 2;
    h = my_canvas -> Ch - 2 * FromBorder; h /= my_canvas -> scale * 2;
    x = my_canvas -> Wx + w;
    y = my_canvas -> Wy - h;
    if ((cx = my_canvas -> deltaX - FromBorder) != 0) x -= cx / my_canvas -> scale;
    if ((cy = my_canvas -> deltaY - FromBorder) != 0) y += cy / my_canvas -> scale;

    if (wx < x-w || wx > x+w || wy < y-h || wy > y+h) {
	worldCoordinates (wx-w, wx+w, wy-h, wy+h);
	redrawPicture (0);
    }
    X2dDrawDot (gcWhite, wx, wy, 'x', 8, 0);
}

void goLeftRight (int mode) // '<-' && '->' && '^' && 'v'
{
    float x, y, w, h;
    int d;

    if (!worldCoordinatesDone) return;

    w = my_canvas -> Cw - 2 * FromBorder; w /= my_canvas -> scale * 2;
    h = my_canvas -> Ch - 2 * FromBorder; h /= my_canvas -> scale * 2;
    x = my_canvas -> Wx + w;
    y = my_canvas -> Wy - h;
    if ((d = my_canvas -> deltaX - FromBorder) != 0) x -= d / my_canvas -> scale;
    if ((d = my_canvas -> deltaY - FromBorder) != 0) y += d / my_canvas -> scale;

         if (mode == 1) { x -= w; } /* Left */
    else if (mode == 2) { x += w; } /* Right */
    else if (mode == 3) { y -= h; } /* Down */
    else                { y += h; } /* Up */
    worldCoordinates (x-w, x+w, y-h, y+h);
    redrawPicture (0);
}

double setCamera (int mode, int value) // keypad '<-' && '->' && '^' && 'v' && '+' && '-'
{
    double v;
    if (mode == 0) {
	if ((value < 0 && camLatitude == 0)
	 || (value > 0 && camLatitude == 90)) return camLatitude;
    }
    if (worldCoordinatesDone && goptDraw3D && drawGreenLine) { /* clear green line */
	v = 0;
	drawGreenLine = 2;
	drawGreen (v, v, v, v, v, v);
	drawGreenLine = 3; // mark for redraw
    }
    if (mode >= 2) { /* change distance */
	if (mode == 2)
	    v = camDistance *= 2; /* + */
	else
	    v = camDistance /= 2; /* - */
    }
    else if (mode == 0) {
	camLatitude += value;
	if (camLatitude <  0.0) camLatitude =  0.0;
	if (camLatitude > 90.0) camLatitude = 90.0;
	v = camLatitude;
    }
    else {
	camLongitude += value;
	if (camLongitude <  -90.0) camLongitude += 360.0;
	if (camLongitude >= 360.0) camLongitude -= 360.0;
	v = camLongitude;
    }
    if (worldCoordinatesDone && goptDraw3D) {
	if (mode < 2) X3dSetSimplePerspective (W(camLongitude), W(camLatitude), W(0));
	clearDisplay ();
	redrawPicture (0);
    }
    return v;
}
#endif // CONFIG_XSPACE

#ifdef CONFIG_SPACE3D
Private char *colorStr (mask_t *colorp)
{
    char *s = colorHexStr (colorp);
    while (*s == '0') ++s;
    return (*s ? s : s-1);
}
#endif

int drawPairBoundary (coor_t xl, coor_t yl, coor_t xr, coor_t yr, int etype)
{
#ifdef CONFIG_SPACE3D
    if (xout) {
	fprintf (xout, "e%d %ld %ld %ld %ld\n", etype, L(xl), L(yl), L(xr), L(yr));
	fflush (xout);
    }
#else
    if (etype < 2) {
	if (!goptPairBoundary) return 0;
	if (!etype && goptPairConductOnly) return 0;
    }
    else if (!goptDrawSubContact) return 0;
    if (F(xr) < xl_clip || (F(yl) < yb_clip && F(yr) < yb_clip)
     || F(xl) > xr_clip || (F(yl) > yt_clip && F(yr) > yt_clip)) return 0;
    if (xl == -INF || xr == INF) return 0;
    if (yl == -INF) {
	if (!goptPairToInfinity) return 0;
	yl = yb_clip;
    }
    if (yr == INF) {
	if (!goptPairToInfinity) return 0;
	yr = yt_clip;
    }
    X2dDrawLine (gcWhite, W(xl), W(yl), W(xr), W(yr));
#endif
    return 1;
}

#ifdef CONFIG_XSPACE
static int numscale;
Private char *getnum (char *s, int *num)
{
    int n, p = 1, sign = 0;
    while (*s == ' ') ++s;
    if (*s == '-') { ++s; ++sign; }
    if (!isdigit ((int)*s)) return (0);
    n = *s++ - '0';
    while (isdigit ((int)*s)) n = 10 * n + (*s++ - '0');
    if (*s == '.') while (isdigit ((int)*++s)) { n = 10 * n + (*s - '0'); p *= 10; }
    if (numscale == 2) n = 0.5 + n / (meters * 1e6); // microns -> internal
    else if (numscale == 1) n *= 4; // dbunits -> internal
    if (p > 1) *num = 0.5 + n / p;
    else *num = n;
    if (sign) *num = -*num;
    return (s);
}

void drawFile (char *file)
{
    char line[80], *s, *t;
    int xp, yp, zp, m, n, c;
    FILE *fp;

    numscale = 0;

    if (!(fp = fopen (file, "r"))) return;
    while (fgets (line, 80, fp)) {
	c = 't'; s = line; m = 0;
	switch (*s) {
	case 'x': c = 'x'; m = 2; break;
	case 'X': c = 'x'; m = 4; break;
	case 'p': c = 'p'; m = 2; break;
	case 'P': c = 'p'; m = 4; break;
	case 'h': c = 'h'; m = 2; break;
	case 'H': c = 'h'; m = 4; break;
	case 'v': c = 'v'; m = 2; break;
	case 'V': c = 'v'; m = 4; break;
	case 'l': c = 'l'; m = 8; break;
	case 'r': c = 'r'; m = 8; break;
	case 'y': c = 'y'; m = 8; break;
	case 'z': c = 'z'; m = 8; break;
	case 'i': numscale = 0; continue;
	case 'd': numscale = 1; continue;
	case 'm': numscale = 2; continue;
	default: if (isalpha ((int)*s)) ++m;
	}
	if (m) ++s;
	if (!(s = getnum (s, &xp))) continue;
	if (*s == ',') ++s;
	if (!(s = getnum (s, &yp))) continue;
	zp = 0;
	if (*s == ',' && !(s = getnum (s+1, &zp))) continue;
	if (m == 8) {
	    wcoor xwp, ywp, xw2, yw2;
	    int x2, y2, z2;
	    if (!(s = getnum (s, &x2))) continue;
	    if (*s == ',') ++s;
	    if (!(s = getnum (s, &y2))) continue;
	    z2 = zp;
	    if (*s == ',' && !(s = getnum (s+1, &z2))) continue;
	    xwp = xp; ywp = yp;
	    xw2 = x2; yw2 = y2;
	    if (goptDraw3D) {
		wcoor xw, yw;
		X3dProject (xwp, ywp, W(zp), &xw, &yw); xp = xw; yp = yw;
		X3dProject (xw2, yw2, W(z2), &xw, &yw); x2 = xw; y2 = yw;
		if (c != 'l') {
		    if (c == 'y') {
			X3dProject (xw2, ywp, W(z2), &xw2, &ywp);
			X3dProject (xwp, yw2, W(zp), &xwp, &yw2);
		    }
		    else if (c == 'z') {
			yw = ywp;
			X3dProject (xw2, yw2, W(zp), &xw2, &ywp);
			X3dProject (xwp, yw , W(z2), &xwp, &yw2);
		    }
		    else {
			X3dProject (xw2, ywp, W(zp), &xw2, &ywp);
			X3dProject (xwp, yw2, W(z2), &xwp, &yw2);
		    }
		}
	    }
	    else if (c == 'z') c = 'l';
	    if (xp <= xl_clip && x2 <= xl_clip) continue;
	    if (xp >= xr_clip && x2 >= xr_clip) continue;
	    if (yp <= yb_clip && y2 <= yb_clip) continue;
	    if (yp >= yt_clip && y2 >= yt_clip) continue;
	    if (c != 'l') {
		X2dDrawLine (gcWhite, W(xp), W(yp), xw2, ywp);
		X2dDrawLine (gcWhite, W(x2), W(y2), xw2, ywp);
		X2dDrawLine (gcWhite, W(x2), W(y2), xwp, yw2);
		X2dDrawLine (gcWhite, W(xp), W(yp), xwp, yw2);
		c = 'l'; // for text
	    }
	    else
		X2dDrawLine (gcWhite, W(xp), W(yp), W(x2), W(y2));
	    xp = x2; yp = y2; // for text
	}
	if (*s == ' ') ++s;
	t = s; n = 0;
	while (isprint ((int)*t)) { if (*t++ != ' ') ++n; }
	if (!n) s = 0; else *t = 0;
	if (c == 'l') {
	    if (!s) continue; // no text
	}
	else if (goptDraw3D) {
	    wcoor xw, yw;
	    X3dProject (W(xp), W(yp), W(zp), &xw, &yw);
	    xp = xw; yp = yw;
	}
	if (xp > xl_clip && xp < xr_clip && yp > yb_clip && yp < yt_clip)
	    X2dDrawDot (gcWhite, W(xp), W(yp), c, m, s);
    }
    fclose (fp);
}
#endif // CONFIG_XSPACE

int drawTile (tile_t *tile, int hascond)
{
#ifdef CONFIG_SPACE3D
    int drawSub = tile -> subcont && goptDrawSubContact;
    if (!goptDrawTile && !drawSub) return 0;
    if (xout) {
	fprintf (xout, "t%s %s %ld %ld %ld %ld %ld %ld\n",
	    tile -> subcont ? "2" : (hascond ? "1" : "0"), colorStr (&tile -> color),
	    L(tile -> xl), L(tile -> bl), L(tile -> tl), L(tile -> xr), L(tile -> br), L(tile -> tr));
	fflush (xout);
    }
#else
    GC gc;
    wcoor xl, bl, tl, xr, br, tr;
    int drawSub = (hascond == 2) && goptFillSubContact;

    if (!goptDrawTile && !drawSub) return 0;
    if (F(tile -> xr) < xl_clip || F(tile -> xl) > xr_clip) return 0;
    if (F(tile -> tl) < yb_clip && F(tile -> tr) < yb_clip) return 0;
    if (F(tile -> bl) > yt_clip && F(tile -> br) > yt_clip) return 0;

    if (drawSub)
	gc = goptDrawTile ? gcStippled (&tile -> color) : gcColor (&tile -> color, 1);
    else
	gc = gcColor (&tile -> color, 0);
    if (!gc)  return 0;

    if (tile -> xl == -INF) xl = xl_clip; else xl = tile -> xl;
    if (tile -> xr ==  INF) xr = xr_clip; else xr = tile -> xr;
    if (tile -> bl == -INF) { bl = br = yb_clip; } else { bl = tile -> bl; br = tile -> br; }
    if (tile -> tl ==  INF) { tl = tr = yt_clip; } else { tl = tile -> tl; tr = tile -> tr; }

    X2dFillTile (gc, xl, bl, tl, xr, br, tr);

    if (goptPairBoundary && use_pixel_copy_mode) { /* redraw edges */
	if (!hascond && goptPairConductOnly) return 1;
	if ((tile -> bl == -INF || tile -> tl == INF) && !goptPairToInfinity) goto l3;
	if (bl != tl && xl > xl_clip) X2dDrawLine (gcWhite, xl, bl, xl, tl);
	if (br != tr && xr < xr_clip) X2dDrawLine (gcWhite, xr, br, xr, tr);
l3:	if (tile -> tl !=  INF) X2dDrawLine (gcWhite, xl, tl, xr, tr);
	if (tile -> bl != -INF) X2dDrawLine (gcWhite, xl, bl, xr, br);
    }
#endif
    return 1;
}

int drawEdge (mask_t *colorp, coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
#ifdef CONFIG_SPACE3D
    if (!goptDrawEdge || xl == INF) return 0;
    if (xout) {
	fprintf (xout, "ec %s %ld %ld %ld %ld\n", colorStr (colorp), L(xl), L(yl), L(xr), L(yr));
	fflush (xout);
    }
#else
    GC gc;
    if (!goptDrawEdge
     || F(xr) < xl_clip || (F(yl) < yb_clip && F(yr) < yb_clip)
     || F(xl) > xr_clip || (F(yl) > yt_clip && F(yr) > yt_clip)) return 0;
    if (!(gc = gcEdgeColor (colorp))) return 0;
    X2dDrawLine (gc, W(xl), W(yl), W(xr), W(yr));
#endif
    return 1;
}

int drawScanPosition (coor_t x, coor_t y)
{
#ifdef CONFIG_SPACE3D
    if (xout) {
	fprintf (xout, "p %ld %ld\n", L(x), L(y));
	fflush (xout);
    }
#else
    wcoor wx, wy;
    if (!goptDrawPosition || goptRobot) return 0;
    wx = x;
    wy = y;
    if (wx < xl_clip) wx = xl_clip;
    else if (wx > xr_clip) wx = xr_clip;
    if (wy < yb_clip) wy = yb_clip;
    else if (wy > yt_clip) wy = yt_clip;
    X2dSetPointer (wx, wy);
#endif
    return 1;
}

int drawTriangleEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
    if (!goptDrawTriangle) return 0;
#ifdef CONFIG_SPACE3D
    if (xout) {
	if (xl > xr)
	    fprintf (xout, "f %ld %ld %ld %ld\n", L(xr), L(yr), L(xl), L(yl));
	else
	    fprintf (xout, "f %ld %ld %ld %ld\n", L(xl), L(yl), L(xr), L(yr));
	fflush (xout);
    }
#else
    if (F(xr) < xl_clip || (F(yl) < yb_clip && F(yr) < yb_clip)
     || F(xl) > xr_clip || (F(yl) > yt_clip && F(yr) > yt_clip)) return 0;
    X2dDrawLine (gcWhite, W(xl), W(yl), W(xr), W(yr));
#endif
    return 1;
}

int drawEquiEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
    if (!goptDrawEquiEdge) return 0;
#ifdef CONFIG_SPACE3D
    if (xout) {
	if (xl > xr)
	    fprintf (xout, "l %ld %ld %ld %ld\n", L(xr), L(yr), L(xl), L(yl));
	else
	    fprintf (xout, "l %ld %ld %ld %ld\n", L(xl), L(yl), L(xr), L(yr));
	fflush (xout);
    }
#else
    if (F(xr) < xl_clip || (F(yl) < yb_clip && F(yr) < yb_clip)
     || F(xl) > xr_clip || (F(yl) > yt_clip && F(yr) > yt_clip)) return 0;
    X2dDrawLine (gcHighlight (), W(xl), W(yl), W(xr), W(yr));
#endif
    return 1;
}

int drawResistor (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
#ifdef CONFIG_SPACE3D
    if (xout) {
	if (xl > xr)
	    fprintf (xout, "r %ld %ld %ld %ld\n", L(xr), L(yr), L(xl), L(yl));
	else
	    fprintf (xout, "r %ld %ld %ld %ld\n", L(xl), L(yl), L(xr), L(yr));
	fflush (xout);
    }
#else
    if (F(xr) < xl_clip || (F(yl) < yb_clip && F(yr) < yb_clip)
     || F(xl) > xr_clip || (F(yl) > yt_clip && F(yr) > yt_clip)) return 0;
#ifdef DEBUG
if(DEBUG) fprintf(stderr, "drawResistor: (%d,%d) - (%d,%d)\n", xl, yl, xr, yr);
#endif
    X2dDrawLine (gcDraw, W(xl), W(yl), W(xr), W(yr));
#endif
    return 1;
}

int drawOutResistor (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
    if (!goptOutResistor) return 0;
#ifdef CONFIG_SPACE3D
    if (xout) {
	if (xl > xr)
	    fprintf (xout, "ro %ld %ld %ld %ld\n", L(xr), L(yr), L(xl), L(yl));
	else
	    fprintf (xout, "ro %ld %ld %ld %ld\n", L(xl), L(yl), L(xr), L(yr));
	fflush (xout);
    }
#else
    if (F(xr) < xl_clip || (F(yl) < yb_clip && F(yr) < yb_clip)
     || F(xl) > xr_clip || (F(yl) > yt_clip && F(yr) > yt_clip)) return 0;
#ifdef DEBUG
if(DEBUG) fprintf(stderr, "drawOutResistor: (%d,%d) - (%d,%d)\n", xl, yl, xr, yr);
#endif
    X2dDrawLine (gcDraw2, W(xl), W(yl), W(xr), W(yr));
    X2dDrawDot (gcWhite, W(xl), W(yl), 'r', 4, 0);
    X2dDrawDot (gcWhite, W(xr), W(yr), 'r', 4, 0);
#endif
    return 1;
}

int drawSubResistor (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
    if (!goptDrawSubResistor) return 0;
#ifdef CONFIG_SPACE3D
    if (xout) {
	if (xl > xr)
	    fprintf (xout, "rs %ld %ld %ld %ld\n", L(xr), L(yr), L(xl), L(yl));
	else
	    fprintf (xout, "rs %ld %ld %ld %ld\n", L(xl), L(yl), L(xr), L(yr));
	fflush (xout);
    }
#else
    if (F(xr) < xl_clip || (F(yl) < yb_clip && F(yr) < yb_clip)
     || F(xl) > xr_clip || (F(yl) > yt_clip && F(yr) > yt_clip)) return 0;
#ifdef DEBUG
if(DEBUG) fprintf(stderr, "drawSubResistor: (%d,%d) - (%d,%d)\n", xl, yl, xr, yr);
#endif
    X2dDrawLine (gcDraw2, W(xl), W(yl), W(xr), W(yr));
#endif
    return 1;
}

int undrawResistor (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
#ifdef CONFIG_SPACE3D
    if (xout) {
	if (xl > xr)
	    fprintf (xout, "ru %ld %ld %ld %ld\n", L(xr), L(yr), L(xl), L(yl));
	else
	    fprintf (xout, "ru %ld %ld %ld %ld\n", L(xl), L(yl), L(xr), L(yr));
	fflush (xout);
    }
#else
    if (F(xr) < xl_clip || (F(yl) < yb_clip && F(yr) < yb_clip)
     || F(xl) > xr_clip || (F(yl) > yt_clip && F(yr) > yt_clip)) return 0;
#ifdef DEBUG
if(DEBUG) fprintf(stderr, "undrawResistor: (%d,%d) - (%d,%d)\n", xl, yl, xr, yr);
#endif
    X2dDrawLine (gcUnDraw, W(xl), W(yl), W(xr), W(yr));
#endif
    return 1;
}

int drawSpiderEdge (spider_t *sp1, spider_t *sp2, int contact)
{
#ifdef CONFIG_SPACE3D
    if (xout) {
	if (contact < 0) // substrate
	    fprintf (xout, "m %d %g %g 0 %g %g 0\n", (int)sp1 -> conductor,
		sp1 -> act_x, sp1 -> act_y, sp2 -> act_x, sp2 -> act_y);
	else
	    fprintf (xout, "m %d %g %g %g %g %g %g\n", contact ? -1 : (int)sp1 -> conductor,
		sp1 -> act_x, sp1 -> act_y, sp1 -> act_z,
		sp2 -> act_x, sp2 -> act_y, sp2 -> act_z);
	fflush (xout);
    }
#else
    GC gc = contact ? gcWhite : gcSpiderColor ((int)sp1 -> conductor);
    if (!gc) return 0;

    if (goptDraw3D) X3dDrawLine (gc,
	W(sp1 -> act_x), W(sp1 -> act_y), W(sp1 -> act_z),
	W(sp2 -> act_x), W(sp2 -> act_y), W(sp2 -> act_z));
    else X2dDrawLine (gc,
	W(sp1 -> act_x), W(sp1 -> act_y), W(sp2 -> act_x), W(sp2 -> act_y));
#endif
    return 1;
}

int drawGreen (double x1, double y1, double z1, double x2, double y2, double z2)
{
#ifdef CONFIG_SPACE3D
    if (xout) {
	fprintf (xout, "g %g %g %g %g %g %g\n", x1, y1, z1, x2, y2, z2);
	fflush (xout);
    }
#else
    static wcoor wx1, wy1, wz1, wx2, wy2, wz2;

    if (drawGreenLine) { /* clear old green line */
	if (drawGreenLine == 3) goto draw;
	if (goptDraw3D) X3dDrawLine (gcUnDraw, wx1, wy1, wz1, wx2, wy2, wz2);
	else X2dDrawLine (gcUnDraw, wx1, wy1, wx2, wy2);
	if (drawGreenLine == 2) { drawGreenLine = 0; return 0; }
    }

    wx1 = x1; wx2 = x2;
    wy1 = y1; wy2 = y2;
    wz1 = z1; wz2 = z2;
draw:
    drawGreenLine = 1;
    if (goptDraw3D) X3dDrawLine (gcDraw, wx1, wy1, wz1, wx2, wy2, wz2);
    else X2dDrawLine (gcDraw, wx1, wy1, wx2, wy2);
#endif
    return 1;
}

#ifdef CONFIG_SPACE3D
void drawDelaunay (DM_CELL *cellKey)
{
    long xl, yl, xr, yr;
    char buf[80];
    DM_STREAM *dms;

    if (!goptDrawDelaunay || !xout) return;

    do_not_die = 1;
    dms = dmOpenStream (cellKey, "delgraph", "r");
    do_not_die = 0;
    if (!dms) return;

    while (fscanf (dms -> dmfp, "%s %ld %ld %ld %ld", buf, &xl, &yl, &xr, &yr) == 5) {
	if (strsame (buf, "dashed")) {
	    fprintf (xout, "d %ld %ld %ld %ld\n", xl, yl, xr, yr);
	    fflush (xout);
	}
    }
    dmCloseStream (dms, COMPLETE);
}
#endif

int drawCapacitor (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
#ifdef CONFIG_SPACE3D
    if (xout) {
	if (xl > xr)
	    fprintf (xout, "c %ld %ld %ld %ld\n", L(xr), L(yr), L(xl), L(yl));
	else
	    fprintf (xout, "c %ld %ld %ld %ld\n", L(xl), L(yl), L(xr), L(yr));
	fflush (xout);
    }
#else
    if (F(xr) < xl_clip || (F(yl) < yb_clip && F(yr) < yb_clip)
     || F(xl) > xr_clip || (F(yl) > yt_clip && F(yr) > yt_clip)) return 0;
#ifdef DEBUG
if(DEBUG) fprintf(stderr, "drawCapacitor: (%d,%d) - (%d,%d)\n", xl, yl, xr, yr);
#endif
    X2dDrawLine (gcDraw, W(xl), W(yl), W(xr), W(yr));
#endif
    return 1;
}

int drawOutCapacitor (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
    if (!goptOutCapacitor) return 0;
#ifdef CONFIG_SPACE3D
    if (xout) {
	if (xl > xr)
	    fprintf (xout, "co %ld %ld %ld %ld\n", L(xr), L(yr), L(xl), L(yl));
	else
	    fprintf (xout, "co %ld %ld %ld %ld\n", L(xl), L(yl), L(xr), L(yr));
	fflush (xout);
    }
#else
    if (F(xr) < xl_clip || (F(yl) < yb_clip && F(yr) < yb_clip)
     || F(xl) > xr_clip || (F(yl) > yt_clip && F(yr) > yt_clip)) return 0;
#ifdef DEBUG
if(DEBUG) fprintf(stderr, "drawOutCapacitor: (%d,%d) - (%d,%d)\n", xl, yl, xr, yr);
#endif
    X2dDrawLine (gcDraw2, W(xl), W(yl), W(xr), W(yr));
#endif
    return 1;
}

int undrawCapacitor (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
#ifdef CONFIG_SPACE3D
    if (xout) {
	if (xl > xr)
	    fprintf (xout, "cu %ld %ld %ld %ld\n", L(xr), L(yr), L(xl), L(yl));
	else
	    fprintf (xout, "cu %ld %ld %ld %ld\n", L(xl), L(yl), L(xr), L(yr));
	fflush (xout);
    }
#else
    if (F(xr) < xl_clip || (F(yl) < yb_clip && F(yr) < yb_clip)
     || F(xl) > xr_clip || (F(yl) > yt_clip && F(yr) > yt_clip)) return 0;
#ifdef DEBUG
if(DEBUG) fprintf(stderr, "undrawCapacitor: (%d,%d) - (%d,%d)\n", xl, yl, xr, yr);
#endif
    X2dDrawLine (gcUnDraw, W(xl), W(yl), W(xr), W(yr));
#endif
    return 1;
}

#ifdef CONFIG_XSPACE
int getLine (char *buf, FILE *xin)
{
    int c, i = 0;
    while ((c = fgetc (xin)) == EOF) { // wait for begin line
	if (!extracting || space_ready || doEvent ()) return 0;
    }
    if (c != '\n') { // wait for end of line
	buf[i++] = c;
	while ((c = fgetc (xin)) != '\n') {
	    if (c != EOF) buf[i++] = c;
	    else if (!extracting || space_ready) return 0;
	}
    }
    buf[i++] = 0;
    return i; // i > 0
}

struct gposition {
    long pos;
    struct gposition *next;
};

void displayPicture (char *file, int bmode)
{
    static struct gposition *glist, *glast, *gfree;
    static FILE *xin;
    static long offset, offset2;
    static char buf[1024];
    static spider_t sp1, sp2;
    static tile_t tile;
    mask_t color;
    coor_t xl, yl, xr, yr;
    float x1, y1, z1, x2, y2, z2;
    int rv, n, b1, opened, bytes, gmode = 0;
    long pos;
    char *s;
    struct gposition *newpos;

    if (bmode) {
	removeobjs ();
	drawGreenLine = 0;
    }

    if (xin) { // already open (recursive call)
	if (bmode || !offset) return;
	fseek (xin, offset, 0); // set start position
	opened = 0;
    }
    else {
	if (!bmode && !offset) return;
	b1 = 0;
	while (!(xin = fopen (file, "r"))) {
	    if (space_ready && ++b1 > 5) return;
	}
	if (bmode) {
	    offset2 = offset = 0;
	    if (glist) {
		glast -> next = gfree;
		gfree = glist; glist = 0;
	    }
	}
	else fseek (xin, offset, 0); // set start position
	opened = 1;
    }
    newpos = glist;
    pos = offset; // current xin (byte) position

    b1 = prepass_1b; prepass_1b = 0;

    while ((bytes = getLine (buf, xin)) > 0) {
	if (gmode && *buf != 'g') { // bmode
	    gmode = 0;
	    if (gfree) { newpos = gfree; gfree = gfree -> next; }
	    else newpos = NEW (struct gposition, 1);
	    newpos -> pos = pos;
	    newpos -> next = 0;
	    if (glist) glast -> next = newpos;
	    else glist = newpos;
	    glast = newpos;
	}
	rv = 0;
	switch (*buf) {
	case '#':
	    switch (buf[1]) {
	    case 'e':
		if (buf[2] == 'n' && buf[3] == 'd') {
		    if (bmode) setText ("e");
		    goto endinput;
		}
		break;
	    case 'c':
		strcpy (cellname, buf+3);
		break;
	    case 's':
		rv = sscanf (buf+3, "%d %d %g", &inscale, &outscale, &x1); // lambda
		ASSERT (rv == 3);
		meters = (1e-6 * x1) / outscale;
		break;
	    case 'z':
		rv = sscanf (buf+3, "%g", &x1); // vdim z-max
		ASSERT (rv == 1);
		Z_top = x1 / meters;
		break;
	    case 'p':
		prepass_1b = (buf[2] == '1' && buf[3] == 'b');
		if (bmode) {
		    if (goptPauseAfterPP && offset && setPause ()) {
			showPicture (); // wait for end of pause
			if (interrupt) goto stopinput;
		    }
		    setText (buf);
		    if (!goptSaveImagePP || !offset) {
			if (offset && !goptSaveImagePP) clearDisplay ();
			offset = pos;
		    }
		}
	    }
	    rv = 0;
	    break;
	case 'w':
	    rv = sscanf (buf+2, "%d %d %d %d", &xl, &yl, &xr, &yr);
	    ASSERT (rv == 4);
	    bb_xl = xl; bb_xr = xr; bb_yb = yl; bb_yt = yr; // init cell bbox
	    if (!worldCoordinatesDone || !goptSaveImageEP) {
		opt3D = 0;
		if (goptXl != goptXr)
		    worldCoordinates (F(inscale * goptXl), F(inscale * goptXr),
				      F(inscale * goptYb), F(inscale * goptYt));
		else
		    worldCoordinates (bb_xl, bb_xr, bb_yb, bb_yt);
	    }
	    rv = 0;
	    break;
	case 'e':
	    s = buf+3;
	    if (buf[1] == 'c') {
		while (*s != ' ') ++s;
		*s++ = 0;
		initcolorhex (&color, buf+3);
	    }
	    rv = sscanf (s, "%d %d %d %d", &xl, &yl, &xr, &yr);
	    ASSERT (rv == 4);
	    if (buf[1] == 'c') rv = drawEdge (&color, xl, yl, xr, yr);
	    else rv = drawPairBoundary (xl, yl, xr, yr, (int)(buf[1] - '0'));
	    break;
	case 't':
	    s = buf+3;
	    while (*s != ' ') ++s;
	    *s++ = 0;
	    initcolorhex (&tile.color, buf+3);
	    rv = sscanf (s, "%d %d %d %d %d %d",
		&tile.xl, &tile.bl, &tile.tl, &tile.xr, &tile.br, &tile.tr);
	    ASSERT (rv == 6);
	    rv = drawTile (&tile, (int)(buf[1] - '0'));
	    break;
	case 'p':
	    rv = sscanf (buf+2, "%d %d", &xl, &yl);
	    ASSERT (rv == 2);
	    rv = drawScanPosition (xl, yl);
	    break;
	case 'm':
	    if (!goptDrawSpider) break;
	    rv = sscanf (buf+2, "%d %g %g %g %g %g %g", &n, &x1, &y1, &z1, &x2, &y2, &z2);
	    ASSERT (rv == 7);
	    sp1.conductor = n;
	    sp1.act_x = x1; sp1.act_y = y1; sp1.act_z = z1;
	    sp2.act_x = x2; sp2.act_y = y2; sp2.act_z = z2;
	    rv = drawSpiderEdge (&sp1, &sp2, (int)(n < 0));
	    break;
	case 'd':
	    if (goptDrawDelaunay) {
		rv = sscanf (buf+2, "%d %d %d %d", &xl, &yl, &xr, &yr);
		ASSERT (rv == 4);
		X2dDrawLine (gcDraw2, W(xl), W(yl), W(xr), W(yr));
	    }
	    break;
	case 'g':
	    if (bmode) {
		gmode = 1;
		if (goptDrawGreen) {
		    rv = sscanf (buf+2, "%g %g %g %g %g %g", &x1, &y1, &z1, &x2, &y2, &z2);
		    ASSERT (rv == 6);
		    rv = drawGreen (x1, y1, z1, x2, y2, z2);
		}
	    }
	    else {
		while (newpos && newpos -> pos < pos) newpos = newpos -> next;
		if (newpos) { // skip block of 'g' statements
		    pos = newpos -> pos;
		    fseek (xin, pos, 0); // goto new position
		    continue;
		}
		if (offset2 > pos) { // skip to last 'g' statement
		    pos = offset2;
		    fseek (xin, pos, 0); // goto new position
		    continue;
		}
	    }
	    break;
	case 'r':
	    rv = sscanf (buf+2, "%d %d %d %d", &xl, &yl, &xr, &yr);
	    ASSERT (rv == 4);
	    if (buf[1] == ' ' && bmode) rv = goptDrawResistor ? drawResistor (xl, yl, xr, yr) : 0;
	    if (buf[1] == 'o') rv = drawOutResistor (xl, yl, xr, yr);
	    if (buf[1] == 's') rv = drawSubResistor (xl, yl, xr, yr);
	    if (buf[1] == 'u' && bmode) rv = goptUnDrawResistor ? undrawResistor (xl, yl, xr, yr) : 0;
	    break;
	case 'c':
	    rv = sscanf (buf+2, "%d %d %d %d", &xl, &yl, &xr, &yr);
	    ASSERT (rv == 4);
	    if (buf[1] == ' ' && bmode) rv = goptDrawCapacitor ? drawCapacitor (xl, yl, xr, yr) : 0;
	    if (buf[1] == 'o') rv = drawOutCapacitor (xl, yl, xr, yr);
	    if (buf[1] == 'u' && bmode) rv = goptUnDrawCapacitor ? undrawCapacitor (xl, yl, xr, yr) : 0;
	    break;
	case 'f':
	    rv = sscanf (buf+2, "%d %d %d %d", &xl, &yl, &xr, &yr);
	    ASSERT (rv == 4);
	    rv = drawTriangleEdge (xl, yl, xr, yr);
	    break;
	case 'l':
	    rv = sscanf (buf+2, "%d %d %d %d", &xl, &yl, &xr, &yr);
	    ASSERT (rv == 4);
	    rv = drawEquiEdge (xl, yl, xr, yr);
	    break;
	}

	pos += bytes; // xin position after getLine

	if (bmode) {
	    if (rv) {
		showPicture (); // wait for end of delay
		if (interrupt) goto stopinput;
	    }
	    offset2 = pos;
	}
	else if (pos > offset2) {
	    draw_objects ();
	    if (drawGreenLine == 3) { /* redraw green line */
		drawGreen (x1, y1, z1, x2, y2, z2);
	    }
	    goto endinput;
	}
    }
stopinput:
    if (bmode) setText ("s");
endinput:
    if (bmode && drawGreenLine) { /* clear old green line */
	drawGreenLine = 2;
	drawGreen (x1, y1, z1, x2, y2, z2);
    }
    if (opened) { fclose (xin); xin = 0; }
    prepass_1b = b1;
 // print_objs ();
}
#endif
