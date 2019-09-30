/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "src/space/tecc/define.h"
#include "src/space/tecc/extern.h"

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void do_mkElemKeys (struct layCondRef **condp, int id);
Private void mkElemKeys (int keyval, int weight, struct layerRef *k);
#ifdef __cplusplus
  }
#endif

int specific_res;

static int previd;
static int latcap;
static int elno;
static int e_id;
static struct layCondRef **cS;
static struct elemRef ** ktab;
static struct layerRef *klist;

int condcmp (struct layCondRef *c1, struct layCondRef *c2)
{
    while (c1) {
    if (c2 && c1 -> layC -> lay -> mask == c2 -> layC -> lay -> mask &&
	c1 -> layC -> lay -> occurrence == c2 -> layC -> lay -> occurrence &&
	c1 -> layC -> present == c2 -> layC -> present) {
	c1 = c1 -> next;
	c2 = c2 -> next;
    }
    else
	return (0);
    }
    return (c2 ? 0 : 1);
}

void mkKeys ()
{
    struct layCondRef *cstack[10];
    register int elemno;

    if (specific_res) {
	int i, j, k;
	for (i = 0; i < res_cnt; i++) {
	    for (j = 0; j < vdm_cnt; j++) {
		if (ress[i].mask == vdms[j].mask
		&& condcmp (ress[i].cond, vdms[j].cond)) break;
	    }
	    if (j < vdm_cnt)
		ress[i].val /= vdms[j].thickness;
	    else
		fprintf (stderr,
		" --- vdimension not found for conductor: %s\n", ress[i].name);
	}
	for (i = 0; i < con_cnt; i++) {
	    for (j = 0; j < vdm_cnt; j++)
		if (cons[i].mask1 == vdms[j].mask) break;
	    if (j >= vdm_cnt) fprintf (stderr,
		" --- vdimension not found for mask1 of contact: %s\n", cons[i].name);
	    for (k = 0; k < vdm_cnt; k++)
		if (cons[i].mask2 == vdms[k].mask) break;
	    if (k >= vdm_cnt) fprintf (stderr,
		" --- vdimension not found for mask2 of contact: %s\n", cons[i].name);
	    if (j < vdm_cnt && k < vdm_cnt) {
		double hc = vdms[j].height - vdms[k].height;
		if (hc < 0) { hc = -hc; k = j; }
		hc -= vdms[k].thickness;
		if (hc <= 0) {
		    fprintf (stderr, " --- warning: height of contact %s = %g\n", cons[i].name, hc);
		    hc = 0;
		}
		cons[i].val *= hc;
	    }
	}
    }

    cS = cstack;
    previd = -1;
    elno = 0;

    latcap = 1;
    ktab = keytab2; klist = keylist2; /* edge elements */

    for (elemno = 0; elemno < cap_cnt; elemno++) {
	if (caps[elemno].eltype != LATCAPELEM) continue;
	do_mkElemKeys (&caps[elemno].cond, caps[elemno].id);
    }
    latcap = 0;
    for (elemno = 0; elemno < cap_cnt; elemno++) {
	if (caps[elemno].eltype != EDGECAPELEM) continue;
	do_mkElemKeys (&caps[elemno].cond, caps[elemno].id);
    }

    ktab = keytab; klist = keylist; /* surface elements */

    for (elemno = 0; elemno < cap_cnt; elemno++) {
	if (caps[elemno].eltype != SURFCAPELEM) {
	    if (caps[elemno].eltype != EDGECAPELEM &&
		caps[elemno].eltype != LATCAPELEM)
		fatalErr ("unknown capacitance type", NULL);
	    continue;
	}
	do_mkElemKeys (&caps[elemno].cond, caps[elemno].id);
    }

    for (elemno = 0; elemno < con_cnt; elemno++) { /* contacts */
	do_mkElemKeys (&cons[elemno].cond, cons[elemno].id);
    }

    for (elemno = 0; elemno < tor_cnt; elemno++) {
	do_mkElemKeys (&tors[elemno].cond, tors[elemno].id);
    }

    for (elemno = 0; elemno < bjt_cnt; elemno++) {
	if (bjts[elemno].type == LBJTELEM) {
	     ktab = keytab2; klist = keylist2; }
	else { ktab = keytab; klist = keylist; }
	do_mkElemKeys (&bjts[elemno].cond, bjts[elemno].id);
    }

    for (elemno = 0; elemno < jun_cnt; elemno++) {
	if (juns[elemno].pins[CA] -> occurrence != SURFACE
	 || juns[elemno].pins[AN] -> occurrence != SURFACE) {
	     ktab = keytab2; klist = keylist2; }
	else { ktab = keytab; klist = keylist; }
	do_mkElemKeys (&juns[elemno].cond, juns[elemno].id);
    }

    for (elemno = 0; elemno < cnt_cnt; elemno++) { /* connects */
	if (cnts[elemno].cons[0] -> occurrence != SURFACE
	 || cnts[elemno].cons[1] -> occurrence != SURFACE) {
	     ktab = keytab2; klist = keylist2; }
	else { ktab = keytab; klist = keylist; }
	do_mkElemKeys (&cnts[elemno].cond, cnts[elemno].id);
    }

    ktab = keytab; klist = keylist; /* surface elements */

    for (elemno = 0; elemno < res_cnt; elemno++) {
	do_mkElemKeys (&ress[elemno].cond, ress[elemno].id);
    }

    for (elemno = 0; elemno < vdm_cnt; elemno++) {
	do_mkElemKeys (&vdms[elemno].cond, vdms[elemno].id);
    }

    ktab = keytab2; klist = keylist2; /* edge elements */

    for (elemno = 0; elemno < shp_cnt; elemno++) {
	do_mkElemKeys (&shps[elemno].cond, shps[elemno].id);
    }

    ktab = keytab; klist = keylist; /* surface elements */

    for (elemno = 0; elemno < sbc_cnt; elemno++) {
        do_mkElemKeys (&subconts[elemno].cond, subconts[elemno].id);
    }

    for (elemno = 0; elemno < new_cnt; elemno++) {
        do_mkElemKeys (&newmsks[elemno].cond, newmsks[elemno].id);
    }
}

Private void do_mkElemKeys (struct layCondRef **condp, int id)
{
    struct layCondRef *c, *prevc;
    struct layerRef *k;

    if (previd == id && !latcap) ++e_id;
    else { e_id = 0; previd = id; }

    for (k = klist; k; k = k -> next) {
	k -> present = -1;
	if ((c = *condp))
	for (prevc = NULL; c; c = (prevc = c) -> next) {
	    if (k -> lay -> mask       == c -> layC -> lay -> mask
	     && k -> lay -> occurrence == c -> layC -> lay -> occurrence) {
		if (prevc) prevc -> next = c -> next;
		else *condp = c -> next;
		k -> present = c -> layC -> present;
		break;
	    }
	}
    }

    cS[e_id] = *condp;

    mkElemKeys (0, 1, klist);
    ++elno;
}

Private int testCond (struct layCondRef *c1, struct layCondRef *c2)
{
    struct layCondRef *c;
    int n1, n2;

    if (!c1) return 1;
    if (!c2) return 2;

    n1 = 1; c = c1; while ((c = c -> next)) ++n1;
    n2 = 1; c = c2; while ((c = c -> next)) ++n2;

    if (n2 < n1) { c = c1; c1 = c2; c2 = c; }

    do {
	for (c = c2; c; c = c -> next) {
	    if (c -> layC -> lay -> mask       == c1 -> layC -> lay -> mask &&
		c -> layC -> lay -> occurrence == c1 -> layC -> lay -> occurrence) {
		if (c -> layC -> present != c1 -> layC -> present) return 0;
		break;
	    }
	}
	if (!c) return 0;
    } while ((c1 = c1 -> next));

    return n2 < n1 ? 2 : 1;
}

Private void mkElemKeys (int keyval, int weight, struct layerRef *k)
{
    struct elemRef *e, *pe;
    int done, v;

    if (!k) {
	if (e_id && (e = ktab[keyval]) && e -> elno >= elno - e_id) {
	    done = 0;
	    pe = 0;
	    do {
		v = testCond (cS[e_id], cS[e_id - (elno - e -> elno)]);
		if (v) { /* use one of the two elements */
		    if (v == 2) return; /* use el2 */
		    if (pe) pe -> next = e -> next;
		    else { pe = e; e -> elno = elno; ++done; }
		}
		else pe = e;
	    } while ((e = e -> next) && e -> elno >= elno - e_id);
	    if (done) return;
	}
	ALLOC (e, 1, struct elemRef);
	e -> next = ktab[keyval];
	ktab[keyval] = e;
	e -> elno = elno;
	return;
    }

    if (k -> present != 1) /* 'x' or '0' */
	mkElemKeys (keyval, weight * 2, k -> next);
    if (k -> present != 0) /* 'x' or '1' */
	mkElemKeys (keyval + weight, weight * 2, k -> next);
}
