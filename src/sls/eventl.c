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

static void del_event (NODE *n, int eventtype);
static void ins_event (NODE *n, int eventtype, simtime_t tswitch);
static simtime_t t_next_fevent (void);
static simtime_t t_next_nevent (void);
static simtime_t t_next_pevent (void);
static simtime_t t_read_fevent (void);
static simtime_t t_read_nevent (void);
static simtime_t t_read_pevent (void);

typedef struct eventl_el {
    NODE * node;
    simtime_t tswitch;
    struct eventl_el   *prev;
    struct eventl_el   *next;
} EVENTL_EL;

/* there are three separate event lists.     */
/* one for the Forced state transistions     */
/* one for the Normal state transistions and */
/* one for the Plot state transistions       */

EVENTL_EL * free_fevlarr;
EVENTL_EL * end_fevlarr;
EVENTL_EL * fevlbegin;
EVENTL_EL * fevlend;
EVENTL_EL * free_nevlarr;
EVENTL_EL * end_nevlarr;
EVENTL_EL * nevlbegin;
EVENTL_EL * nevlend;
EVENTL_EL * free_pevlarr;
EVENTL_EL * end_pevlarr;
EVENTL_EL * pevlbegin;
EVENTL_EL * pevlend;
EVENTL_EL * flastread;
EVENTL_EL * nlastread;
EVENTL_EL * plastread;

void initeventl ()
{
    PALLOC (free_nevlarr, essnodes_cnt + 2, EVENTL_EL);
    end_nevlarr = free_nevlarr + essnodes_cnt + 1;
    nevlbegin = free_nevlarr;
    nevlbegin -> tswitch = -1;
    nevlend = free_nevlarr + 1;
    nevlend -> node = NULL;
    nevlbegin -> next = nevlend;
    nevlend -> prev = nevlbegin;
    free_nevlarr = free_nevlarr + 2;

    /* During random initialization, an input event may be scheduled for
       the second time.  This does not give problems as long as we keep
       the event list large enough. */
    PALLOC (free_fevlarr, 2 * inpnodes_cnt + funcoutpnodes_cnt + 2, EVENTL_EL);
    end_fevlarr = free_fevlarr + 2 * inpnodes_cnt + funcoutpnodes_cnt + 1;
    fevlbegin = free_fevlarr;
    fevlbegin -> tswitch = -1;
    fevlend = free_fevlarr + 1;
    fevlend -> node = NULL;
    fevlbegin -> next = fevlend;
    fevlend -> prev = fevlbegin;
    free_fevlarr = free_fevlarr + 2;

    PALLOC (free_pevlarr, plotnodes_cnt + 2, EVENTL_EL);
    end_pevlarr = free_pevlarr + plotnodes_cnt + 1;
    pevlbegin = free_pevlarr;
    pevlbegin -> tswitch = -1;
    pevlend = free_pevlarr + 1;
    pevlend -> node = NULL;
    pevlbegin -> next = pevlend;
    pevlend -> prev = pevlbegin;
    free_pevlarr = free_pevlarr + 2;
}

void reset_read_event ()
{
    flastread = fevlbegin;
    nlastread = nevlbegin;
    plastread = pevlbegin;
}

simtime_t time_read_event ()
{
    simtime_t t;
    simtime_t t_nev = t_read_nevent ();
    simtime_t t_fev = t_read_fevent ();
    simtime_t t_pev = t_read_pevent ();

    if (t_nev < 0)
        t = t_fev;
    else if (t_fev < 0)
        t = t_nev;
    else if (t_nev < t_fev)
	t = t_nev;
    else
	t = t_fev;

    if (t_pev < 0)
        t = t;
    else if (t < 0)
        t = t_pev;
    else if (t < t_pev)
	t = t;
    else
	t = t_pev;

    return (t);
}

NODE * read_event ()
{
    simtime_t t;
    simtime_t t_nev = t_read_nevent ();
    simtime_t t_fev = t_read_fevent ();
    simtime_t t_pev = t_read_pevent ();
    NODE * n;

    if (t_nev < 0)
        t = t_fev;
    else if (t_fev < 0)
        t = t_nev;
    else if (t_nev < t_fev)
	t = t_nev;
    else
	t = t_fev;

    if (t_pev < 0)
        t = t;
    else if (t < 0)
        t = t_pev;
    else if (t < t_pev)
	t = t;
    else
	t = t_pev;

    if (t == t_nev) {
	nlastread = nlastread -> next;
        n = nlastread -> node;
    }
    else if (t == t_fev) {
	flastread = flastread -> next;
        n = flastread -> node;
    }
    else {
	plastread = plastread -> next;
        n = plastread -> node;
    }

    return (n);
}

static simtime_t t_read_nevent ()
{
    if (nlastread -> next == nevlend)
        return (-1);
    else
	return (nlastread -> next -> tswitch);
}

static simtime_t t_read_fevent ()
{
    if (flastread -> next == fevlend)
        return (-1);
    else
	return (flastread -> next -> tswitch);
}

static simtime_t t_read_pevent ()
{
    if (plastread -> next == pevlend)
        return (-1);
    else
	return (plastread -> next -> tswitch);
}

simtime_t time_next_event ()
{
    simtime_t t;
    simtime_t t_nev = t_next_nevent ();
    simtime_t t_fev = t_next_fevent ();
    simtime_t t_pev = t_next_pevent ();

    if (t_nev < 0)
        t = t_fev;
    else if (t_fev < 0)
        t = t_nev;
    else if (t_nev < t_fev)
	t = t_nev;
    else
	t = t_fev;

    if (t_pev < 0)
        t = t;
    else if (t < 0)
        t = t_pev;
    else if (t < t_pev)
	t = t;
    else
	t = t_pev;

    return (t);
}

NODE * next_event ()
{
    simtime_t t;
    simtime_t t_nev = t_next_nevent ();
    simtime_t t_fev = t_next_fevent ();
    simtime_t t_pev = t_next_pevent ();
    NODE * n;

    if (t_nev < 0)
        t = t_fev;
    else if (t_fev < 0)
        t = t_nev;
    else if (t_nev < t_fev)
	t = t_nev;
    else
	t = t_fev;

    if (t_pev < 0)
        t = t;
    else if (t < 0)
        t = t_pev;
    else if (t < t_pev)
	t = t;
    else
	t = t_pev;

    if (t == t_nev) {
        n = nevlbegin -> next -> node;
	if (debugsim) {
	    fprintf (debug, "NEXT: %s for t=%lld ", hiername (n - N), t);
	    fprintf (debug, " Normal\n");
	}
        del_event (n, Normal);
    }
    else if (t == t_fev) {
        n = fevlbegin -> next -> node;
	if (debugsim) {
	    fprintf (debug, "NEXT: %s for t=%lld ", hiername (n - N), t);
	    fprintf (debug, " Forced\n");
	}
        del_event (n, Forced);
    }
    else {
        n = pevlbegin -> next -> node;
	if (debugsim) {
	    fprintf (debug, "NEXT: %s for t=%lld ", hiername (n - N), t);
	    fprintf (debug, " Plot\n");
	}
        del_event (n, Plot);
    }

    if (debugsim) fprintf (debug, "\n");

    return (n);
}

static simtime_t t_next_nevent ()
{
    if (nevlbegin -> next == nevlend)
        return (-1);
    else
	return (nevlbegin -> next -> tswitch);
}

static simtime_t t_next_fevent ()
{
    if (fevlbegin -> next == fevlend)
        return (-1);
    else
	return (fevlbegin -> next -> tswitch);
}

static simtime_t t_next_pevent ()
{
    if (pevlbegin -> next == pevlend)
        return (-1);
    else
	return (pevlbegin -> next -> tswitch);
}

void sched_event (NODE *n, int eventtype, simtime_t tswitch)
{
    if (debugsim) {
	fprintf (debug, "SCHEDULE: %s for t=%lld ", hiername (n - N), tswitch);
	switch (eventtype) {
	    case Forced :
		fprintf (debug, "Forced ");
		if ( n->funcoutp )
		    fprintf (debug, "combination of forcedinfo's\n");
		else if ( n->inp )
		    fprintf (debug, "forcedinfo -> nextfstate : %d\n",
		    n -> forcedinfo -> nextfstate);
		break;
	    case Normal :
		fprintf (debug, "Normal ");
	        fprintf (debug, "nextstate : %d\n", n -> nextstate);
		break;
	    case Plot :
		fprintf (debug, "Plot\n");
		break;
	}
    }
    ins_event (n, eventtype, tswitch);
}

void resched_event (NODE *n, int eventtype, simtime_t tswitch)
{
    if (debugsim) {
	fprintf (debug, "RESCHEDULE: %s for t=%lld ", hiername (n - N), tswitch);
	switch (eventtype) {
	    case Forced :
		fprintf (debug, "Forced ");
		if ( n->funcoutp )
		    fprintf (debug, "combination of forcedinfo's\n");
		else if ( n->inp )
		    fprintf (debug, "forcedinfo -> nextfstate : %d\n",
		    n -> forcedinfo -> nextfstate);
		break;
	    case Normal :
		fprintf (debug, "Normal ");
	        fprintf (debug, "nextstate : %d\n", n -> nextstate);
		break;
	    case Plot :
		fprintf (debug, "Plot\n");
		break;
	}
    }
    del_event (n, eventtype);
    ins_event (n, eventtype, tswitch);
}

void retr_event (NODE *n, int eventtype)
{
    if (debugsim) {
	fprintf (debug, "RETRIEVE: %s", hiername (n - N) );
	switch (eventtype) {
	    case Forced :
		fprintf (debug, " Forced");
		break;
	    case Normal :
		fprintf (debug, " Normal");
		break;
	    case Plot :
		fprintf (debug, " Plot");
		break;
	}
	fprintf (debug, "\n");
    }
    del_event (n, eventtype);
}

static void ins_event (NODE *n, int eventtype, simtime_t tswitch)
{
    EVENTL_EL * search;
    EVENTL_EL * free_evlarr;

    switch (eventtype) {
	case Forced :
            if (free_fevlarr == end_fevlarr + 1) {
	        ERROR_EXIT (1);  /* event list overflow */
            }
            search = fevlend -> prev;
	    free_evlarr = free_fevlarr++;
	    break;
	case Normal :
            if (free_nevlarr == end_nevlarr + 1) {
	        ERROR_EXIT (1);  /* event list overflow */
            }
            search = nevlend -> prev;
	    free_evlarr = free_nevlarr++;
	    break;
	case Plot :
            if (free_pevlarr == end_pevlarr + 1) {
	        ERROR_EXIT (1);  /* event list overflow */
            }
            search = pevlend -> prev;
	    free_evlarr = free_pevlarr++;
	    break;
	default:
            search = free_evlarr = 0;
	    ERROR_EXIT(1);
    }

    while (search -> tswitch > tswitch)
	search = search -> prev;
    free_evlarr -> node = n;
    free_evlarr -> tswitch = tswitch;
    free_evlarr -> next = search -> next;
    free_evlarr -> prev = search;
    free_evlarr -> next -> prev = free_evlarr;
    free_evlarr -> prev -> next = free_evlarr;
}

static void del_event (NODE *n, int eventtype)
{
    EVENTL_EL * search;
    EVENTL_EL * free_evlarr;
    EVENTL_EL * evlend;

    switch (eventtype) {
	case Forced :
            search = fevlbegin -> next;
	    evlend = fevlend;
	    break;
	case Normal :
            search = nevlbegin -> next;
	    evlend = nevlend;
	    break;
	case Plot :
            search = pevlbegin -> next;
	    evlend = pevlend;
	    break;
	default:
            search = evlend = 0;
	    ERROR_EXIT(1);
    }

    while (search -> node != n && search != evlend)
	search = search -> next;

    if (search == evlend) return;

    search -> prev -> next = search -> next;
    search -> next -> prev = search -> prev;

    switch (eventtype) {
	case Forced :
	    free_evlarr = --free_fevlarr;
	    break;
	case Normal :
	    free_evlarr = --free_nevlarr;
	    break;
	case Plot :
	    free_evlarr = --free_pevlarr;
	    break;
	default:
	    free_evlarr = search;
	    ERROR_EXIT(1);
    }
    if (search != free_evlarr) {
	search -> node = free_evlarr -> node;
	search -> tswitch = free_evlarr -> tswitch;
	search -> prev = free_evlarr -> prev;
	search -> next = free_evlarr -> next;
	search -> prev -> next = search;
	search -> next -> prev = search;
    }
}
