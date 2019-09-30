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

/* update.c */
void tileInsertEdge (edge_t *edge);
void tileDeleteEdge (edge_t *edge);
void tileCrossEdge (coor_t thisX, edge_t *edge);
void tileAdvanceScan (void);

/* edge.c */
edge_t *rcreateEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
void rdisposeEdge (edge_t *edge);
void rprintEdge (char *s, edge_t *edge);
void edgeStatistics (FILE *fp);

/* input.c */
#ifdef DM_MAXNAME
void openInput (DM_CELL *cellKey, mask_t *colorp);
#endif
void closeInput (void);
edge_t *rfetchEdge (void);

/* main.c */
extern edge_t *shrinkEdge[2];
extern int shrinkEdgeIndex;
void die (void);

/* tile.c */
tile_t *createTile (coor_t xl, coor_t bl, coor_t xr, coor_t br);
void disposeTile (tile_t *tile);
void printTile (char *s, tile_t *tile);
void tileStatistics (FILE *fp);

/* scan.c */
void rscan (void);
void rscanPrintInfo (FILE *fp);
void scan (void);
void scanPrintInfo (FILE *fp);

/* slant.c */
void rtestIntersection (edge_t *e1, edge_t *e2);
void rsplit (edge_t *e1, coor_t xi, coor_t yi);

/* extract.c */
extern tile_t * neggrow_list;
void initExtract (char *cell, int scale);
void endExtract (void);
void enumPair (tile_t *tile, tile_t *newerTile, int orientation);
void enumTile (tile_t *tile);

/* makesize.c */
void growLayout (tile_t *tile, coor_t grow);
void openDebugEps (char *cell, char *newmask);
void closeDebugEps (void);

/* makegln/output.c */
void outputCleanUp (void);
void outputEdge (edge_t *e);
void outputPrintInfo (FILE *fp);
#ifdef DM_MAXNAME
void openOutput (DM_CELL *cellKey, char *mask);
#endif
void closeOutput (void);

/* makegln/sort.c */
void sortPrintInfo (FILE *fp);

/* extract/gettech.cc */
#ifdef DM_MAXNAME
void getTechnology (DM_PROJECT *dmproject, char *techDef, char *techFile);
char *giveICD (char *filepath);
extern char *usedTechFile;
#endif

/* From other places !!! */
extern bool_t optScale;
extern char * argv0;
extern int nrOfMasks;
extern maskinfo_t * masktable;
extern mask_t cNull;
extern mask_t idcolor;
extern coor_t growSize;
extern int growPos;
extern int newMask;
extern int resizeIndex;
extern int secondTime;

extern int nrOfResizes;
extern resizeData_t *resizes;

#ifdef __cplusplus
  }
#endif
