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

static DM_PROJECT **project_admin;
static int projectno = -1;
static int maxProj = 0;

/*
** This function allocates a structure describing an
** project which is passed to the user and initializes it.
** Information must still be written to this structure.
*/
DM_PROJECT * _dmMk_projectkey ()
{
    DM_PROJECT * key;

    if (++projectno == maxProj) {
	maxProj += DM_MAXPROJECTS;
	project_admin = realloc (project_admin, sizeof (DM_PROJECT *) * maxProj);
	if (!project_admin) _dmFatal ("_dmMk_projectkey: too many open projects", "", "");
    }

    if ((key = (DM_PROJECT *) malloc (sizeof (DM_PROJECT))) == NULL) {
	_dmFatal ("_dmMk_projectkey: cannot alloc key", "", "");
    }

    project_admin[projectno] = key;
    key -> projectno = projectno;
    return (key);
}

/*
** This function removes the structure which is passed to
** the user at dmOpenProject time and which identifies the
** project opened.
*/
void _dmRm_projectkey (DM_PROJECT *key)
{
    if (key -> projectno != projectno) {
	project_admin[key -> projectno] = project_admin[projectno];
	project_admin[projectno] -> projectno = key -> projectno;
    }
    --projectno;
    _dmStrFree (key -> dmpath);
    free (key);
}

/*
** check if some project is already opened
*/
int _dmCh_opproj ()
{
    if (projectno == -1) return (0);
    return (1);
}

/*
** check if project key is valid
*/
int _dmCh_project (DM_PROJECT *key)
{
    if (key == NULL
	    || projectno < 0
	    || project_admin[key -> projectno] != key) {
	dmerrno = DME_BADKEY;
	return (-1);
    }
    return (0);
}

DM_PROJECT * _dmCh_proj (char *path)
{
    int i;
    for (i = 0; i <= projectno; ++i) {
	if (strcmp (project_admin[i] -> dmpath, path) == 0) {
	    return (project_admin[i]);
	}
    }
    return (NULL);
}

void _dmClose_allproj (int mode)
{
    int i;
    dmCkinAll (mode);
    for (i = projectno; i >= 0; --i)
	dmCloseProject (project_admin[i], mode);
}

/*
** _dmSearchProcInProjKeys () consults the project administration that
** is maintained in this source file, to see whether a project key
** exists with the same processid and the maskdata attached to it.
*/
DM_PROCDATA * _dmSearchProcInProjKeys (int processid)
{
    int i;
    for (i = 0; i <= projectno; ++i) {
	if (project_admin[i] != NULL &&
		project_admin[i] -> procid == processid &&
		project_admin[i] -> maskdata != NULL)
	    return (project_admin[i] -> maskdata);
    }
    return (NULL);
}
