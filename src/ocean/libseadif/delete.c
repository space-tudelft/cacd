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

#include "src/ocean/libseadif/libstruct.h"
#include <stdio.h>
#include "src/ocean/libseadif/sea_decl.h"

void sdfdeletecirport (CIRPORTPTR cirport)
{
    CIRPORTPTR nextcirport;

    for (; cirport; cirport = nextcirport)
    {
	nextcirport = cirport->next;
	FreeCirport (cirport);
    }
}

/* remove circuit if circuit->linkcnt gets zero */
void sdfdeletecirinst (CIRINSTPTR cirinst, int recursively)
{
    CIRINSTPTR nextcirinst;

    for (; cirinst; cirinst = nextcirinst)
    {
	if (cirinst->circuit) {
	    if (--(cirinst->circuit->linkcnt) <= 0 && recursively)
		sdfdeletecircuit (cirinst->circuit, recursively);
	}
	nextcirinst = cirinst->next;
	FreeCirinst (cirinst);
    }
}

void sdfdeletenetlist (NETPTR netlist)
{
    CIRPORTREFPTR cpr, nextcpr;
    NETPTR nextnet;

    for (; netlist; netlist = nextnet)
    {
	for (cpr = netlist->terminals; cpr; cpr = nextcpr)
	{
	    nextcpr = cpr->next;
	    FreeCirportref (cpr);
	}
	nextnet = netlist->next;
	FreeNet (netlist);
    }
}

void sdfdeletebuslist (BUSPTR buslist)
{
    NETREFPTR netref, nextnetref;
    BUSPTR nextbus;

    for (; buslist; buslist = nextbus)
    {
	for (netref = buslist->netref; netref; netref = nextnetref)
	{
	    nextnetref = netref->next;
	    FreeNetRef (netref);
	}
	nextbus = buslist->next;
	FreeBus (buslist);
    }
}

/* delete a circuit, even if its linkcnt != 0 */
void sdfdeletecircuit (CIRCUITPTR circuit, int recursively)
{
    CIRCUITPTR cir, ocir;

    if (!circuit) return;
    if (circuit->status) FreeStatus (circuit->status);
    if (circuit->cirport) sdfdeletecirport (circuit->cirport);
    if (circuit->cirinst) sdfdeletecirinst (circuit->cirinst, recursively);
    if (circuit->netlist) sdfdeletenetlist (circuit->netlist);
    if (circuit->buslist) sdfdeletebuslist (circuit->buslist);
    if (circuit->layout)  sdfdeletelayout (circuit->layout, recursively);
    if (circuit->function)
    {
	for (ocir = NULL, cir = circuit->function->circuit; cir; cir = (ocir = cir)->next)
	    if (cir == circuit) break;
	if (!cir) {
	    fprintf (stderr, "sdfdeletecircuit: circuit (%s(%s(%s))) not in function's circuit list\n",
		circuit->name, circuit->function->name, circuit->function->library->name);
	    dumpcore ();
	}
	if (!ocir)
	    cir->function->circuit = cir->next;
	else
	    ocir->next = cir->next;
    }
    FreeCircuit (circuit);
}

/* delete a layout even if its linkcnt != 0 */
void sdfdeletelayout (LAYOUTPTR layout, int recursively)
{
    LAYOUTPTR lay, olay;

    if (!layout) return;
    if (layout->status) FreeStatus (layout->status);
    if (layout->layport) sdfdeletelayport (layout->layport);
    if (layout->laylabel) sdfdeletelaylabel (layout->laylabel);
    if (layout->slice) sdfdeleteslice (layout->slice, recursively);
    if (layout->wire) sdfdeletewire (layout->wire);
    if (layout->circuit)
    {
	for (olay = NULL, lay = layout->circuit->layout; lay; lay = (olay = lay)->next)
	    if (lay == layout) break;
	if (!lay) {
	    fprintf (stderr, "sdfdeletelayout: layout (%s(%s(%s(%s)))) not in circuit's layout list\n",
		layout->name, layout->circuit->name, layout->circuit->function->name,
		layout->circuit->function->library->name);
	    dumpcore ();
	}
	if (!olay)
	    lay->circuit->layout = lay->next;
	else
	    olay->next = lay->next;
    }
    FreeLayout (layout);
}

void sdfdeletelayport (LAYPORTPTR layport)
{
    LAYPORTPTR nextlayport;

    for (; layport; layport = nextlayport)
    {
	nextlayport = layport->next;
	FreeLayport (layport);
    }
}

void sdfdeletelaylabel (LAYLABELPTR laylabel)
{
    LAYLABELPTR nextlaylabel;

    for (; laylabel; laylabel = nextlaylabel)
    {
	nextlaylabel = laylabel->next;
	FreeLaylabel (laylabel);
    }
}

void sdfdeleteslice (SLICEPTR slice, int recursively)
{
    if (!slice) return;

    if (slice->chld_type == SLICE_CHLD)
    {
	sdfdeleteslice (slice->chld.slice, recursively);
	FreeSlice (slice);
    }
    else if (slice->chld_type == LAYINST_CHLD)
    {
	sdfdeletelayinst (slice->chld.layinst, recursively);
	FreeSlice (slice);
    }
    else
	fprintf (stderr, "sdfdeleteslice: corrupt LaySlice, chld_type unknown\n");
}

void sdfdeletelayinst (LAYINSTPTR layinst, int recursively)
    /* if TRUE then remove layout, if layout->linkcnt gets zero */
{
    LAYINSTPTR nextlayinst;

    for (; layinst; layinst = nextlayinst)
    {
	if (layinst->layout) {
	    if (--(layinst->layout->linkcnt) <= 0 && recursively)
		sdfdeletelayout (layinst->layout, recursively);
	}
	nextlayinst = layinst->next;
	FreeLayinst (layinst);
    }
}

void sdfdeletewire (WIREPTR wire)
{
    WIREPTR nextwire;

    for (; wire; wire = nextwire)
    {
	nextwire = wire->next;
	FreeWire (wire);
    }
}

void sdfdeletetiming (TIMINGPTR timing) /* IK, delete timing */
{
    if (timing->status) FreeStatus (timing->status);

    if (timing->tminstlist) /* freeing time mod instances */
    {
	TMMODINST *tmPtr, *instPtr;
	for (tmPtr = timing->tminstlist; tmPtr;) {
	    instPtr = tmPtr;
	    tmPtr = instPtr->next;
	    FreeTmModInst (instPtr);
	}
    }

    if (timing->netmods) /* net models */
    {
	NETMOD *nmPtr, *instPtr;
	for (nmPtr = timing->netmods; nmPtr;) {
	    instPtr = nmPtr;
	    nmPtr = instPtr->next;
	    sdfdeletenetmod (instPtr);
	}
    }

    if (timing->t_terms) /* time terminals */
    {
	TIMETERM *ttPtr, *instPtr;
	for (ttPtr = timing->t_terms; ttPtr;) {
	    instPtr = ttPtr;
	    ttPtr = instPtr->next;
	    sdfdeletetterm (instPtr);
	}
    }

    if (timing->tPaths) /* delay propagation paths */
    {
	TPATH *tpPtr, *instPtr;
	for (tpPtr = timing->tPaths; tpPtr;) {
	    instPtr = tpPtr;
	    tpPtr = instPtr->next;
	    sdfdeletetpath (instPtr);
	}
    }

    if (timing->timeCost) /* time cost function */
	sdfdeletetimecost (timing->timeCost);

    if (timing->delays) /* delay assignments list */
    {
	DELASG *daPtr, *instPtr;
	for (daPtr = timing->delays; daPtr;) {
	    instPtr = daPtr;
	    daPtr = instPtr->next;
	    sdfdeletedelasg (instPtr);
	}
    }
}

void sdfdeletenetmod (NETMODPTR netmod)
{
    NETREF *nrPtr, *instPtr;
    BUSREF *brPtr, * iPtr;

    for (nrPtr = netmod->netlist; nrPtr;)
    {
	instPtr = nrPtr;
	nrPtr = instPtr->next;
	FreeNetRef (instPtr);
    }
    for (brPtr = netmod->buslist; brPtr;)
    {
	iPtr = brPtr;
	brPtr = iPtr->next;
	FreeBusRef (iPtr);
    }
}

void sdfdeletetterm (TIMETERMPTR t_term)
{
    TIMETERMREF *trPtr, *instPtr;
    CIRPORTREF *crPtr, *iPtr;

    for (trPtr = t_term->termreflist; trPtr;)
    {
	instPtr = trPtr;
	trPtr = instPtr->next;
	FreeTimeTermRef (instPtr);
    }
    for (crPtr = t_term->cirportlist; crPtr;)
    {
	iPtr = crPtr;
	crPtr = iPtr->next;
	FreeCirportref (iPtr);
    }
    if (t_term->type == OutputTTerm && t_term->timecost)
	sdfdeletetimecost (t_term->timecost);
}

void sdfdeletetpath (TPATHPTR tPath)
{
    TIMETERMREF *trPtr, *instPtr;

    for (trPtr = tPath->startTermList; trPtr;)
    {
	instPtr = trPtr;
	trPtr = instPtr->next;
	FreeTimeTermRef (instPtr);
    }
    for (trPtr = tPath->endTermList; trPtr;)
    {
	instPtr = trPtr;
	trPtr = instPtr->next;
	FreeTimeTermRef (instPtr);
    }
    sdfdeletetimecost (tPath->timeCost);
}

void sdfdeletetimecost (TIMECOSTPTR tcost)
{
    TCPOINT *tpPtr, *instPtr;

    for (tpPtr = tcost->points; tpPtr;)
    {
	instPtr = tpPtr;
	tpPtr = instPtr->next;
	FreeTcPoint (instPtr);
    }
}

void sdfdeletedelasg (DELASGPTR delasg)
{
    DELASGINST *diPtr, *instPtr;

    for (diPtr = delasg->pathDelays; diPtr;)
    {
	instPtr = diPtr;
	diPtr = instPtr->next;
	FreeDelAsgInst (instPtr);
    }
}
