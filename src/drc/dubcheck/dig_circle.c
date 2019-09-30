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

#include "src/drc/dubcheck/dubcheck.h"

/* This procedure returns the value of the square root	 */
/* of length*length - dx*dx making use of the table made */
/* by ini().						 */
/* If the length is greater then the dimensions of the	 */
/* table, the value is calculated via a square root.	 */
/* If the value has to be calculated such that an	 */
/* overlapping layer holds the distance (rule - overlap) */
/* extra transformations are performed.			 */

int dig_circle (int length, int dx)
{
    int red_length;
    int red_dx;

    if (grow_fact == 0) {
        if (dx >= length)
	    return (0);
        if (length > MAXGAPTABLE || dx > MAXGAPTABLE)
            return ((int) (sqrt((double) (length * length) -
                               (double) (dx * dx)) + 0.99999));
        return (ptable[length].dis[dx]);
    }
    else {
	if (dx <= grow_fact)
	    return (length);
	if (dx >= length)
	    return (0);
	red_length = length - grow_fact;
	red_dx = dx - grow_fact;
	if (red_length > MAXGAPTABLE || red_dx > MAXGAPTABLE)
	    return ((int) (sqrt((double) (red_length * red_length) -
		(double) (red_dx * red_dx)) + grow_fact + 0.99999));
	return (ptable[red_length].dis[red_dx] + grow_fact);
    }
}
