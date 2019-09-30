
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

#include "src/xspice/incl.h"

extern struct node_info *nodetab;
extern FILE *subfp;
extern long netCounter;

static struct net_el netRC;
static struct net_el *nGND = NULL;
static struct net_el *nSUB = NULL;
static struct net_el **hashtab = NULL;
static long hashSize;
static long hashCnt = 4;

char *strGND = "GND";
char *strSUB = "SUBSTR";

static long hashval (struct net_el *eqv)
{
    char *s;
    long val = 0;
    long i = 1;

    if ((s = eqv ->  net_name)) { while (*s) { val += *s++ * i; i *= 9; } }
    if ((s = eqv -> inst_name)) { while (*s) { val += *s++ * i; i *= 9; } }

    return (val < 0 ? -val % hashSize : val % hashSize);
}

/* Note: net_eqv is used as net headnode pointer!
**       net_neqv must be equal to 0!
*/
void findNetInit (struct model_info *ntw, struct net_ref *nets)
{
    char buf[32];
    long xv[10];
    long i, j;
    struct net_ref *nref;
    struct net_el *eqv, *n;
    struct net_el **hteladdr, *fnet;
    struct net_el tnetspace;
    struct term_ref *tref;
    struct cir_term *t;

    i = (5 * (netCounter + 1)) / 75;

    if (!hashtab || (i >= 2 * hashCnt && hashCnt < 256)) {
	if (hashtab) Free (hashtab);
	while (i > hashCnt && hashCnt < 256) hashCnt *= 2;
	hashSize = hashCnt * 75;
	PALLOC (hashtab, hashSize, struct net_el *);
    }
    else {
	for (i = 0; i < hashSize; i++) hashtab[i] = NULL;
    }

    j = 0;
    for (nref = nets; nref; nref = nref -> next) {
	eqv = nref -> n;

	if (++j >= netCounter - 1) {
	    if (!nSUB && strcmp (eqv -> net_name, strSUB) == 0) nSUB = eqv;
	    else
	    if (!nGND && strcmp (eqv -> net_name, strGND) == 0) nGND = eqv;
	}

	if ((i = eqv -> net_neqv) >= 0) {
	    hteladdr = hashtab + hashval (eqv);
	    eqv -> next = *hteladdr;
	    *hteladdr = eqv;

	    if (subfp && !eqv -> inst_name) { /* terminal or label */
		PALLOC (n, 1, struct net_el);
		sprintf (buf, "%ld", eqv -> nx);
		n -> net_name = newStringSpace (buf);
		n -> net_eqv = nref -> n;
		hteladdr = hashtab + hashval (n);
		n -> next = *hteladdr;
		*hteladdr = n;
	    }

	    eqv = eqv -> net_eqv;
	    while (i-- > 0) {
		eqv -> net_eqv = nref -> n;
		hteladdr = hashtab + hashval (eqv);
		eqv -> next = *hteladdr;
		*hteladdr = eqv++;
	    }
	}
    }

    n = &tnetspace;
    n -> inst_name = NULL;
    n -> inst_dim = 0;
    n -> net_lower = xv;

    for (tref = ntw -> terms; tref; tref = tref -> next) {

	t = tref -> t;
	n -> net_name = t -> term_name;
	n -> net_dim  = t -> term_dim;
	for (i = 0; i < t -> term_dim; ++i) xv[i] = t -> term_lower[i];

	do {
	    if ((fnet = findNet (n))) nodetab[fnet->nx].isTerm = 1;
	    for (i = t -> term_dim; --i >= 0;) {
		if (t -> term_lower[i] <= t -> term_upper[i]) {
		    if (++xv[i] <= t -> term_upper[i]) break;
		}
		else {
		    if (--xv[i] >= t -> term_upper[i]) break;
		}
		xv[i] = t -> term_lower[i];
	    }
	} while (i >= 0);
    }
}

struct net_el *findTerm (int node_nr)
{
    char buf[32], *s;
    struct net_el *n;
    long i, val;

    sprintf (buf, "%d", node_nr);
    val = 0; i = 1;
    for (s = buf; *s; ++s) { val += *s * i; i *= 9; }
    if (val < 0) val = -val;

    for (n = hashtab[val % hashSize]; n; n = n -> next) {
	if (!n -> inst_name && !n -> net_dim && strcmp (n -> net_name, buf) == 0) {
	    if (!n -> net_neqv && n -> net_eqv) return (n -> net_eqv);
	}
    }
    return (n);
}

/* input: eqv, a single subnet node
** output: corresponding single head node for the found net
*/
struct net_el *findNet (struct net_el *eqv)
{
    long i;
    struct net_el *n, *nt;
    char *s, *inst_name = eqv -> inst_name;

    if (subfp && inst_name && inst_name[0] == '_' && (inst_name[1] == 'C' || inst_name[1] == 'R')) {
	nt = NULL;
	n = &netRC;
	n -> inst_name = inst_name;
	s = inst_name + 2;
	if (eqv -> net_name[0] == 'p') {
	    if (isdigit (*s)) { n -> nx = atoi(s);
		nt = findTerm (n -> nx);
		return (nt ? nt : n);
	    }
	    else if (*s == 'G') { ++s;
		if (*s == '_')    { return (nGND); }
		if (isdigit (*s)) { n -> nx = atoi(s);
		    nt = findTerm (n -> nx);
		    return (nt ? nt : n);
		}
	    }
	    else if (*s == 'S') { ++s;
		if (*s == '_')    { return (nSUB); }
		if (isdigit (*s)) { n -> nx = atoi(s);
		    nt = findTerm (n -> nx);
		    return (nt ? nt : n);
		}
		if (*s == 'G') { ++s; return (*s == '_' ? nGND : nSUB); }
	    }
	}
	else {
	    if (eqv -> net_name[0] != 'n') fprintf (stderr, "findNet: net_name != 'n'\n");
	    if (isdigit (*s)) {
	      if ((s = strchr (s, '_'))) { ++s;
		if (isdigit (*s)) { n -> nx = atoi(s);
		    nt = findTerm (n -> nx);
		    return (nt ? nt : n);
		}
	      }
	    }
	    else if (*s == 'G') { ++s;
		if (isdigit (*s)) { return (nGND); }
		if (*s == '_') { ++s;
		    if (isdigit (*s)) { n -> nx = atoi(s);
			nt = findTerm (n -> nx);
			return (nt ? nt : n);
		    }
		}
	    }
	    else if (*s == 'S') { ++s;
		if (isdigit (*s)) { return (nSUB); }
		if (*s == '_') { ++s;
		    if (isdigit (*s)) { n -> nx = atoi(s);
			nt = findTerm (n -> nx);
			return (nt ? nt : n);
		    }
		}
		else if (*s == 'G') { ++s; return (*s == '_' ? nSUB : nGND); }
	    }
	}
	fprintf (stderr, "findNet: %s: node_nr not found\n", inst_name);
	return (NULL);
    }

    n = hashtab[ hashval (eqv) ];

    while (n) {
	if ((n -> inst_name == inst_name
	    || (n -> inst_name && inst_name
		&& strcmp (n -> inst_name, inst_name) == 0))
	    && strcmp (n -> net_name, eqv -> net_name) == 0
	    && n -> inst_dim == eqv -> inst_dim
	    && n -> net_dim  == eqv -> net_dim) {

	    for (i = eqv -> inst_dim; --i >= 0;) {
		if (eqv -> inst_lower[i] != n -> inst_lower[i]) goto next;
	    }
	    for (i = eqv -> net_dim; --i >= 0;) {
		if (eqv -> net_lower[i] != n -> net_lower[i]) goto next;
	    }
	    if (!n -> net_neqv && n -> net_eqv) n = n -> net_eqv; /* subnet */
	    return (n);
	}
next:
	n = n -> next;
    }
#if 0
    if (inst_name && inst_name[0] == '_' && (inst_name[1] == 'C' || inst_name[1] == 'R')) {
	s = strchr (inst_name+3, '_');
	if (s) {
	    *s = 0;
	    n = findNet (eqv);
	    *s = '_';
	}
    }
#endif
    return (n);
}
