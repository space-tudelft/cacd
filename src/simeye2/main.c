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

#define TOOLVERSION "4.2.5 (21 Dec 2016)"

extern char readSetMes[];

char *argv0 = "simeye2";
char *usg_msg = "\nUsage: %s [X options] [-Lh] [-l|-m|-c|-P|-T] [-a|-b] [cell]\n\n";

/* Some commands and variables and their defaults.
** The default values may be overwitten in the file ".simeyerc".
*/

char *simdiagFile  = "sim.diag";
char *simdiagFile2 = "sim.diag2";
char *slsCommand   = "sls $cell $cell.cmd";
char *spiceCommand = "nspice $cell $cell.cmd";
char *pstarCommand = "npstar $cell $cell.cmd";
char *platoCommand = "nplato $cell $cell.cmd";
int slsLogicLevel = 2;
char *xdumpFile = "simeye.wd";
char *printCommand = "xpr -device ps -output $cell.ps simeye.wd;lp $cell.ps;rm simeye.wd $cell.ps";
char *printLabel = "$user  $file  $date  $time";
char *settingsFile = "simeye.set";

void beginCommand (Widget w); // events.c
int  checkEvent (Display *display, XEvent *event);
int  checkRaces (void);
int  destroyHandler (Display *display);
void endCommand (void); // events.c
int  errorInFile (char *fn);
void eventsStartup (void); // events.c
void int_hdl (int sig);
void initIntrup (void);
int  loadSettings (void);
char *parseCommand (char *s);
void rcReadError (char *fn, char *s);
void readrcFile (void);
int  readSignals (int flag, int intermediate);
void redrawHandler (Widget w, caddr_t client_data, XEvent *event);
int  runTool (char *s, char *outFile, int monitorSig);

void ClickSim   (Widget w, caddr_t client_data, caddr_t call_data);
void ClickSigType (Widget w, caddr_t client_data, caddr_t call_data);
void ClickSimType (Widget w, caddr_t client_data, caddr_t call_data);
void ClickType  (Widget w, caddr_t client_data, caddr_t call_data);
void Hardcopy   (Widget w, caddr_t client_data, caddr_t call_data);
void Inputs     (Widget w, caddr_t client_data, caddr_t call_data);
void Loadwin    (Widget w, caddr_t client_data, caddr_t call_data);
void Move       (Widget w, caddr_t client_data, caddr_t call_data);
void Pointer    (Widget w, caddr_t client_data, caddr_t call_data);
void Quit       (Widget w, caddr_t client_data, caddr_t call_data);
void Read       (Widget w, caddr_t client_data, caddr_t call_data);
void Redraw     (Widget w, caddr_t client_data, caddr_t call_data);
void Return     (Widget w, caddr_t client_data, caddr_t call_data); // edit.c
void Savewin    (Widget w, caddr_t client_data, caddr_t call_data);
void Shuffle    (Widget w, caddr_t client_data, caddr_t call_data);
void Simulate   (Widget w, caddr_t client_data, caddr_t call_data);
void Windowmax  (Widget w, caddr_t client_data, caddr_t call_data);
void wInvert    (Widget w, caddr_t client_data, caddr_t call_data);
void Zoomin     (Widget w, caddr_t client_data, caddr_t call_data);
void Zoomout    (Widget w, caddr_t client_data, caddr_t call_data);

char *rcFile = ".simeyerc";
char *defaultCommandFile = "simeye.def.d";

Widget commands;
Widget canvas;
Widget canvasbd;
Widget total;
Widget toplevel;
Widget readw;
Widget simulw;
Widget dialogw;
Widget resw;
Widget pltw;
Widget slsw;
Widget slslogw;
Widget slslogtw;
Widget slstimw;
Widget slstimtw;
Widget spicew;
Widget spicetw;
Widget wmaxw;
Widget redraww;
Widget savewinw;
Widget loadwinw;
Widget zoominw;
Widget zoomoutw;
Widget pointerw;
Widget quitw;
Widget inputsw;
Widget shufflew;
Widget signalw;
Widget siglabelw;
Widget leftw;
Widget rightw;
Widget upw;
Widget downw;
Widget hcopyw;
Widget clickedTypew;
Widget clickedSimw;
Widget clickedSimTypew;
Widget clickedCommandw;

int Logic;
int plato = 0;
int pstar = 0;
int Spice;
int Cmd;
int currLogic;      /*    for current contents  */
int Append;
int SimLevel;
int Level3Logic;
int UseSettings;
int doDetailZoom;
int tryNonCapital;
int usedNonCapital = 0;
char slslogtype;
char slstimtype;
int commandType;
int prevCommandType;

struct signal *Begin_signal = NULL;
struct signal *End_signal = NULL;
int Nr_signals;

simtime_t Begintime = 0;
simtime_t Endtime = 0;
simtime_t SimEndtime = 0;
double Timescaling = 0;
double Voltscaling = 0;
int Global_umin;
int Global_umax;

static char *username;

char inputname[128] = "";

Grid stdWidthButton = 51;
Grid stdHeightButton = 20;
Grid defSepButton = 6;

Grid stdCanvasWidth = 628;
Grid stdCanvasHeight = 418;

int rdHandler = 0;
int simWasPressed = 0;

simtime_t s_simperiod;
double s_sigunit;

static Display *display;
static Window window;

static XtCallbackRec callbacks[2];
static Arg args[15];

static int opt_logic;
static int opt_timing;
static int opt_spice;
static int opt_pstar;
static int opt_plato;
static int opt_analog;
static int opt_digital;

static char lastErrMess[128];

DM_PROJECT *projKey = NULL;

int main (int argc, char *argv[])
{
    int n, error = 0;
    char *s;
    XtArgVal bg;
    long mask;
    Grid borderWidth, width;
    char buf[8];
    Pixel pb;
    Dimension dw, dbw;

    String dialogTranslations =
        "Ctrl<Key>J:    end-of-line()\n\
         Ctrl<Key>M:    end-of-line()\n\
         Ctrl<Key>O:    end-of-line()\n\
         Ctrl<Key>V:    end-of-line()\n\
         Meta<Key>Z:    end-of-line()\n\
         <Key>Linefeed: end-of-line()\n\
         <Key>Return:   end-of-line()";

    static XrmOptionDescRec opTable[] = {
        {"-high_lowColor",  ".high_lowColor",  XrmoptionSepArg, (caddr_t)NULL},
        {"-undefinedColor", ".undefinedColor", XrmoptionSepArg, (caddr_t)NULL},
        {"-voltsColor",     ".voltsColor",     XrmoptionSepArg, (caddr_t)NULL},
        {"-pointerColor",   ".pointerColor",   XrmoptionSepArg, (caddr_t)NULL}
    };
    static int opTable_cnt = 4;

    toplevel = XtInitialize ("main", "Simeye", opTable, opTable_cnt, &argc, argv);

    Cmd = 0;

    /* defaults */

    Logic = 1;
    Spice = 0;
    SimLevel = 3;
    slslogtype = 'D';
    slstimtype = 'D';
    Level3Logic = 1;
    UseSettings = 0;
    doDetailZoom = 0;
    tryNonCapital = 0;

    /* end defaults */

    opt_logic = 0;
    opt_timing = 0;
    opt_spice = 0;
    opt_pstar = 0;
    opt_plato = 0;
    opt_analog = 0;
    opt_digital = 0;

    while (--argc > 0) {
        if ( (*++argv)[0] == '-' ) {
	    for (s = *argv + 1; *s != '\0'; s++) {
	        switch (*s) {
                    case 'L' :
			UseSettings = 1;
                        break;
                    case 'l' :
			opt_logic = 1;
                        break;
                    case 'm' :           /* t can not be used because of Xt */
			opt_timing = 1;
                        break;
                    case 'P' :
			opt_pstar = 1;
                        break;
                    case 'T' :
			opt_plato = 1;
                        break;
                    case 'c' :
			opt_spice = 1;
                        break;
                    case 'a' :
			opt_analog = 1;
                        break;
                    case 'b' :           /* d can not be used because of Xt */
			opt_digital = 1;
                        break;
		    default:
			if (*s != 'h') fprintf (stderr, "-%c: illegal option\n", *s);
			error = 1;
	        }
	    }
	}
	else {
	    sprintf (inputname, "%s", *argv);
	}
	if (error) {
	    fprintf (stderr, "\n%s %s\n", argv0, TOOLVERSION);
	    fprintf (stderr, usg_msg, argv0);
	    die (1);
	}
    }

    dmInit (argv0);

    initIntrup ();

    projKey = dmOpenProject (DEFAULT_PROJECT, PROJ_READ);

    readrcFile ();

    if (opt_logic) {
	Spice = 0;
	SimLevel = slsLogicLevel;
    }
    else if (opt_timing) {
	Spice = 0;
    }
    else if (opt_spice) {
	Spice = 1;
    }
    else if (opt_pstar) {
	Spice = 1;
	pstar = 1;
    }
    else if (opt_plato) {
	Spice = 1;
	plato = 1;
    }

    if (opt_analog && !Spice) {
	if (SimLevel == 3)
	    slstimtype = 'A';
	else
	    slslogtype = 'A';
    }
    else if (opt_digital && !Spice) {
	if (SimLevel == 3)
	    slstimtype = 'D';
	else
	    slslogtype = 'D';
    }

    if (Spice) {
	Logic = 0;
    }
    else if (!Spice) {
	if (SimLevel == 3)
	    Logic = (slstimtype == 'D');
	else
	    Logic = (slslogtype == 'D');
    }

    if (!(username = getenv ("LOGNAME"))) username = getenv ("USER");

    callbacks[1].callback = NULL;

    n = 0;
    total = XtCreateManagedWidget ("total", formWidgetClass, toplevel, args, n);

    commands = XtCreateManagedWidget ("commands", formWidgetClass, total, args, n);

    signalw = XtCreateManagedWidget ("cell", formWidgetClass, commands, args, n);

    n = 0;
    XtSetArg (args[n], XtNwidth, 100), n++;
    XtSetArg (args[n], XtNlabel, "cell:"), n++;
    /* XtSetArg (args[n], XtNinternalHeight, 3), n++; */
    XtSetArg (args[n], XtNvertDistance, 1), n++;
    XtSetArg (args[n], XtNhorizDistance, 3), n++;
    siglabelw = XtCreateManagedWidget ("label", labelWidgetClass, signalw, args, n);

    n = 0;
    XtSetArg (args[n], XtNwidth, 100), n++;
    XtSetArg (args[n], XtNstring, inputname), n++;
    XtSetArg (args[n], XtNuseStringInPlace, True), n++;
    XtSetArg (args[n], XtNlength, 32), n++;
    XtSetArg (args[n], XtNeditType, XawtextEdit), n++;
    XtSetArg (args[n], XtNvertDistance, 1), n++;
    XtSetArg (args[n], XtNhorizDistance, 3), n++;
    XtSetArg (args[n], XtNfromVert, siglabelw), n++;
    dialogw = XtCreateManagedWidget ("input", asciiTextWidgetClass, signalw, args, n);

    XtOverrideTranslations (dialogw, XtParseTranslationTable (dialogTranslations));

    callbacks[0].callback = (XtCallbackProc)ClickSimType;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, 79), n++;
    XtSetArg (args[n], XtNheight, (2 * stdHeightButton - 6 + 9) / 3), n++;
    XtSetArg (args[n], XtNhorizDistance, 8), n++;
    XtSetArg (args[n], XtNfromHoriz, signalw), n++;
    XtSetArg (args[n], XtNlabel, "sls-logic"), n++;
    XtSetArg (args[n], XtNvertDistance, 3), n++;
    slslogw = XtCreateManagedWidget ("sls-logic", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)ClickSigType;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, 20), n++;
    XtSetArg (args[n], XtNheight, (2 * stdHeightButton - 6 + 9) / 3), n++;
    XtSetArg (args[n], XtNhorizDistance, 1), n++;
    XtSetArg (args[n], XtNfromHoriz, slslogw), n++;
    buf[0] = slslogtype;
    buf[1] = '\0';
    XtSetArg (args[n], XtNlabel, buf), n++;
    XtSetArg (args[n], XtNvertDistance, 3), n++;
    slslogtw = XtCreateManagedWidget ("sls-log-type", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)ClickSimType;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, 79), n++;
    XtSetArg (args[n], XtNheight, (2 * stdHeightButton - 6 + 9) / 3), n++;
    XtSetArg (args[n], XtNhorizDistance, 8), n++;
    XtSetArg (args[n], XtNfromHoriz, signalw), n++;
    XtSetArg (args[n], XtNvertDistance, 1), n++;
    XtSetArg (args[n], XtNfromVert, slslogw), n++;
    XtSetArg (args[n], XtNlabel, "sls-timing"), n++;
    slstimw = XtCreateManagedWidget ("sls-timing", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)ClickSigType;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, 20), n++;
    XtSetArg (args[n], XtNheight, (2 * stdHeightButton - 6 + 9) / 3), n++;
    XtSetArg (args[n], XtNhorizDistance, 1), n++;
    XtSetArg (args[n], XtNfromHoriz, slstimw), n++;
    XtSetArg (args[n], XtNvertDistance, 1), n++;
    XtSetArg (args[n], XtNfromVert, slslogtw), n++;
    buf[0] = slstimtype;
    buf[1] = '\0';
    XtSetArg (args[n], XtNlabel, buf), n++;
    slstimtw = XtCreateManagedWidget ("sls-tim-type", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)ClickSimType;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, 79), n++;
    XtSetArg (args[n], XtNheight, (2 * stdHeightButton - 6 + 9) / 3), n++;
    XtSetArg (args[n], XtNhorizDistance, 8), n++;
    XtSetArg (args[n], XtNfromHoriz, signalw), n++;
    XtSetArg (args[n], XtNvertDistance, 1), n++;
    XtSetArg (args[n], XtNfromVert, slstimw), n++;
    if (pstar)
	XtSetArg (args[n], XtNlabel, "pstar"), n++;
    else if (plato)
	XtSetArg (args[n], XtNlabel, "plato"), n++;
    else
	XtSetArg (args[n], XtNlabel, "spice"), n++;
    spicew = XtCreateManagedWidget ("spice", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)ClickSigType;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, 20), n++;
    XtSetArg (args[n], XtNheight, (2 * stdHeightButton - 6 + 9) / 3), n++;
    XtSetArg (args[n], XtNhorizDistance, 1), n++;
    XtSetArg (args[n], XtNfromHoriz, spicew), n++;
    XtSetArg (args[n], XtNvertDistance, 1), n++;
    XtSetArg (args[n], XtNfromVert, slstimtw), n++;
    buf[0] = 'A';
    buf[1] = '\0';
    XtSetArg (args[n], XtNlabel, buf), n++;
    spicetw = XtCreateManagedWidget ("spice-type", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Simulate;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, stdWidthButton), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNhorizDistance, 6), n++;
    XtSetArg (args[n], XtNfromHoriz, slstimtw), n++;
    simulw = XtCreateManagedWidget ("run", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Read;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, stdWidthButton), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromVert, simulw), n++;
    XtSetArg (args[n], XtNhorizDistance, 6), n++;
    XtSetArg (args[n], XtNfromHoriz, slstimtw), n++;
    readw = XtCreateManagedWidget ("read", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Redraw;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, stdWidthButton), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, readw), n++;
    redraww = XtCreateManagedWidget ("redraw", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Windowmax;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, stdWidthButton), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, readw), n++;
    XtSetArg (args[n], XtNfromVert, redraww), n++;
    wmaxw = XtCreateManagedWidget ("full", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Zoomout;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, stdWidthButton), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, wmaxw), n++;
    zoomoutw = XtCreateManagedWidget ("out", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Zoomin;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, stdWidthButton), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, wmaxw), n++;
    XtSetArg (args[n], XtNfromVert, zoomoutw), n++;
    zoominw = XtCreateManagedWidget ("in", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Move;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, (stdWidthButton - defSepButton) / 2), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, zoomoutw), n++;
    upw = XtCreateManagedWidget ("/\\", commandWidgetClass, commands, args, n);

    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, (stdWidthButton - defSepButton) / 2), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, upw), n++;
    downw = XtCreateManagedWidget ("\\/", commandWidgetClass, commands, args, n);

    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, (stdWidthButton - defSepButton) / 2), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, zoomoutw), n++;
    XtSetArg (args[n], XtNfromVert, upw), n++;
    leftw = XtCreateManagedWidget ("<", commandWidgetClass, commands, args, n);

    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, (stdWidthButton - defSepButton) / 2), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, leftw), n++;
    XtSetArg (args[n], XtNfromVert, upw), n++;
    rightw = XtCreateManagedWidget (">", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Savewin;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, (stdWidthButton - defSepButton) / 2), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, rightw), n++;
    savewinw = XtCreateManagedWidget ("S", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Loadwin;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, (stdWidthButton - defSepButton) / 2), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, savewinw), n++;
    loadwinw = XtCreateManagedWidget ("L", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Shuffle;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, stdWidthButton), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, rightw), n++;
    XtSetArg (args[n], XtNfromVert, savewinw), n++;
    shufflew = XtCreateManagedWidget ("move", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Hardcopy;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, stdWidthButton), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, loadwinw), n++;
    hcopyw = XtCreateManagedWidget ("print", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Pointer;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, stdWidthButton), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, shufflew), n++;
    XtSetArg (args[n], XtNfromVert, hcopyw), n++;
    pointerw = XtCreateManagedWidget ("value", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Quit;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, stdWidthButton), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, hcopyw), n++;
    /* XtSetArg (args[n], XtNhorizDistance, 11), n++; */
    quitw = XtCreateManagedWidget ("quit", commandWidgetClass, commands, args, n);

    callbacks[0].callback = (XtCallbackProc)Inputs;
    n = 0;
    XtSetArg (args[n], XtNcallback, callbacks), n++;
    XtSetArg (args[n], XtNwidth, stdWidthButton), n++;
    XtSetArg (args[n], XtNheight, stdHeightButton), n++;
    XtSetArg (args[n], XtNfromHoriz, hcopyw), n++;
    /* XtSetArg (args[n], XtNhorizDistance, 11), n++; */
    XtSetArg (args[n], XtNfromVert, quitw), n++;
    inputsw = XtCreateManagedWidget ("input", commandWidgetClass, commands, args, n);

    n = 0;
    XtSetArg (args[n], XtNfromVert, commands), n++;
    XtSetArg (args[n], XtNdefaultDistance, 0), n++;
    XtSetArg (args[n], XtNresizable, True), n++;
    canvasbd = XtCreateManagedWidget ("canvas_bd", formWidgetClass, total, args, n);

    n = 0;
    XtSetArg (args[n], XtNwidth, stdCanvasWidth), n++;
    XtSetArg (args[n], XtNheight, stdCanvasHeight), n++;
    XtSetArg (args[n], XtNresizable, True), n++;
    canvas = XtCreateManagedWidget ("canvas", widgetClass, canvasbd, args, n);

    XtRealizeWidget (toplevel);

    n = 0;
    XtSetArg (args[n], XtNbackground, &pb), n++;
    XtGetValues (siglabelw, args, n);
    bg = pb;

    n = 0;
    XtSetArg (args[n], XtNborderColor, bg), n++;
    XtSetValues (siglabelw, args, n);

    n = 0;
    XtSetArg (args[n], XtNbackground, &pb), n++;
    XtSetArg (args[n], XtNborderWidth, &dbw), n++;
    XtGetValues (canvas, args, n);
    bg = pb;
    borderWidth = dbw;

    n = 0;
    XtSetArg (args[n], XtNborderColor, bg), n++;
    XtSetValues (canvas, args, n);

    n = 0;
    XtSetArg (args[n], XtNwidth, &dw), n++;
    XtGetValues (commands, args, n);
    width = dw - 2 * borderWidth;

    if (width != stdCanvasWidth) {
	/* this may happen because the border width of the command
	   buttons is not 1 */
	n = 0;
	XtSetArg (args[n], XtNwidth, width), n++;
	XtSetValues (canvas, args, n);
    }

    n = 0;
    XtSetArg (args[n], XtNbackground, &pb), n++;
    XtGetValues (slslogw, args, n);
    bg = pb;

    n = 0;
    XtSetArg (args[n], XtNborderColor, bg), n++;
    XtSetValues (slslogw, args, n);

    n = 0;
    XtSetArg (args[n], XtNbackground, &pb), n++;
    XtGetValues (slslogtw, args, n);
    bg = pb;

    n = 0;
    XtSetArg (args[n], XtNborderColor, bg), n++;
    XtSetValues (slslogtw, args, n);

    n = 0;
    XtSetArg (args[n], XtNbackground, &pb), n++;
    XtGetValues (slstimw, args, n);
    bg = pb;

    n = 0;
    XtSetArg (args[n], XtNborderColor, bg), n++;
    XtSetValues (slstimw, args, n);

    n = 0;
    XtSetArg (args[n], XtNbackground, &pb), n++;
    XtGetValues (slstimtw, args, n);
    bg = pb;

    n = 0;
    XtSetArg (args[n], XtNborderColor, bg), n++;
    XtSetValues (slstimtw, args, n);

    n = 0;
    XtSetArg (args[n], XtNbackground, &pb), n++;
    XtGetValues (spicew, args, n);
    bg = pb;

    n = 0;
    XtSetArg (args[n], XtNborderColor, bg), n++;
    XtSetValues (spicew, args, n);

    n = 0;
    XtSetArg (args[n], XtNbackground, &pb), n++;
    XtGetValues (spicetw, args, n);
    bg = pb;

    n = 0;
    XtSetArg (args[n], XtNborderColor, bg), n++;
    XtSetValues (spicetw, args, n);

    if (Spice) {
	wInvert (spicew, NULL, NULL);
	clickedSimTypew = spicew;
    }
    else {
	if (SimLevel == 1 || SimLevel == 2) {
	    wInvert (slslogw, NULL, NULL);
	    clickedSimTypew = slslogw;
	}
	else {
	    wInvert (slstimw, NULL, NULL);
	    clickedSimTypew = slstimw;
	}
    }

    currLogic = Logic;

    display = XtDisplay (toplevel);
    window = XtWindow (toplevel);

    drawStartup ();
    eventsStartup ();

    XSync (display, False);

    if (*inputname) readSignals (0, 0);

    /* activate a redraw handler for exposure and resize events */

    mask = ExposureMask;
    XtAddEventHandler (canvas, mask, False, (XtEventHandler)redrawHandler, NULL);

    /* activate handler for fatal I/O errors (e.g. when a XKillClient was done */
    XSetIOErrorHandler (destroyHandler);

    XtMainLoop ();

    return (0);
}

void redrawHandler (Widget w, caddr_t client_data, XEvent *event)
{
    if (Begin_signal) {

	rdHandler = 1;

	clear ();
	draw (0, 0, 0, 0, 0);

        redrawEvent ();
 	                            /* wait and discard new events to */
	XSync (display, True);      /* prevent boots-trapping of the handler */
    }
    else {
	redrawListOrMessage ();
    }
}

int destroyHandler (Display *display)
{
    int_hdl (SIGTERM);
    return (0);
}

void ClickType (Widget w, caddr_t client_data, caddr_t call_data)
{
    wInvert (clickedTypew, 0, 0);

    if (w == resw) {
	Logic = 1;
    }
    else if (w == pltw) {
	Logic = 0;
    }

    wInvert (w, 0, 0);
    clickedTypew = w;
}

void ClickSim (Widget w, caddr_t client_data, caddr_t call_data)
{
    wInvert (clickedSimw, 0, 0);

    if (w == spicew) {
	Spice = 1;
    }
    else if (w == slsw) {
	Spice = 0;
    }

    wInvert (w, 0, 0);
    clickedSimw = w;
}

void ClickSimType (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (!(clickedSimTypew == w && w == slstimw))
	wInvert (clickedSimTypew, 0, 0);

    if (w == slslogw) {
	Spice = 0;
	Logic = (slslogtype == 'D');
	SimLevel = slsLogicLevel;
	/* somethingChanged = 1;  why would this be necessary ? AvG 10Dec92 */
    }
    else if (w == slstimw) {
	Spice = 0;
	Logic = (slstimtype == 'D');
	SimLevel = 3;
	/* somethingChanged = 1;  why would this be necessary ? AvG 10Dec92 */
    }
    else if (w == spicew) {
	Spice = 1;
	Logic = 0;
	SimLevel = 3;
    }

    if (!(clickedSimTypew == w && w == slstimw)) {
	wInvert (w, 0, 0);
	clickedSimTypew = w;
    }
}

void ClickSigType (Widget w, caddr_t client_data, caddr_t call_data)
{
    int n;
    char *labelvalue;

    if (w == spicetw) {
	     if (plato) { labelvalue = "spice"; plato = 0; }
	else if (pstar) { labelvalue = "plato"; plato = 1; pstar = 0; }
	else /* spice */{ labelvalue = "pstar"; pstar = 1; }
	w = spicew;
    }
    else if (w == slstimtw) {
	if (slstimtype == 'D')
	    labelvalue = "A";
	else
	    labelvalue = "D";
	slstimtype = labelvalue[0];

	if (slstimw == clickedSimTypew)
	    Logic = (slstimtype == 'D');
    }
    else {
	if (slslogtype == 'D')
	    labelvalue = "A";
	else
	    labelvalue = "D";
	slslogtype = labelvalue[0];

	if (slslogw == clickedSimTypew)
	    Logic = (slslogtype == 'D');
    }

    n = 0;
    XtSetArg (args[n], XtNlabel, labelvalue), n++;
    XtSetValues (w, args, n);
}

void wInvert (Widget w, caddr_t client_data, caddr_t call_data)
{
    int n;
    Pixel fg;
    Pixel bg;

    n = 0;
    XtSetArg (args[n], XtNbackground, &bg), n++;
    XtSetArg (args[n], XtNforeground, &fg), n++;
    XtGetValues (w, args, n);

    n = 0;
    XtSetArg (args[n], XtNbackground, fg), n++;
    XtSetArg (args[n], XtNforeground, bg), n++;
    XtSetValues (w, args, n);
}

void Read (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    readSignals (1, 0);
}

void Simulate (Widget w, caddr_t client_data, caddr_t call_data)
{
    char *simulator;
    char commandfile[128];
    char act_commandfile[128];
    char tmp_commandfile[128];
    char command[128];
    char buf[128];
    int u;
    int i;
    struct stat statbuf;

    if (clickedCommandw) endCommand ();

    if (editing) {
	if (somethingChanged) {
	    windowMessage ("finish edit mode first", -1);
	    return;
	}
	else {
	    /* First, execute 'Return' routine.
	       When this WRITE command is finished 'Simulate' will be called again in beginCommand.
	    */
	    simWasPressed = 1;
	    Return (NULL, NULL, NULL);
	    return;
	}
    }

    simWasPressed = 0;

    sprintf (commandfile, "%s.cmd", modNameOf (inputname));

    /* update commandfile for set commands, print commands, level etc. */

    updateCommandfile (inputname);

    /* and prepare for simulation */

    clear ();

    delSigList (Begin_signal); /* delete all signals */

    Nr_signals = 0;

    Begin_signal = NULL;
    End_signal = NULL;

    if (usedNonCapital) {
	sprintf (act_commandfile, "%s", commandfile);
	*act_commandfile += LOWER;
	sprintf (tmp_commandfile, "%d.cxx", (int)getpid ());
	sprintf (command, "mv %s %s", act_commandfile, tmp_commandfile);
	system (command);
	sprintf (command, "arrexp %s > %s", tmp_commandfile, act_commandfile);
	if (system (command)) {
	    windowMessage ("Cannot execute arrexp", -1);
	    goto end_of_simul;
	}
        sprintf (buf, ".... simulating (using commandfile \"%s\" instead; hit control-C to stop) ....", act_commandfile);
    }
    else
        strcpy (buf, ".... simulating (hit control-C to stop) ....");
    windowMessage (buf, -2);

    /* Clean up previous simulation result */

    if (Spice) {
	if (!plato) {
	    sprintf (buf, "%s.%s", inputname, pstar ? "list" : "ana");
	    if (stat (buf, &statbuf) == 0) unlink (buf);
	}
    }
    else {
	strcpy (buf, inputname);
	i = 0;
	while (buf[i] != '\0') i++;
	strcpy (&buf[i], ".res");
	if (stat (buf, &statbuf) == 0) unlink (buf);
	strcpy (&buf[i], ".plt");
	if (stat (buf, &statbuf) == 0) unlink (buf);
    }

    if (Spice) {
	if (plato)
	    simulator = platoCommand;
	else if (pstar)
	    simulator = pstarCommand;
	else
	    simulator = spiceCommand;
    }
    else {
        simulator = slsCommand;
    }

    sprintf (command, "%s", parseCommand (simulator));

    u = runTool (command, simdiagFile, (!Spice && s_simperiod > 0));
    if (u == 0) {
	windowMessage ("simulation ready", -1);
    }
    else if (u == -1) {
	windowMessage ("simulation aborted", -1);
    }
    else if (u == 1) {
	FILE *fp, *fp2;
	int c, n = 0;

	fp = fopen (simdiagFile, "r");
	if (fp)
	    while ((c = fgetc (fp)) != EOF)
		if (c == '\n' && ++n > 20) break;
	if (n > 20 && (fp2 = fopen (simdiagFile2, "w"))) {
	    fprintf (fp2, "File %s contains more than 20 lines!\n", simdiagFile);
	    fprintf (fp2, "For support, contact space-support-ewi@tudelft.nl\n");
	    fprintf (fp2, "The first and last 3 lines of file %s are:\n", simdiagFile);
	    rewind (fp);
	    u = 0;
	    while ((c = fgetc (fp)) != EOF) {
		fprintf (fp2, "%c", c);
		if (c == '\n' && ++u > 2) break;
	    }
	    fprintf (fp2, "...\n");
	    fclose (fp2);
	    sprintf (command, "tail -3 %s >> %s", simdiagFile, simdiagFile2);
	    system (command);
	}
	if (fp) fclose (fp);

	if (n > 20)
	    windowList (simdiagFile2);
	else
	    windowList (simdiagFile);
    }
    /* else u == 2 and a window message has been printed */

end_of_simul:
    if (usedNonCapital) { /* restore actual commandfile */
	sprintf (command, "mv %s %s", tmp_commandfile, act_commandfile);
	system (command);
    }
    usedNonCapital = 0;
}

void Windowmax (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    /* If necessary, clear will be executed in draw */

    draw ('m', 0, 0, 0, 0);
}

void Savewin (Widget w, caddr_t client_data, caddr_t call_data)
{
    char buf[80];
    FILE *fp;
    struct signal *sig;

    if (clickedCommandw) endCommand ();

    if ((fp = fopen (parseCommand (settingsFile), "w")) == NULL) {
	sprintf (buf, "Cannot write %s", parseCommand (settingsFile));
	windowMessage (buf, -1);
	return;
    }

    fprintf (fp, "t_start: %lld\n", startTime);
    fprintf (fp, "t_stop: %lld\n", stopTime);
    sig = Top_signal;
    while (sig && sig -> prev != Bottom_signal) {
	fprintf (fp, "sig: %s\n", sig -> name);
	sig = sig -> next;
    }

    fclose (fp);

    windowMessage ("Settings saved", -1);
}

void Loadwin (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    if (loadSettings ()) {

	clear ();

	draw ('a', 0, 0, 0, 0);

	windowMessage ("Settings loaded", -1);
    }
}

void Zoomin (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    commandType = ZOOMIN;
    beginCommand (w);
}

void Zoomout (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    commandType = ZOOMOUT;
    beginCommand (w);
}

void Redraw (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    clear ();

    draw (0, 0, 0, 0, 0);
}

void Move (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    /* If necessary, clear will be executed in draw */

	 if (w == leftw)  draw ('l', 0, 0, 0, 0);
    else if (w == rightw) draw ('r', 0, 0, 0, 0);
    else if (w == upw)    draw ('u', 0, 0, 0, 0);
    else if (w == downw)  draw ('d', 0, 0, 0, 0);
}

void Pointer (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    commandType = MEASURE;
    beginCommand (w);
}

void Shuffle (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    commandType = SHUFFLE;
    beginCommand (w);
}

void Hardcopy (Widget w, caddr_t client_data, caddr_t call_data)
{
    char command[128];
    Window win;

    if (clickedCommandw) endCommand ();

    win = XtWindow (canvas);

    if (printLabel && printLabel[0] != '\0') {
	windowMessage (parseCommand (printLabel), -2);
    }

    sprintf (command, "xwd -id 0x%lx -out %s 1> pr.diag 2>&1", win, parseCommand (xdumpFile));

    /* redirect both 'stdout' and 'stderr' to pr.diag */

    /* Printing is not done with runTool since more than one
       program may be called in one command.
    */

    if (system (command) == 0) {

        if (printCommand && printCommand[0] != '\0') {
	    sprintf (command, "%s 1> pr.diag 2>&1", parseCommand (printCommand));

	    windowMessage (".... printing ....", -2);

	    if (system (command) == 0) {
		windowMessage ("print ready", -1);
	    }
	    else {
		windowList ("pr.diag");
	    }
	}
	else {
	    windowMessage ("print ready", -1);
	}
    }
    else {
	windowList ("pr.diag");
    }
}

void Inputs (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    clear ();

    Append = 0;

    if (readSet (inputname) == 0) {
	if (readSetMes[0]) windowMessage (readSetMes, -1);
	return;
    }

    if (!editing) {
	enableEditing ();
	Cmd = 1;
    }

    currLogic = 1;

    draw ('f', 0, 0, 0, 0);

    if (readSetMes[0]) windowMessage (readSetMes, -1);
}

void Quit (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    if (editing && somethingChanged) {
        commandType = QUITP;
	beginCommand (w);
    }
    else
	die (0);
}

int readSignals (int flag, int intermediate)
/* flag = 1 : check if shiftMask is pressed
** intermediate = 0  : no intermediate read
**              = 2  : start intermediate read
**              = 1  : intermediate read
**              = -1 : end intermediate read
** return = 0 : error
**        = 1 : ok
**        = 2 : ok, but message printed
*/
{
    Window root, child;
    Grid root_x, root_y, win_x, win_y;
    unsigned int mask;

    if (editing) {
	windowMessage ("no read in edit mode", -1);
	return (0);
    }

    if (flag)
	XQueryPointer (display, window, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);
    else
	mask = 0;

    if ((mask & ShiftMask) && Begin_signal)
	Append = 1;
    else
	Append = 0;

    if (intermediate == 0) clear ();

    if (Append && Logic != currLogic) {
	windowMessage ("Cannot merge logical and analog signals", -1);
	return (0);
    }

    if (editing && !Logic) {
	windowMessage ("Cannot edit analog signals", -1);
	return (0);
    }

    if (Logic) {
	if (Spice) {
	    windowMessage ("No logic signals for spice", -1);
	    return (0);
	}
	else {
	    if (readLogic (inputname, intermediate) == 0)
		return (0);
	}
    }
    else {
	if (Spice) {
	    if (plato) {
		if (readPlato (inputname) == 0) return (0);
	    }
	    else
		if (readSpice (inputname) == 0) return (0);
	}
	else {
	    if (readWaves (inputname, intermediate) == 0)
		return (0);
	}
    }

    Cmd = 0;

    currLogic = Logic;

    if (intermediate && s_simperiod > 0 && s_sigunit > 0) {
	Endtime = s_simperiod * s_sigunit / Timescaling;
    }

    if (UseSettings && (intermediate == 0 || intermediate == 2)) {
	if (loadSettings ()) {
            if (intermediate == 0)
                draw ('a', 0, 0, 0, 0);
            else
                draw ('b', 0, 0, 0, 0);
	    if (!Spice && intermediate == 0) {
		if (checkRaces ()) return (2);
	    }
        }
	else {
	    draw ('f', 0, 0, 0, 0);
	    windowMessage (lastErrMess, -1);
	    return (0);
	}
    }
    else {
        if (intermediate == 0)
            draw ('f', 0, 0, 0, 0);
        else if (intermediate == 2)
            draw ('s', 0, 0, 0, 0);
	else
	    draw ('t', 0, 0, 0, 0);
	if (!Spice && (intermediate == 0 || intermediate == -1)) {
	    if (checkRaces ()) return (2);
	}
    }

    return (1);
}

int checkRaces ()               /* return value: 1 message printed */
{                               /*             : 0 no message printed */
    int x;
    char simoutfile[132];
    char buf[132];
    char c;
    FILE *fp;

    sprintf (simoutfile, "%s.out", inputname);

    if ((fp = fopen (simoutfile, "r")) == NULL) {
	sprintf (buf, "Warning: cannot open %s", simoutfile);
	windowMessage (buf, -1);
	return (1);
    }

    x = 0;
    while ((c = getc (fp)) != EOF) {
	if (x == 0 && c == 'r') x++;
	else if (x == 1 && c == 'a') x++;
	else if (x == 2 && c == 'c') x++;
	else if (x == 3 && c == 'e') x++;
	else if (x == 4 && c == 's') x++;
	else if (x == 5 && c == ' ') x++;
	else if (x == 6 && c == 'o') x++;
	else if (x == 7 && c == 'c') x++;
	else if (x == 8 && c == 'c') x++;
	else x = 0;
	if (x == 9) break;
    }

    fclose (fp);

    if (x == 9) {
	sprintf (buf, "Warning: races occurred (see file '%s')", simoutfile);
	windowMessage (buf, -1);
	return (1);
    }

    return (0);
}

void clear ()
{
    Display *display;
    Window window;

    window = XtWindow (canvas);
    display = XtDisplay (canvas);
    XClearWindow (display, window);
}

int loadSettings ()
{
    FILE *fp;
    char buf[128];
    char c;
    int error;
    char name[128];
    simtime_t newStartTime;
    simtime_t newStopTime;
    struct signal *sig;
    struct signal *newTop_signal;
    struct signal *newBottom_signal;
    int newCurr_nr_signals;

    if ((fp = fopen (parseCommand (settingsFile), "r")) == NULL) {
	sprintf (buf, "Cannot read %s", parseCommand (settingsFile));
	strcpy (lastErrMess, buf);
	windowMessage (buf, -1);
	return (0);
    }

    newTop_signal = NULL;
    newBottom_signal = NULL;
    newCurr_nr_signals = 0;

    while (fscanf (fp, "%s", buf) > 0) {

	error = 0;

	if (buf[0] == '#') {        /* comment, skip until end of line */
	    while ((c = getc (fp)) != '\n' && c != EOF);
	}

	     if (strsame (buf, "spice:")) { }
	else if (strsame (buf, "simlevel:")) { }
	else if (strsame (buf, "level3logic:")) { }
	else if (strsame (buf, "t_start:")) {
	    if (fscanf (fp, "%lld", &newStartTime) <= 0) error = 1;
	}
	else if (strsame (buf, "t_stop:")) {
	    if (fscanf (fp, "%lld", &newStopTime) <= 0) error = 1;
	}
	else if (strsame (buf, "sig:")) {
	    if (fscanf (fp, "%s", name) <= 0) error = 1;
	    else {
		for (sig = (newBottom_signal ?  newBottom_signal -> next : Begin_signal); sig; sig = sig -> next) {
		    if (strsame (sig -> name, name)) break;
		}
		if (sig) {
		    if (newTop_signal == NULL) {
			newTop_signal = sig;
			newBottom_signal = sig;
			if (sig != Begin_signal) {
			    if (sig == End_signal) End_signal = sig -> prev;
			    if (sig -> prev) sig -> prev -> next = sig -> next;
			    if (sig -> next) sig -> next -> prev = sig -> prev;
			    sig -> prev = NULL;
			    sig -> next = Begin_signal;
			    Begin_signal -> prev = sig;
			    Begin_signal = sig;
			}
		    }
		    else if (sig != newBottom_signal) {
			if (newBottom_signal -> next != sig) {
			    if (sig == End_signal)   End_signal = sig -> prev;
			    if (sig == Begin_signal) Begin_signal = sig -> next;
			    if (sig -> prev) sig -> prev -> next = sig -> next;
			    if (sig -> next) sig -> next -> prev = sig -> prev;
			    sig -> prev = newBottom_signal;
			    sig -> next = newBottom_signal -> next;
			    if (newBottom_signal -> next)
				newBottom_signal -> next -> prev = sig;
			    else
				End_signal = sig;
			    newBottom_signal -> next = sig;
			}
			newBottom_signal = sig;
		    }
		    newCurr_nr_signals++;
		}
	    }
	}
	else {
	    rcReadError (parseCommand (settingsFile), buf);
	}

	if (error) rcReadError (parseCommand (settingsFile), buf);

	while ((c = getc (fp)) != '\n' && c != EOF);

	/* skip everything until end of line */
    }

    fclose (fp);

    if (newCurr_nr_signals == 0) {
	windowMessage ("Settings do not refer to current signals", -1);
	strcpy (lastErrMess, "Settings do not refer to current signals");
	return (0);
    }

    startTime = newStartTime;
    stopTime = newStopTime;

    if (startTime <  Begintime) startTime = Begintime;
    if (startTime >= Endtime)   startTime = Endtime - 1;
    if (stopTime  >  Endtime)   stopTime  = Endtime;
    if (stopTime  <= Begintime) stopTime  = Begintime - 1;
    if (startTime >= stopTime)  startTime = stopTime - 1;

    Top_signal = newTop_signal;
    Bottom_signal = newBottom_signal;
    Curr_nr_signals = newCurr_nr_signals;

    curr_umin = Global_umin;
    curr_umax = Global_umax;

    return (1);
}

int myscanf (FILE *fp, char *s)
{
    int c, i, nonspace;

    while ((c = getc (fp)) == ' ' || c == '\t');

    i = 0;
    nonspace = -1;
    if (c != EOF && c != '\n') {
	while (c != EOF && c != '\n') {
	    s[i] = c;
	    if (c != ' ' && c != '\t') nonspace = i;
	    c = getc (fp);
	    i++;
	}
    }

    s[nonspace + 1] = '\0';

    ungetc (c, fp);

    if (i > 0) return (1);
    if (c == EOF) return (-1);
    return (0);
}

void readrcFile ()
{
    char buf[256];
    char c;
    FILE *fp;
    char *fn;
    char *home;

    fn = rcFile;
    if (!(fp = fopen (fn, "r"))) {
	if ((home = getenv ("HOME"))) {
	    NEW (fn, strlen (home) + strlen (rcFile) + 2, char);
	    if (fn) {
		sprintf (fn, "%s/%s", home, rcFile);
		fp = fopen (fn, "r");
	    }
	}
	if (!fp && projKey) {
	    if ((fn = (char *)dmGetMetaDesignData (PROCPATH, projKey, rcFile)))
		fp = fopen (fn, "r");
	}
	if (!fp) return;
    }

    while (fscanf (fp, "%s", buf) > 0) {

	if (buf[0] == '#') {
	    /* comment */
	}
	else if (strsame (buf, "SLS:")) {
	    if (myscanf (fp, buf) > 0) {
		NEW (slsCommand, strlen (buf) + 1, char);
		strcpy (slsCommand, buf);
	    }
	    else {
		rcReadError (fn, "SLS:");
	    }
	}
	else if (strsame (buf, "SPICE:")) {
	    if (myscanf (fp, buf) > 0) {
		NEW (spiceCommand, strlen (buf) + 1, char);
		strcpy (spiceCommand, buf);
	    }
	    else {
		rcReadError (fn, "SPICE:");
	    }
	}
	else if (strsame (buf, "PSTAR:")) {
	    if (myscanf (fp, buf) > 0) {
		NEW (pstarCommand, strlen (buf) + 1, char);
		strcpy (pstarCommand, buf);
	    }
	    else {
		rcReadError (fn, "PSTAR:");
	    }
	}
	else if (strsame (buf, "PLATO:")) {
	    if (myscanf (fp, buf) > 0) {
		NEW (platoCommand, strlen (buf) + 1, char);
		strcpy (platoCommand, buf);
	    }
	    else {
		rcReadError (fn, "PLATO:");
	    }
	}
	else if (strsame (buf, "PRINT:")) {
	    if (myscanf (fp, buf) > 0) {
		NEW (printCommand, strlen (buf) + 1, char);
		strcpy (printCommand, buf);
	    }
	    else {
		rcReadError (fn, "PRINT:");
	    }
	}
	else if (strsame (buf, "PRINT_LABEL:")) {
	    if (myscanf (fp, buf) > 0) {
		NEW (printLabel, strlen (buf) + 1, char);
		strcpy (printLabel, buf);
	    }
	    else {
		rcReadError (fn, "PRINT_LABEL:");
	    }
	}
	else if (strsame (buf, "SLS_LOGIC_LEVEL:")) {
	    if (fscanf (fp, "%d", &slsLogicLevel) > 0) {
		if (slsLogicLevel < 1) slsLogicLevel = 1;
		else if (slsLogicLevel > 2) slsLogicLevel = 2;
	    }
	    else {
		rcReadError (fn, "SLS_LOGIC_LEVEL:");
	    }
	}
	else if (strsame (buf, "SLS_LOGIC_SIGNAL:")) {
	    if (fscanf (fp, "%s", buf) > 0) {
		if (buf[0] == 'D') slslogtype = 'D';
		else if (buf[0] == 'A') slslogtype = 'A';
	    }
	    else {
		rcReadError (fn, "SLS_LOGIC_SIGNAL:");
	    }
	}
	else if (strsame (buf, "SLS_TIMING_SIGNAL:")) {
	    if (fscanf (fp, "%s", buf) > 0) {
		if (buf[0] == 'D') slstimtype = 'D';
		else if (buf[0] == 'A') slstimtype = 'A';
	    }
	    else {
		rcReadError (fn, "SLS_TIMING_SIGNAL:");
	    }
	}
	else if (strsame (buf, "XDUMP_FILE:")) {
	    if (fscanf (fp, "%s", buf) > 0) {
		NEW (xdumpFile, strlen (buf) + 1, char);
		strcpy (xdumpFile, buf);
	    }
	    else {
		rcReadError (fn, "XDUMP_FILE:");
	    }
	}
	else if (strsame (buf, "SETTINGS_FILE:")) {
	    if (myscanf (fp, buf) > 0) {
		NEW (settingsFile, strlen (buf) + 1, char);
		strcpy (settingsFile, buf);
	    }
	    else {
		rcReadError (fn, "SETTINGS_FILE:");
	    }
	}
	else if (strsame (buf, "DETAIL_ZOOM_ON")) {
	    doDetailZoom = 1;
	}
	else if (strsame (buf, "TRY_NON_CAPITAL_ON")) {
	    tryNonCapital = 1;
	}
	else {
	    rcReadError (fn, buf);
	}

	while ((c = getc (fp)) != '\n' && c != EOF);
	if (c == EOF) break;

	/* skip everything until end of line */
    }

    fclose (fp);
}

void rcReadError (char *fn, char *s)
{
    fprintf (stderr, "Error in file '%s' for '%s'\n", fn, s);
    die (1);
}

double slstof (char *s)
{
    char buf[256];
    double val = -1;
    int c, len;

    strcpy (buf, s);
    len = strlen (buf);

    c = buf[len-1];
    if (isdigit (c)) val = atof (buf);
    else {
	buf[len-1] = '\0';
	switch (c) {
	case 'G': val = 1.0e9   * atof (buf); break;
	case 'M': val = 1.0e6   * atof (buf); break;
	case 'k': val = 1.0e3   * atof (buf); break;
	case 'm': val = 1.0e-3  * atof (buf); break;
	case 'u': val = 1.0e-6  * atof (buf); break;
	case 'n': val = 1.0e-9  * atof (buf); break;
	case 'p': val = 1.0e-12 * atof (buf); break;
	case 'f': val = 1.0e-15 * atof (buf); break;
	}
    }
    return (val);
}

char *parseCommand (char *s)
{
    struct tm *s_tm;
    time_t timer;
    static char buf[750];
    char help[20];
    register char *p, *t;
    int cell_cnt = 0;

    s_tm = 0;

    p = buf;
    while (*s) {
    if (*s == '$') {
	++s;
	if (!strncmp (s, "cell", 4)) {

	    cell_cnt++;

	    t = inputname;
	    if (usedNonCapital && cell_cnt == 2) *p++ = (*t++) + LOWER; /* decapitalize 1st char */
	    while (*t) *p++ = *t++;
	    s += 4;
	}
	else if (!strncmp (s, "date", 4) || !strncmp (s, "time", 4)) {

	    if (!s_tm) {
		time (&timer);
		s_tm = localtime (&timer);
	    }
	    if (*s == 't')
		sprintf (help, "%02d:%02d:%02d", s_tm -> tm_hour, s_tm -> tm_min, s_tm -> tm_sec);
	    else {
		switch (s_tm -> tm_mon) {
		case  0: t = "Jan"; break;
		case  1: t = "Feb"; break;
		case  2: t = "Mar"; break;
		case  3: t = "Apr"; break;
		case  4: t = "May"; break;
		case  5: t = "Jun"; break;
		case  6: t = "Jul"; break;
		case  7: t = "Aug"; break;
		case  8: t = "Sep"; break;
		case  9: t = "Oct"; break;
		case 10: t = "Nov"; break;
		default: t = "Dec";
		}
		sprintf (help, "%02d %s %4d", s_tm -> tm_mday, t, s_tm -> tm_year + 1900);
	    }
	    for (t = help; *t;) *p++ = *t++;
	    s += 4;
	}
	else if (!strncmp (s, "file", 4)) {

	    t = inputname;
	    if (usedNonCapital) *p++ = (*t++) + LOWER; /* decapitalize 1st char */
	    while (*t) *p++ = *t++;
	    if (editing) {
		strcpy (p, ".cmd");
	    }
	    else if (Spice) {
		     if (plato) strcpy (p, ".sig");
		else if (pstar) strcpy (p++, ".list");
		else /* spice */strcpy (p, ".ana");
	    }
	    else if (Logic) {
		strcpy (p, ".res");
	    }
	    else {
		strcpy (p, ".plt");
	    }
	    p += 4;
	    s += 4;
	}
	else if (!strncmp (s, "user", 4)) {

	    if ((t = username)) while (*t) *p++ = *t++;
	    s += 4;
	}
	else {
	    *p++ = '$';
	}
    }
    else
	*p++ = *s++;
    }
    *p = '\0';

    return (buf);
}

char *modNameOf (char *name)
{
    static char buf[264];
    register char *s = name;
    register char *t = buf;

    while (*s && *s != '#') *t++ = *s++;
    *t = 0;
    return (buf);
}

/*
** Return codes of fork
*/
#define CHILD 0
#define PARENT 1:default
#define ERROR -1

static int pgid = 0;
static int childpid = 0;
static int childRunning = 0;
static int toolReadyStatus;

int runTool (char *s, char *outFile, int monitorSig)
/* monitorSig = 0 : draw final result
**              1 : draw intermediate and final result
**             -1 : do not draw result
*/
{
    int v, w;
    int argno = 0;
    char *argv[16];
    char *path = NULL;
    char buf[132];
    char *p;
    int readSigMes;
    XEvent *ev;
    XEvent event;
    XEvent leave_event;
    int left;
    int probe_time = 3; /* at every 'probe_time' seconds new simulation results are displayed */
    int mon_cnt;

    strcpy (buf, s);
    p = &buf[0];

    while (*p != '\0') {
	if (!path) path = p;

	if (argno < 16)
	    argv[argno++] = p;
	else
	    break;

	while (*p != ' ' && *p != '\0') p++;
	if (*p == ' ') *(p++) = '\0';
	while (*p == ' ') p++;
    }
    argv[argno] = NULL;

    v = vfork ();

    switch (v) {
        case PARENT:
	    pgid = getpgrp ();
	    childpid = v;
	    childRunning = 1;
	    toolReadyStatus = 1;
	    signal (SIGCHLD, int_hdl);
	    readSigMes = 2;
	    left = 0;
	    mon_cnt = 0;
	    while (childRunning) {

		/* This loop may be exited in three ways:
		   1. The child has finished and childRunning has become 0
		      (due to a SIGCHLD interrupt).
		   2. An event occurred for which checkEvent returned
		      True, e.g. '^C' was typed.
		   3. A fatal IO error occurred (destroyHandler was called
		      when XSync was executed) because the window of simeye
		      has been destroyed.
		*/

		sleep (1);

		/* test on window existance, if window does no longer exists
		   a jump to the destroyHandler will automatically be done */

		XSync (display, False);

		while (XEventsQueued (display, QueuedAfterFlush) > 0) {
		    if (left)
			ev = &event;
		    else
			ev = &leave_event;
		    XNextEvent (display, ev);
		    if (ev -> type == Expose) {
			XtDispatchEvent (ev);   /* for redrawing menu buttons */
		    }
		    if (ev -> type == ConfigureNotify) {
			XtDispatchEvent (ev);   /* for resizing widgets */
		    }
		    if (ev -> type == LeaveNotify && !left) {
			left = 1;
		    }
		    if (checkEvent (display, ev)) {

			/* the tool is stopped by an interrupt from simeye */
			/* kill all processes of the process group, except simeye itself */

			signal (SIGTERM, SIG_IGN);
			kill (-pgid, SIGTERM);
			signal (SIGTERM, int_hdl);

			toolReadyStatus = -1;
			break;
		    }
		}

		mon_cnt++;
		if (monitorSig == 1 && childRunning && mon_cnt == probe_time) {
		    if (readSignals (0, readSigMes)) {
			readSigMes = 1;
		    }
		    mon_cnt = 0;
		}
	    }
	    if (left > 0) {
		XPutBackEvent (display, &leave_event); /* for updating RUN button */
	    }
	    w = 0;
	    if (monitorSig != -1 && errorInFile (outFile))
		toolReadyStatus = 1;
	    else if (monitorSig == 1 && readSigMes == 1)
		w = readSignals (0, -1);  /* end intermediate reading */
	    else if (monitorSig != -1)
		w = readSignals (0, 0);   /* normal (single) read */
	    if (toolReadyStatus == 0 && w == 2)
		toolReadyStatus = 2;
            break;
        case CHILD:
	    /* attach stdout and stderr to "outFile" */
	    freopen (outFile, "w", stdout);
	    freopen (outFile, "w", stderr);
            execvp (path, argv);
	    fprintf (stderr, "Cannot execute %s\n", path);
	    fflush (stderr);
            _exit (1);
            break;
        case ERROR:
            return (1);
            break;
    }

    return (toolReadyStatus);
}

int errorInFile (char *fn)
{
     FILE *fp;
     char buf[256];
     int simulation = 0;

     if ((fp = fopen (fn, "r"))) {
	 while (fscanf (fp, "%s", buf) > 0) {
	     if (simulation && strsame (buf, "aborted")) {
		 fclose (fp);
		 return (1);
	     }
	     if (strsame (buf, "simulation")) simulation = 1;
	     else simulation = 0;
	 }
	 fclose (fp);
	 return (0);   /* OK */
     }
     return (1);
}

int checkEvent (Display *display, XEvent *event)
{
    char buf[32];
    int bufnr = 32;
    KeySym keysym;
    XComposeStatus status;
    XKeyEvent *ev = (XKeyEvent *)event;

    if (event -> type == KeyPress) {
	buf[0] = '\0';
	XLookupString (ev, buf, bufnr, &keysym, &status);

        if (buf[0] == '\003'  /* ^C */
	|| (buf[0] == 'c' && (ev -> state & ControlMask))) return (1);
    }
    return (0);
}

void initIntrup ()
{
    if (signal (SIGINT, SIG_IGN) != SIG_IGN) signal (SIGINT, int_hdl);
        /* only when value was not SIG_IGN, a jump must be done to int_hdl */
    if (signal (SIGQUIT, SIG_IGN) != SIG_IGN) signal (SIGQUIT, int_hdl);
        /* only when value was not SIG_IGN, a jump must be done to int_hdl */
    signal (SIGTERM, int_hdl);
    signal (SIGILL,  int_hdl);
    signal (SIGFPE,  int_hdl);
#ifdef SIGBUS
    signal (SIGBUS,  int_hdl);
#endif
    signal (SIGSEGV, int_hdl);
#if defined(SIGALRM)
/* for some unknown reason, we receive unexpected SIGALRMs on linux ... */
    signal (SIGALRM, int_hdl);
#endif
}

void int_hdl (int sig) /* interrupt handler */
{
    int status;

    switch (sig) {
        case SIGILL :
            fprintf (stderr, "Illegal instruction\n");
            break;
        case SIGFPE :
            fprintf (stderr, "Floating point exception\n");
            break;
#ifdef SIGBUS
        case SIGBUS :
            fprintf (stderr, "Bus error\n");
            break;
#endif
        case SIGSEGV :
            fprintf (stderr, "Segmentation violation\n");
            break;
	case SIGCHLD :
	    signal (SIGCHLD, SIG_DFL);
	    if (childRunning) {
		(void) wait (&status);
		if (toolReadyStatus != -1)
		    toolReadyStatus = (status? 1: 0);
		/* else it was 'stopped' from simeye */
		childRunning = 0;
	    }
	    break;
#ifdef __linux
        case SIGALRM:
            signal (SIGALRM, int_hdl);
            return; /* just ignore this alarm signal (where does it come from ????) */
#endif
        default :
	    if (childRunning > 0) {
		childRunning = 0;
		/* kill all processes with the same process group id */
		kill (-pgid, SIGTERM);
	    }
            break;
    }

    if (sig != SIGCHLD) die (1);
}

void die (int status)
{
    if (status == 0 && projKey) dmCloseProject (projKey, COMPLETE);
    dmQuit ();
    exit (status);
}

void die_alloc ()
{
    fprintf (stderr, "%s: Cannot allocate storage\n", argv0);
    die (1);
}

#ifdef STATIC
/* libX11.a fix for static linking */
void *dlopen (const char *filename, int flag)
{
    return NULL;
}
#endif
