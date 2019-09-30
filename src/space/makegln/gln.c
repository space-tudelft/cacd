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

#include "src/space/makegln/config.h"
#include <stddef.h>
#include <stdio.h>
#include "src/space/auxil/auxil.h"
#include "src/space/makegln/makegln.h"
#include "src/space/makegln/proto.h"

/*  How to read next mnemonics.
    Two characters, first is for left of scanline, second for right.
	B means blank(b)
	S means solid(s)
	D means solid below current y, blank above
	U means solid above current y, blank below

    Examples:
		  s|s
        SU means  -+-
		  s|b

		  s|s
        UU means  -+-
		  b|b
*/

#define BB   0
#define BD  01	/* begin stop edge  */
#define BU  02	/* begin start edge */
#define BS  03
#define DB  04
#define DD  05
#define DU  06
#define DS  07
#define UB 010
#define UD 011
#define UU 012
#define US 013
#define SB 014
#define SD 015
#define SU 016
#define SS 017

#ifdef DEBUG
static char * codes[] = {
    /* BB */ "void",
    /* BD */ "begin stop edge",
    /* BU */ "begin start edge",
    /* BS */ "void",
    /* DB */ "end stop edge",
    /* DD */ "void",
    /* DU */ "end stop, begin start",
    /* DS */ "end stop edge",
    /* UB */ "end start edge",
    /* UD */ "end start, begin stop",
    /* UU */ "void",
    /* US */ "end start edge",
    /* SB */ "void",
    /* SD */ "begin stop edge",
    /* SU */ "begin start edge",
    /* SS */ "void"
};
#endif /* DEBUG */

void glnUpdate (edge_t *edge, coor_t x)
{
    static coor_t prevX = -INF;
    static int nw = 0;
    static int ne = 0;
    static int sw = 0;
    static int se = 0;
    edge_t *newEdge;
    int code;

    if (x != prevX) {
	ASSERT (nw == 0 && ne == 0 && sw == 0 && se == 0);
	prevX = x;
    }

    if (edge -> xl < x) nw += edge -> signLeft;
    if (edge -> xr > x) ne += edge -> sign;

    ASSERT (nw >= 0);
    ASSERT (ne >= 0);

    if (nw == ne && sw == se) {
	se = sw = nw;
	ASSERT (edge -> signLeft == edge -> sign);
	return;
    }

    code = 0;
    if (se) code += 1;
    if (ne) code += 2;
    if (sw) code += 4;
    if (nw) code += 8;

#ifdef DEBUG
    printEdge ("evalCode", edge);
    fprintf (stderr, "nw=%d ne=%d sw=%d se=%d code=0%o (%s)\n", nw, ne, sw, se, code, codes[code]);
#endif

    switch (code) {
        case BD:		/* begin stop edge  	 */
        case BU:		/* begin start edge 	 */
        case SD:		/* begin stop edge 	 */
        case SU:		/* begin start edge 	 */
	    newEdge = createEdge (x, Y (edge, x), 0, 0, edge -> slope, 0);
	    selectForOutput (newEdge);
	    edge -> next = newEdge;
	    break;
        case DB:		/* end stop edge 	 */
        case DS:		/* end stop edge 	 */
        case UB:		/* end start edge 	 */
        case US:		/* end start edge 	 */
	    ASSERT (edge -> next);
	    edge -> next -> xr = x;
	    edge -> next -> yr = Y (edge, x);
	    readyForOutput (edge -> next);
	    break;
        case DU:		/* end stop, begin start */
        case UD:		/* end start, begin stop */
	    ASSERT (edge -> next);
	    edge -> next -> xr = x;
	    edge -> next -> yr = Y (edge, x);
	    readyForOutput (edge -> next);
	    newEdge = createEdge (x, Y (edge, x), 0, 0, edge -> slope, 0);
	    selectForOutput (newEdge);
	    edge -> next = newEdge;
	    break;
    }

    sw = nw;
    se = ne;
    edge -> signLeft = edge -> sign;
}
