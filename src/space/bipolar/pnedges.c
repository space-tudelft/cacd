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

#include "src/space/bipolar/define.h"
#include "src/space/bipolar/extern.h"

/* Update the pnEdges of <pn> if possible, otherwise create new pnEdge.
 */
void pnEdgeAdd (polnode_t *pn, pnEdge_t *b, int orientation)
{
    register pnEdge_t *e;

    for (e = pn -> edges; e; e = e -> next) {
	if (e -> edgeOrien == orientation) {
	    if (e -> edgeOrien == 'h') { /* horizontal edge */
		if (e -> y1 == b -> y1) {
		    if (e -> x1 == b -> x2) { e -> x1 = b -> x1; return; }
		    if (e -> x2 == b -> x1) { e -> x2 = b -> x2; return; }
		}
	    }
	    else { /* vertical edge */
		if (e -> x1 == b -> x1) {
		    if (e -> y1 == b -> y2) { e -> y1 = b -> y1; return; }
		    if (e -> y2 == b -> y1) { e -> y2 = b -> y2; return; }
		}
	    }
	}
    }

    e = NEW (pnEdge_t, 1);

    e -> edgeOrien = orientation;
    e -> x1 = b -> x1;
    e -> y1 = b -> y1;
    e -> x2 = b -> x2;
    e -> y2 = b -> y2;

    e -> next = pn -> edges;
    pn -> edges = e;
}

void pnEdgeDel (polnode_t *pn, pnEdge_t *e)
{
    register pnEdge_t *eh = pn -> edges;

    if (eh == e) pn -> edges = e -> next;
    else {
	while (eh -> next != e) eh = eh -> next;
	eh -> next = e -> next;
    }
    DISPOSE (e, sizeof(pnEdge_t));
}
