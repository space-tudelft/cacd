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
 * Read seadif cells recursively.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/nelsea/prototypes.h"
#include "src/ocean/libseadif/sea_decl.h"
#include "src/ocean/libseadif/sdferrors.h"

static int read_seadif_layout_cell (char*, char*, char*, char*, int);
static void recursive_slice_read (SLICEPTR);
static int read_seadif_circuit_cell (char*, char*, char*, int);
static char *stringsave (char *str);
static void set_sdf_proj (void);
static void set_sdf_environment (int read_only, int print_env);
char *myhostname (void);
void strip_domain_of_hostname (char *hostname);

/*
 * imported variables
 */
extern char
   *Authorstring,
   *this_sdf_lib,
   *layout_str,
   *primitive_str,
   *circuit_str,
   *in_core_str,
   *written_str;

extern LIBRARYPTR thislib;
extern FUNCTIONPTR thisfun;
extern CIRCUITPTR thiscir;
extern LAYOUTPTR thislay;
extern SEADIFPTR seadif; /* the root */
extern int verbose;
extern int Seadif_open, Nelsis_open;
extern MAPTABLEPTR maptable;
extern DM_PROJECT *projectkey;

/* This routine reads all local cells in view 'view', from the maptable.
 */
void readallseadifcellsinmaptable (char *view)
{
    MAPTABLEPTR map;

    for (map = maptable; map; map = map->next)
    {
	if (map->view != view) continue;

	if (map->seanelstatus != written_str &&
	    map->seanelstatus != primitive_str &&
	    map->internalstatus != in_core_str)
	{
	    if (view == circuit_str)
		read_seadif_circuit_cell (map->library, map->function, map->circuit, FALSE);
	    if (view == layout_str)
		read_seadif_layout_cell (map->library, map->function, map->circuit, map->layout, FALSE);
	}
    }
}

/* This front-end just calls read_cell.
 * It will return a MAPTABLEPTR to the new cell.
 * (argument 'cell_name' is nelsis cell name)
 */
MAPTABLEPTR read_seadif_cell (char *view, char *cell_name, int non_hierarchical)
{
    MAPTABLEPTR map;

    map = look_up_map (view, cell_name);

    map->overrule_status = TRUE;      /* read always */

    if (view == layout_str)
	read_seadif_layout_cell (map->library, map->function, map->circuit, map->layout, non_hierarchical);

    if (view == circuit_str)
	read_seadif_circuit_cell (map->library, map->function, map->circuit, non_hierarchical);

    return (map);
}

/* This routine perfroms a recursive read on a seadif layout cell.
 */
static int read_seadif_layout_cell (char *lib, char *func, char *cir, char *lay, int non_hierarchical)
{
    MAPTABLEPTR map;

    if (!lay || strlen (lay) == 0) {
    fprintf (stderr, "WARNING: (read_seadif_layout_cell): invalid null name\n");
    return (FALSE);
    }

    /* 1: look for this cell in the map table.
     * If the cell does not exists in the table this routine
     * will create an entry.
     */
    if (strcmp (lay, "Error_Marker"))
	map = look_up_seadif_map (layout_str, lib, func, cir, lay);
    else { /* use the one we find in nelsis... */
	map = look_up_map (layout_str, lay);
    }

    if (map->seanelstatus == primitive_str ||
	map->internalstatus == in_core_str)
    { /* cell already read in/no need too read */
	return (TRUE);
    }

    /* NO read from remote libraries */
    if (map->library != this_sdf_lib) return (TRUE);

    /* nelsis time is younger? */
    if (map->overrule_status != TRUE &&
	(map->nelsis_time && map->seadif_time) &&
	map->seadif_time <= map->nelsis_time) return (TRUE);

    /* not already a failed read... */
    if (map->num_read_attempts >= 1) return (TRUE);

    /* mark the attempt to read */
    map->num_read_attempts++;

    if (verbose) {
	printf ("------ reading sdf laycell '%s(%s(%s(%s)))': ",
	    map->layout, map->circuit, map->function, map->library);
	fflush (stdout);
    }

    /* mark the attempt to read */
    map->num_read_attempts++;

    /* read the cell */
    if (!existslay (map->layout, map->circuit, map->function, map->library) ||
	!sdfreadlay (SDFLAYALL, map->layout, map->circuit, map->function, map->library))
    { /* error */
	if (verbose) { printf ("FAILED ------\n"); fflush (stdout); }
	map->seanelstatus = canonicstring ("not_found");
	return (FALSE);
    }

    if (verbose) { printf ("OK ------\n"); fflush (stdout); }

    /* attach */
    map->librarystruct = thislib;
    map->functionstruct = thisfun;
    map->circuitstruct = thiscir;
    map->layoutstruct = thislay;

    /* recursion : read the children */
    if (non_hierarchical == FALSE)
    recursive_slice_read (map->layoutstruct->slice);

    /* set flags */
    map->internalstatus = in_core_str;
    return (TRUE);
}

/*
 * recursive help routine to read instances
 */
static void recursive_slice_read (SLICEPTR slice)
{
    LAYINSTPTR inst;

    for (; slice; slice = slice->next)
    {
	if (slice->chld_type == SLICE_CHLD)
	{ /* a slice: recursion */
	    recursive_slice_read (slice->chld.slice);
	    continue;
	}

	/* child contains instances */
	for (inst = slice->chld.layinst; inst; inst = inst->next)
	{
	    read_seadif_layout_cell (inst->layout->circuit->function->library->name,
		inst->layout->circuit->function->name,
		inst->layout->circuit->name,
		inst->layout->name, FALSE);
	}
    }
}

/* This routine perfroms a recursive read on a seadif layout cell.
 */
static int read_seadif_circuit_cell (char *lib, char *func, char *cir, int non_hierarchical)
{
    MAPTABLEPTR map;
    CIRINSTPTR cinst;

    if (!cir || strlen (cir) == 0) {
	fprintf (stderr, "WARNING: (read_seadif_circuit_cell): invalid null name\n");
	return (FALSE);
    }

    /* 1: look for this cell in the map table.
     * If the cell does not exists in the table this routine
     * will create an entry.
     */
    map = look_up_seadif_map (circuit_str, lib, func, cir, cir);

    if (map->seanelstatus == primitive_str ||
	map->internalstatus == in_core_str)
    { /* cell already read in/no need too read */
	return (TRUE);
    }

    /* NO read from remote libraries */
    if (map->library != this_sdf_lib) return (TRUE);

    /* nelsis time is younger? */
    if (map->overrule_status != TRUE &&
	(map->nelsis_time && map->seadif_time) &&
	map->seadif_time < map->nelsis_time) return (TRUE);

    /* not already a failed read... */
    if (map->num_read_attempts >= 1) return (TRUE);

    /* mark the attempt to read */
    map->num_read_attempts++;

    if (verbose) {
	printf ("------ reading sdf circell '%s(%s(%s))': ",
	    map->circuit, map->function, map->library);
	fflush (stdout);
    }

    /* read the cell */
    if (!existscir (map->circuit, map->function, map->library) ||
	!sdfreadcir (SDFCIRALL, map->circuit, map->function, map->library))
    { /* error */
	if (verbose) { printf ("FAILED ------\n"); fflush (stdout); }
	map->seanelstatus = canonicstring ("not_found");
	return (FALSE);
    }

    if (verbose) { printf ("OK ------\n"); fflush (stdout); }

    /* attach */
    map->librarystruct = thislib;
    map->functionstruct = thisfun;
    map->circuitstruct = thiscir;
    map->layoutstruct = NULL;

    /* recursion : read the children */
    if (non_hierarchical == FALSE) {
	for (cinst = map->circuitstruct->cirinst; cinst; cinst = cinst->next)
	{
	    read_seadif_circuit_cell (cinst->circuit->function->library->name,
		       cinst->circuit->function->name,
		       cinst->circuit->name,
		       FALSE);
	}
    }

    /* set flags */
    map->internalstatus = in_core_str;
    return (TRUE);
}

/* This routine reads all seadif cells in 'view'.
 */
void readallseadifcellsinlibrary (char *view)
{
    register FUNCTIONPTR func;
    register CIRCUITPTR cir;
    register LAYOUTPTR lay;
    int what;
    MAPTABLEPTR map;

    if (!sdfreadlib (SDFLIBALL, this_sdf_lib)) {
	error (WARNING, "Cannot read current lib");
	return;
    }

    if (!sdflistfun (SDFFUNBODY, thislib)) {
	error (WARNING, "Cannot read current lib (2)");
	return;
    }

    for (func = thislib->function; func; func = func->next)
    { /* for all functions */
	if (!sdfreadfun (SDFFUNALL, func->name, thislib->name)) {
	    fprintf (stderr, "ERROR: cannot read function '%s' of lib '%s'\n",
		func->name, thislib->name);
	    continue;
	}
	if (!sdflistcir (SDFCIRBODY, thisfun)) {
	    fprintf (stderr, "ERROR: cannot read function '%s' of lib '%s' (2)\n",
		func->name, thislib->name);
	    continue;
	}
	for (cir = thisfun->circuit; cir; cir = cir->next)
	{ /* for all circuits .. */
	    if (view == circuit_str)
	    {
		map = look_up_seadif_map (view,
			  thislib->name, func->name, cir->name, cir->name);
		map->overrule_status = TRUE; /* force writing */
	    }
	    else
	    { /* must read layout */
		what = SDFCIRINST;   /* read inst */
		what |= SDFCIRSTAT;  /* what the hell: read status again to be sure */
		if (!sdfreadcir (what, cir->name, func->name, thislib->name)) {
		    fprintf (stderr, "ERROR: cannot read circuit '%s' of func '%s' of lib '%s'\n",
			cir->name, func->name, thislib->name);
		    continue;
		}
		if (!sdflistlay (SDFLAYBODY, thiscir)) {
		    fprintf (stderr, "ERROR: cannot read circuit '%s' of func '%s' of lib '%s' (2)\n",
			cir->name, func->name, thislib->name);
		    continue;
		}
		for (lay = thiscir->layout; lay; lay = lay->next)
		{ /* for all layouts of this cir */
		    /* SKIP temporary cells */
		    if (strcmp (lay->name, "Tmp_Cell_") == 0)
		    continue;
		    map = look_up_seadif_map (view,
				 thislib->name, func->name, cir->name, lay->name);
		    map->overrule_status = TRUE; /* force writing */
		}
	    }
	}
    }

    /* so now we've filled the maptable: read them */
    readallseadifcellsinmaptable (view);
}

/* This routine is called beforre the seadif library is openend.
 * It sets some global variables to the correct value,
 * and sets the proper default libraries.
 */
int open_seadif (int read_only, int print_env)
{
    int status;

    set_sdf_environment (read_only, print_env);

    if (print_env == TRUE) return (TRUE);

    if (verbose) {
	printf ("------ opening seadif ------\n");
	fflush (stdout);
    }

    /*
     * Now really open seadif
     */

    /* Even if sdfopen() fails, we consider the seadif database to be "open". This
     * is necessary to make error() behave correctly and call sdfexit() in stead of exit().
     * Without this measure, nelsea leaves lock files hanging around!
     */
    Seadif_open = TRUE;
    if ((status = sdfopen()) != SDF_NOERROR)
    {
	if (status == SDFERROR_FILELOCK) {
	    fprintf (stderr, "ERROR: The seadif database is locked by another program.\n");
	    fprintf (stderr, "       Try again later, because only one program at the time\n");
	    fprintf (stderr, "       can access it. If you are sure that nobody else is\n");
	    fprintf (stderr, "       working on the database, you can remove the lockfiles.\n");
	}
	else
	    printf ("ERROR: cannot open seadif database.\n");
	return (status);
    }

    set_sdf_proj();
    return (status);
}

/*
 * This routine sets the proper seadif environemnt variables
 */
static void set_sdf_environment (int read_only, int print_env)
{
    char scanline[513], env[1000], proj[512];
    FILE *fp;
    int count;

    /* check whether we are in a nelsis project directory
     */
    if (access (".dmrc", F_OK)) error (FATAL_ERROR, "You must call this program in a project directory!");

    /* check whether seadif view exists
     */
    if (access ("seadif", F_OK)) { /* seadif directory doesn't exist */
	if (verbose) {
	    printf ("------ making missing seadif directory ------\n");
	    fflush (stdout);
	}
	if (mkdir ("seadif", 0755)) error (WARNING, "Cannot make directory seadif");
    }

    /* Task 1: find out the list of directories with seadif databases
     * This is based on the projlist in nelsis.
     */
    strcpy (env, "SEALIB=./seadif");

    /* open projlist to have a look at it...
     */
    if (!(fp = fopen ("projlist", "r"))) {
	fprintf (stderr, "WARNING: cannot open projlist\n");
    } else {
	while (fp && fgets (scanline, 512, fp)) {
	    if (scanline[0] == '#') continue; /* skip comments */

	    if (sscanf (scanline, "%s", proj) != 1) continue;

	    /* cat it to the env.... */
	    sprintf (env, "%s:%s/seadif", env, proj);
	}
	fclose (fp);
    }

    /*
     * put this new environment
     */
#if 0
    /* 7-1993: leave this stuff for oceanlib to decide... */
    if (putenv (stringsave (env)))
       error (WARNING, "Could not putenv SEALIB");
#endif
    if (putenv (stringsave ("SEALIB=")))
       error (WARNING, "Could not putenv SEALIB");

    /*
     * save environment for future use
     */
    if (!(fp = fopen ("seadif/env", "w"))) {
	fprintf (stderr, "WARNING: Cannot open seadif/env\n");
    }
    else {
	fprintf (fp, "%s\n", env);
	fclose (fp);
    }

    if (print_env == TRUE) {
	printf ("Proper seadif environment variables for this project:\n");
	printf ("setenv SEALIB %s\n", (char *) (strchr (env, '=') + 1));
	if (read_only == TRUE)
	    printf ("setenv SEALIBWRITE ./seadif\n");
	else
	    printf ("setenv SEALIBWRITE\n");
    }

    /*
     * put environment variable SEALIBWRITE
     */
    if (read_only == TRUE)
    { /* for read only: put SEALIBWRITE to a dummy directory */
	if (putenv (cs ("SEALIBWRITE=/tmp")))
	    error (WARNING, "Could not putenv SEALIBWRITE");
    }
    else {
	if (putenv (cs ("SEALIBWRITE=./seadif")))
	    error (WARNING, "Could not putenv SEALIBWRITE");
    }

    /*
     * Also put environment newsealib
     */
    strcpy (env, "seadif/sealib.sdf");
    count = 1;
    while (access (env, F_OK) == 0)
    { /* it exists... */
	/* make a new lib name by adding a number.... */
	sprintf (env, "seadif/sealib%d.sdf", ++count);
    }

    sprintf (scanline, "NEWSEALIB=%s", env);

    if (print_env == TRUE)
	printf ("setenv NEWSEALIB %s\n", env);
    if (putenv (cs (scanline)))
	error (WARNING, "Could not putenv NEWSEALIB");
}

/* Set the current project, find a good name for the
 * seadif project.
 */
static void set_sdf_proj ()
{
    char env[1024+100], hostname[40+1];
    char *hn, *rem_path, *wd, *alias;
    char *rem_path_b;
    int count;

    wd = sdfgetcwd();
    hostname[0] = 0;
    hn = hostname;

    if (!(rem_path = LocToRemPath (&hn, wd))) {
	fprintf (stderr, "WARNING: cannot get remote path of project '%s'\n", wd);
	/* in that case: just add it manually and hope for the best */
	rem_path = wd;
	sprintf (env, "%s:%s", myhostname(), rem_path);
    }
    else {
	strip_domain_of_hostname (hn);
	sprintf (env, "%s:%s", hn, rem_path);
    }
    rem_path_b = bname (rem_path);
    this_sdf_lib = cs (rem_path_b);
    rem_path = cs (env); /* has the form "hostname:dir" */

    /* try to find a lib which has this name */
    count = 1;
    while (sdfexistslib (this_sdf_lib) == TRUE)
    { /* it exists... */
	if ((alias = sdflibalias (this_sdf_lib)))
	{ /* it has an alias */
	    if (alias == rem_path)
		break; /* alias is the same: it is this lib = OK */
	}
	else
	    break; /* hope it's OK, because no further information with missing alias */

	/* in all other cases: make a new lib name by adding a number.... */
	sprintf (env, "%s%d", rem_path_b, ++count);
	this_sdf_lib = cs (env);

	fprintf (stderr, "Warning: A new sealib will be generated: '%s'\n", this_sdf_lib);
	fprintf (stderr, "         This indicates most likeley that the project was renamed.\n");
    }

    /* set default author string */
    sprintf (env, "by user; cpu %s; disk %s", myhostname(), rem_path);
    Authorstring = cs (env);
}

/* This routine gets the hostname, always without the domain.
 */
char *myhostname (void)
{
    static char hostname[40+1];

    if (gethostname (hostname, 40))
	error (FATAL_ERROR, "myhostname: cannot get hostname");

    strip_domain_of_hostname (hostname);

    if (!*hostname) error (FATAL_ERROR, "myhostname: no hostname");

    return hostname;
}

/* This little routine strips the domain extension from the
 * hostname, if it exists. It will modify the string...
 */
void strip_domain_of_hostname (char *hostname)
{
    char *dot;

    if ((dot = strchr (hostname, '.'))) *dot = '\0';
}

/* This routine was added because in some cases
 * cs cannot be used because the string is too long.
 */
static char *stringsave (char *str)
{
    char *p = (char *)malloc (strlen (str) + 2);
    if (!p) error (FATAL_ERROR, "malloc_stringsave");
    strcpy (p, str);
    return (p);
}
