/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van Genderen
 *	S. de Graaf
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

#include "src/sls_exp/extern.h"

/* compares two names of the name table NT[] */
static int qs_nmcmp (const void *el1, const void *el2)
{
    NAMETABLE *e1 = (NAMETABLE *)el1;
    NAMETABLE *e2 = (NAMETABLE *)el2;
    return (strcmp (ST + e1 -> name, ST + e2 -> name));
}

/* sorts name table elements from mcs_NT_cnt to NT_cnt  */
/* into lexographical order.  the routine automatically */
/* updates the references concerned.                    */
/* this procedure beter not be called after compact()   */
void sort ()
{
    NAMETABLE *nt;
    int cnt, number, up, low, xtx, x, xt_cnt;

    qsort ((char *)(NT + mcs_NT_cnt), (unsigned)(NT_cnt - mcs_NT_cnt), sizeof (NAMETABLE), qs_nmcmp);

    /* update the references to the NT[] elements */

    nt = NT + mcs_NT_cnt;
    for (cnt = mcs_NT_cnt; cnt < NT_cnt; cnt++) {
	number = 1;
	if ( nt -> xtx >= 0 ) {
	    xtx = nt -> xtx;
	    for (xt_cnt = XT[ xtx++ ]; xt_cnt > 0; xt_cnt--) {
		low = XT[ xtx++ ];
		up = XT[ xtx++ ];
		number = number * ( up - low + 1 );
	    }
	}
	switch ( nt -> sort ) {
	    case Node :
	    case Node_t :
		x = nt -> x;
		if ( nt -> xtx < 0 ) {
		    N[ x ].ntx = cnt;
		}
		else {
		    while (number-- > 0)
		        N[ XX[x++] ].ntx = cnt;
		}
		break;
	    case Modelcall :
		x = nt -> x;
		while (number-- > 0)
		    MCT[ x++ ].ntx = cnt;
		break;
	    default :
		/* Transistor, Intercap and Functional have no  */
		/* reference back to the name table, so nothing */
		/* has to be updated then                       */
		break;
	}
	nt++;
    }
}
