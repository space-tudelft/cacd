
/*
 * ISC License
 *
 * Copyright (C) 1997-2011 by
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

#include "src/xspf/incl.h"

extern int *Nil;
extern int out_indent;
extern int tog_pnod;
extern int tog_nnod;
extern int use_spef;
extern int in_prHead;

int currTermType;

void prHead (struct model_info *ntw, int submod)
{
    struct cirterm *t;
    struct term_ref *tref;
    int firstinode;
    int prPnod, prNnod;

    if (!use_spef) {
	oprint (0, submod ? "\n.subckt " : "\n* circuit ");
	oprint (1, ntw -> name);
    }

    prPnod = use_spef ? 0 : tog_pnod;
    prNnod = use_spef ? 0 : tog_nnod;

    if (prPnod || prNnod) {
	for (tref = ntw -> terms; tref; tref = tref -> next) {
	    t = tref -> t;
	    if (prPnod && strcmp (t -> term_name, "pbulk") == 0) {
		prPnod = 0;
		if (!prNnod) break;
	    }
	    if (prNnod && strcmp (t -> term_name, "nbulk") == 0) {
		prNnod = 0;
		if (!prPnod) break;
	    }
	}
    }

    if (!(prPnod || prNnod || ntw -> terms)) {
	oprint (0, "\n");
	outPos ();
	return;
    }
    if (!use_spef)
	oprint (0, " ");
    else
	oprint (0, "\n*PORTS\n");

    if ((out_indent = outPos ()) > 24) out_indent = 8;

    currTermType = INOUT;
    in_prHead = 1;
    firstinode = 1;

    if (prPnod) {
	nmprint (0, "pbulk", 0, Nil, Nil, 0);
	firstinode = 0;
    }
    if (prNnod) {
	if (!firstinode) oprint (0, " ");
	nmprint (0, "nbulk", 0, Nil, Nil, 0);
	firstinode = 0;
    }

    for (tref = ntw -> terms; tref; tref = tref -> next) {
	t = tref -> t;
	/* currTermType = tref -> type; */

	if (firstinode) firstinode = 0;
	else if (!use_spef) oprint (0, " ");
	nmprint (0, t -> term_name, t -> term_dim, t -> term_lower, t -> term_upper, 1);
    }

    if (!use_spef) {
	oprint (0, "\n");
	oprint (0, "*|GROUND_NET 0\n");
    }
    out_indent = 0;
    in_prHead = 0;
}
