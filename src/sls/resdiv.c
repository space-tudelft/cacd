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

extern float torpar (TRANSISTOR *trans, int mode, int par);
static void pathbcH (void);
static void pathbcL (void);
static void pathwcH (void);
static void pathwcL (void);
static void single_resistdivide (int step);

NODE ** bcH_rfront = NULL;  /* search front for best case path to H */
NODE ** bcL_rfront = NULL;  /* search front for best case path to L */
NODE ** wcH_rfront = NULL;  /* search front for worst case path to H */
NODE ** wcL_rfront = NULL;  /* search front for worst case path to L */
int     bcHf_cnt;
int     bcLf_cnt;
int     wcHf_cnt;
int     wcLf_cnt;

void initresistdiv ()
{
    if (bcH_rfront) CFREE (bcH_rfront);
    if (bcL_rfront) CFREE (bcL_rfront);
    if (wcH_rfront) CFREE (wcH_rfront);
    if (wcL_rfront) CFREE (wcL_rfront);
    PPALLOC (bcH_rfront, maxnvicin, NODE);
    PPALLOC (bcL_rfront, maxnvicin, NODE);
    PPALLOC (wcH_rfront, maxnvicin, NODE);
    PPALLOC (wcL_rfront, maxnvicin, NODE);
}

/* perform a complete resistance division on the
 * vicinity, including a possibly second step
 * to account for saturated transistors
 */
void resistdivide ()
{
    int cnt;
    TRANSISTOR ** tt;
    TRANSISTOR * t;
    int do2;

    single_resistdivide (1);

    do2 = FALSE;
    tt = vicin.tors;
    for (cnt = vicin.tor_cnt; cnt > 0; cnt--) {
	t = *tt++;
	switch (t-> type) {
	    case Nenh :
		if ((N[t -> drain].ei -> svmin > vswitch
		          && N[t -> source].ei -> svmin > vswitch)
		     || N[t -> drain].ei -> svmin > 0.8 * vH
		     || N[t -> source].ei -> svmin > 0.8 * vH) {
		    do2 = TRUE;
		}
		break;
	    case Penh :
		if ((N[t -> drain].ei -> svmax < vswitch
		          && N[t -> source].ei -> svmax < vswitch)
		    || N[t -> drain].ei -> svmax < 0.2 * vH
		    || N[t -> source].ei -> svmax < 0.2 * vH) {
		    do2 = TRUE;
		}
		break;
	    case Depl :
		break;
	}
	if (do2) {
	    single_resistdivide (2);
	    break;
	}
    }
}

/* perform a single resistance division on the vicinity */
static void single_resistdivide (int step)
{
    int cnt, satu;
    NODE ** nn;
    NODE * n;
    NODE * forn;
    NODE * conn;
    NODE * search;
    NODE * track;
    NODE * trackstop;
    NODE * lastcommon;
    NODE * follow;
    TRANSISTOR ** tt;
    TRANSISTOR * t;
    float   newsvmin;
    float   newsvmax;
    float   resist;

    if (debugsim) {
	fprintf (debug, "***** resistdivide (%d) *****\n\n", step);
    }

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;

	if (n -> type == Forced) {
	    n -> ei -> undetermined = FALSE;
	    continue;
	}

	n -> ei -> undetermined = TRUE;
	n -> ei -> bcH_path = NULL;
	n -> ei -> bcL_path = NULL;
	n -> ei -> wcH_path = NULL;
	n -> ei -> wcL_path = NULL;
	n -> ei -> bcH_done = FALSE;
	n -> ei -> bcL_done = FALSE;
	n -> ei -> wcH_done = FALSE;
	n -> ei -> wcL_done = FALSE;
	n -> ei -> ontrack = FALSE;
	n -> ei -> common = FALSE;
	n -> ei -> bcH_dir = NULL;
	n -> ei -> bcL_dir = NULL;
	n -> ei -> wcH_dir = NULL;
	n -> ei -> wcL_dir = NULL;
    }

    bcHf_cnt = 0;
    bcLf_cnt = 0;
    wcHf_cnt = 0;
    wcLf_cnt = 0;

    /* after the above initializations have been done, nodes connected */
    /* to Forced nodes are put on the front list. */

    tt = vicin.tors;
    for (cnt = vicin.tor_cnt; cnt > 0; cnt--) {
	t = *tt++;

        if (step == 2) {
	    satu = FALSE;
	    switch (t-> type) {
		case Nenh :
		    if ((N[t -> drain].ei -> svmin > vswitch
			      && N[t -> source].ei -> svmin > vswitch)
			 || N[t -> drain].ei -> svmin > 0.8 * vH
			 || N[t -> source].ei -> svmin > 0.8 * vH) {
			satu = TRUE;
		    }
		    break;
		case Penh :
		    if ((N[t -> drain].ei -> svmax < vswitch
			      && N[t -> source].ei -> svmax < vswitch)
			|| N[t -> drain].ei -> svmax < 0.2 * vH
			|| N[t -> source].ei -> svmax < 0.2 * vH) {
			satu = TRUE;
		    }
		    break;
		case Depl :
		    break;
	    }
	    if (satu) {
		resist = torpar (t, 0, Rsatu);
	    }
	    else {
		resist = torpar (t, 0, Rstat);
	    }
	}
	else {
	    resist = torpar (t, 0, Rstat);
	}
	t -> ei -> resist = resist;

	if (N[t -> drain].type == Forced) {
	    forn = &N[t -> drain];
	    conn = &N[t -> source];
	}
	else
	    if (N[t -> source].type == Forced) {
		forn = &N[t -> source];
		conn = &N[t -> drain];
	    }
	    else
		conn = forn = NULL;

	if (forn != NULL && conn -> type != Forced) {
	    if (forn -> ei -> lstate != L_state) {
		if (conn -> ei -> bcH_path == NULL) {
		    conn -> ei -> bcH_r = resist;
		    conn -> ei -> bcH_path = forn;
		    bcH_rfront[bcHf_cnt++] = conn;
		}
		else {
		    conn -> ei -> bcH_r = conn -> ei -> bcH_r * resist
					  / (conn -> ei -> bcH_r + resist);
		}
	    }
	    if (forn -> ei -> lstate != H_state) {
		if (conn -> ei -> bcL_path == NULL) {
		    conn -> ei -> bcL_r = resist;
		    conn -> ei -> bcL_path = forn;
		    bcL_rfront[bcLf_cnt++] = conn;
		}
		else {
		    conn -> ei -> bcL_r = conn -> ei -> bcL_r * resist
					  / (conn -> ei -> bcL_r + resist);
		}
	    }
	    if (vicin.undeftors || vicin.forcedsX) {
	        if (t -> state == Closed && forn -> ei -> lstate == H_state) {
		    if (conn -> ei -> wcH_path == NULL) {
		        conn -> ei -> wcH_r = resist;
		        conn -> ei -> wcH_path = forn;
		        wcH_rfront[wcHf_cnt++] = conn;
		    }
		    else {
			conn -> ei -> wcH_r = conn -> ei -> wcH_r * resist
					      / (conn -> ei -> wcH_r + resist);
		    }
	        }
	        if (t -> state == Closed && forn -> ei -> lstate == L_state) {
		    if (conn -> ei -> wcL_path == NULL) {
		        conn -> ei -> wcL_r = resist;
		        conn -> ei -> wcL_path = forn;
		        wcL_rfront[wcLf_cnt++] = conn;
		    }
		    else {
			conn -> ei -> wcL_r = conn -> ei -> wcL_r * resist
					      / (conn -> ei -> wcL_r + resist);
		    }
	        }
	    }
	}
    }

    /* determine the minimum resistance paths */

    pathbcH ();
    pathbcL ();
    if (vicin.undeftors || vicin.forcedsX) {
        pathwcH ();
        pathwcL ();
    }
    else {
        nn = vicin.nodes;
        for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	    n = *nn++;

	    n -> ei -> wcL_path = n -> ei -> bcL_path;
	    n -> ei -> wcL_r = n -> ei -> bcL_r;
	    n -> ei -> wcH_path = n -> ei -> bcH_path;
	    n -> ei -> wcH_r = n -> ei -> bcH_r;
	}
    }

    /* calculate stable voltages from path values */

    vicin.floatpossible = FALSE;

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;

	if (n -> type != Forced && n -> ei -> wcH_path == NULL && n -> ei -> wcL_path == NULL) {
	    vicin.floatpossible = TRUE;
	}
	else
	    n -> ei -> posfloat = FALSE;

	if (!n -> ei -> undetermined)
	    continue;

        /* find the common part of the paths */

	n -> ei -> common = TRUE;
	lastcommon = trackstop = n;

	if (n -> ei -> bcH_path != NULL && n -> ei -> bcL_path != NULL) {
	    search = n;
	    while (search -> ei -> undetermined
		   && search -> ei -> bcH_path == search -> ei -> bcL_path
		   && (search -> ei -> wcH_path == search -> ei -> bcH_path
		       || search -> ei -> wcH_path == NULL)
		   && (search -> ei -> wcL_path == search -> ei -> bcL_path
		       || search -> ei -> wcL_path == NULL)) {
		search = search -> ei -> bcH_path;
		search -> ei -> common = TRUE;
	    }
	    lastcommon = trackstop = search;

	    if (search -> ei -> undetermined
		&& search -> ei -> bcH_path == search -> ei -> bcL_path) {

	        /* there may be a next common part. */
		/* therefore, first lay a track for the best case paths. */

		track = search;
		while (track -> type != Forced
		       && track -> ei -> bcH_path == track -> ei -> bcL_path) {
		    track = track -> ei -> bcH_path;
		    track -> ei -> ontrack = TRUE;
		}
		trackstop = track;

                /* and find the parts of the worst case path which are common */
		/* with that track */

		while (search -> type != Forced
		       && search != trackstop
		       && (search -> ei -> wcH_path == search -> ei -> wcL_path
			   || search -> ei -> wcH_path == NULL
			   || search -> ei -> wcL_path == NULL)) {
		    if (search -> ei -> wcL_path == NULL)
			search = search -> ei -> wcH_path;
		    else
			search = search -> ei -> wcL_path;
		    if (search -> ei -> ontrack) {
			search -> ei -> common = TRUE;
			lastcommon = search;
		    }
		}
	    }
	}

        /* the stable voltage will be calculated for the */
	/* last common path node */

	if (lastcommon -> type == Forced) {
	    switch (lastcommon -> ei -> lstate) {
		case H_state:
		    newsvmin = vH;
		    newsvmax = vH;
		    break;
		case L_state:
		    newsvmin = 0;
		    newsvmax = 0;
		    break;
		case X_state:
		    newsvmin = 0;
		    newsvmax = vH;
		    break;
		default:
		    newsvmin = newsvmax = 0;
		    ERROR_EXIT(1);
		    break;
	    }
	}
	else {
	    if (lastcommon -> ei -> undetermined) {
		if (lastcommon -> ei -> bcL_path == NULL)
		    lastcommon -> ei -> svmin = vH;
		else
		    if (lastcommon -> ei -> wcH_path == NULL)
			lastcommon -> ei -> svmin = 0;
		    else
			lastcommon -> ei -> svmin =
			    vH * lastcommon -> ei -> bcL_r
			    / (lastcommon -> ei -> bcL_r
				+ lastcommon -> ei -> wcH_r);

		if (lastcommon -> ei -> bcH_path == NULL)
		    lastcommon -> ei -> svmax = 0;
		else
		    if (lastcommon -> ei -> wcL_path == NULL)
			lastcommon -> ei -> svmax = vH;
		    else
			lastcommon -> ei -> svmax =
			    vH * lastcommon -> ei -> wcL_r
			    / (lastcommon -> ei -> wcL_r
				+ lastcommon -> ei -> bcH_r);

		lastcommon -> ei -> undetermined = FALSE;
	    }
	    newsvmin = lastcommon -> ei -> svmin;
	    newsvmax = lastcommon -> ei -> svmax;
	}

        /* and that stable voltage is assigned to all the nodes which */
	/* are on the common path parts */

	follow = n;
	while (follow != trackstop) {
	    follow -> ei -> ontrack = FALSE;
	    if (follow != lastcommon && follow -> ei -> common) {
		follow -> ei -> svmin = newsvmin;
		follow -> ei -> svmax = newsvmax;
		follow -> ei -> undetermined = FALSE;
	    }
	    follow -> ei -> common = FALSE;
	    follow = follow -> ei -> bcH_path;
	}
    }
}

static void pathbcH () /* search best case path to H */
{
    int     cnt;
    int     index;
    int     index_fn;
    int     index_m_r = 0;
    NODE * n;
    NODE * conn;
    TRANSISTOR * cont;
    float   resist;
    float   res;
    float   min_res;

    while (bcHf_cnt > 0) {
	min_res = MAXRESIST;
	for (index_fn = 0; index_fn < bcHf_cnt; index_fn++) {
	    res = bcH_rfront[index_fn] -> ei -> bcH_r;
	    if (res < min_res) {
		min_res = res;
		index_m_r = index_fn;
	    }
	}
	n = bcH_rfront[index_m_r];
	n -> ei -> bcH_done = TRUE;
	bcH_rfront[index_m_r] = bcH_rfront[--bcHf_cnt];

	index = n -> dsx;
	for (cnt = DS[index]; cnt > 0; cnt--) {
	    index++;
	    cont = &T[DS[index]];

	    if (cont -> state == Open)
		continue;

	    if (&N[cont -> source] == n)
		conn = &N[cont -> drain];
	    else
		conn = &N[cont -> source];

	    if (conn -> type == Forced || conn -> ei -> bcH_done)
		continue;

	    resist = n -> ei -> bcH_r + cont -> ei -> resist;
	    if (conn -> ei -> bcH_path == NULL) {
		conn -> ei -> bcH_r = resist;
		conn -> ei -> bcH_path = n;
		bcH_rfront[bcHf_cnt++] = conn;
	    }
	    else {
                if (conn -> ei -> bcH_path == n) {
                    /* merge parallel resistance */
                    conn -> ei -> bcH_r = n -> ei -> bcH_r
				  + (conn -> ei -> bcH_r - n -> ei -> bcH_r)
						* cont -> ei -> resist
				    / (conn -> ei -> bcH_r - n -> ei -> bcH_r
						+ cont -> ei -> resist);
                }
		else {
                   /* Temporarily store the total direct resistance between
                      n and conn; the total resistance may become smaller than
                      conn -> ei -> r, while a single resistance is not
                      smaller. */
                    if (conn -> ei -> bcH_dir != n) {
                        conn -> ei -> bcH_dir = n;
                        conn -> ei -> r_dir = cont -> ei -> resist;
                    }
                    else {
                        conn -> ei -> r_dir =
                                conn -> ei -> r_dir * cont -> ei -> resist
                                / (conn -> ei -> r_dir + cont -> ei -> resist);
                        resist = n -> ei -> bcH_r + conn -> ei -> r_dir;
                    }

		    if (conn -> ei -> bcH_r > resist) {
			conn -> ei -> bcH_r = resist;
			conn -> ei -> bcH_path = n;
		    }
                }
            }
	}
    }
}

static void pathbcL () /* search best case path to L */
{
    int     cnt;
    int     index;
    int     index_fn;
    int     index_m_r = 0;
    NODE * n;
    NODE * conn;
    TRANSISTOR * cont;
    float   resist;
    float   res;
    float   min_res;

    while (bcLf_cnt > 0) {
	min_res = MAXRESIST;
	for (index_fn = 0; index_fn < bcLf_cnt; index_fn++) {
	    res = bcL_rfront[index_fn] -> ei -> bcL_r;
	    if (res < min_res) {
		min_res = res;
		index_m_r = index_fn;
	    }
	}
	n = bcL_rfront[index_m_r];
	n -> ei -> bcL_done = TRUE;
	bcL_rfront[index_m_r] = bcL_rfront[--bcLf_cnt];

	index = n -> dsx;
	for (cnt = DS[index]; cnt > 0; cnt--) {
	    index++;
	    cont = &T[DS[index]];

	    if (cont -> state == Open)
		continue;

	    if (&N[cont -> source] == n)
		conn = &N[cont -> drain];
	    else
		conn = &N[cont -> source];

	    if (conn -> type == Forced || conn -> ei -> bcL_done)
		continue;

	    resist = n -> ei -> bcL_r + cont -> ei -> resist;
	    if (conn -> ei -> bcL_path == NULL) {
		conn -> ei -> bcL_r = resist;
		conn -> ei -> bcL_path = n;
		bcL_rfront[bcLf_cnt++] = conn;
	    }
	    else {
                if (conn -> ei -> bcL_path == n) {
                    /* merge parallel resistance */
                    conn -> ei -> bcL_r = n -> ei -> bcL_r
				  + (conn -> ei -> bcL_r - n -> ei -> bcL_r)
						* cont -> ei -> resist
				    / (conn -> ei -> bcL_r - n -> ei -> bcL_r
						+ cont -> ei -> resist);
                }
                else {
                   /* Temporarily store the total direct resistance between
                      n and conn; the total resistance may become smaller than
                      conn -> ei -> r, while a single resistance is not
                      smaller. */
                    if (conn -> ei -> bcL_dir != n) {
                        conn -> ei -> bcL_dir = n;
                        conn -> ei -> r_dir = cont -> ei -> resist;
                    }
                    else {
                        conn -> ei -> r_dir =
                                conn -> ei -> r_dir * cont -> ei -> resist
                                / (conn -> ei -> r_dir + cont -> ei -> resist);
                        resist = n -> ei -> bcL_r + conn -> ei -> r_dir;
                    }

                    if (conn -> ei -> bcL_r > resist) {
			conn -> ei -> bcL_r = resist;
			conn -> ei -> bcL_path = n;
		    }
		}
            }
	}
    }
}

static void pathwcH () /* search worst case path to H */
{
    int     cnt;
    int     index;
    int     index_fn;
    int     index_m_r = 0;
    NODE * n;
    NODE * conn;
    TRANSISTOR * cont;
    float   resist;
    float   res;
    float   min_res;

    while (wcHf_cnt > 0) {
	min_res = MAXRESIST;
	for (index_fn = 0; index_fn < wcHf_cnt; index_fn++) {
	    res = wcH_rfront[index_fn] -> ei -> wcH_r;
	    if (res < min_res) {
		min_res = res;
		index_m_r = index_fn;
	    }
	}
	n = wcH_rfront[index_m_r];
	n -> ei -> wcH_done = TRUE;
	wcH_rfront[index_m_r] = wcH_rfront[--wcHf_cnt];

	index = n -> dsx;
	for (cnt = DS[index]; cnt > 0; cnt--) {
	    index++;
	    cont = &T[DS[index]];

	    if (cont -> state != Closed)
		continue;

	    if (&N[cont -> source] == n)
		conn = &N[cont -> drain];
	    else
		conn = &N[cont -> source];

	    if (conn -> type == Forced || conn -> ei -> wcH_done)
		continue;

	    resist = n -> ei -> wcH_r + cont -> ei -> resist;
	    if (conn -> ei -> wcH_path == NULL) {
		conn -> ei -> wcH_r = resist;
		conn -> ei -> wcH_path = n;
		wcH_rfront[wcHf_cnt++] = conn;
	    }
	    else {
                if (conn -> ei -> wcH_path == n) {
                    /* merge parallel resistance */
                    conn -> ei -> wcH_r = n -> ei -> wcH_r
				  + (conn -> ei -> wcH_r - n -> ei -> wcH_r)
						* cont -> ei -> resist
				    / (conn -> ei -> wcH_r - n -> ei -> wcH_r
						+ cont -> ei -> resist);
                }
                else {
                   /* Temporarily store the total direct resistance between
                      n and conn; the total resistance may become smaller than
                      conn -> ei -> r, while a single resistance is not
                      smaller. */
                    if (conn -> ei -> wcH_dir != n) {
                        conn -> ei -> wcH_dir = n;
                        conn -> ei -> r_dir = cont -> ei -> resist;
                    }
                    else {
                        conn -> ei -> r_dir =
                                conn -> ei -> r_dir * cont -> ei -> resist
                                / (conn -> ei -> r_dir + cont -> ei -> resist);
                        resist = n -> ei -> wcH_r + conn -> ei -> r_dir;
                    }

                    if (conn -> ei -> wcH_r > resist) {
			conn -> ei -> wcH_r = resist;
			conn -> ei -> wcH_path = n;
		    }
                }
            }
	}
    }
}

static void pathwcL () /* search worst case path to L */
{
    int     cnt;
    int     index;
    int     index_fn;
    int     index_m_r = 0;
    NODE * n;
    NODE * conn;
    TRANSISTOR * cont;
    float   resist;
    float   res;
    float   min_res;

    while (wcLf_cnt > 0) {
	min_res = MAXRESIST;
	for (index_fn = 0; index_fn < wcLf_cnt; index_fn++) {
	    res = wcL_rfront[index_fn] -> ei -> wcL_r;
	    if (res < min_res) {
		min_res = res;
		index_m_r = index_fn;
	    }
	}
	n = wcL_rfront[index_m_r];
	n -> ei -> wcL_done = TRUE;
	wcL_rfront[index_m_r] = wcL_rfront[--wcLf_cnt];

	index = n -> dsx;
	for (cnt = DS[index]; cnt > 0; cnt--) {
	    index++;
	    cont = &T[DS[index]];

	    if (cont -> state != Closed)
		continue;

	    if (&N[cont -> source] == n)
		conn = &N[cont -> drain];
	    else
		conn = &N[cont -> source];

	    if (conn -> type == Forced || conn -> ei -> wcL_done)
		continue;

	    resist = n -> ei -> wcL_r + cont -> ei -> resist;
	    if (conn -> ei -> wcL_path == NULL) {
		conn -> ei -> wcL_r = resist;
		conn -> ei -> wcL_path = n;
		wcL_rfront[wcLf_cnt++] = conn;
	    }
	    else {
                if (conn -> ei -> wcL_path == n) {
                    /* merge parallel resistance */
                    conn -> ei -> wcL_r = n -> ei -> wcL_r
				  + (conn -> ei -> wcL_r - n -> ei -> wcL_r)
						* cont -> ei -> resist
				    / (conn -> ei -> wcL_r - n -> ei -> wcL_r
						+ cont -> ei -> resist);
                }
                else {
                   /* Temporarily store the total direct resistance between
                      n and conn; the total resistance may become smaller than
                      conn -> ei -> r, while a single resistance is not
                      smaller. */
                    if (conn -> ei -> wcL_dir != n) {
                        conn -> ei -> wcL_dir = n;
                        conn -> ei -> r_dir = cont -> ei -> resist;
                    }
                    else {
                        conn -> ei -> r_dir =
                                conn -> ei -> r_dir * cont -> ei -> resist
                                / (conn -> ei -> r_dir + cont -> ei -> resist);
                        resist = n -> ei -> wcL_r + conn -> ei -> r_dir;
                    }

		    if (conn -> ei -> wcL_r > resist) {
			conn -> ei -> wcL_r = resist;
			conn -> ei -> wcL_path = n;
                    }
		}
            }
	}
    }
}
