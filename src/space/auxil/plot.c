/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#include <stddef.h>
#include "src/space/auxil/auxil.h"
#include "src/space/auxil/plot.h"

static char * styles[] = {
    "",			/* SOLID */
    "dotted",		/* DOTTED */
    "dashed"		/* DASHED */
};

static char * positions [] = {
    "",			/* CENTER */
    "above",		/* ABOVE */
    "below",		/* BELOW */
    "ljust",		/* LJUST */
    "rjust"		/* RJUST */
};

static FILE *fpPic = NULL;

/* An interface to produce .pic file.
 *
 * Open the plotfile, its name is <baseName>.pic
 */
void plotInit (char *baseName)
{
    char buf[512];
    sprintf (buf, "%s.pic", baseName);
    fpPic = cfopen (buf, "w");
    fprintf (fpPic, ".PS 6i\n");
}

/* Close plotfile
 */
void plotEnd ()
{
    if (fpPic) { fprintf (fpPic, ".PE\n"); fclose (fpPic); fpPic = NULL; }
}

/* Draw a line from (x1,y1) to (x2,y2)
 * The linestyle is one of the defined names in plot.h
 */
void plotLine (int style, long x1, long y1, long x2, long y2)
{
    if (fpPic) fprintf (fpPic, "line %s from %ld,%ld to %ld,%ld\n", styles[style], x1, y1, x2, y2);
}

/* Plot a string <s> in pointsize <size> at <x>,<y>.
 * <adjust> is one of the names defined in plot.h
 */
void plotString (int size, int adjust, char *s, long x, long y)
{
    if (fpPic) fprintf (fpPic, "\"\\s%d%s\\s0\" at %ld,%ld %s\n", size, s, x, y, positions[adjust]);
}
