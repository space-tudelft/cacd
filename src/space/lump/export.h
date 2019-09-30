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

/*
 * operations exported by this module
 */

#ifdef __cplusplus
  extern "C" {
#endif
#ifdef DM_MAXNAME
void initLump (DM_CELL *cellCirKey, DM_CELL *cellLayKey);
#endif
void endLump (void);
void conAdd (subnode_t *subnA, subnode_t *subnB, double val, int sort);
void capAddS (subnode_t *subnA, subnode_t *subnB, double val);
void capAddSUB (subnode_t *sn, double val);
void conAddS (subnode_t *subnA, subnode_t *subnB, double val);
void conAddSUB (subnode_t *sn, double val);
void capAdd (subnode_t *subnA, subnode_t *subnB, double val, int sort);
void capAddNEG (subnode_t *subnA, subnode_t *subnB, double val, int sort);
void subnodeNew (subnode_t *subn);
void subnodeNew2 (subnode_t *subn, group_t *grp);
void subnodeJoin (subnode_t *subnA, subnode_t *subnB);
void subnodeCopy (subnode_t *subnA, subnode_t *subnB);
void subnodeCopy2 (subnode_t *subnA, subnode_t *subnB);
void subnodeJoin2 (subnode_t *subnA, subnode_t *subnB);
void subnodeDel2  (subnode_t *subn);
void warnSubnodeJoin (int cx, coor_t x, coor_t y, int mode);
void nameAdd (subnode_t *subn, terminal_t *term);
void groupNameAdd (group_t *grp, terminal_t *term);
void subnodeReconnect (subnode_t *subn, subnode_t *subn1, subnode_t *subn2, double val1, double val2);
void makeAreaNode (subnode_t *subn);
void makeLineNode (subnode_t *subn);
void subnodeDel (subnode_t *subn);
void subtorNew (tile_t *tile, elemDef_t *el);
void subtorJoin (tile_t *tileA, tile_t *tileB);
void subtorCopy (tile_t *tileA, tile_t *tileB);
void subtorDel (tile_t *tile);
void portAdd (subnode_t *subn, tile_t *tile, int port);
void subnodeTile (subnode_t *subn, tile_t *tile, int cx);
void subtorTile (tile_t *tile, int cx);
void torBoundary (transistor_t *tor, coor_t x1, coor_t y1, coor_t x2, coor_t y2, subnode_t *subn, int type);
void subnodeSubcontReconnect (subnode_t * subnSrc, subnode_t * subnDst, double frac);
void subnodeSubcontEmpty (subnode_t * subn);
void backInfoInstance (char *instName, coor_t dx, int nx, coor_t dy, int ny, coor_t xl, coor_t yb, coor_t xr, coor_t yt);
void nodeStatistics (FILE *fp);
void elementStatistics (FILE *fp);
void outVBJT (BJT_t *vT);
void outLBJT (BJT_t *lT);

#ifdef __cplusplus
  }
#endif

extern int warnSubnJoin;
