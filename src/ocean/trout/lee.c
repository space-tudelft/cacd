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
 * Perform lee routing.
 */

#include "src/ocean/trout/typedef.h"
#include "src/ocean/trout/grid.h"
#include "src/ocean/trout/lee.h"
#include <unistd.h>
#include <time.h>

void copy_to_marker_layer (GRIDPOINTPTR wirepattern);
static void insert_in_frontlist (GRIDPOINTPTR point);
static int  is_resistor (GRIDADRESSUNIT x, GRIDADRESSUNIT y);
static GRIDPOINTPTR lee_expansion (BOXPTR exp_bbx);
static void melt_pattern (GRIDPOINTPTR pattern, BOXPTR exp_bbx);
static void rough_clear (BOXPTR exp_bbx);
static GRIDPOINTPTR trace_wire (GRIDPOINTPTR list);
static int  via_allowed (GRIDPOINTPTR point, GRIDPOINTPTR offset);

#define MAXINFRONT  50

/*
 * For run-time efficiency, the lee expansion is taylored for a number of different cases.
 * The reason is that the majority of the CPU during routing is spent
 * on checking whether an expansion is allowed.
 * Use PM25 for the 1-layer pm25 image.
 * Use OVERLAPPING_IMAGE if the gridimages feed to neighbours (expensive).
 * Use NONOVERLAPPING_IMAGE is this is not the case (such as fishbone).
 */

/* #define PM25 */
/* #define OVERLAPPING_IMAGE  */
#define NONOVERLAPPING_IMAGE

/*
 * define for rnet->type (already defined)
#define SIGNAL 0
#define CLOCK  1
#define POWER  2
 */

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

extern BOXPTR Bbx;         /* bounding box of Grid */

extern GRIDPOINTPTR
   ***OffsetTable,         /* matrix of accessable neighbour grid points */
   **RestrictedCoreFeed,
   **CoreFeed;             /* matrix of feedthroughs in basic image */

extern long
   RouteToBorder,          /* TRUE to connect parent terminals to the border */
   Verify_only,            /* TRUE to perform only wire checking */
   HaveMarkerLayer,        /* TRUE=1 if marker layer to indicate unconnect */
   Use_borders,            /* TRUE to use feeds which extend over the routing area */
   Resistor_hack,          /* TRUE to consider resistors which are made up
                              of poly gates as open ports. Normally they
                              are considered as a short */
   *LayerOrient,
   NumTearLine[2],         /* number of tearlines in orientation of index */
   *TearLine[2],           /* array containing the coordinales of the tearline */
   Chip_num_layer,
   GridRepitition[2];      /* repitionvector (dx, dy) of grid core image (in grid points) */

extern char *ThisImage;    /* Seadif name of this image */

/*
 * local
 */
static int (* is_free_in_deep_image)(); /* the routing to determine whether it is
                                           possible to expand through the image */
static long Logn, Nextn, Prevn;
static int Penalty_factor;
static int save_source_nr;
static GRIDPOINTPTR front_begin, front_end;
static BOX bbx1, bbx2;     /* bounding boxes of source patterns */
static int front1_size,    /* -1 if not enabled. >= 0 if enabled */
           front2_size;    /* counts number of elements in front */

/*
 * This routine performs lee routing between two points, pointed at by wireptr structs.
 * It will return a pointer to the generated wire and
 * it leaves the path of the wire in the grid.
 *
 * The algorithm uses two fronts, each around the original terminal.
 * The routine will return NULL if no wire was generated.
 */
GRIDPOINTPTR lee (GRIDPOINTPTR point1, GRIDPOINTPTR point2, BOXPTR routing_bbx, int do_reduce)
/* point1      - address of terminal 1 */
/* point2      - address of terminal 2 */
/* routing_bbx - bounding box within which the routing should take place */
{
    int term1_num, term2_num;
    GRIDPOINTPTR terminalpattern1; /* expanded pattern of terminal 1 */
    GRIDPOINTPTR terminalpattern2; /* expanded pattern of terminal 2 */
    GRIDPOINTPTR wirepattern;      /* pattern of the generated wire */
    BOX exp_bbx;

    /* check terminal point1 and point2 */
    if (outside_bbx (point1->x, point1->y, point1->z, routing_bbx) ||
	outside_bbx (point2->x, point2->y, point2->z, routing_bbx)) {
	fprintf (stderr, "WARNING (internal): terminal outside routing area:");
	fprintf (stderr, " (%ld, %ld, %ld) or", point1->x, point1->y, point1->z);
	fprintf (stderr, " (%ld, %ld, %ld)\n",  point2->x, point2->y, point2->z);
	return (NULL);
    }

    /* following test was added by SdeG to return directly 'connected' */
    if (point1->x == point2->x && point1->y == point2->y && point1->z == point2->z) {
	/* make dummy struct */
	wirepattern = new_gridpoint (point2->x, point2->y, point2->z);
	wirepattern->cost = -1; /* disable printing */
	return (wirepattern);
    }

    /* Set the limits of the expansion bounding box.
     * This is the box withing which all routing will have to take place.
     * This in order to reduce the maximum size of the in the 'wrong' direction.
     * Currently the maximum exp_bbx is the size of the core.
     */
    exp_bbx.crd[L] = routing_bbx->crd[L];
    exp_bbx.crd[R] = routing_bbx->crd[R];
    exp_bbx.crd[B] = routing_bbx->crd[B];
    exp_bbx.crd[T] = routing_bbx->crd[T];
    exp_bbx.crd[D] = routing_bbx->crd[D];
    exp_bbx.crd[U] = routing_bbx->crd[U];

    bbx1.crd[L] = bbx1.crd[R] = point1->x;
    bbx1.crd[B] = bbx1.crd[T] = point1->y;
    bbx1.crd[D] = bbx1.crd[U] = point1->z;

    bbx2.crd[L] = bbx2.crd[R] = point2->x;
    bbx2.crd[B] = bbx2.crd[T] = point2->y;
    bbx2.crd[D] = bbx2.crd[U] = point2->z;

    /* make sure that point1 and point2 do not address an empty grid */
    if (is_free (point1)) set_grid_occupied (point1);
    if (is_free (point2)) set_grid_occupied (point2);

    /* Expand the pattern around terminal 1: find all points connected to it.
     * These points are 'saved' in this pattern.
     * The routine will return NULL if the point adresses an empty grid.
     */
    save_source_nr = 0;
    if (!(terminalpattern1 = save_source (point1, &bbx1, &exp_bbx))) return (NULL);
    term1_num = save_source_nr;

    save_source_nr = 0;
    if (!(terminalpattern2 = save_source (point2, &bbx2, &exp_bbx))) { /* clean up */
	if ((get_grid_state (point2) == 0 && get_grid_pattern (point2) != 0)
	  /* following test was added by AvG to return 'connected' */
	|| (point1->x == point2->x && point1->y == point2->y && point1->z == point2->z))
	{ /* it was part of source of point1: already connected */
	    /* make dummy struct */
	    wirepattern = new_gridpoint (point2->x, point2->y, point2->z);
	    wirepattern->cost = -1; /* disable printing */
	}
	else wirepattern = NULL; /* failed for some other reason */

	free_and_restore_wire_pattern (terminalpattern1);
	return (wirepattern);
    }
    term2_num = save_source_nr;

    /* select the number of wavefronts for expansion */

    if (term1_num > MAXINFRONT) {
	if (term2_num > MAXINFRONT) {
	    if (term1_num > term2_num) { front1_size = -1; front2_size = 0; }
	    else front1_size = front2_size = 0;
	}
	else { front1_size = -1; front2_size = 0; }
    }
    else {
	if (term2_num > MAXINFRONT) { front1_size = 0; front2_size = -1; }
	else front1_size = front2_size = 0;
    }

    front_begin = front_end = NULL;

    /* Mark the source and destination patterns as such.
     * The routine will copy the frontpatterns and return the
     * initialized front point list.
     * Null will be returned if the fronts are already connected.
     */
    if (!mark_as_front (terminalpattern1, FRONT1, front1_size)) { /* clean up */
	free_and_restore_wire_pattern (terminalpattern1);
	free_and_restore_wire_pattern (terminalpattern2);
	/* make dummy struct */
	wirepattern = new_gridpoint (point1->x, point1->y, point1->z);
	wirepattern->cost = -1; /* disable printing */
	return (wirepattern);
    }

    if (!mark_as_front (terminalpattern2, FRONT2, front2_size)) { /* clean up */
	free_and_restore_wire_pattern (terminalpattern1);
	free_and_restore_wire_pattern (terminalpattern2);
	/* make dummy struct */
	wirepattern = new_gridpoint (point2->x, point2->y, point2->z);
	wirepattern->cost = -1; /* disable printing */
	return (wirepattern);
    }

    if (do_reduce == TRUE) { /* reduce search area */
	exp_bbx.crd[L] = MIN (bbx1.crd[L], bbx2.crd[L]);
	exp_bbx.crd[R] = MAX (bbx1.crd[R], bbx2.crd[R]);
	exp_bbx.crd[B] = MIN (bbx1.crd[B], bbx2.crd[B]);
	exp_bbx.crd[T] = MAX (bbx1.crd[T], bbx2.crd[T]);
	exp_bbx.crd[L] = MAX (Bbx->crd[L], exp_bbx.crd[L] - 10);
	exp_bbx.crd[R] = MIN (Bbx->crd[R], exp_bbx.crd[R] + 10);
	exp_bbx.crd[B] = MAX (Bbx->crd[B], exp_bbx.crd[B] - 10);
	exp_bbx.crd[T] = MIN (Bbx->crd[T], exp_bbx.crd[T] + 10);
	Penalty_factor = 1; /* not penalty: rely on reducing area */
    }
    else {
	Penalty_factor = 30; /* set penalty factor to direct search */
    }

    /* perform the actual expansion... */
    wirepattern = lee_expansion (&exp_bbx);

    free_and_restore_wire_pattern (terminalpattern1);
    free_and_restore_wire_pattern (terminalpattern2);
    return (wirepattern);
}

/*
 * Special of is_free_in_deep_image for the 1-layer pm25 image.
 */
static int is_free_pm25 (GRIDPOINTPTR point, GRIDPOINTPTR offset)
{
    register GRIDPOINTPTR tunnelpointer;
    register int tunnelend_found = FALSE;
    register COREUNIT pstate;
    register GRIDADRESSUNIT x, y;

    /* TEMP!!!!!!!!!!!!!!
	if (point->y == 69 && point->x == 108) printf ("hit");
     */
    pstate = point->pattern & STATEMASK;

    /* the exit point itself must be free, or destination */
    x = point->x + offset->x;
    y = point->y + offset->y;
    if (is_state (x, y, 0, STATEMASK) || is_state (x, y, 0, pstate)) return (FALSE);

    if (point->y < Bbx->crd[B] + 14 || point->y > Bbx->crd[T] - 14)
    { /* in reach: use more extensive search */
	for_all_tunnels (tunnelpointer, point) {
	    x = point->x + tunnelpointer->x;
	    y = point->y + tunnelpointer->y;
	    if (y < Bbx->crd[B] || y > Bbx->crd[T])
	    { /* ATTENTION: this is important: either disable, or enable */
		if (point->cost == 0 || Use_borders == TRUE)
		    continue; /* enable: use feeds on borders */
		else
		    return (FALSE); /* disable: leave feeds in borders untouched */
	    }
	    if (grid_neighbour (x, y, 0, D)) /* has a via connecting downwards */
		if (point->cost > 0 || !is_state (x, y, 0, pstate)) return (FALSE);
	    tunnelend_found = TRUE;
	}
    }
    else {
	for_all_tunnels (tunnelpointer, point) {
	    x = point->x + tunnelpointer->x;
	    y = point->y + tunnelpointer->y;
	    if (grid_neighbour (x, y, 0, D)) /* has a via connecting downwards */
		if (point->cost > 0 || !is_state (x, y, 0, pstate)) return (FALSE);
	    tunnelend_found = TRUE;
	}
    }

    return (tunnelend_found);
}

/*
 * The next routine performs the actual lee expansion loop
 * it expects:
 *     - an initialized frontlist (for performance reasons, there is
 *       a special front manager (front_begin and front_end should be meaningful)
 *     - a source and destination poked in the grid
 *     - (as parameter) the expansion bounding-box
 *
 * it will return:
 *     - a wire
 *     - a completely 'clean' image, which does not contain source, destination,
 *       or the wire. (they will have to be saved before calling this routine)
 */
static GRIDPOINTPTR lee_expansion (BOXPTR exp_bbx)
{
    COREUNIT pstate;
    GRIDADRESSUNIT x, y, z;
    register GRIDPOINTPTR point, offset, new_point;
    GRIDPOINTPTR wirepattern; /* pattern of the generated wire */
    GRIDPOINTPTR solution_list;
    BOXPTR opp_front;
    register int i, solution_found;
    long tear;

    solution_found = FALSE;
    solution_list = NULL;

    /* The expansion loop: while the front node list exists.
     * Front list must exists and must contain active elements.
     */
    while (solution_found == FALSE && front_begin && front1_size && front2_size)
    {
	/* Take (remove) lowest cost point from front list,
	 * remove this point from the expansion list.
	 */
	point = front_begin; front_begin = front_begin->next;
	if (front_begin) {
	    if (point->prev_block != front_begin) {
		front_begin->prev_block = point->prev_block; front_begin->direction = point->direction;
		front_begin->direction--; front_begin->prev_block->next_block = front_begin;
	    }
	    front_begin->next_block = NULL;
	    front_end->direction--;
	    if (front_end->direction > 4 && front_end->direction <= Prevn) {
		Logn--; Nextn = (Logn + 1)*(Logn + 1); Prevn = (Logn - 1)*(Logn - 1);
	    }
	}
	else front_end = NULL;

	if ((point->pattern & STATEMASK) == FRONT1) front1_size--; else front2_size--;

	if (point->cost > 0) { /* not part of source */
	    if (grid_is_occupied (point)) { free_gridpoint (point); continue; }
	    if (is_destination (point)) {
		link_in_wire_list (point, solution_list);
		solution_found = TRUE;
		continue;
	    }
	    if (!is_free (point)) { free_gridpoint (point); continue; }

	    /* is free: occupy */
	    set_grid_pattern (point, point -> pattern);
	}

	/* Try to expand to all sides: L, B, R, T, D, U.
	 */
	for_all_offsets_of_point (point, offset)
	{
	    if (offset->direction < 0 && exp_bbx->crd[D] >= 0) continue;

	    x = point->x + offset->x;
	    y = point->y + offset->y;
	    z = point->z + offset->z;
	    if (outside_bbx (x, y, z, exp_bbx)) continue;

	    /* Does this offset cross a tear line?
	     * Wires in horizontal layers may not cross horizontal tearlines while
	     * wires in vertical layers may not cross vertical tearlines.
	     */
	    if (LayerOrient[point->z] == VERTICAL && offset->x != 0 && NumTearLine[VERTICAL] > 0)
	    { /* could cross a vertical tear line */
		for (i = 0; i != NumTearLine[VERTICAL]; i++) {
		    tear = TearLine[VERTICAL][i];
		    /* TRUE if the expansion crosses the tearline and is in a vertical layer */
		    if ((point->x <= tear && x > tear) || (x <= tear && point->x > tear)) break;
		}
		if (i < NumTearLine[VERTICAL]) continue; /* it crosses a tearline */
	    }

	    if (LayerOrient[point->z] == HORIZONTAL && offset->y != 0 && NumTearLine[HORIZONTAL] > 0)
	    { /* could cross a horizontal tear line */
		for (i = 0; i != NumTearLine[HORIZONTAL]; i++) {
		    tear = TearLine[HORIZONTAL][i];
		    /* TRUE if the expansion crosses the tearline and is in a horizontal layer */
		    if ((point->y <= tear && y > tear) || (y <= tear && point->y > tear)) break;
		}
		if (i < NumTearLine[HORIZONTAL]) continue; /* it crosses a tearline */
	    }

	    pstate = point->pattern & STATEMASK;

	    /* Can expand into the specified direction?
	     */
	    /* TRUE if the wire is expandable in the specified direction
	     * stacked vias are prevented.
	     * offset->direction >= 0 indicates an ordinary offset,
	     * offset->direction <  0 indicates an offset caused by tunneling.
	     */
	    if (offset->direction < 0 ? (*is_free_in_deep_image)(point, offset) : offset->z == 0 ?
	       (!is_state (x, y, z, STATEMASK) && !is_state (x, y, z, pstate)) : via_allowed (point, offset))
	    { /* a free wire position: expand element */

		new_point = new_gridpoint (x, y, MAX (0, z));
		new_point->pattern = pstate; /* set state to specified front */

		/* calculate cost of the new wire segment (or via) */
		if (point->cost == 0 && offset->direction == -1 && is_free_in_deep_image != is_free_pm25)
		    /* first expansion through a tunnel is (almost) for free */
		    new_point->cost = 2;
		else
		    new_point->cost = point->cost + offset->cost;

		/* strong bias in specified direction? */
		if (Penalty_factor > 1 && offset->direction <= T)
		{ /* vias are excluded from extra penalty */
		    if (pstate == FRONT1) opp_front = &bbx2; else opp_front = &bbx1;

		    /* normal case as well as tunnel */
		    if ((offset->x < 0 && (new_point->x < opp_front->crd[L])) ||
			(offset->x > 0 && (new_point->x > opp_front->crd[R])) ||
			(offset->y < 0 && (new_point->y < opp_front->crd[B])) ||
			(offset->y > 0 && (new_point->y > opp_front->crd[T])))
			/* this is a step into step into the wrong direction! */
			new_point->cost += Penalty_factor; /* add penalty */
		}

		/* set wire pointer backwards */
		if (offset->direction >= 0) { /* normal */
		    add_wire_neighbour (new_point, opposite (offset->direction));
		}
		else { /* deep image */
		    add_grid_neighbour (point, D);
		    add_wire_neighbour (new_point, D);
		}

		/* add new position in front list (list is sorted according to cost of point) */
		insert_in_frontlist (new_point);
	    }
	}

	free_gridpoint (point); /* free the front element */
    }

    /* printf ("frontsizes: %d, %d\n", front1_size, front2_size); */

    /* Get the wire found, store it into structure
     * (if no solutions were found, wirepattern will be NULL).
     */
    wirepattern = trace_wire (solution_list);
    free_gridpoint_list (solution_list);

    /* clear the entire fronts from the trash list */
    rough_clear (exp_bbx);
    free_gridpoint_list (front_begin);
    front_begin = front_end = NULL;

    return (wirepattern);
}

/*
 * This routine expands the terminal pointed at by 'point' and
 * 'saves' these wiring patterns in the wire structure.
 * The expansion is limited by the 'exp_bbx'.
 * The routine will return a bounding box of the terminal.
 */
GRIDPOINTPTR save_source (GRIDPOINTPTR point, BOXPTR terminal_bbx, BOXPTR exp_bbx)
/* point        - input: source point */
/* terminal_bbx - output: bounding box of the source point terminal */
/* exp_bbx      - input: bounding box within which to be searched */
{
    GRIDADRESSUNIT x, y, z;
    GRIDPOINTPTR tunnelendpointer, new_point, new_end, new_beg;
    GRIDPOINTPTR list_end, pattern_list = NULL;
    long offset;

    /* Test if the grid point is already marked (in list).
     * In current implementation marked points are temporarily free.
     */
    if (is_free (point)) return (pattern_list);

    /* create a wire element which stores point */
    pattern_list = point = new_gridpoint (point->x, point->y, point->z);

again:
    if (terminal_bbx) { update_bbx (terminal_bbx, point); }
    set_wire_pattern (point, Grid[point->z][point->y][point->x]);
    list_end = point;
    save_source_nr++;

    /* Mark this grid point as processed in order to pervent looping
     * set_grid_free() will temporarily turn the grid point FREE.
     * A separate routine will have to turn the grid point back ON again.
     * Pointer pattern_list points to the END of the list of wire elements.
     */
    set_grid_free (point);

    new_end = new_beg = NULL;

    for_all_offsets (offset) /* for all possible offsets */
    {
	if (has_neighbour (point, offset)) /* local wiring pattern points to this side */
	{
	    x = point -> x + Xoff[offset];
	    y = point -> y + Yoff[offset];
	    z = point -> z + Zoff[offset];
	    if (outside_bbx (x, y, z, exp_bbx)) continue;

	    if (z < 0) { /* into_deep_image: via which connects to poly/diffusion of image */

		/* walk along all equivalent points */
		for_all_tunnels (tunnelendpointer, point) {

		    x = point -> x + tunnelendpointer -> x;
		    y = point -> y + tunnelendpointer -> y;
		    z = 0;

		    if (is_Free (x, y, z)) continue; /* already in list */

		    if (outside_bbx (x, y, z, exp_bbx)) continue; /* new */

		    if (grid_neighbour (x, y, z, D) /* point must have a via to connect through */
			&& is_resistor (x, y) == FALSE) /* resistor-hack: look forward one */
		    {
			new_point = new_gridpoint (x, y, z);
			if (!new_beg) new_beg = new_point; else new_end -> next = new_point;
			new_end = new_point;
		    }
		}

		/* also expand into restricted tunnels (see remove_restricted_core_feeds) */
		for_all_restricted_tunnels (tunnelendpointer, point) {

		    x = point -> x + tunnelendpointer -> x;
		    y = point -> y + tunnelendpointer -> y;
		    z = 0;

		    /* only if allowed at that position, tunnelend must be free */
		    if (!is_Free (x, y, z)) continue;

		    if (outside_bbx (x, y, z, exp_bbx)) continue; /* new */

		    new_point = new_gridpoint (x, y, z);
		    add_wire_neighbour (new_point, D); /* the via */
		    new_point->direction = -1; /* indicates a temporary part of source */
		    new_point->cost = tunnelendpointer->cost; /* set cost of this expansion */
		    if (!new_beg) new_beg = new_point; else new_end -> next = new_point;
		    new_end = new_point;
		}
	    }
	    else {
		if (is_Free (x, y, z)) continue; /* already in list */
		new_point = new_gridpoint (x, y, z);
		if (!new_beg) new_beg = new_point; else new_end -> next = new_point;
		new_end = new_point;
	    }
	}
    }

    if (new_beg) {
	new_end -> next = point -> next;
	point -> next = new_beg;
    }
    point = point -> next;

    while (point && is_free (point)) {
	new_point = point -> next;
	if (point->direction == -1) { /* indicates a temporary part of source */
	    list_end -> next = point;
	    list_end = point;
	    save_source_nr++;
	}
	else free_gridpoint (point);
	point = new_point;
    }
    list_end -> next = point;
    if (point) goto again;

    return (pattern_list);
}

/*
 * return TRUE if the pattern is probably a resistor
 */
static int is_resistor (GRIDADRESSUNIT x, GRIDADRESSUNIT y)
{
    int offset;

    if (Resistor_hack == FALSE) return (FALSE);

    /* only have connections to either left or right */
    if (grid_neighbour (x, y, 0, B)) return (FALSE);
    if (grid_neighbour (x, y, 0, T)) return (FALSE);
    if (grid_neighbour (x, y, 0, U)) return (FALSE);

    /* is it left or right?, if both, return false */
    offset = L;
    if (grid_neighbour (x, y, 0, L)) {
	if (grid_neighbour (x, y, 0, R)) return (FALSE);
    }
    else if (grid_neighbour (x, y, 0, R)) offset = R;
    else return (FALSE);

    x += Xoff[offset];
    if (!grid_neighbour (x, y, 0, D)) return (FALSE);

    /* find neighbouring point */
    /* may not have connections in direction B, T, U or offset */
    if (grid_neighbour (x, y, 0, B)) return (FALSE);
    if (grid_neighbour (x, y, 0, T)) return (FALSE);
    if (grid_neighbour (x, y, 0, U)) return (FALSE);
    if (grid_neighbour (x, y, 0, offset)) return (FALSE);

    return (TRUE);
}

/*
 * This routine initializes the front list to the wiring pattern of the net.
 * The front nodes are tagged with the front_id.
 */
GRIDPOINTPTR mark_as_front (GRIDPOINTPTR frontp, COREUNIT front_id, int in_front)
/* frontp   - terminal pattern to be copied */
/* front_id - id-tag of front (FRONT1 or FRONT2) */
/* in_front - >= 0 to add into front */
{
    register GRIDPOINTPTR frontpattern, new_point;

    /* step along terminal pattern */
    for (frontpattern = frontp; frontpattern; frontpattern = frontpattern->next)
    {
	/* Belongs the frontpattern already to the opposite front?
	 * We mis-use the fact that the grid points were marked,
	 * therefore the front ID is not used in this test.
	 */
	if (!is_free (frontpattern)) { /* already connected? */
	    if (frontpattern->direction >= 0) { /* not temporary */
		/* error (WARNING, "(mark_as_front): already connected"); */
		free_gridpoint_list (front_begin);
		front_begin = front_end = NULL;
		return (NULL);
	    }
	    else continue; /* skip if temp via of restricted tunnel */
	}
	/* is_free(frontpattern): gridpoint has no state (= ID) */

	if (in_front >= 0) { /* link a new point in the front list */
	    new_point = new_gridpoint (frontpattern->x, frontpattern->y, frontpattern->z);
	    new_point->pattern = front_id; /* tag point with the front ID */
	    insert_in_frontlist (new_point);
	    if (frontpattern->direction < 0) { /* temp connection from restricted tunnel */
		add_wire_neighbour (new_point, D); /* add via */
		new_point->cost = frontpattern->cost; /* cost of restricted feed */
	    }
	}
	if (frontpattern->direction >= 0) { /* tag gridpoint with the front ID */
	    Grid[frontpattern->z][frontpattern->y][frontpattern->x] |= front_id;
	}
    }

    return (frontp);
}

/*
 * trace a solution path from the expansion front
 */
static GRIDPOINTPTR trace_wire (GRIDPOINTPTR list)
{
    GRIDPOINT walkpointstruct;
    register GRIDPOINTPTR tunnelpointer, tunnelendpointer, new_point;
    GRIDPOINTPTR wire_pattern;
    register GRIDPOINTPTR walkpoint;
    register long prev_neighbour, tmpoffset, offset;
    long step, cost, first_offset;
    int hit_terminal, bitval;

    if (!list) return (NULL);

    first_offset = HERE;
    cost = 0;
    wire_pattern = NULL;
    walkpoint = &walkpointstruct;

    for (step = 0; step != 2; step++)
    {
	walkpoint->x = list->x;
	walkpoint->y = list->y;
	walkpoint->z = list->z;

	if (step == 0) { /* first time */
	    /* trace wire pointed at by list */
	    first_offset = offset;
	    exclusive_source (walkpoint, offset);
	    first_offset = offset;
	}
	else { /* second time: trace wire from pattern in list */
	    set_grid_pattern (walkpoint, get_wire_pattern (list));
	    exclusive_source (walkpoint, offset);
	    /* to prevent retrace of same path as first time */
	    if (offset == first_offset) continue;
	}

	new_point = NULL;

	/* find offset
	*/
	prev_neighbour = HERE;

	if (offset == HERE) { /* point of source or destination could result in HERE offset */
	    new_point = new_gridpoint (walkpoint->x, walkpoint->y, walkpoint->z);
	    link_in_wire_list (new_point, wire_pattern);
	    set_wire_pattern (new_point, get_grid_pattern (walkpoint));
	    add_wire_occupied (new_point);
	}

	/* loop until illegular or no pattern
	*/
	while (offset != HERE && offset != prev_neighbour)
	{
	    cost += 1; /*  Cost[walkpoint->z][offset]; */

	    /* create new point struct with current coordinates
	     * and pattern (also link it in pattern list)
	     */
	    new_point = new_gridpoint (walkpoint->x, walkpoint->y, walkpoint->z);
	    link_in_wire_list (new_point, wire_pattern);

	    add_wire_occupied (new_point);
	    add_wire_neighbour (new_point, offset);
	    add_wire_neighbour (new_point, prev_neighbour);
	    prev_neighbour = opposite (offset);

	    if (into_deep_image (walkpoint, offset)) { /* connects to core */
		add_wire_neighbour (new_point, D);
		offset = HERE;
		hit_terminal = FALSE;
		/* find the exit point of the tunnel */
		tunnelendpointer = NULL;
		for_all_tunnels (tunnelpointer, walkpoint)
		{
		    if (walkpoint->x + tunnelpointer->x < Bbx->crd[L] ||
			walkpoint->x + tunnelpointer->x > Bbx->crd[R] ||
			walkpoint->y + tunnelpointer->y < Bbx->crd[B] ||
			walkpoint->y + tunnelpointer->y > Bbx->crd[T])
		    { /* outside image */  /* new */
			/* fprintf (stderr, "WARNING: tunnel outside bbx\n"); */
			continue;
		    }

		    /* look for end of tunnel */
		    exclusive_source_tunnel (walkpoint, tunnelpointer, tmpoffset);

		    bitval = Grid[0][walkpoint->y + tunnelpointer->y][walkpoint->x + tunnelpointer->x] & 0xf;
		    if (tmpoffset != HERE || (has_neighbour_offset (walkpoint, D, tunnelpointer) &&
			    (bitval && bitval != 1 && bitval != 2 && bitval != 4 && bitval != 8))) {
			if (tmpoffset == HERE) hit_terminal = TRUE;

			if (tunnelendpointer == NULL) { /* first time */
			    offset = tmpoffset;
			    tunnelendpointer = tunnelpointer;
			}
			else { /* store the one with the largest offset */
			    if (hit_terminal == FALSE &&
				(ABS(tunnelpointer->x) + ABS(tunnelpointer->y) >
				(ABS(tunnelendpointer->x) + ABS(tunnelendpointer->y))))
			    { /* new maximum */
				offset = tmpoffset;
				tunnelendpointer = tunnelpointer;
			    }
			    if (tmpoffset == HERE &&
				(ABS(tunnelpointer->x) + ABS(tunnelpointer->y) <
				(ABS(tunnelendpointer->x) + ABS(tunnelendpointer->y))))
			    { /* new minumum */
				offset = tmpoffset;
				tunnelendpointer = tunnelpointer;
			    }
			}
		    }
		}

		if (tunnelendpointer) { /* found end of tunnel */
		    GRIDADRESSUNIT x, y, z;
		    x = walkpoint->x + tunnelendpointer->x;
		    y = walkpoint->y + tunnelendpointer->y;
		    z = walkpoint->z + tunnelendpointer->z; if (z < 0) z = 0;

		    if (hit_terminal == TRUE) { /* we hit a spot of the terminal pattern */
			/* make a contact over there.... */
			new_point = new_gridpoint (x, y, z);
			link_in_wire_list (new_point, wire_pattern);
			add_wire_occupied (new_point);
			add_wire_neighbour (new_point, D);
			offset = HERE; /* stop */
			prev_neighbour = D;
		    }
		    else { /* normal... */
			walkpoint->x = x;
			walkpoint->y = y;
			walkpoint->z = z;
			prev_neighbour = D;

			/* stop if outside image */
			if (x < Bbx->crd[L] || x > Bbx->crd[R] ||
			    y < Bbx->crd[B] || y > Bbx->crd[T] ||
			    z < Bbx->crd[D] || z > Bbx->crd[U]) offset = HERE;
		    }
		}

		new_point = NULL;
	    }
	    else { /* step */
		if (outside_image (walkpoint, offset)) break;

		step_point (walkpoint, offset);

		/* get new offset */
		exclusive_source (walkpoint, offset);
	    }
	}
    }

    /* store cost */
    if (wire_pattern) wire_pattern->cost = cost;

    return (wire_pattern);
}

/*
 * brute-force clear of expansion front
 */
static void rough_clear (BOXPTR exp_bbx)
{
    register GRIDADRESSUNIT z, y, x;

    for (z = 0; z <= exp_bbx->crd[U]; ++z)
    for (y = exp_bbx->crd[B]; y <= exp_bbx->crd[T]; ++y)
    for (x = exp_bbx->crd[L]; x <= exp_bbx->crd[R]; ++x) {
	if (!is_occupied (x, y, z)) set_grid_clear_c (x, y, z);
    }
}

/*
 * This routine will return TRUE if it is possible
 * to expand into the fixed poly/diff layers of the image.
 * This is the most time-consuming routine of the program.
 * For efficiency reasons, there are three versions of is_free_in_deep_image():
 * is_free_overlapping(), is_free_nonoverlapping() and is_free_pm25().
 */
static int is_free_nonoverlapping (GRIDPOINTPTR point, GRIDPOINTPTR offset)
{
    register GRIDPOINTPTR tunnelpointer;
    register int tunnelend_found = FALSE;
    register COREUNIT pstate;
    register GRIDADRESSUNIT x, y;

    pstate = point->pattern & STATEMASK;

    /* the exit point itself must be free, or destination */
    x = point->x + offset->x;
    y = point->y + offset->y;
    if (is_state (x, y, 0, STATEMASK) || is_state (x, y, 0, pstate)) return (FALSE);

    /* to avoid stacked contacts */
    if (has_neighbour (point, U)) return (FALSE); /* entered point from top */
    if (grid_neighbour (x, y, 0, U)) return (FALSE);

    for_all_tunnels (tunnelpointer, point) {
	x = point->x + tunnelpointer->x;
	y = point->y + tunnelpointer->y;
	if (grid_neighbour (x, y, 0, D)) /* has a via connecting downwards */
	    if (point->cost > 0 || !is_state (x, y, 0, pstate)) return (FALSE);
	tunnelend_found = TRUE;
    }

    return (tunnelend_found);
}

/*
 * For a general image with overlaps more expensive.
 */
static int is_free_overlapping (GRIDPOINTPTR point, GRIDPOINTPTR offset)
{
    register GRIDPOINTPTR tunnelpointer;
    register int tunnelend_found = FALSE;
    register COREUNIT pstate;
    register GRIDADRESSUNIT x, y;

    pstate = point->pattern & STATEMASK;

    /* the exit point itself must be free, or destination */
    x = point->x + offset->x;
    y = point->y + offset->y;
    if (is_state (x, y, 0, STATEMASK) || is_state (x, y, 0, pstate)) return (FALSE);

    /* to avoid stacked contacts */
    if (has_neighbour (point, U)) return (FALSE); /* entered point from top */
    if (grid_neighbour (x, y, 0, U)) return (FALSE);

    for_all_tunnels (tunnelpointer, point) {
	x = point->x + tunnelpointer->x;
	y = point->y + tunnelpointer->y;
	if (x < Bbx->crd[L] || x > Bbx->crd[R] || y < Bbx->crd[B] || y > Bbx->crd[T])
	{ /* ATTENTION: this is important: eihter disable, or enable */
	    if (point->cost == 0 || Use_borders == TRUE)
		continue; /* enable: use feeds on borders */
	    else
		return (FALSE); /* disable: leave feeds in borders untouched */
	}
	if (grid_neighbour (x, y, 0, D)) /* has a via connecting downwards */
	    if (point->cost > 0 || !is_state (x, y, 0, pstate)) return (FALSE);
	tunnelend_found = TRUE;
    }

    return (tunnelend_found);
}

/*
 * This routine selects which is_free_in_deep_image() routine is to be used.
 */
void set_image_type_for_lee ()
{
    register GRIDPOINTPTR tunnelpointer;
    register GRIDADRESSUNIT x, y;

    /* first case (ugly, I know...) is it pm25?
     */
    if (strncmp (ThisImage, "pm25", 4) == 0 && Chip_num_layer == 1) {
	/* printf ("PM25 image\n"); fflush (stdout); */
	is_free_in_deep_image = is_free_pm25;
	return;
    }

    /* scan the entire image whether it is overlapping or nonoverlapping
     */
    for (x = 0; x < GridRepitition[X]; x++) {
    for (y = 0; y < GridRepitition[Y]; y++) {
	for_all_tunnels2 (tunnelpointer, x, y) {
	    if (x + tunnelpointer->x < 0 || x + tunnelpointer->x >= GridRepitition[X] ||
		y + tunnelpointer->y < 0 || y + tunnelpointer->y >= GridRepitition[Y])
	    { /* overlaps */
		/* printf ("overlapping\n"); fflush (stdout); */
		is_free_in_deep_image = is_free_overlapping;
		return;
	    }
	}
    }}

    /* printf ("nonoverlapping\n"); fflush (stdout); */
    is_free_in_deep_image = is_free_nonoverlapping;
}

/*
 * This routine will return TRUE if it is possible to make a via.
 * This routine is called very often.
 */
static int via_allowed (GRIDPOINTPTR point, GRIDPOINTPTR offset)
/* offset - U or D */
{
    int opp;
    register COREUNIT pstate;
    register GRIDADRESSUNIT x, y, z;

    pstate = point->pattern & STATEMASK;

    /* target point itself must be either free or destination */
    x = point->x;
    y = point->y;
    z = point->z + offset->z;
    if (is_state (x, y, z, STATEMASK) || is_state (x, y, z, pstate)) return (FALSE);

    /* determine opposite */
    if (offset->direction == U) opp = D; else opp = U;

    /* came from other direction: not allowed */
    z = point->z;
    if (grid_neighbour (x, y, z, opp)) return (FALSE);

    z += Zoff[opp];
    if (z <= Bbx->crd[U] && z >= Bbx->crd[D]) /* other side of via exists */
	if (has_neighbour_o (point, offset->direction, opp)) return (FALSE);

    /* patrick NEW, stacked vias in multi-layer */
    if (has_neighbour_o (point, offset->direction, offset->direction)) return (FALSE);

    return (TRUE);
}

/*
 * insert cleverly in list, such that the list is sorted
 *
 *     low cost                                                                        high cost
 *
 *        el --next--> el --next--> el --next--> el --next--> el --next--> el --next--> el
 *        ^                                       ^                                     ^
 *        \--<<-next_block-----prev_block->>-----/\--<<-next_block----prev_block->>-----/
 *
 *        ^                                                                             ^
 *        |                                                                             |
 *    front_begin                                                                   front_end
 *
 * front_end->direction contains the number of elements in the list
 */
static void insert_in_frontlist (GRIDPOINTPTR point)
{
    register GRIDPOINTPTR superblock, hpoint, prev_p;
    long step1, step2;

    /* count number of front elements */
    if ((point -> pattern & STATEMASK) == FRONT1) front1_size++; else front2_size++;

    if (front_end == NULL) {
	front_end = front_begin = point;
	front_end -> direction = 1; /* will be incremented two times */
	/* minimum of 2 in list */
	Logn = 2; Nextn = 9; Prevn = 1;
	return;
    }
    else if (point->cost >= front_end->cost) { /* enlarge list on high-cost end */
	front_end->next = point;
	if (front_end->next_block)
	    superblock = point->next_block = front_end->next_block;
	else
	    superblock = point->next_block = front_end;
	superblock->prev_block = point;
	point->direction = front_end->direction;
	front_end = point;
    }
    else { /* insert somewhere in list */

	/* step to superblock in question */
	for (superblock = front_end->next_block;
	    superblock && point->cost < superblock->cost;
		superblock = superblock->next_block) /* nothing */ ;

	if (superblock == NULL) { /* insert at low-cost beginning */
	    point->next = front_begin;
	    superblock = point;
	    superblock->direction = front_begin->direction; /* nr in block */
	    if (front_begin->prev_block)
		superblock->prev_block = front_begin->prev_block;
	    else
		superblock->prev_block = front_begin;
	    superblock->next_block = NULL;
	    superblock->prev_block->next_block = superblock;
	    front_begin = superblock;
	}
	else { /* step to detailed point */
	    prev_p = superblock;
	    for (hpoint = superblock->next;
		point->cost > hpoint->cost; hpoint = hpoint->next) prev_p = hpoint;
	    /* actual insert */
	    point->next = hpoint;
	    prev_p->next = point;
	}
    }

    superblock -> direction++; /* increment number in block */
    front_end  -> direction++; /* increment number in list */

    if (front_end -> direction >= Nextn) { /* determine logn */
	Logn++;
	Nextn = (Logn + 1)*(Logn + 1);
	Prevn = (Logn - 1)*(Logn - 1);
    }

    if (superblock -> direction > Logn) { /* split superblock if over threshold */
	/* step to middle */
	step1 = superblock->direction/2;
	step2 = superblock->direction - step1;
	for (superblock->direction = 1, hpoint = superblock->next;
	    superblock->direction < step1; superblock->direction++, hpoint = hpoint->next);

	hpoint->direction = step2;
	hpoint->next_block = superblock;
	hpoint->prev_block = superblock->prev_block;
	hpoint->next_block->prev_block = hpoint;
	hpoint->prev_block->next_block = hpoint;
    }
}

/*
 * This routine returns the escape path from 'point' to the tearline.
 */
GRIDPOINTPTR make_escape_path (GRIDPOINTPTR point, int orient, long pos, BOXPTR bbx)
/* point  - point from which expansion should take place */
/* orient - orientation (HORIZONTAL/VERTICAL) of wire */
/* pos    - position of the tear line */
/* bbx    - bounding box of the expansion */
{
    GRIDPOINT pointstruct;
    GRIDPOINTPTR walkpoint, term_pattern, wire_pattern;
    BOX bbxx, exp_bbxstruct; /* bounding boxes of source patterns */
    BOXPTR exp_bbx;
    long num_attempts;

    if (!bbx) bbx = Bbx;

    exp_bbx = &exp_bbxstruct;
    walkpoint = &pointstruct;

    /* find out: left or right of wire
    */
    if (orient == HORIZONTAL) {
	walkpoint->x = 0; /* suppress compiler uninit warning */
	if (point->y > pos) {
	    walkpoint->y = exp_bbx->crd[B] = pos + 1;
	    exp_bbx->crd[T] = bbx->crd[T];
	}
	else {
	    walkpoint->y = exp_bbx->crd[T] = pos;
	    exp_bbx->crd[B] = bbx->crd[B];
	}
    }
    else {
	walkpoint->y = 0; /* suppress compiler uninit warning */
	if (point->x > pos) {
	    walkpoint->x = exp_bbx->crd[L] = pos + 1;
	    exp_bbx->crd[R] = bbx->crd[R];
	}
	else {
	    walkpoint->x = exp_bbx->crd[R] = pos;
	    exp_bbx->crd[L] = bbx->crd[L];
	}
    }
    exp_bbx->crd[D] = bbx->crd[D];
    exp_bbx->crd[U] = bbx->crd[U];

    /* to reduce cpu-time: do first in small strip (-4 to +4)
    */
    num_attempts = 0;
    term_pattern = NULL;

    do {
	if (num_attempts == 0) {
	    /* first time: restrict size of expansion area to small strip */
	    if (orient == HORIZONTAL) {
		exp_bbx->crd[L] = MAX (bbx->crd[L], point->x - 4);
		exp_bbx->crd[R] = MIN (bbx->crd[R], point->x + 4);
	    }
	    else {
		exp_bbx->crd[B] = MAX (bbx->crd[B], point->y - 4);
		exp_bbx->crd[T] = MIN (bbx->crd[T], point->y + 4);
	    }
	}
	else { /* second time: try again, max size bbx */
	    if (orient == HORIZONTAL) {
		exp_bbx->crd[L] = bbx->crd[L];
		exp_bbx->crd[R] = bbx->crd[R];
	    }
	    else {
		exp_bbx->crd[B] = bbx->crd[B];
		exp_bbx->crd[T] = bbx->crd[T];
	    }
	    free_and_restore_wire_pattern (term_pattern);
	}

	/* print destination in each vertical layer */
	for (walkpoint->z = 0; walkpoint->z <= exp_bbx->crd[U]; walkpoint->z++) {
	    if (LayerOrient[walkpoint->z] == orient) continue;

	    /* walk along line
	    */
	    if (orient == HORIZONTAL) {
		for (walkpoint->x = exp_bbx->crd[L]; walkpoint->x <= exp_bbx->crd[R]; walkpoint->x++)
		    if (!is_occupied (walkpoint->x, walkpoint->y, walkpoint->z))
			set_grid_pattern (walkpoint, FRONT2); /* free: set to front 2 */
	    }
	    else {
		for (walkpoint->y = exp_bbx->crd[B]; walkpoint->y <= exp_bbx->crd[T]; walkpoint->y++)
		    if (!is_occupied (walkpoint->x, walkpoint->y, walkpoint->z))
			set_grid_pattern (walkpoint, FRONT2); /* free: set to front 2 */
	    }
	}

	/* Expand the pattern around terminal 1: find all points connected
	* to it. These points are 'saved' in this pattern.
	* The routine will return NULL if the point adresses an empty grid
	*/
	if (!(term_pattern = save_source (point, &bbxx, exp_bbx))) {
	    rough_clear (exp_bbx);
	    return (NULL);
	}

	front_begin = front_end = NULL;

	/* Mark the source and destination patterns as such.
	* The routine will copy the frontpatterns and return the initialized front point list
	* Null will be returned if the fronts are already connected.
	*/
	front1_size = 0; front2_size = -1;
	if (!mark_as_front (term_pattern, FRONT1, front1_size)) { /* clean up */
	    free_and_restore_wire_pattern (term_pattern);
	    rough_clear (exp_bbx);
	    return (NULL);
	}

	/* front 2 is already marked */

	/* perform the actual expansion....
	*/
	wire_pattern = lee_expansion (exp_bbx);

	num_attempts++;
    } while (!wire_pattern && num_attempts < 2);

    /* restore the terminal pattern
    */
    free_and_restore_wire_pattern (term_pattern);

    wire_pattern = reverse_list (wire_pattern);

    return (wire_pattern);
}

/*
 * This routine initializes the front list to the wiring pattern of the net.
 * The front nodes are tagged with the front_id.
 */
static int count_number_of_points (GRIDPOINTPTR gridp)
{
    register int count;
    for (count = 0; gridp; gridp = gridp -> next) count++;
    return (count);
}

/*
 * This routine checks the connectivity of the specified netlist
 * with the one found in the grid.
 */
int verify_connectivity (LAYOUTPTR father, BOXPTR rbbx)
{
    GRIDPOINTPTR terminalpattern; /* expanded pattern of terminal */
    NETPTR hnet, tnet;
    CIRPORTREFPTR hportref, tportref;
    BOX exp_bbxstruct;
    BOXPTR exp_bbx;
    R_PORTPTR rport, tport, last_tport;
    GRIDPOINT point;
    int num_short_circuit_of_net, term_num_check, term_unconn;
    int short_printed, term_in_group, num_not_to_border;
    int num_short, num_unconn, num_net;
    BOXPTR father_bbx, routing_bbx;
    char shortstr[100];
    FILE *fp;
    long the_time, time();

    if (!(fp = fopen ("seadif/trout.error", "w"))) {
	fprintf (stderr, "WARNING: cannot open error file 'seadif/trout.error'\n");
	fp = stderr;
    }
    else {
	the_time = time(0);
	fprintf (fp, "This is the file 'seadif/trout.error' which was created by\n");
	fprintf (fp, "the program 'trout' on %s", asctime (localtime (&the_time)));
	fprintf (fp, "Trout was verifying the netlist description of\n");
	if (strcmp (father->name, "Tmp_Cell_") == 0)
	  fprintf (fp, "your circuit circuit '%s' with the current layout in seadali.\n", father->circuit->name);
	else
	  fprintf (fp, "your circuit circuit '%s' with its layout '%s'.\n", father->circuit->name, father->name);
	fprintf (fp, "The following nets in the layout are not according to\n");
	fprintf (fp, "the netlist description of circuit '%s'. They are also\n", father->circuit->name);
	fprintf (fp, "marked in the layout.\n");
	fprintf (fp, "------------------------- list of errors -------------------------\n");
    }

    printf ("------ Verifying Connectivity ------\n");
    fflush (stdout);

    Bbx = father_bbx = &((R_CELLPTR) father->flag.p)->cell_bbx;

    exp_bbx = &exp_bbxstruct;
    exp_bbx->crd[L] = father_bbx->crd[L];
    exp_bbx->crd[R] = father_bbx->crd[R];
    exp_bbx->crd[B] = father_bbx->crd[B];
    exp_bbx->crd[T] = father_bbx->crd[T];
    exp_bbx->crd[D] = -1;
    exp_bbx->crd[U] = father_bbx->crd[U];

    routing_bbx = rbbx ? rbbx : father_bbx;

    num_short = 0;
    num_unconn = 0;
    num_net = 0;
    num_not_to_border = 0;

    /* reset the routed flag of all terminals to FALSE, meaning that
     * they do not belong to any connected pattern for which a
     * warning could have been printed
     */
    for (hnet = father->circuit->netlist; hnet; hnet = hnet->next) {
	/* have a look at the terminals */
	if (!hnet->terminals || hnet->num_term < 2) continue;

	num_net++;

	/* reset flag.. */
	for (hportref = hnet->terminals; hportref; hportref = hportref->next) {
	    if (!(rport = (R_PORTPTR) hportref->flag.p)) continue;
	    rport->routed = FALSE;
	}
    }

    /* main loop: walk along nets */
    for (hnet = father->circuit->netlist; hnet; hnet = hnet->next)
    {
	/* have a look at the terminals */
	if (!hnet->terminals || hnet->num_term < 2) continue;

	term_num_check = term_unconn = 0;
	for (hportref = hnet->terminals; hportref; hportref = hportref->next)
	{
	    if (!(rport = (R_PORTPTR) hportref->flag.p)) continue;
	    if (rport->routed == TRUE) continue; /* already checked.. */

	    /* set it to routed = mark processed .. */
	    rport->routed = TRUE;

	    short_printed = FALSE;
	    term_in_group = 1;

	    /* if it is an unassigned terminal: check whether it
	       is on the border of the routing bounding box */
	    if (RouteToBorder == TRUE && rport->unassigned == TRUE &&
		hnet->flag.p && ((R_NETPTR)hnet->flag.p)->type != POWER) { /* yes, it is */
		if (rport->crd[X] > routing_bbx->crd[L] &&
		    rport->crd[X] < routing_bbx->crd[R] &&
		    rport->crd[Y] > routing_bbx->crd[B] &&
		    rport->crd[Y] < routing_bbx->crd[T]) {
		    num_not_to_border++;
		    if (fp) fprintf (fp, "WARNING: Net has no connection to the border: '%s'\n", hnet->name);
		    /* add marker... */
		    sprintf (shortstr, "NotToBorder%d", num_not_to_border);
		    add_unconnect (father, shortstr, rport->crd[X], rport->crd[Y], rport->crd[Z]);
		}
	    }

	    /* have a good look at the terminal: expand it */
	    point.x = rport->crd[X];
	    point.y = rport->crd[Y];
	    point.z = rport->crd[Z];
	    if (point.x < father_bbx->crd[L] || point.x > father_bbx->crd[R] ||
		point.y < father_bbx->crd[B] || point.y > father_bbx->crd[T] ||
		point.z < father_bbx->crd[D] || point.z > father_bbx->crd[U]) {
		/*
		if (fp) {
		    fprintf (fp, "ERROR: terminal '%s' of net '%s' is outside the routing area\n", rport->layport->cirport->name, hnet->name);
		    fprintf (fp, "       Therefore it is not connected....\n");
		}
		*/
		continue;
	    }
	    if (!(terminalpattern = save_source (&point, NULL, exp_bbx))) continue;
	    if (!mark_as_front (terminalpattern, FRONT1, -1)) {
		free_and_restore_wire_pattern (terminalpattern);
		continue;
	    }

	    /* melt the wires of adjacent patterns together */
	    if (Verify_only == FALSE && is_free_in_deep_image != is_free_pm25)
		melt_pattern (terminalpattern, exp_bbx);

	    /* check whether all terminals of this group connect to this terminal.. */
	    for (tportref = hportref->next; tportref; tportref = tportref->next) {
		if (!(tport = (R_PORTPTR) tportref->flag.p)) continue;
		if (tport->routed == TRUE) continue; /* already checked.. */
		if (tport->crd[X] < father_bbx->crd[L] ||
		    tport->crd[X] > father_bbx->crd[R] ||
		    tport->crd[Y] < father_bbx->crd[B] ||
		    tport->crd[Y] > father_bbx->crd[T] ||
		    tport->crd[Z] < father_bbx->crd[D] ||
		    tport->crd[Z] > father_bbx->crd[U]) continue; /* outside bbx... */

		term_num_check++;

		if ((Grid[(tport->crd[Z])][(tport->crd[Y])][(tport->crd[X])] & STATEMASK) != FRONT1)
		{ /* unconnect */
		    if (term_unconn == 0)
			add_error_unconnect (hnet, rport->crd[X], rport->crd[Y], rport->crd[Z]);
		    term_unconn++;
		    term_in_group++;
		    add_error_unconnect (hnet, tport->crd[X], tport->crd[Y], tport->crd[Z]);
		}
		else { /* connected... */
		    tport->routed = TRUE;
		}
	    }

	    short_printed = FALSE;

	    /* check whether it shortcuts to any other net */
	    for (tnet = hnet->next; tnet; tnet = tnet->next) {
		num_short_circuit_of_net = 0;
		last_tport = NULL;
		for (tportref = tnet->terminals; tportref; tportref = tportref->next) {
		    if (!(tport = (R_PORTPTR) tportref->flag.p)) continue;
		    if (tport->crd[X] < father_bbx->crd[L] ||
			tport->crd[X] > father_bbx->crd[R] ||
			tport->crd[Y] < father_bbx->crd[B] ||
			tport->crd[Y] > father_bbx->crd[T] ||
			tport->crd[Z] < father_bbx->crd[D] ||
			tport->crd[Z] > father_bbx->crd[U]) continue; /* outside bbx... */

		    if (tport->routed == TRUE) continue;

		    if ((Grid[(tport->crd[Z])][(tport->crd[Y])][(tport->crd[X])] & STATEMASK) == FRONT1)
		    { /* short circuit */
			tport->routed = TRUE;
			last_tport = tport;
			num_short_circuit_of_net++;
		    }
		}
		if (num_short_circuit_of_net > 0) {
		    if (short_printed == FALSE) { /* first time... */
			short_printed = TRUE;
			num_short++;
			if (fp) {
			    fprintf (fp, "##### suspected short-circuit no. %d:\n", num_short);
			    fprintf (fp, "  > net '%s' (%d terms, e.g. terminal  '%s')\n",
				hnet->name, term_in_group, rport->layport->cirport->name);
			}
			/* add marker... */
			sprintf (shortstr, "SHORT_%d", num_short);
			add_unconnect (father, shortstr, rport->crd[X], rport->crd[Y], rport->crd[Z]);
		    }
		    if (fp) fprintf (fp, "  > net '%s' (%d terms, e.g. terminal '%s')\n",
				tnet->name, num_short_circuit_of_net, last_tport->layport->cirport->name);
		    add_unconnect (father, shortstr, last_tport->crd[X], last_tport->crd[Y], last_tport->crd[Z]);
		}
	    }

	    /* checks are over */
	    /* if problems were found: poke terminalpattern in marker layer */
	    if (HaveMarkerLayer == TRUE && short_printed == TRUE) {
		copy_to_marker_layer (terminalpattern);
	    }

	    free_and_restore_wire_pattern (terminalpattern);
	}

	if (term_unconn > 0) { /* net is not fully connected */
	    num_unconn++;
	    if (term_unconn == term_num_check) { /* noting routed */
		if (fp) { /* SdeG: fake cvsoverview */
		    int i; for (i = 0; i < 5; ++i) shortstr[i] = '@'; shortstr[i] = 0;
		    fprintf (fp, "%s entirely unconnected net: '%s' (%d terminals)\n",
				shortstr, hnet->name, term_num_check + 1);
		}
	    }
	    else { /* something connected */
		if (fp) {
		    int i; for (i = 0; i < 5; ++i) shortstr[i] = '@'; shortstr[i] = 0;
		    fprintf (fp, "%s partially unconnected net: '%s' (%d unconnects among %d)\n",
				shortstr, hnet->name, term_unconn, term_num_check);
		}
	    }
	}
    }

    if (fp) fprintf (fp, "\n");

    if (num_unconn > 0) {
	if (fp) {
	    fprintf (fp, "WARNING: %d out of the %d nets are not properly connected.\n", num_unconn, num_net);
	  if (num_unconn == num_net)
	    fprintf (fp, "         Apparently the cell doesn't contain a routing pattern.\n");
	    fprintf (fp, "         In the layout each of the unconnects is marked by\n");
	    fprintf (fp, "         an arrow followed by the name of the net: <=-<net_name>\n");
	}
	fprintf (stderr, "WARNING: %d out of the %d nets are not properly connected.\n", num_unconn, num_net);
    }

    if (num_short > 0) {
	if (fp) {
	    fprintf (fp, "WARNING: %d short-circuit(s) were detected among the %d nets.\n", num_short, num_net);
	    fprintf (fp, "         These nets were separatedly specified in the circuit\n");
	    fprintf (fp, "         but are connected in the layout.  This may occur because of\n");
	    fprintf (fp, "         feed-throughs in sub-cells and it doesn't necessarily\n");
	    fprintf (fp, "         imply an error.\n");
	    fprintf (fp, "         In the layout each short-circuit is indicated by a marker\n");
	    fprintf (fp, "         which indicates a position on the pattern: <=-SHORT_<no>\n");
	}
	fprintf (stderr, "WARNING: %d short-circuit(s) were detected among the %d nets.\n", num_short, num_net);
    }

    if (num_not_to_border > 0) {
	if (fp) {
	    fprintf (fp, "WARNING: %d net(s) are not connected to the border.\n", num_not_to_border);
	    fprintf (fp, "         In the layout these nets are indicated by a marker\n");
	    fprintf (fp, "         which indicates a position on the pattern: <=-NotToBorder\n");
	}
	fprintf (stderr, "WARNING: %d net(s) are not connected to the border.\n", num_not_to_border);
    }

    if (num_short == 0 && num_unconn == 0 && num_not_to_border == 0) {
	if (fp && fp != stderr) {
	    fclose (fp);
	    unlink ("seadif/trout.error"); /* remove the file! */
	}
	printf ("------ No unconnect nor short-circuits ------\n");
	fflush (stdout);
    }
    else {
	if (fp && fp != stderr) {
	    fprintf (fp, "------------------------------------------------------------------\n");
	    fclose (fp);
	}
    }

    return (num_short + num_unconn + num_not_to_border);
}

/*
 * highlight a specific net
 */
void highlight_net (LAYOUTPTR father, char *name)
/* name - name of the net to be highlighted */
{
    GRIDPOINTPTR terminalpattern; /* expanded pattern of terminal */
    GRIDPOINTPTR netpattern;      /* pattern of net */
    GRIDPOINTPTR hpoint;
    GRIDPOINT point;
    NETPTR hnet;
    R_PORTPTR rport;
    CIRPORTREFPTR hportref;
    BOX exp_bbx;
    BOXPTR father_bbx, routing_bbx;
    FILE *fp = stderr;

    if (name) { /* find the net */
	for (hnet = father->circuit->netlist; hnet; hnet = hnet -> next)
	    if (hnet -> name == name) break;
	if (!hnet) {
	    if ((fp = fopen ("seadif/trout.error", "w")))
	    { /* we might overwrite the trout.error of verify! */
		fprintf (fp, "This is the file 'seadif/trout.error' which was created by\n");
		fprintf (fp, "the program 'trout'\n");
		fprintf (fp, "------------------------- list of errors -------------------------\n");
		fprintf (fp, "While trying to highlight net '%s' in circuit '%s':\n", name, father->circuit->name);
		fprintf (fp, "Net '%s' could not be found in the netlist of '%s'\n", name, father->circuit->name);
		fprintf (fp, "Please try another net name\n");
		fclose (fp);
	    }
	    printf ("ERROR: cannot find net '%s' for highlighting\n", name);
	    return;
	}
    }
    else { /* find net based on position */
	fprintf (fp, "Sorry, no terminal name specified!\n\n");
	return;
    }

    printf ("------ Busy highlighting net ------\n");
    fflush (stdout);

    Bbx = father_bbx = &((R_CELLPTR) father->flag.p)->cell_bbx;
    exp_bbx.crd[L] = father_bbx->crd[L];
    exp_bbx.crd[R] = father_bbx->crd[R];
    exp_bbx.crd[B] = father_bbx->crd[B];
    exp_bbx.crd[T] = father_bbx->crd[T];
    exp_bbx.crd[D] = -1;
    exp_bbx.crd[U] = father_bbx->crd[U];
    routing_bbx = father_bbx;

    /* have a look at the terminals */
    netpattern = NULL;
    for (hportref = hnet -> terminals; hportref; hportref = hportref -> next) {
	if (!(rport = (R_PORTPTR) hportref->flag.p)) continue;

	point.x = rport->crd[X];
	point.y = rport->crd[Y];
	point.z = rport->crd[Z];
	if (is_state (point.x, point.y, point.z, FRONT1)) continue; /* already marked due to internal connection! */

	if (point.x < father_bbx->crd[L] || point.x > father_bbx->crd[R] ||
	    point.y < father_bbx->crd[B] || point.y > father_bbx->crd[T] ||
	    point.z < father_bbx->crd[D] || point.z > father_bbx->crd[U]) continue;

	if (!(terminalpattern = save_source (&point, NULL, &exp_bbx))) continue;
	if (!mark_as_front (terminalpattern, FRONT1, -1)) continue;

	/* link to list of net */
	hpoint = terminalpattern;
	wind_wire_to_end (hpoint);
	hpoint->next = netpattern;
	netpattern = terminalpattern;
    }

    /* at this moment the entire net is highlighted in the grid, and netpattern contains the net */
    if (HaveMarkerLayer == TRUE) copy_to_marker_layer (netpattern);

    /* restore and remove terminalpattern */
    free_and_restore_wire_pattern (netpattern);
}

/*
 * This routing creates a wire from point1 to the nearest border of the routing_bbx.
 */
GRIDPOINTPTR lee_to_border (GRIDPOINTPTR point1, BOXPTR routing_bbx, int only_horizontal)
/* point1          - address of terminal 1 */
/* routing_bbx     - bounding box within which the routing should take place */
/* only_horizontal - TRUE, if only the horizontal targets */
{
    register GRIDADRESSUNIT x, y, z;
    GRIDPOINTPTR terminalpattern1; /* expanded pattern of terminal 1 */
    GRIDPOINTPTR terminalpattern2; /* expanded pattern of terminal 2 = border */
    GRIDPOINTPTR wirepattern;      /* pattern of the generated wire */
    BOX exp_bbx;

    /* check terminal point1 */
    if (outside_bbx (point1->x, point1->y, point1->z, routing_bbx)) {
	fprintf (stderr, "WARNING (internal): terminal outside routing area:");
	fprintf (stderr, " (%ld, %ld, %ld)\n", point1->x, point1->y, point1->z);
	return (NULL);
    }

    /* Set the limits of the expansion bounding box.
     * This is the box withing which all routing will have to take place.
     * This in order to reduce the maximum size of the in the 'wrong' direction.
     * Currently the max exp bbx is the size of the core.
     */
    exp_bbx.crd[L] = routing_bbx->crd[L];
    exp_bbx.crd[R] = routing_bbx->crd[R];
    exp_bbx.crd[B] = routing_bbx->crd[B];
    exp_bbx.crd[T] = routing_bbx->crd[T];
    exp_bbx.crd[D] = routing_bbx->crd[D];
    exp_bbx.crd[U] = routing_bbx->crd[U];

    bbx1.crd[L] = bbx1.crd[R] = point1->x;
    bbx1.crd[B] = bbx1.crd[T] = point1->y;
    bbx1.crd[D] = bbx1.crd[U] = point1->z;

    /* make sure that point1 does not address an empty grid */
    if (is_free (point1)) set_grid_occupied (point1);

    /* Expand the pattern around terminal 1: find all points connected to it.
     * These points are 'saved' in this pattern.
     * The routine will return NULL if the point adresses an empty grid
     */
    if (!(terminalpattern1 = save_source (point1, &bbx1, &exp_bbx))) return (NULL);

    /* no penalty for wrong direction */
    Penalty_factor = 1;

    /* Start constructing terminalpattern2, which is a ring around the entire design.
    */
    terminalpattern2 = NULL;

    /* horizontal top and bottom
    */
    for (y = routing_bbx->crd[B];;) { /* for bottom and top row */
	for (z = 1; /* note it was 0, vertical only in metal2 */
		z <= routing_bbx->crd[U]; z++) { /* for all layers */
	    for (x = routing_bbx->crd[L]; x <= routing_bbx->crd[R]; x++) {
		if (!is_Free (x, y, z)) continue;

		if (Grid[z][y][x] != 0) { /* it was marked -- already_connected */
		    free_and_restore_wire_pattern (terminalpattern1);
		    free_gridpoint_list (terminalpattern2);
		    /* make dummy struct */
		    wirepattern = new_gridpoint (x, y, z);
		    wirepattern->cost = -1; /* disable printing */
		    return (wirepattern);
		}
		wirepattern = new_gridpoint (x, y, z);
		wirepattern->next = terminalpattern2;
		terminalpattern2 = wirepattern;
	    }
	}
	if (y == routing_bbx->crd[T]) break;
	y = routing_bbx->crd[T];
    }

    /* vertical on left and right
    */
    if (only_horizontal == FALSE)
    for (x = routing_bbx->crd[L];;) { /* for left and right */
	for (z = 0; z <= routing_bbx->crd[U]; z++) { /* for all layers */
	    for (y = routing_bbx->crd[B] + 1; y < routing_bbx->crd[T]; y++) {
		if (!is_Free (x, y, z)) continue;
		if (Grid[z][y][x] != 0) { /* it was marked -- already_connected */
		    free_and_restore_wire_pattern (terminalpattern1);
		    free_gridpoint_list (terminalpattern2);
		    /* make dummy struct */
		    wirepattern = new_gridpoint (x, y, z);
		    wirepattern->cost = -1; /* disable printing */
		    return (wirepattern);
		}
		wirepattern = new_gridpoint (x, y, z);
		wirepattern->next = terminalpattern2;
		terminalpattern2 = wirepattern;
	    }
	}
	if (x == routing_bbx->crd[R]) break;
	x = routing_bbx->crd[R]; /* right row */
    }

    if (!terminalpattern2) { /* OOOPS, no free path to escape exists... */
	free_and_restore_wire_pattern (terminalpattern1);
	return (NULL);
    }

    front_begin = front_end = NULL;

    /* Mark the source and destination patterns as such.
     * The routine will copy the frontpatterns and return the initialized front point list.
     * Null will be returned if the fronts are already connected.
     */
    front1_size = 0; front2_size = -1;
    if (!mark_as_front (terminalpattern1, FRONT1, front1_size)) { /* clean up */
	free_and_restore_wire_pattern (terminalpattern1);
	free_gridpoint_list (terminalpattern2);
	/* make dummy struct */
	wirepattern = new_gridpoint (point1->x, point1->y, point1->z);
	wirepattern->cost = -1; /* disable printing */
	return (wirepattern);
    }

    if (!mark_as_front (terminalpattern2, FRONT2, front2_size)) { /* clean up */
	free_and_restore_wire_pattern (terminalpattern1);
	free_gridpoint_list (terminalpattern2);
	/* make dummy struct */
	wirepattern = new_gridpoint (point1->x, point1->y, point1->z);
	wirepattern->cost = -1; /* disable printing */
	return (wirepattern);
    }

    /* perform the actual expansion... */
    wirepattern = lee_expansion (&exp_bbx);

    /* restore the terminal patterns */
    free_and_restore_wire_pattern (terminalpattern1);
    free_gridpoint_list (terminalpattern2);
    return (wirepattern);
}

/*
 * This magic routine will modify the wire pattern stored in 'pattern'.
 * to melt adjacent pieces of wire. The 'melting' means to add
 * the pointers such that the wired becomes wider.
 * The routine assumes that the pattern is tagged in the grid as FRONT1.
 */
static void melt_pattern (GRIDPOINTPTR pattern, BOXPTR exp_bbx)
/* pattern - linked list of points in the front */
/* exp_bbx - woring bounding box */
{
    register GRIDADRESSUNIT x, y, z, xl, xr, yb, yt, zu;
    register GRIDPOINTPTR p;

    xl = exp_bbx->crd[L]; xr = exp_bbx->crd[R];
    yb = exp_bbx->crd[B]; yt = exp_bbx->crd[T];
    zu = exp_bbx->crd[U];

    for (p = pattern; p; p = p -> next) { /* step along wire pattern */
	x = p->x;
	y = p->y;
	z = p->z;
	if (x < xl || x > xr || y < yb || y > yt || z < 0 || z > zu) continue; /* outside bbx */

	/* check on left */
	if (x > xl && is_state (x-1, y, z, FRONT1) && !wire_has_neighbour (p, L))
	    add_wire_neighbour (p, L); /* it belongs to the same wire, but L not found */

	/* check on right */
	if (x < xr && is_state (x+1, y, z, FRONT1) && !wire_has_neighbour (p, R))
	    add_wire_neighbour (p, R); /* it belongs to the same wire, but R not found */

	/* check on bottom */
	if (y > yb && is_state (x, y-1, z, FRONT1) && !wire_has_neighbour (p, B))
	    add_wire_neighbour (p, B); /* it belongs to the same wire, but B not found */

	/* check on top */
	if (y < yt && is_state (x, y+1, z, FRONT1) && !wire_has_neighbour (p, T))
	    add_wire_neighbour (p, T); /* it belongs to the same wire, but T not found */
    }
}
