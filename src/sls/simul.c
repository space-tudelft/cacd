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
#include <time.h>

extern void checkbreak (NODE *n, unsigned oldlstate, unsigned oldtype);
extern void dis_end (void);
extern int  eval_forcedinfos (NODE *n, simtime_t tmin, simtime_t *pmintswitch, short *pnextfres);
extern void evalfunctional (FUNCTION *f);
extern void evalvicinity (NODE *vn);
extern void forced_dis (NODE *n, int old, int new);
extern int  getval_forcedinfos (NODE *n, simtime_t t);
extern void interrupt (void);
extern void init_dis (void);
extern void initeval (void);
extern void initeventl (void);
extern void next_forced_event (NODE *n, int eventpres);
extern void nextsig_update (NODE *n);
extern NODE *next_event (void);
extern NODE *read_event (void);
extern void plot_begin (void);
extern void plot_end (void);
extern void reset_read_event (void);
extern simtime_t time_next_event (void);
extern simtime_t time_read_event (void);

static void currtimestep (void);
static void init_random (void);
static void rand_vsearch (NODE *n);
static void set_trans (TRANSISTOR *t, int s);
static void simstep (void);
static void update (NODE *n);
static int  visit_vicin (NODE *vn);

extern simtime_t tlastwritten; /* from intrup.c */

static int doing_init_random = FALSE;

NODE ** steplist;

void initsimul ()
{
    initeventl ();
    initeval ();
    PPALLOC (steplist, maxevents, NODE);
    ol_begin = NULL;
    simstep_cnt = 0;
    timepoint_cnt = 0;
}

void simulate ()
{
    int  cnt;
    char hc;
    NODE *n;
    FILE *fp_init;
    int   n_cnt_dp;
    char  fn_dp[20];
    char  str[20];
    int     vmin;
    int     vmax;
    int     state;
    simtime_t t_n_e;
    FORCEDSIGNAL *fs;

    if (monitoring) {
	monitime ("B simulate");
	monitime ("B state inits");
    }
    if (debugsim) {
	fprintf (debug, "\n================== start simulation ");
	fprintf (debug, "==================\n\n");
    }

    for (cnt = 0; cnt < T_cnt; cnt++) {
	set_trans (&T[cnt], X_state);
    }

    tcurr = -1;
    for (cnt = 0; cnt < N_cnt; cnt++) {
	n = &N[cnt];
	if (n -> redirect) continue;
	n -> svmin = 0;
	n -> svmax = vH;
	n -> state = X_state;
	n -> nextstate = X_state;
	n -> stabstate = X_state;
	n -> tstabmin = tcurr;
	n -> tstabmax = tcurr;
	n -> type = Normal;
	if (n -> inp ) {
	    n -> type = Forced;
	    n -> forcedinfo -> tswitch = tcurr;
	    n -> forcedinfo -> tswitch_stab = tcurr;
	    n -> forcedinfo -> initfstate = X_state;
	    n -> forcedinfo -> nextfstate = X_state;
	    n -> forcedinfo -> stabfstate = X_state;
	    nextsig_update (n);
	    if (n -> forcedinfo -> tswitch > tcurr)
		sched_event (n, Forced, n -> forcedinfo -> tswitch);
	}
	if ( n -> funcoutp) {
	    n -> type = Forced;
	    fs = n->forcedinfo;
	    if (n -> inp) fs = fs -> next;
	    while (fs) {
		fs -> tswitch = tcurr;
		fs -> tswitch_stab = tcurr;
		fs -> initfstate = X_state;
		fs -> nextfstate = X_state;
		fs -> stabfstate = X_state;
		fs = fs->next;
	    }
	}
    }

    if (fn_init != NULL) {
	if (verbose) fprintf (stderr, "Initialization from dumpfile %s\n", fn_init);

        if (debugsim) {
	    fprintf (debug, "\n............ initialization from dumpfile %s ..............\n\n",
	    fn_init);
	}

        OPENR (fp_init, fn_init);
        if (fscanf (fp_init, "%s", str) != 1 || strcmp (str, "dumpfile") != 0
            || fscanf (fp_init, "%s", str) != 1 || strcmp (str, "for") != 0)
	    slserror (fn_init, 0, ERROR1, "this file is not a dump file", NULL);
        if (fscanf (fp_init, "%s", fn_dp) != 1)
	    slserror (fn_init, 0, ERROR1, "error in this dump file", NULL);
	if (strcmp (fn_dp, netwname) != 0)
	    slserror (fn_init, 0, ERROR1, "this file is a dump file for network", fn_dp);
        if (fscanf (fp_init, "%d", &n_cnt_dp) != 1)
	    slserror (fn_init, 0, ERROR1, "error in this dump file", NULL);
	if (N_cnt != n_cnt_dp)
	    slserror (fn_init, 0, ERROR1, "number of nodes does not correspond with network", netwname);

        n = &N[0];
        for (cnt = 0; cnt < N_cnt; cnt++, n++) {
            if (fscanf (fp_init, "%d %d %d", &vmin, &vmax, &state) != 3) {
	        slserror (fn_init, 0, ERROR1, "error in this dump file", NULL);
            }
	    if (n -> redirect)
	        continue;
            if (n -> inp && n -> forcedinfo -> nextfstate != F_state)
                continue;
	    if (debugsim) {
		fprintf (debug, "initialize node %s to state %d\n", hiername (n - N), state);
	    }
	    n -> svmin = vmin;
	    n -> svmax = vmax;
	    n -> nextstate = state;
	    n -> stabstate = state;
	    n -> tstabmin = 0;
	    n -> tstabmax = 0;
	    if (n -> essential) {

                /* Also when the node is Forced, schedule it as a Normal event
		   since the the Forced event list is probably non-empty now,
		   while the Normal eventlist is still empty */

		n -> linked = TRUE;  /* to let update read the next state
				        from n -> nextstate */

		sched_event (n, Normal, 0);
            }
        }

        if (fscanf (fp_init, "%1s", &hc) != 1 || hc != '|')
	    slserror (fn_init, 0, ERROR1, "error in this dump file", NULL);

        for (cnt = 0; cnt < FS_cnt; cnt++) {
            if (FS[cnt] == '*') {
                cnt += sizeof (int);
                cnt += (*(int *)(FS + cnt) + 1) * SIZE_PTR_INT - 1;
            }
            else {
                if ((int)fread (FS + cnt, 1, 1, fp_init) <= 0)
	            slserror (fn_init, 0, ERROR1, "error in this dump file", NULL);
            }
        }

	CLOSE (fp_init);
    }

    if (monitoring) monitime ("E state inits");

    stepdelay = TRUE;
    tcurr = 0;
    events_cnt = 0;

    plot_begin ();

    if (dissip) init_dis ();

    currtimestep ();

    if (random_initialization) init_random ();

    stepdelay = !delaysim;

    stopsim = FALSE;
    while (!stopsim) {
	t_n_e = time_next_event ();
	if (tlastwritten < 0 && ((t_n_e < 0 && tsimduration > 0) || t_n_e > 0)) {
	    intrupm.on = TRUE;
	    interrupt ();       /* force printing of signals at t = 0 */
	}
	else if ((t_n_e > tbreak || t_n_e < 0) && tbreak <= tsimduration) {
	    tcurr = tbreak;
	    intrupm.on = TRUE;
	    intrupm.timebreak = TRUE;
	    interrupt ();
	}
        else if (t_n_e < 0) {
	    stopsim = TRUE;       /* event list is empty */
            if (simperiod >= 0) {
		if (tsimduration > MAXSIMTIME)
		    slserror (NULL, 0, WARNING, "time resolution becomes too large: simulation stopped", NULL);
		tcurr = tsimduration;
                if (tlastwritten < tsimduration) {
                    intrupm.on = TRUE;
                    interrupt ();        /* print values at t = simperiod */
                }
            }
        }
	else if (t_n_e > tsimduration) {
	    stopsim = TRUE;
            if (tsimduration > MAXSIMTIME)
                slserror (NULL, 0, WARNING, "time resolution becomes too large: simulation stopped", NULL);
	    tcurr = tsimduration;
	    if (tlastwritten < tsimduration) {
		intrupm.on = TRUE;
		interrupt ();        /* print values at t = simperiod */
	    }
        }
	else {
	    tcurr = t_n_e;
	    currtimestep ();
	}
    }

    if (dissip) dis_end ();

    plot_end ();

    if (monitoring) monitime ("E simulate");
}

/* Random initialization is done by temporarily setting each X node to
 * a Forced O or I and see if it remains O or I.
 * For a full random initialization (random_td_initialization == 1),
 * the initialization is different each simulation run.
 * In order to obtain a valid initialization in all cases, strongly
 * connected components (component == vicinity) are searched, and the
 * components are initialized depth-first (see the algorithm on page 429
 * of "Algorithms" by Robert Sedgewick).
 */
static NODE **PS;  /* stack for vicinities that are searched */
static int *VN;    /* level for each node */
static int *FOF;   /* gives the corresponding function for each FO[] element */

static int now;    /* current level */
static int psx;    /* index in PS[] */

static void init_random ()
{
    int cnt;
    int cnt2;
    int fox;
    long t;

    if (debugsim) {
	fprintf (debug, "\n================== start random initialization ");
	fprintf (debug, "==================\n\n");
    }

    doing_init_random = TRUE;

    if (random_td_initialization) {
	t = (long)time (0);
	srand48 (t);
    }

    PPALLOC (PS, N_cnt, NODE);
    PALLOC (VN, N_cnt, int);

    for (cnt = 0; cnt < N_cnt; cnt++) {
	VN[cnt] = 0;
    }

    PALLOC (FOF, FO_cnt, int);

    for (cnt = 0; cnt < F_cnt; cnt++) {
	if ((fox = F[cnt].fox) >= 0) {
	    cnt2 = FO[fox].x;
	    FOF[fox++] = cnt2;
	    while ( cnt2-- > 0 ) {
		FOF[fox++] = cnt;
	    }
	}
    }

    now = 0;
    psx = 0;

    for (cnt = 0; cnt < N_cnt; cnt++) {
	if (VN[cnt] == 0) visit_vicin (&N[cnt]);
    }

    if (intrupm.outputchange) {
	intrupm.on = TRUE;
	interrupt ();
    }

    doing_init_random = FALSE;

    CFREE (VN);
    CFREE (PS);
    CFREE (FOF);

    if (debugsim) {
	fprintf (debug, "\n================== end random initialization ");
	fprintf (debug, "==================\n\n");
    }
}

static int visit_vicin (NODE *vn)
{
    int cnt;
    int index;
    int fix;
    NODE *n;
    NODE *in;
    TRANSISTOR *tor;
    FORCEDSIGNAL *fs;
    int orig_type;
    int randomstate;
    int psx_s;
    int min;
    int m;
    long t;
    int x;

    now++;
    min = now;
    psx_s = psx;

    rand_vsearch (vn);

    for (x = psx_s; x < psx; x++) {
        PS[x] -> flag = FALSE;
	VN[ PS[x] - N ] = now;
    }

    for (x = psx_s; x < psx; x++) {

	n = PS[x];

        /* investigate all input nodes 'in' of 'n' */

	if (n -> type != Forced && n -> dsx >= 0) {
	    index = n -> dsx;
	    for (cnt = DS[index]; cnt > 0; cnt--) {
		index++;
		tor = &T[DS[index]];
		if (tor -> state == Undefined && tor -> gate >= 0) {
		    in = &N[tor -> gate];
		    m = VN[in - N];
		    if (!m) m = visit_vicin (in);
		    if (m < min) min = m;
		}
		if (tor -> state != Open) {
		    in = &N[tor -> drain];
		    if (in == n) in = &N[tor -> source];
		    if (in -> type == Forced && LSTATE (in) == X_state) {
			m = VN[in - N];
			if (!m) m = visit_vicin (in);
			if (m < min) min = m;
		    }
		}
	    }
	}

	fs = n -> forcedinfo;
	while (fs) {
	    if (fs -> fox >= 0) {
		fix = F[ FOF[ fs -> fox ] ].fix;
		if (fix >= 0) {
		    cnt = FI[fix++];
		    while (cnt-- > 0) {
			in = &N[FI[fix++]];
			if (LSTATE (in) == X_state) {
			    m = VN[in - N];
			    if (!m) m = visit_vicin (in);
			    if (m < min) min = m;
			}
		    }
		}
	    }
	    fs = fs -> next;
	}
    }

    if (min == VN[vn - N]) {

	do {
	    n = PS[--psx];

	    if (n -> essential && !(n -> inp && n -> type == Forced) && LSTATE (n) == X_state) {

		if (debugsim) {
		    fprintf (debug, "\n++++++++ random initialization node %s ", hiername (n - N));
		    fprintf (debug, "++++++++\n\n");
		}

		t = mrand48 ();
		if (t > 0)
		    randomstate = H_state;
		else
		    randomstate = L_state;

		orig_type = n -> type;
		if (n -> type == Normal)
		    n -> type = Forced;

		n -> nextstate = randomstate;
		n -> stabstate = randomstate;

		/* Although it is a Forced event, we schedule it as a
		   Normal event in order to not confuse with other Forced
		   events that may have already been scheduled for this
		   node.  The Normal eventlist should be empty right now.
		*/

		n -> linked = TRUE;  /* To mark this (special) event
						      (see update ()) */
		sched_event (n, Normal, 0);

		currtimestep ();
		timepoint_cnt--;

		/* Make logic state of node n consistent with its environment */

		if (orig_type == Normal) {
		    n -> nextstate = F_state;
		    n -> stabstate = F_state;
		}

		sched_event (n, Normal, 0);

		currtimestep ();
		timepoint_cnt--;
	    }
	    VN[n - N] = N_cnt + 1;

	}
	while (n != vn);
    }

    return (min);
}

static void rand_vsearch (NODE *n) /* search vicinity recursively */
{
    int cnt, index;
    TRANSISTOR * t;   /* transistor connected to n by drain or source */
    NODE * con;       /* node connected to n by t */

    n -> flag = TRUE;

    PS[psx++] = n;

    if (n -> type == Forced || n -> dsx < 0) return;

    index = n -> dsx;
    for (cnt = DS[index]; cnt > 0; cnt--) {
	index++;
	t = &T[DS[index]];
	if (t -> state != Open) {
	    if (&N[t -> drain] == n)
		con = &N[t -> source];
	    else
		con = &N[t -> drain];
	    if (!con -> flag && con -> type != Forced)
		rand_vsearch (con);
	}
    }
}

static void currtimestep ()
{
    int fraces, step_cnt;
    simtime_t tswitch;
    NODE *n;
    FORCEDSIGNAL *fs;
    short nextfres;

    timepoint_cnt++;

    step_cnt = 0;
    while (time_next_event () == tcurr && step_cnt <= logic_depth && !stopsim) {
	simstep ();
	step_cnt++;
    }

    if (stopsim) return;

    while (time_next_event () == tcurr && !stopsim) {
	reset_read_event ();
	while (time_read_event () == tcurr) {
	    n = read_event ();
            if ( n -> type == Normal ) {
	        if ( n -> stabstate != X_state ) {
	            n -> nextstate = n -> stabstate = X_state;
	            n -> svmin = 0;
	            n -> svmax = vH;
	            if (printraces) {
	                if (ol_begin == NULL) {
		            PALLOC (ol_begin, 1, HISTORY_LIST);
		            ol_end = ol_begin;
	                }
	                else {
		            PALLOC (ol_end -> next, 1, HISTORY_LIST);
		            ol_end = ol_end -> next;
	                }
	                ol_end -> t = tcurr;
	                ol_end -> ntx = (n - N);
	                ol_end -> next = NULL;
	            }
	        }
            }
            else { /* type == Forced */
		fraces = FALSE;
		eval_forcedinfos (n, tcurr - 1, &tswitch, &nextfres);
		if (nextfres != X_state) {
		    fs = n -> forcedinfo;
		    if (n -> inp)
			fs = fs -> next;
		    while (fs) {
			fs -> nextfstate = X_state;
			fs -> stabfstate = X_state;
			fs = fs -> next;
		    }
	            n -> svmin = 0;
	            n -> svmax = vH;
		    fraces = TRUE;
		}
		if ( fraces || printraces) {
		    if (ol_begin == NULL) {
			PALLOC (ol_begin, 1, HISTORY_LIST);
			ol_end = ol_begin;
		    }
		    else {
			PALLOC (ol_end -> next, 1, HISTORY_LIST);
			ol_end = ol_end -> next;
		    }
		    ol_end -> t = tcurr;
		    ol_end -> ntx = (n - N);
		    ol_end -> next = NULL;
                }
            }
	}
	simstep ();

	/* after the races have been detected and all nodes that are      */
	/* involved have been given the X-state, a user-defined function  */
	/* might place another event on the forced-event-list as a        */
	/* result of the X-values of the 'races'-nodes; this is done in   */
	/* the final simstep(); when this is done the simulation will     */
	/* never stop, so it is necessary to take all the events ON       */
	/* t=tcurr off the event-lists                                    */

        reset_read_event ();
	while (time_read_event () == tcurr) {
	    n = next_event ();
	    if (debugsim) {
		fprintf (debug, "event deleted from a certain event list\n");
	    }
	    reset_read_event ();
	}
    }

    if (intrupm.outputchange && !doing_init_random) {
	intrupm.on = TRUE;
	interrupt ();
    }
}

static void simstep ()
{
    NODE ** freesteplist;
    NODE ** nn;
    NODE * n;
    NODE * conn;
    int     event_cnt;
    int     cnt;
    int     concnt;
    int     torcnt;
    TRANSISTOR * t;
    INTERCAP * ic;
    int     index;
    unsigned  oldtorstate;

    simstep_cnt++;

    if (debugsim) {
	fprintf (debug, "\n======================");
	fprintf (debug, " simstep : tcurr = %lld ", tcurr);
	fprintf (debug, "=======================\n\n");
    }

    freesteplist = steplist;
    event_cnt = 0;
    while (time_next_event () == tcurr) {
	*freesteplist++ = next_event ();
	event_cnt++;
    }

    nn = steplist;

    for (cnt = 0; cnt < event_cnt; cnt++) {

	n = *nn++;

        if (n -> plot > 0
        && n -> plotevent
        && (n -> tstabmin == tcurr || n -> tstabmax == tcurr)) {
            if (n -> tstabmin == tcurr)
                plot_node (n, 'l', (int)(n -> svmin));
            if (n -> tstabmax == tcurr)
                plot_node (n, 'u', (int)(n -> svmax));
	    retr_event (n, Plot);
            n -> plotevent = FALSE;      /* event caused by spline changing */
            if (n -> tstabmin > tcurr || n -> tstabmax > tcurr) {
                if (n -> tstabmin > tcurr)                  /* another event */
                    sched_event (n, Plot, n -> tstabmin);
                else
                    sched_event (n, Plot, n -> tstabmax);
                n -> plotevent = TRUE;
            }
            continue;
        }

	update(n);

	if (n -> thisvicin) {
	    n -> evalflag = TRUE;
	    continue;
	}
	if (n -> cx != -1) {
	    index = n -> cx;
	    for (concnt = C[index].c; concnt > 0; concnt--) {
		index++;
		switch (C[index].sort)
		{
		    case Transistor :
			t = &T[ C[index].c ];
			oldtorstate = t -> state;
			set_trans (t, (int)(LSTATE (n)));
			if (t -> state != oldtorstate) {
			    if (N[t -> drain].type != Forced)
				N[t -> drain].evalflag = TRUE;
			    if (N[t -> source].type != Forced)
				N[t -> source].evalflag = TRUE;
			}
			break;
		    case Intercap :
			ic = &I[ C[index].c ];
			if ( &N[ ic -> con1 ] == n ) {
			    /* hier lading injectie doen            */
                            /* (nu is dat nog niet geimplementeerd) */
                            /*
			    N[ ic -> con2 ].evalflag = TRUE;
                            */
			}
			else {
			    /* hier lading injectie doen            */
                            /* (nu is dat nog niet geimplementeerd) */
                            /*
			    N[ ic -> con1 ].evalflag = TRUE;
                            */
			}
			break;
		    case Functional :
			F[ C[index].c ].evalflag = TRUE;
			break;
		}
	    }
	}
    }

    /* The following must be done after the previous since it uses
       transistor states that possibly have been updated.
    */

    nn = steplist;

    for (cnt = 0; cnt < event_cnt; cnt++) {

	n = *nn++;

	if (n -> type == Forced && n -> dsx != -1) {
	    index = n -> dsx;
	    for (torcnt = DS[index]; torcnt > 0; torcnt--) {
		index++;
		t = &T[DS[index]];
		if (t -> state != Open) {
		    if (&N[t -> drain] == n)
			conn = &N[t -> source];
		    else
			conn = &N[t -> drain];
		    if (conn -> type != Forced)
			conn -> evalflag = TRUE;
		}
	    }
	}
    }

    if (intrupm.on) interrupt ();

    if (stopsim) return;

    nn = steplist;

    for (cnt = 0; cnt < event_cnt; cnt++) {

	n = *nn++;

	if (n -> thisvicin) {
	    if (n -> evalflag)
	        evalvicinity (n);
	    n -> thisvicin = FALSE;
	    continue;
	}
	if (n -> cx != -1) {
	    index = n -> cx;
	    for (concnt = C[index].c; concnt > 0; concnt--) {
		index++;
		switch (C[index].sort)
		{
		    case Transistor :
			t = &T[ C[index].c ];
			if (N[t -> drain].evalflag)
			    evalvicinity (&N[t -> drain]);
			if (N[t -> source].evalflag)
			    evalvicinity (&N[t -> source]);
			break;
		    case Intercap :
			ic = &I[ C[index].c ];
			if ( N[ ic -> con1 ].evalflag )
			    evalvicinity (&N[ ic -> con1 ]);
			if ( N[ ic -> con2 ].evalflag )
			    evalvicinity (&N[ ic -> con2 ]);
			break;
		    case Functional :
			if ( F[ C[index].c ].evalflag )
			    evalfunctional ( &F[ C[index].c ] );
			break;
	    	}
	    }
	}
	if (n -> type == Forced && n -> dsx != -1) {
	    index = n -> dsx;
	    for (torcnt = DS[index]; torcnt > 0; torcnt--) {
		index++;
		t = &T[DS[index]];
		if (t -> state != Open) {
		    if (&N[t -> drain] == n)
			conn = &N[t -> source];
		    else
			conn = &N[t -> drain];
		    if (conn -> evalflag) evalvicinity (conn);
		}
	    }
	}
    }
}

static void update (NODE *n)
{
    unsigned oldlstate;
    unsigned oldtype;
    UPAIR * up = 0;
    simtime_t tswitch, mintswitch = -1;
    short nextfres = -1;

    events_cnt++;

    oldlstate = LSTATE(n);
    oldtype = n -> type;    /* In fact, if n -> linked == TRUE, n -> type
			       is not the real old type (n ->type is allways
			       Forced in that case), but that is OK. */

    if ((n->inp || n->funcoutp) && !n -> linked) {
	eval_forcedinfos( n, tcurr-1, &mintswitch, &nextfres );
	if (mintswitch != tcurr)
	    /* When a random initialization is done, update() is called
	       for a Forced node to update its value according to the
	       outputs of connected function blocks.
	       In that case, maybe mintswitch != tcurr and we need
	       getval_forcedinfos() to obtain in nextfres the current
	       Forced value.
	    */
	    nextfres = getval_forcedinfos (n, tcurr);
    }
    else if ( n -> type == Forced ) {
        /* this part is used during (random) initialization */
	mintswitch = 0;
	nextfres = n -> nextstate;
    }

    if (oldtype == Forced && nextfres == F_state) {

	/* Update from Forced to Normal */

	n -> state = n -> nextstate = n -> stabstate = oldlstate;
	n -> type = Normal;
	n -> thisvicin = TRUE;

        if (!n -> linked) {
	    if (n -> inp && n -> forcedinfo -> tswitch == tcurr)
		nextsig_update (n);
	    if (n -> inp || n->funcoutp)
		next_forced_event( n, FALSE );
	}
    }
    else if ((oldtype == Forced && nextfres != F_state)
	     || (oldtype == Normal && ( (n -> inp || n -> funcoutp)
					&& mintswitch == tcurr
					&& nextfres != F_state))) {

	/* Update to Forced */

	/* Note: it is possible that there is an event where
	   oldtype == Forced and nextfres == n -> state != F_state.
	   This may be the case for a node that is connected to a
	   network input and at least one output of a function block,
	   and the input previous state is Free or its next state.
	   Although the state and type do not change then, we need this
	   event in order to update the input signal for its next state.
	*/

	if (oldtype == Forced) {
	    if (dissip) forced_dis (n, n -> state, (unsigned)nextfres);

            n -> ivmin = n -> svmin;
            n -> ivmax = n -> svmax;
	}
	else {
	    if (oldlstate != n -> nextstate) {
		retr_event (n, Normal);
	    }

	    if (n -> plot > 0) up = uminmax (n);

	    n -> type = Forced;
	}

	n -> state = nextfres;
	n -> tstabmin = tcurr;
	n -> tstabmax = tcurr;
	switch (n -> state) {
	    case L_state:
		n -> svmin = 0;
		n -> svmax = 0;
		break;
	    case H_state:
		n -> svmin = vH;
		n -> svmax = vH;
		break;
	    case X_state:
		n -> svmin = 0;
		n -> svmax = vH;
		break;
	}

	if (n -> plot > 0) {
	    if (oldtype == Forced) {
		if (n -> ivmin != n -> svmin || n -> ivmax != n -> svmax) {
		    plot_node (n, 'l', (int)(n -> ivmin));
		    plot_node (n, 'u', (int)(n -> ivmax));
		    plot_node (n, 'l', (int)(n -> svmin));
		    plot_node (n, 'u', (int)(n -> svmax));
		}
	    }
	    else {
                plot_node (n, 'l', (int)(up -> umin));
                plot_node (n, 'u', (int)(up -> umax));
                plot_node (n, 'l', (int)(n -> svmin));
                plot_node (n, 'u', (int)(n -> svmax));
	    }
	}

        if (!n -> linked) {
	    if (n -> inp && n -> forcedinfo -> tswitch == tcurr)
		nextsig_update (n);
	    if (n -> inp || n->funcoutp)
		next_forced_event( n, FALSE );
	}
    }
    else {

	/* Update from Normal to Normal */

	n -> state = n -> nextstate;
	if (n -> nextstate != n -> stabstate) {
	    double ratio;

	    switch (n -> stabstate) {
		case L_state :
		    ratio = (vswitch - n -> ivmax) / ((double)n -> svmax - n -> ivmax);
		    tswitch = (n -> tstabmax - n -> Ttmax) + (simtime_t)(n -> Ttmax * ratio);
		    break;
		case H_state :
		    ratio = (vswitch - n -> ivmin) / ((double)n -> svmin - n -> ivmin);
		    tswitch = (n -> tstabmin - n -> Ttmin) + (simtime_t)(n -> Ttmin * ratio);
		    break;
		default :
		    tswitch = 0;
		    ERROR_EXIT (1);
		    break;
	    }

	    if (tswitch <= tcurr) {
		ERROR_EXIT (1);  /* error on second interval event */
	    }

	    sched_event (n, Normal, tswitch);

	    n -> nextstate = n -> stabstate;
	}
    }

    if (n -> outp)
        checkbreak (n, oldlstate, oldtype);

    if (debugsim) {
	fprintf (debug, "UPDATED: %s : on t=%lld from %d to %d\n",
		hiername (n - N), tcurr, oldlstate, LSTATE(n));
	if (oldtype != n -> type) {
	    fprintf (debug, "type did change to ");
	    if (n -> type == Forced)
		fprintf(debug, "Forced\n");
	    else
		fprintf(debug, "Normal\n");
	}
    }

    if (n -> linked)
	n -> linked = FALSE;
}

static void set_trans (TRANSISTOR *t, int s)
{
    switch (t -> type) {
	case Nenh:
	    switch (s) {
		case H_state:
		    t -> state = Closed;
		    break;
		case L_state:
		    t -> state = Open;
		    break;
		case X_state:
		    t -> state = Undefined;
		    break;
	    }
	    break;
	case Penh:
	    switch (s) {
		case H_state:
		    t -> state = Open;
		    break;
		case L_state:
		    t -> state = Closed;
		    break;
		case X_state:
		    t -> state = Undefined;
		    break;
	    }
	    break;
	case Depl:
	    t -> state = Closed;
	    break;
	case Res:
	    t -> state = Closed;
	    break;
    }
}
