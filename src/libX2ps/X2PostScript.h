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

#ifndef _X2PostScript_h
#define _X2PostScript_h

/******************************************************
 *                                                    *
 * X2PostScript Widget                                *
 *                                                    *
 ******************************************************/

#if defined(__STDC__) || defined(__cplusplus)
#define _FARGS_(s) s
#else
#define _FARGS_(s) ()
#endif

#ifdef __cplusplus
#define DM_FAKE_USE(a) (a) = (a)
#else
#define DM_FAKE_USE(a)
#endif

typedef enum {
    XpX2POSTSCRIPT_ERROR,
    XpX2POSTSCRIPT_WARNING,
    XpX2POSTSCRIPT_FATAL_ERROR
} XpX2PostScriptTypeError;

/* Start Putte defines */
/*
#define XpX2ERROR XpX2POSTSCRIPT_ERROR
#define XpX2WARNING XpX2POSTSCRIPT_WARNING
#define XpX2FATAL_ERROR XpX2POSTSCRIPT_FATAL_ERROR
*/
/*
typedef enum {
	XpX2ERROR,
	XpX2WARNING,
	XpX2FATAL_ERROR
} XpX2PostScriptTypeError;
*/
/* End Putte defines */

typedef struct {
    int	                    code;
    XpX2PostScriptTypeError err_type;
    int                     argint;
    char                    *argstr1;
    char                    *argstr2;
} XpX2PostScriptErrorCallbackStruct;

/* symbolic error codes */
#define XpX2POSTSCRIPT_NO_OPEN_FILE     1    /* could not open file */
#define XpX2POSTSCRIPT_NO_REDRAW_FUN    2    /* no redraw function */
#define XpX2POSTSCRIPT_NO_REDRAW_WIDGET 3    /* no redraw widget */
#define XpX2POSTSCRIPT_FONT_MAPPED      4    /* font is already mapped */
#define XpX2POSTSCRIPT_WRONG_SYNTAX     5    /* not the right syntax */
#define XpX2POSTSCRIPT_NO_OPEN_FONTFILE 6    /* could not open fontmap file */
#define XpX2POSTSCRIPT_MALLOCFAIL       7    /* malloc call failed */
#define XpX2POSTSCRIPT_CALLOCFAIL       8    /* calloc call failed */
#define XpX2POSTSCRIPT_REALLOCFAIL       9    /* realloc call failed */

/* Start Putte defines */
/*
#define XPX2_NO_OPEN_FILE        XpX2POSTSCRIPT_NO_OPEN_FILE
#define XPX2_NO_REDRAW_FUN        XpX2POSTSCRIPT_NO_REDRAW_FUN
#define XPX2_NO_REDRAW_WID        XpX2POSTSCRIPT_NO_REDRAW_WIDGET
#define XPX2_FONT_MAPPED        XpX2POSTSCRIPT_FONT_MAPPED
#define XPX2_WRONG_SYNTAX        XpX2POSTSCRIPT_WRONG_SYNTAX
#define XPX2_NO_OPEN_FONTFILE    XpX2POSTSCRIPT_NO_OPEN_FONTFILE
#define XPX2_MALLOCFAIL            XpX2POSTSCRIPT_MALLOCFAIL
#define XPX2_CALLOCFAIL            XpX2POSTSCRIPT_CALLOCFAIL
#define XPX2_REALLOCFAIL        XpX2POSTSCRIPT_REALLOCFAIL
*/
/* End Putte defines */

/* Resources names */
#define XtNxpinfo                            "xpinfo"
#define XtNdrawWidget                        "drawWidget"

#define XtNfileOrPrinter                     "fileOrPrinter"
#define XtNfileOrPrinterLabelString          "fileOrPrinterLabelString"

#define XtNoutputFileLabelString             "outputFileLabelString"
#define XtNx2PostScriptFile                  "x2PostScriptFile"
#define XtNx2PostScriptDir                   "x2PostScriptDir"
#define XtNoutputFileName                    "outputFileName"
#define XtNformat                            "format"

#define XtNprintCommandLabelString           "printCommandLabelString"
#define XtNprintCommand                      "printCommand"

#define XtNportraitLandscape                 "portraitLandscape"

#define XtNcolorMode                         "colorMode"

#define XtNdrawBackground                    "drawBackground"

#define XtNfontmapFile                       "fontmapFile"
#define XtNfontmapDir                        "fontmapDir"

#define XtNokCallback                        "okCallback"
#define XtNapplyCallback                     "applyCallback"

#ifdef XtNcancelCallback
#undef XtNcancelCallback
#endif
#define XtNcancelCallback                    "cancelCallback"

#define XtNredrawSetupCallback               "redrawSetupCallback"
#define XtNredrawCallback                    "redrawCallback"

#ifdef XtNerrorCallback
#undef XtNerrorCallback
#endif
#define XtNerrorCallback                     "errorCallback"

#define XtNbusyCursor                        "busyCursor"

#define XtNdpi                               "dpi"
#define XtNpaperWidth                        "paperWidth"
#define XtNpaperWidthOffset                  "paperWidthOffset"
#define XtNpaperHeight                       "paperHeight"
#define XtNpaperHeightOffset                 "paperHeightOffset"

/* Class types */

#define XtCXpInfo                            "XpInfo"
#define XtCDrawWidget                        "DrawWidget"

#define XtCFileOrPrinter                     "FileOrPrinter"
#define XtCFileOrPrinterLabelString          "FileOrPrinterLabelString"

#define XtCOutputFileLabelString             "OutputFileLabelString"
#define XtCX2PostScriptFile                  "X2PostScriptFile"
#define XtCX2PostScriptDir                   "X2PostScriptDir"
#define XtCOutputFileName                    "OutputFileName"
#define XtCFormat                            "Format"

#define XtCPrintCommandLabelString           "PrintCommandLabelString"
#define XtCPrintCommand                      "PrintCommand"

#define XtCPortraitLandscape                 "PortraitLandscape"

#define XtCColorMode                         "ColorMode"

#define XtCDrawBackground                    "DrawBackground"

#define XtCFontmapFile                       "FontmapFile"
#define XtCFontmapDir                        "FontmapDir"

#define XtCOkCallback                        "OkCallback"
#define XtCApplyCallback                     "ApplyCallback"
#define XtCCancelCallback                    "CancelCallback"
#define XtCHelpCallback                      "HelpCallback"
#define XtCRedrawSetupCallback               "RedrawSetupCallback"
#define XtCRedrawCallback                    "RedrawCallback"
#define XtCErrorCallback                     "ErrorCallback"

#define XtCBusyCursor                        "BusyCursor"

#define XtCDpi                               "Dpi"
#define XtCPaperWidth                        "PaperWidth"
#define XtCPaperWidthOffset                  "PaperWidthOffset"
#define XtCPaperHeight                       "PaperHeight"
#define XtCPaperHeightOffset                 "PaperHeightOffset"

/* child type defines for XpX2PostScriptWidgetGetChild */
/* token from Xm.h, defines similar to XmDIALOG_xxx... */
#define XpX2POSTSCRIPT_NONE                XmDIALOG_NONE
#define XpX2POSTSCRIPT_APPLY_BUTTON        XmDIALOG_APPLY_BUTTON
#define XpX2POSTSCRIPT_CANCEL_BUTTON       XmDIALOG_CANCEL_BUTTON
#define XpX2POSTSCRIPT_DEFAULT_BUTTON      XmDIALOG_DEFAULT_BUTTON
#define XpX2POSTSCRIPT_OK_BUTTON           XmDIALOG_OK_BUTTON
#define XpX2POSTSCRIPT_HELP_BUTTON         XmDIALOG_HELP_BUTTON

/* declare specific X2PostScriptWidget class and instance datatypes */
typedef struct _XpX2PostScriptClassRec*    XpX2PostScriptWidgetClass;
typedef struct _XpX2PostScriptRec*         XpX2PostScriptWidget;

/* declare the class constant */

extern WidgetClass xpX2PostScriptWidgetClass;

/* public procedures */
#ifdef __cplusplus
extern "C" {
#endif

/* wim: do not use preprocessing directive within macro arg !!!! */
/* (at least newer gcc do not allow this)                        */
#ifdef NeedWidePrototypes
extern Widget XpX2PostScriptWidgetGetChild _FARGS_((Widget widget,
                                                    unsigned int child));
#else
extern Widget XpX2PostScriptWidgetGetChild _FARGS_((Widget widget,
                                                    unsigned char child));
#endif
#ifdef __cplusplus
}
#endif

#endif /* _X2PostScript_h */
