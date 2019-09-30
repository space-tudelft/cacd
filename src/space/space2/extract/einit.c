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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/export.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"
#include "src/space/lump/define.h"
#include "src/space/lump/export.h"
#include "src/space/substr/export.h"
#include "src/space/bipolar/define.h"

extern int hasBipoELEM;
int hasBipoElem = 0;
extern unsigned int nodePointSize;

int msgDoubleJuncCaps;
int condWarnCnt;
int tileCnt;
int tileConCnt;   /* number of tiles with at least one conductor */

tileBoundary_t boundary, *bdr;

/* The following is used by the supply short message
   to print the position where things are connected. */
int joiningCon = -1;
coor_t joiningX;
coor_t joiningY;

bool_t mergeNeighborSubContacts = 1;
bool_t *sep_on_res = NULL; /* Specifies whether derived substrate terminals
	must be separate when resistances are extracted for this conductor. */

int *helpArray;

void initExtract (DM_CELL *circuitKey, DM_CELL *layoutKey)
{
    char buf[128];
    int i;

    bdr = &boundary;

    if (!helpArray) helpArray = NEW (int, nrOfCondStd);

    if (!nodePointSize) {
	nodePointSize = sizeof (nodePoint_t)
		+ nrOfCondStd * sizeof (subnode_t *)
		+ nrOfCondStd * sizeof (subnode_t);
    }

    if ((optSubResSave || optSubRes) && !sep_on_res) {
	sep_on_res = NEW (int, nrOfCondStd);
	for (i = 0; i < nrOfCondStd; i++) {
	    sprintf (buf, "sub_term_distr_%s", masktable[conductorMask[i]].name);
	    sep_on_res[i] = paramLookupB (buf, "off");
	}
	mergeNeighborSubContacts = !paramLookupB ("sep_sub_term", "off");
    }

    hasBipoElem = extrPass ? hasBipoELEM : 0;

    initLump (circuitKey, layoutKey);

    if (prePass) initMeshEdge (layoutKey);
    if (substrRes) initSubstr (layoutKey);
    if (prePass1 && optSimpleSubRes) initContEdge (layoutKey);

    msgDoubleJuncCaps = 0;
    condWarnCnt = 0;
    tileCnt = 0;   /* first tile will have cnt = 1 */
    tileConCnt = 0;
}

void endExtract ()
{
    if (prePass) endMeshEdge ();
    if (substrRes) endSubstr ();
    if (prePass1 && optSimpleSubRes) endContEdge ();
    endLump ();
}
