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

#ifndef _PROTOTYPES_H
#define _PROTOTYPES_H

/*
 * prototypes.h
 */

#ifdef __cplusplus
// must keep this inside #ifdef __cplusplus...:

#include "src/ocean/nelsea/seadifGraph.h"

extern "C" {
#endif

#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/nelsea/grid.h"

/*
 * This routine creates a grid of size (xsize, ysize, zsize)
 * index: z, y, x, in that order;
 * It will return a pointer to an initialized array
 */
COREUNIT *** new_grid (GRIDADRESSUNIT xsize, GRIDADRESSUNIT ysize, GRIDADRESSUNIT zsize);

/*
 * this routine frees a 3-dim grid
 */
void free_grid (COREUNIT ***grid);

void strNcpy (char *dest, char *src, int n);

/*
 * This routine is called by ydparse to allocate all arrays which
 * are related to the number of layers
 */
void allocate_layer_arrays (long num_layer);

/*
 * This routine is called by ydparse to allocate all arrays and
 * mulitdimensional arrays which are related or dependent on
 * the size of the core image.
 * The dimensions of the core image are store in the globals
 * GridRepitition[X] and GridRepitition[Y]
 */
void allocate_core_image (void);

/*
 * This routine (called from parse) removes a via location
 */
int add_grid_block (long ax, long ay, long az,  /* point 1 */
                    long bx, long by, long bz); /* point 2 */

void error   (int errortype, char *string);
void myfatal (int errortype);

/*
 * This routine initializes all global variables
 * and allocates the end pointers
 */
void init_variables (void);

/*
 * this routine sets all global variables which should be set by main
 */
void initialize_globals (void);

/*
 * this routine opens the design rules file and cals the parser
 */
void read_image_file (void);

/*
 * this routine writes a seadif file
 */
void write_seadif_file (void);

/*
 * This routine looks for a nelsis cell in the table
 * It it exists, a pointer to the entry will be returned.
 * if it does not exits, an entry will be created.
 */
MAPTABLEPTR look_up_map (char *view, char *cell_name);

/*
 * this routine returns a pointer to the seadif map
 */
MAPTABLEPTR look_up_seadif_map (char *view, char *lib, char *func, char *cir, char *lay);

/*
 * this routine returns a pointer to the seadif map,
 * but does not create a new seadif cell (canonicstringed names)
 */
MAPTABLEPTR find_seadif_map (char *view, char *lib, char *func, char *cir, char *lay);

/*
 * This routine attaches the cell denoted by 'map' into the
 * library structure.
 * in all cases, the pointers in the map structure
 * will be set the the corresponding lbrary, function, circuit and layout.
 * If the cell (or its library, function, etc.) does not exists it will be created.
 */
void attach_map_to_lib (MAPTABLEPTR map);

/*
 * this routine checks for multiple occurences in the mapfile
 * mapfile
 */
void check_multiple_mapfile (int do_print /* print the errors */);

/*
 * this routine checks for the presence of the seadif cells in the mapfile
 * seadif should be open
 */
void check_nelsea_mapfile (int do_print,   /* print the errors */
                          int do_update,  /* do the update if necessary */
                          char *view);

/*
 * this routine checks for the presence of the nelsis cells in the mapfile
 * nelsis should be open
 */
void check_seanel_mapfile (int do_print,   /* print the errors */
                          int do_update,  /* do the update if necessary */
                          char *view);

/*
 * this routine is used when nelsea is used for routing from dali
 */
int daliconvert (char *cell_name);

void read_map_table (char *filename); /* map file name */

/*
 * this routine writes the maptable in a file
 */
void write_map_table (char *filename); /* map file name */

void init_nelsis (char *progname, int readonly, int do_read_image);

char *RemToLocPath (char *rem_lib);
char *LocToRemPath (char **hostname, char *loc_path);

/*
 * This routine closes it again
 */
void close_nelsis (void);

/*
 * This help routine searches the database for
 * a layout cell called cell_name.
 * It will return -1 was found not found.
 * it will return 0 (local) or 1 (imported) it is was found
 */
int exist_cell (char *cell_name, char *view);

/*
 * This routine reads all local cells in view 'view', from the maptable
 */
void readallnelsiscellsinmaptable (char *view);

/*
 * read all cells in the library
 */
void readallseadifcellsinlibrary (char *view);

/*
 * This routine reads all local cells in view 'view', from the celllist.
 */
void readallnelsiscellsincelllist (char *view);

/*
 * this front-end just calls read_cell
 */
MAPTABLEPTR read_nelsis_cell (char *view,    /* layout or circuit */
                              char *cell_name,
                              int fish      /* TRUE to fish: no read of son cells */
);

/*
 * This routine reads a nelsis circuit description into
 * a seadif datastructure.
 */
int read_circuit_cell (char *cell_name,
                       DM_CELL *cell_key, /* key of already openend cell */
                       MAPTABLEPTR map);

/*
 * This routine attaches indices to 'name', accoriding to
 * the rank-numbers r0 and r1 (indices).
 * The resulting strings will be stored in the string manager.
 * example, if name is 'name':
 *  r0    r1      resulting name
 *
 *  -1    X       name
 *   4    -1      name[4]
 */
char *add_index_to_name (char *name, long r0, long r1);

/*
 * This routine reads a nelsis circuit description into
 * a seadif datastructure.
 */
void read_layout_cell (char *cell_name, DM_CELL *cell_key, MAPTABLEPTR map);

/*
 * This routine reads all local cells in view 'view', from the maptable
 */
void readallseadifcellsinmaptable (char *view);

/*
 * This front-end just calls read_cell.
 * It will return a MAPTABLEPTR to the new cell
 * (argument 'cell_name' is nelsis cell name).
 */
MAPTABLEPTR read_seadif_cell (char *view, char *cell_name, int non_hierarchical);

/*
 * this routine writes all circuits in the current datastruct
 */
void write_nelsis_circuit (void);

/*
 * This routine writes all layouts in the current datastruct
 * (argument 'image_call' is TRUE to add model call to image;
 *  xsize, ysize is size of image (valid if > 0 and if image_call is TRUE)).
 */
void write_nelsis_layout (int image_call, int recursive, long xsize, long ysize);

/*
 * This routine is called to make an empty array of the image
 */
void write_empty_image (char *cell_name, long xsize, long ysize);

/*
 * This routine scans the maptable for cells which
 * need to be written to seadif.
 */
void write_to_seadif (void);

void add_nelsis_primitives (void);

char * bname (char *);

int open_seadif (int read_only, int print_env);
int all_input_is_ok (char *circuit_name, char *layout_name, char *sdf_layout_name);

int runprog (char *prog, char *outfile, int *exitstatus, ...);
int runprogsea (char *prog, char *outfile, int *exitstatus, ...);
int runprognowait (char *prog, char *outfile, int *exitstatus, ...);
int DaliRun (char *prog, char *outfile, ...);
int autopsy_on_sea_child (void);
int looks_like_the_sea_child_died (int no_ignore_kill);
int kill_the_sea_child (void);
int signal_trout_to_stop (void);

MAPTABLEPTR make_map (char *view, char *cell_name,
		     char *func_name, char *cir_name, char *lay_name);
void xfilealert (int type, char *fname);

void powerFix (CIRCUITPTR circuit);
int isPowerNet (NETPTR);
int isGroundNet (NETPTR);

long map_lambda_to_grid_coord (long, int);

void tin_insert (CIRPORTPTR term, CIRINSTPTR inst, NETPTR net);
NETPTR tin_lookup (CIRPORTPTR term, CIRINSTPTR inst);
void tin_cleanup (int n);
void tin_statistics (void);

void csi_insert (CIRCUITPTR, STRING, CIRINSTPTR);
CIRINSTPTR csi_lookup (CIRCUITPTR, STRING);
void csi_cleanup (int n);
void csi_statistics (void);

#ifdef __cplusplus
}

// must keep this inside #ifdef __cplusplus...:
int  instanceMustBeDeleted (grCirInst *gcinst);
int  removeSerPar (grCircuit *gcirc);
void ghoti (grCircuit *, int, int);
#endif

/* the parser has C linkage !!! */
#ifdef __cplusplus
extern "C" int ydparse (void);
#else
int ydparse (void);
#endif

void ghotiSetUpDeletePolicy (int checkGhoti);
int getPrimitiveElements (void);

#endif
