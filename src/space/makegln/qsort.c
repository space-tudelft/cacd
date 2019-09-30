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
#include <stdlib.h>
#include "src/libddm/dmincl.h"
#include "src/space/auxil/auxil.h"
#include "src/space/makegln/makegln.h"
#include "src/space/makegln/proto.h"

/* quicksort implementation based on R. Sedgewick
 * 1. Algorithms, Addison Wesley, ISBN 0-201-06672-6, 1983.
 * 2. The article 'Implementing Quicksort Programs',
 *    Communications of the ACM, Volume 21, Number 10, 1978,
 *    page 847-857. (Very good article.)
 *
 * Main modification: randomization (see below).
 */

#define index_t ecnt_t

#define push(i) stack[p++] = i
#define pop(i)  i = stack[--p]
#define swap(a,b) {_swap_ = a; a = b; b = _swap_;}

/* local operations */
Private ecnt_t partition (_edge_t **base, ecnt_t l, ecnt_t r);
Private void insertionsort (_edge_t **base, ecnt_t L, ecnt_t U);

void sortBlock (_edge_t **base, index_t N)
{
    index_t l = 0;                 /* left bound */
    index_t r = N - 1;             /* right bound */
    index_t p = 2;                 /* stack pointer */
    index_t i;
    index_t stack[50];             /* stack */

    do {
	if (r - l > 15) {
	    /* Partition such that everyting to the left of i
	     * is smaller and to the right of i is larger
	     * than base[i]
	     */
	    i = partition (base, l, r);

            infoPartitionBalance += (double) (i - l) / (r - l);
	    infoNumPartitions++;

	    /* Now, push the largest subfile on the stack
	     * and continue (tail recursion removed)
	     * with the smallest subfile.
	     */
	    if ((i - l) > (r - i)) {
		push (l);
		push (i - 1);
		l = i + 1;
	    }
	    else {
		push (i + 1);
		push (r);
		r = i - 1;
	    }
	}
	else {
	    pop (r);
	    pop (l);
	}
    } while (p > 0);

    insertionsort (base, 0, N-1);
}

/*
 * We want a random index_t in the closed interval [l,r].
 * drand48 returns a random double in the semi-open [0,1).
 * Assignment of double to index_t is defined (ANSI C) to be
 * with truncation of the fraction bits.
 * Hence, adding 0.5 does the trick.
 */
#define RandomInt(l,r) ((l) + ((index_t) ((drand48 () * ((r) - (l))) + 0.5)))

Private index_t partition (_edge_t **base, index_t l, index_t r)
{
    _edge_t ** U, **L, *T;
    _edge_t * v;
    index_t i, j;
    _edge_t * _swap_;

    /* Median of three partioning, see Sedgewick,
     * but with randomization (selection of median probes).
     * The need for this has been experimentally observed.
     * That is, there exist inputs that actually interfere
     * with the plain <l+1, r, (l+r)/2> median probes
     * in such a way that the partitioning process becomes
     * severely unbalanced.
     */

    i = RandomInt (l, r);
    swap (base[i], base[l]);

    i = RandomInt (l+1, r);
    swap (base[i], base[l+1]);

    i = RandomInt (l+2, r);
    swap (base[i], base[r]);

    /* Now we have random elements in l, l+1 and r.
     * Sort them so that l+1 <= l <= r
     * The elements in l+1 and r serve as sentinels
     * in the inner loop.
     */

    if (larger (base[l+1], base[r]))
	swap (base[l+1], base[r]);
    if (larger (base[l],   base[r]))
	swap (base[l],   base[r]);
    if (larger (base[l+1], base[l]))
	swap (base[l+1], base[l]);

    Debug2 (ASSERT (!larger  (base[l+1], base[l])));
    Debug2 ((!smaller (base[r],   base[l])));

    v = base[l];

    /* Pointer based loop.
     * On certain platforms, it does not make much difference when optimizer is used.
     * Should probably write everything with pointers.
     */
    L = base + l + 1;
    U = base + r;
    for (;;) {
	do {
	    L++;
	} while (smaller (*L, v));
	do {
	    U--;
	} while (larger  (*U, v));
	if (U < L)
	    break;
	T = *U; *U = *L; *L = T;        /* swap elements */
    }
    j = U - base;
    swap (base[l], base[j]);

#ifdef POST
    /* post: a[l...j-1] <= a[j] <= a[j+1...r] */
    for (i = l;     i < j; i++) ASSERT (!larger  (base[i], base[j]));
    for (i = j+ 1; i <= r; i++) ASSERT (!smaller (base[i], base[j]));
#endif /* POST */

    return (j);
}

Private void insertionsort (_edge_t **base, index_t L, index_t U)
{
    _edge_t * v;
    index_t i, j;
    for (i = L + 1; i <= U; i++) {
       v = base[i]; j = i;
       while (larger (base[j - 1], v)) {
	   base[j] = base[j-1];
	   if (--j <= L)
	       break;
       }
       base[j] = v;
    }
}
