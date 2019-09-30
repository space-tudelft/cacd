/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

#include "src/makeboxl/extern.h"

#ifdef DEBUG
void pr_mcelmt (struct mc_elmt *ptr)
{
    P_E "===== struct mc_elmt: %p ======\n", ptr);
    if (ptr) {
	P_E "parent: %p\n", ptr -> parent);
	P_E "name: %s\n", ptr -> name);
	P_E "inst_name: %s\n", ptr -> inst_name);
	P_E "mtx[0-2]: %ld, %ld, %ld\n",
	    ptr -> mtx[0], ptr -> mtx[1], ptr -> mtx[2]);
	P_E "mtx[3-5]: %ld, %ld, %ld\n",
	    ptr -> mtx[3], ptr -> mtx[4], ptr -> mtx[5]);
	P_E "dx, nx, dy, ny: %ld, %ld, %ld, %ld\n",
	    ptr -> dx, ptr -> nx, ptr -> dy, ptr -> ny);
	P_E "next: %p\n\n", ptr -> mc_next);
    }
}

void pr_clist (struct clist *clp)
{
    P_E "===== struct clist: %p ======\n", clp);
    if (clp) {
	P_E "name: %s\n", clp -> name);
	P_E "pkey: %p\n", clp -> pkey);
	P_E "ckey: %p\n", clp -> ckey);
	if (clp -> ckey) {
	    P_E "ckey->cell: %s\n", clp -> ckey -> cell);
	    P_E "ckey->pkey: %p\n", clp -> ckey -> dmproject);
	}
	P_E "hier: %d\n", clp -> hier);
	P_E "status: %d\n", clp -> status); /* SdeG4.3 */
	P_E "all_allowed: %d\n", clp -> all_allowed);
	P_E "freemasks_bits: %llx\n", clp -> freemasks_bits);
	P_E "mc_p: %p\n", clp -> mc_p);
	P_E "next: %p\n\n", clp -> cl_next);
    }
}
#endif
