/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	A.J. van Genderen
 *	S. de Graaf
 *	N.P. van der Meijs
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

#include "src/sls/extern.h"

extern void getsignal (NODE *n, simtime_t t, SIGNALEVENT *sigev);
extern void resistdivide (void);
extern void startvsearch (NODE *vn);
extern void vclose (void);

static void initmode_vicinity (NODE *vn);
static int  resBetween (int n1, int n2);
static void resSetFlag (int nx, int val);

void initmode ()
{
    /* determine the initial modes of the transistors */

    int cnt;
    NODE * n;
    SIGNALEVENT nextevent;
    int first;
    int firstval = X_state;

    if (monitoring) monitime ("B initmode");

    if (debugsim) {
	fprintf (debug, "\n================ initmode ================\n\n");
    }

    for (cnt = 0; cnt < T_cnt; cnt++) {
	T[cnt].state = Closed;
    }

    for (cnt = 0; cnt < N_cnt; cnt++) {
	n = &N[cnt];

        if (!n -> inp) {

	    /* certainly, it is not a VDD or VSS node */

	    n -> type = Normal;
	    n -> evalflag = TRUE;
	    continue;
	}

	nextevent.time = -1;
	first = TRUE;
	do {
	    getsignal (n, nextevent.time, &nextevent);
	    if (first) {
		firstval = nextevent.val;
		first = FALSE;
	    }
	}
	while (nextevent.time >= 0 && nextevent.time <= tsimduration
	&& nextevent.val == firstval);

        if (nextevent.val == firstval
	    && (firstval == H_state || firstval == L_state)) {

	    /* the input node doesn't change,
	       so we assume it is a VDD or VSS node */

            n -> type = Forced;
	    n -> state = firstval;
	}
	else {
	    n -> type = Normal;
	    n -> evalflag = TRUE;
	}
    }

    for (cnt = 0; cnt < N_cnt; cnt++) {
	n = &N[cnt];
	if (n -> evalflag)
	    initmode_vicinity (n);
    }

    if (monitoring) monitime ("E initmode");
}

static void initmode_vicinity (NODE *vn)
{
    int     cnt;
    NODE ** nn;
    NODE * n;
    TRANSISTOR ** tt;
    TRANSISTOR * t;

    startvsearch (vn);

    if (vicin.forcedsH && vicin.forcedsL) {

	resistdivide ();

	/* now it can be determined whether the nodes are
	   in the dc current path or not */

	nn = vicin.nodes;
	for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	    n = *nn++;

	    if (n -> type == Forced) {
		n -> ei -> dc = TRUE;
	    }
	    else {
		if (n -> ei -> bcH_path == n -> ei -> bcL_path) {
		    n -> ei -> dc = FALSE;
		}
		else {
		    n -> ei -> dc = TRUE;
		}
	    }
	}
    }

    tt = vicin.tors;
    for (cnt = vicin.tor_cnt; cnt > 0; cnt--) {
	t = *tt++;

	switch (t -> type) {
	    case Nenh:
	    case Penh:
		if (vicin.forcedsH && vicin.forcedsL) {
		    if (N[t -> source].ei -> dc && N[t -> drain].ei -> dc)
			t -> premode = Pullup;
		    else
			t -> premode = Passup;
		}
		else {
		    t -> premode = Passup;
		}
		break;
	    case Depl:
		if (t -> gate == t -> drain || t -> gate == t -> source
		    || resBetween (t -> gate, t -> source)
		    || resBetween (t -> gate, t -> source))
		    t -> premode = Load;
		else
		    t -> premode = Superload;
		break;
	    case Res:
		t -> premode = 0;
		break;
	}
    }

    vclose ();
}

static int resBetween (int n1, int n2)
{
    int yes;

    resSetFlag (n1, 1);

    if (N[n2].flag)
	yes = 1;
    else
	yes = 0;

    resSetFlag (n1, 0);

    return (yes);
}

static void resSetFlag (int nx, int val)
{
    int index;
    int cnt;
    TRANSISTOR * cont;

    N[nx].flag = val;

    index = N[nx].dsx;
    if (index >= 0) {
        for (cnt = DS[index]; cnt > 0; cnt--) {
	    index++;

	    cont = &T[DS[index]];

	    if (cont -> type == Res) {

	        if (N[cont -> drain].flag != val
	        && N[cont -> drain].type == Normal)
		    resSetFlag (cont -> drain, val);

	        if (N[cont -> source].flag != val
	        && N[cont -> source].type == Normal)
		    resSetFlag (cont -> source, val);
	    }
	}
    }
}
