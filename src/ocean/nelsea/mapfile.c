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
 * Manages mapping file for nelsis to seadif (v.v.) conversion.
 */

#include <time.h>
#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/nelsea/prototypes.h"
#include "src/ocean/libseadif/sea_decl.h"

static int tabto (int, int, char *, FILE *);

/*
 * imported
 */
extern char
   *this_sdf_lib,
   *default_sdf_lib,
   *primitive_str,
   *written_str,
   *dummy_str,
   *not_written_str,
   *in_core_str,
   *not_in_core_str,
   *layout_str,           /* string "layout" in string manager */
   *circuit_str;          /* string "circuit" in string manager */

extern MAPTABLEPTR maptable;

extern int verbose;
extern int Seadif_open;
extern long NumViaName; /* number of indices in the arrays ViaCellName and ViaMaskName */

extern char
   ImageName[],            /* NELSIS name of image to be repeated */
   **ViaCellName;          /* array with cell names of these vias. size: NumViaName */

void read_map_table (char *filename /* map file name */)
{
    FILE *fp;
    char scanline[2000], view[200], cell[200], library[200], function[200];
    char circuit[200], layout[200], nelseastatus[200], seanelstatus[200];
    MAPTABLEPTR map, hmap;
    int lineno, num_found;

    /* attempt to open the file */
    if (!(fp = fopen (filename, "r"))) {
	if (!access (filename, F_OK))
	    fprintf (stderr, "WARNING: mapfile '%s' appears to be present, but I'm unable to open it\n", filename);
	/* not found: add basic primitives */
	add_nelsis_primitives();
	return;
    }

    if (verbose) printf ("\n------ reading map file ------\n");

    scanline[1999] = '\0';

    for (lineno = 1; fgets (scanline, 1998, fp); lineno++)
    {
	if (scanline[0] == '#') continue; /* skip comments */

	/* read a mapline */
	if ((num_found = sscanf (scanline, "%s %s %s %s %s %s %s %s",
	    view, cell, library, function, circuit, layout, nelseastatus, seanelstatus)) < 1)
	    continue;  /* skip empty lines silently */

	if (view[0] == '#') continue; /* skip comment signs not on 1st column. */

	/* error checking */
	if (num_found < 2) {
	    fprintf (stderr, "WARNING: line %d of file '%s': to few (%d) items.\n", lineno, filename, num_found);
	    continue;
	}

	/* mape mapfile struct, link it */
	NewMaptable (map);

	/* viewname */
	if (strncmp (view, "layout", 3) == 0)
	    map->view = layout_str;
	else if (strncmp (view, "circuit", 3) == 0)
	    map->view = circuit_str;
	else {
	    fprintf (stderr, "WARNING: file %s, line %d: unknown view name: '%s'\n", filename, lineno, view);
	    map->view = cs (view);
	}

	/* cellname */
	if (cell[0] == '$') {
	    fprintf (stderr, "ERROR: file %s, line %d: cell name cannot be escaped: '%s'\n", filename, lineno, cell);
	    FreeMaptable (map);
	    continue;
	}
	map->cell = cs (cell);

	/* library name */
	if (num_found < 3 || library[0] == '$')
	    /* no lib specified: use default name */
	    map->library = cs (default_sdf_lib);
	else
	    map->library = cs (library);

	/* function name */
	if (num_found < 4 || function[0] == '$') {
	    /* no function specified: use other name */
	    if (num_found >= 5 && circuit[0] != '$')
		/* circuit specified */
		map->function = cs (circuit);
	    else
		/* use cell name */
		map->function = cs (cell);
	}
	else
	    map->function = cs (function);

	/* circuit name */
	if (num_found < 5 || circuit[0] == '$')
	    /* no circuit specified: use cell name */
	    map->circuit = cs (cell);
	else
	    map->circuit = cs (circuit);

	/* layout name */
	if (map->view == circuit_str)
	    /* view is circuit: layoutname = dummy */
	    map->layout = dummy_str;
	else if (num_found < 6 || layout[0] == '$')
	    /* not specified: use cellname */
	    map->layout = cs (cell);
	else
	    map->layout = cs (layout);

	/* nelsea status */
	if (num_found >= 7 && nelseastatus[0] != '$')
	{
	    if (strncmp (nelseastatus, written_str, 3) == 0)
		map->nelseastatus = written_str;
	    else if (strncmp (nelseastatus, primitive_str, 3) == 0)
		map->nelseastatus = primitive_str;
	    else if (strlen (nelseastatus) > 0)
		map->nelseastatus = cs (nelseastatus);
	    else
		map->nelseastatus = not_written_str;
	}
	else
	    map->nelseastatus = not_written_str;

	/* seanel status */
	if (num_found >= 8 && seanelstatus[0] != '$')
	{
	    if (strncmp (seanelstatus, written_str, 3) == 0)
		map->seanelstatus = written_str;
	    else if (strncmp (seanelstatus, primitive_str, 3) == 0)
		map->seanelstatus = primitive_str;
	    else if (strlen (seanelstatus) > 0)
		map->seanelstatus = cs (seanelstatus);
	    else
		map->seanelstatus = not_written_str;
	}
	else
	    map->seanelstatus = not_written_str;

	map->internalstatus = not_in_core_str;
	map->num_read_attempts = 0;
	map->overrule_status = FALSE; /* to overrule nelsea/seanelstatus: if true always write/read */

	/* link in maplist */
	map->next = maptable;
	maptable = map;
    }

    fclose (fp);

    /* reverse the list */
    map = NULL;
    while (maptable)
    {
	hmap = maptable;
	maptable = maptable->next;
	hmap->next = map;
	map = hmap;
    }

    maptable = map;

    /* do simple check: mulitiple occurences */
    check_multiple_mapfile (TRUE);
}

/* This routine writes the maptable in a file.
 */
void write_map_table (char *filename)
{
    FILE *fp;
    MAPTABLEPTR map;
    time_t the_time; /* time(); */
    int column;

    if (!(fp = fopen (filename, "w"))) {
	if (!access (filename, F_OK))
	    fprintf (stderr, "WARNING: mapfile '%s' appears to be present, but it's not writable,\n", filename);
	else
	    fprintf (stderr, "WARNING: cannot write mapfile '%s' in this directory,\n", filename);
	fprintf (stderr, "         No mapfile written!\n");
	return;
    }

    if (!maptable) add_nelsis_primitives ();

    /*
     * print header
     */
    the_time = time (0);
    fprintf (fp, "# This is a maptable for nelsis <--> seadif conversion, generated/maintained by 'nelsea'\n");
    fprintf (fp, "# Written on: %s", asctime (localtime (&the_time)));
    fprintf (fp, "# You can edit this file to control the mapping process\n");
    fprintf (fp, "# Each line contains the name of a nelsis cell and its corresponding seadif cell\n");
    fprintf (fp, "# The status field may contain the following values:\n");
    fprintf (fp, "#    'written'       the cell was successfully written into seadif resp. nelsis\n");
    fprintf (fp, "#    'primitive'     the cell is a primitive which should not be read or converted\n");
    fprintf (fp, "#    [anything else] the cell will be written if necessary\n");
    fprintf (fp, "# To check the consistency of this table: use 'nelsea -C'\n");
    fprintf (fp, "#    N E L S I S     |                   S E A D I F                     | nelsis->sdf sdf->nelsis\n");
    fprintf (fp, "# view    cellname   | library      function     circuit      layout     |   status      status\n");
    fprintf (fp, "#--------------------+---------------------------------------------------+------------------------\n");

    for (map = maptable; map; map = map->next)
    {
	column = 1;

	/* view */
	fprintf (fp, " %s ", map->view);

	column = tabto (9, column, map->view, fp);

	/* cellname */
	fprintf (fp, "%s ", map->cell);
	column = tabto (13, column, map->cell, fp);

	/* library name */
	fprintf (fp, "%s ", map->library);
	column = tabto (13, column, map->library, fp);

	/* function name */
	fprintf (fp, "%s ", map->function);
	column = tabto (13, column, map->function, fp);

	/* circuit name */
	fprintf (fp, "%s ", map->circuit);
	column = tabto (13, column, map->circuit, fp);

	/* layout name */
	fprintf (fp, "%s ", map->layout);
	column = tabto (13, column, map->layout, fp);

	/* nelseastatus */
	fprintf (fp, "%s", map->nelseastatus);
	column = tabto (13, column, map->nelseastatus, fp);

	/* seanelstatus */
	fprintf (fp, "%s\n", map->seanelstatus);
    }

    fclose (fp);
}

/*
 * prints filling spaces
 */
static int tabto (int colwidth, int cur_column, char *wordstring, FILE *fp)
{
    int i;

    i = cur_column + strlen (wordstring) + 1;

    for (; i < cur_column + colwidth; i++) fprintf (fp, " ");

    return (i);
}

#define PRIM_LIB "primitives"
    /* official name of primitives library */

static void getPrimitiveIntoCore (char *name, int lib_found)
{
    MAPTABLEPTR map = look_up_seadif_map (circuit_str, PRIM_LIB, name, name, name);
    if (lib_found) {
	if (map->seanelstatus != primitive_str && map->seadif_time == 0)
	    fprintf (stderr, "WARNING: primitive '%s' is missing in primitive library\n", map->circuit);
	else /* actually we need everything from the primitives (Cirports, etc): */
	    sdfreadcir (SDFCIRALL, map->circuit, map->function, map->library);
    }
    map->nelseastatus = map->seanelstatus = primitive_str;
}

/* This routine add the default nelsis primitives to the maptable,
 * only, if the maptable is empty.
 */
void add_nelsis_primitives ()
{
    int lib_found = 0;

    if (strcmp (this_sdf_lib, PRIM_LIB) == 0) return;

    if (Seadif_open) {
	if (!existslib (cs (PRIM_LIB))) {
	    fprintf (stderr, "WARNING: seadif library for primitives is missing!\n");
	    fprintf (stderr, "         Check whether you have imported the project '%s'\n", PRIM_LIB);
	}
	else lib_found = 1;
    }

    getPrimitiveIntoCore ("nenh", lib_found);
    getPrimitiveIntoCore ("penh", lib_found);
    getPrimitiveIntoCore ("res" , lib_found);
    getPrimitiveIntoCore ("cap" , lib_found);

#if 0
    if (strlen (ImageName) > 0)
    {
	map = look_up_seadif_map (layout_str, PRIM_LIB, ImageName, ImageName, ImageName);
	map->nelseastatus = map->seanelstatus = primitive_str;
    }
    for (i = 0; i < NumViaName; i++)
    {
	if (ViaCellName[i] == 0 || strlen (ViaCellName[i]) == 0) continue;
	map = look_up_seadif_map (layout_str, PRIM_LIB, ViaCellName[i], ViaCellName[i], ViaCellName[i]);
	map->nelseastatus = map->seanelstatus = primitive_str;
    }
#endif
}
