/*
 * ISC License
 *
 * Copyright (C) 1999-2018 by
 *	E.F. Matthijssen
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
#include <string.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/export.h"
#include "src/space/extract/extern.h"
#include "src/space/lump/extern.h"
#include "src/space/extract/define.h"

extern int msgDoubleJuncCaps;

/* Check if there are double junction capacitances with the same
 * connections (parallel or anti-parallel).
 */
void checkDoubleJuncCaps (elemDef_t *f, elemDef_t **elem, coor_t x, coor_t y)
{
    capElemDef_t *fcap, *ecap;
    elemDef_t *e = *elem;

    if (msgDoubleJuncCaps) return;

    fcap = &f -> s.cap;
    ecap = &e -> s.cap;

    if ((ecap -> nCon == fcap -> nCon && ecap -> pCon == fcap -> pCon) ||
	(ecap -> nCon == fcap -> pCon && ecap -> pCon == fcap -> nCon)) goto ret;

    /* check the remaining elements */
    while ((e = *++elem) && e -> type == f -> type) {
	ecap = &e -> s.cap;
	if (capPolarityTab[ecap -> sortNr] != 'x') {
	    if ((ecap -> nCon == fcap -> nCon && ecap -> pCon == fcap -> pCon) ||
		(ecap -> nCon == fcap -> pCon && ecap -> pCon == fcap -> nCon)) goto ret;
	}
    }
    return;
ret:
    msgDoubleJuncCaps = 1;
    say ("warning: double junction capacitances found:\n\telements '%s' and '%s' at position %s\n",
	f -> name, e -> name, strCoorBrackets (x, y));
}
