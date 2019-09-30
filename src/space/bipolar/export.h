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

/* bipopair.c */
void bipoPair    (tile_t *tile_v, tile_t *tile_l, elemDef_t **el, int orientation);
void resBipoPair (tile_t *tile_v, tile_t *tile_l, elemDef_t **el, int orientation, nodePoint_t *np_v, nodePoint_t *np_l);

/* bipotile.c */
void bipoTile (tile_t *tile, elemDef_t **elem, double surface);
void resBipoTile (tile_t *tile, elemDef_t **elem, double surface);

/* devices.c */
void pnTorLinkAdd (polnode_t *pn, BJT_t *tor, int type);

/* junction.c */

/* nodelink.c */
void relinkNodeLink (node_t *nA, nodeLink_t *nL);
void nodeLinkAdd (node_t *n, polnode_t *pn);
void nodeLinkDel (nodeLink_t *nL);

/* pnedges.c */

/* polnode.c */
void polnodeAdd  (subnode_t *sn, int con, int type);
void polnodeCopy (subnode_t *snA, subnode_t *snB);
void polnodeJoin (polnode_t *pnA, polnode_t *pnB);
void polnodeDel  (polnode_t *pn);
void polnodeDispose (polnode_t *pn);

#ifdef __cplusplus
  }
#endif
