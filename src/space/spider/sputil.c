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
#include <math.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/spider/define.h"
#include "src/space/spider/recog.h"
#include "src/space/spider/extern.h"

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private meshCoor_t spiderNomDist (spider_t *sp1, spider_t *sp2);
#ifdef __cplusplus
  }
#endif

/*
 * Return TRUE if sp1, sp2, sp3 are on a straight line.
 * If so, a==s || b == s || c == s;
 * Also see the implementation of feSize.
 */

/*
 * Return TRUE if sp1, sp2, sp3 are on a straight line.
 * Calculations use nom_[xyz].
 */
bool_t spNomLinear (spider_t *sp1, spider_t *sp2, spider_t *sp3)
{
    meshCoor_t a, b, c, s;
    static double fac = -1.0;

    a = spiderNomDist (sp1, sp2);
    b = spiderNomDist (sp2, sp3);
    c = spiderNomDist (sp3, sp1);
    s = (a + b + c) / 2;

    /* To remedy the fact that due to truncation we may not find that 3
       spiders are on a straight line, we make s somewhat smaller.
    */
    if (fac < 0.0) {
#if 0
	/* The following is nice but it requires inclusion of float.h
	   and that file was not available on every machine. */
	/* We assume that meshCoor_t is of type double. */
	fac = 1.0 - pow (10.0, (double)(3 - DBL_DIG));
#else
        fac = 0.999999999;
#endif
    }
    s = s * fac;

    return ((a >= s || b >= s || c >= s) ? TRUE : FALSE);
}

/*
 * Return TRUE if sp1, sp2, sp3 are on a straight line.
 * Calculations use act_[xyz].
 */
bool_t spActLinear (spider_t *sp1, spider_t *sp2, spider_t *sp3)
{
    meshCoor_t a, b, c, s;
    static double fac = -1.0;

    a = spiderDist (sp1, sp2);
    b = spiderDist (sp2, sp3);
    c = spiderDist (sp3, sp1);
    s = (a + b + c) / 2;

    /* To remedy the fact that due to truncation we may not find that 3
       spiders are on a straight line, we make s somewhat smaller.
    */
    if (fac < 0.0) {
#if 0
	/* The following is nice but it requires inclusion of float.h
	   and that file was not available on every machine. */
	/* We assume that meshCoor_t is of type double. */
	fac = 1.0 - pow (10.0, (double)(3 - DBL_DIG));
#else
        fac = 0.999999999;
#endif
    }
    s = s * fac;

    return ((a >= s || b >= s || c >= s) ? TRUE : FALSE);
}

/*
 * Return the distance between sp1 and sp2
 */
meshCoor_t spiderDist (spider_t *sp1, spider_t *sp2)
{
    double x, y, z;

    x = sp2 -> act_x - sp1 -> act_x;
    y = sp2 -> act_y - sp1 -> act_y;
    z = sp2 -> act_z - sp1 -> act_z;

    return sqrt (x * x + y * y + z * z);
}

/*
 * Return the distance between sp1 and sp2, nominal coordinates
 */
Private meshCoor_t spiderNomDist (spider_t *sp1, spider_t *sp2)
{
    double x, y, z;

    x = sp2 -> nom_x - sp1 -> nom_x;
    y = sp2 -> nom_y - sp1 -> nom_y;
    z = sp2 -> nom_z - sp1 -> nom_z;

    return sqrt (x * x + y * y + z * z);
}

/* EOF */
