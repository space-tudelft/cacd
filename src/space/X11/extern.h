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

#include "src/space/X11/export.h"

#ifdef __cplusplus
extern "C" {
#endif

/* draw */
double setCamera (int mode, int value);

/* exposel.c */
void RedisplayLabel (Widget);

/* interact.c */
void MyMainLoop (void);
void redrawPicture (int);
void showPicture (void);
void xMessage (char *str);
void cbSelectCell (Widget widget);

/* rgb.c */
void initColors (Widget widget);
GC gcSpiderColor (int conductor_nr);
GC gcEdgeColor (mask_t *colorp);
GC gcColor (mask_t *colorp, int flag);
GC gcStippled (mask_t *colorp);
GC gcHighlight (void);
GC gcUnhighlight (void);

/* robot.c */
void robotInit (void);
void robotStop (void);
int  robotStart (void);
void robot (XEvent *event);
void printEvent (XEvent *event);

/* x2d.c */
void X2dDrawLine (GC gc, wcoor xl, wcoor yl, wcoor xr, wcoor yr);
void X2dFillTile (GC gc, wcoor xl, wcoor bl, wcoor tl, wcoor xr, wcoor br, wcoor tr);
void X2dFlush (void);
void X2dSetPointer (wcoor wx, wcoor wy);
void X2dSetWorldCoordinates (wcoor xl, wcoor xr, wcoor yb, wcoor yt);
void clearDisplay (void);
void initCanvas (Widget widget);

/* x3d.c */
void X3dSetViewReferencePoint (wcoor x, wcoor y, wcoor z);
void X3dProject (wcoor x, wcoor y, wcoor z, wcoor *x_return, wcoor *y_return);
void X3dProjectReverse (wcoor x, wcoor y, wcoor z, wcoor *x_return, wcoor *y_return);
void X3dDrawLine (GC gc, wcoor x1, wcoor y1, wcoor z1, wcoor x2, wcoor y2, wcoor z2);
void X3dSetSimplePerspective (wcoor longitude, wcoor latitude, wcoor distance);
void X3dSetWorldCoordinates (wcoor x1, wcoor x2, wcoor y1, wcoor y2, wcoor z1, wcoor z2);
void X3dInit (canvas_t *canvas);

#ifdef __cplusplus
}
#endif

extern mask_t cNull;
