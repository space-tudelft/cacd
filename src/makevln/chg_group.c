/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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

#include "src/makevln/incl.h"

extern int term_layer;
extern FILE *vln_file, *teq_file;
extern struct S_vln s_vln;
extern struct S_teq s_teq;

void chg_group (DM_STREAM *fp_vln, DM_STREAM *fp_teq, int term_group_offset)
{
    while (fread ((char *)&s_vln, sizeof (s_vln), 1, vln_file) > 0) {
	gvlnlay.x  = s_vln.x;
	gvlnlay.yb = s_vln.yb;
	gvlnlay.yt = s_vln.yt;
	gvlnlay.occ_type = s_vln.occ;
	gvlnlay.con_type = s_vln.con;
	gvlnlay.chk_type = s_vln.cht;
	gvlnlay.grp_number = fdgrp_name (s_vln.grp) + term_group_offset;
	dmPutDesignData (fp_vln, GEO_VLNLAY);
    }

    if (!term_layer) return;

    while (fread ((char *)&s_teq, sizeof (s_teq), 1, teq_file) > 0) {
	gteq.term_number = s_teq.tnr;
	gteq.grp_number = fdgrp_name (s_teq.grp) + term_group_offset;
	dmPutDesignData (fp_teq, GEO_TEQ);
    }
}
