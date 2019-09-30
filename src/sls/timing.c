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
static void accumulate (int speed);
static void backtrace_T (NODE *n, int speed);
static void det_TD (int change, int speed);
static void fillparam (int change, int speed);
static int  findforcedsource (int change, int speed);
static void findnormalsource (int change, int speed);
static void modedeterm (int change, int speed);
static void path (int speed);
static void tree_TD (int change, int speed);

NODE ** rfront = NULL;   /* search front paths to source nodes */
NODE ** surround = NULL; /* nodes surrounding the forced nodes */
int     f_cnt;
int     s_cnt;

double kchange;

void inittiming ()
{
    if (rfront) CFREE (rfront);
    if (surround) CFREE (surround);
    PPALLOC (rfront, maxnvicin, NODE);
    PPALLOC (surround, maxnvicin, NODE);
}

void timing () /* determine the dynamic behavior parameters */
{
    int     risefast;
    int     fallfast;
    int     riseslow;
    int     fallslow;
    int     minnotmax;
    int     cnt;
    double  Tt;
    NODE ** nn;
    NODE * n;
    simtime_t help;

    risefast = FALSE;
    fallfast = FALSE;
    riseslow = FALSE;
    fallslow = FALSE;
    minnotmax = FALSE;

    /* first it is investigated what kind of time constant calculations
       have to be done */

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;

	if (n -> type == Forced) continue;

	if ((double)n -> svmax > n -> ei -> umax)
	    risefast = TRUE;
	else
	    if ((double)n -> svmax < n -> ei -> umax)
		fallslow = TRUE;

	if ((double)n -> svmin < n -> ei -> umin)
	    fallfast = TRUE;
	else
	    if ((double)n -> svmin > n -> ei -> umin)
		riseslow = TRUE;

	if (n -> ei -> umin != n -> ei -> umax)
	    minnotmax = TRUE;
    }

    riseslow = riseslow && (minnotmax || vicin.undeftors || vicin.forcedsX || !risefast);
    fallslow = fallslow && (minnotmax || vicin.undeftors || vicin.forcedsX || !fallfast);

    if (risefast) {
	det_TD (RISE, FAST);

	nn = vicin.nodes;
	for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	    n = *nn++;

	    if (n -> type == Forced) continue;

	    if ((double)n -> svmax > n -> ei -> umax) {
    	        Tt = 2 * n -> ei -> TD
		     / (((double)n -> svmax - n -> ei -> umax) * outtimeaccur);
                if (Tt > MAXSIMTIME) {
                    slserror (NULL, 0, WARNING, "too large delay occurred", NULL);
		    n -> Ttmax = MAXSIMTIME;
                }
                else if (Tt < 0)
                    n -> Ttmax = 0;
                else
                    n -> Ttmax = Tt;
	    }
	    if ((double)n -> svmin > n -> ei -> umin && !riseslow) {
		Tt = 2 * n -> ei -> TD
		     / (((double)n -> svmin - n -> ei -> umin) * outtimeaccur);
                if (Tt > MAXSIMTIME) {
                    slserror (NULL, 0, WARNING, "too large delay occurred", NULL);
		    n -> Ttmin = MAXSIMTIME;
                }
                else if (Tt < 0)
                    n -> Ttmin = 0;
                else
                    n -> Ttmin = Tt;
	    }
	}
    }

    if (fallfast) {
	det_TD (FALL, FAST);

	nn = vicin.nodes;
	for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	    n = *nn++;

	    if (n -> type == Forced) continue;

	    if ((double)n -> svmin < n -> ei -> umin) {
		Tt = 2 * n -> ei -> TD
		     / ((n -> ei -> umin - (double)n -> svmin) * outtimeaccur);
                if (Tt > MAXSIMTIME) {
                    slserror (NULL, 0, WARNING, "too large delay occurred", NULL);
		    n -> Ttmin = MAXSIMTIME;
                }
                else if (Tt < 0)
                    n -> Ttmin = 0;
                else
                    n -> Ttmin = Tt;
	    }
	    if ((double)n -> svmax < n -> ei -> umax && !fallslow) {
		Tt = 2 * n -> ei -> TD
		     / ((n -> ei -> umax - (double)n -> svmax) * outtimeaccur);
                if (Tt > MAXSIMTIME) {
                    slserror (NULL, 0, WARNING, "too large delay occurred", NULL);
		    n -> Ttmax = MAXSIMTIME;
                }
                else if (Tt < 0)
                    n -> Ttmax = 0;
                else
                    n -> Ttmax = Tt;
	    }
	}
    }

    if (riseslow) {
	det_TD (RISE, SLOW);

	nn = vicin.nodes;
	for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	    n = *nn++;

	    if (n -> type == Forced) continue;

	    if ((double)n -> svmin > n -> ei -> umin) {
		Tt = 2 * n -> ei -> TD
		     / (((double)n -> svmin - n -> ei -> umin) * outtimeaccur);
                if (Tt > MAXSIMTIME) {
                    slserror (NULL, 0, WARNING, "too large delay occurred", NULL);
		    n -> Ttmin = MAXSIMTIME;
                }
                else if (Tt < 0)
                    n -> Ttmin = 0;
                else
                    n -> Ttmin = Tt;
	    }
	}
    }

    if (fallslow) {
	det_TD (FALL, SLOW);

	nn = vicin.nodes;
	for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	    n = *nn++;

	    if (n -> type == Forced) continue;

	    if ((double)n -> svmax < n -> ei -> umax) {
		Tt = 2 * n -> ei -> TD
		     / ((n -> ei -> umax - (double)n -> svmax) * outtimeaccur);
                if (Tt > MAXSIMTIME) {
                    slserror (NULL, 0, WARNING, "too large delay occurred", NULL);
		    n -> Ttmax = MAXSIMTIME;
                }
                else if (Tt < 0)
                    n -> Ttmax = 0;
                else
                    n -> Ttmax = Tt;
	    }
	}
    }

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;

	if (n -> type == Forced) continue;

	if (n -> Ttmin < 0) n -> Ttmin = 0;
	if (n -> Ttmax < 0) n -> Ttmax = 0;
        /* this namely may occur sometimes */

        if (n -> svmin > n -> ivmax) {
 	    if (n -> Ttmin < n -> Ttmax
 		&& (double)n -> svmin > (double)n -> ivmax + ((double)n -> Ttmin / (double)n -> Ttmax)
 				* ((double)n -> svmax - (double)n -> ivmax)) {
 		help = n -> Ttmax;
 		n -> Ttmax = n -> Ttmin;
 		n -> Ttmin = help;
 	    }
 	}
        else if (n -> svmax < n -> ivmin) {
 	    if (n -> Ttmax < n -> Ttmin
 		&& (double)n -> svmax < (double)n -> ivmin + ((double)n -> Ttmax / (double)n -> Ttmin)
 				* ((double)n -> svmin - (double)n -> ivmin)) {
 		help = n -> Ttmin;
 		n -> Ttmin = n -> Ttmax;
 		n -> Ttmax = help;
 	    }
 	}
 	/* to be sure that umin(t) <= umax(t) for all t */

        if ((double)n -> svmin > n -> ei -> umin) {
	    n -> Ttmin = n -> Ttmin * maxdevtime;
	    n -> tstabmin = tcurr + n -> Ttmin;
	}
	else if ((double)n -> svmin < n -> ei -> umin) {
	    n -> Ttmin = n -> Ttmin * mindevtime;
	    n -> tstabmin = tcurr + n -> Ttmin;
	}

        if ((double)n -> svmax > n -> ei -> umax) {
	    n -> Ttmax = n -> Ttmax * mindevtime;
	    n -> tstabmax = tcurr + n -> Ttmax;
	}
	else if ((double)n -> svmax < n -> ei -> umax) {
	    n -> Ttmax = n -> Ttmax * maxdevtime;
	    n -> tstabmax = tcurr + n -> Ttmax;
	}

	if (debugsim) {
	    fprintf (debug, "\nnode: %s\n", hiername (n - N) );
	    fprintf (debug, "tcurr: %lld\n", tcurr);
	    fprintf (debug, "ei -> umin: %f ei -> umax: %f svmin: %d svmax: %d\n",
			n -> ei -> umin, n -> ei -> umax, n -> svmin, n -> svmax);
	    fprintf (debug, "Ttmin: %lld  ", n -> Ttmin);
	    fprintf (debug, "Ttmax: %lld\n", n -> Ttmax);
	    fprintf (debug, "tstabmin: %lld  ", n -> tstabmin);
	    fprintf (debug, "tstabmax: %lld", n -> tstabmax);
	    fprintf (debug, "\n");
	}
    }

    if (debugsim) fprintf (debug, "\n");
}

/* determine a TD for a fast or slow rising or falling voltage */
static void det_TD (int change, int speed)
{
    int cnt;
    NODE **nn, *n;
    TRANSISTOR **tt, *t;

    if (debugsim) {
	fprintf (debug, "***** det_TD *****");
	if (change == RISE)
	    fprintf (debug, " RISE");
	else
	    fprintf (debug, " FALL");
	if (speed == FAST)
	    fprintf (debug, " FAST");
	else
	    fprintf (debug, " SLOW");
	fprintf (debug, " ******\n\n");
    }

    if (vicin.type == Forced) {
	if (!findforcedsource (change, speed))
	    findnormalsource (change, speed);
    }
    else {
	findnormalsource (change, speed);
    }

    modedeterm (change, speed);
    fillparam (change, speed);

    if (debugsim) {
	nn = vicin.nodes;
	for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	    n = *nn++;
	    fprintf (debug, "node=%s  dyncap=%e\n",
		    hiername (n - N), n -> ei -> dyncap);
	}

        tt = vicin.tors;
        for (cnt = vicin.tor_cnt; cnt > 0; cnt--) {
	    t = *tt++;

	    fprintf (debug, "tor=%d  mode=", (int)(t - T));
	    switch (t -> ei -> mode) {
		case Pulldown :
		    fprintf (debug, "Pulldown");
		    break;
		case Pullup :
		    fprintf (debug, "Pullup");
		    break;
		case Passdown :
		    fprintf (debug, "Passdown");
		    break;
		case Passup :
		    fprintf (debug, "Passup");
		    break;
		case Load :
		    fprintf (debug, "Load");
		    break;
		case Superload :
		    fprintf (debug, "Superload");
		    break;
		case 0 :
		    if (t -> type == Res)
			fprintf (debug, "NULL(Resistor)");
		    else
			ERROR_EXIT (1);
		    break;
		default :
		    ERROR_EXIT (1);
	    }

            fprintf (debug, " ");
	    switch (t -> state) {
		case Open :
		    fprintf (debug, "Open");
		    break;
		case Closed :
		    fprintf (debug, "Closed");
		    break;
		case Undefined :
		    fprintf (debug, "Undefined");
		    break;
	    }
	    fprintf (debug, " resist=%e\n", t -> ei -> resist);
	}

	fprintf (debug, "\n");
    }

    tree_TD (change, speed);
}

/* find source(s) which is (are) Forced node
 * returns FALSE if no source can be found
 */
static int findforcedsource (int change, int speed)
{
    int cnt, sourcefound;
    NODE **nn, *n;

    sourcefound = FALSE;
    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;
	if (n -> type == Forced
	    && ((n -> ei -> lstate == X_state && speed == FAST)
	         || (n -> ei -> lstate == H_state && change == RISE)
	         || (n -> ei -> lstate == L_state && change == FALL))) {
	    n -> ei -> source = TRUE;
	    n -> ei -> TD = 0;
	    sourcefound = TRUE;

	    if (debugsim) fprintf (debug, "source: %s\n", hiername (n - N));
	}
	else {
	    n -> ei -> source = FALSE;
	}
    }

    return (sourcefound);
}

/* find source which is a Normal node */
static void findnormalsource (int change, int speed)
{
    float amount, maxamount;
    int   cnt;
    NODE **nn, *n, *source_n;

    maxamount = 0;
    source_n = NULL;
    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;
	n -> ei -> source = FALSE;

	if (n -> type == Forced) continue;

	if (change == RISE) {
	    if (speed == FAST)
		amount = n -> statcap * n -> ei -> umax;
	    else
		amount = n -> statcap * n -> ei -> umin;
	}
	else {
	    if (speed == FAST)
		amount = n -> statcap * (vH - n -> ei -> umin);
	    else
		amount = n -> statcap * (vH - n -> ei -> umax);
	}
	if (amount > maxamount || source_n == NULL) {
	    source_n = n;
	    maxamount = amount;
	}
    }

    if (source_n == NULL) {
	ERROR_EXIT (1);  /* can't find source node */
    }

    source_n -> ei -> source = TRUE;
    source_n -> ei -> TD = 0;

    if (debugsim) fprintf (debug, "source: %s\n", hiername (source_n - N));
}

/* determine the mode of the transistors */
static void modedeterm (int change, int speed)
{
    int cnt;
    TRANSISTOR **tt, *t;

    tt = vicin.tors;
    for (cnt = vicin.tor_cnt; cnt > 0; cnt--) {
	t = *tt++;

	switch (t -> type) {
	    case Nenh:
	    case Penh:
		if (t -> premode == Passup) {
		    if (change == RISE)
			t -> ei -> mode = Passup;
		    else
			t -> ei -> mode = Passdown;
		}
		else if (t -> premode == Pullup) {
		    if (change == RISE)
			t -> ei -> mode = Pullup;
		    else
			t -> ei -> mode = Pulldown;
		}
		else
		    t -> ei -> mode = 0; /* NULL */
		break;
	    case Depl:
		t -> ei -> mode = t -> premode;
		break;
	    case Res:
		t -> ei -> mode = 0; /* NULL */
		break;
	}
    }
}

/* fill in the appropiate capacitances and resistances */
static void fillparam (int change, int speed)
{
    int cnt, cnt2, index;
    NODE **nn, *n;
    TRANSISTOR **tt, *t;
    float cch;

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;

        if (n -> type == Forced) continue;

	n -> ei -> dyncap = n -> dyncap;

	if (n -> dsx != -1) {
	    index = n -> dsx;
	    for (cnt2 = DS[index]; cnt2 > 0; cnt2--) {
		index++;
		t = &T[DS[index]];
		if (change == RISE)
		    n -> ei -> dyncap += torpar (t, 0, Cerise);
		else
		    n -> ei -> dyncap += torpar (t, 0, Cefall);
	    }
	}

	if (n -> cx != -1) {
	    index = n -> cx;
	    for (cnt2 = C[index].c; cnt2 > 0; cnt2--) {
		index++;
		if (C[index].sort == Transistor) {
		    t = &T[C[index].c];
		    if (change == RISE)
			n -> ei -> dyncap += torpar (t, 0, Cgrise);
		    else
			n -> ei -> dyncap += torpar (t, 0, Cgfall);
		}
	    }
	}
    }

    tt = vicin.tors;
    for (cnt = vicin.tor_cnt; cnt > 0; cnt--) {
	t = *tt++;

	cch = torpar (t, (int) t -> ei -> mode, Cch);
	N[t -> drain].ei -> dyncap += cch;
	N[t -> source].ei -> dyncap += cch;

	t -> ei -> resist = torpar (t, (int) t -> ei -> mode, Rdyn);
    }
}

static void tree_TD (int change, int speed)
{
    int cnt;
    double res, resist, vdelta;
    NODE **nn, *n, *conn, *sourcen, *wcpathn;
    TRANSISTOR **tt, *t;

    /* initialize variables and find initial accum charge */

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;

	n -> ei -> path = NULL;
	n -> ei -> dir = NULL;
	n -> ei -> done = FALSE;
	n -> ei -> leaf = TRUE;    /* as long as no path points to n, n is
				      assumed to be a leaf */
	n -> ei -> backtraced = FALSE;
	n -> ei -> accumulated = FALSE;
	if (change == RISE) {
	    if (speed == FAST)
		vdelta = (double)n -> svmax - n -> ei -> umax;
	    else
		vdelta = (double)n -> svmin - n -> ei -> umin;
	}
	else {
	    if (speed == FAST)
		vdelta = n -> ei -> umin - (double)n -> svmin;
	    else
		vdelta = n -> ei -> umax - (double)n -> svmax;
	}
	n -> ei -> accumcharge = n -> ei -> dyncap * vdelta;
    }

    /* nodes connected to source nodes are put on the front list. */
    /* simultaneously the nodes which surround the source nodes are put */
    /* on the front list. */

    s_cnt = 0;
    f_cnt = 0;

    tt = vicin.tors;
    for (cnt = vicin.tor_cnt; cnt > 0; cnt--) {
	t = *tt++;

	if (t -> state == Undefined && speed == SLOW)
	    continue;

	if (N[t -> drain].ei -> source) {
	    conn = &N[t -> source];
	    sourcen = &N[t -> drain];
	}
	else
	    if (N[t -> source].ei -> source) {
		conn = &N[t -> drain];
		sourcen = &N[t -> source];
	    }
	    else
		continue;

	if (!conn -> ei -> source && conn -> type != Forced) {
	    resist = t -> ei -> resist;
	    if (conn -> ei -> path == NULL) {
		conn -> ei -> r = resist;
		conn -> ei -> path = sourcen;
		sourcen -> ei -> leaf = FALSE;
		rfront[f_cnt++] = conn;
		surround[s_cnt++] = conn;
		if (t -> state == Undefined)
		    conn -> ei -> xtorpath = TRUE;
		else
		    conn -> ei -> xtorpath = FALSE;
	    }
	    else {
		/* merge parallel resistance */
		res = conn -> ei -> r;
		conn -> ei -> r = (res * resist) / (res + resist);
		if (t -> state == Undefined)
		    conn -> ei -> xtorpath = TRUE;
		else
		    conn -> ei -> xtorpath = FALSE;
	    }
	}
    }

    /* find the minimum resistance paths */

    path (speed);

    if (speed == SLOW) {
	/* when speed is slow paths are not searched through Undefined
	   transistors and some nodes will have no path.  therefore
	   an additional search is done for the resulting nodes, with
	   Undefined transistors considered to be Closed */

	f_cnt = 0;

        tt = vicin.tors;
        for (cnt = vicin.tor_cnt; cnt > 0; cnt--) {
	    t = *tt++;

	    if (N[t -> drain].ei -> source || N[t -> drain].ei -> path != NULL) {
	        conn = &N[t -> source];
	        wcpathn = &N[t -> drain];
	    }
	    else
	        if (N[t -> source].ei -> source
	        || N[t -> source].ei -> path != NULL) {
		    conn = &N[t -> drain];
		    wcpathn = &N[t -> source];
	        }
	        else
		    continue;

	    if (!conn -> ei -> source && conn -> type != Forced && !conn -> ei -> done) {
	        resist = t -> ei -> resist;
	        if (conn -> ei -> path == NULL) {
		    conn -> ei -> r = resist;
		    conn -> ei -> path = wcpathn;
		    wcpathn -> ei -> leaf = FALSE;
		    rfront[f_cnt++] = conn;
		    if (wcpathn -> ei -> source)
		        surround[s_cnt++] = conn;
		    if (t -> state == Undefined)
		        conn -> ei -> xtorpath = TRUE;
		    else
		        conn -> ei -> xtorpath = FALSE;
	        }
	        else {
		    /* merge parallel resistance */
		    res = conn -> ei -> r;
		    conn -> ei -> r = (res * resist) / (res + resist);
		    if (t -> state == Undefined)
			conn -> ei -> xtorpath = TRUE;
		    else
			conn -> ei -> xtorpath = FALSE;
		}
	    }
        }

	path (FAST);
    }

    accumulate (speed);

    if (change == RISE)
	kchange = krise;
    else if (change == FALL)
	kchange = kfall;
    else
	ERROR_EXIT (1);

    /* start the back tracing at the nodes which are on the surround list */

    while (s_cnt > 0) {
	n = surround[--s_cnt];
	n -> ei -> TD = kchange * n -> ei -> r * n -> ei -> accumcharge;
	n -> ei -> backtraced = TRUE;

	if (debugsim) {
	    fprintf (debug, "backtrace: %s  TD: %e  r: %e  accumcharge: %e\n",
		    hiername (n - N), n -> ei -> TD,
		    n -> ei -> r, n -> ei -> accumcharge);
	}

	backtrace_T (n, speed);
    }
}

/* find the minimum resistance paths to the source nodes */
static void path (int speed)
{
    int cnt, index, index_fn, index_m_r = 0;
    NODE *n, *conn;
    TRANSISTOR *cont;
    float resist, res, min_res;

    while (f_cnt > 0) {
	min_res = MAXRESIST;
	for (index_fn = 0; index_fn < f_cnt; index_fn++) {
	    res = rfront[index_fn] -> ei -> r;
	    if (res < min_res) {
		min_res = res;
		index_m_r = index_fn;
	    }
	}
	n = rfront[index_m_r];
	n -> ei -> done = TRUE;
	rfront[index_m_r] = rfront[--f_cnt];

	index = n -> dsx;
	for (cnt = DS[index]; cnt > 0; cnt--) {
	    index++;
	    cont = &T[DS[index]];

	    if (cont -> state == Open
	    || (cont -> state == Undefined && speed == SLOW))
		continue;

	    if (&N[cont -> source] == n)
		conn = &N[cont -> drain];
	    else
		conn = &N[cont -> source];

	    if (conn -> ei -> source
	    || conn -> type == Forced
	    || conn -> ei -> done)
		continue;

	    resist = n -> ei -> r + cont -> ei -> resist;
	    if (conn -> ei -> path == NULL) {
		conn -> ei -> r = resist;
		conn -> ei -> path = n;
		n -> ei -> leaf = FALSE;
		rfront[f_cnt++] = conn;
		if (cont -> state == Undefined)
		    conn -> ei -> xtorpath = TRUE;
		else
		    conn -> ei -> xtorpath = FALSE;
	    }
	    else {
		if (conn -> ei -> path == n) {
		    /* merge parallel resistance */
		    res = conn -> ei -> r - n -> ei -> r;
		    conn -> ei -> r = n -> ei -> r
			      + res * cont -> ei -> resist / (res + cont -> ei -> resist);
		}
		else {
		    /* Temporarily store the total direct resistance between
		       n and conn; the total resistance may become smaller than
		       conn -> ei -> r, while a single resistance is not
		       smaller.*/
		    if (conn -> ei -> dir != n) {
			conn -> ei -> dir = n;
			conn -> ei -> r_dir = cont -> ei -> resist;
		    }
		    else {
			conn -> ei -> r_dir = conn -> ei -> r_dir * cont -> ei -> resist
					    / (conn -> ei -> r_dir + cont -> ei -> resist);
			resist = n -> ei -> r + conn -> ei -> r_dir;
		    }

		    if (conn -> ei -> r > resist) {
			conn -> ei -> r = resist;
			conn -> ei -> path = n;
			n -> ei -> leaf = FALSE;
			if (cont -> state == Undefined)
			    conn -> ei -> xtorpath = TRUE;
			else
			    conn -> ei -> xtorpath = FALSE;
		    }
		}
	    }
	}
    }
}

/* accumulate charges from the leaves to the source nodes */
static void accumulate (int speed)
{
    int cnt, ncnt, followon, index;
    NODE **nn, *n;
    float loadcharge;
    TRANSISTOR *cont;

    nn = vicin.nodes;
    for (ncnt = vicin.node_cnt; ncnt > 0; ncnt--) {
	n = *nn++;

	if (!n -> ei -> leaf || n -> ei -> source || n -> type == Forced) continue;

	followon = TRUE;
	do {
	    if (speed == FAST && n -> ei -> xtorpath)
		loadcharge = 0;
	    else
	        loadcharge = n -> ei -> accumcharge;
	    n -> ei -> accumulated = TRUE;
	    n = n -> ei -> path;
	    n -> ei -> accumcharge += loadcharge;
	    if (n -> ei -> source) {
		followon = FALSE;
	    }
	    else {
		index = n -> dsx;
		for (cnt = DS[index]; cnt > 0 && followon; cnt--) {
		    index++;
		    cont = &T[DS[index]];

		    if (cont -> state == Open) continue;

		    if ((!N[cont -> source].ei -> accumulated
				&& N[cont -> source].ei -> path == n)
			    || (!N[cont -> drain].ei -> accumulated
				&& N[cont -> drain].ei -> path == n)
			) {
			followon = FALSE;
		    }
		}
	    }
	}
	while (followon);

    }
}

/* trace the paths back from n recursively
 * and determine TD for the nodes on the paths
 */
static void backtrace_T (NODE *n, int speed)
{
    int cnt, index;
    NODE *conn;
    TRANSISTOR *cont;

    index = n -> dsx;
    for (cnt = DS[index]; cnt > 0; cnt--) {
	index++;
	cont = &T[DS[index]];

	if (cont -> state == Open) continue;

	if (&N[cont -> source] == n)
	    conn = &N[cont -> drain];
	else
	    conn = &N[cont -> source];

	if (conn -> type != Forced && conn -> ei -> path == n && !conn -> ei -> backtraced) {
	    if (speed == FAST && conn -> ei -> xtorpath) {
	        conn -> ei -> TD = n -> ei -> TD + (conn -> ei -> r)
		                     * conn -> ei -> accumcharge * kchange;
	    }
	    else {
	        conn -> ei -> TD = n -> ei -> TD + (conn -> ei -> r - n -> ei -> r)
		                     * conn -> ei -> accumcharge * kchange;
	    }
	    conn -> ei -> backtraced = TRUE;

	    if (debugsim) {
		fprintf (debug, "backtrace: %s TD: %e  r: %e accumcharge: %e\n",
			hiername (conn - N) , conn -> ei -> TD,
			conn -> ei -> r, conn -> ei -> accumcharge);
	    }

	    backtrace_T (conn, speed);
	}
    }
}
