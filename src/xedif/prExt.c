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

#include "src/xedif/incl.h"

extern struct cir *beg_extl;

void prExt ()
{
    DM_PROJECT *proj;
    struct cir *bc, *ec;

    for (bc = beg_extl; bc; bc = bc -> next)
    if (bc -> imported == IMPORTED) {

	/* list imported cells per project */

	oprint (0, "(external ");
	oprint (0, projname (proj = bc -> proj));
	oprint (0, "(edifLevel 0)");
	oprint (0, "(technology(numberDefinition))");

	for (ec = bc; ec; ec = ec -> next) {
	    if (ec -> proj == proj) {
		prHead (newNetw (ec -> name, ec -> proj, ec -> imported, ec -> orig_name, 1), 1);
		ec -> imported = LOCAL; /* reset flag! */
		oprint (0, "))");
	    }
	}
	oprint (0, ")");
    }
}
