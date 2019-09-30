/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Patrick Groeneveld
 *	Simon de Graaf
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
 * r_add structures to seadif
 */

#include "src/ocean/trout/typedef.h"
#include "src/ocean/trout/grid.h"

static void add_r_inst (SLICEPTR slice, LAYOUTPTR father);
static int  add_r_net (CIRCUITPTR father);
static void add_r_port (LAYOUTPTR father);
static void repair_r_inst (SLICEPTR slice, LAYOUTPTR father);

/*
 * import
 */
extern long Chip_num_layer; /* number of metal layers to be used */

/*
 * A number of datastructures will have to be added to the
 * basic seadif structure, in order to facilitate the global routing.
 */
void mk_datastr (LAYOUTPTR father)
{
    R_CELLPTR rcell;

    if (!father->circuit) {
	fprintf (stderr, "ERROR: father layout '%s' has no circuit\n", father->name);
	error (FATAL_ERROR, "mk_datastr");
    }

    /* add additions
     */
    NewR_cell (rcell);
    father->circuit->flag.p = (void *) rcell;
    rcell->circuit = father->circuit;
    father->flag.p = (void *) rcell;
    rcell->layout = father;
    rcell->num_net = 0;

    /* add instance structures */
    add_r_inst (father->slice, father);

    /* add instance structures to left-over
     * instances (with no correct name)
     */
    repair_r_inst (father->slice, father);

    /* add net structures */
    rcell->num_net = add_r_net (father->circuit);

    /* add terminal/port structures */
    add_r_port (father);

    /* determine positions of missing layout terminals */
    place_new_terminals (father);

    /* add an error cell (null now) */
    rcell->error = NULL;
}

/*
 * This routine adds/initializes the r_inst structure to the
 * LAYINST datastructure of father.
 * It is a recursive routine due to the storage format.
 */
static void add_r_inst (SLICEPTR slice, LAYOUTPTR father)
{
    R_INSTPTR rinst;
    register CIRINSTPTR cirinst;
    register LAYINSTPTR layinst;
    int help;

    for ( ; slice; slice = slice->next)
    {
	if (slice->chld_type == SLICE_CHLD) { /* another slice: recursion */
	    add_r_inst (slice->chld.slice, father);
	    continue;
	}

	/* child contains instances: read all instances */
	for (layinst = slice->chld.layinst; layinst; layinst = layinst->next)
	{
	    /* clean up, just to be sure */
	    layinst->flag.p = NULL;

	    /* find corresponding circuit instance */
	    for (cirinst = father->circuit->cirinst; cirinst; cirinst = cirinst->next) {
		if (cirinst->name == layinst->name && !cirinst->flag.p)
		{ /* same name, not already assigned */
		    if (layinst->layout->circuit == cirinst->circuit) break; /* link */
		    fprintf (stderr, "WARNING: circuit and layout of instance '%s' are inconsistent\n",
			cirinst->name);
		    fprintf (stderr, "         layout cell name: '%s'; circuit cell name '%s'\n",
			layinst->layout->name, cirinst->circuit->name);
		}
	    }
	    if (!cirinst) { /* not found */
		/*
		fprintf (stderr, "WARNING: no exact cirinst-layinst correspondence for layout instance '%s'\n", layinst->name);
		fprintf (stderr, "         (cell '%s'): random assignment\n", layinst->layout->name);
		*/
		continue;
	    }

	    NewR_inst (rinst); /* make new rinst */
	    /* double link */
	    rinst->layinst = layinst;
	    layinst->flag.p = (void *) rinst;
	    rinst->cirinst = cirinst;
	    cirinst->flag.p = (void *) rinst;

	    /* calc placed coordinates */
	    rinst->crd[L] = layinst->mtx[0] * layinst->layout->off[X] +
			    layinst->mtx[1] * layinst->layout->off[Y] +
			    layinst->mtx[2];

	    rinst->crd[R] = layinst->mtx[0] * (layinst->layout->off[X] + layinst->layout->bbx[X]) +
			    layinst->mtx[1] * (layinst->layout->off[Y] + layinst->layout->bbx[Y]) +
			    layinst->mtx[2];

	    rinst->crd[B] = layinst->mtx[3] * layinst->layout->off[X] +
			    layinst->mtx[4] * layinst->layout->off[Y] +
			    layinst->mtx[5];

	    rinst->crd[T] = layinst->mtx[3] * (layinst->layout->off[X] + layinst->layout->bbx[X]) +
			    layinst->mtx[4] * (layinst->layout->off[Y] + layinst->layout->bbx[Y]) +
			    layinst->mtx[5];

	    if (rinst->crd[L] > rinst->crd[R]) { /* swap */
		help = rinst->crd[L]; rinst->crd[L] = rinst->crd[R]; rinst->crd[R] = help;
	    }
	    if (rinst->crd[B] > rinst->crd[T]) { /* swap */
		help = rinst->crd[B]; rinst->crd[B] = rinst->crd[T]; rinst->crd[T] = help;
	    }

	    /* all other values are still undetermined */
	}
    }
}

/*
 * This routine looks for unassigned layout-circuit instances
 * and links them together, if necessary.
 */
static void repair_r_inst (SLICEPTR slice, LAYOUTPTR father)
{
    R_INSTPTR rinst;
    register CIRINSTPTR cirinst;
    register LAYINSTPTR layinst;
    int help;

    for ( ; slice; slice = slice->next)
    {
	if (slice->chld_type == SLICE_CHLD) { /* another slice: recursion */
	    repair_r_inst (slice->chld.slice, father);
	    continue;
	}

	/* child contains instances: read all instances */
	for (layinst = slice->chld.layinst; layinst; layinst = layinst->next)
	{
	    if (layinst->flag.p) continue; /* already assigned */

	    /* unassigned */

	    /* find a corresponding and free circuit instance */
	    for (cirinst = father->circuit->cirinst; cirinst; cirinst = cirinst->next) {
		if (!cirinst->flag.p && cirinst->circuit == layinst->layout->circuit)
		    break; /* same circuit and not already assigned */
	    }
	    if (!cirinst) { /* not found */
		fprintf (stderr, "WARNING: No circuit for layout instance '%s'\n", layinst->name);
		fprintf (stderr, "         (cell '%s'): it will not be routed\n", layinst->layout->name);
		continue;
	    }

	    /* give layinst other name */
	    fprintf (stderr, "Orphan layinst '%s' was assigned (and renamed) to cirinst '%s'\n",
		layinst->name, cirinst->name);

	    forgetstring (layinst->name);
	    layinst->name = canonicstring (cirinst->name);

	    NewR_inst (rinst); /* make new rinst */
	    /* double link */
	    rinst->layinst = layinst;
	    layinst->flag.p = (void *) rinst;
	    rinst->cirinst = cirinst;
	    cirinst->flag.p = (void *) rinst;

	    /* calc placed coordinates */
	    rinst->crd[L] = layinst->mtx[0] * layinst->layout->off[X] +
			    layinst->mtx[1] * layinst->layout->off[Y] +
			    layinst->mtx[2];

	    rinst->crd[R] = layinst->mtx[0] * (layinst->layout->off[X] + layinst->layout->bbx[X]) +
			    layinst->mtx[1] * (layinst->layout->off[Y] + layinst->layout->bbx[Y]) +
			    layinst->mtx[2];

	    rinst->crd[B] = layinst->mtx[3] * layinst->layout->off[X] +
			    layinst->mtx[4] * layinst->layout->off[Y] +
			    layinst->mtx[5];

	    rinst->crd[T] = layinst->mtx[3] * (layinst->layout->off[X] + layinst->layout->bbx[X]) +
			    layinst->mtx[4] * (layinst->layout->off[Y] + layinst->layout->bbx[Y]) +
			    layinst->mtx[5];

	    if (rinst->crd[L] > rinst->crd[R]) { /* swap */
		help = rinst->crd[L]; rinst->crd[L] = rinst->crd[R]; rinst->crd[R] = help;
	    }
	    if (rinst->crd[B] > rinst->crd[T]) { /* swap */
		help = rinst->crd[B]; rinst->crd[B] = rinst->crd[T]; rinst->crd[T] = help;
	    }
	}
    }
}

/*
 * This routine adds the 'r_net' structure to each net of father.
 */
static int add_r_net (CIRCUITPTR father)
{
    NETPTR hnet;
    R_NETPTR rnet;
    int num_net = 0;

    for (hnet = father->netlist; hnet; hnet = hnet->next) {
	NewR_net (rnet);
	/* double link */
	rnet->net = hnet;
	hnet->flag.p = (void *) rnet;

	rnet->routed = FALSE;
	rnet->routing_attempts = 0;
	rnet->fail_count = 0;

	num_net++;
    }

    return (num_net);
}

/*
 * This routine adds the r_port to seadif.
 */
static void add_r_port (LAYOUTPTR father)
{
    NETPTR cirnet;
    register CIRPORTREFPTR cportref;
    LAYINSTPTR layinst;
    register LAYPORTPTR layport;
    R_PORTPTR rport;

    /*
     * step along all nets
     */
    for (cirnet = father->circuit->netlist; cirnet; cirnet = cirnet->next)
    {
	/*
	 * we re-conut the number of terminals of the net, since some terminals
	 * could be illegal.
	 */
	cirnet->num_term = 0;

	/*
	 * step along all terminals (cirportrefs)
	 */
	for (cportref = cirnet->terminals; cportref; cportref = cportref->next)
	{
	    layinst = NULL;

	    /* just to be sure */
	    cportref->flag.p = NULL;

	    if (!cportref->cirport) {
		fprintf (stderr, "WARNING: cirportref (circuit '%s', net '%s') has no circuit port\n",
		    father->circuit->name, cirnet->name);
		continue;
	    }

	    if (cportref->cirinst) { /* a terminal on a son cell */
		if (!cportref->cirinst->flag.p) continue; /* port on an inconsistent son-cell */

		/* get layinst */
		if (!(layinst = ((R_INSTPTR) cportref->cirinst->flag.p)->layinst)) {
		    fprintf (stderr, "WARNING: missing layinst in rinst\n");
		    continue;
		}

		/*
		 * find the corresponding layout term of cportref
		 */
		if (!cportref->cirinst->circuit) {
		    fprintf (stderr, "WARNING (shit): cportref->cirinst->circuit is NULL\n");
		    continue;
		}

		/* should be equal */
		if (layinst->layout->circuit != cportref->cirinst->circuit) {
		    fprintf (stderr, "WARNING: layinst->layout->circuit != cportref->cirinst->circuit\n");
		    continue;
		}

		/*
		 * find terminal on layinst which points to cportref->cirport,
		 * that is, find the layout terminal belonging to the specified circuit terminal.
		 */
		for (layport = layinst->layout->layport; layport; layport = layport->next)
		    if (layport->cirport == cportref->cirport) break;
		if (!layport) {
		    fprintf (stderr, "WARNING/ERROR: cannot find equiv. layoutport of cirport '%s'\n",
			cportref->cirport->name);
		    fprintf (stderr, "         (circell '%s') on layout cell '%s'.\n",
			cportref->cirinst->circuit->name, layinst->layout->name);
		    continue;
		}

		/* make struct, link it */
		NewR_port (rport);
		rport->cirportref = cportref;
		cportref->flag.p = (void *) rport;

		rport->routed = FALSE;
		rport->unassigned = FALSE;

		rport->layinst = layinst;
		rport->layport = layport;

		rport->net = cirnet;

		/* calc abs. position of port */
		rport->crd[X] = layinst->mtx[0] * layport->pos[X] +
				layinst->mtx[1] * layport->pos[Y] +
				layinst->mtx[2];
		rport->crd[Y] = layinst->mtx[3] * layport->pos[X] +
				layinst->mtx[4] * layport->pos[Y] +
				layinst->mtx[5];
		rport->crd[Z] = layport->layer;

		/* link in terminal list of instance */
		rport->next_inst = ((R_INSTPTR) layinst->flag.p)->portlist;
		((R_INSTPTR) layinst->flag.p)->portlist = rport;
	    }
	    else { /* terminal on parent */

		for (layport = father->layport; layport; layport = layport->next)
		    if (layport->cirport == cportref->cirport) break;
		if (!layport) {
		    /* make new layport, will be placed later on */
		    NewLayport (layport);
		    layport->cirport = cportref->cirport;
		    layport->layer = -1;   /* flag unassigned */
		    layport->next = father->layport;
		    father->layport = layport;
		}

		/* make struct, link it */
		NewR_port (rport);
		rport->cirportref = cportref;
		cportref->flag.p = (void *) rport;

		/* father can contain double link */
		rport->layinst = NULL;
		rport->layport = layport;
		rport->unassigned = FALSE;
		layport->flag.p = (void *) rport;

		rport->routed = FALSE;

		rport->net = cirnet;

		/* store position of port */
		rport->crd[X] = layport->pos[X];
		rport->crd[Y] = layport->pos[Y];
		rport->crd[Z] = layport->layer;
	    }
	    cirnet->num_term++;
	}
    }
}

/*
 * This routine places the terminals which have not been placed before
 * it will also be called before a re-routing.
 */
void place_new_terminals (LAYOUTPTR father)
{
    NETPTR net;
    LAYPORTPTR layport;
    R_PORTPTR rport;
    CIRPORTREFPTR cportref;

    for (layport = father->layport; layport; layport = layport->next) {

	if (!(rport = (R_PORTPTR) layport->flag.p)) continue; /* no rport struct */

	if (rport->unassigned == FALSE && layport->layer >= 0) continue; /* already pre-placed */

	if (!(net = rport->net)) {
	    fprintf (stderr, "ERROR (place_new_terminals): net missing\n");
	    if (rport->cirportref) rport->cirportref->flag.p = NULL;
	    layport->flag.p = NULL;
	    continue;
	}

	/* look for other terminal in net (just take the first) */
	for (cportref = net->terminals; cportref; cportref = cportref->next) {
	    if (cportref->cirinst && cportref->flag.p) break; /* placed terminal */
	}

	if (cportref) { /* use placed position */
	    layport->pos[X] = rport->crd[X] = ((R_PORTPTR) cportref->flag.p)->crd[X];
	    layport->pos[Y] = rport->crd[Y] = ((R_PORTPTR) cportref->flag.p)->crd[Y];
	    layport->layer  = rport->crd[Z] = ((R_PORTPTR) cportref->flag.p)->crd[Z];
	}
	else {
	    /* Unable to assign a position now, we will try to assign a
	     * position later, somewhere on the border
	     */
	    layport->layer  = rport->crd[Z] = NEED_PLACEMENT;
	}

	/* flag that it was placed by the router */
	rport->unassigned = TRUE;
    }
}
