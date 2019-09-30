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
void initEdgeStatistics (void);
void edgeStatistics (FILE *fp);
void printEdge (char *s, edge_t *edge);
char *DX (coor_t x);

/* input.c */
edge_t *openInput (DM_CELL *cellKey);
void closeInput (void);
edge_t *fetchEdge (void);

/* main.c */
void die (void);

/* enumtile.c */
void enumPair (tile_t *tile, tile_t *newerTile, int edgeOrien);
void enumTile (tile_t *tile);
void clearTile (tile_t *tile);

/* gettech.c */
void getTechFile   (DM_PROJECT *dmproject, char *techDef, char *techFile);
void getTechnology (DM_PROJECT *dmproject, char *techDef, char *techFile);
char *giveICD (char *filepath);

/* tile.c */
tile_t *createTile (coor_t xl, coor_t bl, coor_t xr, coor_t br, int cc);
void disposeTile (tile_t *tile);
void printTile (char *s, tile_t *tile);
void initTileStatistics (void);
void tileStatistics (FILE *fp);

/* scan.c */
void scan (edge_t *);
void scanPrintInfo (FILE *fp);

/* slant.c */
void testIntersection (edge_t *e1, edge_t *e2);
edge_t *split (edge_t *e1);

/* subcont.c */
void initSubstr (DM_CELL *lkey);
void endSubstr  (void);
subcontRef_t *subContNew (tile_t *tile);
void subContJoin (tile_t *tileA, tile_t *tileB);
void subContDel (tile_t *tile);
void subContGroupJoin (subcont_t *scInfoA, subcont_t *scInfoB);

/* lump.c */
void initLump (void);
void endLump  (char *cellname);
node_t *createNode (void);
void disposeNode (node_t *n);
void elemAdd (node_t *nA, node_t *nB, double val);
void elemDel (element_t *el);

/* sub3d.c */
coor_t sub3dInit  (void);
coor_t sub3dStart (void);
coor_t sub3dStrip (void);
void   sub3dStop  (void);
void   sub3dEnd   (void);
void   sub3dStatistics (FILE *fp);

/* spedge.c */
spiderEdge_t *newSpiderEdgePair (void);
void disposeSpiderEdge (spiderEdge_t *e);
void pse  (spiderEdge_t *se);
void psef (FILE *fp, spiderEdge_t *se);

/* sptile.c */
void spiderTile (tile_t *tile);

/* sppair.c */
void spiderPair (tile_t *tile, tile_t *newerTile, int orientation);

/* face.c */
face_t *newFace  (void);
void disposeFace (face_t *f);
void faceInitEnumerate (void);
face_t *faceEnumerate  (void);
void freeFaces (void);

/* mesh.c */
void meshMakeEdge (spider_t *sp1, spider_t *sp2, edgeType_t type);
void meshSetFace  (spider_t *sp1, spider_t *sp2, face_t *left);
void meshSetFaces (spider_t *sp1, spider_t *sp2, face_t *left, face_t *right);
void meshSetCorners (face_t *face, spider_t *sp1, spider_t *sp2, spider_t *sp3, spider_t *sp4);
spiderEdge_t *meshFindEdge (spider_t *sp1, spider_t *sp2);
spider_t *meshCcwAdjacent (spider_t *sp, face_t *face, face_t *newface);
spider_t *meshCwAdjacent  (spider_t *sp, face_t *face);
spider_t *meshSplitEdge (spider_t *s1, spider_t *s2, meshCoor_t x, meshCoor_t y, meshCoor_t z);
int  isEdgeFace    (face_t *face);
void meshPrintFace (face_t *face);

/* pqueue.c */
void pqInsert (face_t *face);
void pqChange (face_t *face);
face_t *pqDeleteHead (void);
bool_t pqEmpty (void);

/* refine.c */
void meshRefine (void);
void feSize (face_t *face);

/* spider.c */
void disposeSpider  (spider_t *spider);
spider_t *newSpider (meshCoor_t x, meshCoor_t y, meshCoor_t z, subnode_t *subnode, face_t *face);
meshCoor_t spiderDist (spider_t *sp1, spider_t *sp2);
void psp  (spider_t *sp);
void pspf (FILE *fp, spider_t *sp);

/* strip.c */
void stripInit (coor_t left, coor_t right);
void stripMove (coor_t left, coor_t right);
void stripStop (void);
void stripVerbose (int mode);
void stripAddEdge (spiderEdge_t *edge);
void stripAddSpider  (spider_t *spider);
void stripFreeSpider (spider_t *spider);
spider_t *stripFindSpider (meshCoor_t x, meshCoor_t y, meshCoor_t z);
bool_t inStripA (face_t *face);
char * sprintfStripRcoor (void);
void stripTraverse (int mode);
void printStrips (void);

/* green/green.c */
double green (spider_t *sp1, spider_t *sp2);

/* green/init.cc */
double greenInit (double meters);
void greenStatistics (FILE *fp);
void printIfNoconverge (void);

/* green/mpgreen.cc */
void disposeSpiderMoments (spider_t *spider);

/* schur/print.c */
void printUpperMatrix (FILE *fp, int k, schur_t *r, int ord);

/* schur/schur.c */
void initSchur  (int maxd, int maxo);
void schurRowIn (int kr, schur_t *r, int ord);
void schurStatistics (FILE *fp);

/* From other places !!! */
extern char * argv0;
extern mask_t cNull;
extern int inSubTerm;

extern int greenCase;
extern int greenType;
extern int FeShape;
extern bool_t FeModePwl;

extern bool_t eliminateSubstrNode;
extern bool_t optEstimate3D;
extern bool_t optTime;
extern bool_t optInfo;
extern bool_t optVerbose;
extern coor_t bandWidth;
extern coor_t bbxl, bbxr, bbyb, bbyt;
extern double meters; /* this many meters per internal layout unit */

extern int tileCnt;
extern int tileConCnt;

#ifdef __cplusplus
  }
#endif
