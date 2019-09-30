/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Theo Smedes (T.S.)
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

/* T.S. values for greenCase */
#define DIEL 0
#define SUBS 1
extern int greenCase;
extern int greenType;
extern int FeShape;
extern bool_t FeModePwl;
extern bool_t FeModeGalerkin;

#ifdef __cplusplus
  extern "C" {
#endif

/* cap3d.c */
coor_t cap3dInitParam (void);
#ifdef DM_MAXNAME
coor_t cap3dInit (DM_CELL *key);
#endif
void   cap3dStatistics (FILE *fp);
coor_t cap3dStart (void);
void   cap3dStop  (void);
coor_t cap3dStrip (void);
void   cap3dEnd   (void);
void   cap3dEstimate (FILE *fp);
coor_t findStripForCapStart (coor_t scanX);
coor_t findStripForSubStart (coor_t scanX);
coor_t findStripForCapNext  (coor_t scanX);
coor_t findStripForSubNext  (coor_t scanX);

/* sptile.c */
void spiderTile (tile_t *tile);

/* sppair.c */
void spiderPair (tile_t *tile, tile_t *newerTile, int orientation);

/* sputil.c */
meshCoor_t spiderDist (spider_t *sp1, spider_t *sp2);

/* mesh.c */
spider_t *meshCcwAdjacent (spider_t *sp, face_t *face, face_t *newface);

#ifdef __cplusplus
  }
#endif
