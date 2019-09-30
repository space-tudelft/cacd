/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 *	Paul Stravers
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

#ifndef __GENPART_H
#define __GENPART_H

#include "src/ocean/madonna/partitioner/cost.h"
#include "src/ocean/madonna/partitioner/routing.h"
#include "src/ocean/libseadif/sealib.h"

typedef int (*CFUNC)(int, ...);

typedef struct _PARTCELL
{
CIRINSTPTR        cinst;	  /* ptr to seadif circuit instance */
CIRINSTPTR        copycinst;	  /* ptr to seadif copy of circuit instance */
int    	          area;		  /* area, that is bbx[HOR] * bbx[VER] */
struct _PARTLIST  *partlist;	  /* ptr into one of the partition lists */
struct _PNET      *topnets;	  /* parent nets referring to this child */
struct _PARTITION *partition;	  /* partition it is currently a member of */
int	          flag;		  /* support for algorithms */
struct _PARTCELL  *next, *prev;	  /* doubly linked with other cells */
}
PARTCELL, *PARTCELLPTR;		  /* primary info-node of a cell */

typedef struct _PARTLIST
{
PARTCELLPTR       cell;	   /* ptr to the cell info-node  */
int               gain;	   /* gain if candidate cell moved to this partition */
int               ismember;  /* TRUE for member list, 0 for candidate list */
struct _PARTITION *partition;	/* pointer to the start of the "inpart" list */
struct _PARTLIST  *nextincell;  /* same cell, different partition lists */
struct _PARTLIST  *nextinpart, *previnpart; /* different cells, same partition list */
}
PARTLIST, *PARTLISTPTR;	    /* either a list of member cells of a partition
			     * or a list of candidate cells for a partition. */

typedef struct _PARTITION
{
int		  numid;      /* numerical id used as index in netdistr tables */
PARTLISTPTR       members;    /* The list of cells in this partition */
PARTLISTPTR       candidates; /* list of candidates to be moved to this partition */
int               width, height; /* size of the partition */
int               nmembers;   /* number of cells in partition */
int               cellarea;   /* total area of cells in partition */
int    	          targetarea; /* target for total cell area in this partition */
int    	          permitarea; /* default permissable overshoot on targetarea */
int    	      	  flag;	      /* support for algorithms */
struct _PARTITION *next, *prev; /* bidirectional list of partitions */
}
PARTITION, *PARTITIONPTR;     /* top level module, to be filled with cells */

typedef struct _PNET
{
NETPTR       net;	    /* ptr to net in topcell */
struct _PNET *next;	    /* next net */
int	     ignore;	    /* if TRUE, this net is ignored by the algorithm */
}
PNET, *PNETPTR;		    /* list of parent nets connecting to a child */

typedef struct _TOTALP
{
int          numparts;	   /* number of partitions */
int          nx, ny;	   /* nx * ny = numparts */
int          numcells;	   /* number of cells */
CIRCUITPTR   topcell;	   /* circuit to be partitioned */
GLOBAL_ROUTINGPTR routing; /* information about global routing */
CIRCUITPTR   bestpart;	   /* like topcell, but with extra level of hierarchy. */
int 	     bestnetcost;  /* lowest net cost found; corresponds to bestpart */
int	     bestmove;	   /* the nmove that caused the best partitioing */
int 	     strtnetcost;  /* start net cost (= highest netcost) */
CFUNC        *costfunction;
                           /* the cost functions that guides the partitioning */
int          *tmpstatebuf; /* tmp buffer for netstate->{dist,clm,row} */
PARTITIONPTR partition;	   /* list of partitions, length must be numparts */
PARTCELLPTR  partcell;	   /* list of cells that participate in partitioning */
int	     area;	   /* sum of all areas of children in topcell */
int    	     netcost;	   /* sum of costs of all nets */
int	     nmoves;	   /* number of moves needed to produce bestpart */
int	     repeat;	   /* counts number of dopartitioning() calls. */
int	     stopcriterion;/* stop iterating if strtnetcost-bestnetcost < this */
double       temperature;  /* range [0..1] is probability of random move */
double       cooling;	   /* after each random move temperature *= cooling */
}
TOTALP, *TOTALPPTR;	   /* structure to hold the entire partitioning stuff */

typedef struct _NETCOSTINFO
{
int ncells;		   /* minimum #cells needed for this #numparts */
int numparts, row, clm;	   /* numparts=row*clm, size and  topology */
CFUNC costfunction[2];
                           /* array of ptrs to functions that compute net cost */
}
NETCOSTINFO, *NETCOSTINFOPTR;

#define MAXNETCOSTFUNCTIONS 30

#define NewPartcell(p)	((p) = (PARTCELLPTR) mnew (sizeof(PARTCELL)))
#define NewPartlist(p)	((p) = (PARTLISTPTR) mnew (sizeof(PARTLIST)))
#define NewPartition(p) ((p) = (PARTITIONPTR)mnew (sizeof(PARTITION)))
#define NewPnet(p)	((p) = (PNETPTR)     mnew (sizeof(PNET)))
#define NewTotalp(p)	((p) = (TOTALPPTR)   mnew (sizeof(TOTALP)))

#define FreePartcell(p)	 mfree((char **)(p), sizeof(PARTCELL))
#define FreePartlist(p)  mfree((char **)(p), sizeof(PARTLIST))
#define FreePartition(p) mfree((char **)(p), sizeof(PARTITION))
#define FreePnet(p)	 mfree((char **)(p), sizeof(PNET))
#define FreeTotalp(p)	 mfree((char **)(p), sizeof(TOTALP))

#endif
