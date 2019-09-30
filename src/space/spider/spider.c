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
#include <string.h>		/* memset */
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/green/export.h"

#include "src/space/spider/recog.h"
#include "src/space/spider/extern.h"

extern int numVertices;
static spider_t * freeList = NULL;

/*
 * Create a new spider.
 * The new spider is inserted into the strip data structure (using stripAddSpider).
 */
spider_t * newSpider (meshCoor_t nom_x, meshCoor_t nom_y, meshCoor_t nom_z, meshCoor_t x, meshCoor_t y, meshCoor_t z,
	int level, subnode_t *subnod, subnode_t *subnod2, int conductor, int isGate)
{
    spider_t * spider;

    numVertices++;

    if (!freeList) {
	int i;
#define CHAIN_SIZE 1000
	freeList = NEW (spider_t, CHAIN_SIZE);
	for (i = 0; i < CHAIN_SIZE - 1; i++)
	    (freeList + i) -> next = (freeList + i + 1);
	(freeList + CHAIN_SIZE - 1) -> next = NULL; /* end of chain */
    }
    spider   = freeList;
    freeList = freeList -> next;

    CLEAR (spider, sizeof (spider_t));

    spider -> nom_x = nom_x;
    spider -> act_x = x;
    spider -> nom_y = nom_y;
    spider -> act_y = y;
    spider -> nom_z = nom_z;
#ifdef act_z
    ASSERT (z == nom_z);
#else
    spider -> act_z = z;
#endif
    spider -> subnode = subnod;
    spider -> subnode2 = subnod2;
    spider -> face = NULL;
    spider -> conductor = conductor;
    spider -> flags = 0;
    spider -> isGate = isGate;
    spider -> edge = NULL;
    spider -> moments = NULL;

    Debug (fprintf (stderr, "newSpider %p %g %g %g\n", spider, spider -> nom_x, spider -> nom_y, spider -> nom_z));

    if (level > 0) spiderHashInsert (spider);
#ifdef DEBUG
    else say ("Not-inserted spider\n");
#endif
 // stripAddSpider (spider); /* done somewhere else (SdeG) */

    return (spider);
}

void disposeSpider (spider_t *spider)
{
    if (spider -> hashNext) spiderHashRemove (spider);
#ifdef DEBUG
    spider -> edge = (void*)0xDEADBEEF;
    spider -> nom_x = spider -> nom_y = spider -> nom_z = -1;
    spider -> act_x = spider -> act_y = spider -> act_z = -1;
#endif
    spider -> next = freeList;
    if (spider -> moments) disposeSpiderMoments (spider);
    freeList = spider;
}

/*
 * print spider to stderr (to be called from a debugger).
 */
void psp (spider_t *sp)
{
    pspf (stderr, sp);
}

/*
 * print spider to file
 */
void pspf (FILE *fp, spider_t *sp)
{
    /*
     * Do not change this syntax, it is used by some scripts
     */
    if (sp) {
	fprintf (fp, "spider xyz %g %g %g act %g %g %g\n",
	    (double) sp -> nom_x, (double) sp -> nom_y, (double) sp -> nom_z,
	    (double) sp -> act_x, (double) sp -> act_y, (double) sp -> act_z);
    }
    else
	fprintf (fp, "null spider\n");
}

#ifdef DRIVER
FILE * spiderDebug = NULL;
void main ()
{
    int i;
    printf ("size: %d\n", sizeof (spider_t));
    for (i = 0; i < 16; i ++) {
	pspf (stdout, newSpider (1, 0, 0, 1, 0, 0, 0, NULL, NULL, i, FALSE));
    }
}
#endif /* DRIVER */
