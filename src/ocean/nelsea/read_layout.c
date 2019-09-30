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
 * Read nelsis layout description into seadif database.
 */

#include <time.h>
#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/nelsea/grid.h"
#include "src/ocean/nelsea/prototypes.h"
#include <sys/types.h>
#include <sys/stat.h>

static COREUNIT ***makegrid (DM_CELL *);
static void find_box_bbx (DM_CELL *, long *, long *, long *, long *);
static void read_boxes (MAPTABLEPTR, DM_CELL *);
static void read_vias (MAPTABLEPTR, DM_CELL *);
static void create_wires (MAPTABLEPTR);
static void mk_wire (MAPTABLEPTR, GRIDWIREPTR, int);
static void read_terminals (MAPTABLEPTR, DM_CELL *);
static void read_labels (MAPTABLEPTR, DM_CELL *);
static void read_mc (MAPTABLEPTR, DM_CELL *);
static int guess_alignment (MAPTABLEPTR, struct geo_mc *);
static long map_lambda_to_overlay_grid_coord (long, int);
static void add_error_marker (MAPTABLEPTR map, char *name, long x, long y);

/*
 * imported vars
 */
extern DM_PROJECT *projectkey;
extern char
   *primitive_str,
   *in_core_str,
   *circuit_str,
   *layout_str,
   *written_str;
extern int
   Seadif_open;
extern long
   ChipSize[2],            /* number of elementary core cells in x and y */
   Chip_num_layer,         /* number of metal layers to be used */
   *GridMapping[2],        /* mapping of gridpoints on layout coordinates: size: GridRepitition[X] * GridRepitition[Y] */
   *OverlayGridMapping[2], /* overlaymapping of gridpoints to layout coordinates */
   OverlayBounds[2][2],    /* boundaries of overlaymapping, index 1 = orient, index2 = L/R */
   GridRepitition[2],      /* repitionvector (dx, dy) of grid core image (in grid points) */
   LayoutRepitition[2],    /* repitionvector (dx, dy) of LAYOUT image (in lambda) */
   ***ViaIndex,            /* Viaindex[x][y][z]: */
                           /* if z == 0: Index of via to core image in array ViaCellName (-1 if nonex) */
                           /* if z >  0: -1 if via between z and z-1 is not allowed, 1 otherwise */
   *ViaMaskNo,             /* contains NELSIS mask numbers of via mask string in array ViaMaskName. size: NumViaName */
   *LayerMaskNo,           /* contains NELSIS mask numbers of layer mask string in array LayerMaskName. size: ChipNumLayer */
   DummyMaskNo,
   NumViaName;             /* number of indices in the arrays ViaCellName and ViaMaskName */
                           /* The value of this index is >= Chip_num_layer */
extern char
   *ThisImage,              /* name identifier of image */
   ImageName[DM_MAXNAME + 1],  /* NELSIS name of image to be repeated */
   *ChipName,              /* output name of entire circuit */
   **LayerMaskName,        /* array with mask names of each layer. size: Chip_num_layer */
   **ViaMaskName,          /* array with mask names of via to layer. size: NumViaName */
   **ViaCellName;          /* array with cell names of these vias. size: NumViaName */

/* grid-related */
extern COREUNIT
   ***Grid;
extern GRIDADRESSUNIT
   Xoff[HERE+1],
   Yoff[HERE+1],
   Zoff[HERE+1];
extern COREUNIT
   Pat_mask[HERE+1];
extern BOX
   Image_bbx;
extern long
   LayerRead[MAXLAYERS], /* array of booleans: TRUE to read the indicated layer */
   ViaRead[MAXLAYERS],   /* array of booleans: TRUE to read the indicated via */
   TermRead,             /* boolean: true to read terminals in seadif */
   MCRead;               /* boolean: true to read mc's into seadif */
extern int
   Auto_move,            /* TRUE to move the cell to the leftbottom corner */
   extra_verbose,
   Hierarchical_fish;    /* TRUE to perform hierarchical fishing */

/*
 * static
 */
static long
   autoshift[2],	/* the default offset vector for everything which is read in */
   nelsisshift[2];	/* the same offset, in nelsis lambda coordinates */

/* This routine reads a nelsis circuit description into
 * a seadif datastructure.
 */
void read_layout_cell (char *cell_name, DM_CELL *cell_key, MAPTABLEPTR map)
{
    /* get basic layout cell, attach it into the library */
    attach_map_to_lib (map);

    if (map->internalstatus == in_core_str)
	fprintf (stderr, "WARNING (read_layout_cell): layout cell '%s' is already in core.\n", cell_name);

    /* allocate a grid as large as necessary */
    Grid = makegrid (cell_key);

    /* read the boxes into the grid */
    read_boxes (map, cell_key);

    /* read the vias into the grid */
    read_vias (map, cell_key);

    /* convert the contents of the grid into wire statements */
    create_wires (map);

    /* the grid can be freed now */
    free_grid (Grid);

    /* read the terminals directly in the datastructure */
    read_terminals (map, cell_key);

    /* read the labels directly in the datastructure */
    read_labels (map, cell_key);

    /* read the model-calls directly into the datastructure */
    read_mc (map, cell_key);

    /* set status */
    map->layoutstruct->status->timestamp = time (0);

    if (Hierarchical_fish) map->seanelstatus = cs ("not_written"); /* this is implied! */

    map->internalstatus = in_core_str;
}

/* This routine allocates a grid, just large enough for all
 * rectangles in the layout.
 */
static COREUNIT ***makegrid (DM_CELL *cell_key)
{
    long xl, xr, yb, yt;
    COREUNIT ***grid;

    /* find the bounding box of boxes */
    find_box_bbx (cell_key, &xl, &xr, &yb, &yt);

    /* check */
    if (xr < xl || yt < yb) xl = xr = yb = yt = 0; /* nothing found */

    if (xl < 0) {
	fprintf (stderr, "WARNING: objects left of origin (x < 0) found.\n");
	fprintf (stderr, "         They will be truncated.\n");
	/* xl = 0; */
    }

    /* hack: message is irritant in case sons still have image */
    if (yb < -12) {
	fprintf (stderr, "WARNING: objects below origin (y < 0) found.\n");
	fprintf (stderr, "         They will be truncated.\n");
	/* yb = 0; */
    }

    /* auto_move? */
    if (Auto_move == TRUE) {
	autoshift[X]   = -GridRepitition[X]   * (map_lambda_to_grid_coord (xl, X) / GridRepitition[X]);
	autoshift[Y]   = -GridRepitition[Y]   * (map_lambda_to_grid_coord (yb, Y) / GridRepitition[Y]);
	nelsisshift[X] = -LayoutRepitition[X] * (map_lambda_to_grid_coord (xl, X) / GridRepitition[X]);
	nelsisshift[Y] = -LayoutRepitition[Y] * (map_lambda_to_grid_coord (yb, Y) / GridRepitition[Y]);
    }
    else { /* no automove: keep original position */
	autoshift[X] = autoshift[Y] = nelsisshift[X] = nelsisshift[Y] = 0;
    }

    /*
     * set array size
     */
    xl = yb = 0;
    Image_bbx.xl = xl;
    Image_bbx.xr = map_lambda_to_grid_coord (xr, X) + autoshift[X];
    Image_bbx.yb = yb;
    Image_bbx.yt = map_lambda_to_grid_coord (yt, Y) + autoshift[Y];
    Image_bbx.zd = 0;
    Image_bbx.zu = Chip_num_layer - 1;

    /*
     * allocate the array
     */
    grid = new_grid (Image_bbx.xr + 1, Image_bbx.yt + 1, Image_bbx.zu + 1);

    return (grid);
}

/* This routine returns the bounding-box of the vias and the rectangles
 * and the terminals (returned bounding box in lambda's).
 */
static void find_box_bbx (DM_CELL *cell_key, long *xl, long *xr, long *yb, long *yt)
{
    DM_STREAM *fp;
    long centerx, centery;
    int dmresult, i;
    int unknown_found = 0; /* flag for illegal layer */

    *xl = *yb =  BIGNUMBER;
    *xr = *yt = -BIGNUMBER;

    /* read vias as model calls
     */
    if (!(fp = dmOpenStream (cell_key, "mc", "r"))) {
	error (ERROR, "find_box_bbx()"); return;
    }
    while ((dmresult = dmGetDesignData (fp, GEO_MC)) > 0)
    {
	for (i = 0; i < NumViaName; ++i)
	    if (ViaCellName[i] && strcmp (gmc.cell_name, ViaCellName[i]) == 0) break;
	if (i >= NumViaName)
	{ /* model call is not a via */
	    if (Auto_move == FALSE) continue;

	    /* in case of auto move: take leftbottom of mc in account */
	    if (strcmp (gmc.cell_name, ImageName) == 0) continue; /* skip image */
	    if (gmc.bxl < *xl) *xl = gmc.bxl;
	    if (gmc.byb < *yb) *yb = gmc.byb;
	    continue;
	}
	/* a via */
	centerx = (gmc.bxl + gmc.bxr) / 2;
	centery = (gmc.byb + gmc.byt) / 2;
	MIN_UPDATE (*xl, centerx);
	MIN_UPDATE (*yb, centery);
	MAX_UPDATE (*xr, centerx);
	MAX_UPDATE (*yt, centery);
    }
    dmCloseStream (fp, COMPLETE);

    /* open box files for reading
     */
    if (!(fp = dmOpenStream (cell_key, "box", "r"))) {
	error (ERROR, "find_box_bbx()"); return;
    }
    while ((dmresult = dmGetDesignData (fp, GEO_BOX)) > 0)
    {
	/* find layer as declared mask */
	for (i = 0; i != NumViaName; i++)
	    if (ViaMaskNo[i] == gbox.layer_no) break;
	if (i == NumViaName)
	{
	    for (i = 0; i != Chip_num_layer; i++)
		if (LayerMaskNo[i] == gbox.layer_no) break;
	    if (i == Chip_num_layer) { /* not found */
		if (gbox.layer_no != DummyMaskNo) unknown_found = 1;
		continue; /* do not take into account for wires */
	    }
	}
	MIN_UPDATE (*xl, gbox.xl);
	MIN_UPDATE (*yb, gbox.yb);
	MAX_UPDATE (*xr, gbox.xr);
	MAX_UPDATE (*yt, gbox.yt);
    }
    if (dmresult) fprintf (stderr, "ERROR: Something strange going on with dmGetDesignData(GEO_BOX)\n");
    dmCloseStream (fp, COMPLETE);

    /* read terminals
    */
    if (!(fp = dmOpenStream (cell_key, "term", "r"))) {
	error (ERROR, "find_box_bbx()"); return;
    }
    while ((dmresult = dmGetDesignData (fp, GEO_TERM)) > 0)
    {
	MIN_UPDATE (*xl, gterm.xl);
	MIN_UPDATE (*yb, gterm.yb);
	MAX_UPDATE (*xr, gterm.xr);
	MAX_UPDATE (*yt, gterm.yt);
    }
    dmCloseStream (fp, COMPLETE);

    if (unknown_found) fprintf (stderr, "WARNING: Wire segments in unknown mask(s) found. They will be removed.\n");
}

#define map_to_layout_coord(crd, ori) \
(((crd) / GridRepitition[ori]) * LayoutRepitition[ori]) + GridMapping[ori][(crd) % GridRepitition[ori]]

#define IS_UNKNOWN       0
#define IS_METAL         1
#define IS_VIA           2
#define IS_DUMMY         3
#define IS_ILLEGAL_MASK  4
#define IS_DO_NOT_FISH   5

/* This routine stamps/writes the boxes into the grid.
 * Alternatively, (if specified in ReadLayer[x]) it stores in a NELSISOBJECT.
 */
static void read_boxes (MAPTABLEPTR map, DM_CELL *cell_key)
{
   DM_STREAM *fp;
   NELSISOBJECTPTR no;
   long z, gxl, gxr, gyb, gyt, x, y;
   int dmresult, type, reperror = 0;
   COREUNIT b;

   /* open box files for reading */
   if (!(fp = dmOpenStream (cell_key, "box", "r"))) {
      error (ERROR, "read_boxes()"); return;
   }
   while ((dmresult = dmGetDesignData (fp, GEO_BOX)) > 0)
   {
      /*
       * determine the kind of box: metal, via, dummymask, illegal
       */
      /* Is it a metal layer ? */
      for (z = 0; z < Chip_num_layer; ++z)
	 if (LayerMaskNo[z] == gbox.layer_no) break;
      if (z < Chip_num_layer) /* Yes, it is metal layer ``z'' */
      {
	 type = IS_METAL;
	 if (LayerRead[z] != TRUE) type = IS_DO_NOT_FISH;
      }
      else
      { /* a via?? */
	 for (z = 0; z < NumViaName; ++z)
	    if (ViaMaskNo[z] == gbox.layer_no) break;
	 if (z < NumViaName)
	 { /* a via */
	    type = IS_VIA;
	    if (ViaRead[z] != TRUE)
	       type = IS_DO_NOT_FISH;
	 }
	 else if (gbox.layer_no == DummyMaskNo)
	    type = IS_DUMMY;
	 else
	    type = IS_ILLEGAL_MASK;
      }

   /*
    * switch per type...
    */
      if (type == IS_ILLEGAL_MASK) continue; /* warning was given somewhere else */

      if (type == IS_DO_NOT_FISH || type == IS_DUMMY)
      { /* store in nelsis struct */
	 NewNelsisobject (no);
	 no->next = map->list_of_unfished_objects;
	 map->list_of_unfished_objects = no;
	 no->type = GBOX_FISH;

	 /* copy it */
	 no->layer_no = gbox.layer_no;
	 no->xl = gbox.xl + nelsisshift[X];
	 no->xr = gbox.xr + nelsisshift[X];
	 no->yb = gbox.yb + nelsisshift[Y];
	 no->yt = gbox.yt + nelsisshift[Y];
	 no->bxl = gbox.bxl + nelsisshift[X];
	 no->bxr = gbox.bxr + nelsisshift[X];
	 no->byb = gbox.byb + nelsisshift[Y];
	 no->byt = gbox.byt + nelsisshift[Y];
	 no->dx = gbox.dx;
	 no->nx = gbox.nx;
	 no->dy = gbox.dy;
	 no->ny = gbox.ny;

	 continue;
      }

      /* its a box of a via */

      if (gbox.nx > 0 || gbox.ny > 0) reperror = 1;

      if (gbox.xr < 0 || gbox.yt < 0) continue; /* warning was given somewhere else */

      MAX_UPDATE (gbox.xl, 0);
      MAX_UPDATE (gbox.yb, 0);

      /* determine grid equivalents of xl,xr,yb,yt */
      gxl = map_lambda_to_grid_coord (gbox.xl, X);
      gxr = map_lambda_to_grid_coord (gbox.xr, X);
      gyb = map_lambda_to_grid_coord (gbox.yb, Y);
      gyt = map_lambda_to_grid_coord (gbox.yt, Y);
      MAX_UPDATE (gxl, 0);
      MAX_UPDATE (gyb, 0);

      /*
       * look for overlapmapping
       */
      if (OverlayGridMapping[X] &&
	 gyb % GridRepitition[Y] >= OverlayBounds[Y][L] &&
	 gyt % GridRepitition[Y] <= OverlayBounds[Y][R])
      { /* in overlay area: change x */
	 gxl = map_lambda_to_overlay_grid_coord (gbox.xl, X);
	 gxr = map_lambda_to_overlay_grid_coord (gbox.xr, X);
	 MAX_UPDATE (gxl, 0);
      }
      if (OverlayGridMapping[Y] &&
	 gxl%GridRepitition[X] >= OverlayBounds[X][L] &&
	 gxr%GridRepitition[X] <= OverlayBounds[X][R])
      { /* in overlay area: change x */
	 gyb = map_lambda_to_overlay_grid_coord (gbox.yb, Y);
	 gyt = map_lambda_to_overlay_grid_coord (gbox.yt, Y);
	 MAX_UPDATE (gyb, 0);
      }

      gxl += autoshift[X]; gxr += autoshift[X];
      gyb += autoshift[Y]; gyt += autoshift[Y];

      if (gxl < Image_bbx.xl || gxr > Image_bbx.xr ||
	 gyb < Image_bbx.yb || gyt > Image_bbx.yt)
	 continue; /* illegal coordinate */

      /* stamp in grid */
      for (x = gxl; x <= gxr; ++x)
      {
	 for (y = gyb; y <= gyt; ++y)
	 {
	    if (type == IS_METAL)
	    {
	       b = STATEMASK;
	       if (x < gxr) b |= (1 << R);
	       if (x > gxl) b |= (1 << L);
	       if (y < gyt) b |= (1 << T);
	       if (y > gyb) b |= (1 << B);
	       Grid[z][y][x] |= b; /* Mark occupied */
	       continue;
	    }
	    if (type == IS_VIA)
	    {
	       if (z < Chip_num_layer)
	       { /* Found a via connecting layer z to layer z+1. */
		  Grid[z][y][x] |= STATEMASK; /* Mark occupied */
		  Grid[z][y][x] |= (1 << U);  /* Connects up */
		  Grid[z+1][y][x] |= STATEMASK; /* Mark occupied */
		  Grid[z+1][y][x] |= (1 << D); /* Connects down */
	       }
	       else
	       { /* substrate contact */
		  Grid[0][y][x] |= STATEMASK; /* Mark occupied */
		  Grid[0][y][x] |= (1 << D);  /* Connects down only */
	       }
	    }
	 }
      }
   }
   if (dmresult) fprintf (stderr, "\nSomething strange going on with dmGetDesignData(GEO_BOX)\n");

   if (reperror) fprintf (stderr, "WARNING: box array(s) found. Arrays were not expanded!\n");

   dmCloseStream (fp, COMPLETE);
}

/* This routine writes the vias (stored as model calls) into the grid.
 */
static void read_vias (MAPTABLEPTR map, DM_CELL *cell_key)
{
    DM_STREAM *fp;
    NELSISOBJECTPTR no;
    long z, gx, gy, i;
    int dmresult, reperror = 0;

    /* open mc files for reading */
    if (!(fp = dmOpenStream (cell_key, "mc", "r"))) {
	error (ERROR, "read_vias()"); return;
    }
    while ((dmresult = dmGetDesignData (fp, GEO_MC)) > 0)
    {
	for (z = 0; z < NumViaName; ++z)
	    if (ViaCellName[z] && strcmp (gmc.cell_name, ViaCellName[z]) == 0) break;
	if (z >= NumViaName) continue; /* this model call is not a via */

	/* no repitition? */
	if (gmc.nx > 0 || gmc.ny > 0) reperror = 1;

	/* find the grid point closest to the via's center of mass */
	gx = map_lambda_to_grid_coord ((gmc.bxl+gmc.bxr)>>1, X);
	gy = map_lambda_to_grid_coord ((gmc.byb+gmc.byt)>>1, Y);

	gx += autoshift[X];
	gy += autoshift[Y];

	if (gx < Image_bbx.xl || gx > Image_bbx.xr ||
	    gy < Image_bbx.yb || gy > Image_bbx.yt) continue; /* illegal coordinate */

	if (OverlayGridMapping[X] &&
	    gy%GridRepitition[Y] >= OverlayBounds[Y][L] &&
	    gy%GridRepitition[Y] <= OverlayBounds[Y][R])
	{ /* in overlay area */
	    gx = map_lambda_to_overlay_grid_coord ((gmc.bxl+gmc.bxr)>>1, X) + autoshift[X];
	}

	if (OverlayGridMapping[Y] &&
	    gx%GridRepitition[X] >= OverlayBounds[X][L] &&
	    gx%GridRepitition[X] <= OverlayBounds[X][R])
	{
	    gy = map_lambda_to_overlay_grid_coord ((gmc.byb+gmc.byt)>>1, Y) + autoshift[Y];
	}

	if (ViaRead[z] == TRUE)
	{ /* stamp into grid */
	    if (z >= Chip_num_layer) {
		/* substrate contact */
		Grid[0][gy][gx] |= STATEMASK; /* Mark occupied */
		Grid[0][gy][gx] |= (1 << D);  /* Connects down */
	    }
	    else {
		Grid[z][gy][gx] |= STATEMASK; /* Mark occupied */
		Grid[z][gy][gx] |= (1 << U);  /* Connects up */
		if (z < Chip_num_layer-1) {
		    Grid[z+1][gy][gx] |= STATEMASK; /* Mark occupied */
		    Grid[z+1][gy][gx] |= (1 << D); /* Connects down */
		}
		else
		    fprintf (stderr, "Cannot connect to the sky.\n");
	    }
	}
	else
	{ /* store in nelsisstruct */
	    NewNelsisobject (no);
	    no->next = map->list_of_unfished_objects;
	    map->list_of_unfished_objects = no;

	    no->type = GMC_FISH;

	    /* copy it */
	    STRINGSAVE(no->name, gmc.inst_name);
	    STRINGSAVE(no->cell_name, gmc.cell_name);
	    no->imported = gmc.imported;
	    for (i = 0; i != 6; i++) no->mtx[i] = gmc.mtx[i];
	    no->mtx[2] += nelsisshift[X];
	    no->mtx[5] += nelsisshift[Y];
	    no->bxl = gmc.bxl + nelsisshift[X];
	    no->bxr = gmc.bxr + nelsisshift[X];
	    no->byb = gmc.byb + nelsisshift[Y];
	    no->byt = gmc.byt + nelsisshift[Y];
	    no->dx = gmc.dx; no->nx = gmc.nx;
	    no->dy = gmc.dy; no->ny = gmc.ny;
	}
    }
    if (dmresult) fprintf (stderr, "\nSomething strange going on with dmGetDesignData(GEO_MC)\n");

    if (reperror) fprintf (stderr, "WARNING: via array(s) found. Arrays were not expanded!\n");

    dmCloseStream (fp, COMPLETE);
}

/* This routine converts the contents of the grid into wire statements.
 */
static void create_wires (MAPTABLEPTR map)
{
    GRIDWIRE rtstruct, pointstruct;
    GRIDWIREPTR rt, point;
    WIREPTR wire;
    int errcount;
    long viaindex;

    point = &pointstruct;
    rt = &rtstruct;

    /*
     * write maximal horizontal wires
     */
    for (point->z = Image_bbx.zu; point->z >= 0; point->z--) /* for all layers */
    for (point->y = Image_bbx.yb; point->y <= Image_bbx.yt; point->y++) /* for all rows */
    for (point->x = Image_bbx.xl; point->x <= Image_bbx.xr; point->x++) /* for all points */
	if (grid_is_occupied (point))
	{ /* part of path */
	    if ((point->x == Image_bbx.xl || !(has_neighbour (point, L))) && has_neighbour (point, R))
	    {
		mk_wire (map, point, R);
	    }
	}

    /*
     * write maximal vertical wires
     */
    for (point->z = Image_bbx.zu; point->z >= 0; point->z--) /* for all layers */
    for (point->x = Image_bbx.xl; point->x <= Image_bbx.xr; point->x++) /* for all rows */
    for (point->y = Image_bbx.yb; point->y <= Image_bbx.yt; point->y++) /* for all points */
	if (grid_is_occupied (point))
	{ /* part of path */
	    if ((point->y == Image_bbx.yb || !(has_neighbour (point, B))) && has_neighbour (point, T))
	    {
		mk_wire (map, point, T);
	    }
	}

    /*
     * Fill the holes between wires of same pattern
     */
    for (point->z = Image_bbx.zu; point->z >= 0; point->z--) /* for all layers */
    for (point->y = Image_bbx.yb; point->y < Image_bbx.yt; point->y++)
    for (point->x = Image_bbx.xl; point->x < Image_bbx.xr; point->x++)
    {
	if (!(grid_is_occupied (point))) continue;
	if (!(has_neighbour (point, T))) continue;
	if (!(has_neighbour (point, R))) continue;

	/* step the righttop of potential box */
	rt->y = point->y + 1;
	rt->z = point->z;
	for (rt->x = point->x + 1; rt->x <= Image_bbx.xr; rt->x++)
	{
	    rt->y = point->y + 1;
	    if (!(grid_is_occupied (rt))) break;
	    if (!(has_neighbour (rt, L))) break;
	    rt->y--;
	    if (!(has_neighbour (rt, L))) break;
	}
	rt->y = point->y + 1;
	rt->x--;
	if (rt->x <= point->x) continue; /* no square */

	/* add box */
	NewWire (wire);
	wire->layer = (short) point->z + 1;
	wire->crd[L] = (short) point->x;
	wire->crd[R] = (short) rt->x;
	wire->crd[B] = (short) point->y;
	wire->crd[T] = (short) rt->y;
	wire->next = map->layoutstruct->wire;
	map->layoutstruct->wire = wire;

	point->x = rt->x;
    }

    errcount = 0;
    /*
     * add vias between metal layers
     */
    for (point->z = 0; point->z <  Image_bbx.zu; point->z++) /* for all layers, except top */
    for (point->y = 0; point->y <= Image_bbx.yt; point->y++) /* for all rows */
    for (point->x = 0; point->x <= Image_bbx.xr; point->x++) /* for all points */
	if (has_neighbour (point, U))
	{
	    viaindex = ViaIndex[to_core (point->x, X)][to_core (point->y, Y)][point->z+1];
	    if (viaindex == -1) { /* not allowed there! */
		add_error_marker (map, "illegal_via", point->x, point->y);
		if (errcount == 0) {  /* welcome message */
		    fprintf (stderr, "ERROR: Via(s) at illegal position (not removed): ");
		    errcount = 4;
		}
		if (errcount%6 == 0) { /* newline */
		    fprintf (stderr, "\n       ");
		}

		fprintf (stderr, "(%ld,%ld), ", map_to_layout_coord (point->x, X), map_to_layout_coord (point->y, Y));
		errcount++;
	    }

	    NewWire (wire);
	    wire->crd[L] = wire->crd[R] = (short) point->x;
	    wire->crd[B] = wire->crd[T] = (short) point->y;
	    wire->layer =  (short) (101 + point->z);
	    wire->next = map->layoutstruct->wire;
	    map->layoutstruct->wire = wire;
	}

    if (errcount) fprintf (stderr, "\n");

    errcount = 0;
    /*
     * check for stacked vias
     */
    for (point->z = 0; point->z <  Image_bbx.zu; point->z++) /* for all layers, except top */
    for (point->y = 0; point->y <= Image_bbx.yt; point->y++) /* for all rows */
    for (point->x = 0; point->x <= Image_bbx.xr; point->x++) /* for all points */
	if (has_neighbour (point, U) && has_neighbour (point, D))
	{ /* stacked */
	    if (errcount == 0) {  /* welcome message */
		fprintf (stderr, "ERROR: Stacked via(s) (* = removed): ");
		errcount = 3;
	    }
	    if (errcount % 5 == 0) { /* newline */
		fprintf (stderr, "\n       ");
	    }

	    if (strncmp (ImageName, "fishbone", strlen ("fishbone") - 1) == 0 &&
		point->z == 0 && (point->y % (GridRepitition[Y] / 2)) == 0)
	    { /* substrate contact under power line: remove it */
		Grid[0][point->y][point->x] ^= Pat_mask[D];  /* exor = erase... */
		add_error_marker (map, "stacked_sub!", point->x, point->y);
		fprintf (stderr, "(*%ld,%ld*), ", map_to_layout_coord (point->x, X), map_to_layout_coord (point->y, Y));
	    }
	    else
	    { /* leave it: dunno what is better... */
		add_error_marker (map, "stacked!", point->x, point->y);
		fprintf (stderr, "(%ld,%ld), ", map_to_layout_coord (point->x, X), map_to_layout_coord (point->y, Y));
	    }
	    errcount++;
	}

    if (errcount) fprintf (stderr, "\n");

    errcount = 0;
    /*
     * vias to core
     */
    point->z = 0;

    for (point->y = 0; point->y <= Image_bbx.yt; point->y++) /* for all rows */
    for (point->x = 0; point->x <= Image_bbx.xr; point->x++) /* for all points */
	if (has_neighbour (point, D))
	{   /* via into core */
	    viaindex = ViaIndex[to_core (point->x, X)][to_core (point->y, Y)][0];

	    if (viaindex < 0)
	    { /* not allowed there! */
		add_error_marker (map, "illegal_via", point->x, point->y);
		if (errcount == 0) {  /* welcome message */
		    fprintf (stderr, "ERROR: Via(s) to image at illegal position (not removed):");
		    errcount = 5;
		}
		if (errcount % 6 == 0) { /* newline */
		    fprintf (stderr, "\n       ");
		}

		fprintf (stderr, "(%ld,%ld), ", map_to_layout_coord (point->x, X),
		map_to_layout_coord (point->y, Y));

		errcount++;

		/* continue; */
	    }

	    NewWire (wire);
	    wire->crd[L] = wire->crd[R] = (short) point->x;
	    wire->crd[B] = wire->crd[T] = (short) point->y;
	    wire->layer = (short) 100;              /* via to core */
	    wire->next = map->layoutstruct->wire;
	    map->layoutstruct->wire = wire;
	}

    if (errcount) fprintf (stderr, "\n");
}

/* This routine writes an as long as possible strip in the direction dir.
 */
static void mk_wire (MAPTABLEPTR map, GRIDWIREPTR point, int dir)
{
    GRIDWIRE pointstruct;
    GRIDWIREPTR npoint;
    WIREPTR wire;
    int neighbour_found = 0;

    npoint = &pointstruct;
    copy_point (npoint, point);

    NewWire (wire);
    wire->crd[L] = wire->crd[R] = (short) point->x;
    wire->crd[B] = wire->crd[T] = (short) point->y;
    wire->layer = (short) point->z + 1;

    /* processed is TRUE if the gridpoint is aleady covered
     * by a previous wire
     */
    while (has_neighbour (npoint, dir))
    {
	step_point (npoint, dir);

	if (npoint->x > Image_bbx.xr || npoint->y > Image_bbx.yt) break;

	neighbour_found = 1;

	/* set new rectangle coords */
	wire->crd[R] = (short) npoint->x;
	wire->crd[T] = (short) npoint->y;
    }

    if (neighbour_found) { /* link */
	wire->next = map->layoutstruct->wire;
	map->layoutstruct->wire = wire;
    }
    else {
	FreeWire (wire); /* useless */
    }
}

/* This routine writes the vias (stored as model calls) into the grid.
 */
static void read_terminals (MAPTABLEPTR map, DM_CELL *cell_key)
{
    DM_STREAM *fp;
    NELSISOBJECTPTR no;
    CIRPORTPTR cterm;
    LAYPORTPTR hterm;
    long z, gx, gy;
    int dmresult, reperror = 0;

    /* open term files for reading */
    if (!(fp = dmOpenStream (cell_key, "term", "r"))) {
	error (ERROR, "read_terminals()"); return;
    }
    while ((dmresult = dmGetDesignData (fp, GEO_TERM)) > 0)
    {
	/* is it a metal layer */
	for (z = 0; z < Chip_num_layer; ++z)
	    if (LayerMaskNo[z] == gterm.layer_no) break;

	if (z < Chip_num_layer && TermRead == TRUE) /* yes, it is metal layer ``z'' */
	{
	    if (gterm.nx > 0 || gterm.ny > 0) reperror = 1;

	    gx = map_lambda_to_grid_coord ((gterm.xl+gterm.xr)>>1, X);
	    gy = map_lambda_to_grid_coord ((gterm.yb+gterm.yt)>>1, Y);

	    gx += autoshift[X];
	    gy += autoshift[Y];

	    if (OverlayGridMapping[X] &&
		gy%GridRepitition[Y] >= OverlayBounds[Y][L] &&
		gy%GridRepitition[Y] <= OverlayBounds[Y][R])
	    { /* in overlay area */
		gx = map_lambda_to_overlay_grid_coord ((gterm.bxl+gterm.bxr)>>1, X) + autoshift[X];
	    }

	    if (OverlayGridMapping[Y] &&
		gx%GridRepitition[X] >= OverlayBounds[X][L] &&
		gx%GridRepitition[X] <= OverlayBounds[X][R])
	    {
		gy = map_lambda_to_overlay_grid_coord ((gterm.byb+gterm.byt)>>1, Y) + autoshift[Y];
	    }

	    /* make pointer to circuit cell */
	    /* assumes name-relation between layout and circuit view */
	    for (cterm = map->circuitstruct->cirport;
		cterm && strcmp (cterm->name, gterm.term_name); cterm = cterm->next) ;

	    if (!cterm) { /* not found: make */
		NewCirport (cterm);
		cterm->name = canonicstring (gterm.term_name);
		cterm->next = map->circuitstruct->cirport;
#ifdef SDF_PORT_DIRECTIONS
		cterm->direction = SDF_PORT_UNKNOWN;
#endif
		map->circuitstruct->cirport = cterm;
	    }

	    NewLayport (hterm);
	    hterm->cirport = cterm;
	    hterm->layer = (short) z;
	    hterm->pos[X] = (short) gx;
	    hterm->pos[Y] = (short) gy;
	    hterm->next = map->layoutstruct->layport;
	    map->layoutstruct->layport = hterm;
	}
	else
	{ /* Not metal, or not requested: store in NELSISTRUCT */
	    if (z >= Chip_num_layer) {
		fprintf (stderr, "ERROR: Terminal '%s' is in illegal mask (%ld) (terminal ignored)\n", gterm.term_name, gterm.layer_no);
		continue;
	    }

	    NewNelsisobject (no);
	    no->next = map->list_of_unfished_objects;
	    map->list_of_unfished_objects = no;

	    no->type = GTERM_FISH;

	    /* copy it */
	    STRINGSAVE(no->name, gterm.term_name);
	    no->layer_no = gterm.layer_no;
	    no->xl = gterm.xl + nelsisshift[X];
	    no->xr = gterm.xr + nelsisshift[X];
	    no->yb = gterm.yb + nelsisshift[Y];
	    no->yt = gterm.yt + nelsisshift[Y];
	    no->bxl = gterm.bxl + nelsisshift[X];
	    no->bxr = gterm.bxr + nelsisshift[X];
	    no->byb = gterm.byb + nelsisshift[Y];
	    no->byt = gterm.byt + nelsisshift[Y];
	    no->dx = gterm.dx; no->nx = gterm.nx;
	    no->dy = gterm.dy; no->ny = gterm.ny;
	}
    }
    if (dmresult) fprintf (stderr, "\nSomething strange going on with dmGetDesignData(GEO_TERM)\n");

    if (reperror) fprintf (stderr, "WARNING: terminal array(s) found. Arrays were not expanded!\n");

    dmCloseStream (fp, COMPLETE); /* seen enough of this term stuff */
}

/* This routine writes labels to the database.
 */
static void read_labels (MAPTABLEPTR map, DM_CELL *cell_key)
{
    struct stat statBuf;
    DM_STREAM *fp;
    LAYLABELPTR laylabel;
    long z, gx, gy;
    int dmresult;

    /* is there an annotations stream */
    if (dmStat (cell_key, "annotations", &statBuf) == -1) return;

    /* open annotations files for reading */
    if (!(fp = dmOpenStream (cell_key, "annotations", "r"))) {
	error (ERROR, "read_labels()"); return;
    }

    /* format record first */
    dmGetDesignData (fp, GEO_ANNOTATE);

    while ((dmresult = dmGetDesignData (fp, GEO_ANNOTATE)) > 0)
    {
	if (ganno.type == GA_LABEL)
	{
	    /* is it a metal layer ? */
	    for (z = 0; z < Chip_num_layer; ++z)
		if (LayerMaskNo[z] == ganno.o.Label.maskno) break;

	    if (z < Chip_num_layer) /* yes, it is metal layer ``z'' */
	    {
		gx = map_lambda_to_grid_coord (ganno.o.Label.x, X);
		gy = map_lambda_to_grid_coord (ganno.o.Label.y, Y);

		gx += autoshift[X];
		gy += autoshift[Y];

		if (OverlayGridMapping[X] &&
		    gy%GridRepitition[Y] >= OverlayBounds[Y][L] &&
		    gy%GridRepitition[Y] <= OverlayBounds[Y][R])
		{ /* in overlay area */
		    gx = map_lambda_to_overlay_grid_coord (ganno.o.Label.x, X) + autoshift[X];
		}

		if (OverlayGridMapping[Y] &&
		    gx%GridRepitition[X] >= OverlayBounds[X][L] &&
		    gx%GridRepitition[X] <= OverlayBounds[X][R])
		{
		    gy = map_lambda_to_overlay_grid_coord (ganno.o.Label.y, Y) + autoshift[Y];
		}

		NewLaylabel (laylabel);
		laylabel->name = canonicstring (ganno.o.Label.name);
		laylabel->layer  = (short) z;
		laylabel->pos[X] = (short) gx;
		laylabel->pos[Y] = (short) gy;
		laylabel->next = map->layoutstruct->laylabel;
		map->layoutstruct->laylabel = laylabel;

	    }
	}
    }
    if (dmresult) fprintf (stderr, "\nSomething strange going on with dmGetDesignData(GEO_GEO_ANNOTATE)\n");

    dmCloseStream (fp, COMPLETE); /* seen enough of this term stuff */
}

/* This routine reads the model-calls of the son-cells and attaches them to
 * the layout structure of map.  The routine also sets the bounding box.
 */
static void read_mc (MAPTABLEPTR map, DM_CELL *cell_key)
{
    DM_STREAM *fp;
    NELSISOBJECTPTR no;
    long xnew, ynew, xgrid, ygrid, dx, dy, x0, y0,
    xindex, yindex, xpitch, ypitch, al[2];
    MAPTABLEPTR child_map;
    LAYINSTPTR inst;
    char hstr[256], s1[256], s2[256], *dotname;
    int dmresult, i, instcount, maxnum; /* to make unique instance names */
    struct geo_mc *gmcp;  /* debug help */

    gmcp = &gmc;

    instcount = 1;
    map->layoutstruct->bbx[X] = map->layoutstruct->bbx[Y] = 0;
    map->layoutstruct->off[X] = map->layoutstruct->off[Y] = 0;

    /* make dummy slice, containing chaos */
    if (!map->layoutstruct->slice) {
	NewSlice (map->layoutstruct->slice);
	map->layoutstruct->slice->ordination = CHAOS;
	map->layoutstruct->slice->chld_type = LAYINST_CHLD;
    }

    if (!(fp = dmOpenStream (cell_key, "mc", "r"))) {
	error (ERROR, "read_mc()"); return;
    }
    while ((dmresult = dmGetDesignData (fp, GEO_MC)) > 0)
    {
	/*
	 * filter special model calls
	 */

	/* is it the image */
	if (strcmp (gmc.cell_name, ImageName) == 0) continue;

	/* is it a via */
	for (i = 0; i < NumViaName; ++i)
	    if (ViaCellName[i] && strcmp (gmc.cell_name, ViaCellName[i]) == 0) break;
	if (i < NumViaName) continue;  /* yes, it is a via */

	/* is it an errormarker */
	if (strncmp (gmc.cell_name, "Error_Marker", 10) == 0) continue;

	/* look up child */
	child_map = look_up_map (layout_str, gmc.cell_name);

	if (MCRead == TRUE)
	{ /* snap mc to grid */

	    /* try to align.... */
	    if (child_map->alignment_found == FALSE &&
		child_map->no_alignment_found == FALSE && strcmp (ThisImage, "pm25"))
	    { /* first time: try to align.. */
		if (guess_alignment (child_map, &gmc) == FALSE)
		{  /* alignment failed ... */
		    child_map->no_alignment_found = TRUE;
#ifdef OLDMETHOD
		    /* disabled by Arjan van Genderen (see also below) */
		    fprintf (stderr, "WARNING: No contact to align instance '%s' in cell '%s'.\n", gmc.cell_name, map->cell);
#endif
		    if (Seadif_open) /* nelsea */
		    fprintf (stderr, "         Proper instance placement is not guaranteed\n");
		}
	    }

	    if (child_map->alignment_found == TRUE)
	    { /* do the alignment... */
		/* child_map->align contains the crd of a point which is
		on grid. Transform over instance mtx... */
		al[X] = gmc.mtx[0]*child_map->align[X] +
		gmc.mtx[1]*child_map->align[Y] + gmc.mtx[2];
		al[Y] = gmc.mtx[3]*child_map->align[X] +
		gmc.mtx[4]*child_map->align[Y] + gmc.mtx[5];
		/* snap it to the nearest grid point */
		xgrid = map_lambda_to_grid_coord (al[X], X);
		ygrid = map_lambda_to_grid_coord (al[Y], Y);
		/* calc. lamba crd of the gridpoint.. */
		xnew = map_to_layout_coord (xgrid, X);
		ynew = map_to_layout_coord (ygrid, Y);
		/* determine the offset vector */
		dx = xnew - al[X];
		dy = ynew - al[Y];
		gmc.bxl += dx; gmc.bxr += dx; gmc.mtx[2] += dx;
		gmc.byb += dy; gmc.byt += dy; gmc.mtx[5] += dy;
	    }

#ifdef OLDMETHOD
	    /* The following is disabled now.
		If alignment is not possible, we just leave the instance
		at the position where it was.  Arjan van Genderen
	    */
	    if ((child_map->no_alignment_found == TRUE && Seadif_open) || strcmp (ThisImage, "pm25") == 0)
	    {
		/* emergency alignment:
		 * snap leftbottom of instance to nearest layout repitition
		 */
		/* This is not optimal... if fish, leave it and store into nelsisobject */

		dx = (gmc.bxl + (LayoutRepitition[X]/2))/LayoutRepitition[X];
		dy = (gmc.byb + (LayoutRepitition[Y]/2))/LayoutRepitition[Y];
		xnew = dx * LayoutRepitition[X];
		ynew = dy * LayoutRepitition[Y];
		dx = xnew - gmc.bxl;
		dy = ynew - gmc.byb;
		gmc.bxl += dx; gmc.bxr += dx; gmc.mtx[2] += dx;
		gmc.byb += dy; gmc.byt += dy; gmc.mtx[5] += dy;
	    }
#endif
	}

	xpitch = ypitch = 0;
	if (gmc.nx > 0 && MCRead == TRUE && (child_map->no_alignment_found == FALSE || Seadif_open))
	{ /* determine offset of repitition */
	    xpitch = map_lambda_to_grid_coord (gmc.bxl + gmc.dx, X) - map_lambda_to_grid_coord (gmc.bxl, X);
	    if (xpitch <= 0) {
		fprintf (stderr, "WARNING: strange xpitch for repetition of inst '%s' in cell '%s'.\n", gmc.inst_name, gmc.cell_name);
		fprintf (stderr, "         The array will not be expanded.\n");
		gmc.nx = 0;
	    }
	}

	if (gmc.ny > 0 && MCRead == TRUE && (child_map->no_alignment_found == FALSE || Seadif_open))
	{ /* determine offset of repitition */
	    ypitch = map_lambda_to_grid_coord (gmc.byb + gmc.dy, Y) - map_lambda_to_grid_coord (gmc.byb, Y);
	    if (ypitch <= 0) {
		fprintf (stderr, "WARNING: strange ypitch for repetition of inst '%s' in cell '%s'.\n", gmc.inst_name, gmc.cell_name);
		fprintf (stderr, "         The array will not be expanded.\n");
		gmc.ny = 0;
	    }
	}

	/*
	 * set bounding box
	 */
	if (map_lambda_to_grid_coord (gmc.bxr, X) + autoshift[X] > map->layoutstruct->bbx[X])
	    map->layoutstruct->bbx[X] = (short) (map_lambda_to_grid_coord (gmc.bxr, X) + autoshift[X]);
	if (map_lambda_to_grid_coord (gmc.byt, Y) + autoshift[Y] > map->layoutstruct->bbx[Y])
	    map->layoutstruct->bbx[Y] = (short) (map_lambda_to_grid_coord (gmc.byt, Y) + autoshift[Y]);

#if 0
	if (extra_verbose &&
	    child_map->nelseastatus != written_str &&
	    child_map->nelseastatus != primitive_str &&
	    child_map->internalstatus != in_core_str)
	    fprintf (stderr, "WARNING (read_mc): instance reference to non-converted cell: '%s'.\n", gmc.cell_name);
#endif

	/* already in datastruct ?? */
	if (!child_map->layoutstruct ||
	    !child_map->circuitstruct ||
	    !child_map->functionstruct ||
	    !child_map->librarystruct)
	{ /* make a cell in datastruct */
	    attach_map_to_lib (child_map);
	}

	if (MCRead == TRUE && (child_map->no_alignment_found == FALSE || Seadif_open))
	{
	    for (xindex = 0; xindex <= gmc.nx; xindex++)
	    for (yindex = 0; yindex <= gmc.ny; yindex++)
	    {
		/* make instance */
		NewLayinst (inst);

		/* temporarily set instance name, later we make it unique */
		inst->name = cs (gmc.inst_name);

		/* save the repetition params in the instance, so we can use
		 * them later when we try to make a unique instance name:
		 */
		inst->flag.s[0] = (short) xindex;
		inst->flag.s[1] = (short) yindex;

		inst->layout = child_map->layoutstruct;

		inst->mtx[0] = (short) gmc.mtx[0];
		inst->mtx[1] = (short) gmc.mtx[1];
		inst->mtx[3] = (short) gmc.mtx[3];
		inst->mtx[4] = (short) gmc.mtx[4];

		/* determine mapping of 0,0 */
		x0 = map_to_layout_coord (0, X);
		y0 = map_to_layout_coord (0, Y);
		dx = gmc.mtx[0] * x0 + gmc.mtx[1] * y0 + gmc.mtx[2];
		dy = gmc.mtx[3] * x0 + gmc.mtx[4] * y0 + gmc.mtx[5];

		/* dx, dy is now the offset vector to a legal grid point */
		inst->mtx[2] = (short) map_lambda_to_grid_coord (dx, X);
		inst->mtx[5] = (short) map_lambda_to_grid_coord (dy, Y);

		/* add pitch */
		inst->mtx[2] += (short) ((xindex * xpitch) + autoshift[X]);
		inst->mtx[5] += (short) ((yindex * ypitch) + autoshift[Y]);

		inst->next = map->layoutstruct->slice->chld.layinst;
		map->layoutstruct->slice->chld.layinst = inst;
	    }
	}
	else
	{ /* store in nelsisstruct */
	    NewNelsisobject (no);
	    no->next = map->list_of_unfished_objects;
	    map->list_of_unfished_objects = no;

	    no->type = GMC_FISH;

	    /* copy it */
	    STRINGSAVE(no->name, gmc.inst_name);
	    STRINGSAVE(no->cell_name, gmc.cell_name);
	    no->imported = gmc.imported;
	    for (i = 0; i != 6; i++) no->mtx[i] = gmc.mtx[i];
	    no->mtx[2] += nelsisshift[X];
	    no->mtx[5] += nelsisshift[Y];
	    no->bxl = gmc.bxl + nelsisshift[X];
	    no->bxr = gmc.bxr + nelsisshift[X];
	    no->byb = gmc.byb + nelsisshift[Y];
	    no->byt = gmc.byt + nelsisshift[Y];
	    no->dx = gmc.dx; no->nx = gmc.nx;
	    no->dy = gmc.dy; no->ny = gmc.ny;
	}
    }
    if (dmresult) fprintf (stderr, "Something strange going on with dmGetDesignData(GEO_MC)\n");

    dmCloseStream (fp, COMPLETE);

    /* Make unique and canonic instance names. First find out what's the highest
     * serial number in use (maxnum). Then only generate numbers > maxnum...
     * This way, we don't need no hashing algorithm and still get unique names
     * in linear time.
     */
    for (maxnum = 0, inst = map->layoutstruct->slice->chld.layinst; inst; inst = inst->next)
    {
	int na, nb = 0, nc = 0;
	/* Check for instance names of the form "%s_%d%s" where the second %s may
	 * be empty and the first %s may contain up to 2 underscores.  Find out
	 * what the largest value of %d is.
	 */
	na = sscanf (inst->name, "%[a-zA-Z0-9]_%[0-9]", s1, s2);
	if (na == 1)
	    nb = sscanf (inst->name, "%*[a-zA-Z0-9]_%[a-zA-Z0-9]_%[0-9]", s1, s2);
	if (nb == 1)
	    nc = sscanf (inst->name, "%*[a-zA-Z0-9]_%*[a-zA-Z0-9]_%[a-zA-Z0-9]_%[0-9]", s1, s2);
	if (na == 2 || nb == 2 || nc == 2) {
	    int num = atoi (s2);
	    MAX_UPDATE (maxnum, num); /* never saw a %d as large as this one... */
	}
    }

    instcount = maxnum+1; /* only generate serial numbers > maxnum */
    dotname = cs (".");

    for (inst = map->layoutstruct->slice->chld.layinst; inst;)
    {
	/* determine instance name of each instance
	 */
	if (inst->name == dotname)
	    /* dummy inst name by dali: make inst name */
	    sprintf (hstr, "%s_%d", inst->layout->name, instcount++);
	else /* ordinary name */
	    strcpy (hstr, inst->name);

	/* Now we actually allocate the instance name. We use the fact that
	* instances that are just repeated copies of each other all appear
	* immediately after each other in the ...slice->chld.layinst list.
	* This list was constructed in reverse order of the index, so it is just
	* as if we are executing this nested for-loop:
	*     for (xindex = inst->flag.s[0]; xindex >= 0 ; xindex--)
	*        for (yindex = inst->flag.s[1]; yindex >= 0 ; yindex--)
	* The loop ends when xindex==yindex==0; we check for this condition by
	* testing the equivalent expression (inst->flag.s[0] == 0 &&
	* inst->flag.s[1] == 0).  The basename hstr[] is not changed within the
	* loop, of course.
	*/
	for (; inst; inst = inst->next)
	{
	    if (!inst->flag.s[0] && !inst->flag.s[1]) {
		inst->name = add_index_to_name (hstr, -1, -1); /* no rep */
		break;
	    }
	    else if (inst->flag.s[0] && !inst->flag.s[1])
		inst->name = add_index_to_name (hstr, inst->flag.s[0], -1);
	    else if (!inst->flag.s[0] && inst->flag.s[1])
		inst->name = add_index_to_name (hstr, inst->flag.s[1], -1);
	    else
		inst->name = add_index_to_name (hstr, inst->flag.s[0], inst->flag.s[1]);
	}
	if (inst) inst = inst->next;
    }

    /* if nothing found: remove slice */
    if (!map->layoutstruct->slice->chld.layinst) {
	FreeSlice (map->layoutstruct->slice);
	map->layoutstruct->slice = NULL;
    }

    /* bounding box */
    if (map->layoutstruct->bbx[X] < Image_bbx.xr) map->layoutstruct->bbx[X] = (short) Image_bbx.xr;
    if (map->layoutstruct->bbx[Y] < Image_bbx.yt) map->layoutstruct->bbx[Y] = (short) Image_bbx.yt;

    /* bounding box gives width of cell: increment */
    map->layoutstruct->bbx[X]++;
    map->layoutstruct->bbx[Y]++;
}

/* This routine tries to find a alignment for an instance.
 */
static int guess_alignment (MAPTABLEPTR map, struct geo_mc *geomc)
{
    DM_PROJECT *remote_projectkey;
    DM_CELL    *key;
    DM_STREAM  *fp;
    char *remote_cellname, *maskname;
    MAPTABLEPTR child_map;
    struct geo_mc savegeomc;
    int dmresult, imported, num, z;

    if (!map) {
	fprintf (stderr, "ERROR (guess_alignment): map is NULL\n");
	return (FALSE);
    }

    if (map->alignment_found == TRUE) return (TRUE);

    if (map->no_alignment_found == TRUE) return (FALSE);

    map->no_alignment_found = TRUE; /* default: failed */

 /* if (!map->nelsis_time) return (FALSE); */

    savegeomc = (*geomc);

    /* look for this basic image cell */
    if ((imported = exist_cell (map->cell, layout_str)) < 0) return (FALSE); /* not exist */

    /* open remote project */
    if (!(remote_projectkey = dmFindProjKey (imported, map->cell, projectkey, &remote_cellname, layout_str))) {
	error (ERROR, "cannot find nasty project key");
	return (FALSE);
    }

    /* open remote cell */
    if (!(key = dmCheckOut (remote_projectkey, remote_cellname, ACTUAL, DONTCARE, layout_str, READONLY))) {
	fprintf (stderr, "ERROR: cannot open instance '%s' in database\n", remote_cellname);
	return (FALSE);
    }

    if (map->bbx_found == FALSE && !Seadif_open)
    {
	/* read bounding box, required for write anyway later on */
	if (!(fp = dmOpenStream (key, "info", "r"))) {
	    fprintf (stderr, "ERROR: cannot open info of cell '%s'\n", map->cell);
	    error (FATAL_ERROR, "write_mc()");
	}

	num = 1;
	while (dmGetDesignData (fp, GEO_INFO) > 0 && num < 6) {
	    if (num == 1 || num == 4) { /* take 4th bbx, if possible */
		map->bbx[L] = ginfo.bxl;
		map->bbx[R] = ginfo.bxr;
		map->bbx[B] = ginfo.byb;
		map->bbx[T] = ginfo.byt;
	    }
	    num++;
	}
	if (num <= 1) error (ERROR, "cannot find bbx");

	map->bbx_found = TRUE;
	map->imported = imported;
	dmCloseStream (fp, COMPLETE);
    }

    /* open MC to find contact */
    if (!(fp = dmOpenStream (key, "mc", "r"))) {
	error (ERROR, "mc guess_alignment()"); return (FALSE);
    }
    while ((dmresult = dmGetDesignData (fp, GEO_MC)) > 0)
    {
	if (strcmp (gmc.cell_name, ImageName) == 0) continue;
	for (z = 0; z < NumViaName; ++z) {
	    maskname = ViaCellName[z];
	    if (maskname && strcmp (gmc.cell_name, maskname) == 0) break;
	}
	if (z >= NumViaName) continue;

	/* found a model call to a contact: compute its position */
	map->align[X] = (gmc.bxl+gmc.bxr)>>1;
	map->align[Y] = (gmc.byb+gmc.byt)>>1;
	dmCloseStream (fp, COMPLETE); /* seen enough of this mc stuff */
	dmCheckIn (key, COMPLETE);
	*geomc = savegeomc;
	map->alignment_found = TRUE;
	map->no_alignment_found = FALSE;
	return (TRUE); /* we did it */
    }
    if (dmresult) err (4, "\nSomething strange going on with dmGetDesignData(GEO_MC)\n");
    dmCloseStream (fp, COMPLETE); /* seen enough of this mc stuff */

    /* did not find a contact, open box to find contact */
    if (!(fp = dmOpenStream (key, "box", "r"))) {
	error (ERROR, "boxguess_alignment()"); return (FALSE);
    }
    while ((dmresult = dmGetDesignData (fp, GEO_BOX)) > 0)
    {
	/* is it a via? */
	for (z = 0; z < NumViaName; z++)
	    if (gbox.layer_no == ViaMaskNo[z]) break;
	if (z >= NumViaName) continue; /* not a via */

	map->align[X] = (gbox.xl+gbox.xr)>>1;
	map->align[Y] = (gbox.yb+gbox.yt)>>1;
	dmCloseStream (fp, COMPLETE); /* seen enough of this mc stuff */
	dmCheckIn (key, COMPLETE);
	*geomc = savegeomc;
	map->alignment_found = TRUE;
	map->no_alignment_found = FALSE;
	return (TRUE);
    }
    if (dmresult) err (4, "\nSomething strange going on with dmGetDesignData(GEO_BOX)\n");
    dmCloseStream (fp, COMPLETE); /* seen enough of this mc stuff */

    /*
     * cannot find any via on this level: do recursion:
     */
    if (!(fp = dmOpenStream (key, "mc", "r"))) err (5, "Cannot open mc stream - that's a pitty !");

    while ((dmresult = dmGetDesignData (fp, GEO_MC)) > 0)
    {
	if (strcmp (gmc.cell_name, ImageName) == 0) continue;

	for (z = 0; z < NumViaName; ++z) {
	    maskname = ViaCellName[z];
	    if (maskname && strcmp (gmc.cell_name, maskname) == 0) break;
	}
	if (z < NumViaName) continue;

	child_map = look_up_map (layout_str, gmc.cell_name);

	/* Found a call to an unknown model. */
	/* Try to guess recursively */
	if (guess_alignment (child_map, &gmc) == TRUE)
	{ /* successful: transform point over mtx */
	    map->align[X] = gmc.mtx[0]*child_map->align[X] + gmc.mtx[1]*child_map->align[Y] + gmc.mtx[2];
	    map->align[Y] = gmc.mtx[3]*child_map->align[X] + gmc.mtx[4]*child_map->align[Y] + gmc.mtx[5];
	    dmCloseStream (fp, COMPLETE);	  /* Seen enough of this mc stuff. */
	    dmCheckIn (key, COMPLETE);
	    *geomc = savegeomc;
	    map->alignment_found = TRUE;
	    map->no_alignment_found = FALSE;
	    return (TRUE); /* we did it */
	}
    }
    dmCloseStream (fp, COMPLETE);

    dmCheckIn (key, COMPLETE);

    if (dmresult) err (4, "\nSomething strange going on with dmGetDesignData(GEO_MC)\n");
    *geomc = savegeomc;
    return (FALSE); /* failed to find a contact model call */
}

/* This routine rounds the NELSIS-coordinate lambda to the nearest grid point.
 */
long map_lambda_to_grid_coord (long lcrd, int ori)
{
   long grep;		/* grid repeated */
   long lrest;		/* lambda rest */
   long restgrid;	/* grid points rest */
   long llowerbound, lupperbound; /* lambda bounds */
   long finalgridvalue;	/* value returned by this function */

   grep  = lcrd / LayoutRepitition[ori];
   lrest = lcrd % LayoutRepitition[ori];

   for (restgrid = 0; restgrid < GridRepitition[ori]; ++restgrid)
      if (GridMapping[ori][restgrid]>lrest) break;

   if (!restgrid) {
      /* lowerbound is negative */
      llowerbound = GridMapping[ori][GridRepitition[ori]-1] - LayoutRepitition[ori];
      lupperbound = GridMapping[ori][0];
   }
   else if (restgrid >= GridRepitition[ori]) {
      /* just like !restgrid, but one image blok further away */
      llowerbound = GridMapping[ori][GridRepitition[ori]-1];
      lupperbound = GridMapping[ori][0] + LayoutRepitition[ori];
   }
   else {
      llowerbound = GridMapping[ori][restgrid-1];
      lupperbound = GridMapping[ori][restgrid];
   }

   /* Now decide which one is closest to a grid point.
    * If equally close, llowerbound wins.
    */
   finalgridvalue = grep * GridRepitition[ori] + restgrid;

   if (lrest - llowerbound <= lupperbound - lrest) --finalgridvalue;
   MAX_UPDATE (finalgridvalue, 0);

   return (finalgridvalue);
}

/* This routine rounds the NELSIS-coordinate lambda to the nearest grid point.
 * IN OVERLAY AREA
 */
static long map_lambda_to_overlay_grid_coord (long lcrd, int ori)
{
   long grep;		/* grid repeated */
   long lrest;		/* lambda rest */
   long restgrid;	/* grid points rest */
   long llowerbound, lupperbound; /* lambda bounds */
   long finalgridvalue;	/* value returned by this function */

   grep  = lcrd / LayoutRepitition[ori];
   lrest = lcrd % LayoutRepitition[ori];

   for (restgrid = 0; restgrid < GridRepitition[ori]; ++restgrid)
      if (OverlayGridMapping[ori][restgrid]>lrest) break;
   if (!restgrid) {
      /* lowerbound is negative */
      llowerbound = OverlayGridMapping[ori][GridRepitition[ori]-1] - LayoutRepitition[ori];
      lupperbound = OverlayGridMapping[ori][0];
   }
   else if (restgrid >= GridRepitition[ori]) {
      /* just like !restgrid, but one image blok further away */
      llowerbound = OverlayGridMapping[ori][GridRepitition[ori]-1];
      lupperbound = OverlayGridMapping[ori][0] + LayoutRepitition[ori];
   }
   else {
      llowerbound = OverlayGridMapping[ori][restgrid-1];
      lupperbound = OverlayGridMapping[ori][restgrid];
   }

   /* Now decide which one is closest to a grid point.
    * If equally close, llowerbound wins.
    */
   finalgridvalue = grep * GridRepitition[ori] + restgrid;
   if (lrest - llowerbound <= lupperbound - lrest) --finalgridvalue;
   MAX_UPDATE (finalgridvalue, 0);
   return (finalgridvalue);
}

static void add_error_marker (MAPTABLEPTR map, char *name, long x, long y)
{
    MAPTABLEPTR marker_map;
    LAYINSTPTR linst;
    static int marker_counter;
    int reset_counter;
    char hstr[50];

    /*
     * input checking..
     */
    if (!map || !map->layoutstruct || !name) {
	fprintf (stderr, "add_error_marker: incorrect call\n");
	return;
    }
    if (x > Image_bbx.xr || y > Image_bbx.yt || x < 0 || y < 0) {
	fprintf (stderr, "add_error_marker: point outside bbx\n");
	return;
    }

    /*
     * make dummy slice if neccesary, containing chaos
     */
    if (!map->layoutstruct->slice) {
	NewSlice (map->layoutstruct->slice);
	map->layoutstruct->slice->ordination = CHAOS;
	map->layoutstruct->slice->chld_type = LAYINST_CHLD;
    }

    /*
     * find the marker
     */
    marker_map = look_up_map (layout_str, "Error_Marker");
    if (!marker_map->layoutstruct || !marker_map->internalstatus) return;

    /*
     * test whether there is already a marker on that position
     */
    reset_counter = 1;
    for (linst = map->layoutstruct->slice->chld.layinst; linst; linst = linst->next)
    {
	if (linst->layout == marker_map->layoutstruct) {
	    reset_counter = 0;
	    if (linst->mtx[2] == x && linst->mtx[5] == y) break;
	}
    }
    if (linst) return; /* already there */

    if (reset_counter)
	marker_counter = 1;
    else
	marker_counter++;

    /*
     * make instance of unconnect
     */
    NewLayinst (linst);
    sprintf (hstr, "%s%d", name, marker_counter);
    linst->name = cs (hstr);
    linst->layout = marker_map->layoutstruct;
    linst->mtx[0] = 1; linst->mtx[1] = 0; linst->mtx[2] = (short) x;
    linst->mtx[3] = 0; linst->mtx[4] = 1; linst->mtx[5] = (short) y;
    linst->next = map->layoutstruct->slice->chld.layinst;
    map->layoutstruct->slice->chld.layinst = linst;
}
