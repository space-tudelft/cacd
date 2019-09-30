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

void olap (long xl, long xr, long yb, long yt, long *wdw2, long *owdw)
{
/* determine overlap window */

    if (xl < wdw2[0]) owdw[0] = wdw2[0];
    else              owdw[0] = xl;

    if (xr > wdw2[1]) owdw[1] = wdw2[1];
    else              owdw[1] = xr;

    if (yb < wdw2[2]) owdw[2] = wdw2[2];
    else              owdw[2] = yb;

    if (yt > wdw2[3]) owdw[3] = wdw2[3];
    else              owdw[3] = yt;
}

#ifdef DEBUG
void pr_mcelmt (struct mc_elmt *ptr)
{
    P_E "============ struct mc_elmt: %08x ==============\n", ptr);
    P_E "name <inst_name>: %10s <%s>\n",
	ptr -> name, ptr -> inst_name);
    P_E "bbox[0-3]       : %ld, %ld, %ld, %ld\n",
	ptr -> bbox[0], ptr -> bbox[1],
	ptr -> bbox[2], ptr -> bbox[3]);
    P_E "mtx[0-2]        : %ld, %ld, %ld\n",
	ptr -> mtx[0], ptr -> mtx[1], ptr -> mtx[2]);
    P_E "mtx[3-5]        : %ld, %ld, %ld\n",
	ptr -> mtx[3], ptr -> mtx[4], ptr -> mtx[5]);
    P_E "dx, nx, dy, ny  : %ld, %ld, %ld, %ld\n",
	ptr -> dx, ptr -> nx, ptr -> dy, ptr -> ny);
    P_E "act_regl, parent: %08x, %08x\n",
	ptr -> act_regl, ptr -> parent);
    P_E "child, sibling  : %08x, %08x\n",
	ptr -> child, ptr -> sibling);
    P_E "====================================================\n");
}

void pr_ctree (struct ctree *ptr)
{
    P_E "====== struct ctree: %08x ======\n", ptr);
    P_E "name: %s\n", ptr -> name);
    P_E "lchild, rchild: %08x, %08x\n",
	ptr -> lchild, ptr -> rchild);
    P_E "==================================\n");
}

void pr_cptrl (struct cptrl *ptr)
{
    P_E "==== struct cptrl: %08x ====\n", ptr);
    P_E "cell, next: %08x, %08x\n",
	ptr -> cell, ptr -> next);
    P_E "===============================\n");
}

void pr_wdwl (struct wdw *ptr, char *str)
{
/* print contents of wdw list */

    P_E ">>>> %s <<<<\n", str);
    while (ptr) {
	pr_wdw (ptr);
	ptr = ptr -> next;
    }
}

void pr_wdw (struct wdw *ptr)
{
    P_E "===== struct wdw: %08x =====\n", ptr);
    P_E "wdw[0-3]: %ld, %ld, %ld, %ld\n",
	ptr -> wdw[0], ptr -> wdw[1],
	ptr -> wdw[2], ptr -> wdw[3]);
    P_E "next: %08x\n", ptr -> next);
    P_E "================================\n");
}
#endif
