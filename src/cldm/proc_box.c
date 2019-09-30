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

void proc_box (int xl, int xr, int yb, int yt)
{
    if (xl == xr || yb == yt) {
	pr_exit (014, 30, "box"); /* illegal box coord's */
	return;
    }
    if (xl > xr) { tmp_i = xl; xl = xr; xr = tmp_i; }
    if (yb > yt) { tmp_i = yb; yb = yt; yt = tmp_i; }

    gbox.layer_no = lay_code;
    gbox.xl = xl;
    gbox.xr = xr;
    gbox.yb = yb;
    gbox.yt = yt;

    /* compute bounding box */
    if (nx) {
	if ((tmp_i = nx * dx) < 0)
	    xl += tmp_i;
	else
	    xr += tmp_i;
    }
    if (ny) {
	if ((tmp_i = ny * dy) < 0)
	    yb += tmp_i;
	else
	    yt += tmp_i;
    }

    gbox.dx = dx;
    gbox.nx = nx;
    gbox.dy = dy;
    gbox.ny = ny;
    gbox.bxl = xl;
    gbox.bxr = xr;
    gbox.byb = yb;
    gbox.byt = yt;

    /* update model-bbox */
    if (ini_bbbox) {
	bbnd_xl = xl;
	bbnd_xr = xr;
	bbnd_yb = yb;
	bbnd_yt = yt;
	ini_bbbox = 0;
    }
    else {
	if (xl < bbnd_xl) bbnd_xl = xl;
	if (xr > bbnd_xr) bbnd_xr = xr;
	if (yb < bbnd_yb) bbnd_yb = yb;
	if (yt > bbnd_yt) bbnd_yt = yt;
    }

    dmPutDesignData (fp_box, GEO_BOX);
}
