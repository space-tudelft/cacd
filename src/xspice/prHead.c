
/*
 * ISC License
 *
 * Copyright (C) 1987-2013 by
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

#include "src/xspice/incl.h"

extern char *NBULK, *PBULK;
extern long *Nil;
extern int add_comma;
extern int add_paren;
extern int out_indent;
extern int dialect;
extern int tog_pnod;
extern int tog_nnod;
extern int tog_nodnbr;
extern int in_prHead;

void prHead (struct model_info *ntw, int submod)
{
    struct cir_term *t;
    struct term_ref *tref;
    int firstinode;
    char par[256];
    char *p, *q, *ntw_name;
    int prPnod, prNnod;

    ntw_name = ntw -> name;

    if (dialect == PSTAR) {
	q = 0;
	p = ntw -> name - 1;
	while (*++p) if (*p == '_') { *p = '-'; if (!q) q = p; }

	oprint (0, submod ? "\nmodel: " : "\ncircuit /* ");
	oprint (1, ntw -> name);
	if (q) {
	    p = q - 1;
	    while (*++p) if (*p == '-') *p = '_';
	}
	if (!submod) {
	    oprint (1, " */;\n");
	    outPos ();
	    return;
	}
    }
    else {
#ifdef XSPECTRE
	oprint (0, submod ? "\nsubckt " : "\n//subckt ");
#else
	oprint (0, submod? "\n.subckt " : "\n*.subckt ");
#endif
	oprint (1, ntw -> name);
    }

    prPnod = tog_pnod;
    prNnod = tog_nnod;

    if (prPnod || prNnod) {
	for (tref = ntw -> terms; tref; tref = tref -> next) {
	    t = tref -> t;
	    if (prPnod && strcmp (t -> term_name, PBULK) == 0) {
		prPnod = 0;
		if (!prNnod) goto nxt;
	    }
	    if (prNnod && strcmp (t -> term_name, NBULK) == 0) {
		prNnod = 0;
		if (!prPnod) goto nxt;
	    }
	}
    }

nxt:
    if (!(prPnod || prNnod || ntw -> terms)) {
#ifdef XPSTAR
	oprint (1, " ();\n");
#else
	if (add_paren) oprint (1, " ()\n");
	else oprint (0, "\n");
#endif
	outPos ();
	return;
    }

    if (add_paren)
	oprint (1, " (");
    else
	oprint (0, " ");

    if ((out_indent = outPos ()) > 24) out_indent = 8;

    in_prHead = 1;

    if (prPnod || prNnod) {
	if (prPnod) {
	    nmprint (0, PBULK, 0L, Nil, Nil, 0, tog_nodnbr);
	}
	if (prNnod) {
	    if (prPnod) {
		if (add_comma) oprint (1, ",");
		oprint (0, " ");
	    }
	    nmprint (0, NBULK, 0L, Nil, Nil, 0, tog_nodnbr);
	}
	if (ntw -> terms) {
	    if (add_comma) oprint (1, ",");
	    oprint (0, " ");
	}
    }

    firstinode = 1;

    for (tref = ntw -> terms; tref; tref = tref -> next) {
	t = tref -> t;
	p = t -> term_name;

	if (firstinode) firstinode = 0;
	else {
	    if (add_comma) oprint (1, ",");
	    oprint (0, " ");
	}
	nmprint (0, p, t -> term_dim, t -> term_lower, t -> term_upper, 1, tog_nodnbr);
    }

#ifdef XPSTAR
    oprint (1, ");");
#else
    if (add_paren) oprint (1, ")");
#endif
    oprint (0, "\n");
    out_indent = 0;
    in_prHead = 0;
}
