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

#include <math.h>
#include <stdio.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"

static unsigned int allocs = 0;
static unsigned int frees  = 0;
static unsigned int max    = 0;
unsigned int nodePointSize = 0;

static nodePoint_t * freeList = NULL;

nodePoint_t * createNodePoint (coor_t x, coor_t y)
{
    nodePoint_t *point;

    if (!(point = freeList)) {
	point = (nodePoint_t *) MALLOC (nodePointSize);
	++max;
    }
    else freeList = freeList -> next;

    point -> fixed = 0;
    point -> x = x;
    point -> y = y;
    point -> next = NULL;
    point -> prev = NULL;
    point -> cons = (subnode_t **) (point + 1);

    ++allocs;

    return (point);
}

void disposeNodePoint (nodePoint_t *point)
{
    frees++;
    point -> next = freeList;
    freeList = point;
}

void nodePointStatistics (FILE *fp)
{
    fprintf (fp, "overall nodepoint statistics:\n");
    fprintf (fp, "\tpoints created     : %u\n", allocs);
    fprintf (fp, "\tpoints disposed    : %u\n", frees);
    fprintf (fp, "\tpoints allocated   : %u of %u byte\n", max, nodePointSize);
 // fprintf (fp, "\tmax points in core : %u\n", max);
}
