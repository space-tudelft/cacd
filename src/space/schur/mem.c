/*
 * ISC License
 *
 * Copyright (C) 1989-2018 by
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

#include "src/space/include/config.h"
#include <stdio.h>
#include "src/space/auxil/auxil.h"
#include "src/space/include/schur.h"
#include "src/space/schur/define.h"
#include "src/space/schur/extern.h"

schur_t **scIN, *scDIAG, **scV, **scM, *scP, *scP1; // **scOUT;
static schur_t *inS, *vS, *mS; // *resultS;

int *scOrder, *scPIV;
int maxSchurSize;

void newMatDispose (schur_t **mat, schur_t *S, int dOld, int oOld)
{
    DISPOSE (S, dOld * oOld * sizeof(schur_t));
    DISPOSE (mat, dOld * sizeof(schur_t *));
}

void newVecDispose (schur_t *vec, int dOld)
{
    DISPOSE (vec, dOld * sizeof(schur_t));
}

void newIntVecDisp (int *vec, int dOld)
{
    DISPOSE (vec, dOld * sizeof(int));
}

/*
 * Increases memory for (approximate) matrix inversion such that
 * it can handle a matrix of dimension (nMaxo+1) x (nMaxo+1).
 * Needs as arguments new value for max. dimension (nMaxd) rows
 * and new value (nMaxo) order and global variable that indicates
 * if LU decomposition should be used or not (luFact).
 */
int newSchurMem (int nMaxd, int nMaxo)
{
    static int dMax;
    static int oMax;

    if (scIN) {
	newMatDispose (scIN, inS, dMax, oMax);
	newIntVecDisp (scOrder, dMax);
     // newMatDispose (scV, vS, oMax, oMax);
	newMatDispose (scM, mS, oMax, oMax);
#ifdef TEST_SCHUR
      if (luFact) {
     // newIntVecDisp (scPIV, oMax);
      }
      else {
#endif
	newVecDispose (scDIAG, dMax);
	newVecDispose (scP,  oMax);
	newVecDispose (scP1, oMax);
    //  newMatDispose (scOUT, resultS, dMax, oMax);
#ifdef TEST_SCHUR
      }
#endif
    }

    dMax = 2 * nMaxo + 1;
    if (dMax > nMaxd + 1) dMax = nMaxd + 1;
    oMax = nMaxo + 1;
    maxSchurSize = 0;

    newMat (&scIN, &inS, dMax, oMax);
    newIntVec (&scOrder, dMax);
 // newMat (&scV, &vS, oMax, oMax);
    newMat (&scM, &mS, oMax, oMax);

#ifdef TEST_SCHUR
    if (luFact) {
	scPIV = scOrder;
     // newIntVec (&scPIV, oMax);
	return (dMax);
    }
#endif

    newVec (&scDIAG, dMax);
    newVec (&scP,  oMax);
    newVec (&scP1, oMax);
 // newMat (&scOUT, &resultS, dMax, oMax);
    return (dMax);
}

/*
 * Allocate memory for a schur_t matrix.
 */
void newMat (schur_t ***mat, schur_t **S, int dNew, int oNew)
{
    schur_t **newmat, *newS;
    int k;

    newmat = NEW (schur_t *, dNew);
    maxSchurSize += dNew * sizeof(schur_t *);
    k = dNew * oNew;
    newS = NEW (schur_t, k);
    maxSchurSize += k * sizeof(schur_t);

    for (k = 0; k < dNew; k++) newmat[k] = newS + k * oNew;

    *S = newS; /* this one should be saved since memory is shifted ! */
    *mat = newmat;
}

/*
 * Allocate memory for a schur_t vector.
 */
void newVec (schur_t **vec, int dNew)
{
    *vec = NEW (schur_t, dNew);
    maxSchurSize += dNew * sizeof(schur_t);
}

/*
 * Allocate memory for an integer vector.
 */
void newIntVec (int **vec, int dNew)
{
    *vec = NEW (int, dNew);
    maxSchurSize += dNew * sizeof(int);
}
