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

static char *Xcf = ".dmxdata";

FILE *dmOpenXData (DM_PROJECT *project, char *mode)
{
    char name[DM_MAXLINE];
    char bmode[4];
    _dmSprintf (name, "%s/%s", project -> dmpath, Xcf);
    bmode[0] = mode[0];
    bmode[1] = 'b';
    bmode[2] = mode[1] == '+'? '+' : 0;
    bmode[3] = 0;
    return fopen (name, bmode);
}

int dmStatXData (DM_PROJECT *project, struct stat *buf)
{
    char name[DM_MAXLINE];
    _dmSprintf (name, "%s/%s", project -> dmpath, Xcf);
    return stat (name, buf);
}

int dmUnlinkXData (DM_PROJECT *project)
{
    char name[DM_MAXLINE];
    _dmSprintf (name, "%s/%s", project -> dmpath, Xcf);
    return unlink (name);
}
