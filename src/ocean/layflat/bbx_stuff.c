/*
 * ISC License
 *
 * Copyright (C) 2000-2018 by
 *	Simon de Graaf
 *	Kees-Jan van der Kolk
 *	Patrick Groeneveld
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

#include <stdio.h>
#include "src/libddm/dmincl.h"
#include "src/ocean/layflat/layflat.h"
#include "src/ocean/layflat/prototypes.h"

static MTXELM globalbbx[4] = {0, 0, 0, 0};
static int firsttime = 1;

/* Inspect localbbx to see if it falls outside globalbbx.
 * If so, update globalbbx.
 */
void update_bbx (MTXELM localbbx[4])
{
    if (firsttime)
    { /* cannot init at compile-time because of symbolic constants XL,XR,YB,YT */
	globalbbx[XL] = localbbx[XL];
	globalbbx[XR] = localbbx[XR];
	globalbbx[YB] = localbbx[YB];
	globalbbx[YT] = localbbx[YT];
	firsttime = 0;
	return;
    }

    if (localbbx[XL] < globalbbx[XL]) globalbbx[XL] = localbbx[XL];
    if (localbbx[XR] > globalbbx[XR]) globalbbx[XR] = localbbx[XR];
    if (localbbx[YB] < globalbbx[YB]) globalbbx[YB] = localbbx[YB];
    if (localbbx[YT] > globalbbx[YT]) globalbbx[YT] = localbbx[YT];
}

/* Write the accumulated bouding box to the database.
 */
void output_bbx (DM_CELL *key)
{
    DM_STREAM *dst;

    if (!(dst = dmOpenStream (key, "info", "w"))) err (5, "Cannot open info stream for writing!");

    ginfo.bxl = globalbbx[XL];
    ginfo.bxr = globalbbx[XR];
    ginfo.byb = globalbbx[YB];
    ginfo.byt = globalbbx[YT];

    if (dmPutDesignData (dst, GEO_INFO)) err (5, "Cannot output bbx info");
    if (dmPutDesignData (dst, GEO_INFO)) err (5, "Cannot output bbx info");
    if (dmPutDesignData (dst, GEO_INFO)) err (5, "Cannot output bbx info");

    dmCloseStream (dst, COMPLETE);
}
