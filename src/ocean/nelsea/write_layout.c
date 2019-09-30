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

#include <time.h> /* prototypes the time() syscall */

#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/nelsea/grid.h"
#include "src/ocean/nelsea/tbox.h"
#include "src/ocean/nelsea/prototypes.h"

#define map_to_overlay_layout_coord(crd,ori) \
(((crd) / GridRepitition[ori]) * LayoutRepitition[ori]) + OverlayGridMapping[ori][(crd) % GridRepitition[ori]]

static void write_layout (LAYOUTPTR, int, int, long, long);
static void recursive_write (SLICEPTR);
static void write_image (LAYOUTPTR, DM_STREAM *, long, long);
static void write_mc (MAPTABLEPTR, DM_STREAM *, DM_STREAM *);
static void do_write_mc (SLICEPTR, DM_STREAM *, DM_STREAM *);
static void write_box (MAPTABLEPTR, DM_STREAM *);
static void write_terminals (MAPTABLEPTR, DM_STREAM *, DM_STREAM *);
static void write_labels (MAPTABLEPTR, DM_STREAM *);
static void write_vias (LAYOUTPTR, DM_STREAM *, DM_STREAM *);

extern char
   *primitive_str,
   *in_core_str,
   *not_in_core_str,
   *circuit_str,
   *layout_str,
   *written_str;
extern int
   Flat_vias,              /* TRUE to print the vias as boxes instead of mc's */
  *ViaCellImported,        /* contains flag whether the Via Cell is imported or not */
   Write_Floorplan,        /* TRUE to write a floorplan of the cell... */
   verbose;
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
   *LayerWidth,            /* array with wire width in each layer. size: Chip_num_layer */
   *LayerMaskNo,           /* contains NELSIS mask numbers of layer mask string in array LayerMaskName. size: ChipNumLayer */
   DummyMaskNo,
   *ViaCellBbx[4];         /* contains the boundingboxes of the vias. size: 4 * NumViaName */
extern char
   ImageName[],            /* NELSIS name of image to be repeated */
   **LayerMaskName,        /* array with mask names of each layer. size: Chip_num_layer */
   **ViaMaskName,          /* array with mask names of via to layer. size: NumViaName */
   **ViaCellName;          /* array with cell names of these vias . size: NumViaName */

extern DM_PROJECT *projectkey;
extern MAPTABLEPTR maptable;
extern TBOXPTR    *ViaBox; /* array containing the box structure of each via */

static long bxl, bxr, byb, byt; /* bounding box */

void strNcpy (char *dest, char *src, int n)
{
    int i;
    for (i = 0; i < n && src[i] != '\0'; ++i) dest[i] = src[i];
    dest[i] = '\0';
}

/* This routine writes all layouts in the current datastruct.
 * arguments:
 *	image_call - TRUE to add model call to image
 *	xsize, ysize - size of image (valid if > 0 and if image_call == TRUE)
 */
void write_nelsis_layout (int image_call, int recursive, long xsize, long ysize)
{
    MAPTABLEPTR map;

    for (map = maptable; map; map = map->next)
    {
	if (map->seanelstatus != written_str &&
	    map->internalstatus == in_core_str &&
	    map->seanelstatus != primitive_str &&
	    map->view == layout_str)
	{ /* write it */
	    if (map->overrule_status == TRUE)
		/* it's the parent cell, do with image call if enabled */
		write_layout (map->layoutstruct, image_call, recursive, xsize, ysize);
	    else
		/* always without image */
		write_layout (map->layoutstruct, FALSE, recursive, xsize, ysize);
	}
    }
}

/* This routine dumps the seadif datastructure in a proper NELSIS database cell.
 * Before calling the database should be opened.
 * arguments:
 *	image_call - TRUE to call the image
 *	recursive  - TRUE for recursive write
 *	xsize, ysize - pre-specified size of image only used if image_call == TRUE
 */
static void write_layout (LAYOUTPTR lay, int image_call, int recursive, long xsize, long ysize)
{
    MAPTABLEPTR map; /* points to datastructure */
    DM_STREAM *fp, *vfp, *flp_fp = NULL;
    DM_CELL *key, *flp_key = NULL;
    long rbxl, rbxr, rbyb, rbyt; /* real bounding box, without the image */

    if (!lay) {
	fprintf (stderr, "WARNING (write_layout): null struct.\n");
	return;
    }

    /* find out its mapping */
    map = look_up_seadif_map (layout_str,
		lay->circuit->function->library->name,
		lay->circuit->function->name,
		lay->circuit->name,
		lay->name);

    if (map->seanelstatus == written_str ||
	map->seanelstatus == primitive_str) {
	/* cell is already written or primitive */
	return;
    }

    if (map->internalstatus != in_core_str) {
     // fprintf (stderr, "ERROR: attempt to write cell '%s', which is not in core\n", map->layout);
	return;
    }

    /* recursively write other cells */
    if (recursive == TRUE) recursive_write (lay->slice);

    /* test: is the name too long? */
    if (strlen (map->cell) > DM_MAXNAME) { /* print warning */
	fprintf (stderr, "WARNING (write_layout): cell name %s too long, truncated\n", map->cell);
	map->cell[DM_MAXNAME] = '\0';
    }

    if (verbose) {
	printf ("------ writing sdflay '%s(%s(%s(%s)))' into nelsis '%s' ------\n",
	    lay->name,
	    lay->circuit->name,
	    lay->circuit->function->name,
	    lay->circuit->function->library->name,
	    map->cell);
	fflush (stdout);
    }

    /* open a new model file, called cell_name */
    if (!(key = dmCheckOut (projectkey, map->cell, WORKING, DONTCARE, layout_str, UPDATE)))
    {
	error (ERROR, "Unable to open cell (cell not written)");
	return;
    }

    /* open same in floorplan */
    if (Write_Floorplan) { /* open floorplan */
	if (!(flp_key = dmCheckOut (projectkey, map->cell, WORKING, DONTCARE, FLOORPLAN, UPDATE)))
	    error (WARNING, "Unable to open flp_cell");
    }
    else
	flp_key = NULL;

    /* initialize bounding box */
    bxl = byb = BIGNUMBER;
    bxr = byt = -BIGNUMBER;

    /* write terminals */
    if (!(fp = dmOpenStream (key, "term", "w"))) error (FATAL_ERROR, "write_to_nelsis");

    if (flp_key) flp_fp = dmOpenStream (flp_key, "term", "w");

    write_terminals (map, fp, flp_fp);

    dmCloseStream (fp, COMPLETE);
    if (flp_fp) dmCloseStream (flp_fp, COMPLETE);

    if (map->layoutstruct && map->layoutstruct->laylabel) {
	if (!(fp = dmOpenStream (key, "annotations", "w"))) error (FATAL_ERROR, "write_to_nelsis");
	write_labels (map, fp);
	dmCloseStream (fp, COMPLETE);
    }

    /* write box file= the wires in the grid */
    if (!(fp = dmOpenStream (key, "box", "w"))) error (FATAL_ERROR, "write_to_nelsis");

    write_box (map, fp);

    /* write model calls */
    if (!(vfp = dmOpenStream (key, "mc", "w"))) error (FATAL_ERROR, "write_to_nelsis");

    if (flp_key) flp_fp = dmOpenStream (flp_key, "mc", "w");

    /* vias */
    write_vias (map->layoutstruct, vfp, fp);

    write_mc (map, vfp, flp_fp);

    /* save the 'real' bounding box */
    rbxl = bxl;
    rbxr = bxr;
    rbyb = byb;
    rbyt = byt;

    /* basic image */
    if (image_call == TRUE) write_image (map->layoutstruct, vfp, xsize, ysize);

    dmCloseStream (vfp, COMPLETE);   /* mc */
    dmCloseStream (fp, COMPLETE);    /* box */

    if (flp_fp) dmCloseStream (flp_fp, COMPLETE);

    /* write bounding box */
    if (!(fp = dmOpenStream (key, "info", "w"))) error (FATAL_ERROR, "write_to_nelsis");

    if (flp_key) flp_fp = dmOpenStream (flp_key, "info", "w");

    if (bxl  == BIGNUMBER) bxl = bxr = 0;  /* was unchanged */
    if (byb  == BIGNUMBER) byb = byt = 0;  /* was unchanged */
    if (rbxl == BIGNUMBER) rbxl = rbxr = 0; /* was unchanged */
    if (rbyb == BIGNUMBER) rbyb = rbyt = 0; /* was unchanged */

    ginfo.bxl = finfo.bxl = bxl;
    ginfo.bxr = finfo.bxr = bxr;
    ginfo.byb = finfo.byb = byb;
    ginfo.byt = finfo.byt = byt;

    if (flp_fp) {
	dmPutDesignData (flp_fp, FLP_INFO);
	dmCloseStream (flp_fp, COMPLETE);
    }

    dmPutDesignData (fp, GEO_INFO); /* 1 */
    dmPutDesignData (fp, GEO_INFO); /* 2 */
    dmPutDesignData (fp, GEO_INFO); /* 3 */
    ginfo.bxl = rbxl;
    ginfo.bxr = rbxr;
    ginfo.byb = rbyb;
    ginfo.byt = rbyt;
    dmPutDesignData (fp, GEO_INFO); /* 4= real bounding box */

    dmCloseStream (fp, COMPLETE);

    /* make misterious files */
    if ((fp = dmOpenStream (key, "nor", "w"))) dmCloseStream (fp, COMPLETE);

    if (flp_key) {
	flp_fp = dmOpenStream (flp_key, "chan", "w");
	if (flp_fp) dmCloseStream (flp_fp, COMPLETE);
	dmCheckIn (flp_key, COMPLETE);
    }
    dmCheckIn (key, COMPLETE);

    /* set status */
    map->nelsis_time = time (0);
    map->seanelstatus = written_str;
}

/* This is a recursive help routine to perform depth-first
 * write of the children.
 */
static void recursive_write (SLICEPTR slice)
{
    LAYINSTPTR inst;

    /* depth-first write of children */
    for (; slice; slice = slice->next)
    {
	/* recursion? */
	if (slice->chld_type == SLICE_CHLD) {
	    recursive_write (slice->chld.slice);
	    return;
	}

	if (slice->chld_type != LAYINST_CHLD) {
	    fprintf (stderr, "WARNING (recursive_write): strange type of slice.\n");
	    return;
	}

	for (inst = slice->chld.layinst; inst; inst = inst->next) {
	    /* always without image and no force */
	    write_layout (inst->layout, FALSE, TRUE, 0L, 0L);
	}
    }
}

/* This routine writes the proper nelsis model calls to generate the image
 * below the circuit.
 * arguments:
 *	xsize, ysize - requested array size
 */
static void write_image (LAYOUTPTR lay, DM_STREAM *fp, long xsize, long ysize)
{
    DM_PROJECT *remote_projectkey;
    DM_CELL *cell_key;
    DM_STREAM *tempfp;
    char *remote_cellname;
    long bbx[4];
    int imported;

    /* look for this basic image cell */
    if ((imported = exist_cell (ImageName, layout_str)) < 0) {
	fprintf (stderr, "ERROR: cannot find elementary core image/template cell '%s' in database\n", ImageName);
	return;
    }

    /* open project */
    if (!(remote_projectkey = dmFindProjKey (imported, ImageName, projectkey, &remote_cellname, layout_str)))
    {
	error (ERROR, "cannot find nasty project key");
	return;
    }

    /* open cell */
    if (!(cell_key = dmCheckOut (remote_projectkey, remote_cellname, ACTUAL, DONTCARE, layout_str, READONLY)))
    {
	fprintf (stderr, "ERROR: cannot open core cell '%s' in database\n", ImageName);
	return;
    }

    /* read bounding box of image */
    if (!(tempfp = dmOpenStream (cell_key, "info", "r"))) error (FATAL_ERROR, "write_image");

    if (dmGetDesignData (tempfp, GEO_INFO) == -1) error (FATAL_ERROR, "write_image");

    bbx[L] = ginfo.bxl;
    bbx[R] = ginfo.bxr;
    bbx[B] = ginfo.byb;
    bbx[T] = ginfo.byt;

    /* terminate */
    dmCloseStream (tempfp, COMPLETE);
    dmCheckIn (cell_key, COMPLETE);

    strcpy (gmc.inst_name, "IMAGE");
    strcpy (gmc.cell_name, ImageName);
    gmc.imported = imported;
    gmc.mtx[0] = 1; gmc.mtx[1] = 0; gmc.mtx[2] = 0;
    gmc.mtx[3] = 0; gmc.mtx[4] = 1; gmc.mtx[5] = 0;

    if (strncmp (ImageName, "fishbone", strlen ("fishbone") - 1) == 0)
    { /* fistbone overlaps 1 in x and y */
	gmc.nx = (lay->bbx[X] + lay->off[X] - 1) / GridRepitition[X];
	gmc.ny = (lay->bbx[Y] + lay->off[Y] - 2) / GridRepitition[Y];
	if (gmc.nx < 0) gmc.nx = 0;
	if (gmc.ny < 0) gmc.ny = 0;
    }
    else {
	gmc.nx = (lay->bbx[X] + lay->off[X]) / GridRepitition[X];
	gmc.ny = (lay->bbx[Y] + lay->off[Y]) / GridRepitition[Y];
	if (strncmp (ImageName, "gatearray", 8) == 0 || strncmp (ImageName, "pm25", 4) == 0)
	{ /* hack to get image size of gatearray right */
	    gmc.nx-- ;  gmc.ny--;
	    if (gmc.nx < 0) gmc.nx = 0;
	    if (gmc.ny < 0) gmc.ny = 0;
	}
    }

    if (xsize > 0) gmc.nx = xsize - 1; /* size pre-specified */
    if (ysize > 0) gmc.ny = ysize - 1; /* size pre-specified */

    gmc.dx = LayoutRepitition[X];
    gmc.dy = LayoutRepitition[Y];

    gmc.bxl = bbx[L];
    gmc.byb = bbx[B];
    gmc.bxr = bbx[R] + (gmc.nx * LayoutRepitition[X]);
    gmc.byt = bbx[T] + (gmc.ny * LayoutRepitition[Y]);

    MIN_UPDATE (bxl, gmc.bxl);
    MIN_UPDATE (byb, gmc.byb);
    MAX_UPDATE (bxr, gmc.bxr);
    MAX_UPDATE (byt, gmc.byt);

    dmPutDesignData (fp, GEO_MC);
}

/* This routine writes the model-calls of the slice pointed at.
 * (argument 'flp_fp' is floorplan open stream pointer)
 */
static void do_write_mc (SLICEPTR slice, DM_STREAM *fp, DM_STREAM *flp_fp)
{
    long xl, xr, yb, yt, instcrd, orgcrd, x0, y0;
    DM_CELL *cell_key;
    DM_PROJECT *remote_projectkey;
    DM_STREAM *sfp;
    int imported, j, num;
    char *remote_cellname;
    LAYINSTPTR inst;
    MAPTABLEPTR sonmap;

    for (; slice; slice = slice->next)
    {
	/*
	 * recursion?
	 */
	if (slice->chld_type == SLICE_CHLD) {
	    do_write_mc (slice->chld.slice, fp, flp_fp);
	    continue;
	}

	if (slice->chld_type != LAYINST_CHLD) {
	    fprintf (stderr, "WARNING (write_mc): strange type of slice.\n");
	    return;
	}

	for (inst = slice->chld.layinst; inst; inst = inst->next)
	{

	    if (!inst->layout) {
		fprintf (stderr, "WARNING (write_mc): mc '%s' without layout.\n", inst->name);
		continue;
	    }

	    if (strncmp (inst->layout->name, "Error_Marker", 10) == 0)
	    { /* its'a error marker.. */
		sonmap = look_up_map (layout_str, "Error_Marker");
	    }
	    else
	    {
		sonmap = look_up_seadif_map (layout_str,
			    inst->layout->circuit->function->library->name,
			    inst->layout->circuit->function->name,
			    inst->layout->circuit->name,
			    inst->layout->name);
	    }

	    if (sonmap->internalstatus != in_core_str && sonmap->nelsis_time == 0)
	    {
		if (sonmap->num_read_attempts >= 2) continue;

		sonmap->num_read_attempts++; /* to prevent many printing */

		if (strncmp (inst->layout->name, "Error_Marker", 10) == 0) {
		    fprintf (stderr, "ERROR: can't find error marker cell '%s'\n", inst->layout->name);
		    continue;
		}

		fprintf (stderr, "WARNING: layout contains a reference to non-existing\n");
		fprintf (stderr, "         son-cell '%s' (skipped)\n", sonmap->cell);
		continue;
	    }

	    if (sonmap->bbx_found == FALSE)
	    { /* bbx was not yet found: do it... */
		/*
		 * look for son cell in database, to find its bbx
		 */
		if ((imported = exist_cell (sonmap->cell, layout_str)) < 0)
		{ /* it does not exist */
		    fprintf (stderr, "ERROR: cannot find son-cell '%s' (mapped '%s') of cell in database\n",
		    inst->layout->name, sonmap->cell);
		    continue;
		}

		/* open project */
		if (!(remote_projectkey = dmFindProjKey (imported, sonmap->cell, projectkey, &remote_cellname, layout_str)))
		{
		    fprintf (stderr, "ERROR: cannot find nasty project key\n");
		    continue;
		}

		/* open cell */
		if (!(cell_key = dmCheckOut (remote_projectkey, remote_cellname, ACTUAL, DONTCARE, layout_str, READONLY)))
		{
		    fprintf (stderr, "ERROR: cannot open son cell '%s' in database\n", sonmap->cell);
		    continue;
		}

		/* read bounding box */
		if (!(sfp = dmOpenStream (cell_key, "info", "r"))) {
		    fprintf (stderr, "ERROR: cannot open info of cell '%s'\n", sonmap->cell);
		    error (FATAL_ERROR, "write_mc()");
		}

		num = 1;
		while (dmGetDesignData (sfp, GEO_INFO) > 0 && num < 6)
		{  /* take 4th bbx, if possible */
		    if (num == 1 || num == 4)
		    {
			sonmap->bbx[L] = ginfo.bxl;
			sonmap->bbx[R] = ginfo.bxr;
			sonmap->bbx[B] = ginfo.byb;
			sonmap->bbx[T] = ginfo.byt;
		    }
		    num++;
		}
		if (num <= 1) error (ERROR, "cannot find bbx");

		sonmap->bbx_found = TRUE;
		sonmap->imported = imported;
		dmCloseStream (sfp, COMPLETE);
		dmCheckIn (cell_key, COMPLETE);
	    }

	    xl = sonmap->bbx[L];
	    xr = sonmap->bbx[R];
	    yb = sonmap->bbx[B];
	    yt = sonmap->bbx[T];

	    strNcpy (gmc.inst_name, inst->name  , DM_MAXNAME);
	    strNcpy (gmc.cell_name, sonmap->cell, DM_MAXNAME);

	    gmc.imported = sonmap->imported;

	    for (j = 0; j < 6; j++) gmc.mtx[j] = inst->mtx[j];

	    x0 = map_to_layout_coord (0, X);
	    y0 = map_to_layout_coord (0, Y);

	    instcrd = map_to_layout_coord (inst->mtx[2], X);
	    orgcrd = gmc.mtx[0] * x0 + gmc.mtx[1] * y0;
	    gmc.mtx[2] = instcrd - orgcrd;

	    instcrd = map_to_layout_coord (inst->mtx[5], Y);
	    orgcrd = gmc.mtx[3] * x0 + gmc.mtx[4] * y0;
	    gmc.mtx[5] = instcrd - orgcrd;

	    gmc.bxl = gmc.mtx[0] * xl + gmc.mtx[1] * yb + gmc.mtx[2];
	    gmc.bxr = gmc.mtx[0] * xr + gmc.mtx[1] * yt + gmc.mtx[2];
	    if (gmc.bxl > gmc.bxr) { x0 = gmc.bxl; gmc.bxl = gmc.bxr; gmc.bxr = x0; } /* swap */
	    gmc.byb = gmc.mtx[3] * xl + gmc.mtx[4] * yb + gmc.mtx[5];
	    gmc.byt = gmc.mtx[3] * xr + gmc.mtx[4] * yt + gmc.mtx[5];
	    if (gmc.byb > gmc.byt) { y0 = gmc.byb; gmc.byb = gmc.byt; gmc.byt = y0; } /* swap */

	    MIN_UPDATE (bxl, gmc.bxl);
	    MIN_UPDATE (byb, gmc.byb);
	    MAX_UPDATE (bxr, gmc.bxr);
	    MAX_UPDATE (byt, gmc.byt);

	    gmc.nx = gmc.ny = 0;       /* no repitition */

	    /* copy to floorplan */
	    strcpy (fmc.inst_name, gmc.inst_name);
	    strcpy (fmc.cell_name, gmc.cell_name);
	    fmc.imported = gmc.imported;
	    for (j = 0; j < 6; ++j) fmc.mtx[j] = gmc.mtx[j];
	    fmc.bxl = gmc.bxl; fmc.bxr = gmc.bxr;
	    fmc.byb = gmc.byb; fmc.byt = gmc.byt;
	    fmc.nx = fmc.ny = 0;

	    dmPutDesignData (fp, GEO_MC);

	    if (flp_fp) dmPutDesignData (flp_fp, FLP_MC);
	}
    }
}

/* This routine writes the model-calls of the slice pointed at.
 * (argument 'flp_fp' is floorplan open stream pointer)
 */
static void write_mc (MAPTABLEPTR map, DM_STREAM *fp, DM_STREAM *flp_fp)
{
    NELSISOBJECTPTR no;
    int j;

    /* call the real recursive writing routine */
    do_write_mc (map->layoutstruct->slice, fp, flp_fp);

    /* and now scan the nelsis objects for MC's
     * which were not fished
     */
    for (no = map->list_of_unfished_objects; no; no = no->next)
    {
	if (no->type != GMC_FISH) continue;

	strcpy (gmc.inst_name, no->name);
	strcpy (gmc.cell_name, no->cell_name);
	gmc.imported = no->imported;
	for (j = 0; j != 6; j++) gmc.mtx[j] = no->mtx[j];
	gmc.bxl = no->bxl; gmc.bxr = no->bxr;
	gmc.byb = no->byb; gmc.byt = no->byt;
	gmc.dx = no->dx; gmc.nx = no->nx;
	gmc.ny = no->ny; gmc.dy = no->dy;

	/* copy to floorplan */
	strcpy (fmc.inst_name, gmc.inst_name);
	strcpy (fmc.cell_name, gmc.cell_name);
	fmc.imported = gmc.imported;
	for (j = 0; j != 6; j++) fmc.mtx[j] = gmc.mtx[j];
	fmc.bxl = gmc.bxl; fmc.bxr = gmc.bxr;
	fmc.byb = gmc.byb; fmc.byt = gmc.byt;
	fmc.nx = fmc.ny = 0;

	MIN_UPDATE (bxl, gmc.bxl);
	MIN_UPDATE (byb, gmc.byb);
	MAX_UPDATE (bxr, gmc.bxr);
	MAX_UPDATE (byt, gmc.byt);

	dmPutDesignData (fp, GEO_MC);

	if (flp_fp) dmPutDesignData (flp_fp, FLP_MC);
    }
}

/*
 * writes the image to the database
 */
static void write_box (MAPTABLEPTR map, DM_STREAM *fp)
{
    WIREPTR box;
    LAYOUTPTR lay;
    NELSISOBJECTPTR no;
    long rest;
    long lwidth;
    int layer, no_dummy, illegal_layer, negative_wire;

    lay = map->layoutstruct;

    illegal_layer = negative_wire = no_dummy = FALSE;

    for (box = lay->wire; box; box = box->next)
    {

	if (box->layer < 0) { /* cannot write negative wires */
	    negative_wire = TRUE;
	    continue;
	}

	if (box->layer >= 100 && box->layer < 200)
	    continue;  /* not a box, but a via */

	if (box->layer >= 200)
	{ /* dummy mask */
	    if (box->layer == 200)
	    { /* real dummy */
		if (DummyMaskNo == -1) { no_dummy = TRUE; continue; }
		layer = 0;  /* use width of layer 0 */
		gbox.layer_no = DummyMaskNo;
		lwidth = LayerWidth[layer];
	    }
	    else { /* overlay */
		layer = box->layer - 201;
		if (layer < 0 || layer >= Chip_num_layer) {
		    fprintf (stderr, "illegal: %d\n", layer);
		    illegal_layer = TRUE;
		    continue;
		}
		gbox.layer_no = LayerMaskNo[layer];
		lwidth = LayerWidth[layer];

		/* find out the width of a via */
		/* Note: this takes the bounding box, which my not be correct.... */
		if ((rest = ViaCellBbx[R][0] - ViaCellBbx[L][0]) > lwidth) lwidth = rest;
	    }
	}
	else { /* metal mask */
	    layer = box->layer - 1;
	    if (layer < 0 || layer >= Chip_num_layer) { illegal_layer = TRUE; continue; }
	    gbox.layer_no = LayerMaskNo[layer];
	    lwidth = LayerWidth[layer];
	}

	rest = lwidth%2;
	if (OverlayGridMapping[X] == NULL ||
	    box->crd[T]%GridRepitition[Y] < OverlayBounds[Y][L] ||
	    box->crd[T]%GridRepitition[Y] > OverlayBounds[Y][R] ||
	    box->crd[B]%GridRepitition[Y] < OverlayBounds[Y][L] ||
	    box->crd[B]%GridRepitition[Y] > OverlayBounds[Y][R])
	{ /* ordinary */
	    gbox.xl = gbox.bxl = map_to_layout_coord (box->crd[L], X) - (lwidth/2);
	    gbox.xr = gbox.bxr = map_to_layout_coord (box->crd[R], X) + (lwidth/2) + rest;
	}
	else
	{ /* overlay */
	    gbox.xl = gbox.bxl = map_to_overlay_layout_coord (box->crd[L], X) - (lwidth/2);
	    gbox.xr = gbox.bxr = map_to_overlay_layout_coord (box->crd[R], X) + (lwidth/2) + rest;
	}

	if (OverlayGridMapping[Y] == NULL ||
	    box->crd[L]%GridRepitition[X] < OverlayBounds[X][L] ||
	    box->crd[L]%GridRepitition[X] > OverlayBounds[X][R] ||
	    box->crd[R]%GridRepitition[X] < OverlayBounds[X][L] ||
	    box->crd[R]%GridRepitition[X] > OverlayBounds[X][R])
	{ /* ordinary */
	    gbox.yb = gbox.byb = map_to_layout_coord (box->crd[B], Y) - (lwidth/2);
	    gbox.yt = gbox.byt = map_to_layout_coord (box->crd[T], Y) + (lwidth/2) + rest;
	}
	else
	{ /* overlay */
	    gbox.yb = gbox.byb = map_to_overlay_layout_coord (box->crd[B], Y) - (lwidth/2);
	    gbox.yt = gbox.byt = map_to_overlay_layout_coord (box->crd[T], Y) + (lwidth/2) + rest;
	}
	gbox.nx = gbox.ny = 0;

	MIN_UPDATE (bxl, gbox.bxl);
	MIN_UPDATE (byb, gbox.byb);
	MAX_UPDATE (bxr, gbox.bxr);
	MAX_UPDATE (byt, gbox.byt);

	dmPutDesignData (fp, GEO_BOX);
    }

    if (negative_wire == TRUE)
	fprintf (stderr, "WARNING (write_box): negative cannot be written into nelsis.\n");

    if (illegal_layer == TRUE)
	fprintf (stderr, "WARNING (write_box): found illegal metal layer.\n");

    if (no_dummy == TRUE)
	fprintf (stderr, "WARNING (write_box): found dummy layer, but no dummy mask was defined.\n");

    /*
     * and now scan the nelsisobjects for terminals
     */
    for (no = map->list_of_unfished_objects; no; no = no->next)
    {
	if (no->type != GBOX_FISH) continue;

	gbox.layer_no = no->layer_no;
	gbox.xl = no->xl; gbox.xr = no->xr;
	gbox.yb = no->yb; gbox.yt = no->yt;
	gbox.bxl = no->bxl; gbox.bxr = no->bxr;
	gbox.byb = no->byb; gbox.byt = no->byt;
	gbox.dx = no->dx; gbox.nx = no->nx;
	gbox.ny = no->ny; gbox.dy = no->dy;

	MIN_UPDATE (bxl, gbox.bxl);
	MIN_UPDATE (byb, gbox.byb);
	MAX_UPDATE (bxr, gbox.bxr);
	MAX_UPDATE (byt, gbox.byt);

	dmPutDesignData (fp, GEO_BOX);
    }
}

/* This routine writes a terminallist to the database.
 * (argument 'fp' is layout and 'flp_fp' is floorplan)
 */
static void write_terminals (MAPTABLEPTR map, DM_STREAM *fp, DM_STREAM *flp_fp)
{
    register LAYPORTPTR hterm;
    LAYOUTPTR lay;
    NELSISOBJECTPTR no;

    lay = map->layoutstruct;

    for (hterm = lay->layport; hterm; hterm = hterm->next)
    {
	if (hterm->layer < 0 || hterm->layer >= Chip_num_layer) {
	    fprintf (stderr, "WARING: terminal in illegal layer: '%s' (not written)\n", hterm->cirport->name);
	    continue; /* do not write illegal terminals */
	}

	if (LayerMaskNo[hterm->layer] < 0) continue; /* layer not defined */

	if (OverlayGridMapping[X] == NULL ||
	    hterm->pos[Y]%GridRepitition[Y] < OverlayBounds[Y][L] ||
	    hterm->pos[Y]%GridRepitition[Y] > OverlayBounds[Y][R])
	{ /* ordinary */
	    gterm.xl = gterm.bxl = map_to_layout_coord (hterm->pos[X], X) - (LayerWidth[hterm->layer]/2);
	}
	else
	{ /* overlay */
	    gterm.xl = gterm.bxl = map_to_overlay_layout_coord (hterm->pos[X], X) - (LayerWidth[hterm->layer]/2);
	}
	gterm.xr = gterm.bxr = gterm.xl + LayerWidth[hterm->layer];

	if (OverlayGridMapping[Y] == NULL ||
	    hterm->pos[X]%GridRepitition[X] < OverlayBounds[X][L] ||
	    hterm->pos[X]%GridRepitition[X] > OverlayBounds[X][R])
	{ /* ordinary */
	    gterm.yb = gterm.byb = map_to_layout_coord (hterm->pos[Y], Y) - (LayerWidth[hterm->layer]/2);
	}
	else
	{ /* overlay */
	    gterm.yb = gterm.byb = map_to_layout_coord (hterm->pos[Y], Y) - (LayerWidth[hterm->layer]/2);
	}
	gterm.yt = gterm.byt = gterm.yb + LayerWidth[hterm->layer];

	gterm.nx = gterm.ny = 0;
	strcpy (gterm.term_name, hterm->cirport->name);
	gterm.layer_no = LayerMaskNo[hterm->layer];

	MIN_UPDATE (bxl, gterm.bxl);
	MIN_UPDATE (byb, gterm.byb);
	MAX_UPDATE (bxr, gterm.bxr);
	MAX_UPDATE (byt, gterm.byt);

	/* mcopy to flp terminal */
	strcpy (fterm.term_name, gterm.term_name);
	fterm.term_attribute = NULL;
	fterm.layer_no = gterm.layer_no;
	fterm.side = 0;
	fterm.xl = fterm.bxl = gterm.xl;
	fterm.xr = fterm.bxr = gterm.xr;
	fterm.yb = fterm.byb = gterm.yb;
	fterm.yt = fterm.byt = gterm.yt;
	fterm.nx = fterm.ny = 0;

	dmPutDesignData (fp, GEO_TERM);

	if (flp_fp) dmPutDesignData (flp_fp, FLP_TERM);
    }

    /*
     * and now scan the nelsisobjects for terminals
     */
    for (no = map->list_of_unfished_objects; no; no = no->next)
    {
	if (no->type != GTERM_FISH) continue;

	strcpy (gterm.term_name, no->name);
	gterm.layer_no = no->layer_no;
	gterm.xl = no->xl; gterm.xr = no->xr;
	gterm.yb = no->yb; gterm.yt = no->yt;
	gterm.bxl = no->bxl; gterm.bxr = no->bxr;
	gterm.byb = no->byb; gterm.byt = no->byt;
	gterm.dx = no->dx; gterm.nx = no->nx;
	gterm.ny = no->ny; gterm.dy = no->dy;

	/* mcopy to flp terminal */
	strcpy (fterm.term_name, gterm.term_name);
	fterm.term_attribute = NULL;
	fterm.layer_no = gterm.layer_no;
	fterm.side = 0;
	fterm.xl = fterm.bxl = gterm.xl;
	fterm.xr = fterm.bxr = gterm.xr;
	fterm.yb = fterm.byb = gterm.yb;
	fterm.yt = fterm.byt = gterm.yt;
	fterm.nx = fterm.ny = 0;

	MIN_UPDATE (bxl, gterm.bxl);
	MIN_UPDATE (byb, gterm.byb);
	MAX_UPDATE (bxr, gterm.bxr);
	MAX_UPDATE (byt, gterm.byt);

	dmPutDesignData (fp, GEO_TERM);
	if (flp_fp) dmPutDesignData (flp_fp, FLP_TERM);
    }
}

/* This routine writes labels to the database.
 * (argument 'fp' is layout stream)
 */
static void write_labels (MAPTABLEPTR map, DM_STREAM *fp)
{
    register LAYLABELPTR hterm;
    LAYOUTPTR lay;

    /* format record first */
    ganno.type = GA_FORMAT;
    ganno.o.format.fmajor = ganno.o.format.fminor = 1;
    dmPutDesignData (fp, GEO_ANNOTATE);

    lay = map->layoutstruct;

    for (hterm = lay->laylabel; hterm; hterm = hterm->next)
    {
	if (hterm->layer < 0 || hterm->layer >= Chip_num_layer) {
	    fprintf (stderr, "WARING: label in illegal layer: '%s' (not written)\n", hterm->name);
	    continue; /* do not write illegal terminals */
	}

	if (LayerMaskNo[hterm->layer] < 0) continue; /* layer not defined */

	if (OverlayGridMapping[X] == NULL ||
	    hterm->pos[Y]%GridRepitition[Y] < OverlayBounds[Y][L] ||
	    hterm->pos[Y]%GridRepitition[Y] > OverlayBounds[Y][R])
	{ /* ordinary */
	    ganno.o.Label.x = map_to_layout_coord (hterm->pos[X], X);
	}
	else
	{ /* overlay */
	    ganno.o.Label.x = map_to_overlay_layout_coord (hterm->pos[X], X);
	}

	if (OverlayGridMapping[Y] == NULL ||
	    hterm->pos[X]%GridRepitition[X] < OverlayBounds[X][L] ||
	    hterm->pos[X]%GridRepitition[X] > OverlayBounds[X][R])
	{ /* ordinary */
	    ganno.o.Label.y = map_to_layout_coord (hterm->pos[Y], Y);
	}
	else
	{ /* overlay */
	    ganno.o.Label.y = map_to_layout_coord (hterm->pos[Y], Y);
	}

	ganno.o.Label.ay = 0;
	ganno.o.Label.ax = 0;
	ganno.type = GA_LABEL;
	strcpy (ganno.o.Label.name, hterm->name);
	ganno.o.Label.Class[0] = '\0';
	ganno.o.Label.Attributes[0] = '\0';
	ganno.o.Label.maskno = (int)LayerMaskNo[hterm->layer];
	dmPutDesignData (fp, GEO_ANNOTATE);
    }
}

/* This routine writes the via model calls of cell.
 * (argument 'bfp' is box stream)
 */
static void write_vias (LAYOUTPTR lay, DM_STREAM *fp, DM_STREAM *bfp)
{
    int wrong_via, viacount;
    WIREPTR via;
    TBOXPTR tbox;
    long viaindex;

    viacount = 0;
    wrong_via = FALSE;

    for (via = lay->wire; via; via = via->next)
    {
	if (via->layer < 100) continue;  /* not a via */

	if (via->layer >= 200) continue;  /* dummy mask */

	viaindex = via->layer - 100;

	if (viaindex == 0)
	{ /* via to core (not between metal layers */
	    viaindex = ViaIndex[to_core (via->crd[L], X)][to_core (via->crd[B], Y)][0];
	    if (viaindex < 0)
	    { /* does not exist */
		wrong_via = TRUE; continue;
	    }
	}
	else
	{ /* via between metal layers */
	    viaindex--;
	    if (viaindex >= Chip_num_layer - 1)
	    {
		wrong_via = TRUE; continue;
	    }
	    if (ViaIndex[to_core (via->crd[L], X)][to_core (via->crd[B], Y)][viaindex+1] == -1)
	    {
		wrong_via = TRUE; // continue;
	    }
	}

	if (ViaCellImported[viaindex] < 0) continue; /* was not found in database */

	sprintf (gmc.inst_name, "via_%d", ++viacount);
	strcpy (gmc.cell_name, ViaCellName[viaindex]);
	gmc.imported = ViaCellImported[viaindex];
	gmc.nx = 0; gmc.ny = 0;

	/* set coordinates of placed via */
	if (OverlayGridMapping[X] == NULL ||
	    via->crd[B]%GridRepitition[Y] < OverlayBounds[Y][L] ||
	    via->crd[B]%GridRepitition[Y] > OverlayBounds[Y][R])
	{ /* ordinary */
	    gmc.bxl = map_to_layout_coord (via->crd[L], X) -
	    ((ViaCellBbx[R][viaindex] - ViaCellBbx[L][viaindex])/2);
	}
	else
	{  /* in overlay area */
	    gmc.bxl = map_to_overlay_layout_coord (via->crd[L], X) -
	    ((ViaCellBbx[R][viaindex] - ViaCellBbx[L][viaindex])/2);
	}

	if (OverlayGridMapping[Y] == NULL ||
	    via->crd[L]%GridRepitition[X] < OverlayBounds[X][L] ||
	    via->crd[L]%GridRepitition[X] > OverlayBounds[X][R])
	{
	    gmc.byb = map_to_layout_coord (via->crd[B], Y) -
	    ((ViaCellBbx[T][viaindex] - ViaCellBbx[B][viaindex])/2);
	}
	else
	{  /* in overlay area */
	    gmc.byb = map_to_overlay_layout_coord (via->crd[B], Y) -
	    ((ViaCellBbx[T][viaindex] - ViaCellBbx[B][viaindex])/2);
	}

	gmc.bxr = gmc.bxl + (ViaCellBbx[R][viaindex] - ViaCellBbx[L][viaindex]);
	gmc.byt = gmc.byb + (ViaCellBbx[T][viaindex] - ViaCellBbx[B][viaindex]);

	gmc.mtx[0] = 1; gmc.mtx[1] = 0; gmc.mtx[2] = gmc.bxl - ViaCellBbx[L][viaindex];
	gmc.mtx[3] = 0; gmc.mtx[4] = 1; gmc.mtx[5] = gmc.byb - ViaCellBbx[B][viaindex];

	MIN_UPDATE (bxl, gmc.bxl);
	MIN_UPDATE (byb, gmc.byb);
	MAX_UPDATE (bxr, gmc.bxr);
	MAX_UPDATE (byt, gmc.byt);

	if (!Flat_vias) { /* vias as model call */
	    dmPutDesignData (fp, GEO_MC);
	}
	else { /* vias as boxes */

	    for (tbox = ViaBox[viaindex]; tbox; tbox = tbox->next)
	    {
		gbox.layer_no = tbox->layer_no;
		gbox.xl = tbox->xl + gmc.mtx[2]; gbox.xr = tbox->xr + gmc.mtx[2];
		gbox.yb = tbox->yb + gmc.mtx[5]; gbox.yt = tbox->yt + gmc.mtx[5];
		gbox.bxl = tbox->bxl + gmc.mtx[2]; gbox.bxr = tbox->bxr + gmc.mtx[2];
		gbox.byb = tbox->byb + gmc.mtx[5]; gbox.byt = tbox->byt + gmc.mtx[5];
		gbox.dx = tbox->dx; gbox.nx = tbox->nx;
		gbox.dy = tbox->dy; gbox.ny = tbox->ny;

		dmPutDesignData (bfp, GEO_BOX);
	    }
	}
    }

    if (wrong_via == TRUE) {
	fprintf (stderr, "WARNING: (write_vias)\n");
	fprintf (stderr, "         Some vias in an illegal layer or at an illegal position.\n");
    }
}

/* This routine is called to make an empty array of the image.
 */
void write_empty_image (char *cell_name, long xsize, long ysize)
{
    DM_PROJECT *remote_projectkey;
    DM_CELL *cell_key;
    DM_STREAM *fp;
    char *remote_cellname;
    long bbx[4];
    int  imported;

    /*** 1: look for image ***/

    /* look for this basic image cell */
    if ((imported = exist_cell (ImageName, layout_str)) < 0)
    {
	fprintf (stderr, "ERROR: cannot find elementary core image/template cell '%s' in database\n", ImageName);
	return;
    }

    /* open it: key for project */
    if (!(remote_projectkey = dmFindProjKey (imported, ImageName, projectkey, &remote_cellname, layout_str)))
    {
	error (ERROR, "cannot find nasty project key");
	return;
    }

    /* open it */
    if (!(cell_key = dmCheckOut (remote_projectkey, remote_cellname, ACTUAL, DONTCARE, layout_str, READONLY)))
    {
	fprintf (stderr, "ERROR: cannot open core cell '%s' in database\n", ImageName);
	return;
    }

    /* read bounding box of image */
    if (!(fp = dmOpenStream (cell_key, "info", "r")))
	error (FATAL_ERROR, "write_image");

    if (dmGetDesignData (fp, GEO_INFO) == -1)
	error (FATAL_ERROR, "write_image");

    bbx[L] = ginfo.bxl;
    bbx[R] = ginfo.bxr;
    bbx[B] = ginfo.byb;
    bbx[T] = ginfo.byt;

    /* terminate */
    dmCloseStream (fp, COMPLETE);
    dmCheckIn (cell_key, COMPLETE);

    /*** 2: make the actual cell ***/

    /* test: is the name too long? */
    if (strlen (cell_name) >= DM_MAXNAME)
    { /* print warning */
	fprintf (stderr, "WARNING (write_empty_image): cell name %s too long, truncated\n", cell_name);
	cell_name[DM_MAXNAME - 1] = '\0';
    }

    if (verbose) {
	printf ("\n------ writing empty array %ldx%ld into cell '%s' ------\n", xsize, ysize, cell_name);
	fflush (stdout);
    }

    /* open a new model file, called cell_name */
    if (!(cell_key = dmCheckOut (projectkey, cell_name, DERIVED, DONTCARE, layout_str, UPDATE)))
    {
	error (ERROR, "Unable to open cell (cell not written)");
	return;
    }

    bxl = byb =  BIGNUMBER;
    bxr = byt = -BIGNUMBER;

    /* write mc */
    if (!(fp = dmOpenStream (cell_key, "mc", "w"))) error (FATAL_ERROR, "write_empty_image");

    strcpy (gmc.inst_name, "IMAGE");
    strcpy (gmc.cell_name, ImageName);
    gmc.imported = imported;
    gmc.mtx[0] = 1; gmc.mtx[1] = 0; gmc.mtx[2] = 0;
    gmc.mtx[3] = 0; gmc.mtx[4] = 1; gmc.mtx[5] = 0;

    gmc.nx = xsize - 1;
    gmc.ny = ysize - 1;
    gmc.dx = LayoutRepitition[X];
    gmc.dy = LayoutRepitition[Y];

    gmc.bxl = bbx[L];
    gmc.byb = bbx[B];
    gmc.bxr = bbx[R] + (gmc.nx * LayoutRepitition[X]);
    gmc.byt = bbx[T] + (gmc.ny * LayoutRepitition[Y]);

    bxl = gmc.bxl;
    bxr = gmc.bxr;
    byb = gmc.byb;
    byt = gmc.byt;

    dmPutDesignData (fp, GEO_MC);
    dmCloseStream (fp, COMPLETE);

    /* write empty term */
    if (!(fp = dmOpenStream (cell_key, "term", "w"))) error (FATAL_ERROR, "write_empty_image");
    dmCloseStream (fp, COMPLETE);

    /* write empty box */
    if (!(fp = dmOpenStream (cell_key, "box", "w"))) error (FATAL_ERROR, "write_empty_image");
    dmCloseStream (fp, COMPLETE);

    /* write bbox info */
    if (!(fp = dmOpenStream (cell_key, "info", "w"))) error (FATAL_ERROR, "write_empty_image");
    ginfo.bxl = bxl; ginfo.bxr = bxr;
    ginfo.byb = byb; ginfo.byt = byt;
    dmPutDesignData (fp, GEO_INFO);
    dmCloseStream (fp, COMPLETE);

    /* write empty nor */
    if (!(fp = dmOpenStream (cell_key, "nor", "w"))) error (FATAL_ERROR, "write_empty_image");
    dmCloseStream (fp, COMPLETE);

    dmCheckIn (cell_key, COMPLETE);
}
