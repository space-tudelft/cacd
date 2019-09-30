
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

#include "src/xvhdl/incl.h"

struct tnet {
    struct net_el *n;
    struct tnet *next;
    struct tnet *tnetnext;
};

static struct tnet *tn_free = NULL;
static struct tnet *tn_beg, *tn_end;

#define NEW_TNET(ptr) { \
    if (tn_free) { \
	ptr = tn_free; \
	tn_free = ptr -> next; \
	ptr -> next = NULL; \
    } \
    else { \
	PALLOC (ptr, 1, struct tnet); \
    } \
}

#define FREE_TNET(ptr) { \
    tn_end -> next = tn_free; \
    tn_free = ptr; \
}

extern struct node_ref *Node_list;
extern struct node_ref *Node_list_free;
extern struct node_ref *Node_list_last;

extern long *Nil;
extern long mapdim, mapxv[];
extern int out_indent;
extern int prPortType;
extern int currTermType;

extern char *xvhdl_net_prefix;

struct term_ref *isTerminal (struct model_info *ntw, char *name)
{
    struct term_ref *tref;
    tref = ntw -> terms;
    while (tref && strcmp (tref -> t -> term_name, name)) tref = tref -> next;
    return (tref);
}

void prNets (struct model_info *ntw, struct net_ref *nets)
{
    struct node_ref *ptr;
    struct tnet *tf, *tl, *tn;
    struct term_ref *tref;
    struct net_ref *nref;
    struct net_el *n, *n_e;
    char buf[DM_MAXNAME + DM_MAXNAME + 52];
    char *nname, *s;
    long *lower, *upper, xvector[10];
    long i;
    int tnt, type;
    int node_cnt = 0;

    out_indent = 2;

    for (nref = nets; nref; nref = nref -> next) {
	n = nref -> n;
	if (!n -> net_neqv) continue; /* net decl. of array */

	s = nname = n -> net_name;
	while (isdigit ((int)*s)) ++s;
	if (!*s) { /* local node */
	    if (node_cnt++ == 0) oprint (0, "SIGNAL ");
	    else oprint (1, ", ");
	    sprintf (buf, "%s%s", xvhdl_net_prefix, nname);
	    nmprint (1, buf, 0L, Nil, Nil, 1);
	    if (node_cnt == 8) {
		node_cnt = 0;
		oprint (1, ": STD_LOGIC;\n");
	    }
	}
	else {
	    n_e = NULL;
	    if ((tref = isTerminal (ntw, nname))) {
		n_e = n;
		type = tref -> type;
	    }
	    else type = INOUT; // suppress compiler warning

	    /* search for terminals in subnets */
	    tf = tl = NULL;
	    i = n -> net_neqv;
	    n = n -> net_eqv;
	    do { /* for all subnets */
		if (!n -> inst_name && (tref = isTerminal (ntw, n -> net_name))) {
		    if (!n_e) { /* first terminal in net */
			/* use terminal as net_name */
			n_e = nref -> n;
			n_e -> net_name  = n -> net_name;
			n_e -> net_dim   = n -> net_dim;
			n_e -> net_lower = n -> net_lower;
			type = tref -> type;
		    }
		    else {
			if (!tf) {
			    NEW_TNET (tf);
			    tf -> n = n_e;
			    tl = tf;
			}
			NEW_TNET (tn);
			tn -> n = n;
			tnt = tref -> type;
			if (type != INPUT && (tnt == INOUT || tnt == INPUT)) {
			    tn -> next = tf;
			    tf = tn;
			    type = tnt;
			}
			else {
			    tl -> next = tn;
			    tl = tn;
			}
		    }
		}
		++n;
	    } while (--i);

	    if (tf) {
		if (!tn_beg) tn_beg = tf;
		else tn_end -> tnetnext = tf;
		tn_end = tf;
	    }

	    if (!n_e) { /* no terminals in net */
		char *s_u;
		long *lxv;

		n = nref -> n;
		if ((mapdim = n -> net_dim) == 0) {
		    s_u = vhdl_mapping (&nname);
		    lxv = mapxv;
		}
		else {
		    s_u = NULL;
		    lxv = n -> net_lower;
		}

		for (ptr = Node_list; ptr; ptr = ptr -> next)
		    if (strcmp (ptr -> node_name, nname) == 0) break;
		if (ptr) {
		    if (ptr -> node_dim != mapdim)
			fatalErr ("internal error on node_dim:", nname);
		    for (i = 0; i < mapdim; ++i) {
			if (lxv[i] < ptr -> node_lower[i])
			    ptr -> node_lower[i] = lxv[i];
			else if (lxv[i] > ptr -> node_upper[i])
			    ptr -> node_upper[i] = lxv[i];
		    }
		}
		else {
		    NEW_NODE (ptr);
		    ptr -> node_name  = newStringSpace (nname);
		    ptr -> node_dim   = mapdim;
		    ptr -> node_lower = newIndexSpace (mapdim);
		    ptr -> node_upper = newIndexSpace (mapdim);
		    for (i = 0; i < mapdim; ++i) {
			ptr -> node_lower[i] = lxv[i];
			ptr -> node_upper[i] = lxv[i];
		    }
		}
		if (s_u) *s_u = '_';
	    }
	}
    }

    if (node_cnt > 0) {
	node_cnt = 0;
	oprint (1, ": STD_LOGIC;\n");
    }
    if (Node_list) {
	currTermType = 0;
	prPortType = 1;
	for (ptr = Node_list; ptr; ptr = ptr -> next) {
	    oprint (0, "SIGNAL ");
	    nmprint (1, ptr -> node_name, ptr -> node_dim, ptr -> node_lower, ptr -> node_upper, 0);
	    oprint (1, ";\n");
	}
	prPortType = 0;
	FREE_NODES ();
    }

    out_indent = 0;
    oprint (0, "\nBEGIN\n\n");
    outPos ();
    out_indent = 2;

    if (tn_beg) {
	char *q, *q_u, *s_u;
	long qdim, *qlower;

	tn_end -> tnetnext = NULL;
	for (tn = tn_beg; tn; tn = tn -> tnetnext) {
	    n = tn -> n;
	    q = n -> net_name;
	    if ((qdim = n -> net_dim) == 0) {
		q_u = vhdl_mapping (&q);
		if ((qdim = mapdim)) {
		    xvector[0] = mapxv[0];
		    xvector[1] = mapxv[1];
		}
		qlower = xvector;
		if (q != n -> net_name) q = newStringSpace (q);
	    }
	    else { q_u = NULL; qlower = n -> net_lower; }

	    for (tn_end = tn -> next;; tn_end = tn_end -> next) {
		n = tn_end -> n;
		s = n -> net_name;
		if ((mapdim = n -> net_dim) == 0) {
		    s_u = vhdl_mapping (&s);
		    lower = mapxv;
		}
		else { s_u = NULL; lower = n -> net_lower; }

		nmprint (1, s, mapdim, lower, lower, 0);
		if (s_u) *s_u = '_';
		oprint (1, " <= ");
		nmprint (0, q, qdim, qlower, qlower, 0);
		oprint (1, ";\n");
		if (!tn_end -> next) break;
	    }
	    if (q_u) *q_u = '_';
	    FREE_TNET (tn);
	}
	tn_beg = NULL;
    }

    out_indent = 0;
}
