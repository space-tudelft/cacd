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

#ifndef __DMGLOBAL_H
#define __DMGLOBAL_H

char   *dmviews[] = {
	LAYOUT,
	CIRCUIT,
	FLOORPLAN
};
/*
** See dmincl.h: DM_NOVIEWS == sizeof (dmviews) / sizeof (char *)
*/

/*
** DDM environment
*/
char   *dmpls = NULL;
char   *dmhome = NULL;
char   *dmcur_project = NULL;
char   *dmcur_view = NULL;
char   *dmcur_cell = NULL;
char   *dmprompt = NULL;
char   *dmtoolbox = NULL;

/*
** DDM error handling
*/
int     dmerrno = 0;

char   *dmerrlist[] =
{
    "DDM error number 0",
    "error in system call",
    "DDM internal error",
    "bad arguments to DDM function call",
    "mode is invalid for key",
    "bad project",
    "bad view",
    "cell does not exist",
    "bad key",
    "cell is already checked out",
    "cannot open primary file for writing",
    "illegal recursion",
    "cell already exists",
    "cannot open technology file",
    "dmPutDesignData write error",
    "dmGetDesignData read error",
    "bad format",
    "unknown process id",
    "cannot access process data",
    "no more core",
    "process data read error",
    "bad current view",
    "bad name",
    "view does not exist",
    "view already exists",
    "project is locked",
    "cannot open file",
    "bad DDM environment",
    "no celllist present",
    "already initialized",
    "no project list present",
    "no imported celllist present",
    "cannot open file .dmrc",
    "bad release",
    "not yet initialized",
    "design manager error",
    "design manager rejects request",
    "cell is not a root",
    "no father cell",
    "no/wrong celltype",
    "no/wrong interfacetype",
    "no/wrong mask present",
    "cannot get license, contact your service organization.",
};

int     dmnerr = sizeof (dmerrlist) / sizeof (char *);

#endif /* __DMGLOBAL_H */

