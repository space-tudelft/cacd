/*
 * ISC License
 *
 * Copyright (C) 1995-2018 by
 *	Xianfeng Ni
 *	Ulrich Geigenmuller
 *	Simon de Graaf
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

#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>

#include <Xm/Xm.h>
#include <Xm/CascadeB.h>
#include <Xm/FileSB.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/SelectioB.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>
#undef atexit /* Motif header files make this malicious definition */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#include "src/helios/option.h"
#include "src/helios/realist.h"
#include "src/helios/externs.h"
#include "src/space/auxil/auxil.h"

#ifndef FLT_MAX
#define FLT_MAX   3.40282347e+38        /* Max decimal value of a float */
#endif

#define T_BOOL 1
#define T_INT  2
#define T_INTC 3
#define T_REAL 4
#define T_STR  5
#define T_STRC 6

#define G_BOOL GeneralAdmin(action,1,fp,
#define G_INT  GeneralAdmin(action,2,fp,
#define G_INTC GeneralAdmin(action,3,fp,
#define G_REAL GeneralAdmin(action,4,fp,
#define G_STR  GeneralAdmin(action,5,fp,
#define G_STRC GeneralAdmin(action,6,fp,

extern void AdjustRetrieve (Widget, XtPointer, XtPointer);
extern void AdjustSelectabilityC (Widget, XtPointer, XtPointer);
extern void AdjustSelectabilityR (Widget, XtPointer, XtPointer);
extern void AdjustSelectabilityS (Widget, XtPointer, XtPointer);
extern void AdjustSelectabilitySC(Widget, XtPointer, XtPointer);
extern void AdjustSelectabilityH (Widget, XtPointer, XtPointer);
extern void AdjustSelectabilityF (Widget, XtPointer, XtPointer);
extern void AdmWindow (int, int);
extern void LoadLayoutFilterCallback (Widget, XtPointer, XtPointer);
extern void OpenWindow (Widget, XtPointer, XtPointer);
extern void PopupMessageBox (char *);
extern void SetImportCellSelectionPolicy (Widget, XtPointer, XtPointer);

void InsertIntoPfile (FILE *fp, char *name);
void GeneralAdmin (int action, int dataType, FILE *fp, char *keyword,
                   void *variable, char *iniString, Widget textField);
float stringToFloat (char *string);
char *floatToString (float value);
int stringToInt (char *string);
char *intToString (int);
int stringToBool (char *string);
char *boolToString (int);

/* The following variables are defined outside the routine
 * AdmVariable, so that if later a routine is added to
 * read the general defaults, it will have easy access
 * to these variables.
 */

extern int InitialSetValues;
int loadAlsoOpenForms = 1;

static int get_before_write = 0;
static int set_after_read = 0;
static int s_separator = 0;
static int s_notempty = 0;

static int   b_value;
static int   i_value;
static float r_ok = 0;
static float r_scale = 1;
static Widget w_value;

/******* General Settings ************/
static bool_param_t SetUserDefault;
static bool_param_t SetFactoryDefault;
static bool_param_t SetGeneralDefault;
bool_param_t ShowCommandLine;
bool_param_t ShowQuickReference;

/* Option for Extraction */
static intc_param_t ExtractionMode;
static bool_param_t ExtrTopCell;
static bool_param_t ExtrVerbose;
static intc_param_t BackAnnotation;
static intc_param_t SubstrateResExtraction;
static intc_param_t SubstrateCapExtraction;
static intc_param_t ResistanceExtraction;
static intc_param_t SelectiveResistance;
static intc_param_t CapacitanceExtraction;
static intc_param_t TypeOfCapacitance;
static intc_param_t FreqDepReduction;
static float_param_t max_frequency;
static bool_param_t AutoRunPreprocessors;
static bool_param_t ElemDefCustom;
static str_param_t ElemDefFileName;

/* parameters for fast capacitance extraction fine control */
static float_param_t lat_cap_window;
static float_param_t lat_compensate;
static strc_param_t jun_caps;
static str_param_t name_ground;
static str_param_t name_substrate;

/* parameters for accurate capacitance extraction fine control */
static bool_param_t Cap3dHierExtr;
static bool_param_t Cap3dOmitGate;
static float_param_t Cap3d_window;
static float_param_t ReqPrecInfMat;
static int_param_t Cap3dMaxGreenTerms;
static strc_param_t Cap3dBeMode;
static float_param_t Cap3dBeMaxArea;
static float_param_t Cap3dBeMaxCoArea;
static intc_param_t Cap3dBeShape;
static float_param_t Cap3dStepSlope;
static float_param_t Cap3dBeRatio;
static float_param_t Cap3dBeSplit;
static float_param_t Cap3dBeLw;
static float_param_t Cap3dMpMindist;
static intc_param_t Cap3dHighestMultipole;
static strc_param_t Cap3djun_caps;
static str_param_t Cap3dname_ground;
static str_param_t Cap3dname_substrate;

/* parameters for resistance extraction fine control */
static float_param_t low_sheet_res;
static int_param_t max_delayed;
static float_param_t max_obtuse;

/* parameters for accurate substrate resistance extraction fine control */
static float_param_t Sub3dBeWindow;
static float_param_t Sub3dReqPrecInfMat;
static int_param_t   Sub3dMaxGreenTerms;
static float_param_t Sub3dSawDist;
static float_param_t Sub3dEdgeDist;
static strc_param_t  Sub3dBeMode;
static float_param_t Sub3dBeMaxArea;
static intc_param_t  Sub3dBeShape;
static float_param_t Sub3dBeRatio;
static float_param_t Sub3dBeSplit;
static float_param_t Sub3dBeLw;
static float_param_t Sub3dMpMindist;
static intc_param_t  Sub3dHighestMultipole;

static float_param_t *floatpara_sub3d;
static int_param_t   *intpara_sub3d;
static strc_param_t  *strcpara_sub3d;
static intc_param_t  *intcpara_sub3d;

/* parameters for fast substrate capacitance extraction fine control */
static float_param_t sub_rc_const;

/* parameters for accurate substrate capacitance extraction fine control */
static float_param_t SubCap3dBeWindow;
static float_param_t SubCap3dReqPrecInfMat;
static int_param_t   SubCap3dMaxGreenTerms;
static float_param_t SubCap3dSawDist;
static float_param_t SubCap3dEdgeDist;
static strc_param_t  SubCap3dBeMode;
static float_param_t SubCap3dBeMaxArea;
static intc_param_t  SubCap3dBeShape;
static float_param_t SubCap3dBeRatio;
static float_param_t SubCap3dBeSplit;
static float_param_t SubCap3dBeLw;
static float_param_t SubCap3dMpMindist;
static intc_param_t  SubCap3dHighestMultipole;

/* parameters for circuit reduction fine control */
static intc_param_t ApplyHeuristics;
static int_param_t min_art_degree;
static int_param_t min_degree;
static float_param_t min_res;
static float_param_t min_sep_res;
static float_param_t max_par_res;
static bool_param_t no_neg_res;
static bool_param_t no_neg_cap;
static float_param_t min_coup_cap;
static float_param_t min_ground_cap;
static float_param_t frag_coup_cap;
static float_param_t min_coup_area;
static float_param_t min_ground_area;
static float_param_t frag_coup_area;
static float_param_t equi_line_ratio;

/* parameters for miscellaneous fine control */
static float_param_t lat_base_width;

static bool_param_t no_labels;
static bool_param_t hier_labels;
static bool_param_t hier_terminals;
static bool_param_t leaf_terminals;
static bool_param_t term_is_netname;
static bool_param_t cell_pos_name;
static bool_param_t node_pos_name;

static str_param_t hier_name_sep;
static str_param_t inst_term_sep;
static str_param_t net_node_sep;
static str_param_t pos_name_prefix;
static str_param_t tru_name_prefix;

/* parameters for "Retrieve Circuit from Database" */
static str_param_t RetrieveExclLibs;
static strc_param_t RetrieveFormat;
static str_param_t RetrievalOutputFile;
static intc_param_t RetrieveToFile;
static intc_param_t RetrieveLevel;
static int_param_t RetrieveUseExclLibs;
static int_param_t RetrieveVerbose;

/* parameters for retrieving SPICE/PSTAR/SPF/SPEF netlist */
static bool_param_t SpiceUseInstNames;
static bool_param_t SpiceUseNetNames;	/* SPICE only */
static bool_param_t SpiceGndNodeGnd;
static bool_param_t SpiceGndNodeVss;
static bool_param_t SpiceGndNodeCustom;
static bool_param_t SpiceUseGndNodeName;
static str_param_t SpiceGndNodePrefix;
static str_param_t SpiceGndNodeName;
static bool_param_t SpiceAutoBulk;
static bool_param_t SpiceAlwaysPBulk;
static bool_param_t SpiceAlwaysNBulk;
static bool_param_t SpiceFloatPatch;
static bool_param_t SpiceNoUncnnct;
static bool_param_t SpiceNoTitle;
static bool_param_t SpiceUseCntrlFile;
static str_param_t SpiceControlFile;
static bool_param_t SpiceUseDefLabel;
static str_param_t SpiceDefLabel;
static bool_param_t SpiceOmitModDef;	/* SPICE/SPF only */
static bool_param_t SpiceExpandNames;	/* SPF/SPEF only */
static bool_param_t UsePstarLl;	/* PSTAR only */
static bool_param_t UsePstarPn;	/* PSTAR only */

/* parameters for retrieving SLS netlist */
static bool_param_t SlsExpandNames;
static bool_param_t SlsRestDef;
static bool_param_t SlsNamesDatabase;
static bool_param_t SlsNoUncnnct;
static bool_param_t SlsUseCntrlFile;
static str_param_t SlsControlFile;
static bool_param_t SlsUseDefLabel;
static str_param_t SlsDefLabel;

/* parameters for retrieving EDIF netlist */
static bool_param_t EdifExpandNames;
static bool_param_t EdifShiftIndices;
static bool_param_t EdifCadenceCompatible;
static bool_param_t EdifCadenceSchematic;
static bool_param_t EdifUseCntrlFile;
static str_param_t EdifControlFile;
static bool_param_t EdifUseDefLabel;
static str_param_t EdifDefLabel;

/* parameters for retrieving NLE netlist */
static bool_param_t NLELong;
static str_param_t NLEpbulkName;
static str_param_t NLEnbulkName;
static bool_param_t NleUseCntrlFile;
static str_param_t NleControlFile;
static bool_param_t NleUseDefLabel;
static str_param_t NleDefLabel;

/* parameters for retrieving VHDL netlist */
static bool_param_t VhdlAlsoEntity;
static bool_param_t VhdlUseCntrlFile;
static str_param_t VhdlControlFile;
static bool_param_t VhdlUseDefLabel;
static str_param_t VhdlDefLabel;

/* parameters for Xspace */
static bool_param_t XUse;
static bool_param_t XSynch;
static bool_param_t XShowMenu;
static int_param_t XWidth;
static int_param_t XHeight;
static bool_param_t XBeMeshOnly;
static bool_param_t X3d;
static bool_param_t XGreen;
static bool_param_t XBeMesh;
static bool_param_t XTiles;
static bool_param_t XTileBound;
static bool_param_t XInputEdge;
static bool_param_t XFeMesh;
static bool_param_t XSubTerm;
static bool_param_t XDelaunay;
static bool_param_t XSubRes;
static bool_param_t XResistor;
static bool_param_t XOutputRes;
static bool_param_t XEquiPot;

/* parameters for usage of external parameter-file */
static bool_param_t ExtParaFront;
static bool_param_t ExtParaEnd;
static str_param_t ExtParaFrontFile;
static str_param_t ExtParaEndFile;

/* parameters for load layout */
static strc_param_t LoadFormat;
static bool_param_t LoadVerbose;
static bool_param_t Test45;
static bool_param_t CellOverwrite;
static bool_param_t NoOrigin;
static bool_param_t OnlySyntaxCheck;
static bool_param_t MaskList;
static bool_param_t TextToTerminal;
static float_param_t TerminalWidth;
static float_param_t LoadOptionCIFunit;
static str_param_t LoadOptionMaskList;

/* parameters for highlay */
int HighlayOptEnabled;
char *Namefile;
static intc_param_t HighlayUseNamefile;
static intc_param_t HighlayLightMode;
static str_param_t HighlayLightMask;
static str_param_t HighlayNamefile;
static str_param_t HighlayNetGroups;
static str_param_t HighlayPortGroups;
static str_param_t HighlayDevGroups;
static intc_param_t HighlaySelectItem;
static bool_param_t HighlayNets;
static bool_param_t HighlayPorts;
static bool_param_t HighlayDevs;
static bool_param_t HighlayVerbose;

/* parameters for match */
static bool_param_t MatchByname;
static bool_param_t MatchEdif;
static bool_param_t MatchExpand;
static bool_param_t MatchFbind;
static bool_param_t MatchIgCap;
static bool_param_t MatchIgRes;
static bool_param_t MatchNomap;
static bool_param_t MatchParams;
static bool_param_t MatchVerbose;

/* parameters for import */
static str_param_t DbaseForImport;
static intc_param_t ImpCellName;

/* parameters for device models */
static intc_param_t DevModInputSource;
static str_param_t DevModInputFile;
static str_param_t DevModInputDbase;
static intc_param_t DevModSaveMode;

void ADD_TO_CMDLINE (char *addendum)
{
    if (strlen (CmdLine) + strlen (addendum) + 1 >= sizeof (CmdLine))
	say ("Command line must be shorter than %d characters!", sizeof (CmdLine));
    else
	strcat (CmdLine, addendum);
}

char *trim (char *str)
{
    register char *s, *p, *t;
    register int comma = 0;

    if (!(s = str)) return (str);
    while (*s && (*s == ' ' || *s == ',')) ++s;
    if (s != str) {
	p = str; t = s; while ((*p++ = *t++));
	s = str;
    }
    while (*s) {
	if (*s == ' ' || *s == ',') {
	    if (!comma) { comma = 1; *s++ = ','; }
	    else { p = s; t = s + 1; while ((*p++ = *t++)); }
	}
	else { comma = 0; ++s; }
    }
    if (comma) *--s = 0;
    return (str);
}

/* If the file resides in the currently opened database,
** the path does not need to be included explicitly!
*/
char *trimPath (char *path)
{
    char *s;
    if (strstr (path, OpenedDatabase) == path) {
	s = path + strlen (OpenedDatabase);
	if (*s == '/') return (s + 1);
    }
    return (path);
}

/************************************************************************
*
* This routine has the purpose to
* -  communicate user- or factory-defaults used internally to the
*    widgets/gadgets that control the appearance on the screen
*    (filling in text-fields, setting toggle buttons): use
*       action = SET_VALUES
*              = SET_U_DEF (for at the same time setting the user default
*                            of the Udefault), or
*              = SET_F_DEF (for at the same time setting the factory
*                            default of the Udefault);
*              = SET_G_DEF (for setting the general default)
*
* -  extract current values from widgets/gadgets: use
*       action = GET_VALUES
*       action = ACC_VALUES
* -  extract current values from accepted values: use
*       action = GET_C_DEF
* -  read factory defaults from a specified file: use
*       action = READ_F_DEF
* -  read user defaults from a specified file: use
*       action = READ_U_DEF
* -  write user the present state of variables as user-defaults to a file: use
*       action = WRITE_HELIOS   (fp must be OK)
* -  write options to the parameter file and/or command line: use
*       action = WRITE_PARAMS
* -  initialize internal variables: use
*       action = INIT_VALUES
* All relevant variables of sets of variables are identified by integer
* "item" number;  mnenomic names for these numbers are defined in the
* file "options.h".
*
* Restriction of the internal variables to this routine was done to facilitate
* the maintenance of the program.
*
************************************************************************/
void AdmVariable (int item, int action, FILE *fp)
{
    char  *str;
    Widget selectedButton;
    int setAction = 0;
    int readAction = 0;
    int win_set;

    switch (action) {
    case READ_U_DEF:
    case READ_F_DEF:
	readAction = 1;
	break;
    case SET3D_VALUES:
    case SET_VALUES:
    case SET_F_DEF:
    case SET_G_DEF:
    case SET_U_DEF:
	setAction = 1;
	break;
    case WRITE_HELIOS:
    case WRITE_PARAMS:
    case INIT_VALUES:
    case ACC_VALUES:
    case GET_VALUES:
    case GET_C_DEF:
	break;
    default:
	say ("unknown action specified!");
	return;
    }

    switch (item) {
    case GENERAL:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### General settings #####\n");
	    fprintf (fp,   "############################\n");
	    get_before_write = 1;
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms) set_after_read = 1;
	}
	G_BOOL (char*)"ShowCommandLine", &ShowCommandLine, (char*)"0", NULL);
	win_set = 0;
	if (setAction || set_after_read) {
	    if (b_value){ XtManageChild (CmdLineWin->CmdLineDialog); win_set = 1; }
	    else	XtUnmanageChild (CmdLineWin->CmdLineDialog);
	}
	G_BOOL (char*)"ShowQuickReference", &ShowQuickReference, (char*)"1", NULL);
	if (setAction || set_after_read) {
	    if (b_value) { OpenWindow (NULL, (XtPointer)HELP, NULL); win_set = 1; }
	    else XtUnmanageChild (QuickRefWin->QuickRefDialog);
	}
	if (win_set && XtIsManaged (MessageWin->MessageDialog)) {
		/* be sure that this important window is not obscured! */
	    XtUnmapWidget (MessageWin->MessageWin);
	    XtMapWidget (MessageWin->MessageWin);
	}
	G_BOOL (char*)"SetUserDefault",    &SetUserDefault,    (char*)"0", Shell->SetUserDflt_Tggl);
	G_BOOL (char*)"SetFactoryDefault", &SetFactoryDefault, (char*)"0", Shell->SetFactoryDflt_Tggl);
	G_BOOL (char*)"SetGeneralDefault", &SetGeneralDefault, (char*)"0", Shell->SetGeneralDflt_Tggl);
	break;

    case EXTRACTION_OPTION:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Options for Extraction #####\n");
	    fprintf (fp,   "##################################\n");
	    get_before_write = XtIsManaged (ExtractWin->ExtractDialog);
        }
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (ExtractWin->ExtractDialog))
		set_after_read = 1;
	}
	else if (action == WRITE_PARAMS) {
	    char buf[8];
	    int i;

	    i = XtIsManaged (ExtractWin->ExtractDialog);
	    AdmWindow (EXTRACTION_OPTION, i ? GET_VALUES : GET_C_DEF);
	    i = XtIsManaged (ParMisWin->ParMisDialog);
	    AdmWindow (MISCELLANEOUS_FINE, i ? GET_VALUES : GET_C_DEF);

	    if (ExtParaFront.value) InsertIntoPfile (fp, ExtParaFrontFile.value);

	    buf[i=0] = ' ';
	    buf[++i] = '-';
		 if (ExtractionMode.value == 1) buf[++i] = 'F';
	    else if (ExtractionMode.value == 3) buf[++i] = 'I';
            if (ExtrTopCell.value && ExtractionMode.value != 1) buf[++i] = 'T';
            if (ExtrVerbose.value) buf[++i] = 'v';
	    if (buf[i] != '-') {
		buf[++i] = 0;
                ADD_TO_CMDLINE (buf);
	    }

	    switch (CapacitanceExtraction.value) {
	    case 1:		/* Fast capacitance extraction */
		switch (TypeOfCapacitance.value) {
		case 1: ADD_TO_CMDLINE ((char*)" -c"); break;
		case 2: ADD_TO_CMDLINE ((char*)" -C"); break;
		case 3: ADD_TO_CMDLINE ((char*)" -l"); break;
		}
		break;
	    case 2:		/* 3D capacitance extraction */
		switch (TypeOfCapacitance.value) {
		case 1: ADD_TO_CMDLINE ((char*)" -c3"); break;
		case 3: ADD_TO_CMDLINE ((char*)" -C3"); break;
		}
	    }

	    if (ResistanceExtraction.value) {
		buf[i=0] = ' ';
		buf[++i] = '-';
		buf[++i] = ResistanceExtraction.value == 1 ? 'r' : 'z';
		if (SelectiveResistance.value == 2) buf[++i] = 'k';
		else if (SelectiveResistance.value == 3) buf[++i] = 'j';
		buf[++i] = 0;
                ADD_TO_CMDLINE (buf);
	    }

	    if (SubstrateResExtraction.value > 0) {
		if (SubstrateResExtraction.value == 1) ADD_TO_CMDLINE ((char*)" -b");
		else ADD_TO_CMDLINE ((char*)" -B");

		if (SubstrateCapExtraction.value >= 2) ADD_TO_CMDLINE ((char*)" -Sadd_sub_caps=2");
		else
		if (SubstrateCapExtraction.value == 1) ADD_TO_CMDLINE ((char*)" -Sadd_sub_caps=1");
		else ADD_TO_CMDLINE ((char*)" -Sadd_sub_caps=0");
	    }

	    if (FreqDepReduction.value) {
		if (CapacitanceExtraction.value &&
		    ResistanceExtraction.value) ADD_TO_CMDLINE ((char*)" -G");
		AdmVariable (MAX_FREQUENCY, action, fp);
	    }

	    if (CapacitanceExtraction.value == 1)
		AdmVariable (CAPACITANCE_FAST, action, fp);
	    else if (CapacitanceExtraction.value == 2)
		AdmVariable (CAPACITANCE_ACCURATE, action, fp);

	    if (ResistanceExtraction.value)
		AdmVariable (RESISTANCE_EXTRACT, action, fp);

	    if (SubstrateResExtraction.value == 2)
		AdmVariable (SUBRES_ACCRT_FINE, action, fp);

	    if (SubstrateCapExtraction.value == 1)
		AdmVariable (SUBCAP_FAST_FINE, action, fp);
	    else
	    if (SubstrateCapExtraction.value == 2)
		AdmVariable (SUBCAP_ACCRT_FINE, action, fp);

	    if (ApplyHeuristics.value)
		AdmVariable (CIRCUIT_REDUCTION, action, fp);
	    else
		ADD_TO_CMDLINE ((char*)" -n");

	    i = XtIsManaged (XspaceWin->XspaceDialog);
	    AdmVariable (XSPACE, i ? GET_VALUES : GET_C_DEF, fp);
	    if (XUse.value) {
		ADD_TO_CMDLINE ((char*)" -X");
		AdmVariable (XSPACE_REST, i ? GET_VALUES : GET_C_DEF, fp);
		AdmVariable (XSPACE_REST, action, fp);
	    }

            if (BackAnnotation.value == 2)
                ADD_TO_CMDLINE ((char*)" -t");
            else if (BackAnnotation.value == 3)
                ADD_TO_CMDLINE ((char*)" -x");

	    AdmVariable (BACK_ANNOTATION_PARAMS, action, fp);

	    if (ElemDefCustom.value && *ElemDefFileName.value)
		ADD_TO_CMDLINE (mprintf (" -E%s", trimPath (ElemDefFileName.value)));

	    if (!AutoRunPreprocessors.value) ADD_TO_CMDLINE ((char*)" -u");

	    AdmVariable (LATERAL_BASE_WIDTH, action, fp);

	    if (ExtParaEnd.value) InsertIntoPfile (fp, ExtParaEndFile.value);
	}
	break;

    case EXTRACTION_MODE:
	if (action == INIT_VALUES) {
	    ExtractionMode.widgetChoice = NEW (Widget, 3);
	    ExtractionMode.widgetChoice[0] = ExtractWin->ExtractFlat_Bttn;
	    ExtractionMode.widgetChoice[1] = ExtractWin->ExtractHierIncr_Bttn;
	    ExtractionMode.widgetChoice[2] = ExtractWin->ExtractHierNonIncr_Bttn;
	}
	G_INTC (char*)"HierarchyLevel", &ExtractionMode, (char*)"1 3 3", ExtractWin->ExtractMode_OptnMn);
	if (setAction || set_after_read) {
	    XtSetSensitive (ExtractWin->ExtrTop_Tggl, i_value > 1 ? TRUE : FALSE);
	}
	break;

    case EXTRACTION_SETTINGS:
	G_BOOL (char*)"ExtractOnlyTopCell", &ExtrTopCell, (char*)"0", ExtractWin->ExtrTop_Tggl);
	G_BOOL (char*)"ExtractVerbose", &ExtrVerbose, (char*)"0", ExtractWin->ExtrVerbose_Tggl);
	break;

    case RESISTANCE_EXTRACTION:
	if (action == INIT_VALUES) {
	    ResistanceExtraction.widgetChoice = NEW (Widget, 3);
	    ResistanceExtraction.widgetChoice[0] = ExtractWin->ResExtractNone_Bttn;
	    ResistanceExtraction.widgetChoice[1] = ExtractWin->ResExtractFast_Bttn;
	    ResistanceExtraction.widgetChoice[2] = ExtractWin->ResExtractAccrt_Bttn;
	}
	G_INTC (char*)"InterconnectResExtraction", &ResistanceExtraction, (char*)"0 2 1", ExtractWin->ResExtract_OptnMn);
	if (setAction || set_after_read) AdjustSelectabilityR (w_value, NULL, NULL);
	break;

    case SELECTIVE_RESISTANCE:
	if (action == INIT_VALUES) {
	    SelectiveResistance.widgetChoice = NEW (Widget, 3);
	    SelectiveResistance.widgetChoice[0] = ExtractWin->ResExtrAll_Bttn;
	    SelectiveResistance.widgetChoice[1] = ExtractWin->ResExtrSpeci_Bttn;
	    SelectiveResistance.widgetChoice[2] = ExtractWin->ResExtrButSpeci_Bttn;
	}
	G_INTC (char*)"SelectiveInterconnectResExt", &SelectiveResistance, (char*)"1 3 1", ExtractWin->ResType_OptnMn);
	break;

    case SUBSTRATE_RESISTANCE:
	if (action == INIT_VALUES) {
	    SubstrateResExtraction.widgetChoice = NEW (Widget, 3);
	    SubstrateResExtraction.widgetChoice[0] = ExtractWin->SubstrateNone_Bttn;
	    SubstrateResExtraction.widgetChoice[1] = ExtractWin->SubstrateFast_Bttn;
	    SubstrateResExtraction.widgetChoice[2] = ExtractWin->SubstrateAccrt_Bttn;
	}
	G_INTC (char*)"SubstrateResExtraction", &SubstrateResExtraction, (char*)"0 2 0", ExtractWin->SubstrateRes_OptnMn);
	if (setAction || set_after_read) AdjustSelectabilityS (w_value, NULL, NULL);
	break;

    case SUBSTRATE_CAPACITANCE:
	if (action == INIT_VALUES) {
	    SubstrateCapExtraction.widgetChoice = NEW (Widget, 3);
	    SubstrateCapExtraction.widgetChoice[0] = ExtractWin->SubstrateCapNone_Bttn;
	    SubstrateCapExtraction.widgetChoice[1] = ExtractWin->SubstrateCapFast_Bttn;
	    SubstrateCapExtraction.widgetChoice[2] = ExtractWin->SubstrateCapAccrt_Bttn;
	}
	G_INTC (char*)"SubstrateCapExtraction", &SubstrateCapExtraction, (char*)"0 2 0", ExtractWin->SubstrateCap_OptnMn);
	if (setAction || set_after_read) AdjustSelectabilitySC (w_value, NULL, NULL);
	break;

    case CAPACITANCE_EXTRACTION:
	if (action == INIT_VALUES) {
	    CapacitanceExtraction.widgetChoice = NEW (Widget, 3);
	    CapacitanceExtraction.widgetChoice[0] = ExtractWin->CapExtractNone_Bttn;
	    CapacitanceExtraction.widgetChoice[1] = ExtractWin->CapExtractFast_Bttn;
	    CapacitanceExtraction.widgetChoice[2] = ExtractWin->CapExtractAccrt_Bttn;
	}
	G_INTC (char*)"CapacitanceExtraction", &CapacitanceExtraction, (char*)"0 2 1", ExtractWin->CapExtract_OptnMn);
	if (setAction || set_after_read) AdjustSelectabilityC (w_value, NULL, NULL);
	break;

    case TYPE_OF_CAPACITANCE:
	if (action == INIT_VALUES) {
	    TypeOfCapacitance.widgetChoice = NEW (Widget, 3);
	    TypeOfCapacitance.widgetChoice[0] = ExtractWin->CapSubstrOnly_Bttn;
	    TypeOfCapacitance.widgetChoice[1] = ExtractWin->CapCouplSubstr_Bttn;
	    TypeOfCapacitance.widgetChoice[2] = ExtractWin->CapAll_Bttn;
	}
	G_INTC (char*)"TypeOfCapacitance", &TypeOfCapacitance, (char*)"1 3 3", ExtractWin->CapType_OptnMn);
	break;

    case APPLY_CIRC_RED_HEUR:
	if (action == INIT_VALUES) {
	    ApplyHeuristics.widgetChoice = NEW (Widget, 2);
	    ApplyHeuristics.widgetChoice[0] = ExtractWin->NoApplyHeur_Bttn;
	    ApplyHeuristics.widgetChoice[1] = ExtractWin->ApplyHeur_Bttn;
	}
	G_INTC (char*)"ApplyHeuristics", &ApplyHeuristics, (char*)"0 1 1", ExtractWin->ReducHeur_OptnMn);
	if (setAction || set_after_read) AdjustSelectabilityH (w_value, NULL, NULL);
	break;

    case FREQ_DEP_REDUCTION:
	if (action == INIT_VALUES) {
	    FreqDepReduction.widgetChoice = NEW (Widget, 2);
	    FreqDepReduction.widgetChoice[0] = ExtractWin->FreqIndep_Bttn;
	    FreqDepReduction.widgetChoice[1] = ExtractWin->FreqDep_Bttn;
	}
	G_INTC (char*)"FrequencyDependentReduction", &FreqDepReduction, (char*)"0 1 0", ExtractWin->FreqDep_OptnMn);
	if (setAction || set_after_read) AdjustSelectabilityF (w_value, NULL, NULL);
	break;

    case MAX_FREQUENCY:
	r_scale = 1.e9;
	G_REAL (char*)"sne.frequency", &max_frequency, (char*)"0. inf 1.e9", ExtractWin->max_frequency_text);
	r_scale = 1;
	break;

    case MISCELLANEOUS_FINE:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Additional extraction options #####\n");
	    fprintf (fp,   "#########################################\n");
	    get_before_write = XtIsManaged (ParMisWin->ParMisDialog);
        }
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (ParMisWin->ParMisDialog))
		set_after_read = 1;
	}
	break;

    case BACK_ANNOTATION:
	if (action == INIT_VALUES) {
	    BackAnnotation.widgetChoice = NEW (Widget, 3);
	    BackAnnotation.widgetChoice[0] = ParMisWin->BackAnnNone_Bttn;
	    BackAnnotation.widgetChoice[1] = ParMisWin->BackAnnSome_Bttn;
	    BackAnnotation.widgetChoice[2] = ParMisWin->BackAnnFull_Bttn;
	}
	G_INTC (char*)"BackAnnotationDetail", &BackAnnotation, (char*)"1 3 1", ParMisWin->BackAnn_OptnMn);
	break;

    case BACK_ANNOTATION_PARAMS:
	if (action != WRITE_PARAMS || no_labels.value)
	    G_BOOL (char*)"no_labels", &no_labels, (char*)"0", ParMisWin->NoLabels_Tggl);
	if (action != WRITE_PARAMS || hier_labels.value)
	    G_BOOL (char*)"hier_labels", &hier_labels, (char*)"0", ParMisWin->HierLabels_Tggl);
	if (action != WRITE_PARAMS || hier_terminals.value)
	    G_BOOL (char*)"hier_terminals", &hier_terminals, (char*)"0", ParMisWin->HierTerminals_Tggl);
	if (action != WRITE_PARAMS || leaf_terminals.value)
	    G_BOOL (char*)"leaf_terminals", &leaf_terminals, (char*)"0", ParMisWin->LeafTerminals_Tggl);
	if (action != WRITE_PARAMS || term_is_netname.value)
	    G_BOOL (char*)"term_is_netname", &term_is_netname, (char*)"0", ParMisWin->TermIsNetname_Tggl);
	if (action != WRITE_PARAMS || cell_pos_name.value)
	    G_BOOL (char*)"cell_pos_name", &cell_pos_name, (char*)"0", ParMisWin->CellPosName_Tggl);
	if (action != WRITE_PARAMS || node_pos_name.value)
	    G_BOOL (char*)"node_pos_name", &node_pos_name, (char*)"0", ParMisWin->NodePosName_Tggl);
	s_separator = 1;
	if (action != WRITE_PARAMS || strcmp (hier_name_sep.value, "."))
	    G_STR (char*)"hier_name_sep", &hier_name_sep, (char*)".", ParMisWin->HierNameSep_Txt);
	if (action != WRITE_PARAMS || strcmp (inst_term_sep.value, "."))
	    G_STR (char*)"inst_term_sep", &inst_term_sep, (char*)".", ParMisWin->InstTermSep_Txt);
	if (action != WRITE_PARAMS || strcmp (net_node_sep.value, "_"))
	    G_STR (char*)"net_node_sep", &net_node_sep, (char*)"_", ParMisWin->NetNodeSep_Txt);
	s_separator = 0;
	G_STR (char*)"pos_name_prefix", &pos_name_prefix, (char*)"", ParMisWin->PosNamePrefix_Txt);
	if (action != WRITE_PARAMS || strcmp (tru_name_prefix.value, "n")) {
	    s_notempty = 1;
	    G_STR (char*)"trunc_name_prefix", &tru_name_prefix, (char*)"n", ParMisWin->TruNamePrefix_Txt);
	    s_notempty = 0;
	}
	break;

    case ELEM_DEF_FILE:
	G_BOOL (char*)"ElemDefCustom", &ElemDefCustom, (char*)"0", ParMisWin->ElemDefCustom_Tggl);
	G_STR (char*)"ElemDefFileName", &ElemDefFileName, (char*)"", ParMisWin->ElemDefFile_Txt);
	break;

    case EXTERNAL_PARAMETER_FRONT:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Options for use of external parameter files #####\n");
	    fprintf (fp,   "#######################################################\n");
	}
	G_BOOL (char*)"ExtParaFront", &ExtParaFront, (char*)"0", ParMisWin->ParaFront_Tggl);
	G_STR (char*)"ExtParaFrontFile", &ExtParaFrontFile, (char*)"", ParMisWin->ParaFront_Txt);
	break;

    case EXTERNAL_PARAMETER_END:
	G_BOOL (char*)"ExtParaEnd", &ExtParaEnd, (char*)"0", ParMisWin->ParaEnd_Tggl);
	G_STR (char*)"ExtParaEndFile", &ExtParaEndFile, (char*)"", ParMisWin->ParaEnd_Txt);
	break;

    case AUTO_RUN_PREPRO:
	G_BOOL (char*)"AutoRunPreprocessors", &AutoRunPreprocessors, (char*)"1", ParMisWin->AutoRunPreprocessorsButton);
	break;

    case LATERAL_BASE_WIDTH:
	G_REAL (char*)"lat_base_width", &lat_base_width, (char*)"0. inf 3.", ParMisWin->lat_base_width_text);
	break;

    case RESISTANCE_EXTRACT:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Options for fine control of interconnect resistance extraction #####\n");
	    fprintf (fp,   "##########################################################################\n");
	    get_before_write = XtIsManaged (ParResWin->ParResDialog);
	}
	else if (action == WRITE_PARAMS) {
	    int item, m = XtIsManaged (ParResWin->ParResDialog);

	    for (item = RESISTANCE_EXTRACT + 1; item < MAX_DELAYED; item++) {
		AdmVariable (item, m ? GET_VALUES : GET_C_DEF, fp);
		AdmVariable (item, WRITE_PARAMS, fp);
	    }
	    if (ResistanceExtraction.value == 2)
	    for (item = MAX_DELAYED; item < END_RESISTANCE_EXTRACT; item++) {
		AdmVariable (item, m ? GET_VALUES : GET_C_DEF, fp);
		AdmVariable (item, WRITE_PARAMS, fp);
	    }
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (ParResWin->ParResDialog))
		set_after_read = 1;
	}
	break;

    case MIN_SHEET_RESISTANCE:
	G_REAL (char*)"low_sheet_res", &low_sheet_res, (char*)"0. inf 1.", ParResWin->low_sheet_res_text);
	break;

    case MAX_DELAYED:
	G_INT (char*)"max_delayed", &max_delayed, (char*)"0 inf 500", ParResWin->max_delayed_text);
	break;

    case MAX_MESH_ANGLE:
	G_REAL (char*)"max_obtuse", &max_obtuse, (char*)"0. inf 110.", ParResWin->max_mesh_angle_text);
	break;

    case SUBRES_ACCRT_FINE:
	if (action == WRITE_HELIOS) {
            fprintf (fp, "\n##### Options for fine control of accurate substrate-resistance extraction #####\n");
            fprintf (fp,   "################################################################################\n");
	    get_before_write = XtIsManaged (ParSubAccrtWin->ParSubAccrtDialog);
	}
	else if (action == WRITE_PARAMS) {
	    int item, m = XtIsManaged (ParSubAccrtWin->ParSubAccrtDialog);

	    for (item = SUBRES_ACCRT_FINE + 1; item < END_SUBRES_ACCRT_FINE; item++) {
		AdmVariable (item, m ? GET_VALUES : GET_C_DEF, fp);
		AdmVariable (item, WRITE_PARAMS, fp);
	    }
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (ParSubAccrtWin->ParSubAccrtDialog))
		set_after_read = 1;
	}
	break;

    case SUB3D_WINDOW:
	G_REAL (char*)"sub3d.be_window", &Sub3dBeWindow, (char*)"0. inf 4.", ParSubAccrtWin->Sub3dWindow_Txt);
	break;

    case SUB3D_REQ_PREC_INF_MAT:
	G_REAL (char*)"sub3d.green_eps", &Sub3dReqPrecInfMat, (char*)"0. inf 0.001", ParSubAccrtWin->Sub3dReqPrec_Txt);
	break;

    case SUB3D_MAX_GREEN_TERMS:
	G_INT (char*)"sub3d.max_green_terms", &Sub3dMaxGreenTerms, (char*)"1 500 500", ParSubAccrtWin->Sub3dMaxGreenTerms_Txt);
	break;

    case SUB3D_SAWDIST:
	G_REAL (char*)"sub3d.saw_dist", &Sub3dSawDist, (char*)"0. inf inf", ParSubAccrtWin->SawDist_Txt);
	break;

    case SUB3D_EDGEDIST:
	G_REAL (char*)"sub3d.edge_dist", &Sub3dEdgeDist, (char*)"0. inf 0.", ParSubAccrtWin->EdgeDist_Txt);
	break;

    case SUB3D_BE_METHOD:
	if (action == INIT_VALUES) {
	    Sub3dBeMode.nr = 3;
	    Sub3dBeMode.restrictedChoice = NEW (char *, 3);
	    Sub3dBeMode.restrictedChoice[0] = (char*)"0c";
	    Sub3dBeMode.restrictedChoice[1] = (char*)"0g";
	    Sub3dBeMode.restrictedChoice[2] = (char*)"1g";
	    Sub3dBeMode.widgetChoice = NEW (Widget, 3);
	    Sub3dBeMode.widgetChoice[0] = ParSubAccrtWin->Sub3d0c_Bttn;
	    Sub3dBeMode.widgetChoice[1] = ParSubAccrtWin->Sub3d0g_Bttn;
	    Sub3dBeMode.widgetChoice[2] = ParSubAccrtWin->Sub3d1g_Bttn;
	}
	G_STRC (char*)"sub3d.be_mode", &Sub3dBeMode, (char*)"0", ParSubAccrtWin->Sub3dBeMode_OptnMn);
	break;

    case SUB3D_MAX_BE_AREA:
	G_REAL (char*)"sub3d.max_be_area", &Sub3dBeMaxArea, (char*)"0 inf 3", ParSubAccrtWin->Sub3dBeMaxArea_Txt);
	break;

    case SUB3D_BE_SHAPE:
	if (action != WRITE_PARAMS || Sub3dBeShape.value > 2) {
	    if (action == INIT_VALUES) {
		Sub3dBeShape.widgetChoice = NEW (Widget, 3);
		Sub3dBeShape.widgetChoice[0] = ParSubAccrtWin->Sub3dBeShapeNoPref_Bttn;
		Sub3dBeShape.widgetChoice[1] = ParSubAccrtWin->Sub3dBeShape3_Bttn;
		Sub3dBeShape.widgetChoice[2] = ParSubAccrtWin->Sub3dBeShape4_Bttn;
	    }
	    G_INTC (char*)"sub3d.be_shape", &Sub3dBeShape, (char*)"2 4 2", ParSubAccrtWin->Sub3dBeShape_OptnMn);
	}
	break;

    case SUB3D_BE_RATIO:
	G_REAL (char*)"sub3d.edge_be_ratio", &Sub3dBeRatio, (char*)"0. inf 1.", ParSubAccrtWin->Sub3dBeRatio_Txt);
	break;

    case SUB3D_BE_SPLIT:
	G_REAL (char*)"sub3d.edge_be_split", &Sub3dBeSplit, (char*)"0. inf 0.5", ParSubAccrtWin->Sub3dBeSplit_Txt);
	break;

    case SUB3D_BE_LW:
	G_REAL (char*)"sub3d.edge_be_split_lw", &Sub3dBeLw, (char*)"0. inf 4.", ParSubAccrtWin->Sub3dBeLw_Txt);
	break;

    case SUB3D_MP_MINDIST:
	G_REAL (char*)"sub3d.mp_min_dist", &Sub3dMpMindist, (char*)"0. inf 2.", ParSubAccrtWin->Sub3dMpMindist_Txt);
	break;

    case SUB3D_HIGHEST_MP:
	if (action == INIT_VALUES) {
	    Sub3dHighestMultipole.widgetChoice = NEW (Widget, 4);
	    Sub3dHighestMultipole.widgetChoice[0] = ParSubAccrtWin->Sub3dMnpl_Bttn;
	    Sub3dHighestMultipole.widgetChoice[1] = ParSubAccrtWin->Sub3dDpl_Bttn;
	    Sub3dHighestMultipole.widgetChoice[2] = ParSubAccrtWin->Sub3dQdrpl_Bttn;
	    Sub3dHighestMultipole.widgetChoice[3] = ParSubAccrtWin->Sub3dOctpl_Bttn;
	}
	G_INTC (char*)"sub3d.mp_max_order", &Sub3dHighestMultipole, (char*)"0 3 2", ParSubAccrtWin->Sub3dMpOrder_OptnMn);
	break;

    case SUBCAP_FAST_FINE:
	if (action == WRITE_HELIOS) {
            fprintf (fp, "\n##### Options for fine control of fast substrate-capacitance extraction #####\n");
            fprintf (fp,   "#############################################################################\n");
	    get_before_write = XtIsManaged (ParSubCapFastWin->ParSubCapFastDialog);
	}
	else if (action == WRITE_PARAMS) {
	    int item, m = XtIsManaged (ParSubCapFastWin->ParSubCapFastDialog);

	    for (item = SUBCAP_FAST_FINE + 1; item < END_SUBCAP_FAST_FINE; item++) {
		AdmVariable (item, m ? GET_VALUES : GET_C_DEF, fp);
		AdmVariable (item, WRITE_PARAMS, fp);
	    }
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (ParSubCapFastWin->ParSubCapFastDialog))
		set_after_read = 1;
	}
	break;

    case SUB_RC_CONSTANT:
	G_REAL (char*)"sub_rc_const", &sub_rc_const, (char*)"0. inf 8.855e-12", ParSubCapFastWin->sub_rc_const_text);
	break;

    case SUBCAP_ACCRT_FINE:
	if (action == WRITE_HELIOS) {
            fprintf (fp, "\n##### Options for fine control of accurate substrate-capacitance extraction ####\n");
            fprintf (fp,   "################################################################################\n");
	    get_before_write = XtIsManaged (ParSubCapAccrtWin->ParSubAccrtDialog);
	}
	else if (action == WRITE_PARAMS) {
	    int item, m = XtIsManaged (ParSubCapAccrtWin->ParSubAccrtDialog);

	    for (item = SUBCAP_ACCRT_FINE + 1; item < END_SUBCAP_ACCRT_FINE; item++) {
		AdmVariable (item, m ? GET_VALUES : GET_C_DEF, fp);
		AdmVariable (item, WRITE_PARAMS, fp);
	    }
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (ParSubCapAccrtWin->ParSubAccrtDialog))
		set_after_read = 1;
	}
	break;

    case SUBCAP3D_WINDOW:
	floatpara_sub3d = &Sub3dBeWindow;
	G_REAL (char*)"subcap3d.be_window", &SubCap3dBeWindow, (char*)"0. inf 4.", ParSubCapAccrtWin->Sub3dWindow_Txt);
	floatpara_sub3d = 0;
	break;

    case SUBCAP3D_REQ_PREC_INF_MAT:
	floatpara_sub3d = &Sub3dReqPrecInfMat;
	G_REAL (char*)"subcap3d.green_eps", &SubCap3dReqPrecInfMat, (char*)"0. inf 0.001", ParSubCapAccrtWin->Sub3dReqPrec_Txt);
	floatpara_sub3d = 0;
	break;

    case SUBCAP3D_MAX_GREEN_TERMS:
	intpara_sub3d = &Sub3dMaxGreenTerms;
	G_INT (char*)"subcap3d.max_green_terms", &SubCap3dMaxGreenTerms, (char*)"1 500 500", ParSubCapAccrtWin->Sub3dMaxGreenTerms_Txt);
	intpara_sub3d = 0;
	break;

    case SUBCAP3D_SAWDIST:
	floatpara_sub3d = &Sub3dSawDist;
	G_REAL (char*)"subcap3d.saw_dist", &SubCap3dSawDist, (char*)"0. inf inf", ParSubCapAccrtWin->SawDist_Txt);
	floatpara_sub3d = 0;
	break;

    case SUBCAP3D_EDGEDIST:
	floatpara_sub3d = &Sub3dEdgeDist;
	G_REAL (char*)"subcap3d.edge_dist", &SubCap3dEdgeDist, (char*)"0. inf 0.", ParSubCapAccrtWin->EdgeDist_Txt);
	floatpara_sub3d = 0;
	break;

    case SUBCAP3D_BE_METHOD:
	if (action == INIT_VALUES) {
	    SubCap3dBeMode.nr = 3;
	    SubCap3dBeMode.restrictedChoice = NEW (char *, 3);
	    SubCap3dBeMode.restrictedChoice[0] = (char*)"0c";
	    SubCap3dBeMode.restrictedChoice[1] = (char*)"0g";
	    SubCap3dBeMode.restrictedChoice[2] = (char*)"1g";
	    SubCap3dBeMode.widgetChoice = NEW (Widget, 3);
	    SubCap3dBeMode.widgetChoice[0] = ParSubCapAccrtWin->Sub3d0c_Bttn;
	    SubCap3dBeMode.widgetChoice[1] = ParSubCapAccrtWin->Sub3d0g_Bttn;
	    SubCap3dBeMode.widgetChoice[2] = ParSubCapAccrtWin->Sub3d1g_Bttn;
	}
	strcpara_sub3d = &Sub3dBeMode;
	G_STRC (char*)"subcap3d.be_mode", &SubCap3dBeMode, (char*)"0", ParSubCapAccrtWin->Sub3dBeMode_OptnMn);
	strcpara_sub3d = 0;
	break;

    case SUBCAP3D_MAX_BE_AREA:
	floatpara_sub3d = &Sub3dBeMaxArea;
	G_REAL (char*)"subcap3d.max_be_area", &SubCap3dBeMaxArea, (char*)"0 inf 3", ParSubCapAccrtWin->Sub3dBeMaxArea_Txt);
	floatpara_sub3d = 0;
	break;

    case SUBCAP3D_BE_SHAPE:
	if (action != WRITE_PARAMS || SubCap3dBeShape.value > 2) {
	    if (action == INIT_VALUES) {
		SubCap3dBeShape.widgetChoice = NEW (Widget, 3);
		SubCap3dBeShape.widgetChoice[0] = ParSubCapAccrtWin->Sub3dBeShapeNoPref_Bttn;
		SubCap3dBeShape.widgetChoice[1] = ParSubCapAccrtWin->Sub3dBeShape3_Bttn;
		SubCap3dBeShape.widgetChoice[2] = ParSubCapAccrtWin->Sub3dBeShape4_Bttn;
	    }
	    intcpara_sub3d = &Sub3dBeShape;
	    G_INTC (char*)"subcap3d.be_shape", &SubCap3dBeShape, (char*)"2 4 2", ParSubCapAccrtWin->Sub3dBeShape_OptnMn);
	    intcpara_sub3d = 0;
	}
	break;

    case SUBCAP3D_BE_RATIO:
	floatpara_sub3d = &Sub3dBeRatio;
	G_REAL (char*)"subcap3d.edge_be_ratio", &SubCap3dBeRatio, (char*)"0. inf 1.", ParSubCapAccrtWin->Sub3dBeRatio_Txt);
	floatpara_sub3d = 0;
	break;

    case SUBCAP3D_BE_SPLIT:
	floatpara_sub3d = &Sub3dBeSplit;
	G_REAL (char*)"subcap3d.edge_be_split", &SubCap3dBeSplit, (char*)"0. inf 0.5", ParSubCapAccrtWin->Sub3dBeSplit_Txt);
	floatpara_sub3d = 0;
	break;

    case SUBCAP3D_BE_LW:
	floatpara_sub3d = &Sub3dBeLw;
	G_REAL (char*)"subcap3d.edge_be_split_lw", &SubCap3dBeLw, (char*)"0. inf 4.", ParSubCapAccrtWin->Sub3dBeLw_Txt);
	floatpara_sub3d = 0;
	break;

    case SUBCAP3D_MP_MINDIST:
	floatpara_sub3d = &Sub3dMpMindist;
	G_REAL (char*)"subcap3d.mp_min_dist", &SubCap3dMpMindist, (char*)"0. inf 2.", ParSubCapAccrtWin->Sub3dMpMindist_Txt);
	floatpara_sub3d = 0;
	break;

    case SUBCAP3D_HIGHEST_MP:
	if (action == INIT_VALUES) {
	    SubCap3dHighestMultipole.widgetChoice = NEW (Widget, 4);
	    SubCap3dHighestMultipole.widgetChoice[0] = ParSubCapAccrtWin->Sub3dMnpl_Bttn;
	    SubCap3dHighestMultipole.widgetChoice[1] = ParSubCapAccrtWin->Sub3dDpl_Bttn;
	    SubCap3dHighestMultipole.widgetChoice[2] = ParSubCapAccrtWin->Sub3dQdrpl_Bttn;
	    SubCap3dHighestMultipole.widgetChoice[3] = ParSubCapAccrtWin->Sub3dOctpl_Bttn;
	}
	intcpara_sub3d = &Sub3dHighestMultipole;
	G_INTC (char*)"subcap3d.mp_max_order", &SubCap3dHighestMultipole, (char*)"0 3 2", ParSubCapAccrtWin->Sub3dMpOrder_OptnMn);
	intcpara_sub3d = 0;
	break;

    case CAPACITANCE_FAST:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Options for fine control of fast capacitance extraction #####\n");
	    fprintf (fp,   "###################################################################\n");
	    get_before_write = XtIsManaged (ParCapFastWin->ParCapFastDialog);
	}
	else if (action == WRITE_PARAMS) {
	    int item, m = XtIsManaged (ParCapFastWin->ParCapFastDialog);

	    for (item = CAPACITANCE_FAST + 1; item < END_CAPACITANCE_FAST; item++) {
		AdmVariable (item, m ? GET_VALUES : GET_C_DEF, fp);
		AdmVariable (item, WRITE_PARAMS, fp);
	    }
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (ParCapFastWin->ParCapFastDialog))
		set_after_read = 1;
	}
	break;

    case LATERAL_CAP_WINDOW:
	G_REAL (char*)"lat_cap_window", &lat_cap_window, (char*)"0. inf 3.", ParCapFastWin->lat_cap_window_text);
	break;

    case LATERAL_COMPENSATE:
	G_REAL (char*)"compensate_lat_part", &lat_compensate, (char*)"0. inf 1.", ParCapFastWin->CompLatPart_Txt);
	break;

    case JUNCTION_CAP_TYPE:
	if (action != WRITE_PARAMS || jun_caps.value > 0) {
	    if (action == INIT_VALUES) {
		jun_caps.nr = 5;
		jun_caps.restrictedChoice = NEW (char *, 5);
		jun_caps.restrictedChoice[0] = (char*)"linear";
		jun_caps.restrictedChoice[1] = (char*)"non-linear";
		jun_caps.restrictedChoice[2] = (char*)"area";
		jun_caps.restrictedChoice[3] = (char*)"area-perimeter";
		jun_caps.restrictedChoice[4] = (char*)"separate";
		jun_caps.widgetChoice = NEW (Widget, 5);
		jun_caps.widgetChoice[0] = ParCapFastWin->JC_linear_Bttn;
		jun_caps.widgetChoice[1] = ParCapFastWin->JC_non_lin_Bttn;
		jun_caps.widgetChoice[2] = ParCapFastWin->JC_area_Bttn;
		jun_caps.widgetChoice[3] = ParCapFastWin->JC_area_p_Bttn;
		jun_caps.widgetChoice[4] = ParCapFastWin->JC_sepa_Bttn;
	    }
	    G_STRC (char*)"jun_caps", &jun_caps, (char*)"0", ParCapFastWin->JunCaps_OptnMn);
	}
	break;

    case CAP_GND_NODE_NAME:
	G_STR (char*)"name_ground", &name_ground, (char*)"GND", ParCapFastWin->NameGndFast_Txt);
	break;

    case CAP_SUB_NODE_NAME:
	G_STR (char*)"name_substrate", &name_substrate, (char*)"SUBSTR", ParCapFastWin->NameSubFast_Txt);
	break;

    case CAPACITANCE_ACCURATE:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Options for fine control of accurate capacitance extraction #####\n");
	    fprintf (fp,   "#######################################################################\n");
	    get_before_write = XtIsManaged (ParCapAccurateWin->ParCapAccurateDialog);
	}
	else if (action == WRITE_PARAMS) {
	    int item, m = XtIsManaged (ParCapAccurateWin->ParCapAccurateDialog);

	    for (item = CAPACITANCE_ACCURATE + 1; item < END_CAPACITANCE_ACCURATE; item++) {
		AdmVariable (item, m ? GET_VALUES : GET_C_DEF, fp);
		AdmVariable (item, WRITE_PARAMS, fp);
	    }
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (ParCapAccurateWin->ParCapAccurateDialog))
		set_after_read = 1;
	}
	break;

    case CAP3D_HIER_EXTRACT:
	if (action != WRITE_PARAMS || Cap3dHierExtr.value)
	    G_BOOL (char*)"allow_hierarchical_cap3d", &Cap3dHierExtr, (char*)"0", ParCapAccurateWin->AllowHierCap3D_Tggl);
	break;

    case CAP3D_OMIT_GATE:
	if (action != WRITE_PARAMS || Cap3dOmitGate.value)
	    G_BOOL (char*)"cap3d.omit_gate_ds_cap", &Cap3dOmitGate, (char*)"0", ParCapAccurateWin->Cap3dOmitGate_Tggl);
	break;

    case CAP3D_WINDOW:
	G_REAL (char*)"cap3d.be_window", &Cap3d_window, (char*)"0. inf 4.", ParCapAccurateWin->CapWin3D_Txt);
	break;

    case CAP3D_REQ_PREC_INF_MAT:
	G_REAL (char*)"cap3d.green_eps", &ReqPrecInfMat, (char*)"1.e-6 inf 0.001", ParCapAccurateWin->ReqPrecInfMat_Txt);
	break;

    case CAP3D_MAX_GREEN_TERMS:
	G_INT (char*)"cap3d.max_green_terms", &Cap3dMaxGreenTerms, (char*)"1 500 500", ParCapAccurateWin->Cap3dMaxGreenTerms_Txt);
	break;

    case CAP3D_BE_METHOD:
	if (action == INIT_VALUES) {
	    Cap3dBeMode.nr = 3;
	    Cap3dBeMode.restrictedChoice = NEW (char *, 3);
	    Cap3dBeMode.restrictedChoice[0] = (char*)"0c";
	    Cap3dBeMode.restrictedChoice[1] = (char*)"0g";
	    Cap3dBeMode.restrictedChoice[2] = (char*)"1g";
	    Cap3dBeMode.widgetChoice = NEW (Widget, 3);
	    Cap3dBeMode.widgetChoice[0] = ParCapAccurateWin->ConstCollocation_Bttn;
	    Cap3dBeMode.widgetChoice[1] = ParCapAccurateWin->ConstGalerkin_Bttn;
	    Cap3dBeMode.widgetChoice[2] = ParCapAccurateWin->LinGalerkin_Bttn;
	}
	G_STRC (char*)"cap3d.be_mode", &Cap3dBeMode, (char*)"0", ParCapAccurateWin->Cap3dBeMode_OptnMn);
	break;

    case CAP3D_MAX_BE_AREA:
	G_REAL (char*)"cap3d.max_be_area", &Cap3dBeMaxArea, (char*)"0 inf 3", ParCapAccurateWin->Cap3dBeMaxArea_Txt);
	break;

    case CAP3D_MAX_COARSE_BE_AREA:
	r_ok = -1;
	if (action != WRITE_PARAMS || Cap3dBeMaxCoArea.value != r_ok)
	G_REAL (char*)"cap3d.max_coarse_be_area", &Cap3dBeMaxCoArea, (char*)"0 inf -1", ParCapAccurateWin->Cap3dBeMaxCoArea_Txt);
	r_ok = 0;
	break;

    case CAP3D_BE_SHAPE:
	if (action != WRITE_PARAMS || Cap3dBeShape.value > 2) {
	    if (action == INIT_VALUES) {
		Cap3dBeShape.widgetChoice = NEW (Widget, 3);
		Cap3dBeShape.widgetChoice[0] = ParCapAccurateWin->Cap3dBeShapeNoPref_Bttn;
		Cap3dBeShape.widgetChoice[1] = ParCapAccurateWin->Cap3dBeShape3_Bttn;
		Cap3dBeShape.widgetChoice[2] = ParCapAccurateWin->Cap3dBeShape4_Bttn;
	    }
	    G_INTC (char*)"cap3d.be_shape", &Cap3dBeShape, (char*)"2 4 2", ParCapAccurateWin->Cap3dBeShape_OptnMn);
	}
	break;

    case CAP3D_STEP_SLOPE:
	if (action != WRITE_PARAMS || Cap3dStepSlope.value != 2.0)
	    G_REAL (char*)"cap3d.default_step_slope", &Cap3dStepSlope, (char*)"0.0001 10000. 2.", ParCapAccurateWin->Cap3dStepSlope_Txt);
	break;

    case CAP3D_BE_RATIO:
	G_REAL (char*)"cap3d.edge_be_ratio", &Cap3dBeRatio, (char*)"0.0001 10000. 1.", ParCapAccurateWin->Cap3dBeRatio_Txt);
	break;

    case CAP3D_BE_SPLIT:
	G_REAL (char*)"cap3d.edge_be_split", &Cap3dBeSplit, (char*)"0.0001 10000. 0.5", ParCapAccurateWin->Cap3dBeSplit_Txt);
	break;

    case CAP3D_BE_LW:
	G_REAL (char*)"cap3d.edge_be_split_lw", &Cap3dBeLw, (char*)"0.0001 10000. 4.", ParCapAccurateWin->Cap3dBeLw_Txt);
	break;

    case CAP3D_MP_MINDIST:
	G_REAL (char*)"cap3d.mp_min_dist", &Cap3dMpMindist, (char*)"1. inf 2.", ParCapAccurateWin->Cap3dMpMindist_Txt);
	break;

    case CAP3D_HIGHEST_MP:
	if (action == INIT_VALUES) {
	    Cap3dHighestMultipole.widgetChoice = NEW (Widget, 4);
	    Cap3dHighestMultipole.widgetChoice[0] = ParCapAccurateWin->Cap3dMnpl_Bttn;
	    Cap3dHighestMultipole.widgetChoice[1] = ParCapAccurateWin->Cap3dDpl_Bttn;
	    Cap3dHighestMultipole.widgetChoice[2] = ParCapAccurateWin->Cap3dQdrpl_Bttn;
	    Cap3dHighestMultipole.widgetChoice[3] = ParCapAccurateWin->Cap3dOctpl_Bttn;
	}
	G_INTC (char*)"cap3d.mp_max_order", &Cap3dHighestMultipole, (char*)"0 3 2", ParCapAccurateWin->Cap3dMpOrder_OptnMn);
	break;

    case CAP3D_JUNCTION_CAP_TYPE:
	if (action != WRITE_PARAMS || Cap3djun_caps.value > 0) {
	    if (action == INIT_VALUES) {
		Cap3djun_caps.nr = 5;
		Cap3djun_caps.restrictedChoice = NEW (char *, 5);
		Cap3djun_caps.restrictedChoice[0] = (char*)"linear";
		Cap3djun_caps.restrictedChoice[1] = (char*)"non-linear";
		Cap3djun_caps.restrictedChoice[2] = (char*)"area";
		Cap3djun_caps.restrictedChoice[3] = (char*)"area-perimeter";
		Cap3djun_caps.restrictedChoice[4] = (char*)"separate";
		Cap3djun_caps.widgetChoice = NEW (Widget, 5);
		Cap3djun_caps.widgetChoice[0] = ParCapAccurateWin->JC3d_linear_Bttn;
		Cap3djun_caps.widgetChoice[1] = ParCapAccurateWin->JC3d_non_lin_Bttn;
		Cap3djun_caps.widgetChoice[2] = ParCapAccurateWin->JC3d_area_Bttn;
		Cap3djun_caps.widgetChoice[3] = ParCapAccurateWin->JC3d_area_p_Bttn;
		Cap3djun_caps.widgetChoice[4] = ParCapAccurateWin->JC3d_sepa_Bttn;
	    }
	    str = (action == WRITE_HELIOS || action == READ_U_DEF)? (char *)"jun_caps3d" : (char *)"jun_caps";
	    G_STRC str, &Cap3djun_caps, (char*)"0", ParCapAccurateWin->JunCaps3d_OptnMn);
	}
	break;

    case CAP3D_GND_NODE_NAME:
	str = (action == WRITE_HELIOS || action == READ_U_DEF)? (char *)"name_ground3d" : (char *)"name_ground";
	G_STR str, &Cap3dname_ground, (char*)"GND", ParCapAccurateWin->NameGndAccrt_Txt);
	break;

    case CAP3D_SUB_NODE_NAME:
	str = (action == WRITE_HELIOS || action == READ_U_DEF)? (char *)"name_substrate3d" : (char *)"name_substrate";
	G_STR str, &Cap3dname_substrate, (char*)"SUBSTR", ParCapAccurateWin->NameSubAccrt_Txt);
	break;

    case CIRCUIT_REDUCTION:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Options for fine control of circuit reduction #####\n");
	    fprintf (fp,   "#########################################################\n");
	    get_before_write = XtIsManaged (ParHeuristicsWin->ParHeuristicsDialog);
	}
	else if (action == WRITE_PARAMS) {
	    int item, m = XtIsManaged (ParHeuristicsWin->ParHeuristicsDialog);

	    for (item = CIRCUIT_REDUCTION + 1; item < END_CIRCUIT_REDUCTION; item++) {
		AdmVariable (item, m ? GET_VALUES : GET_C_DEF, fp);
		AdmVariable (item, WRITE_PARAMS, fp);
	    }
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (ParHeuristicsWin->ParHeuristicsDialog))
		set_after_read = 1;
	}
	break;

    case MIN_ART_DEGREE:
	G_INT (char*)"min_art_degree", &min_art_degree, (char*)"0 inf 3", ParHeuristicsWin->min_art_degree_text);
	break;

    case MIN_DEGREE:
	G_INT (char*)"min_degree", &min_degree, (char*)"0 inf 4", ParHeuristicsWin->min_degree_text);
	break;

    case MIN_RES:
	G_REAL (char*)"min_res", &min_res, (char*)"1. inf 100.", ParHeuristicsWin->min_res_text);
	break;

    case MIN_SEP_RES:
	G_REAL (char*)"min_sep_res", &min_sep_res, (char*)"0. inf 10.", ParHeuristicsWin->min_sep_res_text);
	break;

    case MAX_PAR_RES:
	G_REAL (char*)"max_par_res", &max_par_res, (char*)"1. inf 25.", ParHeuristicsWin->max_par_res_text);
	break;

    case NO_NEG_RES:
	G_BOOL (char*)"no_neg_res", &no_neg_res, (char*)"1", ParHeuristicsWin->no_neg_res_Tggl);
	break;

    case NO_NEG_CAP:
	G_BOOL (char*)"no_neg_cap", &no_neg_cap, (char*)"1", ParHeuristicsWin->no_neg_cap_Tggl);
	break;

    case MIN_COUP_CAP:
	G_REAL (char*)"min_coup_cap", &min_coup_cap, (char*)"-inf inf 0.04", ParHeuristicsWin->MinCoupCap_Txt);
	break;

    case MIN_GROUND_CAP:
	G_REAL (char*)"min_ground_cap", &min_ground_cap, (char*)"0. inf 1.e-15", ParHeuristicsWin->min_ground_cap_text);
	break;

    case FRAG_COUP_CAP:
	G_REAL (char*)"frag_coup_cap", &frag_coup_cap, (char*)"0. inf 0.2", ParHeuristicsWin->frag_coup_cap_text);
	break;

    case MIN_COUP_AREA:
	G_REAL (char*)"min_coup_area", &min_coup_area, (char*)"-inf inf 0.04", ParHeuristicsWin->MinCoupArea_Txt);
	break;

    case MIN_GROUND_AREA:
	G_REAL (char*)"min_ground_area", &min_ground_area, (char*)"0. inf 1.e-11", ParHeuristicsWin->min_ground_area_text);
	break;

    case FRAG_COUP_AREA:
	G_REAL (char*)"frag_coup_area", &frag_coup_area, (char*)"0. inf 0.2", ParHeuristicsWin->frag_coup_area_text);
	break;

    case EQUI_LINE_RATIO:
	G_REAL (char*)"equi_line_ratio", &equi_line_ratio, (char*)"0. inf 1.", ParHeuristicsWin->EquiPot_Txt);
	break;

    case RETRIEVE_OPTION:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Retrieval options #####\n");
	    fprintf (fp,   "#############################\n");
	    get_before_write = XtIsManaged (RetrieveWin->RetrieveDialog);
        }
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (RetrieveWin->RetrieveDialog))
		set_after_read = 1;
	}
	else if (action == WRITE_PARAMS) {
	    int m = XtIsManaged (RetrieveWin->RetrieveDialog);
	    AdmWindow (RETRIEVE_OPTION, m ? GET_VALUES : GET_C_DEF);

	    switch (RetrieveFormat.value) {
		case 0: strcpy (CmdLine, "xspice"); break;
		case 1: strcpy (CmdLine, "xspice -E"); break;
		case 2: strcpy (CmdLine, "xspice -H"); break;
		case 3: strcpy (CmdLine, "xpstar"); break;
		case 4: strcpy (CmdLine, "xsls"); break;
		case 5: strcpy (CmdLine, "xedif"); break;
		case 6: strcpy (CmdLine, "xnle"); break;
		case 7: strcpy (CmdLine, "xspf"); break;
		case 8: strcpy (CmdLine, "xspef"); break;
		case 9: strcpy (CmdLine, "xvhdl"); break;
	    }

	    Namefile = 0;
	    if (RetrieveToFile.value) {
		if (*RetrievalOutputFile.value)
		    Namefile = trimPath (RetrievalOutputFile.value);
		else
		    ADD_TO_CMDLINE ((char*)" -f");
	    }

	    switch (RetrieveLevel.value) {
	    case 2:
		ADD_TO_CMDLINE ((char*)" -h");
		break;
	    case 3:
		if (RetrieveFormat.value == 6) { /* NLE */
		    ADD_TO_CMDLINE ((char*)" -h");
		    break;
		}
		ADD_TO_CMDLINE ((char*)" -hi");
		if (RetrieveUseExclLibs.value && *RetrieveExclLibs.value) {
		    char *s = RetrieveExclLibs.value;
		    for (;;) {
			if ((s = strchr ((str = s), ','))) *s = '\0';
			ADD_TO_CMDLINE (mprintf (" -X %s", str));
			if (!s) break;
			*s++ = ',';
		    }
		}
		break;
	    }

            if (RetrieveVerbose.value) ADD_TO_CMDLINE ((char*)" -v");

	    if (RetrieveFormat.value == 4) {
		m = XtIsManaged (RetrieveOptionSLSWin->RetrieveOptionSLSDialog);
		AdmVariable (SLS_ADVANCED_OPTION, m ? GET_VALUES : GET_C_DEF, fp);

		if (SlsExpandNames.value) ADD_TO_CMDLINE ((char*)" -e");
		if (SlsRestDef.value) ADD_TO_CMDLINE ((char*)" -r");
		if (SlsNamesDatabase.value) ADD_TO_CMDLINE ((char*)" -d");
		if (SlsNoUncnnct.value) ADD_TO_CMDLINE ((char*)" -t");
		if (SlsUseCntrlFile.value && *SlsControlFile.value) {
		    ADD_TO_CMDLINE (mprintf (" -C %s", trimPath (SlsControlFile.value)));
		    if (SlsUseDefLabel.value && *SlsDefLabel.value)
			ADD_TO_CMDLINE (mprintf (" -D %s", SlsDefLabel.value));
		}
	    }
	    else if (RetrieveFormat.value == 5) {
		m = XtIsManaged (RetrieveOptionEDIFWin->RetrieveOptionEDIFDialog);
		AdmVariable (EDIF_ADVANCED_OPTION, m ? GET_VALUES : GET_C_DEF, fp);

		if (EdifExpandNames.value)  ADD_TO_CMDLINE ((char*)" -e");
		if (EdifShiftIndices.value) ADD_TO_CMDLINE ((char*)" -s");
		if (EdifCadenceCompatible.value) ADD_TO_CMDLINE ((char*)" -c");
		if (EdifCadenceSchematic.value)  ADD_TO_CMDLINE ((char*)" -I");
		if (EdifUseCntrlFile.value && *EdifControlFile.value) {
		    ADD_TO_CMDLINE (mprintf (" -C %s", trimPath (EdifControlFile.value)));
		    if (EdifUseDefLabel.value && *EdifDefLabel.value)
			ADD_TO_CMDLINE (mprintf (" -D %s", EdifDefLabel.value));
		}
	    }
	    else if (RetrieveFormat.value == 6) {
		m = XtIsManaged (RetrieveOptionNLEWin->RetrieveOptionNLEDialog);
		AdmVariable (NLE_ADVANCED_OPTION, m ? GET_VALUES : GET_C_DEF, fp);

		if (NLELong.value) ADD_TO_CMDLINE ((char*)" -l");
		if (*NLEpbulkName.value)
		    ADD_TO_CMDLINE (mprintf (" -p %s", NLEpbulkName.value));
		if (*NLEnbulkName.value)
		    ADD_TO_CMDLINE (mprintf (" -n %s", NLEnbulkName.value));
		if (NleUseCntrlFile.value && *NleControlFile.value) {
		    ADD_TO_CMDLINE (mprintf (" -C %s", trimPath (NleControlFile.value)));
		    if (NleUseDefLabel.value && *NleDefLabel.value)
			ADD_TO_CMDLINE (mprintf (" -D %s", NleDefLabel.value));
		}
	    }
	    else if (RetrieveFormat.value == 9) {
		m = XtIsManaged (RetrieveOptionVHDLWin->RetrieveOptionVHDLDialog);
		AdmVariable (VHDL_ADVANCED_OPTION, m ? GET_VALUES : GET_C_DEF, fp);

		if (VhdlAlsoEntity.value) ADD_TO_CMDLINE ((char*)" -r");
		if (VhdlUseCntrlFile.value && *VhdlControlFile.value) {
		    ADD_TO_CMDLINE (mprintf (" -C %s", trimPath (VhdlControlFile.value)));
		    if (VhdlUseDefLabel.value && *VhdlDefLabel.value)
			ADD_TO_CMDLINE (mprintf (" -D %s", VhdlDefLabel.value));
		}
	    }
	    else {
		m = XtIsManaged (RetrieveOptionSPICEWin->RetrieveOptionSPICEDialog);
		AdmVariable (SPICE_ADVANCED_OPTION, m ? GET_VALUES : GET_C_DEF, fp);

		if (RetrieveFormat.value == 3) { /* PSTAR */
			 if (UsePstarPn.value) ADD_TO_CMDLINE ((char*)" -Q");
		    else if (UsePstarLl.value) ADD_TO_CMDLINE ((char*)" -P");
		}
		else if (RetrieveFormat.value > 6) { /* SPF && SPEF */
		    if (SpiceExpandNames.value) ADD_TO_CMDLINE ((char*)" -e");
		    if (SpiceOmitModDef.value && RetrieveFormat.value != 8) /* not SPEF */
			ADD_TO_CMDLINE ((char*)" -o");
		}
		else { /* xSPICE */
		    if (SpiceUseNetNames.value) ADD_TO_CMDLINE ((char*)" -a");
		    if (SpiceOmitModDef.value)  ADD_TO_CMDLINE ((char*)" -o");
		}
		if (RetrieveFormat.value != 8) { /* not SPEF */
		    if (SpiceUseInstNames.value)ADD_TO_CMDLINE ((char*)" -d");
		    if (!SpiceAutoBulk.value)	ADD_TO_CMDLINE ((char*)" -u");
		    if (SpiceAlwaysPBulk.value)	ADD_TO_CMDLINE ((char*)" -p");
		    if (SpiceAlwaysNBulk.value)	ADD_TO_CMDLINE ((char*)" -n");
		    if (SpiceNoUncnnct.value)	ADD_TO_CMDLINE ((char*)" -t");
		    if (SpiceNoTitle.value)	ADD_TO_CMDLINE ((char*)" -k");
		    if (SpiceFloatPatch.value)	ADD_TO_CMDLINE ((char*)" -g");
		}
		if (SpiceGndNodeGnd.value) ADD_TO_CMDLINE ((char*)" -x");
		if (SpiceGndNodeVss.value) ADD_TO_CMDLINE ((char*)" -y");
		if (SpiceGndNodeCustom.value && *SpiceGndNodePrefix.value)
		    ADD_TO_CMDLINE (mprintf (" -z %s", SpiceGndNodePrefix.value));
		if (SpiceUseGndNodeName.value && *SpiceGndNodeName.value)
		    ADD_TO_CMDLINE (mprintf (" -O %s", SpiceGndNodeName.value));
		if (SpiceUseCntrlFile.value && *SpiceControlFile.value) {
		    ADD_TO_CMDLINE (mprintf (" -C %s", trimPath (SpiceControlFile.value)));
		    if (SpiceUseDefLabel.value && *SpiceDefLabel.value)
			ADD_TO_CMDLINE (mprintf (" -D %s", SpiceDefLabel.value));
		}
	    }
	}
	break;

    case RETRIEVE_FORMAT:
	if (action == INIT_VALUES) {
	    RetrieveFormat.nr = 10;
	    RetrieveFormat.restrictedChoice = NEW (char *, 10);
	    RetrieveFormat.restrictedChoice[0] = (char*)"SPICE";
	    RetrieveFormat.restrictedChoice[1] = (char*)"ESPICE";
	    RetrieveFormat.restrictedChoice[2] = (char*)"HSPICE";
	    RetrieveFormat.restrictedChoice[3] = (char*)"PSTAR";
	    RetrieveFormat.restrictedChoice[4] = (char*)"SLS";
	    RetrieveFormat.restrictedChoice[5] = (char*)"EDIF";
	    RetrieveFormat.restrictedChoice[6] = (char*)"NLE";
	    RetrieveFormat.restrictedChoice[7] = (char*)"SPF";
	    RetrieveFormat.restrictedChoice[8] = (char*)"SPEF";
	    RetrieveFormat.restrictedChoice[9] = (char*)"VHDL";
	    RetrieveFormat.widgetChoice = NEW (Widget, 10);
	    RetrieveFormat.widgetChoice[0] = RetrieveWin->RetrieveSPICEButton;
	    RetrieveFormat.widgetChoice[1] = RetrieveWin->RetrieveESPICEButton;
	    RetrieveFormat.widgetChoice[2] = RetrieveWin->RetrieveHSPICEButton;
	    RetrieveFormat.widgetChoice[3] = RetrieveWin->RetrievePSTARButton;
	    RetrieveFormat.widgetChoice[4] = RetrieveWin->RetrieveSLSButton;
	    RetrieveFormat.widgetChoice[5] = RetrieveWin->RetrieveEDIFButton;
	    RetrieveFormat.widgetChoice[6] = RetrieveWin->RetrieveNLEButton;
	    RetrieveFormat.widgetChoice[7] = RetrieveWin->RetrieveSPFButton;
	    RetrieveFormat.widgetChoice[8] = RetrieveWin->RetrieveSPEFButton;
	    RetrieveFormat.widgetChoice[9] = RetrieveWin->RetrieveVHDLButton;
	}
	G_STRC (char*)"RetrieveFormat", &RetrieveFormat, (char*)"0", RetrieveWin->RetrieveFormat_OptnMn);
	if (setAction || set_after_read) AdjustRetrieve (w_value, NULL, NULL);
	break;

    case RETRIEVE_TO_FILE:
	if (action == INIT_VALUES) {
	    RetrieveToFile.widgetChoice = NEW (Widget, 2);
	    RetrieveToFile.widgetChoice[0] = RetrieveWin->RetrieveToStdout_Bttn;
	    RetrieveToFile.widgetChoice[1] = RetrieveWin->RetrieveToFile_Bttn;
	}
	G_INTC (char*)"RetrieveToFile", &RetrieveToFile, (char*)"0 1 0", NULL);
	G_STR (char*)"RetrievalOutputFile", &RetrievalOutputFile, (char*)"", RetrieveWin->RetrievalOutputFile_Txt);
	break;

    case RETRIEVE_HIERARCHY:
	if (action == INIT_VALUES) {
	    RetrieveLevel.widgetChoice = NEW (Widget, 3);
	    RetrieveLevel.widgetChoice[0] = RetrieveWin->RetrieveToplevel_Bttn;
	    RetrieveLevel.widgetChoice[1] = RetrieveWin->RetrieveLocalHier_Bttn;
	    RetrieveLevel.widgetChoice[2] = RetrieveWin->RetrieveFullHier_Bttn;
	}
	G_INTC (char*)"RetrieveLevel", &RetrieveLevel, (char*)"1 3 1", NULL);
	G_STR (char*)"RetrieveExclLibs", &RetrieveExclLibs, (char*)"", RetrieveWin->RetrExclLibs_Txt);
	G_BOOL (char*)"RetrieveUseExclLibs", &RetrieveUseExclLibs, (char*)"0", RetrieveWin->RetrExclLibs_Tggl);
	G_BOOL (char*)"RetrieveVerbose", &RetrieveVerbose, (char*)"0", RetrieveWin->RetrVerbose_Tggl);
	break;

    case SPICE_ADVANCED_OPTION:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Advanced options for SPICE/PSTAR/SPF/SPEF format #####\n");
	    fprintf (fp,   "############################################################\n");
	    get_before_write = XtIsManaged (RetrieveOptionSPICEWin->RetrieveOptionSPICEDialog);
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (RetrieveOptionSPICEWin->RetrieveOptionSPICEDialog))
		set_after_read = 1;
	}
	G_BOOL (char*)"SpiceGndNodeGnd", &SpiceGndNodeGnd, (char*)"0", RetrieveOptionSPICEWin->SpiceGndNodeGnd_Tggl);
	G_BOOL (char*)"SpiceGndNodeVss", &SpiceGndNodeVss, (char*)"0", RetrieveOptionSPICEWin->SpiceGndNodeVss_Tggl);
	G_BOOL (char*)"SpiceGndNodeCustom", &SpiceGndNodeCustom, (char*)"0", RetrieveOptionSPICEWin->SpiceGndNodeCustom_Tggl);
	G_STR (char*)"SpiceGndNodePrefix", &SpiceGndNodePrefix, (char*)"", RetrieveOptionSPICEWin->SpiceGndNodeCustom_Txt);
	G_BOOL (char*)"SpiceUseGndNodeName", &SpiceUseGndNodeName, (char*)"0", RetrieveOptionSPICEWin->SpiceGndNodeName_Tggl);
	G_STR (char*)"SpiceGndNodeName", &SpiceGndNodeName, (char*)"GND", RetrieveOptionSPICEWin->SpiceGndNodeName_Txt);
	G_BOOL (char*)"SpiceAutoBulk", &SpiceAutoBulk, (char*)"1", RetrieveOptionSPICEWin->SpiceAutoBulk_Tggl);
	G_BOOL (char*)"SpiceAlwaysPBulk", &SpiceAlwaysPBulk, (char*)"0", RetrieveOptionSPICEWin->SpiceAlwaysPBulk_Tggl);
	G_BOOL (char*)"SpiceAlwaysNBulk", &SpiceAlwaysNBulk, (char*)"0", RetrieveOptionSPICEWin->SpiceAlwaysNBulk_Tggl);
	G_BOOL (char*)"SpiceUseInstNames", &SpiceUseInstNames, (char*)"0", RetrieveOptionSPICEWin->SpiceUseInstNames_Tggl);
	G_BOOL (char*)"SpiceNoUncnnct", &SpiceNoUncnnct, (char*)"0", RetrieveOptionSPICEWin->SpiceNoUncnnct_Tggl);
	G_BOOL (char*)"SpiceNoTitle", &SpiceNoTitle, (char*)"0", RetrieveOptionSPICEWin->SpiceNoTitle_Tggl);
	G_BOOL (char*)"SpiceFloatPatch", &SpiceFloatPatch, (char*)"0", RetrieveOptionSPICEWin->SpiceFloatPatch_Tggl);
	G_BOOL (char*)"SpiceUseCntrlFile", &SpiceUseCntrlFile, (char*)"0", RetrieveOptionSPICEWin->SpiceUseCntrlFile_Tggl);
	G_STR (char*)"SpiceControlFile", &SpiceControlFile, (char*)"xspicerc", RetrieveOptionSPICEWin->SpiceCntrlFile_Txt);
	G_BOOL (char*)"SpiceUseDefLabel", &SpiceUseDefLabel, (char*)"0", RetrieveOptionSPICEWin->SpiceDefLabel_Tggl);
	G_STR (char*)"SpiceDefLabel", &SpiceDefLabel, (char*)"", RetrieveOptionSPICEWin->SpiceDefLabel_Txt);

	/* SPICE only: */
	G_BOOL (char*)"SpiceUseNetNames", &SpiceUseNetNames, (char*)"0", RetrieveOptionSPICEWin->SpiceUseNetNames_Tggl);
	G_BOOL (char*)"SpiceOmitModDef", &SpiceOmitModDef, (char*)"0", RetrieveOptionSPICEWin->SpiceOmitModDef_Tggl);

	/* SPF only: */
	G_BOOL (char*)"SpiceExpandNames", &SpiceExpandNames, (char*)"0", RetrieveOptionSPICEWin->SpiceExpandNames_Tggl);

	/* PSTAR only: */
	G_BOOL (char*)"UsePstarLl", &UsePstarLl, (char*)"0", RetrieveOptionSPICEWin->UsePstarLl_Tggl);
	G_BOOL (char*)"UsePstarPn", &UsePstarPn, (char*)"0", RetrieveOptionSPICEWin->UsePstarPn_Tggl);
	break;

    case SLS_ADVANCED_OPTION:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Advanced options for SLS format #####\n");
	    fprintf (fp,   "###########################################\n");
	    get_before_write = XtIsManaged (RetrieveOptionSLSWin->RetrieveOptionSLSDialog);
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (RetrieveOptionSLSWin->RetrieveOptionSLSDialog))
		set_after_read = 1;
	}
	G_BOOL (char*)"SlsExpandNames", &SlsExpandNames, (char*)"0", RetrieveOptionSLSWin->SlsExpandNames_Tggl);
	G_BOOL (char*)"SlsRestDef", &SlsRestDef, (char*)"0", RetrieveOptionSLSWin->SlsRestDef_Tggl);
	G_BOOL (char*)"SlsNamesDatabase", &SlsNamesDatabase, (char*)"1", RetrieveOptionSLSWin->SlsNamesDatabase_Tggl);
	G_BOOL (char*)"SlsNoUncnnct", &SlsNoUncnnct, (char*)"0", RetrieveOptionSLSWin->SlsNoUncnnct_Tggl);
	G_BOOL (char*)"SlsUseCntrlFile", &SlsUseCntrlFile, (char*)"0", RetrieveOptionSLSWin->SlsUseCntrlFile_Tggl);
	G_STR (char*)"SlsControlFile", &SlsControlFile, (char*)"xslsrc", RetrieveOptionSLSWin->SlsCntrlFile_Txt);
	G_BOOL (char*)"SlsUseDefLabel", &SlsUseDefLabel, (char*)"0", RetrieveOptionSLSWin->SlsDefLabel_Tggl);
	G_STR (char*)"SlsDefLabel", &SlsDefLabel, (char*)"", RetrieveOptionSLSWin->SlsDefLabel_Txt);
	break;

    case EDIF_ADVANCED_OPTION:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Advanced options for EDIF format #####\n");
	    fprintf (fp,   "############################################\n");
	    get_before_write = XtIsManaged (RetrieveOptionEDIFWin->RetrieveOptionEDIFDialog);
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (RetrieveOptionEDIFWin->RetrieveOptionEDIFDialog))
		set_after_read = 1;
	}
	G_BOOL (char*)"EdifExpandNames", &EdifExpandNames, (char*)"1", RetrieveOptionEDIFWin->EdifExpandNames_Tggl);
	G_BOOL (char*)"EdifShiftIndices", &EdifShiftIndices, (char*)"0", RetrieveOptionEDIFWin->EdifShiftIndices_Tggl);
	G_BOOL (char*)"EdifCadenceCompatible", &EdifCadenceCompatible, (char*)"0", RetrieveOptionEDIFWin->EdifCadence_Tggl);
	G_BOOL (char*)"EdifCadenceSchematic", &EdifCadenceSchematic, (char*)"0", RetrieveOptionEDIFWin->EdifSchematic_Tggl);
	G_BOOL (char*)"EdifUseCntrlFile", &EdifUseCntrlFile, (char*)"0", RetrieveOptionEDIFWin->EdifUseCntrlFile_Tggl);
	G_STR (char*)"EdifControlFile", &EdifControlFile, (char*)"xslsrc", RetrieveOptionEDIFWin->EdifCntrlFile_Txt);
	G_BOOL (char*)"EdifUseDefLabel", &EdifUseDefLabel, (char*)"0", RetrieveOptionEDIFWin->EdifDefLabel_Tggl);
	G_STR (char*)"EdifDefLabel", &EdifDefLabel, (char*)"", RetrieveOptionEDIFWin->EdifDefLabel_Txt);
	break;

    case NLE_ADVANCED_OPTION:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Advanced options for NLE format #####\n");
	    fprintf (fp,   "###########################################\n");
	    get_before_write = XtIsManaged (RetrieveOptionNLEWin->RetrieveOptionNLEDialog);
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (RetrieveOptionNLEWin->RetrieveOptionNLEDialog))
		set_after_read = 1;
	}
	G_BOOL (char*)"NLELong", &NLELong, (char*)"0", RetrieveOptionNLEWin->NLE_long_Tggl);
	G_STR (char*)"NLEnbulkName", &NLEnbulkName, (char*)"vss", RetrieveOptionNLEWin->NLE_nbulk_Txt);
	G_STR (char*)"NLEpbulkName", &NLEpbulkName, (char*)"vdd", RetrieveOptionNLEWin->NLE_pbulk_Txt);
	G_BOOL (char*)"NleUseCntrlFile", &NleUseCntrlFile, (char*)"0", RetrieveOptionNLEWin->NleUseCntrlFile_Tggl);
	G_STR (char*)"NleControlFile", &NleControlFile, (char*)"xslsrc", RetrieveOptionNLEWin->NleCntrlFile_Txt);
	G_BOOL (char*)"NleUseDefLabel", &NleUseDefLabel, (char*)"0", RetrieveOptionNLEWin->NleDefLabel_Tggl);
	G_STR (char*)"NleDefLabel", &NleDefLabel, (char*)"", RetrieveOptionNLEWin->NleDefLabel_Txt);
	break;

    case VHDL_ADVANCED_OPTION:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Advanced options for VHDL format #####\n");
	    fprintf (fp,   "############################################\n");
	    get_before_write = XtIsManaged (RetrieveOptionVHDLWin->RetrieveOptionVHDLDialog);
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (RetrieveOptionVHDLWin->RetrieveOptionVHDLDialog))
		set_after_read = 1;
	}
	G_BOOL (char*)"VhdlAlsoEntity", &VhdlAlsoEntity, (char*)"0", RetrieveOptionVHDLWin->VHDL_option_r);
	G_BOOL (char*)"VhdlUseCntrlFile", &VhdlUseCntrlFile, (char*)"0", RetrieveOptionVHDLWin->VhdlUseCntrlFile_Tggl);
	G_STR (char*)"VhdlControlFile", &VhdlControlFile, (char*)"xslsrc", RetrieveOptionVHDLWin->VhdlCntrlFile_Txt);
	G_BOOL (char*)"VhdlUseDefLabel", &VhdlUseDefLabel, (char*)"0", RetrieveOptionVHDLWin->VhdlDefLabel_Tggl);
	G_STR (char*)"VhdlDefLabel", &VhdlDefLabel, (char*)"", RetrieveOptionVHDLWin->VhdlDefLabel_Txt);
	break;

    case LOAD_LAYOUT_OPTION:
	if (action == WRITE_PARAMS) {
	    AdmVariable (item, GET_VALUES, fp);

	    XtVaGetValues (LoadWin->LoadLayout_OptnMn, XmNmenuHistory, &selectedButton, NULL);

	    if (selectedButton == LoadWin->GDSII_Bttn) {
		strcpy (CmdLine, "cgi");

		if (MaskList.value && *LoadOptionMaskList.value)
		    ADD_TO_CMDLINE (mprintf (" -m %s", LoadOptionMaskList.value));

		if (TextToTerminal.value) {
		    if (TerminalWidth.value > 0)
			ADD_TO_CMDLINE (mprintf (" -tw %g", TerminalWidth.value));
		    else
			ADD_TO_CMDLINE ((char*)" -t");
		}
	    }
	    else {
		if (selectedButton == LoadWin->LDM_Bttn) {
		    strcpy (CmdLine, "cldm");
		}
		else if (selectedButton == LoadWin->CIF_Bttn) {
		    strcpy (CmdLine, "ccif");
		    ADD_TO_CMDLINE (mprintf (" -u %g", LoadOptionCIFunit.value));
		}
		if (NoOrigin.value)	ADD_TO_CMDLINE ((char*)" -o");
		else			ADD_TO_CMDLINE ((char*)" -x");
		if (OnlySyntaxCheck.value) ADD_TO_CMDLINE ((char*)" -s");
	    }
	    if (Test45.value)		ADD_TO_CMDLINE ((char*)" -4");
	    if (CellOverwrite.value)	ADD_TO_CMDLINE ((char*)" -f");
	    if (LoadVerbose.value)	ADD_TO_CMDLINE ((char*)" -v");
	    break;
	}
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Layout loading options #####\n");
	    fprintf (fp,   "##################################\n");
	    get_before_write = XtIsManaged (LoadWin->LoadDialog);
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (LoadWin->LoadDialog))
		set_after_read = 1;
	}
	if (action == INIT_VALUES) {
	    LoadFormat.nr = 3;
	    LoadFormat.restrictedChoice = NEW (char *, 3);
	    LoadFormat.restrictedChoice[0] = (char*)"CIF";
	    LoadFormat.restrictedChoice[1] = (char*)"GDS2";
	    LoadFormat.restrictedChoice[2] = (char*)"LDM";
	    LoadFormat.widgetChoice = NEW (Widget, 3);
	    LoadFormat.widgetChoice[0] = LoadWin->CIF_Bttn;
	    LoadFormat.widgetChoice[1] = LoadWin->GDSII_Bttn;
	    LoadFormat.widgetChoice[2] = LoadWin->LDM_Bttn;
	}
	G_STRC (char*)"LoadFormat", &LoadFormat, (char*)"1", LoadWin->LoadLayout_OptnMn);
	if (setAction || set_after_read) LoadLayoutFilterCallback (w_value, NULL, NULL);
	G_BOOL (char*)"LoadVerbose", &LoadVerbose, (char*)"0", LoadWin->LoadVerbose_Tggl);
	G_BOOL (char*)"Test45", &Test45, (char*)"1", LoadWin->Test45_Tggl);
	G_BOOL (char*)"CellOverwrite", &CellOverwrite, (char*)"1", LoadWin->CellOverwrite_Tggl);
	G_BOOL (char*)"NoOrigin", &NoOrigin, (char*)"1", LoadWin->NoOrigin_Tggl);
	G_BOOL (char*)"OnlySyntaxCheck", &OnlySyntaxCheck, (char*)"0", LoadWin->OnlySyntaxCheck_Tggl);
	G_BOOL (char*)"MaskList", &MaskList, (char*)"0", LoadWin->MaskList_Tggl);
	G_STR (char*)"LoadOptionMaskList", &LoadOptionMaskList, (char*)"", LoadWin->LoadOptionMaskListText);
	G_BOOL (char*)"TextToTerminal", &TextToTerminal, (char*)"0", LoadWin->GDSTerminalForText_Tggl);
	G_REAL (char*)"TerminalWidth", &TerminalWidth, (char*)"0 inf 0", LoadWin->GDSTerminalWidth_Txt);
	G_REAL (char*)"LoadOptionCIFunit", &LoadOptionCIFunit, (char*)"0 inf 100", LoadWin->LoadOptionCIFunitText);
	break;

    case HIGHLAY:
	if (action == WRITE_PARAMS) {
	    AdmVariable (item, GET_VALUES, fp);

	    Namefile = HighlayUseNamefile.value ? HighlayNamefile.value : 0;
	    if (Namefile && !*Namefile) break;

	    strcpy (CmdLine, "highlay");

	    if (!HighlayUseNamefile.value)
		switch (HighlaySelectItem.value) {
		case 1: ADD_TO_CMDLINE ((char*)" -d"); break;
		case 2: ADD_TO_CMDLINE ((char*)" -i"); break;
		case 3: ADD_TO_CMDLINE ((char*)" -m"); break;
		}

	    HighlayOptEnabled = 0;
	    if (HighlayNets.value) {
		HighlayOptEnabled = 1;
		str = HighlayUseNamefile.value ? 0 : HighlayNetGroups.value;
		if (str && *str)
		    ADD_TO_CMDLINE (mprintf (" -N %s", str));
		else
		    ADD_TO_CMDLINE ((char*)" -n");
	    }
	    if (HighlayPorts.value) {
		HighlayOptEnabled = 1;
		str = HighlayUseNamefile.value ? 0 : HighlayPortGroups.value;
		if (str && *str)
		    ADD_TO_CMDLINE (mprintf (" -P %s", str));
		else
		    ADD_TO_CMDLINE ((char*)" -p");
	    }
	    if (HighlayDevs.value) {
		HighlayOptEnabled = 1;
		str = HighlayUseNamefile.value ? 0 : HighlayDevGroups.value;
		if (str && *str)
		    ADD_TO_CMDLINE (mprintf (" -E %s", str));
		else
		    ADD_TO_CMDLINE ((char*)" -e");
	    }
	    if (HighlayLightMode.value == 1)
		ADD_TO_CMDLINE ((char*)" -L");
	    else if (HighlayLightMode.value == 2) {
		str = HighlayLightMask.value;
		if (!str || !*str) str = (char*)"xxx";
		ADD_TO_CMDLINE (mprintf (" -l %s", str));
	    }
	    if (HighlayVerbose.value)
		ADD_TO_CMDLINE ((char*)" -v");
	    break;
	}
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Highlay options #####\n");
	    fprintf (fp,   "###########################\n");
	    get_before_write = XtIsManaged (HighlayWin->HighlayDialog);
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (HighlayWin->HighlayDialog))
		set_after_read = 1;
	}
	else if (action == INIT_VALUES) {
	    HighlayUseNamefile.widgetChoice = NEW (Widget, 2);
	    HighlayUseNamefile.widgetChoice[0] = HighlayWin->UseMatch_Bttn;
	    HighlayUseNamefile.widgetChoice[1] = HighlayWin->UseNamefile_Bttn;
	    HighlayLightMode.widgetChoice = NEW (Widget, 3);
	    HighlayLightMode.widgetChoice[0] = HighlayWin->HLorig_Bttn;
	    HighlayLightMode.widgetChoice[1] = HighlayWin->HLcond_Bttn;
	    HighlayLightMode.widgetChoice[2] = HighlayWin->HLspec_Bttn;
	    HighlaySelectItem.widgetChoice = NEW (Widget, 3);
	    HighlaySelectItem.widgetChoice[0] = HighlayWin->HighDef_Tggl;
	    HighlaySelectItem.widgetChoice[1] = HighlayWin->HighIncon_Tggl;
	    HighlaySelectItem.widgetChoice[2] = HighlayWin->HighMatch_Tggl;
	}
	G_INTC (char*)"HighlayUseNamefile", &HighlayUseNamefile, (char*)"0 1 0", NULL);
	G_STR (char*)"HighlayNamefile", &HighlayNamefile, (char*)"", HighlayWin->Namefile_Txt);
	G_INTC (char*)"HighlayLightMode", &HighlayLightMode, (char*)"0 2 0", HighlayWin->HighLightMode);
	G_STR (char*)"HighlayLightMask", &HighlayLightMask, (char*)"", HighlayWin->HLmask_Txt);
	G_INTC (char*)"HighlaySelectItem", &HighlaySelectItem, (char*)"1 3 1", NULL);
	G_BOOL (char*)"HighlayDevs", &HighlayDevs, (char*)"0", HighlayWin->HighDevs_Tggl);
	G_STR (char*)"HighlayDevGroups", &HighlayDevGroups, (char*)"", HighlayWin->HighDevs_Txt);
	G_BOOL (char*)"HighlayNets", &HighlayNets, (char*)"1", HighlayWin->HighNets_Tggl);
	G_STR (char*)"HighlayNetGroups", &HighlayNetGroups, (char*)"", HighlayWin->HighNets_Txt);
	G_BOOL (char*)"HighlayPorts", &HighlayPorts, (char*)"0", HighlayWin->HighPorts_Tggl);
	G_STR (char*)"HighlayPortGroups", &HighlayPortGroups, (char*)"", HighlayWin->HighPorts_Txt);
	G_BOOL (char*)"HighlayVerbose", &HighlayVerbose, (char*)"0", HighlayWin->HighVerbose_Tggl);
	break;

    case MATCH:
	if (action == WRITE_PARAMS) {
		AdmVariable (item, GET_VALUES, fp);
		strcpy (CmdLine, "match");
		if (MatchByname.value)	ADD_TO_CMDLINE ((char*)" -byname");
		if (MatchEdif.value)	ADD_TO_CMDLINE ((char*)" -edif");
		if (MatchExpand.value)	ADD_TO_CMDLINE ((char*)" -expand");
		if (MatchFbind.value)	ADD_TO_CMDLINE ((char*)" -fullbindings");
		if (MatchIgCap.value)	ADD_TO_CMDLINE ((char*)" -cap");
		if (MatchIgRes.value)	ADD_TO_CMDLINE ((char*)" -res");
		if (MatchNomap.value)	ADD_TO_CMDLINE ((char*)" -nomap");
		if (MatchParams.value)	ADD_TO_CMDLINE ((char*)" -param");
		if (MatchVerbose.value)	ADD_TO_CMDLINE ((char*)" -verbose");
	    break;
	}
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Match options #####\n");
	    fprintf (fp,   "#########################\n");
	    get_before_write = XtIsManaged (MatchWin->MatchDialog);
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (MatchWin->MatchDialog))
		set_after_read = 1;
	}
	G_BOOL (char*)"MatchByname", &MatchByname, (char*)"0", MatchWin->MatchByname_Tggl);
	G_BOOL (char*)"MatchEdif",   &MatchEdif,   (char*)"0", MatchWin->MatchEdif_Tggl);
	G_BOOL (char*)"MatchExpand", &MatchExpand, (char*)"0", MatchWin->MatchExpand_Tggl);
	G_BOOL (char*)"MatchFbind",  &MatchFbind,  (char*)"0", MatchWin->MatchFbind_Tggl);
	G_BOOL (char*)"MatchIgCap",  &MatchIgCap,  (char*)"0", MatchWin->MatchIgCap_Tggl);
	G_BOOL (char*)"MatchIgRes",  &MatchIgRes,  (char*)"0", MatchWin->MatchIgRes_Tggl);
	G_BOOL (char*)"MatchNomap",  &MatchNomap,  (char*)"0", MatchWin->MatchNomap_Tggl);
	G_BOOL (char*)"MatchParams", &MatchParams, (char*)"0", MatchWin->MatchParams_Tggl);
	G_BOOL (char*)"MatchVerbose",&MatchVerbose,(char*)"0", MatchWin->MatchVerbose_Tggl);
	break;

    case XSPACE:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Options for X-Window display of mesh etc. #####\n");
	    fprintf (fp,   "#####################################################\n");
	    get_before_write = XtIsManaged (XspaceWin->XspaceDialog);
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (XspaceWin->XspaceDialog))
		set_after_read = 1;
	}
	G_BOOL (char*)"UseXDisplay", &XUse, (char*)"0", XspaceWin->XUse_Tggl);
	break;

    case XSPACE_REST:
	G_BOOL (char*)"disp.sync", &XSynch, (char*)"1", XspaceWin->XSynch_Tggl);
	G_BOOL (char*)"disp.show_menu", &XShowMenu, (char*)"0", XspaceWin->XShowMenu_Tggl);
	G_INT (char*)"disp.width", &XWidth, (char*)"0 10000 800", XspaceWin->XwindowWidth_Txt);
	G_INT (char*)"disp.height", &XHeight, (char*)"0 10000 600", XspaceWin->XwindowHeight_Txt);
	G_BOOL (char*)"disp.be_mesh_only", &XBeMeshOnly, (char*)"0", XspaceWin->XBeMeshOnly_Tggl);
	G_BOOL (char*)"disp.draw_green", &XGreen, (char*)"0", XspaceWin->XGreen_Tggl);
	G_BOOL (char*)"disp.draw_be_mesh", &XBeMesh, (char*)"0", XspaceWin->XBeMesh_Tggl);
	if (action == WRITE_PARAMS && XBeMesh.value) {
	    if (SubstrateResExtraction.value == 2 && CapacitanceExtraction.value != 2)
		fprintf (fp, "save_prepass_image\n");
	}
	G_BOOL (char*)"disp.3_dimensional", &X3d, (char*)"0", XspaceWin->X3d_Tggl);
	G_BOOL (char*)"disp.draw_tile", &XTiles, (char*)"0", XspaceWin->XTiles_Tggl);
	G_BOOL (char*)"disp.pair_boundary", &XTileBound, (char*)"0", XspaceWin->XTileBound_Tggl);
	G_BOOL (char*)"disp.draw_edge", &XInputEdge, (char*)"0", XspaceWin->XInputEdge_Tggl);
	G_BOOL (char*)"disp.draw_fe_mesh", &XFeMesh, (char*)"0", XspaceWin->XFeMesh_Tggl);
	G_BOOL (char*)"disp.draw_sub_term", &XSubTerm, (char*)"0", XspaceWin->XSubTerm_Tggl);
	G_BOOL (char*)"disp.draw_delaunay", &XDelaunay, (char*)"0", XspaceWin->XDelaunay_Tggl);
	G_BOOL (char*)"disp.draw_sub_resistor", &XSubRes, (char*)"0", XspaceWin->XSubRes_Tggl);
	G_BOOL (char*)"disp.draw_resistor", &XResistor, (char*)"0", XspaceWin->XResistor_Tggl);
	if (action == WRITE_PARAMS && XResistor.value) fprintf (fp, "disp.undraw_resistor\n");
	G_BOOL (char*)"disp.draw_out_resistor", &XOutputRes, (char*)"0", XspaceWin->XOutputRes_Tggl);
	G_BOOL (char*)"disp.draw_equi_lines", &XEquiPot, (char*)"0", XspaceWin->XEquiPot_Tggl);
	break;

    case IMPORT_OPTION:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Options for cell import #####\n");
	    fprintf (fp,   "###################################\n");
	    get_before_write = XtIsManaged (ImportWin->ImportDialog);
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (ImportWin->ImportDialog))
		set_after_read = 1;
	}
	break;

    case DATABASE_FOR_IMPORT:
	G_STR (char*)"DbaseForImport", &DbaseForImport, (char*)"", ImportWin->ImportRemoteDatabaseText);
	break;

    case SELECT_LOCAL_NAME:
	if (action == INIT_VALUES) {
	    ImpCellName.widgetChoice = NEW (Widget, 3);
	    ImpCellName.widgetChoice[0] = ImportWin->ImpCellRemName_Bttn;
	    ImpCellName.widgetChoice[1] = ImportWin->ImpCellPrefixedRemName_Bttn;
	    ImpCellName.widgetChoice[2] = ImportWin->ImpCellNewName_Bttn;
	}
	G_INTC (char*)"ImpCellName", &ImpCellName, (char*)"1 3 3", ImportWin->ImpCellName_OptnMn);
	if (setAction || set_after_read) SetImportCellSelectionPolicy (w_value, NULL, NULL);
	break;

    case DEVICE_MODELS:
	if (action == WRITE_HELIOS) {
	    fprintf (fp, "\n##### Options for device model import #####\n");
	    fprintf (fp,   "###########################################\n");
	    get_before_write = XtIsManaged (DeviceModelWin->DeviceModelDialog);
	}
	else if (readAction) {
	    set_after_read = 0;
	    if (loadAlsoOpenForms && XtIsManaged (DeviceModelWin->DeviceModelDialog))
		set_after_read = 1;
	}
	if (action == INIT_VALUES) {
	    DevModInputSource.widgetChoice = NEW (Widget, 2);
	    DevModInputSource.widgetChoice[0] = DeviceModelWin->DevModFromFile_Tggl;
	    DevModInputSource.widgetChoice[1] = DeviceModelWin->DevModFromDbase_Tggl;
	    DevModSaveMode.widgetChoice = NEW (Widget, 3);
	    DevModSaveMode.widgetChoice[0] = DeviceModelWin->SaveInFile;
	    DevModSaveMode.widgetChoice[1] = DeviceModelWin->SaveInDb;
	    DevModSaveMode.widgetChoice[2] = DeviceModelWin->SaveInDbEquiv_Tggl;
	}
	G_INTC (char*)"DevModInputSource", &DevModInputSource, (char*)"0 1 0", NULL);
	G_STR (char*)"DevModInputFile",  &DevModInputFile,  (char*)"", DeviceModelWin->DevModFromFile_Txt);
	G_STR (char*)"DevModInputDbase", &DevModInputDbase, (char*)"", DeviceModelWin->DevModFromDbase_Txt);
	G_INTC (char*)"DevModSaveMode", &DevModSaveMode, (char*)"0 2 1", NULL);
	break;

    case LAST_EXTRACTION_PARAMETER:
    case LAST_PARAMETER:
	get_before_write = set_after_read = 0; /* MUST BE PUT OFF! */
    }
}

void Set3dWholeForm (Widget w, XtPointer p, XtPointer q)
{
    int m = XtIsManaged (ParSubAccrtWin->ParSubAccrtDialog);
    int item;

    for (item = SUBRES_ACCRT_FINE + 1; item < END_SUBRES_ACCRT_FINE; item++) {
	AdmVariable (item, m ? GET_VALUES : GET_C_DEF, NULL);
    }
    for (item = SUBCAP_ACCRT_FINE + 1; item < END_SUBCAP_ACCRT_FINE; item++) {
	AdmVariable (item, SET3D_VALUES, NULL);
    }
}

/**********************************************************************
*
* Insert the content of file with name in the string `name' into the
* file with filepointer fp.
*
**********************************************************************/
void InsertIntoPfile (FILE *fp, char *name)
{
    FILE *input;
    char *see = (char*)"\n(see Extractor -> Extraction Options -> More Options)";
    int suppressWarning = 0;

    if (!fp || !*name) return;

    if (!(input = fopen (name, "r"))) {
	PopupMessageBox (mprintf ("Could not find external parameter file %s%s", name, see));
	return;
    }

    while (fgets (globalTextBuffer, sizeof (globalTextBuffer) , input)) {
	if (!strchr (globalTextBuffer, '\n') && !suppressWarning) {
	    suppressWarning = 1;
	    PopupMessageBox (mprintf ("Line(s) of additional parameter file too long -%s%s",
		"\n may be included in mutilated form", see));
	}
	fprintf (fp, "%s", globalTextBuffer);
    }
    fclose (input);
}

/********************************************************************************
*
* Look in file fp for a line like
* "keyword  actualValue"  or "keyword = actualValue" or "keyword : actualValue".
* If found, copy at most n-1 characters of the string actualValue into value.
* Omit everything from an '#' onwards. behind Take care of '\0' termination.
*
*********************************************************************************/
int GetNValueGivenKeyword (FILE *fp, char *keyword, char *value, int n)
{
    char *marker, *s, c;
    char lineBuffer[MAX_LINE_LENGTH];
    int  len;

    if (fp && (len = strlen (keyword))) {
        rewind (fp);
        while (fgets (lineBuffer, sizeof (lineBuffer), fp) != NULL) {
            if (!strchr (lineBuffer, '\n'))
                say ("Line just read is too long for line buffer - read text may be mutilated!");
            if ((s = strchr (lineBuffer, '#'))) *s = '\0';
            if (!strncmp (lineBuffer, keyword, len)) {
                marker = lineBuffer + len - 1;
                while ((c = *(++marker)) == ' ' || c == '\t' || c == ':' || c == '=');
                if ((s = strchr (marker, '\n'))) *s = '\0';
                strncpy (value, marker, n-1);
                *(value+n-1) = '\0';
		return (1);
            }
        }
    }
    *value = '\0';
    return (0);
}

void writeKeyword (FILE *fp, char *keyword, char *value)
{
    int i, imax = 25;

    i = strlen (keyword);
    while (imax < i + 2) imax += 5;
    imax -= i;
    fprintf (fp, "%s%*s%s\n", keyword, imax, "", value);
}

/* private 'strtok' function is only used by iniString,
   this string contains no tabs */
char *strTok (char *string)
{
    static char *s;

    if (!string) {
	while (*s && *s != ' ') ++s;
    }
    else s = string;
    while (*s == ' ') ++s;
    return s;
}

/*********************************************************************************************
*
* Routine that comprises the general administrative tasks that do not depend on the
* particular variable.
*
**********************************************************************************************/
void GeneralAdmin (int action, int dataType, FILE *fp, char *keyword,
		void *variable, char *iniString, Widget textField)
{
    char valueAsString[MAX_LINE_LENGTH + 1], *marker, *xtStr;
    float r_value = 0;
    int ok;

    marker = (char*)""; // init, to eliminate compiler warning

    if (get_before_write) action = GET_VALUES;

again:
    if (dataType == T_STR) {
	str_param_t *strpara = (str_param_t *) variable;

	switch (action) {
	case INIT_VALUES:
	    strpara->gdflt = strsave (iniString);
	    strpara->fdflt = strsave (iniString);
	    strpara->udflt = strsave (iniString);
	    strpara->cdflt = strsave (iniString);
	    strpara->value = strsave (iniString);
	    break;
	case READ_U_DEF:
	case READ_F_DEF:
	    ok = GetNValueGivenKeyword (fp, keyword, valueAsString, sizeof (valueAsString));
	    if (*valueAsString) {
		marker = valueAsString;
		if (s_separator) {
		    if (!ispunct (*marker)) ok = 0;
		    else marker[1] = '\0';
		}
	    }
	    if (!ok) {
		marker = action == READ_U_DEF ? strpara->fdflt : strpara->gdflt;
		if (!*marker) break;
	    }
	    if (action == READ_F_DEF) {
		DISPOSE (strpara->fdflt, 0);
		strpara->fdflt = strsave (marker);
	    }
	    DISPOSE (strpara->udflt, 0);
	    strpara->udflt = strsave (marker);
	    DISPOSE (strpara->cdflt, 0);
	    strpara->cdflt = strsave (marker);
	    if (InitialSetValues) {
		DISPOSE (strpara->value, 0);
		strpara->value = strsave (marker);
	    }
	    if (set_after_read) { action = SET_VALUES; goto again; }
	    break;
	case SET_G_DEF:
		marker = strpara->gdflt;
	case SET_F_DEF:
	    if (action == SET_F_DEF)
		marker = strpara->fdflt;
	case SET_U_DEF:
	    if (action == SET_U_DEF)
		marker = strpara->udflt;
	case SET_VALUES:
	    if (action == SET_VALUES)
		marker = strpara->cdflt;
	    if (*marker) XmTextFieldSetString (textField, marker);
	    break;
	case GET_C_DEF:
	    DISPOSE (strpara->value, 0);
	    strpara->value = strsave (strpara->cdflt);
	    break;
	case ACC_VALUES:
	case GET_VALUES:
	    ok = 1;
	    xtStr = trim (XmTextFieldGetString (textField));
	    if (s_separator) {
		if (!ispunct (*xtStr)) ok = 0;
		else xtStr[1] = '\0';
	    }
	    else if (s_notempty && !*xtStr) ok = 0;
	    if (ok) {
		DISPOSE (strpara->value, 0);
		strpara->value = strsave (xtStr);
	    }
	    XtFree (xtStr);
	    if (action == ACC_VALUES) {
		DISPOSE (strpara->cdflt, 0);
		strpara->cdflt = strsave (strpara->value);
		break;
	    }
	    XmTextFieldSetString (textField, strpara->value);
	    if (!get_before_write) break;
	case WRITE_PARAMS:
	case WRITE_HELIOS:
	    marker = (action == WRITE_PARAMS || get_before_write)? strpara->value : strpara->cdflt;
	    if (fp && *marker) {
		writeKeyword (fp, keyword, marker);
	    }
	}
    }
    else if (dataType == T_STRC) {
	strc_param_t *strpara = (strc_param_t *) variable;

	switch (action) {
	case INIT_VALUES:
	    i_value = atoi (iniString);
	    strpara->fdflt = strpara->gdflt = i_value;
	    strpara->udflt = strpara->fdflt;
	    strpara->value = strpara->udflt;
	    strpara->cdflt = strpara->udflt;
	    break;
	case READ_U_DEF:
	case READ_F_DEF:
	    ok = GetNValueGivenKeyword (fp, keyword, valueAsString, sizeof (valueAsString));
	    for (i_value = 0; i_value < strpara->nr; ++i_value)
		if (!strcmp (strpara->restrictedChoice[i_value], valueAsString)) break;
	    if (i_value == strpara->nr) { // strpara not found
		if (strcpara_sub3d) // "subcap3d.xxx" value not found!
		    i_value = strcpara_sub3d->cdflt; // use "sub3d.xxx" value
		else if (action == READ_U_DEF)
		    i_value = strpara->fdflt;
		else
		    i_value = strpara->gdflt;
	    }
	    if (action == READ_F_DEF)
		strpara->fdflt = i_value;
	    strpara->udflt = i_value;
	    strpara->cdflt = i_value;
	    if (InitialSetValues) strpara->value = i_value;
	    if (set_after_read) { action = SET_VALUES; goto again; }
	    break;
	case SET_G_DEF:
		i_value = strpara->gdflt;
	case SET_F_DEF:
	    if (action == SET_F_DEF)
		i_value = strpara->fdflt;
	case SET_U_DEF:
	    if (action == SET_U_DEF)
		i_value = strpara->udflt;
	case SET_VALUES:
	    if (action == SET_VALUES)
		i_value = strpara->cdflt;
	case SET3D_VALUES:
	    if (action == SET3D_VALUES) {
		ASSERT (strcpara_sub3d);
		i_value = strcpara_sub3d->value; // use "sub3d.xxx" value
	    }
	    w_value = strpara->widgetChoice[i_value];
	    XtVaSetValues (textField, XmNmenuHistory, w_value, NULL);
	    break;
	case GET_C_DEF:
	    strpara->value = strpara->cdflt;
	    break;
	case ACC_VALUES:
	case GET_VALUES:
	    XtVaGetValues (textField, XmNmenuHistory, &w_value, NULL);
	    for (i_value = 0; i_value < strpara->nr; ++i_value)
		if (strpara->widgetChoice[i_value] == w_value) break;
	    if (i_value == strpara->nr) i_value = strpara->udflt;
	    strpara->value = i_value;
	    if (action == ACC_VALUES) { strpara->cdflt = i_value; break; }
	    if (!get_before_write) break;
	case WRITE_HELIOS:
	case WRITE_PARAMS:
	    if (fp) {
		int i;
		i = (action == WRITE_PARAMS || get_before_write)? strpara->value : strpara->cdflt;
		writeKeyword (fp, keyword, strpara->restrictedChoice[i]);
	    }
	}
    }
    else if (dataType == T_REAL) {
	float_param_t *floatpara = (float_param_t *) variable;

	switch (action) {
	case INIT_VALUES:
	    floatpara->min = stringToFloat (strTok (iniString));
	    floatpara->max = stringToFloat (strTok (NULL));
	    r_value = stringToFloat (strTok (NULL));
		 if (r_value < floatpara->min) r_value = (r_ok && r_value == r_ok)? r_ok : floatpara->min;
	    else if (r_value > floatpara->max) r_value = floatpara->max;
	    floatpara->gdflt = r_value;
	    floatpara->fdflt = r_value;
	    floatpara->udflt = r_value;
	    floatpara->cdflt = r_value;
	    floatpara->value = r_value;
	    break;
	case READ_U_DEF:
	case READ_F_DEF:
	    ok = GetNValueGivenKeyword (fp, keyword, valueAsString, sizeof (valueAsString));
	    if (*valueAsString) {
		r_value = stringToFloat (valueAsString);
		     if (r_value < floatpara->min) r_value = (r_ok && r_value == r_ok)? r_ok : floatpara->min;
		else if (r_value > floatpara->max) r_value = floatpara->max;
	    }
	    else if (floatpara_sub3d) // "subcap3d.xxx" value not found!
		r_value = floatpara_sub3d->cdflt; // use "sub3d.xxx" value
	    else if (action == READ_U_DEF)
		r_value = floatpara->fdflt;
	    else
		r_value = floatpara->gdflt;
	    if (action == READ_F_DEF)
		floatpara->fdflt = r_value;
	    floatpara->udflt = r_value;
	    floatpara->cdflt = r_value;
	    if (InitialSetValues) floatpara->value = r_value;
	    if (set_after_read) { action = SET_VALUES; goto again; }
	    break;
	case SET_G_DEF:
		r_value = floatpara->gdflt;
	case SET_F_DEF:
	    if (action == SET_F_DEF)
		r_value = floatpara->fdflt;
	case SET_U_DEF:
	    if (action == SET_U_DEF)
		r_value = floatpara->udflt;
	case SET_VALUES:
	    if (action == SET_VALUES)
		r_value = floatpara->cdflt;
	case SET3D_VALUES:
	    if (action == SET3D_VALUES) {
		ASSERT (floatpara_sub3d);
		r_value = floatpara_sub3d->value; // use "sub3d.xxx" value
	    }
	    XmTextFieldSetString (textField, floatToString (r_value / r_scale));
	    break;
	case GET_C_DEF:
	    floatpara->value = floatpara->cdflt;
	    break;
	case ACC_VALUES:
	case GET_VALUES:
	    r_value = r_scale * stringToFloat (xtStr = XmTextFieldGetString (textField));
	    XtFree (xtStr);
		 if (r_value < floatpara->min) r_value = (r_ok && r_value == r_ok)? r_ok : floatpara->min;
	    else if (r_value > floatpara->max) r_value = floatpara->max;
	    floatpara->value = r_value;
	    if (action == ACC_VALUES) { floatpara->cdflt = r_value; break; }
	    XmTextFieldSetString (textField, floatToString (r_value / r_scale));
	    if (!get_before_write) break;
	case WRITE_HELIOS:
	case WRITE_PARAMS:
	    if (fp) {
		r_value = (action == WRITE_PARAMS || get_before_write)? floatpara->value : floatpara->cdflt;
		writeKeyword (fp, keyword, floatToString (r_value));
	    }
	}
    }
    else if (dataType == T_INT) {
	int_param_t *intpara = (int_param_t *) variable;

	switch (action) {
	case INIT_VALUES:
	    intpara->min = stringToInt (strTok (iniString));
	    intpara->max = stringToInt (strTok (NULL));
	    intpara->gdflt = stringToInt (strTok (NULL));
	    if (intpara->gdflt < intpara->min)
		intpara->gdflt = intpara->min;
	    else if (intpara->gdflt > intpara->max)
		intpara->gdflt = intpara->max;
	    intpara->fdflt = intpara->gdflt;
	    intpara->udflt = intpara->fdflt;
	    intpara->value = intpara->udflt;
	    intpara->cdflt = intpara->udflt;
	    break;
	case READ_U_DEF:
	case READ_F_DEF:
	    ok = GetNValueGivenKeyword (fp, keyword, valueAsString, sizeof (valueAsString));
	    if (*valueAsString) {
		i_value = stringToInt (valueAsString);
		     if (i_value < intpara->min) i_value = intpara->min;
		else if (i_value > intpara->max) i_value = intpara->max;
	    }
	    else if (intpara_sub3d) // "subcap3d.xxx" value not found!
		i_value = intpara_sub3d->cdflt; // use "sub3d.xxx" value
	    else if (action == READ_U_DEF)
		i_value = intpara->fdflt;
	    else
		i_value = intpara->gdflt;
	    if (action == READ_F_DEF)
		intpara->fdflt = i_value;
	    intpara->udflt = i_value;
	    intpara->cdflt = i_value;
	    if (InitialSetValues) intpara->value = i_value;
	    if (set_after_read) { action = SET_VALUES; goto again; }
	    break;
	case SET_G_DEF:
		i_value = intpara->gdflt;
	case SET_F_DEF:
	    if (action == SET_F_DEF)
		i_value = intpara->fdflt;
	case SET_U_DEF:
	    if (action == SET_U_DEF)
		i_value = intpara->udflt;
	case SET_VALUES:
	    if (action == SET_VALUES)
		i_value = intpara->cdflt;
	case SET3D_VALUES:
	    if (action == SET3D_VALUES) {
		ASSERT (intpara_sub3d);
		i_value = intpara_sub3d->value; // use "sub3d.xxx" value
	    }
	    XmTextFieldSetString (textField, intToString (i_value));
	    break;
	case GET_C_DEF:
	    intpara->value = intpara->cdflt;
	    break;
	case ACC_VALUES:
	case GET_VALUES:
	    i_value = stringToInt (xtStr = XmTextFieldGetString (textField));
	    XtFree (xtStr);
		 if (i_value < intpara->min) i_value = intpara->min;
	    else if (i_value > intpara->max) i_value = intpara->max;
	    intpara->value = i_value;
	    if (action == ACC_VALUES) { intpara->cdflt = i_value; break; }
	    XmTextFieldSetString (textField, intToString (i_value));
	    if (!get_before_write) break;
	case WRITE_HELIOS:
	case WRITE_PARAMS:
	    if (fp) {
		i_value = (action == WRITE_PARAMS || get_before_write)? intpara->value : intpara->cdflt;
		writeKeyword (fp, keyword, intToString (i_value));
	    }
	}
    }
    else if (dataType == T_INTC) {
	intc_param_t *intpara = (intc_param_t *) variable;

	switch (action) {
	case INIT_VALUES:
	    intpara->min = atoi (strTok (iniString));
	    intpara->max = atoi (strTok (NULL));
	    i_value = atoi (strTok (NULL));
		 if (i_value < intpara->min) i_value = intpara->min;
	    else if (i_value > intpara->max) i_value = intpara->max;
	    intpara->fdflt = intpara->gdflt = i_value;
	    intpara->udflt = intpara->fdflt;
	    intpara->cdflt = intpara->udflt;
	    intpara->value = intpara->udflt;
	    break;
	case READ_U_DEF:
	case READ_F_DEF:
	    ok = GetNValueGivenKeyword (fp, keyword, valueAsString, sizeof (valueAsString));
	    if (*valueAsString) {
		i_value = atoi (valueAsString);
		     if (i_value < intpara->min) i_value = intpara->min;
		else if (i_value > intpara->max) i_value = intpara->max;
	    }
	    else if (intcpara_sub3d) // "subcap3d.xxx" value not found!
		i_value = intcpara_sub3d->cdflt; // use "sub3d.xxx" value
	    else if (action == READ_U_DEF)
		i_value = intpara->fdflt;
	    else
		i_value = intpara->gdflt;
	    if (action == READ_F_DEF)
		intpara->fdflt = i_value;
	    intpara->udflt = i_value;
	    intpara->cdflt = i_value;
	    if (InitialSetValues) intpara->value = i_value;
	    if (set_after_read) { action = SET_VALUES; goto again; }
	    break;
	case SET_G_DEF:
		i_value = intpara->gdflt;
	case SET_F_DEF:
	    if (action == SET_F_DEF)
		i_value = intpara->fdflt;
	case SET_U_DEF:
	    if (action == SET_U_DEF)
		i_value = intpara->udflt;
	case SET_VALUES:
	    if (action == SET_VALUES)
		i_value = intpara->cdflt;
	case SET3D_VALUES:
	    if (action == SET3D_VALUES) {
		ASSERT (intcpara_sub3d);
		i_value = intcpara_sub3d->value; // use "sub3d.xxx" value
	    }
	    w_value = intpara->widgetChoice[i_value - intpara->min];
	    if (textField)
		XtVaSetValues (textField, XmNmenuHistory, w_value, NULL);
	    else
		XmToggleButtonSetState (w_value, TRUE, TRUE);
	    break;
	case GET_C_DEF:
	    intpara->value = intpara->cdflt;
	    break;
	case ACC_VALUES:
	case GET_VALUES:
	    if (textField) {
		XtVaGetValues (textField, XmNmenuHistory, &w_value, NULL);
		for (i_value = intpara->min; i_value <= intpara->max; ++i_value)
		    if (intpara->widgetChoice[i_value - intpara->min] == w_value) break;
	    }
	    else {
		for (i_value = intpara->min; i_value <= intpara->max; ++i_value)
		    if (XmToggleButtonGetState (intpara->widgetChoice[i_value - intpara->min])) break;
	    }
	    if (i_value > intpara->max) i_value = intpara->udflt;
	    intpara->value = i_value;
	    if (action == ACC_VALUES) { intpara->cdflt = i_value; break; }
	    if (!get_before_write) break;
	case WRITE_HELIOS:
	case WRITE_PARAMS:
	    if (fp) {
		i_value = (action == WRITE_PARAMS || get_before_write)? intpara->value : intpara->cdflt;
		writeKeyword (fp, keyword, intToString (i_value));
	    }
	}
    }
    else if (dataType == T_BOOL) {
	bool_param_t *boolpara = (bool_param_t *) variable;

	switch (action) {
	case INIT_VALUES:
	    boolpara->gdflt = *iniString == '1' ? 1 : 0;
	    boolpara->fdflt = boolpara->gdflt;
	    boolpara->udflt = boolpara->fdflt;
	    boolpara->cdflt = boolpara->udflt;
	    boolpara->value = boolpara->udflt;
	    break;
	case READ_U_DEF:
	case READ_F_DEF:
	    ok = GetNValueGivenKeyword (fp, keyword, valueAsString, sizeof (valueAsString));
	    if (*valueAsString)
		b_value = stringToBool (valueAsString);
	    else if (action == READ_U_DEF)
		b_value = boolpara->fdflt;
	    else
		b_value = boolpara->gdflt;
	    if (action == READ_F_DEF)
		boolpara->fdflt = b_value;
	    boolpara->udflt = b_value;
	    boolpara->cdflt = b_value;
	    /* The 'boolpara->value' must also be set after 'set_after_read'
		because the 'textField' can be NULL and
		in that case 'get_before_write' does not work! */
	    if (InitialSetValues || set_after_read) boolpara->value = b_value;
	    if (set_after_read) { action = SET_VALUES; goto again; }
	    break;
	case SET_G_DEF:
		b_value = boolpara->gdflt;
	case SET_F_DEF:
	    if (action == SET_F_DEF)
		b_value = boolpara->fdflt;
	case SET_U_DEF:
	    if (action == SET_U_DEF)
		b_value = boolpara->udflt;
	case SET_VALUES:
	    if (action == SET_VALUES)
		b_value = boolpara->cdflt;
	    if (textField)
		XmToggleButtonSetState (textField, b_value, TRUE);
	    break;
	case GET_C_DEF:
	    boolpara->value = boolpara->cdflt;
	    break;
	case ACC_VALUES:
	case GET_VALUES:
	    if (textField)
		boolpara->value = b_value = XmToggleButtonGetState (textField);
	    if (action == ACC_VALUES) { boolpara->cdflt = boolpara->value; break; }
	    if (!get_before_write) break;
	case WRITE_HELIOS:
	case WRITE_PARAMS:
	    if (fp) {
		b_value = (action == WRITE_PARAMS || get_before_write)? boolpara->value : boolpara->cdflt;
		writeKeyword (fp, keyword, boolToString (b_value));
	    }
	}
    }
    else
	say ("Invalid parameter data type %d (keyword = \"%s\")!", dataType, keyword);
}

float stringToFloat (char *string)
{
    float value;
    char *s = string;
    if (*s == '-') ++s;
    if (*s == 'i' || *s == 'I') {
	++s;
	if (*s == 'n' || *s == 'N') {
	    ++s;
	    if (*s == 'f' || *s == 'F') {
		value = (*string == '-') ? -FLT_MAX : FLT_MAX;
		return value;
	    }
	}
    }
    value = atof (string);
    return value;
}

char *floatToString (float value)
{
    char *string;

    if (value >= 0.999 * FLT_MAX)
        string = (char*)"inf";
    else if (value <= -0.999 * FLT_MAX)
        string = (char*)"-inf";
    else
        string = mprintf ("%g", value);
    return string;
}

int stringToInt (char *string)
{
    int value;
    char *s = string;
    if (*s == '-') ++s;
    if (*s == 'i' || *s == 'I') {
	++s;
	if (*s == 'n' || *s == 'N') {
	    ++s;
	    if (*s == 'f' || *s == 'F') {
		value = (*string == '-') ? -INT_MAX : INT_MAX;
		return value;
	    }
	}
    }
    value = atoi (string);
    return value;
}

char *intToString (int value)
{
    char *string;

    if (value >= (int) (0.999 * INT_MAX))
        string = (char*)"inf";
    else if (value <= -(0.999 * INT_MAX))
        string = (char*)"-inf";
    else
        string = mprintf ("%d", value);
    return string;
}

int stringToBool (char *s)
{
    if (*s == '1') return 1;
    if (*s == 'o' || *s == 'O') {
	++s;
	if (*s == 'n' || *s == 'N') return 1;
    }
    return 0;
}

char *boolToString (int value)
{
    return (value ? (char *)"on" : (char *)"off");
}

