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
#include <string.h>
#include <strings.h>

#include "src/space/tecc/define.h"
#include "src/space/tecc/extern.h"

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private int checkRes (char *elem, struct layCondRef *condp, int occur, int resMask);
#ifdef ALLCHECKS
Private void swapEdgeSurface (struct layCondRef *cp);
Private int isSubSet (struct layCondRef *cp, struct layCondRef *cpS);
#endif
#ifdef __cplusplus
  }
#endif

void checkConnections ()
{
    int i, p, n, q;

    for (i = 0; i < con_cnt; i++) {
	if (cons[i].mask1 >= 0)
	    checkRes (cons[i].name, cons[i].cond, SURFACE, cons[i].mask1);
	if (cons[i].mask2 >= 0)
	    checkRes (cons[i].name, cons[i].cond, SURFACE, cons[i].mask2);
    }

    for (i = 0; i < cnt_cnt; i++) {
	p = checkRes (cnts[i].name, cnts[i].cond, SURFACE, cnts[i].cons[0] -> mask);
	n = checkRes (cnts[i].name, cnts[i].cond, SURFACE, cnts[i].cons[1] -> mask);
	if (p >= 0 && n >= 0) {
	    if (ress[p].type != ress[n].type) {
		char *s = mprintf ("element %s:", cnts[i].name);
		warningMes (s, "matching conductors don't have same polarity");
	    }
	}
    }

    for (i = 0; i < tor_cnt; i++) {
	checkRes (tors[i].name, tors[i].cond, SURFACE, tors[i].g);
	checkRes (tors[i].name, tors[i].cond, SURFACE, tors[i].ds);
	if (tors[i].s != tors[i].ds)
	    checkRes (tors[i].name, tors[i].cond, SURFACE, tors[i].s);
	if (tors[i].b >= 0)
	    checkRes (tors[i].name, tors[i].cond, SURFACE, tors[i].b);
    }

    for (i = 0; i < bjt_cnt; i++) {
	p = checkRes (bjts[i].name, bjts[i].cond, SURFACE, bjts[i].pins[EM] -> mask);
	n = checkRes (bjts[i].name, bjts[i].cond, SURFACE, bjts[i].pins[BA] -> mask);
	q = checkRes (bjts[i].name, bjts[i].cond, SURFACE, bjts[i].pins[CO] -> mask);
	if (p >= 0 && n >= 0 && q >= 0) {
	    if (ress[p].type == 0 || ress[n].type == 0 || ress[q].type == 0) {
		char *s;
		if (ress[p].type == 0)
		    s = mprintf ("element %s: matching emitter", bjts[i].name);
		else if (ress[n].type == 0)
		    s = mprintf ("element %s: matching base", bjts[i].name);
		else
		    s = mprintf ("element %s: matching collector", bjts[i].name);
		fatalErr (s, "pin conductor has no polarity\n");
	    }
	    if (ress[p].type == ress[n].type) {
		char *s = mprintf ("element %s:", bjts[i].name);
		fatalErr (s, "matching emitter/base conductors have same polarity\n");
	    }
	    if (ress[q].type == ress[n].type) {
		char *s = mprintf ("element %s:", bjts[i].name);
		fatalErr (s, "matching base/collector conductors have same polarity\n");
	    }
	}
	if (bjts[i].pins[SU] -> mask >= 0) {
	    p = checkRes (bjts[i].name, bjts[i].cond, SURFACE, bjts[i].pins[SU] -> mask);
	    if (p >= 0 && ress[p].type == 0) {
		char *s = mprintf ("element %s:", bjts[i].name);
		warningMes (s, "matching bulk pin conductor has no polarity");
	    }
	}
    }

    for (i = 0; i < cap_cnt; i++) {
	if (caps[i].pLay -> mask >= 0)
	    checkRes (caps[i].name, caps[i].cond,
		      caps[i].pLay -> occurrence, caps[i].pLay -> mask);
	if (caps[i].nLay -> mask >= 0)
	    checkRes (caps[i].name, caps[i].cond,
		      caps[i].nLay -> occurrence, caps[i].nLay -> mask);
    }

    for (i = 0; i < vdm_cnt; i++) {
	if (vdms[i].mask > 0)
	    checkRes (vdms[i].name, vdms[i].cond, SURFACE, vdms[i].mask);
    }

    for (i = 0; i < shp_cnt; i++) {
	if (shps[i].mask > 0)
	    checkRes (shps[i].name, shps[i].cond, SURFACE, shps[i].mask);
    }
}

Private int checkRes (char *elem, struct layCondRef *condp, int occur, int resMask)
{
    int i;

    UseArg (condp);
    UseArg (occur);

    /* at this moment, not all checks can properly be executed ! */

#ifdef ALLCHECKS
    if (occur == EDGE) swapEdgeSurface (condp);
#endif

    for (i = 0; i < res_cnt; i++) {
	if (ress[i].mask == resMask) {
#ifdef ALLCHECKS
	    if (isSubSet (condp, ress[i].cond))
#endif
		break;
	}
    }

#ifdef ALLCHECKS
    if (occur == EDGE) swapEdgeSurface (condp);
#endif

    if (i >= res_cnt) {
	char *s = mprintf ("%s   pin mask '%s' of element '%s'",
	    "Layer Condition Error :\n", maskname (resMask), elem);
	fatalErr (s, "will not connect to a conductor layer\n");
	return (-1);
    }
    return (i);
}

#ifdef ALLCHECKS
Private void swapEdgeSurface (struct layCondRef *cp)
{
    while (cp) {
	if (cp -> layC -> lay -> occurrence == SURFACE)
	    cp -> layC -> lay -> occurrence = EDGE;
	else if (cp -> layC -> lay -> occurrence == EDGE)
	    cp -> layC -> lay -> occurrence = SURFACE;
	cp = cp -> next;
    }
}

Private int isSubSet (struct layCondRef *cp, struct layCondRef *cpS)
{
    int is, layFound;
    struct layCondRef *cpfind;

    is = 1;
    while (cpS && is) {

	cpfind = cp;
	layFound = 0;
	while (cpfind && !layFound) {

	    if (cpS -> layC -> lay -> mask == cpfind -> layC -> lay -> mask
	    && cpS -> layC -> lay -> occurrence
	       == cpfind -> layC -> lay -> occurrence) {

		layFound = 1;
		if (cpS -> layC -> present != cpfind -> layC -> present) is = 0;
	    }

	    cpfind = cpfind -> next;
	}

	if (!layFound) is = 0;

	cpS = cpS -> next;
    }

    return (is);
}
#endif
