/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
 *	J. Annevelink
 *	J. Liedorp
 *	S. de Graaf
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

#include "src/drc/nbool/nbool.h"

static int cmp_occ (int ind1, int ind2);

void reheap ()
{
    register int i, j, k;

    for (i = 1; 2 * i <= nf; i = j) {
	j = 2 * i;
	if (j < nf) /* find smaller child */
	    if (cmp_occ (edge_heap[j - 1], edge_heap[j]) > 0) j++;
	if (cmp_occ (edge_heap[i - 1], edge_heap[j - 1]) <= 0) break;
	k = edge_heap[i - 1];
	edge_heap[i - 1] = edge_heap[j - 1];
	edge_heap[j - 1] = k;
    }
}

void mk_heap ()
{
    register int i, j, k;

    for (i = nf; i > 1; i = j) {
	j = i / 2;
	if (cmp_occ (edge_heap[i - 1], edge_heap[j - 1]) > 0) break;
	k = edge_heap[i - 1];
	edge_heap[i - 1] = edge_heap[j - 1];
	edge_heap[j - 1] = k;
    }
#ifdef DEBUG
    fprintf (stderr, "mk_heap(): Exit\n");
    /* pr_eheap (0, 20); */
#endif
}

static int cmp_occ (int ind1, int ind2) /* compare two occurences */
{
    if (edges[ind1].pos < edges[ind2].pos) return (-1);

    if (edges[ind1].pos == edges[ind2].pos) {
	if (edges[ind1].yb < edges[ind2].yb) return (-1);
	if (edges[ind1].yb == edges[ind2].yb) return (0);
    }
    return (1);
}
