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

void proc_wire ()
{
    register int i, sign, xl, xr, yb, yt;

    if (w_width <= 0) {
	pr_exit (014, 32, "wire");
	return;
    }

    --int_ind;

    for (i = 0; i <= int_ind; i++) {
	if (int_val[i] > 0)
	    sign = 1;
	else
	    if (int_val[i] < 0)
		sign = -1;
	    else {
		pr_exit (014, 31, "wire"); /* incr. value is 0 */
		return;
	    }

	if (w_dir == -1) {	/* add box in x-dir */
	    if (i > 0)
		xl = w_x - sign * w_width;
	    else
		xl = w_x;

	    w_x += int_val[i];

	    if (i < int_ind)
		xr = w_x + sign * w_width;
	    else
		xr = w_x;

	    yb = w_y - w_width;
	    yt = w_y + w_width;
	    w_dir = 1;
	}
	else {			/* add box in y-dir */
	    if (i > 0)
		yb = w_y - sign * w_width;
	    else
		yb = w_y;

	    w_y += int_val[i];

	    if (i < int_ind)
		yt = w_y + sign * w_width;
	    else
		yt = w_y;

	    xl = w_x - w_width;
	    xr = w_x + w_width;
	    w_dir = -1;
	}

	if ((xl & 1) || (xr & 1) || (yb & 1) || (yt & 1)) {
	    pr_exit (0614, 45, 0); /* coord not on lambda grid */
	}

	proc_box (xl / 2, xr / 2, yb / 2, yt / 2);
    }
}
