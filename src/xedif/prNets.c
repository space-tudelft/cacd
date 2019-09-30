/*
 * ISC License
 *
 * Copyright (C) 1987-2011 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
 *	Bastiaan Sneeuw
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
#include "src/xedif/search.h"

extern struct inst_ref *Inst_list;
extern long *Nil;
extern int dialectCds;
extern int tog_nobrack;
extern int tog_srange;

/* Added by B.Sneeuw: Printing of Cadence nets */
static char *prCdsNets (char *iname, char *tname)
{
    struct model_info *mod;
    struct term_ref *tref;
    ENTRY item;
    ENTRY *p;

    item.key = iname;
    if (!(p = hsearch (item, FIND))) {
	P_E "error in net file: unknown instance name %s\n", iname);
	cannot_die (4, "prCdsNets");
    }
    mod = (struct model_info *)(p -> data);

    /* search for instance terminal */
    for (tref = mod -> terms; tref; tref = tref -> next) {
	if (strcmp (tref -> t -> term_name, tname) == 0) /* found */
	    return (tref -> alter_name);
    }
    return (0);
}

struct term_ref *isTerminal (struct model_info *ntw, char *name)
{
    struct term_ref *tref;
    tref = ntw -> terms;
    while (tref && strcmp (tref -> t -> term_name, name)) tref = tref -> next;
    return (tref);
}

void prNets (struct model_info *ntw, struct net_ref *nets)
{
    struct term_ref *tref;
    struct net_ref *nref;
    struct net_el *n;
    char *iname, *nname, *sname, *s;
    long *lower, *upper;
    long i, j, k;

    for (nref = nets; nref; nref = nref -> next) {
	n = nref -> n;
	if (!n -> net_neqv) continue; /* net decl. of array */

	nname = n -> net_name;
	oprint (0, "(net ");
	tref = ntw -> terms;
	while (tref && strcmp (tref -> t -> term_name, nname)) {
	    tref = tref -> next;
	}

	i = tog_nobrack;
	tog_nobrack = 1;
	nmprint (0, nname, n -> net_dim, n -> net_lower, Nil, 0);
	tog_nobrack = i;

	oprint (0, "(joined");

	if (tref) {
	    oprint (0, "(portRef ");
	    if (tog_srange && n -> net_dim > 0) {
		lower = tref -> t -> term_lower;
		upper = tref -> t -> term_upper;
		for (i = 0; i < n -> net_dim; ++i) {
		    if (upper[i] < lower[i])
			n -> net_lower[i] -= upper[i];
		    else
			n -> net_lower[i] -= lower[i];
		}
	    }
	    nmprint (0, nname, n -> net_dim, n -> net_lower, Nil, 0);
	    oprint (0, ")");
	}

	j = 0;
	i = n -> net_neqv;
	n = n -> net_eqv;
	do { /* for all subnets */

	    sname = n -> net_name;
	    iname = n -> inst_name;

	    /* we assume that 'arrays of nets' are not specified */

	    tref = ntw -> terms;
	    if (!iname) {
		while (tref && strcmp (tref -> t -> term_name, sname)) {
		    tref = tref -> next;
		}
	    }

	    if (iname || tref) { /* it's a portRef ! */

		if (tog_srange) {
		    if (!iname) {
			if (n -> net_dim > 0) {
			    lower = tref -> t -> term_lower;
			    upper = tref -> t -> term_upper;
			    for (k = 0; k < n -> net_dim; ++k) {
				if (upper[k] < lower[k])
				    n -> net_lower[k] -= upper[k];
				else
				    n -> net_lower[k] -= lower[k];
			    }
			}
		    }
		    else if (n -> net_dim > 0 || n -> inst_dim > 0) {
			struct inst_ref *iref;

			iref = Inst_list;
			while (iref && strcmp (iref -> inst_name, iname)) {
			    iref = iref -> next;
			}
			if (!iref) {
			    P_E "Warning: instance_ref \"%s\" not found!\n", iname);
			    goto lx;
			}
			if (n -> inst_dim > 0) {
			    for (k = 0; k < n -> inst_dim; ++k)
				n -> inst_lower[k] -= iref -> inst_lower[k];
			}
			if (n -> net_dim > 0) {
			    tref = iref -> terms;
			    while (tref && strcmp (tref -> t -> term_name, sname)) {
				tref = tref -> next;
			    }
			    if (!tref) {
				P_E "Warning: terminal_ref \"%s\" not found!\n", sname);
				goto lx;
			    }
			    lower = tref -> t -> term_lower;
			    upper = tref -> t -> term_upper;
			    for (k = 0; k < n -> net_dim; ++k) {
				if (upper[k] < lower[k])
				    n -> net_lower[k] -= upper[k];
				else
				    n -> net_lower[k] -= lower[k];
			    }
			}
		    }
		}
lx:
		if (dialectCds && iname) {
		    s = prCdsNets (iname, sname);
		    if (!s) {
			P_E "Warning: terminal_ref \"%s\" for instance \"%s\" not found!\n", sname, iname);
			goto ly; /* skip portRef */
		    }
		    sname = s;
		}
		oprint (0, "(portRef ");
		nmprint (0, sname, n -> net_dim, n -> net_lower, Nil, 0);
		if (iname) {
		    oprint (0, "(instanceRef ");
		    nmprint (0, iname, n -> inst_dim, n -> inst_lower, Nil, 0);
		    oprint (0, "))");
		}
		else oprint (0, ")");
ly:	;
	    }

	    ++n;
	} while (--i);

        oprint (0, "))"); /* close joined/net */
    }

    oprint (0, ")"); /* close contents */
    /* Added by B.Sneeuw: SCHEMATIC view instead of (default) NETLIST view */
    if (dialectCds > 1) oprint (0, ")"); /* close sheet page SH1 */
}
