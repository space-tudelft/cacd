/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
 *	S. de Graaf
 *	A.J. van Genderen
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

#include "src/makeboxh/extern.h"

/*
** traverse the mc-tree of the cell recursively
*/
void trav_mctree (struct mc_elmt *pmc, DM_PROJECT *pkey)
{
    struct mc_elmt *p;
    struct wdw *t1, *t2;
    char *name;

    arl_ptr = &pmc -> act_regl;

    if (level > 1) { /* not root level */
	if (level == 2) {
	/*
	** each level 2 cell has an unique checktype
	*/
	    chtype += 10;
	}

	if (!*arl_ptr && (part_exp == TRUE /* AvG 911009 */)) {
	/*
	** the active region list is empty
	*/
#ifdef DEBUG
P_E "don't expand cell: %s (level = %d) no act reg list\n",
    pmc->name, level);
#endif
	    if (level >= usr_chlev) goto ret;
	}
    }

    if (!cellkey) {
	pkey = dmFindProjKey (pmc -> imported, pmc -> name,
			pkey, &name, LAYOUT);
	cellkey = dmCheckOut (pkey, name,
			ACTUAL, DONTCARE, LAYOUT, READONLY);
    }

    if (level == 1 || *arl_ptr || (part_exp == FALSE /* AvG 911009 */)) {
#ifdef DEBUG
P_E "expand cell: %s (level = %d, chtype = %d)\n", pmc->name, level, chtype);
P_E "=> read_box(pmc): %08x\n", pmc);
#endif
	read_box (pmc); /* read the boxes of the cell */

#ifdef DEBUG
P_E "=> read_term(pmc): %08x\n", pmc);
#endif
	read_term (pmc); /* read the terminals of the cell */
#ifdef DEBUG1
if(level==1) pr_wdwl(*arl_ptr,"act reg list");
#endif
    }

#ifdef DEBUG
P_E "=> read_mc(pmc): %08x\n", pmc);
#endif
    read_mc (pmc); /* read the cell calls of the cell */
#ifdef DEBUG1
for(p = pmc->child; p; p = p->sibling) pr_mcelmt(p);
#endif

    if (cellkey != top_key)
	dmCheckIn (cellkey, COMPLETE);

    cellkey = 0;

    /*
    ** make active region list;
    ** the active region list of a cell call is a list of
    ** regions in which the cell has to be expanded,
    ** i.e. outside these regions the cell is NOT expanded
    */

#ifdef DEBUG1
if (level == 1) pr_wdwl(mc_bboxl, "mc bbox list");
#endif

    if (*arl_ptr || (part_exp == FALSE /* AvG 911009 */)) {
#ifdef DEBUG
P_E "=> make_actreg(pmc): %08x\n", pmc);
#endif
	make_actreg (pmc);

#ifdef DEBUG1
P_E "free(pmc->act_regl): %08x\n", *arl_ptr);
#endif
	/* free active region list */
	for (t1 = *arl_ptr; t1; t1 = t2) {
	    t2 = t1 -> next;
	    FREE (t1);
	}
    }
    else {
	if (level >= usr_chlev) goto ret;
    }

    ++level;
    for (p = pmc -> child; p; p = p -> sibling) {
#ifdef DEBUG
P_E "=> trav_mctree(p): %08x\n", p);
#endif
	trav_mctree (p, pkey);
    }
    --level;

ret:
    if (cellkey && cellkey != top_key) {
	dmCheckIn (cellkey, COMPLETE);
	cellkey = 0;
    }
#ifdef DEBUG1
P_E "free(pmc): %08x\n", pmc);
#endif
    FREE (pmc);
#ifdef DEBUG
P_E "<= trav_mctree()\n");
#endif
}
