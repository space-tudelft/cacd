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
#include "src/space/spider/export.h"

extern bool_t useAdptvIntgrtn, forceAdptvIntgrtn; /* U.G. */
extern green_t greenEps;

static pointR3_t *Cp1, *Cp2, *Cp3, *Cp4;

/*
 * Compute the Galerkin integral for the
 * elements given by (op1, op2, op3, op4) and (cp1, cp2, cp3, cp4).
 *
 * If FeModePwl == TRUE, assume a piece-wise linear shape function.
 * Also, op1 and cp1 are the points where shape-function is non-zero
 * in case of linear elements.
 * Thus, op1 and cp1 are the nodes with which the value in the
 * influence matrix must be associated.
 *
 * By convention, op4 == op1 and/or cp4 == cp1 in case of triangles.
 *
 * In the case of linear shape functions, one call will generally not
 * produce the full value of the entry in the influence matrix,
 * you will have to iterate over all triangles connected to
 * op1 and over all triangles connected to cp1.
 * Moreover, the result must still be normalized
 * (to the area of the elements). See green.c for how this is done.
 * For PWL, The algorithm is in fact something as follows:
 *
 *      ac = 0, result = 0
 *      for all triangles (c) connected to cp1
 *          ac += area (c), ao = 0, o_result = 0
 *          for all triangles (o) connected to op1
 *              ao += area (o)
 *              o_result += doGalerkinGreen (op1, , , , cp1, , , )
 *          result += o_result / ao;
 *      result /= ac
 *
 * Of course, it does not matter which one is the inner loop.
 * See als green.c.
 */

Private green_t Inner (pointR3_t *opp)
{
    return doCollocationGreen (*opp, *Cp1, *Cp2, *Cp3, *Cp4);
}

Private void noConvergence_c (green_t error)
{
    static green_t maxError = 0;

    if (error > maxError) {
	char s[100];
	maxError = error;
	sprintf (s, "==> largest estimated error so far: %g", (double) error);
	say ("Galerkin integral not converged,\n%s", s);
    }
}

green_t doGalerkinGreen (pointR3_t op1, pointR3_t op2, pointR3_t op3, pointR3_t op4,
			 pointR3_t cp1, pointR3_t cp2, pointR3_t cp3, pointR3_t cp4)
{
    integrate_t result;

    Cp1 = &cp1;
    Cp2 = &cp2;
    Cp3 = &cp3;
    Cp4 = &cp4;

/* U.G.: conditional use of integrate2DAdptv */
    if (!forceAdptvIntgrtn)
	result = integrate2D (FeModePwl, Inner, &op1, &op2, &op3, &op4, 1000, greenEps);

    if (forceAdptvIntgrtn || (result.error > greenEps && useAdptvIntgrtn)) {
	if (!forceAdptvIntgrtn) {
	    fprintf(stderr, "with conventional integration routine\n");
	    fprintf(stderr, "   result.value = %10.3e, result.error = %10.3e, greenEps = %10.3e\n",
		result.value, result.error, greenEps);
	}
	result = integrate2DAdptv (FeModePwl, Inner, &op1, &op2, &op3, &op4, 10000, greenEps);
	if (!forceAdptvIntgrtn) {
	    fprintf(stderr, "with adaptive integration routine\n");
	    fprintf(stderr, "   result.value = %10.3e, result.error = %10.3e, greenEps = %10.3e\n\n",
		result.value, result.error, greenEps);
	}
    }

    if (result.error > greenEps) noConvergence_c (result.error);

    Debug (fprintf (stderr, "galerkin result %e error estimate %e\n", result.value, result.error));

    return (result.value);
}

#ifdef DRIVER
#include "src/space/green/gputil.h"
#ifdef __cplusplus
  extern "C" {
#endif
char *giveICD (char *filepath);
#ifdef __cplusplus
  }
#endif

coor_t bbxl, bbxr, bbyb, bbyt; /* NOT USED: only for SUBS */
dielectric_t * diels;
substrate_t * substrs;
int diel_cnt = 0;
int substr_cnt = 0;
int greenType, greenCase = 0; /* DIEL */
int FeModeGalerkin = 1, FeModePwl = 0;
char *usedTechFile = (char*)"xxx";
char *giveICD (char *path) { return path; }

Private green_t Inner2D (pointR3_t *opp)
{
    return doCollocationGreen2D (*opp, *Cp1, *Cp2);
}

Private green_t doGalerkinGreen2D (pointR3_t op1, pointR3_t op2, pointR3_t cp1, pointR3_t cp2)
{
    integrate_t result;

    op1.y = op2.y = cp1.y = cp2.y = 0;

    Cp1 = &cp1;
    Cp2 = &cp2;

    result = integrate1D (FeModePwl, Inner2D, &op1, &op2, 1000, greenEps);

    if (result.error > greenEps) noConvergence_c (result.error);

    Debug (fprintf (stderr, "galerkin result %e error estimate %e\n", result.value, result.error));

    return (result.value);
}

/*
 * Test driver for doGalerkinGreen.
 */
int main ()
{
    green_t a, b;
    pointR3_t o1, o2, o3;
    pointR3_t c1, c2, c3;

    verboseSetMode (TRUE);

    diel_cnt = 1;
    diels = NEW (dielectric_t, diel_cnt);
    strcpy (diels[0].name, "a"); /* dielectric1 */
    diels[0].permit = 1.0;
    diels[0].bottom = 0.0;

    (void) greenInit (1.0);
    useAdptvIntgrtn = 1; /* to use also adaptive integration */

    R3Assign (o1, 4,  0, 1.0868);
    R3Assign (o2, 4, 80, 1.614);
    R3Assign (o3, 4,  0, 1.614);

    R3Assign (c1, 4,  0, 1.0868);
    R3Assign (c2, 4, 80, 1.0868);
    R3Assign (c3, 4, 80, 1.614);

    a = doGalerkinGreen (o1, o2, o3, o1, c1, c2, c3, c1);
    b = doGalerkinGreen (c1, c2, c3, c1, o1, o2, o3, o1);

    fprintf (stderr, "a: %g, b: %g, should be equal.\n", a, b);

    R2Assign (o1, 0,  0.01);
    R2Assign (o2, 10, 0.01);

    a = doGalerkinGreen2D (o1, o2, o1, o2);
    b = doGalerkinGreen2D (o2, o1, o1, o2);

    fprintf (stderr, "a: %g, b: %g, should be equal.\n", a, b);

    return (0);
}
#endif /* DRIVER */
