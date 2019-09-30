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

extern void chargeshare (void);
extern void initchargesh (void);
extern void initresistdiv (void);
extern void inittiming (void);
extern void resistdivide (void);
extern int  stabstateof (double vmin, double vmax);
extern void startvsearch (NODE *vn);
extern void timing (void);
extern void vclose (void);
extern void vicin_dis (void);
extern void vicin_nsource_dis (void);

static void vsched (void);
static void vsearch (NODE *n);

NODE ** vnptrs;        /* vicinity node pointers */
TRANSISTOR ** vtptrs;  /* vicinity transistor pointers */
NEVALINFO * neis;      /* node evaluation-info's */
TEVALINFO * teis;      /* transistor evaluation-info's */

NODE ** freevnptrs;
TRANSISTOR ** freevtptrs;
NEVALINFO * freeneis;
TEVALINFO * freeteis;

void initeval ()
{
    maxnvicin = 50;   /* initial guess */
    maxtvicin = 50;   /* initial guess */
    initchargesh ();
    initresistdiv ();
    inittiming ();
    PPALLOC (vnptrs, maxnvicin, NODE);
    PPALLOC (vtptrs, maxtvicin, TRANSISTOR);
    PALLOC (neis, maxnvicin, NEVALINFO);
    PALLOC (teis, maxtvicin, TEVALINFO);
    act_maxnvicin = 0;
    act_maxtvicin = 0;
}

void evalvicinity (NODE *vn) /* evaluate vicinity of node vn */
{
    int cnt;
    NODE ** nn;
    NODE * n;

    if (debugsim)
	fprintf (debug, "\n---------- evalvicinity : %s ----------\n\n",
	hiername (vn - N) );

    startvsearch (vn);

    if (dissip) vicin_dis ();

    /* find stable voltages */

    if (vicin.forcedsH && !vicin.forcedsL && !vicin.forcedsX && !vicin.undeftors) {
	vicin.type = Forced;
	nn = vicin.nodes;
	for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	    n = *nn++;

	    if (n -> type == Forced) continue;

	    n -> ei -> svmin = vH;
	    n -> ei -> svmax = vH;
	}
    }
    else
	if (vicin.forcedsL && !vicin.forcedsH && !vicin.forcedsX && !vicin.undeftors) {
	    vicin.type = Forced;
	    nn = vicin.nodes;
	    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
		n = *nn++;

		if (n -> type == Forced) continue;

		n -> ei -> svmin = 0;
		n -> ei -> svmax = 0;
	    }
	}
	else
	    if (vicin.forcedsX && !vicin.forcedsH && !vicin.forcedsL) {
		vicin.type = Forced;
		nn = vicin.nodes;
		for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
		    n = *nn++;

		    if (n -> type == Forced) continue;

		    n -> ei -> svmin = 0;
		    n -> ei -> svmax = vH;
		}
	    }
	    else
		if (!vicin.forcedsH && !vicin.forcedsL && !vicin.forcedsX) {
		    vicin.type = Normal;
		    chargeshare ();
		}
		else {
		    vicin.type = Forced;
		    resistdivide ();
		    if (vicin.floatpossible)
			chargeshare ();
		}

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;

	if (n -> type == Forced) continue;

	n -> stabstate = stabstateof (n -> ei -> svmin, n -> ei -> svmax);
	n -> ivmin = n -> ei -> umin;
	n -> ivmax = n -> ei -> umax;
	n -> svmin = n -> ei -> svmin;
	n -> svmax = n -> ei -> svmax;

	if (! proclogic) {
            if (n -> stabstate == X_state) {
                n -> svmin = 0;
                n -> svmax = vH;
            }
            else {
	        if ((double)n -> svmin < vswitch) n -> svmin = 0;
	        else n -> svmin = vH;
	        if ((double)n -> svmax < vswitch) n -> svmax = 0;
	        else n -> svmax = vH;
            }
	    if ((double)n -> ivmin < vswitch) n -> ivmin = 0;
	    else n -> ivmin = vH;
	    if ((double)n -> ivmax < vswitch) n -> ivmax = 0;
	    else n -> ivmax = vH;
	}
    }

    if (debugsim) {
	nn = vicin.nodes;
	for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	    n = *nn++;

	    if (n -> type == Forced) continue;

	    fprintf (debug, "%s :  svmin = %d svmax = %d ",
		    hiername (n - N), n -> svmin, n -> svmax);
	    fprintf (debug, "stabstate : %d\n", n -> stabstate);
	}
	fprintf (debug, "\n");
    }

    /* now, find dynamical behavior parameters */

    if (stepdelay) {
        nn = vicin.nodes;
        for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	    n = *nn++;

	    if (n -> type == Forced) continue;

	    n -> tstabmin = tcurr;
	    n -> tstabmax = tcurr;
	    n -> Ttmin = 0;
	    n -> Ttmax = 0;
        }
    }
    else {
	timing ();
    }

    vsched ();

    if (dissip) vicin_nsource_dis ();

    vclose ();
}

void startvsearch (NODE *vn)
{
    int cnt;
    int nbr;
    NODE ** nn;
    NODE * n;
    TRANSISTOR ** tt;
    TRANSISTOR * t;
    UPAIR * up;

    freevnptrs = vnptrs;
    vicin.nodes = freevnptrs;
    freevtptrs = vtptrs;
    vicin.tors = freevtptrs;
    vicin.node_cnt = 0;
    vicin.tor_cnt = 0;
    freeneis = neis;
    freeteis = teis;

    vsearch (vn);

    nbr = freevnptrs - vnptrs;
    if (nbr > act_maxnvicin) act_maxnvicin = nbr;
    nbr = freevtptrs - vtptrs;
    if (nbr > act_maxtvicin) act_maxtvicin = nbr;

    vicin.forcedsH = vicin.forcedsL = vicin.forcedsX = FALSE;
    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;

	n -> flag = FALSE;
	if (n -> essential)
	    n -> ei -> lstate = LSTATE(n);
	if (n -> essential && n -> type == Normal
	&& n -> ei -> lstate != n -> nextstate)
	    n -> ei -> eventpending = TRUE;
	else
	    n -> ei -> eventpending = FALSE;
	if (n -> type == Forced) {
	    switch (n -> ei -> lstate) {
		case H_state:
		    n -> ei -> svmin = vH;
		    n -> ei -> svmax = vH;
		    vicin.forcedsH = TRUE;
		    break;
		case L_state:
		    n -> ei -> svmin = 0;
		    n -> ei -> svmax = 0;
		    vicin.forcedsL = TRUE;
		    break;
		case X_state:
		    n -> ei -> svmin = 0;
		    n -> ei -> svmax = vH;
		    vicin.forcedsX = TRUE;
		    break;
	    }
	}
	else {
	    up = uminmax (n);
	    n -> ei -> umin = up -> umin;
	    n -> ei -> umax = up -> umax;
	}
    }

    vicin.undeftors = FALSE;
    tt = vicin.tors;
    for (cnt = vicin.tor_cnt; cnt > 0; cnt--) {
	t = *tt++;
	t -> flag = FALSE;
	if (t -> state == Undefined)
	    vicin.undeftors = TRUE;
    }
}

static void vsearch (NODE *n) /* search vicinity recursively */
{
    int     i;
    int     cnt;
    int     index;
    TRANSISTOR * t;   /* transistor connected to n by drain or source */
    NODE * con;       /* node connected to n by t */
    NODE ** nn;
    TRANSISTOR ** tt;

    n -> flag = TRUE;
    if (vicin.node_cnt >= maxnvicin) {
        /* maxnvicin appears to be too small, so it is enlarged */
	CFREE (neis);
	PALLOC (neis, (1.5 * maxnvicin), NEVALINFO);
	freeneis = neis;
	PPALLOC (vnptrs, (1.5 * maxnvicin), NODE);
	freevnptrs = vnptrs;
	nn = vicin.nodes;
	for (i = vicin.node_cnt; i > 0; i--) {
	    *freevnptrs++ = *nn;
	    (*nn++) -> ei = freeneis++;
	}
	CFREE (vicin.nodes);
	vicin.nodes = vnptrs;
	maxnvicin = (1.5 * maxnvicin);
	initchargesh ();
	initresistdiv ();
	inittiming ();
    }
    *freevnptrs++ = n;
    vicin.node_cnt++;
    n -> ei = freeneis++;

    if (n -> type == Forced) {
	n -> ei -> posfloat = FALSE;
	return;
    }
    n -> ei -> posfloat = TRUE;
    if (n -> dsx < 0)
	return;

    index = n -> dsx;
    for (cnt = DS[index]; cnt > 0; cnt--) {
	index++;
	t = &T[DS[index]];
	if (!t -> flag && t -> state != Open) {
	    t -> flag = TRUE;
	    if (vicin.tor_cnt >= maxtvicin) {
                /* maxtvicin appears to be too small, so it is enlarged */
	        CFREE (teis);
	        PALLOC (teis, (1.5 * maxtvicin), TEVALINFO);
	        freeteis = teis;
	        PPALLOC (vtptrs, (1.5 * maxtvicin), TRANSISTOR);
	        freevtptrs = vtptrs;
	        tt = vicin.tors;
	        for (i = vicin.tor_cnt; i > 0; i--) {
	            *freevtptrs++ = *tt;
	            (*tt++) -> ei = freeteis++;
	        }
	        CFREE (vicin.tors);
	        vicin.tors = vtptrs;
	        maxtvicin = (1.5 * maxtvicin);
	        initchargesh ();
	        initresistdiv ();
	        inittiming ();
	    }
	    *freevtptrs++ = t;
	    vicin.tor_cnt++;
	    t -> ei = freeteis++;
	    if (&N[t -> drain] == n)
		con = &N[t -> source];
	    else
		con = &N[t -> drain];
	    if (!con -> flag)
		vsearch (con);
	}
    }
}

void vclose ()
{
    NODE ** nn;
    NODE * n;
    TRANSISTOR ** tt;
    int cnt;

    /* reset flags */

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;
	n -> evalflag = FALSE;
	n -> ei = NULL;
    }

    tt = vicin.tors;
    for (cnt = vicin.tor_cnt; cnt > 0; cnt--) {
	(*tt++) -> ei = NULL;
    }
}

static void vsched ()
{	/* schedule reschedule or retrieve      */
	/* logic state (and spline) transitions */
	/* of the nodes of the vicinity         */
    int     cnt;
    NODE ** nn;
    NODE * n;
    simtime_t tmin, tmax, tevent, tswitch;
    double ratio, vminswitch, vmaxswitch;

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;

        if (n -> type == Forced) continue;

	if (n -> essential) {
	    if (n -> ei -> lstate != n -> stabstate) {

                if ( n -> stabstate == X_state
		&& (((double)n -> svmin > vmaxL && (double)n -> svmin < vminH)
                 || ((double)n -> svmax > vmaxL && (double)n -> svmax < vminH)) ) {
                    vminswitch = vminH;
                    vmaxswitch = vmaxL;
                }
                else {
                    vminswitch = vswitch;
                    vmaxswitch = vswitch;
                }

		if (n -> svmin == n -> ivmin)
		    tmin = tcurr;
		else {
		    ratio = (vminswitch - n -> ivmin) / (n -> svmin - n -> ivmin);
		    tmin = tcurr + (simtime_t)(n -> Ttmin * ratio);
		}

		if (n -> svmax == n -> ivmax)
		    tmax = tcurr;
		else {
		    ratio = (vmaxswitch - n -> ivmax) / (n -> svmax - n -> ivmax);
		    tmax = tcurr + (simtime_t)(n -> Ttmax * ratio);
		}

		switch (n -> ei -> lstate) {
		    case L_state :
			tswitch = tmax;
			if (n -> stabstate == H_state && tswitch < tmin)
			    n -> nextstate = X_state;
			else
			    n -> nextstate = n -> stabstate;
			break;
		    case H_state :
			tswitch = tmin;
			if (n -> stabstate == L_state && tswitch < tmax)
			    n -> nextstate = X_state;
			else
			    n -> nextstate = n -> stabstate;
			break;
		    case X_state :
			switch (n -> stabstate) {
			    case L_state :
			        tswitch = tmax;
				break;
			    case H_state :
			        tswitch = tmin;
				break;
			    default :
			        tswitch = 0;
				ERROR_EXIT (1);
			}
			n -> nextstate = n -> stabstate;
			break;
		    default :
			tswitch = 0;
			ERROR_EXIT (1);
		}

	        if (tswitch < tcurr) tswitch = tcurr;

	        if (n -> ei -> eventpending) {
	            resched_event (n, Normal, tswitch);
	        }
	        else {
		    sched_event (n, Normal, tswitch);
	        }
	    }
	    else {
		n -> nextstate = n -> stabstate;
	        if (n -> ei -> eventpending) {
		    retr_event (n, Normal);
	        }
	    }
	}

        if (n -> plot > 0) {
            if (n -> ivmin != n -> svmin || n -> ivmax != n -> svmax) {
                if (n -> ivmin == n -> svmin)
                    tevent = n -> tstabmax;
                else if (n -> ivmax == n -> svmax)
                    tevent = n -> tstabmin;
                else
                    tevent = MIN (n -> tstabmin, n -> tstabmax);
                if (! n -> plotevent) {
                    plot_node (n, 'l', (int)(n -> ivmin));
                    plot_node (n, 'u', (int)(n -> ivmax));
                    sched_event (n, Plot, tevent);
                    n -> plotevent = TRUE;
                }
                else {
                    plot_node (n, 'l', (int)(n -> ivmin));
                    plot_node (n, 'u', (int)(n -> ivmax));
                    resched_event (n, Plot, tevent);
                }
            }
            else {
                if (n -> plotevent) {
                    plot_node (n, 'l', (int)(n -> ivmin));
                    plot_node (n, 'u', (int)(n -> ivmax));
                    retr_event (n, Plot);
                    n -> plotevent = FALSE;
                }
            }
        }
    }
}

int lstate (NODE *n) /* find logic state by using the voltages of the node */
{
    UPAIR * up;

    /* this function will be called when n is only a a 'read function input' */

    up = uminmax (n);

    if (((double)n -> svmin > vmaxL && (double)n -> svmin < vminH)
     || ((double)n -> svmax > vmaxL && (double)n -> svmax < vminH)) {
        if (up -> umin >= vminH) return (H_state);
        if (up -> umax <= vmaxL) return (L_state);
    }
    else {
        if (up -> umin >= vswitch) return (H_state);
        if (up -> umax <= vswitch) return (L_state);
    }
    return (X_state);
}

UPAIR uvals;

UPAIR *uminmax (NODE *n) /* find current min and max voltage */
{
    if (tcurr < n -> tstabmin - n -> Ttmin) {
	uvals.umin = n -> ivmin;
    }
    else
	if (tcurr >= n -> tstabmin) {
	    uvals.umin = n -> svmin;
	}
	else {
	    uvals.umin = (double)n -> ivmin +
		((unsigned) n -> svmin - (unsigned) n -> ivmin)
		* (double) (tcurr - n -> tstabmin + n -> Ttmin) / n -> Ttmin;
	    /* double cast has been used because */
	    /* it shouldn't be an integer division */
	}

    if (uvals.umin > (vH * 1.1) || uvals.umin < (-vH * 0.1)) {
	ERROR_EXIT (1);
    }

    if (uvals.umin < 0) uvals.umin = 0;
    else if (uvals.umin > vH) uvals.umin = vH;

    if (tcurr < n -> tstabmax - n -> Ttmax) {
	uvals.umax = n -> ivmax;
    }
    else
	if (tcurr >= n -> tstabmax) {
	    uvals.umax = n -> svmax;
	}
	else {
	    uvals.umax = (double)n -> ivmax +
		((unsigned) n -> svmax - (unsigned) n -> ivmax)
		* (double) (tcurr - n -> tstabmax + n -> Ttmax) / n -> Ttmax;
	}

    if (uvals.umax > (vH * 1.1) || uvals.umax < (-vH * 0.1)) {
	ERROR_EXIT (1);
    }

    if (uvals.umax < 0) uvals.umax = 0;
    else if (uvals.umax > vH) uvals.umax = vH;

    if (n -> essential) {                /* correct possible inconsistancy */
                                         /* due to truncation */
	if (((double)n -> svmin > vmaxL && (double)n -> svmin < vminH)
         || ((double)n -> svmax > vmaxL && (double)n -> svmax < vminH)) {
            switch (n -> state) {
	        case H_state:
		    if (uvals.umin < vminH) uvals.umin = vminH;
		    if (uvals.umax < vminH) uvals.umax = vminH;
	            break;
	        case L_state:
		    if (uvals.umin > vmaxL) uvals.umin = vmaxL;
		    if (uvals.umax > vmaxL) uvals.umax = vmaxL;
	            break;
	        case X_state:
		    if (uvals.umin > vminH) uvals.umin = vminH;
		    if (uvals.umax < vmaxL) uvals.umax = vmaxL;
	            break;
	    }
        }
        else {
            switch (n -> state) {
	        case H_state:
		    if (uvals.umin < vswitch) uvals.umin = vswitch;
		    if (uvals.umax < vswitch) uvals.umax = vswitch;
	            break;
	        case L_state:
		    if (uvals.umin > vswitch) uvals.umin = vswitch;
		    if (uvals.umax > vswitch) uvals.umax = vswitch;
	            break;
	        case X_state:
		    if (uvals.umin > vswitch) uvals.umin = vswitch;
		    if (uvals.umax < vswitch) uvals.umax = vswitch;
	            break;
	    }
        }
    }

    return (&uvals);
}

/* returns stable logic state, derived from these stable voltages
*/
int stabstateof (double vmin, double vmax)
{
    if (vmin >= vminH) return (H_state);
    if (vmax <= vmaxL) return (L_state);
    return (X_state);
}
