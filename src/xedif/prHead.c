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

extern int dialectCds;

int currTermType = 0;
int prPortType = 0;

void prHead (struct model_info *ntw, int submod)
{
    struct cir_term *t;
    struct term_ref *tref;
    int firstinode;
    char par[256], *p;

    oprint (0, "(cell ");
    oprint (0, dialectCds ? ntw -> out_name : ntw -> orig_name);

    /* Added by B.Sneeuw
     * If the -c option is set view and viewType are set to NETLIST.
     * For the -I option view and viewType will be SCHEMATIC.
     */
    oprint (0, "(cellType GENERIC)");
    if (dialectCds) {
	if (submod) {
	    if (dialectCds > 1) /* Schematic */
		oprint (0, "(view symbol (viewType SCHEMATIC)");
	    else
		oprint (0, "(view symbol (viewType NETLIST)");
	}
	else {
	    if (dialectCds > 1) /* Schematic */
		oprint (0, "(view SCHEMATIC (viewType SCHEMATIC)");
	    else
		oprint (0, "(view NETLIST (viewType NETLIST)");
	}
    }
    else
	oprint (0, "(view VIEWNAMEDEFAULT(viewType NETLIST)");

    oprint (0, "(interface");

    currTermType = 0;
    prPortType = 1;
    firstinode = 1;

    for (tref = ntw -> terms; tref; tref = tref -> next) {
	t = tref -> t;

	currTermType = tref -> type;

	if (!dialectCds)
	    p = t -> term_name;
	else
	    if (!(p = tref -> alter_name)) continue;

	if (firstinode) firstinode = 0;
	else oprint (0, " ");

	nmprint (0, p, t -> term_dim, t -> term_lower, t -> term_upper, 1);
    }

    prPortType = 0;

    if (ntw -> param) {
	p = ntw -> param;
	while (*p) {
	    sscanf (p, "%s", par);
	    oprint (0, "(parameter ");
	    oprint (0, par);
	    /*
	    if (strcmp (ntw -> name, "res") == 0 && par[0] == 'v') {
		oprint (0, "(number 0)(unit RESISTANCE)");
	    }
	    else if (strcmp (ntw -> name, "cap") == 0 && par[0] == 'v') {
		oprint (0, "(number 0)(unit CAPACITANCE)");
	    }
	    else if (par[0] == 'w' && !par[1]) {
		oprint (0, "(number(e 1 -6))(unit DISTANCE)");
	    }
	    else if (par[0] == 'l' && !par[1]) {
		oprint (0, "(number(e 1 -6))(unit DISTANCE)");
	    }
	    else {
	    */
		oprint (0, "(number)");
	    /*
	    }
	    */
	    oprint (0, ")");
	    while (*p && *p != ' ') p++;
	    if (*p == ' ') p++;
	}
    }
    oprint (0, ")"); /* end of interface */
}
