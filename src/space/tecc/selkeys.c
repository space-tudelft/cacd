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
#include <signal.h>
#include <stdlib.h>

#include "src/space/tecc/define.h"
#include "src/space/tecc/extern.h"

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void countCond (struct layCondRef *cond);
#ifdef __cplusplus
  }
#endif

int maskWidth = 8;
static int *eMaskCond;
static int *sMaskCond;
static int *sMaskCond2;
static int noEdgeElts;
static int noSurfElts;

void selectKeys ()
{
    int i, j, nextMask, nextOccur, maxCnt;
    struct layerRef *nextKey, *lastKey;

    ALLOC (sMaskCond,  procdata -> nomasks + subdata -> nomasks, int);
    ALLOC (sMaskCond2, procdata -> nomasks + subdata -> nomasks, int);
    ALLOC (eMaskCond,  procdata -> nomasks + subdata -> nomasks, int);

    /* ALLOC does 'calloc' and inits to zero */

    for (i = 0; i < res_cnt; ++i) countCond (ress[i].cond);
    for (i = 0; i < tor_cnt; ++i) countCond (tors[i].cond);
    for (i = 0; i < bjt_cnt; ++i) countCond (bjts[i].cond);
    for (i = 0; i < jun_cnt; ++i) countCond (juns[i].cond);
    for (i = 0; i < cnt_cnt; ++i) countCond (cnts[i].cond);
    for (i = 0; i < con_cnt; ++i) countCond (cons[i].cond);
    for (i = 0; i < cap_cnt; ++i) countCond (caps[i].cond);
    for (i = 0; i < vdm_cnt; ++i) countCond (vdms[i].cond);
    for (i = 0; i < shp_cnt; ++i) countCond (shps[i].cond);
    for (i = 0; i < sbc_cnt; ++i) countCond (subconts[i].cond);
    for (i = 0; i < new_cnt; ++i) countCond (newmsks[i].cond);

if (prVerbose) {
    for (i = 0; i < procdata -> nomasks; ++i) {
	if ((j = strlen (procdata -> mask_name[i])) > maskWidth) maskWidth = j;
    }
    for (i = 0; i < subdata -> nomasks; ++i) {
	if ((j = strlen (subdata -> mask_name[i])) > maskWidth) maskWidth = j;
    }
    fprintf (stderr, "\nNumber of masks found in element conditions:\n");
    fprintf (stderr, "   m#  %-*s sMask# sMask2# eMask#\n", maskWidth, "maskname");
    for (i = maskWidth + 30; --i > 0;) fprintf (stderr, "-"); fprintf (stderr, "\n");
    for (i = 0; i < procdata -> nomasks; ++i) {
	fprintf (stderr, "(%4d) %-*s %6d %6d %6d\n",
	    i, maskWidth, procdata -> mask_name[i], sMaskCond[i], sMaskCond2[i], eMaskCond[i]);
    }
    for (i = maskWidth + 30; --i > 0;) fprintf (stderr, "-"); fprintf (stderr, "\n");
if (subdata -> nomasks > 0) {
    for (j = 0; j < subdata -> nomasks; ++j, ++i) {
	fprintf (stderr, "(%4d) %-*s %6d %6d %6d\n",
	    i, maskWidth, subdata -> mask_name[j], sMaskCond[i], sMaskCond2[i], eMaskCond[i]);
    }
    for (i = maskWidth + 30; --i > 0;) fprintf (stderr, "-"); fprintf (stderr, "\n");
}
    fprintf (stderr, "number of surface elements: %d\n", noSurfElts);
    fprintf (stderr, "number of edge elements: %d\n", noEdgeElts);
}

    Keylist = NULL;
    lastKey = NULL;
    maxKeys = 0;
    maxKeys2 = 0;

    while (1) {
	maxCnt = 0;
	nextMask = 0;

	/* edge layers have highest priority */

	nextOccur = EDGE;
	for (i = 0; i < procdata -> nomasks + subdata -> nomasks; ++i) {
	    if (eMaskCond[i] > maxCnt) {
		maxCnt = eMaskCond[i];
		nextMask = i;
	    }
	}
	if (maxCnt) { ++maxKeys2; goto add_it; }

	for (i = 0; i < procdata -> nomasks + subdata -> nomasks; ++i) {
	    if (sMaskCond2[i] > maxCnt) {
		maxCnt = sMaskCond2[i];
		nextMask = i;
	    }
	}
	if (maxCnt) { ++maxKeys2; goto add_it; }

	nextOccur = SURFACE;
	for (i = 0; i < procdata -> nomasks + subdata -> nomasks; ++i) {
	    if (sMaskCond[i] > maxCnt) {
		maxCnt = sMaskCond[i];
		nextMask = i;
	    }
	}
	if (!maxCnt) break;

	/* only surface & edge layers will be included in the Keylist */
add_it:
	++maxKeys;
	ALLOC (nextKey, 1, struct layerRef);
	ALLOC (nextKey -> lay, 1, struct layer);
	nextKey -> lay -> mask = nextMask;
	nextKey -> lay -> occurrence = nextOccur;

	eMaskCond[nextMask] = 0;
	sMaskCond[nextMask] = 0;
	sMaskCond2[nextMask] = 0;

	if (!Keylist) Keylist = nextKey;
	else  lastKey -> next = nextKey;
	lastKey = nextKey;
	lastKey -> next = NULL;
    }

    FREE (sMaskCond);
    FREE (sMaskCond2);
    FREE (eMaskCond);
}

Private void countCond (struct layCondRef *cond)
{
    struct layCondRef *c = cond;
    struct layer *lay;
    int edge = 0;

    while (cond) {
	lay = cond -> layC -> lay;

	if (lay -> occurrence   == SURFACE) ++sMaskCond[lay -> mask];
	else if (lay -> occurrence == EDGE){ ++eMaskCond[lay -> mask]; ++edge; }

	/* only surface & edge layers will be included in the Keylist */

	cond = cond -> next;
    }

    if (edge) { /* edge element */
	/* count surface conditions again for edge table */
	++noEdgeElts;
	do {
	    lay = c -> layC -> lay;
	    if (lay -> occurrence == SURFACE) ++sMaskCond2[lay -> mask];
	} while ((c = c -> next));
    }
    else
	++noSurfElts;
}
