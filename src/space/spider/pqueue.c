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
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/spider/define.h"
#include "src/space/spider/recog.h"
#include "src/space/spider/extern.h"

#define swap(i,j) pq[0] = pq[i]; pq[i] = pq[j]; pq[j] = pq[0];

static int size = 0;		/* number of entries now in queue */
static int maxSize = 0;		/* max number of entries in queue */
static face_t **pq = NULL;     	/* head of queue */

/* Priority queue module cf Bentley, programming pearls,
 * communictions of the ACM, March 1985, volume 28, no 3, pp 245-250.
 *
 * The largest element is at the top of the heap, implemented by
 * swapping < and > as compared to paper.
 *
 * This module assumes heap is in pq[1...N], pq[0] is unused for the heap,
 * but is used for temporary storage.  Thus N+1 entries are allocated.
 */

Private int pqCompare (face_t *f1, face_t *f2)
{
    if (f1 -> len > f2 -> len) return (1);
    if (f1 -> len < f2 -> len) return (-1);
    if (f1 -> area > f2 -> area) return (1);
    if (f1 -> area < f2 -> area) return (-1);
    return (0);
}

/* Insert new element in priority queue module.
 */
void pqInsert (face_t *face)
{
    register int i, p;

    if (size == maxSize) { /* grow the queue */
	if (maxSize == 0) {
	    maxSize = 1000;
	    pq = NEW (face_t *, maxSize + 1);
	}
	else { /* double the size of the heap */
	    pq = GROW (face_t *, pq,  maxSize + 1, 2 * maxSize + 1);
	    maxSize += maxSize;
	}
    }

    Debug (fprintf (stderr, "pqInsert area=%g\n", (double) face -> area));

    ASSERT (face -> pqIndex == 0); /* not in pq */
    pq[++size] = face;
    face -> pqIndex = 1; /* face in pq */

    i = size; /* siftup */
    while ((p = i / 2) >= 1) { /* p is parent */
	if (pqCompare (pq[p], pq[i]) >= 0) break;
	swap (p, i);
	i = p;
    }
}

/* Delete top most (largest) element from pq.
 */
face_t *pqDeleteHead ()
{
    face_t *face;
    register int i, c;

    if (size < 1) return (NULL); /* pq empty */

    face = pq[1];
    pq[1] = pq[size--];

    i = 1; /* siftdown */
    while ((c = i + i) <= size) { /* c is left child of i */
	if (c+1 <= size		/* c+1 is right child of i */
	&& (pqCompare (pq[c+1], pq[c]) > 0)) c += 1;

	/* c is least child of i */
	if (pqCompare (pq[i], pq[c]) >= 0) break;
	swap (i, c);
	i = c;
    }

    Debug (fprintf (stderr, "pqDelete area=%g\n", (double) face -> area));

    face -> pqIndex = 0; /* face not in pq */
    return (face);
}
