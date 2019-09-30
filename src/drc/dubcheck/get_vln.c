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

/* This procedure reads a line from a vln_file and
 * places its contents in the structure event.
 * On the end of file EOF is returned.
 */
int get_vln (DM_STREAM *fp, int nbr_file)
{
    int edge;

    do {
	if (dmGetDesignData (fp, GEO_VLNLAY) <= 0) return (0);
	edge = gvlnlay.occ_type & 7;
    } while ((edge == 0) || (edge == 3));

    event[nbr_file].e_xi    = gvlnlay.x;
    event[nbr_file].e_yb    = gvlnlay.yb;
    event[nbr_file].e_yt    = gvlnlay.yt;
    event[nbr_file].e_occ   = edge;
    event[nbr_file].e_group = gvlnlay.grp_number;
    event[nbr_file].e_ctype = gvlnlay.chk_type;
    return (1);
}
