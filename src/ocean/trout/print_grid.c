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

extern COREUNIT
   Pat_mask[HERE+1];         /* look-up table for bit-patterns */

extern GRIDADRESSUNIT
   Xoff[HERE+1],           /* look-up tables for offset values */
   Yoff[HERE+1],
   Zoff[HERE+1];

static void print_grid_layer (GRIDADRESSUNIT z, LAYOUTPTR father)
/* z - layer to be printed */
{
    GRIDPOINT pointstruct;
    GRIDPOINTPTR point;
    int hor, vert;
    R_CELLPTR rcell;
    COREUNIT ***Grid; /* the basic grid */

    rcell = (R_CELLPTR) father->flag.p;

    Grid = rcell->grid;

    point = &pointstruct;

    point->z = z;

    printf ("&&&&&&&&&&&&&&&&&& layer %ld &&&&&&&&&&&&&&&&&&&&\n", z);
    for (point->y = rcell->cell_bbx.crd[T]; point->y >= rcell->cell_bbx.crd[B]; point->y--)
    {
	printf ("%ld\t", point->y);
	for (point->x = rcell->cell_bbx.crd[L]; point->x != rcell->cell_bbx.crd[R]; point->x++)
	{
	    if (is_free (point)) {
		printf (" "); continue;
	    }

	    if (has_neighbour (point, U) || has_neighbour (point, D)) { /* via */
		printf ("*"); continue;
	    }

	    if (has_neighbour (point, L) || has_neighbour (point, R))
		hor = TRUE;
	    else
		hor = FALSE;

	    if (has_neighbour (point, B) || has_neighbour (point, T))
		vert = TRUE;
	    else
		vert = FALSE;

	    if (vert == TRUE && hor == FALSE) { /* vertical */
		printf ("|"); continue;
	    }

	    if (vert == FALSE && hor == TRUE) { /* horizontal */
		printf ("-"); continue;
	    }
	    /* the rest: something else */
	    printf ("+");
	}
	printf ("\n");
    }
}

void print_all_layers (LAYOUTPTR father)
{
    GRIDADRESSUNIT z;
    R_CELLPTR rcell;

    rcell = (R_CELLPTR) father->flag.p;

    for (z = rcell->cell_bbx.crd[D]; z <= rcell->cell_bbx.crd[U]; z++)
	print_grid_layer (z, father);
}
