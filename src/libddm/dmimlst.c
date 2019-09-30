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

IMPCELL ** _dmImportedCelllist (DM_PROJECT *dmproject, char *view)
{
    IMPCELL **iclp;
    FILE    *fp;
    char    *b, *s;
    char    path[3*MAXLINE];
    int	    cnt, i, view_entry;

    if ((view_entry = _dmValidView (view)) == -1) return (NULL);

    if (_dmCh_project (dmproject) != 0) { dmError ("project key"); return (NULL); }

    /* a (possibly inconsistent) impcelllist is already present: free it */
    _dmFreeImportedCelllist (dmproject, view_entry);

    _dmSprintf (path, "%s/%s/impcelllist", dmproject -> dmpath, view);

    if (!(fp = fopen (path, "r"))) {
	dmerrno = DME_NOIMPCL; dmError ("_dmImportedCelllist"); return (NULL);
    }

    i = 0;
    while (fgets (path, 3*MAXLINE, fp)) ++i;

    if (!(iclp = (IMPCELL **) malloc ((unsigned)((i + 1) * sizeof (IMPCELL *))))) goto mem_err;

    cnt = i;
    i = 0;
    rewind (fp);
    while (fgets (path, 3*MAXLINE, fp)) { /* format: alias, cellname, project-path */
	if (!(iclp[i] = (IMPCELL *) malloc (sizeof (IMPCELL)))) goto mem_err;
	b = path;
	if (!(s = strchr (b, ' '))) goto read_err;
	*s = 0;
	if (!(iclp[i] -> alias = _dmStrSave (b))) goto mem_err;
	b = s + 1;
	if (!(s = strchr (b, ' '))) goto read_err;
	*s = 0;
	if (!(iclp[i] -> cellname = _dmStrSave (b))) goto mem_err;
	b = s + 1;
	if (!(s = strchr (b, '\n'))) goto read_err;
	*s = 0;
	if (!(iclp[i] -> dmpath = _dmStrSave (dmSubstituteEnvironmentVars(b)))) goto mem_err;
	++i;
    }
    if (i != cnt) _dmFatal ("_dmImportedCelllist: read error", "", "");
    iclp[i] = NULL;
    fclose (fp);

    dmproject -> impcelllist[view_entry] = iclp;

#ifdef DM_DEBUG
    IFDEBUG {
	fprintf (stderr, "_dmImportedCelllist:\n");
	iclp = dmproject -> impcelllist[view_entry];
	for (; *iclp != NULL; ++iclp) {
	    fprintf (stderr, "%s %s %s\n", (*iclp) -> alias, (*iclp) -> cellname, (*iclp) -> dmpath);
	}
    }
#endif /* DM_DEBUG */

    return (dmproject -> impcelllist[view_entry]);

read_err:
    dmerrno = DME_FMT;
    dmError ("_dmImportedCelllist");
    return (NULL);
mem_err:
    dmerrno = DME_CORE;
    dmError ("_dmImportedCelllist");
    return (NULL);
}

void _dmFreeImportedCelllist (DM_PROJECT *dmproject, int view_entry)
{
    register IMPCELL **iclp;

    if ((iclp = dmproject -> impcelllist[view_entry])) {
	while (*iclp) {
	    _dmStrFree ((*iclp) -> cellname);
	    _dmStrFree ((*iclp) -> alias);
	    _dmStrFree ((*iclp) -> dmpath);
	    free (*iclp++);
	}
	free (dmproject -> impcelllist[view_entry]);
	dmproject -> impcelllist[view_entry] = NULL;
    }
}
