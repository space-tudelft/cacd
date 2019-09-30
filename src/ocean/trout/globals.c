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

/*
 * general stuff
 */

int NoUniversalFeed,	/* Change UniversalFeed into RestrictedFeed */
   Verbose_parse,	/* True to print unknown keywords */
   verbose;

long
   Finish_up,              /* TRUE to connect all unused transistors to power */
   Overlap_cell,           /* TRUE to make an overlap cell only */
   Alarm_flag,             /* TRUE if alarm was set to stop routing */
   Use_borders,            /* TRUE to use feeds which extend over the routing area */
   New_scheduling,         /* TRUE to do segment oriented scheduling */
   No_power_route,         /* TRUE to skip power nets */
   Verify_only,            /* TRUE to perform only wire checking */
   RouteToBorder,          /* TRUE to connect parent terminals to the border */
   Resistor_hack,          /* TRUE to consider resistors which are made up
                              of poly gates as open ports. Normally they
                              are considered as a short */
   HaveMarkerLayer,        /* TRUE=1 if marker layer to indicate unconnect */
   clk_tck;		   /* system clock ticks per second (CLK_TCK) */

COREUNIT ***Grid;          /* the working grid */
BOXPTR Bbx;                /* bounding box of working grid */

COREUNIT Pat_mask[HERE+1]; /* look-up table for bit-patterns */

GRIDADRESSUNIT
   Xoff[HERE+1],           /* look-up tables for offset values */
   Yoff[HERE+1],
   Zoff[HERE+1];

GRIDPOINTPTR
   ***OffsetTable,         /* matrix of accessable neighbour grid points */
   **CoreFeed,             /* matrix of universal feedthroughs in basic image */
   **RestrictedCoreFeed,   /* matrix of restricted feedthroughs in basic image */
   **SaveRestrictedCoreFeed;   /* saved version */

NETPTR Vssnet, Vddnet;     /* pointers to the power and ground net */

POWERLINEPTR PowerLineList; /* list of template power lines */

long
   Chip_num_layer,         /* number of metal layers to be used */
   GridRepitition[2],      /* repitionvector (dx, dy) of grid core image (in grid points) */
   *LayerOrient,           /* array of orientations of each layer */
   **ViaIndex,             /* Index of via to core image in array ViaCellName. size: GridRepitition[X] * GridRepitition[Y] */
                           /* negative value indicates no via possible */
   NumTearLine[2],         /* number of tearlines in orientation of index (HORIZONTAL/VERTICAL) */
   *TearLine[2],           /* array containing the coordinates of the tearline of some grid */
   NumImageTearLine[2],    /* number of tearlines in orientation of index (HORIZONTAL/VERTICAL) */
   *ImageTearLine[2];      /* array containing the coordinates of the tearline of basic image */

char *ThisImage;           /* Seadif name of this image */

GRIDADRESSUNIT xl_chiparea, xr_chiparea, yb_chiparea, yt_chiparea;

