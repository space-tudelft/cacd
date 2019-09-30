/*
 * ISC License
 *
 * Copyright (C) 2016 by
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

#include "src/net2net/incl.h"

extern int nxGND, nxSUB;
extern struct net_ref **nreftab;

char *sorts[20];
int fd_nr[20];
int sort_cnt = 0;

void findNetInit (struct net_ref *nets)
{
    struct net_ref *nref;

    sort_cnt = 0;
    sorts[sort_cnt++] = "cap";
    sorts[sort_cnt++] = "res";

    nreftab[0] = NULL;
    for (nref = nets; nref; nref = nref -> next) {
	nreftab[nref -> nx] = nref;
    }
}

void findNetRC (double val)
{
    struct net_ref *ref1, *ref2;
    struct net_el2 *n, *np;
    int  i, cd1, cd2, n1, n2, nl, sort, type;
    char *s, *t, *inst_name = cmc.inst_name;

    type = inst_name[1];
    if (inst_name[0] == '_' && (type == 'C' || type == 'R')) {

	s = inst_name + 2;
	if (*s == 'S') { ++s; ++type;
	    if (!nxSUB) fatalErr ("error: net SUB not found, instance", inst_name);
	}
	if (*s == 'G') { ++s;
		 if (type == typC) type = typCG;
	    else if (type == typCS) type = typCSG;
	    else fatalErr ("error:", "incorrect C type");
	    if (!nxGND) fatalErr ("error: net GND not found, instance", inst_name);
	}
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

	nl = n1;
	if (n2) {
	    if (type != 'C' && type != 'R') fatalErr ("error in findNetRC:", "type != RC");
	}
	else if (type == typCG) n2 = nxGND;
	else if (type == typCSG) { n1 = nxSUB; n2 = nxGND; }
	else if (type == typCS || type == typRS) n2 = nxSUB;
	else fatalErr ("error in findNetRC:", "incorrect type");

	if (!(n2 > n1)) fatalErr ("error in findNetRC:", "n2 <= n1");

	for (sort = 0; sort < sort_cnt; ++sort) {
	    if (strcmp (cmc.cell_name, sorts[sort]) == 0) break;
	}
	if (sort == sort_cnt) {
	    if (sort_cnt == 20) fatalErr ("error in findNetRC:", "too many sorts");
	    sorts[sort_cnt++] = strsave (cmc.cell_name);
	}
	++fd_nr[sort];

	/* find or add 1st pin to subnet list */

	if (!(ref1 = nreftab[n1])) fatalErr ("error in findNetRC:", "no ref1");
	if (!(ref2 = nreftab[n2])) fatalErr ("error in findNetRC:", "no ref2");
	cd1 = ref1 -> cd;
	cd2 = ref2 -> cd;
	if (cd2 < cd1 && cd2) { ref1 = ref2; n2 = n1; }

	np = NULL;
	if (type == 'C') {
	    for (n = ref1 -> nC; n; n = (np = n) -> nexte) {
		if (n -> inst_nr < n2) { n = NULL; break; }
		if (n -> inst_nr == n2) break;
	    }
	}
	else if (type == 'R') {
	    if (cd2 != cd1 && cd2) warning2 ("instance %s pin2 not in same dnet", inst_name);
	    for (n = ref1 -> nR; n; n = (np = n) -> nexte) {
		if (n -> inst_nr < n2) { n = NULL; break; }
		if (n -> inst_nr == n2) break;
	    }
	}
	else {
	    n2 = nl;
	    for (n = ref1 -> nl; n; n = n -> nexte) {
		if (n -> inst_type == type && n -> sort == sort) {
		    if (n -> inst_nr != n2) fatalErr ("error in findNetRC:", "inst_nr != n2");
		    break;
		}
	    }
	}

	if (!n) { /* add after np */
	    PALLOC (n, 1, struct net_el2);
	    n -> inst_nr = n2;
	    n -> inst_type = type;
	    n -> sort = sort;
	    n -> val = val;

	    if (type == 'C') {
		if (np) { n -> nexte = np -> nexte; np -> nexte = n; }
		else { n -> nexte = ref1 -> nC; ref1 -> nC = n; }
	    }
	    else if (type == 'R') {
		if (np) { n -> nexte = np -> nexte; np -> nexte = n; }
		else { n -> nexte = ref1 -> nR; ref1 -> nR = n; }
	    }
	    else { n -> nexte = ref1 -> nl; ref1 -> nl = n; }
	    ++ref1 -> netcnt;
	}
	else {
	    n -> val += val;
	}
	++n -> times; /* 1st pin */
	return;
    }
ret:
    warning2 ("incorrect instance name: %s", inst_name);
}
