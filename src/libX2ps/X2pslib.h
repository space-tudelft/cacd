/*
 * ISC License
 *
 * Copyright (C) 1994-2018 by
 *	P.C. van der Wekken
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

#ifndef _X2pslib_h
#define _X2pslib_h

#include <stdio.h>
/*
#include <sys/types.h>
#include <sys/stat.h>
*/
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#if defined(__STDC__) || defined(__cplusplus)
#define _FARGS_(s) s
#else
#define _FARGS_(s) ()
#endif

#define XpCOLOR True
#define XpBW False

#define XpFILE False
#define XpPRINTER True

#define XpDISPLAY 0
#define XpEPS 1
#define XpPS 2
#define XpGETBBOX 10

#define XpPORTRAIT False
#define XpLANDSCAPE True

typedef struct {
	char*	XLFD;
	char*	PSfont;
}XpFontmap;

typedef struct {
	char*		id;
	Display*	display;
	int			flag;
	char*		outputfile;
	FILE*		PSfile;
	int			llx;
	int			lly;
	int			width;
	int			height;
	char*		fontmapfile;
	XpFontmap*	xpfontmap;
	Colormap	colormap;
	Boolean		colormode;
	Boolean		drawbackground;
	Pixel		background;
}XpStruct;


/* Xlib functions */

extern void XpClearArea(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    Bool		/* exposures */
#endif
);

extern void XpClearWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);

extern void XpCopyArea(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* src */,
    Drawable		/* dest */,
    GC			/* gc */,
    int			/* src_x */,
    int			/* src_y */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    int			/* dest_x */,
    int			/* dest_y */
#endif
);

extern void XpDrawArc(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    int			/* angle1 */,
    int			/* angle2 */
#endif
);

extern void XpDrawImageString(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    const char*		/* string */,
    int			/* length */
#endif
);

extern void XpDrawLine(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x1 */,
    int			/* x2 */,
    int			/* y1 */,
    int			/* y2 */
#endif
);

extern void XpDrawLines(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    XPoint*		/* points */,
    int			/* npoints */,
    int			/* mode */
#endif
);

extern void XpDrawRectangle(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */
#endif
);

extern void XpDrawSegments(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    XSegment*		/* segments */,
    int			/* nsegments */
#endif
);

extern void XpDrawString(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    const char*		/* string */,
    int			/* length */
#endif
);

extern void XpFillArc(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    int			/* angle1 */,
    int			/* angle2 */
#endif
);

extern void XpFillPolygon(
#if NeedFunctionPrototypes
    Display*            /* display */,
    Drawable            /* d */,
    GC                  /* gc */,
    XPoint*             /* points */,
    int                 /* npoints */,
    int                 /* shape */,
    int                 /* mode */
#endif
);

extern void XpFillRectangle(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */
#endif
);

extern void XpFillRectangles(
#if NeedFunctionPrototypes
    Display*            /* display */,
    Drawable            /* d */,
    GC                  /* gc */,
    XRectangle*         /* rectangles */,
    int                 /* nrectangles */
#endif
);

#endif /* _X2pslib_h */
