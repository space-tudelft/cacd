/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
 *	J. Liedorp
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

#include "src/drc/nbool/nbool.h"

void free_formstruct ()
{
/* This procedure frees the form_structure made by mk_frmstrct.
*/
    int     i;			/* loop variable	 */
    struct form *frm_pntr;	/* ptr to form struct.	 */
    struct min_term *mt_pntr;	/* ptr to min_term struct */

    frm_pntr = fp_head;
    while (frm_pntr) {
	for (i = 0; i < BUFLEN; i++) {
	    FREE (frm_pntr -> b_pntr[i]);
	}
	mt_pntr = frm_pntr -> mt_pntr;
	while (mt_pntr) {
	    FREE (mt_pntr);
	    mt_pntr = mt_pntr -> next;
	}
	FREE (frm_pntr);
	frm_pntr = frm_pntr -> next;
    }
}
