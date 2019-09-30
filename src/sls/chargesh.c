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

extern int stabstateof (double vmin, double vmax);

static void agglomchargesh (void);
static void asearch (GROUP *g);
static void gsearch (NODE *n);
static void set_unprotect (GROUP *g);

NODE ** gnptrs = NULL;   /* group node pointers */
NODE ** nbptrs = NULL;   /* group neighbor node pointers */
GROUP * vgs = NULL;      /* vicinity groups */
GROUP ** agptrs = NULL;     /* agglomeration group pointers */

NODE ** freegnptrs;
NODE ** freenbptrs;
GROUP * freevgs;
GROUP ** freeagptrs;

void initchargesh ()
{
    if (gnptrs) CFREE (gnptrs);
    if (nbptrs) CFREE (nbptrs);
    if (vgs)    CFREE (vgs);
    if (agptrs) CFREE (agptrs);
    PPALLOC (gnptrs, maxnvicin, NODE);
    PPALLOC (nbptrs, 2 * maxtvicin, NODE);
    PALLOC (vgs, maxnvicin, GROUP);
    PPALLOC (agptrs, maxnvicin, GROUP);
}

void chargeshare () /* performs a charge sharing on the vicinity */
{
    int     cnt;
    NODE ** nn;
    GROUP * g;
    NODE * n;

    if (debugsim) {
	fprintf (debug, "***** chargeshare *****\n\n");
    }

    vicin.groups = freevgs = vgs;
    vicin.group_cnt = 0;
    freegnptrs = gnptrs;
    freenbptrs = nbptrs;

    /* determine groups which have no Forced nodes within them */

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;

	if (n -> ei -> posfloat && !n -> flag) {
	    /* node is or may be isolated from Forced nodes,
	     * and hasn't been visited yet : new group without
	     * Forced nodes has been found
	    */
	    vicin.group_cnt++;
	    freevgs -> qmin = 0;
	    freevgs -> qmax = 0;
	    freevgs -> cap = 0;
	    freevgs -> nodes = freegnptrs;
	    freevgs -> node_cnt = 0;
	    freevgs -> nbs = freenbptrs;
	    freevgs -> nb_cnt = 0;
	    freevgs -> agglomdone = FALSE;
	    gsearch (n);
	    freevgs++;
	}
    }

    /* reset search flags */

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	(*nn++) -> flag = FALSE;
    }

    /* perform a charge sharing, agglomeration after aggomeration */

    g = vicin.groups;
    for (cnt = vicin.group_cnt; cnt > 0; cnt--) {
	if (!g -> agglomdone) {
	    agglom.group_cnt = 0;
	    agglom.groups = freeagptrs = agptrs;
	    asearch (g);
	    agglomchargesh ();
	}
	g++;
    }
}

static void gsearch (NODE *n) /* searches nodes of groups recursively */
{
    int     cnt;
    int     * ds;
    TRANSISTOR * cont;
    NODE * conn;

    n -> flag = TRUE;
    n -> ei -> g = freevgs;
    freevgs -> qmin += n -> statcap * n -> ei -> umin;
    freevgs -> qmax += n -> statcap * n -> ei -> umax;
    freevgs -> cap += n -> statcap;
    freevgs -> node_cnt++;
    *freegnptrs++ = n;

    if (n -> dsx < 0) return;

    for (cnt = *(ds = &DS[n -> dsx]); cnt > 0; cnt--) {
	cont = &T[*++ds];
	if (cont -> state == Closed) {
	    if (&N[cont -> drain] == n)
		conn = &N[cont -> source];
	    else
		conn = &N[cont -> drain];
	    if (conn -> ei -> posfloat && !conn -> flag)
		gsearch (conn);
	}
	else
	    if (cont -> state == Undefined) {
		if (&N[cont -> drain] == n)
		    conn = &N[cont -> source];
		else
		    conn = &N[cont -> drain];
		if (conn -> ei -> posfloat) {
		    *freenbptrs++ = conn;
		    freevgs -> nb_cnt++;
		}
	    }
    }
}

static void asearch (GROUP *g) /* searches groups of an agglomeration recursively */
{
    int     cnt;
    NODE * n;
    NODE ** nn;

    g -> agglomdone = TRUE;
    *freeagptrs++ = g;
    agglom.group_cnt++;

    nn = g -> nbs;
    for (cnt = g -> nb_cnt; cnt > 0; cnt--) {
	n = *nn++;
	if (!n -> ei -> g -> agglomdone)
	    asearch (n -> ei -> g);
    }
}

static void agglomchargesh () /* performs a charge sharing on the agglomeration */
{
    int     cnt;
    int     cnt2;
    NODE ** nn;
    GROUP * g;
    GROUP ** gg;
    unsigned  protectstate;  /* state of Strong groups */
    unsigned  s;
    float   v1;
    float   v2;
    float   incrmin;
    float   decrmax;
    float   capH;
    float   capL;
    float   capX;
    NODE * n;

    /* determine svmin1 and svmax1 of groups,
     * and determine capHtot, capLtot and capXtot
    */

    agglom.capHtot = agglom.capLtot = agglom.capXtot = 0;

    gg = agglom.groups;
    for (cnt = agglom.group_cnt; cnt > 0; cnt--) {
	g = *gg++;

	if (g -> cap > 0) {
	    g -> svmin1 = g -> qmin / g -> cap;
	    g -> svmax1 = g -> qmax / g -> cap;
	}
	else {
	    slserror (NULL, 0, ERROR2, "non-positive group capacitance occurred", NULL);
	}

	g -> state = stabstateof (g -> svmin1, g -> svmax1);

	switch (g -> state) {
	    case H_state:
		agglom.capHtot += g -> cap;
		break;
	    case L_state:
		agglom.capLtot += g -> cap;
		break;
	    case X_state:
		agglom.capXtot += g -> cap;
		break;
	}
    }

    if (agglom.group_cnt == 1) {
	/* because there is only one group we now already
	 * know the svmin and svmax
	*/
	g = *agglom.groups;
	nn = g -> nodes;
	for (cnt2 = g -> node_cnt; cnt2 > 0; cnt2--) {
	    n = *nn++;
	    if (vicin.type == Normal) {
	        n -> ei -> svmin = g -> svmin1;
	        n -> ei -> svmax = g -> svmax1;
	    }
	    else {
	        if (g -> svmin1 < n -> ei -> svmin)
		    n -> ei -> svmin = g -> svmin1;
	        if (g -> svmax1 > n -> ei -> svmax)
		    n -> ei -> svmax = g -> svmax1;
	    }
	}
	return;
    }

    /* determine svmin2 and svmax2 of groups when they are combined,
     * and determine their strength
    */

    protectstate = X_state;

    gg = agglom.groups;
    for (cnt = agglom.group_cnt; cnt > 0; cnt--) {
	g = *gg++;
	switch (g -> state) {
	    case H_state:
		capH = agglom.capHtot - g -> cap;
		capL = agglom.capLtot;
		capX = agglom.capXtot;
		break;
	    case L_state:
		capH = agglom.capHtot;
		capL = agglom.capLtot - g -> cap;
		capX = agglom.capXtot;
		break;
	    case X_state:
		capH = agglom.capHtot;
		capL = agglom.capLtot;
		capX = agglom.capXtot - g -> cap;
		break;
	    default:
		capH = capL = capX = 0;
		ERROR_EXIT (1);
	}

	v1 = g -> qmin / (g -> cap + capL + capX);
	v2 = (g -> qmin + vminH * capH) / (g -> cap + capL + capX + capH);
	if (v1 < v2)
	    g -> svmin2 = v1;
	else
	    g -> svmin2 = v2;

	v1 = (g -> qmax + vH * (capH + capX))
	    / (g -> cap + capH + capX);
	v2 = (g -> qmax + vH * (capH + capX) + vmaxL * capL)
	    / (g -> cap + capH + capX + capL);
	if (v1 > v2)
	    g -> svmax2 = v1;
	else
	    g -> svmax2 = v2;

	s = stabstateof (g -> svmin2, g -> svmax2);
	if (s != X_state) {
	    protectstate = s;
	    g -> strength = Strong;
	}
	else
	    g -> strength = Weak;
    }

    if (protectstate == H_state || protectstate == L_state) {
	/* only when there are Strong groups it has use to determine
	 * which groups are not unprotected
	*/

        /* determine which Weak groups are unprotected */

        gg = agglom.groups;
        for (cnt = agglom.group_cnt; cnt > 0; cnt--) {
	    g = *gg++;
	    if (g -> state != protectstate) {
	        g -> unprotected = TRUE;
	    }
	    else {
	        g -> unprotected = FALSE;
	    }
        }

        /* let unprotected (Weak) groups set the unprotection of their
         * Weak neigbor groups
        */

        gg = agglom.groups;
        for (cnt = agglom.group_cnt; cnt > 0; cnt--) {
	    g = *gg++;
	    if (g -> unprotected) set_unprotect (g);
        }

        /* find new minimum or maximum of not unprotected Weak groups */

        incrmin = vH;
        decrmax = 0;
        gg = agglom.groups;
        for (cnt = agglom.group_cnt; cnt > 0; cnt--) {
	    g = *gg++;
	    if (g -> strength == Strong) {
	        if (protectstate == H_state && g -> svmin2 < incrmin) {
                    incrmin = g -> svmin2;
	        }
	        else
		    if (protectstate == L_state && g -> svmax2 > decrmax) {
		        decrmax = g -> svmax2;
		    }
	    }
	    else
	        if (g -> strength == Weak && !g -> unprotected) {
	            if (protectstate == H_state && g -> svmin1 < incrmin) {
                        incrmin = g -> svmin1;
	            }
	            else
		        if (protectstate == L_state && g -> svmax1 > decrmax) {
		            decrmax = g -> svmax1;
		        }
	        }
        }

        /* improve bounds of not unprotected Weak groups */

        gg = agglom.groups;
        for (cnt = agglom.group_cnt; cnt > 0; cnt--) {
	    g = *gg++;
	    if (g -> strength == Weak && !g -> unprotected) {
	        if (g -> state == H_state)
		    g -> svmin2 = incrmin;
	        else
		    if (g -> state == L_state)
		        g -> svmax2 = decrmax;
	    }
        }

    }  /* end if */

    gg = agglom.groups;
    for (cnt = agglom.group_cnt; cnt > 0; cnt--) {
	g = *gg++;
	nn = g -> nodes;
	for (cnt2 = g -> node_cnt; cnt2 > 0; cnt2--) {
	    n = *nn++;
	    if (vicin.type == Normal) {
		n -> ei -> svmin = g -> svmin2;
		n -> ei -> svmax = g -> svmax2;
	    }
	    else {
		if (g -> svmin2 < n -> ei -> svmin) n -> ei -> svmin = g -> svmin2;
		if (g -> svmax2 > n -> ei -> svmax) n -> ei -> svmax = g -> svmax2;
	    }
	}
    }
}

static void set_unprotect (GROUP *g) /* sets unprotection of neighbor Weak groups recursively */
{
    int cnt;
    NODE ** nn;
    GROUP * nbg;

    g -> unprotected = TRUE;

    nn = g -> nbs;
    for (cnt = g -> nb_cnt; cnt > 0; cnt--) {
	nbg = (*nn++) -> ei -> g;
	if (nbg -> strength == Weak && !nbg -> unprotected) {
	    set_unprotect (nbg);
	}
    }
}
