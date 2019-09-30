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

#define Val(f) D(f -> area)

#define swap(pq,i,j) { \
    pq[0] = pq[i]; pq[i] = pq[j]; pq[j] = pq[0]; \
    pq[i] -> pqIndex = i; pq[j] -> pqIndex = j; \
}

static int size;			/* number of entries now in queue */
static int maxSize = 0;			/* max number of entries in queue */
static face_t ** pq = NULL;     	/* head of queue */

#ifdef PQDEBUG
static face_t prev;
#endif

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void siftup   (int i);
Private void siftdown (int i);
Private int pqCompare (face_t *f1, face_t *f2);
#ifdef __cplusplus
  }
#endif

/* Priority queue module cf Bentley, programming pearls,
 * communictions of the ACM, March 1985, volume 28, no 3, pp 245-250.
 *
 * The largest element is at the top of the heap, implemented by
 * swapping < and > as compared to paper.
 *
 * This module assumes heap is in pq[1...N], pq[0] is unused for the heap,
 * but for temporary storage.
 * So N+1 entries are allocated.
 */

/* Insert new element in priority queue module.
 * The queue grows as needed.
 */
void pqInsert (face_t *face)
{
#ifdef PQDEBUG
    prev.area = INF;
    prev.len = INF;
#endif

    if (size == maxSize) {
	if (maxSize == 0) {
	    maxSize = 1000;
	    pq = NEW (face_t *, maxSize + 1);
	}
	else {
	    /* double the size of the heap */
	    pq = GROW (face_t *, pq,  maxSize + 1, 2 * maxSize + 1);
	    maxSize += maxSize;
	}
    }

    Debug (fprintf (stderr, "pqInsert %g\n", Val (face)));

    ASSERT (face -> pqIndex == 0);

    pq[++size] = face;
    face -> pqIndex = size;

    siftup (size);

    ASSERT (face -> pqIndex >= 1 && face -> pqIndex <= size);
}

/* Change priority of face
 */
void pqChange (face_t *face)
{
    Debug (fprintf (stderr, "pqChange %d\n", face -> pqIndex));

    siftup   (face -> pqIndex);
    siftdown (face -> pqIndex);
}

bool_t pqEmpty ()
{
    return size < 1;
}

/* Delete top most (largest) element from pq.
 */
face_t *pqDeleteHead ()
{
    face_t *face;

#ifdef PQDEBUG
    if (size < 1) {
	prev.area = INF;
	prev.len = INF;
    }
#endif

    if (size < 1) return (NULL);

    face = pq[1];
    ASSERT (face -> pqIndex == 1);

    pq[1] = pq[size--];
    pq[1] -> pqIndex = 1;

    siftdown (1);

    Debug (fprintf (stderr, "pqDelete %g\n", Val (face)));

#ifdef PQDEBUG
    ASSERT (pqCompare (&prev, face) >= 0);
    prev.area = face -> area;
    prev.len = face -> len;
#endif

    face -> pqIndex = 0;
    return (face);
}

Private void siftup (int i)
{
    register int p;		/* parent */

    for (;;) {
	if (i == 1) break;
	p = i / 2;
	if (pqCompare (pq[p], pq[i]) >= 0) break;

	ASSERT (p >= 1 && p <= size);
	ASSERT (i >= 1 && i <= size);

	swap (pq, p, i);

	ASSERT (pqCompare (pq[p], pq[i]) >= 0);
	i = p;
    }
}

Private void siftdown (int i)
{
    register int c;		/* child */

    for (;;) {
	c = i + i;		/* c is left child of i */
	if (c > size) break;
	if (c+1 <= size 		/* c + 1 is right child of i */
	&& (pqCompare (pq[c+1], pq[c]) > 0))
	    c += 1;

	/* c is least child of i */
	if (pqCompare (pq[i], pq[c]) >= 0) break;

	ASSERT (i >= 1 && i <= size);
	ASSERT (c >= 1 && c <= size);

        swap (pq, i, c);

	ASSERT (pqCompare (pq[i], pq[c]) >= 0);
	i = c;
    }
}

Private int pqCompare (face_t *f1, face_t *f2)
{
    if (f1 -> len > f2 -> len) return (1);
    if (f1 -> len < f2 -> len) return (-1);

    if (f1 -> area > f2 -> area) return (1);
    if (f1 -> area < f2 -> area) return (-1);

    return (0);
}
