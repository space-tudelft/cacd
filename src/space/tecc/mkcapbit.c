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
#include <signal.h>

#include "src/space/tecc/define.h"
#include "src/space/tecc/extern.h"

static mask_t sBitPresent, sBitAbsent;
static mask_t eBitPresent, eBitAbsent;
static mask_t oBitPresent, oBitAbsent;

Private void setBitmasks (struct layCondRef **Cond)
{
    struct layCondRef *cond, *prevcond;
    int ci;

    COLORINIT (sBitPresent);
    COLORINIT (sBitAbsent);
    COLORINIT (eBitPresent);
    COLORINIT (eBitAbsent);
    COLORINIT (oBitPresent);
    COLORINIT (oBitAbsent);

    prevcond = NULL;
    for (cond = *Cond; cond; cond = cond -> next) {

	ci = maskTransf[ cond -> layC -> lay -> mask ];

	if (cond -> layC -> lay -> occurrence == SURFACE) {
	    if (cond -> layC -> present) {
		COLOR_ADDINDEX (sBitPresent, ci); }
	    else {
		COLOR_ADDINDEX (sBitAbsent, ci); }
	    prevcond = cond;
	}
	else if (cond -> layC -> lay -> occurrence == EDGE) {
	    if (cond -> layC -> present) {
		COLOR_ADDINDEX (eBitPresent, ci); }
	    else {
		COLOR_ADDINDEX (eBitAbsent, ci); }
	    prevcond = cond;
	}
	else { /* OTHEREDGE */
	    if (cond -> layC -> present) {
		COLOR_ADDINDEX (oBitPresent, ci); }
	    else {
		COLOR_ADDINDEX (oBitAbsent, ci); }

	    /* remove this redundant element condition */
	    if (prevcond) prevcond -> next = cond -> next;
	    else *Cond = cond -> next;
	}
    }
}

/* Make the bitmasks for searching the lateral capacitances.
 * Also remove the redundant element conditions then.
 */
void mkCapBitmasks ()
{
    int i;

    for (i = 0; i < cap_cnt; i++) {
	setBitmasks (&caps[i].cond);
	caps[i].sBitPresent = sBitPresent;
	caps[i].sBitAbsent  = sBitAbsent;
	caps[i].eBitPresent = eBitPresent;
	caps[i].eBitAbsent  = eBitAbsent;
	caps[i].oBitPresent = oBitPresent;
	caps[i].oBitAbsent  = oBitAbsent;
    }

    for (i = 0; i < con_cnt; i++) {
	setBitmasks (&cons[i].cond);
	cons[i].sBitPresent = sBitPresent;
	cons[i].sBitAbsent  = sBitAbsent;
    }

    for (i = 0; i < cnt_cnt; i++) {
	setBitmasks (&cnts[i].cond);
	cnts[i].sBitPresent = sBitPresent;
	cnts[i].sBitAbsent  = sBitAbsent;
	cnts[i].eBitPresent = eBitPresent;
	cnts[i].eBitAbsent  = eBitAbsent;
    }

    for (i = 0; i < res_cnt; i++) {
	setBitmasks (&ress[i].cond);
	ress[i].sBitPresent = sBitPresent;
	ress[i].sBitAbsent  = sBitAbsent;
    }

    for (i = 0; i < tor_cnt; i++) {
	setBitmasks (&tors[i].cond);
	tors[i].sBitPresent = sBitPresent;
	tors[i].sBitAbsent  = sBitAbsent;
    }

    for (i = 0; i < bjt_cnt; i++) {
	setBitmasks (&bjts[i].cond);
	bjts[i].sBitPresent = sBitPresent;
	bjts[i].sBitAbsent  = sBitAbsent;
	bjts[i].eBitPresent = eBitPresent;
	bjts[i].eBitAbsent  = eBitAbsent;
	bjts[i].oBitPresent = oBitPresent;
	bjts[i].oBitAbsent  = oBitAbsent;
    }

    for (i = 0; i < jun_cnt; i++) {
	setBitmasks (&juns[i].cond);
	juns[i].sBitPresent = sBitPresent;
	juns[i].sBitAbsent  = sBitAbsent;
	juns[i].eBitPresent = eBitPresent;
	juns[i].eBitAbsent  = eBitAbsent;
    }

    for (i = 0; i < vdm_cnt; i++) {
	setBitmasks (&vdms[i].cond);
	vdms[i].sBitPresent = sBitPresent;
	vdms[i].sBitAbsent  = sBitAbsent;
    }

    for (i = 0; i < shp_cnt; i++) {
	setBitmasks (&shps[i].cond);
	shps[i].sBitPresent = sBitPresent;
	shps[i].sBitAbsent  = sBitAbsent;
	shps[i].eBitPresent = eBitPresent;
	shps[i].eBitAbsent  = eBitAbsent;
    }

    for (i = 0; i < sbc_cnt; i++) {
	setBitmasks (&subconts[i].cond);
	subconts[i].sBitPresent = sBitPresent;
	subconts[i].sBitAbsent  = sBitAbsent;
    }

    for (i = 0; i < new_cnt; i++) {
	setBitmasks (&newmsks[i].cond);
	newmsks[i].sBitPresent = sBitPresent;
	newmsks[i].sBitAbsent  = sBitAbsent;
    }
}
