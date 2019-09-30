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
#include <stdlib.h>
#include <math.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Scrollbar.h>

//#include <X11/Shell.h>

#define XtNputbackCursor   "putbackCursor"
#define XtNmenuUnderCursor "menuUnderCursor"

#ifndef MENUSHELL
extern WidgetClass menuShellWidgetClass;
#endif

/* #define MAX_SELECT 20 */
#define MAX_SELECT 1

typedef struct _Popup_Menu {
	Widget shell;
	Widget box;
	Widget title;
	Widget enable;
	/* int num_select; */
	Widget select[MAX_SELECT];
	char *name;
} PopupMenu;

#include "src/space/X11/menu.h"
#include "src/space/X11/pane.h"
#include "src/space/include/tile.h"
#include "src/space/X11/extern.h"

#ifdef __cplusplus
  extern "C" {
#endif
extern  void delayMode (void);
Private void cbDelay (Widget widget, caddr_t clientData, caddr_t callData);
PopupMenu *MenuCreate (Widget parent, Widget enable, char *name);
void MenuAddSelection (PopupMenu *menu, char *name, XtCallbackProc action, caddr_t arg, int init);
#ifdef __cplusplus
  }
#endif

extern int optPrick;
extern int resMenuAside;
extern int goptCap;
extern int goptCap3D;
extern int goptLatCap;
extern int goptCoupCap;
extern int goptAllRes;
extern int goptIntRes;
extern int goptResMesh;
extern int goptSelectiveElimination;
extern int goptSimpleSubRes;
extern int goptSubRes;
extern int goptRobot;
extern Display *display;
extern Widget draw3dWidget;
extern Widget pauseWidget;
extern Widget robotWidget;
extern Widget runfreeWidget;
extern Widget runstepWidget;
extern Window Root;
extern int delay, olddelay;

static Widget button = NULL;	/* previous button */
static Widget sbbutton = NULL;	/* scrollbar button */
static Widget wCapc, wCapC, wCapL, wCap3, wSE;
static Widget wResr, wResR, wResz, wResb, wResB;
static float percent = 0.95;	/* initial Thumb */

void menuDraw (Widget TopWidget, Widget father, char *label, myMenu_t *menu, int init)
{
    int i;
    PopupMenu * popup;
    char    menuTrans[100];
    Arg args[10];
    Cardinal argcnt;
    XtJustify menuUnderCursor;

    argcnt = 0;
    XtSetArg (args[argcnt], XtNlabel, label), argcnt++;

    if (!resMenuAside && button) {
	XtSetArg (args[argcnt], XtNfromHoriz, button), argcnt++;
    }

    button = XtCreateManagedWidget (label, commandWidgetClass, father, args, argcnt);

    popup = MenuCreate (TopWidget, button, label);

    menuUnderCursor = !resMenuAside ? XtJustifyCenter : XtJustifyLeft;
    argcnt = 0;
    XtSetArg (args[argcnt], XtNmenuUnderCursor, menuUnderCursor), argcnt++;
    XtSetValues (popup -> shell, args, argcnt);

    for (i = 0; menu[i].label; i++) {
	MenuAddSelection (popup, menu[i].label, menu[i].callback, menu[i].client_data, init);

	if (*label == 'O') { /* Option menu */
	    if ((int *)menu[i].client_data == &goptCoupCap)
		wCapC = popup -> select[0];
	    else if ((int *)menu[i].client_data == &goptLatCap)
		wCapL = popup -> select[0];
	    else if ((int *)menu[i].client_data == &goptCap)
		wCapc = popup -> select[0];
	    else if ((int *)menu[i].client_data == &goptCap3D)
		wCap3 = popup -> select[0];
	    else if ((int *)menu[i].client_data == &goptIntRes)
		wResr = popup -> select[0];
	    else if ((int *)menu[i].client_data == &goptAllRes)
		wResR = popup -> select[0];
	    else if ((int *)menu[i].client_data == &goptResMesh)
		wResz = popup -> select[0];
	    else if ((int *)menu[i].client_data == &goptSimpleSubRes)
		wResb = popup -> select[0];
	    else if ((int *)menu[i].client_data == &goptSubRes)
		wResB = popup -> select[0];
	    else if ((int *)menu[i].client_data == &goptSelectiveElimination)
		wSE = popup -> select[0];
	}
	else if (*label == 'E') { /* Extract menu */
	    if (strsame (menu[i].label, "pause"))
		pauseWidget = popup -> select[0];
	    else if (strsame (menu[i].label, "robot"))
		robotWidget = popup -> select[0];
	    else if (strsame (menu[i].label, "run free"))
		runfreeWidget = popup -> select[0];
	    else if (strsame (menu[i].label, "single step"))
		runstepWidget = popup -> select[0];
	}
	else if (*label == 'D' && init != 2) { /* Display menu */
	    if (*menu[i].label == '3')
		draw3dWidget = popup -> select[0];
	}
    }

    if (*label == 'E') { /* Extract menu */
	ASSERT (pauseWidget);
	ASSERT (robotWidget);
	ASSERT (runfreeWidget);
	ASSERT (runstepWidget);
    }

    /*  A translation that pops-up after clicking:
     * 		"<BtnDown>:MenuPopup(%s)"
     */
    (void) sprintf (menuTrans, "<EnterWindow>:MenuPopup(%s) ", label);
    XtOverrideTranslations (button, XtParseTranslationTable (menuTrans));
    XtRealizeWidget (popup->shell); // MenuReady
}

/* ARGSUSED */
void menuCallbackToggle (Widget widget, caddr_t clientData, caddr_t callData)
{
    int *flag = (int *)clientData;

    ASSERT (flag);

    widgetInvert (widget);

    if (*flag) {
	*flag = 0;
	if (widget == wCapc) {
	    if (goptCoupCap) widgetInvert (wCapC), goptCoupCap = 0;
	    if (goptLatCap) widgetInvert (wCapL), goptLatCap = 0;
	    if (goptCap3D) widgetInvert (wCap3), goptCap3D = 0;
	    if (goptSelectiveElimination) widgetInvert (wSE), goptSelectiveElimination = 0;
	}
	else if (widget == wCapC) {
	    if (goptLatCap) widgetInvert (wCapL), goptLatCap = 0;
	}
	else if (widget == wResr) {
	    if (optPrick) {
		widgetInvert (widget); goptIntRes = 1;
		say ("Selective res option active (-j or -k).");
		say ("Cannot put off resistance extraction!");
		return;
	    }
	    if (goptAllRes) widgetInvert (wResR), goptAllRes = 0;
	    if (goptResMesh) widgetInvert (wResz), goptResMesh = 0;
	    if (goptSelectiveElimination) widgetInvert (wSE), goptSelectiveElimination = 0;
	}
    }
    else {
	*flag = 1;
	if (flag == &goptLatCap) {
	    if (!goptCoupCap) widgetInvert (wCapC), goptCoupCap = 1;
	    if (!goptCap) widgetInvert (wCapc), goptCap = 1;
	}
	else if (flag == &goptCoupCap) {
	    if (!goptCap) widgetInvert (wCapc), goptCap = 1;
	}
	else if (flag == &goptCap3D) {
	    if (!goptCap) {
		widgetInvert (wCapc), goptCap = 1;
		widgetInvert (wCapC), goptCoupCap = 1;
	    }
	}
	else if (flag == &goptAllRes || flag == &goptResMesh) {
	    if (!goptIntRes) widgetInvert (wResr), goptIntRes = 1;
	}
	else if (flag == &goptSimpleSubRes) {
	    if (goptSubRes) widgetInvert (wResB), goptSubRes = 0;
	}
	else if (flag == &goptSubRes) {
	    if (goptSimpleSubRes) widgetInvert (wResb), goptSimpleSubRes = 0;
	}
	else if (flag == &goptSelectiveElimination) {
	    if (!goptIntRes) widgetInvert (wResr), goptIntRes = 1;
	    if (!goptCap) widgetInvert (wCapc), goptCap = 1;
	}
    }

    XSync (XtDisplay (widget), 0);
}

static char scrollTranslations[] =
   "<Btn1Down>:   StartScroll(Continuous) MoveThumb() NotifyThumb() \n\
    <Btn2Down>:   StartScroll(Forward) \n\
    <Btn3Down>:   StartScroll(Backward) \n\
    <Btn1Motion>: MoveThumb() NotifyThumb() \n\
    <BtnUp>:      NotifyScroll(Proportional) EndScroll()";

void menuDelayWidget (Widget menuregion)
{
    XtTranslations scrolltranstbl;
    Arg arg[10];
    Cardinal n;
    Dimension w, h;

    /* init scrollbar translation table */
    scrolltranstbl = XtParseTranslationTable (scrollTranslations);

    if (button) {
	n = 0;
	XtSetArg(arg[n], XtNwidth, &w), n++;
	XtSetArg(arg[n], XtNheight, &h), n++;
	XtGetValues (button, arg, n);
    }
    else {
	w = 30, h = 10;
    }

    n = 0;
    XtSetArg (arg[n], XtNwidth, w), n++;
    XtSetArg (arg[n], XtNheight, h), n++;
    XtSetArg (arg[n], XtNorientation, XtorientHorizontal);	n++;
    XtSetArg (arg[n], XtNshown, (float) 0.1);			n++;
    XtSetArg (arg[n], XtNtranslations, scrolltranstbl);		n++;

    if (!resMenuAside && button) {
	XtSetArg (arg[n], XtNfromHoriz, button), n++;
    }

    button = XtCreateManagedWidget ("speed_scrollbar", scrollbarWidgetClass, menuregion, arg, n);
    sbbutton = button;

    XtAddCallback (button, XtNjumpProc, (XtCallbackProc) cbDelay, NULL);
    XawScrollbarSetThumb (button, percent, (float) -1.0);
    cbDelay ((Widget) NULL, (caddr_t) NULL, (caddr_t) &percent);
}

/* ARGSUSED */
Private void cbDelay (Widget widget, caddr_t clientData, caddr_t callData)
{
    percent = *(float*) callData;

    if (percent > 1.0) percent = 1.0;
    delay = olddelay = (int) exp ((1.0 - (double) percent) * 13.0);
#ifdef DEBUG
if(DEBUG) fprintf (stderr, "cbDelay: percent=%.2f delay=%d\n", percent, delay);
#endif
    if (widget) delayMode (); /* if (not first time) set mode */
}

void setDelay (int pos)
{
    XEvent Event;
    XButtonPressedEvent *e;
    Arg arg[4];
    Cardinal n;
    Dimension w, h;
    float d;
    int x, y;

    if (pos) {
	if (percent >= 1.0 || delay == 1) return;
	d = percent + (1.0 - percent) / 2;
    }
    else {
	if (percent <= 0.0) return;
	d = percent - percent / 2;
    }

    if (sbbutton && !goptRobot) {
	w = 30; h = 10; /* init */
	n = 0;
	/* get current scrollbar values */
	XtSetArg (arg[n], XtNwidth,  &w), n++;
	XtSetArg (arg[n], XtNheight, &h), n++;
	XtGetValues (sbbutton, arg, n);
	x = d * w;
	y = h / 2;
#ifdef DEBUG
if(DEBUG) fprintf (stderr, "setDelay: x=%d y=%d (w=%d h=%d)\n", x, y, w, h);
#endif
	/* move pointer to the correct scrollbar position */
	XWarpPointer (XtDisplay(sbbutton), None, XtWindow(sbbutton), 0, 0, 0, 0, x, y);

	/* simulate scrollbar click to move the thumb */
	e = (XButtonPressedEvent *) &Event;
	e -> type = ButtonPress;
	e -> serial = 0;
	e -> send_event = True;
	e -> display = display;
	e -> window  = XtWindow(sbbutton);
	e -> root = Root;
	e -> subwindow = 0;
	e -> time = 0;
	e -> x = x;
	e -> y = y;
	e -> x_root = 0;
	e -> y_root = 0;
	e -> state = 0;
	e -> button = 1;
	e -> same_screen = True;
	XSendEvent (XtDisplay(sbbutton), XtWindow(sbbutton), False, 0, &Event);
    }
    else {
	cbDelay ((Widget) 1, (caddr_t) NULL, (caddr_t) &d);
    }
}

/*
 * binds the button argument with translation BtnDown to the menu name
 */
static void MenuBind (Widget button, char *name)
{
    static char *base = "<BtnDown>: MenuPopup( ";
    static char *tail = " )";
    static XtTranslations menuTranslations;
    char c, trans_table[80];
    int i, j;

    i = 0;
    j = 0; while ((c = base[j++])) trans_table[i++] = c;
    j = 0; while ((c = name[j++])) if (c != ' ') trans_table[i++] = c;
    j = 0; while ((c = tail[j++])) trans_table[i++] = c;
    if (i >= 80) { fprintf (stderr, "MenuBind: too long menu name\n"); die (); }
    trans_table[i] = c;

    menuTranslations = XtParseTranslationTable (trans_table);
    XtOverrideTranslations (button, menuTranslations);
}

/*
 * call this to create the menu shell and box widgets, and the title.
 * MenuBind is called to bind this menu to an enabling widget with translation BtnDown
 */
PopupMenu *MenuCreate (Widget parent, Widget enable, char *name)
{
    Arg args[4];
    Cardinal i;
    PopupMenu *menu;

    if (!(menu = XtNew (PopupMenu))) {
	fprintf (stderr, "MenuCreate: cannot malloc new menu\n");
	die ();
    }
    menu->name = name;
    menu->enable = enable;
 /* menu->num_select = 0; */

    MenuBind (enable, name);
    i = 0;
    XtSetArg (args[i], XtNputbackCursor, FALSE); i++;
    menu->shell = XtCreatePopupShell (menu->name, menuShellWidgetClass, parent, args, i);
    i = 0;
    menu->box   = XtCreateManagedWidget ("MenuBox", menuboxWidgetClass, menu->shell, args, i);
    menu->title = XtCreateManagedWidget (menu->name, labelWidgetClass, menu->box, args, i);
    return menu;
}

/*
 * add a selection to an existing menu
 * it should be possible to delete selections
 */
/*
static char buttonTranslations[] =
     "<EnterWindow>:	set()\n\
     <LeaveWindow>:	unset()";
*/
void MenuAddSelection (PopupMenu *menu, char *name, XtCallbackProc action, caddr_t arg, int init)
{
    Arg args[4];
    Cardinal i;
    Widget w;

    i = 0;
    w = XtCreateManagedWidget (name, commandWidgetClass, menu->box, args, i);
    menu->select[0] = w;
 /* menu->select[menu->num_select] = w; */

    if (action) XtAddCallback (w, XtNcallback, action, arg);

    /* pre-turn "on" menu entries if init == TRUE */
    if (init && arg && *(int *) arg) {
	if (init == 2)
	    cbSelectCell (w);
	else
	    widgetInvert (w);
    }
 /* XtOverrideTranslations (w, XtParseTranslationTable (buttonTranslations)); */
}

void widgetInvert (Widget widget)
{
    Arg args[4];
    Cardinal i;
    Pixel fg, bg;

    if (!widget) return;
    i = 2;
    XtSetArg (args[0], XtNforeground, &fg);
    XtSetArg (args[1], XtNbackground, &bg);
    XtGetValues (widget, args, i);
    XtSetArg (args[0], XtNforeground, bg);
    XtSetArg (args[1], XtNbackground, fg);
    XtSetValues (widget, args, i);
}
