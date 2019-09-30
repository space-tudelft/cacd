
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

#include "src/xnle/incl.h"

extern struct cir *beg_extl;
extern struct model_info *Devs;
extern struct model_info *Funcs;
extern struct model_info *preFuncs; /* predefined functions */

int NLEMapStatement = 0;

static void prExtHead (struct model_info *ntw)
{
    struct cir_term *t;
    struct term_ref *tref;
    int firstinode;

    if (ntw -> imported == IMPORTED) {
	oprint (0, "M MDE ");
	oprint (0, ntw -> orig_name);
	oprint (0, " ");
	oprint (0, projname (ntw -> proj));
	oprint (0, " |");
    }
    else {
	oprint (0, "M NLE ");
	oprint (0, ntw -> orig_name);
	oprint (0, " * |");
    }

    NLEMapStatement = 1;

    firstinode = 0;
    for (tref = ntw -> terms; tref; tref = tref -> next) {
	if (tref -> type) firstinode = 1;
	t = tref -> t;
	nmprint (0, t -> term_name, t -> term_dim, t -> term_lower, t -> term_upper, 1);
    }

    NLEMapStatement = 0;

    if (firstinode) {
	oprint (0, " | ");
	for (tref = ntw -> terms; tref; tref = tref -> next) {
	    int i, n, w;

	    t = tref -> t;
	    w = 1;
	    for (i = 0; i < t -> term_dim; ++i) {
		n = t -> term_upper[i] - t -> term_lower[i];
		if (n >= 0)
		    w *= n + 1;
		else
		    w *= -n + 1;
	    }
	    do {
		switch (tref -> type) {
		case INPUT:
		case INREAD:
		    oprint (0, "i"); break;
		case INOUT:
		    oprint (0, "b"); break;
		case OUTPUT:
		    oprint (0, "o"); break;
		case OUTPUT3:
		    oprint (0, "t"); break;
		default:
		    oprint (0, "u");
		}
	    } while (--w);
	}
	oprint (0, ";\n");
    }
    else
	oprint (0, " | ;\n");
}

void prExt ()
{
    struct model_info *ntw;
    struct cir *ec;

    for (ec = beg_extl; ec; ec = ec -> next)
	prExtHead (newNetw (ec -> name, ec -> proj, ec -> imported, ec -> orig_name, 1));

    for (ntw = Devs; ntw; ntw = ntw -> next) if (!*ntw -> prefix) prExtHead (ntw);
    for (ntw = Funcs; ntw; ntw = ntw -> next) prExtHead (ntw);
    for (ntw = preFuncs; ntw; ntw = ntw -> next) prExtHead (ntw);
}
