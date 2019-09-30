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

#include "src/ocean/trout/typedef.h"
#include "src/ocean/trout/grid.h"

static void add_to_power (GRIDPOINTPTR point, BOXPTR exp_bbx);
static void add_to_territory (GRIDPOINTPTR point, int dir, R_PORTPTR rport);
static int can_be_power (GRIDPOINTPTR point, int dir, BOXPTR exp_bbx, int off);
static int connect_two_power_rails (LAYOUTPTR lay, int type, int floor, int force_create);
static int gate_is_connected (GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate);
static int gate_is_powerable (GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate, GRIDPOINTPTR ground);
static int gate_to_ground (GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate, GRIDPOINTPTR ground);
static int left_step (int probe_only, GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate, GRIDPOINTPTR ground);
static void lotsa_vias (LAYOUTPTR lay, char *type, int floor);
static int make_cap (LAYOUTPTR lay, char *type, int floor);
static void make_fat_power_rail (LAYOUTPTR lay, char *type, int floor, int pre_capacitor);
static int right_step (GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate, GRIDPOINTPTR ground);
static int route_term_to_power (CIRPORTREFPTR hterm, BOXPTR wire_bbx, int try2ndNearest);
static int source_is_usable (GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate, GRIDPOINTPTR ground);
static int terminate_gate (int probe_only, GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate, GRIDPOINTPTR ground);
static void unmark_power (BOXPTR exp_bbx);

/*
 * import
 */
extern COREUNIT
   ***Grid,                /* the current working grid */
   Pat_mask[HERE+1];       /* look-up table for bit-patterns */
extern GRIDADRESSUNIT
   Xoff[HERE+1],           /* look-up tables for offset values */
   Yoff[HERE+1],
   Zoff[HERE+1];
extern BOXPTR
   Bbx,                    /* bounding box of Grid */
   Rbbx;                   /* routing bounding box */
extern int verbose;
extern long
   Chip_num_layer,         /* number of metal layers to be used */
   GridRepitition[2],
   Finish_up;              /* TRUE to connect all unused transistors to power */
extern char *ThisImage;    /* Seadif name of this image */
extern GRIDPOINTPTR
   ***OffsetTable,
   **SaveRestrictedCoreFeed, /* matrix of restricted feeds */
   **CoreFeed;             /* matrix of feedthroughs in basic image */

extern POWERLINEPTR PowerLineList; /* list of template power lines */

extern NETPTR Vssnet, Vddnet; /* pointers to the power and ground net */

extern int Hacklevel;

/*
 * define for rnet->type
 * (already defined)
#define SIGNAL 0
#define CLOCK  1
#define POWER  2
 */

/*
 * This routine checks the compatibility of the specified power pattern
 * we only support images with two power wires per image,
 * which are horizontal, and which are in metal1.
 */
int check_power_capabilities (int printit)
{
    POWERLINEPTR pline;
    int numvss, numvdd, layerror, orienterror;

    /* check whether we can do it in this image....
    */
    layerror = orienterror = FALSE;
    numvss = numvdd = 0;
    for (pline = PowerLineList; pline; pline = pline->next) {
	if (pline->z != 0) {
	    if (printit == TRUE) layerror = TRUE;
	    else return (FALSE);
	}
	if (pline->orient != HORIZONTAL) {
	    if (printit == TRUE) orienterror = TRUE;
	    else return (FALSE);
	}

	if (pline->type == VDD) numvdd++; else numvss++;
    }

    if (numvdd == 0 || numvss == 0 || (layerror == TRUE && orienterror == TRUE)) {
    if (printit == TRUE) {
	fprintf (stderr, "WARNING: Power routing options not fully supported for this image\n");
      if (layerror == TRUE)
	fprintf (stderr, "         because power lines must be in layer 0 (metal1)\n");
      if (orienterror == TRUE)
	fprintf (stderr, "         because power lines must be horizontal\n");
      if (numvdd == 0 || numvss == 0)
	fprintf (stderr, "         because vdd and/or vss power line description is missing\n");
	fprintf (stderr, "         hint: check image.seadif\n");
    }
	return (FALSE);
    }

    if (numvdd > 1 || numvss > 1)
    if (printit == TRUE) {
	fprintf (stderr, "WARNING: multiple vss or vdd power rails in image cell is not supported:\n");
	fprintf (stderr, "         therefore power nets are most likely not entirely connected.\n");
    }

    if (Chip_num_layer < 2) return (FALSE);

    return (TRUE);
}

/*
 * connect power rails to eachother
 */
void connect_power_rails (LAYOUTPTR lay, int fat_power)
/* fat_power - TRUE to connect may power rails to eachother */
{
    BOXPTR Bbx; /* bounding box of working grid */
    int floor, wire_found;

    Bbx = &((R_CELLPTR) lay->flag.p)->cell_bbx;

    if (check_power_capabilities (FALSE) == FALSE) return;

    for (floor = 0; floor * GridRepitition[Y] < Bbx->crd[T]; floor++)
    { /* for all power line rows */
	if (connect_two_power_rails (lay, VSS, floor, FALSE) == FALSE) { /* repair by enlarging grid */
	    Bbx->crd[R]++;
	    print_power_lines (lay, TRUE);
	    if (connect_two_power_rails (lay, VSS, floor, FALSE) == FALSE)
		fprintf (stderr, "WARNING: level %d of vss power line was not connected\n", floor + 1);
	}
	if (connect_two_power_rails (lay, VDD, floor, FALSE) == FALSE) { /* repair by enlarging grid */
	    Bbx->crd[R]++;
	    print_power_lines (lay, TRUE);
	    if (connect_two_power_rails (lay, VDD, floor, FALSE) == FALSE)
		fprintf (stderr, "WARNING: level %d of vdd power lines was not connected\n", floor + 1);
	}
    }

    if (fat_power == FALSE) return;

    /* continue if we want to finish_up the stuff
	(this is not a very time efficient implementation...)
    */
    wire_found = 1;
    while (wire_found > 0) {
	wire_found = 0;
	for (floor = 0; floor * GridRepitition[Y] < Bbx->crd[T]; floor++)
	{ /* for all power line rows */
	    if (connect_two_power_rails (lay, VSS, floor, TRUE) == TRUE) wire_found++;
	    if (connect_two_power_rails (lay, VDD, floor, TRUE) == TRUE) wire_found++;
	}
    }
}

/*
 * This routine will generate a new power wires in a 'floor'
 * of 4 vertical transistors in the image. It will
 * return FALSE if it was not capable to do so, TRUE otherwise.
 */
static int connect_two_power_rails (LAYOUTPTR lay, int type, int floor, int force_create)
/* type  - vss or vdd */
/* floor - floor number (e.g. 0 = 0th row of image) */
/* force_create - TRUE to force creation of a new one, even if one already exists */
{
    GRIDPOINT pointstruct;
    register GRIDPOINTPTR point;
    COREUNIT ***Grid;              /* the working grid */
    BOXPTR Bbx;                    /* bounding box of working grid */
    int dir, stop, existing_power,
       off;                    /* initial offsets of the two power lines */
    POWERLINEPTR pline;

    Grid = ((R_CELLPTR) lay->flag.p)->grid;
    Bbx = &((R_CELLPTR) lay->flag.p)->cell_bbx;
    point = &pointstruct;

    for (pline = PowerLineList; pline; pline = pline -> next) {
       if (pline -> type == type) break;
    }
    if (!pline) return (FALSE);

    /* set offset */
    off = pline->y + (floor * GridRepitition[Y]);

    /* outside image */
    if (off + GridRepitition[Y] > Bbx->crd[T]) { if (force_create == TRUE) return (FALSE); else return (TRUE); }

    /* check for existing power connection, to prevent double wires */
    if (force_create == FALSE) {
	point->z = 1; /* metal 2 */
	for (point->x = Bbx->crd[L]; point->x <= Bbx->crd[R]; point->x++)
	{ /* sweep over all columns */
	    point->y = off;
	    if (is_free (point)) continue; /* must be occupied */
	    if (!has_neighbour (point, D)) continue; /* extremes must have contacts */
	    point->y = off + GridRepitition[Y];
	    if (!has_neighbour (point, D)) continue; /* extremes must have contacts */
	    for (point->y = off + 1; point->y <= off + GridRepitition[Y]; point->y++)
	    { /* sweep vertically over wire */
		if (is_free (point)) break; /* may not be free */
		if (!has_neighbour (point, B)) break; /* must have neighbour on bottom */
	    }
	    if (point->y > off + GridRepitition[Y]) break; /* gotcha!! */
	}
	if (point->x <= Bbx->crd[R]) return (TRUE); /* found existing power */
	/* else, look for spot to make one */
    }

    /* find the first column which is free in metal 2
     * we start lookung in the center, that looks nicer.
     */
    point->z = 1; /* metal 2 */
    existing_power = FALSE;

    dir = -1;
    stop = FALSE;
    point->x = (Bbx->crd[L] + Bbx->crd[R])/2;

    while (dir != 0 && stop == FALSE) { /* for a sweep to the left and the right */

	for ( /* point->x was set */ ;
	    point->x >= Bbx->crd[L] && point->x <= Bbx->crd[R] ; point->x += dir)
	{
	    /* check for stacked contacts...
	    */
	    point->z = 0;    /* metal 1 */
	    point->y = off;
	    if (has_neighbour (point, D)) continue; /* substrate contact exists on bottom-most: skip */
	    point->y = off + GridRepitition[Y];
	    if (has_neighbour (point, D)) continue; /* substrate contact exists on top-most: skip */
	    point->z = 1;    /* metal 2 */

	    /* first point: must be free or have via downwards */
	    existing_power = FALSE;
	    for (point->y = off; point->y <= off + GridRepitition[Y]; point->y++)
	    {
		if (!is_free (point))
		{ /* hmmm */
		    if (point->y == off || point->y == off + GridRepitition[Y])
		    { /* bottom or top extreme */
			/* may also have contact down top the power */
			if (!has_neighbour (point, D)) break;
			else {
			    if (point->y == off) existing_power = TRUE; /* first one */
			    else { /* top one: MUST have a connection downwards */
				if (!has_neighbour (point, B)) {
				    existing_power = FALSE; break;
				}
			    }
			}
		    }
		    else { /* break (reject power line) conditionally */
			if (existing_power == TRUE)
			{ /* then point MUST have a connection downwards */
			    if (!has_neighbour (point, B)) {
				existing_power = FALSE; break; /* no: abort */
			    }
			}
			else break;  /* abort */

			/* abort if not forcing creation */
			if (force_create == TRUE) break;
		    }
		}
		else existing_power = FALSE;
	    }

	    if (point->y > off + GridRepitition[Y] ||
		(existing_power == TRUE && force_create == FALSE)) {
		stop = TRUE; break; /* empty column found, or is already connected */
	    }
	}

	if (dir == -1) {
	    dir = 1; /* swap to right now */
	    if (stop == FALSE) point->x = ((Bbx->crd[L] + Bbx->crd[R])/2) +1;
	}
	else dir = 0; /* stop */
    }

    if (point->x > Bbx->crd[R]) return (FALSE); /* no empty line was found */

    if (force_create == FALSE && existing_power == TRUE) return (TRUE);

    /* poke power line into grid
     */
    for (point->y = off; point->y <= off + GridRepitition[Y]; point->y++)
    {
	set_grid_occupied (point);
	if (point->y > off)
	    add_grid_neighbour (point, B);
	else {
	    add_grid_neighbour (point, D);
	    point->z = 0; /* metal 1 */
	    set_grid_occupied (point);
	    add_grid_neighbour (point, U);
	    point->z = 1;
	}
	if (point->y < off + GridRepitition[Y])
	    add_grid_neighbour (point, T);
	else {
	    add_grid_neighbour (point, D);
	    point->z = 0; /* metal 1 */
	    set_grid_occupied (point);
	    add_grid_neighbour (point, U);
	    point->z = 1;
	}
    }

    return (TRUE);
}

/*
 * This routine routes all unconnected terminals to the first
 * unconnected terminal.
 */
int special_power_route (NETPTR net, BOXPTR given_bbx)
{
    CIRPORTREFPTR source_term;
    POWERLINEPTR pline;
    BOX wire_bbx;
    R_PORTPTR pport;
    R_NETPTR  pnet = (R_NETPTR) net -> flag.p;

    if (check_power_capabilities (FALSE) == FALSE) return ((int) pnet -> routed);

    pnet -> routed = TRUE;

    /* We ignore (during routing) the given routing boundingbox,
     * due to problems with top and bottom rows.
     */
    wire_bbx.crd[L] = Bbx->crd[L];
    wire_bbx.crd[R] = Bbx->crd[R];
    wire_bbx.crd[B] = Bbx->crd[B];
    wire_bbx.crd[T] = Bbx->crd[T];
    wire_bbx.crd[D] = -1; /* always use tunnels */
    wire_bbx.crd[U] = Bbx->crd[U];

    /* step to first appropriate source term
     */
    for (source_term = net->terminals; source_term; source_term = source_term->next)
    {
	pport = (R_PORTPTR) source_term -> flag.p;
	if (!pport || pport -> routed == TRUE) continue;

	/* check wheter it is already on a power line... */
	for (pline = PowerLineList; pline; pline = pline->next) {
	    if ((pline->z == pport->crd[Z]) &&
		(pline->x == pport->crd[Y] % GridRepitition[Y]))
	    { /* terminal is already connected because it is on the power rails */
		pport -> routed = TRUE; break;
	    }
	}
	if (pline) continue; /* skip, already connected */

	remove_territory (pport);

	if (term_in_bbx (source_term, given_bbx) == FALSE) continue;

	if (route_term_to_power (source_term, &wire_bbx, 0) != TRUE &&
	    route_term_to_power (source_term, &wire_bbx, 1) != TRUE) {
	    pnet -> routed = FALSE; /* routing not successful */
	}
    }

    return ((int) pnet -> routed);
}

/*
 * route the power terminal to the nearest power line
 */
static int route_term_to_power (CIRPORTREFPTR hterm, BOXPTR wire_bbx, int try2ndNearest)
{
    R_PORTPTR sport;
    GRIDPOINT spoint, dpoint;
    GRIDPOINTPTR path;
    int type;
    long off, nearest;
    POWERLINEPTR pline;
    BOX power_bbx;

    /* set the source */
    sport = (R_PORTPTR) hterm->flag.p;
    spoint.x = sport->crd[X];
    spoint.y = sport->crd[Y];
    spoint.z = sport->crd[Z];

    /* destination is the nearest power wire */
    if (strncmp (hterm->net->name, "vdd", 3) == 0 ||
	strncmp (hterm->net->name, "VDD", 3) == 0 ||
	strncmp (hterm->net->name, "Vdd", 3) == 0)
	type = VDD;
    else
	type = VSS;

    for (pline = PowerLineList; pline; pline = pline->next) {
	if (pline->type == type) break;
    }
    if (!pline) return (FALSE);

    off = pline->y;
    nearest = (sport->crd[Y] + (GridRepitition[Y]/2) - off) / GridRepitition[Y];
    dpoint.y = (GridRepitition[Y] * nearest) + off;
    if (try2ndNearest == 1) {
	if (spoint.y > dpoint.y)
	    dpoint.y = dpoint.y + GridRepitition[Y];
	else if (spoint.y < dpoint.y)
	    dpoint.y = dpoint.y - GridRepitition[Y];
    }

    dpoint.x = sport->crd[X];
    dpoint.z = 0;  /* metal1 */

    if (dpoint.y < wire_bbx->crd[B] || dpoint.y > wire_bbx->crd[T]) { /* not in box */
	sport->routed = FALSE;
	/* printf ("failed2: y=%ld, wire_bbx=%ld,%ld  off=%ld, nearest=%ld\n",
	   (long)dpoint.y, (long)wire_bbx->crd[B], (long)wire_bbx->crd[T], (long)off, (long)nearest); */
	return (sport->routed);
    }

    /* set the power routing bbx
     * we heavily reduce the routing area
     */
 /* power_bbx.crd[L] = MAX (wire_bbx->crd[L], sport->crd[X] - 20); */
 /* power_bbx.crd[R] = MIN (wire_bbx->crd[R], sport->crd[X] + 20); */

    /* I set the left and right borders of the bounding box to the
     * borders of the total routing space because otherwise we miss
     * long horizontal wires that are connected to power at the left
     * or right side (like for the case of bond_ring in the SOGAR chip).
     */
    power_bbx.crd[L] = wire_bbx->crd[L];
    power_bbx.crd[R] = wire_bbx->crd[R];

    if (spoint.y > dpoint.y) { /* point above power line */
	power_bbx.crd[B] = dpoint.y;
	power_bbx.crd[T] = dpoint.y + GridRepitition[Y];
	MIN_UPDATE (power_bbx.crd[T], wire_bbx->crd[T]);
    }
    else {
	power_bbx.crd[T] = dpoint.y;
	power_bbx.crd[B] = dpoint.y - GridRepitition[Y];
	MAX_UPDATE (power_bbx.crd[B], wire_bbx->crd[B]);
    }
    power_bbx.crd[D] = wire_bbx->crd[D];
    power_bbx.crd[U] = wire_bbx->crd[U];

    /* make path */
    path = lee (&spoint, &dpoint, &power_bbx, FALSE);

    if (!path) { /* failed */
	if (verbose) { printf ("x"); fflush (stdout); }
	sport->routed = FALSE;
    }
    else { /* ok: free, print path */
	if (verbose) { printf ("."); fflush (stdout); }

	/* print path */
	/* Total_wire_length += count_length (path); */
	count_restore_melt_and_free_wire (path);
	//restore_wire_pattern (path);
	//melt_new_wire (path); /* add possible missing pointers */
	//free_gridpoint_list (path); /* remove struct */

	sport->routed = TRUE;
    }

    return (sport->routed);
}

/*
 * print power lines into grid
 */
void print_power_lines (LAYOUTPTR lay, int mode)
/* mode - TRUE to print, FALSE to erase */
{
    register GRIDADRESSUNIT x, y;
    register COREUNIT b;
    POWERLINEPTR pline;
    COREUNIT ***Grid; /* the working grid */
    BOXPTR Bbx; /* bounding box of working grid */

    /* hack */
    if (Hacklevel == 1) return;
    if (Hacklevel == 2 && mode == FALSE) return;

    Grid = ((R_CELLPTR) lay->flag.p)->grid;
    Bbx = &((R_CELLPTR) lay->flag.p)->cell_bbx;

    if (!PowerLineList)
    if (strcmp (ThisImage, "fishbone") == 0) { /* patch for fishbone */
	if (mode == TRUE) fprintf (stderr, "Warning: power line pattern missing: image.seadif obsolete\n");
	NewPowerLine (pline);
	pline->orient = X;
	pline->x = 0;
	pline->y = 0;
	pline->z = 0;
	pline->type = VSS;
	PowerLineList = pline;
	NewPowerLine (pline);
	pline->orient = X;
	pline->x = 0;
	pline->y = 14;
	pline->z = 0;
	pline->type = VDD;
	pline->next = PowerLineList;
	PowerLineList = pline;
    }

    for (pline = PowerLineList; pline; pline = pline->next)
    { /* for all declared power lines */
	b = STATEMASK;
	if (pline->orient == HORIZONTAL) { /* horizontal power line */
	    /* the pattern to poke */
	    b |= (1<<R);
	    b |= (1<<L);
	    for (y = Bbx->crd[B] + pline->y; y <= Bbx->crd[T]; y += GridRepitition[Y])
	    for (x = Bbx->crd[L]; x <= Bbx->crd[R]; x++)
	    {
		if (mode == TRUE) { /* print */
		    Grid[pline->z][y][x] |= b;
		}
		else { /* erase */
		    if (Grid[pline->z][y][x] == b)
			Grid[pline->z][y][x] = 0; /* nothing left */
		    else { /* some survive */
			/* reverse pointer bits = bitwise exor */
			/* and into grid */
			Grid[pline->z][y][x] &= (b ^ PATTERNMASK);
		    }
		}
	    }
	}
	else { /* the pattern to poke */
	    b |= (1<<B);
	    b |= (1<<T);
	    for (x = Bbx->crd[L] + pline->x; x <= Bbx->crd[R]; x += GridRepitition[X])
	    for (y = Bbx->crd[B]; y <= Bbx->crd[T]; y++)
	    {
		if (mode == TRUE) { /* print */
		    Grid[pline->z][y][x] |= b;
		}
		else { /* erase */
		    if (Grid[pline->z][y][x] == b)
			Grid[pline->z][y][x] = 0; /* nothing left */
		    else { /* some survive */
			/* reverse pointer bits = bitwise exor */
			/* and into grid */
			Grid[pline->z][y][x] &= (b ^ PATTERNMASK);
		    }
		}
	    }
	}
    }
}

/*
 * This routine sets the net type, according to a wild guess from the name.
 */
void guess_net_type (NETPTR netlist)
{
    NETPTR hnet;

    for (hnet = netlist; hnet; hnet = hnet->next) {
	if (!hnet->flag.p) continue;

	switch (gimme_net_type (hnet->name)) {
	case VSS:
	case VDD:
	case POWER: ((R_NETPTR) hnet->flag.p)->type = POWER; break;
	case CLOCK: ((R_NETPTR) hnet->flag.p)->type = CLOCK; break;
	case SIGNAL:
	default:    ((R_NETPTR) hnet->flag.p)->type = SIGNAL;
	}
    }

    /* look for the vss and vdd net
     */
    Vssnet = Vddnet = NULL;
    for (hnet = netlist; hnet; hnet = hnet->next) {
	if (!hnet->flag.p) continue;

	if (((R_NETPTR) hnet->flag.p)->type == POWER) {
	    if (gimme_net_type (hnet->name) == VSS) {
		if (strcmp (hnet->name, "vss") == 0) { Vssnet = hnet; continue; }
		if (strcmp (hnet->name, "gnd") == 0 && Vssnet == NULL) { Vssnet = hnet; continue; }
		if (strcmp (hnet->name, "VSS") == 0 && Vssnet == NULL) { Vssnet = hnet; continue; }
		continue;
	    }
	    if (gimme_net_type (hnet->name) == VDD) {
		if (strcmp (hnet->name, "vdd") == 0) { Vddnet = hnet; continue; }
		if (strcmp (hnet->name, "VDD") == 0 && Vddnet == NULL) { Vddnet = hnet; continue; }
	    }
	}
    }
}

/*
 * This routine returns the net type the string name
 * it is a pretty wild guess...
 */
int gimme_net_type (char *name)
{
    /* what we recognize for vss... */
    if (strncmp (name, "gnd", 3) == 0) return (VSS);
    if (strncmp (name, "Gnd", 3) == 0) return (VSS);
    if (strncmp (name, "GND", 3) == 0) return (VSS);
    if (strncmp (name, "vss", 3) == 0) return (VSS);
    if (strncmp (name, "Vss", 3) == 0) return (VSS);
    if (strncmp (name, "VSS", 3) == 0) return (VSS);

    /* what we recognize for vdd... */
    if (strncmp (name, "vdd", 3) == 0) return (VDD);
    if (strncmp (name, "Vdd", 3) == 0) return (VDD);
    if (strncmp (name, "VDD", 3) == 0) return (VDD);

    /* what we recognize for a clock net.. */
    if (strncmp (name, "ck", 2) == 0) return (CLOCK);
    if (strncmp (name, "Ck", 2) == 0) return (CLOCK);
    if (strncmp (name, "CK", 2) == 0) return (CLOCK);
    if (strncmp (name, "cl", 2) == 0) return (CLOCK);
    if (strncmp (name, "Cl", 2) == 0) return (CLOCK);
    if (strncmp (name, "CLqq", 2) == 0) return (CLOCK);

    /* default returns SIGNAL */
    return (SIGNAL);
}

/*
 * This routine makes all power wires in fishbone as fat as possible.
 */
void make_fat_power (LAYOUTPTR lay, int pre_capacitor)
/* pre_capacitor - TRUE if preparation step before making capacitors */
{
    int floor;

    Bbx = &((R_CELLPTR) lay->flag.p)->cell_bbx;

    if (verbose && pre_capacitor == TRUE) {
	printf ("------ Making fat power wires ------\n");
	fflush (stdout);
    }

    if (strcmp (ThisImage, "fishbone") != 0) {
	fprintf (stderr, "WARNING: don't know how to make fat power nets for image '%s'\n", ThisImage);
	return;
    }

    /* for all power line rows */
    for (floor = 0; floor * GridRepitition[Y] <= Bbx->crd[T]; floor++) {
	make_fat_power_rail (lay, "vss", floor, pre_capacitor);
	make_fat_power_rail (lay, "vdd", floor, pre_capacitor);
    }
}

static void make_fat_power_rail (LAYOUTPTR lay, char *type, int floor, int pre_capacitor)
/* type  - vss or vdd */
/* floor - floor number (e.g. 0 = 0th row of image) */
{
    GRIDPOINT pointstruct;
    register GRIDPOINTPTR point;
    GRIDPOINTPTR power_pattern;
    BOXPTR exp_bbx;
    BOX exp_bbxstruct;
    int dir, width, point_found;
    int off; /* initial offsets of the two power lines */

    Grid = ((R_CELLPTR) lay->flag.p)->grid;
    Bbx = &((R_CELLPTR) lay->flag.p)->cell_bbx;
    point = &pointstruct;
    exp_bbx = &exp_bbxstruct;

    /* find the first column which is free in metal 2 */

    if (strcmp (type, "vss") == 0) { /* ground net */
	off = floor * GridRepitition[Y];
	width = 6;
    } else { /* power net */
	off = 14 + (floor * GridRepitition[Y]);
	width = 7;
    }
    if (pre_capacitor == TRUE) width -= 2; /* before capacitors: leave two free */

    /* set the bounding box for the fat power wire */
    exp_bbx->crd[L] = Bbx->crd[L];
    exp_bbx->crd[R] = Bbx->crd[R];
    exp_bbx->crd[B] = MAX (Bbx->crd[B], off - width);
    exp_bbx->crd[T] = MIN (Bbx->crd[T], off + width);
    exp_bbx->crd[D] = 0;
    exp_bbx->crd[U] = 0;

    if (exp_bbx->crd[B] >= exp_bbx->crd[T] ||
	exp_bbx->crd[B] > Bbx->crd[T] || exp_bbx->crd[T] < Bbx->crd[B]) return;

    /* set starting point */
    point->x = (Bbx->crd[R] - Bbx->crd[L])/2;
    point->y = off;
    point->z = 0;

    /* find out pattern of power */
    if (!(power_pattern = save_source (point, NULL, exp_bbx))) {
	fprintf (stderr, "ERROR (internal): cannot find power pattern\n");
	return;
    }

    /* mark the power pattern in the grid */
    if (!mark_as_front (power_pattern, FRONT1, 0)) { /* clean up */
	free_and_restore_wire_pattern (power_pattern);
	fprintf (stderr, "ERROR (internal): cannot set power pattern\n");
	return;
    }

    point->z = 0;
    point_found = TRUE;
    for (dir = 1; dir >= -1; dir -= 2) { /* sweep up, then sweep down */

	point_found = TRUE;
	for (point->y = off + dir;
	    point->y >= exp_bbx->crd[B] &&
	    point->y <= exp_bbx->crd[T] &&
	    point_found == TRUE; point->y += dir)
	{ /* for a row */
	    for (point->x = Bbx->crd[L]; point->x <= Bbx->crd[R]; point->x++)
	    { /* for a column */
		if (can_be_power (point, dir, exp_bbx, off) == TRUE) add_to_power (point, exp_bbx);
	    }
	}
    }

    free_gridpoint_list (power_pattern);
    unmark_power (exp_bbx);
}

/*
 * This routine returns TRUE if the indicated spot is a potential part of the power net.
 */
static int can_be_power (GRIDPOINTPTR point, int dir, BOXPTR exp_bbx, int off)
{
    if (grid_is_occupied (point)) return (FALSE); /* belongs to something else */

    if ((Grid[point->z][point->y][point->x] & STATEMASK) == FRONT1) return (TRUE); /* already part of front */

    /* point is free */

    if ((Grid[point->z][point->y - dir][point->x] & STATEMASK) != FRONT1)
	return (FALSE); /* previous point in column must be part of power */

    return (TRUE); /* PATRICK, NEW: just make as fat as possible */

#ifdef BULLSHIT
    /* this code is ignored */
    if ((point->x > exp_bbx->crd[L]) &&
	((Grid[point->z][point->y - dir][point->x - 1] & STATEMASK) != FRONT1)) return (FALSE);

    if ((point->x < exp_bbx->crd[R]) &&
	((Grid[point->z][point->y - dir][point->x + 1] & STATEMASK) != FRONT1)) return (FALSE);

    if (((point->x > exp_bbx->crd[L]) &&
	((Grid[point->z][point->y][point->x - 1] & STATEMASK) == STATEMASK)) &&
	((point->x < exp_bbx->crd[R]) &&
	((Grid[point->z][point->y][point->x + 1] & STATEMASK) == STATEMASK))) {
	return (FALSE); /* both neighbours are occupied: do't do it */
    }

    if ((point->x - 2   >= exp_bbx->crd[L]) &&
	(point->y - dir != off) &&                   /* not first row */
	(point->y - (2*dir) >= exp_bbx->crd[B]) &&
	(point->y - (2*dir) <= exp_bbx->crd[T]) &&
	((Grid[point->z][point->y - (2*dir)][point->x - 2] & STATEMASK) != FRONT1)) return (FALSE);

    if ((point->x + 2   <= exp_bbx->crd[R]) &&
	(point->y - dir != off) &&                   /* not first row */
	(point->y - (2*dir) >= exp_bbx->crd[B]) &&
	(point->y - (2*dir) <= exp_bbx->crd[T]) &&
	((Grid[point->z][point->y - (2*dir)][point->x + 2] & STATEMASK) != FRONT1)) return (FALSE);

    return (TRUE);
#endif
}

/*
 * add the indicated point to the power net, with front ID FRONT1
 */
static void add_to_power (GRIDPOINTPTR point, BOXPTR exp_bbx)
{
    GRIDADRESSUNIT x, y, z;

    x = point -> x;
    y = point -> y;
    z = point -> z;

    if (is_Free (x, y, z)) set_grid_pattern (point, FRONT1);

    /* melt with the neighbours */
    if (x > exp_bbx->crd[L] && is_state (x-1, y, z, FRONT1)) { /* left */
	Grid[z][y][x]   |= Pat_mask[L];
	Grid[z][y][x-1] |= Pat_mask[R];
    }
    if (x < exp_bbx->crd[R] && is_state (x+1, y, z, FRONT1)) { /* right */
	Grid[z][y][x]   |= Pat_mask[R];
	Grid[z][y][x+1] |= Pat_mask[L];
    }
    if (y > exp_bbx->crd[B] && is_state (x, y-1, z, FRONT1)) { /* bottom */
	Grid[z][y][x]   |= Pat_mask[B];
	Grid[z][y-1][x] |= Pat_mask[T];
    }
    if (y < exp_bbx->crd[T] && is_state (x, y+1, z, FRONT1)) { /* top */
	Grid[z][y][x]   |= Pat_mask[T];
	Grid[z][y+1][x] |= Pat_mask[B];
    }
}

/*
 * mark all elements of FRONT1 in exp_bbx as occupied
 */
static void unmark_power (BOXPTR exp_bbx)
{
    register GRIDADRESSUNIT x, y, z;

    for (x = exp_bbx->crd[L]; x <= exp_bbx->crd[R]; ++x)
    for (y = exp_bbx->crd[B]; y <= exp_bbx->crd[T]; ++y)
    for (z = exp_bbx->crd[D]; z <= exp_bbx->crd[U]; ++z)
	if (is_state (x, y, z, FRONT1)) set_occupied (x, y, z);
}

/*
 * This routine makes vias between adjacent layes
 * of the same power net.
 */
void make_lotsa_vias (LAYOUTPTR lay)
{
    int floor;

    Bbx = &((R_CELLPTR) lay->flag.p)->cell_bbx;

    if (verbose) {
	printf ("------ Making lotsa vias ------\n");
	fflush (stdout);
    }

    if (strcmp (ThisImage, "fishbone") != 0) {
	/* fprintf (stderr, "WARNING: don't yet know how to make lotsa vias for image '%s'\n", ThisImage); */
	return;
    }

    for (floor = 0; floor * GridRepitition[Y] <= Bbx->crd[T]; floor++) { /* for all power line rows */
	lotsa_vias (lay, "vss", floor);
	lotsa_vias (lay, "vdd", floor);
    }
}

static void lotsa_vias (LAYOUTPTR lay, char *type, int floor)
/* type  - vss or vdd */
/* floor - floor number (e.g. 0 = 0th row of image) */
{
    GRIDPOINT pointstruct;
    register GRIDPOINTPTR point, off;
    GRIDPOINTPTR power_pattern;
    BOXPTR exp_bbx;
    BOX exp_bbxstruct;
    int width;
    int offset; /* initial offsets of the two power lines */

    point = &pointstruct;
    Grid = ((R_CELLPTR) lay->flag.p)->grid;
    Bbx = &((R_CELLPTR) lay->flag.p)->cell_bbx;
    exp_bbx = &exp_bbxstruct;

    /* find the first column which is free in metal 2 */
    if (strcmp (type, "vss") == 0) { /* ground net */
	offset = floor * GridRepitition[Y];
	width = 6;
    }
    else { /* power net */
	offset = 14 + (floor * GridRepitition[Y]);
	width = 7;
    }

    /* set the bounding box for the fat power wire */
    exp_bbx->crd[L] = Bbx->crd[L];
    exp_bbx->crd[R] = Bbx->crd[R];
    exp_bbx->crd[B] = MAX (Bbx->crd[B], offset - width);
    exp_bbx->crd[T] = MIN (Bbx->crd[T], offset + width);
    exp_bbx->crd[D] = 0;
    exp_bbx->crd[U] = Bbx->crd[U];

    if (exp_bbx->crd[B] >= exp_bbx->crd[T] ||
	exp_bbx->crd[B] > Bbx->crd[T] || exp_bbx->crd[T] < Bbx->crd[B]) return;

    /* set starting point */
    point->x = (Bbx->crd[R] - Bbx->crd[L])/2;
    point->y = offset;
    point->z = 0;

    /* find out pattern of power */
    if (!(power_pattern = save_source (point, NULL, exp_bbx))) {
	fprintf (stderr, "ERROR (internal): cannot find power pattern\n");
	return;
    }

    /* mark the power pattern in the grid */
    if (!mark_as_front (power_pattern, FRONT1, 0)) { /* clean up */
	free_and_restore_wire_pattern (power_pattern);
	fprintf (stderr, "ERROR (internal): cannot set power pattern\n");
	return;
    }

    /* step along all points of power pattern which are in layer 0 */
    for (point = power_pattern; point; point = point->next)
    {
	if (point->z != 0) continue; /* not bottom layer */
	if (has_neighbour (point, D)) continue; /* a via is already there */
	if (has_neighbour (point, U)) continue; /* stacked.. */

	/* normal feed: connect it to power if it is free, or if
	 * it is already connected at another place
	 */
	if (CoreFeed[to_core(point->x,X)][to_core(point->y,Y)])
	{ /* there are feeds */
	    for (off = CoreFeed[to_core(point->x,X)][to_core(point->y,Y)]; off; off = off->next) {
		if (has_neighbour_offset (point, D, off))
		if ((Grid[point->z+off->z][point->y+off->y][point->x+off->x] & STATEMASK) != FRONT1)
		    break;  /* connects to alien pattern */
	    }
	    if (off) continue; /* alien found */
	    add_grid_neighbour (point, D); /* poke via */
	    continue;
	}

	/* restricted feed: connect it to power only if it is already connected */
	if (SaveRestrictedCoreFeed[to_core(point->x,X)][to_core(point->y,Y)]) { /* there are feeds */
	    for (off = SaveRestrictedCoreFeed[to_core(point->x,X)][to_core(point->y,Y)]; off; off = off->next) {
		if (has_neighbour_offset (point, D, off))
		if ((Grid[point->z+off->z][point->y+off->y][point->x+off->x] & STATEMASK) == FRONT1)
		    break; /* connects to power pattern */
	    }
	    if (!off) continue; /* no power found */
	    add_grid_neighbour (point, D); /* poke via */
	}
    } /* point->z == 0 */

    /* step along all points of power pattern which are in layer layers > 0 */
    for (point = power_pattern; point; point = point -> next) {
	if (point->z <= 0) continue; /* bottom layer */
	if (has_neighbour (point, D)) continue; /* a via is already there */
	if ((Grid[point->z-1][point->y][point->x] & STATEMASK) != FRONT1) continue; /* not of power */
	if (Grid[point->z-1][point->y][point->x] & Pat_mask[D]) continue; /* stacked */
	for (off = OffsetTable[to_core(point->x,X)][to_core(point->y,Y)][point->z]; off; off = off->next) {
	    if (off->direction == D) break;
	}
	if (!off) continue; /* offset was not found in list: via not allowed there */
	/* poke via */
	add_grid_neighbour (point, D); /* down */
	Grid[point->z-1][point->y][point->x] |= Pat_mask[U]; /* up */
    }

    free_gridpoint_list (power_pattern);
    unmark_power (exp_bbx);
}

/*
 * Make a temporary territory around some threatened terminals
 * assumes that global Grid is set.
 */
void make_piefjes (LAYOUTPTR father)
{
    NETPTR hnet;
    CIRPORTREFPTR hportref;
    R_PORTPTR rport;
    GRIDPOINT pointstruct;
    register GRIDPOINTPTR point;
    long ypos;
    int check_down, check_up;

    point = &pointstruct;

    if (strcmp (ThisImage, "fishbone") != 0) {
	fprintf (stderr, "WARNING: don't know how to make territories for image '%s'\n", ThisImage);
	return;
    }

    for (hnet = father->circuit->netlist; hnet; hnet = hnet->next)
    {
	/* walk along terminals */
	if (hnet->terminals == NULL || hnet->num_term < 2) continue;
	for (hportref = hnet->terminals; hportref; hportref = hportref->next)
	{
	    if ((rport = (R_PORTPTR) hportref->flag.p) == NULL) continue;  /* no struct: skip */

	    if (rport->crd[Z] != 0) continue; /* must be metal 1 and in middle */

	    /* y-coordinate in danger area?? */
	    ypos = rport->crd[Y] % GridRepitition[Y];
	    if (ypos != 6 && ypos != 7 && ypos != 21 && ypos != 22)
		continue; /* only on middle trans connect rail */

	    check_down = check_up = FALSE;
	    point->x = rport->crd[X];
	    point->y = rport->crd[Y];
	    point->z = rport->crd[Z];
	    if (ypos == 6 || ypos == 21) {
		check_down = TRUE;
		if (has_neighbour (point, T)) check_up = TRUE;
	    }
	    if (ypos == 7 || ypos == 22) {
		check_up = TRUE;
		if (has_neighbour (point, B)) check_down = TRUE;
	    }

	    if (check_down == TRUE) {
		/* add a piefje below, if possible or neccesary */
		point->y = rport->crd[Y] / GridRepitition[Y];
		point->y *= GridRepitition[Y];
		if (ypos == 6 || ypos == 7)
		    point->y += 5;
		else
		    point->y += 20;
		if (is_free (point)) { /* only condition: must be free */
		    add_to_territory (point, T, rport);
		}
	    }
	    if (check_up == TRUE) {
		/* add a piefje up, if possible or neccesary */
		point->y = rport->crd[Y] / GridRepitition[Y];
		point->y *= GridRepitition[Y];
		if (ypos == 6 || ypos == 7)
		    point->y += 8;
		else
		    point->y += 23;
		if (is_free (point)) { /* only condition: must be free */
		    add_to_territory (point, B, rport);
		}
	    }
	}
    }
}

static void add_to_territory (GRIDPOINTPTR point, int dir, R_PORTPTR rport)
{
    GRIDPOINTPTR wirepattern;

    /* occupy the grid */
    set_grid_occupied (point);
    add_grid_neighbour (point, dir);

    /* store in escape path
     * we keep wirepattern->pattern = 0, so a restore removes it
     * new_gridpoint() inits 'cost' and 'pattern' to 0
     */
    wirepattern = new_gridpoint (point->x, point->y, point->z);
    /* link */
    wirepattern->next = rport->territory;
    rport->territory = wirepattern;
}

/*
 * This routine removes all the terrirties if still present.
 */
void remove_territories (LAYOUTPTR father)
{
    NETPTR hnet;
    CIRPORTREFPTR hportref;
    R_PORTPTR rport;

    for (hnet = father->circuit->netlist; hnet; hnet = hnet->next) {
	/* walk along terminals */
	if (!hnet->terminals || hnet->num_term < 2) continue;
	for (hportref = hnet->terminals; hportref; hportref = hportref->next) {
	    if (!(rport = (R_PORTPTR) hportref->flag.p)) continue; /* no struct: skip */
	    remove_territory (rport);
	}
    }
}

/*
 * remove a single territory
 */
void remove_territory (R_PORTPTR rport)
{
    GRIDPOINTPTR dwire;

    while (rport->territory) {
	dwire = rport->territory;
	rport->territory = rport->territory->next;

	if (dwire->cost >= 0) set_grid_clear (dwire);
	free_gridpoint (dwire);
    }
}

/*
 * This routine erases the vertical power wires from grid
 * it also rmoves the substrate contacts.
 */
void delete_vertical_power (LAYOUTPTR lay, int delete_substrate)
/* delete_substrate - TRUE to delete substrate constacts */
{
    GRIDPOINT pointstruct;
    register GRIDPOINTPTR point;
    COREUNIT ***Grid;	/* the working grid */
    BOXPTR Bbx;		/* bounding box of working grid */
    register int off;	/* initial offsets of the two power lines */

    if (strcmp (ThisImage, "fishbone") != 0) {
	fprintf (stderr, "WARNING: don't know how to make power nets for image '%s'\n", ThisImage);
	return;
    }

    printf ("------ deleting vertical power ------\n");
    fflush (stdout);

    Grid = ((R_CELLPTR) lay->flag.p)->grid;
    Bbx = &((R_CELLPTR) lay->flag.p)->cell_bbx;
    point = &pointstruct;

    point->z = 0; /* metal 1 */
    for (off = Bbx->crd[B]; off + GridRepitition[Y] <= Bbx->crd[T]; off += (GridRepitition[Y]/2))
    {
	point->y = off;
	/* for all power rails.... */
	for (point->x = Bbx->crd[L]; point->x <= Bbx->crd[R]; point->x++)
	{
	    point->z = 0;
	    if (delete_substrate == TRUE && has_neighbour (point, D))
	    { /* if exists: remove substrate contact */
		Grid[point->z][point->y][point->x] =
		Grid[point->z][point->y][point->x] ^ Pat_mask[D]; /* exor it out... */
	    }
	    point->z = 1;    /* metal 2 */

	    for (point->y = off; point->y <= off + GridRepitition[Y]; point->y++)
	    { /* step upwards */
		if (is_free (point)) break; /* not occupied */
		if (has_neighbour (point, L) || has_neighbour (point, R))
		    break; /* left/right connection not allowed */
		if (point->y == off || point->y == off + GridRepitition[Y])
		{ /* bottom or top extreme */
		    if (!has_neighbour (point, D)) break; /* must have via down.. */
		    if (point->y == off && !has_neighbour (point, T))
			break; /* must have top neighbor if bottom-most */
		    if (point->y == off + GridRepitition[Y] && !has_neighbour (point, B))
			break; /* must have bottom neighbor if top-most */
		}
		else { /* ordinary point */
		    if (has_neighbour (point, D)) break; /* no via down.. */
		    if (!has_neighbour (point, T) || !has_neighbour (point, B))
			break; /* must have top and bottom neighbour */
		}
	    }

	    if (point->y > off + GridRepitition[Y]) { /* a 'clean' power line was found: erase it */
		point->y = off;
		if (has_neighbour (point, B))
		{ /* has bottom neighbour: keep the point, erase top pointer */
		    Grid[point->z][point->y][point->x] ^= Pat_mask[T];
		}
		else { /* completely erase it... */
		    set_grid_clear (point);
		    /* remove pointer upward of below */
		    point->z = 0;
		    if (has_neighbour (point, U)) Grid[point->z][point->y][point->x] ^= Pat_mask[U];
		    point->z = 1;
		}

		/* erase all intermediate points */
		for (point->y = off + 1; point->y < off + GridRepitition[Y]; point->y++)
		    set_grid_clear (point);

		/* last point */
		point->y = off + GridRepitition[Y];
		if (has_neighbour (point, T))
		{ /* participates in something else: only remove pointer */
		    Grid[point->z][point->y][point->x] ^= Pat_mask[B];
		}
		else {
		    set_grid_clear (point);
		    point->z = 0;
		    if (has_neighbour (point, U)) Grid[point->z][point->y][point->x] ^= Pat_mask[U];
		    point->z = 1;
		}
	    }
	}
    }
}

static void do_add_missing_term (LAYOUTPTR father, char *tname)
{
    LAYPORTPTR layport;
    CIRPORTPTR cirport;
    POWERLINEPTR pline;

    for (layport = father->layport; layport; layport = layport->next)
	if (layport->cirport->name == tname) return; /* it exits */

    /* the layport does not exist, make it! */
    /* find proper power line */
    for (pline = PowerLineList; pline; pline = pline->next) {
	if (pline->type == VDD && strcmp (tname, "vdd") == 0) break;
	if (pline->type == VSS && strcmp (tname, "vss") == 0) break;
    }
    if (!pline) return;

    for (cirport = father->circuit->cirport; cirport; cirport = cirport->next)
	if (cirport->name == tname) break;
    if (!cirport) {
	NewCirport (cirport);
	cirport->name = tname;
	cirport->next = father->circuit->cirport;
	father->circuit->cirport = cirport;
    }

    NewLayport (layport);
    if (pline->orient == HORIZONTAL) {
	layport->pos[X] = 0;
	layport->pos[Y] = pline->y;
    }
    else {
	layport->pos[X] = pline->x;
	layport->pos[Y] = 0;
    }
    layport->layer = pline->z;
    layport->cirport = cirport;
    layport->next = father->layport;
    father->layport = layport;
}

void add_missing_power_terms (LAYOUTPTR father)
{
    do_add_missing_term (father, cs("vss"));
    do_add_missing_term (father, cs("vdd"));
}

/*
 * This routing makes territories for the terminals,
 * that is, a piece of wire is allocated for each terminal,
 * shich should protect its routability.
 * if passno <= 2, only territories are added for nets
 * which failed before.
 */
void make_real_territories (LAYOUTPTR father, int only_failed_nets)
{
    NETPTR hnet;
    CIRPORTREFPTR hportref;
    R_PORTPTR rport;
    GRIDPOINT point;
    GRIDPOINTPTR p, path;
    BOX ebbx;

    if (strcmp (ThisImage, "fishbone") != 0) {
	fprintf (stderr, "WARNING: don't know how to make territories for image '%s'\n", ThisImage);
	return;
    }

    for (hnet = father->circuit->netlist; hnet; hnet = hnet->next)
    {
	/* walk along terminals */
	if (hnet->terminals == NULL || hnet->num_term < 2) continue;
	/* skip power nets... */
	if (((R_NETPTR)hnet->flag.p)->type == POWER) continue;

	if (only_failed_nets == TRUE && ((R_NETPTR)hnet->flag.p)->fail_count == 0)
	{ /* in lower passes:
	    skip a territory for the nets which did not fail before */
	    continue;
	}

	/* remove existing territories
	*/
	for (hportref = hnet->terminals; hportref; hportref = hportref->next) {
	    if (!(rport = (R_PORTPTR) hportref->flag.p)) continue; /* no struct: skip */
	    remove_territory (rport);
	}

	/* make new ones
	*/
	for (hportref = hnet->terminals; hportref; hportref = hportref->next)
	{
	    if (!(rport = (R_PORTPTR) hportref->flag.p)) continue; /* no struct: skip */

	    /* no territories for unassigned terminals */
	    if (rport->unassigned == TRUE) continue;

	    point.x = rport->crd[X];
	    point.y = rport->crd[Y];
	    point.z = rport->crd[Z];

	    ebbx.crd[L] = MAX (point.x - 6, Bbx->crd[L]);
	    ebbx.crd[R] = MIN (point.x + 6, Bbx->crd[R]);
	    /* to nearest power line */
	    ebbx.crd[B] = point.y / (GridRepitition[Y] / 2);
	    ebbx.crd[B] = (ebbx.crd[B] * GridRepitition[Y])/2;
	    ebbx.crd[T] = ebbx.crd[B] + (GridRepitition[Y]/2) - 1;
	    MIN_UPDATE (ebbx.crd[T], Bbx->crd[T]);
	    ebbx.crd[D] = -1;
	    ebbx.crd[U] = Bbx->crd[U];

	    if (!(path = lee_to_border (&point, &ebbx, TRUE))) {
		/* fprintf (stderr, "FAILED: %s\n", hportref->cirport->name); */
		continue;
	    }

	    for (p = path; p; p = p->next) {
		if (!is_free (p)) p->cost = -1;
		else if (p->cost != -1) restore_grid_pattern (p);
	    }
	    /* melt_new_wire (path); */
	    rport->territory = path; /* store the path */
	}
    }
}

/*
 * This routine makes all capacitors.
 */
void make_capacitors (LAYOUTPTR lay)
{
    int capsize, floor;

    Bbx = &((R_CELLPTR) lay->flag.p)->cell_bbx;

    if (check_power_capabilities (FALSE) == FALSE) return;

    if (verbose) {
	printf ("------ Making capacitors ------\n");
	fflush (stdout);
    }
    capsize = 0;

    if (strcmp (ThisImage, "fishbone") != 0) {
	fprintf (stderr, "WARNING: don't know how to make capacitors for image '%s'\n", ThisImage);
	return;
    }

    for (floor = 0; floor * GridRepitition[Y] <= Bbx->crd[T]; floor++) { /* for all power line rows */
	capsize += make_cap (lay, "vss", floor);
	if (verbose) { printf ("%d,", capsize); fflush (stdout); }
	capsize += make_cap (lay, "vdd", floor);
	if (verbose) { printf ("%d,", capsize); fflush (stdout); }
	if (floor%6 == 0 && verbose) printf ("\n");
    }

    if (capsize != 0 && floor%6 != 0 && verbose) printf ("\n");

    if (verbose) {
	printf ("------ Converted %d unused transistors into capacitors ------\n", capsize);
	fflush (stdout);
    }
}

/*
 * Make all the capacitors in one horizontal row between the power lines.
 */
static int make_cap (LAYOUTPTR lay, char *type, int floor)
/* type  - vss or vdd */
/* floor - floor number (e.g. 0 = 0th row of image) */
{
    GRIDPOINT struct1, struct2, struct3, struct4;
    GRIDPOINTPTR bot_gate, top_gate, met_conn, ground;
    GRIDPOINT pointstruct;
    register GRIDPOINTPTR point;
    GRIDPOINTPTR top_power_pattern, bot_power_pattern;
    BOXPTR exp_bbx;
    BOX exp_bbxstruct;
    COREUNIT state;
    register int x, y, z;
    int result, capsize, no_of_caps, bot_width, top_width;
    int off; /* initial offsets of the two power lines */

    Grid = ((R_CELLPTR) lay->flag.p)->grid;
    Bbx = &((R_CELLPTR) lay->flag.p)->cell_bbx;
    point = &pointstruct;
    exp_bbx = &exp_bbxstruct;
    bot_gate = &struct1;
    top_gate = &struct2;
    met_conn = &struct3;
    ground = &struct4;
    capsize = no_of_caps = 0;

    /* find the first column which is free in metal 2 */
    if (strcmp (type, "vss") == 0) { /* ground net */
	off = floor * GridRepitition[Y];
	bot_width = 6; top_width = 7;
    }
    else { /* power net */
	off = 14 + (floor * GridRepitition[Y]);
	bot_width = 7; top_width = 6;
    }

    /* set the bounding box for the capacitor */
    exp_bbx->crd[L] = Bbx->crd[L];
    exp_bbx->crd[R] = Bbx->crd[R];
    exp_bbx->crd[B] = MAX (Bbx->crd[B], off);
    exp_bbx->crd[T] = MIN (Bbx->crd[T], off + 14);
    exp_bbx->crd[D] = 0;
    exp_bbx->crd[U] = 1;

    /* need a free row */
    if (exp_bbx->crd[T] - exp_bbx->crd[B] < 14) return (0);

    /* set starting point of bottom power */
    point->x = (Bbx->crd[R] - Bbx->crd[L])/2;
    point->y = off;
    point->z = 0;

    /* find out pattern of power */
    if (!(bot_power_pattern = save_source (point, NULL, exp_bbx))) {
	fprintf (stderr, "ERROR (internal): cannot find power pattern\n");
	return (0);
    }

    /* mark the power pattern in the grid */
    if (!mark_as_front (bot_power_pattern, FRONT1, 0)) { /* clean up */
	free_and_restore_wire_pattern (bot_power_pattern);
	fprintf (stderr, "ERROR (internal): cannot set power pattern1\n");
	return (0);
    }

    /* set starting point of top power */
    point->x = (Bbx->crd[R] - Bbx->crd[L])/2;
    point->y = exp_bbx->crd[T];
    point->z = 0;

    /* find out pattern of power */
    if (!(top_power_pattern = save_source (point, NULL, exp_bbx))) {
	fprintf (stderr, "ERROR (internal): cannot find power pattern2\n");
	return (0);
    }

    /* mark the power pattern in the grid */
    if (!mark_as_front (top_power_pattern, FRONT2, 0)) { /* clean up */
	free_and_restore_wire_pattern (bot_power_pattern);
	free_and_restore_wire_pattern (top_power_pattern);
	fprintf (stderr, "ERROR (internal): cannot set power pattern3\n");
	return (0);
    }

    /*** sweep over the rail.. ***/

    bot_gate->z = top_gate->z = met_conn->z = ground->z = 0;

    /* find possible starting points, with power connections
    */
    for (x = exp_bbx->crd[L]; x < exp_bbx->crd[R]; x++)
    {
	bot_gate->x = top_gate->x = met_conn->x = ground->x = x;

	/* bottom side of the rail */
	ground->y = exp_bbx->crd[B];
	bot_gate->y = exp_bbx->crd[B] + 1;
	top_gate->y = exp_bbx->crd[B] + bot_width;
	met_conn->y = top_gate->y - 1;
	ground->pattern = FRONT1;

	if (is_state (met_conn->x, met_conn->y, 1, FRONT2) && is_free (met_conn))
	{ /* found starting point! */
	    result = right_step (bot_gate, top_gate, ground);
	    if (result > 0) { no_of_caps++; capsize += result; }
	}

	/* top side of the rail */
	ground->y = exp_bbx->crd[T];
	bot_gate->y = exp_bbx->crd[T] - 1;
	top_gate->y = exp_bbx->crd[T] - top_width;
	met_conn->y = top_gate->y + 1;
	ground->pattern = FRONT2;

	if (is_state (met_conn->x, met_conn->y, 1, FRONT1) && is_free (met_conn))
	{ /* found starting point! */
	    result = right_step (bot_gate, top_gate, ground);
	    if (result > 0) { no_of_caps++; capsize += result; }
	}
    }

    /* join neighbouring power wires in metal2
    */
    z = 1;
    for (y = exp_bbx->crd[B] + 1; y <= exp_bbx->crd[T]; y++)
    for (x = exp_bbx->crd[L] + 1; x <= exp_bbx->crd[R]; x++)
    {
	state = Grid[z][y][x] & STATEMASK;

	if (state != FRONT1 && state != FRONT2) continue; /* must be front */
	/* below */
	if (is_state (x, y-1, z, state)) { /* they're the same power */
	    Grid[z][y][x] |= Pat_mask[B];
	    Grid[z][y-1][x] |= Pat_mask[T];
	}
	/* left */
	if (is_state (x-1, y, z, state)) { /* they're the same power */
	    Grid[z][y][x] |= Pat_mask[L];
	    Grid[z][y][x-1] |= Pat_mask[R];
	}
    }

    /* reset the grid to normal pattern
    */
    for (x = exp_bbx->crd[L]; x <= exp_bbx->crd[R]; x++)
    for (y = exp_bbx->crd[B]; y <= exp_bbx->crd[T]; y++)
    for (z = 0; z <= exp_bbx->crd[U]; z++) {
	state = Grid[z][y][x] & STATEMASK;
	if (state == FRONT1 || state == FRONT2)
	    Grid[z][y][x] |= STATEMASK; /* part of power, set to occupied */
    }

    free_gridpoint_list (bot_power_pattern);
    free_gridpoint_list (top_power_pattern);

    return (capsize);
}

/* return values for left_step and right_step */
#define TERMINATOR          0
#define MUST_BE_TERMINATED  -1

/*
 * Recursive routing to step to the right from a starting point,
 * each time checking whether it is possible to make the capacitor.
 * if it hits the end, it will call the right step to make
 * the acutal capacitor.
 * If it returns > 0, a capacitor was made. The valkue is
 * the size of the capacitor.
 */
static int right_step (GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate, GRIDPOINTPTR ground)
/* bot_gate - bottom of gate */
/* top_gate - top of gate */
/* ground   - points to ground gate (the one below bottom gate) */
{
    GRIDPOINT struct1, struct2, struct3;
    GRIDPOINTPTR bot_gate2, top_gate2, ground2;
    int result;

    bot_gate2 = &struct1; top_gate2 = &struct2; ground2 = &struct3;
    bot_gate2->x = top_gate2->x = ground2->x = bot_gate->x + 1;
    bot_gate2->y = bot_gate->y; top_gate2->y = top_gate->y; ground2->y = ground->y;
    bot_gate2->z = top_gate2->z = ground2->z = bot_gate->z;
    ground2->pattern = ground->pattern;

    /* 1 */
    if (source_is_usable (bot_gate, top_gate, ground) == FALSE) {
	/* the source is connected to a foreign pattern or is not connectable */
	return (MUST_BE_TERMINATED);
    }

    /* 2 */
    if (gate_is_connected (bot_gate, top_gate) == TRUE) { /* the gate is already in use */
	if (gate_to_ground (bot_gate, top_gate, ground) == TRUE) return (TERMINATOR);
	return (MUST_BE_TERMINATED);
    }

    /* 3 */
    if (bot_gate->x >= Bbx->crd[R] - 1) { /* try to terminate */
	if (terminate_gate (FALSE, bot_gate, top_gate, ground) == TRUE) return (TERMINATOR);
	return (MUST_BE_TERMINATED);
    }

    /* could be used, recursive step to right */
    result = right_step (bot_gate2, top_gate2, ground2);

    if (result == MUST_BE_TERMINATED) { /* try to terminate */
	if (terminate_gate (FALSE, bot_gate, top_gate, ground) == TRUE) return (TERMINATOR);
	return (MUST_BE_TERMINATED);
    }

    if (result != TERMINATOR) return (result);

    /* we are at a possible rightmost point of a capacitor */
    /* probe only, see how big it is */
    result = left_step (TRUE, bot_gate, top_gate, ground);

    if (result < 1) return (TERMINATOR);

    /* big enough: do it */
    result = left_step (FALSE, bot_gate, top_gate, ground);
    return (result);
}

/*
 * Recursive routing to step to the left, each time checking
 * whether it is possible to make the capacitor.
 * If probe_only == TRUE, the capacitor is made.
 */
static int left_step (int probe_only, GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate, GRIDPOINTPTR ground)
/* probe_only - TRUE to only probe the position */
/* bot_gate   - bottom of gate */
/* top_gate   - top of gate */
/* ground     - points to ground gate (the one below bottom gate) */
{
    GRIDPOINT struct1, struct2, struct3;
    GRIDPOINTPTR bot_gate2, top_gate2, ground2;
    COREUNIT power_pattern, groundpattern;
    int numc, dir, result;

    bot_gate2 = &struct1; top_gate2 = &struct2; ground2 = &struct3;
    bot_gate2->x = top_gate2->x = ground2->x = bot_gate->x - 1;
    bot_gate2->y = bot_gate->y; top_gate2->y = top_gate->y; ground2->y = ground->y;
    bot_gate2->z = top_gate2->z = ground2->z = bot_gate->z;
    ground2->pattern = ground->pattern;

    groundpattern = ground->pattern & STATEMASK;
    power_pattern = groundpattern == FRONT1 ? FRONT2 : FRONT1;

    /* 1 */
    if (gate_is_connected (bot_gate, top_gate) == TRUE) { /* the gate is already in use */
	if (gate_to_ground (bot_gate, top_gate, ground) == TRUE) return (TERMINATOR);
	return (MUST_BE_TERMINATED);
    }

    /* 2 */
    if (source_is_usable (bot_gate, top_gate, ground) == FALSE)
    { /* the source is connected to a foreign pattern or is not connectable */
	if (terminate_gate (probe_only, bot_gate, top_gate, ground) == TRUE) return (TERMINATOR);
	return (MUST_BE_TERMINATED);
    }

    /* 3 */
    if (bot_gate->x <= Bbx->crd[L]) { /* walks left of bounding box... */
	if (terminate_gate (probe_only, bot_gate, top_gate, ground) == TRUE) return (TERMINATOR);
	return (MUST_BE_TERMINATED);
    }

    /* 4 */
    if (gate_is_powerable (bot_gate, top_gate, ground) == FALSE)
    { /* the gate cannot be connected to power */
	if (terminate_gate (probe_only, bot_gate, top_gate, ground) == TRUE) return (TERMINATOR);
	return (MUST_BE_TERMINATED);
    }

    /* 5 */
    /* at this point we could use this transistor for the capacitor,
     * but that depends on it left neighbour...
     */
    result = left_step (probe_only, bot_gate2, top_gate2, ground2);

    if (result == MUST_BE_TERMINATED) { /* can't use this for cap, must terminate... */
	if (terminate_gate (probe_only, bot_gate, top_gate, ground) == TRUE) return (TERMINATOR);
	return (MUST_BE_TERMINATED);
    }

    /* YIPPEEEEEE, we add this transistor to the cap! */
    if (probe_only == TRUE) return (result + 1);

    if (bot_gate->y < top_gate->y) dir = 1; else dir = -1;

    /* 2nd power wire */
    top_gate2->x = top_gate->x;
    top_gate2->y = top_gate->y - dir;
    top_gate2->z = top_gate->z;

    /* walking point over source */
    bot_gate2->x = bot_gate->x;
    bot_gate2->y = bot_gate->y + dir;
    bot_gate2->z = bot_gate->z;

    /* 1: connect the gate
    */
    set_grid_pattern (top_gate, power_pattern);
    add_grid_neighbour (top_gate, D);
    if (result > 0) { /* connect to left */
	add_grid_neighbour (top_gate, L);
	Grid[top_gate->z][top_gate->y][top_gate->x - 1] |= Pat_mask[R];
    }

    /* wire below the power..
    */
    if (is_free (top_gate2) || is_state (top_gate2->x, top_gate2->y, top_gate2->z, power_pattern))
    { /* can make wire below power */
	unsigned char hh;
	if (is_free (top_gate2)) set_grid_pattern (top_gate2, power_pattern);
	if (dir == 1) {
	    add_grid_neighbour (top_gate, B);
	    add_grid_neighbour (top_gate2, T);
	}
	else {
	    add_grid_neighbour (top_gate, T);
	    add_grid_neighbour (top_gate2, B);
	}

	hh = Grid[0][top_gate2->y][top_gate2->x - 1];
	hh = hh & STATEMASK;
	if (hh == power_pattern) {
	    add_grid_neighbour (top_gate2, L);
	    Grid[top_gate2->z][top_gate2->y][top_gate2->x - 1] |= Pat_mask[R];
	}

	/* let's connect it to power if possible */
	if (is_state (top_gate2->x, top_gate2->y, 1, power_pattern) &&
		!has_neighbour (top_gate2, D)) { /* do it */
	    add_grid_neighbour (top_gate2, U);
	    Grid[1][top_gate2->y][top_gate2->x] |= Pat_mask[D];
	}
    }

    /* 2: connect the source
    */
    numc = 0;
    for (bot_gate2->y = top_gate->y - (2 * dir); bot_gate2->y != bot_gate->y; bot_gate2->y -= dir)
    {
	if (!is_state (bot_gate2->x, bot_gate2->y, bot_gate2->z, groundpattern)) continue; /* not power */
	if (has_neighbour (bot_gate2, U)) continue; /* stacked contact */

	/* poke, but not always */
	if (numc < 2) {
	    add_grid_neighbour (bot_gate2, D);
	    numc++;
	    continue;
	}
	if (is_state (bot_gate2->x, bot_gate2->y, 1, groundpattern)) { /* make power-metal2 contact */
	    if (!has_neighbour (bot_gate2, D)) {
		add_grid_neighbour (bot_gate2, U);
		Grid[1][bot_gate2->y][bot_gate2->x] |= Pat_mask[D];
		numc = 0;
	    }
	    continue;
	}

	add_grid_neighbour (bot_gate2, D);
	numc++;
    }

    return (result + 1);
}

/*
 * TRUE if the source pattern is connected to any net which is not ground
 */
static int source_is_usable (GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate, GRIDPOINTPTR ground)
/* bot_gate - bottom of gate */
/* top_gate - top of gate */
/* ground   - points to ground gate (the one below bottom gate) */
{
    register GRIDADRESSUNIT x, y, z;
    int dir, groundable, is_gstate;

    x = bot_gate->x;
    y = bot_gate->y;
    z = bot_gate->z;

    if (y < top_gate->y)
	dir = 1; /* upwards, normal case */
    else
	dir = -1; /* downwards */

    /* step over source pattern */
    groundable = FALSE;
    for (y = y + dir; y != top_gate->y; y += dir) {
	is_gstate = (Grid[z][y][x] & STATEMASK) == (ground->pattern & STATEMASK);
	if (grid_neighbour (x, y, z, D)) return (is_gstate ? TRUE : FALSE);
	if (is_gstate && !grid_neighbour (x, y, z, U)) groundable = TRUE;
    }

    /* entire source not connected */
    return (groundable);
}

/*
 * TRUE if the gate is connected
 */
static int gate_is_connected (GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate)
/* bot_gate - bottom of gate */
/* top_gate - top of gate */
{
    if (has_neighbour (bot_gate, D)) return (TRUE);
    if (has_neighbour (top_gate, D)) return (TRUE);
    return (FALSE);
}

/*
 * TRUE if the gate is connected
 */
static int gate_is_powerable (GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate, GRIDPOINTPTR ground)
/* bot_gate - bottom of gate */
/* top_gate - top of gate */
/* ground   - points to ground gate (the one below bottom gate) */
{
    if (is_free (top_gate)) return (TRUE);
    return (FALSE);
}

/*
 * TRUE if the gate is connected and to ground
 */
static int gate_to_ground (GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate, GRIDPOINTPTR ground)
/* bot_gate - bottom of gate */
/* top_gate - top of gate */
/* ground   - points to ground gate (the one below bottom gate) */
{
    COREUNIT gstate = ground->pattern & STATEMASK;

    if (has_neighbour (bot_gate, D) && is_state (bot_gate->x, bot_gate->y, bot_gate->z, gstate)) return (TRUE);
    if (has_neighbour (top_gate, D) && is_state (top_gate->x, top_gate->y, top_gate->z, gstate)) return (TRUE);
    return (FALSE);
}

/*
 * TRUE if the gate is connectable to the ground pattern
 * if probe_only == false, the gate will actually be terminated
 */
static int terminate_gate (int probe_only, GRIDPOINTPTR bot_gate, GRIDPOINTPTR top_gate, GRIDPOINTPTR ground)
/* bot_gate - bottom of gate */
/* top_gate - top of gate */
/* ground   - points to ground gate (the one below bottom gate) */
{
    COREUNIT gstate = ground->pattern & STATEMASK;

    /* this was already done, just to be sure.. */
    if (gate_is_connected (bot_gate, top_gate) == TRUE) {
	if (gate_to_ground (bot_gate, top_gate, ground) == TRUE) return (TRUE);
	return (FALSE);
    }

    if (is_state (bot_gate->x, bot_gate->y, bot_gate->z, gstate))
    { /* a ground pattern is floating over the bottom gate connection */
	if (probe_only == FALSE) add_grid_neighbour (bot_gate, D); /* poke via */
	return (TRUE);
    }

    if (is_free (bot_gate))
    { /* the gate pattern is still free: make via and add terminate it */
	set_grid_pattern (bot_gate, gstate);
	add_grid_neighbour (bot_gate, D);
	if (bot_gate->y < top_gate->y) {
	    add_grid_neighbour (bot_gate, B);
	    add_grid_neighbour (ground, T);
	}
	else {
	    add_grid_neighbour (bot_gate, T);
	    add_grid_neighbour (ground, B);
	}
	return (TRUE);
    }
    return (FALSE);
}
