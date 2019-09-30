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

#include <sys/types.h>
#include <X11/IntrinsicP.h>
#define  XAW_BC  1        /* for sun */
#include <X11/StringDefs.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/AsciiText.h>

void beginCommand (Widget w); // events.c
void endCommand   (void); // events.c

extern Widget commands;
extern Widget canvasbd;
extern Widget total;
extern Widget toplevel;
extern Widget zoomoutw;

extern Widget clickedCommandw;
extern int commandType;

struct sig_value *storedVals = NULL;
simtime_t storedTD;

int editing = 0;
int somethingChanged;

Widget editcommands;
Widget unitw;
Widget newsigw;
Widget delsigw;
Widget clearallw;
Widget copysigw;
Widget changew;
Widget yankw;
Widget putbufw;
Widget speedw;
Widget t_endw;
Widget returnw;

void Change   (Widget w, caddr_t client_data, caddr_t call_data);
void Clearall (Widget w, caddr_t client_data, caddr_t call_data);
void Copysig  (Widget w, caddr_t client_data, caddr_t call_data);
void Delsig   (Widget w, caddr_t client_data, caddr_t call_data);
void Newsig   (Widget w, caddr_t client_data, caddr_t call_data);
void Put      (Widget w, caddr_t client_data, caddr_t call_data);
void Return   (Widget w, caddr_t client_data, caddr_t call_data);
void Speed    (Widget w, caddr_t client_data, caddr_t call_data);
void T_end    (Widget w, caddr_t client_data, caddr_t call_data);
void Unit     (Widget w, caddr_t client_data, caddr_t call_data);
void Yank     (Widget w, caddr_t client_data, caddr_t call_data);
char eval2cmdval (int v);
int  sval2eval   (int v);

#define NR_BUTTON  11

void enableEditing ()
{
    int n;
    Grid butWidth;
    Grid butHeight;
    Grid butSep;
    Grid butBD;
    Grid butDist;
    Grid butRest;
    Grid editWidth;
    Grid editHeight;
    Grid editVertDist;
    XtCallbackRec callbacks[2];
    Arg args[15];
    Dimension dw, dh, dbw;
    Position posy, posv;

    if (editing) return;

    editing = 1;
    somethingChanged = 0;

    n = 0;
    XtSetArg (args[n], XtNwidth, &dw), n++;
    XtSetArg (args[n], XtNheight, &dh), n++;
    XtSetArg (args[n], XtNy, &posy), n++;
    XtSetArg (args[n], XtNborderWidth, &dbw), n++;
    XtGetValues (zoomoutw, args, n);

    butHeight = dh;
    butSep = posy;
    butBD = dbw;

    n = 0;
    XtSetArg (args[n], XtNwidth, &dw), n++;
    XtSetArg (args[n], XtNheight, &dh), n++;
    XtSetArg (args[n], XtNvertDistance, &posv), n++;
    XtSetArg (args[n], XtNborderWidth, &dbw), n++;
    XtGetValues (canvasbd, args, n);

    editWidth = dw;
    editHeight = butHeight + 2 * butSep + 2 * butBD;
    editVertDist = posv;

    XtResizeWidget (canvasbd, dw, dh - editHeight - posv - 2 * dbw, dbw);

    callbacks[1].callback = NULL;

    n = 0;
    XtSetArg (args[n], XtNfromVert, canvasbd), n++;
    XtSetArg (args[n], XtNvertDistance, editVertDist), n++;
    XtSetArg (args[n], XtNwidth, editWidth), n++;
    XtSetArg (args[n], XtNheight, editHeight), n++;
    XtSetArg (args[n], XtNdefaultDistance, butSep), n++;
    XtSetArg (args[n], XtNborderWidth, butBD), n++;
    editcommands = XtCreateManagedWidget ("commands", formWidgetClass, total, args, n);

    butWidth = (editWidth - (NR_BUTTON + 1) * butSep - NR_BUTTON * 2 * butBD) / NR_BUTTON;
    butDist = butSep;
    butRest = (editWidth - 2 * butSep - NR_BUTTON * (butWidth + 2 * butBD)) - butDist * (NR_BUTTON - 1);

    callbacks[0].callback = (XtCallbackProc)Unit;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, butWidth), n++;
    XtSetArg (args[n], XtNheight, butHeight), n++;
    XtSetArg (args[n], XtNborderWidth, butBD), n++;
    unitw = XtCreateManagedWidget ("grid", commandWidgetClass, editcommands, args, n);

    if (butRest == 10) butDist++;

    callbacks[0].callback = (XtCallbackProc)Newsig;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, butWidth), n++;
    XtSetArg (args[n], XtNheight, butHeight), n++;
    XtSetArg (args[n], XtNfromHoriz, unitw), n++;
    XtSetArg (args[n], XtNhorizDistance, butDist), n++;
    XtSetArg (args[n], XtNborderWidth, butBD), n++;
    newsigw = XtCreateManagedWidget ("new", commandWidgetClass, editcommands, args, n);

    if (butRest == 9) butDist++;

    callbacks[0].callback = (XtCallbackProc)Delsig;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, butWidth), n++;
    XtSetArg (args[n], XtNheight, butHeight), n++;
    XtSetArg (args[n], XtNfromHoriz, newsigw), n++;
    XtSetArg (args[n], XtNhorizDistance, butDist), n++;
    XtSetArg (args[n], XtNborderWidth, butBD), n++;
    delsigw = XtCreateManagedWidget ("delete", commandWidgetClass, editcommands, args, n);

    if (butRest == 8) butDist++;

    callbacks[0].callback = (XtCallbackProc)Clearall;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, butWidth), n++;
    XtSetArg (args[n], XtNheight, butHeight), n++;
    XtSetArg (args[n], XtNfromHoriz, delsigw), n++;
    XtSetArg (args[n], XtNhorizDistance, butDist), n++;
    XtSetArg (args[n], XtNborderWidth, butBD), n++;
    clearallw = XtCreateManagedWidget ("clear", commandWidgetClass, editcommands, args, n);

    if (butRest == 7) butDist++;

    callbacks[0].callback = (XtCallbackProc)Copysig;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, butWidth), n++;
    XtSetArg (args[n], XtNheight, butHeight), n++;
    XtSetArg (args[n], XtNfromHoriz, clearallw), n++;
    XtSetArg (args[n], XtNhorizDistance, butDist), n++;
    XtSetArg (args[n], XtNborderWidth, butBD), n++;
    copysigw = XtCreateManagedWidget ("copy", commandWidgetClass, editcommands, args, n);

    if (butRest == 6) butDist++;

    callbacks[0].callback = (XtCallbackProc)Change;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, butWidth), n++;
    XtSetArg (args[n], XtNheight, butHeight), n++;
    XtSetArg (args[n], XtNfromHoriz, copysigw), n++;
    XtSetArg (args[n], XtNhorizDistance, butDist), n++;
    XtSetArg (args[n], XtNborderWidth, butBD), n++;
    changew = XtCreateManagedWidget ("edit", commandWidgetClass, editcommands, args, n);

    if (butRest == 5) butDist++;

    callbacks[0].callback = (XtCallbackProc)Yank;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, butWidth), n++;
    XtSetArg (args[n], XtNheight, butHeight), n++;
    XtSetArg (args[n], XtNfromHoriz, changew), n++;
    XtSetArg (args[n], XtNhorizDistance, butDist), n++;
    XtSetArg (args[n], XtNborderWidth, butBD), n++;
    yankw = XtCreateManagedWidget ("yank", commandWidgetClass, editcommands, args, n);

    if (butRest == 4) butDist++;

    callbacks[0].callback = (XtCallbackProc)Put;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, butWidth), n++;
    XtSetArg (args[n], XtNheight, butHeight), n++;
    XtSetArg (args[n], XtNfromHoriz, yankw), n++;
    XtSetArg (args[n], XtNhorizDistance, butDist), n++;
    XtSetArg (args[n], XtNborderWidth, butBD), n++;
    putbufw = XtCreateManagedWidget ("put", commandWidgetClass, editcommands, args, n);

    if (butRest == 3) butDist++;

    callbacks[0].callback = (XtCallbackProc)Speed;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, butWidth), n++;
    XtSetArg (args[n], XtNheight, butHeight), n++;
    XtSetArg (args[n], XtNfromHoriz, putbufw), n++;
    XtSetArg (args[n], XtNhorizDistance, butDist), n++;
    XtSetArg (args[n], XtNborderWidth, butBD), n++;
    speedw = XtCreateManagedWidget ("speed", commandWidgetClass, editcommands, args, n);

    if (butRest == 2) butDist++;

    callbacks[0].callback = (XtCallbackProc)T_end;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, butWidth), n++;
    XtSetArg (args[n], XtNheight, butHeight), n++;
    XtSetArg (args[n], XtNfromHoriz, speedw), n++;
    XtSetArg (args[n], XtNhorizDistance, butDist), n++;
    XtSetArg (args[n], XtNborderWidth, butBD), n++;
    t_endw = XtCreateManagedWidget ("t_end", commandWidgetClass, editcommands, args, n);

    if (butRest == 1) butDist++;

    callbacks[0].callback = (XtCallbackProc)Return;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, butWidth), n++;
    XtSetArg (args[n], XtNheight, butHeight), n++;
    XtSetArg (args[n], XtNfromHoriz, t_endw), n++;
    XtSetArg (args[n], XtNhorizDistance, butDist), n++;
    XtSetArg (args[n], XtNborderWidth, butBD), n++;
    returnw = XtCreateManagedWidget ("ready", commandWidgetClass, editcommands, args, n);

    XtRealizeWidget (editcommands);
}

void Unit (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    commandType = UNIT;
    beginCommand (w);
}

void Newsig (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    if (Timescaling <= 0) {
	windowMessage ("enter grid first", -1);
	return;
    }

    commandType = NEWSIG;
    beginCommand (w);
}

void Delsig (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    commandType = DELSIG;
    beginCommand (w);
}

void Clearall (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    if (Begin_signal) {
	commandType = CLEARALL;
	beginCommand (w);
    }
}

void Copysig (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    commandType = COPYSIG;
    beginCommand (w);
}

void Change (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    commandType = CHANGE;
    beginCommand (w);
}

void Yank (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    commandType = YANK;
    beginCommand (w);
}

void Put (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    if (!storedVals) {
	windowMessage ("empty buffer", -1);
    }
    else {
	commandType = PUT;
	beginCommand (w);
    }
}

void Speed (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    commandType = SPEED;
    beginCommand (w);
}

void T_end (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    commandType = TEND;
    beginCommand (w);
}

void Return (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    commandType = WRITE;
    beginCommand (w);
}

void disableEditing ()
{
    int n;
    Grid sum;
    Arg args[15];
    Dimension dw, dh, dbw;
    Position posv;

    if (storedVals) {
        delSigval (storedVals, (struct sig_value *)0);
	storedVals = NULL;
    }

    n = 0;
    XtSetArg (args[n], XtNheight, &dh), n++;
    XtGetValues (total, args, n);

    sum = dh;

    n = 0;
    XtSetArg (args[n], XtNvertDistance, &posv), n++;
    XtSetArg (args[n], XtNheight, &dh), n++;
    XtSetArg (args[n], XtNborderWidth, &dbw), n++;
    XtGetValues (commands, args, n);

    sum = sum - posv - dh - 2 * dbw;

    XtUnmanageChild (editcommands);
    XtDestroyWidget (editcommands);

    n = 0;
    XtSetArg (args[n], XtNwidth, &dw), n++;
    XtSetArg (args[n], XtNheight, &dh), n++;
    XtSetArg (args[n], XtNvertDistance, &posv), n++;
    XtSetArg (args[n], XtNborderWidth, &dbw), n++;
    XtGetValues (canvasbd, args, n);

    XtResizeWidget (canvasbd, dw, sum - 2 * (posv + dbw), dbw);

    editing = 0;
}

int changeSignal (struct signal *sig, simtime_t t1, simtime_t t2, int state, struct sig_value **prev)
/* prev : previous sig_value (if known!) */
{
    int after2value;
    struct sig_value *sval, *tmp, *new1, *new2;
    struct sig_value *before1, *before2;
    simtime_t help;

    if (t1 == t2) return (0);

    if (sig -> expr) { /* the periodicity of the signal is destroyed */
	delSigexpr (sig -> expr);
	sig -> expr = NULL;
	sig -> endless = 0;
    }

    somethingChanged = 1;

    if (t2 < t1) { help = t1; t1 = t2; t2 = help; }

    if (*prev) /* use prev to speed up searching process! */
	sval = *prev;
    else
	sval = sig -> begin_value;

    before1 = NULL;
    while (sval && sval -> time < t1) {
	before1 = sval;
	*prev = sval;
	sval = sval -> next;
    }

    /* if (before1 && sval) then: before1 -> next == sval */

    if (!before1 || before1 -> value != state) {

	NEWSVAL (new1);
	new1 -> time = sig -> begin_value ? t1 : 0;
	new1 -> value = state;
	new1 -> next = sval;
	new1 -> prev = before1;
	if (sval) sval -> prev = new1;

        if (!before1) {
            if (!sig -> begin_value) {
                sig -> begin_value = new1;
                sig -> end_value = new1;
            }
            else {
                sig -> begin_value = new1;
            }
            after2value = state;
        }
        else {
            before1 -> next = new1;
            after2value = before1 -> value;
        }
        before2 = new1;
    }
    else {
        /* else a previous sval already has a value 'state': */
        /*  before1 && before1 -> value == state             */

        before2 = before1;
        after2value = state;
    }

    while (sval && sval -> time <= t2) {

        /* delete everything with t1 <= time <= t2 */

        after2value = sval -> value;
        tmp = sval;
        sval = sval -> next;

        if (tmp -> next)
            tmp -> next -> prev = tmp -> prev;
        else
            sig -> end_value = tmp -> prev;

        if (tmp -> prev)
            tmp -> prev -> next = tmp -> next;
        else
            sig -> begin_value = tmp -> next;

        DELETESVAL (tmp);
    }

    if (sval && sval -> value == state && after2value == state) return (1);

    if (before2 && before2 -> prev && before2 -> value == before2 -> prev -> value) {
	before2 -> time = t2;
    }
    else {
	NEWSVAL (new2);
	new2 -> time = t2;
	new2 -> value = sval ? after2value : state;
	new2 -> next = sval;
	new2 -> prev = before2;
	if (sval)
	    sval -> prev = new2;
	else
	    sig -> end_value = new2;
	if (before2) before2 -> next = new2;
    }

    if (t2 > Endtime) Endtime = t2;
    if (t2 > SimEndtime) SimEndtime = t2;

    return (1);
}

void copySignal (struct signal *sig1, struct signal *sig2)
{
    struct sig_value *sval1, *sval2, *newval;
    SIGNALELEMENT *sigel, *last_sigel, *new_sigel;
    char flag;

    if (sig1 == sig2) return;

    somethingChanged = 1;

    delSigval (sig2 -> begin_value, sig2 -> end_value);
    sig2 -> begin_value = NULL;
    sig2 -> end_value = NULL;

    delSigexpr (sig2 -> expr);
    sig2 -> expr = NULL;

    sval2 = NULL;
    for (sval1 = sig1 -> begin_value; sval1; sval1 = sval1 -> next) {
	NEWSVAL (newval);
	newval -> time = sval1 -> time;
	newval -> value = sval1 -> value;
	newval -> next = NULL;
	newval -> prev = sval2;
	if (sval2)
	    sval2 -> next = newval;
	else
	    sig2 -> begin_value = newval;
	sval2 = newval;
    }
    sig2 -> end_value = sval2;

    last_sigel = NULL;
    flag = 's';
    sigel = sig1 -> expr;
    while (sigel) {
        NEW (new_sigel, 1, SIGNALELEMENT);
	new_sigel -> val = sigel -> val;
	new_sigel -> len = sigel -> len;
	if (sig2 -> expr) {
	    if (flag == 'c')
		last_sigel -> child = new_sigel;
	    else
		last_sigel -> sibling = new_sigel;
	}
	else
	    sig2 -> expr = new_sigel;
	last_sigel = new_sigel;

        if (sigel -> child) {
	    flag = 'c';
	    sigel = sigel -> child;
	}
	else {
	    flag = 's';
	    sigel = sigel -> sibling;
	}
    }

    sig2 -> endless = sig1 -> endless;
}

void storeSignalPart (struct signal *sig, simtime_t t1, simtime_t t2)
{
    struct sig_value *sval, *newval, *oldval;
    simtime_t help;

    if (storedVals) {
        delSigval (storedVals, (struct sig_value *)0);
	storedVals = NULL;
    }

    if (t1 == t2) return;
    if (t1 > t2) { help = t2; t2 = t1; t1 = help; }

    sval = sig -> begin_value;
    while (sval && sval -> time <= t1) sval = sval -> next;
    /* sval -> time > t1 */
    if (sval) sval = sval -> prev;
    /* sval -> time <= t1 && t1 < t2 */
    if (!sval) return;

    NEWSVAL (oldval);
    oldval -> time  = 0;
    oldval -> value = sval -> value;
    oldval -> prev  = NULL;
    storedVals = oldval;

    while ((sval = sval -> next) && sval -> time < t2) {
	NEWSVAL (newval);
	newval -> time  = sval -> time - t1;
	newval -> value = sval -> value;
	newval -> prev  = oldval;
	oldval -> next  = newval;
	oldval = newval;
    }
    oldval -> next = NULL;
    storedTD = t2 - t1;
}

void addStoredSignalPart (struct signal *sig, simtime_t t, int factor)
{
    struct sig_value *sval, *help;
    SIGNALELEMENT *new_sig_el, *last_sig_el, *new_part;
    simtime_t t1, t2;
    int i;

    if (!storedVals) return; /* empty buffer */

    if (factor < 0) { /* an endless part is added: use 'sig -> expr' to store the periodical part */

	/* delete old expr */

	delSigexpr (sig -> expr);

	NEW (sig -> expr, 1, SIGNALELEMENT);
	sig -> expr -> child = NULL;
	sig -> expr -> sibling = NULL;
	sig -> expr -> len = -1;
	sig -> endless = 1;

	last_sig_el = sig -> expr;

	/* create expr until t */

	sval = sig -> begin_value;
	t1 = 0;
	while (sval && t1 < t) {
	    if (sval -> time > t1) {
		NEW (new_sig_el, 1, SIGNALELEMENT);
		new_sig_el -> child = NULL;
		new_sig_el -> sibling = NULL;
		new_sig_el -> val = sval2eval (sval -> prev -> value);
		if (sval -> time < t)
		    new_sig_el -> len = sval -> time - t1;
		else
		    new_sig_el -> len = t - t1;

		last_sig_el -> sibling = new_sig_el;
		last_sig_el = new_sig_el;

		t1 = sval -> time;
	    }
	    sval = sval -> next;
	}

	/* add contents of buffer to expr */

	NEW (new_part, 1, SIGNALELEMENT);
	new_part -> sibling = NULL;
	new_part -> len = -1;

	NEW (new_part -> child, 1, SIGNALELEMENT);
	new_part -> child -> child = NULL;
	new_part -> child -> sibling = NULL;
	new_part -> child -> len = storedTD;

	last_sig_el -> sibling = new_part;

	last_sig_el = new_part -> child;

	sval = storedVals;
	while (sval) {
	    t1 = sval -> time + t;
	    if (sval -> next)
		t2 = sval -> next -> time + t;
	    else
		t2 = storedTD + t;
	    NEW (new_sig_el, 1, SIGNALELEMENT);
	    new_sig_el -> child = NULL;
	    new_sig_el -> sibling = NULL;
	    new_sig_el -> val = sval2eval (sval -> value);
	    new_sig_el -> len = t2 - t1;

	    last_sig_el -> sibling = new_sig_el;
	    last_sig_el = new_sig_el;

	    sval = sval -> next;
	}

	sig -> expr -> val = new_part -> val = new_part -> child -> val = last_sig_el -> val;

        /* update sig values */

	delSigval (sig -> begin_value, sig -> end_value);
	sig -> begin_value = NULL;
	sig -> end_value = NULL;
	curr_time = 0;
	simperiod = SimEndtime;
	addSgnPart (sig, sig -> expr, 1);
	adjustSgnPart (sig);
	somethingChanged = 1;
	return;
    }

    help = NULL;

    for (i = 0; i < factor; ++i) {
	sval = storedVals;
	t1 = t + i * storedTD;
	do {
	    if (sval -> next)
		t2 = t + sval -> next -> time + i * storedTD;
	    else
		t2 = t + (i + 1) * storedTD;
	    changeSignal (sig, t1, t2, sval -> value, &help);
	    t1 = t2;
	} while ((sval = sval -> next));
    }
}

int sval2eval (int v)
{
    switch (v) {
	case  0: return (L_state);
	case  1: return (X_state);
	case  2: return (H_state);
	case -1: return (F_state);
    }
    return (X_state);
}

int writeSet (char *filename)
{
    char help[128];
    char outbuf[128];
    char tmpfilename[128];
    FILE *fp1;
    FILE *fp2;
    char *w;
    int c, cnt, len;
    char state, prevstate;
    simtime_t duration;
    struct signal *sig;
    struct sig_value *sval;
    int sigunitPre, sigunitNow, sigunitDone;
    int simperiodPre, simperiodNow, simperiodDone;
    int backslash, copycommand, goOn;
    long firstcharpos;
    struct signalelement *sigel;

    state = prevstate = 'x'; // init, to suppress compiler warning

    for (sig = Begin_signal; sig; sig = sig -> next) {
	if (!sig -> begin_value) {
	    windowMessage ("description has unspecified signal(s)", -1);
	    return (0);
	}
    }

    fp1 = fopen (filename, "r");

    sprintf (tmpfilename, "%d.cxx", (int)getpid ());
    if (!(fp2 = fopen (tmpfilename, "w"))) {
	sprintf (help, "Cannot open %s", tmpfilename);
	windowMessage (help, -1);
	return (0);
    }

    /* first copy fp1 to fp2 but without 'editable' set commands
       and options sigunit and simperiod changed to their correct value */

    sigunitDone = 0;
    simperiodDone = 0;

    if (fp1) {

	while ((c = getc (fp1)) != EOF) {

	    /* read command by command */

            ungetc (c, fp1);

	    firstcharpos = ftell (fp1);

	    while ((c = getc (fp1)) == ' ' || c == '\t');

            copycommand = 0;

	    if (c == 's' && getc (fp1) == 'e' && getc (fp1) == 't'
		&& ((c = getc (fp1)) == ' ' || c == '\t' || c == '\\')) {

		while ((c = getc (fp1)) != '=') {
		    if (c == 'n') {
		        if ((c = getc (fp1)) == 'o'
		            && (c = getc (fp1)) == '_'
		            && (c = getc (fp1)) == 'e'
		            && (c = getc (fp1)) == 'd'
		            && (c = getc (fp1)) == 'i'
		            && (c = getc (fp1)) == 't') {

			    /* It is a set command with 'no_edit',
			       copy only if (1) the command is disabled,
			       or (2) the signal still exists in the program
			       as 'non-editable'
			    */

			    while ((c = getc (fp1)) == ' ');
			    if (c == '*' && (c = getc (fp1)) == '#') {
				copycommand = 1;            /* case (1) */
			    }
			    else {
				fscanf (fp1, "%s", help);
				sig = existSignal (help);
				if (sig && sig -> no_edit)
				    copycommand = 1;         /* case (2) */
			    }
			    break;
			}
			else {
			    ungetc (c, fp1);
			}
		    }
		}
	    }
	    else {
		copycommand = 1;    /* it is no set command */
	    }

	    fseek (fp1, firstcharpos, 0);

	    sigunitPre = 0; sigunitNow = 0;
	    simperiodPre = 0; simperiodNow = 0;

	    backslash = 0;
	    goOn = 1;
	    while (goOn) {

		w = getword (fp1);

		if (sigunitNow && *w != '\\') {
		    fprintf (fp2, "%le ", Timescaling);
		    sigunitNow = 0;
		    sigunitDone = 1;
		}
		else if (simperiodNow && *w != '\\') {
		    fprintf (fp2, "%lld ", SimEndtime);
		    simperiodNow = 0;
		    simperiodDone = 1;
		}
		else if (copycommand && *w != EOF) {
		    fprintf (fp2, "%s", w);
		}

                if (sigunitPre) {
		    if (*w == '=') {
		        sigunitNow = 1;
		        sigunitPre = 0;
		    }
		    else if (*w != '\\')
		        sigunitPre = 0;
		}
                if (simperiodPre) {
		    if (*w == '=') {
		        simperiodNow = 1;
		        simperiodPre = 0;
		    }
		    else if (*w != '\\')
		        simperiodPre = 0;
		}

		if (compareWord (w, "sigunit")) sigunitPre = 1;
		if (compareWord (w, "simperiod")) simperiodPre = 1;

		if ((*w == '\n' && !backslash) || *w == EOF) goOn = 0;

		if (*w == '\\')
		    backslash = 1;
		else if (*w != ' ' && *w != '\t' && !(*w == '\n' && backslash))
		    backslash = 0;
	    }
	}
	fclose (fp1);
    }

    if (!sigunitDone)   fprintf (fp2, "option sigunit = %le\n", Timescaling);
    if (!simperiodDone) fprintf (fp2, "option simperiod = %lld\n", SimEndtime);

    /* then write new set commands to fp2 */

    for (sig = Begin_signal; sig; sig = sig -> next) {

        if (sig -> no_edit) {
	    sig = sig -> next;
	    continue;
	}

        sprintf (outbuf, "set %s = ", sig -> name);

        cnt = strlen (outbuf);
        fprintf (fp2, "%s", outbuf);

        if (sig -> endless) {
	    sigel = sig -> expr;
	    if (sigel) sigel = sigel -> sibling;
	    while (sigel && sigel -> sibling) {
		state = eval2cmdval (sigel -> val);
		sprintf (outbuf, "%c*%lld ", state, sigel -> len);

		len = strlen (outbuf);
		if (cnt + len > 80 - 2) {
		    fprintf (fp2, " \\\n     ");
		    cnt = 5;
		}
		cnt = cnt + len;
		fprintf (fp2, "%s", outbuf);

		sigel = sigel -> sibling;
	    }
	    if (sigel && sigel -> child && sigel -> child -> sibling) {
		sigel = sigel -> child -> sibling;

		if (cnt + 1 > 80 - 2) {
		    fprintf (fp2, " \\\n     ");
		    cnt = 5;
		}
		fprintf (fp2, "(");

		while (sigel) {
		    state = eval2cmdval (sigel -> val);
		    sprintf (outbuf, "%c*%lld ", state, sigel -> len);

		    if (cnt + strlen (outbuf) > 80 - 2) {
			fprintf (fp2, " \\\n     ");
			cnt = 5;
		    }
		    cnt = cnt + strlen (outbuf);
		    fprintf (fp2, outbuf);

		    sigel = sigel -> sibling;
		}

		if (cnt + 3 > 80) {
		    fprintf (fp2, " \\\n     ");
		    cnt = 5;
		}
		fprintf (fp2, ")*~");
	    }
	    fprintf (fp2, "\n");

	    sig = sig -> next;
	    continue;
	}

        sval = sig -> begin_value;
        while (sval) {
		 if (sval -> value ==  0) state = 'l';
            else if (sval -> value ==  1) state = 'x';
            else if (sval -> value ==  2) state = 'h';
            else if (sval -> value == -1) state = 'f';

            if (sval -> next) {
                duration = sval -> next -> time - sval -> time;
                if (duration == 0)
                    outbuf[0] = '\0';
                else
                    sprintf (outbuf, "%c*%lld ", state, duration);
            }
            else {
                if (prevstate == state)
                    outbuf[0] = '\0';
                else
                    sprintf (outbuf, "%c", state);
            }

            if ((sval -> next && cnt + strlen (outbuf) > 80 - 2)
                || (!sval -> next && cnt + strlen (outbuf) > 80)) {
                fprintf (fp2, " \\\n     ");
                cnt = 5;
            }
            cnt = cnt + strlen (outbuf);
            fprintf (fp2, outbuf);

            prevstate = state;

            sval = sval -> next;
        }

        fprintf (fp2, "\n");
    }

    fclose (fp2);

    /* finally move tmpfile to file */
    sprintf (help, "mv %s %s", tmpfilename, filename);
    system (help);

    sprintf (help, "\"%s\" ready", filename);
    windowMessage (help, -1);
    return (1);
}

#define MAXWORD 511

char *getword (FILE *fp)
{
    static char buf[MAXWORD+1];
    int c, i;

    /* a word is
       1. the character '\n' or EOF.
       2. the string "/ *", "* /" (space = nil) "#*" or "*#", possibly
	  preceded by one of the characters '\\' or '=', or by an
	  identifier string containing none of the characters '\\', '\n'
	  or '=' or the strings "/ *", "* /", "#*" or "*#".
       3. a string of blanks (' ' or '\t'), possibly preceded by one
	  of the characters '\\' or '=', or by an identifier string
	  containing none of the characters '\\', '\n' or '='
	  or the strings "/ *" or "* /", "#*" or "*#".
    */

    c = getc (fp);
    if (c == '\n' || c == EOF) {
	buf[0] = c;
	buf[1] = '\0';
	return (buf);
    }

#define isBEG_COMMENT (c == '*' && (buf[i-1] == '/' || buf[i-1] == '#'))
#define isEND_COMMENT ((c == '/' || c == '#') && buf[i-1] == '*')

    i = 0;
    if (!isspace (c)) {
	buf[i++] = c;
	if (c != '\\' && c != '=') {
	    c = getc (fp);
	    while (!isspace (c) && !isBEG_COMMENT && !isEND_COMMENT
		   && c != '\\' && c != '=' && c != EOF && i < MAXWORD) {
		buf[i++] = c;
		c = getc (fp);
	    }
	    if (isBEG_COMMENT || isEND_COMMENT) {
		buf[i++] = c;
		goto ret;
	    }
	}
	else {
	    c = getc (fp);
	}
    }

    while (c != '\n' && isspace (c) && i < MAXWORD) {
	buf[i++] = c;
	c = getc (fp);
    }
    if (c != EOF) ungetc (c, fp);
ret:
    buf[i] = '\0';
    return (buf);
}

int compareWord (char *w1, char *w2)
{
    int i = 0;

    while (w1[i] == w2[i] && w2[i] != '\0') i++;

    if (w2[i] == '\0' && (w1[i] == '\0' || w1[i] == ' ' || w1[i] == '\t')) {
	return (1); /* match */
    }
    return (0); /* no match */
}

char eval2cmdval (int v)
{
    switch (v) {
	case L_state: return ('l');
	case X_state: return ('x');
	case H_state: return ('h');
	case F_state: return ('f');
    }
    return ('x');
}
