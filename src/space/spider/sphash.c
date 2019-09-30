/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Frederik Beeftink
 *	Frode Fjeld
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
#include <math.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"
#include "src/space/lump/define.h"
#include "src/space/lump/extern.h"
#include "src/space/spider/extern.h"
#include "src/space/spider/define.h"

extern bool_t spider_hash;
static int numHashed;
static int numHashHits;
static int sHtableSize;
static int sHnumSpiders;
static int sHmaxSpiders;
static int sHmersenneExp;
static spider_t **spiderHashTable;
static spider_t *NIL = (spider_t *)1;

Private int spiderHashFunction (meshCoor_t x, meshCoor_t y, meshCoor_t z)
{
    double f = (129.4 * (double)x + 19.83 * (double)y + 43.72 * (double)z);

    return ((unsigned int)f % sHtableSize);
}

int spiderHashInitTable ()
{
    int i;

    ASSERT (!spiderHashTable);
    if (!spider_hash) return 0;

    numHashed = 0;
    numHashHits = 0;
    sHnumSpiders = 0;
    sHmaxSpiders = 0;
    sHmersenneExp = 13;
    sHtableSize = (1 << sHmersenneExp) - 1;
    if (sHtableSize <= 0) return -1;
    if (!(spiderHashTable = NEW (spider_t *, sHtableSize))) return -1;

    for (i = 0; i < sHtableSize; i++) spiderHashTable[i] = NIL;

    Debug ((void) fprintf (stderr, "Hashtable init: size %d\n", sHtableSize));
    return 0;
}

void spiderHashFreeTable ()
{
    spider_t **old_table = spiderHashTable;

    if (!spiderHashTable) return;
    Debug ((void) fprintf (stderr, "Hashtable free: size %d (spiders %d)\n", sHtableSize, sHnumSpiders));

    spiderHashTable = NULL;

    if (sHnumSpiders > 0) { /* some spiders left in hashtable */
	spider_t *sp;
	int i, num = 0;
	//fprintf (stderr, "-- warning: %d spiders left in hashtable (size=%d)\n", sHnumSpiders, sHtableSize);
	for (i = 0; i < sHtableSize; i++) {
	    for (sp = old_table[i]; sp != NIL; sp = sp -> hashNext) {
		//fprintf (stderr, "-- spider(%g %g %g) in slot %d num=%d\n", sp -> nom_x, sp -> nom_y, sp -> nom_z, i, num+1);
		disposeSpider (sp); /* spiderHashRemove */
		if (++num == sHnumSpiders) goto ret;
	    }
	}
    }
ret:
    DISPOSE (old_table, sizeof(spider_t *) * sHtableSize);
}

Private void spiderHashRehash ()
{
    int i, k, old_size;
    spider_t **old_table, *spider, *next;

    if (sHmersenneExp >= 20) return; /* Unable to rehash */

    old_size = sHtableSize;
    old_table = spiderHashTable;

    sHmersenneExp += (sHmersenneExp >= 19 ? 1 : 2);
    sHtableSize = (sHmersenneExp == 15 ? 32749 : (1 << sHmersenneExp) - 1);

    if (sHtableSize > old_size)
	spiderHashTable = NEW (spider_t *, sHtableSize);
    else
	spiderHashTable = NULL;

    if (!spiderHashTable) { /* Unable to rehash */
        sHmersenneExp = 20;
        sHtableSize = old_size;
        spiderHashTable = old_table;
	Debug ((void) fprintf (stderr, "Unable to rehash from %d elements.\n", old_size));
        return;
    }

    for (i = 0; i < sHtableSize; i++) spiderHashTable[i] = NIL;

    for (i = 0; i < old_size; i++) {
	next = old_table[i];
	while ((spider = next) != NIL) {
	    next = spider -> hashNext;
	    /* Insert spider in HashTable */
	    k = spiderHashFunction (spider -> nom_x, spider -> nom_y, spider -> nom_z);
	    spider -> hashNext = spiderHashTable[k];
	    spiderHashTable[k] = spider;
	}
    }

    DISPOSE (old_table, sizeof(spider_t *) * old_size);

    Debug ((void) fprintf (stderr, "Spider rehash from %d to %d elements.\n", old_size, sHtableSize));
}

void spiderHashInsert (spider_t *spider)
{
    int i;

    if (!spiderHashTable) return;

    i = spiderHashFunction (spider -> nom_x, spider -> nom_y, spider -> nom_z);
    spider -> hashNext = spiderHashTable[i];
    spiderHashTable[i] = spider;

    Debug ((void) fprintf (stderr, "Hashed spider %p in slot %d\n", spider, i));
    ++numHashed;
    if (++sHnumSpiders >= sHtableSize) spiderHashRehash ();
    if (sHnumSpiders > sHmaxSpiders) sHmaxSpiders = sHnumSpiders;
}

spider_t * spiderHashLookUp (meshCoor_t x, meshCoor_t y, meshCoor_t z)
{
    spider_t *sp;
    int i;

    if (!spiderHashTable) return NULL;

    i = spiderHashFunction (x, y, z);
    for (sp = spiderHashTable[i]; sp != NIL; sp = sp -> hashNext) {
	if (sp->nom_z == z && Nearby(sp->nom_x,x) && Nearby(sp->nom_y,y)) {
	    ++numHashHits;
	    goto found;
	}
    }
    sp = NULL;
found:
    Debug ((void) fprintf (stderr, "hashLookup %s spider(%g %g %g) in slot %d\n", sp ? "of" : "NO", x, y, z, i));
    return sp;
}

void spiderHashRemove (spider_t *spider)
{
    spider_t *sp, *pp;
    int i;

    if (!spiderHashTable) return; /* used by spiderHashFreeTable */

    i = spiderHashFunction (spider -> nom_x, spider -> nom_y, spider -> nom_z);
    pp = NULL;
    sp = spiderHashTable[i];
    while (sp != spider && sp != NIL) sp = (pp = sp) -> hashNext;
    if (sp == spider) {
	if (pp)  pp -> hashNext = spider -> hashNext;
	else spiderHashTable[i] = spider -> hashNext;
	sHnumSpiders--;
    }
    else say ("warning: hashRemove: spider (%g %g %g) NOT found!\n", spider -> nom_x, spider -> nom_y, spider -> nom_z);
}

void hashStatistics (FILE *fp)
{
    if (spider_hash) {
	fprintf (fp, "overall hash statistics:\n");
	fprintf (fp, "\thashtable size      : %d bytes\n", sHtableSize * (int)sizeof (spider_t *));
	fprintf (fp, "\thashed spiders      : %d\n", numHashed);
	fprintf (fp, "\tmax spiders in hash : %d\n", sHmaxSpiders);
	fprintf (fp, "\tspiders left in hash: %d\n", sHnumSpiders);
	fprintf (fp, "\tnumber of hash hits : %d\n", numHashHits);
    }
}
