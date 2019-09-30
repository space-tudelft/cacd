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
#include "src/libX2ps/X2pslib.h"

#define B_GRID 8
#define S_GRID 4

extern Arg args[];
extern Colormap colormap;
extern Display *display;
extern Display *pdisplay;
extern Widget HorBar, VerBar;
extern Widget canvas;

extern int doSlider;
extern int rdHandler;
extern char **SV;

extern struct sig_value *storedVals;

static int  determWindow (char modif, Grid x1, Grid y1, Grid x2, Grid y2);
static void drawDottedLine (Grid x1, Grid y1, Grid x2, Grid y2);
static void drawInit (void);
static void drawLineSignal (Grid x1, Grid y1, Grid x2, Grid y2, int flag);
static void drawSignal (struct signal *sig, int index, char type, int flag, int inc_only);
static void drawSignalName (char *name, Grid y);
static void drawYScale (Grid yb, int drawDL);
static void findValSig (struct signal *sig, Grid t, char type, double *fval, char *pval);
static void findVoltSig (struct signal *sig, Grid x, Grid y, double *pv, char *s);
static void set_max_sliders (void);

#define MINPIXTIMESTEP  50
#define MINPIXGRID       5
#define MINPIXVOLTSTEP  15
#define MINPIXVOLTSPACE 25 /* below this value, only min-max levels are drawn */
#define MINPIXLEVELSPACE 8 /* below this value, no levels are drawn at all */

#define MAXSHOWYVALS    50 /* maximum nr. of signals for which y val can
							       be displayed */
#define SIGLEN 11 /* maximum length of a signal name */

static XGCValues values;
static GC gc, gc1, gc2;
GC gc3;
static Window window;

static int clip_mask = 0;
static int doViewValues = 0;

static int fontWidth  = 6; /* font 6x13 */
static int fontHeight = 8;

static double grid_time;
static double grid_volt;
static double t_outunit;
static double v_outunit;
static double Voltunit;
static double t_unit;
static double v_unit;
static double tprec_hlp;

Grid canvasHeight;
Grid canvasWidth;
Grid canvasTop;
static Grid canvasBottom;
static Grid canvasLeft;
Grid drawP_y;
static Grid sigDelta;
static Grid sigSpace;
static Grid axisLength;
Grid axisStart;
Grid axisStop;
Grid axisY;
static Grid oY;

char tprec_str[16];
char tSprec_str[16];
char vprec_str[16];
char vSprec_str[16];

struct signal *Top_signal    = NULL;
struct signal *Bottom_signal = NULL;
int Curr_nr_signals = 0;

simtime_t startTime;
simtime_t stopTime;

int curr_umin;
int curr_umax;

struct sig_value *Comb_vals_U[MAXSHOWYVALS + 1];
struct sig_value *Comb_vals_L[MAXSHOWYVALS + 1];

extern Pixel std_red;
Pixel analogColor1;
Pixel analogColor2;
Pixel analogColor3;
Pixel analogColor4;
Pixel analogColor5;
Pixel analogColor6;
Pixel backgrColor;
Pixel normalColor;
Pixel logicColor;
Pixel logicxColor;
Pixel valuesColor;

int showVoltScale;
int showYValues;
int showYValuesPos;
int showYValuesOn = 0;

#define MBLEN 511
static char windowMesBuf[MBLEN+1];
static char windowListFn[128];
static Grid windowMesX;

void calc_canvas ()
{
    canvasTop    = 4 * fontHeight;
    canvasBottom = 6 * fontHeight;
    canvasLeft   = 2 * fontWidth;
    sigDelta = canvasHeight - canvasTop - canvasBottom;
}

void drawStartup ()
{
    XrmDatabase db;
    XrmValue rmval;
    char *str_type = 0;
    XColor screenColor;
    XColor exactColor;
    Pixel pointerColor;
    Dimension dw, dh;
    int n = 0;

    XtSetArg (args[n], XmNwidth, &dw); n++;
    XtSetArg (args[n], XmNheight, &dh); n++;
    XtSetArg (args[n], XmNbackground, &backgrColor); n++;
    XtSetArg (args[n], XmNforeground, &normalColor); n++;
    XtGetValues (canvas, args, n);
    canvasWidth  = dw;
    canvasHeight = dh;

    window = XtWindow (canvas);

    db = XtDatabase (display);

#define getResource(A,B) (XrmGetResource (db, A, B, &str_type, &rmval) == True && \
	XAllocNamedColor (display, colormap, rmval.addr, &screenColor, &exactColor))

    logicColor        = getResource ("simeye.high_lowColor", "Simeye.Foreground") ? screenColor.pixel : normalColor;
    values.foreground = getResource ("simeye.freestateColor","Simeye.Foreground") ? screenColor.pixel : logicColor;
    logicxColor       = getResource ("simeye.undefinedColor","Simeye.Foreground") ? screenColor.pixel : logicColor;
    valuesColor       = getResource ("simeye.valuesColor"  , ""                 ) ? screenColor.pixel : std_red;

    if (getResource ("simeye.voltsColor1", "Simeye.Foreground"))
	analogColor1 = screenColor.pixel;
    else if (getResource ("simeye.voltsColor", "Simeye.Foreground"))
	analogColor1 = screenColor.pixel;
    else
	analogColor1 = normalColor;

    analogColor2 = getResource ("simeye.voltsColor2", "Simeye.Foreground") ? screenColor.pixel : normalColor;
    analogColor3 = getResource ("simeye.voltsColor3", "Simeye.Foreground") ? screenColor.pixel : normalColor;
    analogColor4 = getResource ("simeye.voltsColor4", "Simeye.Foreground") ? screenColor.pixel : normalColor;
    analogColor5 = getResource ("simeye.voltsColor5", "Simeye.Foreground") ? screenColor.pixel : normalColor;
    analogColor6 = getResource ("simeye.voltsColor6", "Simeye.Foreground") ? screenColor.pixel : normalColor;
    pointerColor = getResource ("simeye.pointerColor","Simeye.Foreground") ? screenColor.pixel : normalColor;
    pointerColor ^= backgrColor;

    gc1 = XCreateGC (display, window, 0, NULL);

    values.line_style = LineOnOffDash;
    gc2 = XCreateGC (display, window, GCForeground | GCLineStyle, &values);

    values.foreground = pointerColor;
    values.function   = GXxor;
    gc3 = XCreateGC (display, window, GCForeground | GCFunction, &values);

    gc = gc1;
    XSetBackground (display, gc, backgrColor);
    XSetForeground (display, gc, normalColor);

    /* among other things for windowMessage: */

    calc_canvas ();

    axisStart = 2 * canvasLeft;   /* for windowMessage */
    axisLength = canvasWidth - axisStart - 2 * fontWidth;
    axisStop = axisStart + axisLength;

    windowMesBuf[0] = '\0';
    windowListFn[0] = '\0';
}

static int widthline;

void setLineAttr ()
{
    widthline = 1;
    XSetLineAttributes (display, gc1, 1, LineSolid, CapButt, JoinBevel);
    XSetLineAttributes (display, gc2, 1, LineOnOffDash, CapButt, JoinBevel);
}

void resetLineAttr ()
{
    widthline = 0;
    XSetLineAttributes (display, gc1, 0, LineSolid, CapButt, JoinBevel);
    XSetLineAttributes (display, gc2, 0, LineOnOffDash, CapButt, JoinBevel);
}

void set_clip_area (XRectangle R[], int nr)
{
    XSetClipRectangles (display, gc1, 0, 0, R, nr, Unsorted);
    XSetClipRectangles (display, gc2, 0, 0, R, nr, Unsorted);
    XSetClipRectangles (display, gc3, 0, 0, R, nr, Unsorted);
    clip_mask = 1;
}

void unset_clip_area ()
{
    if (clip_mask) {
	clip_mask = 0;
	XSetClipMask (display, gc1, None);
	XSetClipMask (display, gc2, None);
	XSetClipMask (display, gc3, None);
    }
}

void clear ()
{
    if (!clip_mask) XpClearWindow (pdisplay, window);
}

void redrawMessage ()
{
    if (windowMesBuf[0]) windowMessage (windowMesBuf, windowMesX);
}

void draw (char modif, Grid x1, Grid y1, Grid x2, Grid y2)
{
    struct signal *sig;
    int inc_only;

    inc_only = (modif == 't') ? 1 : 0;

    if (rdHandler) {
	if (windowListFn[0]) {
	    windowList (NULL);   /* redraw */
	    goto ret;
	}
	if (!Begin_signal) goto ret;
    }

    if (modif <= 1 || modif == 'I' || modif == 'i' || modif == 'o') {
	if (modif != 1) windowMesBuf[0] = '\0';
	clear ();
    }

    windowListFn[0] = '\0';

    setPrintButton ();

    if (!Begin_signal) {
	if (modif == 'f') set_max_sliders ();
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

    if (modif > 1) {
	if (modif != 'a' && modif != 'b' && modif != 't') {
	    if (!determWindow (modif, x1, y1, x2, y2)) return;
	}
	else if (!doSlider)
	    doViewValues = 0;

	if (modif == 'f' || modif == 'a' || modif == 'm') clear ();
    }
    else if (!rdHandler && !doSlider)
	doViewValues = 0;

    if (!inc_only) drawInit ();

    if (currLogic) {
	XSetForeground (display, gc, logicColor);
	for (sig = Top_signal; sig; sig = sig -> next) {
	    drawSignal (sig, sig -> i, 0, 1, inc_only);
	    if (sig -> layover2)
		drawSignal (sig -> layover2, sig -> i, 0, -1, inc_only);
	    if (sig == Bottom_signal) break;
	}
    }
    else if (Spice) {
	XSetForeground (display, gc, analogColor1);
	for (sig = Top_signal; sig; sig = sig -> next) {
	    drawSignal (sig, sig -> i, 'u', 1, inc_only);
	    if (sig -> layover2) {
	        XSetForeground (display, gc, analogColor2);
		drawSignal (sig -> layover2, sig -> i, 'u', 2, inc_only);
            }
	    if (sig -> layover3) {
	        XSetForeground (display, gc, analogColor3);
		drawSignal (sig -> layover3, sig -> i, 'u', 3, inc_only);
            }
	    if (sig -> layover4) {
	        XSetForeground (display, gc, analogColor4);
		drawSignal (sig -> layover4, sig -> i, 'u', 4, inc_only);
            }
	    if (sig -> layover5) {
	        XSetForeground (display, gc, analogColor5);
		drawSignal (sig -> layover5, sig -> i, 'u', 5, inc_only);
            }
	    if (sig -> layover6) {
	        XSetForeground (display, gc, analogColor6);
		drawSignal (sig -> layover6, sig -> i, 'u', 6, inc_only);
            }
	    if (sig == Bottom_signal) break;
	}
    }
    else {
	XSetForeground (display, gc, analogColor1);
	for (sig = Top_signal; sig; sig = sig -> next) {
	    drawSignal (sig, sig -> i, 'l', 1, inc_only);
	    if (sig -> layover2) {
	        XSetForeground (display, gc, analogColor2);
		drawSignal (sig -> layover2, sig -> i, 'l', 2, inc_only);
            }
	    drawSignal (sig, sig -> i, 'u', 1, inc_only);
	    if (sig -> layover2) {
	        XSetForeground (display, gc, analogColor2);
		drawSignal (sig -> layover2, sig -> i, 'u', 2, inc_only);
            }
	    if (sig == Bottom_signal) break;
	}
    }

    if (doViewValues) viewValues (0, 0, 0);

    XSetForeground (display, gc, normalColor);

    if (!(rdHandler && windowMesBuf[0]) &&
	modif != 's' && modif != 't' && modif != 'b') windowMesBuf[0] = '\0';

    if (rdHandler) {
        redrawEvent ();
ret:
	redrawMessage ();
	unset_clip_area ();
	rdHandler = 0;
    }
    XSync (display, False);
}

static void viewSignalValues (struct sig_value *sval, Grid yb)
{
    Grid x, y;
    double d;
    int z = 1;

    if (widthline) ++z;

    while (sval && sval -> time < startTime) sval = sval -> next;
    while (sval && sval -> time <= stopTime) {
	x = axisStart + (sval -> time - startTime) * grid_time;
	d = yb - grid_volt * sval -> value;
	if (d >= 0 && d < canvasHeight) {
	    y = d;
	    XpDrawLine (pdisplay, window, gc, x-z, y, x+z, y);
	    XpDrawLine (pdisplay, window, gc, x, y-z, x, y+z);
	}
	sval = sval -> next;
    }
}

void viewValues (Widget w, caddr_t client_data, caddr_t call_data)
{
    struct signal *sig;
    Grid y, yb;

    if (currLogic || !Begin_signal) return;

    doViewValues = 1;

    XSetForeground (display, gc, valuesColor);

    y = oY + canvasTop - sigSpace / 8;

    for (sig = Top_signal; sig; sig = sig -> next) {
	yb = y + sigDelta * (sig -> i) / Curr_nr_signals;
	if (!Spice) viewSignalValues (sig -> begin_value_L, yb);
	viewSignalValues (sig -> begin_value_U, yb);
	if (sig == Bottom_signal) break;
    }

    XSetForeground (display, gc, normalColor);
}

static int determWindow (char modif, Grid x1, Grid y1, Grid x2, Grid y2)
{
    Grid tmp;
    int i, new_nr_signals;
    simtime_t newStartTime, newStopTime, deltaTime;
    double d;
    int new_umin, new_umax;

    deltaTime = stopTime - startTime;

    if (modif == 'f' || modif == 's' || modif == 'm') {
			    /* first draw ('f', 's') or full window ('m') */

	doViewValues = 0;

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

	if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
	if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }

	if (Curr_nr_signals > 1) {
	    new_nr_signals = Curr_nr_signals;

	    i = Curr_nr_signals * (y1 - canvasTop) / sigDelta;
	    while (i-- > 0 && new_nr_signals > 1) {
		Top_signal = Top_signal -> next;
		new_nr_signals--;
	    }

	    i = Curr_nr_signals * (canvasHeight - canvasBottom - y2) / sigDelta;
	    while (i-- > 0 && new_nr_signals > 1) {
		Bottom_signal = Bottom_signal -> prev;
		new_nr_signals--;
	    }

	    Curr_nr_signals = new_nr_signals;
	}
	else if ((modif == 'I' || doDetailZoom) && !currLogic) {

            /* compute in double to prevent integer overflow */

	    new_umax = curr_umax - (double)(curr_umax - curr_umin)
			* (y1 - canvasTop - sigSpace / 8)
			/ (sigDelta - sigSpace / 4);

	    new_umin = curr_umin + (double)(curr_umax - curr_umin)
			* (canvasHeight - canvasBottom - y2 - sigSpace / 8)
			/ (sigDelta - sigSpace / 4);

	    if (new_umax < curr_umax) curr_umax = new_umax;
	    if (new_umin > curr_umin) curr_umin = new_umin;
	    if (curr_umin >= curr_umax) curr_umin = curr_umax - 1;
	    if (curr_umin < Global_umin) curr_umin = Global_umin;
	}

	newStartTime = startTime + (double)((x1 - axisStart) / (double)axisLength) * deltaTime;
	newStopTime  = startTime + (double)((x2 - axisStart) / (double)axisLength) * deltaTime;

	if (newStartTime >= startTime) startTime = newStartTime;
	if (newStopTime  <= stopTime ) stopTime  = newStopTime;
	if (stopTime     <= startTime) stopTime  = startTime + 1;
    }
    else if (modif == 'o') {         /* zoom out */

	doViewValues = 0;

	if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
	if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }

	if ((d = y2 - y1) == 0) d = 1;

	if (Curr_nr_signals == 1 && !currLogic
	    && (curr_umin > Global_umin || curr_umax < Global_umax)) {

            /* compute in double to prevent integer overflow */

	    new_umax = curr_umax + (double)(curr_umax - curr_umin) *
			(y1 - canvasTop - sigSpace / 8) / d;

	    new_umin = curr_umin - (double)(curr_umax - curr_umin) *
			(canvasHeight - canvasBottom - y2 - sigSpace / 8) / d;

	    if (new_umax > curr_umax) curr_umax = new_umax > Global_umax ? Global_umax : new_umax;
	    if (new_umin < curr_umin) curr_umin = new_umin < Global_umin ? Global_umin : new_umin;
	}
	else {
	    curr_umin = Global_umin;
	    curr_umax = Global_umax;
	}

	if (!(Curr_nr_signals == 1 && !currLogic
	    && (curr_umin > Global_umin || curr_umax < Global_umax))) {

	    new_nr_signals = Curr_nr_signals;

	    i = Curr_nr_signals * (y1 - canvasTop) / d;
	    while (i-- > 0 && Top_signal != Begin_signal) {
		Top_signal = Top_signal -> prev;
		new_nr_signals++;
	    }

	    i = Curr_nr_signals * (canvasHeight - canvasBottom - y2) / d;
	    while (i-- > 0 && Bottom_signal != End_signal) {
		Bottom_signal = Bottom_signal -> next;
		new_nr_signals++;
	    }

	    Curr_nr_signals = new_nr_signals;
	}

	if ((d = x2 - x1) == 0) d = 1;

	newStartTime = stopTime - (double)((x2 - axisStart) / d) * deltaTime;
	newStopTime  = startTime + (double)((axisStop - x1) / d) * deltaTime;

	if (newStartTime < Begintime) startTime = Begintime;
	else if (newStartTime < startTime) startTime = newStartTime;

	if (newStopTime > Endtime && !editing) /* <== Notice !! */
	    stopTime = Endtime;
	else if (newStopTime > stopTime)
	    stopTime = newStopTime;

	if (stopTime <= startTime) stopTime = startTime + 1;
    }
    else
	doViewValues = 0;

    if (curr_umin >= curr_umax) curr_umax = curr_umin + 1;

    return (1);
}

static void drawInit ()
{
    int tdivider;
    int tprecision;      /* for pointer info */
    int vprecision;
    char t_str[20];
    char v_str[20];
    Grid canvasNames;
    Grid canvasVolts;
    Grid yb, x, y;
    int i, maxlen;
    double help;
    int div2_5;
    struct signal *sig;
    simtime_t deltaTime;

    /* drawing of time axis */

#ifdef OLD
    maxlen = 0;
    sig = Top_signal;
    for (i = 1; i <= Curr_nr_signals; ++i) {
	if (strlen (sig -> name) > maxlen) maxlen = strlen (sig -> name);
	sig = sig -> next;
    }
    if (maxlen < 5) maxlen = 5;
    else if (maxlen > SIGLEN) maxlen = SIGLEN;
#else
    maxlen = SIGLEN;
#endif

    canvasNames = (maxlen + 1) * fontWidth;

    sigSpace = sigDelta / (double)Curr_nr_signals;
    if (sigSpace < 1) sigSpace = 1;

    if (!currLogic
	&& sigSpace * 0.75 >= MINPIXVOLTSPACE
	&& sigSpace * 0.75 >= MINPIXLEVELSPACE)
	showVoltScale = 1;
    else
        showVoltScale = 0;

    if ((currLogic || showVoltScale)
        && sigSpace * 0.75 >= MINPIXLEVELSPACE
        && Curr_nr_signals <= MAXSHOWYVALS)
        showYValuesPos = 1;
    else
        showYValuesPos = 0;

    canvasVolts = 10 * fontWidth; /* just a guess of what is required */

    axisY = canvasHeight - (3 * canvasBottom) / 4;
    axisStart = canvasLeft + canvasNames;
    axisLength = canvasWidth - axisStart - canvasVolts;
    axisStop = axisStart + axisLength;

    if ((deltaTime = stopTime - startTime) < 1) {
	deltaTime = 1;
	stopTime = startTime + 1;
    }
    grid_time = (double) axisLength / (double) deltaTime;

    XpDrawLine (pdisplay, window, gc, axisStart, axisY, axisStop, axisY);

    t_unit = convdec (Timescaling * deltaTime);
    if (t_unit * grid_time / Timescaling < MINPIXTIMESTEP)
	t_unit *= 10;

    while (t_unit * grid_time / Timescaling >= MINPIXTIMESTEP * 10) {
	t_unit /= 10;
    }

    if (t_unit / Timescaling >= 10) {
	if (t_unit * grid_time / Timescaling >= MINPIXTIMESTEP * 5) {
	    t_unit /= 5;
	    if (t_unit / Timescaling >= 10)
		tdivider = 4;
	    else
		tdivider = 2;
	}
	else if (t_unit * grid_time / Timescaling >= MINPIXTIMESTEP * 2) {
	    t_unit /= 2;
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
    help = t_outunit / t_unit;
    while (help > 1) { help /= 10; ++tprecision; }
    tprec_hlp = 1;
    while (tprecision) { tprec_hlp *= 10; --tprecision; }
    sprintf (t_str, "%.0le", help = t_outunit / tprec_hlp);
    while (help > 1) { help /= 10; ++tprecision; }
    sprintf (tprec_str, "%%+.%dlf", tprecision+1);
    strcpy (tSprec_str, "%.0lf");

    help = tprec_hlp * t_unit;

    *t_str = ' '; /* remove mantisse */

    y = axisY + (canvasHeight - axisY + fontHeight) / 2;
    XpDrawString (pdisplay, window, gc, canvasLeft + 2*fontWidth, y, t_str, 5);

    i = startTime * Timescaling / t_unit;
    if (i * t_unit < startTime * Timescaling) ++i;
    while (i * t_unit < stopTime * Timescaling + Timescaling / grid_time) {

	x = axisStart + (i * t_unit / Timescaling - startTime) * grid_time;
	drawDottedLine (x, axisY, x, canvasTop);

	XpDrawLine (pdisplay, window, gc, x, axisY, x, axisY + B_GRID);
	sprintf (t_str, tSprec_str, i * help / t_outunit);
	XpDrawString (pdisplay, window, gc, x, y, t_str, strlen (t_str));
	++i;
    }

    i = startTime * tdivider * Timescaling / t_unit;
    if (i * t_unit / tdivider < startTime * Timescaling) ++i;
    while (i * t_unit / tdivider < stopTime * Timescaling + Timescaling / grid_time) {

	x = axisStart + (i * t_unit / (Timescaling * tdivider) - startTime) * grid_time;
	XpDrawLine (pdisplay, window, gc, x, axisY, x, axisY + S_GRID);
	++i;
    }

    if (grid_time >= MINPIXGRID) {    /* display smallest grid */

	i = 0;
	while (i * grid_time <= axisLength) {

	    x = axisStart + i * grid_time;
	    XpDrawLine (pdisplay, window, gc, x, axisY, x, axisY - S_GRID);
	    ++i;
	}
    }


    /* drawing of names and voltage levels */


    grid_volt = (double) sigSpace * 0.75 / (curr_umax - curr_umin);
    if (Spice)
    while (grid_volt > 50) {
	grid_volt = (double) sigSpace * 0.75 / (++curr_umax - curr_umin);
    }

    oY = curr_umin * grid_volt;
    /* this value has to be added to the y value of a signal
       to fit within the space that is reserved for the signal */

    if (showVoltScale) {

        /* v_unit and v are in Volts ! */

	v_unit = convdec ((double)(curr_umax - curr_umin) * Voltscaling);
	if (v_unit * grid_volt / Voltscaling < MINPIXVOLTSTEP) v_unit *= 10;

	while (v_unit * grid_volt / Voltscaling >= MINPIXVOLTSTEP * 10) v_unit /= 10;

        div2_5 = 0;
	if (v_unit * grid_volt / Voltscaling >= MINPIXVOLTSTEP * 4) {
	    v_unit /= 4;
	    div2_5 = 1;
	}
	else if (v_unit * grid_volt / Voltscaling >= MINPIXVOLTSTEP * 5) {
	    v_unit /= 5;

	    /* Since maximum voltage is usually 5V we prefer division by 4 above division by 5 */
	}
	else if (v_unit * grid_volt / Voltscaling >= MINPIXVOLTSTEP * 2) {
	    v_unit /= 2;
	}

	v_outunit = convdec ((Max (Abs (curr_umin), Abs (curr_umax))) * Voltscaling);
	Voltunit = Voltscaling / v_outunit;

        /* (Voltscaling / grid_volt) == Volts per pixel */

	vprecision = 0;
	help = v_outunit / (Voltscaling / grid_volt);
	while (help > 1) { help /= 10; vprecision++; }
	if (Spice) vprecision += 2;
	sprintf (vprec_str, "%%+.%dlf", vprecision);

	vprecision = 0;
	help = v_outunit / v_unit;
	while (help > 1) { help /= 10; vprecision++; }
	if (div2_5) vprecision++;  /* ! */
	sprintf (vSprec_str, "%%+.%dlf", vprecision);

	sprintf (v_str, "%.0le", v_outunit);
	*v_str = ' '; /* remove mantisse */
	XpDrawString (pdisplay, window, gc, axisStop + fontWidth / 2, (canvasTop + fontHeight)/ 2, v_str, 5);
    }

    if (showYValuesPos && showYValues)
	showYValues = 1;
    else
	showYValues = 0;

    sig = Top_signal;
    for (i = 1; i <= Curr_nr_signals; ++i) {

        sig -> i = i;
	if (showYValuesPos) {
	    Comb_vals_L[i] = NULL;
	    Comb_vals_U[i] = NULL;
	}

	yb = canvasTop + sigDelta * i / Curr_nr_signals - sigSpace / 8;

	if ((curr_umax - curr_umin) * grid_volt < MINPIXLEVELSPACE) {

	    /* do not draw levels */

	}
	else if (showVoltScale) {

            drawYScale (yb, 1);
	}
	else {

            /* draw only the minimum and maximum level for each signal */

	    y = yb + oY - grid_volt * curr_umin;
	    drawDottedLine (axisStart, y, axisStop, y);

	    y = yb + oY - grid_volt * curr_umax;
	    drawDottedLine (axisStart, y, axisStop, y);
	}

        y = yb;
        if (Spice) XSetForeground (display, gc, analogColor1);
        drawSignalName (sig -> name, y);
        if (sig -> layover2) {
            y -= 3 * fontHeight / 2;
            if (Spice) XSetForeground (display, gc, analogColor2);
            drawSignalName (sig -> layover2 -> name, y);
        }
        if (sig -> layover3) {
            y -= 3 * fontHeight / 2;
	    if (Spice) XSetForeground (display, gc, analogColor3);
            drawSignalName (sig -> layover3 -> name, y);
        }
        if (sig -> layover4) {
            y -= 3 * fontHeight / 2;
	    if (Spice) XSetForeground (display, gc, analogColor4);
            drawSignalName (sig -> layover4 -> name, y);
        }
        if (sig -> layover5) {
            y -= 3 * fontHeight / 2;
	    if (Spice) XSetForeground (display, gc, analogColor5);
            drawSignalName (sig -> layover5 -> name, y);
        }
        if (sig -> layover6) {
            y -= 3 * fontHeight / 2;
	    if (Spice) XSetForeground (display, gc, analogColor6);
            drawSignalName (sig -> layover6 -> name, y);
        }
        if (Spice) XSetForeground (display, gc, normalColor);

	if (editing && sig -> endless)
	    XpDrawString (pdisplay, window, gc, axisStop + fontWidth, yb, "~", 1);

	sig = sig -> next;
    }

    XtSetArg (args[0], XmNminimum, Begintime);
    XtSetArg (args[1], XmNmaximum, stopTime > Endtime ? stopTime : Endtime);
    XtSetArg (args[2], XmNvalue, startTime);
    XtSetArg (args[3], XmNsliderSize, deltaTime);
    if ((i = deltaTime / 10) < 1) i = 1;
    XtSetArg (args[4], XmNincrement, i);
    if ((i = deltaTime -  i) < 1) i = 1;
    XtSetArg (args[5], XmNpageIncrement, i);
    XtSetValues (HorBar, args, 6);

    i = 0;
    for (sig = End_signal; sig; sig = sig -> prev) {
	if (sig == Bottom_signal) break;
	++i;
    }

    XtSetArg (args[0], XmNminimum, 0);
    XtSetArg (args[1], XmNmaximum, Nr_signals);
    XtSetArg (args[2], XmNvalue, i);
    XtSetArg (args[3], XmNsliderSize, Curr_nr_signals);
    if ((i = Curr_nr_signals - 1) < 1) i = 1;
    XtSetArg (args[4], XmNpageIncrement, i);
    XtSetValues (VerBar, args, 5);
}

static void drawSignalName (char *name, Grid y)
{
    char *s = name;
    char c = '*';
    int len = strlen (s);
    if (len > SIGLEN) {
        len = SIGLEN+1;
        c = s[len-1]; s[len-1] = '*';
    }
    XpDrawString (pdisplay, window, gc, canvasLeft, y, s, len);
    if (c != '*') s[len-1] = c;
}

static void set_max_sliders ()
{
    XtSetArg (args[0], XmNminimum, 0);
    XtSetArg (args[1], XmNmaximum, 1);
    XtSetArg (args[2], XmNvalue, 0);
    XtSetArg (args[3], XmNsliderSize, 1);
    XtSetValues (HorBar, args, 4);
    XtSetValues (VerBar, args, 4);
}

static void drawYScale (Grid yb, int drawDL)
{
    int first;
    Grid h;
    double v;
    char v_str[20];

    v = ((int)(curr_umin * Voltscaling / v_unit)) * v_unit;

    /* to obtain a multiple value of v_unit ! */

    if (v < curr_umin * Voltscaling) v += v_unit;

    first = 1;
    while (v <= curr_umax * Voltscaling + 0.001 * (curr_umax - curr_umin) * Voltscaling) {

	if (drawDL) {
	    h = yb + oY - grid_volt * v / Voltscaling;
	    drawDottedLine (axisStart, h, axisStop, h);
	}

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
	    h += yb + oY - grid_volt * v / Voltscaling;
	    XpDrawString (pdisplay, window, gc, axisStop + fontWidth / 2, h, v_str, strlen (v_str));
	}

	v += v_unit;
    }
}

double convdec (double f)
{
    double d = 1;

    if (f <= 0) return (f);

    if (f >= 1) {
	while (f > 10) { f /= 10; d *= 10; }
    }
    else {
	while (f <  1) { f *= 10; d /= 10; }
    }

    return (d);
}

static Grid xlatest, ylatest;

/* argument 'flag' = layover */
static void drawSignal (struct signal *sig, int index, char type, int flag, int inc_only)
{
    struct sig_value *sval;
    Grid xcurr, xprev, ycurr, yprev, yb, xl, gv2;
    int  stringValue = sig -> stringValue;

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

    if (flag < 0) xlatest = ylatest = -100; /* init for stipple line */

    xl = axisStart;
    yb = oY + canvasTop + sigDelta * index / Curr_nr_signals - sigSpace / 8;

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

	if (showYValuesPos && flag == 1) {
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
		drawLineSignal (xprev, yb, xcurr, yb, flag);
		drawLineSignal (xprev, yb - gv2, xcurr, yb - gv2, flag);
		if (ycurr != yprev)
		    drawLineSignal (xcurr, yb, xcurr, yb - gv2, flag);
	    }
	    else {
		ycurr = yb - grid_volt * sval -> value;
	    if (sval -> prev -> value >= 0) {
		if (sval -> prev -> value == 1) {
		    XSetForeground (display, gc, logicxColor);
		    drawLineSignal (xprev, yprev, xcurr, yprev, flag);
		    XSetForeground (display, gc, logicColor);
		}
		else
		    drawLineSignal (xprev, yprev, xcurr, yprev, flag);
		if (ycurr != yprev && sval -> value >= 0)
		    drawLineSignal (xcurr, yprev, xcurr, ycurr, flag);
	    }
            else {           /* Free state */
		gc = gc2;
		yprev -= gv2;
		drawLineSignal (xprev, yprev, xcurr, yprev, flag);
		gc = gc1;
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
		drawLineSignal (xprev, yb, xcurr, yb, flag);
		drawLineSignal (xprev, yb - gv2, xcurr, yb - gv2, flag);
	    }
	    else if (sval -> prev -> value >= 0) {
		if (sval -> prev -> value == 1) {
		    XSetForeground (display, gc, logicxColor);
		    drawLineSignal (xprev, yprev, xcurr, yprev, flag);
		    XSetForeground (display, gc, logicColor);
		}
		else
		    drawLineSignal (xprev, yprev, xcurr, yprev, flag);
	    }
            else {           /* Free state */
		gc = gc2;
		yprev -= gv2;
		drawLineSignal (xprev, yprev, xcurr, yprev, flag);
		gc = gc1;
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

static void drawLineSignal (Grid x1, Grid y1, Grid x2, Grid y2, int flag)
{
    double ya, yb, d1, d2, d3;
    Grid x, y, sep = 3;

    if (x2 < x1) {
	fprintf (stderr, "He x2 < x1\n");
	die (1);                     /* algorithm does not handle x2 < x1 */
    }

    if (!currLogic) {
	/* perform clipping (in case of detail zoom in) */

	x = canvasTop + sigSpace / 8;
	y = canvasHeight - canvasBottom - sigSpace / 8;

	if (y1 < x) {
	    if (y2 < x) return;

	    x1 = x1 + (x2 - x1) * (x - y1) / (y2 - y1);
	    y1 = x;
	}
	else if (y1 > y) {
	    if (y2 > y) return;

	    x1 = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
	    y1 = y;
	}

	if (y2 < x) {
	    x2 = x1 + (x2 - x1) * (x - y1) / (y2 - y1);
	    y2 = x;
	}
	else if (y2 > y) {
	    x2 = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
	    y2 = y;
	}
    }

    if (flag < 0) {

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
		XpDrawLine (pdisplay, window, gc, x, y, x+widthline, y);
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
		    XpDrawLine (pdisplay, window, gc, x, y, x+widthline, y);
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
		    XpDrawLine (pdisplay, window, gc, x, y, x+widthline, y);
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
	XpDrawLine (pdisplay, window, gc, x1, y1, x2, y2);
    }
}

static void drawDottedLine (Grid x1, Grid y1, Grid x2, Grid y2)
{
    Grid x, y;
    Grid sep = 6;

    x = x1;
    y = y1;

    /* Note: Only orthogonal dotted lines */

    if (x1 == x2) { /* vertical */
	if (y2 > y1) {
	    y = y + sep;
	    while (y <= y2) {
		XpDrawLine (pdisplay, window, gc, x, y, x, y+widthline);
		y = y + sep;
	    }
	}
	else {
	    y = y - sep;
	    while (y >= y2) {
		XpDrawLine (pdisplay, window, gc, x, y, x, y-widthline);
		y = y - sep;
	    }
	}
    }
    else if (y1 == y2) { /* horizontal */
	if (x2 > x1) {
	    x = x + sep;
	    while (x <= x2) {
		XpDrawLine (pdisplay, window, gc, x, y, x+widthline, y);
		x = x + sep;
	    }
	}
	else {
	    x = x - sep;
	    while (x >= x2) {
		XpDrawLine (pdisplay, window, gc, x, y, x-widthline, y);
		x = x - sep;
	    }
	}
    }
}

static int drawPointerMode = 0;

void drawPointerInfo (int initial, Grid x, Grid y, int flag, int showLogic, int moving)
/* initial:   if (1) start; if (-1) end */
/* flag:      if (1) then toggle memory ON or OFF */
/* showLogic: if (!= NOstate) then show logic state */
/* moving:    if (1) display info at position x */
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
    static char smem = 'U';
    double v, dt;
    int len, store;
    struct signal *sig;

    if (initial >= 0) {

	store = 0;
	if (flag) {
	    if (memory) memory = 0;
	    else {
		store = 1;
		memory = 1;
	    }
	}

	if (initial == 1) {
	    sig = NULL;
	    sprintf (buf, "- - - -");
	}
	else {
	    sig = findSignal (y, 0);

	if (x < axisStart)
	    sprintf (buf, "> > > >");
	else if (x > axisStop)
	    sprintf (buf, "< < < <");
	else {
	    dt = (double) startTime + (x - axisStart) / grid_time;
	    dt *= tprec_hlp * Timescaling / t_outunit;

	    sprintf (buf1, tprec_str, dt);
	    *buf1 = ' ';

            if (store) tmem = dt;

	    if (memory) {
		sprintf (buf1a, tprec_str, tmem);
		*buf1a = ' ';
		dt -= tmem;
		sprintf (buf1b, tprec_str, dt);
		if (*buf1b == '+') *buf1b = dt >= 0 ? ' ' : '-';
	    }

	    if (!showVoltScale) {
		if (currLogic && showYValuesPos) {
		    drawPointerMode = 1;
		    findValSig (sig, x, 'u', &v, buf2);
		    drawPointerMode = 0;
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
		findVoltSig (sig, x, y, &v, buf2);

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
	    }

	    if (showLogic != NOstate) { /* CHANGE-command */
		if (!memory || store) {
		    switch (showLogic) {
			case H_state: smem = 'I'; break;
			case X_state: smem = 'X'; break;
			case L_state: smem = 'O'; break;
			case F_state: smem = 'F'; break;
			default:      smem = 'U';
		    }
		}
		len = strlen (buf);
		sprintf (&buf[len], " '%c'", smem);
	    }
	}
	}
	windowMessage (buf, moving ? x : 0);
	if (sig) {
	    register char *s;
	    len = 0;
	    s = sig -> name;
	    while (*s) {
		buf[len++] = *s++;
		if (len == SIGLEN) break;
	    }
	    buf[len++] = ':';
	    XDrawString (display, window, gc, canvasLeft, (canvasTop + fontHeight) / 2, buf, len);
	}
    }
    else {
	memory = 0;
        windowMessage (NULL, -1);
    }
}

void drawValueInfo (int initial, Grid xt)
/* initial: 1=begin, -1=end, 0=draw */
{
    register int i;
    char pval[16];
    double v;
    struct signal *sig;
    Grid top, w, x, y, yb;

    x = axisStop + fontWidth / 2;
    w = canvasWidth - x;

    if (initial == 1) {
	if (showVoltScale)
	    XClearArea (display, window, /* x, y, w, h */
		x, canvasTop, w, sigDelta, False);
	showYValuesOn = 1;
	return;
    }

    if (initial == -1) {
	XClearArea (display, window, /* x, y, w, h */
	    x, canvasTop, w, sigDelta, False);
	showYValuesOn = 0;

	top = canvasTop - sigSpace / 8;

	if (editing) {
	    sig = Top_signal;
	    x += fontWidth / 2;
	    for (i = 1; i <= Curr_nr_signals; ++i) {
		if (sig -> endless) {
		    yb = top + sigDelta * i / Curr_nr_signals;
		    XDrawString (display, window, gc, x, yb, "~", 1);
		}
		sig = sig -> next;
	    }
	}
	else if (showVoltScale) {
	    for (i = 1; i <= Curr_nr_signals; ++i) {
		yb = top + sigDelta * i / Curr_nr_signals;
		drawYScale (yb, 0);
	    }
	}
	return;
    }

    if (xt < axisStart) xt = axisStart;
    else if (xt > axisStop) xt = axisStop;
#ifdef OLD
    y = 3 * sigSpace / 4 - fontHeight;
#else
    y = 3 * fontHeight / 2;
#endif

    top = canvasTop - sigSpace / 8;
    sig = Top_signal;

    for (i = 1; i <= Curr_nr_signals; ++i) {

	yb = top + sigDelta * i / Curr_nr_signals;

	if (currLogic || Spice) {
	    findValSig (sig, xt, 'u', &v, pval);
	    XDrawImageString (display, window, gc, x, yb, pval, strlen (pval));
	}
	else {
	    findValSig (sig, xt, 'l', &v, pval);
	    XDrawImageString (display, window, gc, x, yb, pval, strlen (pval));
	    findValSig (sig, xt, 'u', &v, pval);
	    XDrawImageString (display, window, gc, x, yb-y, pval, strlen (pval));
	}
	sig = sig -> next;
    }
}

static void findValSig (struct signal *sig, Grid t, char type, double *fval, char *pval)
{
    register struct sig_value *sval;
    Grid x, z;

    sval = (type == 'l') ? Comb_vals_L[sig -> i] : Comb_vals_U[sig -> i];
    strcpy (pval, " 'U'");
    if (!sval) return;

    t = t - axisStart + startTime * grid_time;
    z = sval -> time * grid_time;
    while (z <= t && sval -> next) {
	sval = sval -> next;
	z = sval -> time * grid_time;
    }
    x = z;
    while (x >  t && sval -> prev) {
	sval = sval -> prev;
	x = sval -> time * grid_time;
    }
    if ((x > t && !sval -> prev) || (x < t && !sval -> next)) goto ret;

    if (currLogic) {
	if (sig -> stringValue) {
	    if (drawPointerMode)
		sprintf (pval, " %s", SV[sval -> value]);
	    else
		sprintf (pval, " %-9s", SV[sval -> value]);
	}
	else if (sval -> value == H_state) pval[2] = 'I';
	else if (sval -> value == X_state) pval[2] = 'X';
	else if (sval -> value == L_state) pval[2] = 'O';
	else if (sval -> value == F_state) pval[2] = 'F';
    }
    else {
	int dv;
	double v;

	if (x < t && (dv = sval -> next -> value - sval -> value) != 0)
	    v = (sval -> value + ((t - x) / (double)(z - x)) * dv) * Voltunit;
	else
	    v = sval -> value * Voltunit;

	sprintf (pval, vprec_str, v);
	if (*pval == '+') *pval = v >= 0 ? ' ' : '-';
	*fval = v;
    }
ret:
    if (type == 'l')
	Comb_vals_L[sig -> i] = sval;
    else
	Comb_vals_U[sig -> i] = sval;
}

struct signal *findSignal (Grid y, int extend)
/* extend: signal below bottom signal can also be selected */
{
    struct signal *sig;
    int i;

    if (extend) y = y + sigSpace / 2;

    if (y >= canvasHeight - canvasBottom) {
	if (!extend) return (Bottom_signal);
	return (Bottom_signal -> next);
    }

    i = Curr_nr_signals * (y - canvasTop) / sigDelta;
    if (i <= 0) return (Top_signal);

    sig = Top_signal;
    while (i-- && sig != Bottom_signal) sig = sig -> next;

    return (sig);
}

static void findVoltSig (struct signal *sig, Grid x, Grid y, double *pv, char *s)
{
    double d, z;

    findValSig (sig, x, 'u', pv, s);
    z = oY + canvasTop + sigDelta * sig -> i / Curr_nr_signals - sigSpace / 8;
    d = z - grid_volt * *pv / Voltunit;
    if (!Spice) {
	double d1, d2, dpv;
	char s2[32];

	if ((d1 = y - d) < 0) d1 = -d1;
	findValSig (sig, x, 'l', &dpv, s2);
	z = z - grid_volt * dpv / Voltunit;
	if ((d2 = y - z) < 0) d2 = -d2;
	if (d2 < d1) {
	    d = z;
	    *pv = dpv;
	    strcpy (s, s2);
	}
    }
    drawP_y = d <= 0 ? 0 : (d < canvasHeight ? d : canvasHeight - 1);
}

int findState (Grid y) /* INPUT mode, CHANGE-command */
{
    Grid yb;
    int i, v;

    if (Curr_nr_signals == 0) return (X_state);

    i = Curr_nr_signals * (y - canvasTop) / sigDelta;

    if (i <= 0) i = 1;
    else if (++i > Curr_nr_signals) i = Curr_nr_signals;

    yb = canvasTop + sigDelta * i / Curr_nr_signals - sigSpace / 8;

    v = (yb + oY - y) / grid_volt + 0.5;

	 if (v < L_state) v = L_state;
    else if (v > H_state) v = H_state;

    return (v);
}

simtime_t findTime (Grid x)
{
    simtime_t t;

    if (x <= axisStart) return (startTime);
    if (x > axisStop) x = axisStop;

    t = (double) startTime + (x - axisStart) / grid_time + 0.5;
    return (t);
}

void findGrid (int sig_i, simtime_t time, int lval, double fval, Grid *x, Grid *y)
{
    Grid yb;

    yb = canvasTop + sigDelta * sig_i / Curr_nr_signals - sigSpace / 8;

    *x = axisStart + (time - startTime) * grid_time;
    if (currLogic) {
	if (lval < 0) lval = 1; /* draw level free state at level x state */
	*y = yb + oY - grid_volt * lval;
    }
    else
	*y = yb + oY - grid_volt * fval / Voltunit;
}

void findBBox (struct signal *sig, simtime_t t1, simtime_t t2, Grid *x1, Grid *y1, Grid *x2, Grid *y2)
{
    Grid yb;

    yb = canvasTop + sigDelta * (sig -> i) / Curr_nr_signals - sigSpace / 8;

    *x1 = axisStart + (t1 - startTime) * grid_time - 1;
    *x2 = axisStart + (t2 - startTime) * grid_time + 1;
    *y1 = yb + oY + 1 + grid_volt * 0.15;
    *y2 = yb + oY - 1 - grid_volt * 2.15;
}

int    Put_nr;
XPoint Put_points[100];

void findPutBBox (struct signal *sig, simtime_t t1, Grid *px1, Grid *py1, Grid *px2, Grid *py2)
{
    Grid x1, x2, y1, y2, yb, yb1, yb2;
    struct sig_value *sval = storedVals;

    yb = canvasTop + sigDelta * (sig -> i) / Curr_nr_signals - sigSpace / 8;
    x1 = axisStart + (t1 - startTime) * grid_time;

    *px1 = x1 - 1;
    *px2 = x1 + 1 + storedTD * grid_time;
    *py1 = yb + 1 + grid_volt * 0.15;
    *py2 = yb - 1 - grid_volt * 2.15;

    yb1 = yb - grid_volt;
    yb2 = yb - grid_volt * 2;

    switch (sval -> value) {
        case 0:  y1 = yb; break;
        case 2:  y1 = yb2; break;
        default: y1 = yb1;
    }
    Put_points[0].x = x1;
    Put_points[0].y = y1;
    Put_nr = 1;

    x1 = 0;
    while ((sval = sval -> next)) {
        x2 = sval -> time * grid_time;
        switch (sval -> value) {
            case 0:  y2 = yb; break;
            case 2:  y2 = yb2; break;
            default: y2 = yb1;
        }
        Put_points[Put_nr].x = x2 - x1;
        Put_points[Put_nr].y = 0;
        if (++Put_nr >= 98) return;
        if (y2 != y1) {
            Put_points[Put_nr].x = 0;
            Put_points[Put_nr].y = y2 - y1;
            ++Put_nr;
            y1 = y2;
        }
        x1 = x2;
    }
    x2 = storedTD * grid_time;
    Put_points[Put_nr].x = x2 - x1;
    Put_points[Put_nr].y = 0;
    ++Put_nr;
}

void _markSignal (struct signal *sig, int mark)
{
    static Grid yb;
    static int len;

    if (mark) {
	yb = canvasTop + sigDelta * (sig -> i) / Curr_nr_signals - sigSpace / 8;
	if ((len = strlen (sig -> name)) > SIGLEN) len = SIGLEN;
	XSetBackground (display, gc, normalColor);
	XSetForeground (display, gc, backgrColor);
    }
    XDrawImageString (display, window, gc, canvasLeft, yb, sig -> name, len);
    if (mark) {
	XSetForeground (display, gc, normalColor);
	XSetBackground (display, gc, backgrColor);
    }
}

void markSignal (struct signal *sig)
{
    _markSignal (sig, 1);
}

void unMarkSignal (struct signal *sig)
{
    _markSignal (sig, 0);
}

void moveSigOnCanvas (struct signal *sig, struct signal *sig2)
{
    /* Note: There must be at least two signals on Canvas!
     *		(Curr_nr_signals >= 2)
     *       Because End/Begin_signal and Top/Bottom_signal
     *       may not be set to NULL!
     */
    if (sig -> next)
	sig -> next -> prev = sig -> prev;
    else
	End_signal = sig -> prev;

    if (sig -> prev)
	sig -> prev -> next = sig -> next;
    else
	Begin_signal = sig -> next;

    if (sig == Top_signal)
	Top_signal = sig -> next;
    else if (sig == Bottom_signal)
	Bottom_signal = sig -> prev;

    /* sig is inserted above (previous to) sig2.
       if sig2 == NULL, sig is inserted as End_signal */

    sig -> next = sig2;

    if (!sig2) {
	sig -> prev = Bottom_signal;
	Bottom_signal -> next = sig;
	End_signal = Bottom_signal = sig;
    }
    else {
	sig -> prev = sig2 -> prev;
	if (sig2 -> prev) {
	    if (sig2 -> prev == Bottom_signal) Bottom_signal = sig;
	    sig2 -> prev -> next = sig;
	}
	else
	    Begin_signal = sig;
	sig2 -> prev = sig;
	if (sig2 == Top_signal) Top_signal = sig;
    }
}

void delSigFromCanvas (struct signal *sig)
{
    if (Bottom_signal -> next) {
	Bottom_signal = Bottom_signal -> next;
    }
    else if (Top_signal -> prev) {
	Top_signal = Top_signal -> prev;
    }
    else Curr_nr_signals--;

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

    Nr_signals--;

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
		sval -> value = val;
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
    static int w = 0;
    Grid x_mess;
    int len, lfw;
    char c;

    /* prints a message on the canvas */

    if (windowListFn[0]) {
	windowListFn[0] = '\0';
	clear ();
    }
    else if (!clip_mask && w)
	XpClearArea (pdisplay, window, 0, 4, w, canvasTop / 2 + fontHeight, False);

    if (s) {
	len = strlen (s);
	if (len > MBLEN) {
	    len = MBLEN;
	    c = s[len]; s[len] = 0;
	}
	else
	    c = 0;
	lfw = len * fontWidth;
	if (x >= 0) {
	    x_mess = x - lfw / 2;
	    if (x_mess > axisStop - lfw) x_mess = axisStop - lfw;
	    if (x_mess < axisStart) x_mess = axisStart;
	}
	else {
	    x_mess = canvasLeft;
	}
	w = x_mess + lfw + 1;
	XpDrawString (pdisplay, window, gc, x_mess, (canvasTop + fontHeight) / 2, s, len);
	strcpy (windowMesBuf, s);
	if (c) s[len] = c;
	windowMesX = x;
    }
    else
	windowMesBuf[0] = '\0';

    if (x == -2) XFlush (display);
}

void windowList (char *fn)
{
    char buf[256];
    int c, i;
    FILE *fp;

    if (fn) strcpy (windowListFn, fn);
    fp = fopen (windowListFn, "r");

    clear ();

    if (fp) {
	Grid y = (canvasTop + fontHeight) / 2;
	while ((c = getc (fp)) != EOF && y < canvasHeight + fontHeight) {
	    i = 0;
	    while (c != '\n') {
		if (i < 256) buf[i++] = c;
		else if (c == EOF) break;
		c = getc (fp);
	    }
	    XpDrawString (pdisplay, window, gc, canvasLeft, y, buf, i);
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
