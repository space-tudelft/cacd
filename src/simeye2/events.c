/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	A.J. van Genderen
 *	S. de Graaf
 *	N.P. van der Meijs
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

#include "src/simeye2/define.h"

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

void delSig (struct signal *sig); // read.c
void Simulate (Widget w, caddr_t client_data, caddr_t call_data); // main.c
void wInvert  (Widget w, caddr_t client_data, caddr_t call_data); // main.c

extern int commandType;
extern int prevCommandType;

extern Widget total;
extern Widget canvas;
extern Widget canvasbd;
extern Widget clickedCommandw;
extern Widget commands;
extern Widget editcommands;
extern Widget unitw;
extern Widget newsigw;
extern Widget writew;

extern Grid axisY;
extern Grid canvasHeight;
extern Grid canvasTop;
extern Grid drawP_x;
extern Grid drawP_y;

extern Pixel normalColor;

extern char inputname[];

extern int simWasPressed;
extern Widget simulw;

extern int showYValues;
extern int showYValuesPos;
extern int showVoltScale;
extern GC gc3;

static GC gc = NULL;
static Display *display;
static Window window;

static XColor fgColor;
static XColor bgColor;
static Cursor crossCursor;

#define mess_size  128

static char message[mess_size];
static int mess_head;
static int mess_cnt;

static Grid x1, x2, y1, y2;
static Grid x, y;

static int pressed;
static int crosslet;
static int vbar;
static int hbar;
static int box;
static int secondvbar;
static int asknewname;
static int askfactor;
static int firstClickShift;

static struct signal * sig1;
static struct signal * sig2;
static struct signal * sigNoEdit;

static char filename[128];
static int  state;
static simtime_t t1, t2;

void DrawCrosslet     (Grid x, Grid y);
void DrawRectangle    (Grid x1, Grid y1, Grid x2, Grid y2);
void DrawSecondVertBar(Grid x, Grid y);
void DrawVertBar      (Grid x, Grid y);
void buttonPressEvent (Widget w, XEvent *event, String *params, Cardinal num_params);
void enterEvent       (Widget w, XEvent *event, String *params, Cardinal num_params);
void keyPressEvent    (Widget w, XEvent *event, String *params, Cardinal num_params);
void keyReleaseEvent  (Widget w, XEvent *event, String *params, Cardinal num_params);
void leaveEvent       (Widget w, XEvent *event, String *params, Cardinal num_params);
void motionEvent      (Widget w, XEvent *event, String *params, Cardinal num_params);

void eventsStartup ()
{
    int n;
    Arg args[15];
    Pixel pb;

    static XtActionsRec canvasActions[] = {
	{"enterEvent",		(XtActionProc)enterEvent},
	{"motionEvent",		(XtActionProc)motionEvent},
	{"buttonPressEvent",	(XtActionProc)buttonPressEvent},
	{"leaveEvent",		(XtActionProc)leaveEvent},
    };

    static XtActionsRec totalActions[] = {
	{"keyPressEvent",	(XtActionProc)keyPressEvent},
	{"keyReleaseEvent",	(XtActionProc)keyReleaseEvent},
    };

    static String canvasTranslations =
	"<EnterWindow>: enterEvent()\n\
	 <Motion>:      motionEvent()\n\
	 <BtnDown>:     buttonPressEvent()\n\
	 <LeaveWindow>: leaveEvent()";

    static String totalTranslations =
	"<KeyPress>:    keyPressEvent()\n\
	 <KeyRelease>:  keyReleaseEvent()";

    window = XtWindow (canvas);
    display = XtDisplay (canvas);

    n = 0;
    XtSetArg (args[n], XtNbackground, &pb), n++;
    XtGetValues (canvas, args, n);

    fgColor.pixel = normalColor;
    bgColor.pixel = pb;

    XQueryColor (display, DefaultColormapOfScreen (DefaultScreenOfDisplay (display)), &fgColor);
    XQueryColor (display, DefaultColormapOfScreen (DefaultScreenOfDisplay (display)), &bgColor);

    gc = gc3;

    crossCursor = XCreateFontCursor (display, XC_crosshair);
    XRecolorCursor (display, crossCursor, &fgColor, &bgColor);

    XtAppAddActions (XtWidgetToApplicationContext (canvas), canvasActions, 4);
    XtOverrideTranslations (canvas, XtParseTranslationTable (canvasTranslations));

    XtAppAddActions (XtWidgetToApplicationContext (total), totalActions, 2);
    XtOverrideTranslations (total, XtParseTranslationTable (totalTranslations));
}

void beginCommand (Widget w)
{
    int i;
    simtime_t t;
    char buf[132];

    windowMessage (NULL, -1);

    if (Begin_signal == NULL && !(commandType == UNIT || commandType == NEWSIG || commandType == WRITE)) {
	prevCommandType = commandType;
	commandType = 0;
	return;
    }

    switch (commandType) {

        case MEASURE:
            drawPointerInfo (1, 0, 0, 0, 0, 0, 0);
	    if (showYValues) drawValueInfo (1, 0);
	    break;

        case QUITP:
	    strcpy (message, "no write, are you sure ?: ");
            mess_head = strlen (message);
            mess_cnt = mess_head;
            windowMessage (message, -1);
	    break;

        case UNIT:
            if (Timescaling > 0)
                sprintf (message, "enter grid (current = %.2le): ", Timescaling);
	    else
		sprintf (message, "enter grid: ");
            mess_head = strlen (message);
            mess_cnt = mess_head;
            windowMessage (message, -1);
	    break;

        case SPEED:
	    sprintf (message, "enter signal speed-up factor: ");
            mess_head = strlen (message);
            mess_cnt = mess_head;
            windowMessage (message, -1);
	    break;

        case TEND:
            if (SimEndtime > 0) {
		t = SimEndtime;
		i = 0;
		while (t > 9) { t = t / 10; i++; }
                sprintf (buf, "enter end time (current = %%.%dle): ", i);
                sprintf (message, buf, SimEndtime * Timescaling);
	    }
	    else
		sprintf (message, "enter end time: ");

            mess_head = strlen (message);
            mess_cnt = mess_head;
            windowMessage (message, -1);
	    break;

	case CLEARALL:
	    strcpy (message, "delete all signals, are you sure ?: ");
            mess_head = strlen (message);
            mess_cnt = mess_head;
            windowMessage (message, -1);
	    break;

	case NEWSIG:
	    strcpy (message, "enter name: ");
            mess_head = strlen (message);
            mess_cnt = mess_head;
            windowMessage (message, -1);
	    break;

        case YANK:
            drawPointerInfo (1, 0, 0, 0, 0, 0, 0);
	    break;

	case PUT:
            drawPointerInfo (1, 0, 0, 0, 0, 0, 0);
	    askfactor = 0;
	    break;

	case WRITE:
	    if (somethingChanged) {
		sprintf (filename, "%s.cmd", inputname);
		if (usedNonCapital) *filename += LOWER;

		sprintf (message, "update \"%s\" ?: ", filename);
		mess_head = strlen (message);
		mess_cnt = mess_head;
		windowMessage (message, -1);
		asknewname = 0;
	    }
	    else {
		disableEditing ();

		if (simWasPressed) {
		    Simulate (simulw, NULL, NULL);
		}
		else {
		    prevCommandType = commandType;
		    commandType = 0;
		}
		return;
	    }
	    break;
    }

    sig1 = NULL;
    sigNoEdit = NULL;
    pressed = 0;
    crosslet = 0;
    vbar = 0;
    hbar = 0;
    box = 0;
    secondvbar = 0;

    if (w) {
	clickedCommandw = w;
	wInvert (w, NULL, NULL);
    }
}

void endCommand ()
{
    switch (commandType) {
        case SHUFFLE:
	    if (sig1) unMarkSignal (sig1);
            break;

        case MEASURE:
            if (secondvbar) DrawSecondVertBar (x1, y1); /* to clear it */
	    if (vbar) DrawVertBar (x, y); /* to clear it */
	    if (showYValues) drawValueInfo (-1, 0);

	case YANK:
	case PUT:
        case CHANGE:
	    if (pressed) {
		if (hbar) XDrawLine (display, window, gc, x1, y1, x2, y1);
		if (crosslet) DrawCrosslet (x1, y1);
                if (box) DrawRectangle (x1, y1, x2, y2);
	    }
	    drawPointerInfo (-1, 0, 0, 0, 0, 0, 0);
	    if (sig1) unMarkSignal (sig1);
	    break;
    }

    if (clickedCommandw) {
	wInvert (clickedCommandw, NULL, NULL);
	clickedCommandw = NULL;
    }
    prevCommandType = commandType;
    commandType = 0;
}

/* This routine is called by the redraw handler
   to prevent mixing up the drawing of 'crosslet' etc.
*/
void redrawEvent ()
{
    crosslet = 0;
    vbar = 0;
    box = 0;

    if (secondvbar) DrawSecondVertBar (x1, y1); /* to redraw it */
}

void enterEvent (Widget w, XEvent *event, String *params, Cardinal num_params)
{
    switch (commandType) {
        case MEASURE:
	    /* XDefineCursor (display, window, crossCursor); */
            break;
    }
    sigNoEdit = NULL;
}

void motionEvent (Widget w, XEvent *event, String *params, Cardinal num_params)
{
    Window root, child;
    Grid root_x, root_y;
    unsigned int mask;

    switch (commandType) {

        case ZOOMIN:
        case ZOOMOUT:

	    if (pressed) {
		if (box) DrawRectangle (x1, y1, x2, y2);
		XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x2, &y2, &mask);
		DrawRectangle (x1, y1, x2, y2);
		box = 1;
	     }
	     break;

        case MEASURE:

	    if (vbar) {
		DrawVertBar (x, y);    /* to clear it */
		vbar = 0;
	    }

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x, &y, &mask);

	    drawPointerInfo (0, x, y, 0, 0, 0, 1);
	    if (showYValues) drawValueInfo (0, x);

	    if (showVoltScale) y = drawP_y;

            if (!secondvbar || x != x1) {
		DrawVertBar (x, y);
		vbar = 1;
	    }
	    break;

	case CHANGE:

	    if (pressed) {
		if (hbar)
		    XDrawLine (display, window, gc, x1, y1, x2, y1);
	    }
	    else {
		if (crosslet) DrawCrosslet (x2, y2);
	    }

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x2, &y2, &mask);

	    if (pressed) {
		t2 = findTime (x2, 1);
	        findGrid (sig1 -> i, t2, state, 0.0, &x2, &y2);

		XDrawLine (display, window, gc, x1, y1, x2, y1);
		hbar = 1;
	    }
	    else {
		t2 = findTime (x2, 1);
		sig1 = findSignal (y2, 0);
		state = findState (y2);
                if (event -> xmotion.state & ShiftMask) state = -1;
	        findGrid (sig1 -> i, t2, state, 0.0, &x2, &y2);

		DrawCrosslet (x2, y2);
		crosslet = 1;
	    }

            if (sig1 != sigNoEdit) {
		if (event -> xmotion.state & ShiftMask)
		    drawPointerInfo (0, x2, y2, 0, -1, 0, 0);
		else
		    drawPointerInfo (0, x2, y2, 0, 1, 0, 0);
		sigNoEdit = NULL;
	    }
	    break;

        case YANK:

	    if (pressed) {

                if (box) DrawRectangle (x1, y1, x2, y2);

		XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x2, &y2, &mask);

		t2 = findTime (x2, 1);

		findBBox (sig1, t1, t2, &x1, &y1, &x2, &y2);
		DrawRectangle (x1, y1, x2, y2);
		box = 1;
	    }
	    else {
		XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x2, &y2, &mask);
	    }
	    drawPointerInfo (0, x2, y2, 0, 0, 1, 0);
	    break;

	case PUT:

	    if (askfactor) return;

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x, &y, &mask);

	    sig1 = findSignal (y, 0);
	    if (sig1 != sigNoEdit) {
		drawPointerInfo (0, x, y, 0, 0, 1, 0);
		sigNoEdit = NULL;
	    }
	    break;
    }
}

void buttonPressEvent (Widget w, XEvent *event, String *params, Cardinal num_params)
{
    switch (commandType) {

        case ZOOMIN:
        case ZOOMOUT:

	    if (!pressed) {                /* first click of button */
	        x2 = x1 = event -> xbutton.x;
	        y2 = y1 = event -> xbutton.y;

	        DrawRectangle (x1, y1, x2, y2);
		box = 1;
	        pressed = 1;

		if (event -> xbutton.state & ShiftMask)
		    firstClickShift = 1;
		else
		    firstClickShift = 0;
	    }
	    else {                         /* second click of button */
	        DrawRectangle (x1, y1, x2, y2);
		box = 0;

	        x2 = event -> xbutton.x;
	        y2 = event -> xbutton.y;

                clear ();
		windowMessage (NULL, -1);

                if (commandType == ZOOMIN) {
		    if (firstClickShift && event -> xbutton.state & ShiftMask)
			draw ('I', x1, y1, x2, y2);
		    else
			draw ('i', x1, y1, x2, y2);
		}
		else {
		    draw ('o', x1, y1, x2, y2);
		}
		pressed = 0;
	    }
            break;

        case MEASURE:

            if (event -> xbutton.button == Button1) {
		drawPointerInfo (0, x, y, 1, 0, 0, 1);

		if (secondvbar) {
		    DrawSecondVertBar (x1, y1);
		    secondvbar = 0;
		    if (!vbar) {
			DrawVertBar (x, y);
			vbar = 1;
		    }
		}
		else {
		    DrawSecondVertBar (x, y);
		    x1 = x;
		    y1 = y;
		    secondvbar = 1;
		    if (vbar) {
		        /* prevents line + line = nothing */
			DrawVertBar (x, y);
			vbar = 0;
		    }
		}
	    }
	    else {
		if (showYValuesPos) { /* toggle showYValue */
		    if (showYValues) {
			showYValues = 0;
			drawValueInfo (-1, 0);
		    }
		    else {
			showYValues = 1;
			drawValueInfo (1, 0);
			drawValueInfo (0, x);
		    }
		}
	    }
	    break;

        case SHUFFLE:

	    x = event -> xbutton.x;
	    y = event -> xbutton.y;

	    if (!pressed) {                             /* first click */
		sig1 = findSignal (y, 0);
                markSignal (sig1);

	        if (sig1 && (event -> xbutton.state & ShiftMask)) { /* it's a clear event */
		    sig1 -> layover = NULL;
		    clear ();
		    windowMessage (NULL, -1);
		    draw (0, 0, 0, 0, 0);
		}
		else
		    pressed = 1;
	    }
	    else {                                      /* second click */
                if (event -> xbutton.state & ShiftMask) { /* it's a layover event */
		    sig2 = findSignal (y, 0);
		    sig2 -> layover = sig1;
		}
		else { /* it's an insert event */
		    sig2 = findSignal (y, 1);

		    if (sig1 != sig2) {
		        delSigFromCanvas (sig1, 1);
			insSigOnCanvas (sig1, sig2);
			somethingChanged = 1;
                    }
		}

		clear ();
		windowMessage (NULL, -1);
		draw (0, 0, 0, 0, 0);

		pressed = 0;
		sig1 = NULL;
	    }
            break;

        case CHANGE:

	    if (!pressed) {                /* first click of button */
		x1 = event -> xbutton.x;
		y1 = event -> xbutton.y;

		sig1 = findSignal (y1, 0);
		if (sig1 -> no_edit) {
		    windowMessage ("This signal can not be edited", -1);
		    sigNoEdit = sig1;
		    sig1 = NULL;
		    break;
		}

		x2 = x1;
		y2 = y1;

		t1 = findTime (x1, 1);
                if (event -> xbutton.state & ShiftMask) {
		    state = -1;                   /* free state ! */
		    drawPointerInfo (0, x1, y1, 1, -1, 0, 0);
		}
		else {
		    state = findState (y1);
		    drawPointerInfo (0, x1, y1, 1, 1, 0, 0);
		}

		markSignal (sig1);

		findGrid (sig1 -> i, t1, state, 0.0, &x1, &y1);
		if (state == 0)
		    y1--;                       /* for visibility */
		else if (state == 2)
		    y1++;
		XDrawLine (display, window, gc, x1, y1, x2, y1);
		hbar = 1;
		pressed = 1;
	    }
	    else {                         /* second click of button */
		struct sig_value *help = NULL;

		if (hbar) XDrawLine (display, window, gc, x1, y1, x2, y1);
		hbar = 0;

		x2 = event -> xbutton.x;
		y2 = event -> xbutton.y;

                if (event -> xbutton.state & ShiftMask)
		    drawPointerInfo (0, x2, y2, 1, -1, 0, 0);
		else
		    drawPointerInfo (0, x2, y2, 1, 1, 0, 0);

		t2 = findTime (x2, 1);

		changeSignal (sig1, t1, t2, state, &help);
		clear ();
		windowMessage (NULL, -1);
		draw (0, 0, 0, 0, 0);

		pressed = 0;
		sig1 = NULL;
		crosslet = 0;
	    }
	    break;

        case YANK:
	    if (!pressed) {                /* first click of button */
		x2 = x1 = event -> xbutton.x;
		y2 = y1 = event -> xbutton.y;
		t2 = t1 = findTime (x1, 1);
		sig1 = findSignal (y1, 0);

		markSignal (sig1);
		findBBox (sig1, t1, t2, &x1, &y1, &x2, &y2);
		DrawRectangle (x1, y1, x2, y2);
		box = 1;
		drawPointerInfo (0, x1, y1, 1, 0, 1, 0);
		pressed = 1;
	    }
	    else {                         /* second click of button */
		DrawRectangle (x1, y1, x2, y2);
		box = 1;

		x2 = event -> xbutton.x;
		y2 = event -> xbutton.y;
		t2 = findTime (x2, 1);

                unMarkSignal (sig1);
		drawPointerInfo (0, x2, y2, 1, 0, 1, 0);
		storeSignalPart (sig1, t1, t2);
		clear ();
		windowMessage (NULL, -1);
		draw (0, 0, 0, 0, 0);
		pressed = 0;
		sig1 = NULL;
	    }
	    break;

	case PUT:
	    if (askfactor) {
		unMarkSignal (sig1);
		DrawRectangle (x1, y1, x2, y2);
	    }

	    x = event -> xbutton.x;
	    y = event -> xbutton.y;

	    t1 = findTime (x, 1);
	    sig1 = findSignal (y, 0);
	    if (sig1 -> no_edit) {
		windowMessage ("This signal can not be edited", -1);
		sigNoEdit = sig1;
		sig1 = NULL;
		break;
	    }
	    drawPointerInfo (0, x, y, 0, 0, 1, 0);
	    markSignal (sig1);
	    findBBox (sig1, t1, t1+storedTD, &x1, &y1, &x2, &y2);
	    DrawRectangle (x1, y1, x2, y2);
	    box = 1;

	    strcpy (message, "periods (-1 = infinite): ");
            mess_head = strlen (message);
            mess_cnt = mess_head;
            windowMessage (message, -1);
	    askfactor = 1;
	    break;

	case DELSIG:

	    sig1 = findSignal (event -> xbutton.y, 0);
	    delSigFromCanvas (sig1, 0);
	    delSig (sig1);
	    somethingChanged = 1;
	    clear ();
	    windowMessage (NULL, -1);
	    draw (0, 0, 0, 0, 0);
	    sig1 = NULL;
	    break;

        case COPYSIG:

	    if (!pressed) {
		sig1 = findSignal (event -> xbutton.y, 0);
		markSignal (sig1);
		pressed = 1;
	    }
	    else {
		sig2 = findSignal (event -> xbutton.y, 0);
		if (sig2 -> no_edit) {
		    windowMessage ("This signal can not be edited", -1);
		    break;
		}
                copySignal (sig1, sig2);
		somethingChanged = 1;
                clear ();
		windowMessage (NULL, -1);
	        draw (0, 0, 0, 0, 0);
		pressed = 0;
		sig1 = NULL;
	    }
	    break;
    }
}

void leaveEvent (Widget w, XEvent *event, String *params, Cardinal num_params)
{
    switch (commandType) {

        case MEASURE:
            /*
	    if (vbar) {
		DrawVertBar (x, y);
		vbar = 0;
	    }
	    */
	    if (hbar) {
		XDrawLine (display, window, gc, x1, y1, x2, y1);
		hbar = 0;
	    }
	    /* XUndefineCursor (display, window); */
	    break;

        case CHANGE:
	    if (crosslet && !pressed) {
		DrawCrosslet (x2, y2);
		crosslet = 0;
	    }
	    break;
    }
}

void keyPressEvent (Widget w, XEvent *ev, String *params, Cardinal num_params)
{
    char buf[32];
    int bufnr = 32;
    KeySym keysym;
    XComposeStatus status;
    double newTimescaling;
    double newSimEndtimeS;
    int factor;
    double tfactor;
    XKeyEvent *event = (XKeyEvent *)ev;

    if (commandType == PUT && !askfactor) return;

    switch (commandType) {

        case QUITP:
	case UNIT:
	case SPEED:
	case TEND:
	case NEWSIG:
	case CLEARALL:
	case WRITE:
	case PUT:

	    buf[0] = '\0';
            XLookupString (event, buf, bufnr, &keysym, &status);

	    if ((keysym == XK_Delete || keysym == XK_BackSpace) && mess_cnt > mess_head) { /* delete char */
		message[--mess_cnt] = '\0';
		windowMessage (message, -1);
	    }
            else if ((event -> state & ControlMask) && buf[0] == 'u') { /* erase line */
	        mess_cnt = mess_head;
	        message[mess_cnt] = '\0';
	        windowMessage (message, -1);
            }
	    else if (isalnum ((int)buf[0])
		|| buf[0] == '_' || buf[0] == '.'
		|| buf[0] == '[' || buf[0] == ']'
		|| buf[0] == '-' || buf[0] == '+') {           /* add char */

		if (mess_cnt + 1 < mess_size) {
		    message[mess_cnt++] = buf[0];
		    message[mess_cnt] = '\0';
		    windowMessage (message, -1);
		}
            }
            else if (keysym == XK_Return) {              /* new input ready */

                if (commandType == QUITP) {
                    if (mess_cnt > mess_head && (message[mess_head] == 'y' || message[mess_head] == 'Y')) {
			die (0);
		    }
		    windowMessage (NULL, -1);
		    endCommand ();
		}
                else if (commandType == NEWSIG) {

                    if (existSignal (&message[mess_head])) {
			sprintf (buf, "signal '%s' already exists", &message[mess_head]);
			windowMessage (buf, -1);
		    }
		    else {
			if (mess_cnt > mess_head) {
			    newSigOnCanvas (&message[mess_head]);
			    somethingChanged = 1;
			    clear ();
			    draw (0, 0, 0, 0, 0);
			}
			windowMessage (NULL, -1);
		    }
		    endCommand ();
                }
                else if (commandType == UNIT) {
                    if (mess_cnt > mess_head) {

                        if (sscanf (&message[mess_head], "%le", &newTimescaling) == 1) {

                            if (Timescaling <= 0) {
				Global_umin = 0;
				Global_umax = 2;
                                changeTimescaling (newTimescaling);
			    }
			    else {
				if (Endtime * Timescaling / newTimescaling >= MAXTIME) {

				    windowMessage ("time resolution becomes too high", -1);
				}
				else {
				    changeTimescaling (newTimescaling);
				    clear ();
				    draw (0, 0, 0, 0, 0);
				}
			    }
			    somethingChanged = 1;
			    windowMessage (NULL, -1);
                        }
                        else
		            windowMessage ("error; value unchanged", -1);
                    }
                    else
		        windowMessage (NULL, -1);

		    endCommand ();
                }
                else if (commandType == SPEED) {
                    if (mess_cnt > mess_head) {
                        if (sscanf (&message[mess_head], "%le", &tfactor) == 1 && tfactor > 0) {

			    Timescaling = Timescaling / tfactor;
			    clear ();
			    draw (0, 0, 0, 0, 0);
			    somethingChanged = 1;
			    windowMessage (NULL, -1);
			}
			else
		            windowMessage ("error; value unchanged", -1);
                    }
                    else
		        windowMessage (NULL, -1);

		    endCommand ();
		}
                else if (commandType == TEND) {
                    if (mess_cnt > mess_head) {
                        if (sscanf (&message[mess_head], "%le", &newSimEndtimeS) == 1) {

			    if (newSimEndtimeS / Timescaling > MAXTIME) {
				windowMessage ("too large value specified", -1);
			    }
			    else {
				changeSimEndtime (newSimEndtimeS);
				clear ();
				draw (0, 0, 0, 0, 0);
				somethingChanged = 1;
				windowMessage (NULL, -1);
			    }
			}
			else
		            windowMessage ("error; value unchanged", -1);
                    }
                    else
		        windowMessage (NULL, -1);

		    endCommand ();
		}
                else if (commandType == CLEARALL) {

                    if (mess_cnt > mess_head && (message[mess_head] == 'y' || message[mess_head] == 'Y')) {

			delSigList (Begin_signal);    /* delete all signals */

			Nr_signals = 0;
			Begin_signal = NULL;
			End_signal = NULL;

			somethingChanged = 1;
			clear ();
			draw ('f', 0, 0, 0, 0);
		    }
		    windowMessage (NULL, -1);
		    endCommand ();
		}
                else if (commandType == WRITE) {
		    if (asknewname) {
			if (mess_cnt > mess_head)
			    writeSet (&message[mess_head]);
			else
		            windowMessage (NULL, -1);
		    }
		    else {
		        if (mess_cnt > mess_head && (message[mess_head] == 'y' || message[mess_head] == 'Y'))
			    writeSet (filename);
		        else
		            windowMessage (NULL, -1);
		    }
		    endCommand ();
		    disableEditing ();
                }
		else if (commandType == PUT) {

                    if (mess_cnt > mess_head) {
                        if (sscanf (&message[mess_head], "%d", &factor) != 1)
			    factor = 1;
		    }
		    else
			factor = 1;

		    addStoredSignalPart (sig1, t1, factor);
		    clear ();
                    windowMessage (NULL, -1);
		    draw (0, 0, 0, 0, 0);
		    askfactor = 0;
		    sig1 = NULL;
		}
	    }
	    break;

	case CHANGE:

            if (!pressed) {
		XLookupString (event, buf, bufnr, &keysym, &status);
		if (keysym == XK_Shift_L || keysym == XK_Shift_R || event -> state & ShiftMask)
		    drawPointerInfo (0, x2, y2, 0, -1, 0, 0); /* free state */
	    }
	    break;
    }
}

void keyReleaseEvent (Widget w, XEvent *ev, String *params, Cardinal num_params)
{
    char buf[32];
    int bufnr = 32;
    KeySym keysym;
    XComposeStatus status;
    XKeyEvent *event = (XKeyEvent *)ev;

    if (commandType == CHANGE && !pressed) {
	XLookupString (event, buf, bufnr, &keysym, &status);
	if (event -> state & ShiftMask && (keysym == XK_Shift_L || keysym == XK_Shift_R))
	    drawPointerInfo (0, x2, y2, 0, 1, 0, 0); /* no more free state */
    }
}

void DrawRectangle (Grid x1, Grid y1, Grid x2, Grid y2)
{
    XDrawLine (display, window, gc, x1, y1, x1, y2);
    XDrawLine (display, window, gc, x1, y2, x2, y2);
    XDrawLine (display, window, gc, x2, y2, x2, y1);
    XDrawLine (display, window, gc, x2, y1, x1, y1);
}

static int vertExten = 6;

void DrawVertBar (Grid x, Grid y)
{
    XDrawLine (display, window, gc, x, canvasTop - vertExten, x, axisY + vertExten);

    /* the following is for when a crosshair cursor is used
    {
    int cursorSpace = 12;

    if (y - cursorSpace > canvasTop - vertExten)
	XDrawLine (display, window, gc, x, canvasTop - vertExten, x, Min (y - cursorSpace, axisY + vertExten));
    if (y + cursorSpace < axisY + vertExten)
	XDrawLine (display, window, gc, x, Max (y + cursorSpace, canvasTop - vertExten), x, axisY + vertExten);
    }
    */
}

void DrawCrosslet (Grid x, Grid y)
{
    int leg = 3;

    XDrawLine (display, window, gc, x - leg, y - leg, x + leg, y + leg);
    XDrawLine (display, window, gc, x - leg, y + leg, x + leg, y - leg);
}

void DrawSecondVertBar (Grid x, Grid y)
{
    int cursorSpace = 12;
    int cursorLeg = 8;

    if (showVoltScale) {
	XDrawLine (display, window, gc, x, y - cursorLeg, x, y + cursorLeg);
	XDrawLine (display, window, gc, x - cursorLeg, y, x + cursorLeg, y);

	if (y - cursorSpace > canvasTop - vertExten)
	    XDrawLine (display, window, gc, x, canvasTop - vertExten, x, Min (y - cursorSpace, axisY + vertExten));
	if (y + cursorSpace < axisY + vertExten)
	    XDrawLine (display, window, gc, x, Max (y + cursorSpace, canvasTop - vertExten), x, axisY + vertExten);
    }
    else {
	XDrawLine (display, window, gc, x, canvasTop - vertExten, x, axisY + vertExten);
    }
}
