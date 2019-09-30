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

#include "src/space/auxil/auxil.h"

struct dat {
    int mapnumber;
    int amount;
};

struct summary {
    char * mapname;
    int maxalloc;
    int allocced;
    int freed;
    int allocs;
    int frees;
};

#define SUMTABSIZE 1000

struct summary *sumtab[SUMTABSIZE];

Private int compar (const void *e1, const void *e2)
{
    struct summary **s1 = (struct summary **) e1;
    struct summary **s2 = (struct summary **) e2;

    return ((*s2) -> maxalloc - (*s1) -> maxalloc);
}

Private void sortSumTab (struct summary **tab, int nel)
{
    qsort ((void *) tab, (size_t) nel, sizeof (*tab), compar);
}

/* See the comments in new.c on malloc_p and friends.
 * Not finished yet.
 * E.g, need to print an explanation of
 * the different fields in the summary output.
 */
int main (int argc, char *argv[])
{
    int lastmap;
    char buf[200];
    const char *basename;
    FILE *fpmap = NULL;
    FILE *fpdat = NULL;
    int i, len, width;
    struct dat item;

    /* Do this or otherwise the data files
     * you want to analyse are junked.
     */
    memprofTurnOff ();

    if (argc > 1)
	basename = argv[1];
    else if ((basename = getenv ("MEMPROF")) == NULL)
	basename = "malloc";

    fpmap = cfopen (mprintf ("%s.map", basename), "r");
    fpdat = cfopen (mprintf ("%s.dat", basename), "r");

    sumtab[0] = NEW (struct summary, 1);
    sumtab[0] -> mapname = strsave ("TOTAL");

    width = 20;

    /* start at 1: we have reserved postion zero for the totals */
    for (i = 1; i < SUMTABSIZE; i++) {
	if (fgets (buf, sizeof (buf), fpmap) == NULL) break;
	len = strlen (buf) - 1;
	if (len > width) width = len;
	buf[len] = '\0';
	sumtab[i] = NEW (struct summary, 1);
	sumtab[i] -> mapname = strsave (buf);
    }
    lastmap = i-1;

    while (fread (&item, sizeof (item), 1, fpdat) == 1) {
	/* we have reserved postion zero for the totals */
	item.mapnumber++;
	ASSERT (item.mapnumber < SUMTABSIZE);
	if (item.amount > 0) {
	    sumtab[item.mapnumber] -> allocced += item.amount;
	    sumtab[item.mapnumber] -> allocs   += 1;
	    sumtab[0] -> allocced += item.amount;
	    sumtab[0] -> allocs   += 1;
	}
	else {
	    sumtab[item.mapnumber] -> freed    -= item.amount;
	    sumtab[item.mapnumber] -> frees    += 1;
	    sumtab[0] -> freed    -= item.amount;
	    sumtab[0] -> frees    += 1;
	}
	sumtab[item.mapnumber] -> maxalloc = Max (
	    sumtab[item.mapnumber] -> maxalloc,
	    sumtab[item.mapnumber] -> allocced -
	    sumtab[item.mapnumber] -> freed);

	sumtab[0] -> maxalloc = Max (
	    sumtab[0] -> maxalloc,
	    sumtab[0] -> allocced -
	    sumtab[0] -> freed);
    }

    fprintf (stdout, "%-*s %10s %10s %10s %10s %10s\n", width,
	"", "bytes", "bytes", "bytes", "chunks", "chunks");

    fprintf (stdout, "%-*s %10s %10s %10s %10s %10s\n", width,
	"", "occupied", "alloc'ed", "leaking", "alloc'ed", "leaking");

    fprintf (stdout, "%-*s %10s %10s %10s %10s %10s\n", width,
	"", "==========", "==========", "==========", "==========", "==========");

    sortSumTab (sumtab, lastmap + 1);

    for (i = 0; i <= lastmap; i++) {
	fprintf (stdout, "%-*s %10d %10d %10d %10d %10d\n", width,
	    sumtab[i] -> mapname,
	    sumtab[i] -> maxalloc,
	    sumtab[i] -> allocced,
	    sumtab[i] -> allocced - sumtab[i] -> freed,
	    sumtab[i] -> allocs,
	    sumtab[i] -> allocs - sumtab[i] -> frees
	    );
    }
    return (0);
}
