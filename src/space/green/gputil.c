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
#include "src/space/green/green.h"
#include "src/space/green/extern.h"

/*
 * Return the euclidean distance between p1 and p2
 */
real_t r3Dist (pointR3_t *p1, pointR3_t *p2)
{
    real_t dx, dy, dz;
    dx = p1 -> x - p2 -> x;
    dy = p1 -> y - p2 -> y;
    dz = p1 -> z - p2 -> z;
    return (Sqrt (dx * dx + dy * dy + dz * dz));
}

/*
 * Return the square of euclidean distance between p1 and p2
 */
real_t r3Dist2 (pointR3_t *p1, pointR3_t *p2)
{
    real_t dx, dy, dz;
    dx = p1 -> x - p2 -> x;
    dy = p1 -> y - p2 -> y;
    dz = p1 -> z - p2 -> z;
    return (dx * dx + dy * dy + dz * dz);
}

/*
 * Return area of triangle (p1,p2,p3)
 */
real_t r3TriArea (pointR3_t *p1, pointR3_t *p2, pointR3_t *p3)
{
    real_t a, b, c, s;

    /* compute triangle area
     * cf. schaum, s = semiperimeter
     */
    a = r3Dist (p1, p2);
    b = r3Dist (p2, p3);
    c = r3Dist (p3, p1);
    s = (a + b + c) / 2;
    return (Sqrt (s * (s - a) * (s - b) * (s - c)));
}

/*
 * Return area of quadrilateral (p1,p2,p3,p4)
 */
real_t r3QuadArea (pointR3_t *p1, pointR3_t *p2, pointR3_t *p3, pointR3_t *p4)
{
    return (r3TriArea (p1, p2, p3) + r3TriArea (p1, p3, p4));
}

/*
 * Print pointR3_t p, prefixed by string s, to stderr.
 */
void r3Print (char *s, pointR3_t *p)
{
    fprintf (stderr, "\t\t\t%s: %g %g %g\n", s, p -> x, p -> y, p -> z);
}
