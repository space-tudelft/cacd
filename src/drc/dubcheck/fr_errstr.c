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

#include "src/drc/dubcheck/dubcheck.h"

void rmv_errstr ()
{
/* This procedure frees the structure containing the
 * equivalence of the groups of mask1 and mask2.
 */
    unsigned int i, j;
    struct group   *g_pntr;
    struct eq  *eq_pntr;

    for (i = 0; i < GRP_BUFLEN; i++) {
	if (group_arr[i]) {
	    g_pntr = group_arr[i];
	    for (j = 0; j < GRP_BUFLEN; j++) {
		eq_pntr = g_pntr[i].eq_pntr;
		while (eq_pntr) {
		    FREE (eq_pntr);
		    eq_pntr = eq_pntr -> next;
		}
	    }
	    FREE (g_pntr);
	    group_arr[i] = NULL;
	}
    }
}

void free_errs (struct chk_err *err_head)
{
/* This procedure frees the error linked_list.
*/
    struct chk_err *c_err;
    c_err = err_head;
    while (c_err) {
	FREE (c_err);
	c_err = c_err -> next;
    }
}
