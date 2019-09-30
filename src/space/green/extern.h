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

#include "src/space/green/export.h"

#ifdef __cplusplus
  extern "C" {
#endif

/* init.c */
char *layerName (int i);
void greenNoConvergence (int i, int j, int terms);

/* analytic.c */
real_t analyticPotIntegral   (int do_shape, pointR3_t *vr, pointR3_t *vr1, pointR3_t *vr2, pointR3_t *vr3, pointR3_t *vr4);
real_t analyticPotIntegral2D (int do_shape, pointR3_t *vr, pointR3_t *vrm, pointR3_t *vrp);

/* colloc.c */
green_t doCollocationGreen   (pointR3_t pp, pointR3_t e1, pointR3_t e2, pointR3_t e3, pointR3_t e4);
green_t doCollocationGreen2D (pointR3_t pp, pointR3_t e1, pointR3_t e2);
green_t greenFunc   (pointR3_t *p1, pointR3_t *p2);
green_t greenFunc2D (pointR3_t *p1, pointR3_t *p2);

/* galerkin.c */
green_t doGalerkinGreen (pointR3_t op1, pointR3_t op2, pointR3_t op3, pointR3_t op4,
			 pointR3_t cp1, pointR3_t cp2, pointR3_t cp3, pointR3_t cp4);

/* gputil.c */
real_t r3Dist     (pointR3_t *p1, pointR3_t *p2);
real_t r3Dist2    (pointR3_t *p1, pointR3_t *p2);
real_t r3TriArea  (pointR3_t *p1, pointR3_t *p2, pointR3_t *p3);
real_t r3QuadArea (pointR3_t *p1, pointR3_t *p2, pointR3_t *p3, pointR3_t *p4);
void r3Print (char *s, pointR3_t *p);

/* intadptv.C */
integrate_t integrate2DAdptv (bool_t do_shape, real_t (*func)(pointR3_t *op),
    pointR3_t *p1, pointR3_t *p2, pointR3_t *p3, pointR3_t *p4, int maxNrOfNodes, real_t reqRelPrec);

/* integrat.c */
integrate_t integrate2D (int do_shape, real_t (*func)(pointR3_t *op),
    pointR3_t *p1, pointR3_t *p2, pointR3_t *p3, pointR3_t *p4, int max_refines, real_t eps);
integrate_t integrate1D (int do_shape, real_t (*func)(pointR3_t *op),
    pointR3_t *p1, pointR3_t *p2, int max_refines, real_t eps);

/* mkgreen.c */
void cap3dSettings (int n, double mat [], double bnd []);
void sub3dSettings (int n, double mat [], double bnd []);
imageGroup_t getImageGroup (int iLayer, int jLayer, int group_num);
int mediumNumber  (real_t z);
int mediumNumber2 (real_t z, real_t z2);
int mediumNumber4 (real_t z1, real_t z2, real_t z3, real_t z4);

/* mpgreen.c */
#ifdef TEST_MULTIPOLE_METHOD
void TestMultipoles (spider_t *spo, spider_t *spc, green_t val, int cnt);
#endif
bool_t greenMpole (spider_t *spo, spider_t *spc, green_t *val);

#ifdef __cplusplus
  }
#endif
