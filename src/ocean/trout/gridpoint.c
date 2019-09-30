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
 * gridpoint.h
 *	operations using gridpoint structure
 *********************************************************/
#include "src/ocean/trout/typedef.h"
#include "src/ocean/trout/grid.h"

extern GRIDPOINTPTR w_junk;	/* the linked list of unused gridpoints */
extern COREUNIT ***Grid;	/* the working grid */
extern COREUNIT Pat_mask[];     /* look-up table for bit-patterns */
extern BOXPTR Bbx;		/* bounding box of working grid */
extern long HaveMarkerLayer;	/* TRUE=1 if marker layer to indicate unconnect */

/*
 * This routine 'frees' one element gridp by recycling.
 */
void free_gridpoint (GRIDPOINTPTR p)
{
    p -> next = w_junk; w_junk = p;
}

/*
 * remove a list of gridpoints
 */
void free_gridpoint_list (GRIDPOINTPTR listp)
{
    register GRIDPOINTPTR p;

    if ((p = listp)) {
	while (p -> next) p = p -> next;
	p -> next = w_junk; w_junk = listp;
    }
}

/*
 * This routine copies the wirepattern into the marker layer.
 */
void copy_to_marker_layer (GRIDPOINTPTR wirepattern)
{
    register GRIDPOINTPTR hwire;

    if (HaveMarkerLayer == FALSE) return;

    for (hwire = wirepattern; hwire; hwire = hwire->next) {
	if (hwire->cost == -1) continue;
	Grid[Bbx->crd[U]+1][hwire->y][hwire->x] |= hwire->pattern;
    }
}

/*
 * This routine restores the wire patttern which is stored
 * the the list of GRIDPOINT structure elements.
 * wire pattern list is also freed.
 * only non-temporary elemnets are restored.
 * The flag 'cost' indicates this 'temporary' nature.
 */
void free_and_restore_wire_pattern (GRIDPOINTPTR list)
{
    register GRIDPOINTPTR p = list;

    if (p) { /* only restore if NOT temporary */
	if (p->cost != -1) restore_grid_pattern (p);
	while (p->next) {
	    p = p->next;
	    if (p->cost != -1) restore_grid_pattern (p);
	}
	p->next = w_junk; w_junk = list; /* free */
    }
}

/*
 * This routine restores the wire patttern which is stored
 * the the list of GRIDPOINT structure elements.
 * Restore only, no free.
 */
void restore_wire_pattern (GRIDPOINTPTR list)
{
    register GRIDPOINTPTR p = list;

    while (p) { /* only restore if NOT temporary */
	if (p->cost != -1) restore_grid_pattern (p);
	p = p->next;
    }
}

/*
 * This routine combines a number of routine functions, i.e.
 * count_length, restore_wire_pattern, melt_new_wire and free_gridpoint_list.
 * It returns the wire length count.
 */
long count_restore_melt_and_free_wire (GRIDPOINTPTR path)
{
    GRIDADRESSUNIT xl, xr, yb, yt, zd, zu;
    register GRIDADRESSUNIT x, y, z;
    register GRIDPOINTPTR p, pp;
    long length = 0;

    xl = Bbx->crd[L]; xr = Bbx->crd[R];
    yb = Bbx->crd[B]; yt = Bbx->crd[T];
    zd = Bbx->crd[D]; zu = Bbx->crd[U];

#define gn grid_neighbour
#define addgn add_grid_neighbour_c

    /* restore and count wire length */
    p = pp = path;
    if (p->cost != -1) restore_grid_pattern (p);
    while ((p = p->next)) {
	if (p->cost != -1) restore_grid_pattern (p);
	length += ABS (pp->x - p->x) + ABS (pp->y - p->y);
	pp = p;
    }

    /* melt and free the wire points */
    while ((p = path)) {
	if (p->cost != -1) { /* no temp path */
	    x = p->x; y = p->y; z = p->z;
	    if (!is_Free (x, y, z)) { /* melt if possible */
		if (gn (x, y, z, L) && x > xl && !gn (x-1, y, z, R)) addgn (x-1, y, z, R);
		if (gn (x, y, z, R) && x < xr && !gn (x+1, y, z, L)) addgn (x+1, y, z, L);
		if (gn (x, y, z, B) && y > yb && !gn (x, y-1, z, T)) addgn (x, y-1, z, T);
		if (gn (x, y, z, T) && y < yt && !gn (x, y+1, z, B)) addgn (x, y+1, z, B);
		if (gn (x, y, z, D) && z > zd && !gn (x, y, z-1, U)) addgn (x, y, z-1, U);
		if (gn (x, y, z, U) && z < zu && !gn (x, y, z+1, D)) addgn (x, y, z+1, D);
	    }
	}
	path = p->next;
	p->next = w_junk; w_junk = p; /* free_gridpoint */
    }
    return (length);
}

/*
 * This routine reverses the order of the gridpoint list
 */
GRIDPOINTPTR reverse_list (GRIDPOINTPTR gridp)
{
    register GRIDPOINTPTR hgridp, list;

    list = NULL;

    while (gridp) {
	hgridp = gridp;
	gridp = gridp->next;
	hgridp->next = list;
	list = hgridp;
    }
    return (list);
}
