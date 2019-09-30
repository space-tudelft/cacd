/*
 * ISC License
 *
 * Copyright (C) 2014-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include "src/space/include/config.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "src/libddm/dmincl.h"
#include "src/space/auxil/auxil.h"

void docell (char *cellname, char *version);

DM_CELL    *cellKey;
DM_PROJECT *dmproject;
extern char *argv0;

int main (int argc, char **argv)
{
    argv0 = "wrmesh";
    if (--argc < 1) {
	say ("No cellname specified\n");
	printf ("Usage: %s cell\n", argv0);
	return (1);
    }
    dmInit ("makemesh");
    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);
    docell (argv[argc], WORKING);
    dmCloseProject (dmproject, COMPLETE);
    dmQuit ();
    return (0);
}

void dmError (char *s) { dmPerror (s); die (); }
void die () { dmQuit (); exit (1); }
void extraSay () { }

void docell (char *cellname, char *version) /* do a cell */
{
    DM_STREAM *outputStream;

    cellKey = dmCheckOut (dmproject, cellname, version, DONTCARE, LAYOUT, ATTACH);
    outputStream = dmOpenStream (cellKey, "mesh_gln", "w");
//#define EMPTY_GLN
#ifndef EMPTY_GLN
    ggln.xl = 0*4; ggln.xr = 8*4; ggln.yl = ggln.yr = -5*4; dmPutDesignData (outputStream, GEO_GLN);
    ggln.xl = 0*4; ggln.xr = 8*4; ggln.yl = ggln.yr = 24*4; dmPutDesignData (outputStream, GEO_GLN);
    ggln.xl =32*4; ggln.xr =40*4; ggln.yl = ggln.yr = -5*4; dmPutDesignData (outputStream, GEO_GLN);
    ggln.xl =32*4; ggln.xr =40*4; ggln.yl = ggln.yr = 24*4; dmPutDesignData (outputStream, GEO_GLN);
#endif
    dmCloseStream (outputStream, COMPLETE);
    dmCheckIn (cellKey, COMPLETE);
}
