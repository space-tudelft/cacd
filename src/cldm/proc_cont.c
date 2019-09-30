/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#include "src/cldm/extern.h"

void proc_cont (int dir, int width)
{
    if (w_dir == dir || dir == 0) {
    /* change of direction, update wire start position */
	if (int_val[0] > 0) {
	    if (w_dir == -1)	/* next box in pos x_direction */
		w_x = w_x - w_width;
	    else		/* next box in pos y-direction */
		w_y = w_y - w_width;
	    int_val[0] = int_val[0] + w_width;
	}
	else {
	    if (w_dir == -1)	/* next box in neg x-direction */
		w_x = w_x + w_width;
	    else		/* next box in neg y-direction */
		w_y = w_y + w_width;
	    int_val[0] = int_val[0] - w_width;
	}
    }

    if (dir) w_dir = dir;

    w_width = width;

    proc_wire ();
}
