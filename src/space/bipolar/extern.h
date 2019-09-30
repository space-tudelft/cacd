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

#include "src/space/bipolar/export.h"

#ifdef __cplusplus
  extern "C" {
#endif

/* bipotile.c */
void mkConnect (subnode_t *snA, subnode_t *snB, elemDef_t *el);

/* devices.c */
BJT_t * newVBJT (subnode_t *snC, subnode_t *snB, subnode_t *snE, subnode_t *snS,
		struct elemDef *type, double dim, int sidewall);
BJT_t * newLBJT (polnode_t *pn, subnode_t *snB, subnode_t *snE, subnode_t *snS,
		struct elemDef *type, int econ, coor_t basew);
pnTorLink_t * parBJT (pnTorLink_t *tp, pnTorLink_t *tl);
void pnTorReLink (polnode_t *pnB, polnode_t *pn);
void deviceDel (polnode_t *pn, pnTorLink_t *tl);

/* junction.c */
void junAdd (polnode_t *pnA, polnode_t *pnB);
void junctionDel (junction_t *j, polnode_t *pn);

/* nodelink.c */
void nodeLinksDel (polnode_t *pn);

/* pnedges.c */
void pnEdgeAdd (polnode_t *pn, tileBoundary_t *bdr, int orientation);
void pnEdgeDel (polnode_t *pn, pnEdge_t *e);

/* polnode.c */

#ifdef __cplusplus
  }
#endif
