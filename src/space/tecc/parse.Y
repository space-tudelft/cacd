%{
/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Kees-Jan van der Kolk
 *	Edwin Matthijssen (EM)
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

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "src/space/libmin/minrobdd.h"
#include "src/space/tecc/define.h"
#include "src/space/tecc/extern.h"

	/* ROBDD
	*/
static 	MINRobdd robdd;
#define ROBDD_NOT(x)   robdd.ite (x, 0, 1)
#define ROBDD_AND(x,y) robdd.ite (x, y, 0)
#define ROBDD_OR(x,y)  robdd.ite (x, 1, y)

	/* local functions
	*/
static struct layCondRef * add_cond (struct layCondRef *cond);
static struct condListRef *add_condlist (struct condListRef *cref);
static void dumpExpression (ostream& ostr, const MINTable& table,
		const std::vector<MINSymbol*>& variables);
static void expression2CondList (char *name, const MINTable& table,
                const std::vector<MINSymbol*>& variables);

Private void makeConductor (struct condListRef *cref, char *msk, int mskno, double value, double tn);
Private void makeContact   (struct condListRef *cref, struct layCondRef *cvia,
			int k, int msk1, int msk2, double value);
Private void makeSubCont (struct condListRef *cref, int msk);
Private int makeNewMask (struct condListRef *cref, char *msk, int mskno);
Private struct resistance * newres (void);
Private struct transistor * newtor (void);
Private struct contact * newcon (void);
Private struct capacitance * newcap (void);
Private struct vdimension * newvdim (void);
Private struct shape * newshape (void);
Private struct junction * newjun (void);
Private struct bipoTor * newbjt (void);
Private struct connect * newcnt (void);
Private struct resizedata * newresize (void);
Private struct waferdata * newwafer (void);
Private struct subcont * newsubcont (void);
Private int setsubcontccnr (int ccmsk, int id, int cap);
Private int maskno (char *name);
Private int polarityno (char *name);
Private int yesno (char *name);
Private int bjtno (char *name);
Private char * checkElemName (char *name);
Private struct newMaskRef *addNewMask (int mask);
Private struct newMaskRef *findNewMask (int mask);
Private void handle_min_terms (char *name, STDSDWord sdword);
Private STDSDWord make_minterm (struct layCondRef *c, int p);
Private void checkTransistorDSMask (char *name, int mask, int pol);
Private int  checkPinPol (char *name, int msk, int occ, int mode);
Private void checkShapeCond (STDSDWord cond);
Private int  checkPinCond (int msk, int occ, STDSDWord e1, int mode);
Private void checkResizeCond (int mask, struct newMaskRef *nm);
Private double getDielConstant (double z0, double z1);
Private void setDsCap (STDSDWord sdword, int smode);

#ifdef __cplusplus
  extern "C" {
#endif
extern int printCondList;
extern int specific_res;
#ifdef __cplusplus
  }
#endif

extern FILE *yyin;
extern int yylineno;
extern int notFatal;

int doingBjt = 0;
int doingCap = 0;
int doingConnect = 0;
int doingNewmask = 0;
int doingResize = 0;
int doingShape = 0;

#define MAXMINTERM    8
#define MAX_NAMES  2048

struct wmlist {
    char *name;
    int  mskno;
    struct wmlist *next;
};
struct wmlist *begin_wmlist;

char **nameTab = NULL;
static int name_cnt = 0;
static int maxsubmasks = 0;
int profile_cnt = 0;
int prev_mskno, restype, subconn, viamask;
int w_depth = 0;
struct condListRef *prev_cref;
STDSDWord prev_sw = 0;

char namebuf1[132];
char namebuf2[132];
char *cap_name;

int nmBufToggle = 0;
struct layCondRef **condStack = NULL;

int capCurrent = 0;	/* EM: indicates whether the current
			 *     element is a capacitor or not
			 */
char ressort[80];
char capsort[80];
char consort[80];

struct contact *curr_con;
struct contact *other_con;
struct transistor *curr_tor;
struct transistor *other_tor;
struct transistor *end_tor;
struct capacitance *curr_cap;
struct capacitance *other_cap;
struct capacitance *curr_ecap;
struct capacitance *other_ecap;
struct resistance *curr_res;
struct resistance *other_res;
struct vdimension *curr_vdim;
struct vdimension *other_vdim;
struct shape *curr_shape;
struct shape *other_shape;
struct bipoTor *curr_bjt;
struct bipoTor *other_bjt;
struct junction *curr_jun;
struct junction *other_jun;
struct connect *curr_cnt;
struct connect *other_cnt;
struct subcont *curr_subcont;
struct subcont *other_subcont;
struct resizedata *curr_resize = NULL;
struct waferdata *curr_wafer;

struct maskRef *last_mask_list;
struct layerRef *last_layer_list;
struct layCondRef *last_cond_list;

struct newMaskRef *first_newmask_list = NULL;
struct newMaskRef *last_newmask_list;

char shapetype;
int val_cnt;
double retval[4];

static struct x_y an_x_y;
static struct x_y *curr_x_y_vals;
static struct x_y *last_x_y_vals;
static int curr_nr_x_y_vals;

static double res_unit = 1.0;          /* default values */
static double c_res_unit = 1.0;
static double s_res_unit = 1.0;
static double cap_unit = 1.0;
static double a_cap_unit = 1.0;
static double e_cap_unit = 1.0;
static double x_cap_unit = 1.0;
static double vdim_unit = 1.0;
static double shape_unit = 1.0;
static double depth_unit = 1.0;
static double resize_unit = 1.0;
static double BD = 0.0;

static int junction_cap;
static int minTermCnt;

%}

%union		{
		    int ival;
		    double dval;
                    char *sval;
		    struct layer *layp;
		    struct layerCond *layCp;
		    struct maskRef *maskr;
		    struct layerRef *layr;
		    struct layCondRef *layCr;
		    struct x_y *xy_val;
		    STDSDWord sdword;
		}

%token 		MAXKEYS MINUS EQUAL COLON NOT LPS RPS SLASH
%token		KEYS CONTACTS TRANSISTORS CAPACITANCES CONDUCTORS OR
%token		VDIMENSIONS ESHAPES CSHAPES UNIT RESISTANCE C_RESISTANCE
%token		S_RESISTANCE SET SPECIFIC_RES
%token		CAPACITANCE A_CAPACITANCE E_CAPACITANCE VDIMENSION SHAPE
%token		OMIT_CAP3D KEEP_CAP2D
%token		DISTANCE LAYERDEPTH JUNCTIONS BIPOTORS CONNECTS
%token          NEWMASK FETS BJTS ILLEGAL PERC SUB GND JUNCTION
%token		SUBLAYERS SUBCAPLAYERS DIELECTRICS SELFSUBRES COUPSUBRES COLORS
%token		GRID_COUNT
%token		NEUMANN_SIMULATION_RATIO
%token		NUM_ANNEALING_ITERATIONS
%token		MAX_DETERMINANT_BINNING_ERROR
%token		MAX_ADJOINT_BINNING_ERROR
%token          MAX_ANNEALED_INVERSE_MATRIX_BINNING_ERROR
%token          MAX_PREPROCESSED_ANNEALING_MATRICES_BINNING_ERROR
%token          MAX_ANNEALING_ERROR
%token          MAX_REDUCE_ERROR
%token		ZQ_VALUES ZP_VALUES R_VALUES R_SWITCH
%token		RESIZEMASK
%token		WAFER VIAMASK SUBCONN RESTYPE BEM_DEPTH
%token <ival>	INTEGER
%token <sval>	IDENTIFIER HEX_NAME
%token <dval>	DOUBLE

%type  <sval>	color_name name identif primary_name
%type  <dval>	val resizeval vdim_cap2d_val vdim_spacing
%type  <ival>	mask bulkmask mask_s contact_mask_s ivals
%type  <layp>	layer layer_s layer_s2 substrterm
%type  <layr>	layer_list
%type  <xy_val>	cap_val
%type  <sdword>	min_terms and_expression or_expression unary_expression
%type  <sdword>	literal primary

%%

tech_descr	: header_descr conduc_descr tor_descr bjt_descr
		  jun_descr cnt_descr contact_descr cap_descr vdim_descr
		  eshape_descr cshape_descr dielec_descr sublay_descr subcaplay_descr
		  selfsub_descr mutsub_descr
		;

header_descr	: /* empty */
		| header_descr header_item
		;

header_item	: unit
		| settings
		| key_info
		| resizemask_def
		| newmask_def
		| wafer_def
		| color_descr
		;

settings	: SET SPECIFIC_RES { specific_res = 1; }
		| SET BEM_DEPTH val
		{
		    if ((bem_depth = (int)($3 * 1e3 + 0.5)) < 100) {
			char *s = mprintf ("%.3f micron", $3);
			if (bem_depth <= 0)
			    fatalErr ("illegal value for bem_depth:", s);
			else
			    warningMes ("small value for bem_depth:", s);
		    }
		    BD = bem_depth / 1e3;
		}
		;

key_info	: KEYS COLON layer_list
		{
		    if (keylist)
			warningMes ("new keys statement overwrites previous one!", 0);
		    if (maxNbrKeys >= 0)
			warningMes ("keys statement overwrites maxkeys statement!", 0);
		    keylist = $3;
		    maxNbrKeys = -2; /* no maximum for nbrKeys */
		}
		| MAXKEYS ivals
		{
		    if (keylist)
			warningMes ("maxkeys statement overwrites keys statement!", 0);
		    if (maxNbrKeys >= 0)
			warningMes ("new maxkeys statement overwrites previous one!", 0);
		    keylist = NULL;
		    if ((maxNbrKeys = $2) < 0) maxNbrKeys = 0;
		}
		;
ivals		: INTEGER INTEGER INTEGER
		{ $$ = $1; maxNbrKeys2 = $2; maxEdgeKeys = $3; }
		| INTEGER INTEGER
		{ $$ = $1; maxNbrKeys2 = $2; maxEdgeKeys = 0; }
		| INTEGER
		{ $$ = $1; maxNbrKeys2 = -1; maxEdgeKeys = 0; }
		;

layer_list	: layer
		{
		    ALLOC (last_layer_list, 1, struct layerRef);
		    last_layer_list -> lay = $1;
		    last_layer_list -> next = NULL;
		    $$ = last_layer_list;
		}
		| layer_list layer
		{
		    ALLOC (last_layer_list -> next, 1, struct layerRef);
		    last_layer_list = last_layer_list -> next;
		    last_layer_list -> lay = $2;
		    last_layer_list -> next = NULL;
		    $$ = $1;
		}
		;

color_descr     : COLORS
		{
		    notFatal = 1;
		}
		  COLON mask_colors
		{
		    notFatal = 0;
		}
		;

mask_colors	: /* empty */
		| mask_colors mask_color_def
		;

mask_color_def  : mask_s color_name
		{
		    int i = $1;
		    if (i != -1) {
			if (i < 0) masksubcolor = strsave ($2);
			else if (i < procdata -> nomasks)
			    maskdrawcolor[i] = strsave ($2);
			else
			    masknewcolor[i - procdata -> nomasks] = strsave ($2);
		    }
		}
		;

color_name	: name
		{
		    $$ = $1;
		}
		| HEX_NAME
		{
		    $$ = mprintf ("#%s", $1);
		}
		;

resizemask_def	: RESIZEMASK
		{
		    doingResize = 1;
		}
		  COLON min_terms COLON name COLON resizeval
		{
		    struct layCondRef *c;
		    struct newMaskRef *nm;
		    resizeCond * cond;
		    int id;

		    if ((id = maskno ($6)) < 0) {
			yyerror (mprintf ("Unknown new mask '%s' for resize", $6));
                    }

		    nm = 0;
		    if (id >= procdata -> nomasks && resizemask (id) < 0) { // first resize of new mask
			nm = findNewMask (id);
			ASSERT (nm);
			nm -> resized = 1;
			maxprocmasks++;
			(void) robdd.symbolNode (mprintf ("$%s", $6));
		    }

		    curr_resize = newresize ();
		    curr_resize -> id = id;
		    curr_resize -> newmaskname = strsave ($6);
		    curr_resize -> val = resize_unit * $8;
		    curr_resize -> condcnt = 1;
		    curr_resize -> cond = cond = new resizeCond;

		    // define for min_terms a unique resize symbol name
		    handle_min_terms (mprintf ("r#%d$%s", resize_cnt - 1, $6), $4);

		    if (minTermCnt > 0) {
			checkResizeCond (curr_resize -> id, nm);
			COLORINIT (cond -> present);
			COLORINIT (cond -> absent);
			for (c = condStack[--minTermCnt]; c; c = c -> next) {
			    id = c -> layC -> lay -> mask;
			    if (c -> layC -> present) {
				   COLOR_ADDINDEX (cond -> present, id); }
			    else { COLOR_ADDINDEX (cond -> absent , id); }
			}
			while (minTermCnt > 0) {
			    curr_resize -> condcnt++;
			    cond -> next = new resizeCond;
			    cond = cond -> next;
			    COLORINIT (cond -> present);
			    COLORINIT (cond -> absent);
			    for (c = condStack[--minTermCnt]; c; c = c -> next) {
				id = c -> layC -> lay -> mask;
				if (c -> layC -> present) {
				       COLOR_ADDINDEX (cond -> present, id); }
				else { COLOR_ADDINDEX (cond -> absent , id); }
			    }
			}
		    }
		    else yyerror ("resize mask condition list not found");

		    cond -> next = NULL;
		    doingResize = 0;
		}
		;

resizeval	: /* empty */
		{
		    $$ = 0;
		}
		| val
		{
		    $$ = $1;
		}
		;

newmask_def	: NEWMASK
		{
		    doingNewmask = 1;
		}
		  COLON min_terms COLON name
		{
                    STDSDWord e1, e2;
		    struct condListRef *cref;
		    struct newMaskRef *nm;
		    char *msk = $6;

		    if (maskno (msk) >= 0) yyerror (mprintf ("Redefinition of mask '%s'", msk));

		    handle_min_terms (msk, $4);

		    if (subdata -> nomasks == maxsubmasks) {
			maxsubmasks += MAXSUBMASKS;
			REALLOC (subdata -> mask_name, maxsubmasks, char *, maxsubmasks);
			REALLOC (masknewcolor, maxsubmasks, char *, subdata -> nomasks);
		    }
		    subdata -> mask_name[subdata -> nomasks] = strsave (msk);
		    subdata -> nomasks++;

		    nm = addNewMask (maskno (msk));
		    nm -> name = checkElemName (mprintf ("new$%s", msk));
		    nm -> id = name_cnt - 1;

		    ALLOC (cref, 1, struct condListRef);
		    nm -> cond_lists = cref;

		    e1 = e2 = 1;
		    if (minTermCnt > 0) {
			cref -> cond_list = condStack[--minTermCnt];
			e1 = make_minterm (cref -> cond_list, '-');
			e2 = make_minterm (cref -> cond_list, '=');
			while (minTermCnt > 0) {
			    ALLOC (cref -> next, 1, struct condListRef);
			    cref = cref -> next;
			    cref -> cond_list = condStack[--minTermCnt];
			    e1 = ROBDD_OR (e1, make_minterm (cref -> cond_list, '-'));
			    e2 = ROBDD_OR (e2, make_minterm (cref -> cond_list, '='));
			}
		    }

		    (void) robdd.defineSymbol (mprintf ("-%s", msk), e1);
		    (void) robdd.defineSymbol (mprintf ("=%s", msk), e2);

		    doingNewmask = 0;
		}
		;

wafer_def	: WAFER
		{
		    curr_wafer = newwafer ();
		    viamask = -1;
		    subconn = 1;
		    restype = P_TYPE;
                    doingNewmask = 1;
		}
		  COLON min_terms COLON val val val w_options
		{
		    static STDSDWord e1;
		    struct layCondRef viaCondRef, *cvia;
		    struct layerCond *via0, *via1;
		    struct layer *lay;
		    struct condListRef *cref, *crf;
		    char *msk = 0;
		    double d, baseSheetRes, baseContactRes, w_conduc;
		    int w_layers, w_thickness, i, k;

		    if (restype == M_TYPE) restype = N_TYPE;

		    if ((w_conduc = $6) > 1e4) {
			char *s = mprintf ("%.1f S/m", w_conduc);
			warningMes ("large value for wafer conductivity:", s);
		    }
		    else if (w_conduc <= 0.01) {
			char *s = mprintf ("%g S/m", w_conduc);
			if (w_conduc <= 1e-20)
			    fatalErr ("too small value for wafer conductivity:", s);
			warningMes ("small value for wafer conductivity:", s);
		    }

		    w_thickness = (int)($7 * 1e3 + 0.5);
		    if (w_thickness > 6000 && w_thickness > bem_depth) {
			char *s = mprintf ("%.3f micron", w_thickness * 1e-3);
			warningMes ("large value for wafer thickness:", s);
		    }
		    else if (w_thickness < 100) {
			char *s = mprintf ("%.3f micron", w_thickness * 1e-3);
			if (w_thickness <= 0)
			    fatalErr ("too small value for wafer thickness:", s);
			warningMes ("small value for wafer thickness:", s);
		    }

		    if ((w_layers = (int)($8)) > 7) {
			char *s = mprintf ("%d", w_layers);
			warningMes ("large value for wafer layers:", s);
		    }
		    else if (w_layers < 1) {
			fatalErr (mprintf ("wafer_%d:", waf_cnt),
			    "illegal number of layers specified (< 1)");
		    }

		    d = w_thickness * 1e-9;
		    if (w_layers > 1) {
			baseContactRes = d / w_conduc;
			d /= (w_layers - 1);
		    }
		    else {
			w_layers = 2;
			baseContactRes = 0;
			viamask = -1;
		    }
		    baseSheetRes = 1 / (w_conduc * d);

		    if (prev_sw != $4) { // new wafer profile
			if (!prev_sw) { // first profile
			    if (bem_depth <= 0) fatalErr ("no bem_depth", "specified");
			}
			else { // not first profile
			    set_profile_subcont ();
			}
			profile_cnt = waf_cnt;
			w_depth = 0;
			msk = strsave (mprintf ("w%d_1", waf_cnt));
			handle_min_terms (msk, $4);
			e1 = 1;
			ALLOC (cref, 1, struct condListRef);
			if (minTermCnt > 0) {
			    cref -> cond_list = condStack[--minTermCnt];
			    crf = cref;
			    e1 = make_minterm (cref -> cond_list, '-');
			    while (minTermCnt > 0) {
				ALLOC (crf -> next, 1, struct condListRef);
				crf = crf -> next;
				crf -> cond_list = condStack[--minTermCnt];
				e1 = ROBDD_OR (e1, make_minterm (crf -> cond_list, '-'));
			    }
			}
			(void) robdd.defineSymbol (mprintf ("-%s", msk), e1);
			prev_cref = cref;
		    }
		    else
			cref = prev_cref;

		    curr_wafer -> cref = cref;
		    curr_wafer -> depth = w_depth;
		    curr_wafer -> tn = w_thickness;
		    curr_wafer -> lays = w_layers;
		    curr_wafer -> type = restype;

		    if (viamask >= 0) {
			ALLOC (lay, 1, struct layer);
			lay -> mask = viamask;
			lay -> occurrence = SURFACE;
			ALLOC (via1, 1, struct layerCond);
			via1 -> present = 1; via1 -> lay = lay;
			ALLOC (via0, 1, struct layerCond);
			via0 -> present = 0; via0 -> lay = lay;
		    }
		    else via0 = via1 = 0;

		    for (k = i = 1; i <= w_layers; ++i) {
			struct w_list *p, *w;
			int w_depth2, mskno = -1;

			if (i+1 >= w_layers) {
			    w_depth2 = curr_wafer -> depth + w_thickness;
			}
			else {
			    d = i * (double)w_thickness / (w_layers - 1);
			    w_depth2 = curr_wafer -> depth + (int)(d + 0.5);
			}

			if (i > 1 || prev_sw == $4) {
			    msk = strsave (mprintf ("w%d_%d", waf_cnt, i));
			    (void) robdd.defineSymbol (msk, $4);
			    (void) robdd.defineSymbol (mprintf ("-%s", msk), e1);
			}

			// Is there already a conductor on currect wafer depth?
			p = 0;
			w = restype == P_TYPE ? w_list_p : w_list_n;
			while (w) {
			    if (w -> depth >= w_depth) { // stop
				if (w -> depth == w_depth) {
				    if (i < w_layers) w -> begin = 1;
				    mskno = w -> mskno;
				}
				break;
			    }
			    w = (p = w) -> next;
			}

			if (mskno == -1) { // mskno not found
			    struct w_list *n;
			    ALLOC (n, 1, struct w_list);
			    if (i < w_layers) n -> begin = 1;
			    n -> depth = w_depth;
			    n -> next = w;
			    if (p) p -> next = n;
			    else if (restype == P_TYPE) w_list_p = n;
			    else w_list_n = n;

			    ++fem_cnt;
			    n -> mskno = mskno = makeNewMask (cref, msk, mskno);
			}
			else {
			    struct wmlist *n;
			    ALLOC (n, 1, struct wmlist);
			    n -> name = msk;
			    n -> next = begin_wmlist;
			    n -> mskno = makeNewMask (cref, msk, mskno);
			    begin_wmlist = n;
			}

			d = baseSheetRes;
			if (i == 1 || i == w_layers) d *= 2;
			if (i == 1 && prev_sw == $4 && curr_res -> mask == mskno) {
			    double tn = (w_depth - w_depth2) * 1e-9;
			    d = 1 / d + 1 / curr_res -> val;
			    curr_res -> val = 1 / d;
			    curr_res -> thickness = tn;
			    if (other_res)
			    do {
				ASSERT (other_res -> id == curr_res -> id);
				other_res -> val = curr_res -> val;
				other_res -> thickness = tn;
			    } while (--other_res != curr_res);
			}
			else {
			    double tn = (w_depth - w_depth2) * 1e-9;
			    makeConductor (cref, msk, mskno, d, tn);
			}

			if (i == 1) prev_mskno = mskno;
			else if (i == w_layers) {
			    if (viamask < 0) {
				makeContact (cref, cvia = 0, k++, prev_mskno, mskno, baseContactRes);
			    }
			    else {
				cvia = &viaCondRef;
				cvia -> layC = via1;
				makeContact (cref, cvia, k++, prev_mskno, mskno, baseContactRes);
				cvia -> layC = via0;
				makeContact (cref, cvia, k++, prev_mskno, mskno, 0.0);
			    }

			    if (subconn && w_depth == bem_depth) {
				if (!bem_type) bem_type = restype;
				if (restype == bem_type)
				    makeContact (cref, cvia = 0, k++, mskno, -4, 0.0);
				else
				    warningMes ("cannot connect different wafer profile types", "to bem");
			    }
			    prev_mskno = mskno;
			}

			if (w_depth >= bem_depth) set_profile_subcont ();

			w_depth = w_depth2;
		    }

		    prev_sw = $4;
                    doingNewmask = 0;
		}
		;
w_options	: /* empty */
		| COLON w_opt_list
		;
w_opt_list	: /* empty */
		| w_opt_list w_option
		;
w_option	: VIAMASK EQUAL mask { viamask = $3; }
		| SUBCONN EQUAL name { subconn = yesno ($3); }
		| RESTYPE EQUAL name { restype = polarityno ($3); }
		;

conduc_descr	: /* empty */
		| conduc_descr conduc_group
		;

conduc_group    : CONDUCTORS name COLON
		{
		    strcpy (ressort, $2);
		}
		  conduc_list
		| CONDUCTORS COLON
		{
		    strcpy (ressort, "res");
		}
		  conduc_list
		;

conduc_list	: /* empty */
		| conduc_list conduc
		;

conduc		: name COLON
		{
		    curr_res = newres ();
		    curr_res -> name = checkElemName ($1);
		    curr_res -> id = name_cnt - 1;
		    curr_res -> sort = strsave (ressort);
		}
		  min_terms COLON mask COLON cond_type
		{
		    handle_min_terms (curr_res -> name, $4);
		    curr_res -> mask = $6;
		    checkPinCond (curr_res -> mask, SURFACE, $4, 5);

                    if (minTermCnt > 0) {
			curr_res -> cond = condStack[--minTermCnt];
			while (minTermCnt > 0) {
			    other_res = newres ();
			    other_res -> cond = condStack[--minTermCnt];
			    other_res -> name = curr_res -> name;
			    other_res -> id   = curr_res -> id;
			    other_res -> sort = curr_res -> sort;
			    other_res -> mask = curr_res -> mask;
			    other_res -> val  = curr_res -> val;
			    other_res -> type = curr_res -> type;
			}
		    }
		    else curr_res -> cond = NULL;
		}
		;

cond_type	: val
		{
		   if (specific_res)
		       curr_res -> val = s_res_unit * $1;
		   else
		       curr_res -> val = res_unit * $1;
		   curr_res -> type = M_TYPE; /* default */
		}
		| val COLON name
		{
		   if (specific_res)
		       curr_res -> val = s_res_unit * $1;
		   else
		       curr_res -> val = res_unit * $1;
		   curr_res -> type = polarityno ($3);
		}
		;

jun_descr	: /* empty */
		| jun_group
		;

jun_group	: JUNCTIONS COLON jun_list
		;

jun_list	: /* empty */
		| jun_list jun
		;

jun		: name COLON
		{
		   curr_jun = newjun ();
		   curr_jun -> name = checkElemName ($1);
		   curr_jun -> id = name_cnt - 1;
		}
		  min_terms COLON jun_con COLON jun_depth COLON jun_pars
		{
		   handle_min_terms (curr_jun -> name, $4);
                   if (minTermCnt > 0) {
		       curr_jun -> cond = condStack[--minTermCnt];
		       while (minTermCnt > 0) {
			 other_jun = newjun ();
			 other_jun -> cond = condStack[--minTermCnt];
			 other_jun -> name = curr_jun -> name;
			 other_jun -> id  = curr_jun -> id;
			 other_jun -> vbr = curr_jun -> vbr;
			 other_jun -> cap = curr_jun -> cap;
			 other_jun -> pins[AN] = curr_jun -> pins[AN];
			 other_jun -> pins[CA] = curr_jun -> pins[CA];
		       }
		   }
		   else
		       curr_jun -> cond = NULL;
		}
		;

jun_con		: layer
		{
		   curr_jun -> pins[CA] = $1;
		   ALLOC (curr_jun -> pins[AN], 1, struct layer);
		   curr_jun -> pins[AN] -> mask = -1;
		   curr_jun -> pins[AN] -> occurrence = SURFACE;
		}
		| layer layer
		{
		   curr_jun -> pins[CA] = $1;
		   curr_jun -> pins[AN] = $2;
		}
		;

jun_depth	: /* empty */
		{
		   curr_jun -> dep = 0;
		}
		| val
		{
		   curr_jun -> dep = depth_unit * $1;
		}
		;

jun_pars	: /* empty */
		{
		   curr_jun -> cap = 0;
		   curr_jun -> vbr = 0;
		}
		| val
		{
		   if (curr_jun -> pins[CA] -> occurrence == SURFACE
		       && curr_jun -> pins[AN] -> occurrence == SURFACE)
		       curr_jun -> cap = a_cap_unit * $1;
		   else
		       curr_jun -> cap = e_cap_unit * $1;
		   curr_jun -> vbr = 0;
		}
		| val val
		{
		   if (curr_jun -> pins[CA] -> occurrence == SURFACE
		       && curr_jun -> pins[AN] -> occurrence == SURFACE)
		       curr_jun -> cap = a_cap_unit * $1;
		   else
		       curr_jun -> cap = e_cap_unit * $1;
		   curr_jun -> vbr = $2;
		}
		;

tor_descr	: /* empty */
		| tor_group
		;

tor_group 	: TRANSISTORS COLON tor_list
		| FETS COLON tor_list
		;

tor_list	: /* empty */
		| tor_list tor
		;

tor		: name COLON
	        {
		    curr_tor = newtor ();
		    if (strlen ($1) > MAX_TORNAME) {
			fprintf (stderr, "warning: too long tor name (truncated to %d chars)\n", MAX_TORNAME);
			$1[MAX_TORNAME] = 0;
		    }
		    curr_tor -> name = checkElemName ($1);
		    curr_tor -> id = name_cnt - 1;
		    curr_tor -> dscap = 0;	/* FALSE */
		}
		  min_terms COLON mask mask
		{
		    handle_min_terms (curr_tor -> name, $4);
		    curr_tor -> g = $6;
		    curr_tor -> ds = $7;

                    end_tor = curr_tor;
                    if (minTermCnt > 0) {
			curr_tor -> cond = condStack[--minTermCnt];
			while (minTermCnt > 0) {
			    other_tor = newtor ();
			    other_tor -> cond = condStack[--minTermCnt];
			    other_tor -> name = curr_tor -> name;
			    other_tor -> id = curr_tor -> id;
			    other_tor -> g = curr_tor -> g;
			    other_tor -> ds = curr_tor -> ds;
			    other_tor -> b = curr_tor -> b;
			    other_tor -> dscap = curr_tor -> dscap;
			}
                        end_tor = other_tor;
		    }
                    else
			curr_tor -> cond = NULL;
		}
                  src_mask dscap_spec dscap_spec_s bulkmask
                {
		    curr_tor -> b = $12;
		    for (other_tor = curr_tor + 1;
                         other_tor <= end_tor; other_tor++) {
                        other_tor -> b = curr_tor -> b;
                        other_tor -> dscap = curr_tor -> dscap;
                    }
		}
		;

src_mask	: /* empty */
		{
		    curr_tor -> s = curr_tor -> ds;
		}
		| SLASH mask
		{
		    curr_tor -> s = $2;
		}
		;
dscap_spec	: /* empty */
		| LPS
		{
		    /* Since min_terms will use the last name from the name
		       table, we make the name now already.
		    */
		    cap_name = checkElemName (mprintf ("%c%s%s-a", DSCAP_PREFIX, curr_tor -> name, DSCAP_SUFFIX));
		}
                  min_terms RPS
		{
		    setDsCap ($3, 0);
		}
		;
dscap_spec_s	: /* empty */
		| LPS
		{
		    cap_name = checkElemName (mprintf ("%c%s%s-b", DSCAP_PREFIX, curr_tor -> name, DSCAP_SUFFIX));
		}
                  min_terms RPS
		{
		    setDsCap ($3, 1);
		}
		;

bulkmask	: COLON mask_s
		{
		    if ($2 == -1) {
			yyerror ("Incorrect bulkmask '@gnd'"); /* POSSIBLE_FUTURE_IMPLEMENTATION */
			$$ = -3; /* because -1 is used for no bulk connection */
		    }
		    else $$ = $2;
		}
		| COLON perc_subcont
		{
		    $$ = -4;
		}
		| /* empty */
		{
		    $$ = -1;
		}
		;

bjt_descr	: /* empty */
		| bjt_group
		;

bjt_group	: BIPOTORS COLON bjt_list
		| BJTS COLON bjt_list
		;

bjt_list	: /* empty */
		| bjt_list bjt
		;

bjt		: name COLON
		{
		   curr_bjt = newbjt ();
		    if (strlen ($1) > MAX_TORNAME) {
			fprintf (stderr, "warning: too long bjt name (truncated to %d chars)\n", MAX_TORNAME);
			$1[MAX_TORNAME] = 0;
		    }
		   curr_bjt -> name = checkElemName ($1);
		   curr_bjt -> id = name_cnt - 1;
		   doingBjt = 1;
		}
		  min_terms COLON bjt_type COLON layer layer layer substrterm
		{
		   if ($11 -> mask == -2)
			fatalErr ("Incorrect pin layer @sub for", curr_bjt -> name);

		   if (curr_bjt -> type == VBJTELEM) {
			if ($8 -> occurrence != SURFACE ||
			    $9 -> occurrence != SURFACE ||
			   $10 -> occurrence != SURFACE ||
			   $11 -> occurrence != SURFACE)
			fatalErr ("Incorrect pin layer specification for vertical bjt",
				curr_bjt -> name);
		     if (doingBjt > 1) {
			fprintf (stderr, "warning: found edge condition ");
			fprintf (stderr, "for vertical bjt %s\n", curr_bjt -> name);
		     }
		   }
		   else {
		     if (doingBjt <= 1) {
			fprintf (stderr, "warning: found no edge condition ");
			fprintf (stderr, "for lateral bjt %s\n", curr_bjt -> name);
		     }
		   }
		   doingBjt = 0;
		   curr_bjt -> pins[EM] = $8;
		   curr_bjt -> pins[BA] = $9;
		   curr_bjt -> pins[CO] = $10;
		   curr_bjt -> pins[SU] = $11;

		   handle_min_terms (curr_bjt -> name, $4);
                   if (minTermCnt > 0) {
		       curr_bjt -> cond = condStack[--minTermCnt];
		       while (minTermCnt > 0) {
			 other_bjt = newbjt ();
			 other_bjt -> cond = condStack[--minTermCnt];
			 other_bjt -> name = curr_bjt -> name;
			 other_bjt -> id   = curr_bjt -> id;
			 other_bjt -> type = curr_bjt -> type;
			 other_bjt -> pins[EM] = curr_bjt -> pins[EM];
			 other_bjt -> pins[BA] = curr_bjt -> pins[BA];
			 other_bjt -> pins[CO] = curr_bjt -> pins[CO];
			 other_bjt -> pins[SU] = curr_bjt -> pins[SU];
		       }
		    }
		    else
		       curr_bjt -> cond = NULL;
		}
		;

bjt_type	: name
		{
		   curr_bjt -> type = bjtno ($1);
		}
		;

substrterm	: /* empty */
		{
                    ALLOC ($$, 1, struct layer);
                    $$ -> mask = -1;		/* -1 is used for no bulk */
                    $$ -> occurrence = SURFACE;
		}
		| COLON layer
		{
		    $$ = $2;
		}
		| COLON perc_subcont
		{
                    ALLOC ($$, 1, struct layer);
                    $$ -> mask = -4;
                    $$ -> occurrence = SURFACE;
		}
                /* The possibility to use @sub for the bulk connection
                   has not been implemented.  This is because, compared
                   to other elements, it is less clear for bipolar
                   transistors where the substrate contact area is.
                */
		| COLON SUB
		{
                    ALLOC ($$, 1, struct layer);
                    $$ -> mask = -2;
                    $$ -> occurrence = SURFACE;
		}
		;

cnt_descr	: /* empty */
		| cnt_group
		;

cnt_group	: CONNECTS COLON cnt_list
		;

cnt_list	: /* empty */
		| cnt_list cnt
		;

cnt		: name COLON
		{
		   curr_cnt = newcnt ();
		   curr_cnt -> name = checkElemName ($1);
		   curr_cnt -> id = name_cnt - 1;
		   doingConnect = 1;
		}
		  min_terms COLON layer_s layer_s
		{
		   if ( $6 -> occurrence == OTHEREDGE ||
			$7 -> occurrence == OTHEREDGE ||
			($6 -> occurrence == EDGE && $7 -> occurrence == EDGE) ||
			$6 -> mask < 0 ||
			$7 -> mask < 0 || $6 -> mask == $7 -> mask)
			fatalErr ("Incorrect pin layer specification for", curr_cnt -> name);

		   handle_min_terms (curr_cnt -> name, $4);
		   curr_cnt -> cons[0] = $6;
		   curr_cnt -> cons[1] = $7;

		   if (doingConnect > 1 &&
			$6 -> occurrence == SURFACE &&
			$7 -> occurrence == SURFACE) {
			fprintf (stderr, "warning: found edge condition for connect '%s'\n", curr_cnt -> name);
		   }
		   else {
		       checkPinCond ($6 -> mask, $6 -> occurrence, $4, 1);
		       checkPinCond ($7 -> mask, $7 -> occurrence, $4, 1);
		   }
		   doingConnect = 0;

                   if (minTermCnt > 0) {
		       curr_cnt -> cond = condStack[--minTermCnt];
		       while (minTermCnt > 0) {
			 other_cnt = newcnt ();
			 other_cnt -> cond = condStack[--minTermCnt];
			 other_cnt -> name = curr_cnt -> name;
			 other_cnt -> id = curr_cnt -> id;
			 other_cnt -> cons[0] = curr_cnt -> cons[0];
			 other_cnt -> cons[1] = curr_cnt -> cons[1];
		       }
		   }
		   else curr_cnt -> cond = NULL;
		}
		;

contact_descr	: /* empty */
		| contact_descr contact_group
		;

contact_group	: CONTACTS name COLON
		{
		    strcpy (consort, $2);
		}
		  contact_list
		| CONTACTS COLON
		{
		    strcpy (consort, "res");
		}
		  contact_list
		;

contact_list	: /* empty */
		| contact_list contact
		;

contact		: name COLON
		{
		    curr_con = newcon ();
		    curr_con -> name = checkElemName ($1);
		    curr_con -> id = name_cnt - 1;
		    curr_con -> sort = strsave (consort);
		}
		  min_terms COLON contact_mask_s contact_mask_s COLON val
		{
		    int i, ccmsk;

		    if (($6 < 0 && $7 < 0) || $6 == -1 || $7 == -1 || $6 == $7)
			fatalErr ("Incorrect pin layer specification for", curr_con -> name);

		         if ($6 == -4) ccmsk = $7; /* >= 0 */
		    else if ($7 == -4) ccmsk = $6; /* >= 0 */
		    else ccmsk = -1;

		    if (ccmsk >= 0 && setsubcontccnr (ccmsk, curr_con -> id, 0))
			fatalErr ("subcont not found for contact element", curr_con -> name);

		    handle_min_terms (curr_con -> name, $4);

		    curr_con -> mask1 = $6;
		    curr_con -> mask2 = $7;
		    if (specific_res)
			curr_con -> val = s_res_unit * $9;
		    else
			curr_con -> val = c_res_unit * $9;

		    i  = checkPinCond (curr_con -> mask1, SURFACE, $4, 4);
		    i += checkPinCond (curr_con -> mask2, SURFACE, $4, 4);

                    if (minTermCnt > 0) {
			curr_con -> cond = condStack[--minTermCnt];
			while (minTermCnt > 0) {
			    other_con = newcon ();
			    other_con -> cond = condStack[--minTermCnt];
			    other_con -> name = curr_con -> name;
			    other_con -> id   = curr_con -> id;
			    other_con -> sort = curr_con -> sort;
			    other_con -> mask1 = curr_con -> mask1;
			    other_con -> mask2 = curr_con -> mask2;
			    other_con -> val = curr_con -> val;
			}
		    }
		    else curr_con -> cond = NULL;

		    if (!i) { /* check for present via mask */
			struct layCondRef *c;
			for (c = curr_con -> cond; c; c = c -> next) {
			    i = c -> layC -> lay -> mask;
			    if (i != curr_con -> mask1 &&
				i != curr_con -> mask2 &&
				c -> layC -> present) break;
			}
			if (!c) {
			    fprintf (stderr, "warning: via mask not found in condition ");
			    fprintf (stderr, "for contact element %s\n", curr_con -> name);
			}
		    }
		}
		;

cap_descr	: /* empty */
		| cap_descr cap_group
		;

cap_group	: cap_type CAPACITANCES name COLON
		{
		    strcpy (capsort, $3);
		}
		  cap_list
		| cap_type CAPACITANCES COLON
		{
		    strcpy (capsort, "cap");
		}
		  cap_list
		;

cap_type        : JUNCTION
		{
		    junction_cap = 1;
		}
		| /* empty */
		{
		    junction_cap = 0;
		}
		;

cap_list	: /* empty */
		| cap_list cap
		;

cap		: name COLON
		{
		    cap_name = checkElemName ($1);
		    capCurrent = name_cnt; /* id */

		    curr_nr_x_y_vals = 0;
		    curr_x_y_vals = NULL;
		    doingCap = 1;
		}
		  min_terms COLON layer_s layer_s2 COLON cap_val
		{
		    struct capacitance *help_cap;
		    struct capacitance *stop_cap;
		    struct layCondRef *cond_new;
		    struct layCondRef *cond_end;
		    struct layCondRef *cond_el;
		    struct layer *pLay, *nLay;
		    int ccmsk;

		    handle_min_terms (cap_name, $4);

		    curr_cap = newcap ();
		    curr_cap -> name = cap_name;
		    curr_cap -> id = capCurrent - 1;
		    curr_cap -> sort = strsave (capsort);
		    curr_cap -> junc = junction_cap;

		    pLay = $6;
		    nLay = $7;

		    if (!nLay) {
			ALLOC (nLay, 1, struct layer);
			nLay -> mask = -1; /* GND */
			nLay -> occurrence = SURFACE;
		    }

		    if ((pLay -> occurrence == OTHEREDGE && nLay -> occurrence == OTHEREDGE)
			|| (pLay -> mask < 0 && nLay -> mask < 0))
			fatalErr ("Incorrect pin layer specification for", cap_name);

		    if ((pLay -> occurrence == OTHEREDGE && nLay -> occurrence == SURFACE) ||
			(pLay -> occurrence == SURFACE && nLay -> occurrence == OTHEREDGE))
			fatalErr ("Incorrect pin layer specification for", cap_name);

		    if (pLay -> mask == nLay -> mask &&
			pLay -> occurrence != OTHEREDGE && nLay -> occurrence != OTHEREDGE)
			fatalErr ("Incorrect pin layer specification for", cap_name);

                    /* SG: Check polarity of the pins
		     */
		    if (junction_cap) {
			int p = -1;
			int n = -1;
			if (minTermCnt <= 0) {
			    char *s = mprintf ("junction_cap %s:", cap_name);
			    fatalErr (s, "has empty condition list\n");
			}
			if (pLay -> mask >= 0) {
			    p = checkPinPol (cap_name, pLay -> mask, pLay -> occurrence, 1);
			    if (p >= 0) p = ress[p].type;
			}
			if (nLay -> mask >= 0) {
			    n = checkPinPol (cap_name, nLay -> mask, nLay -> occurrence, 2);
			    if (n >= 0) {
				n = ress[n].type;
				if (p == n) {
				    char *s = mprintf ("both pins of element '%s'", cap_name);
				    fatalErr (s, "conductors have same polarity\n");
				}
			    }
			}

			/* EM: Check for double definition of d/s capacitances */
			if (p > 0) checkTransistorDSMask (cap_name, pLay -> mask, p);
			if (n > 0) checkTransistorDSMask (cap_name, nLay -> mask, n);

			if ((p == N_TYPE && n != p) || (n == P_TYPE && p != n)) {
			    char *s = mprintf ("swapping pins of element '%s'", cap_name);
			    warningMes (s, "because of conductor polarity");
			    Swap (struct layer *, pLay, nLay);
			}
		    }

		    curr_cap -> pLay = pLay;
		    curr_cap -> nLay = nLay;

		    if (nLay -> mask == -4)
			ccmsk = pLay -> mask; /* >= 0 */
		    else if (pLay -> mask == -4)
			ccmsk = nLay -> mask; /* >= 0 */
		    else ccmsk = -1;

		    if (ccmsk >= 0 && setsubcontccnr (ccmsk, curr_cap -> id, 1))
			fatalErr ("subcont not found for cap element", cap_name);

		    curr_subcont = NULL;

		    if (nLay -> occurrence == OTHEREDGE || pLay -> occurrence == OTHEREDGE) {
			curr_cap -> eltype = LATCAPELEM;
			if (curr_cap -> junc) {
			    fatalErr ("no lateral capacitance allowed for junction capacitance element", cap_name);
			}
			if (curr_nr_x_y_vals == 0)
			    curr_cap -> val = cap_unit * $9 -> y;
			else if (curr_nr_x_y_vals == 1) {
			    fprintf (stderr, "warning: only one (dist,cap)-pair, ");
			    fprintf (stderr, "using a value for cap element %s\n", curr_cap -> name);
			    curr_cap -> val = $9 -> x * $9 -> y;
			    curr_nr_x_y_vals = 0;
			}
			else {
			    curr_cap -> val = 0.0;
			    curr_cap -> x_y_vals = $9;
			}
		    }
		    else if (pLay -> occurrence == EDGE || nLay -> occurrence == EDGE) {
			curr_cap -> eltype = EDGECAPELEM;
			if (curr_nr_x_y_vals == 0)
			    curr_cap -> val = e_cap_unit * $9 -> y;
                        else {
			    curr_cap -> val = 0.0;
			    curr_cap -> x_y_vals = $9;
			    if (curr_nr_x_y_vals == 1) {
				fprintf (stderr, "warning: only one (dist,cap)-pair, ");
				fprintf (stderr, "no compensation for cap element %s\n", curr_cap -> name);
			    }
			}

			if (nLay -> mask == -4 || pLay -> mask == -4) {
			    fprintf (stderr, "warning: don't use '%%()' notation for ");
			    fprintf (stderr, "edge cap element %s\n", curr_cap -> name);
			}
		    }
		    else { /* SURFACE CAP */
			curr_cap -> eltype = SURFCAPELEM;
			if (curr_nr_x_y_vals > 0)
			    fatalErr ("multiple values for area cap", curr_cap -> name);
			curr_cap -> val = a_cap_unit * $9 -> y;

			/* Add substrate contact only for surface caps!
			*/
			if (nLay -> mask == -2 || pLay -> mask == -2) {
			    curr_subcont = newsubcont ();
			    curr_subcont -> name = checkElemName (mprintf ("%csubcont%d", SUBCONT_PREFIX, sbc_cnt - 1));
			    curr_subcont -> id = name_cnt - 1;
			    if (nLay -> mask == -2) {
				nLay -> mask = -4;
				curr_subcont -> ccnr = pLay -> mask;
			    }
			    else {
				pLay -> mask = -4;
				curr_subcont -> ccnr = nLay -> mask;
			    }
			    curr_subcont -> captype = 1;
			}
		    }

		    if (curr_nr_x_y_vals == 0 && curr_cap -> val <= 0) {
			curr_cap -> val = 0;
			fprintf (stderr, "warning: capacitance value is zero for ");
			fprintf (stderr, "cap element %s\n", curr_cap -> name);
		    }

                    if (minTermCnt > 0) {
			curr_cap -> cond = condStack[--minTermCnt];
			other_cap = curr_cap;
			while (minTermCnt > 0) {
			    other_cap = newcap ();
			    other_cap -> cond = condStack[--minTermCnt];
			    other_cap -> name = curr_cap -> name;

			if (curr_subcont) {
			    other_subcont = newsubcont ();
			    other_subcont -> cond = add_cond (other_cap -> cond);
			    other_subcont -> name = curr_subcont -> name;
			    other_subcont -> id   = curr_subcont -> id;
			    other_subcont -> ccnr = curr_subcont -> ccnr;
			    other_subcont -> captype = 1;
			}
			    other_cap -> id   = curr_cap -> id;
			    other_cap -> sort = curr_cap -> sort;
			    other_cap -> junc = curr_cap -> junc;
			    other_cap -> eltype = curr_cap -> eltype;
			    other_cap -> pLay = curr_cap -> pLay;
			    other_cap -> nLay = curr_cap -> nLay;
			    other_cap -> val  = curr_cap -> val;
			    other_cap -> x_y_vals = curr_cap -> x_y_vals;
			}
		    }
		    else curr_cap -> cond = NULL;

		    if (curr_subcont)
			curr_subcont -> cond = add_cond (curr_cap -> cond);

		    /* Test for incorrect cap pin conditions */

		    if (doingCap > 1 && curr_cap -> eltype == SURFCAPELEM) {
			fprintf (stderr, "warning: found edge condition for ");
			fprintf (stderr, "surface cap element %s\n", curr_cap -> name);
		    }
		    else {
			checkPinCond (pLay -> mask, pLay -> occurrence, $4, 0);
			checkPinCond (nLay -> mask, nLay -> occurrence, $4, 0);
		    }
		    doingCap = 0;

		    if (curr_cap -> eltype == LATCAPELEM && pLay -> mask != nLay -> mask) {

			/* For lateral coup. capacitances between different
			   masks we also have to add the capacitance with
			   the occurrences swapped */

			help_cap = curr_cap;
			stop_cap = other_cap;
			while (help_cap <= stop_cap) {

			    other_cap = newcap ();
			    other_cap -> eltype = curr_cap -> eltype;

			    other_cap -> cond = cond_end = NULL;
			    cond_el = help_cap -> cond;
			    while (cond_el) {
				ALLOC (cond_new, 1, struct layCondRef);
				cond_new -> next = NULL;
				ALLOC (cond_new -> layC, 1, struct layerCond);
				cond_new -> layC -> present = cond_el -> layC -> present;
				ALLOC (cond_new -> layC -> lay, 1, struct layer);
				if (cond_el -> layC -> lay -> occurrence == EDGE)
				    cond_new -> layC -> lay -> occurrence = OTHEREDGE;
				else if (cond_el -> layC -> lay -> occurrence == OTHEREDGE)
				    cond_new -> layC -> lay -> occurrence = EDGE;
				else
				    cond_new -> layC -> lay -> occurrence =
					 cond_el -> layC -> lay -> occurrence;
				cond_new -> layC -> lay -> mask = cond_el -> layC -> lay -> mask;
				if (cond_end)
				    cond_end -> next = cond_new;
				else
				    other_cap -> cond = cond_new;
				cond_end = cond_new;
				cond_el = cond_el -> next;
			    }
			    other_cap -> name = help_cap -> name;
			    other_cap -> id   = help_cap -> id;
			    other_cap -> sort = help_cap -> sort;
			    other_cap -> junc = help_cap -> junc;
			    ALLOC (other_cap -> pLay, 1, struct layer);
			    ALLOC (other_cap -> nLay, 1, struct layer);
			    other_cap -> pLay -> occurrence = help_cap -> pLay -> occurrence;
			    other_cap -> nLay -> occurrence = help_cap -> nLay -> occurrence;
			    other_cap -> pLay -> mask = help_cap -> nLay -> mask;
			    other_cap -> nLay -> mask = help_cap -> pLay -> mask;
			    other_cap -> val = help_cap -> val;
			    other_cap -> x_y_vals = help_cap -> x_y_vals;
			    help_cap++;
			}
		    }

		    capCurrent = 0;
		}
		;

layer_s2	: /* empty */ { $$ = 0; }
		| layer_s
		;

cap_val		: val
		{
		    curr_x_y_vals = &an_x_y;
		    an_x_y.y = $1;
		    $$ = curr_x_y_vals;
		}
		| cap_pair_vals
		{
		    $$ = curr_x_y_vals;
		}
		;

cap_pair_vals	: cap_pair
		| cap_pair_vals cap_pair
		;

cap_pair	: val val
		{
		    struct x_y *new_x_y;

		    ALLOC (new_x_y, 1, struct x_y);
		    new_x_y -> x = x_cap_unit * $1;
		    new_x_y -> y = e_cap_unit * $2;
		    new_x_y -> next = NULL;

		    if (curr_x_y_vals == NULL) {
			curr_x_y_vals = new_x_y;
			last_x_y_vals = new_x_y;
		    }
		    else {
			last_x_y_vals -> next = new_x_y;
			last_x_y_vals = new_x_y;
		    }
		    curr_nr_x_y_vals++;
		}
		;

vdim_descr	: /* empty */
		| VDIMENSIONS COLON vdim_list vdim_options
		;

vdim_list	: /* empty */
		| vdim_list vdim
		;

vdim		: name COLON
		{
		    curr_vdim = newvdim ();
		    curr_vdim -> name = checkElemName ($1);
		    curr_vdim -> id = name_cnt - 1;
		}
		  min_terms COLON mask COLON val val vdim_spacing
		{
		    curr_vdim -> mask = $6;
		    curr_vdim -> height = vdim_unit * $8;
		    curr_vdim -> thickness = vdim_unit * $9;
		    if ($10 > 0)
			curr_vdim -> spacing = vdim_unit * $10;
		    else
			curr_vdim -> spacing = curr_vdim -> thickness / 2;

		    if (curr_vdim -> height >= 0.01) {
			char *s = mprintf ("%f meter", curr_vdim -> height);
			warningMes ("large value for conductor height:", s);
		    }
		    if (curr_vdim -> thickness >= 0.01) {
			char *s = mprintf ("%f meter", curr_vdim -> thickness);
			warningMes ("large value for conductor thickness:", s);
		    }

		    handle_min_terms (curr_vdim -> name, $4);
                    if (minTermCnt > 0) {
			curr_vdim -> cond = condStack[--minTermCnt];
			while (minTermCnt > 0) {
			    other_vdim = newvdim ();
			    other_vdim -> cond = condStack[--minTermCnt];
			    other_vdim -> name = curr_vdim -> name;
			    other_vdim -> id   = curr_vdim -> id;
			    other_vdim -> mask = curr_vdim -> mask;
			    other_vdim -> height = curr_vdim -> height;
			    other_vdim -> thickness = curr_vdim -> thickness;
			}
		    }
		    else curr_vdim -> cond = NULL;
		}
		;

vdim_spacing	: /* empty */ { $$ = 0; }
		| val
		;

vdim_options	: /* empty */
		| vdim_options vdim_option
		;
vdim_option	: OMIT_CAP3D COLON name name vdim_cap2d_val
		{
		    STDSDWord e1, e2, e3;
		    MINSymbol *symbol;
		    int i, n1 = -1, n2 = -1;

		    for (i = 0; i < vdm_cnt; ++i) {
			if (n1 < 0 && strsame (vdms[i].name, $3)) n1 = i;
			if (n2 < 0 && strsame (vdms[i].name, $4)) n2 = i;
		    }
		    if (n1 < 0) fatalErr ("Invalid vdim name specified for `omit_cap3d':", $3);
		    if (n2 < 0) fatalErr ("Invalid vdim name specified for `omit_cap3d':", $4);
		    if (n1 == n2) fatalErr ("Two the same vdim names specified for `omit_cap3d'!", NULL);
		    if (vdms[n1].mask == vdms[n2].mask) fatalErr ("Two the same vdim masks specified for `omit_cap3d'!", NULL);

		    if (vdms[n1].height > vdms[n2].height) { i = n1; n1 = n2; n2 = i; } // swap

		    curr_cap = newcap ();
		    curr_cap -> name = checkElemName (mprintf ("$%s$%s", vdms[n1].name, vdms[n2].name));
		    curr_cap -> id = name_cnt - 1;
		    curr_cap -> sort = (char*)"cap";
		    curr_cap -> eltype = SURFCAPELEM;

		    if ($5 < 0) { /* calculate_2d */
			curr_cap -> z1 = vdms[n1].height + vdms[n1].thickness;
			curr_cap -> z2 = vdms[n2].height;
			if (curr_cap -> z2 <= curr_cap -> z1) fatalErr ("Invalid vdim heights for `omit_cap3d'!", NULL);
			curr_cap -> cap3d = 2;
		    }
		    else {
			curr_cap -> cap3d = 1;
			curr_cap -> val = a_cap_unit * $5;
		    }

		    ALLOC (curr_cap -> pLay, 1, struct layer);
		    curr_cap -> pLay -> mask = vdms[n2].mask;
		    curr_cap -> pLay -> occurrence = SURFACE;

		    ALLOC (curr_cap -> nLay, 1, struct layer);
		    curr_cap -> nLay -> mask = vdms[n1].mask;
		    curr_cap -> nLay -> occurrence = SURFACE;

		    symbol = robdd.findSymbol (vdms[n1].name);
		    ASSERT (symbol);
		    e1 = symbol->getDefinition();
		    symbol = robdd.findSymbol (vdms[n2].name);
		    ASSERT (symbol);
		    e2 = symbol->getDefinition();
		    e3 = ROBDD_AND (e1, e2);

		    handle_min_terms (curr_cap -> name, e3);
		    if (minTermCnt > 0) {
			curr_cap -> cond = condStack[--minTermCnt];
			while (minTermCnt > 0) {
			    other_cap = newcap ();
			    other_cap -> cond = condStack[--minTermCnt];
			    other_cap -> name = curr_cap -> name;
			    other_cap -> id   = curr_cap -> id;
			    other_cap -> sort = curr_cap -> sort;
			    other_cap -> eltype = curr_cap -> eltype;
			    other_cap -> cap3d = curr_cap -> cap3d;
			    other_cap -> pLay = curr_cap -> pLay;
			    other_cap -> nLay = curr_cap -> nLay;
			    other_cap -> val = curr_cap -> val;
			}
		    }
		    else curr_cap -> cond = NULL;
		}
		| KEEP_CAP2D COLON name
		{
		    int i;
		    for (i = 0; i < cap_cnt; ++i) {
			if (strsame (caps[i].name, $3)) break;
		    }
		    if (i >= cap_cnt) fatalErr ("Invalid cap name specified for `keep_cap2d':", $3);
		    for (; i < cap_cnt; ++i) {
			if (!strsame (caps[i].name, $3)) break;
			caps[i].cap3d = 1;
		    }
		}
		;
vdim_cap2d_val	: /* empty */
		{
		    $$ = -1;
		}
		| COLON val
		{
		    $$ = $2;
		}
		;

eshape_descr	: /* empty */
		| ESHAPES
		{
		    shapetype = 'e';
		}
		  COLON shape_list
		;

cshape_descr	: /* empty */
		| CSHAPES
		{
		    shapetype = 'c';
		}
		  COLON shape_list
		;

shape_list	: /* empty */
		| shape_list shape
		;

shape		: name COLON
		{
		    curr_shape = newshape ();
		    curr_shape -> name = checkElemName ($1);
		    curr_shape -> id = name_cnt - 1;
		    doingShape = 1;
		}
		  min_terms COLON layer COLON vals_1_2_or_4
		{
		    doingShape = 0;
		    curr_shape -> mask = $6 -> mask;
		    FREE ($6);

		    if (shapetype == 'e') {
			curr_shape -> type = ESHAPEELEM;
			if (val_cnt == 1) {
			    curr_shape -> xb1 = shape_unit * retval[0];
			    curr_shape -> xt1 = curr_shape -> xb1;
			}
			else if (val_cnt == 2) {
			    curr_shape -> xb1 = shape_unit * retval[0];
			    curr_shape -> xt1 = shape_unit * retval[1];
			}
			else {
			    fatalErr (mprintf("eshape %s:", curr_shape -> name),
				"illegal number of values");
			}
			curr_shape -> xb2 = 0.0; /* random number */
			curr_shape -> xt2 = 0.0; /* random number */
		    }
		    else if (shapetype == 'c') {
			curr_shape -> type = CSHAPEELEM;
			if (val_cnt == 2) {
			    curr_shape -> xb1 = shape_unit * retval[0];
			    curr_shape -> xt1 = curr_shape -> xb1;
			    curr_shape -> xb2 = shape_unit * retval[1];
			    curr_shape -> xt2 = curr_shape -> xb2;
			}
			else if (val_cnt == 4) {
			    curr_shape -> xb1 = shape_unit * retval[0];
			    curr_shape -> xt1 = shape_unit * retval[1];
			    curr_shape -> xb2 = shape_unit * retval[2];
			    curr_shape -> xt2 = shape_unit * retval[3];
			}
			else {
			    fatalErr (mprintf("cshape %s:", curr_shape -> name),
				"illegal number of values");
			}
		    }
		    else
			fatalErr ("Invalid SHAPE-type!", NULL);

		    if (Abs (curr_shape -> xb1) >= 0.01) {
			char *s = mprintf ("%f meter", curr_shape -> xb1);
			warningMes ("large value for conductor shape:", s);
		    }
		    if (Abs (curr_shape -> xt1) >= 0.01) {
			char *s = mprintf ("%f meter", curr_shape -> xt1);
			warningMes ("large value for conductor shape:", s);
		    }
		    if (Abs (curr_shape -> xb2) >= 0.01) {
			char *s = mprintf ("%f meter", curr_shape -> xb2);
			warningMes ("large value for conductor shape:", s);
		    }
		    if (Abs (curr_shape -> xt2) >= 0.01) {
			char *s = mprintf ("%f meter", curr_shape -> xt2);
			warningMes ("large value for conductor shape:", s);
		    }

		    if (shapetype == 'e' && curr_shape -> xb1 == 0.0
					 && curr_shape -> xt1 == 0.0) {
			fprintf (stderr, "warning: skipping eshape element ");
			fprintf (stderr, "'%s' with zero values\n", curr_shape -> name);
			--shp_cnt;
		    }
		    else {
			handle_min_terms (curr_shape -> name, $4);
			checkShapeCond ($4);

			if (minTermCnt > 0) {
			    curr_shape -> cond = condStack[--minTermCnt];
			    while (minTermCnt > 0) {
				other_shape = newshape ();
				other_shape -> cond = condStack[--minTermCnt];
				other_shape -> name = curr_shape -> name;
				other_shape -> id   = name_cnt - 1;
				other_shape -> type = curr_shape -> type;
				other_shape -> mask = curr_shape -> mask;
				other_shape -> xb1 = curr_shape -> xb1;
				other_shape -> xt1 = curr_shape -> xt1;
				other_shape -> xb2 = curr_shape -> xb2;
				other_shape -> xt2 = curr_shape -> xt2;
			    }
			}
			else curr_shape -> cond = NULL;
		    }
		}
		;

unit		: UNIT RESISTANCE val
                {
		    res_unit = $3;
		}
		| UNIT C_RESISTANCE val
                {
		    c_res_unit = $3;
		}
		| UNIT S_RESISTANCE val
                {
		    s_res_unit = $3;
		}
		| UNIT CAPACITANCE val
                {
		    cap_unit = $3;
		}
		| UNIT A_CAPACITANCE val
                {
		    a_cap_unit = $3;
		}
		| UNIT E_CAPACITANCE val
                {
		    e_cap_unit = $3;
		}
		| UNIT DISTANCE val
                {
		    x_cap_unit = $3;
		}
		| UNIT VDIMENSION val
                {
		    vdim_unit = $3;
		}
		| UNIT SHAPE val
                {
		    shape_unit = $3;
		}
		| UNIT LAYERDEPTH val
		{
		    depth_unit = $3;
		}
		| UNIT RESIZEMASK val
		{
                    if (curr_resize)
                        warningMes ("specification of resize unit after a resize statement", NULL);
		    resize_unit = $3;
		}
		;

dielec_descr	: /* empty */
		| DIELECTRICS COLON dielec_list dielec_options
		{
		    double v = 0;
		    int i, id = 0;
		    for (i = 0; i < cap_cnt; ++i) {
			if (caps[i].cap3d == 2) {
			    if (id != caps[i].id) {
				id = caps[i].id;
				v = getDielConstant (caps[i].z1, caps[i].z2);
			    }
			    caps[i].val = v;
			}
		    }
		}
		;

dielec_list	: /* empty */
		| dielec_list dielec_item
		;

dielec_item	: name val val
		{
		    static int maxdiel = 0;
		    if ($2 <= 0) fatalErr (mprintf("diellayer %s:", $1), "permittivity must be > 0");

		    if (diel_cnt == 0) {
			if ($3 != 0.0)
			    fatalErr (mprintf("diellayer %s:", $1), "bottom of first dielectric must be 0");
		    }
		    else {
			if ($2 == diels[diel_cnt-1].permit)
			    warningMes (mprintf("diellayer %s:", $1), "permittivity == previous one");
			if ($3 <= diels[diel_cnt-1].bottom)
			    fatalErr (mprintf("diellayer %s:", $1), "bottom must be > previous bottom");
		    }

		    if ($3 > 120) {
			char *s = mprintf ("%.1f micron", $3);
			warningMes ("large value for diellayer bottom:", s);
		    }
		    if (diel_cnt == maxdiel) {
			maxdiel += MAXDIEL;
			REALLOC (diels, maxdiel, struct dielectric, diel_cnt);
		    }
		    diels[diel_cnt].name = checkElemName ($1);
		    diels[diel_cnt].permit = $2;
		    diels[diel_cnt].bottom = $3;
		    diel_cnt++;
		}
		;

dielec_options  : /* empty */
                | dielec_options dielec_option
                ;

dielec_option   : ZQ_VALUES COLON diel_zq_value_list
                | ZP_VALUES COLON diel_zp_value_list
                | R_VALUES COLON diel_r_value_list
                | MAX_DETERMINANT_BINNING_ERROR COLON val
                {
                    diel_max_determinant_binning_error = $3;
                    if(diel_max_determinant_binning_error < 0 || diel_max_determinant_binning_error > 1)
                        fatalErr ("Invalid value specified for `max_determinant_binning_error'.", NULL);
                }
                | MAX_ADJOINT_BINNING_ERROR COLON val
                {
                    diel_max_adjoint_binning_error = $3;
                    if(diel_max_adjoint_binning_error < 0 || diel_max_adjoint_binning_error > 1)
                        fatalErr ("Invalid value specified for `max_adjoint_binning_error'.", NULL);
                }
                | MAX_ANNEALED_INVERSE_MATRIX_BINNING_ERROR COLON val
                {
                    diel_max_annealed_inverse_matrix_binning_error = $3;
                    if(diel_max_annealed_inverse_matrix_binning_error < 0 || diel_max_annealed_inverse_matrix_binning_error > 1)
                        fatalErr ("Invalid value specified for `max_annealed_inverse_matrix_binning_error'.", NULL);
                }
                | MAX_PREPROCESSED_ANNEALING_MATRICES_BINNING_ERROR COLON val
                {
                    diel_max_preprocessed_annealing_matrices_binning_error = $3;
                    if(diel_max_preprocessed_annealing_matrices_binning_error < 0 || diel_max_preprocessed_annealing_matrices_binning_error > 1)
                        fatalErr ("Invalid value specified for `max_preprocessed_annealing_matrices_binning_error'.", NULL);
                }
                | MAX_REDUCE_ERROR COLON val
                {
                    diel_max_reduce_error = $3;
                    if(diel_max_reduce_error < 0 || diel_max_reduce_error > 1)
                        fatalErr ("Invalid value specified for `max_reduce_error'.", NULL);
                }
                | MAX_ANNEALING_ERROR COLON val
                {
                    diel_max_annealing_error = $3;
                    if(diel_max_annealing_error >= -1)
                        fatalErr ("Please specify a value between -10 and -1 for `max_annealing_error'.", NULL);
                }
                | GRID_COUNT COLON val
                {
                    diel_grid_count = int($3);
                    if(diel_grid_count < 2 || diel_grid_count > 10000)
                        fatalErr ("Invalid value specified for `grid_count': value should be between 2 and 10000.", NULL);
                }
                | NUM_ANNEALING_ITERATIONS COLON val
                {
                    diel_num_annealing_iterations = int($3);
                    if(diel_num_annealing_iterations < 1)
                        fatalErr ("Invalid value specified for `num_annealing_iterations'.", NULL);
                }
                | R_SWITCH COLON val
                {
                    diel_r_switch = $3 * 1e-6;
                    if(diel_r_switch < 0)
                        fatalErr ("Invalid value specified for `r_switch': value should be >= 0.", NULL);
                }
                ;

diel_zq_value_list: /* empty */
                | diel_zq_value_list val
                {
		    static int max_diel_zq = 0;
		    if (diel_zq_values_cnt == max_diel_zq) {
			max_diel_zq += MAX_DIEL_INTERPOLATION_POINTS;
			REALLOC (diel_zq_values, max_diel_zq, double, max_diel_zq);
		    }
                    diel_zq_values[diel_zq_values_cnt++] = $2 * 1e-6;
                }
                ;
diel_zp_value_list: /* empty */
                | diel_zp_value_list val
                {
		    static int max_diel_zp = 0;
		    if (diel_zp_values_cnt == max_diel_zp) {
			max_diel_zp += MAX_DIEL_INTERPOLATION_POINTS;
			REALLOC (diel_zp_values, max_diel_zp, double, max_diel_zp);
		    }
                    diel_zp_values[diel_zp_values_cnt++] = $2 * 1e-6;
                }
                ;
diel_r_value_list: /* empty */
                | diel_r_value_list val
                {
		    static int max_diel_r = 0;
		    if (diel_r_values_cnt == max_diel_r) {
			max_diel_r += MAX_DIEL_INTERPOLATION_POINTS;
			REALLOC (diel_r_values, max_diel_r, double, max_diel_r);
		    }
                    diel_r_values[diel_r_values_cnt++] = $2 * 1e-6;
                }
                ;

subcaplay_descr	: /* empty */
		| SUBCAPLAYERS COLON subcaplay_list
		;

subcaplay_list	: /* empty */
		| subcaplay_list subcaplay_item
		;

subcaplay_item	: name val val
		{
		    static int maxsubcap = 0;
		    if ($2 <= 0) fatalErr (mprintf("subcaplayer %s:", $1), "permittivity must be > 0");

		    if (subcap_cnt == 0) {
			char *err = 0;
			if (waf_cnt) {
			    if ($3 != -BD) err = (char*)"top of first layer != bem_depth";
			}
			else if ($3 != 0.0) err = (char*)"top of first layer must be 0";
			if (err) fatalErr (mprintf("subcaplayer %s:", $1), err);
		    }
		    else {
			if ($2 == subcaps[subcap_cnt-1].conduc)
			    warningMes (mprintf("subcaplayer %s:", $1), "permittivity == previous one");
			if ($3 >= subcaps[subcap_cnt-1].top)
			    fatalErr (mprintf("subcaplayer %s:", $1), "top must be < previous top");
		    }

		    if ($3 < -100) {
			char *s = mprintf ("%.1f micron", $3);
			warningMes ("large value for subcaplayer top:", s);
		    }
		    if (subcap_cnt == maxsubcap) {
			maxsubcap += MAXSUBSTR;
			REALLOC (subcaps, maxsubcap, struct substrate, subcap_cnt);
		    }
		    subcaps[subcap_cnt].name = checkElemName ($1);
		    subcaps[subcap_cnt].conduc = $2;
		    subcaps[subcap_cnt].top = $3;
		    if (waf_cnt) subcaps[subcap_cnt].top += BD;
		    subcap_cnt++;
		}
		;

sublay_descr	: /* empty */
		| SUBLAYERS COLON sublay_list sublay_options
		;

sublay_list	: /* empty */
		| sublay_list sublay_item
		;

sublay_item	: name val val
		{
		    static int maxsubstr = 0;
		    if ($2 <= 0) fatalErr (mprintf("sublayer %s:", $1), "conductivity must be > 0");

		    if (substr_cnt == 0) {
			char *err = 0;
			if (waf_cnt) {
			    if ($3 != -BD) err = (char*)"top of first layer != bem_depth";
			}
			else if ($3 != 0.0) err = (char*)"top of first layer must be 0";
			if (err) fatalErr (mprintf("sublayer %s:", $1), err);
		    }
		    else {
			if ($2 == substrs[substr_cnt-1].conduc)
			    warningMes (mprintf("sublayer %s:", $1), "conductivity == previous one");
			if ($3 >= substrs[substr_cnt-1].top)
			    fatalErr (mprintf("sublayer %s:", $1), "top must be < previous top");
		    }

		    if ($3 < -100) {
			char *s = mprintf ("%.1f micron", $3);
			warningMes ("large value for sublayer top:", s);
		    }
		    if (substr_cnt == maxsubstr) {
			maxsubstr += MAXSUBSTR;
			REALLOC (substrs, maxsubstr, struct substrate, substr_cnt);
		    }
		    substrs[substr_cnt].name = checkElemName ($1);
		    substrs[substr_cnt].conduc = $2;
		    substrs[substr_cnt].top = $3;
		    if (waf_cnt) substrs[substr_cnt].top += BD;
		    substr_cnt++;
		}
		;

sublay_options  : /* empty */
                | sublay_options sublay_option
                ;

sublay_option   : ZQ_VALUES COLON subs_zq_value_list
                | ZP_VALUES COLON subs_zp_value_list
                | R_VALUES COLON subs_r_value_list
                | NEUMANN_SIMULATION_RATIO COLON val
                {
                    subs_neumann_simulation_ratio = $3;
                    if (subs_neumann_simulation_ratio <= 0)
                        fatalErr ("Invalid value specified for `neumann_simulation_ratio'.", NULL);
                }
                | MAX_DETERMINANT_BINNING_ERROR COLON val
                {
                    subs_max_determinant_binning_error = $3;
                    if(subs_max_determinant_binning_error < 0 || subs_max_determinant_binning_error > 1)
                        fatalErr ("Invalid value specified for `max_determinant_binning_error'.", NULL);
                }
                | MAX_ADJOINT_BINNING_ERROR COLON val
                {
                    subs_max_adjoint_binning_error = $3;
                    if(subs_max_adjoint_binning_error < 0 || subs_max_adjoint_binning_error > 1)
                        fatalErr ("Invalid value specified for `max_adjoint_binning_error'.", NULL);
                }
                | MAX_ANNEALED_INVERSE_MATRIX_BINNING_ERROR COLON val
                {
                    subs_max_annealed_inverse_matrix_binning_error = $3;
                    if(subs_max_annealed_inverse_matrix_binning_error < 0 || subs_max_annealed_inverse_matrix_binning_error > 1)
                        fatalErr ("Invalid value specified for `max_annealed_inverse_matrix_binning_error'.", NULL);
                }
                | MAX_PREPROCESSED_ANNEALING_MATRICES_BINNING_ERROR COLON val
                {
                    subs_max_preprocessed_annealing_matrices_binning_error = $3;
                    if(subs_max_preprocessed_annealing_matrices_binning_error < 0 || subs_max_preprocessed_annealing_matrices_binning_error > 1)
                        fatalErr ("Invalid value specified for `max_preprocessed_annealing_matrices_binning_error'.", NULL);
                }
                | MAX_REDUCE_ERROR COLON val
                {
                    subs_max_reduce_error = $3;
                    if(subs_max_reduce_error < 0 || subs_max_reduce_error > 1)
                        fatalErr ("Invalid value specified for `max_reduce_error'.", NULL);
                }
                | MAX_ANNEALING_ERROR COLON val
                {
                    subs_max_annealing_error = $3;
                    if(subs_max_annealing_error >= -1)
                        fatalErr ("Please specify a value between -10 and -1 for `max_annealing_error'.", NULL);
                }
                | GRID_COUNT COLON val
                {
                    subs_grid_count = int($3);
                    if(subs_grid_count < 2 || subs_grid_count > 10000)
                        fatalErr ("Invalid value specified for `grid_count': value should be between 2 and 10000.", NULL);
                }
                | NUM_ANNEALING_ITERATIONS COLON val
                {
                    subs_num_annealing_iterations = int($3);
                    if(subs_num_annealing_iterations < 1)
                        fatalErr ("Invalid value specified for `num_annealing_iterations'.", NULL);
                }
                | R_SWITCH COLON val
                {
                    subs_r_switch = $3 * 1e-6;
                    if(subs_r_switch < 0)
                        fatalErr ("Invalid value specified for `r_switch': value should be >= 0.", NULL);
                }
                ;

subs_zq_value_list: /* empty */
		| subs_zq_value_list val
		{
		    static int max_subs_zq = 0;
		    if (subs_zq_values_cnt == max_subs_zq) {
			max_subs_zq += MAX_SUBS_INTERPOLATION_POINTS;
			REALLOC (subs_zq_values, max_subs_zq, double, max_subs_zq);
		    }
		    subs_zq_values[subs_zq_values_cnt++] = $2 * 1e-6;
		}
		;
subs_zp_value_list: /* empty */
		| subs_zp_value_list val
		{
		    static int max_subs_zp = 0;
		    if (subs_zp_values_cnt == max_subs_zp) {
			max_subs_zp += MAX_SUBS_INTERPOLATION_POINTS;
			REALLOC (subs_zp_values, max_subs_zp, double, max_subs_zp);
		    }
		    subs_zp_values[subs_zp_values_cnt++] = $2 * 1e-6;
		}
		;
subs_r_value_list: /* empty */
		| subs_r_value_list val
		{
		    static int max_subs_r = 0;
		    if (subs_r_values_cnt == max_subs_r) {
			max_subs_r += MAX_SUBS_INTERPOLATION_POINTS;
			REALLOC (subs_r_values, max_subs_r, double, max_subs_r);
		    }
		    subs_r_values[subs_r_values_cnt++] = $2 * 1e-6;
		}
		;

selfsub_descr	: /* empty */
		| SELFSUBRES COLON selfsub_list
		;
selfsub_list	: /* empty */
		| selfsub_list selfsub_item
		;
selfsub_item	: val val val val
		{
		    static int maxself = 0;
		    if (self_cnt == maxself) {
			maxself += MAXSELFSUB;
			REALLOC (selfs, maxself, struct selfsubdata, self_cnt);
		    }
		    selfs[self_cnt].area = $1;
		    selfs[self_cnt].perim = $2;
		    selfs[self_cnt].val = $3;
		    selfs[self_cnt].rest = $4;
		    self_cnt++;
		}
		;

mutsub_descr	: /* empty */
		| COUPSUBRES COLON mutsub_list
		;
mutsub_list	: /* empty */
		| mutsub_list mutsub_item
		;
mutsub_item	: val val val val val
		{
		    static int maxmut = 0;
		    if (mut_cnt == maxmut) {
			maxmut += MAXMUTSUB;
			REALLOC (muts, maxmut, struct mutualsubdata, mut_cnt);
		    }
		    muts[mut_cnt].area1 = $1;
		    muts[mut_cnt].area2 = $2;
		    muts[mut_cnt].dist = $3;
		    muts[mut_cnt].val = $4;
		    muts[mut_cnt].decr = $5;
		    mut_cnt++;
		}
		;

/* The following part was written by Kees Jan van der Kolk
/* and embedded in the existing version of tecc by Arjan van Genderen.

/*------------------------------------------------------------------------------
/* References
/*
/* [1] De Micheli, G., "Synthesis and optimization of digital circuits,"
/*     McGraw-Hill, 1994.
/*  */

min_terms : or_expression
	{
		$$ = $1;
	}
	| /* empty */
	{
		$$ = 1;
	}
	;

or_expression : and_expression
	{
		$$ = $1;
	}
	| or_expression OR and_expression
	{
		/* note: see table 2.2 on page 81 of [1]
		*/
		$$ = ROBDD_OR ($1, $3);
	}
	;

and_expression : unary_expression
	{
		$$ = $1;
	}
	| and_expression unary_expression
	{
		/* note: see table 2.2 on page 81 of [1]
		*/
		$$ = ROBDD_AND ($1, $2);
	}
	;

unary_expression : primary
	{
		$$ = $1;
	}
	| NOT unary_expression
	{
		/* note: see table 2.2 on page 81 of [1]
		*/
		$$ = ROBDD_NOT ($2);
	}
	;

primary : LPS min_terms RPS
	{
		$$ = $2;
	}
	| primary_name
	{
		$$ = robdd.symbolNode ($1);
	}
	| literal
	{
		$$ = $1;
	}
	;

primary_name : name
	{
		int n;
		char *s;
		if ((n = maskno ($1)) < 0) yyerror (mprintf ("Unknown mask '%s'", $1));
		if (n >= procdata -> nomasks && resizemask (n) >= 0) s = mprintf ("$%s", $1);
		else s = $1;
		$$ = s;
	}
	| MINUS name
	{
		int n;
		char *s;
		if ((n = maskno ($2)) < 0) yyerror (mprintf ("Unknown mask '%s'", $2));
		if (doingNewmask)
		    yyerror ("Cannot use '-' in condition list for new mask");
		else if (doingResize)
		    yyerror ("Cannot use '-' in condition list for resize mask");
		else if (doingCap) ++doingCap;
		else if (doingBjt) ++doingBjt;
		else if (doingConnect) ++doingConnect;
		else if (!doingShape)
		    yyerror ("Cannot use '-' in condition list for element");

		if (n >= procdata -> nomasks && resizemask (n) >= 0) s = mprintf ("-$%s", $2);
		else s = mprintf ("-%s", $2);
		$$ = s;
	}
	| EQUAL name
	{
		int n;
		char *s;
		if ((n = maskno ($2)) < 0) yyerror (mprintf ("Unknown mask '%s'", $2));
		if (doingNewmask)
		    yyerror ("Cannot use '=' in condition list for new mask");
		else if (doingResize)
		    yyerror ("Cannot use '=' in condition list for resize mask");
		else if (doingCap) ++doingCap;
		else if (doingBjt) ++doingBjt;
		else yyerror ("Cannot use '=' in condition list for element");

		if (n >= procdata -> nomasks && resizemask (n) >= 0) s = mprintf ("=$%s", $2);
		else s = mprintf ("=%s", $2);
		$$ = s;
	}
	;

literal : INTEGER
	{
	    if ($1 == 0) $$ = 0;
	    else {
		if ($1 != 1) yyerror ("syntax error");
		$$ = 1;
	    }
	}
	;

/* End of part written by Kees Jan
*/

layer_s		: layer
		{
		    $$ = $1;
		}
		| GND
		{
		    ALLOC ($$, 1, struct layer);
		    $$ -> mask = -1;
		    $$ -> occurrence = SURFACE;
		}
		| SUB
		{
		    ALLOC ($$, 1, struct layer);
		    $$ -> mask = -2;
		    $$ -> occurrence = SURFACE;
		}
		| perc_subcont
		{
                    ALLOC ($$, 1, struct layer);
                    $$ -> mask = -4;
                    $$ -> occurrence = SURFACE;
		}
		;

/* EM: support for %(min_terms). creates a substrate terminal
 *     which will be recognized in space whenever min_terms is true.
 *     This notation should replace the '@sub' notation
 */

perc_subcont	: PERC
		{
                    curr_subcont = newsubcont ();
		    curr_subcont -> name = checkElemName (mprintf ("%csubcont%d", SUBCONT_PREFIX, sbc_cnt - 1));
		    curr_subcont -> id   = name_cnt - 1;
                    curr_subcont -> ccnr = -1;
		    curr_subcont -> captype = 0;
		}
		  LPS min_terms RPS
		{
		    handle_min_terms (curr_subcont -> name, $4);
                    if (minTermCnt > 0) {
                	curr_subcont -> cond = condStack[--minTermCnt];
                	while (minTermCnt > 0) {
                	    other_subcont = newsubcont ();
                	    other_subcont -> cond = condStack[--minTermCnt];
                	    other_subcont -> name = curr_subcont -> name;
                	    other_subcont -> id   = curr_subcont -> id;
			    other_subcont -> ccnr = -1;
			    other_subcont -> captype = 0;
                	}
                    }
                    else curr_subcont -> cond = NULL;
		}
                ;

layer		: mask
		{
		    ALLOC ($$, 1, struct layer);
		    $$ -> mask = $1;
		    $$ -> occurrence = SURFACE;
		}
		| MINUS mask
		{
		    ALLOC ($$, 1, struct layer);
		    $$ -> mask = $2;
		    $$ -> occurrence = EDGE;
		}
		| EQUAL mask
		{
		    ALLOC ($$, 1, struct layer);
		    $$ -> mask = $2;
		    $$ -> occurrence = OTHEREDGE;
		}
		;

mask_s		: SUB
		{
		    $$ = -2;
		}
		| GND	{ $$ = -1; }
		| mask
		{
		    $$ = $1;
		}
		;

contact_mask_s	: SUB
                {
                    $$ = -2;
                }
		| perc_subcont
		{
                    $$ = -4;
		}
                | mask
                {
                    $$ = $1;
                }
		| MINUS mask { $$ = -1; }
		| EQUAL mask { $$ = -1; }
		| GND	{ $$ = -1; }
		;

mask		: name
		{
		    if (($$ = maskno ($1)) < 0) yyerror (mprintf ("Unknown mask '%s'", $1));
		}
		;

name		: identif
		{
                    if (nmBufToggle) {
			strcpy (namebuf1, $1);
			$$ = namebuf1;
			nmBufToggle = 0;
		    }
		    else {
			strcpy (namebuf2, $1);
			$$ = namebuf2;
			nmBufToggle = 1;
		    }
		}
		;

identif		: IDENTIFIER { $$ = $1; }
		;

vals_1_2_or_4	: val
		{
		    retval[0] = $1;
		    val_cnt = 1;
		}
		| val val
		{
		    retval[0] = $1;
		    retval[1] = $2;
		    val_cnt = 2;
		}
		| val val val val
		{
		    retval[0] = $1;
		    retval[1] = $2;
		    retval[2] = $3;
		    retval[3] = $4;
		    val_cnt = 4;
		}
		;

val		: INTEGER { $$ = (double)$1; }
		| DOUBLE  { $$ = $1; }
		;

%%

extern "C" void doparse (char *filename) {
    specific_res = 0;
    keylist = NULL;
    maxNbrKeys = maxNbrKeys2 = -1;
    maxEdgeKeys = 0;
    yyin = cfopen (filename, "r");
    yylineno = 1;
    yyparse ();
    fclose (yyin);
}

Private struct resistance * newres (void)
{
    static int maxres = 0;
    if (res_cnt == maxres) {
	maxres += MAXRES;
	REALLOC (ress, maxres, struct resistance, res_cnt);
    }
    return &ress[res_cnt++];
}

Private struct transistor * newtor (void)
{
    static int maxtor = 0;
    if (tor_cnt == maxtor) {
	maxtor += MAXTOR;
	REALLOC (tors, maxtor, struct transistor, tor_cnt);
    }
    return &tors[tor_cnt++];
}

Private struct contact * newcon (void)
{
    static int maxcon = 0;
    if (con_cnt == maxcon) {
	maxcon += MAXCON;
	REALLOC (cons, maxcon, struct contact, con_cnt);
    }
    return &cons[con_cnt++];
}

Private struct capacitance * newcap (void)
{
    static int maxcap = 0;
    if (cap_cnt == maxcap) {
	maxcap += MAXCAP;
	REALLOC (caps, maxcap, struct capacitance, cap_cnt);
    }
    return &caps[cap_cnt++];
}

Private struct vdimension * newvdim ()
{
    static int maxvdm = 0;
    if (vdm_cnt == maxvdm) {
	maxvdm += MAXVDIM;
	REALLOC (vdms, maxvdm, struct vdimension, vdm_cnt);
    }
    return &vdms[vdm_cnt++];
}

Private struct shape * newshape ()
{
    static int maxshp = 0;
    if (shp_cnt == maxshp) {
	maxshp += MAXSHAPE;
	REALLOC (shps, maxshp, struct shape, shp_cnt);
    }
    return &shps[shp_cnt++];
}

Private struct junction * newjun ()
{
    static int maxjun = 0;
    if (jun_cnt == maxjun) {
	maxjun += MAXJUN;
	REALLOC (juns, maxjun, struct junction, jun_cnt);
    }
    return &juns[jun_cnt++];
}

Private struct bipoTor * newbjt ()
{
    static int maxbjt = 0;
    if (bjt_cnt == maxbjt) {
	maxbjt += MAXBJT;
	REALLOC (bjts, maxbjt, struct bipoTor, bjt_cnt);
    }
    return &bjts[bjt_cnt++];
}

Private struct connect * newcnt ()
{
    static int maxcnt = 0;
    if (cnt_cnt == maxcnt) {
	maxcnt += MAXCNT;
	REALLOC (cnts, maxcnt, struct connect, cnt_cnt);
    }
    return &cnts[cnt_cnt++];
}

Private struct subcont * newsubcont ()
{
    static int maxsbc = 0;
    if (sbc_cnt == maxsbc) {
	maxsbc += MAXSUBCONTS;
	REALLOC (subconts, maxsbc, struct subcont, sbc_cnt);
    }
    return &subconts[sbc_cnt++];
}

Private int setsubcontccnr (int ccmsk, int id, int cap)
{
    int nr = sbc_cnt;
    if (!curr_subcont || curr_subcont -> id != id + 1) return 1;
    do {
	other_subcont = &subconts[ --nr ];
	if (other_subcont -> id != id + 1) return 1;
	other_subcont -> ccnr = ccmsk;
	other_subcont -> captype = cap;
    } while (other_subcont != curr_subcont);
    return 0;
}

Private struct resizedata * newresize ()
{
    static int maxresize = 0;
    if (resize_cnt == maxresize) {
	maxresize += MAXRESIZE;
	REALLOC (resizes, maxresize, struct resizedata, resize_cnt);
    }
    return &resizes[resize_cnt++];
}

Private struct waferdata * newwafer ()
{
    static int maxwaf = 0;
    if (waf_cnt == maxwaf) {
	maxwaf += MAXWAFER;
	REALLOC (wafers, maxwaf, struct waferdata, waf_cnt);
    }
    return &wafers[waf_cnt++];
}

int resizemask (int mask)
{
    for (int i = 0; i < resize_cnt; i++) {
	if (mask == resizes[i].id) return i;
    }
    return -1;
}

Private int maskno (char *name)
{
    int i;
    if (*name == '$') { ++name; goto newmasks; }
    for (i = 0; i < procdata -> nomasks; i++) {
	if (strcmp (name, procdata -> mask_name[i]) == 0)
	    return (i);
    }
newmasks:
    for (i = 0; i < subdata -> nomasks; i++) {
	if (strcmp (name, subdata -> mask_name[i]) == 0)
	    return (procdata -> nomasks + i);
    }
    if (begin_wmlist && *name == 'w') {
	struct wmlist *w = begin_wmlist;
	do {
	    if (strcmp (name, w -> name) == 0) return (w -> mskno);
	} while ((w = w -> next));
    }
    return (-1);
}

Private int polarityno (char *name)
{
    if (!name[1]) {
	if (*name == 'm') return M_TYPE;
	if (*name == 'n') return N_TYPE;
	if (*name == 'p') return P_TYPE;
    }
    fatalErr ("Invalid RES-type", name);
    return M_TYPE;
}

Private int yesno (char *name)
{
    if (strsame (name, "off") || strsame (name, "no")) return 0;
    if (strsame (name, "on") || strsame (name, "yes")) return 1;
    fatalErr ("Invalid yes/no or on/off:", name);
    return 0;
}

Private int bjtno (char *name)
{
    if (strsame (name, "npn") || strsame (name, "ver")) return (VBJTELEM);
    if (strsame (name, "pnp") || strsame (name, "lat")) return (LBJTELEM);
    fatalErr ("Invalid BJT-type", name);
    return 0;
}

Private char *checkElemName (char *name)
{
    static int maxnames = 0;
    if (strlen (name) > MAX_ELEM_NAME) {
	fprintf (stderr, "warning: too long element name (truncated to %d chars)\n", MAX_ELEM_NAME);
	name[MAX_ELEM_NAME] = 0;
    }
    if (maskno (name) >= 0) {
	fatalErr ("Element name already used as mask name:", name);
    }
    for (int i = 0; i < name_cnt; i++) {
	if (strcmp (name, nameTab[i]) == 0) {
	    yyerror (mprintf ("Multiple use of element name '%s'", name));
	    //fatalErr ("Multiple use of element name", name);
	}
    }
    if (name_cnt == maxnames) {
	maxnames += MAX_NAMES;
	REALLOC (nameTab, maxnames, char *, maxnames);
    }
    nameTab[name_cnt] = strsave (name);
    return nameTab[name_cnt++];
}

Private struct newMaskRef *addNewMask (int mask)
{
    struct newMaskRef *nm;

    ALLOC (nm, 1, struct newMaskRef);
    nm -> mask = mask;

    nm -> cond_lists = NULL;
    nm -> next = NULL;

    if (first_newmask_list) {
	last_newmask_list -> next = nm;
    }
    else {
	first_newmask_list = nm;
    }
    last_newmask_list = nm;

    return (nm);
}

Private struct newMaskRef *findNewMask (int mask)
{
    struct newMaskRef *nm;

    for (nm = first_newmask_list; nm; nm = nm -> next) {
	if (nm -> mask == mask) return (nm);
    }
    fprintf (stderr, "warning: new mask '%s' not found!\n", maskname (mask));
    return (nm);
}

/* EM: The following routine checks if there exists a transistor for
 *     which a drain/source capacitance has been specified and of which
 *     the drain/source mask is the same as 'mask'.
 *     'name' should be the name of the junction capacitance for which
 *     this check is being performed.
 */
Private void checkTransistorDSMask (char *name, int mask, int pol)
{
    for (int i = 0; i < tor_cnt; i++)
	if (tors[i].ds == mask && (tors[i].dscap == pol || tors[i].dscap < 0)) {
	    char *s = mprintf ("transistor %s: includes d/s cap definition, but also '%s'",
				tors[i].name, name);
	    warningMes (s, "definition found");
	}
}

/* The following routines were written by Kees Jan van der Kolk
 * and modified for tecc by Arjan van Genderen.
*/

/*----------------------------------------------------------------------------*/

Private void handle_min_terms (char *name, STDSDWord sdword)
{
	MINSymbol *symbol;

	ASSERT (sdword >= 0);
	symbol = robdd.defineSymbol (name, sdword);
	ASSERT (symbol != 0);

#ifdef DEBUG_OUTPUT
	cerr << "message: " << ": defined `" << name << "' as: (" <<
		symbol->getDefinition() << ") ";
	robdd.doOutput (cerr, symbol->getDefinition());
	cerr << endl;
	cerr << "nodes: " << robdd.getNumNodes() << endl;
#endif

	/* create table
	*/
	std::vector<MINSymbol*> variables;
	variables.reserve(32);
	MINTable* table = robdd.createTable(symbol->getDefinition(), variables);

#ifdef DEBUG_OUTPUT
	cerr << "TABLE\n" << *table << endl;
#endif
	MINTable old_table(*table);

	/* minimize table
	*/
	table->minimize();

	ASSERT(table->isEquivalent(old_table));

#ifdef DEBUG_OUTPUT
	cerr << "MINIMIZED\n" << *table << endl;
#endif
	if (printCondList) { /* dump minimized expression */
		cout << name << ": ";
		dumpExpression (cout, *table, variables);
		cout << endl;
	}

	expression2CondList (name, *table, variables);

	delete table;
}

Private STDSDWord getNode (struct layerCond *layC, int p)
{
    STDSDWord w;
    int n = layC -> lay -> mask;
    ASSERT (layC -> lay -> occurrence == SURFACE);
    w = robdd.symbolNode (mprintf ("%c%s%s", p, (n >= procdata -> nomasks ? "$" : ""), maskname (n)));
    if (!layC -> present) w = ROBDD_NOT (w);
    return w;
}

Private STDSDWord make_minterm (struct layCondRef *c, int p)
{
    STDSDWord w;
    ASSERT (c);
    w = getNode (c -> layC, p);
    while ((c = c -> next)) w = ROBDD_AND (w, getNode (c -> layC, p));
    return w;
}

/*----------------------------------------------------------------------------*/

static void
dumpExpression (ostream& ostr, const MINTable& table, const std::vector<MINSymbol*>& variables)
{
	char name[132], *s;

	ASSERT(table.getNumVars() == variables.size());

	if (table.getNumRows() == 0) ostr << "0";
	else
	for (STDDWord row = 0; row < table.getNumRows(); row++)
	{
		if (row) ostr << " | ";

		bool printed = false;

		for (STDDWord var = 0; var < table.getNumVars(); var++)
		{
			STDDWord field = table.getField(row, var);
			if (field == 0x3) continue;

			if (printed) ostr << ".";
			else printed = true;

			if (field == 0x1) ostr << "!";

			strcpy (s = name, variables[var]->get_name().c_str());
			     if (*s == '-') { if (*(s+1) == '$') *++s = '-'; }
			else if (*s == '=') { if (*(s+1) == '$') *++s = '='; }
			else if (*s == '$') ++s;
			ostr << s;
		}
		if (!printed) ostr << "1";
	}

	ostr << "\n";
}

/*----------------------------------------------------------------------------*/

static void
expression2CondList (char *name, const MINTable& table,
			const std::vector<MINSymbol*>& variables)
{
	static int maxminterm = 0;
        struct layer * lay_el;
        struct layerCond * layC_el;
        struct layCondRef * last_cond_list;
        char *msk;

	minTermCnt = 0;

	ASSERT(table.getNumVars() == variables.size());

	if (table.getNumRows() == 0) {
	 	fatalErr ("Impossible condition list for new mask / element:", name);
	}

	for (STDDWord row = 0; row < table.getNumRows(); row++)
	{
		last_cond_list = NULL;

		for (STDDWord var = 0; var < table.getNumVars(); var++)
		{
			STDDWord field = table.getField(row, var);

			if (field == 0x3) continue;

			ALLOC (lay_el, 1, struct layer);
			msk = (char*)variables[var]->get_name().c_str();
			switch (*msk) {
			case '-': lay_el -> occurrence = EDGE; ++msk; break;
			case '=': lay_el -> occurrence = OTHEREDGE; ++msk; break;
			default : lay_el -> occurrence = SURFACE;
			}
			lay_el -> mask = maskno (msk);
			if (lay_el -> mask < 0) yyerror (mprintf ("Unknown mask '%s'", msk));

			ALLOC (layC_el, 1, struct layerCond);
			layC_el -> lay = lay_el;
			layC_el -> present = (field == 0x1)? 0 : 1;

			if (last_cond_list == NULL) {
			    ALLOC (last_cond_list, 1, struct layCondRef);
			    if (minTermCnt == maxminterm) {
				maxminterm += MAXMINTERM;
				REALLOC (condStack, maxminterm, struct layCondRef *, maxminterm);
			    }
			    condStack[minTermCnt++] = last_cond_list;
			}
			else {
			    ALLOC (last_cond_list -> next, 1, struct layCondRef);
			    last_cond_list = last_cond_list -> next;
			}
			last_cond_list -> layC = layC_el;
			last_cond_list -> next = NULL;
		}
	}

	if (minTermCnt >= MAXMINTERM) fprintf (stderr, "warning: %d minterms in condition at line %d\n", minTermCnt, yylineno);
}

static struct layCondRef * add_cond (struct layCondRef *cond)
{
    struct layCondRef *c, *c_new;

    if ((c = cond)) {
	ALLOC (cond, 1, struct layCondRef);
	c_new = cond;
	c_new -> layC = c -> layC;
	while ((c = c -> next)) {
	    ALLOC (c_new -> next, 1, struct layCondRef);
	    c_new = c_new -> next;
	    c_new -> layC = c -> layC;
	}
	c_new -> next = NULL;
    }
    return cond;
}

static struct condListRef *add_condlist (struct condListRef *cref)
{
    struct condListRef *c;
    ALLOC (c, 1, struct condListRef);
    c -> cond_list = add_cond (cref -> cond_list);
    for (cref = cref -> next; cref; cref = cref -> next) {
	ALLOC (c -> next, 1, struct condListRef);
	c = c -> next;
	c -> cond_list = add_cond (cref -> cond_list);
    }
    return c;
}

Private int checkPinCond (int msk, int occ, STDSDWord e1, int mode)
{
    STDSDWord e2;
    MINSymbol *symbol;
    char *name, *p, *s, *mask;

    if (msk < 0) return 0;
    mask = maskname (msk);

    if (occ == SURFACE) s = (char*)"";
    else if (occ == EDGE) s = (char*)"-";
    else s = (char*)"=";

    /* If new mask is used in a resize, then use a '$' prefix.
     */
    if (msk >= procdata -> nomasks && resizemask (msk) >= 0) p = (char*)"$"; else p = (char*)"";

    /* Check for pin mask (not for the condition of the pin mask).
     * If the pin mask is a new mask, then the condition is used.
     */
    e2 = e1 + 1;
    if ((symbol = robdd.findSymbol (mprintf ("%s%s%s", s, p, mask)))) {
	e2 = ROBDD_AND (e1, symbol->getDefinition());
    }
    if (e2 != e1) {
	     if (mode == 0) { p = (char*)"cap";     name = curr_cap -> name; }
	else if (mode == 1) { p = (char*)"connect"; name = curr_cnt -> name; }
	else if (mode == 2) { p = (char*)"eshape";  name = curr_shape -> name; }
	else if (mode == 3) { p = (char*)"cshape";  name = curr_shape -> name; }
	else if (mode == 4) { p = (char*)"contact"; name = curr_con -> name; }
	else { p = (char*)"conductor"; name = curr_res -> name; }
	fprintf (stderr, "warning: pin layer '%s%s' not found", s, mask);
	fprintf (stderr, " in condition for %s element %s\n", p, name);
	return 1;
    }
    if (occ != SURFACE) { /* check if we must give an advice message */
	if ((symbol = robdd.findSymbol (mprintf ("%s%s", p, mask)))) {
	    e2 = ROBDD_AND (e1, ROBDD_NOT (symbol->getDefinition()));
	}
	if (!symbol || e2 != e1) {
		 if (mode == 0) { p = (char*)"edgecap"; name = curr_cap -> name; }
	    else if (mode == 1) { p = (char*)"connect"; name = curr_cnt -> name; }
	    else if (mode == 2) { p = (char*)"eshape"; name = curr_shape -> name; }
	    else { p = (char*)"edge"; name = (char*)"??"; }
	    fprintf (stderr, "warning: advice to use condition '!%s %s%s'", mask, s, mask);
	    fprintf (stderr, " for %s element %s\n", p, name);
	}
    }
    return 0;
}

Private int mask_in_condStack (int mask, int present)
{
    struct layCondRef *c;
    int i;
    for (i = 0; i < minTermCnt; ++i) {
	for (c = condStack[i]; c; c = c -> next) {
	    if (c -> layC -> lay -> mask == mask && c -> layC -> present == present) return 1;
	}
    }
    return 0;
}

Private void checkResizeCond (int mask, struct newMaskRef *nm)
{
    if (nm) {
	struct condListRef *cref;
	struct layCondRef *c;
	for (cref = nm -> cond_lists; cref; cref = cref -> next) {
	    for (c = cref -> cond_list; c; c = c -> next) {
		if (!mask_in_condStack (c -> layC -> lay -> mask, c -> layC -> present)) {
		    fprintf (stderr, "warning: mask '%s%s' of",
			c -> layC -> present ? "" : "!", maskname (c -> layC -> lay -> mask));
		    goto err;
		}
	    }
	}
    }
    else if (!mask_in_condStack (mask, 1)) {
	fprintf (stderr, "warning:");
err:	fprintf (stderr, " output layer '%s' not found", maskname (mask));
	fprintf (stderr, " in resize condition at line %d\n", yylineno);
    }
}

Private int checkPinPol (char *name, int msk, int occ, int mode)
{
    struct layCondRef *cl, *co;
    struct layerCond *layC, *layC1, *layC2;
    char *s;
    int i, n, ok, cnt;
    int res_nr = -1;

    ASSERT (minTermCnt >= 1);

    for (i = 0; i < res_cnt; ++i) {
	if (ress[i].mask == msk) {
	    cnt = minTermCnt - 1;
	    layC2 = 0;
again:
	    layC1 = 0;
	    for (cl = ress[i].cond; cl && cnt >= 0; cl = cl -> next) {
		/* The conductor condition 'cl' must be ok in none, one or more condStacks.
		** If the condition is !ok for a condStack, that condStack must be removed.
		*/
		/* test all remaining condStacks */
		layC = cl->layC;
		for (n = cnt; n >= 0; --n) {
		    ok = -1;
		    for (co = condStack[n]; co; co = co -> next) {
			if (co->layC->lay->mask == layC->lay->mask &&
			    co->layC->lay->occurrence == occ) {
			    ok = (co->layC->present == layC->present);
			    if (!ok) { /* remove condStack */
				if (n < cnt) {
				    co = condStack[n];
				    condStack[n] = condStack[cnt];
				    condStack[cnt] = co;
				}
				--cnt;
			    }
			    break;
			}
		    }
		    if (ok < 0 && !layC1) layC1 = layC;
		}
	    }

	    if (cnt >= 0) {
		if (layC1) { /* don't care condition found, try again to be sure that it is true */
		    if (layC2 != layC1) { layC2 = layC1; goto again; }
		    s = mprintf ("element %s: conductor %s: %s%s%s", name, ress[i].name,
			layC1->present ? "" : "!", occ == SURFACE ? "" : "-", maskname (layC1->lay->mask));
		    if (mode == 0)
			warningMes (s, "not found in d/s cap condition");
		    else
			warningMes (s, "not found in element condition");
		}
		if (ress[i].type == M_TYPE) {
		    if (mode == 0)
			s = mprintf ("element %s: d/s cap condition:", name);
		    else
			s = mprintf ("element %s: pin mask %s:", name, maskname (msk));
		    fatalErr (s, "matching conductor has no polarity\n");
		}
		if (res_nr < 0) res_nr = i;
		else if (ress[i].type != ress[res_nr].type) {
		    if (mode == 0)
			s = mprintf ("element %s: d/s cap condition:", name);
		    else
			s = mprintf ("element %s: pin mask %s:", name, maskname (msk));
		    fatalErr (s, "matching conductors have different polarity\n");
		}
		if (ress[i].val != 0) res_nr = i;
	    }
	}
    }

    if (res_nr < 0) {
      if (mode == 0)
	s = mprintf ("%s   d/s cap pin of element '%s'",
	    "Layer Condition Error :\n", name);
      else
	s = mprintf ("%s   pin mask '%s' of element '%s'",
	    "Layer Condition Error :\n", maskname (msk), name);
	fatalErr (s, "will not connect to a conductor layer\n");
    }
    return res_nr;
}

Private void checkShapeCond (STDSDWord cond)
{
    if (curr_shape -> type == ESHAPEELEM)
	checkPinCond (curr_shape -> mask, EDGE, cond, 2);
    else if (checkPinCond (curr_shape -> mask, SURFACE, cond, 3) == 0) {
	struct layCondRef *c, *d;
	int i;
	for (i = 0; i < minTermCnt; ++i) {
	    for (c = condStack[i]; c; c = c -> next) {
		if (c -> layC -> lay -> occurrence == EDGE && c -> layC -> present) {
		    if (c -> layC -> lay -> mask == curr_shape -> mask) continue;
		    for (d = condStack[i]; d; d = d -> next) {
			if (!d -> layC -> present && d -> layC -> lay -> occurrence == SURFACE) {
			    if (d -> layC -> lay -> mask == c -> layC -> lay -> mask) goto ok;
			}
		    }
		}
	    }
	    fprintf (stderr, "warning: advice to use '!lay -lay' in condition");
	    fprintf (stderr, " for cshape element %s\n", curr_shape -> name);
	    return;
ok:	    ;
	}
    }
}

Private void makeConductor (struct condListRef *cref,
			char *msk, int mskno, double value, double tn)
{
    ++fem_res_cnt;
    curr_res = newres ();
    curr_res -> cond = add_cond (cref -> cond_list);
    curr_res -> name = checkElemName (mprintf ("cnd$%s", msk));
    curr_res -> id   = name_cnt - 1;
    curr_res -> mask = mskno;
    curr_res -> sort = (char*)"res";
    curr_res -> type = restype;
    curr_res -> val  = value;
    curr_res -> height = w_depth * -1e-9;
    curr_res -> thickness = tn;

    other_res = 0;
    while ((cref = cref -> next)) {
	++fem_res_cnt;
	other_res = newres ();
	other_res -> cond = add_cond (cref -> cond_list);
	other_res -> name = curr_res -> name;
	other_res -> id   = curr_res -> id;
	other_res -> mask = curr_res -> mask;
	other_res -> sort = curr_res -> sort;
	other_res -> type = curr_res -> type;
	other_res -> val  = curr_res -> val;
	other_res -> height = curr_res -> height;
	other_res -> thickness = curr_res -> thickness;
    }
}

Private void makeContact (struct condListRef *cref, struct layCondRef *cvia,
			int k, int msk1, int msk2, double value)
{
    curr_con = newcon ();
    if (cvia) {
	cvia -> next = cref -> cond_list;
	curr_con -> cond = add_cond (cvia);
    }
    else
	curr_con -> cond = add_cond (cref -> cond_list);
    curr_con -> name = checkElemName (mprintf ("cnt$w%d_%d", waf_cnt, k));
    curr_con -> id   = name_cnt - 1;
    curr_con -> sort = curr_res -> sort;
    curr_con -> val  = value;
    curr_con -> mask1 = msk1;
    curr_con -> mask2 = msk2;

    while ((cref = cref -> next)) {
	other_con = newcon ();
	if (cvia) {
	    cvia -> next = cref -> cond_list;
	    other_con -> cond = add_cond (cvia);
	}
	else
	    other_con -> cond = add_cond (cref -> cond_list);
	other_con -> name = curr_con -> name;
	other_con -> id   = curr_con -> id;
	other_con -> sort = curr_con -> sort;
	other_con -> val  = value;
	other_con -> mask1 = msk1;
	other_con -> mask2 = msk2;
    }
}

Private void makeSubCont (struct condListRef *cref, int msk)
{
    curr_subcont = newsubcont ();
    curr_subcont -> cond = add_cond (cref -> cond_list);
    curr_subcont -> name = checkElemName (mprintf ("%csubcont%d", SUBCONT_PREFIX, sbc_cnt - 1));
    curr_subcont -> id   = name_cnt - 1;
    curr_subcont -> ccnr = msk;
    curr_subcont -> captype = 0;

    while ((cref = cref -> next)) {
	other_subcont = newsubcont ();
	other_subcont -> cond = add_cond (cref -> cond_list);
	other_subcont -> name = curr_subcont -> name;
	other_subcont -> id   = curr_subcont -> id;
	other_subcont -> ccnr = msk;
	other_subcont -> captype = 0;
    }
}

void set_profile_subcont ()
{
    static int pcnt, wcnt;

    if (profile_cnt) { // there is a wafer profile specified
	if (w_depth != bem_depth && wcnt != profile_cnt) {
	    wcnt = profile_cnt;
	    warningMes (mprintf ("wafer_%d:", waf_cnt), "profile does not match bem_depth");
	}
	if (pcnt != profile_cnt) { // subcont not set for wafer profile
	    pcnt = profile_cnt;
	    makeSubCont (prev_cref, prev_mskno);
	}
    }
}

Private int makeNewMask (struct condListRef *cref, char *msk, int mskno)
{
    struct newMaskRef *nm;

    if (maskno (msk) >= 0) {
	fatalErr (mprintf ("wafer_%d: %s:", waf_cnt, msk), "new mask already exists");
    }
    if (mskno < 0) {
	if (subdata -> nomasks == maxsubmasks) {
	    maxsubmasks += MAXSUBMASKS;
	    REALLOC (subdata -> mask_name, maxsubmasks, char *, maxsubmasks);
	    REALLOC (masknewcolor, maxsubmasks, char *, subdata -> nomasks);
	}
	subdata -> mask_name[subdata -> nomasks] = msk;
	subdata -> nomasks++;
	mskno = maskno (msk);
    }
    nm = addNewMask (mskno);
    nm -> resized = 2;
  //nm -> name = checkElemName (mprintf ("new$%s", msk));
  //nm -> id = name_cnt - 1;
    nm -> cond_lists = add_condlist (cref);
    return nm -> mask;
}

Private double getDielConstant (double z0, double z1)
{
    int i;
    double d, dp, dz, e;

    dz = z1 - z0; // meters

         if (diel_cnt < 1) e = 1.0; // vacuum
    else if (diel_cnt < 2) e = diels[0].permit;
    else {
        z0 *= 1e6; // microns
        z1 *= 1e6;
        dp = e = 0;
        for (i = 1; i < diel_cnt; i++) {
            if (z1 <= diels[i].bottom) break;
            if (z0 < diels[i].bottom) {
                d = diels[i].bottom - z0;
                e += (d - dp) / diels[i-1].permit;
                dp = d;
            }
        }
        --i;
        if (e > 0) {
            d = z1 - diels[i].bottom;
            e += d / diels[i].permit;
            return (8.855e-6 / e);
        }
        e = diels[i].permit;
    }
    return (8.855e-12 * e / dz);
}

Private void setDsCap (STDSDWord sdword, int smode)
{
    int len, c, i, dsmsk;
    STDSDWord e0, e1, e2, e3, e4, e5;
    MINSymbol *symbol;

    dsmsk = smode == 0 ? curr_tor -> ds : curr_tor -> s;

    handle_min_terms (cap_name, sdword);
    if (minTermCnt <= 0) {
	char *s = mprintf ("transistor %s: includes d/s cap definition,", curr_tor -> name);
	fatalErr (s, "but condition list is empty\n");
    }

    /* EM: Check if the resistivity of the drain/source conductor has been set to zero,
     *     because d/s resistances will probably be computed in the spice model too.
     * SG: The d/s conductor must have a polarity type 'p' or 'n'. This polarity is
     *     stored in the 'dscap' member. Thus, it can be used to check for double d/s
     *     junction cap definitions. We must check only the transistor for which the
     *     polarity type is matching.
     *     The polarity is currently not used here below to set the correct pin order.
     *     And the 'p' pin is always connected to @gnd. This can savely be done, because
     *     these caps are only used for setting additional transistor parameters in a
     *     spice netlist. But the bulk conductor was a better choice.
     */
    i = checkPinPol (curr_tor -> name, dsmsk, SURFACE, 0);
#if 0
    /* SG: resistivity does not give problems anymore */
    if (i >= 0 && ress[i].val != 0) {
	char *s = mprintf ("transistor %s: includes d/s cap definition, but '%s'",
			curr_tor -> name, ress[i].name);
	warningMes (s, "resistivity is != 0");
    }
#endif
    ASSERT (i < 0 || ress[i].type > 0);
    curr_tor -> dscap = i < 0 ? i : ress[i].type; /* TRUE */

    curr_cap = newcap();
    curr_cap -> name = cap_name;
    curr_cap -> id = name_cnt - 1;

    len = strlen (cap_name) - 2;
    c = cap_name[len];
    cap_name[len] = '\0';
    curr_cap -> sort = strsave (cap_name);
    cap_name[len] = c;

    curr_cap -> junc = 1;
    curr_cap -> eltype = SURFCAPELEM;
    curr_cap -> val = a_cap_unit;
    curr_cap -> x_y_vals = NULL;

    /* SG: Don't change the pin order below, because xspice (xsls) is using
     *     the 'n' pin to find the d/s node and net number.
     */
    ALLOC (curr_cap -> nLay, 1, struct layer);
    curr_cap -> nLay -> mask = dsmsk;
    curr_cap -> nLay -> occurrence = SURFACE;
    ALLOC (curr_cap -> pLay, 1, struct layer);
    curr_cap -> pLay -> mask = -1;
    curr_cap -> pLay -> occurrence = SURFACE;

    /* Since elements with the same id must be succeeding, we
       first do all area elements and later all edge elements.
    */
    e0 = 1;
    if (minTermCnt > 0) {
	curr_cap -> cond = condStack[--minTermCnt];
	e0 = make_minterm (curr_cap -> cond, '-');
	while (minTermCnt > 0) {
	    other_cap = newcap ();
	    other_cap -> cond = condStack[--minTermCnt];
	    e0 = ROBDD_OR (e0, make_minterm (other_cap -> cond, '-'));
	    other_cap -> name = curr_cap -> name;
	    other_cap -> id   = curr_cap -> id;
	    other_cap -> sort = curr_cap -> sort;
	    other_cap -> junc = curr_cap -> junc;
	    other_cap -> eltype = curr_cap -> eltype;
	    other_cap -> pLay = curr_cap -> pLay;
	    other_cap -> nLay = curr_cap -> nLay;
	    other_cap -> val  = curr_cap -> val;
	    other_cap -> x_y_vals = NULL;
	}
    }
    else curr_cap -> cond = NULL;

    cap_name = checkElemName (mprintf ("%c%s%s-%c",
	DSCAP_PREFIX, curr_tor -> name, DSCAP_SUFFIX, smode == 0 ? 'e' : 'f'));

    curr_ecap = newcap();
    curr_ecap -> name = cap_name;
    curr_ecap -> id = name_cnt - 1;

    len = strlen (cap_name) - 2;
    c = cap_name[len];
    cap_name[len] = '\0';
    curr_ecap -> sort = strsave (cap_name);
    cap_name[len] = c;

    curr_ecap -> junc = 1;
    curr_ecap -> eltype = EDGECAPELEM;
    curr_ecap -> val = e_cap_unit;
    curr_ecap -> x_y_vals = NULL;

    ALLOC (curr_ecap -> nLay, 1, struct layer);
    curr_ecap -> nLay -> mask = dsmsk;
    curr_ecap -> nLay -> occurrence = EDGE;
    ALLOC (curr_ecap -> pLay, 1, struct layer);
    curr_ecap -> pLay -> mask = -1;
    curr_ecap -> pLay -> occurrence = SURFACE;

    /* Now we make a min_term list for the edge of
       the drain/source area that consists of:
	  NOT (drain/source mask)
	  AND NOT (the previous list)
	  AND (the previous list with '-' before it)
    */
    symbol = robdd.findSymbol (maskname (dsmsk));
    ASSERT (symbol);
    e1 = symbol->getDefinition();
    e2 = ROBDD_NOT (e1); // NOT (drain/source mask)
    e3 = ROBDD_NOT (sdword); // NOT (previous list)
    e4 = ROBDD_AND (e2, e3);
    e5 = ROBDD_AND (e4, e0); // AND (previous '-' list)

    handle_min_terms (cap_name, e5);

    if (minTermCnt > 0) {
	curr_ecap -> cond = condStack[--minTermCnt];
	while (minTermCnt > 0) {
	    other_cap++;
	    other_ecap = newcap ();
	    other_ecap -> cond = condStack[--minTermCnt];
	    other_ecap -> name = curr_ecap -> name;
	    other_ecap -> id   = curr_ecap -> id;
	    other_ecap -> sort = curr_ecap -> sort;
	    other_ecap -> junc = curr_ecap -> junc;
	    other_ecap -> eltype = curr_ecap -> eltype;
	    other_ecap -> pLay = curr_ecap -> pLay;
	    other_ecap -> nLay = curr_ecap -> nLay;
	    other_ecap -> val  = curr_ecap -> val;
	    other_ecap -> x_y_vals = NULL;
	}
    }
    else curr_ecap -> cond = NULL;
}
