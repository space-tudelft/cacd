
/*
 * ISC License
 *
 * Copyright (C) 1987-2016 by
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

extern long *Nil;
extern int out_indent;
extern int useNodeNrs;
extern int tog_nobrack;

#define MAXDECL  80

#ifdef __cplusplus
  extern "C" {
#endif
static int  unequalNet (struct net_el *n1, struct net_el *n2);
#ifdef __cplusplus
  }
#endif

struct term_ref *isTerminal (struct model_info *ntw, char *name)
{
    struct term_ref *tref;
    tref = ntw -> terms;
    while (tref && strcmp (tref -> t -> term_name, name)) tref = tref -> next;
    return (tref);
}

void prNets (struct model_info *ntw, struct net_ref *nets)
{
    char buf[32];
    struct term_ref *tref;
    struct net_ref *nref;
    struct net_el *fnet, *n;
    char *nname;
    char *declared[MAXDECL];
    long i, decl_nr = 0;

    out_indent = 4;

    for (nref = nets; nref; nref = nref -> next) {
	fnet = nref -> n;
	if (fnet -> inst_name) continue;

	nname = fnet -> net_name;

	if (!fnet -> net_neqv) { /* net decl. of array */
	    n = fnet;

	    /* if it has a dimension and is not a terminal, it must be declared */

	    if (n -> net_dim > 0 && !tog_nobrack && n -> net_lower[0] != n -> inst_lower[0]) {
		tref = ntw -> terms;
		while (tref && strcmp (tref -> t -> term_name, nname) != 0) {
		    tref = tref -> next;
		}
		if (!tref) { /* not a terminal */

		    /* also, check if it has not been declared before */

		    for (i = 0; i < decl_nr; i++)
			if (strcmp (declared[i], nname) == 0) break;

		    if (i == decl_nr) { /* declare it */
			oprint (0, "net {(");
			nmprint (0, nname, n -> net_dim, n -> net_lower, n -> inst_lower, 1);
			oprint (1, ")};");
			oprint (1, "\n");
			if (decl_nr < MAXDECL - 1) declared[decl_nr++] = nname;
		    }
		    continue;
		}
	    }
	}

	if ((i = fnet -> net_neqv)) {
	    n = fnet -> net_eqv;
	    do { /* for all subnets */
		if (!n -> inst_name) { /* find the connections (name equivalences) */
		    if (unequalNet (fnet, n)) { /* unequal terminal net */
			oprint (0, "net {");
			nmprint (1, nname, fnet -> net_dim, fnet -> net_lower, Nil, 0);
			oprint (1, ",");
			oprint (0, " ");
			nmprint (1, n -> net_name, n -> net_dim, n -> net_lower, Nil, 0);
			oprint (1, "};");
			oprint (1, "\n");
		    }
		}
		++n;
	    } while (--i);
	}

	if (useNodeNrs && !isdigit (*nname)) {
	    oprint (0, "net {");
	    nmprint (1, nname, fnet -> net_dim, fnet -> net_lower, Nil, 0);
	    oprint (1, ",");
	    sprintf (buf, " %ld};\n", fnet -> nx);
	    nmprint (0, buf, 0L, Nil, Nil, 0);
	}
    }

    out_indent = 0;
}

static int unequalNet (struct net_el *n1, struct net_el *n2)
{
    char *s;
    int i, k;

    if (n1 -> net_dim != n2 -> net_dim) {
	if (n1 -> net_dim == 0 && (s = strchr (n1 -> net_name, '['))) {
	    *s = 0;
	    i = strcmp (n1 -> net_name, n2 -> net_name);
	    *s = '[';
	    if (i) return (1);
	    for (i = 0; i < n2 -> net_dim; ++i) {
		++s;
		if (!isdigit (*s)) return (1);
		k = (*s++ - '0');
		while (isdigit (*s)) k = 10*k + (*s++ - '0');
		if (k != n2 -> net_lower[i]) return (1);
		if (*s != ',') break;
	    }
	    if (*s++ != ']') return (1);
	    if (*s || i+1 != n2 -> net_dim) return (1);
	    return (0);
	}
	return (1);
    }

    if (strcmp (n1 -> net_name, n2 -> net_name)) return (1);

    for (i = n1 -> net_dim; --i >= 0;) {
	if (n1 -> net_lower[i] != n2 -> net_lower[i]) return (1);
    }
    return (0);
}
