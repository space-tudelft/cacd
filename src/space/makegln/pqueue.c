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

#include "src/space/makegln/config.h"
#include <stddef.h>
#include <stdio.h>
#include "src/space/auxil/auxil.h"
#include "src/space/makegln/makegln.h"
#include "src/space/makegln/proto.h"

typedef struct pqEntry_t {
    _edge_t * edge;
    int       block;
} pqEntry_t;

#define swap(pq,i,j) {{ \
register _edge_t * E = pq[i].edge; pq[i].edge = pq[j].edge; pq[j].edge = E;} {\
register int   B = pq[i].block; pq[i].block = pq[j].block; pq[j].block = B;}}

/* a type for indexing in priority queue */
#define pqi_t ecnt_t

static pqi_t size;			/* number of entries now in queue */
static pqi_t maxSize = 0;		/* max number of entries in queue */
static pqEntry_t * pq = NULL;		/* head of queue */

/* local operations */
Private void siftup   (pqEntry_t *base, ecnt_t N);
Private void siftdown (pqEntry_t *base, ecnt_t N);

/* This module assumes heap is in pq[1...N], pq[0] is unused.
 * So allocate N+1 entries
 */
void pqInit (pqi_t N)
{
    size = 0;
    if (maxSize < N) {		/* re-initialization */
	if (pq) DISPOSE (pq, sizeof(pqEntry_t) * (maxSize+1));
	pq = NEW (pqEntry_t, N + 1);
	maxSize = N;
    }
}

void pqHead (_edge_t **ep, pqi_t *np)
{
    *ep = pq[1].edge;
    *np = pq[1].block;
}

void pqInsert (_edge_t *edge, int block)
{
    size++;

    ASSERT (size <= maxSize);

    pq[size].edge  = edge;
    pq[size].block = block;

    siftup (pq, size);
}

void pqReplaceHead (_edge_t *edge, int block)
{
    pq[1].edge  = edge;
    pq[1].block = block;

    siftdown (pq, size);
}

Private void siftup (pqEntry_t *base, pqi_t N)
{
    pqi_t i = N;
    pqi_t p;		/* parent */

    for (;;) {
	if (i == 1) break;
	p = i / 2;
	if (!larger (base[p].edge, base[i].edge)) break;
	swap (base, p, i);
	/* ASSERT (!larger (base[p].edge, base[i].edge)); */
	i = p;
    }
}

Private void siftdown (pqEntry_t *base, pqi_t N)
{
    pqi_t i = 1;
    pqi_t c;		/* child */

    for (;;) {
	c = i + i;		/* c is left child of i */
	if (c > N) break;
	if (c+1 <= N 		/* c + 1 is right child of i */
	&& (smaller (base[c+1].edge, base[c].edge)))
	    c += 1;

	/* c is least child of i */
	if (!larger (base[i].edge, base[c].edge)) break;

        swap (base, i, c);

	/* ASSERT (!larger (base[i].edge, base[c].edge)); */
	i = c;
    }
}
