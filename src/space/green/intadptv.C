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

/********************************************************
 *  File "intgrtAdptv.C"                                *
 *  Contains 2-d adaptive integration routine for the   *
 *    SPACE project.  More description can be found at  *
 *    the beginning of the function "intgrtTrnglAdptv"  *
 *  Author:  U. Geigenmuller                            *
 *  last revision: Feb. 8, 1996                         *
 ********************************************************/

//#define PLOTGRID
	/* define this to get the generated mesh
	 * printed to a file.  May be used to investigate
	 * the reason of convergence problems.
	 * But beware: the generated file may by HUGE!
	 */

//#define TESTRUN /* for testing intgrtTrnglAdptv() */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/green/gputil.h"
#include "src/space/green/green.h"

extern "C" {
integrate_t integrate2DAdptv (bool_t do_shape,
	real_t (*func)(pointR3_t *),
	pointR3_t *p1, pointR3_t *p2, pointR3_t *p3, pointR3_t *p4,
	int maxTriNodes, real_t reqRelPrec);
}

Private integrate_t intgrtTrnglAdptv (bool_t do_shape,
	real_t (*func)(pointR3_t *),
	pointR3_t *p1, pointR3_t *p2, pointR3_t *p3,
	int maxTriNodes, real_t reqRelPrec);

#ifdef TESTRUN
Private int callcount;
Private int choice;
Private int EXACT;
#endif

typedef struct TriangPara { /* collection of parameters used repeatedly */
  int nrOfTriNodes;
  pointR3_t locex, locOrigin;
  real_t (*func)(pointR3_t *), height, base;
  bool_t do_shape;
} TriangPara_t;

class TriNode { /* a point where the integrand is evaluated */
public:
    TriangPara_t *para;
    int nrOfLinks;
    pointR3_t R;
    real_t itgrdVal;
    TriNode (pointR3_t, TriangPara_t *);
};

TriNode::TriNode (pointR3_t R_in, TriangPara_t *para_in)
{
  para = para_in;
  (para -> nrOfTriNodes)++;
  nrOfLinks = 0;  // counts nr. of links from any Triangles
  R3Copy (R_in, R);
  real_t factor = 1.;

  if (para -> do_shape) {
    pointR3_t auxvec;
    R3Subtract (R, para -> locOrigin, auxvec);
    factor = R3Dot (auxvec, para -> locex);
    factor = 3. - 3. * factor / para -> height;
  }

  if (factor > 1.e-6) itgrdVal = (para -> func)(&R) * factor;
  else itgrdVal = 0.;

#ifdef TESTRUN
  static FILE *itgrd = 0;
/*if (!itgrd) itgrd = fopen ("itgrid", "w"); */
  if (itgrd) {
    fprintf (itgrd, "%15.9e %15.9e %15.9e\n", R.x, R.y, itgrdVal);
    fflush (itgrd);
  }
#endif
}

class Triangle {
//private:
/* the declaration `private' is not really necessary, since everything
   is private be default.  Putting the declaration in causes the compiler
   to complain about syntax-errors when auxil.h is included ... because
   there 'private' is re-defined as static!  So just leave out 'private'
   here.
*/
  TriNode* node[7];
  TriangPara_t *para;
  real_t area, fraction;
  int generation, isParent, isGrandparent, isChildNr;
  Triangle *child[4], *parent;
public:
  real_t integral, roughAbsErrEst;
  Triangle (TriNode* node_in[7], int generation_in,
                   Triangle* parent_in, int isChildNr_in);
  ~Triangle ();
  void newGeneration (real_t * reqAbsPrec);
  void integrate (real_t *sum);
  void getAbsErrEst (real_t * totAbsErrEst);
  void giveBirth ();
#ifdef PLOTGRID
  void grid (FILE *);
#endif
#ifdef TESTRUN
  void intinfo ();
#endif
};

Triangle::~Triangle ()
{
  int i;
  if (isParent) for (i = 0; i < 4; i++) delete child[i];

  for (i = 0; i < 7; i++) {
    node[i] -> nrOfLinks -= 1;
    if (node[i] -> nrOfLinks == 0) delete node[i];
  }
  if (isChildNr >= 0) {
    parent -> isParent -= 1;
    parent -> child[isChildNr] = 0;
  }
}

Triangle::Triangle (TriNode* node_in[7], int generation_in,
                   Triangle* parent_in, int isChildNr_in)
{
  isParent = 0;
  isGrandparent = 0;
  parent = parent_in;
  para = node_in[0] -> para;
  generation = generation_in;
  if (generation > 0) {
    isChildNr = isChildNr_in;
    parent -> isParent += 1;
    fraction = parent -> fraction / 4.;
    area = parent -> area / 4.;
  }
  else {
    isChildNr = -1;
    parent = 0;
    area = para -> height * para -> base / 2.;
    fraction = 1.;
  }
  for (int i = 0; i < 7; i++) {
    node[i] = node_in[i];
    node[i] -> nrOfLinks += 1;
  }

  integral =
    (   3 * (node[0] -> itgrdVal + node[2] -> itgrdVal + node[4] -> itgrdVal)
     +  8 * (node[1] -> itgrdVal + node[3] -> itgrdVal + node[5] -> itgrdVal)
     + 27 * node[6] -> itgrdVal
    ) * area / 60.;

  real_t integral1pt = node[6] -> itgrdVal * area;
  real_t integral3pt = (node[1] -> itgrdVal +
                        node[3] -> itgrdVal + node[5] -> itgrdVal) * area / 3.;
  roughAbsErrEst = Fabs (integral3pt - integral1pt);
}

void Triangle::newGeneration (real_t *reqAbsPrec)
{
  int i;

  if (isGrandparent) {
    for (i = 0; i < 4; i++) child[i] -> newGeneration (reqAbsPrec);
  }
  else {
    if (isParent) {
      real_t sumChildInt = 0.;
      for (i = 0; i < 4; i++) sumChildInt += child[i] -> integral;
      real_t absErrEst = Fabs (integral - sumChildInt);  // improve error est.
      if (absErrEst  > (*reqAbsPrec) * fraction)
        for (i = 0; i < 4; i++) child[i] -> giveBirth ();
    }
    else {
      if (roughAbsErrEst > (*reqAbsPrec) * fraction)
         this -> giveBirth ();
    }
  }
}

void Triangle::giveBirth ()
{
  TriNode * auxTriNode[7];
  pointR3_t auxvec, auxvec1;

  if (parent) parent -> isGrandparent = 1;

 /***** Child 0 *****/
  auxTriNode[0] = node[0];

  R3Add (node[0] -> R, node[1] -> R, auxvec);
  R3Scalar2 (auxvec, 2., auxvec);
  auxTriNode[1] = new TriNode (auxvec, para);
  ASSERT (auxTriNode[1]);

  auxTriNode[2] = node[1];

  R3Add (node[1] -> R, node[5] -> R, auxvec);
  R3Scalar2 (auxvec, 2., auxvec);
  auxTriNode[3] = new TriNode (auxvec, para);
  ASSERT (auxTriNode[3]);

  auxTriNode[4] = node[5];

  R3Add (node[0] -> R, node[5] -> R, auxvec1);
  R3Scalar2 (auxvec1, 2., auxvec);
  auxTriNode[5] = new TriNode (auxvec, para);
  ASSERT (auxTriNode[5]);

  R3Add (node[1] -> R, auxvec1, auxvec);
  R3Scalar2 (auxvec, 3., auxvec);
  auxTriNode[6] = new TriNode (auxvec, para);
  ASSERT (auxTriNode[6]);

  child[0] = new Triangle (auxTriNode, generation+1, this, 0);
  ASSERT (child[0]);

 /***** Child 1 *****/
  auxTriNode[0] = node[1];

  R3Add (node[1] -> R, node[2] -> R, auxvec);
  R3Scalar2 (auxvec, 2., auxvec);
  auxTriNode[1] = new TriNode (auxvec, para);
  ASSERT (auxTriNode[1]);

  auxTriNode[2] = node[2];

  R3Add (node[2] -> R, node[3] -> R, auxvec);
  R3Scalar2 (auxvec, 2., auxvec);
  auxTriNode[3] = new TriNode (auxvec, para);
  ASSERT (auxTriNode[3]);

  auxTriNode[4] = node[3];

  R3Add (node[1] -> R, node[3] -> R, auxvec1);
  R3Scalar2 (auxvec1, 2., auxvec);
  auxTriNode[5] = new TriNode (auxvec, para);
  ASSERT (auxTriNode[5]);

  R3Add (node[2] -> R, auxvec1, auxvec);
  R3Scalar2 (auxvec, 3., auxvec);
  auxTriNode[6] = new TriNode (auxvec, para);
  ASSERT (auxTriNode[6]);

  child[1] = new Triangle (auxTriNode, generation+1, this, 1);
  ASSERT (child[1]);

 /***** Child 2 *****/
  auxTriNode[0] = node[3];

  R3Add (node[3] -> R, node[5] -> R, auxvec);
  R3Scalar2 (auxvec, 2., auxvec);
  auxTriNode[1] = new TriNode (auxvec, para);
  ASSERT (auxTriNode[1]);

  auxTriNode[2] = node[5];
  auxTriNode[3] = child[0] -> node[3];
  auxTriNode[4] = node[1];
  auxTriNode[5] = child[1] -> node[5];
  auxTriNode[6] = node[6];

  child[2] = new Triangle (auxTriNode, generation+1, this, 2);
  ASSERT (child[2]);

 /***** Child 3 *****/
  auxTriNode[0] = node[5];
  auxTriNode[1] = child[2] -> node[1];
  auxTriNode[2] = node[3];

  R3Add (node[3] -> R, node[4] -> R, auxvec);
  R3Scalar2 (auxvec, 2., auxvec);
  auxTriNode[3] = new TriNode (auxvec, para);
  ASSERT (auxTriNode[3]);

  auxTriNode[4] = node[4];

  R3Add (node[4] -> R, node[5] -> R, auxvec1);
  R3Scalar2 (auxvec1, 2., auxvec);
  auxTriNode[5] = new TriNode (auxvec, para);
  ASSERT (auxTriNode[5]);

  R3Add (node[3] -> R, auxvec1, auxvec);
  R3Scalar2 (auxvec, 3., auxvec);
  auxTriNode[6] = new TriNode (auxvec, para);
  ASSERT (auxTriNode[6]);

  child[3] = new Triangle (auxTriNode, generation+1, this, 3);
  ASSERT (child[3]);
}

void Triangle::integrate (real_t *sum)
{
  if (isParent) {
    int i;
    for (i = 0; i < 4; i++) child[i] -> integrate (sum);
  }
  else {
   *sum += integral;
#ifdef TESTRUN
 /* callcount += 7; */
    callcount += 1;
#endif
  }
}

void Triangle::getAbsErrEst (real_t *totAbsErrEst)
{
  int i;
  if (isGrandparent) {
    for (i = 0; i < 4; i++) child[i] -> getAbsErrEst (totAbsErrEst);
  }
  else {
    if (isParent) {
      real_t sumChildInt = 0.;
      for (i = 0; i < 4; i++) sumChildInt += child[i] -> integral;
      *totAbsErrEst += Fabs (integral - sumChildInt);
    }
    else *totAbsErrEst += roughAbsErrEst;
  }
}

#ifdef PLOTGRID
void Triangle::grid (FILE *gridfile)
{
  if (isParent) {
    for (int i = 0; i < 4; i++) child[i] -> grid (gridfile);
  }
  else {
    fprintf (gridfile, "%11.5e %11.5e %11.5e\n", (node[0] -> R).x, (node[0] -> R).y, (node[0] -> R).z);
    fprintf (gridfile, "%11.5e %11.5e %11.5e\n", (node[2] -> R).x, (node[2] -> R).y, (node[2] -> R).z);
    fprintf (gridfile, "%11.5e %11.5e %11.5e\n", (node[4] -> R).x, (node[4] -> R).y, (node[4] -> R).z);
    fprintf (gridfile, "%11.5e %11.5e %11.5e\n\n", (node[0] -> R).x, (node[0] -> R).y, (node[0] -> R).z);
  }
}
#endif

#ifdef TESTRUN
void Triangle::intinfo ()
{
}
#endif

Private integrate_t intgrtTrnglAdptv (bool_t do_shape,
	real_t (*func)(pointR3_t *),
	pointR3_t *p1, pointR3_t *p2, pointR3_t *p3,
	int maxTriNodes, real_t reqRelPrec)
/*
 *  "intgrtTrnglAdptv" = "integrate over triangular domain, adaptive method"
 *  Integration over triangular domain, using 1, 3 and 7 point integration
 *  rules suggested in "Finite element methods" (Handbook of Numerical
 *  Analysis, vol. II), edited by P.G. Ciarlet & J.L. Lions, pp 187-189.
 *  If the required relative precision cannot be obtained, the routine
 *  adapts to the situation by splitting the integration domain into
 *  4 similar triangles and trying again.  This refinement of the
 *  integration mesh is repeated for those triangles where the error
 *  is still too large, until the required precision is achieved or
 *  the specified maximum number of integrand evaluations (maxTriNodes)
 *  is exceeded significantly.  For do_shape == TRUE, the integrand
 *  func is multiplied with a linear weight function vanishing on
 *  the line from corner *p2 to corner *p3 of the triangle; the normalization
 *  is such that for func == 1 the result of the integral gives the
 *  area of the triangle.
 *    In general, the routine is slower and requires more integrand
 *  evaluations than the routine integrateTriangle.  Due to the possibility
 *  to adapt,  however, it still works where the latter fails, which is the
 *  case for very short distance between potential source and integration
 *  domain.
 */
{
  TriNode* auxTriNode[7];
  pointR3_t auxvec, avec, bvec, locOrigin, locex, locey, locez;
  real_t base, height;

#ifdef TESTRUN
#define maxiter 20
  int nrOfFctnCalls[maxiter+1], iter;
  real_t integralApprox[maxiter+1], totAbsErrEst[maxiter+1];
  for (int i = 0; i <= maxiter; i++) integralApprox[i] = totAbsErrEst[i] = 0;
#endif

  /* determine local basis vectors etc. */
  R3Copy (*p1, locOrigin);
  R3Subtract (*p2, *p1, avec);
  R3Subtract (*p3, *p1, bvec);
  R3Subtract (bvec, avec, locey);
  base = R3Norm (locey);
  R3Normalize (locey);
  R3Cross (avec, bvec, locez);
  R3Normalize (locez);
  R3Cross (locey, locez, locex);
  height = R3Dot (avec, locex);

// make a set of parameters available to root triangle and all descendents
  TriangPara para;
  para.func = func;
  para.height = height;
  para.base = base;
  R3Copy (locOrigin, para.locOrigin);
  R3Copy (locex, para.locex);
  para.nrOfTriNodes = 0;
  para.do_shape = do_shape;

// make nodes of root triangle
  auxTriNode[0] = new TriNode (*p1, &para);
  ASSERT (auxTriNode[0]);

  R3Add (*p1, *p2, auxvec);
  R3Scalar2 (auxvec, 2., auxvec);
  auxTriNode[1] = new TriNode (auxvec, &para);
  ASSERT (auxTriNode[1]);

  auxTriNode[2] = new TriNode (*p2, &para);
  ASSERT (auxTriNode[2]);

  R3Add (*p2, *p3, auxvec);
  R3Scalar2 (auxvec, 2., auxvec);
  auxTriNode[3] = new TriNode (auxvec, &para);
  ASSERT (auxTriNode[3]);

  auxTriNode[4] = new TriNode (*p3, &para);
  ASSERT (auxTriNode[4]);

  R3Add (*p1, *p3, auxvec);
  R3Scalar2 (auxvec, 2., auxvec);
  auxTriNode[5] = new TriNode (auxvec, &para);
  ASSERT (auxTriNode[5]);

  R3Add (*p1, *p3, auxvec);
  R3Add (*p2, auxvec, auxvec);
  R3Scalar2 (auxvec, 3., auxvec);
  auxTriNode[6] = new TriNode (auxvec, &para);
  ASSERT (auxTriNode[6]);

// creat root triangle
  Triangle *triptr;
  triptr = new Triangle (auxTriNode, 0, 0, -1);
  ASSERT (triptr);
  real_t value = 0, absErrEst = 0;
#ifdef TESTRUN
  callcount = 0;
#endif
  triptr -> integrate (&value);
  triptr -> getAbsErrEst (&absErrEst);

#ifdef TESTRUN
  iter = 1;
  integralApprox[iter] = value;
  totAbsErrEst[iter] = absErrEst;
  nrOfFctnCalls[iter] = callcount;
#endif

// start refining
  real_t reqAbsPrec;
  reqAbsPrec = reqRelPrec * value;
  if (maxTriNodes < 19) maxTriNodes = 19; /* at least one refinement */
  while (absErrEst > reqAbsPrec && para.nrOfTriNodes < maxTriNodes) {
    triptr -> newGeneration (&reqAbsPrec);
    value = absErrEst = 0;
#ifdef TESTRUN
    callcount = 0;
#endif
    triptr -> integrate (&value);
    triptr -> getAbsErrEst (&absErrEst);
    reqAbsPrec = reqRelPrec * value;
#ifdef TESTRUN
    if (iter+1 <= maxiter) {
      iter++;
      integralApprox[iter] = value;
      totAbsErrEst[iter] = absErrEst;
      nrOfFctnCalls[iter] = callcount;
      real_t change = Fabs (integralApprox[iter] - integralApprox[iter-1]);
      if (choice == 0)
        printf ("iter=%2d intgrl=%10.5e nrCalls=%4d req.absPrec=%10.5e change=%10.5e absErrEst=%10.5e\n",
            iter, integralApprox[iter], nrOfFctnCalls[iter], reqAbsPrec, change, totAbsErrEst[iter]);
    }
#endif
  }

#ifdef TESTRUN
  real_t RESULT = value; /* ? */
  static FILE *rslt = 0;

  if (EXACT == 0 && choice == 0) {
    real_t actualError, incError, estError;
    if (!rslt) rslt = fopen ("rslt", "w");
    for (int i = 1; i <= iter; i++) {
      incError = integralApprox[i-1] / integralApprox[i] - 1.;
      actualError = integralApprox[i] / RESULT - 1.;
      estError = totAbsErrEst[i] / integralApprox[i];
      fprintf (rslt, "%2d %15.7e %15.7e %15.7e %5d %15.7e %15.7e\n",
		i, integralApprox[i], actualError, Fabs (actualError),
		nrOfFctnCalls[i], Fabs (incError), Fabs (estError));
    }
    triptr -> intinfo ();
  }
  real_t incRelErrEstN = Fabs (integralApprox[iter-1] / integralApprox[iter] - 1.);
  if (choice == 0) printf ("incRelErrEstN=%10.5e\n", incRelErrEstN);
#endif

#ifdef PLOTGRID
  static FILE *gridfile = 0;
  if (!gridfile) gridfile = fopen ("grid", "w");
  triptr -> grid (gridfile);
  fflush (gridfile);
  char dummy;
  printf ("grid written to file grid, enter any character to continue\n");
  scanf ("%c", &dummy);
#endif

// delete root triangle
  delete triptr;
  integrate_t res;
  res.value = value;
  res.error = absErrEst / value;
  return res;
}

integrate_t integrate2DAdptv (bool_t do_shape,
	real_t (*func)(pointR3_t *),
	pointR3_t *p1, pointR3_t *p2, pointR3_t *p3, pointR3_t *p4,
	int maxTriNodes, real_t reqRelPrec)
{
  integrate_t result;

  ASSERT (reqRelPrec >= 0);

  if (!p4 || p4 == p1 || (p4 -> x == p1 -> x && p4 -> y == p1 -> y && p4 -> z == p1 -> z)) {
    /* integration domain is a triangle */
      result = intgrtTrnglAdptv (do_shape, func, p1, p2, p3, maxTriNodes, reqRelPrec);
  }
  else {
    /* The quadrilateral integration domain is handled by
     * splitting it up into two triangles.  This is not the most
     * efficient way, but it should work --- and things were
     * handled that way in the non-adaptive routine,
     * too, except for the special case of a parallelogram.
     * This code has basically been copied from integrat.c, in particular,
     * the splitting of the quadrilateral into two triangles:
     * p1 p2 p3 p4  -->  p1 p2 p3  and  p1 p3 p4
     * This splitting is  o n l y  OK if p1 ... p4 are  o r d e r e d  along
     * the periphery.
     */
    ASSERT (do_shape == FALSE);
    integrate_t r1, r2;

    r1 = intgrtTrnglAdptv (FALSE, func, p1, p2, p3, maxTriNodes, reqRelPrec);
    r2 = intgrtTrnglAdptv (FALSE, func, p1, p3, p4, maxTriNodes, reqRelPrec);
    result.value = r1.value + r2.value;
    result.error = (r1.error * r1.value + r2.error * r2.value) / result.value;
  }
  return (result);
}
