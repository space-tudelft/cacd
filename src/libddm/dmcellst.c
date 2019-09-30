/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van der Hoeven
 *	P. van der Wolf
 *	P. Bingley
 *	T.G.R. van Leuken
 *	T. Vogel
 *	F. Beeftink
 *	M. Grueter
 *	E.F. Matthijssen
 *	G.W. Sloof
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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

#include "src/libddm/dmstd.h"

/*
** This functions reads the kernel tables and allocates
** some memory in which the celllist for
** the specified view is written.
** A pointer to the area is returned.
*/

char ** _dmCelllist (DM_PROJECT *dmproject, char *view)
{
    FILE * fp;
    register char **clp;
    char   buf[DM_MAXNAME + 10];
    char   path[MAXLINE];
    int	   cnt, i, view_entry;

    if ((view_entry = _dmValidView (view)) == -1) return (NULL);

    if (_dmCh_project (dmproject) != 0) { dmError ("project key"); return (NULL); }

    if ((clp = dmproject -> celllist[view_entry]) != NULL) {
	/* a (possibly inconsistent) celllist is already present: free it */
        while (*clp != NULL) _dmStrFree (*clp++);
	free (dmproject -> celllist[view_entry]);
    	dmproject -> celllist[view_entry] = NULL;
    }

    _dmSprintf (path, "%s/%s/celllist", dmproject -> dmpath, view);

    if (!(fp = fopen (path, "r"))) {
	dmerrno = DME_NOCELLL; dmError ("_dmCelllist"); return (NULL);
    }

    i = 0;
    while (fscanf (fp, "%s", buf) != EOF) ++i;

    if (!(clp = (char **) malloc ((unsigned) ((i + 1) * sizeof (char *))))) goto mem_err;

    cnt = i;
    i = 0;
    rewind (fp);
    while (fscanf (fp, "%s", buf) != EOF) {
	if (!(clp[i] = _dmStrSave (buf))) goto mem_err;
	++i;
    }
    if (i != cnt) _dmFatal ("_dmCelllist: read error", "", "");
    clp[i] = NULL;
    fclose (fp);

    dmproject -> celllist[view_entry] = clp;

#ifdef DM_DEBUG
    IFDEBUG {
	fprintf (stderr, "_dmCelllist:\n");
	clp = dmproject -> celllist[view_entry];
	while (*clp != NULL) fprintf (stderr, "%s\n", *clp++);
    }
#endif /* DM_DEBUG */

    return (dmproject -> celllist[view_entry]);

mem_err:
    dmerrno = DME_CORE;
    dmError ("_dmCelllist");
    return (NULL);
}
