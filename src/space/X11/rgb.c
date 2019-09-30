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
#include <ctype.h>
#include "src/space/include/config.h"

#include <X11/Xlib.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/extract/define.h"
#include "src/space/extract/export.h"
#include "src/space/X11/pane.h"
#include "src/space/X11/extern.h"

extern int prepass_1b;
extern elemDef_t * elemDefTab;
extern int elemDefTabSize;

#define FULL  65535		/* max gun intensity */
#define NR_COLORS 20		/* max color planes */

typedef struct {
union {
    struct {
	short assigned;
	unsigned short red, green, blue;
    } c;
    struct {
	short assigned, dummy;
	unsigned long pixel;
    } p;
} u;
} vcolor_t;

GC gcBlack = NULL;
GC gcWhite = NULL;
GC gcDraw  = NULL;
GC gcDraw2 = NULL;
GC gcUnDraw = NULL;

static XColor * Vmapco;
static Colormap cmap;
static GC gc_color = NULL;
static GC gc_glass = NULL;
static GC gc_high  = NULL;

static unsigned long white_pixel = 0;
static unsigned long black_pixel = 0;
static unsigned long grey_pixel = 0;

extern Display * display;
static Window    window;
static int num_planes;
int use_pixel_copy_mode;

static int BLACK_NR;	/* black color nr */
static int WHITE_NR;	/* white color nr */
static int VMAPSIZE;
static int *convertColor;

extern int nrOfMasks;
static int nrOfMasksGln = 0;
static int newMaskHasColor = 0;
static int nrOfColoredMasks = 0;
extern maskinfo_t * masktable;
extern int * conductorMask;
extern char ** maskdrawcolor;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private unsigned long getPixelValue (int color);
Private unsigned long allocPixel (int r, int g, int b);
Private int  doConvertColor (mask_t *colorp);
Private void initVmap (void);
#ifdef __cplusplus
  }
#endif

/* Init this module
 */
void initColors (Widget widget)
{
    XGCValues values;
    int screen_number;

    window = XtWindow (widget);
    ASSERT (window);
    screen_number = DefaultScreen (display);
    num_planes = PlanesOfScreen (ScreenOfDisplay (display, screen_number));

    if (num_planes <= 1) { /* monochrome */
	say ("Sorry, too less (%d) planes!", num_planes);
	die ();
    }
    Debug (say ("%d planes", num_planes));

    cmap = DefaultColormap (display, screen_number);

    /* using read only colormap */

    white_pixel = WhitePixel (display, screen_number);
    black_pixel = BlackPixel (display, screen_number);

    /* setBackground */
    {
	Arg arg[2];
	Debug (say ("set background: 0x%06X", black_pixel));
	XtSetArg (arg[0], XtNbackground, black_pixel);
	XtSetValues (widget, arg, 1);

	XClearWindow (display, window);
    }

    use_pixel_copy_mode = paramLookupB ("disp.use_pixel_copy_mode", "off");
    if (!use_pixel_copy_mode && (black_pixel != 0 || white_pixel != 0xFFFFFF)) {
	Debug (say ("using pixel_copy_mode, white_pixel = 0x%06X", white_pixel));
	use_pixel_copy_mode = 1;
    }
    values.function = use_pixel_copy_mode ? GXcopy : GXor;
    gc_color = XCreateGC (display, window, GCFunction, &values);
    ASSERT (gc_color);

    values.foreground = black_pixel;
    gcBlack = XCreateGC (display, window, GCForeground, &values);
    ASSERT (gcBlack);

    values.foreground = white_pixel;
    gcWhite = XCreateGC (display, window, GCForeground, &values);
    ASSERT (gcWhite);

    gcDraw = XCreateGC (display, window, GCForeground, &values);
    ASSERT (gcDraw);
    gcDraw2 = XCreateGC (display, window, GCForeground, &values);
    ASSERT (gcDraw2);
    gcUnDraw = XCreateGC (display, window, GCForeground, &values);
    ASSERT (gcUnDraw);

    XSetLineAttributes (display, gcDraw, 2, LineSolid, CapButt, JoinBevel);
    XSetLineAttributes (display, gcDraw2, 2, LineSolid, CapButt, JoinBevel);

    if (!white_pixel) values.foreground = black_pixel;
    values.function = GXxor;
    gc_high = XCreateGC (display, window, GCForeground | GCFunction, &values);
    ASSERT (gc_high);

    initVmap ();

    grey_pixel = allocPixel (FULL/2, FULL/2, FULL/2);
}

GC gcSpiderColor (int conductor_nr)
{
    unsigned long pixel;
    int new_nr = convertColor[conductorMask[conductor_nr]];

    if (new_nr < 0) return gc_glass;

	 if (new_nr == BLACK_NR) pixel = black_pixel;
    else if (new_nr == WHITE_NR) pixel = white_pixel;
    else pixel = Vmapco[new_nr].pixel;
    XSetForeground (display, gc_color, pixel);
    return gc_color;
}

GC gcEdgeColor (mask_t *colorp)
{
    unsigned long pixel;
    int i, new_nr;

    if (IS_COLOR (colorp)) {
        if (prepass_1b) { /* colorindex is conductor_nr */
            i = colorindex (colorp);
            return gcSpiderColor (i);
        }
	for (i = 0; i < nrOfMasks; i++)
	if (COLOR_EQ_COLOR (colorp, &masktable[i].color)) {
	    new_nr = convertColor[i];
	    if (new_nr < 0) break;
		 if (new_nr == BLACK_NR) pixel = black_pixel;
	    else if (new_nr == WHITE_NR) pixel = white_pixel;
	    else pixel = Vmapco[new_nr].pixel;
	    goto ret;
	}
	return gc_glass;
    }
    pixel = grey_pixel; /* mesh_gln */
ret:
    XSetForeground (display, gc_color, pixel);
    return gc_color;
}

GC gcColor (mask_t *colorp, int flag) /* if flag == 1 then make black and glass grey */
{
    unsigned long pixel;
    int newcolor;

    if (prepass_1b) { /* colorindex is conductor_nr */
        newcolor = colorindex (colorp);
        if (newcolor < 0) return gc_glass;
        return gcSpiderColor (newcolor);
    }

    newcolor = doConvertColor (colorp);
    if (flag && newcolor <= 0) { pixel = grey_pixel; goto ret; }
    if (newcolor < 0) return gc_glass;

    pixel = getPixelValue (newcolor);
ret:
    XSetForeground (display, gc_color, pixel);
    return gc_color;
}

GC gcHighlight ()
{
    return gc_high;
}

GC gcUnhighlight ()
{
    return gc_high;
}

GC gcStippled (mask_t *colorp) /* graphical context to draw dashed polygons */
{
    static GC gc = NULL;
    static char *sub_def_color;
    static unsigned long sub_def_pixel;
#define bitmap_width  16
#define bitmap_height 16
#define DRAW_DASHED
#ifdef  DRAW_DASHED
    static char bitmap_bits[] = { /* DRAW_DASHED */
	0x81, 0x81, 0xc0, 0xc0, 0x60, 0x60, 0x30, 0x30, 0x18, 0x18, 0x0c, 0x0c,
	0x06, 0x06, 0x03, 0x03, 0x81, 0x81, 0xc0, 0xc0, 0x60, 0x60, 0x30, 0x30,
	0x18, 0x18, 0x0c, 0x0c, 0x06, 0x06, 0x03, 0x03 };
#else
    static char bitmap_bits[] = { /* DRAW_STIPPLED */
	0x33, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x33,
	0x00, 0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00,
	0x33, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00 };
#endif
    unsigned long pixel;
    int newcolor;

    if (prepass_1b) { /* colorindex is conductor_nr */
	newcolor = colorindex (colorp);
	if (newcolor >= 0) newcolor = convertColor[conductorMask[newcolor]];
	if (newcolor < 0 || newcolor == BLACK_NR) pixel = black_pixel;
	else if (newcolor == WHITE_NR) pixel = white_pixel;
	else pixel = Vmapco[newcolor].pixel;
    }
    else {
	newcolor = doConvertColor (colorp);
	pixel = getPixelValue (newcolor);
    }

    if (!gc) {
	XGCValues values;
	Pixmap stipple;

        sub_def_color = paramLookupS ("disp.def_color_sub_con", "white");
	if (strsame (sub_def_color, "white"))
	    sub_def_pixel = white_pixel;
	else {
	    XColor color;
	    if (*sub_def_color == '@') *sub_def_color = '#';
	    if (XParseColor (display, cmap, sub_def_color, &color) == 0) {
		say ("def_color_sub_con: %s not found, using white", sub_def_color);
		sub_def_pixel = white_pixel;
	    }
	    else
		sub_def_pixel = allocPixel ((int)color.red, (int)color.green, (int)color.blue);
	}

	if ((stipple = XCreateBitmapFromData (display, window, bitmap_bits,
				  bitmap_width, bitmap_height)) == False) {
	    say ("Couldn't create bitmap");
	    die ();
	}
	values.function = use_pixel_copy_mode ? GXcopy : GXor;
	values.stipple = stipple;
	gc = XCreateGC (display, window, GCFunction | GCStipple, &values);
	ASSERT (gc);
	XSetFillStyle (display, gc, FillOpaqueStippled);
    }

    if (pixel == black_pixel) pixel = sub_def_pixel;

    XSetForeground (display, gc, pixel);
    return (gc);
}

/*
 * private procedures
 */

Private void initVmap ()
{
#ifdef DEBUG
    int R, G, B;
#endif
    int i, j, k;
    char buf[100];
    char * col;

    BLACK_NR = nrOfMasks;
    WHITE_NR = nrOfMasks + 1;
    convertColor = NEW (int, nrOfMasks);

    Vmapco = NEW (XColor, nrOfMasks);

    nrOfColoredMasks = j = 0;
    for (i = 0; i < nrOfMasks; i++) {
	convertColor[i] = -1;

	sprintf (buf, "disp.color_%s", masktable[i].name);
	col = paramLookupS (buf, maskdrawcolor[i]);
	if (strcmp (col, "glass") != 0) {
	    if (*col == '@') *col = '#';
	    if (XParseColor (display, cmap, col, &Vmapco[j]) == 0) {
		say ("Color %s not found, can't display mask %s",
		    col, masktable[i].name);
		continue;
	    }

	    if (masktable[i].gln) nrOfMasksGln = i + 1;
	    else if (i < nrOfMasks-1) newMaskHasColor = 1;

	    Vmapco[j].flags = DoRed | DoGreen | DoBlue;
#ifdef DEBUG
	    R = Vmapco[j].red;
	    G = Vmapco[j].green;
	    B = Vmapco[j].blue;
#endif
	    XAllocColor (display, cmap, &Vmapco[j]);
#ifdef DEBUG
	    if (DEBUG) fprintf (stderr, "alloc pixel 0x%06X: ask: %d %d %d, get: %d %d %d\n",
		Vmapco[j].pixel, R, G, B, Vmapco[j].red, Vmapco[j].green, Vmapco[j].blue);
#endif

	    /* Note: No color plane is reserved for black & white colors!
	    		Thus, we can minimize the VMAPSIZE (SdeG).
	    */
	    if (Vmapco[j].pixel == black_pixel) {
		convertColor[i] = BLACK_NR;
		continue;
	    }
	    if (Vmapco[j].pixel == white_pixel) {
		convertColor[i] = WHITE_NR;
		continue;
	    }

	    for (k = 0; k < j; ++k) {
		if (Vmapco[k].pixel == Vmapco[j].pixel) break;
	    }
	    convertColor[i] = k;
	    if (k < j) continue; /* found */
	    j++;
	    if (nrOfColoredMasks == NR_COLORS) {
		say ("More than %d different colors, using 'white' for tile mask %s",
		    NR_COLORS, masktable[i].name);
	    }
	    else nrOfColoredMasks++;
#ifdef DEBUG
	    if (DEBUG) fprintf (stderr, "color[%d] = %s, vmap[%d] = %d %d %d\n",
		k, col, (1 << k), Vmapco[k].red, Vmapco[k].green, Vmapco[k].blue);
#endif
	}
    }

    if (newMaskHasColor) {
	elemDef_t *el;
	newMaskHasColor = 0;
	for (i = elemDefTabSize; --i >= 0;) {
	    el = &elemDefTab[i];
	    if (el -> type != NEWMASKELEM) break;
	    if (convertColor[el -> s.newmask.mask] >= 0) { newMaskHasColor = 1; break; }
	    --elemDefTabSize;
	}
    }

    VMAPSIZE = 1 << nrOfColoredMasks;

#ifdef DEBUG
if (DEBUG) {
    for (i = 0; i < nrOfMasks; i++) {
	fprintf (stderr, "convertColor[%d] = %d\n", i, convertColor[i]);
    }
    fprintf (stderr, "nrOfColoredMasks=%d VMAPSIZE=%d (%d bytes)\n",
	nrOfColoredMasks, VMAPSIZE, VMAPSIZE * sizeof (vcolor_t));
    fprintf (stderr, "blackColor#=%d whiteColor#=%d\n", BLACK_NR, WHITE_NR);
}
#endif
}

Private unsigned long getPixelValue (int color)
{
    static vcolor_t * vmap = 0;

    if (color <= 0) return (black_pixel);
    if (color >= VMAPSIZE) return (white_pixel);

if (!vmap) {
    float r, g, b, I;
    int i, k, max;

    for (i = 0; i < nrOfColoredMasks; i++) {
	k = 1 << i;
	if (color == k) return (Vmapco[i].pixel);
    }

#ifdef DEBUG
    if (DEBUG) fprintf (stderr, "-- alloc vmap\n");
#endif
    vmap = NEW (vcolor_t, VMAPSIZE);

    for (i = 0; i < nrOfColoredMasks; i++) {
	k = 1 << i;
	vmap[k].u.c.red   = Vmapco[i].red;
	vmap[k].u.c.green = Vmapco[i].green;
	vmap[k].u.c.blue  = Vmapco[i].blue;
    }

    /* interpolate the vmap */

    for (i = 0; i < VMAPSIZE; i++) {
	vmap[i].u.c.assigned = 0;
	r = g = b = 0;
	max = 0;
	for (k = 1; k < i; k <<= 1) {
	    if (i & k) {
		r += vmap[k].u.c.red;
		g += vmap[k].u.c.green;
		b += vmap[k].u.c.blue;
		if (vmap[k].u.c.red   > max) max = vmap[k].u.c.red;
		if (vmap[k].u.c.green > max) max = vmap[k].u.c.green;
		if (vmap[k].u.c.blue  > max) max = vmap[k].u.c.blue;
	    }
	}
	if (!max) continue;
	I = r;
	if (b > I) I = b;
	if (g > I) I = g;
	I /= max;
	vmap[i].u.c.red   = r / I;
	vmap[i].u.c.green = g / I;
	vmap[i].u.c.blue  = b / I;
    }

    for (i = 0; i < nrOfColoredMasks; i++) {
	k = 1 << i;
	vmap[k].u.c.assigned = 1;
	vmap[k].u.p.pixel = Vmapco[i].pixel;
    }

    vmap[0].u.c.red   = 0;
    vmap[0].u.c.green = 0;
    vmap[0].u.c.blue  = 0;

#if 0
#ifdef DEBUG
    if (DEBUG) for (i = 0; i < VMAPSIZE; i++)
	fprintf (stderr, "vmap[%d] = %d %d %d\n", i,
	    vmap[i].u.c.red, vmap[i].u.c.green, vmap[i].u.c.blue);
#endif
#endif
}

    if (vmap[color].u.c.assigned == 0)  {
	int r, g, b;

	vmap[color].u.c.assigned = 1;	 /* assign it now */

	r = vmap[color].u.c.red;
	g = vmap[color].u.c.green;
	b = vmap[color].u.c.blue;

	vmap[color].u.p.pixel = allocPixel (r, g, b);
    }
    return (vmap[color].u.p.pixel);
}

Private unsigned long allocPixel (int r, int g, int b)
{
    XColor xcolor;

    xcolor.flags = DoRed | DoGreen | DoBlue;
    xcolor.red   = r;
    xcolor.green = g;
    xcolor.blue  = b;

    XAllocColor (display, cmap, &xcolor);
#ifdef DEBUG
    if (DEBUG) fprintf (stderr, "alloc pixel 0x%06X: ask: %d %d %d, get: %d %d %d\n",
	xcolor.pixel, r, g, b, xcolor.red, xcolor.green, xcolor.blue);
#endif
    return (xcolor.pixel);
}

Private int doConvertColor (mask_t *colorp)
{
    int i, newcolor = -1;

    if (IS_COLOR (colorp))
    for (i = 0; i < nrOfMasksGln; i++) if (convertColor[i] >= 0) {
	/* Only real masks have an own color bit (test for gln).
	* Use newMaskHasColor for the presence of unreal masks (SdeG).
	*/
	if (masktable[i].gln && !COLOR_ABSENT (colorp, &masktable[i].color)) {
	    if (convertColor[i] >= NR_COLORS && convertColor[i] != BLACK_NR) {
		/* A too high color index results in white_pixel.
		* Return directly, because white is most dominant!
		*/
		return (VMAPSIZE);
	    }
	    if (newcolor == 0 || convertColor[i] == BLACK_NR) newcolor = 0;
	    else {
		if (newcolor < 0) newcolor = 0;
		newcolor |= 1 << convertColor[i];
	    }
	}
    }

    if (newMaskHasColor) {
	elemDef_t *el;
	int m;
	for (i = elemDefTabSize; --i >= 0;) {
	    el = &elemDefTab[i];
	    if (el -> type != NEWMASKELEM) break;
	    if (!COLOR_PRESENT (colorp, &el -> sBitPresent)) continue;
	    if (!COLOR_ABSENT  (colorp, &el -> sBitAbsent )) continue;
	    if (convertColor[m = el -> s.newmask.mask] >= 0) {
		if (convertColor[m] >= NR_COLORS && convertColor[m] != BLACK_NR)
		    return (VMAPSIZE);
		if (newcolor == 0 || convertColor[m] == BLACK_NR) newcolor = 0;
		else {
		    if (newcolor < 0) newcolor = 0;
		    newcolor |= 1 << convertColor[m];
		}
	    }
	}
    }

    return (newcolor);
}
