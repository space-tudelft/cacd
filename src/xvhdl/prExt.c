
/*
 * ISC License
 *
 * Copyright (C) 1992-2011 by
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

#include "src/xvhdl/incl.h"

extern int out_indent;
extern int tog_prExt;

void prExt (struct model_info *ntw)
{
    DM_STREAM *dsp;
    struct model_ref *beg_modl, *ml;
    struct model_info *mod;
    char attribute_string[256];
    long lower[10], upper[10];
    int imported;

    dsp = dmOpenStream (ntw -> dkey, "mc", "r");

    cmc.inst_attribute = attribute_string;
    cmc.inst_lower = lower;
    cmc.inst_upper = upper;

    beg_modl = NULL;

    while (dmGetDesignData (dsp, CIR_MC) > 0) {
	if (is_ap ()) continue;
	imported = cmc.imported;
	if (is_func ()) {
	    mod = findFunc (cmc.cell_name, imported, ntw -> proj);
	    if (!mod -> dkey) continue; /* preFunc */
	}
	else if (!(mod = findDev (cmc.cell_name, imported, ntw -> proj))) {
	    mod = findNetw (cmc.cell_name, imported, ntw -> proj);
	}

	if (tog_prExt || imported == 0) {
	    for (ml = beg_modl; ml; ml = ml -> next) /* search model_ref list */
		if (ml -> m == mod) break;

	    if (!ml) { /* not in model_ref list, add and print component */
		PALLOC (ml, 1, struct model_ref);
		ml -> m = mod;
		ml -> next = beg_modl;
		beg_modl = ml;

		out_indent = 2;
		prHead (mod, 1);
	    }
	}
    }

    oprint (0, "\n");

    while ((ml = beg_modl)) {
	beg_modl = ml -> next;
	Free (ml);
    }

    dmCloseStream (dsp, COMPLETE);
}
