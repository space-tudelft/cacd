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
 * Write datastruct into seadif.
 */
#include <time.h> /* prototypes the time() syscall */

#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/libseadif/sealibio.h"
#include "src/ocean/nelsea/prototypes.h"

/*
 * imported functions (from read_seadif)
 */
char *myhostname (void);
void strip_domain_of_hostname (char *hostname);

/*
 * imported variables
 */
extern char
   *this_sdf_lib,
   *primitive_str,
   *layout_str,
   *circuit_str,
   *in_core_str,
   *written_str;

extern int
   NoAliases,             /* FALSE to use seadif aliases for mapping */
   No_sdf_write,
   verbose;

extern MAPTABLEPTR maptable;

/*
 * This routine scans the maptable for cells which
 * need to be written to seadif.
 */
void write_to_seadif ()
{
    MAPTABLEPTR map;
    int must_write;
    char *hn, *wd, hostname[40], filepath[256], *rem_path;

    if (No_sdf_write) return;

    must_write = FALSE;

    for (map = maptable; map; map = map->next)
    {
	if (map->nelseastatus != written_str &&
	    map->internalstatus == in_core_str &&
	    map->nelseastatus != primitive_str)
	{ /* must be written */
	    must_write = TRUE;
	}
    }

    if (must_write == FALSE) {
	// fprintf (stderr, "WARNING: nothing written into seadif\n");
	return;
    }

    for (map = maptable; map; map = map->next)
    {
	if (map->nelseastatus != written_str &&
	    map->internalstatus == in_core_str &&
	    map->nelseastatus != primitive_str)
	{ /* must be written */

	    if (map->view == layout_str && map->layoutstruct)
	    { /* layout view */
		if (verbose) {
		    printf ("------ writing nelsis '%s' as sdflay '%s(%s(%s(%s)))' ------\n",
			map->cell, map->layout, map->circuit, map->function, map->library);
		    fflush (stdout);
		}

		/* now add the the alias from the maptable */
		if (!NoAliases) {
		    if (!sdfmakelayalias (map->cell, map->layout, map->circuit, map->function, map->library))
			error (WARNING, "Cannot make alias");
		}
		sdfwritelay (SDFLAYALL, map->layoutstruct);
	    }

	    if (map->view == circuit_str && map->circuitstruct)
	    { /* layout view */
		if (verbose) {
		    printf ("------ writing nelsis '%s' as sdfcir '%s(%s(%s)) ------\n",
			map->cell, map->circuit, map->function, map->library);
		    fflush (stdout);
		}

		/* now add the the alias from the maptable */
		if (!NoAliases) {
		    if (!sdfmakeciralias (map->cell, map->circuit, map->function, map->library))
			error (WARNING, "Cannot make alias");
		}

		/* perform the actual write */
		sdfwritecir (SDFCIRALL, map->circuitstruct);
	    }

	    map->nelseastatus = written_str;  /* mark write .... */
	    /* set time */
	    map->seadif_time = time (NULL);
	}
    }

    /* OK, now add library alias */
    if (!sdfreadlib (SDFLIBALL, this_sdf_lib)) {
	error (WARNING, "Cannot read lib");
	return;
    }

    /* Now set the library alias in the form
     * form: 'hostname:dir'. hostname points to
     * the filesystem of the cell, and dir is the path
     * in that local file system
     */
    wd = sdfgetcwd();
    *hostname = 0;
    hn = hostname;

    if (!(rem_path = LocToRemPath (&hn, wd))) {
	fprintf (stderr, "WARNING: cannot get remote path of project '%s'\n", wd);
	/* in that case: just add it manually and hope for the best */
	sprintf (filepath, "%s:%s", myhostname(), wd);
    }
    else {
	strip_domain_of_hostname (hn);
	sprintf (filepath, "%s:%s", hn, rem_path);
    }
    rem_path = cs (filepath);

    if (!sdfmakelibalias (rem_path, this_sdf_lib)) {
	error (WARNING, "Cannot make lib alias");
	return;
    }

    /* and write the bloody thing again */
    if (!sdfwritelib (SDFLIBALL, thislib)) {
	error (WARNING, "Cannot write lib");
	return;
    }
}
