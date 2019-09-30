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

#ifndef _X2PostScriptP_h
#define _X2PostScriptP_h

#include <X11/IntrinsicP.h>
/* include superclass private header file */
#include <Xm/BulletinBP.h>

/* include public header file for this widget */
#include "src/libX2ps/X2PostScript.h"
#include "src/libX2ps/X2pslib.h"

#include <stdio.h>
/*#include "dmfargs.h"*/

/* New fields for the X2PostScript widget class record */
typedef struct {
	int dummy;
} XpX2PostScriptClassPart;

typedef struct _XpX2PostScriptClassRec {
	CoreClassPart				core_class;
	CompositeClassPart			composite_class;
	ConstraintClassPart			constraint_class;
	XmManagerClassPart			manager_class;
	XmBulletinBoardClassPart	bulletinboard_class;
	XpX2PostScriptClassPart		x2postscript_class;
} XpX2PostScriptClassRec;

extern XpX2PostScriptClassRec xpX2PostScriptClassRec;

typedef struct {
    /* resources */
	Widget		draw_widget;		/* the drawwidget to print*/
	XpStruct	*Xpinfo;			/* the Xpinfo struct */
	char		*PSfilename;
	Boolean		FileOrPrinter;
	Boolean		PortraitLandscape;
	Boolean		ColorMode;
	Boolean		DrawBackground;
	char*		printcommand;
	char*		fontmapfile;
	char*		fontmapdir;
	char*		X2PostScriptfile;
	char*		X2PostScriptdir;
	char*		outputfilename;
	int			Format;
	Cursor		busycursor;

	char*		outputfilelabelstring;
	char*		printcommandlabelstring;

	int			dpi;
	float		paperwidth;
	float		paperwidthoffset;
	float		paperheight;
	float		paperheightoffset;

	XtCallbackList	okCallback; /* function for print */
	XtCallbackList	applyCallback; /* function for print */
	XtCallbackList	cancelCallback; /* function for cancel */
	XtCallbackList  redrawSetupCallback; /* function for setup redraw */
	XtCallbackList  redrawCallback; /* function for redraw */
	XtCallbackList  errorCallback; /* function for error */

    /* private state */
	Widget		PrintWidgetFrame;
	Widget		PrintWidget;

	Widget		FileOrPrinterLabel;
	Widget		FileOrPrinterRadio;
	Widget		FileRadioButton;
	Widget		PrinterRadioButton;

	Widget		OutputLabel;
	Widget		OutputText;

	Widget		OutputFileLabel;
	Widget		OutputFileText;

	Widget		FormatPulldownMenu;
	Widget		FormatOptionMenu;
	Widget		PSOptionButton;
	Widget		EPSOptionButton;

	Widget		PrintCommandLabel;
	Widget		PrintCommandText;

	Widget		PortraitLandscapeLabel;
	Widget		PortraitLandscapeRadio;
	Widget		PortraitRadioButton;
	Widget		LandscapeRadioButton;

	Widget		ColorLabel;
	Widget		ColorRadio;
	Widget		ColorRadioButton;
	Widget		BWRadioButton;

	Widget		BackgroundToggleButton;

	Widget		Hori_Sep1;

	Widget		ButtonBox;

	Widget		PSFileSelectionBox;

    Dimension	oldw;                   /* prev width of widget */
    Dimension	oldh;                   /* prev height of widget */
} XpX2PostScriptPart;

typedef struct _XpX2PostScriptRec {
    CorePart			core;
    CompositePart		composite;
    ConstraintPart		constraint;
    XmManagerPart		manager;
	XmBulletinBoardPart	bulletinboard;
    XpX2PostScriptPart	x2PostScript;
} XpX2PostScriptRec;

/* function prototypes for ANSI C / C++ */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* _X2PostScriptP_h  */
