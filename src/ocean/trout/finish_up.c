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
 * connect all unused transistors
 */

#include "src/ocean/trout/typedef.h"
#include "src/ocean/trout/grid.h"
#include "src/ocean/libseadif/sealibio.h"

static void mk_overlap_wire (LAYOUTPTR lay, LAYOUTPTR overlapcell, GRIDPOINTPTR point, int dir);
static void mk_overlap_wires (LAYOUTPTR lay, LAYOUTPTR overlapcell);

extern long
   GridRepitition[2];      /* repitionvector (dx, dy) of grid core image (in grid points) */
extern GRIDPOINTPTR
   **CoreFeed;             /* matrix of feedthroughs in basic image */
extern char *ThisImage;    /* Seadif name of this image */
extern int verbose;
extern COREUNIT
   Pat_mask[HERE+1];       /* look-up table for bit-patterns */

extern GRIDADRESSUNIT
   Xoff[HERE+1],           /* look-up tables for offset values */
   Yoff[HERE+1],
   Zoff[HERE+1];

extern POWERLINEPTR PowerLineList; /* list of template power lines */
extern BOXPTR Rbbx;        /* routing bounding box */

/*
 * This routine finishes up the routing, by connecting all
 * unconnected transistors to the power net.
 */
void connect_unused_transistors (LAYOUTPTR lay)
{
    R_CELLPTR rcell;
    register int x, y, y2;
    register COREUNIT b, ob;
    COREUNIT ***Grid; /* the working grid */
    GRIDPOINTPTR off;
    int connected;
    POWERLINEPTR pline;
    int x2;

    if (!(rcell = (R_CELLPTR) lay->flag.p)) return;

    if (!(Grid = rcell->grid)) return;

    if (check_power_capabilities (FALSE) == FALSE) {
	fprintf (stderr, "WARNING, connecting unused transistors not supported for this image.\n");
	return;
    }

    if (verbose) {
	printf ("------ connecting unused transistors to power ------\n");
	fflush (stdout);
    }

    for (pline = PowerLineList; pline; pline = pline->next)
    {
	for (y2 = rcell->cell_bbx.crd[B] + pline->y; y2 <= rcell->cell_bbx.crd[T]; y2 += GridRepitition[Y])
	{ /* for all power line rows */
	    /*
	     * connect unused transistors to power/ground
	     */
	    for (y = y2 - 1; y <= y2 + 1; y += 2)
	    { /* one row below and one row above */
		if (y < rcell->cell_bbx.crd[B] || y > rcell->cell_bbx.crd[T] ||
		    (Rbbx && y < Rbbx->crd[B]) || (Rbbx && y > Rbbx->crd[T])) continue;

		if (y == y2 - 1) { /* below */
		    b  = STATEMASK + (1<<T) + (1<<D);
		    ob = STATEMASK + (1<<B);
		}
		else {
		    b  = STATEMASK + (1<<B) + (1<<D);
		    ob = STATEMASK + (1<<T);
		}

		x  = rcell->cell_bbx.crd[L];
		x2 = rcell->cell_bbx.crd[R];
		if (Rbbx) {
		    MAX_UPDATE (x,  Rbbx->crd[L]);
		    MIN_UPDATE (x2, Rbbx->crd[R]);
		}

		for (; x <= x2; ++x)
		{
		    if (!(off = CoreFeed[to_core(x,X)][to_core(y,Y)])) continue; /* if no feeds there */

		    /* step through offsets */
		    if (Grid[0][y][x] != 0) continue; /* already connected */

		    connected = FALSE;

		    for (; off; off = off->next) { /* step through offset */
			if (y + off->y >= rcell->gridsize[Y] ||
			    x + off->x >= rcell->gridsize[X]) continue; /* outside image */

			if (Grid[0][y + off->y][x + off->x] & (1<<D)) connected = TRUE;
		    }
		    if (connected == TRUE) continue;

		    /* make connection */
		    Grid[0][y][x] |= b;
		    Grid[0][y2][x] |= ob;
		}
	    }
	}
    }
}

/*
 * This routine pokes the substrate contacts in the grid.
 * if all_of_them == TRUE as many as possible will be connected,
 * otherwise, once every three are connected.
 */
void make_substrate_contacts (LAYOUTPTR lay, int all_of_them)
{
    R_CELLPTR rcell;
    register int x, y2, pitch;
    COREUNIT ***Grid; /* the working grid */
    POWERLINEPTR pline;
    int xL, xR, yB, yB2, yT;

    if (!(rcell = (R_CELLPTR) lay->flag.p)) return;

    if (!(Grid = rcell->grid)) return;

    if (check_power_capabilities (FALSE) == FALSE) {
	fprintf (stderr, "WARNING, substrate contacts not supported this image\n");
	return;
    }

    if (verbose && all_of_them) {
	printf ("------ making substrate contacts ------\n");
	fflush (stdout);
    }

    if (all_of_them) pitch = 1; else pitch = 3;

    xL = rcell->cell_bbx.crd[L];
    xR = rcell->cell_bbx.crd[R];
    yB = rcell->cell_bbx.crd[B];
    yT = rcell->cell_bbx.crd[T];
    if (Rbbx) {
	MAX_UPDATE (xL, Rbbx->crd[L]);
	MIN_UPDATE (xR, Rbbx->crd[R]);
	MIN_UPDATE (yT, Rbbx->crd[T]);
	yB2 = Rbbx->crd[B];
    }
    else yB2 = yB;

    for (pline = PowerLineList; pline; pline = pline->next)
    {
	if (pline->z != 0) continue;

	for (y2 = yB + pline->y; y2 <= yT; y2 += GridRepitition[Y]) { /* for all power line rows */
	    /*
	     * make substrate contacts every 3 positions
	     */
	    if (y2 < yB2) continue;

	    for (x = xL; x <= xR; x += pitch) {
		/* check for stracked vias */
		if (Grid[0][y2][x] & (1<<U)) {
		    /* it is connected by metal2 wire: try neighbours */
		    if (pitch <= 1) continue;
		    if (x == xL || (Grid[0][y2][x-1] & (1<<U))) {
			if (x == xR || (Grid[0][y2][x+1] & (1<<U)))
			    continue; /* exhausted */
			else
			    Grid[0][y2][x+1] |= (1<<D);
		    }
		    else
			Grid[0][y2][x-1] |= (1<<D);
		}
		else
		    Grid[0][y2][x] |= (1<<D);
	    }
	}
    }
}

/*
 * This routine makes the overlap cell which is
 * attached as an instance to the father cell.
 */
void mk_overlap_cell (LAYOUTPTR lay)
{
    LAYOUTPTR overlapcell;

    if (verbose) {
	printf ("\n------ making overlap elements ------\n");
	fflush (stdout);
    }

    overlapcell = lay;

    /* OK! make the overlap wires */
    mk_overlap_wires (lay, overlapcell);
}

/*
 * This routine looks for overlapping wires between metal1 and metal2
 * the wires are attached to overlapcell.
 */
static void mk_overlap_wires (LAYOUTPTR lay, LAYOUTPTR overlapcell)
{
    COREUNIT ***Grid; /* the grid of this cell */
    BOX boxspace;
    BOXPTR Bbx;       /* bbx of this cell */
    GRIDPOINT pointstruct, pointstruct2;
    GRIDPOINTPTR point, point2;
    R_CELLPTR rcell;
    WIREPTR wire;

    point = &pointstruct;
    point2 = &pointstruct2;

    if (!(rcell = (R_CELLPTR) lay->flag.p)) {
	fprintf (stderr, "ERROR: no grid\n");
	return;
    }

    Grid = rcell->grid; /* set the grid */
    Bbx = &rcell->cell_bbx;

    if (Rbbx) {
	BOXPTR Box = &boxspace;
	Box->crd[L] = MAX (Bbx->crd[L], Rbbx->crd[L]);
	Box->crd[B] = MAX (Bbx->crd[B], Rbbx->crd[B]);
	Box->crd[D] = MAX (Bbx->crd[D], Rbbx->crd[D]);
	Box->crd[R] = MIN (Bbx->crd[R], Rbbx->crd[R]);
	Box->crd[T] = MIN (Bbx->crd[T], Rbbx->crd[T]);
	Box->crd[U] = MIN (Bbx->crd[U], Rbbx->crd[U]);
	Bbx = Box;
    }

    if (Bbx->crd[U] < 1) {
	fprintf (stderr, "WARNING, ovelap wires requires at least two layers\n");
	return;
    }

    /*
     * write maximal horizontal wires
     */
    point->z = 0;
    point2->z = 1;
    for (point->y = Bbx->crd[B]; point->y <= Bbx->crd[T]; point->y++)
    { /* for all rows */
	point2->y = point->y;
	for (point->x = Bbx->crd[L]; point->x <= Bbx->crd[R]; point->x++)
	{ /* for all points */
	    point2->x = point->x;
	    if (grid_is_occupied (point) && grid_is_occupied (point2) )
	    { /* overlap! */
		if ((point->x == Bbx->crd[L] ||
		    !has_neighbour (point, L) || !has_neighbour (point2, L)) &&
		    has_neighbour (point, R) && has_neighbour (point2, R))
		{
		    mk_overlap_wire (lay, overlapcell, point, R);
		}
	    }
	}
    }


    /*
     * vertical wires
     */
    point->z = 0;
    point2->z = 1;
    for (point->x = Bbx->crd[L]; point->x <= Bbx->crd[R]; point->x++)
    { /* for all cols */
	point2->x = point->x;
	for (point->y = Bbx->crd[B]; point->y <= Bbx->crd[T]; point->y++)
	{ /* for all points */
	    point2->y = point->y;
	    if (grid_is_occupied (point) && grid_is_occupied (point2) )
	    { /* overlap! */
		if ((point->x == Bbx->crd[B] ||
		    !has_neighbour (point, B) || !has_neighbour (point2, B)) &&
		    has_neighbour (point, T) && has_neighbour (point2, T))
		{
		    mk_overlap_wire (lay, overlapcell, point, T);
		}
	    }
	}
    }

    /*
     * overlap on top of poly gate
     */
    point->z = 0;
    point2->z = 1;
    for (point->y = Bbx->crd[B]; point->y <= Bbx->crd[T]; point->y++)
    { /* for all rows */
	if (!CoreFeed[0][to_core(point->y,Y)]) continue; /* only if row has poly feed */
	point2->y = point->y;
	for (point->x = Bbx->crd[L]; point->x <= Bbx->crd[R]; point->x++)
	{ /* for all points */
	    point2->x = point->x;
	    if (has_neighbour (point, D) &&
		grid_is_occupied (point) &&
		grid_is_occupied (point2) )
	    {
		NewWire (wire);
		wire->crd[L] = wire->crd[R] = point->x;
		wire->crd[B] = wire->crd[T] = point->y;
		wire->layer = 202; /* metal2 */
		wire->next = overlapcell->wire;
		overlapcell->wire = wire;
	    }
	}
    }
}

/*
 * This routine writes an as long as possible strip in the direction dir.
 */
static void mk_overlap_wire (LAYOUTPTR lay, LAYOUTPTR overlapcell, GRIDPOINTPTR point, int dir)
{
    COREUNIT ***Grid; /* the grid of this cell */
    BOXPTR Bbx;       /* bbx of this cell */
    GRIDPOINT pointstruct, pointstruct2;
    GRIDPOINTPTR npoint, npoint2;
    int neighbour_found;
    WIREPTR wire;

    /* set the grid */
    Grid = ((R_CELLPTR) lay->flag.p)->grid;
    Bbx = &((R_CELLPTR) lay->flag.p)->cell_bbx;

    npoint = &pointstruct;
    npoint->z = 0;
    npoint->y = point->y;
    npoint->x = point->x;

    npoint2 = &pointstruct2;
    npoint2->z = 1;
    npoint2->y = point->y;
    npoint2->x = point->x;

    NewWire (wire);
    wire->crd[L] = wire->crd[R] = point->x;
    wire->crd[B] = wire->crd[T] = point->y;
    wire->layer = 201;  /* dummy layer code + 1 = metal 1*/

    neighbour_found = FALSE;

    /*
    * processed is TRUE if the gridpoint is aleady covered by a previous wire
    */

    while (has_neighbour (npoint, dir) && has_neighbour (npoint2, dir))
    {
	step_point (npoint, dir);
	step_point (npoint2, dir);

	if (npoint->x > Bbx->crd[R] || npoint->y > Bbx->crd[T]) break;

	neighbour_found = TRUE;

	/* set new rectangle coords */
	wire->crd[R] = npoint->x;
	wire->crd[T] = npoint->y;
    }

    if (neighbour_found == TRUE) { /* link */
	wire->next = overlapcell->wire;
	overlapcell->wire = wire;
    }
    else {
	FreeWire (wire); /* useless */
    }
}
