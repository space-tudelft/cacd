
/*
 * ISC License
 *
 * Copyright (C) 1997-2016 by
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

extern FILE *subfp;
extern struct net_el *free_el_list;
extern struct node_info *nodetab;
extern int  nodeCounter;
extern int  cd_cnt;

char *nNET = "n";
char *pNET = "p";

struct net_el *fnetGND;
struct net_el *fnetSUB;

static struct net_el **hashtab = NULL;
static int hashSize;

static int hashval (char *s)
{
    int hv = 0, i = 1;

    if (s && *s) { while (*s) { hv += *s++ * i; i *= 9; } }
    else fatalErr ("internal error", "no inst_name");

    return (hv < 0 ? -hv % hashSize : hv % hashSize);
}

void hash (struct net_el *n2, struct net_el *n)
{
    if (n) {
	n2 -> next = n -> next;
	n -> next = n2;
    }
    else {
	int hv = hashval (n2 -> inst_name);
	n2 -> next = hashtab[hv];
	hashtab[hv] = n2;
    }
}

void hashInit ()
{
    int i;

    if (!hashtab) {
	hashSize = 25600;
	PALLOC (hashtab, hashSize, struct net_el *);
    }

    for (i = 0; i < hashSize; i++) hashtab[i] = NULL;
}

struct net_el *findInst (char *inst_name)
{
    struct net_el *n;
    for (n = hashtab[hashval (inst_name)]; n; n = n -> next) {
	if (!strcmp (n -> inst_name, inst_name)) break;
    }
    return (n);
}

/* input: eqv, a single subnet node (must have inst_name)
** output: corresponding single head node for the found net
*/
struct net_el *findNet (struct net_el *eqv)
{
    int i;
    struct net_el *n;
    char *s, *inst_name = eqv -> inst_name;

    n = hashtab[ hashval (inst_name) ];

    while (n) {
	if ((n -> inst_name == inst_name || strcmp (n -> inst_name, inst_name) == 0)
	    && strcmp (n -> net_name, eqv -> net_name) == 0
	    && n -> inst_dim == eqv -> inst_dim
	    && n -> net_dim  == eqv -> net_dim) {

	    for (i = eqv -> inst_dim; --i >= 0;) {
		if (eqv -> inst_lower[i] != n -> inst_lower[i]) goto next;
	    }
	    for (i = eqv -> net_dim; --i >= 0;) {
		if (eqv -> net_lower[i] != n -> net_lower[i]) goto next;
	    }
	    return (nodetab[n->nx].netref -> n);
	}
next:
	n = n -> next;
    }

    if (subfp && inst_name[0] == '_'
		&& (inst_name[1] == 'C' || inst_name[1] == 'R') && eqv -> net_name[0] == 'n') {
	s = inst_name + 2;
	if (*s == 'G') return (fnetGND);
	if (*s == 'S') { ++s;
	    if (*s == 'G') return (fnetGND);
	    return (fnetSUB);
	}
	if (!isdigit (*s)) goto ret;
	++s; while (isdigit (*s)) ++s;
	if (*s++ != '_') goto ret;
	i = (*s++ - '0');
	if (i < 1 || i > 9) goto ret;
	while (isdigit (*s)) i = i*10 + (*s++ - '0');
	if (i > nodeCounter) goto ret;
	return (nodetab[i].netref -> n);
ret:
	fprintf (stderr, "findNet: %s: incorrect instance name\n", inst_name);
    }
    return (n);
}

void findNetRC (struct net_el *eqv, double val)
{
    struct net_el *n;
    struct net_ref *nref;
    int i, hv, n1, n2;
    char *s, *t, *inst_name = eqv -> inst_name;

    if (inst_name && inst_name[0] == '_' && (inst_name[1] == 'C' || inst_name[1] == 'R')) {

	eqv -> net_name = pNET;
	s = inst_name + 2;
	if (*s == 'S') ++s;
	if (*s == 'G') ++s;
	t = s;
	if (*t == '_') ++t;
	n1 = (*t++ - '0');
	if (n1 < 1 || n1 > 9) {
	    if (s - inst_name != 4 || n1 != 0) goto ret;
	}
	while (isdigit (*t)) n1 = n1 * 10 + (*t++ - '0');
	if (*t == '_') { ++t;
	    n2 = (*t++ - '0'); if (n2 < 1 || n2 > 9) goto ret;
	    while (isdigit (*t)) n2 = n2 * 10 + (*t++ - '0');
	    if (n2 == n1) goto ret;
	    if (n2 < n1) { i = n1; n1 = n2; n2 = i; }
	}
	else n2 = 0;
	if (*t) goto ret;
	if (n2) sprintf (s, "%d_%d", n1, n2);
	else    sprintf (s, "%d", n1);

	if (s - inst_name == 4) { /* _CSG instance */
	    if (!fnetSUB) {
		fprintf (stderr, "findNetRC: %s: no node number\n", inst_name);
		return;
	    }
	    n1 = fnetSUB -> nx;
	}

	hv = hashval (inst_name);
	for (n = hashtab[hv]; n; n = n -> next) {
	    if (n -> net_name == pNET && !strcmp (n -> inst_name, inst_name)) break;
	}
	if (!n) {
	    if (!subfp) {
		fprintf (stderr, "findNetRC: warning: %s: instance not found\n", inst_name);
		return;
	    }
	    if ((n = free_el_list)) free_el_list = free_el_list -> net_eqv;
	    else PALLOC (n, 1, struct net_el);
	    n -> net_name = pNET;
	    n -> net_dim  = 0;
	    n -> inst_name = newStringSpace (inst_name);
	    n -> inst_dim = 0;
	    n -> nx = n1;
	    /* add to hashtab */
	    n -> next = hashtab[hv];
	    hashtab[hv] = n;
	    /* add to subnet list */
	    nref = nodetab[n1].netref;
	    nref -> nl -> net_eqv = n;
	    nref -> nl = n;
	    n -> net_eqv = NULL;
	    if (n -> valid) fatalErr ("internal error:", "n->valid != 0");
	    n -> v.val = val;
	    n -> valid = 1;
	}
	else if (n -> valid == 0 && !subfp) {
	    n -> v.val = val;
	    n -> valid = 1;
	}
	else if (n -> valid == 1) {
	    if (!subfp) fprintf (stderr, "findNetRC: warning: found twice instance: %s\n", inst_name);
	    if (inst_name[1] == 'C') n -> v.val += val;
	    else n -> v.val = (n -> v.val * val) / (n -> v.val + val);
	}
	else fatalErr ("internal error:", "n->valid != 1");
	return;
    }
ret:
    fprintf (stderr, "findNetRC: %s: incorrect instance name\n", inst_name ? inst_name : "");
}

struct net_el *findNetSub (struct net_el *eqv) /* only for RC instances */
{
    struct net_el *n;
    for (n = hashtab[hashval (eqv -> inst_name)]; n; n = n -> next) {
	if (n -> net_name == eqv -> net_name && !strcmp (n -> inst_name, eqv -> inst_name)) break;
    }
    return n;
}

void addNetSub (struct net_el *eqv, int nx) /* add subnet of RC instance */
{
    struct net_el *n;
    struct net_ref *nref;

    if ((n = free_el_list)) free_el_list = free_el_list -> net_eqv;
    else PALLOC (n, 1, struct net_el);

    n -> net_name = eqv -> net_name;
    n -> net_dim  = 0;
    n -> inst_name = eqv -> inst_name;
    n -> inst_dim = 0;
    n -> nx = nx;
    n -> next = NULL;

    /* add to subnet list */
    nref = nodetab[nx].netref;
    nref -> nl -> net_eqv = n;
    nref -> nl = n;
    n -> net_eqv = NULL;
}
