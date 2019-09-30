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
#include <errno.h>
#include <dirent.h>

/*
** dmRemoveCell removes a 'cell' with viewtype 'view', with
** versionstatus 'versionstatus', and versionnumber 'versionnumber
** from the project identified by 'dmproject'.
** The arguments are: dmproject, cell, versionstatus, versionnumber, view)
** It calles the (for internal use added) function dmRmCellForce which has an
** extra argument : a force-flag.
** In the next release it will be possible to call this function with a
** list of pointers to keys instead of only one cell-name.
*/
int dmRemoveCell (DM_PROJECT *dmproject, char *cell, char *versionstatus, int versionnumber, char *view)
{
    return (dmRmCellForce (dmproject, cell, versionstatus, versionnumber, view, 0));
}

int dmRmCellForce (DM_PROJECT *dmproject, char *cell, char *versionstatus, int versionnumber, char *view, int force)
/* if force == 0 : check if cell is a root-cell */
/* else no check */
{
    FILE   *celllist_fp;
    char  **celllist_array;
    char    celllist_member[DM_MAXNAME + 1];
    char    path[MAXLINE];
    int     exist, found, i, nr_cells_inlist;

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "_dmRmCell: cell: %s, version: %s, view: %s\n", cell, versionstatus, view);
#endif

    if (_dmCh_project (dmproject) != 0) { dmError ("_dmRmCell: project key"); return (-1); }

 /* Check existence of cell in view.  The cellname and view are also tested.
  * If exist == 0 cell doesnot exist. If exist == 1 cell does exist. */
    if ((exist = _dmExistCell (dmproject, cell, view)) <= 0) {
	if (exist == 0) dmError2 ("_dmRmCell", cell);
	return (-1);
    }

 /* check if cell has already been checked out by this tool */
    if (_dmCh_cell (dmproject, cell, view)) { dmError2 ("_dmRmCell", cell); return (-1); }

    if (strcmp (versionstatus, ACTUAL))
	if (strcmp (versionstatus, WORKING))
	    if (strcmp (versionstatus, BACKUP))
		if (strcmp (versionstatus, DERIVED)) {
		    dmerrno = DME_BADARG; dmError2 ("_dmRmCell", versionstatus); return (-1); }

 /* extra (superfluous) check of permissions */
    if (!((dmproject -> mode) & PROJ_WRITE)) { dmerrno = DME_MODE; dmError ("_dmRmCell"); return (-1); }

    _dmSprintf (path, "%s/%s/celllist", dmproject -> dmpath, view);

    if ((celllist_fp = fopen (path, "r")) == NULL) {
	dmerrno = DME_NOCELLL; dmError2 ("_dmRmCell", path); return (-1);
    }

    i = 0;
    while (fscanf (celllist_fp, "%s", celllist_member) != EOF) ++i;

    if (!(celllist_array = (char **) malloc ((unsigned) ((i + 1) * sizeof (char *))))) {
	dmerrno = DME_CORE; dmError ("_dmRmCell"); return (-1);
    }

    nr_cells_inlist = i;
    i = 0;
    rewind (celllist_fp);
    while (fscanf (celllist_fp, "%s", celllist_member) != EOF) {
	if (!(celllist_array[i] = _dmStrSave (celllist_member))) {
	    dmerrno = DME_CORE; dmError ("_dmRmCell"); nr_cells_inlist = i; goto err_ret;
	}
	++i;
    }
    if (i != nr_cells_inlist) _dmFatal ("_dmRmCell: read error", "", "");
    celllist_array[i] = NULL;

    fclose (celllist_fp);

    if (force == 0 && (i = _dmCellIsRoot2 (dmproject, cell, view, celllist_array)) != 1) {
	if (i == 0) { dmerrno = DME_NOROOT; dmError2 ("_dmRmCell", cell); }
	goto err_ret;
    }

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "_dmRmCell: trying to remove '%s' of viewtype '%s'\n", cell, view);
#endif

 /* Everything seems to work out fine: perform the actual operations. */
 /* Perform removal of cell-streams and cell. */

    if ((celllist_fp = fopen (path, "w")) == NULL) {
	dmerrno = DME_FOPEN; dmError2 ("_dmRmCell", path); goto err_ret;
    }

 /* Write new celllist. */
    for (found = i = 0; i < nr_cells_inlist; ++i) {
	if (!found && strcmp (cell, celllist_array[i]) == 0) ++found;
	else fprintf (celllist_fp, "%s\n", celllist_array[i]);
	_dmStrFree (celllist_array[i]);
    }
    free (celllist_array);
    fclose (celllist_fp);

    return (_dmRmCell (dmproject, cell, view));

err_ret:
    for (i = 0; i < nr_cells_inlist; ++i) _dmStrFree (celllist_array[i]);
    free (celllist_array);
    return (-1);
}

/*
** _dmRmCell executes the actual removal of the 'cell' with viewtype 'view'.
*/
int _dmRmCell (DM_PROJECT *dmproject, char *cell, char *view)
{
    struct stat st_buf;
    char path[DM_MAXPATHLEN];

    _dmSprintf (path, "%s/%s/%s", dmproject -> dmpath, view, cell);
#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "_dmRmCell: path = '%s'\n", path);
#endif
    if (lstat (path, &st_buf)) {
	dmerrno = DME_SYS;
	dmError2 ("_dmRmCell: stat", path);
	return (-1);
    }
    if (S_ISDIR (st_buf.st_mode)) {
	return (_dmRmDir (dmproject, path));
    }
    if (unlink (path)) { /* remove link */
	dmerrno = DME_SYS;
	dmError2 ("_dmRmCell: unlink", path);
	return (-1);
    }
    return (0);
}

int _dmRmDir (DM_PROJECT *dmproject, char *path)
{
    if (_dmRmDirContents (dmproject, path)) return (-1);

    if (rmdir (path)) {
	dmerrno = DME_SYS;
	dmError2 ("_dmRmDir: rmdir", path);
	return (-1);
    }
    return (0);
}

int _dmRmDirContents (DM_PROJECT *dmproject, char *path)
{
    char   newpath[DM_MAXPATHLEN];
    DIR * dirp;
    struct stat st_buf;
    struct dirent  *dp;

    dirp = opendir (path);
    for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {
	if (strcmp (dp -> d_name, ".") != 0 && strcmp (dp -> d_name, "..") != 0) {
	    sprintf (newpath, "%s/%s", path, dp -> d_name);
	    if (lstat (newpath, &st_buf) != 0) {
		dmerrno = DME_SYS;
		dmError2 ("_dmRmDir: stat", path);
		return (-1);
	    }
	    if (S_ISDIR (st_buf.st_mode)) {
		if (_dmRmDir (dmproject, newpath)) return (-1);
	    }
	    else
		unlink (newpath);
	}
    }
    closedir (dirp);
    return (0);
}
