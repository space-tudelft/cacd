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

typedef struct _father { /* describes info for a father link */
    struct _cell   *cell;
    struct _father *next;
} FATHER;

typedef struct _cell { /* describes info for cell definition */
    char cellname[DM_MAXNAME + 1];
    struct _father *father, *currentfather;
    struct _cell   *next;
} CELL;

static CELL *celllist;
static CELL *listentry[DM_NOVIEWS];

#ifdef __cplusplus
  extern "C" {
#endif
CELL * _FindCell (char *name);
CELL * _MakeCellListElement (void);
void _FreeCellList (void);
void _PrintCellList (void);
int _BuildTree (DM_PROJECT  *project, char *view);
#ifdef __cplusplus
  }
#endif

/*
**  The function _dmFatherCell returns a father of the cell with
**  name 'cellName' in the specified project and view.
**  On the first call a data structure of cells from the (imp-)
**  celllist is built. Repeated calls of this function with the
**  same specified (son)cell will return its next father.
**  If no (more) father(s) exist, a NULL pointer is returned.
**  The following call with the same (son)cell will return its
**  first father, and so on.
**  See also the manual page dmGetMetaDesignData(5ICD),
**  request FATHERCELL.
*/
char * _dmFatherCell (DM_PROJECT *project, char *cellName, char *view)
{
    CELL *fathercell, *tmpc;

    /* test argument project */
    if (_dmCh_project (project) != 0) {
	dmError ("_dmFatherCell");
	return (NULL);
    }

    if (_BuildTree (project, view)) {
	_FreeCellList ();
	return (NULL);
    }

    if (!(tmpc = _FindCell (cellName))) {
	dmerrno = DME_NOCELL;
	dmError2 ("_dmFatherCell", cellName);
        return (NULL);
    }

    if (tmpc -> currentfather) {
	fathercell = tmpc -> currentfather -> cell;
	tmpc -> currentfather = tmpc -> currentfather -> next;
	return (fathercell -> cellname);
    }
    else {
	/* no currentfather, reset the currentfather pointer */
	tmpc -> currentfather = tmpc -> father;
	dmerrno = DME_NOFATH;
	return (NULL);
    }
}

/*
** Check if cell with name 'name' is already in celllist.
*/
CELL * _FindCell (char *name)
{
    register CELL *tmp;

    for (tmp = celllist; tmp; tmp = tmp -> next)
	if (strcmp (tmp -> cellname, name) == 0) break;

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "-- _FindCell(name = %s) %s\n", name, (tmp ? "FOUND" : "NOT FOUND"));
#endif
    return (tmp);
}

CELL * _MakeCellListElement ()
{
    CELL *tmp;
    if (!(tmp = (CELL *) calloc (1, sizeof (CELL)))) {
	dmerrno = DME_CORE;
	dmError ("_dmFatherCell");
	return (NULL);
    }
    tmp -> next = celllist;
    celllist = tmp;
    return (tmp);
}

/* free allocated memory */
void _FreeCellList ()
{
    CELL   *tmpc;
    FATHER *tmpf;

    while (celllist) {
	while (celllist -> father) {
	    tmpf = celllist -> father -> next;
	    free (celllist -> father);
	    celllist -> father = tmpf;
	}
	tmpc = celllist -> next;
	free (celllist);
	celllist = tmpc;
    }
}

#ifdef DM_DEBUG
void _PrintCellList ()
{
    CELL   *tmpc;
    FATHER *tmpf;

    fprintf (stderr, "-- _PrintCellList():\n");
    for (tmpc = celllist; tmpc; tmpc = tmpc -> next) {
	fprintf (stderr, "-- CELL: %s\n", tmpc -> cellname);
	for (tmpf = tmpc -> father; tmpf; tmpf = tmpf -> next) {
	    fprintf (stderr, "-- FATHER: %s\n", tmpf -> cell -> cellname);
	}
    }
}
#endif /* DM_DEBUG */

/*
** Build data structure of cells from (imp-)celllist.
** If data structure of specified view already exists:
** return valid celllist pointer.
*/
int _BuildTree (DM_PROJECT *project, char *view)
{
    struct stat buf;
    DM_STREAM  xxx;
    DM_STREAM *mcstream = &xxx;
    DM_CELL    yyy;
    CELL   *fathercell;
    char    path[MAXLINE];
    char    path2[MAXLINE];
    register CELL     *tmpc;
    register FATHER   *tmpf;
    register char    **celllst;
    register IMPCELL **impcelllst;
    char  **dmcelllst;
    char   *name;
    char   *mccell_name;
    int     mcfmt;
    int     view_entry;

    /* This is necessary because dmGetDesignData accesses
       mcstream -> dmkey -> dmproject */
    mcstream -> dmkey = &yyy;
    mcstream -> dmkey -> dmproject = project;

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "-- _BuildTree(view = %s)\n", view);
#endif

    if ((view_entry = _dmValidView (view)) == -1) {
	return (-1);
    }

    if (listentry[view_entry]) {
	celllist = listentry[view_entry];
	return (0);
    }

    /* Build structure. Get lists with (imp) cell names;
    ** project -> (imp)celllist will not be destroyed
    */
    if (strcmp (view, CIRCUIT) == 0) {
	mcfmt = CIR_MC;
	mccell_name = cmc.cell_name;
    }
    else if (strcmp (view, LAYOUT) == 0) {
	mcfmt = GEO_MC;
	mccell_name = gmc.cell_name;
    }
    else if (strcmp (view, FLOORPLAN) == 0) {
	mcfmt = FLP_MC;
	mccell_name = fmc.cell_name;
    }
    else {
	dmerrno = DME_BADVIEW;
	dmError2 ("_dmFatherCell", view);
	return (-1);
    }

    if (!(celllst = project -> celllist[view_entry])) {
	celllst = (char **)
	    dmGetMetaDesignData (CELLLIST, project, view);
	if (!celllst) return (-1);
    }
    if (!(impcelllst = project -> impcelllist[view_entry])) {
	impcelllst = (IMPCELL **)
	    dmGetMetaDesignData (IMPORTEDCELLLIST, project, view);
	if (!impcelllst) return (-1);
    }

    dmcelllst = celllst;

    for (; *celllst; ++celllst) {
	if (!(tmpc = _MakeCellListElement ())) return (-1);
	strcpy (tmpc -> cellname, *celllst);
    }

    for (; *impcelllst; ++impcelllst) {
	if (!(tmpc = _MakeCellListElement ())) return (-1);
	strcpy (tmpc -> cellname, (*impcelllst) -> alias);
    }

    /* Read for each local cell the mc-file and
    ** make the father linklist(s).
    */
    for (celllst = dmcelllst; (name = *celllst); ++celllst) {

	_dmSprintf (path, "%s/%s/%s/mc", project -> dmpath, view, name);

	if (mcfmt == CIR_MC) {
	    if (stat (path, &buf) != 0) {
		/* Cannot stat mc-file:
		** If circuit cell is function block,
		** no mc-file needs to exist.  (SdeG)
		*/
		_dmSprintf (path2, "%s/%s/%s/fterm", project -> dmpath, view, name);
		if (stat (path2, &buf) == 0) {
		    /* ok, function block exists, skip this cell */
		    continue;
		}
	    }
	}

	/* open 'mc' stream */
	if (!(mcstream -> dmfp = fopen (path, "r"))) {
	    dmerrno = DME_FOPEN;
	    dmError2 ("_dmFatherCell", path);
	    return (-1);
	}

	if (!(fathercell = _FindCell (name))) {
	    dmerrno = DME_NOCELL;
	    dmError2 ("_dmFatherCell", name);
	    return (-1);
	}

	while (dmGetDesignData (mcstream, mcfmt) > 0) {

	    /* Now check if it is a call to a cell mentioned in the
	    ** celllist. If not, it will not be integrated in the tree.
	    ** So cell-calls to built in functions and devices in the
	    ** circuit view will not be referred to.
	    */
	    if ((tmpc = _FindCell (mccell_name))) {

		/* Check if father cell 'name'
		** is already in father linklist of the child cell.
		*/
		for (tmpf = tmpc -> father; tmpf; tmpf = tmpf -> next)
		   if (strcmp (tmpf -> cell -> cellname, name) == 0) break;

		if (!tmpf) {
		    /* The father cell is not in the father linklist
		    ** of child cell, add father cell to the list.
		    */
		    if (!(tmpf = (FATHER *) calloc (1, sizeof (FATHER)))) {
			dmerrno = DME_CORE;
			dmError ("_dmFatherCell");
			fclose (mcstream -> dmfp);
			return (-1);
		    }
		    tmpf -> cell = fathercell;
		    tmpf -> next = tmpc -> father;
		    tmpc -> father = tmpf;
		}
	    }
	}
	fclose (mcstream -> dmfp);
    }

    /* make currentfather equals father (first level) */
    for (tmpc = celllist; tmpc; tmpc = tmpc -> next) {
	tmpc -> currentfather = tmpc -> father;
    }

    listentry[view_entry] = celllist; /* conserve celllist pointer */

#ifdef DM_DEBUG
    IFDEBUG _PrintCellList ();
#endif

    return (0);
}
