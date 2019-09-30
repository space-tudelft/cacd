/*
 * ISC License
 *
 * Copyright (C) 2000-2018 by
 *	Simon de Graaf
 *	Kees-Jan van der Kolk
 *	Patrick Groeneveld
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

/*
 * Memory manager for various types of dynamic data structures. Serves as
 * a more efficient substitute for malloc(). Suppose we have a structure
 * named DTYPE, then we can define
 *
 * #define NewDtype(dptr) { \
 *	(dptr) = (DTYPE *)mnew (sizeof(DTYPE)); \
 *	(dptr)->field = initvalue; \
 * }
 * #define FreeDtype(dptr) mfree ((char **)dptr, sizeof(DTYPE))
 *
 * or something like this. Note that no initialization of the functions mnew()
 * and mfree() is necessary. Furthermore, mnew() sets all fields of the
 * structure it returns to zero, so the above initialization of dptr->field
 * would not be necessary if initvalue equals zero.
 */

#include <stdio.h>
#include <stdlib.h>
#include "src/libddm/dmincl.h"
#include "src/ocean/layflat/layflat.h"
#include "src/ocean/layflat/prototypes.h"

#define MAXBLKSIZ 516		  /* We do not deal with blocks > MAXBLKSIZ bytes. */
#define HEAPINCR  16384		  /* Default increment is 16 Kbyte. */
#define ALIGNMOD  sizeof(double)  /* Aligned if (address % ALIGNMOD) == 0 */
#define ALIGNBITS (ALIGNMOD-1)

static char **freelist[MAXBLKSIZ+1]; /* Initialized to NULL by compiler! */
static char *heap = NULL;
static int heapleft = 0;
static int heapincr = (HEAPINCR < MAXBLKSIZ ? MAXBLKSIZ : HEAPINCR);

static void askformore (void);
static char *heaplowerbound, *heapupperbound;

char *mnew (int siz) /* Siz is in bytes. */
{
    char **newblk, **tmpblk;

    while (siz & ALIGNBITS) ++siz;

    if (siz <= 0 || siz > MAXBLKSIZ) return (NULL);

    if (freelist[siz]) /* Get blok from freelist. */
    {
	newblk = (char **)freelist[siz];
	freelist[siz] = (char **)*freelist[siz]; /* Unlink from free list. */
	for (tmpblk = newblk; siz > 0; siz -= sizeof(char *)) *tmpblk++ = NULL; /* init */
	return ((char *)newblk);
    }

    /* No block of siz bytes available from free list, get one from the heap. */
    if (heapleft < siz) askformore (); /* Can we get some more ? */

    newblk = (char **)heap;
    heap += siz;
    heapleft -= siz;
    for (tmpblk = newblk; siz > 0; siz -= sizeof(char *)) *tmpblk++ = NULL; /* init */

    return ((char *)newblk);
}

static void askformore ()
{
    int firsttime = !heap;

    if (heapleft >= sizeof(char *))  /* Don't throw away the last bytes... */
    {
	*((char **)heap) = (char *)freelist[heapleft];
	freelist[heapleft] = (char **)heap; /* ...but link 'm in the free list. */
    }

    while (heapincr & ALIGNBITS) ++heapincr;

    if (!(heap = (char *)malloc ((unsigned)heapincr)))
	err (3, "askformore: cannot get enough memory for heap.");

    if (firsttime) heaplowerbound = heap; /* initialize lower bound */
    heapupperbound = heap + heapincr;

    /* arrange for correct alignment of the heap: */
    while ((long)heap & ALIGNBITS) ++heap, --heapincr;
    heapleft = heapincr;
}

/* Return blk to list of free blocks with size siz.
 */
void mfree (char **blk, int siz)
{
    if (((char *)blk) < heaplowerbound || ((char *)blk) > heapupperbound) {
	fprintf (stderr, "\nmfree: attempt to free memory that was never allocated by mnew()\n");
	dumpcore ();
    }

    while (siz & ALIGNBITS) ++siz;

    if (siz < sizeof(char**) || siz > MAXBLKSIZ) return; /* no free lists for these sizes */

    *blk = (char *)freelist[siz];
    freelist[siz] = blk; /* Link into the free list. */
}
