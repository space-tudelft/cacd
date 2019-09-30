/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
 *	J. Annevelink
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

#include "src/drc/nbool/nbool.h"

#define cur_edge edges[edge_heap[0]]

/*
** select next edge from the vertical line segment file
*/
int select_edge (int *pos_p, int *yb_p, int *yt_p, char *edge_type_p, int *chk_type_p, unsigned int *mask_p)
/* pos_p       - position of new edge */
/* yb_p, yt_p  - bottom, top coordinate of new edge */
/* chk_type_p  - check_type of new edge */
/* mask_p      - mask code of new edge */
/* edge_type_p - occurence type of new edge */
{
    if (nf <= 0) return (0);

    /* assign attributes of edge to formal parameters */
    *pos_p       = cur_edge.pos;
    *yb_p        = cur_edge.yb;
    *yt_p        = cur_edge.yt;
    *edge_type_p = cur_edge.edge_type;
    *chk_type_p  = cur_edge.chk_type;
    *mask_p = cur_edge.mask;

    /* read new edge from line segment file */

    if (dmGetDesignData (cur_edge.fp, GEO_VLNLAY) > 0) {
	cur_edge.pos       = gvlnlay.x;
	cur_edge.yb        = gvlnlay.yb;
	cur_edge.yt        = gvlnlay.yt;
	cur_edge.edge_type = gvlnlay.occ_type;
	cur_edge.chk_type  = gvlnlay.chk_type;
    }
    else {
	dmCloseStream (cur_edge.fp, COMPLETE);
	edge_heap[0] = edge_heap[--nf];
    }

    if (nf > 0) reheap ();

    return (1);
}
