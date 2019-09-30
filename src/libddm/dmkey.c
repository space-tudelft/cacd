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

#define DM_MAXKEYS 2048

static DM_CELL **key_admin;
static int keyno = -1;
static int maxKeys = 0;

/*
** This function allocates a structure describing an
** object which is passed to the user and initializes it.
** Information must still be written to this structure.
*/
DM_CELL * _dmMk_cellkey ()
{
    DM_CELL * key;

    if (++keyno == maxKeys) {
	maxKeys += DM_MAXKEYS;
	key_admin = realloc (key_admin, sizeof (DM_CELL *) * maxKeys);
	if (!key_admin) _dmFatal ("_dmMk_cellkey: too many open cells", "", "");
    }

    if ((key = (DM_CELL *) malloc (sizeof (DM_CELL))) == NULL) {
	_dmFatal ("_dmMk_cellkey: cannot alloc key", "", "");
    }

    key_admin[keyno] = key;
    key -> keyno = keyno;
    return (key);
}

/*
** This function removes the structure which is passed to
** the user at check_out time and which identificates the
** object checked out.
*/
void _dmRm_cellkey (DM_CELL *key)
{
    if (key -> keyno != keyno) {
	key_admin[key -> keyno] = key_admin[keyno];
	key_admin[keyno] -> keyno = key -> keyno;
    }
    --keyno;
    _dmStrFree (key -> cell);
    _dmStrFree (key -> versionstatus);
    _dmStrFree (key -> view);
    free (key);
}

/*
** check if some cell is checked out
*/
int _dmCh_chkout ()
{
    if (keyno == -1) return (0);
    return (1);
}

/*
** check if key is valid
*/
int _dmCh_key (DM_CELL *key)
{
    if (key == NULL
	    || keyno < 0
	    || key_admin[key -> keyno] != key) {
	dmerrno = DME_BADKEY;
	return (-1);
    }
    return (0);
}

/* Check if the cell identified by 'proj', 'cell' and 'view'
** has already been checked out by this process.
*/
int _dmCh_cell (DM_PROJECT *proj, char *cell, const char *view)
{
    int i;
    for (i = 0; i <= keyno; ++i) {
	if (key_admin[i] -> dmproject == proj &&
	    strcmp (key_admin[i] -> cell, cell) == 0 &&
	    strcmp (key_admin[i] -> view, view) == 0) {
	    dmerrno = DME_LOCK;
	    return (-1);
	}
    }
    return (0);
}

void dmCkinAll (int mode)
{
    int i;
    for (i = keyno; i >= 0; --i)
	dmCheckIn (key_admin[i], mode);
}
