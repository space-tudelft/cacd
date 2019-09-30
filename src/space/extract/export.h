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

extern char * usedTechFile;
extern double vdimZtop;
extern dielectric_t * diels;
extern substrate_t * substrs;
extern selfsubdata_t * selfs;
extern mutualsubdata_t * muts;
extern int diel_cnt;
extern int substr_cnt;
extern int self_cnt;
extern int mut_cnt;
extern int hasBipoElem;
extern int hasEdgeCaps;
extern int hasSurfCaps;

extern int nrOfCondStd;
extern int vdimIndex;
extern int vdimCount;
extern elemDef_t * elemDefTab;

extern resizeData_t * resizes;
extern int nrOfResizes;

extern bool_t extractDiffusionCap3d;
extern mask_t cNull;
extern mask_t filterBitmask;

/*
 * operations exported by this module
 */

#ifdef __cplusplus
  extern "C" {
#endif

/* clrtile.c */
void clearTile (tile_t *tile);

/* enumpair.c */
void enumPair (tile_t *tile, tile_t *newerTile, int edgeOrien);
void missingCon (int mask, int occurrence, tile_t *tile,
	tile_t *eTile, tile_t *oeTile, elemDef_t *el, coor_t x, coor_t y);
void missingSubarea (int mask, tile_t *tile, tile_t *eTile,
        elemDef_t *el, coor_t x, coor_t y);

/* enumtile.c */
void enumTile (tile_t *tile);

/* gettech.c */
#ifdef DM_MAXNAME
void getTechFile (DM_PROJECT *dmproject, char *techDef, char *techFile);
void getTechnology (DM_PROJECT *dmproject, char *techDef, char *techFile);
#endif
char *conNr2Name (int n);
int conName2Nr (char *name);
char *giveICD (char *filepath);
void printRecogCnt (void);
void pp1SetColors   (void);
void pp1ResetColors (void);

/* init.c */
#ifdef DM_MAXNAME
void initExtract (DM_CELL *circuitKey, DM_CELL *layoutKey);
#endif
void endExtract (void);
int peekTileXY (void);
void testTileXY (tile_t *tile, tile_t *tile_above, coor_t xr);
void advanceTileXY (coor_t x);

/* nodepnt.c */
void nodePointStatistics (FILE *fp);

/* recog.c */
void recogInit(void);
elemDef_t **recogEdge (tile_t *sTile, tile_t *eTile, elemDef_t **elemListIn);
elemDef_t **recogSurface (tile_t *sTile);
elemDef_t * hasLatCap (tile_t *sTile, tile_t *eTile);
elemDef_t * nextLatCap ();

#ifdef __cplusplus
  }
#endif /* __cplusplus */

