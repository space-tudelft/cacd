/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	A.J. van Genderen
 *	S. de Graaf
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

#include "src/sls/extern.h"

/*

void prsig1 (SIGNALELEMENT *sgn_ptr)
{
    if (sgn_ptr)
	printf ("%u\t", sgn_ptr);
    else
	printf ("%u\t\t", sgn_ptr);
    if (sgn_ptr -> sibling)
	printf ("%u\t", sgn_ptr -> sibling);
    else
	printf ("%u\t\t", sgn_ptr -> sibling);
    if (sgn_ptr -> child)
	printf ("%u\t", sgn_ptr -> child);
    else
	printf ("%u\t\t", sgn_ptr -> child);

    printf ("%d\t%d\n", sgn_ptr -> val, sgn_ptr -> len);


    if (sgn_ptr -> child)
	prsig1 (sgn_ptr -> child);

    if (sgn_ptr -> sibling)
	prsig1 (sgn_ptr -> sibling);
}

void pr_signal (SIGNALELEMENT *sgn_ptr)
{
    printf ("phead\t\tpsibling\tpchild\t\tval\tlen\n");
    prsig1 (sgn_ptr);
}

*/
