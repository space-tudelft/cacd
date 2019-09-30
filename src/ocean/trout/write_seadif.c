/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Patrick Groeneveld
 *	Paul Stravers
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
 * write grid back into seadif
 */

#include <unistd.h>
#include <sys/types.h>
#include "src/ocean/trout/typedef.h"
#include "src/ocean/trout/grid.h"
#include "src/ocean/libseadif/sealibio.h"

static void create_wires (LAYOUTPTR lay);
static void fill_holes (LAYOUTPTR lay);
static void link_fat_wires (LAYOUTPTR father);
static void mk_wire (LAYOUTPTR lay, GRIDPOINTPTR point, int dir);
static void process_grid (LAYOUTPTR lay);
static void set_bbx (LAYOUTPTR lay);

extern LAYOUTPTR thislay;

extern COREUNIT
   Pat_mask[HERE+1];         /* look-up table for bit-patterns */

extern GRIDADRESSUNIT
   Xoff[HERE+1],           /* look-up tables for offset values */
   Yoff[HERE+1],
   Zoff[HERE+1];

extern int verbose;
extern long
   Overlap_cell,
   HaveMarkerLayer,        /* TRUE=1 if marker layer to indicate unconnect */
   **ViaIndex;             /* Index of via to core image in array ViaCellName */

/*
 * This routine writes the layout of father back into the database
 */
void write_seadif (LAYOUTPTR lay, int overlap_wires, int flood_holes)
/* lay           - father cell which is to be written */
/* overlap_wires - True to add fat wires at overlap of Metal1-metal2 */
/* flood_holes   - True to fill the 'holes' between wires, to prevent a mesh */
{
    char hostname[40], author[100];

    if (!lay->flag.p) {
	fprintf (stderr, "ERROR (write_seadif): no cell struct.\n");
	return;
    }

    /* cut of any existing wires in the cell */
    lay->wire = NULL;

    /* if requested: make overlap cell for proper designrules */
    if (overlap_wires == TRUE) mk_overlap_cell (lay);

    /* if requested: make fat wires for critical nets */
    if (overlap_wires == TRUE) link_fat_wires (lay);

    /* if requested: fill the 'holes' between wires
     * to prevent a mesh of wires. This is done before the son-cells are erased
     */
    if (flood_holes == TRUE) fill_holes (lay);

    /* erase all son-cells from the grid
     */
    read_seadif_wires_into_grid (((R_CELLPTR) lay->flag.p)->grid, lay,
		1, 0, 0, 0, 1, 0, &((R_CELLPTR) lay->flag.p)->cell_bbx, FALSE);

    /* purify what's left over of the grid */
    process_grid (lay);

    /* convert the grid into wire statements */
    create_wires (lay);

    /* default: fill the 'holes' between wires
     * to prevent a mesh of wires. This is AFTER son-cells are erased.
     */
    if (flood_holes == FALSE) fill_holes (lay);

    /* correct the bounding box of this cell */
    set_bbx (lay);

    /* set proper author string...
     */
    if (!lay->status) NewStatus (lay->status);
    lay->status->program = cs ("trout");

    if (gethostname (hostname, 40) != 0) {
	error (WARNING, "write_seadif/gethostname");
	strcpy (hostname, "unknown");
    }

    sprintf (author, "by user; %s:%s", hostname, sdfgetcwd ());
    lay->status->author = cs (author);

    /* write the result away into seadif
     */
    sdfwritelay (SDFLAYALL, lay);
}

/*
 * This routine writes the error cell, if necessary
 */
void write_error_cell (LAYOUTPTR lay)
/* lay - father cell of which error is to be written */
{
    LAYOUTPTR errorcell;
    LAYINSTPTR linst;

    /*
     * write the errorcell, if it exists
     */
    if (!(errorcell = ((R_CELLPTR) lay->flag.p)->error)) return;

    if (!errorcell->slice || (!errorcell->slice->chld.layinst && !errorcell->wire)) return;

    /*
     * write it, instance found !
     */

    if (verbose) {
	printf ("\n------ writing errors into cell '%s(%s(%s(%s)))' ------\n",
		errorcell->name,
		errorcell->circuit->name,
		errorcell->circuit->function->name,
		errorcell->circuit->function->library->name);
	fflush (stdout);
    }

    /*
     * 1: write unconnect cell
     *
    {
	LAYOUTPTR unconnect;
	if ((unconnect = ((R_CELLPTR) errorcell->flag.p)->error)) sdfwritelay (SDFLAYALL, unconnect);
    }
    */

    /* to be sure: set actual bbx coordinates */
    errorcell->bbx[X] = lay->bbx[X];
    errorcell->bbx[Y] = lay->bbx[Y];
    errorcell->off[X] = lay->off[X];
    errorcell->off[Y] = lay->off[Y];

    /*
     * add model call to lay itself
     */
    NewLayinst (linst);
    linst->name = canonicstring (lay->name);
    linst->layout = lay;
    linst->mtx[0] = 1; linst->mtx[1] = 0; linst->mtx[2] = 0;
    linst->mtx[3] = 0; linst->mtx[4] = 1; linst->mtx[5] = 0;
    linst->next = errorcell->slice->chld.layinst;
    errorcell->slice->chld.layinst = linst;

    sdfwritelay (SDFLAYALL, errorcell);
}

/*
 * This routine is a nasty 'purifier' which makes sure that
 * if a gridpoint has a pointer to a neighbour, this neighbour
 * has also a pointer to the gridpoint.
 */
static void process_grid (LAYOUTPTR lay)
{
    COREUNIT ***Grid; /* the grid of this cell */
    BOXPTR Bbx;       /* bbx of this cell */
    GRIDPOINT pointstruct;
    register GRIDPOINTPTR point;
    register int offset;
    int opp;

    point = &pointstruct;

    if (!lay -> flag.p) {
	fprintf (stderr, "ERROR: no grid\n");
	return;
    }

    /* set the grid */
    Grid = ((R_CELLPTR) lay -> flag.p) -> grid;
    Bbx = &((R_CELLPTR) lay -> flag.p) -> cell_bbx;

    /*
     * restore the grid
     */
    for (point->z = Bbx->crd[U] + HaveMarkerLayer; point->z >= Bbx->crd[D]; point->z--)
    { /* for all layers */
	for (point->y = Bbx->crd[B]; point->y <= Bbx->crd[T]; point->y++)
	{ /* for all rows */
	    for (point->x = Bbx->crd[L]; point->x <= Bbx->crd[R]; point->x++)
	    { /* for all points */

		if (is_free (point)) continue;

		for_all_offsets (offset) {
		    if ((offset == D && point->z == Bbx->crd[D]) ||
			(offset == U && point->z >= Bbx->crd[U]) ||
			(offset == D && point->z >  Bbx->crd[U]) ||
			(offset == L && point->x == Bbx->crd[L]) ||
			(offset == R && point->x == Bbx->crd[R]) ||
			(offset == B && point->y == Bbx->crd[B]) ||
			(offset == T && point->y == Bbx->crd[T])) continue; /* outside image */

		    if (has_neighbour (point, offset)) {
			opp = opposite (offset);
			if (!has_neighbour_o (point, opp, offset))
			{ /* opposite pointer missing!: add */
			    Grid[point->z + Zoff[offset]][point->y + Yoff[offset]][point->x + Xoff[offset]] |=
				(STATEMASK | Pat_mask[opp]);
			}
		    }
		}
	    }
	}
    }
}

/*
 * this routine converts the contents of the grid into wire statements
 */
static void create_wires (LAYOUTPTR lay)
{
    COREUNIT ***Grid; /* the grid of this cell */
    BOXPTR Bbx;       /* bbx of this cell */
    GRIDPOINT rtstruct, pointstruct;
    register GRIDPOINTPTR rt, point;
    WIREPTR wire;

    point = &pointstruct;
    rt = &rtstruct;

    if (!lay -> flag.p) {
	fprintf (stderr, "ERROR: no grid\n");
	return;
    }

    /* set the grid */
    Grid = ((R_CELLPTR) lay -> flag.p) -> grid;
    Bbx = &((R_CELLPTR) lay -> flag.p) -> cell_bbx;

    /*
     * write maximal horizontal wires
     */
    for (point->z = Bbx->crd[U] + HaveMarkerLayer; point->z >= Bbx->crd[D]; point->z--)
    { /* for all layers */
	for (point->y = Bbx->crd[B]; point->y <= Bbx->crd[T]; point->y++)
	{ /* for all rows */
	    for (point->x = Bbx->crd[L]; point->x <= Bbx->crd[R]; point->x++)
	    { /* for all points */
		if (grid_is_occupied (point)) { /* part of path */
		    if ((point->x == Bbx->crd[L] || !has_neighbour (point, L)) && has_neighbour (point, R)) {
			mk_wire (lay, point, R);
		    }
		}
	    }
	}
    }

    /*
     * write maximal vertical wires
     */
    for (point->z = Bbx->crd[U] + HaveMarkerLayer; point->z >= Bbx->crd[D]; point->z--)
    { /* for all layers */
	for (point->x = Bbx->crd[L]; point->x <= Bbx->crd[R]; point->x++)
	{
	    for (point->y = Bbx->crd[B]; point->y <= Bbx->crd[T]; point->y++)
	    {
		if (grid_is_occupied (point)) { /* part of path */
		    if ((point->y == Bbx->crd[B] || !has_neighbour (point, B)) && has_neighbour (point, T)) {
			mk_wire (lay, point, T);
		    }
		}
	    }
	}
    }

    /*
     * add vias between metal layers
     */
    for (point->z = Bbx->crd[U]; point->z >= Bbx->crd[D]; point->z--)
    { /* for all layers */
	for (point->y = Bbx->crd[B]; point->y <= Bbx->crd[T]; point->y++)
	{ /* for all rows */
	    for (point->x = Bbx->crd[L]; point->x <= Bbx->crd[R]; point->x++)
	    { /* for all points */
		if (has_neighbour (point, U)) {
		    NewWire (wire);
		    wire->crd[L] = wire->crd[R] = point->x;
		    wire->crd[B] = wire->crd[T] = point->y;
		    wire->layer =  101 + point->z;
		    wire->next = lay->wire;
		    lay->wire = wire;
		}
	    }
	}
    }

    /*
     * vias to core
     */
    point->z = 0;
    for (point->y = Bbx->crd[B]; point->y <= Bbx->crd[T]; point->y++)
    { /* for all rows */
	for (point->x = Bbx->crd[L]; point->x <= Bbx->crd[R]; point->x++)
	{ /* for all points */
	    if (has_neighbour (point, D)) { /* via into core */
		NewWire (wire);
		wire->crd[L] = wire->crd[R] = point->x;
		wire->crd[B] = wire->crd[T] = point->y;
		wire->layer = 100; /* via to core */
		wire->next = lay->wire;
		lay->wire = wire;
	    }
	}
    }
}

/*
 * This routine writes an as long as possible strip in the direction dir
 */
static void mk_wire (LAYOUTPTR lay, GRIDPOINTPTR point, int dir)
{
    COREUNIT ***Grid; /* the grid of this cell */
    BOXPTR Bbx;       /* bbx of this cell */
    GRIDPOINT pointstruct;
    GRIDPOINTPTR npoint;
    int neighbour_found;
    WIREPTR wire;

    /* set the grid */
    Grid = ((R_CELLPTR) lay -> flag.p) -> grid;
    Bbx = &((R_CELLPTR) lay -> flag.p) -> cell_bbx;

    npoint = &pointstruct;
    npoint->z = point->z;
    npoint->y = point->y;
    npoint->x = point->x;

    NewWire (wire);
    wire->crd[L] = wire->crd[R] = point->x;
    wire->crd[B] = wire->crd[T] = point->y;
    if (point->z > Bbx->crd[U])
	wire->layer = 200; /* marker layer: dummy layer output */
    else
	wire->layer = point->z + 1;

    neighbour_found = FALSE;

    /*
     * processed is TRUE if the gridpoint is aleady covered by a previous wire
     */

    while (has_neighbour (npoint, dir))
    {
	step_point (npoint, dir);

	if (npoint->x > Bbx->crd[R] || npoint->y > Bbx->crd[T]) break;

	neighbour_found = TRUE;

	/* set new rectangle coords */
	wire->crd[R] = npoint->x;
	wire->crd[T] = npoint->y;
    }

    if (neighbour_found == TRUE) { /* link */
	wire->next = lay->wire;
	lay->wire = wire;
    }
    else {
	FreeWire (wire);     /* useless */
    }
}

/*
 * This routine fills the holes of wide wires which
 * occupy more than one gridpoint
 */
static void fill_holes (LAYOUTPTR lay)
{
    COREUNIT ***Grid; /* the grid of this cell */
    BOXPTR Bbx;       /* bbx of this cell */
    GRIDPOINT rtstruct, pointstruct;
    register GRIDPOINTPTR rt, point;
    WIREPTR wire;

    point = &pointstruct;
    rt = &rtstruct;

    if (!lay -> flag.p) {
	fprintf (stderr, "ERROR: no grid\n");
	return;
    }

    /* set the grid */
    Grid = ((R_CELLPTR) lay -> flag.p) -> grid;
    Bbx = &((R_CELLPTR) lay -> flag.p) -> cell_bbx;

    /*
     * Fill the holes between wires of same pattern
     */
    for (point->z = Bbx->crd[U]; point->z >= Bbx->crd[D]; point->z--)
    { /* for all layers */
	for (point->y = Bbx->crd[B]; point->y < Bbx->crd[T]; point->y++)
	{ /* for all rows */
	    for (point->x = Bbx->crd[L]; point->x < Bbx->crd[R]; point->x++)
	    { /* for all points */
		if (!grid_is_occupied (point)) continue;
		if (!has_neighbour (point, T)) continue;
		if (!has_neighbour (point, R)) continue;
		/* step the righttop of potential box */
		rt->y = point->y + 1;
		rt->z = point->z;
		for (rt->x = point->x + 1; rt->x <= Bbx->crd[R]; rt->x++) {
		    rt->y = point->y + 1;
		    if (!grid_is_occupied (rt)) break;
		    if (!has_neighbour (rt, L)) break;
		    rt->y--;
		    if (!has_neighbour (rt, L)) break;
		}
		rt->y = point->y + 1;
		rt->x--;
		if (rt->x <= point->x) continue; /* no square */

		/* add box */
		NewWire (wire);
		wire->layer = point->z + 1;
		wire->crd[L] = point->x;
		wire->crd[R] = rt->x;
		wire->crd[B] = point->y;
		wire->crd[T] = rt->y;
		wire->next = lay->wire;
		lay->wire = wire;

		point->x = rt->x;
	    }
	}
    }
}

/*
 * This routine sets lay->bbx and lay->off according to the actual size of the wires.
 * If no bounding box is found, the original values are kept.
 */
static void set_bbx (LAYOUTPTR lay)
{
    register CIRINSTPTR cinst;
    register R_INSTPTR rinst;
    register WIREPTR wire;
    register LAYPORTPTR layport;
    GRIDADRESSUNIT savebbx[2], saveoff[2];

    saveoff[X] = lay->off[X];
    saveoff[Y] = lay->off[Y];
    savebbx[X] = lay->bbx[X];
    savebbx[Y] = lay->bbx[Y];

    lay->bbx[X] = 0;
    lay->off[X] = 0;
    lay->bbx[Y] = 0;
    lay->off[Y] = 0;

    for (cinst = lay->circuit->cirinst; cinst; cinst = cinst->next) {
	if (!(rinst = (R_INSTPTR) cinst->flag.p)) continue;
	if (rinst->crd[L] < lay->off[X]) lay->off[X] = rinst->crd[L];
	if (rinst->crd[R] > lay->off[X] + lay->bbx[X]) lay->bbx[X] = rinst->crd[R] - lay->off[X];
	if (rinst->crd[B] < lay->off[Y]) lay->off[Y] = rinst->crd[B];
	if (rinst->crd[T] > lay->off[Y] + lay->bbx[Y]) lay->bbx[Y] = rinst->crd[T] - lay->off[Y];
    }

    for (wire = lay->wire; wire; wire = wire->next) {
	if (wire->crd[L] < lay->off[X]) lay->off[X] = wire->crd[L];
	if (wire->crd[R] > lay->off[X] + lay->bbx[X]) lay->bbx[X] = wire->crd[R] - lay->off[X];
	if (wire->crd[B] < lay->off[Y]) lay->off[Y] = wire->crd[B];
	if (wire->crd[T] > lay->off[Y] + lay->bbx[Y]) lay->bbx[Y] = wire->crd[T] - lay->off[Y];
    }

    for (layport = lay->layport; layport; layport = layport->next) {
	if (layport->pos[X] < lay->off[X]) lay->off[X] = layport->pos[X];
	if (layport->pos[X] > lay->off[X] + lay->bbx[X]) lay->bbx[X] = layport->pos[X] - lay->off[X];
	if (layport->pos[Y] < lay->off[Y]) lay->off[Y] = layport->pos[Y];
	if (layport->pos[Y] > lay->off[Y] + lay->bbx[Y]) lay->bbx[Y] = layport->pos[Y] - lay->off[Y];
    }

    if (lay->bbx[X] == 0 || lay->bbx[Y] == 0) {
	fprintf (stderr, "WARNING (set_bbx): no bounding box found\n");
	lay->off[X] = saveoff[X];
	lay->off[Y] = saveoff[Y];
	lay->bbx[X] = savebbx[X];
	lay->off[Y] = savebbx[Y];
    }
}

/*
 * This routine links the fat wires which may occur in critical nets
 * to the wire list of father.
 */
static void link_fat_wires (LAYOUTPTR father)
{
    NETPTR hnet;
    WIREPTR hwire;

    for (hnet = father->circuit->netlist; hnet; hnet = hnet->next) {
	if (!(hwire = ((R_NETPTR) hnet->flag.p)->wire)) continue;
	while (hwire->next) hwire = hwire->next; /* wind hwire to end */
	hwire->next = father->wire; /* link to existing list */
	father->wire = ((R_NETPTR) hnet->flag.p)->wire;
	((R_NETPTR) hnet->flag.p)->wire = NULL;
    }
}
