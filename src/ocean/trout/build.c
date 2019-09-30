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
 * general datastructure building routines
 */

#include "src/ocean/trout/typedef.h"
#include <signal.h>

#ifndef SIG_ERR
#define SIG_ERR ((int (*) ()) -1)
#endif

/*
 * import
 */
extern long
   Alarm_flag,             /* TRUE if alarm was set to stop routing */
   *LayerOrient,           /* array with the oriantation of each layer */
   Chip_num_layer,         /* number of metal layers to be used */
   **ViaIndex,             /* Index of via to core image in array ViaCellName */
   NumTearLine[2],         /* number of tearlines in orientation of index (HORIZONTAL/VERTICAL) */
   NumImageTearLine[2],    /* number of tearlines in orientation of index (HORIZONTAL/VERTICAL) */
   *ImageTearLine[2],      /* tearlines of image */
   GridRepitition[2];      /* repitionvector (dx, dy) of grid image (in grid points) */

extern COREUNIT
   Pat_mask[HERE+1];       /* look-up table for bit-patterns */

extern GRIDADRESSUNIT
   Xoff[HERE+1],           /* look-up tables for offset values */
   Yoff[HERE+1],
   Zoff[HERE+1];

extern GRIDPOINTPTR
   ***OffsetTable,         /* matrix of accessable neighbour grid points */
   **CoreFeed,             /* matrix of universal feedthroughs in basic image */
   **RestrictedCoreFeed,   /* matrix of restricted feedthroughs in basic image */
   **SaveRestrictedCoreFeed;   /* saved previous one */

extern POWERLINEPTR
   PowerLineList;          /* list of template power lines */

extern int ydlineno;

void init_gridpoint (void);
int  ydparse (void);
static void fill_tables (void);
static void initsignal (void);

/*
 * This routine initializes the variables.
 */
void init_variables ()
{
    fill_tables (); /* fill mask pattern tables */

    init_gridpoint (); /* init gridpoint memory manager */

    /* default: no tearlines */
    NumTearLine[X] = NumTearLine[Y] = 0;
    NumImageTearLine[X] = NumImageTearLine[Y] = 0;
    ImageTearLine[X] = ImageTearLine[Y] = NULL;

    PowerLineList = NULL; /* empty list of template power lines */

    /* set signalling */
    initsignal ();
    Alarm_flag = FALSE; /* no alarm pressed */
}

/*
 * This routing calls the parser to read the image description file.
 * It will return the success of the read.
 */
int read_image_file (char *name)
/* name - path to file */
{
    if (!(freopen (name, "r", stdin))) { /* FAILED */
	fprintf (stderr, "ERROR: Cannot open seadif image file '%s'.\n", name);
	return (FALSE);
    }

    ydlineno = 1;
    ydparse ();

    return (TRUE);
}

/*
 * This routine is called by ydparse to process the feed information.
 * It will return FALSE if no feed was added.
 */
int process_feeds (GRIDPOINTPTR feedlist, GRIDPOINTPTR **mtx)
/* mtx - the array to be processed: CoreFeed or RestrictedCoreFeed */
{
    GRIDPOINTPTR wpoint, hpoint, new_point;

    /*
     * input checking and set the values of ViaIndex
     */
    for (wpoint = feedlist; wpoint; wpoint = wpoint -> next) {
	if (wpoint->x < 0 || wpoint->x >= GridRepitition[X] ||
	    wpoint->y < 0 || wpoint->y >= GridRepitition[Y]) continue; /* ignore feeds outside image */

	/* set Viaindex, which was stored in point->cost */
	if (ViaIndex[wpoint->x][wpoint->y] != -1) {
	    fprintf (stderr, "WARNING (image file): Feed multiply declared: %ld %ld (feeds ignored)\n", wpoint->x, wpoint->y);
	    free_gridpoint_list (feedlist);
	    return (FALSE);
	}
	ViaIndex[wpoint->x][wpoint->y] = wpoint->cost;
    }

    for (wpoint = feedlist; wpoint; wpoint = wpoint -> next) {
	if (wpoint->x < 0 || wpoint->x >= GridRepitition[X] ||
	    wpoint->y < 0 || wpoint->y >= GridRepitition[Y]) continue; /* ignore feeds outside image */

	for (hpoint = feedlist; hpoint; hpoint = hpoint -> next) {
	    if (hpoint == wpoint) continue;
	    /* we store offsets, not absolute positions */
	    new_point = new_gridpoint (hpoint->x - wpoint->x, hpoint->y - wpoint->y, 0);
	    new_point->cost = 1;       /* default cost is 1 */
	    new_point->direction = -1; /* neg direction marks tunnel */
	    new_point->next = mtx[wpoint->x][wpoint->y];
	    mtx[wpoint->x][wpoint->y] = new_point;
	}
    }

    free_gridpoint_list (feedlist);

    return (TRUE);
}

/*
 * This routine links the end of the lists connected to all OffsetTable elements
 * to the beginning of the (non-restricted) feed list of layer 0.
 * It is called by ydparse() after the gridconnectlist has been processed.
 */
void link_feeds_to_offsets ()
{
    long x, y;
    register GRIDPOINTPTR point;

    for (x = 0; x != GridRepitition[X]; ++x)
    for (y = 0; y != GridRepitition[Y]; ++y) {
	for (point = OffsetTable[x][y][0]; point && point->next; point = point->next);

	if (point)
	    point->next = CoreFeed[x][y];
	else
	    OffsetTable[x][y][0] = CoreFeed[x][y];
    }
}

/*
 * This routine removes a connection between two adjacent points.
 */
int add_grid_block (long ax, long ay, long az, long bx, long by, long bz)
/* ax, ay, az - point 1 */
/* bx, by, bz - point 2 */
{
    GRIDPOINTPTR point, previous;
    long help, offset;

    /* try to sane points */
    if (bx < ax) { help = ax; ax = bx; bx = help; } /* swap */
    if (by < ay) { help = ay; ay = by; by = help; } /* swap */
    if (bz < az) { help = az; az = bz; bz = help; } /* swap */

    for_all_offsets (offset) {
	if ((bx - ax) == Xoff[offset] &&
	    (by - ay) == Yoff[offset] &&
	    (bz - az) == Zoff[offset]) break;
    }
    if (offset == HERE) return (FALSE);

    /* offset is seen from point a */

    if (ax < 0) {
	if (bx != 0) return (FALSE);
	ax = GridRepitition[X] - 1;
    }
    if (ay < 0) {
	if (by != 0) return (FALSE);
	ay = GridRepitition[Y] - 1;
    }
    if (az < 0) {
	if (bz != 0) return (FALSE);
	az = Chip_num_layer - 1;
    }

    if (bx >= GridRepitition[X]) {
	if (ax != GridRepitition[X] - 1) return (FALSE);
	bx = 0;
    }
    if (by >= GridRepitition[Y]) {
	if (ay != GridRepitition[Y] - 1) return (FALSE);
	by = 0;
    }
    if (bz >= Chip_num_layer) {
	if (az != Chip_num_layer - 1) return (FALSE);
	bz = 0;
    }

    /*
     * look for first in list
     */
    previous = NULL;
    for (point = OffsetTable[ax][ay][az]; point; point = point->next) {
	if (point->direction == offset) break;
	previous = point;
    }

    if (point) { /* something found */
	if (previous == NULL)
	    OffsetTable[ax][ay][az] = point->next;
	else
	    previous->next = point->next;
	free_gridpoint (point);
    }

    /*
     * look for second point in list
     */
    offset = opposite (offset);
    previous = NULL;
    for (point = OffsetTable[bx][by][bz]; point; point = point->next) {
	if (point->direction == offset) break;
	previous = point;
    }

    if (point) { /* something found */
	if (previous == NULL)
	    OffsetTable[bx][by][bz] = point->next;
	else
	    previous->next = point->next;
	free_gridpoint (point);
    }
    return (TRUE);
}

/*
 * This routine is called by ydparse to set the cost of a grid to a certain value.
 * The routine operates on the array OffsetTable, of which the cost values
 * contain the cost of an expansion step.
 */
int set_grid_cost (long cost, long off_x, long off_y, long off_z, long ax, long ay, long az, long bx, long by, long bz)
/* cost       - cost value */
/* off_x, off_y, off_z - offset value for which this cost applies */
/* ax, ay, az - startpoint of range */
/* bx, by, bz - endpoint of range */
{
    long help, found;
    register long x, y, z;
    register GRIDPOINTPTR point;

    if (bx < ax) { help = ax; ax = bx; bx = help; } /* swap */
    if (by < ay) { help = ay; ay = by; by = help; } /* swap */
    if (bz < az) { help = az; az = bz; bz = help; } /* swap */

    MAX_UPDATE (ax, 0); MIN_UPDATE (ax, GridRepitition[X] - 1);
    MAX_UPDATE (bx, 0); MIN_UPDATE (bx, GridRepitition[X] - 1);
    MAX_UPDATE (ay, 0); MIN_UPDATE (ay, GridRepitition[Y] - 1);
    MAX_UPDATE (by, 0); MIN_UPDATE (by, GridRepitition[Y] - 1);
    MAX_UPDATE (az, 0); MIN_UPDATE (az, Chip_num_layer - 1);
    MAX_UPDATE (bz, 0); MIN_UPDATE (bz, Chip_num_layer - 1);

    /* for all points in range */
    found = FALSE;
    for (z = az; z <= bz; ++z)
    for (y = ay; y <= by; ++y)
    for (x = ax; x <= bx; ++x) {
	/* step along all offsets of this point */
	for (point = OffsetTable[x][y][z]; point; point = point -> next) {
	    if (point -> x == off_x && point -> y == off_y && point -> z == off_z) {
		point -> cost = cost;
		found = TRUE;
	    }
	}
    }
    if (found == FALSE) fprintf (stderr, "WARNING: no offset found on range\n");

    return (found);
}

/*
 * This routine is called by ydparse to set the cost of a grid to a certain value.
 * The routine operates on the arrays CoreFeed and RestrictedCoreFeed of which the cost values
 * contain the cost of an expansion step.
 */
int set_feed_cost (long cost, long off_x, long off_y, long off_z, long ax, long ay, long az, long bx, long by, long bz)
/* cost        - cost value */
/* off_x, off_y, off_z - offset value for which this cost applies */
/* ax, ay, az  - startpoint of range */
/* bx, by, bz  - endpoint of range */
{
    long help, found;
    register long x, y;
    register GRIDPOINTPTR point;

    if (bx < ax) { help = ax; ax = bx; bx = help; } /* swap */
    if (by < ay) { help = ay; ay = by; by = help; } /* swap */

    MAX_UPDATE (ax, 0); MIN_UPDATE (ax, GridRepitition[X] - 1);
    MAX_UPDATE (bx, 0); MIN_UPDATE (bx, GridRepitition[X] - 1);
    MAX_UPDATE (ay, 0); MIN_UPDATE (ay, GridRepitition[Y] - 1);
    MAX_UPDATE (by, 0); MIN_UPDATE (by, GridRepitition[Y] - 1);

    /* for all points in range */
    found = FALSE;
    for (y = ay; y <= by; ++y)
    for (x = ax; x <= bx; ++x) {
	/* step along all offsets of this point */
	for (point = CoreFeed[x][y]; point; point = point -> next) {
	    if (point -> x == off_x && point -> y == off_y) { /* HIT */
		point -> cost = cost;
		found = TRUE;
	    }
	}
	/* step along all offsets of this point */
	for (point = RestrictedCoreFeed[x][y]; point; point = point -> next) {
	    if (point -> x == off_x && point -> y == off_y) { /* HIT */
		point -> cost = cost;
		found = TRUE;
	    }
	}
    }
    return (found);
}

/*
 * This routine fills the basic tables.
 * which are fixed (that is, could be treated as global variables).
 */
static void fill_tables ()
{
    int i;

    /*
     * for L, B, R, T, U and D
     */
    for_all_offsets (i) {
	Xoff[i] = Yoff[i] = Zoff[i] = 0;
	Pat_mask[i] = (COREUNIT) (1 << i);
    }

    Xoff[HERE] = Yoff[HERE] = Zoff[HERE] = 0;
    Xoff[L] = -1; Xoff[R] = 1;
    Yoff[B] = -1; Yoff[T] = 1;
    Zoff[D] = -1; Zoff[U] = 1;
}

/*
 * This routine removes the resticted corefeed table.
 */
void remove_restricted_core_feeds (int doit)
{
    int x, y, xsize, ysize;

    SaveRestrictedCoreFeed = RestrictedCoreFeed; /* save old */

    if (doit == FALSE) return;

    /* overwrite 'old' with an empty one */

    xsize = GridRepitition[X];
    ysize = GridRepitition[Y];

    /* allocate first index = x */
    if (!(RestrictedCoreFeed = (GRIDPOINT ***) calloc (xsize, sizeof(GRIDPOINT **))))
	error (FATAL_ERROR, "calloc for RestricedCoreFeed in x failed");
    /* allocate y */
    for (x = 0; x < xsize; ++x) {
	if (!(RestrictedCoreFeed[x] = (GRIDPOINT **) calloc (ysize, sizeof(GRIDPOINT *))))
	    error (FATAL_ERROR, "calloc for RestricedCoreFeed in y failed");
	// for (y = 0; y < ysize; ++y) RestrictedCoreFeed[x][y] = NULL;
    }
}

/*
 * routine is called if signal alarm is caught
 */
void set_alarm_flag (int signumber)
{
    fprintf (stderr, "\n**** O.K.  caught signal ARLM (you want premature end) *****\n");
    fprintf (stderr, "\n**** I will after routing this the current segment/net *****\n");
    Alarm_flag = TRUE;
}

static void initsignal ()
{
    if (signal (SIGALRM, set_alarm_flag) == SIG_ERR) fprintf (stderr, "WARNING: problem with SIGALRM\n");
}
