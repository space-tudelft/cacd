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
 * Check everything before placing or routing.
 */
#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/nelsea/prototypes.h"
#include "src/ocean/libseadif/sdferrors.h"
#include <sys/stat.h>

extern char
   *this_sdf_lib,
   *layout_str,
   *circuit_str,
   *in_core_str,
   *written_str,
   *not_written_str;

extern CIRCUITPTR thiscir;
extern int verbose;

static int check_circuit (char *circuit_name);
static int check_layout (char *circuit_name, char *layout_name, char *sdf_layout_name);
static LAYINSTPTR find_corr_lay_inst (SLICEPTR slice, CIRINSTPTR cirinst, int desperate);
static int investigate_flag (SLICEPTR slice, int print_missing);

/* Program string, to indicate that the cell was checked before and OK */
# define CHECKED "sea; checked and OK"

/* This routine is checking the consistency of the circuit cell.
 * The tests consist of two parts:
 *  1) the circuit of the circuit (everything is OK for the placer)
 *  2) the layout of the circuit  (everything is OK for the router)
 * If layout_name != null, we will only do the first part.
 */
int all_input_is_ok (char *circuit_name, char *layout_name, char *sdf_layout_name)
{
    int status;

    /* check circuit */
    if ((status = check_circuit (circuit_name))) return (status);

    /* only check layout if a layout name is given */
    if (!layout_name || strlen (layout_name) == 0) return (status);

    if (!sdf_layout_name || strlen (sdf_layout_name) == 0) sdf_layout_name = layout_name;

    return (check_layout (circuit_name, layout_name, sdf_layout_name));
}

/* This routine is checking the consistency of the circuit cell.
 * The tests consist of two parts:
 *  1) the circuit of the circuit (everything is OK for the placer)
 *  2) the layout of the circuit  (everything is OK for the router)
 * If layout_name != null, we will only do the first part.
 */
static int check_circuit (char * circuit_name)
{
    MAPTABLEPTR cirmap, child_map;
    int num_cirinst, num_error;
    CIRINSTPTR cirinst;

    if (verbose) {
	printf ("------ checking your circuit '%s' ------\n", circuit_name);
	fflush (stdout);
    }

    /*
     * have a look at the circuit cell...
     */
    cirmap = look_up_map (circuit_str, circuit_name);

    if (!cirmap->nelsis_time) {
	fprintf (stderr, "ERROR: Circuit cell '%s' does not exist in nelsis database.\n", circuit_name);
	fprintf (stderr, "Hint:  write your circuit in the database.\n");
	return (1);
    }

    if (cirmap->library != this_sdf_lib)
    {
	fprintf (stderr, "ERROR: Circuit cell '%s' is imported.\n", circuit_name);
	fprintf (stderr, "       You can only place and route cells in the local project\n");
	return (1);
    }

    /*
     * is seadif circuit cell still up-to-date??
     */
    if (cirmap->seadif_time >= cirmap->nelsis_time &&
	cirmap->circuitstruct &&
	cirmap->circuitstruct->status &&
	strncmp (cirmap->circuitstruct->status->program, CHECKED, strlen (CHECKED)-1) == 0)
    { /* appears to be up to date: read only instances and terminals from seadif */
	int what;
	what = SDFCIRPORT;   /* read ports.. */
	what |= SDFCIRINST;  /* and read instances */
	what |= SDFCIRSTAT;  /* what the hell: read status again to be sure */
	if (sdfreadcir (what, cirmap->circuit, cirmap->function, cirmap->library))
	{ /* OK, it was checked previously */
	    cirmap->circuitstruct = thiscir;

	    /* we have to be really sure that not something was changed */
	    /* therefore we look at the times of all the children */
	    for (cirinst = cirmap->circuitstruct->cirinst; cirinst; cirinst = cirinst->next)
	    {
		if (!cirinst->circuit)
		{
		    fprintf (stderr, "ERROR: No sdf circuit for instance '%s' in cell '%s'\n",
			cirinst->name, cirmap->circuit);
		    continue;
		}

		/* check circuit of son */
		child_map = look_up_map (circuit_str, cirinst->circuit->name);
		if ((child_map->nelsis_time && child_map->seadif_time) &&
		    child_map->seadif_time >= child_map->nelsis_time)
		    /* nothing */; /* OK */
		else
		    break;    /* not present or up-to-date */

		/* check layout of son */
		child_map = look_up_map (layout_str, cirinst->circuit->name);
		if ((child_map->nelsis_time && child_map->seadif_time) &&
		    child_map->seadif_time >= child_map->nelsis_time)
		    continue; /* OK */
		else
		    break;    /* not present or up-to-date */
	    }

	    if (!cirinst)
	    { /* looped successfully: seems up-to-date */
		if (verbose) {
		    printf ("------ circuit '%s' is still up-to-date ------\n", circuit_name);
		    fflush (stdout);
		}
		return (0);
	    }
	    /* else: just read the ordinary way... */
	    /* to prevent problems in seadif: erase the existing
	    instance and port list */
	    cirmap->circuitstruct->cirport = NULL;
	    cirmap->circuitstruct->cirinst = NULL;
	    cirmap->circuitstruct = NULL;
	}
	/* else: just read the ordinary way... */
    }

    /* (recursively) read the circuit cell */
    read_nelsis_cell (circuit_str, circuit_name, FALSE);

    if (cirmap->internalstatus != in_core_str)
    { /* doesn't exist or problems.... */
	fprintf (stderr, "ERROR: Cannot read nelsis circuit cell '%s' into core.\n", circuit_name);
	return (1);
    }

    /* do not write the cell if it turned out to be up-to-date... */
    if (cirmap->seadif_time >= cirmap->nelsis_time)
    { /* no need to write it away again ... */
	cirmap->nelseastatus = written_str;
    }

    /*
     * check if for each circuit instance the corresponding
     * layout exists in nelsis and in seadif
     */
    for (num_error = 0, num_cirinst = 0,
	cirinst = cirmap->circuitstruct->cirinst; cirinst; cirinst = cirinst->next)
    {
	num_cirinst++;
	if (!cirinst->circuit) {
	fprintf (stderr, "ERROR: No sdf circuit for instance '%s' in cell '%s'\n",
	cirinst->name, cirmap->circuit);
	num_error++;
	}

	child_map = look_up_map (layout_str, cirinst->circuit->name);

	/* is it read during the previous read_nelsis_cell to be converted? */
	if (child_map->nelseastatus == in_core_str) continue;

	/* does corresponding nelsis layout cell exists? */
	if (!child_map->nelsis_time) { /* no */
	    fprintf (stderr, "ERROR: Your son-circuit '%s' has no corresponding layout\n", child_map->cell);
	    num_error++;
	    continue;
	}

	/* just check consistency of seadif tree
	if (child_map->layoutstruct->circuit != cirinst->circuit)
	{
	    fprintf (stderr, "ERROR: (internal) layout of son cell '%s' inconsistent with its circuit.\n", child_map->cell);
	    num_error++;
	    continue;
	}
	*/

	/* is the sdf layout son up to date ? */
	if (child_map->nelsis_time > child_map->seadif_time)
	{ /* no: it is not up to date... */
	    if (child_map->library != this_sdf_lib)
	    { /* remote... */
		printf ("WARNING: Imported nelsis soncell '%s' is younger than its corresponding\n", child_map->cell);
		printf ("         seadif cell %s(%s(%s(%s))). Are you sure that the seadif\n",
		child_map->layout, child_map->circuit,
		child_map->function, child_map->library);
		printf ("         library '%s' is up to date?\n", child_map->library);
		fflush (stdout);
	    }
	    else
	    {
		/* (recursively) read the layout son-cell */
		read_nelsis_cell (layout_str, child_map->cell, FALSE);
		if (child_map->internalstatus != in_core_str)
		{ /* doesn't exist or problems.... */
		    fprintf (stderr, "ERROR: Cannot read nelsis son layout cell '%s' into core.\n",
			child_map->cell);
		    num_error++;
		}
	    }
	}
    }

    if (num_error > 0) {
	fprintf (stderr, "ERROR: %d error(s) have occurred during checking the %d\n", num_error, num_cirinst);
	fprintf (stderr, "       son-cells of your nelsis circuit cell '%s'.\n", cirmap->cell);
	fprintf (stderr, "       Please check the consistency of your cell.\n");
	return (1);
    }

    /*
     * ready with circuit checks
     */

    /*
     * flag that we are ready by setting the appropriate athor string
     * In this way the next time the circuit will be read we do not
     * have to check all over again.
     */
    if (!cirmap->circuitstruct || !cirmap->circuitstruct->status) {
	error (WARNING, "check_circuit: cannot set new status");
	return (0); /* but it's OK */
    }

    /* set program.. */
    cirmap->circuitstruct->status->program = cs (CHECKED);

    if (cirmap->nelseastatus == written_str)
    { /* cell would not be written with write_seadif: write status ourselves.. */
	if (!sdfwritecir (SDFCIRSTAT, cirmap->circuitstruct))
	    error (WARNING, "check_circuit: write circuit status");
    }

    return (0);
}

/* This routine is checking the consistency of the circuit cell.
 * The tests consist of two parts:
 *  1) the circuit of the circuit (everything is OK for the placer)
 *  2) the layout of the circuit  (everything is OK for the router)
 * If layout_name != null, we will only do the first part.
 */
static int check_layout (char *circuit_name, char *layout_name, char *sdf_layout_name)
{
    MAPTABLEPTR cirmap, laymap;
    int return_status, num_error, num_direct, num_layinst, num_cirinst;
    CIRINSTPTR cirinst;
    CIRPORTPTR first_cirport;
    LAYINSTPTR layinst;

    return_status = 0; /* OK */

    /* get circuit */
    cirmap = look_up_map (circuit_str, circuit_name);

    /* get layout */
    laymap = make_map (layout_str, layout_name, circuit_name, circuit_name, sdf_layout_name);

    /* does it exists? */
    if (!laymap->nelsis_time) {
	fprintf (stderr, "ERROR: Placement layout '%s' for circuit '%s' doesn't exists.\n", laymap->cell, cirmap->cell);
	fprintf (stderr, "Hint:  Before routing circuit '%s' a corresponding layout\n", cirmap->cell);
	fprintf (stderr, "       cell should be made which contains the placement\n");
	return (1);
    }

    /*
     * is seadif layout cell still up-to-date??
     */
    if (laymap->seadif_time >= laymap->nelsis_time &&
	laymap->layoutstruct &&
	laymap->layoutstruct->status &&
	strncmp (laymap->layoutstruct->status->program, CHECKED, strlen (CHECKED)-1) == 0)
    { /* appears to be up to date no checking/conversion required.. */
	if (verbose) {
	    printf ("------ placement layout '%s' is still up-to-date ------\n", layout_name);
	    fflush (stdout);
	}
    /* DISABLED, hardly ever happens, and son-layouts could have changed */
    /* return (0); */
    }
    /* else: just read the ordinary way... */

    if (verbose) {
	printf ("------ checking match with placement layout '%s' ------\n", layout_name);
	fflush (stdout);
    }

    /*
     * it should not be imported...
     */
    if (laymap->library != this_sdf_lib)
    {
	fprintf (stderr, "ERROR: Placement layout '%s' for (local) circuit '%s' is imported.\n",
	    laymap->cell, cirmap->cell);
	fprintf (stderr, "       You can only place and route cells in the local project.\n");
	return (1);
    }

    /* remember first cirport */
    first_cirport = cirmap->circuitstruct->cirport;

    /*
     * next step: (recursively) read the layout
     */
    read_nelsis_cell (layout_str, layout_name, FALSE);

    if (laymap->internalstatus != in_core_str)
    { /* doesn't exist or problems.... */
	fprintf (stderr, "ERROR: Cannot read nelsis (placement) layout cell '%s' into core.\n",
	    circuit_name);
	return (1);
    }

    /*
     * do we have to re-write the circuit, because the cirportlist was changed??
     */
    if (first_cirport != cirmap->circuitstruct->cirport)
    { /* yes */
	cirmap->nelseastatus = in_core_str;
    }

    /*
     * do not write the cell if it turned out to be up-to-date...
     * patrick: added a two-minute overlap to prevent problems
     * on dis-synchronized computers which are linked via NFS-mount
     */
    if (laymap->seadif_time - 120 >= laymap->nelsis_time)
    { /* no need to write it away again ... */
	laymap->nelseastatus = written_str;
    }

    /*
     * check if for each circuit instance the corresponding
     * layout INSTANCE is present in the instancelist of layout.
     * We set flag.l to TRUE if a match is found, both in
     * circuit as in layout.
     */

    /* reset the flag.l in layout */
    num_layinst = investigate_flag (laymap->layoutstruct->slice, FALSE);
    /* step through the circuit instances */
    for (num_error = 0, num_cirinst = 0,
	cirinst = cirmap->circuitstruct->cirinst; cirinst; cirinst = cirinst->next)
    {
	num_cirinst++;

	/* we use flag.l as 'found' flag */
	cirinst->flag.l = FALSE;

	if (find_corr_lay_inst (laymap->layoutstruct->slice, cirinst, FALSE))
	    cirinst->flag.l = TRUE;
	else
	    num_error++;
    }

    /* num_error contains the number of missing layout instances */
    num_direct = num_error;
    /* try a second time: now attaching the 'orphans' to a circuit, if possible */
    for (cirinst = cirmap->circuitstruct->cirinst;
	    cirinst && num_error > 0; cirinst = cirinst->next)
    {
	if (cirinst->flag.l != FALSE) continue; /* already assigned */

	/* find layout instance, with 'desperate' flag */
	if ((layinst = find_corr_lay_inst (laymap->layoutstruct->slice, cirinst, TRUE)))
	{
	    cirinst->flag.l = TRUE;
	    layinst->name = cirinst->name;
	    /* we do not print a message.... */
	    num_error--;
	}
    }

    if (num_error > 0)
    {
	fprintf (stderr, "WARNING: The following %d instance(s) are missing in your layout:\n", num_error);
	for (cirinst = cirmap->circuitstruct->cirinst; cirinst; cirinst = cirinst->next)
	{
	    if (cirinst->flag.l != FALSE) continue; /* already assigned */
	    fprintf (stderr, "    >inst '%s' of cell '%s'\n", cirinst->name, cirinst->circuit->name);
	}
	fprintf (stderr, "Hint: This indicates that your circuit '%s' contains some\n", circuit_name);
	fprintf (stderr, "      instances which could not be found in the layout.\n");
	fprintf (stderr, "      Therefore the routing will certainly be incomplete.\n");
	return_status = SDFERROR_WARNINGS;
    }

    if (num_direct - num_error > 0)
    {
	printf ("For your information:\n");
	printf ("    %d orphan layout instances were assigned to circuit instances.\n",
	    num_direct - num_error);
	printf ("    This means that you specified instances in your layout without \n");
	printf ("    a corresponding name in your circuit '%s'.\n", circuit_name);
	printf ("    Notice that this is a RANDOM assignment, which is not necessarily\n");
	printf ("    optimal. Hint: use the placer for a nice initial placement.\n");
	fflush (stdout);
	return_status = SDFERROR_WARNINGS;
	/* force the writing of this layout cell */
	laymap->nelseastatus = not_written_str;
    }

    /*
     * Are there any unassigned layout instances left over?
     */
    if (num_layinst > num_cirinst - num_error)
    { /* yes */
	printf ("For your information:\n");
	printf ("    The following %d instance(s) in your layout placement cell could\n",
	    num_layinst - num_cirinst + num_error);
	printf ("    not be assigned to a corresponding instance in circuit '%s'.\n",
	    circuit_name);
	printf ("    This could have been your intention. Anyway, notice that these\n");
	printf ("    instances will not be routed.\n");
	investigate_flag (laymap->layoutstruct->slice, TRUE);
	fflush (stdout);
	return_status = SDFERROR_WARNINGS;
    }

    /*
     * ready with checking...
     */

    /*
     * flag that we are ready by setting the appropriate athor string
     * In this way the next time the circuit will be read we do not
     * have to check all over again.
     */
    if (!laymap->layoutstruct || !laymap->layoutstruct->status) {
	error (WARNING, "check_layout: cannot set new status");
	return (0); /* but it's OK */
    }

    /* set program.. */
    if (return_status)
	laymap->layoutstruct->status->program = cs ("sea; checked and nearly OK");
    else
	laymap->layoutstruct->status->program = cs (CHECKED);

    if (laymap->nelseastatus == written_str)
    { /* cell would not be written with write_seadif: write status ourselves.. */
       if (!sdfwritelay (SDFLAYSTAT, laymap->layoutstruct))
	  error (WARNING, "check_layout: write circuit status");
    }

    return (return_status);
}

/* This routine (recursively) steps through the list of instances of the
 * layout, which is indiected by 'slice'.
 * It will return the number of layout instances found.
 * If print_missing == FALSE, the flag.l flag of each instance
 * will be reset.
 * If print_missing == TRUE, all instances will be printed of
 * which the flag is not set (to TRUE)
 */
static int investigate_flag (SLICEPTR slice, int print_missing)
{
    register LAYINSTPTR layinst;
    int num_inst;

    num_inst = 0;
    for ( ; slice; slice = slice->next)
    {
	if (slice->chld_type == SLICE_CHLD)
	{ /* another slice: recursion */
	    num_inst += investigate_flag (slice->chld.slice, print_missing);
	    continue;
	}

	/* child contains instances: step along all instances */
	for (layinst = slice->chld.layinst; layinst; layinst = layinst->next)
	{
	    num_inst++;
	    if (print_missing == FALSE)
		layinst->flag.l = FALSE;
	    else if (layinst->flag.l == FALSE)
		printf ("    >inst '%s' of layout cell '%s'\n", layinst->name, layinst->layout->name);
	}
    }
    return (num_inst);
}

/* This routine returns the pointer to the corresponding
 * layout instance of cirinst. NULL will be returned if
 * nothing was found.
 * If desperate == FALSE, only exact matches will be found,
 * and if desperate == TRUE, it will assign the first still
 * unassigned instance of the appropriate cell
 */
static LAYINSTPTR find_corr_lay_inst (SLICEPTR slice, CIRINSTPTR cirinst, int desperate)
{
    register LAYINSTPTR layinst;

    for ( ; slice; slice = slice->next)
    {
	if (slice->chld_type == SLICE_CHLD)
	{ /* another slice: recursion */
	    if (!(layinst = find_corr_lay_inst (slice->chld.slice, cirinst, desperate)))
		continue;  /* nothing found.. */
	    else
		return (layinst);
	}

	/* child contains instances: step along all instances */
	for (layinst = slice->chld.layinst; layinst; layinst = layinst->next)
	{
	    if (layinst->layout->circuit->name != cirinst->circuit->name)
		continue; /* different circuits */
	    if (layinst->flag.l != FALSE)
		continue; /* already assigned... */
	    if (layinst->name != cirinst->name && desperate == FALSE)
		continue; /* not this instance... */
	    /* FOUND IT!: set flag and return.. */
	    layinst->flag.l = TRUE;
	    return (layinst);
	}
    }
    return (NULL);
}

/*
 * The following routine pops up an xterm window in which fname is displayed
 */
void xfilealert (int type, char *fname)
{
    char *fgcolor, *bgcolor, *displayname;
    struct stat buf;
    int columns, lines, exitstatus, i;
    FILE *fp;
    char geo[100], hstr[100], scanline[2000];

    if (type == 2) { /* warning */
	bgcolor = cs ("white");
	fgcolor = cs ("black");
    }
    else { /* error */
	bgcolor = cs ("black");
	fgcolor = cs ("red");
    }

    if (stat (fname, &buf) == 0) {
	if (buf.st_size == 0) {
	    fprintf (stderr, "WARNING (xfilealert): file '%s' has zero size.\n", fname);
	    return;
	}
    } else {
	fprintf (stderr, "WARNING (xfilealert): file '%s' cannot be accessed.\n", fname);
	return;
    }

    if (!(displayname = getenv ("DISPLAY"))) {
	fprintf (stderr, "WARNING (xfilealert): cannot get DISPLAY environment.\n");
	return;
    }

    /*
     * lock output
     */
    if ((fp = fopen (fname, "a"))) {
	fprintf (fp, "lock\n");
	fclose (fp);
    }

    /*
     * find out the number of lines and columns in the file
     */
    if (!(fp = fopen (fname, "r"))) {
	fprintf (stderr, "WARNING (xfilealert): file '%s' cannot be opened.\n", fname);
    }

    for (lines = 0, columns = 30; fp && fgets (scanline, 1998, fp); lines++) {
	if ((i = strlen (scanline)) > columns) columns = i > 150 ? 150 : i;
    }
    if (lines < 4) lines = 4;
    else if (lines > 45) lines = 45;

    fclose (fp);

    sprintf (geo, "%dx%d", columns, lines);

    if (type == 2)
	sprintf (hstr, "Warning message file '%s'    (hit 'q' to erase this window)", fname);
    else
	sprintf (hstr, "Error message file '%s'    (hit 'q' to erase this window)", fname);

    if (runprognowait ("xterm", "seadif/tmp.out", &exitstatus,
		  "-display", displayname,
		  "-title", hstr,
		  "-n", "alert",
		  "-sl", "500",        /* 500 savelines */
		  "-sb",               /* scrollbar */
		  "-ut",               /* no utmp */
/*		  "-fn", "9x15",          font */
		  "-bg", bgcolor, "-fg", fgcolor,
		  "-geometry", geo,
		  "-e", "seatail", fname, NULL))
	fprintf (stderr, "WARNING: xterm alert failed for file '%s'\n", fname);
}
