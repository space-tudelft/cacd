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

static int equiv_grp (void);

void print_err (int kind)
{
/* This procedure prints the errors stored in the tempory
 * file bufil. It uses the procedure equiv_grp to suppres
 * errors between equivalent groups.
 */
    while (fread ((char *) & buff, sizeof (buff), 1, bufil) != 0) {
	if ((kind == 0) || (equiv_grp () == FALSE)) {
	    error_meas ("gap", buff.x1, buff.y1, buff.x2, buff.y2);
	    Errno++;
	}
    }
}

static int equiv_grp ()
{
/* This procedure looks if the group_numbers in the
 * buffer are equivalent, i.e. if the groups overlap.
 * If they do TRUE is returned; otherwise FALSE.
 */
    struct group   *g_pntr;
    struct eq  *eq_pntr;
    unsigned int i, j, mask = GRP_BUFLEN - 1;

    i = buff.group1 & mask;
    j = buff.group1 / GRP_BUFLEN;
    g_pntr = group_arr[j];
    if (g_pntr == NULL)
	return (FALSE);
    eq_pntr = g_pntr[i].eq_pntr;
    while (eq_pntr != NULL) {
	if (eq_pntr -> eq_group == buff.group2)
	    return (TRUE);
	eq_pntr = eq_pntr -> next;
    }
    return (FALSE);
}
