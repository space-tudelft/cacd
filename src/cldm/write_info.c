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

void write_info ()
{
    if (!ini_bbbox && !ini_mcbbox) {
	ginfo.bxl = Min (bbnd_xl, mcbb_xl);
	ginfo.bxr = Max (bbnd_xr, mcbb_xr);
	ginfo.byb = Min (bbnd_yb, mcbb_yb);
	ginfo.byt = Max (bbnd_yt, mcbb_yt);
    }
    else
	if (ini_bbbox) {
	    ginfo.bxl = mcbb_xl;
	    ginfo.bxr = mcbb_xr;
	    ginfo.byb = mcbb_yb;
	    ginfo.byt = mcbb_yt;
	    bbnd_xl = bbnd_xr = bbnd_yb = bbnd_yt = 0;
	}
	else {
	    ginfo.bxl = bbnd_xl;
	    ginfo.bxr = bbnd_xr;
	    ginfo.byb = bbnd_yb;
	    ginfo.byt = bbnd_yt;
	    mcbb_xl = mcbb_xr = mcbb_yb = mcbb_yt = 0;
	}

    append_tree (ms_name, &mod_tree);
    ALLOC (tree_ptr -> bbox, mod_bbox);
    tree_ptr -> bbox -> xl = ginfo.bxl;
    tree_ptr -> bbox -> xr = ginfo.bxr;
    tree_ptr -> bbox -> yb = ginfo.byb;
    tree_ptr -> bbox -> yt = ginfo.byt;
    tree_ptr -> impcell = 0;

    dmPutDesignData (fp_info, GEO_INFO);

    ginfo.bxl = mcbb_xl;
    ginfo.bxr = mcbb_xr;
    ginfo.byb = mcbb_yb;
    ginfo.byt = mcbb_yt;
    dmPutDesignData (fp_info, GEO_INFO);

    ginfo.bxl = bbnd_xl;
    ginfo.bxr = bbnd_xr;
    ginfo.byb = bbnd_yb;
    ginfo.byt = bbnd_yt;
    dmPutDesignData (fp_info, GEO_INFO);
}
