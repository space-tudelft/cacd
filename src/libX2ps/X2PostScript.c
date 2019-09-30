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
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>   /* for bcopy */
#include <string.h>    /* for strchr etc */
#include <malloc.h>

#ifdef X11R4
#ifdef __cplusplus
#define class   xtclass
#define new     xtnew
#endif
#endif

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>     /* XNM can be removed when all uses of
				 * XtN... and XtC... are replaced by Xm... */
#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/Label.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
/* XNM
*/
#include <X11/Shell.h>

#ifdef X11R4
#ifdef __cplusplus
#undef class
#undef new
#endif
#endif

#include "src/libX2ps/X2pslib.h"
#include "src/libX2ps/X2psapi.h"
#include "src/libX2ps/X2PostScriptP.h"

/* uncomment next line for debug information */
/*#define DEBUGME*/

#ifdef DEBUGME
#define IFDEBUG(A) A
#else
#define IFDEBUG(A)
#endif

#define XpHasErrorCallback(x2psw) XtHasCallbacks((Widget)x2psw, \
XtNerrorCallback) == XtCallbackHasSome

/* functions visible to other modules */

/* local procedures */
static void FileOrPrinterChange _FARGS_((Widget w, XtPointer client_data,
					 XtPointer call_data));
static void FormatOption _FARGS_((Widget w, XtPointer client_data,
				  XtPointer call_data));
static void PortraitLandscapeChange _FARGS_((Widget w, XtPointer client_data,
					     XtPointer call_data));
static void ColorModeChange _FARGS_((Widget w, XtPointer client_data,
				     XtPointer call_data));
static void BackgroundToggle _FARGS_((Widget w, XtPointer client_data,
				      XtPointer call_data));

static void Print _FARGS_((Widget w, XtPointer client_data,
			   XtPointer call_data));
static void Ok _FARGS_((Widget w, XtPointer client_data,
			XtPointer call_data));
static void Apply _FARGS_((Widget w, XtPointer client_data,
			   XtPointer call_data));
static void Cancel _FARGS_((Widget w, XtPointer client_data,
			    XtPointer call_data));
static void create_children _FARGS_((Widget nju));
static void Initialize _FARGS_((Widget request, Widget nju,
				ArgList args, Cardinal *num_args));
static void Realize _FARGS_((Widget w, XtValueMask *valueMask,
			     XSetWindowAttributes *attributes));
static void Resize _FARGS_((Widget w));
static void Destroy _FARGS_((Widget w));
static void ChangeManaged _FARGS_((Widget w));
static XtGeometryResult GeometryManager _FARGS_((Widget w,
						 XtWidgetGeometry *request,
						 XtWidgetGeometry *reply));
static XtGeometryResult QueryGeometry _FARGS_((Widget w,
					       XtWidgetGeometry *intended,
					       XtWidgetGeometry *preferred));
static Boolean SetValues _FARGS_((Widget current, Widget request, Widget nju,
				  ArgList args, Cardinal *num_arg));
#if 0
static void destroyshell _FARGS_((Widget w, XtPointer client_data,
				  XtPointer call_data));
#endif
static void estimate_dimensions_x2postscriptwidget _FARGS_((XpX2PostScriptWidget w,
							    XtWidgetGeometry *x2pswg ));
static void UpdateOutputFields _FARGS_((XpX2PostScriptWidget x2psw));
static void UpdateOutputFileName _FARGS_((XpX2PostScriptWidget x2psw));
static void UpdateFormatOptionMenu _FARGS_((XpX2PostScriptWidget x2psw));

static XtResource resources[] = {
#define offset(field) XtOffset(XpX2PostScriptWidget, x2PostScript.field)
    /* {name, class, type, size, offset, default_type, default_addr}, */
    { XtNxpinfo, XtCXpInfo,
	  XtRPointer, sizeof(caddr_t), offset(Xpinfo),
	  XtRPointer, (caddr_t)NULL },

    { XtNdrawWidget, XtCDrawWidget,
	  XmRWidget, sizeof(Widget), offset(draw_widget),
	  XmRWidget, (XtPointer) NULL },

    { XtNfileOrPrinter, XtCFileOrPrinter,
	  XtRBoolean, sizeof(Boolean), offset(FileOrPrinter),
	  XtRImmediate, (caddr_t)XpFILE },

    { XtNoutputFileLabelString, XtCOutputFileLabelString,
	  XtRString, sizeof(String), offset(outputfilelabelstring),
	  XtRString, (caddr_t)"Outputfile :"},
    { XtNx2PostScriptDir, XtCX2PostScriptDir,
	  XtRString, sizeof(String), offset(X2PostScriptdir),
	  XtRString, (caddr_t) "." },
    { XtNx2PostScriptFile, XtCX2PostScriptFile,
	  XtRString, sizeof(String), offset(X2PostScriptfile),
	  XtRString, (caddr_t) "X2ps.ps" },
    { XtNoutputFileName, XtCOutputFileName,
	  XtRString, sizeof(String), offset(outputfilename),
	  XtRString, (caddr_t) NULL },
    { XtNformat, XtCFormat,
	  XtRInt, sizeof(int), offset(Format),
	  XtRImmediate, (caddr_t)XpPS },

    { XtNprintCommandLabelString, XtCPrintCommandLabelString,
	  XtRString, sizeof(String), offset(printcommandlabelstring),
	  XtRString, (caddr_t)"Printcommand :"},
    { XtNprintCommand, XtCPrintCommand,
	  XtRString, sizeof(String), offset(printcommand),
	  XtRString, (caddr_t)"lpr" },

    { XtNportraitLandscape, XtCPortraitLandscape,
	  XtRBoolean, sizeof(Boolean), offset(PortraitLandscape),
	  XtRImmediate, (caddr_t)XpPORTRAIT },

    { XtNcolorMode, XtCColorMode,
	  XtRBoolean, sizeof(Boolean), offset(ColorMode),
	  XtRImmediate, (caddr_t)XpCOLOR },

    { XtNdrawBackground, XtCDrawBackground,
	  XtRBoolean, sizeof(Boolean), offset(DrawBackground),
	  XtRImmediate, (caddr_t)False },
    { XtNfontmapDir, XtCFontmapDir,
	  XtRString, sizeof(String), offset(fontmapdir),
	  XtRString, (caddr_t) "." },
    { XtNfontmapFile, XtCFontmapFile,
	  XtRString, sizeof(String), offset(fontmapfile),
	  XtRString, (caddr_t) ".X2PSfontmapfile" },

    { XtNokCallback, XtCOkCallback,
	  XtRCallback, sizeof(XtCallbackList), offset(okCallback),
	  XtRCallback, (caddr_t) NULL },
    { XtNapplyCallback, XtCApplyCallback,
	  XtRCallback, sizeof(XtCallbackList), offset(applyCallback),
	  XtRCallback, (caddr_t) NULL },
    { XtNcancelCallback, XtCCancelCallback,
	  XtRCallback, sizeof(XtCallbackList), offset(cancelCallback),
	  XtRCallback, (caddr_t) NULL },
    { XtNredrawSetupCallback, XtCRedrawSetupCallback,
	  XtRCallback, sizeof(XtCallbackList), offset(redrawSetupCallback),
	  XtRCallback, (caddr_t) NULL },
    { XtNredrawCallback, XtCRedrawCallback,
	  XtRCallback, sizeof(XtCallbackList), offset(redrawCallback),
	  XtRCallback, (caddr_t) NULL },
    { XtNerrorCallback, XtCErrorCallback,
	  XtRCallback, sizeof(XtCallbackList), offset(errorCallback),
	  XtRCallback, (caddr_t) NULL },

    { XtNbusyCursor, XtCBusyCursor,
	  XtRCursor, sizeof(Cursor), offset(busycursor),
	  XtRString, (caddr_t) "watch" },

    { XtNdpi, XtCDpi,
	  XtRInt, sizeof(int), offset(dpi),
	  XtRImmediate, (caddr_t) 72},
    { XtNpaperWidth, XtCPaperWidth,
	  XtRFloat, sizeof(float), offset(paperwidth),
	  XtRString, (caddr_t) "8.25"},
    { XtNpaperWidthOffset, XtCPaperWidthOffset,
	  XtRFloat, sizeof(float), offset(paperwidthoffset),
	  XtRString, (caddr_t) ".25"},
    { XtNpaperHeight, XtCPaperHeight,
	  XtRFloat, sizeof(float), offset(paperheight),
	  XtRString, (caddr_t) "11.66"},
    { XtNpaperHeightOffset, XtCPaperHeightOffset,
	  XtRFloat, sizeof(float), offset(paperheightoffset),
	  XtRString, (caddr_t) ".25"},
#undef offset
};

XpX2PostScriptClassRec xpX2PostScriptClassRec = {
    { /* core fields */
      /* superclass        */    (WidgetClass) &xmBulletinBoardClassRec,
      /* class_name        */    "XpX2PostScript",
      /* widget_size        */    sizeof(XpX2PostScriptRec),
      /* class_initialize        */    NULL, /* ClassInitialize, */
      /* class_part_initialize    */    NULL,
      /* class_inited        */    FALSE,
      /* initialize        */    Initialize,
      /* initialize_hook        */    NULL,
      /* realize            */    Realize,
      /* actions            */    NULL,
      /* num_actions        */    0,
      /* resources        */    resources,
      /* num_resources        */    XtNumber(resources),
      /* xrm_class        */    NULLQUARK,
      /* compress_motion        */    TRUE,
      /* compress_exposure    */    TRUE,
      /* compress_enterleave    */	TRUE,
      /* visible_interest        */    FALSE,
      /* destroy            */    Destroy,
      /* resize            */    Resize,
      /* expose            */    NULL,
      /* set_values        */    SetValues,
      /* set_values_hook        */    NULL,
      /* set_values_almost    */    XtInheritSetValuesAlmost,
      /* get_values_hook        */    NULL,
      /* accept_focus        */    NULL,
      /* version            */    XtVersion,
      /* callback_private        */    NULL,
      /* tm_table            */    NULL,
      /* query_geometry        */    QueryGeometry,
      /* display_accelerator    */    XtInheritDisplayAccelerator,
      /* extension        */    NULL
    },
    { /* composite class fields */
      /* geometry manager        */    GeometryManager,
      /* change_managed        */    ChangeManaged,
      /* insert_child        */    XtInheritInsertChild,
      /* delete_child        */    XtInheritDeleteChild,
      /* extension        */    NULL
    },
    { /* constraint_class fields */
      /* resources */     NULL,
      /* num_resources  */     0,
      /* constraint_size */     0,
      /* initialize */     NULL,
      /* destroy */     NULL,
      /* set_values */     NULL,
      /* extension */     NULL
    },
    { /* manager_class fields */
      /* translations                    */  XtInheritTranslations,
      /* syn_resources                    */  NULL,
      /* num_syn_resources                */  0,
      /* syn_constraint_resources        */  NULL,
      /* num_syn_constraint_resources    */  0,
      /* parent_process                    */  XmInheritParentProcess,
      /* extension                        */  NULL
    },
    {   /* bulletinboard_class fields */
        /* always_install_accelerators */ FALSE,
        /* geo_matrix_create           */ NULL,
        /* focus_moved_proc            */ NULL,
        /* extension                   */ NULL
    },
    { /* x2PostScript fields */
      /* dummy            */    0
    }
};

WidgetClass xpX2PostScriptWidgetClass = (WidgetClass)&xpX2PostScriptClassRec;

static void set_default_labelString(w, default_string)
    Widget w;
    char* default_string;
{
    Arg arg;
    XmString LabelString;
    XmString CompareString;

    XtSetArg(arg, XmNlabelString,&LabelString);
    XtGetValues(w,&arg,1);
    CompareString =
        XmStringCreateSimple(w->core.name);
    if(XmStringCompare(LabelString,CompareString)) {
        XmStringFree(LabelString);
        LabelString = XmStringCreateSimple(default_string);
        XtSetArg(arg, XmNlabelString,LabelString);
        XtSetValues(w,&arg,1);
    }
    XmStringFree(LabelString);
    XmStringFree(CompareString);
}

static void create_children(nju)
    Widget nju;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget) nju;
    XpX2PostScriptPart *x2psp = &(x2psw->x2PostScript);
    Widget OutputForm;
    int n;
    Arg arg[20];
    XmString SimpleString;
    XmString CompareString;
    XtCallbackRec callbacklist[3];

#ifdef BORDERFRAME
    n = 0;
    x2psp->PrintWidgetFrame = XmCreateFrame(nju, "PrintWidgetFrame", arg, n);
    XtManageChild(x2psp->PrintWidgetFrame);
#endif /* BORDERFRAME */

    n = 0;
    XtSetArg(arg[n], XmNresizable, True); n++;
#ifndef BORDERFRAME
    x2psp->PrintWidget = XmCreateForm(nju, "PrintWidget", arg, n);
#else /* BORDERFRAME */
    x2psp->PrintWidget = XmCreateForm(x2psp->PrintWidgetFrame, "PrintWidget", arg, n);
#endif /* BORDERFRAME */
    XtManageChild(x2psp->PrintWidget);

    n = 0;
    XtSetArg(arg[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNdialogType, XmDIALOG_PROMPT); n++;
    XtSetArg(arg[n], XmNallowOverlap, False); n++;
    XtSetArg(arg[n], XmNnoResize, False); n++;
    x2psp->ButtonBox = XmCreateSelectionBox(x2psp->PrintWidget, "ButtonBox", arg, n);
    XtManageChild(XmSelectionBoxGetChild(x2psp->ButtonBox, XmDIALOG_APPLY_BUTTON));
    XtUnmanageChild(XmSelectionBoxGetChild(x2psp->ButtonBox, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmSelectionBoxGetChild(x2psp->ButtonBox, XmDIALOG_TEXT));
    XtUnmanageChild(XmSelectionBoxGetChild(x2psp->ButtonBox, XmDIALOG_SELECTION_LABEL));
    XtManageChild(x2psp->ButtonBox);

    XtAddCallback(XmSelectionBoxGetChild(x2psp->ButtonBox, XmDIALOG_OK_BUTTON),
		  XmNactivateCallback, Ok, NULL);
    XtAddCallback(XmSelectionBoxGetChild(x2psp->ButtonBox, XmDIALOG_APPLY_BUTTON),
                  XmNactivateCallback, Apply, NULL);
    XtAddCallback(XmSelectionBoxGetChild(x2psp->ButtonBox, XmDIALOG_CANCEL_BUTTON),
                  XmNactivateCallback, Cancel, NULL);

    n = 0;
    XtSetArg(arg[n], XmNleftAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNtopAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNrightAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNresizable, True); n++;
    OutputForm = XmCreateForm(x2psp->PrintWidget, "OutputForm", arg, n);
    XtManageChild(OutputForm);

    n = 0;
    XtSetArg(arg[n], XmNleftAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNtopAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNmarginHeight,10);  n++;
    XtSetArg(arg[n], XmNmarginLeft,10);  n++;
    x2psp->FileOrPrinterLabel = XmCreateLabel(OutputForm, "FileOrPrinterLabel", arg, n);
    XtManageChild(x2psp->FileOrPrinterLabel);
    set_default_labelString(x2psp->FileOrPrinterLabel,"Output to :");

    callbacklist[0].callback = FileOrPrinterChange;
    callbacklist[0].closure = NULL;
    callbacklist[1].callback = NULL;
    callbacklist[1].closure = NULL;
    n = 0;
    XtSetArg(arg[n], XmNorientation,XmHORIZONTAL); n++;
    XtSetArg(arg[n], XmNentryCallback, callbacklist); n++;
    XtSetArg(arg[n], XmNtopAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNleftAttachment,XmATTACH_WIDGET); n++;
    XtSetArg(arg[n], XmNleftWidget,x2psp->FileOrPrinterLabel); n++;
    x2psp->FileOrPrinterRadio = XmCreateRadioBox(OutputForm, "FileOrPrinterRadio", arg, n);
    XtManageChild(x2psp->FileOrPrinterRadio);

    n = 0;
    x2psp->FileRadioButton = XmCreateToggleButton(x2psp->FileOrPrinterRadio, "FileRadioButton", arg, n);
    XtManageChild(x2psp->FileRadioButton);
    set_default_labelString(x2psp->FileRadioButton,"File");

    n = 0;
    x2psp->PrinterRadioButton = XmCreateToggleButton(x2psp->FileOrPrinterRadio, "PrinterRadioButton", arg, n);
    XtManageChild(x2psp->PrinterRadioButton);
    set_default_labelString(x2psp->PrinterRadioButton,"Printer");

    n = 0;
    XtSetArg(arg[n], XmNset, True); n++;
    switch(x2psp->FileOrPrinter) {
        case XpFILE:
        XtSetValues(x2psp->FileRadioButton,arg, n);
        break;
        case XpPRINTER:
        XtSetValues(x2psp->PrinterRadioButton,arg, n);
        break;
        default:
        XtSetValues(x2psp->FileRadioButton,arg, n);
    }

    n = 0;
    XtSetArg(arg[n], XmNleftAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNtopAttachment,XmATTACH_WIDGET); n++;
    XtSetArg(arg[n], XmNtopWidget,x2psp->FileOrPrinterLabel); n++;
    XtSetArg(arg[n], XmNrightAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNmarginLeft,10);  n++;
    XtSetArg(arg[n], XmNalignment,XmALIGNMENT_BEGINNING); n++;
    x2psp->OutputLabel = XmCreateLabel(OutputForm, "OutputLabel", arg, n);
    XtManageChild(x2psp->OutputLabel);

    callbacklist[0].callback = FormatOption;
    callbacklist[0].closure = NULL;
    callbacklist[1].callback = NULL;
    callbacklist[1].closure = NULL;
    n = 0;
    XtSetArg(arg[n], XmNentryCallback, callbacklist); n++;
    x2psp->FormatPulldownMenu = XmCreatePulldownMenu(OutputForm,
                                                     "FormatPulldownMenu",
                                                     arg,
                                                     n);

    n = 0;
    x2psp->PSOptionButton = XtCreateManagedWidget("PS",
                                                  xmPushButtonGadgetClass,
                                                  x2psp->FormatPulldownMenu,
                                                  arg,
                                                  n);
    n = 0;
    x2psp->EPSOptionButton = XtCreateManagedWidget("EPS",
                                                   xmPushButtonGadgetClass,
                                                   x2psp->FormatPulldownMenu,
                                                   arg,
                                                   n);

    SimpleString = XmStringCreateSimple("");
    n = 0;
    XtSetArg(arg[n], XmNtopAttachment,XmATTACH_WIDGET); n++;
    XtSetArg(arg[n], XmNtopWidget,x2psp->OutputLabel); n++;
    XtSetArg(arg[n], XmNrightAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNsubMenuId,x2psp->FormatPulldownMenu); n++;
    XtSetArg(arg[n], XmNlabelString, SimpleString); n++;
    x2psp->FormatOptionMenu = XmCreateOptionMenu(OutputForm,
                                                 "FormatOptionMenu",
                                                 arg,
                                                 n);
    XtManageChild(x2psp->FormatOptionMenu);
    XmStringFree(SimpleString);

    n = 0;
    switch(x2psp->Format) {
        case XpEPS:
        XtSetArg(arg[n], XmNmenuHistory, x2psp->EPSOptionButton); n++;
        break;
        case XpPS:
        XtSetArg(arg[n], XmNmenuHistory, x2psp->PSOptionButton); n++;
        break;
        default:
        XtSetArg(arg[n], XmNmenuHistory, x2psp->PSOptionButton); n++;
    }
    XtSetValues(x2psp->FormatOptionMenu,arg,n);

    n = 0;
    XtSetArg(arg[n], XmNleftAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNrightAttachment,XmATTACH_WIDGET); n++;
    XtSetArg(arg[n], XmNrightWidget,x2psp->FormatOptionMenu); n++;
    XtSetArg(arg[n], XmNtopAttachment,XmATTACH_WIDGET); n++;
    XtSetArg(arg[n], XmNtopWidget,x2psp->OutputLabel); n++;
    XtSetArg(arg[n], XmNmarginWidth,10);  n++;
    x2psp->OutputText = XmCreateText(OutputForm,
                                     "OutputText",
                                     arg,
                                     n);
    XtManageChild(x2psp->OutputText);

    n = 0;
    XtSetArg(arg[n], XmNtopAttachment,XmATTACH_WIDGET); n++;
/*    XtSetArg(arg[n], XmNtopWidget,x2psp->FormatOptionMenu); n++;*/
    XtSetArg(arg[n], XmNtopWidget,OutputForm); n++;
    XtSetArg(arg[n], XmNleftAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNrightAttachment,XmATTACH_FORM); n++;
    x2psp->Hori_Sep1 = XmCreateSeparator(x2psp->PrintWidget,
                                         "Hori_Sep1",
                                         arg,
                                         n);
    XtManageChild(x2psp->Hori_Sep1);

    n = 0;
    XtSetArg(arg[n], XmNleftAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNtopAttachment,XmATTACH_WIDGET); n++;
    XtSetArg(arg[n], XmNtopWidget,x2psp->Hori_Sep1); n++;
    XtSetArg(arg[n], XmNmarginWidth,10);  n++;
    XtSetArg(arg[n], XmNmarginTop,10);  n++;
    x2psp->PortraitLandscapeLabel = XmCreateLabel(x2psp->PrintWidget,
                                                  "PortraitLandscapeLabel",
                                                  arg,
                                                  n);
    XtManageChild(x2psp->PortraitLandscapeLabel);
    set_default_labelString(x2psp->PortraitLandscapeLabel,"Orientation :");

    callbacklist[0].callback = PortraitLandscapeChange;
    callbacklist[0].closure = NULL;
    callbacklist[1].callback = NULL;
    callbacklist[1].closure = NULL;
    n = 0;
    XtSetArg(arg[n], XmNorientation,XmHORIZONTAL); n++;
    XtSetArg(arg[n], XmNentryCallback, callbacklist); n++;
    XtSetArg(arg[n], XmNleftAttachment,XmATTACH_WIDGET); n++;
    XtSetArg(arg[n], XmNleftWidget,x2psp->PortraitLandscapeLabel); n++;
    XtSetArg(arg[n], XmNtopAttachment,XmATTACH_WIDGET); n++;
    XtSetArg(arg[n], XmNtopWidget,x2psp->Hori_Sep1); n++;
    x2psp->PortraitLandscapeRadio =    XmCreateRadioBox(x2psp->PrintWidget,
                                                     "PortraitLandscapeRadio",
                                                     arg,
                                                     n);
    XtManageChild(x2psp->PortraitLandscapeRadio);

    n = 0;
    x2psp->PortraitRadioButton =
        XmCreateToggleButton(x2psp->PortraitLandscapeRadio,
                                   "PortraitRadioButton",
                                   arg,
                                   n);
    XtManageChild(x2psp->PortraitRadioButton);
    set_default_labelString(x2psp->PortraitRadioButton,"Portrait");

    n = 0;
    x2psp->LandscapeRadioButton =
        XmCreateToggleButton(x2psp->PortraitLandscapeRadio,
                                   "LandscapeRadioButton",
                                   arg,
                                   n);
    XtManageChild(x2psp->LandscapeRadioButton);
    set_default_labelString(x2psp->LandscapeRadioButton,"Landscape");

    n = 0;
    XtSetArg(arg[n], XmNset, True); n++;
    switch(x2psp->PortraitLandscape) {
        case XpPORTRAIT:
        XtSetValues(x2psp->PortraitRadioButton,arg, n);
        break;
        case XpLANDSCAPE:
        XtSetValues(x2psp->LandscapeRadioButton,arg, n);
        break;
        default:
        XtSetValues(x2psp->PortraitRadioButton,arg, n);
    }

    n = 0;
    XtSetArg(arg[n], XmNleftAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNtopAttachment,XmATTACH_WIDGET); n++;
    XtSetArg(arg[n], XmNtopWidget,x2psp->PortraitLandscapeRadio); n++;
    XtSetArg(arg[n], XmNmarginWidth,10);  n++;
    XtSetArg(arg[n], XmNmarginHeight,10);  n++;
    x2psp->ColorLabel = XmCreateLabel(x2psp->PrintWidget,
                                      "ColorLabel",
                                      arg,
                                      n);
    XtManageChild(x2psp->ColorLabel);
    set_default_labelString(x2psp->ColorLabel,"Color :");

    callbacklist[0].callback = ColorModeChange;
    callbacklist[0].closure = NULL;
    callbacklist[1].callback = NULL;
    callbacklist[1].closure = NULL;
    n = 0;
    XtSetArg(arg[n], XmNtopAttachment,XmATTACH_WIDGET); n++;
    XtSetArg(arg[n], XmNtopWidget,x2psp->PortraitLandscapeRadio); n++;
    XtSetArg(arg[n], XmNleftAttachment,XmATTACH_WIDGET); n++;
    XtSetArg(arg[n], XmNleftWidget,x2psp->ColorLabel); n++;
    XtSetArg(arg[n], XmNorientation,XmHORIZONTAL); n++;
    XtSetArg(arg[n], XmNentryCallback, callbacklist); n++;
    x2psp->ColorRadio = XmCreateRadioBox(x2psp->PrintWidget,
                                            "ColorRadio",
                                            arg,
                                            n);

    XtManageChild(x2psp->ColorRadio);

    n = 0;
    x2psp->ColorRadioButton = XmCreateToggleButton(x2psp->ColorRadio,
                                                         "ColorRadioButton",
                                                         arg,
                                                         n);
    XtManageChild(x2psp->ColorRadioButton);
    set_default_labelString(x2psp->ColorRadioButton,"Color");

    n = 0;
    x2psp->BWRadioButton = XmCreateToggleButton(x2psp->ColorRadio,
                                                      "BWRadioButton",
                                                      arg,
                                                      n);

    XtManageChild(x2psp->BWRadioButton);
    set_default_labelString(x2psp->BWRadioButton,"B/W");

    n = 0;
    XtSetArg(arg[n], XmNset, True); n++;
    switch(x2psp->ColorMode) {
        case XpCOLOR:
        XtSetValues(x2psp->ColorRadioButton,arg, n);
        break;
        case XpBW:
        XtSetValues(x2psp->BWRadioButton,arg, n);
        break;
        default:
        XtSetValues(x2psp->ColorRadioButton,arg, n);
    }

    n = 0;
    XtSetArg(arg[n], XmNtopAttachment,XmATTACH_WIDGET); n++;
    XtSetArg(arg[n], XmNtopWidget,x2psp->ColorLabel); n++;
    XtSetArg(arg[n], XmNleftAttachment,XmATTACH_FORM); n++;
    XtSetArg(arg[n], XmNleftOffset,1); n++;
    XtSetArg(arg[n], XmNmarginWidth,10);  n++;
    XtSetArg(arg[n], XmNset,x2psp->DrawBackground); n++;
    x2psp->BackgroundToggleButton =
        XmCreateToggleButton(x2psp->PrintWidget,
                             "BackgroundToggleButton",
                             arg,
                             n);
    XtManageChild(x2psp->BackgroundToggleButton);
    XtAddCallback(x2psp->BackgroundToggleButton,
                  XmNvalueChangedCallback,
                  BackgroundToggle,
                  (XtPointer)x2psw);

    n = 0;
    XtSetArg(arg[n], XmNlabelString,&SimpleString); n++;
    XtGetValues(x2psp->BackgroundToggleButton,arg,n);
    CompareString =
        XmStringCreateSimple(x2psp->BackgroundToggleButton->core.name);
    if(XmStringCompare(SimpleString,CompareString)) {
        XmStringFree(SimpleString);
        SimpleString = XmStringCreateSimple("Draw Background");
        n = 0;
        XtSetArg(arg[n], XmNlabelString,SimpleString); n++;
        XtSetValues(x2psp->BackgroundToggleButton,arg,n);
    }
    XmStringFree(SimpleString);
    XmStringFree(CompareString);
}

#if 0
/*ARGSUSED*/
static void destroyshell(w, client_data, call_data)
    Widget w;
    XtPointer client_data; /* UNUSED */
    XtPointer call_data;  /* UNUSED */
{
    DM_FAKE_USE(client_data);
    DM_FAKE_USE(call_data);
    XtDestroyWidget((Widget)XtParent(w));
}
#endif

static void Realize(w, valueMask, attributes)
    Widget w;
    XtValueMask *valueMask;
    XSetWindowAttributes *attributes;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget) w;
    XpX2PostScriptPart *x2psp = &(x2psw->x2PostScript);
    XtWidgetGeometry x2pswg;

    IFDEBUG(printf("X2PostScript():Realize() \n"));

    estimate_dimensions_x2postscriptwidget(x2psw,&x2pswg);
    XtResizeWidget(x2psp->PrintWidget,x2pswg.width,x2pswg.height,
                   x2psw->core.border_width);
#ifdef BORDERFRAME
    XtResizeWidget(x2psp->PrintWidgetFrame,x2pswg.width,x2pswg.height,
                   x2psw->core.border_width);
#endif /* BORDERFRAME */
    XtMakeResizeRequest((Widget)x2psw,x2pswg.width,x2pswg.height,
                        NULL,NULL);

   /* use Realize method from superclass */
/*
    (*(compositeClassRec.core_class.realize))(w, valueMask, attributes);
*/
    (*(xmBulletinBoardClassRec.core_class.realize))(w, valueMask, attributes);

}

static void Initialize(request, nju, args, num_args)
    Widget request, nju;
    ArgList args;
    Cardinal *num_args;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget) nju;
    XpX2PostScriptPart *x2psp = &(x2psw->x2PostScript);
    XtWidgetGeometry x2pswg;
    XpX2PostScriptErrorCallbackStruct xpx2pserr;

    DM_FAKE_USE(args);
    DM_FAKE_USE(num_args);

    IFDEBUG(printf("X2PostScript:Initialize()\n"));

    x2psp->Xpinfo = XpStructCreate();
    x2psp->PSfilename = NULL;

    if(x2psp->outputfilelabelstring)
        x2psp->outputfilelabelstring =
            XtNewString(x2psp->outputfilelabelstring);
    if(x2psp->printcommandlabelstring)
        x2psp->printcommandlabelstring =
            XtNewString(x2psp->printcommandlabelstring);

    if(x2psp->printcommand)
        x2psp->printcommand = XtNewString(x2psp->printcommand);

    if(x2psp->fontmapfile) {
        x2psp->fontmapfile = XtNewString(x2psp->fontmapfile);
    }
    if(x2psp->fontmapdir) {
        x2psp->fontmapdir = XtNewString(x2psp->fontmapdir);
    }
    if(x2psp->X2PostScriptfile)
        x2psp->X2PostScriptfile = XtNewString(x2psp->X2PostScriptfile);
    if(x2psp->X2PostScriptdir)
        x2psp->X2PostScriptdir = XtNewString(x2psp->X2PostScriptdir);
    if(x2psp->outputfilename)
        free(x2psp->outputfilename);
    x2psp->outputfilename = calloc(strlen(x2psp->X2PostScriptdir)+
                                   strlen(x2psp->X2PostScriptfile)+2,
                                   sizeof(char));
    if (!x2psp->outputfilename) {
        if (XpHasErrorCallback(x2psw)) {
            xpx2pserr.code = XpX2POSTSCRIPT_CALLOCFAIL;
            xpx2pserr.err_type = XpX2POSTSCRIPT_ERROR;
            xpx2pserr.argint = 0;
            xpx2pserr.argstr1 = "Initialize()";
            xpx2pserr.argstr2 = NULL;
            XtCallCallbacks((Widget)x2psw,
                            XtNerrorCallback,
                            (caddr_t) &xpx2pserr);
            return;
        }
        else {
            printf("X2PostScript: Calloc fail in Initialize()\n");
            return;
        }
    }
    strcpy(x2psp->outputfilename,x2psp->X2PostScriptdir);
    strcat(x2psp->outputfilename,"/");
    strcat(x2psp->outputfilename,x2psp->X2PostScriptfile);
    UpdateOutputFileName(x2psw);

   /* create children */
    create_children((Widget)x2psw);
    UpdateOutputFields(x2psw);
    estimate_dimensions_x2postscriptwidget(x2psw, &x2pswg);

    /*
     * Why resize in initialise, not yet realised.....
     */
/*
    if (request->core.width == 0)
        x2psw->core.width = x2pswg.width;
    if (request->core.height == 0)
        x2psw->core.height = x2pswg.height;
    XtResizeWidget(x2psp->PrintWidget, x2psw->core.width, x2psw->core.height,
                   x2psw->core.border_width);
*/
    /*
     * If the extra frame borders the widget
     */
#ifdef BORDERFRAME
/*
     XtResizeWidget(x2psp->PrintWidgetFrame, x2psw->core.width,
                   x2psw->core.height, x2psw->core.border_width);
*/
#endif /* BORDERFRAME */
    x2psp->oldw = x2psw->core.width;
    x2psp->oldh = x2psw->core.height;

}

static void Destroy(w)
    Widget w;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget) w;
    XpX2PostScriptPart *x2psp = &(x2psw->x2PostScript);

    /* destroy private data */
#ifdef X2PS_DEBUG
    printf("Destroy private data\n");
#endif
    XpStructFree(x2psp->Xpinfo);
    /*
    printf("XpStructFree done\n");
    */
    if(x2psp->PSfilename)
        free(x2psp->PSfilename);
    /*
    printf("PSfilename done\n");
    */
    if(x2psp->outputfilelabelstring)
        free(x2psp->outputfilelabelstring);
    /*
    printf("outputfilelabelstring done\n");
    */
    if(x2psp->printcommandlabelstring)
        free(x2psp->printcommandlabelstring);
    /*
    printf("printcommandlabelstring done\n");
    */
    if(x2psp->printcommand)
        free(x2psp->printcommand);
    /*
    printf("printcommand done\n");
    */
    if(x2psp->fontmapdir)
        free(x2psp->fontmapdir);
    /*
    printf("fontmapdir done\n");
    */
    if(x2psp->fontmapfile)
        free(x2psp->fontmapfile);
    /*
    printf("fontmapfile done\n");
    */
    if(x2psp->X2PostScriptdir)
        free(x2psp->X2PostScriptdir);
    /*
    printf("X2PostScriptdir done\n");
    */
    if(x2psp->X2PostScriptfile)
        free(x2psp->X2PostScriptfile);
    /*
    printf("X2PostScriptfile done\n");
    */
}

/*ARGSUSED*/
static void estimate_dimensions_x2postscriptwidget(x2psw, x2pswg)
    XpX2PostScriptWidget x2psw;
    XtWidgetGeometry *x2pswg;
{
    XpX2PostScriptPart *x2psp;
    XtWidgetGeometry intended, preferred;
    Dimension width,height;
    Dimension maxwidth,maxheight;

    IFDEBUG(printf("estimate_dimensions()\n"));

    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return; /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);

    x2pswg->width = x2psp->PrintWidget->core.width;
    x2pswg->height = x2psp->PrintWidget->core.height;
    IFDEBUG(printf("%18s width = %3d height = %3d\n","PrintWidget",
                   x2pswg->width,x2pswg->height));

    intended.request_mode = XtCWQueryOnly | CWWidth | CWHeight;
    intended.width = 0;
    intended.height = 0;

    XtQueryGeometry(x2psp->FileOrPrinterLabel,&intended,&preferred);
    maxwidth = preferred.width;
    height = preferred.height;
    XtQueryGeometry(x2psp->FileOrPrinterRadio,&intended,&preferred);
    maxwidth += preferred.width;
    if(height<preferred.height)
        height = preferred.height;
    maxheight = height;
    IFDEBUG(printf("%18s w = %3d h = %3d mw = %3d mh = %3d\n","FileOrPrinter",
                   preferred.width,preferred.height,maxwidth,maxheight));

    XtQueryGeometry(x2psp->OutputLabel,&intended,&preferred);
/*    if(preferred.width>maxwidth)
        maxwidth = preferred.width;*/
    maxheight += preferred.height;
    IFDEBUG(printf("%18s w = %3d h = %3d mw = %3d mh = %3d\n","OutputLabel",
                   preferred.width,preferred.height,maxwidth,maxheight));

    XtQueryGeometry(x2psp->OutputText,&intended,&preferred);
    height = preferred.height;
    XtQueryGeometry(x2psp->FormatOptionMenu,&intended,&preferred);
    if(height<preferred.height)
        height = preferred.height;
    maxheight += height;
    IFDEBUG(printf("%18s w = %3d h = %3d mw = %3d mh = %3d\n","OutputText",
                   preferred.width,preferred.height,maxwidth,maxheight));

    XtQueryGeometry(x2psp->PortraitLandscapeLabel,&intended,&preferred);
    width = preferred.width;
    height = preferred.height;
    XtQueryGeometry(x2psp->PortraitLandscapeRadio,&intended,&preferred);
    width += preferred.width;
    if(width>maxwidth)
        maxwidth = width;
    if(height<preferred.height)
        height = preferred.height;
    maxheight += height;
    IFDEBUG(printf("%18s w = %3d h = %3d mw = %3d mh = %3d\n","PortraitLandscape",
                   preferred.width,preferred.height,maxwidth,maxheight));

    XtQueryGeometry(x2psp->ColorLabel,&intended,&preferred);
    width = preferred.width;
    height = preferred.height;
    XtQueryGeometry(x2psp->ColorRadio,&intended,&preferred);
    width += preferred.width;
    if(width>maxwidth)
        maxwidth = width;
    if(height<preferred.height)
        height = preferred.height;
    maxheight += height;
    IFDEBUG(printf("%18s w = %3d h = %3d mw = %3d mh = %3d\n","Color",
                   preferred.width,preferred.height,maxwidth,maxheight));

    XtQueryGeometry(x2psp->BackgroundToggleButton,&intended,&preferred);
    if(preferred.width>maxwidth)
        maxwidth = preferred.width;
    maxheight += preferred.height;
    IFDEBUG(printf("%18s w = %3d h = %3d mw = %3d mh = %3d\n","BackgroudToggle",
                   preferred.width,preferred.height,maxwidth,maxheight));

    intended.request_mode = XtCWQueryOnly |CWWidth | CWHeight;
    intended.width = 0;
    intended.height = 0;
    XtQueryGeometry(x2psp->ButtonBox,&intended,&preferred);
    if(preferred.width>maxwidth)
        maxwidth = preferred.width;
    maxheight += preferred.height;
    IFDEBUG(printf("%18s w = %3d h = %3d mw = %3d mh = %3d\n","ButtonBox",
                   preferred.width,preferred.height,maxwidth,maxheight));
    IFDEBUG(printf("%18s w = %3d h = %3d mw = %3d mh = %3d\n","ButtonBox",
                   x2psp->ButtonBox->core.width,
                   x2psp->ButtonBox->core.height,maxwidth,maxheight));

    x2pswg->width = maxwidth;
    x2pswg->height = maxheight + 2;
    IFDEBUG(printf("estimated w %3d h %3d\n",x2pswg->width,x2pswg->height));
}


/*ARGSUSED*/
static void Resize(w)
    Widget w;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget) w;
    XpX2PostScriptPart *x2psp = &(x2psw->x2PostScript);
    XtWidgetGeometry x2pswg;

    IFDEBUG(printf("X2PostScript:Resize() "));
    IFDEBUG(printf("Widget width %d height %d\n", x2psw->core.width,x2psw->core.height));
    estimate_dimensions_x2postscriptwidget(x2psw, &x2pswg);

    /*
     * If estimated size is bigger then requested size then clip sizes
     * to the max values. This makes the parent 'clip' the widget and the
     * placement of the children of the widget won't mess up.
     */
    if (x2pswg.width < x2psw->core.width) x2pswg.width = x2psw->core.width;
    if (x2pswg.height < x2psw->core.height) x2pswg.height = x2psw->core.height;
    /*
     * Resize the form
     */
    IFDEBUG(printf("XtResizeWidget w = %d, h = %d ", x2pswg.width,x2pswg.height));
    XtResizeWidget(x2psp->PrintWidget,x2pswg.width,x2pswg.height,
                   x2psw->core.border_width);
#ifdef BORDERFRAME
    XtResizeWidget(x2psp->PrintWidgetFrame,x2pswg.width,x2pswg.height,
                   x2psw->core.border_width);
#endif /* BORDERFRAME */
    IFDEBUG(printf("done\n"));
    x2psp->oldw = x2pswg.width;
    x2psp->oldh = x2pswg.height;
}

static void ChangeManaged(w)
    Widget w;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget)w;
    XpX2PostScriptPart *x2psp = &(x2psw->x2PostScript);
    XtWidgetGeometry x2pswg;

    IFDEBUG(printf("X2PostScript:ChangeManaged()\n"));
    x2psp->oldw = x2psw->core.width;
    x2psp->oldh = x2psw->core.height;

    estimate_dimensions_x2postscriptwidget(x2psw, &x2pswg);

   /* if the size changed we must change the size of the form as well */
/*
   if(x2psp->oldw != x2psw->core.width || x2psp->oldh != x2psw->core.height) {
*/

    IFDEBUG(printf("core w %d h %d\n",x2psw->core.width,x2psw->core.height));
    if(x2pswg.width != x2psw->core.width ||
       x2pswg.height != x2psw->core.height) {
        (*((XpX2PostScriptWidgetClass)XtClass(w))->core_class.resize) (w);
    }
    else {
        /* adjust draw widget dimensions */
/*      estimate_dimensions_x2postscriptwidget(x2psw, &x2pswg);*/
    }
}

static XtGeometryResult QueryGeometry(w, intended, preferred)
    Widget w;
    XtWidgetGeometry *intended;
    XtWidgetGeometry *preferred;
{
    XtWidgetGeometry x2pswg;

    /* what size do we want to be */
    IFDEBUG(printf("X2PostScript:QueryGeometry()....  "));
    estimate_dimensions_x2postscriptwidget((XpX2PostScriptWidget) w, &x2pswg);

    preferred->request_mode = CWWidth | CWHeight;
    preferred->width = x2pswg.width;
    preferred->height = x2pswg.height;
    if(((intended->request_mode & (CWWidth|CWHeight)) == (CWWidth|CWHeight))
       && intended->width == preferred->width
       && intended->height == preferred->height) {
        IFDEBUG(printf("return XtGeometryYes\n"));
        return(XtGeometryYes);
    }
    else {
	if(preferred->width == w->core.width &&
	   preferred->height == w->core.height) {
	    IFDEBUG(printf("return XtGeometryNo\n"));
	    return(XtGeometryNo);
	}
        else {
	    IFDEBUG(printf("return XtGeometryAlmost\n"));
	    return(XtGeometryAlmost);
	}
    }
}

/*ARGSUSED*/
static XtGeometryResult GeometryManager(w, request, reply)
    Widget w;
    XtWidgetGeometry *request, *reply;
{
    Dimension width,height;
    XtWidgetGeometry x2pswg, x2pswrg;
    XtGeometryResult result;

    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget)w;
    XpX2PostScriptPart* x2psp;
    IFDEBUG(printf("X2PostScript:GeometryManager() "));

    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return(XtGeometryNo); /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);

    IFDEBUG(
        printf("request_mode: (%x) ",request->request_mode);
        if(request->request_mode & CWX)
            printf("CWX | ");
        if(request->request_mode & CWY)
            printf("CWY | ");
        if(request->request_mode & CWWidth)
            printf("CWWidth | ");
        if(request->request_mode & CWHeight)
            printf("CWHeight | ");
        if(request->request_mode & CWBorderWidth)
            printf("CWBorderWidth | ");
        if(request->request_mode & CWSibling)
            printf("CWSibling | ");
        if(request->request_mode & CWStackMode)
            printf("CWStackMode | ");
        if(request->request_mode & XtCWQueryOnly)
            printf("XtCWQueryOnly ");
        printf("\n");
        )
    IFDEBUG(printf("request = w %d h %d\n",request->width,request->height));
/*    estimate_dimensions_x2postscriptwidget((XpX2PostScriptWidget) w, reply);*/

    if(!(request->request_mode & (XtCWQueryOnly | CWWidth | CWHeight))) {
        /*
         * If request_mode =! XtCWQueryOnly || request_mode =! CWWidth ||
         *    request_mode =! CWHeight
         * no geometry managing can be done....
         */
        IFDEBUG(printf("!(request_mode & (XtCWQueryOnly | CWWidth | CWHeight)) => XtGeometryNo\n"));
        return XtGeometryNo;
    }

    if(request->request_mode & CWWidth)
        width = request->width;
    else
        width = x2psw->core.width;
    if(request->request_mode & CWHeight)
        height = request->height;
    else
        height = x2psw->core.height;

    if((width == x2psw->core.width) && (height == x2psw->core.height)) {
        IFDEBUG(printf("(width == x2psw->core.width) && (height == x2psw->core.height) => XtGeometryNo\n"));
        return XtGeometryNo;
    }

    estimate_dimensions_x2postscriptwidget((XpX2PostScriptWidget) w, reply);
    if(reply->width <= width && reply->height <= height) {
        x2pswg.width = width;
        x2pswg.height = height;
        x2pswg.request_mode = CWWidth | CWHeight;
        IFDEBUG(printf("going to XtMakeGeometryRequest(x2psw) w %d h %d\n",
                       x2pswg.width,x2pswg.height));
        result = XtMakeGeometryRequest((Widget) x2psw, &x2pswg, &x2pswrg);
        Resize((Widget)x2psw);
    }
    else {
        if(reply->width == width && reply->height == height) {
            result = XtGeometryYes;
        }
        else {
            reply->request_mode = XtCWQueryOnly | CWWidth | CWHeight;
            if(XtMakeGeometryRequest((Widget) x2psw, reply, &x2pswrg)
               & XtGeometryNo)  {
                result = XtGeometryNo;
            }
            else {
                reply->request_mode = CWWidth | CWHeight;
                result = XtGeometryAlmost;
            }
        }
    }

    if (result == XtGeometryYes) {
        /* Avoid calling XtResizeWidget */
/*        XtResizeWidget(x2psp->PrintWidget,reply->width,reply->height,
                       x2psw->core.border_width);*/
        x2psp->oldw = width;
        x2psp->oldh = height;
    }
    IFDEBUG(
        switch(result) {
            case XtGeometryYes:
            printf("return XtGeometryYes\n");
            break;
            case XtGeometryNo:
            printf("return XtGeometryNo\n");
            break;
            case XtGeometryAlmost:
            printf("return XtGeometryAlmost\n");
            break;
            case XtGeometryDone:
            printf("return XtGeometryDone\n");
            break;
            default:
            printf("return XtGeometryUnknown %d\n",result);
        });
    return result;
}

/*ARGSUSED*/
static Boolean SetValues(current, request, nju, args, num_args)
    Widget current, request, nju;
    ArgList args;
    Cardinal *num_args;
{
    XpX2PostScriptWidget nx2psw = (XpX2PostScriptWidget) nju;
    XpX2PostScriptWidget cx2psw = (XpX2PostScriptWidget) current;
    XpX2PostScriptPart *nx2psp = &(nx2psw->x2PostScript);
    XpX2PostScriptPart *cx2psp = &(cx2psw->x2PostScript);
    Boolean update_output = False;
    Boolean make_outputfilename = False;
    XpX2PostScriptErrorCallbackStruct xpx2pserr;
    Arg arg;

    IFDEBUG(printf("X2PostScript:SetValues()\n"));
    DM_FAKE_USE(request);
    DM_FAKE_USE(args);
    DM_FAKE_USE(num_args);

    if(cx2psp->Xpinfo != nx2psp->Xpinfo) {
        nx2psp->Xpinfo = cx2psp->Xpinfo;
        XtWarning("X2PostScript: Xpinfo cannot be set by XtSetValues.");
    }

    if(cx2psp->FileOrPrinter != nx2psp->FileOrPrinter) {
        nx2psp->FileOrPrinter = cx2psp->FileOrPrinter;
        XtWarning("X2PostScript: FileOrPrinter cannot be set by XtSetValues.");
    }

    if(cx2psp->PortraitLandscape != nx2psp->PortraitLandscape) {
        nx2psp->PortraitLandscape = cx2psp->PortraitLandscape;
        XtWarning("X2PostScript: PortraitLandscape cannot be set by XtSetValues.");
    }

    if(cx2psp->outputfilename != nx2psp->outputfilename) {
        nx2psp->outputfilename = cx2psp->outputfilename;
        XtWarning("X2PostScript: outputFileName cannot be set by XtSetValues.");
    }

    if(cx2psp->DrawBackground != nx2psp->DrawBackground) {
        XtSetArg(arg, XmNset, nx2psp->DrawBackground);
        XtSetValues(nx2psp->BackgroundToggleButton,&arg,1);
    }

    if(cx2psp->printcommandlabelstring != nx2psp->printcommandlabelstring) {
        if(cx2psp->printcommandlabelstring)
            free(cx2psp->printcommandlabelstring);
        nx2psp->printcommandlabelstring =
            XtNewString(nx2psp->printcommandlabelstring);
    }

    if(cx2psp->outputfilelabelstring != nx2psp->outputfilelabelstring) {
        if(cx2psp->outputfilelabelstring)
            free(cx2psp->outputfilelabelstring);
        nx2psp->outputfilelabelstring =
            XtNewString(nx2psp->outputfilelabelstring);
    }

    if(cx2psp->printcommand != nx2psp->printcommand) {
        if(cx2psp->printcommand)
            free(cx2psp->printcommand);
        nx2psp->printcommand = XtNewString(nx2psp->printcommand);
        if(cx2psp->FileOrPrinter == XpPRINTER)
            update_output = True;
    }

    if(cx2psp->fontmapfile != nx2psp->fontmapfile) {
        if(cx2psp->fontmapfile)
            free(cx2psp->fontmapfile);
        nx2psp->fontmapfile = XtNewString(nx2psp->fontmapfile);
    }
    if(cx2psp->fontmapdir != nx2psp->fontmapdir) {
        if(cx2psp->fontmapdir)
            free(cx2psp->fontmapdir);
        nx2psp->fontmapdir = XtNewString(nx2psp->fontmapdir);
    }

    if(cx2psp->X2PostScriptfile != nx2psp->X2PostScriptfile) {
        if(cx2psp->X2PostScriptfile)
            free(cx2psp->X2PostScriptfile);
        nx2psp->X2PostScriptfile = XtNewString(nx2psp->X2PostScriptfile);
        make_outputfilename = True;
    }
    if(cx2psp->X2PostScriptdir != nx2psp->X2PostScriptdir) {
        if(cx2psp->X2PostScriptdir)
            free(cx2psp->X2PostScriptdir);
        nx2psp->X2PostScriptdir = XtNewString(nx2psp->X2PostScriptdir);
        make_outputfilename = True;
    }

    if(cx2psp->Format != nx2psp->Format) {
        UpdateFormatOptionMenu(nx2psw);
    }

    if(make_outputfilename) {
        if(nx2psp->outputfilename)
            free(nx2psp->outputfilename);
        nx2psp->outputfilename = calloc(strlen(nx2psp->X2PostScriptdir)+
                                       strlen(nx2psp->X2PostScriptfile)+2,
                                       sizeof(char));
        if (!nx2psp->outputfilename) {
            if (XpHasErrorCallback(nx2psw)) {
                xpx2pserr.code = XpX2POSTSCRIPT_CALLOCFAIL;
                xpx2pserr.err_type = XpX2POSTSCRIPT_ERROR;
                xpx2pserr.argint = 0;
                xpx2pserr.argstr1 = "SetValues()";
                xpx2pserr.argstr2 = NULL;
                XtCallCallbacks((Widget)nx2psw,
                                XtNerrorCallback,
                                (caddr_t) &xpx2pserr);
                return(False);
            }
            else {
                printf("X2PostScript: Calloc fail in SetValues()\n");
                return(False);
            }
        }
        strcpy(nx2psp->outputfilename,nx2psp->X2PostScriptdir);
        strcat(nx2psp->outputfilename,"/");
        strcat(nx2psp->outputfilename,nx2psp->X2PostScriptfile);
        if(cx2psp->FileOrPrinter == XpFILE)
            update_output = True;
    }

    if(update_output) {
        UpdateOutputFields(nx2psw);
    }

    return(False); /* no redraw needed ?*/
}

static void UpdateOutputFields(x2psw)
    XpX2PostScriptWidget x2psw;
{
    XpX2PostScriptPart *x2psp = &(x2psw->x2PostScript);
    Arg arg;
    XmString SimpleString;

    if(x2psp->FileOrPrinter==XpPRINTER) {
        SimpleString = XmStringCreateSimple(x2psp->printcommandlabelstring);
        XtSetArg(arg, XmNlabelString, SimpleString);
        XtSetValues(x2psp->OutputLabel,&arg,1);
        XmStringFree(SimpleString);
        XtSetArg(arg, XmNvalue, x2psp->printcommand);
        XtSetValues(x2psp->OutputText,&arg,1);
        /*
         * Force FormatOptionMenu to PS, then set insensitive.
         */
        XtSetArg(arg, XmNmenuHistory, x2psp->PSOptionButton);
        XtSetValues(x2psp->FormatOptionMenu,&arg,1);
        XtSetSensitive(x2psp->FormatOptionMenu,False);
    }
    else {
        SimpleString = XmStringCreateSimple(x2psp->outputfilelabelstring);
        XtSetArg(arg, XmNlabelString, SimpleString);
        XtSetValues(x2psp->OutputLabel,&arg,1);
        XmStringFree(SimpleString);
        XtSetArg(arg, XmNvalue, x2psp->outputfilename);
        XtSetValues(x2psp->OutputText,&arg,1);
    }
}

static void FileOrPrinterChange(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    Arg arg;
    Boolean set;

    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget)w;
    XpX2PostScriptPart* x2psp;
    XmRowColumnCallbackStruct* cbstruct =
        (XmRowColumnCallbackStruct*)call_data;

    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return; /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);
    DM_FAKE_USE(call_data);

    XtSetArg(arg, XmNset, &set);
    XtGetValues(cbstruct->widget,&arg,1);
    if(set) {
        if((cbstruct->widget == x2psp->PrinterRadioButton) &&
           (x2psp->FileOrPrinter != XpPRINTER)) {
            IFDEBUG(printf("Toggle to XpPRINTER "));
            x2psp->FileOrPrinter = XpPRINTER;
            if(x2psp->outputfilename)
                XtFree(x2psp->outputfilename);
            x2psp->outputfilename = XmTextGetString(x2psp->OutputText);
            IFDEBUG(printf("saved outputfilename: %s\n",
                           x2psp->outputfilename));
            XtSetArg(arg, XmNmenuHistory, x2psp->PSOptionButton);
            XtSetValues(x2psp->FormatOptionMenu,&arg,1);
            XtSetSensitive(x2psp->FormatOptionMenu,False);
            UpdateOutputFields(x2psw);
        }

        if((cbstruct->widget == x2psp->FileRadioButton) &&
           (x2psp->FileOrPrinter != XpFILE)) {
            x2psp->FileOrPrinter = XpFILE;
            IFDEBUG(printf("Toggle to XpFILE "));
            if(x2psp->printcommand)
                XtFree(x2psp->printcommand);
            x2psp->printcommand = XmTextGetString(x2psp->OutputText);
            IFDEBUG(printf("saved printcommand: %s\n",x2psp->printcommand));
	    UpdateFormatOptionMenu(x2psw);
            XtSetSensitive(x2psp->FormatOptionMenu,True);
            UpdateOutputFields(x2psw);
        }
    }
}

static void UpdateFormatOptionMenu(w)
    XpX2PostScriptWidget w;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget)w;
    XpX2PostScriptPart* x2psp;
    Arg arg;

    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return; /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);

    /*
     * FormatOptionMenu only may change in "XpFILE" mode.
     */
    if(x2psp->FileOrPrinter == XpFILE) {
        switch(x2psp->Format) {
            case XpEPS:
            XtSetArg(arg, XmNmenuHistory, x2psp->EPSOptionButton);
            break;
            case XpPS:
            XtSetArg(arg, XmNmenuHistory, x2psp->PSOptionButton);
            break;
            default:
            XtSetArg(arg, XmNmenuHistory, x2psp->PSOptionButton);
        }
        XtSetValues(x2psp->FormatOptionMenu,&arg,1);
    }
}

static void UpdateOutputFileName(w)
    XpX2PostScriptWidget w;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget)w;
    XpX2PostScriptPart* x2psp;
    char* filename;
    char* filenameptr;
    char* dotptr;

    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return; /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);

    filename = strdup(x2psp->outputfilename);
    filenameptr = strrchr(filename,'/');
    if(!filenameptr)
	filenameptr = filename;
    dotptr = strrchr(filenameptr,'.');
    if(dotptr)
	*dotptr = '\0';
    switch(x2psp->Format) {
	case XpEPS:
	filename = strcat(filename,".eps");
	break;
	case XpPS:
	filename = strcat(filename,".ps");
	break;
	default:
	filename = strcat(filename,".ps");
    }
    free(x2psp->outputfilename);
    x2psp->outputfilename = filename;
}
static void FormatOption(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget)w;
    XpX2PostScriptPart* x2psp;

    XmRowColumnCallbackStruct* cbstruct =
        (XmRowColumnCallbackStruct*)call_data;

    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return; /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);

    IFDEBUG(printf("FormatOption()\n"));

    if(cbstruct->widget==x2psp->PSOptionButton) {
        x2psp->Format = XpPS;
        if(x2psp->FileOrPrinter == XpFILE) {
	    if(x2psp->outputfilename)
		XtFree(x2psp->outputfilename);
	    x2psp->outputfilename = XmTextGetString(x2psp->OutputText);
	    UpdateOutputFileName(x2psw);
        }
    }
    if(cbstruct->widget==x2psp->EPSOptionButton) {
        x2psp->Format = XpEPS;
        if(x2psp->FileOrPrinter == XpFILE) {
	    if(x2psp->outputfilename)
		XtFree(x2psp->outputfilename);
	    x2psp->outputfilename = XmTextGetString(x2psp->OutputText);
	    UpdateOutputFileName(x2psw);
        }
    }
    UpdateOutputFields(x2psw);
}

static void PortraitLandscapeChange(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    Arg arg;
    Boolean set;

    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget)w;
    XpX2PostScriptPart* x2psp;
    XmRowColumnCallbackStruct* cbstruct =
        (XmRowColumnCallbackStruct*)call_data;

    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return; /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);
    DM_FAKE_USE(call_data);

    XtSetArg(arg, XmNset, &set);
    XtGetValues(cbstruct->widget,&arg,1);
    if(set) {
        if((cbstruct->widget == x2psp->PortraitRadioButton) &&
           (x2psp->PortraitLandscape != XpPORTRAIT)) {
            IFDEBUG(printf("Toggle to XpPORTRAIT\n"));
            x2psp->PortraitLandscape = XpPORTRAIT;
        }
        if((cbstruct->widget == x2psp->LandscapeRadioButton) &&
           (x2psp->PortraitLandscape != XpLANDSCAPE)) {
            IFDEBUG(printf("Toggle to XpLANDSCAPE\n"));
            x2psp->PortraitLandscape = XpLANDSCAPE;
        }
    }
}

static void ColorModeChange(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    Arg arg;
    Boolean set;

    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget)w;
    XpX2PostScriptPart* x2psp;
    XmRowColumnCallbackStruct* cbstruct =
        (XmRowColumnCallbackStruct*)call_data;

    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return; /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);
    DM_FAKE_USE(call_data);

    XtSetArg(arg, XmNset, &set);
    XtGetValues(cbstruct->widget,&arg,1);
    if(set) {
        if((cbstruct->widget == x2psp->ColorRadioButton) &&
           (x2psp->ColorMode != XpCOLOR)) {
            IFDEBUG(printf("Toggle to XpCOLOR\n"));
            x2psp->ColorMode = XpCOLOR;
        }
        if((cbstruct->widget == x2psp->BWRadioButton) &&
           (x2psp->ColorMode != XpBW)) {
            IFDEBUG(printf("Toggle to XpBW\n"));
            x2psp->ColorMode = XpBW;
        }
    }
}

static void BackgroundToggle(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget) w;
    XpX2PostScriptPart *x2psp;
    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return; /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);
    DM_FAKE_USE(client_data);
    DM_FAKE_USE(call_data);

    if(x2psp->DrawBackground)
        x2psp->DrawBackground = FALSE;
    else
        x2psp->DrawBackground = TRUE;
}

static void Ok(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget) w;
    XpX2PostScriptPart *x2psp;
 /* XpX2PostScriptWidget X2PostScriptWidget = (XpX2PostScriptWidget)client_data; */
    Arg arg;
    Boolean autounmanage;

    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return; /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);

    XDefineCursor(XtDisplay(x2psw),XtWindow(x2psw),x2psp->busycursor);

    XtCallCallbacks((Widget)x2psw,XtNredrawSetupCallback,call_data);

    Print((Widget)x2psw, client_data, call_data);

    XtCallCallbacks((Widget)x2psw,XtNokCallback,call_data);

    XUndefineCursor(XtDisplay(x2psw),XtWindow(x2psw));

    XtSetArg(arg,XmNautoUnmanage,&autounmanage);
    XtGetValues((Widget)x2psw,&arg,1);
    if (autounmanage && XtIsManaged((Widget)x2psw)) XtUnmanageChild((Widget)x2psw);
}

static void Apply(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget) w;
    XpX2PostScriptPart *x2psp;
 /* XpX2PostScriptWidget X2PostScriptWidget = (XpX2PostScriptWidget)client_data; */

    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return; /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);

    XDefineCursor(XtDisplay(x2psw),XtWindow(x2psw),x2psp->busycursor);

    XtCallCallbacks((Widget)x2psw,XtNredrawSetupCallback,call_data);

    Print((Widget)x2psw, client_data, call_data);

    XtCallCallbacks((Widget)x2psw,XtNapplyCallback,call_data);

    XUndefineCursor(XtDisplay(x2psw),XtWindow(x2psw));
}

static void Cancel(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget) w;
    XpX2PostScriptPart *x2psp;
 /* XpX2PostScriptWidget X2PostScriptWidget = (XpX2PostScriptWidget)client_data; */
    Arg arg;
    Boolean autounmanage;

    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return; /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);

    XtCallCallbacks((Widget)x2psw,XtNcancelCallback,call_data);

    XtSetArg(arg,XmNautoUnmanage,&autounmanage);
    XtGetValues((Widget)x2psw,&arg,1);
    if (autounmanage && XtIsManaged((Widget)x2psw)) XtUnmanageChild((Widget)x2psw);
}

static void Print(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget) w;
    XpX2PostScriptPart* x2psp;

    FILE* PSfile;
    Arg arg[10];
    int n;
    char* fontmapfilename;
    Dimension width,height;
    Colormap colormap;
    Pixel background;
    char systembuffer[100];

    XpStruct* Xpinfo;
    XpPSStruct PSinfo;
    XpX2PostScriptErrorCallbackStruct xpx2err;

    DM_FAKE_USE(client_data);
    DM_FAKE_USE(call_data);

    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return; /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);

    Xpinfo = x2psp->Xpinfo;

    IFDEBUG(printf("X2PostScript:Print()\n"));
    if(x2psp->FileOrPrinter == XpPRINTER)
        x2psp->PSfilename = (char *)tempnam(NULL,"X2ps");
    else {
        x2psp->PSfilename = XmTextGetString(x2psp->OutputText);
        if(x2psp->outputfilename)
            free(x2psp->outputfilename);
        x2psp->outputfilename = strdup(x2psp->PSfilename);
    }

    PSfile = XpOpenOutput(Xpinfo,x2psp->PSfilename);
    if(!PSfile) {
        if (XpHasErrorCallback(x2psw)) {
            xpx2err.code = XpX2POSTSCRIPT_NO_OPEN_FILE;
            xpx2err.err_type = XpX2POSTSCRIPT_ERROR;
            xpx2err.argint = 0;
            xpx2err.argstr1 = "Print()";
            xpx2err.argstr2 = NULL;
            XtCallCallbacks((Widget)x2psw,
                            XtNerrorCallback,
                            (caddr_t) &xpx2err);
        }
        else {
            printf("X2PostScript:Could not open file: %s\n",x2psp->PSfilename);
        }
        if(x2psp->PSfilename)
            free(x2psp->PSfilename);
        x2psp->PSfilename=NULL;
        return;
    }

    if(x2psp->redrawCallback && x2psp->draw_widget)
    {
        if(x2psp->fontmapdir) {
            fontmapfilename = (char*)calloc(strlen(x2psp->fontmapdir) +
					    strlen(x2psp->fontmapfile) + 2,
					    sizeof(char));
            strcpy(fontmapfilename,x2psp->fontmapdir);
            strcat(fontmapfilename,"/");
            strcat(fontmapfilename,x2psp->fontmapfile);
        }
        else {
            fontmapfilename = (char*)calloc(strlen(x2psp->fontmapfile) + 1,
					    sizeof(char));
            strcpy(fontmapfilename,x2psp->fontmapfile);
        }
        XpSetFontmap(Xpinfo, fontmapfilename);
        free(fontmapfilename);

        n = 0;
        XtSetArg(arg[n], XmNbackground, &background); n++;
        XtSetArg(arg[n], XmNcolormap, &colormap); n++;
        XtSetArg(arg[n],XmNwidth,&width); n++;
        XtSetArg(arg[n],XmNheight,&height); n++;
        XtGetValues(x2psp->draw_widget,arg,n);

        XpSetBackground(Xpinfo, background);
        XpSetDrawBackground(Xpinfo, x2psp->DrawBackground);

        XpSetColormap(Xpinfo, colormap);
        XpSetColorMode(Xpinfo, x2psp->ColorMode);

        XpSetDimension(Xpinfo, width, height);

/* print header */
        if((x2psp->FileOrPrinter == XpPRINTER) || (x2psp->Format == XpPS))
            PSinfo.format = XpPS;
        else
            PSinfo.format = XpEPS;
        PSinfo.orientation = x2psp->PortraitLandscape;
        PSinfo.paperwidth = x2psp->paperwidth;
        PSinfo.paperwidthoffset = x2psp->paperwidthoffset;
        PSinfo.paperheight = x2psp->paperheight;
        PSinfo.paperheightoffset = x2psp->paperheightoffset;
        PSinfo.dpi = x2psp->dpi;

	if (Xpinfo->display == NULL)
		Xpinfo->display = XtDisplay(x2psw); /* RVL 291096 */
        XpPrintHeader(Xpinfo,&PSinfo);

        XpSetOutputMode(Xpinfo,XpPS);
        /* to draw the picture */
        XtCallCallbacks((Widget)x2psw,XtNredrawCallback,(caddr_t)NULL);

        XpSetOutputMode(Xpinfo,XpDISPLAY);

	/* When redraw of drawing is needed */
	/* XtCallCallbacks((Widget)x2psw,XtNredrawCallback,(caddr_t)NULL);*/

        XpPrintFooter(Xpinfo,&PSinfo);
    }
    else {
        if(!x2psp->redrawCallback) {
	    if (XpHasErrorCallback(x2psw)) {
		xpx2err.code = XpX2POSTSCRIPT_NO_REDRAW_FUN;
		xpx2err.err_type = XpX2POSTSCRIPT_ERROR;
		xpx2err.argint = 0;
		xpx2err.argstr1 = "Print()";
		xpx2err.argstr2 = NULL;
		XtCallCallbacks((Widget)x2psw,
				XtNerrorCallback,
				(caddr_t) &xpx2err);
	    }
	    else {
		printf("X2PostScript: Have no redraw function.\n");
            }
	}
        if(!x2psp->draw_widget) {
	    if (XpHasErrorCallback(x2psw)) {
		xpx2err.code = XpX2POSTSCRIPT_NO_REDRAW_WIDGET;
		xpx2err.err_type = XpX2POSTSCRIPT_ERROR;
		xpx2err.argint = 0;
		xpx2err.argstr1 = "Print()";
		xpx2err.argstr2 = NULL;
		XtCallCallbacks((Widget)x2psw,
				XtNerrorCallback,
				(caddr_t) &xpx2err);
	    }
	    else {
		printf("X2PostScript: Have no draw_widget.\n");
            }
        }
    }
    XpCloseOutput(Xpinfo);

    if(x2psp->redrawCallback && x2psp->draw_widget)
    {
        if(x2psp->FileOrPrinter == XpPRINTER) {
            if(x2psp->printcommand)
                free(x2psp->printcommand);
            x2psp->printcommand = XmTextGetString(x2psp->OutputText);
            sprintf(systembuffer,"sh -c \"%s %s; rm %s\" &\n",
                    x2psp->printcommand,x2psp->PSfilename,x2psp->PSfilename);
            system(systembuffer);
            free(x2psp->PSfilename);
            x2psp->PSfilename=NULL;
        }
    }
}

Widget XpX2PostScriptWidgetGetChild(widget, child)
    Widget widget;
#ifdef NeedWidePrototypes
    unsigned int child;
#else
    unsigned char child;
#endif
{
    XpX2PostScriptWidget x2psw = (XpX2PostScriptWidget) widget;
    XpX2PostScriptPart* x2psp;
 /* XpX2PostScriptErrorCallbackStruct xpx2pserr; */

    Widget return_widget = NULL;
    char  warning_message[80];

    while (!XtIsSubclass((Widget)x2psw, xpX2PostScriptWidgetClass)) {
        x2psw = (XpX2PostScriptWidget)XtParent((Widget)x2psw);
        if (!x2psw) return return_widget; /* XpX2PostScriptWidget not found */
    }
    x2psp = &(x2psw->x2PostScript);

    switch(child) {
        case XpX2POSTSCRIPT_OK_BUTTON:
        return_widget = XmSelectionBoxGetChild (x2psp->ButtonBox, XmDIALOG_OK_BUTTON);
        break;
        case XpX2POSTSCRIPT_APPLY_BUTTON:
        return_widget = XmSelectionBoxGetChild (x2psp->ButtonBox, XmDIALOG_APPLY_BUTTON);
        break;
        case XpX2POSTSCRIPT_CANCEL_BUTTON:
        return_widget = XmSelectionBoxGetChild (x2psp->ButtonBox, XmDIALOG_CANCEL_BUTTON);
        break;
        case XpX2POSTSCRIPT_HELP_BUTTON:
        return_widget = XmSelectionBoxGetChild (x2psp->ButtonBox, XmDIALOG_HELP_BUTTON);
        break;
        case XpX2POSTSCRIPT_NONE:
        break;
        default:
        sprintf(warning_message,
                "\n    Name: %s\n    Class: %s\n    Invalid child type.\n",
                x2psw->core.name, (char*)xpX2PostScriptClassRec.core_class.class_name);
        XtWarning(warning_message);
    }
    return return_widget;
}

