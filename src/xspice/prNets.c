
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

extern struct node_info *nodetab;

extern long *Nil;
extern int add_comma;
extern int add_paren;
extern int out_indent;
extern int dialect;
extern int tog_nodnbr;
extern int tog_use0;
extern int groundVnet;
extern int no_zero_res;

#ifdef __cplusplus
  extern "C" {
#endif
static char *nameToStr (int nr, char *name, int dim, long *lower);
static int  unequalNet (struct net_el *n1, struct net_el *n2);
static int  unequalTermNet (struct cir_term *t, struct net_el *n, int tb);
struct term_ref *isTerminal (struct model_info *ntw, char *name);
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
    struct cir_term *t;
    struct term_ref *tref;
    struct net_ref *nref;
    struct net_el *fnet, *gnet, *anet, *n, *n_e;
    char buf[DM_MAXNAME + DM_MAXNAME + 52];
    char *iname, *nname, *sname, *s;
    char *rightName, *leftName;
    long rightNameNbr, leftNameNbr;
    long *lower, *upper, xvector[10];
    long i, j, k, kstop;
    long net_cnt = 0;
    int  not, term_ground, already_grounded;

    if (tog_use0) {
	for (tref = ntw -> terms; tref; tref = tref -> next) {
	    t = tref -> t;
	    for (i = 0; i < t -> term_dim; ++i) xvector[i] = t -> term_lower[i];
	    do {
		nname = nameToStr (0, t -> term_name, t -> term_dim, xvector);
		if (test0 (nname) == 0) { /* terminal connected to GND */
		    s = no_zero_res? "vnet" : "rnet";
		    sprintf (buf, "%s%ld %s", s, ++net_cnt, add_paren? "(" : "");
		    oprint (0, buf);
		    nmprint (1, nname, 0L, Nil, Nil, 0, tog_nodnbr);
#ifdef XSPECTRE
		    s = no_zero_res? "vsource dc=0" : "resistor r=0";
#else
		    s = "0";
#endif
		    sprintf (buf, "%s 0%s %s\n", add_comma? "," : "", add_paren? ")" : "", s);
		    oprint (1, buf);
		}
		for (i = t -> term_dim; --i >= 0;) {
		    if (t -> term_lower[i] <= t -> term_upper[i]) {
			if (++xvector[i] <= t -> term_upper[i]) break;
		    }
		    else {
			if (--xvector[i] >= t -> term_upper[i]) break;
		    }
		    xvector[i] = t -> term_lower[i];
		}
	    } while (i >= 0);
	}
    }

    for (nref = nets; nref; nref = nref -> next) {
	fnet = n = nref -> n;
	nname = n -> net_name;

	if (!n -> net_neqv) continue; /* net decl. of array */

	already_grounded = 0;
	term_ground = 0;
	gnet = NULL;

	leftName = nameToStr (0, fnet -> net_name, fnet -> net_dim, fnet -> net_lower);
	leftNameNbr = testNameNbr (leftName);

	anet = NULL;
	if (nodetab[fnet->nx].isTerm) { /* terminal net */
	    for (tref = ntw -> terms; tref; tref = tref -> next) {
		if (!unequalTermNet (tref -> t, fnet, 1)) {
		anet = fnet; break; }
	    }
	}
	if (tog_use0 && test0 (leftName) == 0) { /* connected to GND */
	    if (anet) term_ground = 1;
	    gnet = fnet;
	}

	if (!anet) {
	    if (!strcmp (fnet -> net_name, "GND")) anet = fnet;
	    else
	    if (!strcmp (fnet -> net_name, "SUBSTR")) anet = fnet;
	    if (anet) {
		if (leftNameNbr >= 0) cannot_die (4, "not new number");
		leftNameNbr = assignNameNbr (leftName, leftNameNbr);
	    }
	}

	n_e = NULL; j = 0;
	i = fnet -> net_neqv;
	n = fnet -> net_eqv;
	do { /* for all subnets */

	    sname = n -> net_name;
	    iname = n -> inst_name;

	    if (!iname) {

                /* Find the connections (name equivalences) */

		not = 0;

		/* try to remove useless net statements */

		if (nodetab[fnet->nx].isTerm) { /* terminal net */
		    for (tref = ntw -> terms; tref; tref = tref -> next) {
			if (!unequalTermNet (tref -> t, n, 1)) break;
		    }
		}
		else tref = NULL;

		rightName = nameToStr (1, sname, n -> net_dim, n -> net_lower);
		if (tog_use0 && test0 (rightName) == 0) { /* connected to GND */
		    if (tref) term_ground = 1;
		    gnet = n;
		}

		if (!tref) {
		    not = 1;
		    if (!n_e) { n_e = n; j = i; }
		}
		else {
		    if (!anet) {
			if (tog_nodnbr) leftNameNbr = testNameNbr (rightName);
			anet = n; /* use 'b' as node name */
			not = 1;
		    }
		}

		if (!not && unequalNet (anet, n)) { /* unequal terminal net */

		    net_cnt++;

		    kstop = (dialect != ELDO && groundVnet && !gnet)? 3 : 1;

		    /* generate a resistor and a voltage source
		       for a SPICE net since SPICE does not handle
		       a node with one element connected to it.
		    */

		    for (k = 1; k <= kstop; k++) {
#ifdef XPSTAR
			if (k == 1)
			    sprintf (buf, "  %s_net%ld ", no_zero_res? "e" : "r", net_cnt);
			else if (k == 2) {
			    if (already_grounded++) continue;
			    sprintf (buf, "  r_net%lda ", net_cnt);
			} else
			    sprintf (buf, "  r_net%ldb ", net_cnt);
#else
			if (dialect == ELDO)
			    sprintf (buf, ".connect ");
			else if (k == 1)
			    sprintf (buf, "%snet%ld ", no_zero_res? "v" : "r", net_cnt);
			else if (k == 2) {
			    if (already_grounded++) continue;
			    sprintf (buf, "rnet%lda ", net_cnt);
			} else
			    sprintf (buf, "rnet%ldb ", net_cnt);
#endif
			if (add_paren) strcat (buf, "(");
			oprint (0, buf);

			if (k == 3)
			    nmprint (1, n -> net_name, n -> net_dim, n -> net_lower, Nil, 0, tog_nodnbr);
			else
			    nmprint (1, anet -> net_name, anet -> net_dim, anet -> net_lower, Nil, 0, tog_nodnbr);
			if (add_comma) oprint (1, ",");
			oprint (0, " ");

			if (k == 1)
			    nmprint (1, n -> net_name, n -> net_dim, n -> net_lower, Nil, 0, tog_nodnbr);
			else
			    oprint (1, "0"); /* GND */

			if (dialect != ELDO) {
#ifdef XSPECTRE
			    if (k == 1) s = no_zero_res? "vsource dc=0" : "resistor r=0";
			    else s = "resistor r=100G";
#else
			    s = (k == 1)? "0" : "100g";
#endif
#ifdef XPSTAR
			    sprintf (buf, ") %s;", s);
#else
			    sprintf (buf, "%s %s", add_paren? ")" : "", s);
#endif
			    oprint (1, buf);
			}
			oprint (1, "\n");
		    }
		}
	    }

	    ++n;
	} while (--i);

	if (tog_nodnbr) { /* assign numbers */
	    if (anet) {
		if (leftNameNbr <= 0) cannot_die (4, "not terminal number");
		if (anet != fnet) assignNameNbr (leftName, leftNameNbr);
	    }
	    else {
		/* SdeG: When there are terminals with the same name, then
		 *       this can happen. assignNameNbr does not need to be
		 *       called, else it creates a double entry.
		 * if (leftNameNbr >= 0) cannot_die (4, "not new number");
		 */
		if (leftNameNbr < 0)
		    leftNameNbr = assignNameNbr (leftName, gnet ? 0L : leftNameNbr);
	    }
	    if ((n = n_e)) {
		do { /* assign subnet nodes */
		    if (!n -> inst_name) {
			nname = nameToStr (1, n -> net_name, n -> net_dim, n -> net_lower);
			if (testNameNbr (nname) < 0) assignNameNbr (nname, leftNameNbr);
		    }
		    ++n;
		} while (--j);
	    }
	}

	if (gnet && !term_ground) { /* label GND connect */
	    if (anet) { /* label net contains a terminal */
		s = no_zero_res? "vnet" : "rnet";
		sprintf (buf, "%s%ld %s", s, ++net_cnt, add_paren? "(" : "");
		oprint (0, buf);
		nmprint (1, anet -> net_name, anet -> net_dim, anet -> net_lower, Nil, 0, tog_nodnbr);
#ifdef XSPECTRE
		s = no_zero_res? "vsource dc=0" : "resistor r=0";
#else
		s = "0";
#endif
		sprintf (buf, "%s 0%s %s\n", add_comma? "," : "", add_paren? ")" : "", s);
		oprint (1, buf);
	    }
	    else anet = gnet;
	}

	if (anet && anet != fnet) { /* use anet as first net */
	    fnet -> net_name  = anet -> net_name;
	    fnet -> net_dim   = anet -> net_dim;
	    fnet -> net_lower = anet -> net_lower;
	}
    }

    out_indent = 0;
}

static char *nameToStr (int nr, char *name, int dim, long *lower)
{
    static char bufx[DM_MAXNAME + 132];
    static char bufy[DM_MAXNAME + 132];
    char *buf, *s;
    int i;

    buf = nr ? bufy : bufx;

    s = buf;
    *s = *name;
    while (*s) *++s = *++name;

    if (dim > 0) {
	*s++ = '[';
	for (i = 0;;) {
	    sprintf (s, "%ld", lower[i]);
	    while (*++s) ;
	    if (++i >= dim) break;
	    *s++ = ',';
	}
	*s++ = ']';
	*s = 0;
    }

    return (buf);
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

static int unequalTermNet (struct cir_term *t, struct net_el *n, int tb)
{
    char *s;
    int i, k;

    if (t -> term_dim != n -> net_dim) {
	if (tb && n -> net_dim == 0 && (s = strchr (n -> net_name, '['))) {
	    *s = 0;
	    i = strcmp (t -> term_name, n -> net_name);
	    *s = '[';
	    if (i) return (1);
	    for (i = 0; i < t -> term_dim; ++i) {
		++s;
		if (!isdigit (*s)) return (1);
		k = (*s++ - '0');
		while (isdigit (*s)) k = 10*k + (*s++ - '0');
		if (t -> term_lower[i] <= t -> term_upper[i]) {
		    if (k < t -> term_lower[i] || k > t -> term_upper[i]) return (1);
		}
		else {
		    if (k < t -> term_upper[i] || k > t -> term_lower[i]) return (1);
		}
		if (*s != ',') break;
	    }
	    if (*s++ != ']') return (1);
	    if (*s || i+1 != t -> term_dim) return (1);
	    return (0);
	}
	return (1);
    }

    if (strcmp (t -> term_name, n -> net_name)) return (1);

    for (i = t -> term_dim; --i >= 0;) {
	k = n -> net_lower[i];
	if (t -> term_lower[i] <= t -> term_upper[i]) {
	    if (k < t -> term_lower[i] || k > t -> term_upper[i]) return (1);
	}
	else {
	    if (k < t -> term_upper[i] || k > t -> term_lower[i]) return (1);
	}
    }
    return (0);
}
