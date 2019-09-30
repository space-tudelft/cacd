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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>               /* for atof */
#include "src/space/include/config.h"

#ifdef CONFIG_XSPACE
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/keysym.h>

#include <X11/Xaw/Form.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Label.h>
#endif // CONFIG_XSPACE

#include "src/space/auxil/auxil.h"
#include "src/libddm/dmincl.h"
#include "src/space/include/type.h"
#include "src/space/scan/scan.h"
#include "src/space/scan/export.h"
#include "src/space/scan/extern.h"
#ifdef CONFIG_XSPACE
#include "src/space/X11/menu.h"
#include "src/space/X11/pane.h"
#include "src/space/X11/extern.h"

Widget canvasWidget;
Widget draw3dWidget;
Widget pauseWidget;
Widget robotWidget;
Widget runfreeWidget;
Widget runstepWidget;
Widget prev_cell_widget = 0;

int goptOnly3DMesh;
int goptSelectiveElimination = 0;
int goptCap;
int goptLatCap;
int goptCoupCap;
int goptCap3D;
int goptSubRes;
int goptSimpleSubRes;
int goptIntRes;
int goptResMesh;
int goptAllRes;
int goptNoReduc;
#endif // CONFIG_XSPACE

int goptPairBoundary = FALSE;
int goptDrawTile = FALSE;
int goptDrawEdge = FALSE;
int goptDrawPosition = FALSE;
int goptDrawResistor = FALSE;
int goptUnDrawResistor = FALSE;
int goptOutResistor = FALSE;
int goptDrawCapacitor = FALSE;
int goptUnDrawCapacitor = FALSE;
int goptOutCapacitor = FALSE;
int goptDrawTriangle = FALSE;
int goptDrawEquiEdge = FALSE;
int goptDrawSpider = FALSE;
int goptDrawGreen = FALSE;
int goptDrawDelaunay = FALSE;
int goptDrawSubContact = FALSE;
int goptFillSubContact = FALSE;
int goptDrawSubResistor = FALSE;

#ifdef CONFIG_SPACE3D
FILE *xout;
#else
int goptPauseAfterPP = FALSE;
int goptSaveImageEP = FALSE;
int goptSaveImagePP = FALSE;
int goptCoordInDbUnits = FALSE;
int goptCoordInMicrons = FALSE;
int goptDraw3D = FALSE;
int goptPairConductOnly = FALSE;
int goptPairToInfinity = FALSE;
int goptRobot = FALSE;
int goptSync = FALSE;

int goptXl, goptXr, goptYb, goptYt;
int olddelay = 1;
int delay = 1;
int camParallel;
double camDistance  = 2;
double camLatitude  = 30;
double camLongitude = 270;
extern float bb_xl, bb_xr, bb_yb, bb_yt;

char cellname[DM_MAXNAME + 1];
Display * display;
Widget cellWidget;
Widget topWidget = NULL;
Widget messwidget;
extern char *prep_bin;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
extern void goLeftRight  (int mode);
extern void gotoCoord    (int cx, int cy, int cz);
extern int  getCoorPoint (coor_t *x, coor_t *y, int cz);
extern void setwPosition (int mode);
extern void zoomPosition (int mode);
extern void zoomBbox (int mode);
extern void zoomUndo (void);
extern void drawFile (char *file);
extern void displayPicture (char *file, int bmode);

Private void destroyHandler (void);
Private void selectCellWidget (Widget top, Widget parent, DM_PROJECT *project);
Private void cbAgain	(Widget widget, caddr_t clientData, caddr_t callData);
Private void cbDraw3D	(Widget widget, caddr_t clientData, caddr_t callData);
Private void cbExtract	(Widget widget, caddr_t clientData, caddr_t callData);
Private void cbPause	(Widget widget, caddr_t clientData, caddr_t callData);
Private void cbQuit	(Widget widget, caddr_t clientData, caddr_t callData);
Private void cbRunFree	(Widget widget, caddr_t clientData, caddr_t callData);
Private void cbRunStep	(Widget widget, caddr_t clientData, caddr_t callData);
Private void cbSync	(Widget widget, caddr_t clientData, caddr_t callData);
Private void apClear	(Widget widget, caddr_t clientData, caddr_t callData);
void cbRobot		(Widget widget, caddr_t clientData, caddr_t callData);
void cbRobotReset	(Widget widget, caddr_t clientData, caddr_t callData);
Private void cbParams	(Widget widget, caddr_t clientData, caddr_t callData);
#ifdef __cplusplus
  }
#endif

#define Extracting (extracting || key_a)

int extracting = 0;
int interrupt  = 0;
int space_ready = 1;

static int gmode, zmode, cx, cy, cz, punt;
static pid_t space_pid;
static int key_a = 0;
static int key_A = 0;
static int singleStepMode = 0;
static int stopped = 0;
static String selectedCell = 0;
static char *draw_file;
static char *excl_cells = "";
static char *only_cells = "";
static char *argv2[100];
static int argc2;

#define A(x) (caddr_t)(x)
#define P(x) (XtCallbackProc)(x)
#define W(x) (Widget)(x)
#define cbDisplay menuCallbackToggle

static myMenu_t optionsMenu[] = {
    {"resistance"	, P(cbDisplay), A(&goptIntRes)},
    {"metal resistance"	, P(cbDisplay), A(&goptAllRes)},
    {"mesh refinement"	, P(cbDisplay), A(&goptResMesh)},
    {"capacitance"	, P(cbDisplay), A(&goptCap)},
    {"coupling cap."	, P(cbDisplay), A(&goptCoupCap)},
    {"lat. coup. cap."	, P(cbDisplay), A(&goptLatCap)},
#ifdef CAP3D
    {"3D capacitance"	, P(cbDisplay), A(&goptCap3D)},
#endif
    {"inter. sub. res."	, P(cbDisplay), A(&goptSimpleSubRes)},
#ifdef CAP3D
    {"3D sub. res."	, P(cbDisplay), A(&goptSubRes)},
#endif
    {"heuristics off"	, P(cbDisplay), A(&goptNoReduc)},
#ifdef CAP3D
    {"BE mesh only"	, P(cbDisplay), A(&goptOnly3DMesh)},
#endif
#ifdef SNE
    {"selective elim."	, P(cbDisplay), A(&goptSelectiveElimination)},
#endif
    {NULL, P(NULL), A(NULL)}
};

static myMenu_t displayMenu[] = {
    {"PairBoundary"	, P(cbDisplay), A(&goptPairBoundary)},
    {"DrawTile"		, P(cbDisplay), A(&goptDrawTile)},
    {"DrawEdge"		, P(cbDisplay), A(&goptDrawEdge)},
    {"DrawPosition"	, P(cbDisplay), A(&goptDrawPosition)},
    {"DrawResistor"	, P(cbDisplay), A(&goptDrawResistor)},
    {"UndrawResistor"	, P(cbDisplay), A(&goptUnDrawResistor)},
    {"DrawOutResistor"	, P(cbDisplay), A(&goptOutResistor)},
    {"DrawFEMesh"	, P(cbDisplay), A(&goptDrawTriangle)},
    {"DrawEquiLines"	, P(cbDisplay), A(&goptDrawEquiEdge)},
    {"DrawCapacitor"	, P(cbDisplay), A(&goptDrawCapacitor)},
    {"UndrawCapacitor"	, P(cbDisplay), A(&goptUnDrawCapacitor)},
    {"DrawOutCapacitor"	, P(cbDisplay), A(&goptOutCapacitor)},
#ifdef CAP3D
    {"DrawBEMesh"	, P(cbDisplay), A(&goptDrawSpider)},
    {"DrawGreen"	, P(cbDisplay), A(&goptDrawGreen)},
    {"3 dimensional"	, P(cbDraw3D) , A(&goptDraw3D)},
#endif
    {"DrawSubTerm"	, P(cbDisplay), A(&goptDrawSubContact)},
    {"FillSubTerm"	, P(cbDisplay), A(&goptFillSubContact)},
    {"DrawDelaunay"	, P(cbDisplay), A(&goptDrawDelaunay)},
    {"DrawSubResistor"	, P(cbDisplay), A(&goptDrawSubResistor)},
    {NULL, P(NULL), A(NULL)}
};

static myMenu_t mainMenu [] = {
    {"extract"	    , P(cbExtract)	, A(NULL)},
    {"extract again", P(cbAgain)	, A(NULL)},
    {"save pp image", P(cbDisplay)	, A(&goptSaveImagePP)},
    {"save ep image", P(cbDisplay)	, A(&goptSaveImageEP)},
    {"display clear", P(apClear)	, A(NULL)},
    {"quit"	    , P(cbQuit)		, A(NULL)},
    {"pause"	    , P(cbPause)	, A(NULL)},
    {"pause afterpp", P(cbDisplay)	, A(&goptPauseAfterPP)},
    {"sync"	    , P(cbSync)		, A(&goptSync)},
    {"run free"	    , P(cbRunFree)	, A(NULL)},
    {"single step"  , P(cbRunStep)	, A(NULL)},
    {"robot"	    , P(cbRobot)	, A(NULL)},
    {"reset robot"  , P(cbRobotReset)	, A(NULL)},
    {"load params"  , P(cbParams)	, A(NULL)},
    { NULL	    , P(NULL)		, A(NULL)}
};

/*
 * Where resources go
 */

typedef struct _XSpaceResources {
    Boolean MenuAside;
    Boolean DelayWidget;
    Boolean MessageWindow;
} XSpaceResources;

XSpaceResources resources;

/*
 * The structure containing the resource information for the
 * Xspace application resources.
 */

#define Offset(field) (XtOffset(XSpaceResources *, field))

static XtResource my_resources[] = {
  {"menuAside", XtCBoolean, XtRBoolean, sizeof(Boolean), Offset(MenuAside), XtRImmediate, A(FALSE)},
  {"delayWidget", XtCBoolean, XtRBoolean, sizeof(Boolean), Offset(DelayWidget), XtRImmediate, A(TRUE)},
  {"messageWindow", XtCBoolean, XtRBoolean, sizeof(Boolean), Offset(MessageWindow), XtRImmediate, A(FALSE)},
};

#undef Offset
int resMenuAside;

void bannerText (char *s1, char *s2)
{
    char buf[300];
    XTextProperty xtp;
    char *strlist[2];

    if (*s1) { sprintf (buf, "%s %s", s1, s2); s2 = buf; }
    strlist[0] = s2;
    strlist[1] = NULL;
    XStringListToTextProperty (strlist, 1, &xtp);
    XSetWMName (display, XtWindow (topWidget), &xtp);
    XFlush (display);
}

void setText (char *s)
{
    static char prev_text[8];
    char *mode = "";

    if (s) {
	if (*s == 'e') { s = "finished"; goto ret; }
	if (*s == 's') { s = "extract stopped"; goto ret; }
	strcpy (prev_text, s);
    }
    s = "extracting";
    if (stopped) mode = " pause";
    else if (singleStepMode) mode = " pause ss";
    else if (!delay) mode = " free";
ret:
    bannerText (cellname, mprintf ("--- %s %s ---%s", s, prev_text, mode));
}

Private void cancelStepMode (int do_text)
{
    if (singleStepMode) {
	singleStepMode = 0;
	if (runstepWidget) widgetInvert (runstepWidget); /* OFF */
	if (do_text) setText (0);
    }
}

void delayMode ()
{
    if (!delay) {
	delay = olddelay;
	if (runfreeWidget) widgetInvert (runfreeWidget); /* OFF */
    }
    cancelStepMode (0);
    if (Extracting) setText (0);
    else bannerText ("Xspace:", mprintf ("run delay=%d", delay));
}

Private void adjustCoord (char *b, int v, int p)
{
    char *t;
    int d, len;
    if (v < 0) sprintf (b, "%05d", v);
    else sprintf (b, "%04d", v);
    len = strlen (b);
    t = b + len;
    *t = *(t-1); --t; *t = *(t-1); --t; *t = *(t-1); --t; *t = '.';
    if (p) {
	++t;
	for (d = 1; d != p; d *= 10) ++t;
    }
    else {
	t = b + len; while (*t == '0') --t;
	if (*t != '.') ++t;
    }
    *t = 0;
}

Private void printCoord (char *s, int x, int y)
{
    char bx[20], by[20], bz[20];
    if (goptDraw3D && *s == 'x') s = "x,y,z";
    if (zmode) {
	if (goptCoordInMicrons) {
	    adjustCoord (bz, cz, zmode == 2 ? punt : 0);
	    bannerText ("Xspace:", mprintf ("z-value=%s", bz));
	}
	else
	    bannerText ("Xspace:", mprintf ("z-value=%d", cz));
    }
    else if (goptCoordInMicrons) {
	adjustCoord (bx, x, gmode == 2 ? punt : 0);
	adjustCoord (by, y, gmode == 4 ? punt : 0);
	if (goptDraw3D) {
	    adjustCoord (bz, cz, gmode == 6 ? punt : 0);
	    bannerText ("Xspace:", mprintf ("%s-point=%s,%s,%s", s, bx, by, bz));
	}
	else
	    bannerText ("Xspace:", mprintf ("%s-point=%s,%s", s, bx, by));
    }
    else if (goptDraw3D)
	bannerText ("Xspace:", mprintf ("%s-point=%d,%d,%d", s, x, y, cz));
    else
	bannerText ("Xspace:", mprintf ("%s-point=%d,%d", s, x, y));
}

/* ARGSUSED */
void hotkeyHandler (Widget w, XtPointer client_data, XEvent *ev, Boolean *b)
{
    static int ctrl_mode, shift_mode;
    XKeyEvent *event;
    KeySym ks;
    Modifiers modi;
    coor_t x, y;
    double v;
    int k, n;

    *b = False;
    event = (XKeyEvent *) ev;
    modi = shift_mode ? ShiftMask : 0;
    XtTranslateKeycode (display, event -> keycode, modi, &modi, &ks);

    if (ev -> type == KeyRelease) {
	     if (ks == XK_Control_L) ctrl_mode -= 1;
	else if (ks == XK_Control_R) ctrl_mode -= 2;
	else if (ks == XK_Shift_L)  shift_mode -= 1;
	else if (ks == XK_Shift_R)  shift_mode -= 2;
	return;
    }
    if (ctrl_mode) {
	if (ks == XK_c) {
	    if (Extracting) {
		if (extracting) displayMode (0); // kill extract process
		interrupt = 1;
	    }
	    return;
	}
	if (ks == XK_l) {
	    redrawPicture (0); // but don't clear
	    return;
	}
	if (ks != XK_Shift_L && ks != XK_Shift_R && ks != XK_Control_L && ks != XK_Control_R) {
	    return;
	}
    }

    switch (ks) { /* KeyPress */
    case XK_Control_L: ctrl_mode += 1; return;
    case XK_Control_R: ctrl_mode += 2; return;
    case XK_Shift_L:  shift_mode += 1; return;
    case XK_Shift_R:  shift_mode += 2; return;
    case XK_minus:
	if (gmode) {
	         if (gmode <= 2) cx = -cx;
	    else if (gmode <= 4) cy = -cy;
	    else cz = -cz;
	    goto ret;
	}
	if (zmode) { cz = -cz; goto ret; }
	break;
    case XK_equal:
	if (!getCoorPoint (&x, &y, cz)) x = y = 0;
	if (gmode) {
	    if (gmode <= 2) { cx = x; gmode = 1; }
	    else            { cy = y; gmode = 3; }
	    punt = 0;
	    goto ret;
	}
	cx = x;
	cy = y;
	break;
    case XK_period:
	if (goptCoordInMicrons && zmode) {
	    punt = 1; zmode = 2; cz /= 1000; cz *= 1000;
	    goto ret;
	}
	if (goptCoordInMicrons && gmode) {
	    punt = 1;
		 if (gmode <= 2) { gmode = 2; cx /= 1000; cx *= 1000; }
	    else if (gmode <= 4) { gmode = 4; cy /= 1000; cy *= 1000; }
	    else                 { gmode = 6; cz /= 1000; cz *= 1000; }
	    goto ret;
	}
	gmode = zmode = 0;
	if (getCoorPoint (&x, &y, cz)) printCoord ("x,y", x, y);
	break;
    case XK_Page_Down:
	setDelay (0);
	break;
    case XK_Page_Up:
	setDelay (1);
	break;
    case XK_KP_Down:
    case XK_KP_Up:
	v = setCamera (0, ks == XK_KP_Down ? -15 : 15);
	if (!Extracting) bannerText ("Xspace:", mprintf ("cam_latitude=%g", v));
	break;
    case XK_KP_Left:
    case XK_KP_Right:
	v = setCamera (1, ks == XK_KP_Left ? -15 : 15);
	if (!Extracting) bannerText ("Xspace:", mprintf ("cam_longitude=%g", v));
	break;
    case XK_KP_Add:      /* keypad '+' */
    case XK_KP_Subtract: /* keypad '-' */
	v = setCamera (ks == XK_KP_Add ? 2 : 3, 0);
	if (!Extracting) bannerText ("Xspace:", mprintf ("cam_distance=%g", v));
	break;
    case XK_KP_Multiply: /* keypad '*' */
	cbDraw3D (W(0), A(0), A(0));
	break;
    case XK_c:
	apClear (W(0), A(0), A(0));
	break;
    case XK_d:
	if (stopped) cbPause (W(0), A(0), A(0));
	delayMode ();
	break;
    case XK_e:
	if (!optNoMenus) cbExtract (W(0), A(0), A(0));
	break;
    case XK_f:
	cbRunFree (W(0), A(0), A(0));
	break;
    case XK_s:
	cbRunStep (W(0), A(0), A(0));
	break;
    case XK_Escape:
	if (!Extracting) {
	    if (goptCoordInMicrons) {
#define D(x) 1e-3*(int)(meters * 1e9 * x)
		bannerText ("Xspace:", mprintf ("%s(%g,%g,%g,%g)", cellname,
		    D(bb_xl), D(bb_xr), D(bb_yb), D(bb_yt)));
	    }
	    else {
		k = goptCoordInDbUnits ? 4 : 1;
#define I(x) (int)(x / k)
		bannerText ("Xspace:", mprintf ("%s(%d,%d,%d,%d)", cellname,
		    I(bb_xl), I(bb_xr), I(bb_yb), I(bb_yt)));
	    }
	}
	break;
    case XK_z:
	punt = 0;
	if (gmode && goptDraw3D) gmode = 5;
	else zmode = 1;
	goto ret;
    case XK_g:
	gmode = 1;
	zmode = punt = 0;
	goto ret;
    case XK_comma:
	gmode = (gmode > 2 && gmode < 5 && goptDraw3D)? 5 : 3;
	zmode = punt = 0;
	goto ret;
    case XK_Return:
	gotoCoord (cx, cy, cz);
	break;
    case XK_BackSpace:
	if (gmode || zmode) {
	    if (gmode > 4 || zmode) n = cz;
	    else   n = gmode > 2 ? cy : cx;
	    if (goptCoordInMicrons) {
		if (!punt && (n % 1000)) {
		    if (n % 10) punt = 1000;
		    else if (n % 100) punt = 100;
		    else punt = 10;
		}
		k = 10000;
		if (punt > 1) { k /= punt; punt /= 10; }
		else if (punt == 1) { punt = 0; goto ret; }
		n /= k;
		if (k > 100) n *= 1000; else n *= k;
	    }
	    else n /= 10;
	    if (zmode) { cz = n; zmode = 2; }
	    else if (gmode > 4) { cz = n; gmode = 6; }
	    else if (gmode > 2) { cy = n; gmode = 4; }
	    else { cx = n; gmode = 2; }
	    goto ret;
	}
	break;
    case XK_0: k = 0; goto num;
    case XK_1: k = 1; goto num;
    case XK_2: k = 2; goto num;
    case XK_3: k = 3; goto num;
    case XK_4: k = 4; goto num;
    case XK_5: k = 5; goto num;
    case XK_6: k = 6; goto num;
    case XK_7: k = 7; goto num;
    case XK_8: k = 8; goto num;
    case XK_9: k = 9; goto num;
    case XK_l:
	if (!optNoMenus) cbParams (W(0), A(0), A(0));
	break;
    case XK_p:
	cbPause (W(0), A(0), A(0));
	break;
    case XK_r:
	if (!optNoMenus) cbRobot (W(0), A(0), A(0));
	break;
    case XK_R:
	if (!optNoMenus) cbRobotReset (W(0), A(0), A(0));
	break;
    case XK_A: /* extract Attach */
	key_A = 1;
    case XK_a: /* extract again */
	cbAgain (W(0), A(0), A(0));
	break;
    case XK_b:
	zoomBbox (0);
	break;
    case XK_u:
	zoomUndo ();
	break;
    case XK_I:
    case XK_O:
	zoomPosition (ks == XK_I ? 3 : 4);
	break;
    case XK_i:
    case XK_o:
	zoomPosition (ks == XK_i ? 1 : 2);
	break;
    case XK_m:
	zoomPosition (0);
	break;
    case XK_x:
    case XK_y:
	setwPosition (ks == XK_x ? 1 : 2);
	break;
    case XK_Left:
    case XK_Right:
	goLeftRight (ks == XK_Left ? 1 : 2);
	break;
    case XK_Up:
    case XK_Down:
	goLeftRight (ks == XK_Down ? 3 : 4);
	break;
    case XK_q:
	cbQuit (W(0), A(0), A(0));
    }
    gmode = zmode = 0;
    return;
num:
    if ((!gmode && !zmode) || punt > 100) return;
    if (goptCoordInMicrons) k *= 1000;
    if (zmode == 1) { ++zmode; cz = k; }
    else if (gmode == 1 || gmode == 3 || gmode == 5) {
	if (++gmode == 2) cx = k;
	else if (gmode == 4) cy = k;
	else cz = k;
    }
    else {
	if (gmode == 6 || zmode == 2) n = cz;
	else n = gmode == 4 ? cy : cx;
	if (n < 0 && n < -99999999) return;
	if (n > 0 && n >  99999999) return;
	if (punt) { punt *= 10; k /= punt; } else n *= 10;
	if (n < 0) n -= k; else n += k;
	if (gmode == 6 || zmode == 2) cz = n;
	else if (gmode == 4) cy = n;
	else cx = n;
    }
ret:
    printCoord ("goto", cx, cy);
}

void sighandler (int sig, siginfo_t *info, void *bla)
{
    if (info->si_pid == space_pid) {
	int status;
	char *s = "??";
	switch (info->si_code) {
	case CLD_EXITED   : s = "has exited";  space_ready = 1; break;
	case CLD_KILLED   : s = "was killed";  space_ready = 1; break;
	case CLD_DUMPED   : s = "terminated";  space_ready = 1; break;
	case CLD_TRAPPED  : s = "has trapped"; space_ready = 1; break;
	case CLD_STOPPED  : s = "has stopped"; break;
	case CLD_CONTINUED: s = "has continued";
	}
	if (space_ready) (void)waitpid (space_pid, &status, WNOHANG);
	Debug (fprintf (stderr, "-- space3d %s (status=%d utime=%ld stime=%ld)\n", s,
	    info->si_status, info->si_utime, info->si_stime));
    }
}

void initXpre (int *argc, char *argv[])
{
    struct sigaction act;

    act.sa_sigaction = sighandler;
    act.sa_flags = (SA_NODEFER | SA_SIGINFO);
    sigaction (SIGCHLD, &act, NULL);

    topWidget = XtInitialize ("xspace", "Xspace", NULL, 0, argc, argv);
}

void initX (int argc, char *argv[], char *optstring)
{
    char *s, *t, *skiplist = "%bBcCl3rzRnUGX";
    int i, j = 0;

    argc2 = 1;
    argv2[argc2++] = "-%%";
    for (i = 1; i < argc; ++i) {
	s = argv[i]; j = 0;
	if (*s == '-') {
	    ++j;
	    while (*++s) {
		if (strchr (skiplist, *s)) continue;
		if ((t = strchr (optstring, *s)) && *(t+1) == ':') {
		    argv[i][j++] = *s++;
		    if (*s) { do { argv[i][j++] = *s; } while (*++s); }
		    else { argv[i][j] = 0; argv2[argc2++] = argv[i++]; j = 0; }
		    --s;
		}
		else
		    argv[i][j++] = *s;
	    }
	    if (j > 1) argv[i][j] = 0;
	}
	if (!j || j > 1) argv2[argc2++] = argv[i];
    }

    ASSERT (topWidget);

    display = XtDisplay (topWidget);

    s = mprintf ("%s/share/lib/app-defaults/%s", icdpath, "Xspace");
    if (access (s, F_OK) == -1)
	fprintf (stderr, "%s: can't access '%s'\n", argv0, s);
    else {
	XrmDatabase db, db2;
	XrmInitialize ();
	if ((db = XrmGetFileDatabase (s))) {
	    if ((db2 = XrmGetDatabase (display))) XrmMergeDatabases (db2, &db);
	    XrmSetDatabase (display, db);
	}
    }

    XtGetApplicationResources (topWidget, A(&resources), my_resources, XtNumber (my_resources), NULL, (Cardinal)0);

    resMenuAside = resources.MenuAside;

    XtAddEventHandler (topWidget, KeyPressMask|KeyReleaseMask, False, (XtEventHandler)hotkeyHandler, (XtPointer)0);
}
#endif // CONFIG_XSPACE

void interactive (DM_PROJECT *project, char **cellNames)
{
#ifdef CONFIG_XSPACE
    Widget menuWidget, spaceWidget = 0;
    Arg	arg[25];
    Cardinal n;
    Dimension w, h;
    double v;

    meters = (1e-6 * project -> lambda) / 4; /* initial internal unit */

    if (cellNames && cellNames[0])
	strcpy (cellname, cellNames[0]);
    else
        cellname[0] = '\0';		/* no cell selected as yet */

    n = 0;
    spaceWidget = XtCreateManagedWidget ("space", formWidgetClass, topWidget, arg, n);

    goptPauseAfterPP = paramLookupB ("disp.pause_after_pp", "off");
    goptSaveImageEP = paramLookupB ("disp.save_extract_image", "off");
    goptSaveImagePP = paramLookupB ("disp.save_prepass_image", "off");
    if ((v = paramLookupD ("disp.cam_distance", "-1")) >= 0) camDistance = v;
    if ((v = paramLookupD ("disp.cam_latitude", "-1")) >= 0) camLatitude = v;
    if ((v = paramLookupD ("disp.cam_longitude", "-380")) > -370) camLongitude = v;
    camParallel = paramLookupB ("disp.cam_parallel", "off");

    goptCoordInDbUnits	= paramLookupB ("disp.coord_in_dbunits", "off");
    goptCoordInMicrons	= paramLookupB ("disp.coord_in_microns", "off");
    if (!(goptOnly3DMesh = optEstimate3D))
	goptOnly3DMesh	= paramLookupB ("disp.be_mesh_only", "off");
    goptPairConductOnly	= paramLookupB ("disp.pair_conduct_only", "off");
    goptPairToInfinity	= paramLookupB ("disp.pair_to_infinity", "off");
#ifdef CAP3D
    goptDraw3D		= paramLookupB ("disp.3_dimensional", "off");
#endif
    {
	char *s;
	if ((s = paramLookupS ("disp.layout_window", 0))) {
	    if (sscanf (s, "%d %d %d %d", &goptXl, &goptXr, &goptYb, &goptYt) != 4) goptXl = goptXr;
	    else if (goptYt <= goptYb) goptXl = goptXr;
	    else if (goptXr <= goptXl) goptXl = goptXr;
	}
    }
#endif // CONFIG_XSPACE

    goptPairBoundary	= paramLookupB ("disp.pair_boundary", "off");
    goptDrawTile	= paramLookupB ("disp.draw_tile", "off");
    goptDrawEdge	= paramLookupB ("disp.draw_edge", "off");
    goptDrawPosition	= paramLookupB ("disp.draw_position", "off");
    goptDrawResistor	= paramLookupB ("disp.draw_resistor", "off");
    goptUnDrawResistor	= paramLookupB ("disp.undraw_resistor", "off");
    goptOutResistor	= paramLookupB ("disp.draw_out_resistor", "off");
    goptDrawCapacitor	= paramLookupB ("disp.draw_capacitor", "off");
    goptUnDrawCapacitor	= paramLookupB ("disp.undraw_capacitor", "off");
    goptOutCapacitor	= paramLookupB ("disp.draw_out_capacitor", "off");
    goptDrawTriangle	= paramLookupB ("disp.draw_fe_mesh", "off");
    goptDrawEquiEdge	= paramLookupB ("disp.draw_equi_lines", "off");
#ifdef CAP3D
    goptDrawSpider	= paramLookupB ("disp.draw_be_mesh", "off");
    goptDrawGreen	= paramLookupB ("disp.draw_green", "off");
#endif
    goptDrawSubContact	= paramLookupB ("disp.draw_sub_term", "off");
    goptFillSubContact	= paramLookupB ("disp.fill_sub_term", goptDrawSubContact ? "on" : "off");
#ifdef CONFIG_SPACE3D
    if (goptFillSubContact) goptDrawSubContact = 1;
#endif
    goptDrawDelaunay	= paramLookupB ("disp.draw_delaunay", "off");
    goptDrawSubResistor	= paramLookupB ("disp.draw_sub_resistor", "off");

#ifdef CONFIG_XSPACE
    goptSync	= paramLookupB ("disp.sync", "off");
    draw_file	= paramLookupS ("disp.draw_file", 0);
    excl_cells	= paramLookupS ("disp.excl_cells", "");
    only_cells	= paramLookupS ("disp.only_cells", "");

    /* Now we are going to create the menu region and the canvas region.
     * It seems that these must be created in the order in
     * which they are placed in the spaceWidget.
     * Otherwise, the spaceWidget becomes to small.
     * This is the reason for the particular structure of the code below.
     */

    /* menu above, goes before canvas */

    menuWidget = 0;

    if (!resources.MenuAside && !optNoMenus) {
	/* Use a form for the menu region, since this one
	 * does something reasonable in scaling.
	 */
	n = 0;
	XtSetArg (arg[n], XtNresizable, TRUE), n++;
	XtSetArg (arg[n], XtNbottom, XtChainTop), n++;
	XtSetArg (arg[n], XtNtop,    XtChainTop), n++;
	XtSetArg (arg[n], XtNleft,   XtChainLeft), n++;
	XtSetArg (arg[n], XtNright,  XtChainRight), n++;

	menuWidget = XtCreateManagedWidget ("menu", formWidgetClass, spaceWidget, arg, n);
    }

    /* canvasWidget */
    n = 0;
    if (menuWidget) XtSetArg (arg[n], XtNfromVert, menuWidget), n++;
    XtSetArg (arg[n], XtNresizable, TRUE), n++;
    XtSetArg (arg[n], XtNbottom, XtChainBottom), n++;
    XtSetArg (arg[n], XtNleft,   XtChainLeft), n++;
    XtSetArg (arg[n], XtNright,  XtChainRight), n++;
    XtSetArg (arg[n], XtNtop,    XtChainTop), n++;

    canvasWidget = XtCreateManagedWidget ("canvas", widgetClass, spaceWidget, arg, n);

    /* Set width and height of canvas if user
     * did no give any preferences
     */
    w = h = 0;
    n = 0;
    XtSetArg (arg[n], XtNwidth, &w); n++;
    XtSetArg (arg[n], XtNheight, &h); n++;
    XtGetValues (canvasWidget, arg, n);

    if (!(n = paramLookupI ("disp.width", "0"))) { if (!w) w = 500; }
    else w = n;
    if (w < 100) w = 100;
    else if (w > 1280) w = 1280;

    if (!(n = paramLookupI ("disp.height", "0"))) { if (!h) h = 375; }
    else h = n;
    if (h < 75) h = 75;
    else if (h > 1024) h = 1024;

    n = 0;
    XtSetArg (arg[n], XtNwidth, w); n++;
    XtSetArg (arg[n], XtNheight, h); n++;
    XtSetValues (canvasWidget, arg, n);

    /* menu aside, goes after canvas */

    if (resources.MenuAside && !optNoMenus) {
	/* Use a menu box, since this one makes all buttons
	 * to have equal width (and heigth).
	 */
	n = 0;
	XtSetArg (arg[n], XtNheight, &h); n++;
	XtGetValues (canvasWidget, arg, n);

	n = 0;
	XtSetArg (arg[n], XtNheight, h), n++;
	XtSetArg (arg[n], XtNwidth, 10), n++;	/* automat. increased */
	XtSetArg (arg[n], XtNfromHoriz, canvasWidget), n++;
	XtSetArg (arg[n], XtNresizable, TRUE), n++;
	XtSetArg (arg[n], XtNleft,   XtChainRight), n++;
	XtSetArg (arg[n], XtNright,  XtChainRight), n++;
	XtSetArg (arg[n], XtNbottom, XtChainBottom), n++;
	XtSetArg (arg[n], XtNtop,    XtChainTop), n++;
	menuWidget = XtCreateManagedWidget ("menu", menuboxWidgetClass, spaceWidget, arg, n);
    }

    n = 0;
    XtSetArg (arg[n], XtNwidth, &w); n++;
    XtGetValues (canvasWidget, arg, n);

    if (resources.MessageWindow) {
	n = 0;
	XtSetArg (arg[n], XtNresizable, TRUE), n++;
	XtSetArg (arg[n], XtNwidth, w), n++;
	XtSetArg (arg[n], XtNfromVert, canvasWidget), n++;
	XtSetArg (arg[n], XtNbottom, XtChainBottom), n++;
	XtSetArg (arg[n], XtNtop,    XtChainBottom), n++;
	XtSetArg (arg[n], XtNleft,   XtChainLeft), n++;
	XtSetArg (arg[n], XtNright,  XtChainRight), n++;

#if 0 /* !!! */
	XtSetArg (arg[n], XtNeditType, XawtextEdit), n++;
	messwidget = XtCreateManagedWidget ("messageWindow", asciiTextWidgetClass, spaceWidget, arg, n);
	XawTextDisplayCaret (messwidget, False);
#else
	XtSetArg (arg[n], XtNlabel, "Demonstration of SPACE"), n++;
	messwidget = XtCreateManagedWidget ("messageWindow", labelWidgetClass, spaceWidget, arg, n);
#endif
    }

    /* Create the menu buttons */

    goptCap     = optCap;
    goptCoupCap = optCoupCap;
    goptLatCap  = optLatCap;
    goptCap3D   = optCap3D;

    goptAllRes  = optAllRes;
    goptIntRes  = optIntRes;
    goptResMesh = optResMesh == 1;
    goptSimpleSubRes = optSimpleSubRes;
    goptSubRes  = optSubRes;

    goptSelectiveElimination = optSelectiveElimination;
    goptNoReduc = optNoReduc;

    if (!optNoMenus) {
	menuDraw (topWidget, menuWidget, "Extract", mainMenu, 1);

	selectCellWidget (topWidget, menuWidget, project);

	menuDraw (topWidget, menuWidget, "Options", optionsMenu, 1);
	menuDraw (topWidget, menuWidget, "Display", displayMenu, 1);

	if (resources.DelayWidget) menuDelayWidget (menuWidget);
    }

    /* make it */
    XtRealizeWidget (topWidget);

    XSync (display, False);

    XSynchronize (display, goptSync);

    if (!optNoMenus) {
	/* We have to respecify the dimension of the menu region.
	 * It does not work to do it before the children are added.
	 */
	if (!resources.MenuAside) {
	    n = 1;
	    XtSetArg (arg[0], XtNwidth, &w);
	    XtGetValues (canvasWidget, arg, n);
	    XtSetArg (arg[0], XtNwidth, &h);
	    XtGetValues (menuWidget, arg, n);
	    if (h > w) { /* set width of canvas to width of menu */
		XtSetArg (arg[0], XtNwidth, h);
		XtSetValues (canvasWidget, arg, n);
		if (resources.MessageWindow)
		    XtSetValues (messwidget, arg, n);
	    }
	    else {
		XtSetArg (arg[0], XtNwidth, w);
		XtSetValues (menuWidget, arg, n);
	    }
	}
	else {
	    n = 1;
	    XtSetArg (arg[0], XtNheight, &h);
	    XtGetValues (canvasWidget, arg, n);
	    XtSetArg (arg[0], XtNheight, h);
	    XtSetValues (menuWidget, arg, n);
	}
    }

    /* Can not do this before realize: there will not be any windows */
    initColors (canvasWidget);

    initCanvas (canvasWidget);

    /* Activate handler for fatal I/O errors (e.g. when a XKillClient was done) */
    XSetIOErrorHandler ((XIOErrorHandler)destroyHandler);

    robotInit ();

    MyMainLoop ();
    /* XtMainLoop(); */
#endif // CONFIG_XSPACE
}

#ifdef CONFIG_XSPACE
Private void destroyHandler ()
{
    die ();
}

void dispatchEvent (XEvent *event)
{
    if (!XtDispatchEvent (event)) {
	if (event -> type == KeyPress || event -> type == KeyRelease) {
	    Boolean b;
	    hotkeyHandler (W(0), (XtPointer)0, event, &b);
	}
    }
}

void MyMainLoop ()
{
    XEvent Event;
    int extractOnce = TRUE;

    for (;;) {
	XtNextEvent (&Event);

	Debug (if (goptRobot) printEvent (&Event));

	dispatchEvent (&Event);

        if (optNoMenus && extractOnce) {
            extractOnce = FALSE;
	    cbExtract (W(0), A(0), A(0));
	}

	/* Could make this an installed event handler,
	 * but should check the other dispatchers.
	 */
	robot (&Event);
    }
}

int doEvent ()
{
    XEvent event;
    if (XtPending ()) {
	XtNextEvent (&event);
	dispatchEvent (&event);
    }
    return (interrupt || space_ready);
}

int setPause ()
{
    if (delay && !singleStepMode) {
	if (!stopped) cbPause (W(0), A(0), A(0));
	return stopped;
    }
    return 0;
}

void showPicture ()
{
    XEvent event;
    int d;

    if (stopped) {
	while (stopped) {
	    XtNextEvent (&event);
	    dispatchEvent (&event);
	    if (interrupt) cbPause (W(0), A(0), A(0));
	}
    }
    else if (singleStepMode) {
ss:	singleStepMode = 2;
	while (singleStepMode == 2) { /* wait for single step done or cancel */
	    XtNextEvent (&event);
	    dispatchEvent (&event);
	    if (interrupt) return;
	}
    }
    else for (d = delay; d > 0; d--) {
	if (XtPending ()) {
	    XtNextEvent (&event);
	    dispatchEvent (&event);
	    if (interrupt || !delay) return;
	    if (singleStepMode) goto ss;
	}
    }
}

/* Private procedures
 */

#define exclCell(name) findCell (name, excl_cells)
#define onlyCell(name) findCell (name, only_cells)

Private int findCell (char *name, char *list)
{
    register char *p, *q;

    p = list;
    if (!*p) return 0;
    do {
	while (*p == ' ') ++p;
	q = name;
	while (*q && (*p == *q || *p == '?')) { ++p; ++q; }
	if (*p == '*' || (!*q && (!*p || *p == ' '))) return 1;
	while (*p && *p != ' ') ++p;
    } while (*p);
    return 0;
}

Private int compar (const void *A, const void *B)
{
    char **a = (char **) A;
    char **b = (char **) B;
    return (strcmp (*a, *b));
}

#define MAX_NUM_CELLS 102

Private void selectCellWidget (Widget top, Widget parent, DM_PROJECT *project)
{
    char **modlist, **pmod;
    int i, j, num_cells, toomany, var_int = TRUE;
    myMenu_t cellMenu [MAX_NUM_CELLS+3];
    char *cellPresent, *c;

    pmod = (char **) dmGetMetaDesignData (CELLLIST, project, LAYOUT);

    /* count number of cells */
    num_cells = 0;
    for (modlist = pmod; *modlist; ++modlist) ++num_cells;

    /* discard unwanted cells from the Database menu */
    /* a present 'cellname' is never discarded */
    cellPresent = cellname[0] ? 0 : "";
    for (i = 0; i < num_cells; ++i) {
	if (!cellPresent && strsame (pmod[i], cellname))
	    cellPresent = pmod[i];
	else if ((!*only_cells && exclCell (pmod[i])) ||
		 (*only_cells && !onlyCell (pmod[i]))) {
	    c = pmod[i]; pmod[i--] = pmod[--num_cells]; pmod[num_cells] = c;
	}
    }

    /* sort the cell names of the Database menu */
    if (num_cells > 1) qsort (pmod, num_cells, sizeof (char *), compar);

    /* A present 'cellname' is not more put at the top of the menu,
     * because the menu is sorted.  When there are too many cells,
     * only a part around the present 'cellname' is shown.
     * A dummy present 'cellname' can be used to position the menu.
     */
    toomany = i = 0;
    if (num_cells > MAX_NUM_CELLS) {
	toomany = 1;
	say ("Warning: only %d cells can be selected from the menu", MAX_NUM_CELLS);
	if (*cellname) {
	    int len = strlen (cellname);
	    modlist = pmod + MAX_NUM_CELLS / 3;
	    num_cells -= MAX_NUM_CELLS;
	    for (i = 0; i < num_cells; ++i) {
		if (strncmp (modlist[i], cellname, len) >= 0) break;
	    }
	    if (i > 0) ++toomany;
	    if (i == num_cells) ++toomany;
	}
	num_cells = MAX_NUM_CELLS;
    }
    modlist = pmod + i;

    j = 0;
    if (toomany > 1) {
	cellMenu[j].label = "<<<MORE<<<";
	cellMenu[j].callback = NULL;
	cellMenu[j].client_data = NULL;
	++j;
    }
    for (i = 0; i < num_cells; i++) {
	cellMenu[j].label = modlist[i];
	cellMenu[j].callback = P(cbSelectCell);
	if (modlist[i] == cellPresent)
	    cellMenu[j].client_data = A(&var_int);
	else
	    cellMenu[j].client_data = NULL;
	++j;
    }
    if (toomany && toomany < 3) {
	cellMenu[j].label = ">>>MORE>>>";
	cellMenu[j].callback = NULL;
	cellMenu[j].client_data = NULL;
	++j;
    }
    cellMenu[j].label = NULL;
    *cellname = 0; /* if found, the cell is selected in the menu,
		      else it is an incorrect (dummy) cell */

    if (num_cells > 68) {
	i = (num_cells + 1) / 3;
	j = 2 * i;
	if (toomany > 1) { ++i; ++j; }
	c = cellMenu[i].label; cellMenu[i].label = NULL;
	menuDraw (top, parent, "Database1", cellMenu, 2);
	cellMenu[i].label = c;
	c = cellMenu[j].label; cellMenu[j].label = NULL;
	menuDraw (top, parent, "Database2", cellMenu + i, 2);
	cellMenu[j].label = c;
	menuDraw (top, parent, "Database3", cellMenu + j, 2);
    }
    else if (num_cells > 34) {
	i = num_cells / 2;
	cellMenu[i].label = NULL;
	menuDraw (top, parent, "Database1", cellMenu, 2);
	cellMenu[i].label = modlist[i];
	menuDraw (top, parent, "Database2", cellMenu + i, 2);
    }
    else
	menuDraw (top, parent, "Database", cellMenu, 2);
}

/* the callback functions. */

/* ARGSUSED */
Private void cbDraw3D (Widget widget, caddr_t clientData, caddr_t callData)
{
    if (draw3dWidget) widgetInvert (draw3dWidget);
    goptDraw3D = !goptDraw3D;
    if (!widget || Extracting) zoomBbox (1);
}

/* ARGSUSED */
Private void cbRunFree (Widget widget, caddr_t clientData, caddr_t callData)
{
    if (stopped) cbPause (W(0), A(0), A(0));

    if (delay) {
	delay = 0;
	if (runfreeWidget) widgetInvert (runfreeWidget); /* ON */
    }
    cancelStepMode (0);
    if (Extracting) setText (0);
    else bannerText ("Xspace:", "run free");
}

/* ARGSUSED */
Private void cbRunStep (Widget widget, caddr_t clientData, caddr_t callData)
{
    if (stopped) cbPause (W(0), A(0), A(0));

    if (singleStepMode) goto ret; /* cancel wait */
    if (!delay) {
	delay = olddelay;
	if (runfreeWidget) widgetInvert (runfreeWidget); /* OFF */
    }
    if (runstepWidget) widgetInvert (runstepWidget); /* ON */
ret:
    singleStepMode = 1;
    if (Extracting) setText (0);
    else bannerText ("Xspace:", "run single step");
}

/* ARGSUSED */
Private void cbSync (Widget widget, caddr_t clientData, caddr_t callData)
{
    widgetInvert (widget);
    goptSync = !goptSync;
    XSynchronize (display, goptSync);
}

/* ARGSUSED */
Private void cbPause (Widget widget, caddr_t clientData, caddr_t callData)
{
    if (!Extracting) return;
    if (singleStepMode) cancelStepMode (0);
    else {
	stopped = !stopped;
	if (pauseWidget) widgetInvert (pauseWidget);
    }
    setText (0);
}

/* ARGSUSED */
Private void cbParams (Widget widget, caddr_t clientData, caddr_t callData)
{
    char *s;
    double v;
    int new1, new2;

    if (Extracting) return;
    reGetParameters ();
    bannerText ("Xspace:", "parameters loaded");
    draw_file = paramLookupS ("disp.draw_file", 0);

    if ((s = paramLookupS ("disp.layout_window", 0))) {
	if (sscanf (s, "%d %d %d %d", &goptXl, &goptXr, &goptYb, &goptYt) != 4) goptXl = goptXr;
	else if (goptYt <= goptYb) goptXl = goptXr;
	else if (goptXr <= goptXl) goptXl = goptXr;
    }

    goptPairConductOnly	= paramLookupB ("disp.pair_conduct_only", goptPairConductOnly? "on" : "off");
    goptPairToInfinity  = paramLookupB ("disp.pair_to_infinity" , goptPairToInfinity ? "on" : "off");
    if ((v = paramLookupD ("disp.cam_distance", "-1")) >= 0) camDistance = v;
    if ((v = paramLookupD ("disp.cam_latitude", "-1")) >= 0) camLatitude = v;
    if ((v = paramLookupD ("disp.cam_longitude", "-380")) > -370) camLongitude = v;
    camParallel = paramLookupB ("disp.cam_parallel", camParallel ? "on" : "off");

    new1 = paramLookupB ("disp.coord_in_dbunits", goptCoordInDbUnits ? "on" : "off");
    new2 = paramLookupB ("disp.coord_in_microns", goptCoordInMicrons ? "on" : "off");
    if (new1 != goptCoordInDbUnits || new2 != goptCoordInMicrons) {
	// change old values first to internal units
	if (goptCoordInMicrons) {
	    cx /= meters * 1e9;
	    cy /= meters * 1e9;
	    cz /= meters * 1e9;
	}
	else if (goptCoordInDbUnits) {
	    cx *= 4; cy *= 4; cz *= 4;
	}
	goptCoordInDbUnits = new1;
	goptCoordInMicrons = new2;
	if (goptCoordInMicrons) {
	    cx *= meters * 1e9;
	    cy *= meters * 1e9;
	    cz *= meters * 1e9;
	}
	else if (goptCoordInDbUnits) {
	    cx /= 4; cy /= 4; cz /= 4;
	}
    }
    gmode = zmode = 0;
}

Private void run_extract ()
{
    char path[1000];
    char opts[40];
    int i;

    unlink ("display.out");

    i = 0;
    opts[i++] = '-';
    opts[i++] = 'X';
    if (goptAllRes )	opts[i++] = 'R';
    if (goptIntRes )	opts[i++] = 'r';
    if (goptResMesh)	opts[i++] = 'z';
    if (goptCap3D  )	opts[i++] = '3';
    if (goptCap    )	opts[i++] = 'c';
    if (goptCoupCap)	opts[i++] = 'C';
    if (goptLatCap )	opts[i++] = 'l';
    if (goptSimpleSubRes) opts[i++] = 'b';
    if (goptSubRes )	opts[i++] = 'B';
    if (goptNoReduc)	opts[i++] = 'n';
    if (goptOnly3DMesh) opts[i++] = 'U';
    if (goptSelectiveElimination) opts[i++] = 'G';
    opts[i] = 0;

    i = argc2;
    argv2[i++] = opts;
    if (goptPairBoundary)    argv2[i++] = "-Sdisp.pair_boundary";
    if (goptDrawTile)        argv2[i++] = "-Sdisp.draw_tile";
    if (goptDrawEdge)        argv2[i++] = "-Sdisp.draw_edge";
    if (goptDrawPosition)    argv2[i++] = "-Sdisp.draw_position";
    if (goptDrawResistor)    argv2[i++] = "-Sdisp.draw_resistor";
    if (goptUnDrawResistor)  argv2[i++] = "-Sdisp.undraw_resistor";
    if (goptOutResistor)     argv2[i++] = "-Sdisp.draw_out_resistor";
    if (goptDrawCapacitor)   argv2[i++] = "-Sdisp.draw_capacitor";
    if (goptUnDrawCapacitor) argv2[i++] = "-Sdisp.undraw_capacitor";
    if (goptOutCapacitor)    argv2[i++] = "-Sdisp.draw_out_capacitor";
    if (goptDrawTriangle)    argv2[i++] = "-Sdisp.draw_fe_mesh";
    if (goptDrawEquiEdge)    argv2[i++] = "-Sdisp.draw_equi_lines";
    if (goptDrawSpider)      argv2[i++] = "-Sdisp.draw_be_mesh";
    if (goptDrawGreen)       argv2[i++] = "-Sdisp.draw_green";
    if (goptDrawSubContact || goptFillSubContact) argv2[i++] = "-Sdisp.draw_sub_term";
    if (goptDrawDelaunay)    argv2[i++] = "-Sdisp.draw_delaunay";
    if (goptDrawSubResistor) argv2[i++] = "-Sdisp.draw_sub_resistor";
    argv2[i++] = cellname;
    argv2[i] = 0;

    if (prep_bin)
	sprintf (path, "%s/space3d", prep_bin);
    else
	sprintf (path, "space3d");
    argv2[0] = path;

    Debug (while (--i >= 0) fprintf (stderr, "i=%d argv='%s'\n", i, argv2[i]));
    while (--i >= 0) fprintf (stderr, "i=%d argv='%s'\n", i, argv2[i]);

    if ((space_pid = vfork ()) == 0) { /* child */
	FILE *fp;
	execvp (path, argv2);
	fprintf (stderr, "Xspace: cannot exec '%s'\n", path);
	if ((fp = fopen ("display.out", "w"))) {
	    fprintf (fp, "#end\n");
	    fclose (fp);
	}
	_exit (1);
    }
    space_ready = 0;
}

static char *disp_file;

void redrawPicture (int again)
{
    int d = delay;
    int s = singleStepMode;
    if (!again) { singleStepMode = 0; delay = 0; }
    displayPicture (disp_file ? disp_file : "display.out", again);
    disp_file = NULL;
    drawFile (draw_file);
    interrupt = 0;
    if (!again) { singleStepMode = s; delay = d; }
    else if (!delay) { /* cancel run free */
	delay = olddelay;
	if (runfreeWidget) widgetInvert (runfreeWidget); /* OFF */
    }
}

/* ARGSUSED */
Private void cbAgain (Widget widget, caddr_t clientData, caddr_t callData)
{
    struct stat stat_buf;

    if (key_A) { key_A = 0;
	fprintf (stderr, "extract Attach!\n");
	disp_file = "./display.out";
	if (!*cellname && selectedCell) strcpy (cellname, selectedCell);
	if (Extracting) {
	    fprintf (stderr, "sorry: Extracting!\n");
	} else if (!*cellname) {
	    fprintf (stderr, "sorry: No cellname!\n");
	} else {
	    char *file = mprintf ("./circuit/%s/display.out", cellname);
	    if (!stat (file, &stat_buf)) disp_file = file;
	    else fprintf (stderr, "sorry: Can't stat: %s\n", file);
	}
	fprintf (stderr, "using: %s\n", disp_file);
    }
    gmode = zmode = 0;
    if (stopped) cbPause (W(0), A(0), A(0));
    if (Extracting) { cancelStepMode (1); return; }
    key_a = 1;
    redrawPicture (1);
    key_a = 0;
    robot ((XEvent *) NULL);
}

/* ARGSUSED */
Private void cbExtract (Widget widget, caddr_t clientData, caddr_t callData)
{
    gmode = zmode = 0;
    if (stopped) cbPause (W(0), A(0), A(0));
    if (Extracting) { cancelStepMode (1); return; }
    if (selectedCell) strcpy (cellname, selectedCell);
    if (!*cellname) {
	bannerText ("Xspace:", "no cell selected");
	say ("Select a cell first!");
	return;
    }
    extracting = 1;
    setText ("#??");
    run_extract ();
    redrawPicture (1);
    extracting = 0;
    robot ((XEvent *) NULL);
}

/* ARGSUSED */
void apClear (Widget widget, caddr_t clientData, caddr_t callData)
{
    clearDisplay ();
}
#endif // CONFIG_XSPACE

void displayMode (char *s)
{
#ifdef CONFIG_SPACE3D
    if (!xout) xout = fopen ("display.out", "w");
    if (xout) { fprintf (xout, "#%s\n", s); fflush (xout); }
#else
    // die signal
    if (!space_ready && space_pid) kill (space_pid, SIGINT);
#endif
}

#ifdef CONFIG_XSPACE
/* ARGSUSED */
Private void cbQuit (Widget widget, caddr_t clientData, caddr_t callData)
{
    quit ();
}

/* ARGSUSED */
void cbRobot (Widget widget, caddr_t clientData, caddr_t callData)
{
    if (goptRobot) {
	goptRobot = FALSE;
	widgetInvert (robotWidget);
	bannerText ("Xspace:", "robot stop");
	robotStop ();
    }
    else if (robotStart ()) {
	goptRobot = TRUE;
	widgetInvert (robotWidget);
	bannerText ("Xspace:", "robot start");
	if (Extracting) return;
	robot ((XEvent *) NULL);
    }
    else
	bannerText ("Xspace:", "no program");
}

/* ARGSUSED */
void cbRobotReset (Widget widget, caddr_t clientData, caddr_t callData)
{
    if (goptRobot) {
	goptRobot = FALSE;
	widgetInvert (robotWidget);
    }
    bannerText ("Xspace:", "robot reset");
    robotInit ();
}

void cbSelectCell (Widget widget)
{
    Arg	arg[2];
    if (prev_cell_widget) widgetInvert (prev_cell_widget);
    widgetInvert (prev_cell_widget = widget);
    XtSetArg (arg[0], XtNlabel, &selectedCell);
    XtGetValues (widget, arg, 1);
}

void xMessage (char *str)
{
    Arg	arg[2];
    if (!resources.MessageWindow) {
	fprintf (stderr, "%s\n", str);
	return;
    }
    XtSetArg (arg[0], XtNlabel, str);
    XtSetValues (messwidget, arg, 1);
    XFlush (display);
    RedisplayLabel (messwidget);
}
#endif // CONFIG_XSPACE
