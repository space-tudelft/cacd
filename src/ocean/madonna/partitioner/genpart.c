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
#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/madonna/partitioner/genpart.h"
#include "src/ocean/madonna/partitioner/cost.h"
#include "src/ocean/madonna/partitioner/part.h"
#include <stdlib.h>
#include <unistd.h>

/***************************************************/
extern int test_movsrcfreq[3];
extern int test_movdstfreq[3];
extern int test_xx[3];
/***************************************************/

PRIVATE int initpartitionstructures (TOTALPPTR total);
PRIVATE int makepartcelllist (TOTALPPTR total);
PRIVATE int randommakemembersandcandidates (TOTALPPTR total);
PRIVATE void clearflagsofpartcells (TOTALPPTR total);
PRIVATE void createpartitionlist (TOTALPPTR total);
PRIVATE void createrandommemberlist (PARTITIONPTR partition, int staticarea, PARTCELLPTR *globalpcell);
PRIVATE void createcandidatelist (PARTITIONPTR partition, TOTALPPTR total);
PRIVATE void createprefabmemberlist (PARTITIONPTR partition,
              CIRCUITPTR prefab, PARTCELLPTR partcell, int staticarea);
PRIVATE int computebbx (LAYOUTPTR layout);
PRIVATE int makenetdistributiontables (TOTALPPTR total);
PRIVATE int computegains (TOTALPPTR total);

extern int stopquotient;
extern int processid; /* for reproducing programming errors after a core dump */
extern double initial_temperature, initial_cooling;

extern int permutateClustersAtExit, maxNumberOfClusterPermutations;

/* Partition into numparts divisions the list of children of topcell.
 * The caller must supply a pointer to an array of cost functions.
 * These functions should be defined as follows:
 *
 *  costfunction[COSTSTATE]:     int thefirstcostfunction (id, netstate)
 *                               int id;   (* not used ... *)
 *                               NETSTATEPTR netstate;
 *  costfunction[COSTVEC]:       int thesecondcostfunction (id, xr, yb, yt, deviation)
 *                               int id;   (* not used ... *)
 *                               int xl, xr, yb, yt, deviation;
 */
int genpart (TOTALPPTR *totalout, CIRCUITPTR topcell, int nx, int ny,
		   CFUNC *costfunction, int repeat)
{
    extern int alarm_flag, alarm_flag_reset;
    TOTALPPTR total;
    int numparts = nx*ny, nclusterPerms = maxNumberOfClusterPermutations;

    NewTotalp (total);
    *totalout = total;
    total->topcell = topcell;
    total->routing = NULL;
    total->numparts = numparts;
    total->nx = nx;
    total->ny = ny;
    total->costfunction = costfunction;
    total->partition = NULL; total->partcell = NULL;
    total->bestpart = NULL;
    total->repeat = (repeat > 1 ? repeat : 1);
    total->stopcriterion = 0;
    total->temperature = initial_temperature;
    total->cooling = initial_cooling;

    while (total->repeat > 0)
    {
	if (nclusterPerms-- > 0) clusterPermutate (total);
	if (!initpartitionstructures (total)) return (0);
	if (!dopartitioning (total)) return (0);

	printf ("------ cost@start = %d, cost@end = %d, moves = %d\n",
	    total->strtnetcost, total->bestnetcost, total->nmoves);

	if (!exitpartitionstructures (total)) return (0);
	total->repeat -= 1;
	if (alarm_flag) {
	    if (alarm_flag_reset) alarm_flag = 0;
	    break; /* async. request for proper abortion */
	}
	if (total->strtnetcost - total->bestnetcost <= (total->strtnetcost / stopquotient))
	    break; /* last call to dopartitioning() did not bring enough improvement */
	if (total->nmoves == 0)
	    break; /* last call to dopartitioning() did not improve anything */
    }
    if (permutateClustersAtExit) clusterPermutate (total);
    return (TRUE);
}

/* Build and initialize the datastructures for the partitioner.
 */
PRIVATE int initpartitionstructures (TOTALPPTR total)
{
    /* make list of children of topcell */
    if (!makepartcelllist (total)) return (0);

    /* make partitions and assign each of them a
     * number of member-children and candidate-children
     */
    if (!total->bestpart) { /* make random initial distribution */
	if (!randommakemembersandcandidates (total)) return (0);
    }
    else { /* make intial distribution equal to partitioning in total->bestpart */
	if (!prefabmakemembersandcandidates (total)) return (0);
    }

    /* create and initialize tables containing net distribution and net costs */
    if (!makenetdistributiontables (total)) return (0);

    /* foreach cell in the candidate lists compute the gain if it
     * would be moved to the partition where it is candidate for
     */
    if (!computegains (total)) return (0);
    return (TRUE);
}

/* Read all the children's layout information from the data base.
 */
int readlayoutofchildren (int what, CIRCUITPTR topcell, int expandtobottom)
{
    CIRINSTPTR cinst;
    int tudobem = TRUE;

    /* we need the bounding boxes of all the layouts of the cirinstances */
    for (cinst = topcell->cirinst; cinst; cinst = cinst->next)
	tudobem &= plazareadlayout (what, cinst->circuit, expandtobottom);
    return (tudobem);
}

int plazareadlayout (int what, CIRCUITPTR c, int expandtobottom)
{
    LAYOUTPTR l;
    int tudobem = TRUE;

    if (expandtobottom) {
	if (!sdflistalllay (what, c)) return (0);
    }
    else {
	if (!sdflistlay (what, c)) return (0);
    }

    if (!(l = c->layout)) {
	fprintf (stderr, "no layout for circuit (%s(%s(%s))) specified\n",
	    c->name, c->function->name, c->function->library->name);
	tudobem = 0;
    }
    else if (l->bbx[HOR] <= 0 || l->bbx[VER] <= 0)
	if (!computebbx (l)) {
	    fprintf (stderr, "cannot compute bounding box for layout (%s(%s(%s(%s))))\n",
		l->name, c->name, c->function->name, c->function->library->name);
	    tudobem = 0;
	}
    return (tudobem);
}

/* Make list of children of topcell. CIRINST.flag points to pcell.
 * Pcell->topnets is constructed in makenetdistributiontables().
 */
PRIVATE int makepartcelllist (TOTALPPTR total)
{
    CIRINSTPTR  cinst = NULL, *cells;
    PARTCELLPTR pcell = NULL, opcell = NULL;
    LAYOUTPTR   lay;
    int     numcells, j;
    extern int set_srand;

    /* count number of cells */
    for (numcells = 0, cinst = total->topcell->cirinst; cinst; cinst = cinst->next) ++numcells;

    total->area = 0;
    total->numcells = numcells;

    if (!(cells = (CIRINSTPTR*)malloc ((numcells+1) * sizeof(CIRINSTPTR))))
    {
	/* this is not fatal, it just means we cannot randomize the initial distribution */
	for (cinst = total->topcell->cirinst; cinst; cinst = cinst->next)
	{
	    NewPartcell (pcell);
	    cinst->flag.p = (void *)pcell;
	    pcell->cinst = cinst;
	    pcell->partlist = NULL; pcell->topnets = NULL; pcell->flag = 0;
	    lay = cinst->circuit->layout;
	    total->area += (pcell->area = lay->bbx[HOR] * lay->bbx[VER]);
	    pcell->next = opcell;
	    if (opcell) opcell->prev = pcell;
	    opcell = pcell;
	}
	total->partcell = pcell;
	return (TRUE);
    }

    /* make an array of pointers to the circuit instances */
    for (j = 0, cinst = total->topcell->cirinst; cinst; cinst = cinst->next) cells[j++] = cinst;

    processid = (int)getpid();

    if (set_srand > 0) srand (set_srand);
    else srand (processid); /* ititialize random seed with process id */

    /* now perform a number of random exchangements. numcells times seems like enough */
    for (j = 0; j < numcells; ++j) {
	int cella = rand() % numcells;
	int cellb = rand() % numcells;
	CIRINSTPTR zzap = cells[cella];
	cells[cella] = cells[cellb];
	cells[cellb] = zzap;
    }

    for (j = 0; j < numcells; ++j) {
	NewPartcell (pcell);
	cinst = cells[j];
	cinst->flag.p = (void *)pcell;
	pcell->cinst = cinst;
	pcell->partlist = NULL; pcell->topnets = NULL; pcell->flag = 0;
	lay = cinst->circuit->layout;
	total->area += (pcell->area = lay->bbx[HOR] * lay->bbx[VER]);
	pcell->next = opcell;
	if (opcell) opcell->prev = pcell;
	opcell = pcell;
    }
    total->partcell = pcell;
    free (cells);
    return (TRUE);
}

#define AREAOVERSHOOT 30 /* allowable percentage overshoot, see dopartitioning() */

/* Make partitions and assign each of them a
 * number of member-children and candidate-children.
 */
PRIVATE int randommakemembersandcandidates (TOTALPPTR total)
{
    int areaforonepartition;
    PARTITIONPTR partition;
    PARTCELLPTR  pcell = total->partcell;

    /* First create a list of empty partitions */
    createpartitionlist (total);
    clearflagsofpartcells (total);
    areaforonepartition = total->area / total->numparts;

    for (partition = total->partition; partition; partition = partition->next)
    {
	/* add members to the partition until area exceeds maximum */
	createrandommemberlist (partition, areaforonepartition, &pcell);
	/* add remaining cells as candidates */
	createcandidatelist (partition, total);
    }
    return (TRUE);
}

PRIVATE void clearflagsofpartcells (TOTALPPTR total)
{
    PARTCELLPTR tmpcell;
    for (tmpcell = total->partcell; tmpcell; tmpcell = tmpcell->next) tmpcell->flag = 0;
}

PRIVATE void createpartitionlist (TOTALPPTR total)
{
    PARTITIONPTR partition, opartition = NULL;
    int numid, numparts;

    for (numparts = total->numparts, numid = 1; numid <= numparts; ++numid)
    {
	NewPartition (partition);
	if (numid == 1) total->partition = partition;
	partition->numid = numid;
	partition->members = partition->candidates = NULL;
	partition->prev = opartition;
	if (opartition) opartition->next = partition;
	opartition = partition;
    }
}

/* Add members to the partition until area exceeds maximum.
 */
PRIVATE void createrandommemberlist (PARTITIONPTR partition, int staticarea, PARTCELLPTR *globalpcell)
{
    int thisarea = 0, nmembers = 0;
    PARTLISTPTR plist = NULL, oplist = NULL;
    PARTCELLPTR pcell = (*globalpcell);

    while (pcell) {
	NewPartlist (plist);
	pcell->flag = TRUE;      /* indicate that this is a member */
	nmembers += 1;
	plist->cell = pcell;
	plist->ismember = TRUE;
	plist->partition = partition;
	plist->nextinpart = oplist;
	if (oplist) oplist->previnpart = plist;
	oplist = plist;
	plist->nextincell = pcell->partlist;
	pcell->partlist = plist;
	pcell->partition = partition;
	thisarea += pcell->area;
	pcell = pcell->next;
	if (thisarea >= staticarea) break;
    }
    partition->members  = plist;
    partition->nmembers = nmembers;
    partition->cellarea = thisarea;
    partition->targetarea = staticarea;
    partition->permitarea = staticarea + (AREAOVERSHOOT * staticarea) / 100;
    (*globalpcell) = pcell;
}

/* Add cells not yet marked as member to the candidate list.
 */
PRIVATE void createcandidatelist (PARTITIONPTR partition, TOTALPPTR total)
{
    PARTLISTPTR plist = NULL, oplist = NULL;
    PARTCELLPTR tmpcell;

    for (tmpcell = total->partcell; tmpcell; tmpcell = tmpcell->next) {
	if (tmpcell->flag) {
	    /* this flag need to be reused in next iteration of makemembersandcandidates() */
	    tmpcell->flag = 0;
	    continue; /* already used as a member in this partition */
	}
	NewPartlist (plist);
	plist->cell = tmpcell;
	plist->ismember = 0;
	plist->partition = partition;
	plist->nextinpart = oplist;
	if (oplist) oplist->previnpart = plist;
	oplist = plist;
	plist->nextincell = tmpcell->partlist;
	tmpcell->partlist = plist;
    }
    partition->candidates = plist;
}

/* Make intial distribution equal to partitioning in total->bestpart.
 */
int prefabmakemembersandcandidates (TOTALPPTR total)
{
    int        areaforonepartition;
    PARTITIONPTR partition;
    CIRINSTPTR   ci;

    /* First create a list of empty partitions */
    createpartitionlist (total);
    clearflagsofpartcells (total);
    areaforonepartition = total->area / total->numparts;

    for (partition = total->partition; partition; partition = partition->next) {
	for (ci = total->bestpart->cirinst; ci; ci = ci->next)
	    if (partition->numid == atoi (ci->name)) break;
	if (!ci) err (7, "prefabmakemembersandcandidates: internal error #858");

	/* add members to this partition */
	createprefabmemberlist (partition, ci->circuit, total->partcell, areaforonepartition);
	/* add remaining cells as candidates */
	createcandidatelist (partition, total);
    }
    return (TRUE);
}

PRIVATE void createprefabmemberlist (PARTITIONPTR partition,
              CIRCUITPTR prefab, PARTCELLPTR partcell, int staticarea)
{
    CIRINSTPTR  ci;
    PARTCELLPTR pcell;
    PARTLISTPTR plist = NULL, oplist = NULL;
    int nmembers = 0, thisarea = 0;

    for (ci = prefab->cirinst; ci; ci = ci->next) {
	for (pcell = partcell; pcell; pcell = pcell->next)
	    if (pcell->cinst->name == ci->name) break;
	if (!pcell) err (7, "createprefabmemberlist: internal error #54");

	pcell->flag = TRUE; /* indicate that this is a member */
	NewPartlist (plist);
	plist->cell = pcell;
	plist->ismember = TRUE;
	plist->partition = partition;
	plist->nextinpart = oplist;
	if (oplist) oplist->previnpart = plist;
	oplist = plist;
	plist->nextincell = pcell->partlist;
	pcell->partlist = plist;
	pcell->partition = partition;
	thisarea += pcell->area;
	nmembers += 1;
    }
    partition->members  = plist;
    partition->nmembers = nmembers;
    partition->cellarea = thisarea;
    partition->targetarea = staticarea;
    partition->permitarea = staticarea + (AREAOVERSHOOT * staticarea) / 100;
}

/* Compute the bounding box of a layout. Return 0 if failed, TRUE otherwise.
 */
PRIVATE int computebbx (LAYOUTPTR dummyARG)
{
    return (0);
}

/* Create and initialize tables containing net distribution and net costs.
 */
PRIVATE int makenetdistributiontables (TOTALPPTR total)
{
    int           nnets = 0, *netstatepool, netid, j, thispartition, ignorenet;
    int          *netdentry, bufsize;
    NETSTATEPTR   netstate;
    NETPTR        net;
    PARTCELLPTR   pcell;
    CIRPORTREFPTR cpref;
    PNETPTR       pnet;

    /* count number of nets */
    for (net = total->topcell->netlist; net; net = net->next) nnets += 1;

    /* allocate space for all the net states: */
    bufsize = sizeofstatebuf2 (total);

    /* this multiplying by sizeof(int) is not correct if sizeof(int) != sizeof(int*): */
    if (!(netstatepool = (int*)malloc ((nnets+1) * bufsize * sizeof(int)))) {
	fprintf (stderr, "cannot allocate memory for net distribution and cost tables\n");
	return (0);
    }

    total->tmpstatebuf = netstatepool; /* used by domove_and_copy_netstate() */
    init_row_clm_transformation_arrays (total);

    /* for all cells clear the flag: */
    for (pcell = total->partcell; pcell; pcell = pcell->next) pcell->flag = 0;

    total->netcost = 0;

    for (netid = 1, net = total->topcell->netlist; net; ++netid, net = net->next)
    {
	NewNetstate (netstate);
	netstate->nx = total->nx;
	netstate->ny = total->ny;
	netstate->numparts = total->numparts;
	netstate->netid = netid;
	assignstatebuf (netstate, netstatepool+netid*bufsize);

	/* initialize row list and column list to zero */
	for (j = 0; j <= netstate->nx; ++j) {
	    netstate->clm[LIST][j] = 0;
	    netstate->clm[PREV][j] = 0;
	    netstate->clm[NEXT][j] = 0;
	}
	for (j = 0; j <= netstate->ny; ++j) {
	    netstate->row[LIST][j] = 0;
	    netstate->row[PREV][j] = 0;
	    netstate->row[NEXT][j] = 0;
	}

	netdentry = netstate->dist;

	/* netstate->dist==netdentry[1..j..numparts] contain number of
	* children connected to net that are members of partition j.
	*/
	for (j = 0; j <= total->numparts; ++j) netdentry[j] = 0;

	/* Heuristic to decrease number of updates needed. Long nets don't
	 * help us finding the clusters, but they do eat a lot of CPU:
	 */
	ignorenet = (net->num_term > total->numparts + 6);

	for (cpref = net->terminals; cpref; cpref = cpref->next)
	{
	    if (!cpref->cirinst) continue; /* terminal of topcell itself, skip */

	    pcell = (PARTCELLPTR)(cpref->cirinst->flag.p);
	    if (pcell->flag) /* need this flag because one net may connect... */
		continue;    /* to several terminals of the same child */
	    pcell->flag = 1; /* mark as seen */

	    if ((thispartition = pcell->partition->numid) < 1 || thispartition > total->numparts) {
		fprintf (stderr, "makenetdistributiontables: internal error 23\n");
		dumpcore();
	    }
	    netdentry[thispartition] += 1;
	    /* build the list of back references from the children to the parent nets */
	    NewPnet (pnet);
	    pnet->net = net;
	    pnet->ignore = ignorenet;
	    pnet->next = pcell->topnets;
	    pcell->topnets = pnet;
	}

	if (!ignorenet) { /* compute initial cost of this netdistribution and build state */
	    initnetstate (netstate, total->costfunction);
	    total->netcost += netstate->cost;
	}
	else
	    netstate->cost = 0;

	net->flag.p = (void *)netstate;
	for (cpref = net->terminals; cpref; cpref = cpref->next)
	    if (cpref->cirinst) ((PARTCELLPTR)(cpref->cirinst->flag.p))->flag = 0; /* reset mark */
    }
    return (TRUE);
}

/* For all candidates compute the decrease of the netcost ("gain") if
 * the candidate where to be moved from the srcparti (=partition where
 * it is currently a member of) to the dstparti (=partition where it
 * is candidate for).
 */
PRIVATE int computegains (TOTALPPTR total)
{
    CFUNC       *costfunction = total->costfunction;
    int          dstparti, srcparti;
    NETSTATEPTR  netstate;
    PARTITIONPTR partition;
    PARTLISTPTR  cand;
    PNETPTR      pnet;

    for (partition = total->partition; partition; partition = partition->next)
    {
	dstparti = partition->numid;   /* destination partition */
	for (cand = partition->candidates; cand; cand = cand->nextinpart)
	{
	    cand->gain = 0;
	    srcparti = cand->cell->partition->numid; /* source partition */
	    for (pnet = cand->cell->topnets; pnet; pnet = pnet->next)
	    {
		if (pnet->ignore) continue; /* heuristic to decrease number of updates needed */
		netstate = (NETSTATEPTR)pnet->net->flag.p;
		if (netstate->dist[srcparti] == 1 || netstate->dist[dstparti] == 0)
		    /* distribution of this net is CRITICAL */
		    cand->gain += gainnetstate (netstate, dstparti, srcparti, costfunction);
		else
		    ; /* in this branch gainnetstate() always returns 0, no use to call it */
	    }
	}
    }
    return (TRUE);
}

/* Remove the data structures used by the partitioner.
 */
int exitpartitionstructures (TOTALPPTR total)
{
    PARTITIONPTR part, nextpart;
    PARTLISTPTR  plist, nextplist;
    PARTCELLPTR  pcell, nextpcell;
    PNETPTR      pnet, nextpnet;

    for (part = total->partition; part; part = nextpart)
    {
	for (plist = part->members; plist; plist = nextplist) {
	    nextplist = plist->nextinpart;
	    FreePartlist (plist);
	}
	for (plist = part->candidates; plist; plist = nextplist) {
	    nextplist = plist->nextinpart;
	    FreePartlist (plist);
	}
	nextpart = part->next;
	FreePartition (part);
    }
    total->partition = NULL;

    for (pcell = total->partcell; pcell; pcell = nextpcell)
    {
	for (pnet = pcell->topnets; pnet; pnet = nextpnet) {
	    nextpnet = pnet->next;
	    FreePnet (pnet);
	}
	nextpcell = pcell->next;
	FreePartcell (pcell);
    }
    total->partcell = NULL;
    return (TRUE);
}
