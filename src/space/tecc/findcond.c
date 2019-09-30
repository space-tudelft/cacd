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

#include <stdio.h>
#include <stdlib.h>

#include "src/space/tecc/define.h"
#include "src/space/tecc/extern.h"

mask_t filterBitmask; /* contains conductor bits */
mask_t subBitmask;    /* contains special bit */
mask_t *newMaskColor; /* used for unreal masks */

void findConducTransf ()
{
    struct w_list *w;
    struct layCondRef *cond;
    mask_t color, color2, colorstack[4];
    int i, j, k, n, new_i, new_k, set, pco;

    conducCnt = 0;

    ALLOC (conducTransf, procdata -> nomasks + subdata -> nomasks, int);
    if (subdata -> nomasks) ALLOC (newMaskColor, subdata -> nomasks, mask_t);

    for (i = 0; i < procdata -> nomasks + subdata -> nomasks; i++) conducTransf[i] = -1;

    /* Start makeing the filterBitmask
	and the conducTransf for all conductor masks. */

    for (i = fem_res_cnt; i < res_cnt; i++) {
	k = ress[i].mask;
	if (conducTransf[k] < 0) { /* first time of mask k */
	    conducTransf[k] = conducCnt++;
	    if (k < procdata -> nomasks || resizemask(k) >= 0) { /* real mask */
		COLOR_ADDINDEX (filterBitmask, maskTransf[k]);
	    }
	}
    }

    conducCntStd = conducCnt;
    conducCntPos = conducCnt;

    if (w_list_p || w_list_n) {
	for (w = w_list_p; w; w = w -> next) { // fem p-type
	    k = w -> mskno;
	    ASSERT (k >= procdata -> nomasks);
	    ASSERT (conducTransf[k] < 0);
	    ++conducCntPos;
	    conducTransf[k] = conducCnt++;
	}
	for (w = w_list_n; w; w = w -> next) { // fem n-type
	    k = w -> mskno;
	    ASSERT (k >= procdata -> nomasks);
	    ASSERT (conducTransf[k] < 0);
	    conducTransf[k] = conducCnt++;
	}
    }

    /* filterBitmask contains now the color bits of all real
	mask conductors. Now we must add missing color bits
	and the newMaskColor's for all unreal conductor masks. */

    new_i = res_cnt ? 0 : -1;
    while (new_i >= 0) { /* new mask is conductor mask */
	i = new_i; new_i = -1;
	new_k = ress[i].mask;
	k = new_k - procdata -> nomasks;
	n = 0;
found_i:
	color2 = cNull; pco = 0;
	for (cond = ress[i].cond; cond; cond = cond -> next) {
	    if (cond -> layC -> present) { ++pco;
		COLORINITINDEX (color, maskTransf[cond -> layC -> lay -> mask]);
		if (COLOR_PRESENT (&filterBitmask, &color)) {
		    if (k >= 0) COLOR_ADD (newMaskColor[k], color);
		    goto found;
		}
		COLOR_ADD (color2, color);
	    }
	}
	set = 0;

	if (pco && n) // present colors && color stack
	for (cond = ress[i].cond; cond; cond = cond -> next) {
	    if (cond -> layC -> present) {
		COLORINITINDEX (color, maskTransf[cond -> layC -> lay -> mask]);
		for (j = 0; j < n; ++j) {
		    if (COLOR_PRESENT (&colorstack[j], &color)) break;
		}
		if (j < n) { // found color in stack
		    while (++j < n) colorstack[j-1] = colorstack[j];
		    set = 1;
		    COLOR_ADD (filterBitmask, color);
		    if (k >= 0) COLOR_ADD (newMaskColor[k], color);
		    if (--n == 0) break;
		}
	    }
	}

	if (pco && !set) { // present colors && not in stack
	    if (pco == 1) {
		COLOR_ADD (filterBitmask, color2);
		if (k >= 0) COLOR_ADD (newMaskColor[k], color2);
	    }
	    else { // pco > 1
		ASSERT (n < 4);
		colorstack[n++] = color2;
	    }
	}
found:
	while (++i < res_cnt) {
	    j = ress[i].mask;
	    if (j == new_k) { if (pco) goto found_i; }
	    else if (new_i < 0) new_i = i;
	}

	// newmask 'new_k' is ready

	if (!pco) { // no present color
	    COLOR_ADDINDEX (subBitmask, maxprocmasks); // first free index
	    if (k >= 0) newMaskColor[k] = subBitmask;
	    COLOR_ADD (filterBitmask, subBitmask);
	}
	else if (n) { // colors in stack
	    while (--n >= 0) {
		if (k >= 0) COLOR_ADD (newMaskColor[k], colorstack[n]);
		COLOR_ADD (filterBitmask, colorstack[n]);
	    }
	}
    }
}
