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

static spider_t * freeList = NULL;
extern int numVertices;

spider_t * newSpider (meshCoor_t x, meshCoor_t y, meshCoor_t z, subnode_t *subnod, face_t *face)
{
    spider_t * spider;

    numVertices++;

    if (!(spider = freeList)) {
#define CHAIN_SIZE 1000
	freeList = NEW (spider_t, CHAIN_SIZE);
	spider = freeList + CHAIN_SIZE - 1;
	spider -> next = NULL;
	while (spider != freeList) {
	    --spider;
	    spider -> next = spider + 1;
	}
    }
    freeList = freeList -> next;

    CLEAR (spider, sizeof (spider_t));

    spider -> act_x = x;
    spider -> act_y = y;
    spider -> act_z = z;
    spider -> subnode = subnod;
    spider -> edge    = NULL;
    spider -> face    = face;
    spider -> moments = NULL;

    Debug (fprintf (stderr, "newSpider %p %g %g %g\n", spider,
	D(spider -> act_x), D(spider -> act_y), D(spider -> act_z)));

    stripAddSpider (spider);
    return (spider);
}

void disposeSpider (spider_t *spider)
{
    if (spider -> moments) disposeSpiderMoments (spider);
    spider -> next = freeList;
    freeList = spider;
}

/* Return the distance between sp1 and sp2
 */
meshCoor_t spiderDist (spider_t *sp1, spider_t *sp2)
{
    double x = sp2 -> act_x - sp1 -> act_x;
    double y = sp2 -> act_y - sp1 -> act_y;
    return sqrt (x * x + y * y);
}

#ifdef DEBUG
/*
 * Print spider to stderr (to be called from a debugger).
 */
void psp (spider_t *sp) { pspf (stderr, sp); }

void pspf (FILE *fp, spider_t *sp)
{
    if (!sp) { fprintf (fp, "null spider\n"); return; }
    fprintf (fp, "spider xyz %g %g %g", D(sp -> act_x), D(sp -> act_y), D(sp -> act_z));
    if (sp -> face) fprintf (fp, " f: %p\n", sp -> face);
    else fprintf (fp, "\n");
}
#endif // DEBUG
