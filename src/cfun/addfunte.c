/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
 *	O. Hol
 *	P.E. Menchen
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

#include "src/cfun/func_parsedefs.h"

/* Function addfun_term
** creates for the function block the files 'fterm' and
** 'fstate' in the circuit database and fills the files
** with data about the terminals and state variables.
*/
void addfun_term (FTERM *list)
{
    register FTERM * sc_ptr;
    char type;
    long low[2];
    long ind[2];

    fdes = dmOpenStream (key, "fterm", "w");

    low[0] = 0;
    low[1] = 0;
    for (sc_ptr = list; sc_ptr && (sc_ptr -> type != StateChar &&
		sc_ptr -> type != StateInt && sc_ptr -> type != StateFloat
		&& sc_ptr -> type != StateDouble);
	    sc_ptr = sc_ptr -> next) {
	ind[0] = sc_ptr -> ind[0];
	ind[1] = sc_ptr -> ind[1];
	strcpy (cterm.term_name, sc_ptr -> name);
	if (ind[0] > 0) {
	    if (ind[1] > 0)
		cterm.term_dim = 2;
	    else
		cterm.term_dim = 1;
	}
	else
	    cterm.term_dim = 0;

	ind[0]--;
	ind[1]--;
	cterm.term_lower = low;
	cterm.term_upper = ind;

	switch (sc_ptr -> type) {
	    case OutpTerm:
		cterm.term_attribute = "ftt=0";
		break;
	    case InoTerm:
		cterm.term_attribute = "ftt=3";
		break;
	    case InpTerm:
		cterm.term_attribute = "ftt=1";
		break;
	    case InrTerm:
		cterm.term_attribute = "ftt=2";
		break;
	}

	dmPutDesignData (fdes, CIR_TERM);
    }

    dmCloseStream (fdes, COMPLETE);

    if (sc_ptr) {
	fdes = dmOpenStream (key, "fstate", "w");

	for (; sc_ptr; sc_ptr = sc_ptr -> next) {
	    ind[0] = sc_ptr -> ind[0];
	    ind[1] = sc_ptr -> ind[1];
	    switch (sc_ptr -> type) {
		case StateChar:
		    type = 'c';
		    break;
		case StateInt:
		    type = 'i';
		    break;
		case StateFloat:
		    type = 'f';
		    break;
		case StateDouble:
		    type = 'd';
		    break;
		default:
		    type = '?';
	    }
	    fprintf (fdes -> dmfp, "%s %ld %ld %c\n",
		sc_ptr -> name, ind[0], ind[1], type);
	}

	dmCloseStream (fdes, COMPLETE);
    }
}
