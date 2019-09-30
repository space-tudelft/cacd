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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include "src/libddm/dmincl.h"
#include "src/simeye/define.h"
#include "src/simeye/type.h"
#include "src/simeye/extern.h"
#include "src/libX2ps/X2pslib.h"
#include "src/libX2ps/X2PostScript.h"

#define addCallback(w, name, proc, data) XtAddCallback (w, name, (XtCallbackProc)proc, (XtPointer)data)

#ifndef DM_MAXPATHLEN
#define DM_MAXPATHLEN 1024
#endif

#define TOOLVERSION "4.2.5 (21 Dec 2016)"

/* #define CORE  *//*coredump wanted for debugger*/
/* #define DEBUG *//*Expose*/

#define MAXLIST 200

#define infoMessage(s) startDialog (iDialog, s)

extern Widget XtCreateWidget();
extern Widget XmCreateDialogShell();
extern Widget XmCreateCascadeButton ();
extern Widget XmCreateErrorDialog();
extern Widget XmCreateForm();
extern Widget XmCreateFormDialog();
extern Widget XmCreateInformationDialog();
extern Widget XmCreateLabel ();
extern Widget XmCreateMainWindow();
extern Widget XmCreateMenuBar();
extern Widget XmCreateOptionMenu();
extern Widget XmCreatePromptDialog();
extern Widget XmCreatePulldownMenu();
extern Widget XmCreatePushButton ();
extern Widget XmCreateQuestionDialog();
extern Widget XmCreateScrollBar();
extern Widget XmCreateScrolledList();
extern Widget XmCreateScrolledWindow();
extern Widget XmCreateSelectionDialog();
extern Widget XmCreateSeparator();
extern Widget XmCreateTextField();
extern Widget XmCreateToggleButton ();
extern Widget XmMessageBoxGetChild ();
extern Widget XmSelectionBoxGetChild ();

extern Boolean XmStringGetLtoR ();
extern char *XmTextFieldGetString ();
extern void  XmTextFieldSetString ();
extern void  XmListDeleteAllItems ();
extern void  XmListSelectPos ();
extern void  XmListSetPos (Widget widget, int position);

void build_Dialogs      (Widget toplevel);
void build_Edit_menu    (Widget commands);
void build_File_menu    (Widget commands);
void build_Help_menu    (Widget commands);
void build_MainWindow   (Widget toplevel);
void build_Options_menu (Widget commands);
void build_psprDialog   (Widget toplevel);
void build_Simu_menu    (Widget commands);
Widget build_StatusArea (Widget WorkRegion);
void build_View_menu    (Widget commands);
void CancelCB (Widget w, caddr_t client_data, caddr_t call_data);
Bool checkEvent (XEvent *event);
int  checkRaces (void);
int  destroyHandler (Display *display);
void DetailZoomCB (Widget w, caddr_t clientData, caddr_t callData);
int  errorInFile (char *fn);
void Hardcopy (Widget w, caddr_t done, caddr_t call_data);
void HelpCB (Widget w, caddr_t clientData, caddr_t callData);
void InfoCB (Widget w, int mode, caddr_t callData);
void initIntrup (void);
void Inputs (Widget w, int done, caddr_t call_data);
void int_hdl (int sig);
int  loadSettings (void);
void Loadwin (Widget w, caddr_t client_data, caddr_t call_data);
void LoadWinOption (Widget w, caddr_t clientData, caddr_t callData);
void MoveCB (Widget w, caddr_t client_data, caddr_t call_data);
void OkCB   (Widget w, caddr_t client_data, caddr_t call_data);
void OpenOkCB (Widget w, int ok, caddr_t call_data);
char *parseCommand (char *s);
void Pointer  (Widget w, caddr_t client_data, caddr_t call_data);
void Print    (Widget w, caddr_t client_data, caddr_t call_data);
void qDialogCB  (Widget w, char *client_data, caddr_t call_data);
void rcReadError (char *fn, char *s);
void Read   (Widget w, int done, caddr_t call_data);
void ReadOkCB (Widget w, int ok, caddr_t call_data);
void readrcFile (void);
int  readSignals (int flag, int intermediate);
void readTypeCB (Widget w, caddr_t client_data, caddr_t call_data);
void Redraw   (Widget w, caddr_t client_data, caddr_t call_data);
void redrawHandler (Widget w, caddr_t client_data, XEvent *event);
void resetLineAttr (void);
int  runTool (char *s, int monitorSig);
void SaveAsCB (Widget w, caddr_t client_data, caddr_t call_data);
void SaveEditCB (Widget w, caddr_t clientData, caddr_t callData);
void Savewin  (Widget w, caddr_t client_data, caddr_t call_data);
void set_clip_area (XRectangle R[], int nr);
void set_input_mode (void);
void setLineAttr (void);
void Simulate (Widget w, int done, caddr_t call_data);
void SimuOkCB (Widget w, int ok, caddr_t call_data);
void simuReadCB (Widget w, caddr_t set_value, caddr_t call_data);
void simuTypeCB (Widget w, caddr_t client_data, caddr_t call_data);
void SliderCB  (Widget w, caddr_t clientData, caddr_t callData);
void startDialog (Widget d, char *s);
int  updateCommandfile (char *cname, char *sname);
void Windowmax (Widget w, caddr_t client_data, caddr_t call_data);
void Zoomin    (Widget w, caddr_t client_data, caddr_t call_data);
void Zoomout   (Widget w, caddr_t client_data, caddr_t call_data);

extern Grid canvasHeight;
extern Grid canvasWidth;
extern char icdpath[];
extern char fn_err[];

char *argv0 = "simeye";
char *usg_msg = "\nUsage: %s [X options] [-IRLehv] [-l|-m|-c|-P] [-a|-b] [name]\n\n";

/* Some commands and variables and their defaults.
** The default values may be overwitten in the file ".simeyerc".
*/

int slsLogicLevel = 2;

char *slsCommand   = "sls $circuit $stimuli";
char *spiceCommand = "nspice $circuit $stimuli";
char *pstarCommand = "npstar $circuit $stimuli";
char *simdiagFile  = "sim.diag";
char *simdiagFile2 = "sim.diag2";
char *xdumpFile    = "simeye.wd";
char *printCommand = "xpr -device ps -output $cell.ps simeye.wd;lp $cell.ps;rm simeye.wd $cell.ps";
char *printLabel   = "$user  $file  $date  $time";
char *settingsFile = "simeye.set";
char *rcFile       = ".simeyerc";
char *defCmdFile   = "simeye.def.d";

XmString E_str;
XmString V_str;
XmString N_str;

Widget FileInfo;
Widget FileText;
Widget StatusLabel;
Widget SimuCmd;
Widget RunInfo;
Widget ADInfo;

/* Edit menu: */
Widget GridButton;
Widget EraseAllButton;
Widget DeleteButton;
Widget CopyButton;
Widget ChangeButton;
Widget AddButton;
Widget YankButton;
Widget PutButton;
Widget RenameButton;
Widget SpeedButton;
Widget TendButton;

Widget ExitButton;
Widget NewButton;
Widget HorBar, VerBar;
Widget OpenButton;
Widget openOkButton;
Widget HardcButton;
Widget PrintButton;
Widget EditButton;
Widget RunButton;
Widget SimuButton;
Widget SaveButton;
Widget SaveAsButton;
Widget ReadButton;
Widget ReadAppButton;
Widget canvas;
Widget openDialog;
Widget openList;
Widget openSelect;
Widget readDialog;
Widget readList;
Widget readSelect;
Widget readType;
Widget readTypeCur, readTypeTmp;
Widget readTypeOpt[4];
Widget simuDialog;
Widget cirList;
Widget cmdList;
Widget cirSelect;
Widget cmdSelect;
Widget simuType;
Widget simuRead;
Widget simuTypeCur, simuTypeTmp;
Widget simuReadCur, simuReadTmp;
Widget simuTypeOpt[4];
Widget simuReadOpt[2];
Widget eDialog;
Widget iDialog;
Widget pDialog;
Widget qDialog;
Widget cW, pW, qW, txtW;
Widget clickedCommandw;
Widget Toplevel;

Colormap colormap;
Pixel std_bg;
Pixel std_red;

#define ERRMAX 506
char errbuf[ERRMAX+6];
static int errlen = 0;
static int question = 0;

static int sim_type_nr;
static int sim_read_nr;

static int NOdm_msg = 0;
static int firstExpose = 1;
static int keymn = 1;
static int Logic;
int Spice;
int currLogic;      /*    for current contents  */
int Append;
int SimLevel;
int UseSettings;
int SaveEdit = 0;
int doDetailZoom;
int tryNonCapital;
int usedNonCapital = 0;
int old_spice_mode = 0;
char slslogtype;
char slstimtype;

struct signal *Begin_signal = NULL;
struct signal *End_signal   = NULL;
int Nr_signals = 0;

simtime_t Begintime = 0;
simtime_t Endtime = 0;
simtime_t SimEndtime = 0;
double Timescaling = 0;
double Voltscaling = 0;
int Global_umin;
int Global_umax;

static int Slen = 0;
static char Sfile[MAXNAME+MAXNAME];
static char *username;

char readTypeSel[4][MAXNAME+2];
char circuitname[MAXNAME+2];
char stimuliname[MAXNAME+2];
char inputname[MAXNAME+2];
char outputname[MAXNAME+2];

Grid stdCanvasWidth  = 628;
Grid stdCanvasHeight = 418;

int doSlider = 0;
int rdHandler = 0;

simtime_t s_simperiod;
double s_sigunit;

Arg args[20];
Display *display;
Display *pdisplay;

static int opt_analog  = 0;
static int opt_digital = 0;
static int opt_ext     = 0;
static int opt_logic   = 0;
static int opt_timing  = 0;
static int opt_pstar   = 0;
static int opt_spice   = 0;
static int opt_input   = 0;
static int opt_prepare = 0;
static int opt_verbose = 0;

static char lastErrMess[1024];

DM_PROJECT *projKey = NULL;
char *cellName = NULL;

int skipchar (char *str)
{
    register char *s = str;
    register char *t;

    while (*s > ' ' && *s < '\177') ++s;
    if (*s) { /* something to skip */
	t = s;
	while (*++s) if (*s > ' ' && *s < '\177') *t++ = *s;
	*t = '\0';
	return (1);
    }
    return (0);
}

/* to suppress ``just Traversed into different shell'' messages */
static void warningHandler (String message)
{
 /* fprintf (stderr, "-- warningHandler: %s\n", message); */
}

int main (int argc, char *argv[])
{
    Widget toplevel;
    XColor color, hdwcolor;
    char *s;
    int error = 0;
    XrmValue rmval;
    char *std_bg_color;

    static XrmOptionDescRec opTable[] = {
        {"-freestateColor", ".freestateColor", XrmoptionSepArg, NULL},
        {"-high_lowColor",  ".high_lowColor",  XrmoptionSepArg, NULL},
        {"-pointerColor",   ".pointerColor",   XrmoptionSepArg, NULL},
        {"-undefinedColor", ".undefinedColor", XrmoptionSepArg, NULL},
        {"-voltsColor",     ".voltsColor",     XrmoptionSepArg, NULL},
        {"-std_bg",         ".std_bg",         XrmoptionSepArg, NULL},
    };

    Toplevel = toplevel = XtInitialize ("main", "Simeye", opTable, XtNumber (opTable), &argc, argv);
    XtSetArg (args[0], XmNminWidth,  400);
    XtSetArg (args[1], XmNminHeight, 350);
    XtSetValues (toplevel, args, 2);

    display = XtDisplay (toplevel);

    if (XrmGetResource (XtDatabase (display), "simeye.std_bg", "", &s, &rmval) == True) {
	std_bg_color = rmval.addr;
    }
    else {
	std_bg_color = "grey";
    }

    if (XrmGetResource (XtDatabase (display), "simeye.keymnemonic", "", &s, &rmval) == True) {
	s = rmval.addr; --s;
	while (*++s) if (isupper ((int)*s)) *s += LOWER;
	s = rmval.addr;
	if (strsame (s, "false") || strsame (s, "off")) keymn = 0;
    }

    colormap = XDefaultColormap (display, DefaultScreen (display));
    XLookupColor (display, colormap, std_bg_color, &color, &hdwcolor);
    XAllocColor (display, colormap, &hdwcolor);
    std_bg = hdwcolor.pixel;

    XLookupColor (display, colormap, "red", &color, &hdwcolor);
    XAllocColor (display, colormap, &hdwcolor);
    std_red = hdwcolor.pixel;

    E_str = XmStringCreateLtoR ("editing:", XmSTRING_DEFAULT_CHARSET);
    V_str = XmStringCreateLtoR ("viewing:", XmSTRING_DEFAULT_CHARSET);
    N_str = XmStringCreateLtoR ("", XmSTRING_DEFAULT_CHARSET);

    /* defaults */

    Logic = 1;
    Spice = 0;
    SimLevel = slsLogicLevel;
    slslogtype = 'D';
    slstimtype = 'D';
    UseSettings = 0;
    doDetailZoom = 0;
    tryNonCapital = 0;

    /* end defaults */

    while (--argc > 0) {
        if ( (*++argv)[0] == '-' ) {
	    for (s = *argv + 1; *s; s++) {
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
                    case 'c' :
			opt_spice = 1;
                        break;
                    case 'a' :
			opt_analog = 1;
                        break;
                    case 'b' :           /* d can not be used because of Xt */
			opt_digital = 1;
                        break;
                    case 'e' :
			opt_ext = 1;
                        break;
                    case 'I' :           /* i can not be used because of Xt */
			opt_input = 1;
                        break;
                    case 'R' :
			opt_input = 1;
			opt_prepare = 1;
                        break;
                    case 'v' :
			opt_verbose = 1;
                        break;
		    default:
			if (*s != 'h') fprintf (stderr, "-%c: illegal option\n", *s);
			error = 1;
	        }
	    }
	}
	else {
	    cellName = *argv;
	}
	if (error) {
	    fprintf (stderr, "\n%s %s\n", argv0, TOOLVERSION);
	    fprintf (stderr, usg_msg, argv0);
	    die (1);
	}
    }

    if (opt_verbose) fprintf (stderr, "Version %s\n", TOOLVERSION);

    if (dmInit (argv0)) {
	fprintf (stderr, "%s: Cannot initialize the framework services\n", argv0);
	die (1);
    }
    initIntrup ();

    projKey = dmOpenProject (DEFAULT_PROJECT, PROJ_READ);

    readrcFile ();

    sim_type_nr = 0; /* sls-logic */

    if (opt_logic) {
	Spice = 0;
	SimLevel = slsLogicLevel;
    }
    else if (opt_timing) {
	sim_type_nr = 1;
	Spice = 0;
	SimLevel = 3;
    }
    else if (opt_spice) {
	sim_type_nr = 2;
	Spice = 1;
	SimLevel = 3;
    }
    else if (opt_pstar) {
	sim_type_nr = 3;
	Spice = 2;
	SimLevel = 3;
    }

    if (!Spice) {
	if (opt_analog) {
	    if (SimLevel == 3)
		slstimtype = 'A';
	    else
		slslogtype = 'A';
	}
	else if (opt_digital) {
	    if (SimLevel == 3)
		slstimtype = 'D';
	    else
		slslogtype = 'D';
	}
	if (SimLevel == 3)
	    Logic = (slstimtype == 'D');
	else
	    Logic = (slslogtype == 'D');
    }
    else {
	Logic = 0;
    }

    sim_read_nr = Logic;

    if (cellName) {
	(void) skipchar (cellName);
	if (!opt_ext) {
	    s = cellName;
	    while (*s && *s != '.') ++s;
	    if (*s) { /* dot found */
		if (!strncmp (s, ".cmd", 4)) {
		    opt_input = 1;
		}
		else if (!strncmp (s, ".res", 4)) {
		    opt_input = 0; Logic = 1; Spice = 0;
		}
		else if (!strncmp (s, ".plt", 4)) {
		    opt_input = 0; Logic = 0; Spice = 0;
		}
		else if (!strncmp (s, ".ana", 4)) {
		    opt_input = 0; Logic = 0; Spice = 1;
		}
		else if (!strncmp (s, ".list", 5)) {
		    opt_input = 0; Logic = 0; Spice = 2;
		}
		s = "";
	    }
	    else { /* add ext */
		if (opt_input)  s = ".cmd";
		else if (Spice) s = Spice == 1 ? ".ana" : ".list";
		else if (Logic) s = ".res";
		else            s = ".plt";
	    }
	}
	else s = "";

	if (strlen (cellName) + strlen (s) > MAXNAME) {
	    fprintf (stderr, "%s: Too long name argument specified\n", argv0);
	    die (1);
	}
	if (opt_input)
	    sprintf (inputname, "%s%s", cellName, s);
	else
	    sprintf (outputname, "%s%s", cellName, s);
        if (opt_prepare) {
	    sprintf (circuitname, cellName);
	    sprintf (stimuliname, "%s%s", cellName, ".cmd");
        }
    }

    if (!(username = getenv ("LOGNAME"))) username = getenv ("USER");

    build_Dialogs    (toplevel);
    build_MainWindow (toplevel);
    build_psprDialog (toplevel);
    XtRealizeWidget  (toplevel);

    currLogic = Logic;

    drawStartup ();
    eventsStartup ();

    XWarpPointer (display, None, XtWindow (canvas), 0, 0, 0, 0, canvasWidth / 2, canvasHeight / 2);

    /* activate a redraw handler for exposure and resize events
     */
    XtAddEventHandler (canvas, ExposureMask | StructureNotifyMask, False, (XtEventHandler)redrawHandler, NULL);

    XtAppSetWarningHandler (XtDisplayToApplicationContext (display), (XtErrorHandler)warningHandler);

    /* activate handler for fatal I/O errors
     * (e.g. when a XKillClient was done
     */
    XSetIOErrorHandler (destroyHandler);

    /*
     * Note: expose event starts initial drawing!
     */
    XtMainLoop ();

    return (0);
}

void openListCB (Widget w, caddr_t client_data, caddr_t call_data)
{
    XmString *p;
    char *s;

    XtSetArg (args[0], XmNselectedItems, &p);
    XtGetValues (w, args, 1);
    if (XmStringGetLtoR (*p, XmSTRING_DEFAULT_CHARSET, &s) == True)
	XmTextFieldSetString (openSelect, s);
}

void set_readSelect (char *s)
{
    int n = 0;

    if (readTypeTmp == readTypeOpt[1]) n = 1;
    else if (readTypeTmp == readTypeOpt[2]) n = 2;
    else if (readTypeTmp == readTypeOpt[3]) n = 3;
    strcpy (readTypeSel[n], s);
}

void readListCB (Widget w, caddr_t client_data, caddr_t call_data)
{
    XmString *p;
    char *s;

    XtSetArg (args[0], XmNselectedItems, &p);
    XtGetValues (w, args, 1);
    if (XmStringGetLtoR (*p, XmSTRING_DEFAULT_CHARSET, &s) == True) {
	if (strlen (s) > MAXNAME) {
	    errorMessage ("Too long name!");
	    s[MAXNAME] = 0;
	}
	set_readSelect (s);
	XmTextFieldSetString (readSelect, s);
    }
}

int existCell (char *cell, char *view)
{
    if (!projKey) return (0);
    if (_dmExistCell (projKey, cell, view) != 1) return (0);
    return (1); /* OK */
}

void build_openDialog (Widget toplevel)
{
    Widget w;
    int n = 0;

    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNshadowThickness, 2); n++;
    XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
    XtSetArg (args[n], XmNnoResize, True); n++;
    XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
    XtSetArg (args[n], XmNautoUnmanage, False); n++;
    openDialog = XmCreateFormDialog (toplevel, "openDialog", args, n);

    n = 1;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 10); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopOffset, 8); n++;
    w = XmCreateLabel (openDialog, "Stimuli:", args, n);
    XtManageChild (w);

    n = 0;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 10); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightOffset, 28); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, w); n++;
    XtSetArg (args[n], XmNtopOffset, 2); n++;
    XtSetArg (args[n], XmNvisibleItemCount, 8); n++;
    XtSetArg (args[n], XmNwidth, 142); n++;
    XtSetArg (args[n], XmNautomaticSelection, True); n++;
    openList = XmCreateScrolledList (openDialog, "openList", args, n);
    XtManageChild (openList);
    addCallback (openList, XmNbrowseSelectionCallback, openListCB, NULL);

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, XtParent (openList)); n++;
    XtSetArg (args[n], XmNtopOffset, 8); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 10); n++;
    w = XmCreateLabel (openDialog, "Selected stimuli:", args, n);
    XtManageChild (w);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, w); n++;
    XtSetArg (args[n], XmNtopOffset, 2); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightOffset, 10); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 8); n++;
    openSelect = XmCreateTextField (openDialog, "openSelect", args, n);
    XtManageChild (openSelect);
    addCallback (openSelect, XmNactivateCallback, OpenOkCB, 1);

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, openSelect); n++;
    XtSetArg (args[n], XmNtopOffset, 10); n++;
    w = XmCreateSeparator (openDialog, "Sep", args, n);
    XtManageChild (w);

    n = 1;
    XtSetArg (args[n], XmNshowAsDefault, 1); n++;
    XtSetArg (args[n], XmNmarginTop, 2); n++;
    XtSetArg (args[n], XmNmarginBottom, 2); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, w); n++;
    XtSetArg (args[n], XmNtopOffset, 10); n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNbottomOffset, 10); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 15); n++;
    openOkButton = XmCreatePushButton (openDialog, " Open ", args, n);
    XtManageChild (openOkButton);
    addCallback (openOkButton, XmNactivateCallback, OpenOkCB, 1);

    n = 9;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightOffset, 15); n++;
    w = XmCreatePushButton (openDialog, "Cancel", args, n);
    XtManageChild (w);
    addCallback (w, XmNactivateCallback, OpenOkCB, 0);
}

void build_readDialog (Widget toplevel)
{
    Widget p, w, t;
    XmString str;
    int n = 0;

    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNshadowThickness, 2); n++;
    XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
    XtSetArg (args[n], XmNnoResize, True); n++;
    XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
    XtSetArg (args[n], XmNautoUnmanage, False); n++;
    str = XmStringLtoRCreate ("Simeye Read Dialog", XmSTRING_DEFAULT_CHARSET);
    XtSetArg (args[n], XmNdialogTitle, str); n++;
    readDialog = XmCreateFormDialog (toplevel, "readDialog", args, n);
    XmStringFree (str);

    n = 1; /* use std_bg */
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 10); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopOffset, 8); n++;
    w = XmCreateLabel (readDialog, "Simout:", args, n);
    XtManageChild (w);

    n = 1; /* use std_bg */
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, w); n++;
    XtSetArg (args[n], XmNleftOffset, 109); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, w); n++;
    XtSetArg (args[n], XmNtopOffset, 2); n++;
    t = XmCreateLabel (readDialog, "Type:", args, n);
    XtManageChild (t);

    n = 0;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, w); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNrightWidget, t); n++;
    XtSetArg (args[n], XmNrightOffset, 13); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, w); n++;
    XtSetArg (args[n], XmNtopOffset, 2); n++;
    XtSetArg (args[n], XmNvisibleItemCount, 8); n++;
    XtSetArg (args[n], XmNautomaticSelection, True); n++;
    readList = XmCreateScrolledList (readDialog, "readList", args, n);
    XtManageChild (readList);
    addCallback (readList, XmNbrowseSelectionCallback, readListCB, NULL);

    p = XmCreatePulldownMenu (readDialog, "readTypes", NULL, 0);
    readTypeOpt[0] = XmCreatePushButton (p, "Analog", NULL, 0);
    readTypeOpt[1] = XmCreatePushButton (p, "Digital", NULL, 0);
    readTypeOpt[2] = XmCreatePushButton (p, "Spice", NULL, 0);
    readTypeOpt[3] = XmCreatePushButton (p, "Pstar", NULL, 0);
    XtManageChildren (readTypeOpt, 4);
    addCallback (readTypeOpt[0], XmNactivateCallback, readTypeCB, NULL);
    addCallback (readTypeOpt[1], XmNactivateCallback, readTypeCB, NULL);
    addCallback (readTypeOpt[2], XmNactivateCallback, readTypeCB, NULL);
    addCallback (readTypeOpt[3], XmNactivateCallback, readTypeCB, NULL);
    if (Spice) {
	readTypeCur = Spice == 2 ? readTypeOpt[3] : readTypeOpt[2];
    }
    else {
	readTypeCur = Logic ? readTypeOpt[1] : readTypeOpt[0];
    }
    readTypeTmp = readTypeCur;
    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, t); n++;
    XtSetArg (args[n], XmNleftOffset, -12); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, t); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightOffset, 10); n++;
    XtSetArg (args[n], XmNsubMenuId, p); n++;
    XtSetArg (args[n], XmNmenuHistory, readTypeCur); n++;
    XtSetArg (args[n], XmNlabelString, N_str); n++;
    readType = XmCreateOptionMenu (readDialog, "readType", args, n);
    XtManageChild (readType);

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, XtParent (readList)); n++;
    XtSetArg (args[n], XmNtopOffset, 8); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 10); n++;
    w = XmCreateLabel (readDialog, "Selected simout:", args, n);
    XtManageChild (w);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, w); n++;
    XtSetArg (args[n], XmNtopOffset, 2); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightOffset, 10); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 8); n++;
    readSelect = XmCreateTextField (readDialog, "readSelect", args, n);
    XtManageChild (readSelect);
    addCallback (readSelect, XmNactivateCallback, ReadOkCB, 1);

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, readSelect); n++;
    XtSetArg (args[n], XmNtopOffset, 10); n++;
    w = XmCreateSeparator (readDialog, "Sep", args, n);
    XtManageChild (w);

    n = 1;
    XtSetArg (args[n], XmNshowAsDefault, 1); n++;
    XtSetArg (args[n], XmNmarginTop, 2); n++;
    XtSetArg (args[n], XmNmarginBottom, 2); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, w); n++;
    XtSetArg (args[n], XmNtopOffset, 10); n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNbottomOffset, 10); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 15); n++;
    p = XmCreatePushButton (readDialog, " Read ", args, n);
    XtManageChild (p);
    addCallback (p, XmNactivateCallback, ReadOkCB, 1);

    n = 9;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightOffset, 15); n++;
    w = XmCreatePushButton (readDialog, "Cancel", args, n);
    XtManageChild (w);
    addCallback (w, XmNactivateCallback, ReadOkCB, 0);
}

void cirListCB (Widget w, caddr_t client_data, caddr_t call_data)
{
    XmString *p;
    char *s;

    XtSetArg (args[0], XmNselectedItems, &p);
    XtGetValues (w, args, 1);
    if (XmStringGetLtoR (*p, XmSTRING_DEFAULT_CHARSET, &s) == True)
	XmTextFieldSetString (cirSelect, s);
}

void cmdListCB (Widget w, caddr_t client_data, caddr_t call_data)
{
    XmString *p;
    char *s;

    XtSetArg (args[0], XmNselectedItems, &p);
    XtGetValues (w, args, 1);
    if (XmStringGetLtoR (*p, XmSTRING_DEFAULT_CHARSET, &s) == True)
	XmTextFieldSetString (cmdSelect, s);
}

void build_simuDialog (Widget toplevel)
{
    Widget p, w, t;
    XmString str;
    int n = 0;

    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNshadowThickness, 2); n++;
    XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
    XtSetArg (args[n], XmNnoResize, True); n++;
    XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
    XtSetArg (args[n], XmNautoUnmanage, False); n++;
    str = XmStringLtoRCreate ("Simeye Simulate Dialog", XmSTRING_DEFAULT_CHARSET);
    XtSetArg (args[n], XmNdialogTitle, str); n++;
    simuDialog = XmCreateFormDialog (toplevel, "simuDialog", args, n);
    XmStringFree (str);

    n = 1; /* use std_bg */
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 10); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopOffset, 8); n++;
    w = XmCreateLabel (simuDialog, "Circuit:", args, n);
    XtManageChild (w);

    n = 1; /* use std_bg */
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, w); n++;
    XtSetArg (args[n], XmNleftOffset, 100); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopOffset, 8); n++;
    p = XmCreateLabel (simuDialog, "Stimuli:", args, n);
    XtManageChild (p);

    n = 1; /* use std_bg */
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, p); n++;
    XtSetArg (args[n], XmNleftOffset, 103); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, p); n++;
    XtSetArg (args[n], XmNtopOffset, 2); n++;
    t = XmCreateLabel (simuDialog, "Type:", args, n);
    XtManageChild (t);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, w); n++;
    XtSetArg (args[n], XmNtopOffset, 2); n++;
    XtSetArg (args[n], XmNvisibleItemCount, 8); n++;
    XtSetArg (args[n], XmNautomaticSelection, True); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, w); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNrightWidget, p); n++;
    XtSetArg (args[n], XmNrightOffset, 10); n++;
    cirList = XmCreateScrolledList (simuDialog, "cirList", args, n);
    XtManageChild (cirList);
    addCallback (cirList, XmNbrowseSelectionCallback, cirListCB, NULL);

    n = 6;
    XtSetArg (args[n], XmNleftWidget, p); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNrightWidget, t); n++;
    XtSetArg (args[n], XmNrightOffset, 13); n++;
    cmdList = XmCreateScrolledList (simuDialog, "cmdList", args, n);
    XtManageChild (cmdList);
    addCallback (cmdList, XmNbrowseSelectionCallback, cmdListCB, NULL);

    p = XmCreatePulldownMenu (simuDialog, "simuTypes", NULL, 0);
    simuTypeOpt[0] = XmCreatePushButton (p, "sls-logic ", NULL, 0);
    simuTypeOpt[1] = XmCreatePushButton (p, "sls-timing", NULL, 0);
    simuTypeOpt[2] = XmCreatePushButton (p, "spice     ", NULL, 0);
    simuTypeOpt[3] = XmCreatePushButton (p, "pstar     ", NULL, 0);
    XtManageChildren (simuTypeOpt, 4);
    addCallback (simuTypeOpt[0], XmNactivateCallback, simuTypeCB, NULL);
    addCallback (simuTypeOpt[1], XmNactivateCallback, simuTypeCB, NULL);
    addCallback (simuTypeOpt[2], XmNactivateCallback, simuTypeCB, NULL);
    addCallback (simuTypeOpt[3], XmNactivateCallback, simuTypeCB, NULL);
    simuTypeTmp = simuTypeCur = simuTypeOpt[sim_type_nr];

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, t); n++;
    XtSetArg (args[n], XmNleftOffset, -12); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, t); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightOffset, 10); n++;
    XtSetArg (args[n], XmNsubMenuId, p); n++;
    XtSetArg (args[n], XmNmenuHistory, simuTypeCur); n++;
    XtSetArg (args[n], XmNlabelString, N_str); n++;
    simuType = XmCreateOptionMenu (simuDialog, "simuType", args, n);
    XtManageChild (simuType);

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, t); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, simuType); n++;
    XtSetArg (args[n], XmNtopOffset, 20); n++;
    w = XmCreateLabel (simuDialog, "Read:", args, n);
    XtManageChild (w);

    p = XmCreatePulldownMenu (simuDialog, "simuReads", NULL, 0);
    simuReadOpt[0] = XmCreatePushButton (p, "Analog", NULL, 0);
    simuReadOpt[1] = XmCreatePushButton (p, "Digital", NULL, 0);
    XtManageChildren (simuReadOpt, 2);
    addCallback (simuReadOpt[0], XmNactivateCallback, simuReadCB, NULL);
    addCallback (simuReadOpt[1], XmNactivateCallback, simuReadCB, NULL);
    simuReadTmp = simuReadCur = simuReadOpt[sim_read_nr];

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, simuType); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, w); n++;
    XtSetArg (args[n], XmNsubMenuId, p); n++;
    XtSetArg (args[n], XmNmenuHistory, simuReadCur); n++;
    XtSetArg (args[n], XmNlabelString, N_str); n++;
    simuRead = XmCreateOptionMenu (simuDialog, "simuRead", args, n);
    XtManageChild (simuRead);

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, XtParent (cirList)); n++;
    XtSetArg (args[n], XmNtopOffset, 8); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 10); n++;
    w = XmCreateLabel (simuDialog, "Selected circuit:", args, n);
    XtManageChild (w);

    n = 0;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 8); n++;
    XtSetArg (args[n], XmNrightOffset, 8); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNrightWidget, XtParent (cmdList)); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, w); n++;
    XtSetArg (args[n], XmNtopOffset, 2); n++;
    cirSelect = XmCreateTextField (simuDialog, "cirSelect", args, n);
    XtManageChild (cirSelect);

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, XtParent (cmdList)); n++;
    XtSetArg (args[n], XmNtopOffset, 8); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, cirSelect); n++;
    XtSetArg (args[n], XmNleftOffset, 8); n++;
    w = XmCreateLabel (simuDialog, "Selected stimuli:", args, n);
    XtManageChild (w);

    n = 0;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, cirSelect); n++;
    XtSetArg (args[n], XmNleftOffset, 6); n++;
    XtSetArg (args[n], XmNrightOffset, 15); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, w); n++;
    XtSetArg (args[n], XmNtopOffset, 2); n++;
    cmdSelect = XmCreateTextField (simuDialog, "cmdSelect", args, n);
    XtManageChild (cmdSelect);

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, cirSelect); n++;
    XtSetArg (args[n], XmNtopOffset, 10); n++;
    w = XmCreateSeparator (simuDialog, "Sep", args, n);
    XtManageChild (w);

    n = 1;
    XtSetArg (args[n], XmNshowAsDefault, 1); n++;
    XtSetArg (args[n], XmNmarginTop, 2); n++;
    XtSetArg (args[n], XmNmarginBottom, 2); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopWidget, w); n++;
    XtSetArg (args[n], XmNtopOffset, 10); n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNbottomOffset, 10); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 15); n++;
    p = XmCreatePushButton (simuDialog, "  Set  ", args, n);
    XtManageChild (p);
    addCallback (p, XmNactivateCallback, SimuOkCB, 1);

    n = 9;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, p); n++;
    XtSetArg (args[n], XmNleftOffset, 20); n++;
    p = XmCreatePushButton (simuDialog, "  Run  ", args, n);
    XtManageChild (p);
    addCallback (p, XmNactivateCallback, SimuOkCB, 2);

    n = 9;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, p); n++;
    XtSetArg (args[n], XmNleftOffset, 20); n++;
    p = XmCreatePushButton (simuDialog, " Edit ", args, n);
    XtManageChild (p);
    addCallback (p, XmNactivateCallback, SimuOkCB, 3);

    n = 9;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightOffset, 15); n++;
    w = XmCreatePushButton (simuDialog, " Cancel ", args, n);
    XtManageChild (w);
    addCallback (w, XmNactivateCallback, SimuOkCB, 0);
}

char *Error = "Error message? Needed for libPW.a";

static Widget theX2PostScriptWidget;

static void psprRedraw (Widget theDrawWidget, Region reg)
{
    redrawMessage ();
    setLineAttr ();
    doSlider = 1;
    draw ('b', 0, 0, 0, 0);
    doSlider = 0;
    resetLineAttr ();
}

void build_psprDialog (Widget toplevel)
{
    Widget b, theX2PostScriptShell;
    XpStruct * Xpinfo;
    XmString str;
    char pathname[DM_MAXPATHLEN];
    int n = 0;

    sprintf (pathname, "%s/share/lib", icdpath);

    theX2PostScriptShell = XmCreateDialogShell (toplevel, "Simeye Print Dialog", args, n);
    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); ++n;
    XtSetArg (args[n], XmNshadowThickness, 2); ++n;
    XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); ++n;
    XtSetArg (args[n], XmNnoResize, True); ++n;
    /*
    XtSetArg (args[n], XtNprintCommand, "pspr"); ++n;
    */
    XtSetArg (args[n], XtNdrawWidget, canvas); ++n;
    XtSetArg (args[n], XtNfontmapDir, pathname); ++n;
    XtSetArg (args[n], XtNcolorMode, XpBW); ++n;
    XtSetArg (args[n], XtNportraitLandscape, XpLANDSCAPE); ++n;
    XtSetArg (args[n], XtNfileOrPrinter, XpPRINTER); ++n;
    theX2PostScriptWidget = XtCreateWidget ("X2PSwidget", xpX2PostScriptWidgetClass, theX2PostScriptShell, args, n);
    addCallback (theX2PostScriptWidget, XtNredrawCallback, psprRedraw, NULL);

    b = XpX2PostScriptWidgetGetChild (theX2PostScriptWidget, XpX2POSTSCRIPT_APPLY_BUTTON);
    XtUnmanageChild (b);
    b = XpX2PostScriptWidgetGetChild (theX2PostScriptWidget, XpX2POSTSCRIPT_HELP_BUTTON);
    XtUnmanageChild (b);
    b = XpX2PostScriptWidgetGetChild (theX2PostScriptWidget, XpX2POSTSCRIPT_OK_BUTTON);
    str = XmStringCreateLtoR ("Print", XmSTRING_DEFAULT_CHARSET);
    XtSetArg (args[0], XmNlabelString, str);
    XtSetValues (b, args, 1);
    XmStringFree (str);

    XtSetArg (args[0], XtNxpinfo, &Xpinfo);
    XtGetValues (theX2PostScriptWidget, args, 1);
    Xpinfo -> display = display;
    pdisplay = (Display *) Xpinfo;
}

void mapCB (Widget w, caddr_t clientData, caddr_t callData)
{
    static int n0, n1, n2, n3, n4, n5, n6, n7, n8;
    int x, y, ft = 0;

    switch ((long) clientData) {
    case 0: if (!n0) { n0 = ft = 1; } break;
    case 1: if (!n1) { n1 = ft = 1; } break;
    case 2: if (!n2) { n2 = ft = 1; } break;
    case 3: if (!n3) { n3 = ft = 1; } break;
    case 4: if (!n4) { n4 = ft = 1; } break;
    case 5: if (!n5) { n5 = ft = 1; } break;
    case 6: if (!n6) { n6 = ft = 1; } break;
    case 7: if (!n7) { n7 = ft = 1; } break;
    case 8: if (!n8) { n8 = ft = 1; } break;
    }
    if (ft) { w = canvas; x = canvasWidth / 2; y = canvasHeight / 2; }
    else { x = 72; y = 106; }
    XWarpPointer (display, None, XtWindow (w), 0, 0, 0, 0, x, y);
}

void build_Dialogs (Widget toplevel)
{
    Widget b;
    XmString str, str2;

    str = XmStringCreateLtoR ("Simeye Info Dialog", XmSTRING_DEFAULT_CHARSET);
    XtSetArg (args[0], XmNbackground, std_bg);
    XtSetArg (args[1], XmNshadowThickness, 2);
    XtSetArg (args[2], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
    XtSetArg (args[3], XmNnoResize, True);
    XtSetArg (args[4], XmNdialogTitle, str);
    iDialog = XmCreateInformationDialog (toplevel, "iDialog", args, 5);
    XmStringFree (str);
    addCallback (iDialog, XmNmapCallback, mapCB, 0);
    b = XmMessageBoxGetChild (iDialog, XmDIALOG_CANCEL_BUTTON);
    XtUnmanageChild (b);
    b = XmMessageBoxGetChild (iDialog, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild (b);

    str = XmStringCreateLtoR ("Simeye Error Dialog", XmSTRING_DEFAULT_CHARSET);
    XtSetArg (args[4], XmNdialogTitle, str);
    eDialog = XmCreateErrorDialog (toplevel, "eDialog", args, 5);
    XmStringFree (str);
    addCallback (eDialog, XmNmapCallback, mapCB, 1);
    b = XmMessageBoxGetChild (eDialog, XmDIALOG_CANCEL_BUTTON);
    XtUnmanageChild (b);
    b = XmMessageBoxGetChild (eDialog, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild (b);

    str = XmStringCreateLtoR ("  No  ", XmSTRING_DEFAULT_CHARSET);
    str2 = XmStringCreateLtoR ("  Yes  ", XmSTRING_DEFAULT_CHARSET);
    XtSetArg (args[4], XmNcancelLabelString, str);
    XtSetArg (args[5], XmNokLabelString, str2);
    qDialog = XmCreateQuestionDialog (toplevel, "qDialog", args, 6);
    XmStringFree (str);
    XmStringFree (str2);
    addCallback (qDialog, XmNokCallback, OkCB, NULL);
    addCallback (qDialog, XmNcancelCallback, CancelCB, NULL);
    addCallback (qDialog, XmNmapCallback, mapCB, 2);
    b = XmMessageBoxGetChild (qDialog, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild (b);

    pDialog = XmCreatePromptDialog (toplevel, "pDialog", args, 4);
    addCallback (pDialog, XmNokCallback, OkCB, NULL);
    addCallback (pDialog, XmNcancelCallback, CancelCB, NULL);
    addCallback (pDialog, XmNmapCallback, mapCB, 3);
    b = XmSelectionBoxGetChild (pDialog, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild (b);

    build_openDialog (toplevel);
    build_readDialog (toplevel);
    build_simuDialog (toplevel);
}

void EditCB (Widget w, caddr_t clientData, caddr_t callData)
{
    if (!*stimuliname) {
	char ms[80];
	strcpy (ms, "Cannot edit:\nNo stimuli file set in Simulate Prepare menu!");
	infoMessage (ms);
	return;
    }
    if (somethingChanged && strsame (stimuliname, Commandfile)) {
	qDialogCB (EditButton, NULL, NULL);
	return;
    }
    if (clickedCommandw) endCommand ();
    strcpy (inputname, stimuliname);
    cW = w;
    if (editing && somethingChanged && Begin_signal) {
	qW = SaveButton;
	if (SaveEdit) OkCB (qDialog, NULL, NULL);
	else qDialogCB (qW, NULL, NULL);
    }
    else
	Inputs (OpenButton, 2, NULL);
}

void RunCB (Widget w, caddr_t clientData, caddr_t callData)
{
    if (!*circuitname || !*stimuliname) {
	char ms[80];
	sprintf (ms, "Cannot simulate:\nNo %s set in Simulate Prepare menu!",
		!*circuitname ? "circuit name" : "stimuli file");
	infoMessage (ms);
	return;
    }
    if (clickedCommandw) endCommand ();
    cW = w;
    if (editing && somethingChanged && Begin_signal) {
	qW = SaveButton;
	if (SaveEdit) OkCB (qDialog, NULL, NULL);
	else qDialogCB (qW, NULL, NULL);
    }
    else
	Simulate (SimuButton, 2, NULL);
}

void build_MainWindow (Widget toplevel)
{
    Widget total;
    Widget commands;
    Widget WorkRegion;
    Widget StatusArea;
    Widget ScrolledWindow;
    Dimension dh, dw;
    int n;

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    total = XmCreateMainWindow (toplevel, "total", args, n);
    XtManageChild (total);

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    commands = XmCreateMenuBar (total, "commands", args, n);
    XtSetArg (args[0], XmNmenuBar, commands);
    XtSetValues (total, args, 1);
    XtManageChild (commands);

    build_File_menu (commands);
    build_Edit_menu (commands);
    build_Simu_menu (commands);
    build_View_menu (commands);
    build_Options_menu (commands);
    build_Help_menu (commands);

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    WorkRegion = XmCreateForm (total, "WorkRegion", args, n);
    XtSetArg (args[0], XmNworkWindow, WorkRegion);
    XtSetValues (total, args, 1);
    XtManageChild (WorkRegion);

    StatusArea = build_StatusArea (WorkRegion);

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 2); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNtopOffset, 2); n++;
    XtSetArg (args[n], XmNtopWidget, StatusArea); n++;
    XtSetArg (args[n], XmNshadowThickness, 2); n++;
    ScrolledWindow = XmCreateScrolledWindow (WorkRegion, "ScrolledWindow", args, n);
    XtManageChild (ScrolledWindow);

    canvas = XmCreateForm (ScrolledWindow, "canvas", args, 0);
    XtSetArg (args[0], XmNworkWindow, canvas);
    XtSetValues (ScrolledWindow, args, 1);
    XtManageChild (canvas);

    XtSetArg (args[0], XmNheight, &dh);
    XtSetArg (args[1], XmNwidth,  &dw);
    XtGetValues (canvas, args, 2);
    n = 0;
    if (dh < 10) { XtSetArg (args[n], XmNheight, stdCanvasHeight); ++n; }
    if (dw < 10) { XtSetArg (args[n], XmNwidth,  stdCanvasWidth);  ++n; }
    if (n) XtSetValues (canvas, args, n);

    n = 0;
    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNsliderSize, 100); n++;
    XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
    HorBar = XmCreateScrollBar (ScrolledWindow, "HorBar", args, n);
    XtManageChild (HorBar);
    addCallback (HorBar, XmNdecrementCallback, SliderCB, NULL);
    addCallback (HorBar, XmNincrementCallback, SliderCB, NULL);
    addCallback (HorBar, XmNdragCallback, SliderCB, NULL);
    addCallback (HorBar, XmNpageDecrementCallback, SliderCB, NULL);
    addCallback (HorBar, XmNpageIncrementCallback, SliderCB, NULL);

    n = 2;
    XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
    VerBar = XmCreateScrollBar (ScrolledWindow, "VerBar", args, n);
    XtManageChild (VerBar);
    addCallback (VerBar, XmNdecrementCallback, SliderCB, NULL);
    addCallback (VerBar, XmNincrementCallback, SliderCB, NULL);
    addCallback (VerBar, XmNdragCallback, SliderCB, NULL);
    addCallback (VerBar, XmNpageDecrementCallback, SliderCB, NULL);
    addCallback (VerBar, XmNpageIncrementCallback, SliderCB, NULL);

    XtSetArg (args[0], XmNhorizontalScrollBar, HorBar);
    XtSetArg (args[1], XmNverticalScrollBar, VerBar);
    XtSetValues (ScrolledWindow, args, 2);
}

void build_File_menu (Widget commands)
{
    Widget FileButton;
    Widget FilePDMenu;
    Widget Separator;
    int n;

    XtSetArg (args[0], XmNbackground, std_bg);
    FilePDMenu = XmCreatePulldownMenu (commands, "FilePDMenu", args, 1);

    n = 1;
    XtSetArg (args[n], XmNmnemonic, 'F'); ++n;
    XtSetArg (args[n], XmNsubMenuId, FilePDMenu); ++n;
    FileButton = XmCreateCascadeButton (commands, "File", args, n);
    XtManageChild (FileButton);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'N'); ++n; }
    NewButton = XmCreatePushButton (FilePDMenu, "New", args, n);
    XtManageChild (NewButton);
    addCallback (NewButton, XmNactivateCallback, qDialogCB, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'O'); ++n; }
    OpenButton = XmCreatePushButton (FilePDMenu, "Open", args, n);
    XtManageChild (OpenButton);
    addCallback (OpenButton, XmNactivateCallback, Inputs, 0);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'S'); ++n; }
    SaveButton = XmCreatePushButton (FilePDMenu, "Save", args, n);
    XtManageChild (SaveButton);
    addCallback (SaveButton, XmNactivateCallback, qDialogCB, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'A'); ++n; }
    SaveAsButton = XmCreatePushButton (FilePDMenu, "Save As", args, n);
    XtManageChild (SaveAsButton);
    addCallback (SaveAsButton, XmNactivateCallback, SaveAsCB, NULL);

    Separator = XmCreateSeparator (FilePDMenu, "Sep", args, 1);
    XtManageChild (Separator);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'R'); ++n; }
    ReadButton = XmCreatePushButton (FilePDMenu, "Read", args, n);
    XtManageChild (ReadButton);
    addCallback (ReadButton, XmNactivateCallback, Read, 0);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'd'); ++n; }
    XtSetArg (args[n], XmNsensitive, False); ++n;
    ReadAppButton = XmCreatePushButton (FilePDMenu, "Read Append", args, n);
    XtManageChild (ReadAppButton);
    addCallback (ReadAppButton, XmNactivateCallback, Read, 0);

    Separator = XmCreateSeparator (FilePDMenu, "Sep", args, 1);
    XtManageChild (Separator);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'H'); ++n; }
    XtSetArg (args[n], XmNsensitive, False); ++n;
    HardcButton = XmCreatePushButton (FilePDMenu, "Hardcopy", args, n);
    XtManageChild (HardcButton);
    addCallback (HardcButton, XmNactivateCallback, Hardcopy, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'P'); ++n; }
    XtSetArg (args[n], XmNsensitive, False); ++n;
    PrintButton = XmCreatePushButton (FilePDMenu, "Print", args, n);
    XtManageChild (PrintButton);
    addCallback (PrintButton, XmNactivateCallback, Print, NULL);

    Separator = XmCreateSeparator (FilePDMenu, "Sep", args, 1);
    XtManageChild (Separator);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'E'); ++n; }
    ExitButton = XmCreatePushButton (FilePDMenu, "Exit", args, n);
    XtManageChild (ExitButton);
    addCallback (ExitButton, XmNactivateCallback, qDialogCB, NULL);
}

void setPrintButton ()
{
    static int sensitive = 0;

    if (!Begin_signal) {
	if (sensitive) {
	    sensitive = 0;
	    XtSetArg (args[0], XmNsensitive, False);
	    XtSetValues (HardcButton, args, 1);
	    XtSetValues (PrintButton, args, 1);
	}
    }
    else
	if (!sensitive) {
	    sensitive = 1;
	    XtSetArg (args[0], XmNsensitive, True);
	    XtSetValues (HardcButton, args, 1);
	    XtSetValues (PrintButton, args, 1);
	}
}

void build_Edit_menu (Widget commands)
{
    Widget e;
    Widget EditPDMenu;
    Widget MoveButton;
    Widget Separator;
    int n;

    XtSetArg (args[0], XmNbackground, std_bg);
    EditPDMenu = XmCreatePulldownMenu (commands, "EditPDMenu", args, 1);

    n = 1;
    XtSetArg (args[n], XmNmnemonic, 'E'); ++n;
    XtSetArg (args[n], XmNsubMenuId, EditPDMenu); ++n;
    e = XmCreateCascadeButton (commands, "Edit", args, n);
    XtManageChild (e);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'A'); ++n; }
    AddButton = XmCreatePushButton (EditPDMenu, "Add", args, n);
    XtManageChild (AddButton);
    addCallback (AddButton, XmNactivateCallback, pDialogCB, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'C'); ++n; }
    ChangeButton = XmCreatePushButton (EditPDMenu, "Change", args, n);
    XtManageChild (ChangeButton);
    addCallback (ChangeButton, XmNactivateCallback, Change, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'D'); ++n; }
    DeleteButton = XmCreatePushButton (EditPDMenu, "Delete", args, n);
    XtManageChild (DeleteButton);
    addCallback (DeleteButton, XmNactivateCallback, Delsig, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'E'); ++n; }
    EraseAllButton = XmCreatePushButton (EditPDMenu, "EraseAll", args, n);
    XtManageChild (EraseAllButton);
    addCallback (EraseAllButton, XmNactivateCallback, qDialogCB, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'M'); ++n; }
    MoveButton = XmCreatePushButton (EditPDMenu, "Move", args, n);
    XtManageChild (MoveButton);
    addCallback (MoveButton, XmNactivateCallback, MoveCB, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'o'); ++n; }
    CopyButton = XmCreatePushButton (EditPDMenu, "Copy", args, n);
    XtManageChild (CopyButton);
    addCallback (CopyButton, XmNactivateCallback, Copysig, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'N'); ++n; }
    RenameButton = XmCreatePushButton (EditPDMenu, "ReName", args, n);
    XtManageChild (RenameButton);
    addCallback (RenameButton, XmNactivateCallback, Rename, NULL);

    Separator = XmCreateSeparator (EditPDMenu, "Sep", args, 1);
    XtManageChild (Separator);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'Y'); ++n; }
    YankButton = XmCreatePushButton (EditPDMenu, "Yank", args, n);
    XtManageChild (YankButton);
    addCallback (YankButton, XmNactivateCallback, Yank, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'P'); ++n; }
    PutButton = XmCreatePushButton (EditPDMenu, "Put", args, n);
    XtManageChild (PutButton);
    addCallback (PutButton, XmNactivateCallback, Put, NULL);

    Separator = XmCreateSeparator (EditPDMenu, "Sep", args, 1);
    XtManageChild (Separator);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'G'); ++n; }
    GridButton = XmCreatePushButton (EditPDMenu, "Grid", args, n);
    XtManageChild (GridButton);
    addCallback (GridButton, XmNactivateCallback, pDialogCB, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'S'); ++n; }
    SpeedButton = XmCreatePushButton (EditPDMenu, "Speed", args, n);
    XtManageChild (SpeedButton);
    addCallback (SpeedButton, XmNactivateCallback, pDialogCB, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'T'); ++n; }
    TendButton = XmCreatePushButton (EditPDMenu, "T_end", args, n);
    XtManageChild (TendButton);
    addCallback (TendButton, XmNactivateCallback, pDialogCB, NULL);
}

void build_Simu_menu (Widget commands)
{
    Widget p, w;
    int n;

    XtSetArg (args[0], XmNbackground, std_bg);
    p = XmCreatePulldownMenu (commands, "SimuPDMenu", args, 1);

    n = 1;
    XtSetArg (args[n], XmNmnemonic, 'S'); ++n;
    XtSetArg (args[n], XmNsubMenuId, p); ++n;
    if (!projKey) { XtSetArg (args[n], XmNsensitive, False); ++n; }
    w = XmCreateCascadeButton (commands, "Simulate", args, n);
    XtManageChild (w);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'R'); ++n; }
    RunButton = XmCreatePushButton (p, "Run", args, n);
    XtManageChild (RunButton);
    addCallback (RunButton, XmNactivateCallback, RunCB, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'E'); ++n; }
    EditButton = XmCreatePushButton (p, "Edit", args, n);
    XtManageChild (EditButton);
    addCallback (EditButton, XmNactivateCallback, EditCB, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'P'); ++n; }
    SimuButton = XmCreatePushButton (p, "Prepare", args, n);
    XtManageChild (SimuButton);
    addCallback (SimuButton, XmNactivateCallback, Simulate, 0);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'C'); ++n; }
    SimuCmd = XmCreatePushButton (p, "Command", args, n);
    XtManageChild (SimuCmd);
    addCallback (SimuCmd, XmNactivateCallback, InfoCB, NULL);
}

void build_View_menu (Widget commands)
{
    Widget w, ViewMenu;
    int n;

    XtSetArg (args[0], XmNbackground, std_bg);
    ViewMenu = XmCreatePulldownMenu (commands, "ViewMenu", args, 1);

    n = 1;
    XtSetArg (args[n], XmNmnemonic, 'V'); ++n;
    XtSetArg (args[n], XmNsubMenuId, ViewMenu); ++n;
    w = XmCreateCascadeButton (commands, "View", args, n);
    XtManageChild (w);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'M'); ++n; }
    w = XmCreatePushButton (ViewMenu, "Measure", args, n);
    XtManageChild (w);
    addCallback (w, XmNactivateCallback, Pointer, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'R'); ++n; }
    w = XmCreatePushButton (ViewMenu, "Redraw", args, n);
    XtManageChild (w);
    addCallback (w, XmNactivateCallback, Redraw, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'V'); ++n; }
    w = XmCreatePushButton (ViewMenu, "Values", args, n);
    XtManageChild (w);
    addCallback (w, XmNactivateCallback, viewValues, NULL);

    w = XmCreateSeparator (ViewMenu, "Sep", args, 1);
    XtManageChild (w);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'F'); ++n; }
    w = XmCreatePushButton (ViewMenu, "Full", args, n);
    XtManageChild (w);
    addCallback (w, XmNactivateCallback, Windowmax, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'Z'); ++n; }
    w = XmCreatePushButton (ViewMenu, "ZoomIn", args, n);
    XtManageChild (w);
    addCallback (w, XmNactivateCallback, Zoomin, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'O'); ++n; }
    w = XmCreatePushButton (ViewMenu, "ZoomOut", args, n);
    XtManageChild (w);
    addCallback (w, XmNactivateCallback, Zoomout, NULL);

    w = XmCreateSeparator (ViewMenu, "Sep", args, 1);
    XtManageChild (w);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'L'); ++n; }
    w = XmCreatePushButton (ViewMenu, "Load Window", args, n);
    XtManageChild (w);
    addCallback (w, XmNactivateCallback, Loadwin, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'S'); ++n; }
    w = XmCreatePushButton (ViewMenu, "Save Window", args, n);
    XtManageChild (w);
    addCallback (w, XmNactivateCallback, Savewin, NULL);
}

void build_Options_menu (Widget commands)
{
    Widget OptPDMenu, o, w;
    int n;

    XtSetArg (args[0], XmNbackground, std_bg);
    OptPDMenu = XmCreatePulldownMenu (commands, "OptPDMenu", args, 1);

    n = 1;
    XtSetArg (args[n], XmNmnemonic, 'O'); ++n;
    XtSetArg (args[n], XmNsubMenuId, OptPDMenu); ++n;
    o = XmCreateCascadeButton (commands, "Options", args, n);
    XtManageChild (o);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'D'); ++n; }
    if (doDetailZoom) { XtSetArg (args[n], XmNset, 1); ++n; }
    w = XmCreateToggleButton (OptPDMenu, "DetailZoomON", args, n);
    XtManageChild (w);
    addCallback (w, XmNvalueChangedCallback, DetailZoomCB, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'L'); ++n; }
    if (UseSettings) { XtSetArg (args[n], XmNset, 1); ++n; }
    w = XmCreateToggleButton (OptPDMenu, "AutoLoadWin", args, n);
    XtManageChild (w);
    addCallback (w, XmNvalueChangedCallback, LoadWinOption, NULL);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'S'); ++n; }
    w = XmCreateToggleButton (OptPDMenu, "AutoSaveEdit", args, n);
    XtManageChild (w);
    addCallback (w, XmNvalueChangedCallback, SaveEditCB, NULL);
}

void build_Help_menu (Widget commands)
{
    Widget HelpButton, m, p;
    int n;

    XtSetArg (args[0], XmNbackground, std_bg);
    p = XmCreatePulldownMenu (commands, "HelpPDMenu", args, 1);

    n = 1;
    XtSetArg (args[n], XmNmnemonic, 'H'); ++n;
    XtSetArg (args[n], XmNsubMenuId, p); ++n;
    HelpButton = XmCreateCascadeButton (commands, "Help", args, n);
    XtManageChild (HelpButton);

    n = 1;
    if (keymn) { XtSetArg (args[n], XmNmnemonic, 'M'); ++n; }
    m = XmCreatePushButton (p, "Manual", args, n);
    XtManageChild (m);
    addCallback (m, XmNactivateCallback, HelpCB, NULL);

    XtSetArg (args[0], XmNmenuHelpWidget, HelpButton);
    XtSetValues (commands, args, 1);
}

void bpEvent (Widget w, XEvent *event, String *params, Cardinal num_params)
{
    if (clickedCommandw) endCommand ();
}

Widget build_StatusArea (Widget WorkRegion)
{
    Widget StatusArea;
    Arg arg;
    Dimension dw;
    int n = 0;

    XtSetArg (args[n], XmNbackground, std_bg); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNshadowThickness, 2); n++;
    XtSetArg (args[n], XmNtopOffset, 1); n++;
    StatusArea = XmCreateForm (WorkRegion, "StatusArea", args, n);
    XtManageChild (StatusArea);

    n = 1;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopOffset, 6); n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNbottomOffset, 6); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, 7); n++;
    XtSetArg (args[n], XmNlabelString, V_str); n++;
    FileInfo = XmCreateLabel (StatusArea, "file:", args, n);
    XtManageChild (FileInfo);

    XtSetArg (arg, XmNwidth, &dw);
    XtGetValues (FileInfo, &arg, 1);
    dw += 40;

    n = 3;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightOffset, dw); n++;
    ADInfo = XmCreateLabel (StatusArea, "       ", args, n);
    XtManageChild (ADInfo);

    n = 3;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNrightWidget, ADInfo); n++;
    RunInfo = XmCreateLabel (StatusArea, "          ", args, n);
    XtManageChild (RunInfo);

    n = 4;
    XtSetArg (args[n], XmNrightWidget, RunInfo); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (args[n], XmNleftWidget, FileInfo); n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
    Slen = 3;
    strcpy (Sfile, "---");
    FileText = XmCreateLabel (StatusArea, Sfile, args, n);
    XtManageChild (FileText);

    n = 2;
    XtSetArg (args[n], XmNtopOffset, 4); n++;
    XtSetArg (args[n], XmNborderColor, std_red); n++;
    XtSetArg (args[n], XmNborderWidth, 2); n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNrightOffset, 10); n++;
    StatusLabel = XmCreateLabel (StatusArea, "Status", args, n);
    XtAddEventHandler (StatusLabel, ButtonPressMask, False, (XtEventHandler)bpEvent, NULL);

    return (StatusArea);
}

void DetailZoomCB (Widget w, caddr_t clientData, caddr_t callData)
{
    doDetailZoom = !doDetailZoom;
}

void LoadWinOption (Widget w, caddr_t clientData, caddr_t callData)
{
    UseSettings = !UseSettings;
}

void SaveEditCB (Widget w, caddr_t clientData, caddr_t callData)
{
    SaveEdit = !SaveEdit;
}

void HelpCB (Widget w, caddr_t clientData, caddr_t callData)
{
    char buf[DM_MAXPATHLEN+40];

 /* sprintf (buf, "xman -notopbox -helpfile %s/share/man/cat1/simeye.1&", icdpath); */
    sprintf (buf, "xterm -geo 68x40 -e icdman simeye&");
    system (buf);
}

void InfoCB (Widget w, int mode, caddr_t callData)
{
    char sb[1024];
    char *ms;

    cW = 0;
    if (clickedCommandw) endCommand ();

    if (w == SaveButton || w == SaveAsButton) {
	if (!Begin_signal)
	    ms = "Sorry, no input signals! (no save)";
	else
	    ms = "Description has unspecified signal(s)! (no save)";
    }
    else if (w == SimuCmd) {
	     if (simuTypeCur == simuTypeOpt[3]) ms = pstarCommand;
	else if (simuTypeCur == simuTypeOpt[2]) ms = spiceCommand;
	else ms = slsCommand;
	sprintf (sb, "Simulation command:\n\n%s 1> %s 2>&1", parseCommand (ms), simdiagFile);
	ms = sb;
    }
    else {
	ms = "Sorry, command not implemented!";
    }

    infoMessage (ms);
}

void edit_sensitive (Boolean mode)
{
    if (mode == True) { /* INPUT mode */
	XtSetArg (args[0], XmNsensitive, False);
	XtSetValues (ReadAppButton, args, 1);
	XtSetArg (args[0], XmNlabelString, E_str);
    }
    else { /* OUTPUT mode */
	XtSetArg (args[0], XmNlabelString, V_str);
    }
    XtSetValues (FileInfo, args, 1);

    XtSetArg (args[0], XmNsensitive, mode);
    XtSetValues (GridButton, args, 1);
    XtSetValues (EraseAllButton, args, 1);
    XtSetValues (CopyButton, args, 1);
    XtSetValues (ChangeButton, args, 1);
    XtSetValues (AddButton, args, 1);
    XtSetValues (YankButton, args, 1);
    XtSetValues (PutButton, args, 1);
    XtSetValues (SpeedButton, args, 1);
    XtSetValues (TendButton, args, 1);
    XtSetValues (SaveButton, args, 1);
    XtSetValues (SaveAsButton, args, 1);
}

void read_sensitive ()
{
    XtSetArg (args[0], XmNsensitive, (Begin_signal ? True : False));
    XtSetValues (ReadAppButton, args, 1);
}

void set_filename (char *name)
{
    static int app;
    XmString str;

    if (Append) {
	if (!name || !*name) return;
	if (app) {
	    strcpy (&Sfile[Slen], " ++ ");
	    strcpy (&Sfile[Slen+4], name);
	}
	else {
	    app = 1;
	    strcpy (&Sfile[Slen], " + ");
	    strcpy (&Sfile[Slen+3], name);
	}
    }
    else {
	app = 0;
	if (!name || !*name) {
	    *Commandfile = 0;
	    name = "---";
	}
	Slen = strlen (name);
	strcpy (Sfile, name);
    }

    str = XmStringCreateLtoR (Sfile, XmSTRING_DEFAULT_CHARSET);
    XtSetArg (args[0], XmNlabelString, str);
    XtSetValues (FileText, args, 1);
    XmStringFree (str);
}

void redrawHandler (Widget w, caddr_t client_data, XEvent *event)
{
    XEvent e;
    Dimension dh, dw;

    if (event -> type == ConfigureNotify) { /* resize */

	rdHandler = 1; /* clear and redraw the screen */

	/* remove the Expose Events if possible */
	while (XCheckTypedWindowEvent (display, XtWindow (canvas), Expose, &e) == True);
	XtSetArg (args[0], XmNwidth, &dw);
	XtSetArg (args[1], XmNheight, &dh);
	XtGetValues (canvas, args, 2);

	canvasWidth  = dw;
	canvasHeight = dh;
#ifdef DEBUG
	fprintf (stderr, "Resize: w=%4d, h=%4d\n", canvasWidth, canvasHeight);
#endif
	calc_canvas ();
    }
    else if (event -> type == Expose) {
#ifdef DEBUG
	fprintf (stderr, "Expose: x=%4d, y=%4d, w=%4d, h=%4d\n",
	    event -> xexpose.x, event -> xexpose.y, event -> xexpose.width, event -> xexpose.height);
#endif
	if (firstExpose > 0) {
	    firstExpose = 0;

	    if (opt_input) {
		set_input_mode ();
		if (*inputname) {
		    XmTextFieldSetString (openSelect, inputname);
		    Inputs (NULL, 0, NULL);
		}
	    }
	    else {
		edit_sensitive (False); /* OUTPUT mode */
		if (*outputname) {
		    set_readSelect (outputname);
		    Read (NULL, 0, NULL);
		}
		read_sensitive ();
	    }

	    firstExpose = -1;
	    if (!question) errorMessage (NULL);
	}
	else {
	    XRectangle R[16];
	    int nr = 0;

	    rdHandler = 1;

	    R[nr].x = event -> xexpose.x;
	    R[nr].y = event -> xexpose.y;
	    R[nr].width  = event -> xexpose.width;
	    R[nr].height = event -> xexpose.height;
	    ++nr;

	    /* get all the Expose Events if possible */
	    while (nr < 16 && XCheckTypedWindowEvent (display,
			XtWindow (canvas), Expose, &e) == True) {
		R[nr].x = e.xexpose.x;
		R[nr].y = e.xexpose.y;
		R[nr].width  = e.xexpose.width;
		R[nr].height = e.xexpose.height;
		++nr;
#ifdef DEBUG
		fprintf (stderr, "Expose%d: x=%4d, y=%4d, w=%4d, h=%4d\n", nr,
		    e.xexpose.x, e.xexpose.y, e.xexpose.width, e.xexpose.height);
#endif
	    }

	    set_clip_area (R, nr);
	}
    }

    if (rdHandler) draw (1, 0, 0, 0, 0);
}

int destroyHandler (Display *display)
{
    int_hdl (SIGTERM);
    return (0);
}

int selectExt (char *name, char *select_ext)
{
    char *s = name;

    while (*s && *s != '.') ++s;

    if (!*s || strncmp (++s, select_ext, 3)) return (0);
    return (1);
}

int stringcompare (const void *e1, const void *e2)
{
    char **s1 = (char **) e1;
    char **s2 = (char **) e2;
    return (strcmp (*s1, *s2));
}

int ownscandir (char *dirname, char ***namelist, char *ext)
{
   int num = 0;
   static char *listbuf[MAXLIST];
   struct dirent *dp;
   DIR *dirp = opendir (dirname);

   while ((dp = readdir (dirp))) {
       if (selectExt (dp -> d_name, ext) && num < MAXLIST) {
           if (!(listbuf[num] = malloc (sizeof (char) * (strlen (dp -> d_name) + 1)))) {
               fprintf (stderr, "Cannot allocate storage\n");
               die (1);
           }
           strcpy (listbuf[num], dp -> d_name);
           num++;
       }
   }
   closedir (dirp);

   qsort (&listbuf[0], num, sizeof (char *), stringcompare);

   *namelist = &listbuf[0];
   return (num);
}

void browse_output ()
{
    char **nlist, **f, *ext, *rs;
    XmString list[MAXLIST+1];
    register int n = 0;
    int num, pos = 0;

    if (readTypeTmp == readTypeOpt[3]) {
	n = 3; ext = "list"; /* Pstar */
    }
    else if (readTypeTmp == readTypeOpt[2]) {
	n = 2; ext = "ana"; /* Spice */
    }
    else if (readTypeTmp == readTypeOpt[0]) {
	n = 0; ext = "plt"; /* Analog */
    }
    else {
	n = 1; ext = "res"; /* Digital */
    }

    rs = readTypeSel[n];
    if (!*rs) pos = -1;

    n = 0;
    if ((num = ownscandir (".", &nlist, ext)) > 0) {
	for (f = nlist; n < num; ++n) {
	    if (n < MAXLIST) {
		list[n] = XmStringCreateLtoR (*f, XmSTRING_DEFAULT_CHARSET);
		if (!pos && strsame (rs, *f)) pos = n + 1;
	    }
	    free (*f++);
	}
    }

    XmListDeleteAllItems (readList);
    if (n) {
	list[n] = NULL;
	XtSetArg (args[0], XmNitems, list);
	XtSetArg (args[1], XmNitemCount, n);
	XtSetValues (readList, args, 2);
	if (pos > 0) XmListSelectPos (readList, pos, False);
	if (n > 8 && pos > 5) {
	    if (pos + 3 > n) pos = n - 3;
	    if (pos > 5) XmListSetPos (readList, pos - 4);
	}
	while (--n >= 0) XmStringFree (list[n]);
    }
    XmTextFieldSetString (readSelect, rs);
}

void readTypeCB (Widget w, caddr_t client_data, caddr_t call_data)
{
    char *s;
    s = XmTextFieldGetString (readSelect);
    (void) skipchar (s);
    if (strlen (s) > MAXNAME) {
	errorMessage ("Too long name!");
	s[MAXNAME] = 0;
    }
    set_readSelect (s);
    XtFree (s);

    readTypeTmp = w;
    browse_output ();
}

void Read (Widget w, int done, caddr_t call_data)
{
    struct stat statbuf;
    char help[MAXNAME+60];

    if (!w) goto skip;

    cW = w;
    if (clickedCommandw) endCommand ();

    if (!done && editing && somethingChanged && Begin_signal) {
	qDialogCB (w, NULL, NULL);
	return;
    }

    if (done < 2) {
	if (w == ReadAppButton) {
	    XtSetArg (args[0], XmNsensitive, False);
	    if (currLogic) {
		XtSetValues (readTypeOpt[0], args, 1);
		XtSetValues (readTypeOpt[2], args, 1);
		XtSetValues (readTypeOpt[3], args, 1);
	    }
	    else {
		XtSetValues (readTypeOpt[1], args, 1);
	    }
	}

	readTypeTmp = readTypeCur;
	XtSetArg (args[0], XmNmenuHistory, readTypeCur);
	XtSetValues (readType, args, 1);
	browse_output ();
	mapCB (readDialog, (caddr_t)5, NULL);
	XtManageChild (readDialog);
	return;
    }
    cW = 0;

    if (!*outputname) {
        strcpy (help, "Cannot read, no simout file specified!");
	goto ret;
    }

skip:
    if (stat (outputname, &statbuf)) {
	sprintf (help, "File \"%s\" not found!", outputname);
	goto ret;
    }

    if (editing) {
	disableEditing ();
	edit_sensitive (False); /* OUTPUT mode */
    }

    if (readTypeCur == readTypeOpt[3]) { /* Pstar */
	Logic = 0;
	Spice = 2;
    }
    else if (readTypeCur == readTypeOpt[2]) { /* Spice */
	Logic = 0;
	Spice = 1;
    }
    else { /* Sls */
	Logic = (readTypeCur == readTypeOpt[0]) ? 0 : 1;
	Spice = 0;
    }

    if (!w || w == ReadButton) {
	set_filename (NULL);
	delSigList (Begin_signal);
	End_signal = Begin_signal = NULL;
	Nr_signals = 0;
	readSignals (0, 0);
	read_sensitive ();
    }
    else if (w == ReadAppButton) readSignals (1, 0);
    return;
ret:
    windowMessage (help, -1);
}

void OpenOkCB (Widget w, int ok, caddr_t call_data)
{
    XtUnmanageChild (openDialog);

    if (ok) {
	char *s = XmTextFieldGetString (openSelect);
	(void) skipchar (s);
	if (strlen (s) > MAXNAME) {
	    errorMessage ("Too long name!");
	    XtFree (s);
	    goto ret;
	}
	strcpy (inputname, s);
	XtFree (s);

	if (cW == SaveAsButton)
	    (void) writeSet (1);
	else
	    Inputs (cW, 2, NULL);
    }
ret:
    cW = 0;
}

void ReadOkCB (Widget w, int ok, caddr_t call_data)
{
    char *s;

    XtUnmanageChild (readDialog);

    if (cW == ReadAppButton) {
	XtSetArg (args[0], XmNsensitive, True);
	if (currLogic) {
	    XtSetValues (readTypeOpt[0], args, 1);
	    XtSetValues (readTypeOpt[2], args, 1);
	    XtSetValues (readTypeOpt[3], args, 1);
	}
	else {
	    XtSetValues (readTypeOpt[1], args, 1);
	}
    }

    s = XmTextFieldGetString (readSelect);
    (void) skipchar (s);
    if (strlen (s) > MAXNAME) {
	errorMessage ("Too long name!");
	s[MAXNAME] = 0;
	ok = 0;
    }
    set_readSelect (s);

    if (ok) {
	readTypeCur = readTypeTmp;
	strcpy (outputname, s);
	Read (cW, 2, NULL);
    }
    XtFree (s);
    cW = 0;
}

void browse_circuit ()
{
    char **l, *rs;
    XmString list[MAXLIST+1];
    register char **s, *t;
    register int n = 0;
    int pos = 0;

    rs = circuitname;
    if (!*rs) pos = -1;
    else {
	for (t = rs; *t; ++t)
	    if (*t == '#') { *t = '\0'; break; }
    }

    if ((l = (char **) dmGetMetaDesignData (CELLLIST, projKey, CIRCUIT))) {

	for (s = l; *s; ++s) ++n;
	qsort (l, n, sizeof (char *), stringcompare);

	n = 0;
	for (s = l; *s && n < MAXLIST; ++s) {
	    list[n++] = XmStringCreateLtoR (*s, XmSTRING_DEFAULT_CHARSET);
	    if (!pos && strsame (rs, *s)) pos = n;
	}
    }

    XmListDeleteAllItems (cirList);
    if (n) {
	list[n] = NULL;
	XtSetArg (args[0], XmNitems, list);
	XtSetArg (args[1], XmNitemCount, n);
	XtSetValues (cirList, args, 2);
	if (pos > 0) XmListSelectPos (cirList, pos, False);
	if (n > 8 && pos > 5) {
	    if (pos + 3 > n) pos = n - 3;
	    if (pos > 5) XmListSetPos (cirList, pos - 4);
	}
	while (--n >= 0) XmStringFree (list[n]);
    }
    XmTextFieldSetString (cirSelect, rs);
}

void browse_stimuli (Widget wList, Widget wSelect)
{
    char **nlist, **f, *rs;
    XmString list[MAXLIST+1];
    register int n = 0;
    int num, pos = 0;

    rs = (wSelect == cmdSelect) ? stimuliname : inputname;
    if (!*rs) pos = -1;

    if ((num = ownscandir (".", &nlist, "cmd")) > 0) {
	for (f = nlist; n < num; ++n) {
	    if (n < MAXLIST) {
		list[n] = XmStringCreateLtoR (*f, XmSTRING_DEFAULT_CHARSET);
		if (!pos && strsame (rs, *f)) pos = n + 1;
	    }
	    free (*f++);
	}
    }

    XmListDeleteAllItems (wList);
    if (n) {
	list[n] = NULL;
	XtSetArg (args[0], XmNitems, list);
	XtSetArg (args[1], XmNitemCount, n);
	XtSetValues (wList, args, 2);
	if (pos > 0) XmListSelectPos (wList, pos, False);
	if (n > 8 && pos > 5) {
	    if (pos + 3 > n) pos = n - 3;
	    if (pos > 5) XmListSetPos (wList, pos - 4);
	}
	while (--n >= 0) XmStringFree (list[n]);
    }
    XmTextFieldSetString (wSelect, rs);
}

void simuTypeCB (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (w != simuTypeTmp) {
	simuTypeTmp = w;
	if (w == simuTypeOpt[0]) { /* sls-logic */
	    simuReadTmp = 0;
	    simuReadCB (simuReadOpt[slslogtype == 'A' ? 0 : 1], (caddr_t)1, NULL);
	}
	else if (w == simuTypeOpt[1]) { /* sls-timing */
	    simuReadTmp = 0;
	    simuReadCB (simuReadOpt[slstimtype == 'A' ? 0 : 1], (caddr_t)1, NULL);
	}
	else { /* spice */
	    simuReadCB (simuReadOpt[0], (caddr_t)1, NULL);
	}
    }
}

void simuReadCB (Widget w, caddr_t set_value, caddr_t call_data)
{
    if (w != simuReadTmp) {
	simuReadTmp = w;
	if (w == simuReadOpt[0]) { /* Analog */
	    if (simuTypeTmp == simuTypeOpt[0]) { /* sls-logic */
		slslogtype = 'A';
	    }
	    else if (simuTypeTmp == simuTypeOpt[1]) { /* sls-timing */
		slstimtype = 'A';
	    }
	}
	else { /* Digital */
	    if (simuTypeTmp == simuTypeOpt[0]) { /* sls-logic */
		slslogtype = 'D';
	    }
	    else if (simuTypeTmp == simuTypeOpt[1]) { /* sls-timing */
		slstimtype = 'D';
	    }
	    else { /* spice */
		simuReadTmp = simuReadOpt[0]; /* Analog */
		goto set;
	    }
	}
	if (set_value) {
set:
	    XtSetArg (args[0], XmNmenuHistory, simuReadTmp);
	    XtSetValues (simuRead, args, 1);
	}
    }
}

void SimuOkCB (Widget w, int ok, caddr_t call_data)
{
    char *s;
    XtUnmanageChild (simuDialog);

    if (ok) {
	XmString str;
	s = XmTextFieldGetString (cirSelect);
	(void) skipchar (s);
	if (strlen (s) > MAXNAME-4) goto err;
	strcpy (circuitname, s);
	XtFree (s);

	s = XmTextFieldGetString (cmdSelect);
	(void) skipchar (s);
	if (strlen (s) > MAXNAME) goto err;
	strcpy (stimuliname, s);
	XtFree (s);

	XtSetArg (args[0], XmNlabelString, &str);
	XtGetValues (simuTypeTmp, args, 1);
	XtSetArg (args[0], XmNlabelString, str);
	XtSetValues (RunInfo, args, 1);
	XmStringFree (str);

	XtSetArg (args[0], XmNlabelString, &str);
	XtGetValues (simuReadTmp, args, 1);
	XtSetArg (args[0], XmNlabelString, str);
	XtSetValues (ADInfo, args, 1);
	XmStringFree (str);

	simuReadCur = simuReadTmp;
	simuTypeCur = simuTypeTmp;

	if (ok == 2) RunCB (RunButton, NULL, NULL);
	if (ok == 3) EditCB (EditButton, NULL, NULL);
    }
    return;
err:
    errorMessage ("Too long name!");
    XtFree (s);
}

void Simulate (Widget w, int done, caddr_t call_data)
{
    struct stat statbuf;
    char tmp_commandfile[20];
    char command[800];
    char buf[MAXNAME+80];
    char *cname = NULL;
    char *sname = NULL;
    char *ms;
    int u, foundCmd;
    XmString str;

    if (clickedCommandw) endCommand ();

    cW = w;
    if (done < 2) {
	simuReadTmp = simuReadCur;
	simuTypeTmp = simuTypeCur;
	XtSetArg (args[0], XmNmenuHistory, simuReadCur);
	XtSetValues (simuRead, args, 1);
	XtSetArg (args[0], XmNmenuHistory, simuTypeCur);
	XtSetValues (simuType, args, 1);

	browse_circuit ();
	browse_stimuli (cmdList, cmdSelect);
	mapCB (simuDialog, (caddr_t)6, NULL);
	XtManageChild (simuDialog);
	return;
    }
    cW = 0;

    if (!*circuitname) {
        strcpy (buf, "Cannot simulate, no circuit name specified!");
	goto ret;
    }
    if (!*stimuliname) {
        strcpy (buf, "Cannot simulate, no stimuli file specified!");
	goto ret;
    }
    *buf = 0;

    cname = circuitname;
    sname = stimuliname;

    if (!existCell (cname, CIRCUIT)) {
        sprintf (buf, "Cannot simulate, cell \"%s\" in view \"circuit\" not found!", circuitname);
	goto ret;
    }

    foundCmd = 0;
    if (stat (stimuliname, &statbuf) == 0) foundCmd = 1;
    else if (tryNonCapital && isupper ((int)*stimuliname)) {

	/* decapitalize the first character and try to open that file */

	*stimuliname += LOWER;
	if (stat (stimuliname, &statbuf) == 0) {
	    foundCmd = 1;
	    usedNonCapital = 1;
	}
	else *stimuliname -= LOWER;
    }
    if (!foundCmd) {
	sprintf (buf, "Cannot simulate, commandfile \"%s\" not found!", stimuliname);
	goto ret;
    }

    if (editing) {
	disableEditing ();
	edit_sensitive (False); /* OUTPUT mode */
    }

    if (simuTypeCur == simuTypeOpt[3]) {
	Spice = 2;
	Logic = 0;
	SimLevel = 3;
    }
    else if (simuTypeCur == simuTypeOpt[2]) {
	Spice = 1;
	Logic = 0;
	SimLevel = 3;
    }
    else {
	Spice = 0;
	Logic = (simuReadCur == simuReadOpt[0]) ? 0 : 1;
	SimLevel = (simuTypeCur == simuTypeOpt[0]) ? slsLogicLevel : 3;
    }

    set_filename (NULL);
    clear ();

    delSigList (Begin_signal);
    End_signal = Begin_signal = NULL;
    Nr_signals = 0;

    /* update commandfile for set, print & plot commands, level etc. */

    if (!updateCommandfile (cname, sname)) goto ret;

    /* and prepare for simulation */

    if (usedNonCapital) {
	sprintf (tmp_commandfile, "s%d.cxx", (int)getpid ());
	sprintf (command, "mv %s %s", stimuliname, tmp_commandfile);
	system (command);
	sprintf (command, "arrexp %s > %s", tmp_commandfile, stimuliname);
	if (system (command) != 0) {
	    windowMessage ("Cannot execute arrexp, no simulation!", -1);
	    goto end_of_simul;
	}
        sprintf (buf, ".... simulating (using commandfile \"%s\" instead; hit control-C to stop) ....", stimuliname);
    }
    else
        strcpy (buf, ".... simulating (hit control-C to stop) ....");

    XtSetArg (args[0], XmNlabelString, &str);
    XtGetValues (RunButton, args, 1);
    XtSetArg (args[0], XmNlabelString, str);
    XtSetValues (StatusLabel, args, 1);
    XmStringFree (str);

    XtManageChild (StatusLabel);

    windowMessage (buf, -2);

    /* Clean up previous simulation result */

    if (Spice && (old_spice_mode || Spice == 2)) {
	sprintf (outputname, "%s.%s", circuitname, Spice == 2 ? "list" : "ana");
	if (stat (outputname, &statbuf) == 0) unlink (outputname);
    }
    else if (Spice) {
	sprintf (outputname, "%s.ana", circuitname);
	if (stat (outputname, &statbuf) == 0) unlink (outputname);
    }
    else {
	sprintf (outputname, "%s.res", circuitname);
	if (stat (outputname, &statbuf) == 0) unlink (outputname);
	sprintf (buf, "%s.plt", circuitname);
	if (stat (buf, &statbuf) == 0) unlink (buf);
	if (!Logic) strcpy (outputname, buf);
    }

	 if (Spice == 2) ms = pstarCommand;
    else if (Spice == 1) ms = spiceCommand;
    else ms = slsCommand;
    sprintf (command, "%s", parseCommand (ms));

    u = runTool (command, (!Spice && s_simperiod > 0));

    XtUnmanageChild (StatusLabel);

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
	sprintf (command, "mv %s %s", tmp_commandfile, stimuliname);
	system (command);
    }
    read_sensitive ();
    *buf = 0;
ret:
    usedNonCapital = 0;
    if (*buf) windowMessage (buf, -1);
}

void Windowmax (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();
    draw ('m', 0, 0, 0, 0);
}

void Savewin (Widget w, caddr_t client_data, caddr_t call_data)
{
    char buf[800];
    FILE *fp;
    struct signal *sig;

    if (clickedCommandw) endCommand ();

    if (!Begin_signal) {
	windowMessage ("No signals, settings not saved!", -1);
	return;
    }

    if (!(fp = fopen (parseCommand (settingsFile), "w"))) {
	sprintf (buf, "Cannot write %s", parseCommand (settingsFile));
	windowMessage (buf, -1);
	return;
    }

    fprintf (fp, "t_start: %lld\n", startTime);
    fprintf (fp, "t_stop: %lld\n", stopTime);

    for (sig = Top_signal; sig; sig = sig -> next) {
	fprintf (fp, "sig: %s\n", sig -> name);
	if (sig == Bottom_signal) break;
    }

    fclose (fp);
    windowMessage ("Settings saved", -1);
}

void Loadwin (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    if (loadSettings ()) {
	draw ('a', 0, 0, 0, 0);
	windowMessage ("Settings loaded", -1);
    }
}

void Zoomin (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();
    beginCommand (w, ZOOMIN);
}

void Zoomout (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();
    beginCommand (w, ZOOMOUT);
}

void Redraw (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();
    draw (1, 0, 0, 0, 0);
}

void SliderCB (Widget w, caddr_t clientData, caddr_t callData)
{
    int nr;

    if (clickedCommandw) endCommand ();

    if (!Begin_signal) return;

    XtSetArg (args[0], XmNvalue, &nr);
    XtGetValues (w, args, 1);

    if (w == HorBar) {
	stopTime = stopTime - startTime + nr;
	startTime = nr;
    }
    else { /* VerBar */
	struct signal *sig;
	int i = -1;

	for (sig = End_signal; sig; sig = sig -> prev) {
	    if (++i == nr) {
		Bottom_signal = sig;
		break;
	    }
	}
	for (i = 0; sig; sig = sig -> prev) {
	    if (++i == Curr_nr_signals) {
		Top_signal = sig;
		break;
	    }
	}
    }

    doSlider = 1;
    draw (0, 0, 0, 0, 0);
    doSlider = 0;
}

void Pointer (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();
    beginCommand (w, MEASURE);
}

void MoveCB (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();
    beginCommand (w, MOVESIG);
}

void Hardcopy (Widget w, caddr_t done, caddr_t call_data)
{
    char prt_cmd[800];

    if (!done) {
	if (clickedCommandw) endCommand ();

	if (printLabel && printLabel[0] != '\0')
	    windowMessage (parseCommand (printLabel), -2);
	else
	    windowMessage (NULL, -2);

	qDialogCB (w, NULL, NULL);
	return;
    }

    sprintf (prt_cmd, "xwd -id 0x%lx -out %s 1> pr.diag 2>&1", XtWindow (canvas), parseCommand (xdumpFile));
    if (system (prt_cmd) == 0) {
	if (printCommand && printCommand[0] != '\0') {
	    windowMessage (".... printing ....", -2);

	    sprintf (prt_cmd, "%s 1> pr.diag 2>&1", parseCommand (printCommand));
	    if (system (prt_cmd) == 0)
		windowMessage ("print ready", -1);
	    else
		windowList ("pr.diag");
	}
	else
	    windowMessage ("print ready", -1);
    }
    else
	windowList ("pr.diag");
}

void Print (Widget w, caddr_t client_data, caddr_t call_data)
{
    char x2pfile[MAXNAME];

    if (clickedCommandw) endCommand ();

    if (printLabel && printLabel[0] != '\0')
	windowMessage (parseCommand (printLabel), -1);
    else
	windowMessage (NULL, -1);

    if (*Sfile == '-')
	strcpy (x2pfile, "simeye.eps");
    else
	sprintf (x2pfile, "%s.eps", parseCommand ("$cell"));

    XtSetArg (args[0], XtNx2PostScriptFile, x2pfile);
    XtSetArg (args[1], XtNformat, XpEPS);
    XtSetValues (theX2PostScriptWidget, args, 2);
    mapCB (theX2PostScriptWidget, (caddr_t)7, NULL);
    XtManageChild (theX2PostScriptWidget);
}

int signals_ok ()
{
    register struct signal *sig;
    if (!(sig = Begin_signal)) return (0);
    do {
	if (!sig -> begin_value) return (0);
    } while ((sig = sig -> next));
    return (1);
}

void SaveAsCB (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    cW = w;
    if (!signals_ok ()) InfoCB (cW, 0, NULL);
    else {
	XmString str = XmStringLtoRCreate ("Simeye SaveAs Dialog", XmSTRING_DEFAULT_CHARSET);
	XtSetArg (args[0], XmNdialogTitle, str);
	XtSetValues (openDialog, args, 1);
	XmStringFree (str);
	str = XmStringLtoRCreate (" Save ", XmSTRING_DEFAULT_CHARSET);
	XtSetArg (args[0], XmNlabelString, str);
	XtSetValues (openOkButton, args, 1);
	XmStringFree (str);
	browse_stimuli (openList, openSelect);
	mapCB (openDialog, (caddr_t)8, NULL);
	XtManageChild (openDialog);
    }
}

void Inputs (Widget w, int done, caddr_t call_data)
{
    if (!w) goto skip;

    cW = w;
    if (clickedCommandw) endCommand ();

    if (!done && editing && somethingChanged && Begin_signal) {
	qDialogCB (w, NULL, NULL);
	return;
    }

    if (done < 2) {
	XmString str = XmStringLtoRCreate ("Simeye Open Dialog", XmSTRING_DEFAULT_CHARSET);
	XtSetArg (args[0], XmNdialogTitle, str);
	XtSetValues (openDialog, args, 1);
	XmStringFree (str);
	str = XmStringLtoRCreate (" Open ", XmSTRING_DEFAULT_CHARSET);
	XtSetArg (args[0], XmNlabelString, str);
	XtSetValues (openOkButton, args, 1);
	XmStringFree (str);
	browse_stimuli (openList, openSelect);
	mapCB (openDialog, (caddr_t)8, NULL);
	XtManageChild (openDialog);
	return;
    }

    disableEditing ();
    clear ();
    set_input_mode ();

skip:
    cW = 0;
    readSet (0);
}

void ask_default (char *s)
{
    question = 1;
    qDialogCB (OpenButton, s, NULL);
}

void set_input_mode ()
{
    currLogic = 1;
    editing = 1; /* enableEditing */
    edit_sensitive (True); /* INPUT mode */
    Global_umin = 0;
    Global_umax = 2;
    usedNonCapital = 0;

    Append = 0;
    Spice = 0;
    delSigList (Begin_signal);
    End_signal = Begin_signal = NULL;
    Nr_signals = 0;

    SimEndtime = Endtime = Begintime = 0;
}

void New ()
{
    if (editing) disableEditing ();

    set_input_mode ();

    set_filename (NULL);
    clear ();
    draw ('f', 0, 0, 0, 0);
}

static XmString pD_Add;
static XmString pD_Grid;
static XmString pD_Put;
static XmString pD_Speed;
static XmString pD_Tend;

void pDialogCB (Widget w, caddr_t client_data, caddr_t call_data)
{
    XmString str0, str1, str2;
    char sb[256];
    char *s;

    pW = w;
    if (clickedCommandw) {
	if ((clickedCommandw != PutButton || w != PutButton) &&
	 (clickedCommandw != RenameButton || w != RenameButton)) endCommand ();
    }

    str0 = NULL;
    if (w == AddButton) {
	if (Timescaling <= 0) goto grid_first_msg;
	str0 = pD_Add;
	s = "Enter new signal name:";
    }
    else if (w == RenameButton) {
	if ((s = client_data)) {
	    if (pD_Add) XmStringFree (pD_Add);
	    pD_Add = XmStringCreateLtoR (s, XmSTRING_DEFAULT_CHARSET);
	}
	str0 = pD_Add;
	if (editing)
	    s = "Enter new signal name:";
	else
	    s = "Signal name:";
    }
    else if (w == GridButton) {
	str0 = pD_Grid;
	if (Timescaling > 0) {
	    sprintf (sb, "Enter grid value: (current = %.2le)", Timescaling);
	    s = sb;
	}
	else
	    s = "Enter grid value:";
    }
    else if (w == PutButton) {
	str0 = pD_Put;
	s = "Enter number of periods (-1 = infinite):";
    }
    else if (w == SpeedButton) {
	str0 = pD_Speed;
	if (Timescaling <= 0) goto grid_first_msg;
	s = "Enter signal speed-up factor:";
    }
    else if (w == TendButton) {
	str0 = pD_Tend;
	if (Timescaling <= 0) goto grid_first_msg;
	if (SimEndtime > 0) {
	    int i;
	    simtime_t t;
	    char buf[132];

	    t = SimEndtime;
	    for (i = 0; t > 9; ++i) t = t / 10;
	    sprintf (buf, "Enter end time: (current = %%.%dle)", i);
	    sprintf (sb, buf, SimEndtime * Timescaling);
	    s = sb;
	}
	else
	    s = "Enter end time:";
    }
    str1 = XmStringCreateLtoR (s, XmSTRING_DEFAULT_CHARSET);

    XtSetArg (args[0], XmNlabelString, &str2);
    XtGetValues (w, args, 1);
    if (XmStringGetLtoR (str2, XmSTRING_DEFAULT_CHARSET, &s) != True) s = "";
    XmStringFree (str2);
    sprintf (sb, "Simeye %s Dialog", s);
    str2 = XmStringCreateLtoR (sb, XmSTRING_DEFAULT_CHARSET);

    if (!str0) str0 = N_str;
    XtSetArg (args[0], XmNdialogTitle, str2);
    XtSetArg (args[1], XmNselectionLabelString, str1);
    XtSetArg (args[2], XmNtextString, str0);
    XtSetValues (pDialog, args, 3);
    XmStringFree (str1);
    XmStringFree (str2);
    XtManageChild (pDialog);
    return;

grid_first_msg:
    infoMessage ("Enter grid first!");
}

void qDialogCB (Widget w, char *client_data, caddr_t call_data)
{
    XmString str1, str2;
    char sb[MAXNAME + 256];
    char cmd[16], *s;

    XtSetArg (args[0], XmNlabelString, &str2);
    XtGetValues (w, args, 1);
    XmStringGetLtoR (str2, XmSTRING_DEFAULT_CHARSET, &s);
    strcpy (cmd, s);
    XmStringFree (str2);

    qW = w;
    if (clickedCommandw) endCommand ();

    if (w == HardcButton) {
	s = "Hardcopy, are you sure?";
    }
    else if (w == EraseAllButton) {
	if (!Begin_signal) return;
	s = "Erase all signals, are you sure?";
    }
    else if (w == ExitButton || w == NewButton) {
	char *x = "";
	char *y = "";
	if (editing && somethingChanged && Begin_signal) {
	    if (!SaveEdit) goto q2;
	    if (!signals_ok ()) {
		y = "Description has unspecified signal(s)!\n";
		x = "(no save) ";
	    }
	    else {
		x = "(and save) ";
		cW = w;
		qW = SaveButton;
	    }
	}
	s = sb;
	sprintf (s, "%s%s %sare you sure?", y, cmd, x);
    }
    else if (w == SaveButton) {
	s = sb;
	if (somethingChanged) {
	    sprintf (s, "There is something changed!\nDo you want to update \"%s\"?",
		*Commandfile ? Commandfile : "...");
	}
	else
	    sprintf (s, "Update \"%s\", are you sure?\nThere is nothing changed!",
		*Commandfile ? Commandfile : "...");
    }
    else if (w == EditButton) {
	s = sb;
	strcpy (s, "You are already editing this stimuli file!\nEdit (no save) are you sure?");
    }
    else {
	if (w == OpenButton && !cW) s = client_data;
	else {
q2:	    s = sb;
	    sprintf (s, "There is something edited!\n%s (no save) are you sure?", cmd);
	}
    }

    str1 = XmStringCreateLtoR (s, XmSTRING_DEFAULT_CHARSET);
    sprintf (sb, "Simeye %s Dialog", cmd);
    str2 = XmStringCreateLtoR (sb, XmSTRING_DEFAULT_CHARSET);
    XtSetArg (args[0], XmNdialogTitle,   str2);
    XtSetArg (args[1], XmNmessageString, str1);
    XtSetValues (qDialog, args, 2);
    XmStringFree (str2);
    XmStringFree (str1);
    XtManageChild (qDialog);
}

void CancelCB (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (w == pDialog) {
	if (pW == PutButton || pW == RenameButton) PutCancel ();
    }
    else if (w == qDialog) {
	if (qW == OpenButton && !cW) readSet (2);
	else if (qW == SaveButton) {
	    if (cW == RunButton) Simulate (SimuButton, 2, NULL);
	    else if (cW == EditButton) Inputs (OpenButton, 2, NULL);
	}
    }
    cW = 0;
}

void OkCB (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (w == pDialog) {
	XmString str;
	char sb[MAXNAME+2];
	char *s;
	register int i;
	int skip = 0;

	if (pW == RenameButton && !editing) { PutCancel (); return; }

	XtSetArg (args[0], XmNtextString, &str);
	XtGetValues (pDialog, args, 1);
	if (XmStringGetLtoR (str, XmSTRING_DEFAULT_CHARSET, &s) != True) s = NULL;
	if (!s) {
	    XmStringFree (str);
	    if (pW == PutButton || pW == RenameButton) PutCancel ();
	    return;
	}
	for (i = 0; *s; ++s) {
	    if (isalnum ((int)*s)
		|| *s == '_' || *s == '.'
		|| *s == '[' || *s == ']'
		|| *s == '-' || *s == '+') {

		if (i < MAXNAME) sb[i++] = *s;
		else { skip = 1; break; }
	    }
	    else skip = 1;
	}
	sb[i] = '\0';
	s = sb;
	if (skip) {
	    XmStringFree (str);
	    windowMessage ("warning: some input chars skipped", -1);
	    str = XmStringCreateLtoR (s, XmSTRING_DEFAULT_CHARSET);
	}
	if (!*s) { /* nothing specified == cancel */
	    XmStringFree (str);
	    if (pW == PutButton || pW == RenameButton) PutCancel ();
	    return;
	}

	if (pW == AddButton || pW == RenameButton) {
	    char buf[MAXNAME+10];

	    if (pD_Add) XmStringFree (pD_Add); pD_Add = str;
	    if (existSignal (s)) {
		sprintf (buf, "Signal '%s' already exists!", s);
		infoMessage (buf);
		if (pW == RenameButton) PutCancel ();
		return;
	    }
	    if (pW == RenameButton) RenameOk (s);
	    else newSigOnCanvas (s);
	    draw (1, 0, 0, 0, 0);
	    somethingChanged = 1;
	}
	else if (pW == GridButton) {
	    double newTs;

	    if (pD_Grid) XmStringFree (pD_Grid); pD_Grid = str;
	    if (sscanf (s, "%le", &newTs) != 1 || newTs <= 0) {
		windowMessage ("error: value unchanged", -1);
		return;
	    }
	    if (Timescaling <= 0) { /* not set */
		Global_umin = 0;
		Global_umax = 2;
		changeTimescaling (newTs);
	    }
	    else {
		if (Endtime * Timescaling / newTs >= MAXTIME) {
		    windowMessage ("time resolution becomes too high", -1);
		    return;
		}
		changeTimescaling (newTs);
		draw (1, 0, 0, 0, 0);
	    }
	    somethingChanged = 1;
	}
	else if (pW == PutButton) {
	    int factor;

	    if (pD_Put) XmStringFree (pD_Put); pD_Put = str;
	    if (sscanf (s, "%d", &factor) != 1) {
		infoMessage ("Dialog error (no put)!");
		factor = 0;
	    }
	    if (factor)
		PutOk (factor);
	    else
		PutCancel ();
	}
	else if (pW == SpeedButton) {
	    double tfactor;

	    if (pD_Speed) XmStringFree (pD_Speed); pD_Speed = str;
	    if (sscanf (s, "%le", &tfactor) == 1 && tfactor > 0) {
		Timescaling = Timescaling / tfactor;
		draw (1, 0, 0, 0, 0);
		somethingChanged = 1;
	    }
	    else
		windowMessage ("error: value unchanged", -1);
	}
	else if (pW == TendButton) {
	    double newEt;

	    if (pD_Tend) XmStringFree (pD_Tend); pD_Tend = str;
	    if (sscanf (s, "%le", &newEt) == 1 && newEt > 0) {
		if (newEt / Timescaling > MAXTIME) {
		    windowMessage ("too large value specified", -1);
		    return;
		}
		changeSimEndtime (newEt);
		draw (1, 0, 0, 0, 0);
		somethingChanged = 1;
	    }
	    else
		windowMessage ("error: value unchanged", -1);
	}
    }
    else if (w == qDialog) {

	if (qW == HardcButton) {
	    XRaiseWindow (display, XtWindow (Toplevel));
	    XtAppAddTimeOut (XtDisplayToApplicationContext (display), (unsigned long)500,
		(XtTimerCallbackProc)Hardcopy, (XtPointer)1);
	}
	else if (qW == EraseAllButton) {
	    delSigList (Begin_signal); /* erase all signals */
	    End_signal = Begin_signal = NULL;
	    Nr_signals = 0;
	    clear ();
	    draw ('f', 0, 0, 0, 0);
	    somethingChanged = 1;
	}
	else if (qW == OpenButton) {
	    if (cW) Inputs (qW, 1, NULL);
	    else readSet (1);
	}
	else if (qW == SaveButton) {
	    if (!signals_ok ()) {
		InfoCB (qW, 0, NULL);
		return;
	    }
	    if (!*Commandfile) SaveAsCB (SaveAsButton, NULL, NULL);
	    else {
		(void) writeSet (0);
		if (cW == RunButton) Simulate (SimuButton, 2, NULL);
		else if (cW == EditButton) Inputs (OpenButton, 2, NULL);
		else if (cW == ExitButton) die (0);
		else if (cW == NewButton) { cW = 0; New (); }
	    }
	}
	else if (qW == ReadButton || qW == ReadAppButton) Read (cW, 1, NULL);
	else if (qW == ExitButton) die (0);
	else if (qW == NewButton)  New ();
	else if (qW == EditButton) {
	    somethingChanged = 0;
	    EditCB (qW, NULL, NULL);
	}
    }
}

int readSignals (int flag, int intermediate)
/* flag = 1 : append mode
** intermediate = 0  : no intermediate read
**              = 2  : start intermediate read
**              = 1  : intermediate read
**              = -1 : end intermediate read
** return = 0 : error
**        = 1 : ok
**        = 2 : ok, but message printed
*/
{
    Append = (flag && Begin_signal);

    if (!intermediate && !Append) clear ();

    if (Append && Logic != currLogic) {
	windowMessage ("Cannot merge logical and analog signals", -1);
	return (0);
    }

    if (Logic) {
	if (Spice) {
	    windowMessage ("No logic signals for spice/pstar", -1);
	    return (0);
	}
	else {
	    if (readLogic (intermediate) == 0) return (0);
	}
    }
    else {
	if (Spice) {
	    if (readSpice (intermediate) == 0) return (0);
	}
	else {
	    if (readWaves (intermediate) == 0) return (0);
	}
    }

    currLogic = Logic;

    if (intermediate && s_simperiod > 0 && s_sigunit > 0) {
	Endtime = s_simperiod * s_sigunit / Timescaling;
    }

    if (UseSettings && !Append && (intermediate == 0 || intermediate == 2)) {
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
    int i;
    int x;
    char simoutfile[MAXNAME + 2];
    char buf[MAXNAME + 60];
    char c;
    FILE *fp;

    strcpy (simoutfile, outputname);

    x = strlen (simoutfile);
    for (i = x - 1; i >= 0 && simoutfile[i] != '.'; i--);

    if (simoutfile[i] == '.')
	strcpy (simoutfile + i, ".out");
    else
	strcpy (simoutfile + x, ".out");

    if (!(fp = fopen (simoutfile, "r"))) {
	sprintf (buf, "Warning: cannot open \"%s\"", simoutfile);
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

int loadSettings ()
{
    FILE *fp;
    char buf[800];
    char c;
    int error;
    char name[128];
    simtime_t newStartTime;
    simtime_t newStopTime;
    struct signal *sig;
    struct signal *newTop_signal;
    struct signal *newBottom_signal;
    int newCurr_nr_signals;

    if (!(fp = fopen (parseCommand (settingsFile), "r"))) {
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

	if (strsame (buf, "spice:")) {
	}
	else if (strsame (buf, "simlevel:")) {
	}
	else if (strsame (buf, "level3logic:")) {
	}
	else if (strsame (buf, "t_start:")) {
	    if (fscanf (fp, "%lld", &newStartTime) <= 0) error = 1;
	}
	else if (strsame (buf, "t_stop:")) {
	    if (fscanf (fp, "%lld", &newStopTime) <= 0) error = 1;
	}
	else if (strsame (buf, "sig:")) {
	    if (fscanf (fp, "%s", name) <= 0) error = 1;
	    else {
		for (sig = (newBottom_signal ?
			    newBottom_signal -> next : Begin_signal);
		     sig; sig = sig -> next) {
		    if (strsame (sig -> name, name)) break;
		}
		if (sig) {
		    if (newTop_signal == NULL) {
			newTop_signal = sig;
			newBottom_signal = sig;
			if (sig != Begin_signal) {
			    if (sig == End_signal)
				End_signal = sig -> prev;
			    if (sig -> prev)
				sig -> prev -> next = sig -> next;
			    if (sig -> next)
				sig -> next -> prev = sig -> prev;
			    sig -> prev = NULL;
			    sig -> next = Begin_signal;
			    Begin_signal -> prev = sig;
			    Begin_signal = sig;
			}
		    }
		    else if (sig != newBottom_signal) {
			if (newBottom_signal -> next != sig) {
			    if (sig == End_signal)
				End_signal = sig -> prev;
			    if (sig == Begin_signal)
				Begin_signal = sig -> next;
			    if (sig -> prev)
				sig -> prev -> next = sig -> next;
			    if (sig -> next)
				sig -> next -> prev = sig -> prev;
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

void testcommand (char *line, char *fn, char *sim)
{
    char ms[512];
    char item[32];
    register char *f, *s, *t;
    int found = 0;

    s = line;
    while (*s) {
	if (*s++ == '$') {
	    if (!strncmp (s, "cell.cmd", 8)) { /* $stimuli */
		*s++ = 's';
		*s++ = 't';
		*s++ = 'i';
		*s++ = 'm';
		*s++ = 'u';
		*s++ = 'l';
		*s++ = 'i';
		t = s;
		f = s + 1;;
		while (*f) *t++ = *f++;
		*t = 0;
		found |= 2;
	    }
	    else if (!strncmp (s, "cell", 4)) { /* $circuit */
		s += 4;
		f = s + strlen (s);
		t = f + 3;
		while (f >= s) *t-- = *f--;
		s -= 4;
		*s++ = 'c';
		*s++ = 'i';
		*s++ = 'r';
		*s++ = 'c';
		*s++ = 'u';
		*s++ = 'i';
		*s++ = 't';
		found |= 1;
	    }
	    else if (!(found&1) && !strncmp (s, "circuit", 7)) found |= 1;
	    else if (found < 2  && !strncmp (s, "stimuli", 7)) found |= 2;
	}
    }
    if (found != 3) {
	if (!found) strcpy (item, "$circuit and $stimuli");
	else if (found == 1) strcpy (item, "$stimuli");
	else strcpy (item, "$circuit");
	sprintf (ms, "File \"%s\", %s: %s not found!", fn, sim, item);
	errorMessage (ms);
    }
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
    char buf[512];
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
		testcommand (buf, fn, "SLS");
		NEW (slsCommand, strlen (buf) + 1, char);
		strcpy (slsCommand, buf);
	    }
	    else {
		rcReadError (fn, "SLS:");
	    }
	}
	else if (strsame (buf, "SPICE:")) {
	    if (myscanf (fp, buf) > 0) {
		testcommand (buf, fn, "SPICE");
		NEW (spiceCommand, strlen (buf) + 1, char);
		strcpy (spiceCommand, buf);
	    }
	    else {
		rcReadError (fn, "SPICE:");
	    }
	}
	else if (strsame (buf, "PSTAR:")) {
	    if (myscanf (fp, buf) > 0) {
		testcommand (buf, fn, "PSTAR");
		NEW (pstarCommand, strlen (buf) + 1, char);
		strcpy (pstarCommand, buf);
	    }
	    else {
		rcReadError (fn, "PSTAR:");
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
    fprintf (stderr, "Error in file \"%s\" for '%s'\n", fn, s);
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
    char c;
    int e, n;

    s_tm = 0;

    p = buf;
    while (*s) {
    if (*s == '$') {
	++s;
	if (!strncmp (s, "cell", 4) || !strncmp (s, "file", 4)) {

	    n = Slen - 4;
	    if (n < 1 || Sfile[n] != '.')
		if (--n < 1 || Sfile[n] != '.') n = Slen;
	    e = Slen - n;

	    if (n > 22) n = 22;
	    c = Sfile[n]; Sfile[n] = 0;
	    for (t = Sfile; *t;) *p++ = *t++;
	    Sfile[n] = c;
	    if (e && *s == 'f') {
		c = Sfile[Slen]; Sfile[Slen] = 0;
		for (t = Sfile + Slen - e; *t;) *p++ = *t++;
		Sfile[Slen] = c;
	    }
	    s += 4;
	}
	else if (!strncmp (s, "circuit", 7) || !strncmp (s, "stimuli", 7)) {

	    t = *s == 'c' ? circuitname : stimuliname;
	    s += 7;
	    if (!*t) { t = s - 8; while (t != s) *p++ = *t++; }
	    else while (*t) *p++ = *t++;
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
		sprintf (help, "%02d %s %d", s_tm -> tm_mday, t, s_tm -> tm_year + 1900);
	    }
	    for (t = help; *t;) *p++ = *t++;
	    s += 4;
	}
	else if (!strncmp (s, "user", 4)) {
	    s += 4;
	    if (!(t = username)) { t = s - 5; while (t != s) *p++ = *t++; }
	    else while (*t) *p++ = *t++;
	}
	else {
	    *p++ = '$';
	}
    }
    else
	*p++ = *s++;
    }
    *p = 0;

    if (p - buf >= 750) {
	fprintf (stderr, "%s: Error, buffer overflow in parseCommand()!\n", argv0);
	die (1);
    }
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

int runTool (char *s, int monitorSig)
/* monitorSig = 0 : draw final result
**              1 : draw intermediate and final result
*/
{
    int v, w;
    int argno = 0;
    char *argv[16];
    char buf[800];
    char *p;
    int readSigMes;
    XEvent event, *ev;
    int probe_time = 3; /* at every 'probe_time' seconds new simulation results are displayed */
    int mon_cnt;

    strcpy (buf, s);
    p = buf;
    if (!*p) {
	fprintf (stderr, "He! No command specified for runTool()!\n");
	return (-1);
    }
    do {
	if (argno < 16)
	    argv[argno++] = p;
	else {
	    fprintf (stderr, "He! Too many arguments for runTool()!\n");
	    return (-1);
	}
	while (*p != ' ' && *p) ++p;
	if (*p == ' ') *p++ = '\0';
	while (*p == ' ') ++p;
    } while (*p);
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
		    ev = &event;
		    XNextEvent (display, ev);
		    if (ev -> type == Expose) {
			XtDispatchEvent (ev);   /* for redrawing menu buttons */
		    }
		    if (ev -> type == ConfigureNotify) {
			XtDispatchEvent (ev);   /* for resizing widgets */
		    }
		    if (checkEvent (ev) == True) {

			/* the tool is stopped by an interrupt from simeye */

			/* kill all processes of the process group, except
			   simeye itself */

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
			/* successful return, start intermediate reading */
			readSigMes = 1;
		    }
		    mon_cnt = 0;
		}
	    }
	    w = 0;
	    if (errorInFile (simdiagFile))
		toolReadyStatus = 1;
	    else if (monitorSig == 1 && readSigMes == 1)
		w = readSignals (0, -1);  /* end intermediate reading */
	    else
		w = readSignals (0, 0);   /* normal (single) read */
	    if (toolReadyStatus == 0 && w == 2)
		toolReadyStatus = 2;
            break;
        case CHILD:
	    /* attach stdout and stderr to "simdiagFile" */
	    freopen (simdiagFile, "w", stdout);
	    freopen (simdiagFile, "w", stderr);
            execvp (argv[0], argv);
	    fprintf (stderr, "Cannot execute %s\n", argv[0]);
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
     char buf[512];
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

Bool checkEvent (XEvent *event)
{
    char buf[32];
    int bufnr = 32;
    KeySym keysym;
    XComposeStatus status;

    if (event -> type == KeyPress) {
	buf[0] = '\0';
	XLookupString ((XKeyEvent *)event, buf, bufnr, &keysym, &status);

        if (buf[0] == '\003'  /* ^C */
	|| (buf[0] == 'c' && (event -> xkey.state & ControlMask)))
	    return (True);
    }
    return (False);
}

void initIntrup ()
{
    if (signal (SIGINT, SIG_IGN) != SIG_IGN) signal (SIGINT, int_hdl);
        /* only when value was not SIG_IGN, a jump must be done to int_hdl */
#ifdef SIGQUIT
    if (signal (SIGQUIT, SIG_IGN) != SIG_IGN) signal (SIGQUIT, int_hdl);
        /* only when value was not SIG_IGN, a jump must be done to int_hdl */
#endif
    signal (SIGTERM, int_hdl);
#ifndef CORE
    signal (SIGILL,  int_hdl);
    signal (SIGFPE,  int_hdl);
#ifdef SIGBUS
    signal (SIGBUS,  int_hdl);
#endif
    signal (SIGSEGV, int_hdl);
#endif /*CORE*/
#ifdef SIGALRM
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
            break; /* just ignore this alarm signal (where does it come from?) */
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
    XCloseDisplay (display);
    if (*fn_err) unlink (fn_err);
    exit (status);
}

void die_alloc ()
{
    fprintf (stderr, "%s: Cannot allocate storage\n", argv0);
    die (1);
}

void startDialog (Widget d, char *s)
{
    XmString str = XmStringCreateLtoR (s, XmSTRING_DEFAULT_CHARSET);
    XtSetArg (args[0], XmNmessageString, str);
    XtSetValues (d, args, 1);
    XmStringFree (str);
    XtManageChild (d);
}

void errorMessage (char *s)
{
    if (firstExpose >= 0) {
	int n;
	if (!s || !*s) return;
	n = strlen (s);
	if (n + errlen > ERRMAX) {
	    if (errlen + 4 < ERRMAX + 6) {
		strcpy (&errbuf[errlen], "\n...");
		errlen += 4;
	    }
	}
	else {
	    if (errlen) errbuf[errlen++] = '\n';
	    strcpy (&errbuf[errlen], s);
	    errlen += n;
	}
	return;
    }
    else if (!s || !*s) {
	if (errlen) {
	    errlen = 0;
	    s = errbuf;
	}
	else return;
    }

    startDialog (eDialog, s);
}

void dmError (char *s)
{
    char ms[ERRMAX+2];
    char ps[80];

    if (NOdm_msg) return;

    *ps = 0;
    if (s && *s && strcmp (s, argv0)) sprintf (ps, "\"%s\" ", s);

    if (dmerrno > 0 && dmerrno <= dmnerr)
	sprintf (ms, "%s%s", ps, dmerrlist[dmerrno]);
    else
	sprintf (ms, "%sDMI error class '%s' (nr = %d)", ps, dmerrlist[0], dmerrno);
    errorMessage (ms);
}

#ifdef STATIC
/* libX11.a fix for static linking */
void *dlopen (const char *filename, int flag)
{
    return NULL;
}
#endif
