/*
 * ISC License
 *
 * Copyright (C) 2004-2018 by
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

/* update.c */
void setXr (coor_t xr);
void tileInsertEdge  (edge_t *edge);
void tileDeleteEdge  (edge_t *edge);
void tileCrossEdge   (edge_t *edge, int split);
void tileAdvanceScan (edge_t *edge);
void tileStopScan    (void);

/* edge.c */
edge_t *createEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
void disposeEdge (edge_t *edge);
void printEdge (char *s, edge_t *edge);
void edgeStatistics (FILE *fp);

/* input.c */
edge_t *openInput (void);
void swapInput (void);
void closeInput (void);
edge_t *fetchEdge (void);
char *gx (double val);
scCoor_t *swappoints (scCoor_t *pt);

/* main.c */
char *strCoorBrackets (coor_t x, coor_t y, double dscale);
char *strCoor (coor_t a, double dscale);
void die (void);

/* enumtile.c */
void enumPair (tile_t *tile, tile_t *newerTile, int edgeOrien);
void enumTile (tile_t *tile);
void initSubstr (void);
void endSubstr  (void);

/* gettech.c */
void getTechnology (DM_PROJECT *dmproject, char *techDef, char *techFile);
char *giveICD (char *filepath);

/* tile.c */
tile_t *createTile (coor_t xl, coor_t bl, coor_t xr, coor_t br, int cc);
void disposeTile (tile_t *tile);
void printTile (char *s, tile_t *tile);
void tileStatistics (FILE *fp);

/* scan.c */
void scan (edge_t *newEdge);
void scanPrintInfo (FILE *fp);

/* slant.c */
void testIntersection (edge_t *e1, edge_t *e2);
edge_t *split (edge_t *e1);

extern char * argv0;
extern char * csMASK;
extern mask_t cNull;
extern DM_CELL *cellKey;
extern FILE *outgeo_fp;

extern bool_t backcontact;
extern bool_t eliminateSubstrNode;
extern bool_t optRunFem;
extern coor_t bbxl, bbxr, bbyb, bbyt;
extern coor_t c_xl, c_xr, c_yb, c_yt;
extern double meters; /* this many meters per internal layout unit */
extern double femX, femY, femZ;
extern double csSIGMA, csTN;

extern int inScale, outScale, cs_ext;
extern int cs_bxl, cs_bxr, cs_byb, cs_byt;
extern int inSubTerm;
extern int tileCnt;
extern int tileConCnt;
extern int tileConCnt2;

#ifdef __cplusplus
  }
#endif
