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

void proc_term (int xl, int xr, int yb, int yt)
{
    if (xl > xr) { tmp_i = xl; xl = xr; xr = tmp_i; }
    if (yb > yt) { tmp_i = yb; yb = yt; yt = tmp_i; }

    /* check for legal terminal layercode */
    if (process -> mask_type[lay_code] != DM_INTCON_MASK) {
	pr_exit (014, 29, layer); /* illegal layercode */
	return;
    }

    strcpy (gterm.term_name, terminal);
    gterm.layer_no = lay_code;
    gterm.xl = xl;
    gterm.xr = xr;
    gterm.yb = yb;
    gterm.yt = yt;

    /* compute bounding box */
    if (nx) {
	if (dx < 0) xl += nx * dx;
	else        xr += nx * dx;
    }
    else dx = 0;

    if (ny) {
	if (dy < 0) yb += ny * dy;
	else        yt += ny * dy;
    }
    else dy = 0;

    gterm.dx = dx;
    gterm.nx = nx;
    gterm.dy = dy;
    gterm.ny = ny;
    gterm.bxl = xl;
    gterm.bxr = xr;
    gterm.byb = yb;
    gterm.byt = yt;

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

    dmPutDesignData (fp_term, GEO_TERM);
}
