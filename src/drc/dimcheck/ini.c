/*
 * ISC License
 *
 * Copyright (C) 1982-2018 by
 *	T.G.R. van Leuken
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

#include "src/drc/dimcheck/dimcheck.h"

/* This procedure makes a table used by gap and width	 */
/* tests in which otherwise the square root of the sum   */
/* of quadrats would have to be used. This is done to	 */
/* speed up these tests.				 */

void ini ()
{
    int i, j;

    ALLOC (h_sr, sr_field);
    NALLOC (ptable, MAXGAPTABLE + 1, struct table);

    for (i = 1; i <= MAXGAPTABLE; i++)
	for (j = 0; j < i; j++)
	    ptable[i].dis[j] = (int) (sqrt ((double) (i * i) -
			(double) (j * j)) + 0.99999999);
}
