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
** _dmCellIsRoot checks if a cell with name 'cellname' and with
** viewtype 'view', being an element in 'dmproject', is a
** root-cell. Therefor it reads all mc_files belonging to cells
** in 'celllist_array'.
** If cellname exists in a mc_file : no root -> return (0)
** else cell with name 'cellname' is root -> return (1)
** Hard error: return (-1)
*/
int _dmCellIsRoot (DM_PROJECT *dmproject, char *cellname, char *viewtype)
{
    FILE   *celllist_fp;
    char  **celllist_array;
    char    celllist_member[DM_MAXNAME + 1];
    char    path[MAXLINE];
    int     i, nr_cells_inlist, cell_is_root = -1;

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "_dmCellIsRoot: cell: %s, view: %s\n", cellname, viewtype);
#endif

    if (_dmCh_project (dmproject) != 0) { dmError ("_dmCellIsRoot: project key"); return (-1); }

    _dmSprintf (path, "%s/%s/celllist", dmproject -> dmpath, viewtype);

    if (!(celllist_fp = fopen (path, "r"))) {
	dmerrno = DME_NOCELLL; dmError2 ("_dmCellIsRoot", path); return (-1);
    }

    i = 0;
    while (fscanf (celllist_fp, "%s", celllist_member) != EOF) ++i;

    if (!(celllist_array = (char **) malloc ((unsigned) ((i + 1) * sizeof (char *))))) {
	dmerrno = DME_CORE; dmError ("_dmCellIsRoot"); return (-1);
    }

    nr_cells_inlist = i;
    i = 0;
    rewind (celllist_fp);
    while (fscanf (celllist_fp, "%s", celllist_member) != EOF) {
	if (!(celllist_array[i] = _dmStrSave (celllist_member))) {
	    dmerrno = DME_CORE; dmError ("_dmCellIsRoot"); nr_cells_inlist = i; goto err_ret;
	}
	++i;
    }
    if (i != nr_cells_inlist) _dmFatal ("_dmCellIsRoot: read error", "", "");
    celllist_array[i] = NULL;

    fclose (celllist_fp);

    cell_is_root = _dmCellIsRoot2 (dmproject, cellname, viewtype, celllist_array);

err_ret:
    for (i = 0; i < nr_cells_inlist; ++i) _dmStrFree (celllist_array[i]);
    free (celllist_array);
    return (cell_is_root);
}

int _dmCellIsRoot2 (DM_PROJECT *dmproject, char *cellname, char *viewtype, char **celllist_array)
{
    DM_STREAM  xxx;
    DM_STREAM *dsp = &xxx;
    DM_CELL    yyy;
    int     found;
    int     fmt;
    char   *cell_name;
    char    path[MAXLINE];
    char    path2[MAXLINE];
    struct stat buf;
    register char **clp;

    /* This is necessary because dmGetDesignData accesses dsp -> dmkey -> dmproject */
    dsp -> dmkey = &yyy;
    dsp -> dmkey -> dmproject = dmproject;

         if (strcmp (viewtype, CIRCUIT)   == 0) { fmt = CIR_MC; cell_name = cmc.cell_name; }
    else if (strcmp (viewtype, LAYOUT)    == 0) { fmt = GEO_MC; cell_name = gmc.cell_name; }
    else if (strcmp (viewtype, FLOORPLAN) == 0) { fmt = FLP_MC; cell_name = fmc.cell_name; }
    else { dmerrno = DME_BADVIEW; dmError ("_dmCellIsRoot"); return (-1); }

    found = 0;

    for (clp = celllist_array; *clp != NULL; ++clp) {

	if (!found && strcmp (cellname, *clp) == 0) { ++found; continue; }

	_dmSprintf (path, "%s/%s/%s/mc", dmproject -> dmpath, viewtype, *clp);

	if (fmt == CIR_MC) {
	    if (stat (path, &buf) != 0) {
		/* cannot stat mc-file:
		** If circuit cell is function block,
		** no mc-file needs to exist.  (SdeG)
		*/
		_dmSprintf (path2, "%s/%s/%s/fterm", dmproject -> dmpath, viewtype, *clp);
		if (stat (path2, &buf) == 0) continue; /* ok, function block exists, skip this cell */
	    }
	}

	if (!(dsp -> dmfp = fopen (path, "r"))) {
	    dmerrno = DME_FOPEN; dmError2 ("_dmCellIsRoot", path); return (-1);
	}
	while (dmGetDesignData (dsp, fmt) > 0) {
	    if (strcmp (cellname, cell_name) == 0) {
		fclose (dsp -> dmfp);
		return (0); /* no root */
	    }
	}
	fclose (dsp -> dmfp);
    }

    if (!found) { dmerrno = DME_NOCELL; dmError2 ("_dmCellIsRoot", cellname); return (-1); }
    return (1); /* cell is root */
}
