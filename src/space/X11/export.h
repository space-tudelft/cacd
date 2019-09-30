/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
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

/* draw.c */
void worldCoordinates (float xl, float xr, float yb, float yt);
int drawPairBoundary (coor_t xl, coor_t yl, coor_t xr, coor_t yr, int etype);
int drawEquiEdge     (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
int drawTriangleEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
int drawEdge (mask_t *colorp, coor_t xl, coor_t yl, coor_t xr, coor_t yr);
int drawTile (tile_t *tile, int hascond);
int drawScanPosition (coor_t x, coor_t y);
int drawResistor     (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
int drawOutResistor  (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
int undrawResistor   (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
int drawSubResistor  (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
int drawCapacitor    (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
int drawOutCapacitor (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
int undrawCapacitor  (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
#ifdef spider_h
int drawSpiderEdge (spider_t *sp1, spider_t *sp2, int contact);
#endif
int drawGreen (double x1, double y1, double z1, double x2, double y2, double z2);
#ifdef DM_MAXNAME
void drawDelaunay (DM_CELL *cellKey);
#endif

/* interact.c */
void initXpre (int *argc, char *argv[]);
void initX (int argc, char *argv[], char *optstring);
#ifdef DM_MAXNAME
void interactive (DM_PROJECT *project, char **cellNames);
#endif
void displayMode (char*);

#ifdef __cplusplus
}
#endif

extern int goptDrawEdge;
extern int goptDrawGreen;
extern int goptDrawPosition;
extern int goptDrawCapacitor;
extern int goptDrawResistor;
extern int goptDrawSpider;
extern int goptDrawSubContact;
extern int goptPairBoundary;
extern int goptOutResistor;
extern int goptOutCapacitor;
extern int goptUnDrawCapacitor;
extern int goptUnDrawResistor;
