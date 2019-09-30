/*
 * ISC License
 *
 * Copyright (C) 1994-2018 by
 *	Frederik Beeftink
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

#include <math.h>
#include <stdio.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"
#include "src/space/lump/extern.h"
#include "src/space/scan/export.h"

#include "src/space/bipolar/define.h"
#include "src/space/bipolar/extern.h"

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
#ifndef CONFIG_SPACE2
Private tileRef_t * tileInfoAdd (tileRef_t *tiles, tile_t *tile, int cx);
#endif
#ifdef __cplusplus
  }
#endif

/* bipoTile (elem, tile, surface)
 *
 * recognizes elements that are related to the surface
 * of <tile> and updates the data-structure.
 */
void bipoTile (tile_t *tile, elemDef_t **elem, double surface)
{
    BJT_t *tor;
    elemDef_t *el;
    pnconElemDef_t *ced;
    bjtorElemDef_t *ted;
    subnode_t **cons, *snSub, *snB, *snC, *snE;
    polnode_t *pn;
    int cx;

    cons = tile -> cons;

    while ((el = *elem++)) {

	switch (el -> type) {

	case RESELEM:

	    if ((cx = el -> s.res.con) >= nrOfCondStd) break;

	    if ((pn = cons[cx] -> pn) && pn -> length > 0) { /* LBJT */
		pn -> area += surface;
		if (tile -> xl < pn -> xl) pn -> xl = tile -> xl;
		if (tile -> bl < pn -> yb) pn -> yb = tile -> bl;
#ifndef CONFIG_SPACE2
		if (optBackInfo > 1) pn -> tiles = tileInfoAdd (pn -> tiles, tile, cx);
#endif
	    }
	    break;

	case PNCONELEM :

	    ced = &el -> s.pnc;
	    if ((cx = ced -> con1) >= nrOfCondStd) break;
	    if (extrPass) ++el -> el_recog_cnt;

	    joiningCon = cx; /* join subnodes */
	    mkConnect (cons[cx], cons[ced -> con2], el);
	    break;

	case VBJTELEM :
	    if (!extrPass) break;
	    ++el -> el_recog_cnt;

	    ted = &el -> s.bjt;

	    snSub = NULL;
	    if (ted -> sCon != -1) {
		if (ted -> sCon >= 0) snSub = cons[ted -> sCon];
		else {
		    if (substrRes) {
			if (tile -> subcont) snSub = tile -> subcont -> subn;
		    }
		    else
			snSub = subnSUB;
		}
		if (!snSub) fprintf (stderr, "warning: missing bulk conductor for VBJT\n");
	    }

	    if (!(snC = cons[ted -> cCon]))
		missingCon (ted -> cMask, SURFACE, tile, (tile_t*)NULL, (tile_t*)NULL, el, tile -> xl, tile -> bl);
	    if (!(snB = cons[ted -> bCon]))
		missingCon (ted -> bMask, SURFACE, tile, (tile_t*)NULL, (tile_t*)NULL, el, tile -> xl, tile -> bl);
	    if (!(snE = cons[ted -> eCon]))
		missingCon (ted -> eMask, SURFACE, tile, (tile_t*)NULL, (tile_t*)NULL, el, tile -> xl, tile -> bl);
	    tor = newVBJT (snC, snB, snE, snSub, el, surface, 0);

	    pn = snE -> pn;
	    if (tile -> xl < pn -> xl) pn -> xl = tile -> xl;
	    if (tile -> bl < pn -> yb) pn -> yb = tile -> bl;
#ifndef CONFIG_SPACE2
	    if (optBackInfo > 1) tor -> tiles = tileInfoAdd (tor -> tiles, tile, ted -> eCon);
#endif
	}
    }
}

#ifndef CONFIG_SPACE2
Private tileRef_t * tileInfoAdd (tileRef_t *tiles, tile_t *tile, int cx)
{
    tileRef_t *tileref = NEW (tileRef_t, 1);

    tileref -> tile = tile -> cnt;
    tileref -> cx = cx;
    tileref -> color = tile -> color;
    tileref -> xl = tile -> xl;
    tileref -> xr = tile -> xr;
    tileref -> bl = tile -> bl;
    tileref -> br = tile -> br;
    tileref -> tl = tile -> tl;
    tileref -> tr = tile -> tr;
    tileref -> next = tiles;
    return (tileref);
}
#endif

/* performs same actions as <bipoTile> only with
 * resistance-extraction active in this case.
 */
void resBipoTile (tile_t *tile, elemDef_t **elem, double surface)
{
    extern nodePoint_t *pTR;
    nodePoint_t *p;
    BJT_t *tor;
    elemDef_t *el;
    pnconElemDef_t *ced;
    bjtorElemDef_t *ted;
    subnode_t *snSub, *snB, *snC, *snE;
    polnode_t *pn;
    int cx;

    while ((el = *elem++)) {

	switch (el -> type) {

	case RESELEM:

	    if ((cx = el -> s.res.con) >= nrOfCondStd) break;

	    if ((pn = pTR -> cons[cx] -> pn) && pn -> length > 0) { /* LBJT */
		pn -> area += surface;
		if (tile -> xl < pn -> xl) pn -> xl = tile -> xl;
		if (tile -> bl < pn -> yb) pn -> yb = tile -> bl;
#ifndef CONFIG_SPACE2
		if (optBackInfo > 1) pn -> tiles = tileInfoAdd (pn -> tiles, tile, cx);
#endif
	    }
	    break;

	case PNCONELEM:

	    ced = &el -> s.pnc;
	    if ((cx = ced -> con1) >= nrOfCondStd) break;
	    ++el -> el_recog_cnt;

	    joiningCon = cx; /* join subnodes of all points (SdeG) */
	    for (p = pTR; p; p = p -> next)
		mkConnect (p -> cons[cx], p -> cons[ced -> con2], el);
	    break;

	case VBJTELEM :
	    ++el -> el_recog_cnt;

	    ted = &el -> s.bjt;

	    snSub = NULL;
	    if (ted -> sCon != -1) {
		if (ted -> sCon >= 0) snSub = pTR -> cons[ted -> sCon];
		else {
		    if (substrRes) {
			if (tile -> subcont) snSub = tile -> subcont -> subn;
		    }
		    else snSub = subnSUB;
		}
		if (!snSub) fprintf (stderr, "warning: missing bulk conductor for VBJT\n");
	    }

	    if (!(snC = pTR -> cons[ted -> cCon]))
		missingCon (ted -> cMask, SURFACE, tile, (tile_t*)NULL, (tile_t*)NULL, el, tile -> xl, tile -> bl);
	    if (!(snB = pTR -> cons[ted -> bCon]))
		missingCon (ted -> bMask, SURFACE, tile, (tile_t*)NULL, (tile_t*)NULL, el, tile -> xl, tile -> bl);
	    if (!(snE = pTR -> cons[ted -> eCon]))
		missingCon (ted -> eMask, SURFACE, tile, (tile_t*)NULL, (tile_t*)NULL, el, tile -> xl, tile -> bl);
	    tor = newVBJT (snC, snB, snE, snSub, el, surface, 0);

	    pn = snE -> pn;
	    if (tile -> xl < pn -> xl) pn -> xl = tile -> xl;
	    if (tile -> bl < pn -> yb) pn -> yb = tile -> bl;
#ifndef CONFIG_SPACE2
	    if (optBackInfo > 1) tor -> tiles = tileInfoAdd (tor -> tiles, tile, ted -> eCon);
#endif
	}
    }
}

void mkConnect (subnode_t *snA, subnode_t *snB, elemDef_t *el)
{
    if (!snA || !snB) {
	if (warnConnect & 2) return;
	warnConnect |= 2;
	say ("warning: invalid connect '%s': missing conductor subnode", el -> name);
	say ("\tfor conductor(s) at position %s.", strCoorBrackets (joiningX, joiningY));
	return;
    }
    if (snA -> cond -> type != snB -> cond -> type) {
	if (warnConnect & 1) return;
	warnConnect |= 1;
	say ("warning: invalid connect '%s': different conductor-type", el -> name);
	say ("\tfor conductors at position %s.", strCoorBrackets (joiningX, joiningY));
	return;
    }
    subnodeJoin (snA, snB);
}
