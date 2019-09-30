/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#ifdef __cplusplus
  extern "C" {
#endif

/* options */
extern bool_t optAllRes;
extern bool_t optBackInfo;
extern bool_t optSimpleSubRes;
extern bool_t optSubRes;
extern bool_t optSubResSave;
extern bool_t optCap3D;
extern bool_t optEstimate3D;
extern bool_t optCap3DSave;
extern bool_t optCap;
extern bool_t optCoupCap;
extern bool_t optDisplay;
extern bool_t optNoMenus;
extern bool_t optDsConJoin;
extern bool_t optFineNtw;
extern bool_t optFlat;
extern bool_t optPseudoHier;
extern bool_t optInfo;
extern bool_t optInvertPrick;
extern bool_t optLatCap;
extern bool_t optMonitor;
extern bool_t optNoPrepro;
extern bool_t optNoReduc;
#ifdef PLOT_CIR_MODE
extern bool_t optPlotCir;
#endif
extern bool_t optPrick;
extern bool_t optPrintOnlyActions;
extern bool_t optPrintRecog;
extern bool_t optIntRes;
extern bool_t optRes;
extern bool_t optResMesh;
extern bool_t optTime;
extern bool_t optTorPos;
extern bool_t optVerbose;
extern bool_t extrPass;
extern bool_t extrEdgeCaps;
extern bool_t extrSurfCaps;
extern bool_t lastPass;
extern int    prePass;
extern bool_t prePass1;
extern bool_t optExtractMoments;
extern bool_t optSelectiveElimination;

extern int optMaxDepth;
extern int optMinDepth;
extern int do_not_die;
extern int verbosityLevel;
extern int inScale;
extern int outScale;

/* other control variables */
extern coor_t bandWidth, bandWidth2;
extern coor_t bigbxl, bigbxr, bigbyb, bigbyt;
extern coor_t bbxl, bbxr, bbyb, bbyt;

extern bool_t substrRes;
extern bool_t useAnnotations;
extern bool_t useHierAnnotations;
extern bool_t useHierTerminals;
extern bool_t useLeafTerminals;
extern char * inst_term_sep;
extern bool_t useCellNames;

extern double meters; /* this many meters per internal layout unit */
extern double min_coup_area;
extern double min_coup_cap;

/* main.c */
char *strCoorBrackets (coor_t x, coor_t y);
char *strCoor (coor_t a);
char *truncDmName (char *s);
char *makeIndex (int nx, int ny, int x, int y);

/* hier.c */
int findTerminal (char *termName);

/* input.c */
terminal_t *lookTerm (void);
void useLastLookTerm (void);
int  newLookTerm (coor_t x1, coor_t y1);

#ifdef __cplusplus
  }
#endif
