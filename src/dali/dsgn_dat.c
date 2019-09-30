/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	P. van der Wolf
 *	H.T. Fassotte
 *	S. de Graaf
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

#include "src/dali/header.h"

extern struct drc **drc_data;
extern DM_PROJECT *dmproject;
extern int NR_lay;
extern int fmode;

/*
** Read in the design rule data for the checker.
*/
void r_design_rules ()
{
    struct drc *new;
    char  line[BUFSIZ];
    char *drc_file = "./dali_drc";
    FILE *pfile;
    int i, masknumber, nr, overlap, gap, exgap, exlength;

    if (drc_data) return;

    /* get absolute path to file "dali_drc" */
    if (!fmode) drc_file = dmGetMetaDesignData (PROCPATH, dmproject, "dali_drc");

    sprintf (line, "Reading drc data file '%s'", drc_file);
    ptext (line);
    sleep (1);

    if (!(pfile = fopen (drc_file, "r"))) {
	fprintf (stderr, "WARNING: cannot open file '%s'!\n", drc_file);
	/*
	sprintf (line, "warning: Can't open file '%s'!", drc_file);
	ptext (line);
	sleep (2);
	*/
	return;
    }

    nr = 0;
    while (fgets (line, BUFSIZ, pfile)) {

	++nr;
	if (*line == '#') continue; /* skip comment line */

	if (sscanf (line, "%d%d%d%d%d", &masknumber, &overlap, &gap, &exgap, &exlength) != 5) {
	    sprintf (line, "Illegal line (%d) in drc_file! (line skipped)", nr);
	    ptext (line);
	    sleep (1);
	    continue; /* skip line */
	}

	if (masknumber >= 0 && masknumber < NR_lay) {
	    if (!drc_data) {
		MALLOCN (drc_data, struct drc *, NR_lay);
		for (i = 0; i < NR_lay; ++i) drc_data[i] = NULL;
	    }
	    MALLOC (new, struct drc);
	    drc_data[masknumber] = new;
	    drc_data[masknumber] -> overlap  = overlap  * QUAD_LAMBDA;
	    drc_data[masknumber] -> gap      = gap      * QUAD_LAMBDA;
	    drc_data[masknumber] -> exgap    = exgap    * QUAD_LAMBDA;
	    drc_data[masknumber] -> exlength = exlength * QUAD_LAMBDA;
	}
	else {
	    sprintf (line, "Illegal layer number (%d) in drc_file!", masknumber);
	    ptext (line);
	    sleep (1);
	}
    }
    fclose (pfile);
}
