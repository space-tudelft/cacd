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

#include "src/space/spider/export.h"
#include "src/space/spider/displace.h"
#include "src/space/spider/recog.h"

#ifdef __cplusplus
  extern "C" {
#endif

/* cap3d.c */
void schurRowOut (int row, schur_t *buf, int ncols);
void cap3dUpdateStatistics (spider_t *sp1, spider_t *sp2);

/* convex.c */
bool_t convexFace (face_t *face);

/* dogreen.c */
schur_t doGreen (int mode, spider_t *s1, spider_t *s2);

/* edge.c */
spiderEdge_t *newSpiderEdgePair (void);
void disposeSpiderEdge (spiderEdge_t *e);
void psef (FILE *f, spiderEdge_t *se);
void pse (spiderEdge_t *se);

/* face.c */
face_t *newFace (void);
void disposeFace (face_t *f);
int faceInitEnumerateR (void);
void faceInitEnumerate (void);
face_t *faceEnumerate (void);
void removeFace (face_t *face);
void setLastFace (void);
void stripAddFace (face_t *face, strip_t *strip);
void stripFreeFaces (strip_t *strip);

/* matherr.c */
void mathInit (void);
void mathStop (void);

/* mesh.c */
spider_t *meshCwAdjacent (spider_t *sp, face_t *face);
spiderEdge_t *meshFindEdge (spider_t *sp1, spider_t *sp2);
spiderEdge_t *meshMakeEdge (spider_t *sp1, spider_t *sp2, edgeType_t type);
spider_t *meshSplitEdge (spider_t *sp1, spider_t *sp2,
			meshCoor_t x, meshCoor_t y, meshCoor_t z,
			meshCoor_t nom_x, meshCoor_t nom_y, meshCoor_t nom_z);
void meshSetFace (spider_t *sp1, spider_t *sp2, face_t *left);
void meshSetFaces (spider_t *sp1, spider_t *sp2, face_t *left, face_t *right);
void meshSetCorners (face_t *face, spider_t *sp1, spider_t *sp2, spider_t *sp3, spider_t *sp4);
int isEdgeFace (face_t *face);
void meshCheckTriangle (spider_t *sp1, spider_t *sp2, spider_t *sp3, face_t *face);
void meshCheckLoop (face_t *face);
void meshPrintFace (face_t *face);
void pstar (spider_t *sp1);

/* pqueue.c */
void pqInsert (face_t *face);
face_t *pqDeleteHead (void);

/* recogm.c */
meshDef_t **recogMesh (tile_t *tile_l, tile_t *tile_r, int *n);

/* refine.c */
face_t *meshRenameFace (spider_t *sp1, spider_t *sp2, face_t *fc);
void faceRecur (face_t *face);
void feSize (face_t *face);
void meshRefine (void);
meshCoor_t weirdoSlope (spider_t *sp1, spider_t *sp2);

/* sphash.c */
int spiderHashInitTable (void);
void spiderHashFreeTable (void);
void spiderHashInsert (spider_t *spider);
spider_t *spiderHashLookUp (meshCoor_t x, meshCoor_t y, meshCoor_t z);
void spiderHashRemove (spider_t *spider);
void hashStatistics (FILE *fp);

/* spider.c */
spider_t *newSpider (meshCoor_t nom_x, meshCoor_t nom_y, meshCoor_t nom_z,
                        meshCoor_t x, meshCoor_t y, meshCoor_t z, int level,
			subnode_t *subnode, subnode_t *subnode2, int conductor, int isGate);
void disposeSpider (spider_t *spider);
void psp (spider_t *sp);
void pspf (FILE *fp, spider_t *sp);

/* sputil.c */
int spNomLinear (spider_t *sp1, spider_t *sp2, spider_t *sp3);
int spActLinear (spider_t *sp1, spider_t *sp2, spider_t *sp3);

/* strip.c */
void stripInit (coor_t xl, coor_t xr);
void stripMove (coor_t left, coor_t right);
void stripStop (void);
void stripVerbose (int mode);
void stripAddSpider (spider_t *spider);
void stripRemoveSpider (spider_t *spider);
void stripAddEdge (spiderEdge_t *edge);
bool_t inStripR (face_t *face);
char * sprintfStripRcoor (void);
void stripTraverse (int dmode, int *maxRow, int *maxCol);
void stripDrawAndPrint (void);

/* triang.c */
void triangulate (face_t *face);

/* displace.c */
void spiderDelayedDisplaceInit (void);
int  spiderDelayedDisplace (displ_t type, spider_t *sp1, spider_t *sp2, meshCoor_t delta, int level, slope_t direction);
void spiderDisplaceNow (void);
void spiderDisplaceRemove (spider_t *sp1, spider_t *sp2, int level);

#ifdef __cplusplus
  }
#endif

extern double new_microns;
extern double new_microns2;
