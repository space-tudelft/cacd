/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
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

void print_sr ()
{
/* This procedure prints the contents of the stateruler.
 * It is intended to be used with the debugger gdb.
 */
    struct sr_field *c_sr;

    printf ("\n yb  yt  x1   x2   lay1 lay2 grp1 grp2 helplay");
    c_sr = h_sr;
    while (c_sr != h_sr -> prev) {
	printf ("\n%d    %d    %d    %d    %d    %d    %d    %d    %d",
		c_sr -> yb, c_sr -> yt, c_sr -> xstart[0], c_sr -> xstart[1],
		c_sr -> lay_status[0], c_sr -> lay_status[1],
		c_sr -> group[0], c_sr -> group[1], c_sr -> helplay_status);
	c_sr = c_sr -> next;
    }
}
