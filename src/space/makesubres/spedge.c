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

#include "src/space/makesubres/define.h"
#include "src/space/makesubres/extern.h"

spiderEdge_t * newSpiderEdgePair ()
{
    /* Always want two halves at once, but allocate them separately
     * because sometimes they are split and not disposed together.
     */
    spiderEdge_t * e = NEW (spiderEdge_t, 1);
    spiderEdge_t * o = NEW (spiderEdge_t, 1);

    CLEAR (e, sizeof (spiderEdge_t));
    CLEAR (o, sizeof (spiderEdge_t));

    e -> oh = o;
    o -> oh = e;

    return (e);
}

void disposeSpiderEdge (spiderEdge_t *e)
{
    DISPOSE (e, sizeof(spiderEdge_t));
}

#ifdef DEBUG
/*
 * Print edge to stderr (to be called from a debugger).
 */
void pse (spiderEdge_t *se) { psef (stderr, se); }

void psef (FILE *fp, spiderEdge_t *se)
{
    if (se) {
	spider_t *sp1 = se -> sp;
	spider_t *sp2 = se -> oh -> sp;
	fprintf (fp, "spideredge %g %g %g <-> %g %g %g f1: %p f2: %p\n",
		D(sp1 -> act_x), D(sp1 -> act_y), D(sp1 -> act_z),
		D(sp2 -> act_x), D(sp2 -> act_y), D(sp2 -> act_z),
		se -> face, se -> oh -> face);
    }
    else fprintf (fp, "null edge\n");
}
#endif // DEBUG
