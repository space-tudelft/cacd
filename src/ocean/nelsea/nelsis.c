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
 * Basic routines for the nelsis database.
 */
#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/nelsea/prototypes.h"
#include "src/ocean/nelsea/tbox.h"
#include "src/ocean/libseadif/sdferrors.h"

static void allocate_MaskNo_arrays (void);
static int  find_dm_layer (DM_PROCDATA *, char *);
static void allocate_ViaCellBbx (void);

char *cs (char *);

/*
 * import
 */
extern int
   Flat_vias,              /* TRUE to print the vias as boxes instead of mc's */
   Nelsis_open,            /* flags whether nelsis was opened */
  *ViaCellImported,        /* contains flag whether the Via Cell is imported or not */
   verbose;

extern long
   Chip_num_layer,         /* number of metal layers to be used */
   *ViaMaskNo,             /* contains NELSIS mask numbers of via mask string in array ViaMaskName. size: NumViaName */
   *LayerMaskNo,           /* contains NELSIS mask numbers of layer mask string in array LayerMaskName. size: ChipNumLayer */
   DummyMaskNo,            /* mask no of dummy layer */
   *ViaCellBbx[4],         /* contains the boundingboxes of the vias. size: 4 * NumViaName */
   NumViaName;             /* number of indices in the arrays ViaCellName and ViaMaskName */
                           /* The value of this index is >= Chip_num_layer */
extern TBOXPTR
   *ViaBox;                /* array containing the box structure of each via */

extern char
   *Technologystring,      /* name of the process */
   **LayerMaskName,        /* array with mask names of each layer. size: Chip_num_layer */
   **ViaMaskName,          /* array with mask names of via to layer. size: NumViaName */
   *DummyMaskName,         /* name of dummy mask */
   **ViaCellName;          /* array with cell names of these vias . size: NumViaName */

extern char
   *Authorstring,
   *Technologystring,
   *this_sdf_lib,
   *default_sdf_lib,
   *written_str,
   *primitive_str,
   *not_written_str,
   *layout_str,
   *dummy_str,
   *circuit_str,
   *in_core_str,
   *not_in_core_str;

/*
 * export
 */
extern DM_PROJECT *projectkey;

extern FILE *ErrorMsgFile; /* file where dmError writes its messages */
extern int DmErrorIsFatal; /* FALSE if a call do dmError should return */

static void find_error_marker (int do_install);
static void open_nelsis (char *progname);

/*
 * This routine opens the nelsis database reads image.seadif
 */
void init_nelsis (char *progname, int readonly, int do_read_image)
{
    if (verbose) {
	printf ("------ opening nelsis ------\n");
	fflush (stdout);
    }
    open_nelsis (progname);

    if (do_read_image == TRUE) read_image_file ();

    allocate_MaskNo_arrays ();

    if (readonly == FALSE) {
	allocate_ViaCellBbx ();
	find_error_marker (TRUE);
    }
    else {
	find_error_marker (FALSE);
    }
}

/*
 * This routine opens the NELSIS database
 * and sets the global variable projectkey
 */
static void open_nelsis (char *progname)
{
    if (access (".dmrc", R_OK))
	error (FATAL_ERROR, "You must call this program in a project directory!");
    if (dmInit (progname))
	error (FATAL_ERROR, "cannot dmInit the nelsis database");
    if (!(projectkey = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE))) {
	fprintf (stderr, "\nHint: this program must be called in a project directory.");
	myfatal (SDFERROR_NELSIS_OPENING);
    }
    Nelsis_open = TRUE;
}

/*
 * This routine closes it again
 */
void close_nelsis (void)
{
    Nelsis_open = FALSE;
    dmCloseProject (projectkey, COMPLETE);
    dmQuit ();
}

/*
 * Nelsis error function
 */
void dmError (char *s)
{
    fprintf (stderr, "dmError: ");
    dmPerror (s);
    if (ErrorMsgFile) {
	fprintf (ErrorMsgFile, "ERROR (from nelsis database): ");
	if (s) fprintf (ErrorMsgFile, "%s: ", s);
	fprintf (ErrorMsgFile, "%s\n", dmStrError ());
	fprintf (ErrorMsgFile, "Something is wrong with you or with the nelsis database.\n");
    }
    if (DmErrorIsFatal) myfatal (SDFERROR_NELSIS_INTERNAL);
}

/*
 * This routine allocates and sets the arrays ViaMaskNo and LayerMaskNo.
 * This routine should only be called once, and after the database has
 * been opened and after the designrules have been parsed.
 */
static void allocate_MaskNo_arrays ()
{
    DM_PROCDATA *process;
    int i;

/*
 * allocate ViaMaskNo array,
 * this is an array of length NumViaName
 */
CALLOC (ViaMaskNo, long, NumViaName);
/*
 * init to -1, indicates undefined
 */
for (i = 0; i != NumViaName; i++) ViaMaskNo[i] = -1;

/*
 * allocate LayerMaskNo array
 * This array has length Chip_num_layer
 */
CALLOC (LayerMaskNo, long, Chip_num_layer);
for (i = 0; i != Chip_num_layer; i++) LayerMaskNo[i] = -1;

/*
 * fetch mask data struct
 */
if (!(process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, projectkey)))
   { /* problem */
   error (SDFERROR_NELSIS_INTERNAL, "Mysterious database problem");
   }

/*
 * set the current process
 */
Technologystring = cs (process->pr_name);

/*
 * find mask numbers for all metal layers
 */
for (i = 0; i != Chip_num_layer; i++)
   {
   if (!LayerMaskName[i])
      {
      fprintf (stderr, "WARNING: Mask name of layer %d not defined (WireMaskName statement missing).\n", i);
      continue;
      }

   if ((LayerMaskNo[i] = find_dm_layer (process, LayerMaskName[i])) == -1)
      {
      fprintf (stderr, "WARNING: mask '%s' of layer %d not found in NELSIS maskdata file.\n", LayerMaskName[i], i);
      }
   }

if (DummyMaskName)
   {
   if ((DummyMaskNo = find_dm_layer (process, DummyMaskName)) == -1)
      {
      fprintf (stderr, "WARNING: dummy mask '%s' was not found in NELSIS maskdata file.\n", DummyMaskName);
      }
   }


/*
 * find mask numbers of all vias
 */
for (i = 0; i != NumViaName; i++)
   {
   if (!ViaMaskName[i])
      {
      if (i < Chip_num_layer - 1)
         fprintf (stderr, "WARNING: Via name of via between layers %d and %d not defined (ViaMaskName statement missing).\n", i, i+1);
      continue;
      }

   if ((ViaMaskNo[i] = find_dm_layer (process, ViaMaskName[i])) == -1)
      {
      fprintf (stderr, "WARNING: via mask name '%s' of layer %d not found in NELSIS maskdata file.\n", LayerMaskName[i], i);
      }
   }

}

/*
 * This routine returns the NELSIS database masknumber of the mask.
 * The routine returns -1 if the mask was not found.
 */
static int find_dm_layer (DM_PROCDATA *process, char *mask)
{
    if (*mask) {
	int i = 0;
	for (; i < process->nomasks; ++i)
	    if (process->mask_name[i] && !strcmp (mask, process->mask_name[i])) return (i);
    }
    return (-1);
}

/*
 * This routine allocates and sets the array ViaCellBbx.
 * This array contains the bounding boxes (placed coordinates)
 * of the via cell in the database.
 * The routine expects the database to be open.
 */
static void allocate_ViaCellBbx ()
{
    DM_PROJECT *remote_projectkey;
    DM_CELL *cell_key;
    DM_STREAM *fp;
    TBOXPTR tbox;
    char *remote_cellname;
    int i;

/*
 * allocate the array
 */
CALLOC (ViaCellBbx[L], long, NumViaName);
CALLOC (ViaCellBbx[R], long, NumViaName);
CALLOC (ViaCellBbx[B], long, NumViaName);
CALLOC (ViaCellBbx[T], long, NumViaName);

/*
 * allocate imported array
 */
CALLOC (ViaCellImported, int, NumViaName);

/*
 * if requested: read contents of of via
 */
if (Flat_vias == TRUE)
   {
   CALLOC (ViaBox, TBOXPTR, NumViaName);
   }

for (i = 0; i != NumViaName; i++)
   {

   if (!ViaCellName[i]) continue;

   /*
    * look for this cell
    */
   if ((ViaCellImported[i] = exist_cell (ViaCellName[i], layout_str)) < 0)
      { /* it does not exist */
      fprintf (stderr, "WARNING: cannot find via cell '%s' in database\n", ViaCellName[i]);
      continue;
      }

   /*
    * open it: key for project
    */
   if (!(remote_projectkey = dmFindProjKey (ViaCellImported[i],
					 ViaCellName[i],
					 projectkey,
					 &remote_cellname,
					 layout_str)))
      {  /* ? */
      fprintf (stderr, "ERROR: cannot find nasty project key\n");
      continue;
      }

   /*
    * open it
    */
   if (!(cell_key = dmCheckOut (remote_projectkey, remote_cellname,
       ACTUAL, DONTCARE, layout_str, READONLY)))
      {  /* ? */
      fprintf (stderr, "ERROR: cannot open via cell '%s' in database\n", ViaCellName[i]);
      continue;
      }

   /*
    * read bounding box
    */
   if (!(fp = dmOpenStream (cell_key, "info", "r")))
      {
      fprintf (stderr, "ERROR: cannot open info of via cell '%s'\n", ViaCellName[i]);
      error (FATAL_ERROR, "allocate_ViaCellBbx");
      }

   if (dmGetDesignData (fp, GEO_INFO) == -1)
      error (FATAL_ERROR, "allocate_ViaCellBbx");

   ViaCellBbx[L][i] = ginfo.bxl;
   ViaCellBbx[R][i] = ginfo.bxr;
   ViaCellBbx[B][i] = ginfo.byb;
   ViaCellBbx[T][i] = ginfo.byt;

   /*
    * terminate
    */
   dmCloseStream (fp, COMPLETE);

   /*
    * if requested: read contents of of via
    * into array in viabox
    */
   if (Flat_vias == TRUE)
      {
      ViaBox[i] = NULL;

      if (!(fp = dmOpenStream (cell_key, "box", "r")))
         {
         fprintf (stderr, "ERROR: cannot open box of via cell '%s'\n", ViaCellName[i]);
         error (FATAL_ERROR, "allocate_ViaCellBbx");
         }

      /* read the boxes of the via */
      while (dmGetDesignData (fp, GEO_BOX) > 0)
         {  /* copy it */
         NewTbox (tbox);
         tbox->layer_no = gbox.layer_no;
         tbox->xl = gbox.xl; tbox->xr = gbox.xr;
         tbox->yb = gbox.yb; tbox->yt = gbox.yt;
         tbox->bxl = gbox.bxl; tbox->bxr = gbox.bxr;
         tbox->byb = gbox.byb; tbox->byt = gbox.byt;
         tbox->dx = gbox.dx; tbox->nx = gbox.nx;
         tbox->dy = gbox.dy; tbox->ny = gbox.ny;
         /* link */
         tbox->next = ViaBox[i];
         ViaBox[i] = tbox;
         }

      dmCloseStream (fp, COMPLETE);

      /* check: modell call should be empty! */
      if (!(fp = dmOpenStream (cell_key, "mc", "r")))
         {
         fprintf (stderr, "ERROR: cannot open mc of via cell '%s'\n", ViaCellName[i]);
         }

      if (dmGetDesignData (fp, GEO_MC) > 0)
         {
         fprintf (stderr, "WARNING: MC of via cell '%s' is not empty!\n", ViaCellName[i]);
         }

      dmCloseStream (fp, COMPLETE);
      }

   dmCheckIn (cell_key, COMPLETE);
   }
}

/*
 * This routine finds the error marker.
 * If it does not exist, it will be created.
 */
static void find_error_marker (int do_install)
{
    MAPTABLEPTR map;
    char *marker = cs ("Error_Marker");

/*
 * look for this cell
 */
map = look_up_map (layout_str, marker);

if (do_install == FALSE)
   { /* no error markers: flag  */
   map->layoutstruct = NULL;
   map->internalstatus = NULL;
   map->nelseastatus = map->seanelstatus = primitive_str;
   return;
   }

if (map->nelsis_time)
   { /* it does exist! */
   map->nelseastatus = primitive_str;  /* do not convert.. */
   map->seanelstatus = primitive_str;
   attach_map_to_lib (map);
   return;
   }

/*
 * link it in the lib..
 */
attach_map_to_lib (map);

NewWire (map->layoutstruct->wire);
map->layoutstruct->wire->crd[L] = map->layoutstruct->wire->crd[R] = 0;
map->layoutstruct->wire->crd[B] = map->layoutstruct->wire->crd[T] = 0;
map->layoutstruct->wire->layer = 200;   /* dummy layer no */
map->nelseastatus = primitive_str;    /* do not convert into seadif */
map->seanelstatus = not_written_str;  /* but write into nelsis */
map->internalstatus = in_core_str;
}

/*
 * This routine searches the database for a view cell called cell_name.
 * It will return -1 if not found.
 * It will return 0 (local) or 1 (imported) if found.
 */
int exist_cell (char *cell_name, char *view)
{
    IMPCELL **icl, *ic;

    if (_dmExistCell (projectkey, cell_name, view) == 1)
	return (LOCAL); /* cell exists locally */

    if (!(icl = projectkey->impcelllist[_dmValidView (view)]))
	if (!(icl = (IMPCELL **) dmGetMetaDesignData (IMPORTEDCELLLIST, projectkey, view)))
	    error (FATAL_ERROR, "exist_cell");
    if (icl)
    while ((ic = *icl++))
	if (!strcmp (cell_name, ic->alias)) return (IMPORTED); /* found */

    return (-1); /* not found */
}
