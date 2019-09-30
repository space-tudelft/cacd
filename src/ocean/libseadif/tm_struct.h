/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
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
/*
 * Extensions to seadif structures which incorporate timing optimization.
 */
#ifndef __TM_STRUCT
#define __TM_STRUCT

#include <math.h>

/* Enumeration types declarations: */

typedef enum
{
   InputTTerm  = 0,		/* external terminal of the circuit */
   OutputTTerm = 1,		/* external terminal of the circuit */
   InternalRegTTerm = 2,	/* point inside circuit */
   InternalClkTTerm = 3,	/* point inside circuit that is clocked */
   BiDirPortTTerm   = 4
} tTermType;

/* This structure would be pointed by an additional field inside */
/* CIRCUIT structure. It represents timing view (model) for this circuit. */

typedef struct _TIMING
{
   STRING             name;	/* Name of the circuit timing model */

   STATUSPTR          status;

   struct _TMMODINST *tminstlist;/* List of timing models used for circuit */
				/* instances. */

   struct _NETMOD    *netmods;	/* models for these networks of the circuit */
				/* which have a conciderable delay. */


   struct _TIMETERM  *t_terms;	/* This is the list of external or internal */
				/* timing terminals. */

   struct _TPATH     *tPaths;	/* A list of internal delay propagation paths. */

   struct _TIMECOST  *timeCost; /* The function which describes the relation */
                                /* between cycle time and cost for clocked */
                                /* part of the circuit which is not connected */
				/* to any of timing terminals. */

   struct _DELASG    *delays;	/* this is non NULL only after delay assignment phase */
				/* and contains list of delay assignments to all */
				/* delay propagation path within the circuit */
				/* There can be more such assignments */

   struct _TIMING    *next;	/* there can be more than one */
                                /* model of timing for given circuit. */

   struct _CIRCUIT   *circuit;

   FLAG               flag;

} TIMING, TIMING_TYPE, *TIMINGPTR;

/* This represents the timing model selected for */
/* given circuit instance - also called - time model instance. */

typedef struct _TMMODINST
{
   STRING             name;     /* name for this time view instance for circuit */
				/* instance; */

   struct  _CIRINST  *cirinst;	/* which circuit instance  \(buses modeled by circuits */
				/* currently not supported ) */

   struct  _TIMING   *timing;	/* which timing model for circuit used */

   struct  _TIMING   *parent;	/* pointer to the parent model to which */
				/* this time instance belongs; */

   struct  _TMMODINST *next;
   FLAG               flag;

} TMMODINST, TMMODINST_TYPE, *TMMODINSTPTR;

/* This structure represents timing model */
/* for interconnections. */

typedef struct _NETMOD
{
   STRING             name;	/* name of the interconnection's model */

   struct  _NETREF   *netlist;	/* which net of our circuit */
   struct  _BUSREF   *buslist;	/* which bus */

   struct  _TIMECOST *cost;	/* cost function describing the relation */
				/* between delay on the network and its cost. */

   struct  _NETMOD   *next;
   FLAG               flag;

} NETMOD, NETMOD_TYPE, *NETMODPTR;

/* Used for referencing buses. */

typedef struct _BUSREF
{
  struct _BUS    *bus;	     /* the bus */

  struct _BUSREF *next;      /* next reference to a bus */
  FLAG            flag;      /* support for algorithms */

} BUSREF, BUSREF_TYPE, *BUSREFPTR;

/* The time terminal is a set of real circuit */
/* terminals which have the property of having */
/* the same time value. */

typedef struct _TIMETERM
{
   STRING             name;	/* name of the timing terminal */

   tTermType          type;	/* It can be external, internal regular, */
				/* or internal clocked. */

   struct _TIMETERMREF *termreflist; /* this is a list of references to t. terminals */
				    /* of timing models\' instances */

   struct _CIRPORTREF *cirportlist;
		                /* here is the list of circuit external terminals */
				/* that are equivalent to this timing */
				/* terminal. It will be something */
				/* only for external terminal type. */

   struct _TIMECOST   *timecost;  /* only for output time term. */
   struct _TIMING     *timing;

   double reqInputTime;	/* for clocked time terminal and output time terminals */
   double outputTime;	/* for clocked time terminals and input time terminals */

   double load;		/* input load for this terminal input time */
			/* terminal or output load for output time */
			/* terminal for current level circuit or */
   double drive;	/* input drive for current level input terminal */

   struct _TIMETERM  *next;

   FLAG               flag;

} TIMETERM, TIMETERM_TYPE, *TIMETERMPTR;

/* This structure is used in list of references to timing terminals of time instances
   or when used with a tpath - to any time terminal from current level. */

typedef struct _TIMETERMREF
{
   struct _TMMODINST	*inst; /* if this is NULL then it's for current circuit */
   struct _TIMETERM	*term;
   struct _TIMETERMREF	*next;
   FLAG               flag;

} TIMETERMREF, TIMETERMREF_TYPE, *TIMETERMREFPTR;

/* The time cost function which represents */
/* the relation between delay in a delay flow */
/* path and cost of such implementation. */
/* May also represent cost of implementing */
/* given circuit for different clock cycle times. */

typedef struct _TIMECOST
{
   long             p_num;
   struct _TCPOINT* points;
   FLAG             flag;

} TIMECOST, TIMECOST_TYPE, *TIMECOSTPTR;

/* The time cost point represents one possible */
/* relation between cost and delay. */
/* It may also contain a pointer to external structure */
/* which can be understood by macro cell generator. */

typedef struct _TCPOINT
{
   STRING           name;	/* it is to distinguish implementation */
				/* alternarives for a path. */
   long             delay;
   long             cost;
   STRING           wayOfImplementing;
   struct _TCPOINT *next;
   FLAG             flag;

} TCPOINT, TCPOINT_TYPE, *TCPOINTPTR;

/* The delay flow path represent a part inside */
/* our circuit which is treated as one path, */
/* e.i. all possible delays inside it have always the same delay. */

typedef struct _TPATH
{
   STRING             name;

   TIMETERMREFPTR     startTermList;
   TIMETERMREFPTR     endTermList;
   TIMINGPTR          parent;
   TIMECOSTPTR        timeCost;	/* delay versus cost function for this path */

   struct   _TPATH   *next;
   FLAG               flag;

} TPATH, TPATH_TYPE, *TPATHPTR;

/* This structure represents one possible assignment */
/* of delays to all delay propagation paths within a circuit. */

typedef struct _DELASG
{
   STRING           name;	/* name of the delays' assignment */

   STATUSPTR        status;

   long             clockCycle;	/* clock cycle chosen if \(internal clocking */
				/* part exists\) */

   struct  _DELASGINST *pathDelays;/* list of path's delays */
   struct  _TIMING  *timing;	/* pointer to the timing model for this */
				/* assignment of delays */

   struct  _DELASG  *next;	/* many assignments of delay for this circuit are */
                                /* possible */

   FLAG             flag;

} DELASG, DELASG_TYPE, *DELASGPTR;

/* The path's  delay contains information about */
/* selected delay for a path. */

typedef struct _DELASGINST
{
   STRING           name;
   struct _TPATH   *tPath;	/* pointer to the delay propagation path */
				/* it may be a current level path or */
				/* time model instance internal path */

   struct _TCPOINT *selected;  /* selected point from cost function */

   struct _DELASGINST *next;

   FLAG             flag;

} DELASGINST, DELASGINST_TYPE, *DELASGINSTPTR;


/* Memory allocation macros: */

#define NewTiming(p)	  ((p) = (TIMINGPTR)	 mnew (sizeof(TIMING_TYPE)))
#define NewTimeTerm(p)	  ((p) = (TIMETERMPTR)	 mnew (sizeof(TIMETERM_TYPE)))
#define NewTimeTermRef(p) ((p) = (TIMETERMREFPTR)mnew (sizeof(TIMETERMREF_TYPE)))
#define NewTmModInst(p)	  ((p) = (TMMODINSTPTR)	 mnew (sizeof(TMMODINST_TYPE)))
#define NewNetMod(p)	  ((p) = (NETMODPTR)	 mnew (sizeof(NETMOD_TYPE)))
#define NewBusRef(p)	  ((p) = (BUSREFPTR)	 mnew (sizeof(BUSREF_TYPE)))
#define NewTPath(p)	  ((p) = (TPATHPTR)	 mnew (sizeof(TPATH_TYPE)))
#define NewTimeCost(p)	  ((p) = (TIMECOSTPTR)	 mnew (sizeof(TIMECOST_TYPE)))
#define NewTcPoint(p)	  ((p) = (TCPOINTPTR)	 mnew (sizeof(TCPOINT_TYPE)))
#define NewDelAsg(p)	  ((p) = (DELASGPTR)	 mnew (sizeof(DELASG_TYPE)))
#define NewDelAsgInst(p)  ((p) = (DELASGINSTPTR) mnew (sizeof(DELASGINST_TYPE)))

/* Memory deallocation macros: */

#define FreeTiming(p) { if ((p)->name) fs ((p)->name); \
		mfree ((char **)(p), sizeof(TIMING_TYPE)); }
#define FreeTimeTerm(p) { if ((p)->name) fs ((p)->name); \
		mfree ((char **)(p), sizeof(TIMETERM_TYPE)); }
#define FreeCirPortRef(p)  { mfree ((char **)(p), sizeof(CIRPORTREF_TYPE)); }
#define FreeTimeTermRef(p) { mfree ((char **)(p), sizeof(TIMETERMREF_TYPE)); }
#define FreeTmModInst(p) { if ((p)->name) fs ((p)->name); \
		mfree ((char **)(p), sizeof(TMMODINST_TYPE)); }
#define FreeNetMod(p) { if ((p)->name) fs ((p)->name); \
		mfree ((char **)(p), sizeof(NETMOD_TYPE)); }
#define FreeBusRef(p) { mfree ((char **)(p), sizeof(BUSREF_TYPE)); }
#define FreeTPath(p) { if ((p)->name) fs ((p)->name); \
		mfree ((char **)(p), sizeof(TPATH_TYPE)); }
#define FreeTimeCost(p) { mfree ((char **)(p), sizeof(TIMECOST_TYPE)); }
#define FreeTcPoint(p) { if ((p)->name) fs ((p)->name); \
		if ((p)->wayOfImplementing) fs ((p)->wayOfImplementing); \
		mfree ((char **)(p), sizeof(TCPOINT_TYPE)); }
#define FreeDelAsg(p) { if ((p)->name) fs ((p)->name); \
		mfree ((char **)(p), sizeof(DELASG_TYPE)); }
#define FreeDelAsgInst(p) { if ((p)->name) fs((p)->name); \
		mfree ((char **)(p), sizeof(DELASGINST_TYPE)); }

#endif
