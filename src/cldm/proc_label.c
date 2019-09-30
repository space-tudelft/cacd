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

void proc_label (int x, int y)
{
    /* check for legal terminal layercode */
    if (process -> mask_type[lay_code] != DM_INTCON_MASK) {
	pr_exit (014, 29, layer); /* illegal layercode */
	return;
    }

    ganno.type = GA_LABEL;
    strcpy (ganno.o.Label.name, label);
    strcpy (ganno.o.Label.Class, "");
    ganno.o.Label.x = x;
    ganno.o.Label.y = y;
    ganno.o.Label.ay = 0;
    ganno.o.Label.ax =  -1;  /* left adjusted */
    ganno.o.Label.maskno = lay_code;

    /* update model-bbox */
    if (ini_bbbox) {
	bbnd_xl = x;
	bbnd_xr = x;
	bbnd_yb = y;
	bbnd_yt = y;
	ini_bbbox = 0;
    }
    else {
	if (x < bbnd_xl) bbnd_xl = x;
	if (x > bbnd_xr) bbnd_xr = x;
	if (y < bbnd_yb) bbnd_yb = y;
	if (y > bbnd_yt) bbnd_yt = y;
    }

    dmPutDesignData (fp_anno, GEO_ANNOTATE);
}
