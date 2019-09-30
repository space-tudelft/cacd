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

#include <stdio.h>
#include "src/libddm/dmincl.h"
#include "src/simeye/define.h"
#include "src/simeye/type.h"
#include "src/simeye/extern.h"

/* #define DEBUG */

extern Display *display;

extern Widget PutButton;
extern Widget RenameButton;
extern Widget StatusLabel;
extern Widget canvas;
extern Widget clickedCommandw;

extern Grid axisStart;
extern Grid axisStop;
extern Grid axisY;
extern Grid canvasTop;
extern Grid drawP_y;

extern int showYValues;
extern int showYValuesPos;
extern int showVoltScale;

extern GC gc3;
static GC gc = NULL;
static Window window;

static Grid x, y, x1, x2, y1, y2;

static int commandType;
static int pressed;
static int crosslet;
static int vbar;
static int hbar;
static int box;
static int secondvbar;
static int askfactor;
static int firstClickShift;
static int State;
static simtime_t t1, t2;

static struct signal * sig1;
static struct signal * sig2;
static struct signal * sigNoEdit;

void DrawCrosslet (Grid x, Grid y);
void DrawHorBar (Grid x1, Grid y1, Grid x2);
void DrawRectangle (Grid x1, Grid y1, Grid x2, Grid y2);
void DrawSecondVertBar (Grid x, Grid y);
void DrawVertBar (Grid x, Grid y);
void buttonPressEvent (Widget w, XEvent *event, String *params, Cardinal num_params);
void motionEvent      (Widget w, XEvent *event, String *params, Cardinal num_params);

void eventsStartup ()
{
    window = XtWindow (canvas);

    gc = gc3;

    XtAddEventHandler (canvas, ButtonPressMask, False, (XtEventHandler)buttonPressEvent, NULL);
}

void beginCommand (Widget w, int cT)
{
    Arg arg;

#ifdef DEBUG
fprintf(stderr,"begCmd: cT = %d\n", cT);
#endif

    if (!Begin_signal) {
	windowMessage ("No signals loaded", -1);
	return;
    }
    windowMessage (NULL, -1);

    switch (commandType = cT) {

        case MEASURE:
            drawPointerInfo (1, 0, 0, 0, NOstate, 0);
	    if (showYValues) drawValueInfo (1, 0);
	    break;

	case PUT:
        case RENAME:
	    askfactor = 0;
        case CHANGE:
        case COPYSIG:
	case DELSIG:
        case MOVESIG:
        case YANK:
            drawPointerInfo (1, 0, 0, 0, NOstate, 0);
    }

    XtAddEventHandler (canvas, PointerMotionMask, False, (XtEventHandler)motionEvent, NULL);

    sig1 = NULL;
    sigNoEdit = NULL;
    pressed = 0;
    crosslet = 0;
    vbar = 0;
    hbar = 0;
    box = 0;
    secondvbar = 0;

    if (w) {
	XmString str;
	clickedCommandw = w;
	XtSetArg (arg, XmNlabelString, &str);
	XtGetValues (w, &arg, 1);
	XtSetArg (arg, XmNlabelString, str);
	XtSetValues (StatusLabel, &arg, 1);
	XmStringFree (str);
	XtManageChild (StatusLabel);
    }
}

void endCommand ()
{
#ifdef DEBUG
fprintf(stderr,"endCmd: cT = %d\n", commandType);
#endif

    XtRemoveEventHandler (canvas, PointerMotionMask, False, (XtEventHandler)motionEvent, NULL);

    switch (commandType) {

        case MEASURE:

            if (secondvbar) DrawSecondVertBar (x1, y1);   /* to clear it */
	    if (vbar) DrawVertBar (x, y);   /* to clear it */
	    if (showYValues) drawValueInfo (-1, 0);

	case YANK:
	case PUT:
        case CHANGE:
        case ZOOMIN:
        case ZOOMOUT:

	    if (pressed) {
		if (hbar) DrawHorBar (x1, y1, x2); /* to clear it */
		if (crosslet) DrawCrosslet (x1, y1); /* to clear it */
	    }
	    else
		if (crosslet) DrawCrosslet (x2, y2); /* to clear it */

	    if (box) DrawRectangle (x1, y1, x2, y2); /* to clear it */

        case COPYSIG:
	case DELSIG:
        case MOVESIG:
        case RENAME:

	    drawPointerInfo (-1, 0, 0, 0, NOstate, 0);

	    if (sig1) unMarkSignal (sig1);
    }

    if (clickedCommandw) {
	clickedCommandw = NULL;
	XtUnmanageChild (StatusLabel);
    }
    commandType = 0;
}

/* This routine is called by the redraw handler
 * to prevent mixing up the drawing of 'crosslet' etc.
 */
void redrawEvent ()
{
#ifdef DEBUG
fprintf(stderr,"redrawEv: cT = %d\n", commandType);
#endif

    switch (commandType) {

        case CHANGE:

	    if (pressed) {
		markSignal (sig1);
		DrawHorBar (x1, y1, x2); /* to redraw it */
		DrawCrosslet (x1, y1);   /* to redraw it */
	    }
	    else if (crosslet) DrawCrosslet (x2, y2); /* to redraw it */
	    break;

        case COPYSIG:
        case MOVESIG:

	    if (pressed) markSignal (sig1);
	    break;

        case MEASURE:

            if (vbar || secondvbar) drawPointerInfo (0, x, y, 0, NOstate, 1);
	    if (showYValues) drawValueInfo (0, x);
	    if (vbar) DrawVertBar (x, y); /* to redraw it */
	    if (secondvbar) DrawSecondVertBar (x1, y1); /* to redraw it */
	    break;

	case PUT:

	    if (box) DrawRectangle (x1, y1, x2, y2);

        case RENAME:

	    if (askfactor) markSignal (sig1);
	    break;

	case YANK:

	    if (pressed) {
		markSignal (sig1);
		DrawRectangle (x1, y1, x2, y2);
	    }
	    break;

        case ZOOMIN:
        case ZOOMOUT:

	    if (pressed) DrawRectangle (x1, y1, x2, y2);
	    break;
    }
}

void motionEvent (Widget w, XEvent *event, String *params, Cardinal num_params)
{
    Window root, child;
    Grid root_x, root_y;
    unsigned int mask;
    XEvent Event;

    while (XCheckTypedEvent (display, MotionNotify, &Event));

#ifdef DEBUG
fprintf(stderr,"motionEv: cT = %d\n", commandType);
#endif

    switch (commandType) {

        case ZOOMIN:
        case ZOOMOUT:

	    if (pressed) {
		XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x, &y, &mask);
		if (x != x2 || y != y2) {
		    DrawRectangle (x1, y1, x2, y2);
		    x2 = x; y2 = y;
		    DrawRectangle (x1, y1, x2, y2);
		}
	     }
	     break;

        case MEASURE:

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x2, &y2, &mask);
	    if (x2 < axisStart) x2 = axisStart;
	    else if (x2 > axisStop) x2 = axisStop;

	    drawPointerInfo (0, x2, y2, 0, NOstate, 1);
	    if (showYValues) drawValueInfo (0, x2);

	    if (showVoltScale) y2 = drawP_y;

            if (!secondvbar || x2 != x1) {
		if (vbar) {
		    if (x2 != x || y2 != y) {
			DrawVertBar (x, y); /* to clear it */
			DrawVertBar (x = x2, y = y2);
		    }
		}
		else {
		    DrawVertBar (x = x2, y = y2);
		    vbar = 1;
		}
	    }
	    else if (vbar) {
		DrawVertBar (x, y); /* to clear it */
		vbar = 0;
	    }
	    break;

	case CHANGE:

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x, &y, &mask);

	    t2 = findTime (x);

	    if (pressed) {
		if (hbar) DrawHorBar (x1, y1, x2); /* to clear it */
		drawPointerInfo (0, x2, y2, 0, State, 0);
	        findGrid (sig1 -> i, t2, State, 0.0, &x2, &y2);
		DrawHorBar (x1, y1, x2);
		hbar = 1;
	    }
	    else {
		if (crosslet) DrawCrosslet (x2, y2); /* to clear it */
		sig2 = findSignal (y, 0);
		if (sig2 != sigNoEdit) {
		    drawPointerInfo (0, x2, y2, 0, State, 0);
		    sigNoEdit = NULL;
		}
		State = findState (y);
	        findGrid (sig2 -> i, t2, State, 0.0, &x2, &y2);
		DrawCrosslet (x2, y2);
		crosslet = 1;
	    }
	    break;

        case COPYSIG:
	case DELSIG:
        case MOVESIG:

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x, &y, &mask);
	    drawPointerInfo (0, x, y, 0, NOstate, 0);
	    break;

        case YANK:

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x, &y, &mask);

	    if (pressed) {
		Grid x_2, y_2;

		t2 = findTime (x);
		findBBox (sig1, t1, t2, &x, &y, &x_2, &y_2);
		if (x != x1 || y != y1 || x_2 != x2 || y_2 != y2) {
		    DrawRectangle (x1, y1, x2, y2);
		    x1 = x; y1 = y; x2 = x_2; y2 = y_2;
		    DrawRectangle (x1, y1, x2, y2);
		}
	    }
	    else {
		x2 = x; y2 = y;
	    }
	    drawPointerInfo (0, x2, y2, 0, NOstate, 0);
	    break;

	case PUT:
        case RENAME:

	    if (askfactor) return;

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x, &y, &mask);

	    sig2 = findSignal (y, 0);
	    if (sig2 != sigNoEdit) {
		drawPointerInfo (0, x, y, 0, NOstate, 0);
		sigNoEdit = NULL;
	    }

	    if (commandType == PUT) {
		if (!sig2 -> no_edit) {
		    t2 = findTime (x);
		    if (!box || sig2 != sig1 || t2 != t1) {
			if (box) DrawRectangle (x1, y1, x2, y2); /* clear */
			findPutBBox (sig2, t2, &x1, &y1, &x2, &y2);
			DrawRectangle (x1, y1, x2, y2); box = 1;
			sig1 = sig2; t1 = t2;
		    }
		}
		else if (box) {
		    DrawRectangle (x1, y1, x2, y2); /* clear */
		    box = 0;
		}
	    }
	    break;
    }
}

void buttonPressEvent (Widget w, XEvent *event, String *params, Cardinal num_params)
{
    Window root, child;
    Grid root_x, root_y;
    unsigned int mask;

#ifdef DEBUG
    fprintf(stderr,"buttonEv: cT = %d\n", commandType);
#endif

    switch (commandType) {

        case ZOOMIN:
        case ZOOMOUT:

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x2, &y2, &mask);

	    if (!pressed) {                /* first click of button */

		firstClickShift = mask & ShiftMask;
	        x1 = x2;
	        y1 = y2;
	        DrawRectangle (x1, y1, x2, y2);
		box = 1;
	        pressed = 1;
	    }
	    else {                         /* second click of button */

                if (commandType == ZOOMIN) {
		    if (firstClickShift)
			draw ('I', x1, y1, x2, y2);
		    else
			draw ('i', x1, y1, x2, y2);
		}
		else {
		    draw ('o', x1, y1, x2, y2);
		}
		box = 0;
		pressed = 0;
	    }
            break;

        case MEASURE:

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x2, &y2, &mask);
	    if (x2 < axisStart) x2 = axisStart;
	    else if (x2 > axisStop) x2 = axisStop;

            if (mask & Button1Mask) {

		if (vbar) {
		    DrawVertBar (x, y); /* to clear it */
		    vbar = 0;
		}

		x = x2; y = y2;
		drawPointerInfo (0, x, y, 1, NOstate, 1);
		if (showVoltScale) y = drawP_y;

		if (secondvbar) { /* 2nd click */
		    DrawSecondVertBar (x1, y1); /* to clear it */
		    secondvbar = 0;
		    DrawVertBar (x, y);
		    vbar = 1;
		}
		else { /* 1st click */
		    x1 = x; y1 = y;
		    DrawSecondVertBar (x1, y1); /* to draw it */
		    secondvbar = 1;
		}
	    }
	    else { /* Button2 or Button3 */

		if (showYValuesPos) { /* toggle showYValue */
		    if (showYValues) {
			showYValues = 0;
			drawValueInfo (-1, 0);
		    }
		    else {
			showYValues = 1;
			drawValueInfo (1, 0);
			drawValueInfo (0, x2);
		    }
		}
	    }

	    break;

        case MOVESIG:

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x, &y, &mask);

	    if (!pressed) {                /* first click of button */

		sig1 = findSignal (y, 0);

	        if (mask & ShiftMask) { /* it's a clear event */
		    if (sig1 -> layover2) {
			sig1 -> layover2 = NULL;
			sig1 -> layover3 = NULL;
			sig1 -> layover4 = NULL;
			sig1 -> layover5 = NULL;
			sig1 -> layover6 = NULL;
			draw (0, 0, 0, 0, 0);
		    }
		    sig1 = NULL;
		}
		else {
		    markSignal (sig1);
		    pressed = 1;
		}
	    }
	    else {                                      /* second click */

                if (mask & ShiftMask) { /* it's a layover event */
		    sig2 = findSignal (y, 0);
		    if (sig2 != sig1) {
			     if (!sig2 -> layover2) sig2 -> layover2 = sig1;
                        else if (!sig2 -> layover3) sig2 -> layover3 = sig1;
                        else if (!sig2 -> layover4) sig2 -> layover4 = sig1;
                        else if (!sig2 -> layover5) sig2 -> layover5 = sig1;
                        else sig2 -> layover6 = sig1;
                    }
		}
		else { /* it's an insert event */
		    sig2 = findSignal (y, 1);
		    if (sig2) {
			if (sig2 -> prev == sig1) sig2 = sig1;
		    }
		    else if (sig1 == End_signal) sig2 = sig1;

		    if (sig2 != sig1) {
			moveSigOnCanvas (sig1, sig2);
			if (editing) somethingChanged = 1;
                    }
		}
		if (sig2 != sig1) draw (0, 0, 0, 0, 0);
		else unMarkSignal (sig1);
		pressed = 0;
		sig1 = NULL;
	    }
            break;

        case CHANGE:

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x, &y, &mask);

	    if (!pressed) {	/* first click of button */

		if (crosslet) {
		    DrawCrosslet (x2, y2); /* to clear it */
		    crosslet = 0;
		}

		sig1 = findSignal (y2 = y, 0);
		if (sig1 -> no_edit) {
		    windowMessage ("This signal can not be edited", -1);
		    sigNoEdit = sig1;
		    sig1 = NULL;
		    break;
		}

		t1 = findTime (x2 = x);
	        if (mask & ShiftMask) /* free state */
		    State = F_state;
		else
		    State = findState (y2);
		findGrid (sig1 -> i, t1, State, 0.0, &x1, &y1);

		drawPointerInfo (0, x2, y2, 1, State, 0);

		markSignal (sig1);

		if (State == L_state)
		    y1--;                       /* for visibility */
		else if (State == H_state)
		    y1++;

		DrawCrosslet (x1, y1);
		DrawHorBar (x1, y1, x2);
		crosslet = hbar = 1;
		pressed = 1;
	    }
	    else {		/* second click of button */
		struct sig_value *help = NULL;

		t2 = findTime (x2);
		if (changeSignal (sig1, t1, t2, State, &help)) {
		    somethingChanged = 1;
		    draw (0, 0, 0, 0, 0);
		}
		else {
		    unMarkSignal (sig1);
		    DrawCrosslet (x1, y1);   /* to clear it */
		    DrawHorBar (x1, y1, x2); /* to clear it */
		}
		drawPointerInfo (0, x, y, 1, State, 0);
		crosslet = hbar = 0;
		sig1 = NULL;
		pressed = 0;
	    }
	    break;

        case YANK:

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x2, &y2, &mask);

	    t2 = findTime (x2);

	    if (!pressed) {                /* first click of button */

		t1 = t2;
		markSignal (sig1 = findSignal (y2, 0));
		findBBox (sig1, t1, t2, &x1, &y1, &x2, &y2);
		drawPointerInfo (0, x1, y1, 1, NOstate, 0);
		DrawRectangle (x1, y1, x2, y2);
		box = 1;
		pressed = 1;
	    }
	    else {                         /* second click of button */

		drawPointerInfo (0, x2, y2, 1, NOstate, 0);
		storeSignalPart (sig1, t1, t2);
		draw (0, 0, 0, 0, 0);
		box = 0;
		sig1 = NULL;
		pressed = 0;
	    }
	    break;

	case PUT:
        case RENAME:

	    if (askfactor) return;

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x, &y, &mask);

	    sig2 = findSignal (y, 0);
	    if (sig2 -> no_edit) {
		windowMessage ("This signal can not be edited!", -1);
		sigNoEdit = sig2;
		break;
	    }

	    if (commandType == PUT) {
		t2 = findTime (x);
		if (!box || sig2 != sig1 || t2 != t1) {
		    if (box) DrawRectangle (x1, y1, x2, y2); /* clear */
		    findPutBBox (sig2, t2, &x1, &y1, &x2, &y2);
		    DrawRectangle (x1, y1, x2, y2); box = 1;
		    t1 = t2;
		}
	    }
	    drawPointerInfo (0, x, y, 0, NOstate, 0);
	    markSignal (sig1 = sig2);
	    askfactor = 1;

	    if (commandType == PUT)
		pDialogCB (PutButton, NULL, NULL);
	    else
		pDialogCB (RenameButton, sig1 -> name, NULL);
	    break;

	case DELSIG:

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x, &y, &mask);
	    sig2 = findSignal (y, 0);
	    delSigFromCanvas (sig2);
	    delSig (sig2);
	    if (editing) somethingChanged = 1;
	    draw (0, 0, 0, 0, 0);
	    if (Begin_signal)
		drawPointerInfo (0, x, y, 0, NOstate, 0);
	    else
		endCommand ();
	    break;

        case COPYSIG:

	    XQueryPointer (display, window, &root, &child, &root_x, &root_y, &x, &y, &mask);
	    if (!pressed) {
		sig1 = findSignal (y, 0);
		markSignal (sig1);
		pressed = 1;
	    }
	    else {
		sig2 = findSignal (y, 0);
		if (sig2 -> no_edit) {
		    windowMessage ("This signal can not be edited", -1);
		    break;
		}
		if (sig2 != sig1) {
		    copySignal (sig1, sig2);
		    somethingChanged = 1;
		    draw (0, 0, 0, 0, 0);
		}
		else unMarkSignal (sig1);
		pressed = 0;
		sig1 = NULL;
	    }
	    break;
    }
}

void PutCancel ()
{
    if (box) DrawRectangle (x1, y1, x2, y2); /* clear */
    box = 0;
    unMarkSignal (sig1);
    sig1 = NULL;
    askfactor = 0;
}

void PutOk (int factor)
{
    if (addStoredSignalPart (sig1, t1, factor)) {
	somethingChanged = 1;
	draw (0, 0, 0, 0, 0);
    }
    else {
	unMarkSignal (sig1);
	DrawRectangle (x1, y1, x2, y2); /* clear */
    }
    box = 0;
    sig1 = NULL;
    askfactor = 0;
}

void RenameOk (char *name)
{
    DELETE (sig1 -> name);
    NEW (sig1 -> name, strlen (name) + 1, char);
    strcpy (sig1 -> name, name);
    sig1 = NULL;
    askfactor = 0;
}

extern int Put_nr;
extern XPoint Put_points[];

void DrawRectangle (Grid x1, Grid y1, Grid x2, Grid y2)
{
    unsigned int w, h;

    if (x1 > x2) { w = x1 - x2; x1 = x2; }
    else w = x2 - x1;
    if (y1 > y2) { h = y1 - y2; y1 = y2; }
    else h = y2 - y1;

    XDrawRectangle (display, window, gc, x1, y1, w, h);

    if (commandType == PUT)
	XDrawLines (display, window, gc, Put_points, Put_nr, CoordModePrevious);
}

void DrawHorBar (Grid x1, Grid y1, Grid x2)
{
    XDrawLine (display, window, gc, x1, y1, x2, y1);
}

void DrawVertBar (Grid x, Grid y)
{
    XDrawLine (display, window, gc, x, canvasTop - 4, x, axisY + 6);
#ifndef old
    if (showVoltScale) DrawCrosslet (x, y);
#endif
}

void DrawCrosslet (Grid x, Grid y)
{
    XDrawLine (display, window, gc, x - 3, y - 3, x + 3, y + 3);
    XDrawLine (display, window, gc, x - 3, y + 3, x + 3, y - 3);
}

void DrawSecondVertBar (Grid x, Grid y)
{
    XDrawLine (display, window, gc, x, canvasTop - 4, x, axisY + 6);
#ifdef old
    if (showVoltScale) XDrawLine (display, window, gc, x - 8, y, x + 8, y);
#else
    if (showVoltScale) DrawCrosslet (x, y);
#endif
}
