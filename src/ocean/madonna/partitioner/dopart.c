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
#include <string.h>
#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/madonna/partitioner/genpart.h"
#include "src/ocean/madonna/partitioner/cost.h"
#include "src/ocean/madonna/partitioner/part.h"

#define MINGAIN -2147483647 /* smaller than any gain could ever be */

PRIVATE int  we_are_going_to_slow (TOTALPPTR total);
PRIVATE void printstatisticsinstatusfield (TOTALPPTR total);

int test_movsrcfreq[3];
int test_movdstfreq[3];
int test_xx[3];

int globalnamecounter = 0; /* used to make "unique" names */

int dopartitioning (TOTALPPTR total)
{
    long         maxareaneed, tmp, maxgain;
    PARTITIONPTR part, thispart = NULL;
    PARTLISTPTR  cand, thiscand = NULL;
    extern int   alarm_flag, acceptCandidateEvenIfNegativeGain;
    extern FILE *sdfcopystream;
    int          too_slow;

    rememberthispartitioning (total);
    total->strtnetcost = total->bestnetcost = total->netcost;
    total->nmoves = total->bestmove = 0;

    for (;;)
    {
	for (part = total->partition; part; part = part->next) part->flag = 0; /* clear flag */

	/* select candidate with maximum gain in partition with greatest
	 * need for cell area increase. If maximum gain <= 0, then try
	 * partition with second most need for area increase, etc.
	 * If no suitable candidate can be found, return.
	 */
	for (maxgain = MINGAIN;
	    (acceptCandidateEvenIfNegativeGain && maxgain == MINGAIN) ||
	    (!acceptCandidateEvenIfNegativeGain && maxgain <= 0);)
	{
	    /* select partition with greatest need for area increase */
	    for (maxareaneed = 0, part = total->partition; part; part = part->next)
	    {
		if (part->flag) continue; /* already tried this one: has no good candidates */
		if ((tmp = part->permitarea - part->cellarea) > maxareaneed) {
		    maxareaneed = tmp;
		    thispart = part;
		}
	    }

	    if (maxareaneed <= 0 /* no partitions ready to accept a candidate */
		|| alarm_flag    /* received async. request for abortion */
		|| (too_slow = we_are_going_to_slow (total)))
	    {
		/*
		printf ("\nStopped because (maxareaneed<=0, alarm_flag, too_slow) = (%d,%d,%d)\n",
			maxareaneed<=0, alarm_flag, too_slow);
		*/
		fixnetlist (total->bestpart, total->topcell);
		printstatisticsinstatusfield (total); /* just a nice feature */
		return (TRUE);
	    }

	    if (makeRandomMove (total)) {
		thiscand = thispart->candidates; /* just take the first candidate */
		updateTemperature (total);
		break; /* must break explicitely from loop, since gain may be < 0 */
	    }
	    else { /* select candidate with largest gain */
		for (maxgain = MINGAIN, cand = thispart->candidates; cand; cand = cand->nextinpart)
		    if (cand->gain > maxgain) {
			maxgain = cand->gain;
			thiscand = cand;
		    }
		if ((acceptCandidateEvenIfNegativeGain && maxgain == MINGAIN) ||
		    (!acceptCandidateEvenIfNegativeGain && maxgain <= 0)) thispart->flag = TRUE;
	    }
	}

	total->nmoves += 1;
	movecandidateandupdategains (thiscand, total);

	if (total->netcost < total->bestnetcost) {
	    total->bestnetcost = total->netcost;
	    rememberthispartitioning (total);
	}
    }
}

/* move candidate from the partition it is currently a member of ("srcpart")
 * to the partition it is currently a candidate for ("dstpart").
 */
void movecandidateandupdategains (PARTLISTPTR thecandidate, TOTALPPTR total)
{
    PARTITIONPTR dstpart, srcpart;
    int          dstparti, srcparti;
    PARTCELLPTR  thecell = thecandidate->cell;
    PARTLISTPTR  plist, oplist;

    if ((dstpart = thecandidate->partition) == (srcpart = thecell->partition)) {
	fprintf (stderr, "dopartitioning: internal error 156\n");
	dumpcore();
    }
    srcparti = srcpart->numid;
    dstparti = dstpart->numid;
    updategains (thecell, dstparti, srcparti, total);

    /* unlink cell to be moved ("thecell") from member list of source partition */
    for (oplist = NULL, plist = thecell->partlist; plist; plist = (oplist = plist)->nextincell)
	if (plist->ismember) break;

    if (!plist || plist->partition != srcpart) {
	fprintf (stderr, "dopartitioning: internal error 523\n");
	dumpcore();
    }

    if (!plist->previnpart)
	srcpart->members = plist->nextinpart;
    else
	plist->previnpart->nextinpart = plist->nextinpart;

    if (plist->nextinpart)
	plist->nextinpart->previnpart = plist->previnpart;
    srcpart->cellarea -= thecell->area;

    /* also unlink from partlist of thecell */
    if (!oplist)
	thecell->partlist = plist->nextincell; /* plist was first element in thecell->partlist */
    else
	oplist->nextincell = plist->nextincell;

    FreePartlist (plist);

    /* unlink candidate from destination partition's candidate list.... */
    if (!thecandidate->previnpart)
	dstpart->candidates = thecandidate->nextinpart;
    else
	thecandidate->previnpart->nextinpart = thecandidate->nextinpart;

    if (thecandidate->nextinpart)
	thecandidate->nextinpart->previnpart = thecandidate->previnpart;

    /* and move it to the front of the destination partition's member list */
    thecandidate->previnpart = NULL;
    thecandidate->nextinpart = dstpart->members;

    if (dstpart->members) dstpart->members->previnpart = thecandidate;
    dstpart->members = thecandidate;
    thecandidate->ismember = TRUE;

    /* keep rest of datastructure consistent */
    thecell->partition = dstpart;
    dstpart->cellarea += thecell->area;
    dstpart->nmembers += 1;
    srcpart->nmembers -= 1;
}

void updategains (PARTCELLPTR thecell, int dstparti, int srcparti, TOTALPPTR total)
{
    PNETPTR pnet;

    /* update distribution and cost of all nets connected to thecell */
    for (pnet = thecell->topnets; pnet; pnet = pnet->next)
    {
	if (pnet->ignore) continue; /* heuristic to decrease number of updates needed */
	updategainsincellsinnet (pnet->net, dstparti, srcparti, total);
    }
}

/* Find out what other cells connect to net and adapt their gains (as
 * far as this net is concerned).
 */
void updategainsincellsinnet (NETPTR net, int movdst, int movsrc, TOTALPPTR total)
{
    int         movsrcfreq, movdstfreq;
    int        *oldnetdistr, srcparti, dstparti;
    int         oldgainforthisnet, newgainforthisnet;
    CFUNC      *costfunction = total->costfunction;
    CIRPORTREFPTR cpref;
    PARTCELLPTR   pcell;
    PARTLISTPTR   cand;
    NETSTATEPTR   oldnetstate, newnetstate;

    /* oldnetstate is the pre-move net distribution */
    oldnetstate = (NETSTATEPTR)net->flag.p;
    oldnetdistr = oldnetstate->dist;
    movsrcfreq = oldnetdistr[movsrc] - 1;
    movdstfreq = oldnetdistr[movdst] + 1;

    if (movsrcfreq > 1 && movdstfreq > 2) {
	/* no CRITICAL change in net distribution, need no gain adjustment
	 * just update the netdistribution
	 */
	oldnetdistr[movsrc] = movsrcfreq;
	oldnetdistr[movdst] = movdstfreq;
	return;
    }

    /* newnetstate is the post-move net distribution */
    newnetstate = domove_and_copy_netstate (oldnetstate, total->tmpstatebuf,
    movdst, movsrc, costfunction);

    /***************************************************/
    if (movsrcfreq == 1 && movdstfreq == 2)
	test_xx[1] += 1;
    else
	test_xx[2] += 1;
    /***************************************************/

    for (cpref = net->terminals; cpref; cpref = cpref->next)
    {
	if (!cpref->cirinst) continue; /* terminal of topcell itself, skip */

	/* set pcell to cell that connects to the redistributed net */
	pcell = (PARTCELLPTR)cpref->cirinst->flag.p;
	srcparti = pcell->partition->numid;

	/* for each partition ("dstparti") where pcell is candidate, adjust
	 * gain (as far as this net is concerned of course)
	 */
	for (cand = pcell->partlist; cand; cand = cand->nextincell)
	{
	    if (cand->ismember) continue; /* only have gains in candidates */

	    dstparti = cand->partition->numid;

	    /* compute the old gain for this net */
	    oldgainforthisnet = gainnetstate (oldnetstate, dstparti, srcparti, costfunction);
	    newgainforthisnet = gainnetstate (newnetstate, dstparti, srcparti, costfunction);

	    /* adjust the gain */
	    cand->gain += (newgainforthisnet - oldgainforthisnet);
	}
    }

    /* adjust the net cost */
    total->netcost += (newnetstate->cost - oldnetstate->cost);

    /* Former tmpstatebuf is now officially incorporated with newnetstate.
     * Now release the space pointed to by oldnetstate:
     */
    total->tmpstatebuf = oldnetstate->dist;
    FreeNetstate (oldnetstate);
    net->flag.p = (void *)newnetstate;
}

#define RECURSIVELY TRUE

/* Store the current partitioning in a seadif CIRCUIT structure.
 * The topcircuit is a copy of total->topcell. An extra level of hierarchy
 * is added that represents the partitions. (This is fake: the netlist
 * is not taken into account at the moment, but fixnetlist() is gonna
 * take care for this when genpart() finishes.)
 */
void rememberthispartitioning (TOTALPPTR total)
{
    CIRCUITPTR   top;
    char         tmpstr[200];
    PARTITIONPTR part;

    total->bestmove = total->nmoves;    /* remember this move */
    if (total->bestpart)
	sdfdeletecircuit (total->bestpart, RECURSIVELY); /* from sealib (delete.c) */
    NewCircuit (top);
    total->bestpart = top;

    /* sprintf (tmpstr, "%s_p", total->topcell->name); */
    sprintf (tmpstr, "%d=%dx%d", total->numparts, total->nx, total->ny);

    top->name = cs (tmpstr);
    top->function = total->topcell->function;
    top->next = top->function->circuit;
    top->function->circuit = top;

    for (part = total->partition; part; part = part->next)
    {
	CIRINSTPTR  newinst;
	CIRCUITPTR  newcirc;
	PARTLISTPTR member;

	NewCirinst (newinst);
	sprintf (tmpstr, "%d", part->numid); /* miniplaza() requires inst name to be a number */
	newinst->name = cs (tmpstr);
	newinst->curcirc = top;
	newinst->next = top->cirinst;
	top->cirinst = newinst;
	NewCircuit (newcirc);
	newinst->circuit = newcirc;
	newcirc->linkcnt = 1;
	sprintf (tmpstr, "partition_%d", ++globalnamecounter);
	newcirc->name = cs (tmpstr);
	/* this is rather horrifying, but fixnetlist() cleans up the mess... */
	newcirc->function = NULL;

	for (member = part->members; member; member = member->nextinpart)
	{
	    CIRINSTPTR child;
	    NewCirinst (child);
	    child->name = cs (member->cell->cinst->name); /* keep old cir-inst name */
	    child->curcirc = newcirc;
	    child->circuit = member->cell->cinst->circuit;
	    child->circuit->linkcnt += 1;
	    child->next = newcirc->cirinst;
	    newcirc->cirinst = child;
	    /* fixnetlist() needs a pointer to the newly created instance: */
	    /* member->cell->cinst->flag.p = (void *)child; */
	    member->cell->copycinst = child;
	}
    }
}

/* Circuit ntop is a three level hierarchy version of the two level otop.
 * This function looks at the netlist of otop and creates an
 * equivalent netlist on ntop, taking into account the new hierarchy.
 * The function also puts the circuits on the new hierarchical level
 * in a function named "&otopfunname_parts&", where otopfunname is the
 * name of the function of ntop. This is a bit of a hack but it avoids
 * creating lots of meaningless functions in the current library.
 */
void fixnetlist (CIRCUITPTR ntop, CIRCUITPTR otop)
{
    NETPTR        onet, nnet;
    CIRPORTREFPTR ocpr;
    FUNCTIONPTR   nfun;
    LIBRARYPTR    lib = otop->function->library;
    char        tmpstr[300], *nfunname;
    CIRINSTPTR  pcinst;
    int         disappears;

    /* First put all circuits in the function "&otopfunname_part&" */
    sprintf (tmpstr, "%s", otop->function->name);
    nfunname = cs (tmpstr);

    if (!alreadyhavefun (nfunname, otop->function->library))
    {
	NewFunction (nfun);
	nfun->name = nfunname;
	nfun->library = lib;
	nfun->next = lib->function;
	lib->function = nfun;
    }
    else
	nfun = thisfun;

    for (pcinst = ntop->cirinst; pcinst; pcinst = pcinst->next)
    {
	CIRCUITPTR pcir = pcinst->circuit;
	pcir->function = nfun;
	pcir->next = nfun->circuit;
	nfun->circuit = pcir;
	pcir->flag.p = (void *)pcinst; /* need this flag in copycpr() */
    }

    /* Now expand the top-level netlist to a two-level netlist */
    for (onet = otop->netlist; onet; onet = onet->next)
    {
	/* create a copy of the old net (onet) on the new top circuit (ntop) */
	NewNet (nnet);
	nnet->name = cs (onet->name);
	nnet->num_term = 0;
	nnet->circuit = ntop;
	disappears = netdisappearsinsinglepartition (onet);

	/* create netlist on partition circuits of ntop (= children of ntop) */
	for (ocpr = onet->terminals; ocpr; ocpr = ocpr->next)
	    copycpr (ntop, nnet, ocpr, disappears);

	/* if the net totally disappeared into one of the partitions, remove it from ntop: */
	if (disappears) {
	    FreeNet (nnet);
	}
	else {
	    nnet->next = ntop->netlist;
	    ntop->netlist = nnet;
	}
    }
}

void copycpr (CIRCUITPTR ntop, NETPTR nnet, CIRPORTREFPTR ocpr, int disappears)
{
    CIRCUITPTR    pcir;
    CIRINSTPTR    pcinst;
    NETPTR        pnet;
    CIRPORTREFPTR ncpr, pcpr;
    CIRPORTPTR    ncp, pcp;

    if (!ocpr->cirinst)
    {
	/* create a copy of the cirport on ntop */
	NewCirport (ncp);
	ncp->name = cs (ocpr->cirport->name);
	ncp->net = nnet;
	ncp->next = ntop->cirport;
	ntop->cirport = ncp;
	NewCirportref (ncpr);
	ncpr->net = nnet;
	ncpr->cirport = ncp;
	ncpr->cirinst = NULL;
	ncpr->next = nnet->terminals;
	nnet->terminals = ncpr;
	nnet->num_term++;
    }
    else
    {
	/* pcinst = (CIRINSTPTR)ocpr->cirinst->flag.p; */
	pcinst = ((PARTCELLPTR)ocpr->cirinst->flag.p)->copycinst;
	pcir = pcinst->curcirc;

	if (!pcir->netlist || pcir->netlist->name != nnet->name)
	{
	    /* create new net and cirport on pcir and link this to the nnet on circuit ntop */
	    NewNet (pnet);
	    pnet->name = cs (nnet->name);
	    pnet->circuit = pcir;
	    pnet->next = pcir->netlist;
	    pcir->netlist = pnet;

	    if (!disappears)
	    { /* provide connection to the "ntop" circuit and its net "nnet" */
		NewCirport (pcp);
		pcp->name = cs (nnet->name);
		pcp->net = pnet;
		pcp->next = pcir->cirport;
		pcir->cirport = pcp;
		NewCirportref (pcpr);
		pcpr->net = pnet;
		pcpr->cirport = pcp;
		pcpr->cirinst = NULL;
		pnet->terminals = pcpr;
		pnet->num_term = 1;
		NewCirportref (ncpr);
		ncpr->net = nnet;
		ncpr->cirport = pcp;
		ncpr->cirinst = (CIRINSTPTR)pcir->flag.p; /* set in fixnetlist() */
		ncpr->next = nnet->terminals;
		nnet->terminals = ncpr;
		nnet->num_term++;
	    }
	}

	/* create a cirportref at the new hierarchical level */
	NewCirportref (pcpr);
	pcpr->net = pcir->netlist;
	pcpr->cirport = ocpr->cirport;
	pcpr->cirinst = pcinst;
	pcpr->next = pcir->netlist->terminals;
	pcir->netlist->terminals = pcpr;
	pcir->netlist->num_term++;
    }
}

int netdisappearsinsinglepartition (NETPTR onet)
{
    CIRPORTREFPTR ocpr;
    CIRCUITPTR    pcir, firstpartcir;

    firstpartcir = NULL;
    for (ocpr = onet->terminals; ocpr; ocpr = ocpr->next)
    {
	if (!ocpr->cirinst) return (0); /* has terminal on circuit otop, cannot disappear */
	/* pcir = ((CIRINSTPTR)ocpr->cirinst->flag.p)->curcirc; */
	pcir = ((PARTCELLPTR)ocpr->cirinst->flag.p)->copycinst->curcirc;
	if (firstpartcir && firstpartcir != pcir) return (0);
	firstpartcir = pcir;
    }
    return (TRUE);
}

/* Return TRUE if too many moves did not improve on bestpart.
 * This is a heuristic to speed up iteration.
 */
#define JUST_A_NUMBER 40

PRIVATE int we_are_going_to_slow (TOTALPPTR total)
{
    return ((total->nmoves > (2 * total->bestmove + total->numparts + JUST_A_NUMBER)));
}

#define MAXSTR 300

/* Some hardware does not like big arrays on the stack:
 */
PRIVATE char str[1+MAXSTR];

PRIVATE void printstatisticsinstatusfield (TOTALPPTR total)
{
    STATUSPTR status = total->bestpart->status;

    if (!status) {
	NewStatus (status);
	total->bestpart->status = status;
    }

    sprintf (str, "genpart[numparts=%d,strtnetcost=%d,bestnetcost=%d,nmoves=%d,area=%d]",
	total->numparts, total->strtnetcost, total->bestnetcost, total->nmoves, total->area);

    if (status->program)
    { /* already some status information, don't destroy */
	strncat (str, "; ", MAXSTR);
	strncat (str, status->program, MAXSTR);
	fs (status->program);
    }
    status->program = cs (str);
}

/* Like sdfexistsfun() but operates on in-core tree in stead of in-file tree.
 */
int alreadyhavefun (STRING fname, LIBRARYPTR lib)
{
    FUNCTIONPTR fun;

    if (!lib) return (0);

    for (fun = lib->function; fun; fun = fun->next)
	if (fun->name == fname) break;

    if (!fun) return (0);

    thisfun = fun;
    return (TRUE);
}
