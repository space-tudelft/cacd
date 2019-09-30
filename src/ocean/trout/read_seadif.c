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
 * read seadif cells recursively
 */

#include "src/ocean/trout/typedef.h"
#include "src/ocean/trout/grid.h"
#include "src/ocean/libseadif/sealibio.h"

static LAYOUTPTR copy_layout (LAYOUTPTR original, char *newname);
static void read_wire_pattern_into_grid (COREUNIT ***grid, WIREPTR wire,
    int mtx0, int mtx1, int mtx2, int mtx3, int mtx4, int mtx5, BOXPTR bbx, int read_mode);
static SLICEPTR recursive_slice_copy (SLICEPTR oldslice);
static void recursive_slice_read (COREUNIT ***grid, SLICEPTR slice,
    int mtx0, int mtx1, int mtx2, int mtx3, int mtx4, int mtx5, BOXPTR bbx, int read_mode);

extern LAYOUTPTR thislay;
extern BOX Image_bbx;      /* the bounding box of Grid[][][] */
extern BOXPTR Rbbx;        /* routing bounding box */
extern long Chip_num_layer; /* number of metal layers to be used */
extern long GridRepitition[2]; /* repitionvector (dx, dy) of grid core image (in grid points) */
extern COREUNIT Pat_mask[HERE+1]; /* look-up table for bit-patterns */
extern GRIDPOINTPTR **CoreFeed; /* matrix of feedthroughs in basic image */
extern char *ThisImage;    /* Seadif name of this image */
extern int Hacklevel;
extern int dontCheckChildPorts;

/*
 * this routine reads the specified cell from seadif file into core for
 * the circuit as well as the layout.
 * It will return a pointer to the layout seadid struct.
 */
LAYOUTPTR read_seadif_into_core (char *lib, char *func, char *cir, char *lay)
/* args - are canonicstringed names */
{
    dontCheckChildPorts = 1;

    /* read the layout cell recursively (until the bottom) */
    if (!sdfreadalllay (SDFLAYALL, lay, cir, func, lib)) {
	fprintf (stderr, "Cannot read layout");
	return (NULL);
    }

    /* I have swapped the order of sdfreadalllay() and sdfreadcir().
	Now first the layout is read and then - when the circuit is read -
	the ports of the layout sub-cells are added to the circuit sub-cells
	(in contrast to the past, we do no longer read the circuit sub-cells). (AvG, Aug.1998)
    */

    /* read circuit cell */
    if (!sdfreadcir (SDFCIRALL, cir, func, lib)) {
	fprintf (stderr, "Cannot read circuit");
	return (NULL);
    }

    if (strcmp (ThisImage, "fishbone") == 0) {
	/* in case of fishbone: if bbx is on boundary: enlarge it by one */
	if ((thislay->bbx[Y] % GridRepitition[Y]) == 0) thislay->bbx[Y]++;
    }

    return (thislay);
}

/*
 * this routine reads the specified cell from seadif into the datastructure,
 * creates a a grid and stamps all wires of the cell (and its sub-cells)
 * into the grid
 */
void convert_seadif_into_grid (LAYOUTPTR father)
{
    R_CELLPTR rcell;

    rcell = (R_CELLPTR) father->flag.p;

    /* make a grid size of bounding box */
    rcell->gridsize[X] = father->off[X] + father->bbx[X];
    rcell->gridsize[Y] = father->off[Y] + father->bbx[Y];
    rcell->gridsize[Z] = Chip_num_layer;

    /* set the reading/working bounding box */
    rcell->cell_bbx.crd[L] = father->off[X];
    rcell->cell_bbx.crd[R] = father->off[X] + father->bbx[X] - 1;
    rcell->cell_bbx.crd[B] = father->off[Y];
    rcell->cell_bbx.crd[T] = father->off[Y] + father->bbx[Y] - 1;
    rcell->cell_bbx.crd[D] = 0;
    rcell->cell_bbx.crd[U] = Chip_num_layer - 1;

    if (Hacklevel == 3 && Rbbx != 0) {
	rcell->cell_bbx.crd[L] = Rbbx->crd[L];
	rcell->cell_bbx.crd[R] = Rbbx->crd[R];
	rcell->cell_bbx.crd[B] = Rbbx->crd[B];
	rcell->cell_bbx.crd[T] = Rbbx->crd[T];
    }

    if (rcell->gridsize[X] == 0 || rcell->gridsize[Y] == 0 || rcell->gridsize[X] == 0) {
	fprintf (stderr, "WARNING: seadif cell '%s' has a zero dimension\n", rcell->layout->name);
	return;
    }

    /* make the grid */
    rcell->grid = new_grid (rcell->gridsize[X], rcell->gridsize[Y], rcell->gridsize[Z], father);

    read_seadif_wires_into_grid (rcell->grid, father, 1, 0, 0, 0, 1, 0, &rcell->cell_bbx, TRUE);

    make_statistics (father, TRUE); /* routing statistics */
}

/*
 * This routine recursively reads wires of lay into the grid
 */
void read_seadif_wires_into_grid (COREUNIT ***grid, LAYOUTPTR lay,
    int mtx0, int mtx1, int mtx2, int mtx3, int mtx4, int mtx5, BOXPTR bbx, int read_mode)
/* lay    - layout pointer */
/* mtx0,5 - orientation mtx */
/* bbx    - read bounding box */
{
    if (!lay) return;

    /* recursively read the children
     * Note: it is essential that the children are read first if
     * any negative wires are present;
     */
    recursive_slice_read (grid, lay->slice, mtx0, mtx1, mtx2, mtx3, mtx4, mtx5, bbx, read_mode);

    /* read the wire pattern */
    read_wire_pattern_into_grid (grid, lay->wire, mtx0, mtx1, mtx2, mtx3, mtx4, mtx5, bbx, read_mode);
}

/*
 * This routine read the wire pattern belonging to 'lay' into the grid
 */
static void read_wire_pattern_into_grid (COREUNIT ***grid, WIREPTR wire,
    int mtx0, int mtx1, int mtx2, int mtx3, int mtx4, int mtx5, BOXPTR bbx, int read_mode)
/* wire   - the wire to be stamped */
/* mtx0,5 - orientation mtx */
/* bbx    - read bounding box */
/* read_mode - TRUE for read, FALSE for erase */
{
    COREUNIT b;
    GRIDADRESSUNIT x, y, z;
    register GRIDADRESSUNIT xl, xr, yb, yt;
    int erase, illegal_layer;

    illegal_layer = 0;

    for ( ; wire; wire = wire->next)
    {
	z = wire->layer;

	if (z < 0) { z = -z;
	    erase = (read_mode == TRUE);
	} else {
	    erase = (read_mode != TRUE);
	}

	if (z < 100) { /* wire */
	    z = z - 1;
	    if (z >= Chip_num_layer || z < 0) { illegal_layer++; continue; }
	    if (z < bbx->crd[D] || z > bbx->crd[U]) continue;

	    /* transformation */
	    xl = mtx0 * wire->crd[L] + mtx1 * wire->crd[B] + mtx2;
	    xr = mtx0 * wire->crd[R] + mtx1 * wire->crd[T] + mtx2;
	    yb = mtx3 * wire->crd[L] + mtx4 * wire->crd[B] + mtx5;
	    yt = mtx3 * wire->crd[R] + mtx4 * wire->crd[T] + mtx5;

	    if (xl > xr) { x = xl; xl = xr; xr = x; } /* swap */
	    if (yb > yt) { y = yb; yb = yt; yt = y; } /* swap */

	    /* stamp in grid */
	    for (x = xl; x <= xr; x++) {
		if (x < bbx->crd[L] || x > bbx->crd[R]) continue; /* outside range */

		for (y = yb; y <= yt; y++) {
		    if (y < bbx->crd[B] || y > bbx->crd[T]) continue; /* outside range */

		    b = STATEMASK;
		    if (x < xr) b |= (1 << R);
		    if (x > xl) b |= (1 << L);
		    if (y < yt) b |= (1 << T);
		    if (y > yb) b |= (1 << B);

		    if (erase) {
			if (grid[z][y][x] == b || grid[z][y][x] == 0)
			    grid[z][y][x] = 0; /* nothing left */
			else { /* some survive */
			    b = b ^ PATTERNMASK; /* reverse pointer bits = bitwise exor */
			    grid[z][y][x] &= b; /* AND into grid */
			}
		    }
		    else grid[z][y][x] |= b; /* OR into grid */
		}
	    }
	}
	else { /* via */
	    if (z >= 200) continue; /* dummy mask */
	    z = z - 100;
	    if (z >= Chip_num_layer) { illegal_layer++; continue; }
	    if (z < bbx->crd[D] || z > bbx->crd[U]) continue;

	    x = mtx0 * wire->crd[L] + mtx1 * wire->crd[B] + mtx2;
	    y = mtx3 * wire->crd[L] + mtx4 * wire->crd[B] + mtx5;
	    if (x < bbx->crd[L] || x > bbx->crd[R]) continue; /* outside range */
	    if (y < bbx->crd[B] || y > bbx->crd[T]) continue; /* outside range */

	    /* stamp into grid */
	    /* downward from layer */
	    b = STATEMASK + (1 << D);
	    if (erase) {
		if (grid[z][y][x] == b || grid[z][y][x] == 0)
		    grid[z][y][x] = 0; /* nothing left */
		else { /* some survive */
		    b = b ^ PATTERNMASK; /* reverse pointer bits = bitwise exor */
		    grid[z][y][x] &= b; /* AND into grid */
		}
	    }
	    else grid[z][y][x] |= b; /* OR into grid */

	    /* upward from layer-1 */
	    if (--z < 0) continue;

	    b = STATEMASK + (1 << U);
	    if (erase) {
		if (grid[z][y][x] == b || grid[z][y][x] == 0)
		    grid[z][y][x] = 0; /* nothing left */
		else { /* some survive */
		    b = b ^ PATTERNMASK; /* reverse pointer bits = bitwise exor */
		    grid[z][y][x] &= b; /* AND into grid */
		}
	    }
	    else grid[z][y][x] |= b;
	}
    }

    if (illegal_layer > 0) {
	fprintf (stderr, "WARNING (read_wire_pattern_into_grid): %d wires in illegal layer found\n", illegal_layer);
    }
}

/*
 * recursive help routine to read instances
 */
static void recursive_slice_read (COREUNIT ***grid, SLICEPTR slice,
    int mtx0, int mtx1, int mtx2, int mtx3, int mtx4, int mtx5, BOXPTR bbx, int read_mode)
/* mtx0,5 - orientation mtx */
/* bbx - read bounding box */
{
    LAYINSTPTR inst;

    for ( ; slice; slice = slice->next) {
	if (slice->chld_type == SLICE_CHLD) { /* a slice: recursion */
	    recursive_slice_read (grid, slice->chld.slice, mtx0, mtx1, mtx2, mtx3, mtx4, mtx5, bbx, read_mode);
	    continue;
	}

	/* child contains instances */
	for (inst = slice->chld.layinst; inst; inst = inst->next) { /* actual recursion */
	    /* transform mtx over instance's transformation */
	    read_seadif_wires_into_grid (grid , inst->layout,
		mtx0 * inst->mtx[0] + mtx1 * inst->mtx[3],
		mtx0 * inst->mtx[1] + mtx1 * inst->mtx[4],
		mtx0 * inst->mtx[2] + mtx1 * inst->mtx[5] + mtx2,
		mtx3 * inst->mtx[0] + mtx4 * inst->mtx[3],
		mtx3 * inst->mtx[1] + mtx4 * inst->mtx[4],
		mtx3 * inst->mtx[2] + mtx4 * inst->mtx[5] + mtx5,
		bbx, read_mode);
	}
    }
}

/*
 * This routine makes a copy of the father, and gives it the name 'layname'.
 */
LAYOUTPTR copy_father (LAYOUTPTR original, char *layname)
{
    char *copyname, newname[300];

    if (!original) {
	fprintf (stderr, "WARNING (make_copy): original is NULL\n");
	return (NULL);
    }

    /* make the backup name */
    if (!layname || strlen (layname) == 0)
	sprintf (newname, "%s_r", original->name);
    else
	strcpy (newname, layname);

    copyname = canonicstring (newname);

    return (copy_layout (original, copyname));
}

/*
 * This routine copies layout cell father and returns a pointer
 * to the net seadif father. The copy will obtain a new name: <name>_r
 */
static LAYOUTPTR copy_layout (LAYOUTPTR original, char *newname)
/* newname - canonicstringed */
{
    LAYOUTPTR replica;
    LAYPORTPTR oport, rport;
    WIREPTR owire, rwire;
    register int i;
    int already_there = FALSE;

    /*
     * find out whether this layout already exists
     */
    if (existslay (newname,
		original->circuit->name,
		original->circuit->function->name,
		original->circuit->function->library->name)) {
	/* already in database : read it */
	if (!sdfreadlay (SDFLAYBODY, newname,
		original->circuit->name,
		original->circuit->function->name,
		original->circuit->function->library->name)) {
	    /* failed: just allocate */
	    NewLayout (replica);
	    replica->name = canonicstring (newname);
	}
	else {
	    already_there = TRUE;
	    replica = thislay;
	}
    }
    else {
	NewLayout (replica);
	replica->name = canonicstring (newname);
    }

    /* copy the terminals */
    for (oport = original->layport; oport; oport = oport->next) {
	NewLayport (rport);
	rport->cirport = oport->cirport;
	rport->layer = oport->layer;
	rport->pos[X] = oport->pos[X];
	rport->pos[Y] = oport->pos[Y];
	/* flag not copied */
	rport->next = replica->layport;
	replica->layport = rport;
    }

    replica->bbx[X] = original->bbx[X];
    replica->bbx[Y] = original->bbx[Y];

    replica->off[X] = original->off[X];
    replica->off[Y] = original->off[Y];

    /* copy slices */
    replica->slice = recursive_slice_copy (original->slice);

    /* copy wires */
    for (owire = original->wire; owire; owire = owire->next) {
	NewWire (rwire);
	for (i = 0; i != 4; i++) rwire->crd[i] = owire->crd[i];
	rwire->layer = owire->layer;
	rwire->next = replica->wire;
	replica->wire = rwire;
    }

    /* the rest */
    replica->status = original->status;
    if (already_there == FALSE) {
	replica->circuit = original->circuit;
	replica->linkcnt = 1;
	replica->next = original->next;
	original->next = replica;
    }

    return (replica);
}

/*
 * This routine performs the copy of the model calls
 */
static SLICEPTR recursive_slice_copy (SLICEPTR oldslice)
{
    register SLICEPTR newslicelist, rslice;
    LAYINSTPTR rinst, layinst;
    int i;

    newslicelist = NULL;

    for ( ; oldslice; oldslice = oldslice->next) {

	NewSlice (rslice);
	rslice->ordination = oldslice->ordination;
	rslice->chld_type = oldslice->chld_type;

	rslice->next = newslicelist;
	newslicelist = rslice;

	if (oldslice->chld_type == SLICE_CHLD) {
	    rslice->chld.slice = recursive_slice_copy (oldslice->chld.slice);
	    continue;
	}

	/* child contains instances: copy them */
	for (layinst = oldslice->chld.layinst; layinst; layinst = layinst->next) {
	    NewLayinst (rinst);
	    rinst->name = canonicstring (layinst->name);
	    rinst->layout = layinst->layout;
	    for (i=0; i != 6; i++) rinst->mtx[i] = layinst->mtx[i];
	    rinst->next = rslice->chld.layinst;
	    rslice->chld.layinst = rinst;
	}

	/* reverse the intance list (copy creates list in opposite order) */
	layinst = rslice->chld.layinst;
	rslice->chld.layinst = NULL;
	while (layinst) {
	    rinst = layinst; layinst = layinst->next;
	    rinst->next = rslice->chld.layinst;
	    rslice->chld.layinst = rinst;
	}
    }

    /*
     * reverse the order of the slices
     */
    oldslice = newslicelist;
    newslicelist = NULL;
    while (oldslice) {
	rslice = oldslice;
	oldslice = oldslice->next;
	rslice->next = newslicelist;
	newslicelist = rslice;
    }

    return (newslicelist);
}

/*
 * This routine counts the number of used transistors and prints some statistsics.
 */
void make_statistics (LAYOUTPTR father, int before)
/* before - true if called before the routing */
{
    R_CELLPTR rcell;
    GRIDADRESSUNIT x, y;
    COREUNIT ***grid;
    long connected, num_connected, num_transistor, num_offset;
    GRIDPOINTPTR off;
    static long Connected_before;

    if (!(rcell = (R_CELLPTR) father->flag.p)) return;

    if (!(grid = rcell->grid)) return;

    num_transistor = num_connected = 0;

    /*
     * step through the grid
     */
    for (y = rcell->cell_bbx.crd[B]; y <= rcell->cell_bbx.crd[T]; y++) {
	for (x = rcell->cell_bbx.crd[L]; x <= rcell->cell_bbx.crd[R]; x++) {
	    if (!(off = CoreFeed[to_core(x,X)][to_core(y,Y)])) continue; /* no feeds */

	    /* step through offsets */
	    num_offset = 0;
	    if (grid[0][y][x] & Pat_mask[D])
		connected = TRUE;
	    else
		connected = FALSE;

	    for ( ; off; off = off->next) {
		if (off->x < 0 || off->y < 0) {
		    num_offset = -1; /* only the left-bottom most is used */
		    break; /* to prevent multiple counting of same transistor */
		}

		if (y + off->y >= rcell->gridsize[Y] ||
		    x + off->x >= rcell->gridsize[X]) continue; /* outside image */

		if (grid[0][y + off->y][x + off->x] & Pat_mask[D]) connected = TRUE; /* connected */
		num_offset++;
	    }

	    if (num_offset < 0) continue; /* from multiple transistor */
	    if (num_offset > 2) continue; /* very unlikely from transistor */

	    num_transistor++;
	    if (connected == TRUE) num_connected++;
	}
    }

    if (before == TRUE) {
	Connected_before = num_connected;
	return;
    }
    if (num_transistor == 0) return;

    printf ("No. of transistors (total / used):    %ld/%ld = %4.2f %%\n",
	num_transistor, num_connected, (double)num_connected / num_transistor * 100);

    if (num_connected - Connected_before > 0)
	printf ("No. of poly feeds used by router:     %ld\n", (long)(num_connected - Connected_before));
}

/*
 * This routine erases the wires/vias of father (only)
 */
void erase_wires (LAYOUTPTR father)
{
    register WIREPTR delwire;
    LAYPORTPTR delport;

    while (father->wire) {
	delwire = father->wire;
	father->wire = father->wire->next;
	FreeWire (delwire);
    }

    /* also remove terminals, just to be sure */
    while (father->layport) {
	delport = father->layport;
	father->layport = father->layport->next;
	FreeLayport (delport);
    }
}
