/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	P. Bingley
 *	S. de Graaf
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

#include <stdio.h>
#include <stdlib.h>
#include "src/cacdcmap/cmaperror.h"

/* Global Declarations */
int cacdcmaperrno;

char *cacdcmaperrors[] = {
    /* NOINIT	    */	"cacd cmap not initialized",
    /* NOCMAP	    */	"cannot create colormap",
    /* BADCMAP	    */	"invalid standard colormap",
    /* NOGC	    */	"cannot create graphics context",
    /* NOCORE	    */	"no more core",
    /* BADCOLOR	    */	"invalid color specification",
    /* BADID	    */	"invalid id specified",
    /* ARRAYTOSMALL */	"affected id array to small",
    /* BADERROR	    */	"invalid error code"
};

int CacdCmapError (char *string, int fatal)
{
    /* if illegal error code */
    if (cacdcmaperrno > CMAPMAXERROR) {
	cacdcmaperrno = BADERROR;
	if (!fatal) return (0);
    }

    /* print the error message */
    fprintf (stderr, "%s: %s\n", (string ? string : ""),
			cacdcmaperrors[cacdcmaperrno]);

    if (fatal) exit (cacdcmaperrno);
    return (1);
}
