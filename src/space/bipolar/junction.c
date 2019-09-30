/*
 * ISC License
 *
 * Copyright (C) 1994-2018 by
 *	Frederik Beeftink
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

#include <math.h>
#include <stdio.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/lump/extern.h"

#include "src/space/bipolar/define.h"
#include "src/space/bipolar/extern.h"

void junAdd (polnode_t *pnA, polnode_t *pnB)
{
    junction_t *j;
    int opn, spn;

    ASSERT (pnA && pnB);

    opn = OPN (pnA);
    spn = SPN (pnA);

    /* findJun */
    for (j = pnA -> juncs; j; j = j -> next[spn])
	if (j -> pn[opn] == pnB) return;

    inJun++;
    currIntJun++;

    j = NEW (junction_t, 1);

    j -> pn[spn] = pnA;
    j -> next[spn] = pnA -> juncs;
    spn = SPN (pnB);
    if (spn != opn) say ("error: invalid p/n junction for bipolar tor specified"), die ();
    j -> pn[spn] = pnB;
    j -> next[spn] = pnB -> juncs;

    pnA -> juncs = j;
    pnB -> juncs = j;
}

void junctionDel (junction_t *j, polnode_t *pn)
{
    int i;
    junction_t *jp;
    polnode_t *pni;

    currIntJun--;

    for (i = 0; i < 2; i++) {
	pni = j -> pn[i];
	ASSERT (pni);

	if ((jp = pni -> juncs) == j)
	    pni -> juncs = j -> next[i];
	else {
	    while (jp && jp -> next[i] != j) jp = jp -> next[i];

            /* I noticed that when the condition lists for
               bipolar transistors are not correctly specified,
               jp may become NULL (AvG 28 may 1999).
            */
	    if (jp) jp -> next[i] = j -> next[i];
	}

	/* I noticed that 'pni' may become NULL (AvG 3 juni 1999).
	   so I added a test on 'pni'.
	*/
	if (pni != pn && !pni -> tors && !pni -> subs && !pni -> juncs) {
	    polnodeDispose (pni);
	    j -> pn[i] = 0;
	}
    }
    DISPOSE (j, sizeof(junction_t));
}
