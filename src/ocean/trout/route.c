/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Patrick Groeneveld
 *	Arjan van Genderen
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

#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include "src/ocean/trout/typedef.h"
#include "src/ocean/trout/grid.h"

static long count_length (GRIDPOINTPTR path);
static int do_route_nets (LAYOUTPTR father, int passno, int no_territories, BOXPTR Rbbx);
static GRIDPOINTPTR find_border (GRIDPOINTPTR path, BOXPTR wire_bbx);
static int looks_like_no_improvement (LAYOUTPTR father);
static void print_routing_statistics (LAYOUTPTR father, clock_t start_tick, struct tms *start_times);
static void re_initialize_everything (LAYOUTPTR father);
static void reverse_terminals (NETPTR net);
static int  route_pass (LAYOUTPTR father, NETPTR net, BOXPTR wire_bbx, int passno);
static void route_segment2 (NETPTR net);
static int  route_single_net (LAYOUTPTR father, NETPTR net, BOXPTR Rbbx, int passno, int nr);
static int  route_to_border (CIRPORTREFPTR term, BOXPTR wire_bbx);
static void set_netcost2 (NETPTR net);
static NETPTR sort_netlist2 (NETPTR netlist);
static void sort_terminals (NETPTR net);

/*
 * import
 */
extern COREUNIT
   Pat_mask[HERE+1],	/* look-up table for bit-patterns */
   ***Grid;		/* the working grid */
extern BOXPTR Bbx;	/* bounding box of working grid */
extern int verbose;
extern long
   Chip_num_layer,         /* number of metal layers to be used */
   Alarm_flag,             /* TRUE if alarm was set to stop routing */
   New_scheduling,         /* TRUE to do segment oriented scheduling */
   No_power_route,         /* TRUE to skip power nets */
   Verify_only,            /* TRUE to perform only wire checking */
   RouteToBorder,          /* TRUE to connect parent terminals to the border */
   GridRepitition[2],
   clk_tck;
extern GRIDADRESSUNIT
   Xoff[HERE+1],           /* look-up tables for offset values */
   Yoff[HERE+1],
   Zoff[HERE+1];
extern char *ThisImage;    /* Seadif name of this image */
extern NETPTR Vssnet, Vddnet; /* pointers to the power and ground net */

/*
 * define for rnet->type
 */
#define SIGNAL 0
#define CLOCK  1
#define POWER  2

/* abort retrying routing after  MAX_RUN_TIME seconds */
#define MAX_RUN_TIME 240     /* 5 miuntes elapsed time */
/* print a message between at least TIME_INTERVAL seconds */
#define TICK_INTERVAL 5
/* print a message between at least NET_INTERVAL processed nets */
#define NET_INTERVAL 2

/*
 * local
 */
static long Total_wire_length; /* for routing statistics */
static int
   Not_to_border,     /* number of nets wich could not be connected to border */
   Column,
   Num_net,
   Num_routed,
   Total_two_term,    /* total number of two-terminal nets */
   Routed_two_term;   /* number of two-terminal nets routed */

static LAYLABELPTR newlaylabel;

static void freeLabels ();

/*
 * This is the master routing for routing nets.
 */
int route_nets (LAYOUTPTR father, int enable_retry, BOXPTR Rbbx)
/* enable_retry - TRUE to enable re-routing */
/* Rbbx - Routing boundingbox */
{
    NETPTR hnet;
    clock_t curr_tick, start_tick;
    struct tms curr_times, start_times;
    int passno, no_territories, num_unrouted, num_unrouted_no_territories;

    /* count number of nets */
    Num_net = 0;
    for (hnet = father->circuit->netlist; hnet; hnet = hnet->next) {
	if (((R_NETPTR) hnet->flag.p)->routed == FALSE) Num_net++;
    }

    if (verbose) { printf ("%d nets have to be routed.\n", Num_net); fflush (stdout); }

    /* determine the net types of all nets */
    guess_net_type (father->circuit->netlist);

    freeLabels (father);

    /* START routing */

    start_tick = times (&start_times);

    /* First pass
    */
    if (enable_retry == FALSE)
	passno = 10; /* no second pass anyway... */
    else
	passno = 1;

    no_territories = TRUE;
    if ((num_unrouted = do_route_nets (father, passno, no_territories, Rbbx)) == 0)
    { /* successful in 1st pass */
	print_routing_statistics (father, start_tick, &start_times);
	return (num_unrouted);
    }

    /* attempt a second pass? (at most 30% should have failed)
    */
    curr_tick = times (&curr_times);
    if (enable_retry == FALSE || (enable_retry == TRUE &&
	(num_unrouted > 15 || (double)num_unrouted / Num_net > 0.30
		|| (curr_tick - start_tick)/clk_tck > MAX_RUN_TIME)))
    { /* no, too bad, just leave it */
	print_routing_statistics (father, start_tick, &start_times);
	return (num_unrouted);
    }

    /* pass 2: with new cost estimates based on previous failure, and with territories
    */
    passno = 2;
    no_territories = FALSE;
    num_unrouted_no_territories = num_unrouted;
    if (verbose) { printf ("----- Don't worry, I'm trying again -----\n"); fflush (stdout); }
    re_initialize_everything (father);

    if ((num_unrouted = do_route_nets (father, passno, no_territories, Rbbx)) == 0)
    { /* successful */
	print_routing_statistics (father, start_tick, &start_times);
	return (num_unrouted);
    }

    /* continue?
    */
    curr_tick = times (&curr_times);
    if (enable_retry != TRUE+1 && (curr_tick - start_tick)/clk_tck > MAX_RUN_TIME)
    { /* no, too bad, just leave it */
	print_routing_statistics (father, start_tick, &start_times);
	return (num_unrouted);
    }

    /* evaluate: use territories or not?
    */
    if (num_unrouted >= num_unrouted_no_territories)
	no_territories = TRUE; /* better don't, no improvement */
    else
	no_territories = FALSE; /* yes: improvement */

    for (passno = 3; passno < 6; passno++)
    {
	/* check time consumption */
	curr_tick = times (&curr_times);
	if ((curr_tick - start_tick)/clk_tck > MAX_RUN_TIME) break;

	if (verbose) { printf ("----- Don't worry, I'm trying again -----\n"); fflush (stdout); }
	re_initialize_everything (father);

	/* pass 3-6: with new cost estimates based on previous failure (stop at first failure)
	*/
	if ((num_unrouted = do_route_nets (father, passno, no_territories, Rbbx)) == 0)
	{ /* successful */
	    print_routing_statistics (father, start_tick, &start_times);
	    return (num_unrouted);
	}

	/* stop if we keep bunpin' against the same net
	*/
	if (looks_like_no_improvement (father) == TRUE) break; /* stop it, no use */

    }

    if (verbose) { printf ("----- Ooops! almost looks like its not gonna work -----\n"); fflush (stdout); }

    /* pass 10: just do it entirely now
    */
    passno = 10;
    re_initialize_everything (father);

    if ((num_unrouted = do_route_nets (father, passno, no_territories, Rbbx)) == 0) { /* successful */ }

    print_routing_statistics (father, start_tick, &start_times);
    return (num_unrouted);
}

/*
 * The major route routine.
 * This routine routes the nets using lee.
 */
static int do_route_nets (LAYOUTPTR father, int passno, int no_territories, BOXPTR Rbbx)
/* Rbbx - Routing boundingbox */
{
    NETPTR hnet;
    int count;
    double elapsed, expect, fraction;
    clock_t start_tick, last_tick, curr_tick;
    struct tms end_times;

    /* add territories to threatened terminals
    */
    if (strcmp (ThisImage, "fishbone") == 0) {
	/* threatened terminals */
	make_piefjes (father);

	if (passno > 1) {
	    /* real territories, which removes piefjes again */
	    make_real_territories (father, no_territories);
	    /* and add piefjes to remaining places */
	    make_piefjes (father);
	}
    }

    Not_to_border = 0;

    /* sort the netlist according to heuristics */
    father->circuit->netlist = sort_netlist (father->circuit->netlist, Rbbx);

    /* init statistics */
    Total_wire_length = 0;
    Total_two_term = Routed_two_term = 0;
    last_tick = start_tick = times (&end_times);

    /* THE BIG ROUTING LOOP */
    Num_routed = 0;
    count = 0;

    /* printf ("start_tick = %d\n", (int) start_tick); */

    for (hnet = father->circuit->netlist; hnet; hnet = hnet -> next)
    {
	curr_tick = times (&end_times); /* keep track of time statistics */

	/* printf ("curr_tick = %d\n", (int) curr_tick); */
	/* print message with estimate */
	if ((curr_tick - last_tick)/clk_tck > TICK_INTERVAL &&
		count%NET_INTERVAL == 0 && verbose && Num_net > 0)
	{
	    elapsed = curr_tick - start_tick;
	    fraction = (double)count / Num_net;
	    expect = elapsed / fraction;
	    expect /= clk_tck; /* in seconds */
	    /* correct for non-linear distribution */
	    expect = expect * (2 - fraction);
	    /* subtract already spent time */
	    elapsed /= clk_tck;
	    expect = expect - elapsed;
	    printf ("--> %4.2f %% elapsed: %5.1f sec. expect another %5.1f sec. <--\n",
			fraction * 100, elapsed, expect);
	    fflush (stdout);
	    last_tick = curr_tick;
	}

	/* route the net
	*/
	count++;
	if (((R_NETPTR) hnet->flag.p)->routed == FALSE) {
	    if (route_single_net (father, hnet, Rbbx, passno, count) == TRUE) Num_routed++;
	}

	if (Alarm_flag == TRUE) {
	    fprintf (stderr, "WARNING: incomplete routing due to SIGALRM\n");
	    break;
	}

	/* stop at first failure in passes 3 - 9
	if (passno > 2 && passno < 10)
	    if (Num_routed - Not_to_border < count) break;
	*/
    }

    remove_territories (father); /* remove any remaining territories */

    /* return the number of unrouted nets */
    if (Num_routed < Num_net && Num_net > 0) return (Num_net - Num_routed + Not_to_border);
    return (Not_to_border);
}

/*
 * This routine prints the routing statistics.
 */
static void print_routing_statistics (LAYOUTPTR father, clock_t start_tick, struct tms *start_times)
{
    struct tms end_times;
    clock_t curr_tick;
    double elapsed, fraction;

    printf ("READY\n----- Some interesting statistics ------\n");

    if (Num_routed < Num_net && Num_net > 0) {
	printf ("Number of nets successfully routed:   %d out of %d (%4.2f%%)\n",
	    Num_routed, Num_net, (double)Num_routed / Num_net * 100);
    } else {
	printf ("Total number of nets:                 %d (100%% completion)\n", Num_net);
    }

    if (Total_wire_length > 0) {
	printf ("Total length of routed wires:         %ld grids = %4.3f mm\n",
	    Total_wire_length, (double)Total_wire_length * 0.0088);
    } else {
	printf ("Total length of routed wires:         0 (No new wires created)\n");
    }

    /* print area utilisation statistics */
    make_statistics (father, FALSE);

    /* time statistics */
    curr_tick = times (&end_times);

    fraction = end_times.tms_utime - start_times->tms_utime;
    elapsed  = end_times.tms_stime - start_times->tms_stime;
    printf ("CPU-time consumption:               %6.2f sec.\n", fraction / clk_tck);
    /*
    printf ("CPU-time consumption (user/system): %6.2f /%6.2f sec.\n", fraction / clk_tck, elapsed / clk_tck);
    */

    curr_tick -= start_tick;
    if (curr_tick > 0) {
	fraction = (fraction + elapsed) / curr_tick;
    }
    else fraction = 1;

    elapsed = curr_tick; elapsed /= clk_tck;
    printf ("Elapsed time during routing:        %6.2f sec. (%4.2f %% of cpu)\n", elapsed, fraction * 100);

    if (Not_to_border > 0) {
	printf ("Warning: %d net(s) were not connectable to the border.\n", Not_to_border);
    }
    if (Num_routed < Num_net && Num_net > 0) {
	printf ("WARNING:                              INCOMPLETE ROUTING!\n");
    }
    fflush (stdout);
}

/*
 * This routine sorts the netlist according to the size of the bounding
 * box, The shortest (lowest cost-) nets are routed first
 * The routine will return a sorted netlist attached in the r_net struct.
 */
NETPTR sort_netlist (NETPTR netlist, BOXPTR Routingbbx)
{
    NETPTR sort_list, max_net, max_prev, previous;
    register NETPTR hnet;
    CIRPORTREFPTR hportref;
    R_PORTPTR rport;
    BOX net_box; /* stores bounding box of net */
    BOXPTR Rbbx;
    float max_cost;
    R_NETPTR hnetp;

    max_net = NULL; /* suppres uninitialized warning */

    Rbbx = !Routingbbx ? Bbx : Routingbbx;

    for (hnet = netlist; hnet; hnet = hnet->next)
    {
	/* set cost to large number, which flags disabled
	 * (used to be able to use continue)
	 */
	hnetp = (R_NETPTR) hnet->flag.p;
	hnetp->cost = BIGNUMBER;

	/* walk along terminals */
	if (!hnet->terminals || hnet->num_term < 2) continue;

	rport = NULL; /* suppres uninitialized warning */

	/* init box */
	for (hportref = hnet->terminals; hportref; hportref = hportref->next) {
	    if (!(rport = (R_PORTPTR) hportref->flag.p)) continue;
	    if (rport->unassigned == FALSE) break;
	}

	if (!hportref || !rport) {
	    /* No unassigned pins for this net */
	    continue;  /* nothing useful found */
	}

	net_box.crd[L] = net_box.crd[R] = rport->crd[X];
	net_box.crd[B] = net_box.crd[T] = rport->crd[Y];
	net_box.crd[D] = net_box.crd[U] = rport->crd[Z];

	for (hportref = hnet->terminals; hportref; hportref = hportref->next)
	{
	    if (!(rport = (R_PORTPTR) hportref->flag.p)) continue; /* no struct: skip */
	    if (rport->unassigned == TRUE)
	    { /* unassigned on father: position is dummy
		 make estimate of closest point on border */
		int i, mindist, minindex;
		for (i = 0, mindist = BIGNUMBER, minindex = 0; i < 4; i++)
		    if (ABS (net_box.crd[i] - Rbbx->crd[i]) < mindist) {
			mindist = ABS (net_box.crd[i] - Rbbx->crd[i]);
			minindex = i;
		    }
		if (i == L || i == B) {
		    MIN_UPDATE (net_box.crd[i], Rbbx->crd[i]); }
		else
		    MAX_UPDATE (net_box.crd[i], Rbbx->crd[i]);
		if (i == L || i == R) { /* make wider in y-dir */
		    MIN_UPDATE (net_box.crd[B], rport->crd[Y] - 5);
		    MAX_UPDATE (net_box.crd[T], rport->crd[Y] + 5);
		}
		else { /* make wider in x-dir */
		    MIN_UPDATE (net_box.crd[L], rport->crd[X] - 5);
		    MAX_UPDATE (net_box.crd[R], rport->crd[X] + 5);
		}
	    }
	    else { /* assigned terminal */
		MIN_UPDATE (net_box.crd[L], rport->crd[X]);
		MAX_UPDATE (net_box.crd[R], rport->crd[X]);
		MIN_UPDATE (net_box.crd[B], rport->crd[Y]);
		MAX_UPDATE (net_box.crd[T], rport->crd[Y]);
	    }
	    MIN_UPDATE (net_box.crd[D], rport->crd[Z]);
	    MAX_UPDATE (net_box.crd[U], rport->crd[Z]);
	}

	/* truncate on routing bbx */
	MAX_UPDATE (net_box.crd[L], Rbbx->crd[L]);
	MIN_UPDATE (net_box.crd[R], Rbbx->crd[R]);
	MAX_UPDATE (net_box.crd[B], Rbbx->crd[B]);
	MIN_UPDATE (net_box.crd[T], Rbbx->crd[T]);

	/* cost heuristic: manhattan distance */
	hnetp->cost = 100 * ((net_box.crd[R] - net_box.crd[L]) + (net_box.crd[T] - net_box.crd[B]));

	/* nets with more terminals have lower cost */
	hnetp->cost *= (0.5 + (1 / hnet->num_term));

	/* give strong bias (costreduction) to nets which failed before */
	if (hnetp->fail_count > 0) hnetp->cost /= (20 * hnetp->fail_count);

	/* give a strong costreduction to nets of the clock type */
	if (hnetp->type == CLOCK) hnetp->cost /= 20;
    }

    /* make sorted list, lowest cost will be first in list
    */
    sort_list = NULL;
    while (netlist) {
	max_cost = -BIGNUMBER;
	max_prev = previous = NULL;
	for (hnet = netlist; hnet; hnet = hnet->next) {
	    hnetp = (R_NETPTR) hnet->flag.p;
	    if (hnetp->cost > max_cost) { max_cost = hnetp->cost; max_net = hnet; max_prev = previous; }
	    previous = hnet;
	}
	if (max_prev == NULL)
	    netlist = netlist->next; /* first in list (max_cost == netlist) */
	else
	    max_prev->next = max_net->next;

	max_net->next = sort_list;
	sort_list = max_net;
    }

    return (sort_list);
}

/*
 * routes a single net, returns result
 */
static int route_single_net (LAYOUTPTR father, NETPTR net, BOXPTR Rbbx, int passno, int nr)
/* Rbbx   - Routing boundingbox */
/* passno - severity of try: if < 10, failed nets are not completed */
{
    GRIDADRESSUNIT x, y, z; /* grid coordinates */
    int num_to_route, num_routed;
    BOX wire_bbx;
    CIRPORTREFPTR hterm;
    R_NETPTR netp;

    if (verbose) printf ("Routing net(%d) '%s'", nr, net->name);

    netp = (R_NETPTR) net->flag.p;

    netp -> routing_attempts++; /* register attempt to route */

    if (net->num_term < 2) {
	if (verbose) { printf (" : Trivial\n"); fflush (stdout); }
	netp -> routed = TRUE;
	return (TRUE);
    }

    /* set the boundaries of the routing bbx */
    if (Rbbx) { /* routing bbx was set */
	wire_bbx.crd[L] = Rbbx->crd[L]; wire_bbx.crd[R] = Rbbx->crd[R];
	wire_bbx.crd[B] = Rbbx->crd[B]; wire_bbx.crd[T] = Rbbx->crd[T];
	wire_bbx.crd[D] = Rbbx->crd[D]; wire_bbx.crd[U] = Rbbx->crd[U];
    }
    else { /* default entire grid */
	wire_bbx.crd[L] = Bbx->crd[L]; wire_bbx.crd[R] = Bbx->crd[R];
	wire_bbx.crd[B] = Bbx->crd[B]; wire_bbx.crd[T] = Bbx->crd[T];
	wire_bbx.crd[D] = Bbx->crd[D]; wire_bbx.crd[U] = Bbx->crd[U];
    }
    wire_bbx.crd[D] = -1; /* always use tunnels */

    /* treat power nets special */
    if ((net == Vssnet || net == Vddnet)
	&& netp -> type == POWER && check_power_capabilities (FALSE) == TRUE)
    {
	if (verbose) { printf (" : "); fflush (stdout); }
	special_power_route (net, &wire_bbx);
	if (verbose) { printf (" (Power special)\n"); fflush (stdout); }
	return (netp -> routed);
    }

    /* tunnels will hardly be used anyway....
    if (netp -> type != SIGNAL) wire_bbx.crd[D] = 0;
    */

    /* sort terminals according to center of gravity */
    sort_terminals (net);

    /* set to unrouted */
    num_to_route = 0;
    for (hterm = net->terminals; hterm; hterm = hterm->next) {
	R_PORTPTR port;

	if (!(port = (R_PORTPTR) hterm->flag.p)) continue;

	/* Terminals that initially could not be assigned a position
	    because there was no instance port in the net, are now
	    assigned a position somewhere on the routing border.
	*/
	if (port->layport->layer == NEED_PLACEMENT) {
	    x = wire_bbx.crd[R];
	    y = wire_bbx.crd[T];
	    z = wire_bbx.crd[U];

	    while (!is_Free (x, y, z) && y > wire_bbx.crd[B]) y--;
	    if (!is_Free (x, y, z)) {
		while (!is_Free (x, y, z) && x > wire_bbx.crd[L]) x--;
		if (!is_Free (x, y, z)) {
		    while (!is_Free (x, y, z) && y < wire_bbx.crd[T]) y++;
		    if (!is_Free (x, y, z)) {
			while (!is_Free (x, y, z) && x < wire_bbx.crd[R]) x++;
		    }
		}
	    }

	    if (is_Free (x, y, z)) {
		set_occupied (x, y, z);

		port->layport->pos[X] = port->crd[X] = x;
		port->layport->pos[Y] = port->crd[Y] = y;
		port->layport->layer  = port->crd[Z] = z;
		port->unassigned = FALSE; /* to prevent that it will be moved by route_to_border() */
	    }
	    else {
		fprintf (stderr, "WARNING: unable to assign position to port '%s' on father '%s'\n",
					port->layport->cirport->name, father->name);
		if (port->cirportref) port->cirportref->flag.p = NULL;
		port->layport->flag.p = NULL;
	    }
	}

	/* set to unrouted */
	if (!Rbbx || term_in_bbx (hterm, &wire_bbx) == TRUE) {
	    num_to_route++;
	    port -> routed = FALSE;
	}
	else port -> routed = TRUE;
    }

    if (verbose) {
	printf (" (%d/%d): ", net->num_term, num_to_route); fflush (stdout);
    }

    if (num_to_route < 2) {
	if (verbose) { printf (" entirely outside box\n"); fflush (stdout); }
	netp -> routed = TRUE;
	return (TRUE);
    }

    /* remove territory */
    for (hterm = net->terminals; hterm; hterm = hterm->next) {
	if (!hterm->flag.p) continue;
	remove_territory ((R_PORTPTR) hterm->flag.p);
    }

    /* try first pass */
    newlaylabel = NULL;
    num_routed = route_pass (father, net, &wire_bbx, passno);

    /* try second pass if failure */
    if (num_routed < num_to_route - 2 /* && passno >= 10 */)
    { /* not all nets routed, at least two other nets were unrouted */

	if (verbose) { printf ("R>"); fflush (stdout); }

	/* reverse the netlist */
	reverse_terminals (net);

	/* delete label that has been created during first route pass */
	if (newlaylabel) {
	    FreeLaylabel (newlaylabel);
	    newlaylabel = NULL;
	}

	num_routed += route_pass (father, net, &wire_bbx, passno);
    }

    if (newlaylabel) {
	newlaylabel->next = father->laylabel;
	father->laylabel = newlaylabel;
    }

    /* find the terminal on parent */
    for (hterm = net->terminals; hterm && hterm->cirinst; hterm = hterm->next) /* nothing */;

    /* necessary to route to border? */
    if (hterm && RouteToBorder == TRUE && ((R_PORTPTR) hterm->flag.p)->unassigned == TRUE) {
	if (route_to_border (hterm, &wire_bbx) == FALSE) { /* failed */
	    printf ("X (not touching border)");
	    Not_to_border++;
	    /* increment fail count if no other failed */
	    if (num_routed >= num_to_route - 1) netp -> fail_count++;
	}
	else printf ("b"); /* success */
    }

    if (num_routed < num_to_route - 1) {
	if (verbose) printf (" FAILED\n");
	netp -> fail_count++;
    }
    else {
	netp -> routed = TRUE;
	if (verbose) printf (" OK!\n");
    }

    if (verbose) fflush (stdout);

    Total_two_term += num_to_route - 1;
    Routed_two_term += num_routed;

    return ((int) netp -> routed);
}

/*
 * This routine routes all unconnected terminals to the first unconnected terminal.
 */
static int route_pass (LAYOUTPTR father, NETPTR net, BOXPTR wire_bbx, int passno)
{
    int num_routed;
    CIRPORTREFPTR source_term, destination_term, help_term;
    R_PORTPTR sport, dport;
    GRIDPOINT spoint, dpoint;
    GRIDPOINTPTR path;

    /* step to first appropriate source term */
    for (source_term = net->terminals;
	source_term && source_term->flag.p && ((R_PORTPTR) source_term->flag.p)->routed == TRUE;
	source_term = source_term->next) /* nothing */ ;

    /* return if none found */
    if (!source_term || !source_term->flag.p) return (0);

    /* set it to routed */
    sport = (R_PORTPTR) source_term->flag.p;
    spoint.x = sport->crd[X];
    spoint.y = sport->crd[Y];
    spoint.z = sport->crd[Z];

    sport->routed = TRUE;

    /* We only generate a label when the label name is not equal to
	the name of a terminal of the cell itself (AvG).
    */
    for (help_term = net->terminals; help_term; help_term = help_term->next) {
	if (!help_term->cirinst && strcmp (help_term->cirport->name, net->name) == 0) break;
    }
    if (!help_term) { /* Add a label */
	NewLaylabel (newlaylabel);
	newlaylabel->name = canonicstring (net->name);
	newlaylabel->pos[X] = sport->crd[X];
	newlaylabel->pos[Y] = sport->crd[Y];
	newlaylabel->layer  = sport->crd[Z];
    }

    num_routed = 0;

    for (destination_term = source_term->next;
	    destination_term && destination_term->flag.p; destination_term = destination_term->next)
    {
	dport = (R_PORTPTR) destination_term->flag.p;

	if (dport->routed == TRUE) continue;

	dpoint.x = dport->crd[X];
	dpoint.y = dport->crd[Y];
	dpoint.z = dport->crd[Z];

	/* route source to destination */
	path = lee (&spoint, &dpoint, wire_bbx, FALSE);

	if (!path) { /* failed */
	    if (verbose) { printf ("x"); fflush (stdout); }
	    /* just return initially */
	    /* if (passno < 10) return (num_routed); */
	}
	else { /* ok: free, print path */
	    num_routed++;
	    if (verbose) { printf ("."); fflush (stdout); }
	    dport->routed = TRUE;

	    Total_wire_length += count_restore_melt_and_free_wire (path);
	    //Total_wire_length += count_length (path); /* for routing statistics */
	    //restore_wire_pattern (path);
	    /* make fatter wires if critical net: copy path into net statements */
	    /* Disabled, does not work properly yet
	    store_critical_wire (net, path); */
	    //melt_new_wire (path); /* add possible missing pointers */
	    //free_gridpoint_list (path);
	}
    }

    return (num_routed);
}

static int route_to_border (CIRPORTREFPTR term, BOXPTR wire_bbx)
{
    GRIDPOINT spoint;
    R_PORTPTR sport;
    GRIDPOINTPTR path, border;

    if (!term) {
	fprintf (stderr, "ERROR (route_to_border): null term\n");
	return (FALSE);
    }

    /* run lee router to the border */
    if (!(sport = (R_PORTPTR) term->flag.p)) return (FALSE);
    spoint.x = sport->crd[X];
    spoint.y = sport->crd[Y];
    spoint.z = sport->crd[Z];

    if (!(path = lee_to_border (&spoint, wire_bbx, FALSE))) return (FALSE);

    /* move unassigned ports to the border */
    if (sport->unassigned == TRUE) { /* the port was unassigned: move it */
	if (!(border = find_border (path, wire_bbx))) {
	    fprintf (stderr, "WARNING: cannot find border\n");
	}
	else {
	    sport->layport->pos[X] = sport->crd[X] = border->x;
	    sport->layport->pos[Y] = sport->crd[Y] = border->y;
	    sport->layport->layer  = sport->crd[Z] = border->z;
	}
    }

    Total_wire_length += count_restore_melt_and_free_wire (path);
    //Total_wire_length += count_length (path); /* for routing statistics */
    //restore_wire_pattern (path);
    //melt_new_wire (path); /* add possible missing pointers */
    //free_gridpoint_list (path);
    return (TRUE);
}

static GRIDPOINTPTR find_border (GRIDPOINTPTR path, BOXPTR wire_bbx)
{
    register GRIDPOINTPTR border;

    for (border = path; border; border = border->next) {
	if (border->x == wire_bbx->crd[L] ||
	    border->x == wire_bbx->crd[R] ||
	    border->y == wire_bbx->crd[B] ||
	    border->y == wire_bbx->crd[T]) break;
    }
    return (border);
}

/*
 * this routine sorts the netlist in an order
 */
static void sort_terminals (NETPTR net)
{
    long max_dist, center[2];
    R_PORTPTR rport;
    register CIRPORTREFPTR hterm, previous_term;
    CIRPORTREFPTR end_of_list, max_term, prev_max, sort_list;
    int cnt;

    if (net->num_term < 2) return;

    center[X] = center[Y] = NULL;
    cnt = 0;

    /* find center of gravity */
    for (hterm = net->terminals; hterm; hterm = hterm->next) {
	if (!hterm->flag.p || ((R_PORTPTR) hterm->flag.p)->crd[Z] == NEED_PLACEMENT) continue;
	center[X] += ((R_PORTPTR) hterm->flag.p)->crd[X];
	center[Y] += ((R_PORTPTR) hterm->flag.p)->crd[Y];
	cnt++;
    }

    /* num_term is the number of valid (routable) terminals */
    if (cnt > 0) {
	center[X] = center[X]/cnt;
	center[Y] = center[Y]/cnt;
    }
    else {
	center[X] = 0;
	center[Y] = 0;
    }

    /* sort terminals, according to distance from center */
    sort_list = NULL;
    end_of_list = NULL;
    while (net->terminals) { /* as long as one term found */
	max_dist = -BIGNUMBER;
	previous_term = prev_max = max_term = NULL;
	for (hterm = net->terminals; hterm; hterm = hterm->next) {
	    if (!hterm->flag.p) { previous_term = hterm; continue; }

	    rport = (R_PORTPTR) hterm->flag.p;
	    if (ABS (center[X] - rport->crd[X]) + ABS (center[Y] - rport->crd[Y]) > max_dist) {
		max_term = hterm;
		prev_max = previous_term;
		max_dist = ABS (center[X] - rport->crd[X]) + ABS (center[Y] - rport->crd[Y]);
	    }
	    previous_term = hterm;
	}

	if (max_term == NULL) break; /* stop if nothing found */

	/* remove max_term from list */
	if (max_term == net->terminals)
	    net->terminals = max_term->next;
	else
	    prev_max->next = max_term->next;

	/* insert in new list */
	if (sort_list == NULL) end_of_list = max_term;

	max_term->next = sort_list;
	sort_list = max_term;
    }

    /* append unsorted junk terminals */
    if (end_of_list)
	end_of_list->next = net->terminals;
    else
	sort_list = net->terminals;

    /* netlist now contains the nets, with the nearest terminal first */
    net->terminals = sort_list;
}

/*
 * this routine reverses the order of the terminals
 */
static void reverse_terminals (NETPTR net)
{
    CIRPORTREFPTR cport, sort_list;

    if (net->num_term < 2) return;

    sort_list = NULL;
    while (net->terminals) {
	cport = net->terminals->next;
	net->terminals->next = sort_list;
	sort_list = net->terminals;
	net->terminals = cport;
    }
    net->terminals = sort_list;
}

/*
 * This routine checks the new wire with the image in the grid.
 * It makes sure that all pointers are present.
 * This is required because lee generates a wire, but the
 * adjacent source and destination points do not.
 */
void melt_new_wire (GRIDPOINTPTR point)
/* point - startpoint of wire */
{
    GRIDADRESSUNIT x, y, z;

    for ( ; point; point = point->next) {
	if (point->cost == -1) continue; /* temp path */

	x = point->x;
	y = point->y;
	z = point->z;
	if (is_Free (x, y, z)) continue;

	/* if (missing) add opposite pattern */

	if (grid_neighbour (x, y, z, L) && x > Bbx->crd[L] &&
	   !grid_neighbour (x-1, y, z, R)) add_grid_neighbour_c (x-1, y, z, R);
	if (grid_neighbour (x, y, z, R) && x < Bbx->crd[R] &&
	   !grid_neighbour (x+1, y, z, L)) add_grid_neighbour_c (x+1, y, z, L);
	if (grid_neighbour (x, y, z, B) && y > Bbx->crd[B] &&
	   !grid_neighbour (x, y-1, z, T)) add_grid_neighbour_c (x, y-1, z, T);
	if (grid_neighbour (x, y, z, T) && y < Bbx->crd[T] &&
	   !grid_neighbour (x, y+1, z, B)) add_grid_neighbour_c (x, y+1, z, B);
	if (grid_neighbour (x, y, z, D) && z > Bbx->crd[D] &&
	   !grid_neighbour (x, y, z-1, U)) add_grid_neighbour_c (x, y, z-1, U);
	if (grid_neighbour (x, y, z, U) && z < Bbx->crd[U] &&
	   !grid_neighbour (x, y, z+1, D)) add_grid_neighbour_c (x, y, z+1, D);
    }
}

/*
 * This routine counts the length of the path.
 */
static long count_length (GRIDPOINTPTR path)
{
    GRIDPOINTPTR prev_path;
    long length = 0;

    if (!path) return (length);

    prev_path = path;
    for (path = path->next; path; path = path->next) {
	length += ABS (prev_path->x - path->x) + ABS (prev_path->y - path->y);
	prev_path = path;
    }
    return (length);
}

/*
 * intelligent scheduling per 2-term segment
 */
int do_route_nets2 (LAYOUTPTR father, int passno, BOXPTR Rbbx)
/* Rbbx - Routing boundingbox */
{
    NETPTR hnet, netlist;
    BOX wire_bbx;
    clock_t start_tick, last_tick, curr_tick;
    struct tms end_times;
    int count;
    double elapsed, expect, fraction;

    /* add territories to threatened terminals */
    if (strcmp (ThisImage, "fishbone") == 0) {
	/* threatened terminals */
	make_piefjes (father);

	if (passno > 1) {
	    /* real territories, which removes piefjes again */
	    make_real_territories (father, passno);
	    /* and add piefjes to remaining places */
	    make_piefjes (father);
	}
    }

    Not_to_border = 0;

    /* set the boundaries of the routing bbx */
    if (Rbbx) { /* routing bbx was set */
	wire_bbx.crd[L] = Rbbx->crd[L]; wire_bbx.crd[R] = Rbbx->crd[R];
	wire_bbx.crd[B] = Rbbx->crd[B]; wire_bbx.crd[T] = Rbbx->crd[T];
	wire_bbx.crd[D] = Rbbx->crd[D]; wire_bbx.crd[U] = Rbbx->crd[U];
    }
    else { /* default entire grid */
	wire_bbx.crd[L] = Bbx->crd[L]; wire_bbx.crd[R] = Bbx->crd[R];
	wire_bbx.crd[B] = Bbx->crd[B]; wire_bbx.crd[T] = Bbx->crd[T];
	wire_bbx.crd[D] = Bbx->crd[D]; wire_bbx.crd[U] = Bbx->crd[U];
    }
    wire_bbx.crd[D] = -1;

    netlist = father->circuit->netlist;

    /* count number of nets and number of 2-term segments */
    Num_net = 0;
    Total_two_term = 0;
    for (hnet = father->circuit->netlist; hnet; hnet = hnet->next) {
	if (((R_NETPTR) hnet->flag.p)->routed == FALSE) {
	    if (/* No_power_route == TRUE && */
		((R_NETPTR) hnet->flag.p)->type == POWER &&
		check_power_capabilities (FALSE) == TRUE)
	    { /* special power routing */
		special_power_route (hnet, &wire_bbx);
	    }
	    else {
		Total_two_term += (hnet->num_term - 1);
		Num_net++;
	    }
	}
    }

    /*
    if (verbose) {
	printf ("%d nets containing %d segments have to be routed.\n\n", Num_net, Total_two_term);
	fflush (stdout);
    }
    */

    /* init statistics */
    Total_wire_length = 0;
    Routed_two_term = 0;
    Column = 0;
    Num_routed = 0;

    /* set the netcost of each net */
    for (hnet = netlist; hnet; hnet = hnet->next) set_netcost2 (hnet);

    /* route everything */
    Column = 0;
    start_tick = last_tick = times (&end_times);
    printf ("ticks: %d %d\n", (int) start_tick, (int) last_tick);

    count = 0;
    while (((R_NETPTR) (netlist = sort_netlist2 (netlist))->flag.p)->routed == FALSE)
    {
	route_segment2 (netlist);
	set_netcost2 (netlist);

	count++;
	curr_tick = times (&end_times);
	printf ("ticks: %d\n", (int) curr_tick);
	/* print message with estimate */
	if ((curr_tick - last_tick)/clk_tck > TICK_INTERVAL &&
		verbose && count >= 5 && Num_net > 0)
	{
	    elapsed = curr_tick - start_tick;
	    fraction = (double)count / Total_two_term;
	    expect = elapsed / fraction;
	    expect /= clk_tck; /* in seconds */
	    /* correct for non-linear distribution */
	    expect = expect * (2 - fraction);
	    /* subtract already spent time */
	    elapsed /= clk_tck;
	    expect = expect - elapsed;
	    printf ("\n--> %4.2f %% elapsed: %5.1f sec. expect another %5.1f sec. <--\n",
			fraction * 100, elapsed, expect);
	    fflush (stdout);
	    last_tick = curr_tick;
	}

	if (Alarm_flag == TRUE) {
	    fprintf (stderr, "WARNING: incomplete routing due to SIGALRM\n");
	    break;
	}
    }

    father->circuit->netlist = netlist;

    remove_territories (father);

    /* return the number of unrouted nets */
    if (Num_routed < Num_net && Num_net > 0) return (Num_net - Num_routed);
    return (0);
}

/*
 * This routine sorts the netlist according the the
 * shortest distance between two unconnected terminals.
 */
static NETPTR sort_netlist2 (NETPTR netlist)
{
    NETPTR sort_list, max_net, max_prev, previous;
    register NETPTR hnet;
    long max_cost;

    max_net = NULL; /* suppres uninitialized warning */

    /* make sorted list, lowest cost will be first in list */
    sort_list = NULL;
    while (netlist)
    {
	max_cost = -BIGNUMBER;
	max_prev = previous = NULL;
	for (hnet = netlist; hnet; hnet = hnet->next) {
	    if (((R_NETPTR) hnet->flag.p)->cost > max_cost) {
		max_cost = ((R_NETPTR) hnet->flag.p)->cost; max_net = hnet; max_prev = previous;
	    }
	    previous = hnet;
	}
	if (max_prev == NULL)
	    netlist = netlist->next; /* first in list (max_cost == netlist) */
	else
	    max_prev->next = max_net->next;

	max_net->next = sort_list;
	sort_list = max_net;
    }

    return (sort_list);
}

/*
 * this routine sets the cost of a net according to the distance
 * of the lowest cost net
 * it sets rnet->routed to the approriate value
 */
static void set_netcost2 (NETPTR net)
/* net - the net of which the cost have to be (re-)evaluated */
{
    R_NETPTR rnet;
    R_PORTPTR sport, dport;
    CIRPORTREFPTR hportref, tportref;
    int x, y;

    if (!(rnet = (R_NETPTR) net->flag.p)) return;

    if (rnet->cost_mtx == NULL)
    { /* first time: make it */

	if (!(rnet->cost_mtx  = (int **) calloc(net->num_term, sizeof(int *))))
	    error (FATAL_ERROR, "calloc for cost_mtx failed");

	for (x = 0; x != net->num_term; x++) {
	    /* allocate y */
	    CALLOC (rnet->cost_mtx[x], int, net->num_term);
	    for (y=0; y != net->num_term; y++) rnet->cost_mtx[x][y] = -1;
	}

	/* walk along terminals */
	for (hportref = net->terminals, x=0; hportref; x++, hportref = hportref->next)
	{
	    if (!(sport = (R_PORTPTR) hportref->flag.p)) continue;

	    /* first cost with rest of terminals */
	    for (tportref = hportref->next, y=0; tportref; y++, tportref = tportref->next)
	    {
		if (!(dport = (R_PORTPTR) tportref->flag.p)) continue;

		/* calc manhattan distance */
		rnet->cost_mtx[x][y] =
		    ABS (sport->crd[X] - dport->crd[X]) +
		    ABS (sport->crd[Y] - dport->crd[Y]);

		/* heuristic: increase cost if both terminal(groups) were already
		connected once. This is to make sure that all terminals
		are routed as soon as possible */

		if (sport->routed != FALSE) rnet->cost_mtx[x][y] *= 2;
		if (dport->routed != FALSE) rnet->cost_mtx[x][y] *= 2;

		rnet->cost_mtx[y][x] = rnet->cost_mtx[x][y];
	    }
	}
    }
    else { /* not first time: already routed?? */
	if (rnet->routed == TRUE) { rnet->cost = BIGNUMBER; return; }
    }

    rnet->cost = BIGNUMBER;
    rnet->routed = TRUE;

    if (No_power_route == TRUE && rnet->type == POWER) return; /* skip power nets */

    for (hportref = net->terminals, x=0; hportref; x++, hportref = hportref->next)
    {
	if (!(sport = (R_PORTPTR) hportref->flag.p)) continue;

	/* first cost with rest of terminals */
	for (tportref = hportref->next, y=0; tportref; y++, tportref = tportref->next)
	{
	    if (!(dport = (R_PORTPTR) tportref->flag.p)) continue;

	    if (rnet->cost_mtx[x][y] < 0) continue; /* not disabled */

	    if (sport->routed != FALSE && sport->routed == dport->routed) continue; /* already connected */

	    rnet->routed = FALSE;

	    rnet->cost_mtx[x][y] =
		ABS (sport->crd[X] - dport->crd[X]) +
		ABS (sport->crd[Y] - dport->crd[Y]);

	    /* heuristic: increase cost if both terminal(groups) were already
	    connected once. This is to make sure that all terminals
	    are routed as soon as possible */

	    if (sport->routed != FALSE) rnet->cost_mtx[x][y] *= 10;
	    if (dport->routed != FALSE) rnet->cost_mtx[x][y] *= 10;

	    rnet->cost_mtx[y][x] = rnet->cost_mtx[x][y];

	    if (rnet->cost_mtx[x][y] < rnet->cost) rnet->cost = rnet->cost_mtx[x][y];
	}
    }

    if (rnet->routed == TRUE) Num_routed++;
}

/*
 * routes a single segment
 */
static void route_segment2 (NETPTR net)
{
    R_NETPTR rnet;
    R_PORTPTR sport, dport;
    CIRPORTREFPTR hportref, tportref;
    GRIDPOINT spoint, dpoint;
    GRIDPOINTPTR path;
    int x, y;
    BOX wire_bbx;

    if (!(rnet = (R_NETPTR) net->flag.p)) return;

    /*
    * 1: find out which of the terminals should be connected
    */
    for (hportref = net->terminals, x=0; hportref; x++, hportref = hportref->next) {
	if (!(sport = (R_PORTPTR) hportref->flag.p)) continue;

	/* compare with rest of terminals */
	for (tportref = hportref->next, y=0; tportref; y++, tportref = tportref->next) {
	    if (!(dport = (R_PORTPTR) tportref->flag.p)) continue;

	    if (rnet->cost_mtx[x][y] < 0) continue; /* not disabled */

	    if (sport->routed != FALSE && sport->routed == dport->routed) continue; /* already connected */

	    /* found it! */
	    /* disable immediately */
	    rnet->cost_mtx[x][y] = rnet->cost_mtx[y][x] = -1;
	    break;
	}

	if (tportref) break;
    }

    /* sport and dport are the ports to be connected */
    if (!hportref) return;

    /*
    * disable those indices which connect the two groups
    */
    for (hportref = net->terminals, x=0; hportref; x++, hportref = hportref->next) {
	if (!hportref->flag.p) continue;
	if (((R_PORTPTR) hportref->flag.p)->routed != sport->routed) continue; /* not same group */
	if (((R_PORTPTR) hportref->flag.p)->routed == FALSE &&
	     (R_PORTPTR) hportref->flag.p != sport)
	    continue; /* unrouted group and not original: skip */

	/* get rid of territory */
	remove_territory ((R_PORTPTR) hportref->flag.p);

	/* step along rest to find ones connected to dport */
	for (tportref = hportref->next, y=0; tportref; y++, tportref = tportref->next) {
	    if (!tportref->flag.p) continue;
	    if (((R_PORTPTR) tportref->flag.p)->routed != dport->routed) continue; /* not same group */
	    if (((R_PORTPTR) tportref->flag.p)->routed == FALSE &&
		 (R_PORTPTR) tportref->flag.p != dport) continue;

	    /* get rid of territory */
	    remove_territory ((R_PORTPTR) tportref->flag.p);
	    /* disable */
	    rnet->cost_mtx[x][y] = rnet->cost_mtx[y][x] = -1;
	}
    }

    if (No_power_route == TRUE && rnet->type == POWER) return;

    /* set the boundaries of the routing bbx */
    wire_bbx.crd[L] = Bbx->crd[L]; wire_bbx.crd[R] = Bbx->crd[R];
    wire_bbx.crd[B] = Bbx->crd[B]; wire_bbx.crd[T] = Bbx->crd[T];
    wire_bbx.crd[D] = Bbx->crd[D]; wire_bbx.crd[U] = Bbx->crd[U];
    wire_bbx.crd[D] = -1;

    /* the big switch..
    if (rnet->type == SIGNAL) wire_bbx.crd[D] = -1; else wire_bbx.crd[D] =  0;
    */

    /* copy coordinates into source/destination point */
    spoint.x = sport->crd[X];
    spoint.y = sport->crd[Y];
    spoint.z = sport->crd[Z];

    dpoint.x = dport->crd[X];
    dpoint.y = dport->crd[Y];
    dpoint.z = dport->crd[Z];

    /*** do the routing ***/

    /* remove territories (if present..) */
    remove_territory (sport);
    remove_territory (dport);

    path = lee (&spoint, &dpoint, &wire_bbx, FALSE);

    if (!path) {
	if (verbose) {
	    printf ("x");
	    if (++Column >= 40) {
		Column = 0;
		printf (" > %4.2f %% <\n", (double)Routed_two_term / Total_two_term * 100);
	    }
	    fflush (stdout);
	}
	/*
	add_error_unconnect (net, sport->crd[X], sport->crd[Y], sport->crd[Z]);
	add_error_unconnect (net, dport->crd[X], dport->crd[Y], dport->crd[Z]);
	*/
    }
    else { /* ok: free, print path */

	Routed_two_term++; /* maintain counter */

	if (verbose) {
	    printf (".");
	    if (++Column >= 40) {
		Column = 0;
		printf (" > %4.2f %% <\n", (double)Routed_two_term / Total_two_term * 100);
	    }
	    fflush (stdout);
	}

	Total_wire_length += count_restore_melt_and_free_wire (path);
	//Total_wire_length += count_length (path); /* for routing statistics */
	//restore_wire_pattern (path);
	//melt_new_wire (path); /* add possible missing pointers */
	//free_gridpoint_list (path);

	/* index management in port->routed */
	/* this is done to maintain terminal groups */
	if (sport->routed == FALSE && dport->routed == FALSE) {
	    /* both unrouted: set routed to free number */
	    x = 0; /* find free number */
	    for (hportref = net->terminals; hportref; hportref = hportref->next) {
		if (!hportref->flag.p) continue;
		if (((R_PORTPTR) hportref->flag.p)->routed > x) x = ((R_PORTPTR) hportref->flag.p)->routed;
	    }
	    sport->routed = dport->routed = x+1;
	}
	else { /* set to same routed value, which is used as group number */
	    if (sport->routed == FALSE)
		sport->routed = dport->routed;
	    else if (dport->routed == FALSE)
		dport->routed = sport->routed;
	    else { /* both are groups: select one and export */
		for (hportref = net->terminals; hportref; hportref = hportref->next) {
		    if (!hportref->flag.p) continue;
		    /* give all of group dport the same value (sport->routed) */
		    x = dport->routed; /* store value of dport */
		    if (((R_PORTPTR) hportref->flag.p)->routed == x)
			((R_PORTPTR) hportref->flag.p)->routed = sport->routed;
		}
	    }
	}
    }
}

#ifdef print_unrouted_nets
/*
 * this routine prints the unrouted nets
 */
static void print_unrouted_nets (NETPTR netlist)
{
    R_NETPTR rnet;
    NETPTR hnet;
    CIRPORTREFPTR hportref, tportref;
    R_PORTPTR sport, dport;
    int num_missing;

    printf ("\n");
    Num_routed = 0;
    for (hnet = netlist; hnet; hnet = hnet->next)
    {
	if (!(rnet = (R_NETPTR) hnet->flag.p)) continue;

	if (No_power_route == TRUE && rnet->type == POWER) continue; /* skip power nets */

	num_missing = 0;
	for (hportref = hnet->terminals; hportref; hportref = hportref->next) {
	    if (!(sport = (R_PORTPTR) hportref->flag.p)) continue;

	    if (sport->routed == FALSE) num_missing++; /* always add one */
	    else { /* is unique?? */
		for (tportref = hportref->next; tportref; tportref = tportref->next) {
		    if (!(dport = (R_PORTPTR) tportref->flag.p)) continue;
		    if (dport->routed == sport->routed) break; /* not unique */
		}
		if (tportref == NULL) num_missing++;
	    }
	}

	num_missing--; /* number of unconnects = number of groups -1 */

	if (num_missing > 0) {
	    printf ("Incomplete routing: net '%s'  (%d of %d segments routed)\n",
			hnet->name, num_missing, hnet->num_term);
	} else {
	    ((R_NETPTR) hnet->flag.p)->routed = TRUE;
	    Num_routed++;
	}
    }
}
#endif

#ifdef make_error_file
static void make_error_file (NETPTR netlist)
{
    R_NETPTR rnet;
    NETPTR hnet;
    int col;
    FILE *fp;

    if (!(fp = fopen ("seadif/trout.error", "w")))
	fprintf (stderr, "Warning: cannot open error file 'seadif/route.error'\n");
    else {
	fprintf (fp, "The following nets could not be routed, or their routing\n");
	fprintf (fp, "is incomplete. The nets are marked in the layout:\n\n");
    }
    fprintf (stderr, "---------------------------------------------------\n");
    fprintf (stderr, "Improperly routed nets:\n");

    col = 3;
    fprintf (stderr, "    ");

    /* step along nets */
    for (hnet = netlist; hnet; hnet = hnet->next)
    {
	if (!(rnet = (R_NETPTR) hnet->flag.p)) continue;
	if (No_power_route == TRUE && rnet->type == POWER) continue;
	if (((R_NETPTR) hnet->flag.p)->routed == TRUE) continue;
	if (fp) fprintf (fp, "not routed: '%s'\n", hnet->name);
	fprintf (stderr, "'%s', ", hnet->name);
	col = col + strlen (hnet->name) + 4;
	if (col > 75) {
	    fprintf (stderr, "\n    ");
	    col = 3;
	}
    }
    if (fp) fclose (fp);
    fprintf (stderr, "\n");
}
#endif

void remove_error_file ()
{
    if (unlink ("seadif/trout.error") == -1)
	/* fprintf (stderr, "Warning: cannot remove file 'seadif/route.error'\n") */;
}

/*
 * This routine checks if the terminal is in the routing bounding box.
 */
int term_in_bbx (CIRPORTREFPTR hterm, BOXPTR bbx)
{
    R_PORTPTR rport;
    GRIDPOINTPTR point, term_pattern;
    GRIDPOINT pointstruct;
    BOX wire_bbx;
    register GRIDPOINTPTR wire;

    if (!(rport = (R_PORTPTR) hterm->flag.p)) return (FALSE);

    /* OK if terminal is already in bbx */
    if (rport->crd[X] >= bbx->crd[L] && rport->crd[X] <= bbx->crd[R] &&
	rport->crd[Y] >= bbx->crd[B] && rport->crd[Y] <= bbx->crd[T] &&
	rport->crd[Z] >= bbx->crd[D] && rport->crd[Z] <= bbx->crd[U]) return (TRUE);

    /* not OK if terminal is entirely out of range */
    if (rport->crd[X] < Bbx->crd[L] || rport->crd[X] > Bbx->crd[R] ||
	rport->crd[Y] < Bbx->crd[B] || rport->crd[Y] > Bbx->crd[T] ||
	rport->crd[Z] < Bbx->crd[D] || rport->crd[Z] > Bbx->crd[U]) return (FALSE);

    /* is a part of the wire pattern of term in bbx? */
    point = &pointstruct;
    point->x = rport->crd[X];
    point->y = rport->crd[Y];
    point->z = rport->crd[Z];

    wire_bbx.crd[L] = Bbx->crd[L]; wire_bbx.crd[R] = Bbx->crd[R];
    wire_bbx.crd[B] = Bbx->crd[B]; wire_bbx.crd[T] = Bbx->crd[T];
    wire_bbx.crd[D] = Bbx->crd[D]; wire_bbx.crd[U] = Bbx->crd[U];
    wire_bbx.crd[D] = -1; /* use tunnels */

    /* get wire pattern */
    if (!(term_pattern = save_source (point, NULL, &wire_bbx))) return (FALSE);

    for (wire = term_pattern; wire; wire = wire->next) {
	if (wire->x < bbx->crd[L] || wire->x > bbx->crd[R] ||
	    wire->y < bbx->crd[B] || wire->y > bbx->crd[T] ||
	    wire->z < bbx->crd[D] || wire->z > bbx->crd[U]) continue;
	break;
	/* take closest wire to center, not implemented */
    }
    if (wire) { /* use wire location for term */
	rport->crd[X] = wire->x;
	rport->crd[Y] = wire->y;
	rport->crd[Z] = wire->z;
    }
    free_and_restore_wire_pattern (term_pattern);
    return (wire ? TRUE : FALSE);
}

/*
 * This routine clears everything to attemt to route again.
 */
static void re_initialize_everything (LAYOUTPTR father)
{
    NETPTR hnet;
    CIRPORTREFPTR hportref;
    WIREPTR delwire;

    /* erase any remaining territories */
    remove_territories (father);

    /* just get rid of the grid */
    free_grid (((R_CELLPTR) father->flag.p)->grid);
    ((R_CELLPTR) father->flag.p)->grid = NULL;

    /* make new grid and poke seadif into it */
    convert_seadif_into_grid (father);

    /* add implicit power lines to the grid */
    print_power_lines (father, TRUE);

    /* set working grid
    */
    Grid = ((R_CELLPTR) father->flag.p)->grid;
    Bbx = &((R_CELLPTR) father->flag.p)->cell_bbx;

    /* reset the 'routed'-flag
    */
    for (hnet = father->circuit->netlist; hnet; hnet = hnet->next) {
	if (!(R_NETPTR) hnet->flag.p) continue;

	((R_NETPTR) hnet->flag.p)->routed = FALSE;

	for (hportref = hnet->terminals; hportref; hportref = hportref->next) {
	    if (!(R_PORTPTR) hportref->flag.p) continue;
	    ((R_PORTPTR) hportref->flag.p)->routed = FALSE;
	}

	/* remove (fat) wire list of net */
	while ((delwire = ((R_NETPTR) hnet->flag.p)->wire)) {
	    ((R_NETPTR) hnet->flag.p)->wire = delwire->next;
	    FreeWire (delwire);
	}
    }

    freeLabels (father);

    /*
    * reset the position of all not-placed terminals
    * NB: this should not be done in case of segment-oriented routing!!
    */
    place_new_terminals (father);
}

/*
 * free labels of layout
 */
static void freeLabels (LAYOUTPTR lay)
{
    LAYLABELPTR tmp_label;
    while (lay -> laylabel) {
	tmp_label = lay -> laylabel;
	lay -> laylabel = tmp_label -> next;
	FreeLaylabel (tmp_label);
    }
}

static int looks_like_no_improvement (LAYOUTPTR father)
{
    register NETPTR hnet;
    R_NETPTR np;

    /* just find out if there is any net that failed every pass until now */
    for (hnet = father->circuit->netlist; hnet; hnet = hnet->next) {
	if (!(np = (R_NETPTR) hnet -> flag.p)) continue;
	if (np -> fail_count <= 2) continue; /* too early to tell */
	if (np -> fail_count == np -> routing_attempts) return (TRUE); /* gotcha, failed every time! */
    }
    /* its OK */
    return (FALSE);
}

#ifdef isolate_path
/* note: doesn't work properly with tunnels */
GRIDPOINTPTR isolate_path (GRIDPOINTPTR path)
{
    GRIDPOINTPTR newpath, prev_p, delpath;
    register int off;

    prev_p = NULL;
    newpath = path;

    while (path) {
	for (off = 0; off <= U; off++) {
	    if ((path->pattern & Pat_mask[off]) != 0 &&
		(Grid[path->z+Zoff[off]][path->y+Yoff[off]][path->x+Xoff[off]] & STATEMASK) != 0)
		break; /* connects to existing wire pattern */
	}
	if (off > U) {
	    prev_p = path;
	    path = path->next;
	    continue; /* no problem */
	}
	delpath = path;
	if (prev_p == NULL)
	    newpath = path = path->next;
	else
	    prev_p->next = path = path->next;
	free_gridpoint (delpath);
    }
    return (newpath);
}
#endif

void store_critical_wire (NETPTR net, GRIDPOINTPTR path)
{
    WIREPTR wire;
    R_NETPTR netp = (R_NETPTR) net->flag.p;

    /* only if critical net */
    if (netp->type == SIGNAL) return;

    /* copy path into wire structure */
    for ( ; path; path = path->next) {
	if (path->cost == -1) continue;
	NewWire (wire);
	wire->crd[L] = wire->crd[R] = path->x;
	wire->crd[B] = wire->crd[T] = path->y;
	wire->layer = path->z + 201; /* code for fat wire */
	wire->next = netp->wire;
	netp->wire = wire;
    }
}
