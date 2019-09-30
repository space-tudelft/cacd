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
** This function 'checks_out' an existing cell of the
** current project.  The caller is returned a key which
** must be used when invoking the functions 'dmOpenStream' or
** 'dmCloseStream'.  These functions can then determine which
** object to access.
** NULL is returned when the attempt fails.
*/

DM_CELL * dmCheckOut (DM_PROJECT *dmproject, char *cell, const char *versionstatus, int versionnumber, const char *view, int mode)
{
    DM_CELL * key;
    int     exist;
    int     imode = mode;
    char    path[MAXLINE];

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "cell: %s, version: %s, view: %s, mode: %d\n",
		cell, versionstatus, view, mode);
#endif

    if (_dmCh_project (dmproject) != 0) {
	dmError ("project key");
	return (NULL);
    }

 /* Check existence of cell in view. ** The cellname and view are also
    tested. ** If exist == 0 cell doesnot exist, ** if exist == 1 cell
    does exist. */
    if ((exist = _dmExistCell (dmproject, cell, view)) < 0) {
	return (NULL);
    }

 /* check if cell has already been checked out by this tool */
    if (_dmCh_cell (dmproject, cell, view)) {
	dmError (cell);
	return (NULL);
    }

    if (strcmp (versionstatus, ACTUAL))
	if (strcmp (versionstatus, WORKING))
	    if (strcmp (versionstatus, BACKUP))
		if (strcmp (versionstatus, DERIVED)) {
		    dmerrno = DME_BADARG;
		    dmError ((char*)versionstatus);
		    return (NULL);
		}

    if (mode == CREATE) mode = UPDATE; /* for compatibility with release 4 */

    if (!(mode == READONLY || mode == UPDATE || mode == ATTACH)) {
	dmerrno = DME_BADARG;
	dmError ("mode");
	return (NULL);
    }

    if (mode == UPDATE) {
	if (strcmp (versionstatus, WORKING))
	    if (strcmp (versionstatus, DERIVED)) {
		dmerrno = DME_BADARG;
		dmError ("mode");
		return (NULL);
	    }
    }

    if (!exist) {
	if (mode == READONLY || mode == ATTACH) {
	    dmerrno = DME_NOCELL;
	    dmError (cell);
	    return (NULL);
	}
	else {			/* mode == UPDATE */
	    _dmSprintf (path, "%s/%s/%s", dmproject -> dmpath, view, cell);
            if (mkdir (path, 0777)) {
		dmerrno = DME_SYS;
		dmError ("dmCheckOut");
		return (NULL);
	    }
	    mode |= INT_CREATE;
	}
    }
    else if (imode == CREATE) { /* to remove existing old stuff */
	_dmSprintf (path, "%s/%s/%s", dmproject -> dmpath, view, cell);
	_dmRmDirContents (dmproject, path);
    }

    key = _dmMk_cellkey ();
    key -> dmproject = dmproject;
    key -> cell = _dmStrSave (cell);
    key -> versionstatus = _dmStrSave ((char*)versionstatus);
    key -> versionnumber = versionnumber;
    key -> view = _dmStrSave ((char*)view);
    key -> mode = mode;

#ifdef DM_DEBUG
    IFDEBUG {
	fprintf (stderr, "keyno: %d\n", key -> keyno);
	_dm_print_key (key, "dmCheckOut");
    }
#endif

    return (key);
}
