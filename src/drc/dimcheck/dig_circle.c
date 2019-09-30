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

/* This procedure returns the value of the square root	 */
/* of length*length - dx*dx making use of the table made */
/* by ini().						 */
/* If the length is greater then the dimensions of the	 */
/* table, the value is calculated via a square root.	 */

int dig_circle (int length, int dx)
{
    if (dx >= length) return (0);
    if (length > MAXGAPTABLE || dx > MAXGAPTABLE)
        return((int) (sqrt((double) (length * length) -
                           (double) (dx * dx)) + 0.99999));
    return (ptable[length].dis[dx]);
}
