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
#include <unistd.h>
#include <stdlib.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"

#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include "src/space/include/type.h"
#include "src/space/X11/pane.h"
#include "src/space/X11/extern.h"

#define STACKSIZE 100

static int running_state = 0;

static FILE * program = NULL;
static int sp = 0;			/* stack pointer */
static long stack[STACKSIZE+1];

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
void cbRobot	  (Widget widget, caddr_t clientData, caddr_t callData);
void cbRobotReset (Widget widget, caddr_t clientData, caddr_t callData);
void dispatchEvent (XEvent *event);
Private char *findName (Widget widget, Window window);
Private Window findWindow (Widget widget, char *name);
Private void makeEvent (void);
Private void jump (char *type, char *name);
Private char *word (char *buf);
Private char *line (char *buf);
Private void call (char *callee);
Private void doreturn (void);
Private void domessage (char *text);
Private void dosystem (char *cmd);
Private void dowait (char *seconds);
#ifdef __cplusplus
  }
#endif

extern Display *display;
extern Widget topWidget;

Window Root;

Private int pointerChanged ()
{
    static int x_root, y_root;
    Window child;
    int dx, dy, x_new, y_new;
    unsigned int mask = 0;

    XQueryPointer (display, XtWindow (topWidget), &Root, &child, &x_new, &y_new, &dx, &dy, &mask);
    dx = x_new - x_root; x_root = x_new;
    dy = y_new - y_root; y_root = y_new;
    return (dx < -5 || dx > 5 || dy < -5 || dy > 5);
}

void robotInit ()
{
    char *programFile = paramLookupS ("disp.program_file", "program");

    if (program) (void) fclose (program);
    program = fopen (programFile, "r");
#ifdef DEBUG
    if (!program) say ("No program file");
#endif
    (void) pointerChanged (); /* to set the Root window */
    sp = 0; /* reset stack pointer */
    running_state = 0;
}

void robotStop ()
{
    running_state = 0;
}

int robotStart ()
{
    if (!running_state && program) running_state = 1;
    return running_state;
}

void robot (XEvent *event)
{
    static int shake = 1;

    if (!running_state) return;

    if (!event) {		/* boot the robot */
	XSync (display, 0);
	(void) pointerChanged ();
	makeEvent ();
	shake = 0;
	return;
    }

    /* An handshake event for synchronization */
    if (event -> type == ClientMessage) shake = 1;

    if (shake == 1 && QLength (display) == 0) {
	XSync (display, 0);
	if (QLength (display) > 0) return;
	makeEvent ();
	shake = 0;
    }
}

/* Private procedures
 */

Private int generate (char *eventName, char *windowName)
{
    XEvent Event;
    XWindowAttributes windowAttributes;
    Window window;
    int x, y, click;

    Debug (say ("playback %s %s", eventName, windowName));

    if (!(window = findWindow (topWidget, windowName))) {
	say ("playback: can't find window '%s'", windowName);
	return 0;
    }

    XGetWindowAttributes (display, window, &windowAttributes);
    x = windowAttributes.width  / 2;
    y = windowAttributes.height / 2;

    click = strsame (eventName, "click");

    if (click || strsame (eventName, "move") || strsame (eventName, "corner")) {
	if (!click && *eventName == 'c') x = y = 0;
	XWarpPointer (display, None, window, 0, 0, 0, 0, x, y);
	(void) pointerChanged ();
	if (!click) goto shake;
    }

    if (click || strsame (eventName, "press")) {
	XButtonPressedEvent *e = (XButtonPressedEvent *) &Event;
	e -> type = ButtonPress;
	e -> serial = 0;
	e -> send_event = True;
	e -> display = display;
	e -> window = window;
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
	Debug (printEvent (&Event));
	XSendEvent (display, PointerWindow, False, 0, &Event);
	if (!click) goto shake;
    }

    if (click || strsame (eventName, "release")) {
	XButtonReleasedEvent *e = (XButtonReleasedEvent *) &Event;
	e -> type = ButtonRelease;
	e -> serial = 0;
	e -> send_event = True;
	e -> display = display;
	e -> window = window;
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
	Debug (printEvent (&Event));
	XSendEvent (display, PointerWindow, False, 0, &Event);
    }

shake:
    {
	XClientMessageEvent *e = (XClientMessageEvent *) &Event;
	e -> type = ClientMessage;
	e -> serial = 300;
	e -> send_event = True;
	e -> display = display;
	e -> window = window;
	e -> format = 8;
	Debug (printEvent (&Event));
	XSendEvent (display, PointerWindow, False, 0, &Event);
    }
    return 1;
}

#undef  XtIsComposite
#define XtIsComposite(top) XtIsSubclass (top, compositeWidgetClass)

Private char * findName (Widget widget, Window window)
{
    int i;
    char * name = NULL;

    if (widget -> core.window == window) {
	return ((char *) widget -> core.name);
    }

 /* Recurse down normal children */
    if (XtIsComposite(widget)) {
	CompositeWidget cw = (CompositeWidget) widget;
	for (i = 0; i < cw -> composite.num_children; i++) {
	    name = findName (cw -> composite.children[i], window);
	    if (name) return (name);
	}
    }

 /* Recurse down popup children */
    /* if (XtIsWindowObject(widget)) */ {
	for (i = 0; i < widget -> core.num_popups; i++) {
	    name = findName (widget -> core.popup_list[i], window);
	    if (name) return (name);
	}
    }
    return (NULL);
}

Private Window findWindow (Widget widget, char *name)
{
    int i;
    Window window;

    if (!XtIsWidget(widget)) return ((Window) 0);

    if (strsame (widget -> core.name, name)) {
	return (widget -> core.window);
    }

 /* Recurse down normal children */
    if (XtIsComposite(widget)) {
	CompositeWidget cw = (CompositeWidget) widget;
	for (i = 0; i < cw -> composite.num_children; i++) {
	    window = findWindow (cw -> composite.children[i], name);
	    if (window) return (window);
	}
    }

 /* Recurse down popup children */
    /* if (XtIsWindowObject(widget)) */ {
	for (i = 0; i < widget -> core.num_popups; i++) {
	    window = findWindow (widget -> core.popup_list[i], name);
	    if (window) return (window);
	}
    }

    return ((Window) 0);
}

Private void makeEvent ()
{
    static int delay = 1;
    static char buf[1024];
    XEvent event;
    char *command;
    int d;

    Debug (say ("make event"));

    if (pointerChanged ()) {
	Debug (fprintf (stderr, "pointer changed\n"));
	cbRobot ((Widget)0, (caddr_t)0, (caddr_t)0); // robotStop
	return;
    }

    while (fgets (buf, sizeof (buf), program)) {

	command = word (buf);

	Debug (fprintf (stderr, "command='%s'\n", command));

	if (command[0] == '\0');	/* empty line */
	else if (command[0] == '#');	/* comment */
	else if (strsame (command, "click")
	     ||  strsame (command, "corner")
	     ||  strsame (command, "move")
	     ||  strsame (command, "press")
	     ||  strsame (command, "release")) {
	    /* Use 'line', because window names can contain spaces.
	     */
	    if (generate (command, line (NULL))) return;
	}
	else if (strsame (command, "goto"  )) jump ("label", word (NULL));
	else if (strsame (command, "call"  )) call (word (NULL));
	else if (strsame (command, "return")) doreturn ();
	else if (strsame (command, "text"  )) domessage (line (NULL));
	else if (strsame (command, "system")) dosystem (line (NULL));
	else if (strsame (command, "wait"  )) dowait (word (NULL));
	else if (strsame (command, "delay" )) delay = atoi (word (NULL));
	else if (strsame (command, "label" )) ;
	else if (strsame (command, "proc"  )) ;
	else call (command); /* assume subroutine call */

	for (d = delay; d > 0; d--)
	    if (XtPending ()) {
		XtNextEvent (&event);
		dispatchEvent (&event);
		if (!running_state) return;
	    }

	if (pointerChanged ()) {
	    Debug (fprintf (stderr, "pointer changed\n"));
	    cbRobot ((Widget)0, (caddr_t)0, (caddr_t)0); // robotStop
	    return;
	}
    }

    cbRobotReset ((Widget)0, (caddr_t)0, (caddr_t)0); // robotInit
}

/* Jump to procedure or label
 */
Private void jump (char *type, char *name)
{
    static char buf[1024];

    rewind (program);

    for (;;) {
	if (!fgets (buf, sizeof (buf), program)) {
	    say ("%s '%s' not found", type, name);
	    if (*type == 'p') doreturn ();
	    return;
	}
	if (strsame (word (buf),  type)
	&&  strsame (word (NULL), name))
	    return;
    }
}

/* Perform subroutine call,
 * by saving return addres on stack
 * and jumping to "proc <callee>" label.
 */
Private void call (char *callee)
{
    if (sp >= STACKSIZE) { say ("Stack overflow"); return; }
    stack[sp++] = ftell (program);
    jump ("proc", callee);
}

/* Return from subroutine
 */
Private void doreturn ()
{
    if (sp <= 0) { say ("Stack underflow"); return; }
    fseek (program, stack[--sp], 0);
}

Private void domessage (char *text)
{
    xMessage (text);
    XSync (display, 0);
    Debug (fprintf (stderr, "%s\n", text));
}

Private void dosystem (char *cmd)
{
    Debug (fprintf (stderr, "system: %s\n", cmd));
    (void) system (cmd);
}

Private void dowait (char *seconds)
{
    XFlush (display);
    sleep (atoi (seconds));
}

/* Return next word from buf.
 * Words are separated by " \t\n".
 * Leading white space is suppresed.
 */
Private char * word (char *buf)
{
    char * w = strtok (buf, " \t\n");

    while (w && isspace ((int)*w)) w++;

    return w ? w : "";
}

/* Return rest of line,
 * but substitute '\\''n' sequences (two chars) by a single '\n'.
 */
Private char * line (char *buf)
{
    char * l = strtok (buf, "\n");

    while (l && (*l == ' ' || *l == '\t')) l++;

    if (l) {
	char * p;
	for (p = l; *p != '\0'; p++) {
	    if (*p == '\\' && *(p+1) == 'n') {
		char *p1 = p+1;
		char *p2 = p+2;
		*p = '\n';
		while (*p2) *(p1++) = *(p2++);
		*p1 = '\0';
		Debug (fprintf (stderr, "'%s'\n", l));
	    }
	}
    }

    return l ? l : "";
}

void printEvent (XEvent *event)
{
    switch (event -> type) {
	case KeyPress: say ("keypress"); break;
	case ButtonPress: {
	    XButtonPressedEvent *e = (XButtonPressedEvent *) event;
	    say ("press %s", findName (topWidget, e -> window));
	}
	break;
	case ButtonRelease: {
	    XButtonReleasedEvent *e = (XButtonReleasedEvent *) event;
	    say ("release %s", findName (topWidget, e -> window));
	}
	break;
	case ClientMessage: {
	    XClientMessageEvent *e = (XClientMessageEvent *) event;
	    say ("shake %s", findName (topWidget, e -> window));
	}
	break;
	case EnterNotify: {
	    XEnterWindowEvent *e = (XEnterWindowEvent *) event;
	    say ("enter %s", findName (topWidget, e -> window));
	}
	break;
	case LeaveNotify: {
	    XLeaveWindowEvent *e = (XLeaveWindowEvent *) event;
	    say ("leave %s", findName (topWidget, e -> window));
	}
	break;
	default: {
	    say ("event type: %d", event -> type);
	    break;
	}
    }
}
