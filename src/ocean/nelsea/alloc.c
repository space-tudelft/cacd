/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
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
 * Memory manager, allocation.
 */

#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/nelsea/grid.h"
#include "src/ocean/nelsea/prototypes.h"

static void allocate_LayerOrient (long);
static void allocate_LayerWidth (long);
static void allocate_LayerMaskName (long);
static void allocate_ViaMaskName (long);
static void allocate_ViaCellName (long);
static void allocate_GridMapping (void);
static void allocate_ViaIndex (void);

/*
 * import
 */
extern long
   *LayerOrient,
   *LayerWidth,            /* array with wire width in each layer */
   NumViaName,             /* number of indices in the array ViaCellName */
   ***ViaIndex,            /* Viaindex[x][y][z]: */
                           /* if z == 0: Index of via to core image in array ViaCellName (-1 if nonex) */
                           /* if z >  0: -1 if via between z and z-1 is not allowed, 1 otherwise */
   *GridMapping[2],        /* mapping of gridpoints on layout coordinates */
                           /* this is an array of size GridRepitition */
   ChipSize[2],            /* number of elementary core cells in x and y */
   Chip_num_layer,         /* number of metal layers to be used */
   GridRepitition[2],      /* repitionvector (dx, dy) of grid image (in grid points) */
   LayoutRepitition[2];    /* repitionvector (dx, dy) of LAYOUT image (in lambda) */

extern char
   ImageName[],
   **LayerMaskName,        /* array with mask names of each layer */
   **ViaMaskName,          /* array with mask name of via to layer+1 */
   **ViaCellName;          /* array with cell names of these vias */

/* This routine creates a grid of size (xsize, ysize, zsize)
 * index: z, y, x, in that order;
 * It will return a pointer to an initialized array.
 */
COREUNIT *** new_grid (GRIDADRESSUNIT xsize,
                       GRIDADRESSUNIT ysize,
                       GRIDADRESSUNIT zsize) /* size of the grid to be allocated */
{
    GRIDADRESSUNIT x, y, z;
    COREUNIT ***grid;
    char *first_free_mem; /* points to first free memory on allocated block */
    long mblocksize;

    /*
     * allocate a big memory block
     */
    mblocksize = (zsize * ysize * xsize * sizeof(COREUNIT)) +
		 (zsize * ysize *         sizeof(COREUNIT*)) +
		 (zsize *                 sizeof(COREUNIT**)) + 2;

    if (!(first_free_mem = (char *)malloc ((unsigned) mblocksize)))
    {
	fprintf (stderr, "ERROR: Malloc for total grid failed: %ld bytes (%ld x %ld x %ld)\n",
		mblocksize, xsize, ysize, zsize);
	fprintf (stderr, "       size: COREUNIT = %ld, COREUNIT* = %ld, COREUNIT ** = %ld\n",
		(long) sizeof(COREUNIT), (long) sizeof(COREUNIT *), (long) sizeof(COREUNIT **));
	error (FATAL_ERROR, "Memory shortage");
    }

    /*
     * grid points to first element
     */
    grid = (COREUNIT ***) first_free_mem;

    /*
     * increment with size of z-array
     */
    first_free_mem += (zsize * sizeof(COREUNIT **));

    /*
     * set all y pointer arrays
     * 32-bit pointer arrays
     */
    for (z = 0; z != zsize; z++)
    {
	grid[z] = (COREUNIT **) first_free_mem;
	first_free_mem += (ysize * sizeof(COREUNIT *));
    }

    /*
     * set all x-char arrays
     * these are char-arrays
     */
    for (z = 0; z != zsize; z++)
    {
	for (y = 0; y != ysize; y++)
	{
	    grid[z][y] = (COREUNIT *) first_free_mem;
	    first_free_mem += (xsize * sizeof(COREUNIT));

	    /* set to zero, temp, as long as malloc is used */
	    for (x = 0; x != xsize; x++) grid[z][y][x] = 0;
	}
    }

    if (mblocksize < (long) first_free_mem - (long) grid)
    {
	fprintf (stderr, "WARNING: problem with memory manager\n");
	fprintf (stderr, "         mblocksize = %ld bytes (%ld x %ld x %ld)\n",
		mblocksize, xsize, ysize, zsize);
	fprintf (stderr, "         size: COREUNIT = %ld, COREUNIT* = %ld, COREUNIT ** = %ld\n",
		(long) sizeof(COREUNIT), (long) sizeof(COREUNIT *), (long) sizeof(COREUNIT **));
	fprintf (stderr, "         first_free_mem = %ld, grid = %ld\n", (long) first_free_mem, (long) grid);
    }

    return (grid);
}

/* This routine frees a 3-dim grid.
 */
void free_grid (COREUNIT ***grid)
{
    free ((void *)grid);
}

/* This routine is called by ydparse to allocate all arrays which
 * are related to the number of layers.
 */
void allocate_layer_arrays (long num_layer)
{
    /*
     * allocate the layer orientation array LayerOrient[z]
     */
    allocate_LayerOrient (num_layer);

    /*
     * allocate the wire width array which stores the metal wire width in each layer
     */
    allocate_LayerWidth (num_layer);

    /*
     * allocate the wire width array which stores the metal wire width in each layer
     */
    allocate_LayerMaskName (num_layer);

    /*
     * allocate the via mask name array
     */
    allocate_ViaMaskName (num_layer);

    /*
     * allocate the via cell name array
     */
    allocate_ViaCellName (num_layer);
}

/* This routine allocates and initalizes orientation array of the layers
 * default the the zero layer is horizontal-> HVHVHVHV.....
 */
static void allocate_LayerOrient (long num_layer)
{
    GRIDADRESSUNIT z, orient;

    /* allocate first index = z */
    if (!(LayerOrient = (long *) calloc ((unsigned) num_layer, (unsigned) sizeof(long))))
	error (FATAL_ERROR, "calloc for LayerOrient failed");

    orient = HORIZONTAL;
    for (z = 0; z != num_layer; z++)
    {
	LayerOrient[z] = orient;
	orient = opposite (orient);
    }
}

/* This routine allocates the wire width array which stores the metal
 * wire width in each layer. The array is initialized to -1 to indicate undefined.
 */
static void allocate_LayerWidth (long num_layer)
{
    GRIDADRESSUNIT z;

    if (!(LayerWidth = (long *) calloc ((unsigned) num_layer, (unsigned) sizeof(long))))
	error (FATAL_ERROR, "calloc for Layerwidth failed");

    /*
     * set to illegal value to indate undeclared
     */
    for (z = 0; z != num_layer; z++) LayerWidth[z] = -1;
}

/* This routine allocates the array which stores the mask names
 * of the layers. These names are required in the link with NELSIS.
 */
static void allocate_LayerMaskName (long num_layer)
{
    if (!(LayerMaskName = (char **) calloc ((unsigned) num_layer, (unsigned) sizeof(char *))))
	error (FATAL_ERROR, "calloc for LayerMaskName failed");
}

/* This routine allocates the array which stores the mask names
 * of the vias. ViaMaskName[i] stores the mask name of the via
 * between layer i and i+1.
 * NumViaName contains the number of indices in the array.
 */
static void allocate_ViaMaskName (long num_layer)
{
    if (!(ViaMaskName = (char **) calloc ((unsigned) num_layer, (unsigned) sizeof(char *))))
	error (FATAL_ERROR, "calloc for ViaMaskName failed");
}

/* This routine allocates the array which stores the cell names
 * of the vias. ViaMaskName[i] stores the cell name of the via
 * between layer i and i+1.
 * NumViaName contains the number of indices in the array.
 * this is necessary because the array may be re-allocated in ydparse
 * to store additional vias to the core.
 * Vias to the core are stored in the indices NumViaName and higher.
 */
static void allocate_ViaCellName (long num_layer)
{
    NumViaName = num_layer;
    if (!(ViaCellName = (char **) calloc ((unsigned) num_layer, (unsigned) sizeof(char *))))
	error (FATAL_ERROR, "calloc for ViaCellName failed");
}

/* This routine is called by ydparse to allocate all arrays and
 * mulitdimensional arrays which are related or dependent on
 * the size of the core image.
 * The dimensions of the core image are store in the globals
 * GridRepitition[X] and GridRepitition[Y]
 */
void allocate_core_image ()
{
    /* 1: allocate gridmapping array */
    allocate_GridMapping ();

    /* 2: allocate index array for via positions to core */
    allocate_ViaIndex ();
}

/* This routine allocates the gridmapping array.
 * e.g. GridMapping[X][2] will contain the real-life coordinate
 * of all gridpoints with x-coordinate 2.
 */
static void allocate_GridMapping ()
{
    CALLOC (GridMapping[X], long, GridRepitition[X]);
    CALLOC (GridMapping[Y], long, GridRepitition[Y]);
}

/* This routine allocates the three-dimensional array of integers
 * ViaIndex[GridRepitition[X]][GridRepitition[Y]][Chip_num_layer].
 * The values are initialized to -1, which indicates that no
 * via to the core image is allowed on that position (if z = 0).
 * ydparse() may set the value of certain indices to a positive value
 * which is an index in the array ViaCellName[z].
 */
static void allocate_ViaIndex ()
{
    long x, y, z;

    /* allocate first index = x */
    if (!(ViaIndex = (long ***) calloc ((size_t) GridRepitition[X], sizeof(long **))))
	error (FATAL_ERROR, "calloc for ViaIndex in x failed");

    for (x = 0; x != GridRepitition[X]; x++)
    {
	/* allocate y */
	if (!(ViaIndex[x] = (long **) calloc ((size_t) GridRepitition[Y], sizeof(long *))))
	    error (FATAL_ERROR, "calloc for ViaIndex in y failed");

	for (y = 0; y != GridRepitition[Y]; y++)
	{
	    /* allocate z */
	    CALLOC (ViaIndex[x][y], long, Chip_num_layer);

	    /* set via to core default disallowed */
	    ViaIndex[x][y][0] = -1;

	    /* set vias bewteen metal layers default on */
	    for (z = 1; z < Chip_num_layer; z++) ViaIndex[x][y][z] = 1;
	}
    }
}

/* This routine (called from parse) removes a via location.
 */
int add_grid_block (long ax, long ay, long az, /* point 1 */
                    long bx, long by, long bz) /* point 2 */
{
    long help;

    /* try to sane points */
    if (bx < ax) { help = ax; ax = bx; bx = help; } /* swap */
    if (by < ay) { help = ay; ay = by; by = help; } /* swap */
    if (bz < az) { help = az; az = bz; bz = help; } /* swap */

    /* offset is seen from point a */
    if (ax < 0 || ay < 0 || az < 0 ||
	bx >= GridRepitition[X] ||
	by >= GridRepitition[Y] ||
	bz >= Chip_num_layer) return (FALSE);

    /* return if not a via offset */
    if (ax - bx != 0 || ay - by != 0 || bz - az != 1) return (TRUE);

    /* disable index */
    if (ViaIndex[bx][by][bz] == -1)
	fprintf (stderr, "WARNING: via block at (%ld, %ld, %ld) multiply declared\n", bx, by, bz);

    ViaIndex[bx][by][bz] = -1;

    return (TRUE);
}
