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

#include <stdio.h>
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"
#include "src/space/substr/export.h"
#include "src/space/lump/export.h"

/* clearTile (tile)
 *
 * will finish the influence of 'tile' on the network by
 * cleaning the related network datastructures.
 *
 * actions performed :
 *
 * - Subnodes for 'tile' are deleted.
 *   This will result in a possible elimination of nodes,
 *   or outputting of extracted circuit nodes and elements.
 */
void clearTile (tile_t *tile)
{
    register subnode_t **cons;
    register int i;

    if (prePass) {
#ifndef CONFIG_SPACE2
	if (prePass == 1) {
#endif
	    nodePoint_t *p, *q, *pn;
	    if ((pn = tile -> tlPoints)) {
		do { pn = (p = pn) -> next; disposeNodePoint (p); } while (pn);
		pn = tile -> rbPoints;
		do { pn = (q = pn) -> next; disposeNodePoint (q); } while (pn != p);
	    }
#ifndef CONFIG_SPACE2
	}
	if (prePass == 2) {
	    for (i = 0; i < tile -> known; ++i)
		if (tile -> cons[i]) subnodeDel2 (tile -> cons[i]);
	}
#endif
	return;
    }

    if (tile -> subcont) subContDel (tile);

    if (!optRes) {
	if (prePass1) return;
	cons = tile -> cons;
	for (i = 0; i < nrOfCondStd; ++i) {
	    if (cons[i]) {
#ifndef CONFIG_SPACE2
		if (optBackInfo > 1) subnodeTile (cons[i], tile, i);
#endif
		subnodeDel(cons[i]);
	    }
	}
    }
    else if (tile -> rbPoints) {
	nodePoint_t *point, *bl_point, *pnext;

	point = bl_point = tile -> rbPoints;
	pnext = point -> next;

	cons = point -> cons;
	for (i = 0; i < nrOfCondStd; ++i) {
	    if (cons[i]) {
#ifndef CONFIG_SPACE2
		if (optBackInfo > 1) subnodeTile (cons[i], tile, i);
#endif
		subnodeDel (cons[i]);
	    }
	}
	disposeNodePoint (point);

	while (pnext) {
	    point = pnext;
	    if (point == bl_point) break;
	    pnext = point -> next;

	    cons = point -> cons;
	    for (i = 0; i < nrOfCondStd; ++i) {
		if (cons[i]) subnodeDel (cons[i]);
	    }
	    disposeNodePoint (point);
	}

	ASSERT (point == tile -> tlPoints);
    }
}
