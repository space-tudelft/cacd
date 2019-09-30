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
 * Remove hierarchy from a seadif circuit.
 */

#include <stdio.h>
#include "src/ocean/libseadif/libstruct.h"
#include <string.h>
#include "src/ocean/libseadif/sea_decl.h"

PRIVATE char *libprim_kwd = "libprim"; /* keyword in circuit->status->program indicates primitive circuit */
int sdfflattenuntillibprim = TRUE; /* if FALSE flatten until very leave cells (nenh,penh) */

PRIVATE void sdfflatcir_ (CIRCUITPTR circuit, int libprim);
PRIVATE void subst_net_for_cpref (CIRPORTREFPTR *cportref, CIRPORTREFPTR *oldcpr,
                   NETPTR net, NETPTR newnet);
PRIVATE void remove_boarder_terminals (NETPTR net);
PRIVATE STRING newname (STRING oldname, STRING extension);
PRIVATE void treeclearcircuits (CIRCUITPTR circuit);

/* Remove all hierarchy from a circuit, controlled by global sdfflattenuntillibprim.
 */
void sdfflatcir (CIRCUITPTR circuit)
{
    if (circuit) {
	treeclearcircuits (circuit);
	sdfflatcir_ (circuit, sdfflattenuntillibprim);
    }
}

PRIVATE void sdfflatcir_ (CIRCUITPTR circuit, int libprim)
{
    CIRINSTPTR chldinst;
    CIRCUITPTR c;
    int grandchildren = 0;

    if (!circuit) return;

    for (chldinst = circuit->cirinst; chldinst; chldinst = chldinst->next)
    if ((c = chldinst->circuit)->cirinst &&
	!(libprim && c->status && issubstring (c->status->program, libprim_kwd)))
    {
	grandchildren = TRUE;
	sdfflatcir_ (c, libprim);
    }

    if (grandchildren) grandpa_meets_grandchildren (circuit, libprim);
}

void grandpa_meets_grandchildren (CIRCUITPTR grandpa, int libprim)
   /* If TRUE flatten until circuits marked "libprim" in status->program */
{
    NETPTR        onet, net, chldnet, connectnet;
    CIRINSTPTR    chldinst, ochld, removechld;
    CIRPORTREFPTR cportref, oldcpr, removecpr;
    CIRPORTPTR    cirport, prevcp, cp;
    CIRCUITPTR    c, newc, prevc;
    long          linkcnt;
    int           nets_are_connected_thru_switchbox;

    if (!grandpa) return;

    /* Arrange for grandpa to uniquely reference its children which have children
     * themselves, i.e. all the parents of the grandchildren must have linkcnt=1.
     */
    connectnet = NULL; /* suppres uninitialized warning */

    for (chldinst = grandpa->cirinst; chldinst; chldinst = chldinst->next)
    {
	if (!(c = chldinst->circuit)) {
	    fprintf (stderr, "Circuit '%s' has instance referring to NULL pointer !\n\n", c->name);
	    dump_circuit (stderr, c);
	    dumpcore ();
	}

	if (c->linkcnt <= 0) {
	    fprintf (stderr, "Circuit '%s' has linkcnt <= 0 ! \n\n", chldinst->circuit->name);
	    dump_circuit (stderr, c);
	    dumpcore ();
	}

	/* fprintf (stderr, "(%s(%s(%s))), linkcnt=%d\n",
	    c->name, c->function->name, c->function->library->name, c->linkcnt);
	*/
	if (!c->cirinst || (libprim && c->status && issubstring (c->status->program, libprim_kwd)))
	    continue; /* primitive (leave cell): not a parent of grandchildren */
	if (c->linkcnt == 1)
	    continue; /* linkcount already 1 */

	/* Search for last copy in the (doubly linked) chain of identical circuits. */
	for (c = chldinst->circuit; c; c = c->next)
	    if (!c->next || c->name != c->next->name) break;

	newc = copycircuit (c);   /* make a copy of the circuit c */
	newc->linkcnt = 1;
	chldinst->circuit->linkcnt -= 1;
	chldinst->circuit = newc;
	newc->next = c->next;     /* link forward */
	c->next = newc;
	newc->flag.p = (void *)c; /* HACK, link backward */
    }

    /* At this moment, grandpa's terminals still point to the
     * original cirports in stead of the copies. Go fix this.
     */
    for (net = grandpa->netlist; net; net = net->next)
    for (cportref = net->terminals; cportref; cportref = cportref->next)
    {
	if (!cportref->cirinst) /* Reference to grandpa's terminal */
	    continue;
	if (!(c = cportref->cirinst->circuit)->flag.p)
	    continue; /* not a copy */

	if (c->linkcnt != 1) {
	    fprintf (stderr, "grandpa_meets_grandchildren: copied circuit (%s(%s(%s))) has linkcnt != 1\n\n",
		c->name, c->function->name, c->function->library->name);
	    dumpcore ();
	}

	for (cp = cportref->cirport; ; cp = ((CIRPORTPTR)cp->name))
	    if (c == cp->net->circuit) break; /* select the correct copy */
	cportref->cirport = cp;   /* UNHACK */
    }

    /* Now restore the names of the children's cirports. */
    for (chldinst = grandpa->cirinst; chldinst; chldinst = chldinst->next)
    {
	if (!(c = chldinst->circuit)->flag.p) continue; /* not a copy */

	if ((linkcnt = c->linkcnt) != 1) {
	    fprintf (stderr, "grandpa_meets_grandchildren: copied circuit (%s(%s(%s))) has linkcnt=%ld (should be 1)\n\n",
		c->name, c->function->name, c->function->library->name, linkcnt);
	    dumpcore ();
	}

	if (!c->next || c->name != c->next->name) /* last copy in the chain */
	for (; ; c = prevc) {
	    if (!(prevc = (CIRCUITPTR)c->flag.p)) break;
	    for (prevcp = prevc->cirport; prevcp; prevcp = prevcp->next)
		prevcp->name = ((CIRPORTPTR)prevcp->name)->name; /* UNHACK */
	}
    }

    /* And now, at last, we can start integrating the nets
     * of grandpa's children into grandpa's own netlist.
     */
    for (onet = net = grandpa->netlist; net; net = (onet = net)->next)
    {
	nets_are_connected_thru_switchbox = 0;
	for (oldcpr = cportref = net->terminals; cportref; )
	{
	    if (!cportref->cirinst) {
		cportref = (oldcpr = cportref)->next;
		continue; /* skip terminals to outer space... */
	    }

	    /* Check to see if subst_net_for_cpref() has already met this cirport */
	    if (!(cirport = cportref->cirport)->name) /* !!! switchbox !!! */
	    {
		nets_are_connected_thru_switchbox = TRUE;
		connectnet = cirport->net; /* UNHACK, hacked by subst_net_for_cpref() */
		removecpr = cportref;
		if (oldcpr == cportref) /* cportref is first port in net */
		    oldcpr = cportref = net->terminals = cportref->next;
		else
		    oldcpr->next = cportref = cportref->next;
		FreeCirportref (removecpr);
		net->num_term -= 1;
		continue;
	    }

	    if (!cirport->net || cirport->net->num_term <= 1) {
		/* Terminal with no net of child that contains this terminal. */
		if (!(c = cportref->cirinst->circuit)->cirinst  /* no grandchildren... */ ||
		    (libprim && c->status && issubstring (c->status->program, libprim_kwd)))
		{
		    cportref = (oldcpr = cportref)->next;
		    continue;     /* ...must be primitive circuit (nenh,penh,inverter,...) */
		}
		else {
		    /* This is propably a dummy terminal at cportref->cirinst->circuit.
		     * Since this circuit is going to disappear, the dummy terminal
		     * must also be removed from grandpa's net.
		     */
		    removecpr = cportref;
		    if (oldcpr == cportref) /* cportref is first port in net */
			oldcpr = cportref = net->terminals = cportref->next;
		    else
			oldcpr->next = cportref = cportref->next;
		    FreeCirportref (removecpr);
		    net->num_term -= 1;
		    continue;
		}
	    }
	    /* Net is continued in the instance. Join the
	     * two nets and remove their common terminal.
	     */
	    subst_net_for_cpref (&cportref, &oldcpr, net, cirport->net);
	}

	if (nets_are_connected_thru_switchbox)
	{
	    /* OK. So we must somehow join grandpa's current net with another
	     * net of grandpa, namely the one identified by connectnet. <UNHACK>
	     */
	    fprintf (stderr, "\ngrandpa_meets_grandchildren: switchbox detected in (%s(%s(%s))). Sorry, cannot handle (net %s).\n\n",
		grandpa->name, grandpa->function->name, grandpa->function->library->name, connectnet->name);
	    sdfexit (1);
	    /* onet->next = net->next;
	    Unlink current net. Note that always onet != net. */
	}
    }

    /* At this point all nets that needed to be joined have been joined indeed.
     * That leaves us with two tasks. Firstly, the nets of grandpa's children
     * that had no common terminal with any of grandpa's nets must be added to
     * grandpa's netlist (`lifted'). Secondly, the grandchildren must be turned
     * into children, thereby deleting the grandchildren's parents.
     */
    for (ochld = chldinst = grandpa->cirinst; chldinst; )
    {
	if (!(c = chldinst->circuit)->cirinst  /* no grandchildren... */ ||
	    (libprim && c->status && issubstring (c->status->program, libprim_kwd)))
	{
	    chldinst = (ochld = chldinst)->next;
	    continue;
	}

	for (onet = net = chldinst->circuit->netlist; net; )
	{
	    if (net->num_term <= 1 || !net->terminals) {
		net = (onet = net)->next;
		continue;
	    }

	    /* Here we probably got a net to lift. The following remove procedure
	     * is necessary in case net still contains references to a terminal
	     * of chldinst (For example, an output of chldinst which grandpa
	     * did not use). Since we're gonna get rid of chldinst very soon,
	     * a remaining reference to one of its terminals would be disastrous.
	     */
	    remove_boarder_terminals (net);

	    if (net->num_term <= 1) continue; /* nothing left to lift... */

	    chldnet = net;
	    if (onet == net)      /* First net in the netlist */
		onet = net = chldinst->circuit->netlist = net->next;
	    else
		onet->next = net = net->next;
	    chldnet->name = newname (chldnet->name, chldinst->name); /* compose new name */
	    chldnet->circuit = grandpa;
	    chldnet->next = grandpa->netlist;
	    grandpa->netlist = chldnet; /* link chldnet in front of grandpa's netlist */
	}

	removechld = chldinst;
	if (chldinst == ochld)     /* First child in the instance list */
	    grandpa->cirinst = ochld = chldinst = removechld->circuit->cirinst;
	else
	    ochld->next = chldinst = removechld->circuit->cirinst;
	removechld->circuit->cirinst = NULL; /* take away the grandchildren */

	for (; chldinst; chldinst = (ochld = chldinst)->next)
	{
	    chldinst->name = newname (chldinst->name, removechld->name); /* give new name */
	    chldinst->curcirc = grandpa; /* grandchildren get a new parent */
	}
	ochld->next = chldinst = removechld->next;
	freetreecirinst (removechld); /* throw away the whole tree */
    }
}

/* Removes cportref from the net `net'. Then inserts the net `newnet'
 * at the place where cportref used to be. Oldcpr must point to the
 * cirportref immediately preceeding cportref. If cportref is the
 * first terminal on `net' then oldcpr must be equal to cportref.
 * If newnet contains a reference to the same terminal that cportref
 * referred to, this reference is removed from newnet.
 */
PRIVATE void subst_net_for_cpref (CIRPORTREFPTR *cportref, CIRPORTREFPTR *oldcpr,
                   NETPTR net, NETPTR newnet)
{
    CIRPORTREFPTR nextcpr, cpr, ocpr, removecpr, tmpcpr;
    CIRPORTPTR    commoncp, cp;

    removecpr = cpr = (*cportref); ocpr = (*oldcpr);
    commoncp = cpr->cirport; nextcpr = cpr->next;

    if (cpr == ocpr) /* cpr is first term in net */
	net->terminals = ocpr = cpr = newnet->terminals;
    else
	ocpr->next = cpr = newnet->terminals;

    FreeCirportref (removecpr);
    net->num_term += (newnet->num_term - 1);

    if (newnet && newnet->num_term > 0)
    {
	/* Walk down the newnet and remove the common terminal, if it exists */
	while (cpr)
	{
	    cpr->net = net;    /* cpr is now part of another net */
	    if (!cpr->cirinst) /* this cpr is a connection with grandpa */
	    {
		if ((cp = cpr->cirport) != commoncp) /* ...so we have a switchbox... */
		{
		    forgetstring (cp->name); /* mark this cirport by removing its name <HACK> */
		    cp->name = NULL; /* Next time we see grandpa referring to this cp... */
		    cp->net = net;   /*      ...we know that we have to insert this net. */
		}
		tmpcpr = cpr->next;
		if (ocpr == cpr) /* cpr is first terminal of net */
		    net->terminals = tmpcpr;
		else
		    ocpr->next = tmpcpr;
		FreeCirportref (cpr);
		net->num_term -= 1;
		cpr = tmpcpr;
	    }
	    else
		cpr = (ocpr = cpr)->next;
	}
	newnet->num_term = 0;
	newnet->terminals = NULL;
	cpr = ocpr->next = nextcpr;    /* Thus newnet is linked into net */
    }
    *cportref = cpr; *oldcpr = ocpr;
}

/* Remove all the cirportref in a net that refer to a terminal of cirinst.
 */
PRIVATE void remove_boarder_terminals (NETPTR net)
{
    CIRPORTREFPTR cportref, oldcpr, removecpr;

    if (!net) return;

    for (oldcpr = cportref = net->terminals; cportref; )
	if (!cportref->cirinst) {
	    removecpr = cportref;
	    if (oldcpr == cportref) /* cportref is first port in net */
		oldcpr = cportref = net->terminals = cportref->next;
	    else
		oldcpr->next = cportref = cportref->next;
	    FreeCirportref (removecpr);
	    net->num_term -= 1;
	}
	else
	    cportref = (oldcpr = cportref)->next;
}

#define MAXSTR 256

PRIVATE STRING newname (STRING oldname, STRING extension)
{
    char tmpstr[MAXSTR+1];
    int len = strlen (oldname) + strlen (extension) + 1;

    if (len > MAXSTR) sdfreport (Fatal, "too long newname");

    sprintf (tmpstr, "%s.%s", oldname, extension);
    forgetstring (oldname);

    return (canonicstring (tmpstr));
}

void freetreecirinst (CIRINSTPTR cirinst)
{
    if (cirinst) {
	freetreecircuit (cirinst->circuit);
	FreeCirinst (cirinst);
    }
}

void freetreecircuit (CIRCUITPTR circuit)
{
    CIRPORTPTR    cp, ocp;
    NETPTR        net, onet;
    CIRPORTREFPTR cpr, ocpr;
    CIRINSTPTR    ci, oci;
    CIRCUITPTR    c, oldc;

    if (!circuit) return;

    for (cp = circuit->cirport; cp; cp = ocp->next) {
	ocp = cp;
	FreeCirport (cp);
    }

    for (net = circuit->netlist; net; net = onet->next) {
	for (cpr = net->terminals; cpr; cpr = ocpr->next) {
	    ocpr = cpr;
	    FreeCirportref (cpr);
	}
	onet = net;
	FreeNet (net);
    }

    /* temporarely do not remove the layout... */
    for (ci = circuit->cirinst; ci; ci = oci->next) {
	oci = ci;
	freetreecirinst (ci);
    }

    if (circuit->status) FreeStatus (circuit->status);

    /* unlink circuit from list of function implementations */
    for (oldc = c = circuit->function->circuit; c && c != circuit; c = (oldc = c)->next) ;

    if (!c) {
	fprintf (stderr, "freetreecircuit: tried to remove circuit (%s(%s(%s))) that function did not know about !\n",
	    circuit->name, circuit->function->name, circuit->function->library->name);
	dumpcore ();
    }

    if (oldc == c)        /* circuit id first in implementation list */
	circuit->function->circuit = c->next;
    else
	oldc->next = c->next;
    FreeCircuit (circuit);
}

CIRCUITPTR copycircuit (CIRCUITPTR circuit)
{
    CIRPORTPTR    cirport, cp, cplist = NULL;
    NETPTR        net, onet, netlist = NULL;
    CIRPORTREFPTR ocpr, cpr, cprlist = NULL;
    CIRCUITPTR    newcircuit;
    CIRINSTPTR    cirinst, newci, cilist = NULL;
    int           main_term_seen;

    if (!circuit) return (NULL);

    NewCircuit (newcircuit);
    newcircuit->name = canonicstring (circuit->name);
    newcircuit->layout = circuit->layout;
    newcircuit->function = circuit->function;
    newcircuit->linkcnt = 1;
    newcircuit->next = NULL;     /* I don't think this is all right... */

    if (circuit->status)
    {
	NewStatus (newcircuit->status);
	newcircuit->status->program = canonicstring ("Arctic Sardine Delivery");
	newcircuit->status->timestamp = 0;
	/* sdftimecvt (0, 0, 0, 0, 0, 0) */
    }

    for (cirinst = circuit->cirinst; cirinst; cirinst = cirinst->next)
    {
	NewCirinst (newci);
	newci->name = canonicstring (cirinst->name);
	cirinst->name = (STRING)newci;   /* HACK */
	newci->curcirc = newcircuit;
	newci->circuit = cirinst->circuit;
	newci->next = cilist;
	cilist = newci;
    }

    for (cirport = circuit->cirport; cirport; cirport = cirport->next)
    {
	NewCirport (cp);
	cp->name = canonicstring (cirport->name);
	cp->next = cplist;
	cplist = cp;
	cirport->name = (STRING)cp;    /* HACK */
	if (!cirport->net) continue;

	NewNet (net);
	cirport->net->flag.l = TRUE;
	cp->net = net;
	net->name = canonicstring (cirport->net->name);
	net->num_term = cirport->net->num_term;
	net->circuit = newcircuit;
	net->next = netlist;
	netlist = net;
	main_term_seen = 0;
	cprlist = NULL;

	for (ocpr = cirport->net->terminals; ocpr; ocpr = ocpr->next)
	{
	    NewCirportref (cpr);
	    cpr->net = net;
	    if (!ocpr->cirinst) { /* reference to own terminal */
		if (ocpr->cirport != cirport) {
		    fprintf (stderr, "copycircuit: net \"%s\" in circuit (%s(%s(%s))) references more than one border terminal !\n\n",
			net->name, circuit->name, circuit->function->name, circuit->function->library->name);
		    dumpcore ();
		}
		else {
		    main_term_seen += 1;
		    cpr->cirinst = NULL;
		    cpr->cirport = cp;
		}
	    }
	    else { /* reference to child's terminal */
		cpr->cirinst = (CIRINSTPTR)ocpr->cirinst->name; /* HACK */
		cpr->cirport = ocpr->cirport;
	    }
	    cpr->next = cprlist;
	    cprlist = cpr;
	}

	if (main_term_seen != 1) {
	    fprintf (stderr, "copycircuit: net \"%s\" in circuit (%s(%s(%s))) references main terminal %d times !\n\n",
		net->name, circuit->name, circuit->function->name, circuit->function->library->name, main_term_seen);
	    dumpcore ();
	}
	net->terminals = cprlist;
    }

    /* Now copy the `inner' nets, the ones that have no terminal to the outside world. */
    for (onet = circuit->netlist; onet; onet = onet->next)
    {
	if (onet->flag.l) { /* net already copied */
	    onet->flag.l = 0;
	    continue;
	}
	NewNet (net);
	net->name = canonicstring (onet->name);
	net->num_term = onet->num_term;
	net->circuit = newcircuit;
	net->next = netlist;
	netlist = net;
	cprlist = NULL;

	for (ocpr = onet->terminals; ocpr; ocpr = ocpr->next)
	{
	    NewCirportref (cpr);
	    cpr->net = net;
	    if (!ocpr->cirinst) { /* reference to own terminal ? */
		fprintf (stderr, "copycircuit: net \"%s\" in circuit (%s(%s(%s))) illegally references border terminal \"%s\" !\n\n",
		    net->name, circuit->name, circuit->function->name, circuit->function->library->name, ocpr->cirport->name);
		dumpcore ();
	    }
	    else { /* reference to child's terminal */
		cpr->cirinst = (CIRINSTPTR)ocpr->cirinst->name; /* HACK */
		cpr->cirport = ocpr->cirport;
	    }
	    cpr->next = cprlist;
	    cprlist = cpr;
	}
	net->terminals = cprlist;
    }

    for (cirinst = circuit->cirinst; cirinst; cirinst = cirinst->next)
	cirinst->name = ((CIRINSTPTR)cirinst->name)->name; /* UNHACK */
    newcircuit->cirport = cplist;
    newcircuit->cirinst = cilist;
    newcircuit->netlist = netlist;
    return (newcircuit);
}

/* Set to NULL circuit->flag.p of the circuit and recursively all its descendants.
 */
PRIVATE void treeclearcircuits (CIRCUITPTR circuit)
{
    CIRINSTPTR ci;

    circuit->flag.p = NULL;
    for (ci = circuit->cirinst; ci; ci = ci->next) treeclearcircuits (ci->circuit);
}
