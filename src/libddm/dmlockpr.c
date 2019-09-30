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
#include <fcntl.h>

int     dmlock_flag = 0;

#ifdef PRLOCK

int _dmLockProject (char *project)
{
    int     fd;
    char    path[MAXLINE];

    _dmSprintf (path, "%s/.lockpr", project);

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "lockpr: dmlock_flag = %d, path = %s\n", dmlock_flag, path);
#endif

    if (dmlock_flag || (fd = creat (path, 0444)) < 0) {
	dmerrno = DME_PRLOCK;
	dmError (path);
	return (-1);
    }
    close (fd);

    dmlock_flag = 1;
    return (0);
}

int _dmUnlockProject (char *project)
{
    char    path[MAXLINE];

    _dmSprintf (path, "%s/.lockpr", project);

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "unlockpr: dmlock_flag = %d, path = %s\n", dmlock_flag, path);
#endif

    chmod (path, 0666);
    if (!dmlock_flag || unlink (path) != 0) {
	perror (path);
	exit (2);
    /* can not use _dmFatal, because it would try dmQuit, and in *
       release 2 (single-user) dmQuit will try _dmUnlockProject again. */
    }

    dmlock_flag = 0;
    return (0);
}

#endif
