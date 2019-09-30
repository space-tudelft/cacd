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
 * Read nelsis cells recursively.
 */

#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/nelsea/prototypes.h"
#include <ctype.h>

static void read_cell (char *, char *, int);

/*
 * imported variables
 */
extern char
   *this_sdf_lib,
   *layout_str,
   *primitive_str,
   *circuit_str,
   *in_core_str,
   *written_str;

extern int verbose;
extern int Hierarchical_fish;

extern DM_PROJECT *projectkey;
extern char **ViaCellName; /* array with cell names of these vias. size: NumViaName */
extern long NumViaName;
extern char ImageName[DM_MAXNAME + 1]; /* NELSIS name of image to be repeated */

extern MAPTABLEPTR maptable;

/* This routine reads all local cells in view 'view', from the maptable.
 */
void readallnelsiscellsinmaptable (char *view)
{
MAPTABLEPTR map;

for (map = maptable; map; map = map->next)
   {
   if (map->view != view) continue;

   if (map->nelseastatus != written_str &&
      map->nelseastatus != primitive_str &&
      map->internalstatus != in_core_str)
      {
      read_cell (view, map->cell, FALSE);
      }
   }
}

/* This routine reads all local cells in view 'view', from the celllist.
 */
void readallnelsiscellsincelllist (char *view)
{
char **cell_array;   /* array of cell names */
int i;
MAPTABLEPTR map;

if (!(cell_array = (char **) dmGetMetaDesignData (CELLLIST, projectkey, view)))
   {
   fprintf (stderr, "WARNING: no nelsis cell names found (%s celllist empty)\n", view);
   return;
   }

/*
 * read all cells
 */
for (i = 0; cell_array[i]; i++)
   {
   map = look_up_map (view, cell_array[i]);
   if ((map->nelsis_time && map->seadif_time) &&
      map->seadif_time >= map->nelsis_time)
      {
      /* no reason to read */
      }
   else
      read_cell (view, cell_array[i], FALSE);
   }
}

/* This front-end just calls read_cell.
 */
MAPTABLEPTR read_nelsis_cell (char *view, /* layout or circuit */
	char *cell_name, int fish) /* TRUE to fish: no read of son cells */
{
    MAPTABLEPTR map;

    /* read it */
    map = look_up_map (view, cell_name);

    map->overrule_status = TRUE;

    /* just read */
    read_cell (view, cell_name, fish);

    return (map);
}

/* This routine perfroms the recusrion on a nelsis cell.
 * Its mc is openend, to check whether all childs are
 * already read in. If the child is not read in,
 * it will be read in recursively by the same routine.
 * As soon as all children are read in, the actual read process will start.
 */
static void read_cell (char *view, /* layout or circuit */
	char *cell_name, int nonrecursive) /* True for fish: no recursive read */
{
    char *remote_cellname;
    DM_PROJECT *remote_projectkey;
    DM_CELL *cell_key;
    DM_STREAM *fp;
    char child_cell_name[DM_MAXNAME + 1];
    MAPTABLEPTR map, child_map;
    int imported;
    int reopened;

    if (!cell_name || strlen (cell_name) == 0) {
	fprintf (stderr, "WARNING: (read_cell): invalid null name\n");
	return;
    }

    /*
     * 1: look for this cell in the map table.
     * If the cell does not exists in the table this routine
     * will create an entry.
     */
    map = look_up_map (view, cell_name);

    /*
     * Do not read if:
     */

    /* 1: already in core or primitive */
    if (map->nelseastatus == primitive_str ||
    map->internalstatus == in_core_str) return;

    /* 2: already tried but failed */
    if (map->num_read_attempts >= 1) return;

    /* mark the attempt to read */
    map->num_read_attempts++;

    /* does the cell exist? */
    if ((imported = exist_cell (cell_name, view)) < 0)
    { /* it does not exist */
	fprintf (stderr, "ERROR: %s-cell '%s' was not found in nelsis database.\n", view, cell_name);
	map->internalstatus = canonicstring ("error_cell_not_found");
	map->nelseastatus = canonicstring ("not_found");
	return;
    }

    /*
     * Do not read it if it is imported!!
     */
    if (imported == IMPORTED)
    {
	if (map->overrule_status == TRUE)
	{ /* first time: print error */
	    fprintf (stderr, "ERROR: you cannot fish/convert imported cells.\n");
	    fprintf (stderr, "       %s-cell '%s' looks pretty imported to me!\n", map->view, map->cell);
	    return;
	}

	if (Hierarchical_fish || nonrecursive) return;

	if (map->nelseastatus == written_str)
	{
	    if ((map->nelsis_time && map->seadif_time) &&
		 map->seadif_time < map->nelsis_time)
	    {
		printf ("WARNING: Imported nelsis cell '%s' is younger than its corresponding\n", cell_name);
		printf ("         seadif cell %s(%s(%s(%s))). The seadif\n",
		map->layout, map->circuit, map->function, map->library);
		printf ("         library '%s' doesn't seem to be up to date?\n", map->library);
		fflush (stdout);
	    }
	    return;
	}

	if (map->nelsis_time && map->seadif_time && map->seadif_time < map->nelsis_time)
	{
	    printf ("WARNING: Imported nelsis cell '%s' is younger than its corresponding\n", map->cell);
	    printf ("         seadif cell %s(%s(%s(%s))) (%s-view). The seadif\n",
	    map->layout, map->circuit,
	    map->function, map->library,
	    view);
	    printf ("         library '%s' doesn't seem to be up to date.\n", map->library);
	    fflush (stdout);
	    return;
	}

	fprintf (stderr, "ERROR: there is no seadif equivalent of your imported nelsis\n");
	fprintf (stderr, "       instance '%s' (in %s-view).\n", map->cell, map->view);
	fprintf (stderr, "       Hint: do a nelsea on that cell in its project directory\n");
	return;
    }

    /* get key for project */
    if (!(remote_projectkey = dmFindProjKey (imported, cell_name, projectkey, &remote_cellname, view)))
    {  /* ? */
	fprintf (stderr, "ERROR: cannot get nasty project key for cell '%s'\n", cell_name);
	map->internalstatus = canonicstring ("error_cannot_read");
	map->nelseastatus = canonicstring ("error_cannot_read");
	return;
    }

    /* open cell */
    if (!(cell_key = dmCheckOut (remote_projectkey, remote_cellname, ACTUAL, DONTCARE, view, READONLY)))
    {  /* ? */
	fprintf (stderr, "ERROR: cannot open cell '%s' in database\n", cell_name);
	map->internalstatus = canonicstring ("error_cannot_read");
	map->nelseastatus = canonicstring ("error_cannot_read");
	return;
    }

    /* open mc */
    if (!(fp = dmOpenStream (cell_key, "mc", "r"))) {
	fprintf (stderr, "ERROR: cannot open mc of %s-cell '%s'\n", view, cell_name);
	dmCheckIn (cell_key, COMPLETE);
	map->internalstatus = canonicstring ("error_cannot_read");
	map->nelseastatus = canonicstring ("error_cannot_read");
	return;
    }

    /* hack to make it work in release 3 and 4 */
#define give_format(v) ((v) == layout_str ? (GEO_MC) : (CIR_MC))

    /* read mc's
     * THIS IS THE RECURSION
     */
    while (dmGetDesignData (fp, give_format (view)) > 0)
    {
	reopened = 0;

	/*
	 * filter special cells
	 */
	if (view == layout_str)
	{
	    int z;

	    /* is it the image? */
	    if (strcmp (gmc.cell_name, ImageName) == 0) goto next_mc;

	    /* is it a via ?? */
	    for (z = 0; z < NumViaName; ++z)
		if (ViaCellName[z] && strcmp (gmc.cell_name, ViaCellName[z]) == 0) break;
	    if (z < NumViaName) goto next_mc; /* its a via */
	}
	else
	{ /* circuit view: ignore nenh and penh and cap */
	    if (strcmp (cmc.cell_name, "nenh") == 0) goto next_mc;
	    if (strcmp (cmc.cell_name, "penh") == 0) goto next_mc;
	    if (strcmp (cmc.cell_name, "cap" ) == 0) goto next_mc;
	    if (strcmp (cmc.cell_name, "res" ) == 0) goto next_mc;

	    if (cmc.inst_attribute && strncmp (cmc.inst_attribute, "f;", 2) == 0)
	    { /* it is such a nasty functional cell!!
		try to auto-convert it into a non-functional cell... */
		fprintf (stderr, "ERROR: reference to functional cell '%s' not allowed\n", cmc.cell_name);
		goto next_mc;
	    }
	}

	/*
	 * look in map table: already there?
	 */

	/* save cell name */
	if (view == layout_str)
	    strcpy (child_cell_name, gmc.cell_name);
	else
	    strcpy (child_cell_name, cmc.cell_name);

	child_map = look_up_map (view, child_cell_name);

	/*
	 * do not read sub cell if:
	 */

	/* 1: already in core or primitive */
	if (child_map->nelseastatus == primitive_str ||
	    child_map->internalstatus == in_core_str) goto next_mc;

	/* 2: already tried but failed.. */
	if (child_map->num_read_attempts >= 1) goto next_mc;

	/* 3: if we are fishing nonrecursively */
	if (nonrecursive && !Hierarchical_fish)
	{
	    /* give warning if son cell is younger... */
	    if (map->nelsis_time && child_map->nelsis_time && child_map->nelsis_time > map->nelsis_time)
	    {
		printf ("WARNING: Son cell '%s' is younger than parent '%s'\n", child_map->cell, map->cell);
		printf ("         You might have to (re-)fish son cell '%s'\n", child_map->cell);
		fflush (stdout);
	    }
	    child_map->num_read_attempts = 2;  /* disable furter reading */
	    goto next_mc;  /* do not read */
	}

	/* 4: if it exists in seadif, and sdf time is younger */
	if (!Hierarchical_fish)
	{
	    if (child_map->nelsis_time && child_map->seadif_time)
	    {
		if (child_map->seadif_time >= child_map->nelsis_time)
		goto next_mc; /* still up to date */
		if (child_map->library != this_sdf_lib)
		{ /* the child is not local */
#if 0
		    printf ("WARNING: Imported nelsis son-cell '%s' is younger than its corresponding\n",
			child_map->cell);
		    printf ("         seadif cell %s(%s(%s(%s))) (%s-view). The seadif\n",
			child_map->layout, child_map->circuit,
			child_map->function, child_map->library, view);
		    printf ("         library '%s' doesn't seem to be up to date.\n", child_map->library);
		    fflush (stdout);
		    goto next_mc;
#endif
		}
	    }
	}

	/*
	 * apparently it does not exist in seadif: read it
	 */

	/* close the cell */
	dmCloseStream (fp, COMPLETE);
	dmCheckIn (cell_key, COMPLETE);

	/* recursive call */
	read_cell (view, child_cell_name, nonrecursive);

	/* open it again, from the beginning */
	if (!(remote_projectkey = dmFindProjKey (imported, cell_name, projectkey, &remote_cellname, view)))
	{  /* ? */
	    fprintf (stderr, "ERROR: cannot get nasty project key for cell '%s'\n", cell_name);
	    map->internalstatus = canonicstring ("error_cannot_read");
	    map->nelseastatus = canonicstring ("error_cannot_read");
	    return;
	}

	reopened = 1;

	if (!(cell_key = dmCheckOut (remote_projectkey, remote_cellname, ACTUAL, DONTCARE, view, READONLY)))
	{  /* ? */
	    fprintf (stderr, "ERROR: cannot re-open cell '%s' in database\n", cell_name);
	    map->internalstatus = canonicstring ("error_cannot_read");
	    map->nelseastatus = canonicstring ("error_cannot_read");
	    return;
	}

	if (!(fp = dmOpenStream (cell_key, "mc", "r")))
	{
	    fprintf (stderr, "ERROR: cannot re-open mc of cell '%s'\n", cell_name);
	    dmCheckIn (cell_key, COMPLETE);
	    map->internalstatus = canonicstring ("error_cannot_read");
	    map->nelseastatus = canonicstring ("error_cannot_read");
	    return;
	}

next_mc:
	if (!reopened && (view != layout_str))
	if (cmc.inst_attribute) free (cmc.inst_attribute);
    }

    /* close the mc */
    dmCloseStream (fp, COMPLETE);

    /* at this point all children should have been read in
     * perform the actual read action.
     */
    if (verbose) {
	printf ("------ reading nelsis %s-cell '%s' ------\n", view, cell_name);
	fflush (stdout);
    }

    if (view == layout_str)
	read_layout_cell (cell_name, cell_key, map);
    else
	read_circuit_cell (cell_name, cell_key, map);

    dmCheckIn (cell_key, COMPLETE);
}
