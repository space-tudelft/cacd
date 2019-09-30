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
#include <math.h>		/* sqrt */
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/spider/define.h"

#include "src/space/spider/recog.h"
#include "src/space/spider/extern.h"
#include "src/space/green/green.h"
#include "src/space/green/export.h"

extern bool_t optInfo;
extern bool_t check_green;

schur_t doGreen (int mode, spider_t *s1, spider_t *s2)
{
#ifdef DEBUG
    static int fast_green = -1;
#endif
    double result, newresult;

    if (s1 != s2
    &&  s1 -> act_x == s2 -> act_x
    &&  s1 -> act_y == s2 -> act_y
    &&  s1 -> act_z == s2 -> act_z) {
	say ("internal error: duplicate spider at xyz = %g %g %g (cond# %d %d)\n",
	    (double) s1 -> act_x, (double) s1 -> act_y, (double) s1 -> act_z, s1 -> conductor, s2 -> conductor);
	return ((schur_t) 0);
    }

#ifdef DEBUG
    if (fast_green == -1) fast_green = paramLookupB ("fast_green", "off");
    if (fast_green) return ((schur_t) (s1 == s2 ? 1.0 : 0.2 / spiderDist (s1, s2)));
#endif

    if (mode == DOUBLE_STRIP && (s1 -> strip == s2 -> strip) && (s1 -> strip -> flags == SINGLE_STRIP)) {
	if (fread ((char *) &result, sizeof (result), 1, s1 -> strip -> gbuf) != 1) say ("tmp file read error");
	if (check_green) {
	    newresult = green (s1, s2);
	    if (newresult != result) say ("check_green: unequal result found, diff=%g\n", newresult - result);
	}
    }
    else {
	result = green (s1, s2);
	if (optInfo) cap3dUpdateStatistics (s1, s2);
	if (mode == SINGLE_STRIP) {
	    if (fwrite ((char *) &result, sizeof (result), 1, s1 -> strip -> gbuf) != 1) say ("tmp file write error");
	}
    }

    return ((schur_t) result);
}
