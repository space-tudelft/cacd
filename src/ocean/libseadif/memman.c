/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
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
 * named DTYPE, then we can define:
 *
 *	#define NewDtype(dptr) { \
 *		(dptr) = (DTYPE *)mnew (sizeof(DTYPE)); \
 *		(dptr)->field = initvalue; \
 *	}
 *	#define FreeDtype(dptr) mfree ((char**)dptr, sizeof(DTYPE))
 *
 * or something like this. Note that no initialization of the functions mnew()
 * and mfree() is necessary. Furthermore, mnew() sets all fields of the
 * structure it returns to zero, so the above initialization of dptr->field
 * would not be necessary if initvalue equals zero.
 */

#include <stdio.h>
#include <stdlib.h>
#include "src/ocean/libseadif/sysdep.h"
#include "src/ocean/libseadif/sea_decl.h"

#define HEAPINCR  65536     /* Default increment is 64 Kbyte. */
#define MAXBLKSIZ  8196     /* We don't deal with blocks > MAXBLKSIZ bytes. */

#define ALIGNMOD  sizeof(double) /* Aligned if (address % ALIGNMOD) == 0 */
#define ALIGNBITS (ALIGNMOD-1)

PRIVATE void askformore (void);

PRIVATE char **freelist[MAXBLKSIZ+1]; /* Initialized to NULL by compiler! */
PRIVATE char *heap = NULL;
PRIVATE unsigned heapincr = (unsigned) (HEAPINCR < MAXBLKSIZ ? MAXBLKSIZ : HEAPINCR);
PRIVATE unsigned heapleft = 0;

char *mnew (int siz) /* siz is in bytes */
{
    char **newblk;
    char **tmpblk;

    while (siz & ALIGNBITS) ++siz;

    if (siz <= 0 || siz > MAXBLKSIZ) {
	printf ("mnew: cannot allocate object with negative size.\n");
	return (NULL);
    }

    if (freelist[siz]) { /* get blok from freelist */
	newblk = (char**)freelist[siz];
	freelist[siz] = (char**)*freelist[siz]; /* unlink from free list */
	for (tmpblk = newblk; siz > 0; siz -= sizeof(char*)) *tmpblk++ = NULL; /* init */
	return ((char *)newblk);
    }

    /* no block of siz bytes available from free list, get one from the heap */
    if (heapleft < siz) askformore (); /* can we get some more ? */

    newblk = (char**)heap;
    heap += siz;
    heapleft -= siz;
    for (tmpblk = newblk; siz > 0; siz -= sizeof(char*)) *tmpblk++ = NULL; /* init */
    return ((char *)newblk);
}

PRIVATE void askformore ()
{
    /* firsttime (heap == NULL) */
    if (heapleft >= sizeof(char*)) { /* don't throw away the last bytes... */
	*((char**)heap) = (char*)freelist[heapleft];
	freelist[heapleft] = (char**)heap;  /* link 'm in the free list */
    }

    while (heapincr & ALIGNBITS) ++heapincr; /* let heapincr be a nice looking number */

    if (!(heap = (char*) malloc (heapincr)))
	sdfreport (Fatal, "askformore: cannot get enough memory for heap");

    while ((long)heap & ALIGNBITS) { /* arrange for correct alignment */
	++heap; --heapincr;
    }
    heapleft = heapincr;
}

/* Return blk to list of free blocks with size siz.
 */
void mfree (char **b, int siz)
{
    char **blk = (char**)b;

    while (siz & ALIGNBITS) ++siz;

    /* we do not have free lists for these sizes */
    if (siz < sizeof(char**) || siz > MAXBLKSIZ) return;

    *blk = (char*)freelist[siz];
    freelist[siz] = blk; /* link into the free list */
}
