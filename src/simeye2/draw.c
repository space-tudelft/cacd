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
#define  XAW_BC  1        /* for sun */
#include <X11/StringDefs.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/AsciiText.h>

extern Widget canvas;
extern Widget resw;
extern Widget pltw;

extern struct sig_value *storedVals;

extern int rdHandler;
extern char **SV;

#define MINPIXTIMESTEP  50
#define MINPIXGRID       5
#define MINPIXVOLTSTEP  15
#define MINPIXVOLTSPACE 25 /* below this value, only min-max levels are drawn */
#define MINPIXLEVELSPACE 8 /* below this value, no levels are drawn at all */

#define MAXSHOWYVALS    50 /* maximum nr. of signals for which y val can be displayed */

Grid drawP_x, drawP_y;

GC gc3;

Pixel pointerColor;

static XGCValues values;
static GC gc = NULL;
static Display *display;
static Window window;

static Arg args[10];

int fontWidth = 6;    /* font 6x13 */
int fontHeight = 8;

double grid_time;
double grid_volt;
double t_outunit;
double v_outunit;

Grid canvasHeight;
Grid canvasWidth;
Grid canvasTop;
Grid canvasBottom;
Grid canvasLeft;
Grid canvasRight;
Grid canvasNames;
Grid canvasVolts;

Grid sigSpace;
Grid axisLength;
Grid axisStart;
Grid axisY;
Grid oY;
double t_unit;
int tprecision;      /* for pointer info */
char tprec_str[16];
int tSprecision;     /* for the y-axis */
char tSprec_str[16];
double v_unit;      /* double ! */
int vprecision;
char vprec_str[16];
int vSprecision;
char vSprec_str[16];

struct signal *Top_signal;
struct signal *Bottom_signal;
int Curr_nr_signals;

simtime_t startTime;
simtime_t stopTime;

int curr_umin;
int curr_umax;

struct sig_value *Comb_vals_U[MAXSHOWYVALS + 1];
struct sig_value *Comb_vals_L[MAXSHOWYVALS + 1];

Pixel normalColor;
Pixel logicColor;
Pixel logicxColor;
Pixel analogColor;
Pixel backgrColor;

int showVoltScale;
int showYValues;
int showYValuesPos;
int showYValuesOn = 0;

char windowMesBuf[128];
char windowListFn[128];
Grid windowMesX;

int stringValuesPresent;

void ClickType (Widget w, caddr_t client_data, caddr_t call_data); // main.c
int  determWindow (char modif, Grid x1, Grid y1, Grid x2, Grid y2);
void drawDottedLine (Display *display, Window window, GC gc, Grid x1, Grid y1, Grid x2, Grid y2);
void drawInit (void);
void drawLineSignal (Grid x1, Grid y1, Grid x2, Grid y2, int flag);
void drawSignal (struct signal *sig, int index, char type, int flag, int inc_only);
void drawYScale (Grid yb, int drawDL);
void findValSig (int i, int stringValue, simtime_t t, char type, double *fval, char *pval);
void findVoltSig (Grid x, Grid y, double *pv, char *s);
void findXYValSig (Grid x, Grid y, char type, double *fval, char *pval);

void drawStartup ()
{
    XrmDatabase db;
    XrmValue rmval;
    char *str_type = 0;
    XColor screenColor;
    XColor exactColor;
    Colormap colormap;
    Dimension dw, dh;
    Pixel pb;
    int n;

    window = XtWindow (canvas);
    display = XtDisplay (canvas);

    db = XtDatabase (display);

    n = 0;
    XtSetArg (args[n], XtNwidth, &dw); n++;
    XtSetArg (args[n], XtNheight, &dh); n++;
    XtSetArg (args[n], XtNbackground, &backgrColor); n++;
    XtGetValues (canvas, args, n);
    canvasWidth  = dw;
    canvasHeight = dh;

    colormap = XDefaultColormap (display, DefaultScreen (display));

#define getResource(A,B) (XrmGetResource (db, A, B, &str_type, &rmval) == True && \
	XAllocNamedColor (display, colormap, rmval.addr, &screenColor, &exactColor))

    pb = BlackPixel (display, DefaultScreen (display));
    normalColor = getResource ("simeye.total.canvas.foreground", "Simeye.Form.Widget.Foreground")? screenColor.pixel : pb;
    logicColor   = getResource ("simeye.high_lowColor", "Simeye.Foreground") ? screenColor.pixel : normalColor;
    logicxColor  = getResource ("simeye.undefinedColor","Simeye.Foreground") ? screenColor.pixel : logicColor;
    analogColor  = getResource ("simeye.voltsColor",    "Simeye.Foreground") ? screenColor.pixel : normalColor;
    pointerColor = getResource ("simeye.pointerColor",  "Simeye.Foreground") ? screenColor.pixel : normalColor;
    pointerColor ^= backgrColor;

    gc = XCreateGC (display, window, 0, &values);

    XSetForeground (display, gc, normalColor);

    values.foreground = pointerColor;
    values.function   = GXxor;
    gc3 = XCreateGC (display, window, GCForeground | GCFunction, &values);

    /* among other things for windowMessage: */

    canvasTop    = canvasHeight / 10;
    canvasBottom = canvasHeight / 8;
    canvasLeft   = canvasWidth / 30;
    canvasRight  = canvasWidth / 20;

    axisStart = 2 * canvasLeft; /* for windowMessage */
    axisLength = canvasWidth - canvasRight - axisStart;

    windowMesBuf[0] = '\0';
    windowListFn[0] = '\0';
}

void draw (char modif, Grid x1, Grid y1, Grid x2, Grid y2)
{
    struct signal *sig;
    int inc_only;

    if (modif == 't')
	inc_only = 1;
    else
	inc_only = 0;

    if (rdHandler && windowListFn[0]) {
	windowList (windowListFn);   /* redraw window listing */
	rdHandler = 0;
	return;
    }
    else
	windowListFn[0] = '\0';

    if (!Begin_signal) {
	windowMessage ("No signals loaded", -1);
	return;
    }

    if (Endtime <= 0) {
	if (editing) {
	    Endtime = 20;   /* just a useful value */
	    Begintime = 0;
	    startTime = Begintime;
	    stopTime = Endtime;
	    SimEndtime = 0;
	}
	else {
	    windowMessage ("end of simulation time <= 0", -1);
	    return;
	}
    }
    else if (stopTime <= startTime) {
	/* This may occur if Endtime > 0 and first signal is created */
	startTime = Begintime;
	stopTime = Endtime;
    }

#if 0
	if (currLogic && !Logic) {
	    ClickType (resw);
	    /* this is a redraw after click has been modified */
	    /* and no successful read operation has been executed */
	}
	if (!currLogic && Logic) {
	    ClickType (pltw);        /* idem */
	}
#endif

    if (modif) {
	if (modif != 'a' && modif != 'b' && modif != 't') {
	    if (determWindow (modif, x1, y1, x2, y2) == 0) return;
	}

	if (modif == 'f' || modif == 'a' || modif == 'm'
	|| modif == 'l' || modif == 'r' || modif == 'd' || modif == 'u') {
	    clear ();
	}
    }

    if (!inc_only) drawInit ();

    sig = Top_signal;
    while (sig && sig -> prev != Bottom_signal) {
	if (currLogic) {
	    drawSignal (sig, sig -> i, 0, 0, inc_only);
	    if (sig -> layover)
		drawSignal (sig -> layover, sig -> i, 0, 1, inc_only);
	}
	else {
	    drawSignal (sig, sig -> i, 'l', 0, inc_only);
	    if (sig -> layover)
		drawSignal (sig -> layover, sig -> i, 'l', 1, inc_only);
	    drawSignal (sig, sig -> i, 'u', 0, inc_only);
	    if (sig -> layover)
		drawSignal (sig -> layover, sig -> i, 'u', 1, inc_only);
	}
        sig = sig -> next;
    }

    if (rdHandler && windowMesBuf[0])
	windowMessage (windowMesBuf, windowMesX);   /* redraw window message */
    else if (modif != 's' && modif != 't' && modif != 'b')
	windowMesBuf[0] = '\0';

    rdHandler = 0;

    XSync (display, False);
}

void redrawListOrMessage ()
{
    if (windowListFn[0]) { windowList (windowListFn); return; }    // redraw window listing
    if (windowMesBuf[0]) windowMessage (windowMesBuf, windowMesX); // redraw window message
}

int determWindow (char modif, Grid x1, Grid y1, Grid x2, Grid y2)
{
    int i;
    int new_nr_signals;
    simtime_t newStartTime;
    simtime_t newStopTime;
    double hFactor = 0.5;
    int new_umin;
    int new_umax;
    int d;

    if (modif == 'f' || modif == 's' || modif == 'm') {
			    /* first draw ('f', 's') or full window ('m') */

        if (Top_signal == Begin_signal && Bottom_signal == End_signal
	&& startTime == Begintime && stopTime == Endtime && modif == 'm') {
	    return (0);
	}

	Top_signal = Begin_signal;
	Bottom_signal = End_signal;
	Curr_nr_signals = Nr_signals;

	startTime = Begintime;
	stopTime = Endtime;

	curr_umin = Global_umin;
	curr_umax = Global_umax;
    }
    else if (modif == 'i' || modif == 'I') {         /* zoom in */

	if (Curr_nr_signals > 1) {
	    new_nr_signals = Curr_nr_signals;

	    i = Curr_nr_signals * (Min (y1, y2) - canvasTop) / (canvasHeight - canvasTop - canvasBottom);
	    while (i > 0 && new_nr_signals > 1) {
		Top_signal = Top_signal -> next;
		new_nr_signals--;
		i--;
	    }

	    i = Curr_nr_signals * (canvasHeight - canvasBottom - Max (y1, y2))
		/ (canvasHeight - canvasTop - canvasBottom);
	    while (i > 0 && new_nr_signals > 1) {
		Bottom_signal = Bottom_signal -> prev;
		new_nr_signals--;
		i--;
	    }

	    Curr_nr_signals = new_nr_signals;
	}
	else if ((modif == 'I' || doDetailZoom) && !currLogic) {

            /* compute in double to prevent integer overflow */

	    new_umax = curr_umax - (double)(curr_umax - curr_umin)
			     * (Min (y1, y2) - canvasTop - sigSpace / 8)
			     / (canvasHeight - canvasTop - canvasBottom - sigSpace / 4);

	    new_umin = curr_umin + (double)(curr_umax - curr_umin)
			     * (canvasHeight - canvasBottom - Max (y1, y2) - sigSpace / 8)
			     / (canvasHeight - canvasTop - canvasBottom - sigSpace / 4);

	    if (new_umax < curr_umax) curr_umax = new_umax;
	    if (new_umin > curr_umin) curr_umin = new_umin;

	    if (curr_umin >= curr_umax) curr_umin = curr_umax - 1;
	}

	newStartTime = startTime + (double)((Min (x1, x2) - axisStart) / (double)axisLength) * (stopTime - startTime);
	newStopTime  = startTime + (double)((Max (x1, x2) - axisStart) / (double)axisLength) * (stopTime - startTime);

	if (newStartTime >= startTime) startTime = newStartTime;
	if (newStopTime <= stopTime) stopTime = newStopTime;
	if (stopTime <= startTime) stopTime = startTime + 1;
    }
    else if (modif == 'o') {         /* zoom out */

	if (Curr_nr_signals == 1 && !currLogic
	    && (curr_umin > Global_umin || curr_umax < Global_umax)) {

            /* compute in double to prevent integer overflow */

            if (y1 == y2)
		d = 1;
	    else
		d = Max (y1, y2) - Min (y1, y2);

	    new_umax = curr_umax + (double)(curr_umax - curr_umin) * (Min (y1, y2) - canvasTop - sigSpace / 8) / d;
	    new_umin = curr_umin - (double)(curr_umax - curr_umin) * (canvasHeight - canvasBottom - Max (y1, y2) - sigSpace / 8) / d;

	    if (new_umax > curr_umax) curr_umax = new_umax;
	    if (new_umin < curr_umin) curr_umin = new_umin;

	    if (curr_umax > Global_umax) curr_umax = Global_umax;
	    if (curr_umin < Global_umin) curr_umin = Global_umin;
	}
	else {
	    curr_umin = Global_umin;
	    curr_umax = Global_umax;
	}

	if (!(Curr_nr_signals == 1 && !currLogic && (curr_umin > Global_umin || curr_umax < Global_umax))) {

	    new_nr_signals = Curr_nr_signals;

	    i = Curr_nr_signals * (Min (y1, y2) - canvasTop)
		/ (Max (y1, y2) - Min (y1, y2));
	    while (i > 0 && Top_signal != Begin_signal) {
		Top_signal = Top_signal -> prev;
		new_nr_signals++;
		i--;
	    }

	    i = Curr_nr_signals * (canvasHeight - canvasBottom - Max (y1, y2))
		/ (Max (y1, y2) - Min (y1, y2));
	    while (i > 0 && Bottom_signal != End_signal) {
		Bottom_signal = Bottom_signal -> next;
		new_nr_signals++;
		i--;
	    }

	    Curr_nr_signals = new_nr_signals;
	}

	if (x1 == x2)
	    d = 1;
	else
	    d = Max(x1, x2) - Min(x1, x2);

	newStartTime = stopTime - (double)((Max (x1, x2) - axisStart) / (double)d) * (stopTime - startTime);
	newStopTime = startTime + (double)((axisStart + axisLength - Min (x1, x2)) / (double)d) * (stopTime - startTime);

	if (newStartTime < Begintime) startTime = Begintime;
	else if (newStartTime < startTime) startTime = newStartTime;

	if (newStopTime > Endtime && !editing) /* <== Notice !! */
	    stopTime = Endtime;
	else if (newStopTime > stopTime)
	    stopTime = newStopTime;

	if (stopTime <= startTime) stopTime = startTime + 1;
    }
    else if (modif == 'l') {      /* move left */

        if (startTime == Begintime) return (0);

	newStartTime = startTime - hFactor * (stopTime - startTime);

	if (newStartTime < Begintime) newStartTime = Begintime;

	newStopTime = newStartTime + (stopTime - startTime);

        startTime = newStartTime;
        stopTime = newStopTime;
    }
    else if (modif == 'r') {      /* move right */

        if (stopTime == Endtime) return (0);

	newStopTime = stopTime + hFactor * (stopTime - startTime);

	if (newStopTime > Endtime) newStopTime = Endtime;

	newStartTime = newStopTime - (stopTime - startTime);

        startTime = newStartTime;
        stopTime = newStopTime;
    }
    else if (modif == 'd') {      /* move down */

        if (Bottom_signal == End_signal) return (0);

	i = Curr_nr_signals - 1;
	if (i < 1) i = 1;
	while (i > 0 && Bottom_signal != End_signal) {
	    Top_signal = Top_signal -> next;
	    Bottom_signal = Bottom_signal -> next;
	    i--;
	}
    }
    else if (modif == 'u') {      /* move up */

        if (Top_signal == Begin_signal) return (0);

	i = Curr_nr_signals - 1;
	if (i < 1) i = 1;
	while (i > 0 && Top_signal != Begin_signal) {
	    Top_signal = Top_signal -> prev;
	    Bottom_signal = Bottom_signal -> prev;
	    i--;
	}
    }

    if (curr_umin == curr_umax) curr_umax = curr_umin + 1;

    return (1);
}

void drawInit ()
{
    int n;
    int tdivider;
    char t_str[20];
    char t_str2[20];
    char v_str[20];
    char buf[32];
    Grid yb;
    int i;
    int len, maxlen;
    double help;
    int div2_5;
    struct signal *sig;
    Dimension dw, dh;

    n = 0;
    XtSetArg (args[n], XtNwidth, &dw), n++;
    XtSetArg (args[n], XtNheight, &dh), n++;
    XtGetValues (canvas, args, n);
    canvasWidth = dw;
    canvasHeight = dh;
    canvasTop = canvasHeight / 10;
    canvasBottom = canvasHeight / 8;
    canvasLeft = canvasWidth / 30;
    canvasRight = canvasWidth / 20;

    /* drawing of time axis */

    stringValuesPresent = 0;
    maxlen = 0;
    sig = Top_signal;
    for (i = 1; i <= Curr_nr_signals; i++) {
	len = strlen (sig -> name);
	if (len > maxlen)
	    maxlen = len;
	stringValuesPresent = stringValuesPresent || sig -> stringValue;
	sig = sig -> next;
    }
    if (maxlen < 5) maxlen = 5;

    canvasNames = (maxlen + 1) * fontWidth;

    sigSpace = (canvasHeight - canvasTop - canvasBottom) / (double)Curr_nr_signals;

    if (!currLogic
	&& sigSpace * 0.75 >= MINPIXVOLTSPACE
	&& sigSpace * 0.75 >= MINPIXLEVELSPACE)
	showVoltScale = 1;
    else
        showVoltScale = 0;

    if ((currLogic || (!currLogic && showVoltScale))
        && sigSpace * 0.75 >= MINPIXLEVELSPACE
        && Curr_nr_signals <= MAXSHOWYVALS) {
        showYValuesPos = 1;
    }
    else {
        showYValuesPos = 0;
    }

    if ((!currLogic && showVoltScale)
        || (currLogic && stringValuesPresent && showYValuesPos)) {
	canvasVolts = 5 * fontWidth;   /* just a guess of what is required */
    }
    else {
	canvasVolts = 2 * fontWidth;
    }

    axisY = canvasHeight - (3 * canvasBottom) / 4;

    axisStart = canvasLeft + canvasNames;
    axisLength = canvasWidth - canvasLeft - canvasNames - canvasRight - canvasVolts;

    grid_time = (double) (axisLength) / (double) (stopTime - startTime);

    XDrawLine (display, window, gc, axisStart, axisY, axisStart + axisLength, axisY);

    t_unit = convdec ((double)(stopTime - startTime) * Timescaling);
    if (t_unit * grid_time / Timescaling < MINPIXTIMESTEP) t_unit = t_unit * 10;
    while (t_unit * grid_time / Timescaling >= MINPIXTIMESTEP * 10) t_unit = t_unit / 10;

    if (t_unit / Timescaling >= 10) {
	if (t_unit * grid_time / Timescaling >= MINPIXTIMESTEP * 5) {
	    t_unit = t_unit / 5;
	    if (t_unit / Timescaling >= 10)
		tdivider = 4;
	    else
		tdivider = 2;
	}
	else if (t_unit * grid_time / Timescaling >= MINPIXTIMESTEP * 2) {
	    t_unit = t_unit / 2;
	    tdivider = 5;
	}
	else {
	    tdivider = 5;
	}
    }
    else {
	if (t_unit / Timescaling < 1) t_unit = Timescaling;
	tdivider = 1;
    }

    t_outunit = convdec (stopTime * Timescaling);

    /* (Timescaling / grid_time) == Seconds per pixel */

    tprecision = 0;
    help = t_outunit / (Timescaling / grid_time);
    while (help > 1) {
	help = help / 10;
	tprecision++;
    }
    sprintf (tprec_str, "%%+.%dlf", tprecision);

    tSprecision = 0;
    help = t_outunit / t_unit;
    while (help > 1) {
	help = help / 10;
	tSprecision++;
    }
    sprintf (tSprec_str, "%%.%dlf", tSprecision);

    sprintf (t_str, "%.0le", t_outunit);
    i = 0;
    while (t_str[i] != '\0') {     /* remove mantisse */
        t_str[i] = t_str[i + 1];
	i++;
    }
    XDrawString (display, window, gc, canvasLeft,
	    Min (canvasHeight - (2 * canvasBottom) / 9, axisY + canvasBottom / 6 + 5 * fontHeight / 2),
	    t_str, strlen (t_str));

    i = startTime * Timescaling / t_unit;
    if (i * t_unit < startTime * Timescaling) i++;
    while (i * t_unit < stopTime * Timescaling + Timescaling / grid_time) {

	drawDottedLine (display, window, gc,
	       (Grid)(axisStart + (i * t_unit / Timescaling - startTime) * grid_time), axisY,
	       (Grid)(axisStart + (i * t_unit / Timescaling - startTime) * grid_time), canvasTop);

	XDrawLine (display, window, gc,
	       (Grid)(axisStart + (i * t_unit / Timescaling - startTime) * grid_time), axisY,
	       (Grid)(axisStart + (i * t_unit / Timescaling - startTime) * grid_time), axisY + canvasBottom / 6);
	sprintf (t_str2, tSprec_str, i * t_unit / t_outunit);
	XDrawString (display, window, gc,
	       (Grid)(axisStart + (i * t_unit / Timescaling - startTime) * grid_time),
		Min (canvasHeight - (2 * canvasBottom) / 9, axisY + canvasBottom / 6 + 5 * fontHeight / 2),
	       t_str2, strlen (t_str2));
	i++;
    }

    i = startTime * tdivider * Timescaling / t_unit;
    if (i * t_unit / tdivider < startTime * Timescaling) i++;
    while (i * t_unit / tdivider < stopTime * Timescaling + Timescaling / grid_time) {

	XDrawLine (display, window, gc,
         (Grid)(axisStart + (i * t_unit / (Timescaling * tdivider) - startTime) * grid_time), axisY,
         (Grid)(axisStart + (i * t_unit / (Timescaling * tdivider) - startTime) * grid_time), axisY + canvasBottom / 9);
	i++;
    }

    if (grid_time >= MINPIXGRID) {    /* display smallest grid */

	i = 0;
	while (i * grid_time <= axisLength) {

	    XDrawLine (display, window, gc,
	     (Grid)(axisStart + i * grid_time), axisY,
	     (Grid)(axisStart + i * grid_time), axisY - canvasBottom / 9);
	    i++;
	}
    }

    /* For some reason (?) the first draw statement for the canvas
    ** has no effect when the signals are drawn immediately after
    ** startup (by specifying the inputfile on the command line).
    ** Therefore, the first draw statement is repeated here:
    */

    XDrawLine (display, window, gc, axisStart, axisY, axisStart + axisLength, axisY);

    XDrawString (display, window, gc, canvasLeft,
	    Min (canvasHeight - (2 * canvasBottom) / 9, axisY + canvasBottom / 6 + 5 * fontHeight / 2),
	    t_str, strlen (t_str));


    /* drawing of names and voltage levels */

    grid_volt = (double) sigSpace * 0.75 / (curr_umax - curr_umin);

    oY = (curr_umin - 0) * grid_volt;
    /* this value has to be added to the y value of a signal
       to fit within the space that is reserved for the signal */

    if (showVoltScale) {

        /* v_unit and v are in Volts ! */

	v_unit = convdec ((double)(curr_umax - curr_umin) * Voltscaling);
	if (v_unit * grid_volt / Voltscaling < MINPIXVOLTSTEP) v_unit = v_unit * 10;
	while (v_unit * grid_volt / Voltscaling >= MINPIXVOLTSTEP * 10) v_unit = v_unit / 10;

        div2_5 = 0;
	if (v_unit * grid_volt / Voltscaling >= MINPIXVOLTSTEP * 4) {
	    v_unit = v_unit / 4;
	    div2_5 = 1;
	}
	else if (v_unit * grid_volt / Voltscaling >= MINPIXVOLTSTEP * 5) {
	    v_unit = v_unit / 5;

	    /* Since maximum voltage is usually 5V we prefer division by 4
	       above division by 5 */
	}
	else if (v_unit * grid_volt / Voltscaling >= MINPIXVOLTSTEP * 2) {
	    v_unit = v_unit / 2;
	}

	v_outunit = convdec ((Max (Abs (curr_umin), Abs (curr_umax))) * Voltscaling);

        /* (Voltscaling / grid_volt) == Volts per pixel */

	vprecision = 0;
	help = v_outunit / (Voltscaling / grid_volt);
	while (help > 1) {
	    help = help / 10;
	    vprecision++;
	}
	if (Spice) vprecision = vprecision + 2;
	sprintf (vprec_str, "%%+.%dlf", vprecision);

	vSprecision = 0;
	help = v_outunit / v_unit;
	while (help > 1) {
	    help = help / 10;
	    vSprecision++;
	}
	if (div2_5) vSprecision++;  /* ! */
	sprintf (vSprec_str, "%%+.%dlf", vSprecision);

	sprintf (v_str, "%.0le", v_outunit);
	i = 0;
	while (v_str[i] != '\0') {     /* remove mantisse */
	    v_str[i] = v_str[i + 1];
	    i++;
	}
	XDrawString (display, window, gc,
		axisStart + axisLength + fontWidth * 3 / 2,
		(canvasTop + fontHeight)/ 2, v_str, strlen (v_str));
    }

    if (showYValuesPos && showYValues)
	showYValues = 1;
    else
	showYValues = 0;

    sig = Top_signal;
    for (i = 1; i <= Curr_nr_signals; i++) {

        sig -> i = i;

	yb = canvasTop + (canvasHeight - canvasTop - canvasBottom) * i / Curr_nr_signals - (sigSpace / 8);

	if ((curr_umax - curr_umin) * grid_volt < MINPIXLEVELSPACE) {

	    /* do not draw levels */

	}
	else if (showVoltScale) {

            drawYScale (yb, 1);
	}
	else {

            /* draw only the minimum and maximum level for each signal */

	    drawDottedLine (display, window, gc,
		   axisStart, (Grid)(yb + oY - grid_volt * curr_umin),
	           axisStart + axisLength,
		   (Grid)(yb + oY - grid_volt * curr_umin));

	    drawDottedLine (display, window, gc,
		   axisStart, (Grid)(yb + oY - grid_volt * curr_umax),
		   axisStart + axisLength,
		   (Grid)(yb + oY - grid_volt * curr_umax));
	}

	XDrawString (display, window, gc, canvasLeft, yb, sig -> name, strlen (sig -> name));

        if (sig -> layover)
	    XDrawString (display, window, gc, canvasLeft, yb - 3 * fontHeight / 2,
			 sig -> layover -> name, strlen (sig -> layover -> name));

	if (Cmd && sig -> endless) {
	    XDrawString (display, window, gc, axisStart + axisLength + fontWidth, yb,
			 "~", strlen ("~"));
	}

	sig = sig -> next;
    }

    if (Cmd) {
	sprintf (buf, "input signals");

	for (i = 0; i < strlen (buf); i++) {
	    XDrawString (display, window, gc, canvasWidth - 1 * (canvasRight + canvasVolts) / 3,
			 canvasHeight * (3 + i + 1) / (strlen (buf) + 6) - fontHeight, &buf[i], 1);
	}
    }
}

void drawYScale (Grid yb, int drawDL)
{
    int first;
    Grid h;
    double v;
    char v_str[20];


    v = ((int)(curr_umin * Voltscaling / v_unit)) * v_unit;

    /* to obtain a multiple value of v_unit ! */

    if (v < curr_umin * Voltscaling)
	v = v + v_unit;

    first = 1;
    while (v <= curr_umax * Voltscaling + 0.001 * (curr_umax - curr_umin) * Voltscaling) {

	if (drawDL)
	    drawDottedLine (display, window, gc, axisStart,
		   (Grid)(yb + oY - grid_volt * v / Voltscaling),
		   axisStart + axisLength,
		   (Grid)(yb + oY - grid_volt * v / Voltscaling));

	if (0.25 * sigSpace > grid_volt * v_unit / Voltscaling) {
	    h = fontHeight / 2;   /* center text */
	}
	else if (first) {
	    h = 0;
	}
	else {
	    if ((curr_umax - v / Voltscaling) * grid_volt < fontHeight)
		h = fontHeight - (curr_umax - v / Voltscaling) * grid_volt;
	    else
		h = fontHeight / 2;   /* center text */
	}
	first = 0;

	if (!showYValuesOn) {
	    sprintf (v_str, vSprec_str, v / v_outunit);
	    if (*v_str == '+') *v_str = ' ';
	    XDrawString (display, window, gc, axisStart + axisLength + fontWidth * 3 / 2,
		     (Grid)(yb + oY - grid_volt * v / Voltscaling + h),
		     v_str, strlen (v_str));
	}

	v = v + v_unit;
    }
}

double convdec (double f)
{
    int     i = 0;

    if (f == 0) return (f);

    if (f >= 1) {
	while (f > 10) {
	    f = f / 10;
	    i++;
	}
    }
    else {
	while (f < 1) {
	    f = f * 10;
	    i--;
	}
    }

    f = 1;
    if (i >= 0) {
	while (i > 0) {
	    f = f * 10;
	    i--;
	}
    }
    else {
	while (i < 0) {
	    f = f / 10;
	    i++;
	}
    }

    return (f);
}

void drawSignal (struct signal *sig, int index, char type, int flag, int inc_only)
{
    struct sig_value *sval;
    Grid xcurr, xprev, ycurr, yprev, yb, xl, gv2;
    int  stringValue = sig -> stringValue;
    XGCValues xgcval;

    if (flag) drawLineSignal (0, 0, 0, 0, -1); /* initialization for stipple line */

    if (type == 'l')
	sval = sig -> begin_value_L;
    else if (type == 'u')
	sval = sig -> begin_value_U;
    else
	sval = sig -> begin_value;

    if (!sval) {
	if (type == 'l')
	    sig -> last_value_L = NULL;
	else if (type == 'u')
	    sig -> last_value_U = NULL;
	else
	    sig -> last_value = NULL;
	return;
    }

    xl = canvasLeft + canvasNames;
    yb = oY + canvasTop + (canvasHeight - canvasTop - canvasBottom) * index / Curr_nr_signals - (sigSpace / 8);

    if (!flag) {
	if (currLogic)
	    XSetForeground (display, gc, logicColor);
	else
	    XSetForeground (display, gc, analogColor);
    }

    if (inc_only) {
	if (type == 'l') {
	    if (sig -> last_value_L) sval = sig -> last_value_L;
	}
	else if (type == 'u') {
	    if (sig -> last_value_U) sval = sig -> last_value_U;
	}
	else {
	    if (sig -> last_value) sval = sig -> last_value;
	}
    }

    while (sval -> next && sval -> next -> time < startTime) sval = sval -> next;
    if (sval -> time < startTime && sval -> next -> time == startTime) sval = sval -> next;

    /* if inc_only = 0 or first time on canvas:
       sval -> time <= startTime && sval -> next -> time >= startTime ! */

    if (sval -> time <= startTime && sval -> next && sval -> next -> time >= startTime) {

	/* first time on canvas for this signal */

	if (showYValuesPos && !flag) {
	    if (type == 'l')
		Comb_vals_L[index] = sval;
	    else
		Comb_vals_U[index] = sval;
	}

	if (currLogic || sval -> next -> time == sval -> time)
	    yprev = yb - grid_volt * sval -> value;
	else {
	    yprev = yb - grid_volt * (sval -> value +
			   (double) ((startTime - sval -> time)
			   / (double)(sval -> next -> time - sval -> time))
			      * (sval -> next -> value - sval -> value));
	}
	xcurr = xl;
    }
    else {
	/* inc_only = 1 */
	yprev = yb - grid_volt * sval -> value;
	xcurr = xl + (sval -> time - startTime) * grid_time;
    }

    gv2 = 2 * grid_volt;

    while ((sval = sval -> next) && sval -> time <= stopTime) {
	xprev = xcurr;
	xcurr = xl + (sval -> time - startTime) * grid_time;
	if (currLogic) {
	    if (stringValue) {
		ycurr = sval -> value;
		XSetForeground (display, gc, logicColor);
		drawLineSignal (xprev, yb, xcurr, yb, flag);
		drawLineSignal (xprev, yb - gv2, xcurr, yb - gv2, flag);
		if (ycurr != yprev)
		    drawLineSignal (xcurr, yb, xcurr, yb - gv2, flag);
	    }
	    else {
		ycurr = yb - grid_volt * sval -> value;
		if (sval -> prev -> value >= 0) {
		    if (sval -> prev -> value == 1 && !flag)
			XSetForeground (display, gc, logicxColor);
		    drawLineSignal (xprev, yprev, xcurr, yprev, flag);
		    XSetForeground (display, gc, logicColor);
		    if (ycurr != yprev && sval -> value >= 0)
			drawLineSignal (xcurr, yprev, xcurr, ycurr, flag);
		}
		else {           /* Free state */
		    xgcval.line_style = LineDoubleDash;
		    XChangeGC (display, gc, GCLineStyle, &xgcval);
		    yprev -= gv2;
		    drawLineSignal (xprev, yprev, xcurr, yprev, flag);
		    xgcval.line_style = LineSolid;
		    XChangeGC (display, gc, GCLineStyle, &xgcval);
		}
	    }
	}
	else {
	    ycurr = yb - grid_volt * sval -> value;
	    drawLineSignal (xprev, yprev, xcurr, ycurr, flag);
	}
	yprev = ycurr;
    }

    if (sval && sval -> prev && sval -> prev -> time < stopTime) {

	xprev = xcurr;
	xcurr = xl + (stopTime - startTime) * grid_time;
	if (currLogic) {
	    if (stringValue) {
		XSetForeground (display, gc, logicColor);
		drawLineSignal (xprev, yb, xcurr, yb, flag);
		drawLineSignal (xprev, yb - gv2, xcurr, yb - gv2, flag);
	    }
	    else if (sval -> prev -> value >= 0) {
		if (sval -> prev -> value == 1 && !flag)
		    XSetForeground (display, gc, logicxColor);
		drawLineSignal (xprev, yprev, xcurr, yprev, flag);
		if (!flag)
		    XSetForeground (display, gc, logicColor);
	    }
	    else {           /* Free state */
		xgcval.line_style = LineDoubleDash;
		XChangeGC (display, gc, GCLineStyle, &xgcval);
		yprev -= gv2;
		drawLineSignal (xprev, yprev, xcurr, yprev, flag);
		xgcval.line_style = LineSolid;
		XChangeGC (display, gc, GCLineStyle, &xgcval);
	    }
	}
	else {
	    ycurr = yb - grid_volt * (
			sval -> prev -> value +
			(double)((stopTime - sval -> prev -> time)
			 / (double)(sval -> time - sval -> prev -> time))
			* (sval -> value - sval -> prev -> value));
	    drawLineSignal (xprev, yprev, xcurr, ycurr, flag);
	}
    }

    if (type == 'l')
	sig -> last_value_L = sig -> end_value_L;
    else if (type == 'u')
	sig -> last_value_U = sig -> end_value_U;
    else
	sig -> last_value = sig -> end_value;
}

void drawLineSignal (Grid x1, Grid y1, Grid x2, Grid y2, int flag)
{
    static Grid xlatest;
    static Grid ylatest;
    Grid x, y;
    double ya, yb;
    double d1, d2, d3;
    Grid sep = 3;

    if (flag < 0) {
	xlatest = -100;
	ylatest = -100;
	return;
    }

    if (x2 < x1) {
	fprintf (stderr, "He x2 < x1\n");
	die (1);                     /* algorithm does not handle x2 < x1 */
    }

    if (!currLogic) {
	/* perform clipping (in case of detail zoom in) */

	if (y1 < canvasTop + sigSpace / 8) {
	    if (y2 < canvasTop + sigSpace / 8) {
		return;
	    }
	    x1 = x1 + (x2 - x1) * (canvasTop + sigSpace / 8 - y1) / (y2 - y1);
	    y1 = canvasTop + sigSpace / 8;
	}
	else if (y1 > canvasHeight - canvasBottom - sigSpace / 8) {
	    if (y2 > canvasHeight - canvasBottom - sigSpace / 8) {
		return;
	    }
	    x1 = x1 + (x2 - x1) * (canvasHeight - canvasBottom - sigSpace / 8 - y1) / (y2 - y1);
	    y1 = canvasHeight - canvasBottom - sigSpace / 8;
	}

	if (y2 < canvasTop + sigSpace / 8) {
	    x2 = x1 + (x2 - x1) * (canvasTop + sigSpace / 8 - y1) / (y2 - y1);
	    y2 = canvasTop + sigSpace / 8;
	}
	else if (y2 > canvasHeight - canvasBottom - sigSpace / 8) {
	    x2 = x1 + (x2 - x1) * (canvasHeight - canvasBottom - sigSpace / 8 - y1) / (y2 - y1);
	    y2 = canvasHeight - canvasBottom - sigSpace / 8;
	}
    }

    if (flag > 0) {

	/* stipple by placing a dot at every position that is
	   'sep' away from a previous dot
	*/

	x = x1;
	y = y1;

        while (!(x == x2 && y == y2)) {

	    if ((x2 > x1 && x >= xlatest + sep)
		|| (x2 < x1 && x <= xlatest - sep)
		|| (y2 > y1 && y >= ylatest + sep)
		|| (y2 < y1 && y <= ylatest - sep)) {
		XDrawLine (display, window, gc, x, y, x, y);
		xlatest = x;
		ylatest = y;
	    }

            if (x2 > x1) {

		ya = (y2 - y1) * (x - x1) / (double)(x2 - x1) + y1;
		yb = (y2 - y1) * (x + 1 - x1) / (double)(x2 - x1) + y1;

		if (y2 > y1) {

                    d1 = Abs (y - ya);
                    d2 = Abs (y - yb);
                    d3 = Abs (y + 1 - yb);
                    if (d1 < d2 && d1 < d3) {
			y++;
		    }
		    else if (d2 < d3) {
			x++;
		    }
		    else {
			x++;
			y++;
		    }
		}
		else {

                    d1 = Abs (y - ya);
                    d2 = Abs (y - yb);
                    d3 = Abs (y - 1 - yb);
                    if (d1 < d2 && d1 < d3) {
			y--;
		    }
		    else if (d2 < d3) {
			x++;
		    }
		    else {
			x++;
			y--;
		    }
		}
	    }
	    else if (y2 > y1) {
		y++;
	    }
	    else {
		y--;
	    }
	}

	if (x1 != x2) {
	    while (x != x2) {

		y = (y2 - y1) * (x - x1) / (x2 - x1) + y1;

		if ((x2 > x1 && x >= xlatest + sep)
		    || (x2 < x1 && x <= xlatest - sep)
		    || (y2 > y1 && y >= ylatest + sep)
		    || (y2 < y1 && y <= ylatest - sep)) {
		    XDrawLine (display, window, gc, x, y, x, y);
		    xlatest = x;
		    ylatest = y;
		}

		if (x2 > x1)
		    x++;
		else
		    x--;
	    }
	}
	else {
	    while (y != y2) {

		x = (x2 - x1) * (y - y1) / (y2 - y1) + x1;

		if ((x2 > x1 && x >= xlatest + sep)
		    || (x2 < x1 && x <= xlatest - sep)
		    || (y2 > y1 && y >= ylatest + sep)
		    || (y2 < y1 && y <= ylatest - sep)) {
		    XDrawLine (display, window, gc, x, y, x, y);
		    xlatest = x;
		    ylatest = y;
		}

		if (y2 > y1)
		    y++;
		else
		    y--;
	    }
	}
    }
    else {
	XDrawLine (display, window, gc, x1, y1, x2, y2);
    }
}

void drawDottedLine (Display *display, Window window, GC gc, Grid x1, Grid y1, Grid x2, Grid y2)
{
    Grid x, y;
    Grid sep = 6;

    x = x1;
    y = y1;

    if (x1 != x2 && y1 != y2)
	fprintf (stderr, "Sorry only orthogonal dotted lines\n"), die (1);

    if (x1 == x2) {
	if (y2 > y1) {
	    y = y + sep;
	    while (y <= y2) {
		XDrawLine (display, window, gc, x, y, x, y);
		y = y + sep;
	    }
	}
	else {
	    y = y - sep;
	    while (y >= y2) {
		XDrawLine (display, window, gc, x, y, x, y);
		y = y - sep;
	    }
	}
    }
    else {
	if (x2 > x1) {
	    x = x + sep;
	    while (x <= x2) {
		XDrawLine (display, window, gc, x, y, x, y);
		x = x + sep;
	    }
	}
	else {
	    x = x - sep;
	    while (x >= x2) {
		XDrawLine (display, window, gc, x, y, x, y);
		x = x - sep;
	    }
	}
    }
}

int drawPointerInfo (int initial, Grid x, Grid y, int flag, int showLogic, int round, int moving)
/* showLogic: if showLogic != 0 then show the corresponding logic state;
	      if showLogic == -1 then display 'free state' */
/* round:     if not yet indicated by showLogic != 0 or !currLogic,
	      turning on this variable indicates rounding */
{
    char buf1[32];
    char buf2[32];
    char buf[80];
    char buf1a[32];
    char buf1b[32];
    char buf2a[32];
    char buf2b[32];
    static int memory = 0;
    static double vmem;
    static double tmem;
    static char smem;
    double t, v;
    int l, s, store;
    int dPI = 0;

    if (initial >= 0) {

	store = 0;
	if (flag) {
	    if (memory) memory = 0;
	    else {
		store = 1;
		memory = 1;
	    }
	}

	if (initial == 1)
	    sprintf (buf, "- - - - ");
	/*
	else if (y < canvasTop && !currLogic)
	    sprintf (buf, "\\/ \\/ \\/");
	else if (y > axisY && !currLogic)
	    sprintf (buf, "/\\ /\\ /\\");
	*/
	else if ((x - axisStart) / grid_time < 0)
	    sprintf (buf, " > > > >");
	else if (startTime + (x - axisStart) / grid_time > stopTime)
	    sprintf (buf, "< < < < ");
	else {

	    t = findTime (x, (!currLogic || showLogic || round)) * Timescaling / t_outunit;

	    sprintf (buf1, tprec_str, t);
	    *buf1 = ' ';

            if (store) tmem = t;

	    if (memory) {
		sprintf (buf1a, tprec_str, tmem);
		*buf1a = ' ';
		t -= tmem;
		sprintf (buf1b, tprec_str, t);
		if (*buf1b == '+') *buf1b = t >= 0 ? ' ' : '-';
	    }

	    if (!showVoltScale) {
		if (currLogic && showYValuesPos) {
		    findXYValSig (x, y, 'u', &v, buf2);
		    if (memory)
			sprintf (buf, "x =%s  y =%s  ( - %s  = %s )", buf1, buf2, buf1a, buf1b);
		    else
			sprintf (buf, "x =%s  y =%s", buf1, buf2);
		}
		else {
		    if (memory)
			sprintf (buf, "x =%s  ( - %s  = %s )", buf1, buf1a, buf1b);
		    else
			sprintf (buf, "x =%s", buf1);
		}
	    }
	    else {
		findVoltSig (x, y, &v, buf2);

		if (store) vmem = v;

                if (memory) {
		    sprintf (buf2a, vprec_str, vmem);
		    if (*buf2a == '+') *buf2a = vmem >= 0 ? ' ' : '-';
		    v -= vmem;
		    sprintf (buf2b, vprec_str, v);
		    if (*buf2b == '+') *buf2b = v >= 0 ? ' ' : '-';

		    sprintf (buf, "x =%s  y =%s  ( - %s %s  = %s %s )", buf1, buf2, buf1a, buf2a, buf1b, buf2b);
		}
		else
		    sprintf (buf, "x =%s  y =%s", buf1, buf2);

		dPI = 1;   /* valid position to draw horizontal cursor edge */
	    }

            if (showLogic) {
                if (!memory || store) {
		    if (showLogic == -1)
			s = -1; /* free state is forced */
		    else
                        s = findState (y);
			 if (s ==  2) smem = 'I';
                    else if (s ==  1) smem = 'X';
                    else if (s ==  0) smem = 'O';
                    else if (s == -1) smem = 'f';
                }
                l = strlen(buf);
                sprintf (&buf[l], "  %c", smem);
            }
	}

	windowMessage (buf, moving ? x : -1);
    }
    else {
	memory = 0;
        windowMessage (NULL, -1);
    }

    return (dPI);
}

void drawValueInfo (int initial, Grid x)
{
    int i;
    char pval[16];
    double v;
    simtime_t t;
    struct signal *sig;
    Grid yb;

    if (initial == 1) {

	if (showVoltScale)
	    XClearArea (display, window, axisStart + axisLength, canvasTop,
			canvasWidth - axisStart - axisLength,
			canvasHeight - canvasBottom - canvasTop,
			False);

	showYValuesOn = 1;

	return;
    }
    else if (initial == -1) {
	showYValuesOn = 0;
    }

    t = findTime (x, !currLogic);

    sig = Top_signal;
    for (i = 1; i <= Curr_nr_signals; i++) {

	yb = canvasTop
	     + (canvasHeight - canvasTop - canvasBottom) * i / Curr_nr_signals
	     - (sigSpace / 8);

        if (currLogic && !stringValuesPresent)
	    XClearArea (display, window, (Grid)(axisStart + axisLength + fontWidth - 1),
	      (Grid)(yb - sigSpace * 3 / 4 - 1),
	      (Grid)(2 * fontWidth + 2),
	      (Grid)(sigSpace * 3 / 4 + 4), False);
	else
	    XClearArea (display, window, (Grid)(axisStart + axisLength + fontWidth * 1 / 4),
	      (Grid)(yb - sigSpace * 3 / 4 - 1),
	      (Grid)(canvasWidth - axisStart - axisLength - fontWidth * 1 / 4),
	      (Grid)(sigSpace * 3 / 4 + 4), False);

	if (initial == -1) {
	    if (showVoltScale)
		drawYScale (yb, 0);
	}
	else {
	    if (currLogic || Spice) {
		findValSig (i, sig -> stringValue, t, 'u', &v, pval);

		XDrawString (display, window, gc,
			     axisStart + axisLength + fontWidth,
			     (Grid)(yb),
			     pval, strlen (pval));
	    }
	    else {
		findValSig (i, sig -> stringValue, t, 'l', &v, pval);

		XDrawString (display, window, gc,
			     axisStart + axisLength + fontWidth,
			     (Grid)(yb),
			     pval, strlen (pval));

		findValSig (i, sig -> stringValue, t, 'u', &v, pval);

		XDrawString (display, window, gc,
			     axisStart + axisLength + fontWidth,
			     (Grid)(yb - sigSpace * 3 / 4 + fontHeight),
			     pval, strlen (pval));
	    }
	}

	sig = sig -> next;
    }
}

void findXYValSig (Grid x, Grid y, char type, double *fval, char *pval)
{
    simtime_t t;
    struct signal *sig;

    t = findTime (x, !currLogic);

    sig = findSignal (y, 0);

    findValSig (sig -> i, sig -> stringValue, t, type, fval, pval);
}

void findValSig (int i, int stringValue, simtime_t t, char type, double *fval, char *pval)
{
    register struct sig_value *sval;
    double v;

    if (type == 'l')
	sval = Comb_vals_L[i];
    else
	sval = Comb_vals_U[i];

    while (sval && sval -> next && sval -> time <= t) sval = sval -> next;

    if (!sval) {
	strcpy (pval, " U");
	return;
    }

    /* sval -> time > t */
    while (sval -> prev && sval -> time > t) sval = sval -> prev;

    /* (sval -> time <= t && sval -> next -> time > t)
	|| (sval -> time <= t && !sval -> next)
	|| (sval -> time >  t && !sval -> prev) */

    if ((sval -> time > t && !sval -> prev)
     || (sval -> time < t && !sval -> next)) {
	strcpy (pval, " U");
    }
    else {
	if (currLogic) {
	    if (stringValue) sprintf (pval, " %s", SV[sval -> value]);
	    else if (sval -> value ==  2) strcpy (pval, " I");
	    else if (sval -> value ==  1) strcpy (pval, " X");
	    else if (sval -> value ==  0) strcpy (pval, " O");
	    else if (sval -> value == -1) strcpy (pval, " f");
	}
	else {
	    if (sval -> time < t) {
		v = (sval -> value + ((t - sval -> time)
			 / (double)(sval -> next -> time - sval -> time))
			* (sval -> next -> value - sval -> value))
		    * Voltscaling / v_outunit;
	    }
	    else
		v = sval -> value * Voltscaling / v_outunit;

	    sprintf (pval, vprec_str, v);
	    if (*pval == '+') *pval = v >= 0 ? ' ' : '-';

	    *fval = v;
	}
    }

    if (type == 'l')
	Comb_vals_L[i] = sval;
    else
	Comb_vals_U[i] = sval;
}

struct signal *findSignal (Grid y, int extend)
/* extend: signal below bottom signal can also be selected */
{
    struct signal *sig;
    int i;

    if (extend) y = y + sigSpace / 2;

    if (y >= canvasHeight - canvasBottom) {
	if (!extend) {
	    i = Curr_nr_signals - 1;  /* Bottom_signal */
	}
	else {
	    if (Bottom_signal == End_signal)
		return (NULL);
	    else
		return (Bottom_signal -> next);
	}
    }
    else {
	i = Curr_nr_signals * (y - canvasTop) / (canvasHeight - canvasTop - canvasBottom);
    }

    if (i < 0) i = 0;

    sig = Top_signal;
    while (i > 0 && sig != Bottom_signal) {
	sig = sig -> next;
	i--;
    }

    return (sig);
}

int findVolt (Grid y)
{
    int i;
    Grid yb;
    int v;

    i = Curr_nr_signals * (y - canvasTop) / (canvasHeight - canvasTop - canvasBottom);

    if (i < 0) i = 0;
    else if (i >= Curr_nr_signals) i = Curr_nr_signals - 1;

    yb = canvasTop + (canvasHeight - canvasTop - canvasBottom) * (i + 1) / Curr_nr_signals - (sigSpace / 8);

    v = (yb + oY - y) / grid_volt + 0.5;   /* add 0.5 for rounding */

    return (v);
}

void findVoltSig (Grid x, Grid y, double *pv, char *s)
{
    int i;
    simtime_t t;

    i = Curr_nr_signals * (y - canvasTop) / (canvasHeight - canvasTop - canvasBottom);

    if (i < 0) i = 0;
    else if (i >= Curr_nr_signals) i = Curr_nr_signals - 1;
    i++;

    t = findTime (x, !currLogic);

    findValSig (i, 0, t, 'u', pv, s);

    findGrid (i, t, 0, *pv, &drawP_x, &drawP_y);
}

int findState (Grid y)
{
    int s, v;

    v = findVolt (y);

    if (v < Global_umin + (double)(Global_umax - Global_umin) / 3)
        s = Global_umin;
    else if (v > Global_umin + (double) 2 * (Global_umax - Global_umin) / 3)
        s = Global_umax;
    else
        s = Global_umin + (Global_umax - Global_umin) / 2;

    return (s);
}

simtime_t findTime (Grid x, int round)
{
    simtime_t t;
    double a;

    if (x < axisStart)
        x = axisStart;
    else if (x > axisStart + axisLength)
        x = axisStart + axisLength;

    if (round)
	a = 0.5;           /* add 0.5 for rounding */
    else
	a = 0.0;

    t = (double) startTime + (x - axisStart) / grid_time + a;

    return (t);
}

void findGrid (int sig_i, simtime_t time, int lval, double fval, Grid *x, Grid *y)
{
    Grid xl, yb;

    yb = canvasTop + (canvasHeight - canvasTop - canvasBottom) * sig_i / Curr_nr_signals - (sigSpace / 8);
    xl = canvasLeft + canvasNames;

    *x = xl + (time - startTime) * grid_time;
    if (currLogic && lval == -1)
	lval = 1;                /* draw level free state at level x state */
    if (currLogic)
	*y = yb + oY - grid_volt * lval;
    else
	*y = yb + oY - grid_volt * fval * v_outunit / Voltscaling;
}

void findBBox (struct signal *sig, simtime_t t1, simtime_t t2, Grid *x1, Grid *y1, Grid *x2, Grid *y2)
{
    Grid xl, yb;

    yb = canvasTop + (canvasHeight - canvasTop - canvasBottom) * (sig -> i) / Curr_nr_signals - (sigSpace / 8);
    xl = canvasLeft + canvasNames;

    *x1 = xl + (t1 - startTime) * grid_time - 1;
    *x2 = xl + (t2 - startTime) * grid_time + 1;

    *y1 = yb + oY - grid_volt * -0.15 + 1;
    *y2 = yb + oY - grid_volt * 2.15 - 1;
}

void markSignal (struct signal *sig)
{
    Grid yb;
    yb = canvasTop + (canvasHeight - canvasTop - canvasBottom) * (sig -> i) / Curr_nr_signals - (sigSpace / 8);
    XDrawString (display, window, gc, (canvasLeft / 2), yb, "*", 1);
}

void unMarkSignal (struct signal *sig)
{
    Grid yb;
    yb = canvasTop + (canvasHeight - canvasTop - canvasBottom) * (sig -> i) / Curr_nr_signals - (sigSpace / 8);
    XClearArea (display, window, (canvasLeft / 2), yb - fontHeight, fontWidth, fontHeight, False);
}

void insSigOnCanvas (struct signal *sig, struct signal *sig2)
{
    Curr_nr_signals++; /* don't forget ! */
    Nr_signals++;

    /* sig is inserted above (previous to) sig2.
       if sig2 == NULL, sig is inserted as End_signal */

    if (sig2 == Begin_signal) Begin_signal = sig;
    if (sig2 == NULL) End_signal = sig;

    if (sig2 == Top_signal) {
	sig -> next = sig2;
	sig -> prev = sig2 -> prev;
	if (sig2 -> prev) sig2 -> prev -> next = sig;
	sig2 -> prev = sig;
	Top_signal = sig;
    }
    else if (sig2 == NULL) {
	sig -> next = NULL;
	sig -> prev = Bottom_signal;
	Bottom_signal -> next = sig;
	Bottom_signal = sig;
    }
    else {
	if (sig2 -> prev == Bottom_signal) Bottom_signal = sig;
	sig -> next = sig2;
	sig -> prev = sig2 -> prev;
	sig2 -> prev -> next = sig;
	sig2 -> prev = sig;
    }
}

void delSigFromCanvas (struct signal *sig, int onlyMoved)
{
    if (sig == Top_signal) Top_signal = sig -> next;
    if (sig == Bottom_signal) Bottom_signal = sig -> prev;

    if (sig -> next)
	sig -> next -> prev = sig -> prev;
    else
	End_signal = sig -> prev;

    if (sig -> prev)
	sig -> prev -> next = sig -> next;
    else
	Begin_signal = sig -> next;

    Curr_nr_signals--;
    Nr_signals--;

    if (onlyMoved) return;

    Endtime = 0; /* find possible new Endtime */

    for (sig = Begin_signal; sig; sig = sig -> next) {
	if (currLogic) {
	    if (sig -> end_value && sig -> end_value -> time > Endtime)
		 Endtime = sig -> end_value -> time;
	}
	else {
	    if (sig -> end_value_U && sig -> end_value_U -> time > Endtime)
		 Endtime = sig -> end_value_U -> time;
	}
    }
    if (editing) SimEndtime = Endtime;
}

void newSigOnCanvas (char *name)
{
    struct signal *sig;

    NEW (sig, 1, struct signal);
    NEW (sig -> name, strlen (name) + 1, char);
    strcpy (sig -> name, name);
    sig -> begin_value = NULL;
    sig -> begin_value_L = NULL;
    sig -> begin_value_U = NULL;
    sig -> end_value = NULL;
    sig -> end_value_L = NULL;
    sig -> end_value_U = NULL;
    sig -> expr = NULL;
    sig -> no_edit = 0;
    sig -> endless = 0;
    sig -> stringValue = 0;

    if (!Begin_signal) {
        Begin_signal = sig;
        End_signal = sig;
        Top_signal = sig;
        Bottom_signal = sig;
        sig -> next = NULL;
        sig -> prev = NULL;
        Curr_nr_signals = 1;
        Nr_signals = 1;
	curr_umin = Global_umin;
	curr_umax = Global_umax;
    }
    else {
        sig -> next = Bottom_signal -> next;
        sig -> prev = Bottom_signal;
        if (Bottom_signal -> next) Bottom_signal -> next -> prev = sig;
        Bottom_signal -> next = sig;
        if (End_signal == Bottom_signal) End_signal = sig;
        Bottom_signal = sig;
        Curr_nr_signals++;
        Nr_signals++;
    }
}

void changeTimescaling (double newTimescaling)
{
    struct signal *sig;
    struct sig_value *sval, *help;
    SIGNALELEMENT *sigel, *prev_sigel, *next;
    double factor, over;
    simtime_t old;
    int val;
    int remain_expr;
    char descendent;

    factor = Timescaling / newTimescaling;

    if (factor < 1) {
	startTime  = factor * (startTime  + 0.5);
	stopTime   = factor * (stopTime   + 0.5);
	Begintime  = factor * (Begintime  + 0.5);
	Endtime    = factor * (Endtime    + 0.5);
	SimEndtime = factor * (SimEndtime + 0.5);
	simperiod  = factor * (simperiod  + 0.5);
    }
    else {
	startTime  = factor * startTime;
	stopTime   = factor * stopTime;
	Begintime  = factor * Begintime;
	Endtime    = factor * Endtime;
	SimEndtime = factor * SimEndtime;
	simperiod  = factor * simperiod;
    }

    newEndtime = Endtime;

    for (sig = Begin_signal; sig; sig = sig -> next) {

	if (sig -> endless) {

	    /* update possible expression */

	    val = -999;
	    remain_expr = 0;
	    descendent = 's';
	    over = 0;
	    sigel = sig -> expr;
	    prev_sigel = NULL;
	    while (sigel) {
		if (sigel -> len > 0) {
		    if (val == -999 && descendent != 'c') val = sigel -> val;
		    if (factor < 1) {
			old = sigel -> len;
			sigel -> len = (sigel -> len + 0.5 + over) * factor;
			/* keep track of how much time is lost and add
			   it to len the next time to ensure that the global
			   time differences are as less as possible */
			over += old - (sigel -> len / factor);
		    }
		    else {
			sigel -> len = sigel -> len * factor;
		    }
		}

		if (sigel -> len == 0) {
		    /* due to rounding sigel -> len has become 0, remove it from the list */
		    if (sigel -> child) {
			next = sigel -> child;
			descendent = 'c';
		    }
		    else {
			next = sigel -> sibling;
			descendent = 's';
		    }
		    if (prev_sigel) {
			if (prev_sigel -> child)
			    prev_sigel -> child = next;
			else
			    prev_sigel -> sibling = next;
		    }
		    else
			sig -> expr = next;

		    DELETE (sigel);
		    sigel = next;
		}
		else {
		    if (sigel -> len > 0 && descendent != 'c') {
			remain_expr++;
			val = sigel -> val;
		    }
		    prev_sigel = sigel;
		    if (sigel -> child) {
			sigel = sigel -> child;
			descendent = 'c';
		    }
		    else {
			sigel = sigel -> sibling;
			descendent = 's';
		    }
		}
	    }

	    delSigval (sig -> begin_value, sig -> end_value);

	    if (remain_expr > 1) {
		sig -> begin_value = NULL;
		sig -> end_value = NULL;
		curr_time = 0;
		(void) addSgnPart (sig, sig -> expr, 1);
	    }
	    else {
		/* due to rounding, the periodicity of the signal has been lost */
		sig -> endless = 0;
		delSigexpr (sig -> expr);
		sig -> expr = NULL;

		NEWSVAL (sval);
		sval -> time = 0;
		switch (val) {
		    case H_state: sval -> value =  2; break;
		    case L_state: sval -> value =  0; break;
		    case F_state: sval -> value = -1; break;
		    default:      sval -> value =  1;
		}
		sval -> next = NULL;
		sval -> prev = NULL;
		sig -> begin_value = sig -> end_value = sval;
	    }

	    adjustSgnPart (sig);
	}
	else {

	    /* update signal value */

	    if (sig -> expr) {
		/* This expression is no longer valid */
		delSigexpr (sig -> expr);
		sig -> expr = NULL;
	    }

	    for (sval = sig -> begin_value; sval; sval = sval -> next) {
		if (factor < 1) {
		    sval -> time = (sval -> time + 0.5) * factor;
		    if (sval -> prev && sval -> prev -> time == sval -> time) {
			/* Two values at equal time; Delete one */
			help = sval;
			sval = sval -> prev;
			sval -> value = help -> value;
			sval -> next = help -> next;
			if (help -> next) help -> next -> prev = sval;
			DELETESVAL (help);
		    }
		    if (sval -> next && sval -> prev && sval -> prev -> value == sval -> value) {
			/* Equal previous value; Delete one */
			help = sval;
			sval = sval -> prev;
			sval -> next = help -> next;
			help -> next -> prev = sval;
			DELETESVAL (help);
		    }
		}
		else
		    sval -> time = sval -> time * factor;
	    }
	}
    }

    for (sval = storedVals; sval; sval = sval -> next) {
	if (factor < 1) {
	    sval -> time = (sval -> time + 0.5) * factor;
	    if (sval -> prev && sval -> prev -> time == sval -> time) {
		/* Two values at equal time; Delete one */
		help = sval;
		sval = sval -> prev;
		sval -> value = help -> value;
		sval -> next = help -> next;
		if (help -> next) help -> next -> prev = sval;
		DELETESVAL (help);
	    }
	    if (sval -> next && sval -> prev && sval -> prev -> value == sval -> value) {
		/* Equal previous value; Delete one */
		help = sval;
		sval = sval -> prev;
		sval -> next = help -> next;
		help -> next -> prev = sval;
		DELETESVAL (help);
	    }
	}
	else
	    sval -> time = sval -> time * factor;
    }

    Timescaling = newTimescaling;
}

void changeSimEndtime (double newSimEndtimeS)
{
    struct signal *sig;

    newEndtime = newSimEndtimeS / Timescaling;
    simperiod = newEndtime;

    for (sig = Begin_signal; sig; sig = sig -> next) {
	if (sig -> no_edit || sig -> endless) {
	    delSigval (sig -> begin_value, sig -> end_value);
	    sig -> begin_value = NULL;
	    sig -> end_value = NULL;
	    curr_time = 0;
	    /* simperiod has been set above */
	    (void) addSgnPart (sig, sig -> expr, 1);
	}
        adjustSgnPart (sig);
    }

    Endtime = newEndtime;
    SimEndtime = Endtime;

    startTime = Begintime;
    stopTime = Endtime;
}

struct signal *existSignal (char *name)
{
    struct signal *sig;

    sig = Begin_signal;
    while (sig && !strsame (sig -> name, name)) sig = sig -> next;
    return (sig);
}

void windowMessage (char *s, Grid x)
{
    Grid x_mess;
    int len;

    /* prints a message on the canvas */

    if (windowListFn[0])
        clear ();
    else
        XClearArea (display, window, 0, 0, axisStart + axisLength, canvasTop / 2 + fontHeight, False);

    if (s) {
	len = strlen (s);
	if (x > 0) {
	    x_mess = x - (len * fontWidth) / 2;
	    if (x_mess > axisStart + axisLength - len * fontWidth)
	        x_mess = axisStart + axisLength - len * fontWidth;
	    if (x_mess < axisStart)
		x_mess = axisStart;
	}
	else
	    x_mess = axisStart;
	XDrawString (display, window, gc, x_mess, (canvasTop + fontHeight) / 2, s, len);
	strcpy (windowMesBuf, s);
	if (x > 0)
	    windowMesX = x_mess;
	else
	    windowMesX = -1;
    }
    else {
	windowMesBuf[0] = '\0';
    }
    windowListFn[0] = '\0';

    if (x == -2) XFlush (display);
}

void windowList (char *fn)
{
    char buf[256];
    int c, i;
    FILE *fp;

    strcpy (windowListFn, fn);
    fp = fopen (windowListFn, "r");

    clear ();

    if (fp) {
	Grid y = (canvasTop + fontHeight) / 2;
	while ((c = getc (fp)) != EOF && y < canvasHeight) {
	    ungetc (c, fp);
	    i = 0;
	    while ((c = getc (fp)) != '\n' && c != EOF && i < 256) buf[i++] = c;
	    buf[i] = '\0';

	    XDrawString (display, window, gc, axisStart, y, buf, i);
	    y += 1.8 * fontHeight;
	}
	fclose (fp);
        windowMesBuf[0] = '\0';
    }
    else {
	sprintf (buf, "Cannot open %s", fn);
	windowMessage (buf, -1);
    }
}
