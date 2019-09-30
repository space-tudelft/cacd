
/*
 * ISC License
 *
 * Copyright (C) 1987-2011 by
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

#include "src/xsls/incl.h"

extern int out_indent;

void prHead (struct model_info *ntw)
{
    struct cir_term *t;
    struct term_ref *tref;
    int firstinode;

    oprint (0, "\nnetwork ");
    oprint (1, ntw -> name);

    if (!ntw -> terms) {
	oprint (1, " ()\n{\n");
	goto ret;
    }
    oprint (1, " (");

    if ((out_indent = outPos ()) > 24) out_indent = 8;

    oprint (0, "terminal ");

    firstinode = 1;

    for (tref = ntw -> terms; tref; tref = tref -> next) {
	t = tref -> t;

	if (firstinode) firstinode = 0;
	else { oprint (1, ","); oprint (0, " "); }
	nmprint (0, t -> term_name, t -> term_dim, t -> term_lower, t -> term_upper, 1);
    }

    oprint (1, ")");
    oprint (0, "\n");
    out_indent = 0;
    oprint (0, "{\n");
ret:
    outPos ();
}
