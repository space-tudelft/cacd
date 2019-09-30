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

/* gets the interfaces (terminals) of model m */
void getterm (DM_CELL *m)
{
    DM_STREAM * dsp;
    char attribute_string[256];
    long lower[10], upper[10];
    char fn_term[DM_MAXNAME+14];
    int up;
    int low;
    int nbr;
    int i;
    int ntx;
    int nx;
    int xx;
    int new;

    sprintf (fn_term, "circuit/%s/term", m -> cell);

    dsp = dmOpenStream (m, "term", "r");

    dm_get_do_not_alloc = 1;
    cterm.term_attribute = attribute_string;
    cterm.term_lower = lower;
    cterm.term_upper = upper;

    while (dmGetDesignData (dsp, CIR_TERM) > 0) {

        ntx = newname (cterm.term_name);
	NT[ ntx ].sort = Node_t;

	nbr = 1;
        if (cterm.term_dim > 0) {
	    new = NT[ntx].xtx = newxt ();
	    XT[ new ] = cterm.term_dim;
            for (i = 0; i < cterm.term_dim; i++) {
		if (cterm.term_lower[i] <= cterm.term_upper[i]) {
		    low = cterm.term_lower[i];
		    up = cterm.term_upper[i];
		}
		else {
		    low = cterm.term_upper[i];
		    up = cterm.term_lower[i];
		}
		new = newxt ();
		XT[ new ] = low;
		new = newxt ();
		XT[ new ] = up;
		nbr = nbr * (up - low + 1);
	    }
        }

        if (cterm.term_dim <= 0) {
	    nx = newnode ();
	    NT[ ntx ].x = nx;
	    N[ nx ].ntx = ntx;
	}
	else {
	    xx = newxx (nbr) - nbr + 1;
	    NT[ ntx ].x = xx;
	    while (nbr-- > 0) {
		nx = newnode ();
		XX[ xx++ ] = nx;
	        N[ nx ].ntx = ntx;
	    }
	}
    }

    dm_get_do_not_alloc = 0;
    dmCloseStream (dsp, COMPLETE);
}
