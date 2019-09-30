
/*
 * ISC License
 *
 * Copyright (C) 1987-2015 by
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

extern long netCounter;

static struct net_el **hashtab = NULL;
static long hashSize;
static long hashCnt;

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
    long i;
    struct net_ref *nref;
    struct net_el *eqv;
    struct net_el **hteladdr;

    i = (5 * (netCounter + 1)) / 75;

    if (!hashtab) {
	hashCnt = 4;
	while (i > hashCnt && hashCnt < 256) hashCnt *= 2;
	hashSize = hashCnt * 75;
	PALLOC (hashtab, hashSize, struct net_el *);
    }
    else if (i >= 2 * hashCnt && hashCnt < 256) {
	Free (hashtab);
	while (i > hashCnt && hashCnt < 256) hashCnt *= 2;
	hashSize = hashCnt * 75;
	PALLOC (hashtab, hashSize, struct net_el *);
    }

    for (i = 0; i < hashSize; i++) hashtab[i] = NULL;

    for (nref = nets; nref; nref = nref -> next) {
	eqv = nref -> n;
	if ((i = eqv -> net_neqv) > 0) {
	    hteladdr = hashtab + hashval (eqv);
	    eqv -> next = *hteladdr;
	    *hteladdr = eqv;
	    eqv = eqv -> net_eqv;
	    do {
		eqv -> net_eqv = nref -> n;
		hteladdr = hashtab + hashval (eqv);
		eqv -> next = *hteladdr;
		*hteladdr = eqv++;
	    } while (--i);
	}
    }
}

/* input: eqv, a single subnet node
** output: corresponding single head node for the found net
*/
struct net_el *findNet (struct net_el *eqv)
{
    long i;
    struct net_el *n;
    char *s, *inst_name = eqv -> inst_name;

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
	    if (!n -> net_neqv) return (n -> net_eqv); /* subnet */
	    return (n);
	}
next:
	n = n -> next;
    }
    if (inst_name[0] == '_' && (inst_name[1] == 'C' || inst_name[1] == 'R')) {
	s = strchr (inst_name+3, '_');
	if (s) {
	    *s = 0;
	    n = findNet (eqv);
	    *s = '_';
	}
    }
    return (n);
}
