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

#include "src/space/extract/export.h"

#ifdef __cplusplus
  extern "C" {
#endif

/* contedge.c */
#ifdef DM_MAXNAME
void initContEdge (DM_CELL * layoutKey);
#endif
void endContEdge (void);
void contEdge (tile_t *tile, tile_t *newerTile);

/* gettech.c */
int diffusionCap (capElemDef_t *cap);

/* init.c */

/* latcap.c */
void updateLateralCap (tile_t *startTile, tile_t *tile, elemDef_t **elem, int edgeOrien, int nr);

/* nodepnt.c */
nodePoint_t *createNodePoint (coor_t x, coor_t y);
void disposeNodePoint (nodePoint_t *point);

/* meshedge.c */
#ifdef DM_MAXNAME
void initMeshEdge (DM_CELL *layoutKey);
#endif
void endMeshEdge (void);
void meshEdge (tile_t *tile, tile_t *newerTile, int edgeOrien);
void putMeshEdge (coor_t xl, coor_t xr, coor_t yl, coor_t yr);

/* check.c */
void checkDoubleJuncCaps (elemDef_t *f, elemDef_t **elem, coor_t x, coor_t y);

#ifdef __cplusplus
  }
#endif

/* extern data */
extern int nrOfMasks;
extern int nrOfCondStd;

extern int hasSubmask;
extern int hasEdgeConnect;
extern int hasSurfConnect;

extern coor_t bigbxl, bigbxr, bigbyb, bigbyt;

extern maskinfo_t * masktable;
extern int * keyTab;
extern int * keyTab2;
extern elemDef_t ** elemTab;
extern elemDef_t ** elemTab2;
extern elemDef_t * elemDefTab;

extern int elemDefTabSize;

extern double * conVal;
extern int * conNums;
extern int * conSort;
extern int * conductorMask;

extern int sBitmask;
extern int sBitmask2;
extern int eBitmask;
extern int oBitmask;
extern int sKeys, sKeys2;

extern coor_t bandWidth;
extern double effectDist;
extern int tileCnt;
extern int tileConCnt;

extern subnode_t *subnGND;
extern subnode_t *subnSUB;
extern double meters;

extern bool_t optCap;
extern bool_t optCap3D;
extern bool_t optCoupCap;
extern bool_t optLatCap;
extern bool_t optIntRes;
extern bool_t optRes;
extern bool_t substrRes;
extern bool_t optSubRes;
extern bool_t optSimpleSubRes;
extern bool_t optAllRes;
extern bool_t optVerbose;
extern bool_t optPrintRecog;
extern bool_t extrPass;
extern int    prePass;
extern bool_t prePass1;
extern bool_t optResMesh;
extern bool_t optPrick;
extern bool_t optBackInfo;

extern char *capPolarityTab;
extern char **capSortTab;
extern int capSortTabSize;

extern double low_contact_res;
extern double low_sheet_res;
extern int cap_assign_type;
extern double equi_line_ratio;
extern int equi_line_area;

extern bool_t mergeNeighborSubContacts;
extern bool_t *sep_on_res;

extern int joiningCon;
extern coor_t joiningX, joiningY;

