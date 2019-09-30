/*
 * ISC License
 *
 * Copyright (C) 1993-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include "src/space/include/config.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "src/libddm/dmincl.h"
#include "src/space/auxil/auxil.h"
#include "src/space/makegln/makegln.h"
#include "src/space/makegln/proto.h"

void readEdges (DM_CELL *cellKey, char *mask, int masktype, int scaleFactor) // readAln
{
    DM_STREAM *stream;
    slope_t slope;

    stream = dmOpenStream (cellKey, mprintf ("%s_aln", mask), "r");

    while (dmGetDesignData (stream, GEO_GLN) > 0) {

	if (ggln.xr == ggln.xl) {
	    if (ggln.yr > ggln.yl)
		slope = 2;               /* use 2 for 90 degrees */
	    else if (ggln.yr < ggln.yl)
		slope = -2;              /* use -2 for -90 degrees */
	    else
		slope = 0;
	}
	else
	    slope = (ggln.yr - ggln.yl)/(ggln.xr - ggln.xl);

	Debug (fprintf (stderr, "input xl=%ld yl=%ld xr=%ld yr=%ld slope=%d\n",
				ggln.xl, ggln.yl, ggln.xr, ggln.yr, slope));

	sortEdge (ggln.xl, ggln.yl, ggln.xr, ggln.yr, slope, START);
    }

    dmCloseStream (stream, COMPLETE);

    if (optDelete) dmUnlink (cellKey, mprintf ("%s_aln", mask));
}
