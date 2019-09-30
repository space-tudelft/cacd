/*
 * ISC License
 *
 * Copyright (C) 2000-2018 by
 *	Simon de Graaf
 *	Kees-Jan van der Kolk
 *	Patrick Groeneveld
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/libddm/dmincl.h"
#include "src/ocean/layflat/layflat.h"
#include "src/ocean/layflat/prototypes.h"

static void cleanup_equiv_terms (EQUIV_TERM *equiv_terms);
static void makenewname (char *newname);

/* Term_set is the set of terminals with different names. Each element in the
 * term_set is itself a set of terminals with equivalent names (EQUIV_TERM).
 */
static DIFF_TERM *term_set = NULL;

/* If the name gterm.term_name is not already in the term_set add a
 * new entry for this name to the term_set. Add the struct gterm to
 * the appropriate equivalence set.
 */
void store_term ()
{
    EQUIV_TERM *et;
    DIFF_TERM *dt;

    /* Search set of terminal names to see if it includes the new term */
    for (dt = term_set; dt; dt = dt->next)
	if (strcmp (dt->name, gterm.term_name) == 0) break;
    if (!dt) { /* not found, insert new entry */
	dt = (DIFF_TERM *) malloc (sizeof(DIFF_TERM));
	dt->next = term_set;
	term_set = dt;
	strcpy (dt->name, gterm.term_name);
	dt->equiv = NULL;
    }

    /* At this point, dt->equiv points to the set of terminals with the
     * same name. Add the current term (in gterm) to this equivalence set.
     */
    et = (EQUIV_TERM *) malloc (sizeof(EQUIV_TERM));
    et->term = gterm; /* this is a structure assignment */
    et->next = dt->equiv;
    dt->equiv = et;
}

/* Rename all terminals with the same name to a unique name. */
void cleanup_terms ()
{
    DIFF_TERM *dt;

    for (dt = term_set; dt; dt = dt->next) cleanup_equiv_terms (dt->equiv);
}

#define MAXSUFFIX 20

/* Arrange for all instances of the same terminal to have a unique name. */
static void cleanup_equiv_terms (EQUIV_TERM *equiv_terms)
{
    char term_name[DM_MAXNAME+1], new_name[DM_MAXNAME+MAXSUFFIX+1];
    EQUIV_TERM *et;
    int serial = 0;

    if (!equiv_terms) return;

    /* Set serial to the number of equivalent terminals */
    for (et = equiv_terms; et; et = et->next) ++serial;

    if (serial <= 0) err (5, "layflat: fatal internal error --- corrupt data structure...");

    if (serial > 1)
    { /* two or more terminals with the same name: make unique */
	strcpy (term_name, equiv_terms->term.term_name);
	/* Test to see if the new name won't be too long */
	sprintf (new_name, "%s_%d", term_name, serial);
	if (strlen (new_name) > DM_MAXNAME)
	{ /* Replace the original name by a new "unique" and short name */
	    fprintf (stderr, "layflat WARNING: terminal name \"%s\" too long...\n", term_name);
	    makenewname (term_name);
	    fprintf (stderr, "                 renaming it to \"%s\"\n", term_name);
	}
	/* Now concat the name with a unique serial number for each terminal instance */
	for (serial = 1, et = equiv_terms; et; et = et->next, ++serial)
	    sprintf (et->term.term_name, "%s_%d", term_name, serial);
    }
}

/* Return a short unique string that can be used as the name of a terminal.
 */
static void makenewname (char *newname)
{
    static int count = 100;
    int namecollides, len_newname;
    DIFF_TERM *dt;

    do /* think of a new unique terminal name */
    {
	sprintf (newname, "Layf%3d", count++);
	len_newname = strlen (newname);
	/* check that no other terminal name starts with "newname" */
	for (dt = term_set, namecollides = 0; dt; dt = dt->next)
	    if (namecollides |= (strncmp (dt->name, newname, len_newname) == 0))
		break;
    }
    while (namecollides && count < 999);

    if (count >= 999) err (6, "layflat: cannot think of a unique terminal name...");
}

/* Write all the gterm structures stored in term_set to the database.
 */
void output_terms (DM_CELL *dstkey)
{
    DM_STREAM *dst;
    DIFF_TERM *dt;
    EQUIV_TERM *et;

    if (!(dst = dmOpenStream (dstkey, "term", "a")))
	err (5, "Cannot open term stream for writing !");

    for (dt = term_set; dt; dt = dt->next)
    for (et = dt->equiv; et; et = et->next)
    {
	gterm = et->term; /* structure assignment! */
	if (dmPutDesignData (dst, GEO_TERM)) err (5, "Cannot output term");
    }

    dmCloseStream (dst, COMPLETE);
}
