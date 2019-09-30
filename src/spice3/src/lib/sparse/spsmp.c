/*
 *  Spice3 COMPATIBILITY MODULE
 *
 *  Author:                     Advising professor:
 *     Kenneth S. Kundert           Alberto Sangiovanni-Vincentelli
 *     UC Berkeley
 *
 *  This module contains routines that make Sparse1.3 a direct
 *  replacement for the SMP sparse matrix package in Spice3c1 or Spice3d1.
 *  Sparse1.3 is in general a faster and more robust package than SMP.
 *  These advantages become significant on large circuits.
 *
 *  >>> User accessible functions contained in this file:
 *  SMPaddElt
 *  SMPmakeElt
 *  SMPcClear
 *  SMPclear
 *  SMPcLUfac
 *  SMPluFac
 *  SMPcReorder
 *  SMPreorder
 *  SMPcaSolve
 *  SMPcSolve
 *  SMPsolve
 *  SMPmatSize
 *  SMPnewMatrix
 *  SMPdestroy
 *  SMPpreOrder
 *  SMPprint
 *  SMPgetError
 *  LoadGmin
 *  SMPfindElt
 */

/*
 *  To replace SMP with Sparse, rename the file spSpice3.h to
 *  spMatrix.h and place Sparse in a subdirectory of SPICE called
 *  `sparse'.  Then on UNIX compile Sparse by executing `make spice'.
 *  If not on UNIX, after compiling Sparse and creating the sparse.a
 *  archive, compile this file (spSMP.c) and spSMP.o to the archive,
 *  then copy sparse.a into the SPICE main directory and rename it
 *  SMP.a.  Finally link SPICE.
 *
 *  To be compatible with SPICE, the following Sparse compiler options
 *  (in spConfig.h) should be set as shown below:
 *
 *      REAL                            YES
 *      EXPANDABLE                      YES
 *      TRANSLATE                       NO
 *      INITIALIZE                      NO or YES, YES for use with test prog.
 *      DIAGONAL_PIVOTING               YES
 *      ARRAY_OFFSET                    YES
 *      MODIFIED_MARKOWITZ              NO
 *      DELETE                          NO
 *      STRIP                           NO
 *      MODIFIED_NODAL                  YES
 *      QUAD_ELEMENT                    NO
 *      TRANSPOSE                       YES
 *      SCALING                         NO
 *      DOCUMENTATION                   YES
 *      MULTIPLICATION                  NO
 *      DETERMINANT                     YES
 *      STABILITY                       NO
 *      CONDITION                       NO
 *      PSEUDOCONDITION                 NO
 *      FORTRAN                         NO
 *      DEBUG                           YES
 *      spCOMPLEX                       1
 *      spSEPARATED_COMPLEX_VECTORS     1
 *      spCOMPATIBILITY                 0
 *
 *      spREAL  double
 */

/*
 *  Revision and copyright information.
 *
 *  Copyright (c) 1985,86,87,88,89,90
 *  by Kenneth S. Kundert and the University of California.
 *
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted, provided
 *  that the above copyright notice appear in all copies and supporting
 *  documentation and that the authors and the University of California
 *  are properly credited.  The authors and the University of California
 *  make no representations as to the suitability of this software for
 *  any purpose.  It is provided `as is', without express or implied warranty.
 */

/*
 *  IMPORTS
 *
 *  >>> Import descriptions:
 *  spMatrix.h
 *     Sparse macros and declarations.
 *  SMPdefs.h
 *     Spice3's matrix macro definitions.
 */

#include "spice.h"
#include <stdio.h>
#include "spmatrix.h"
#include "smpdefs.h"
#include "spdefs.h"

/* #define NO   0 */
/* #define YES  1 */

static void LoadGmin(char *eMatrix, double Gmin);

int SMPaddElt(SMPmatrix *Matrix, int Row, int Col, double Value)
{
    *spGetElement((char *)Matrix, Row, Col) = Value;
    return spError((char *)Matrix);
}

double * SMPmakeElt(SMPmatrix *Matrix, int Row, int Col)
{
    return spGetElement((char *)Matrix, Row, Col);
}

void SMPcClear(SMPmatrix *Matrix)
{
    spClear((char *)Matrix);
}

void SMPclear(SMPmatrix *Matrix)
{
    spClear((char *)Matrix);
}

int SMPcLUfac(SMPmatrix *Matrix, double PivTol)
{
    spSetComplex((char *)Matrix);
    return spFactor((char *)Matrix);
}

int SMPluFac(SMPmatrix *Matrix, double PivTol, double Gmin)
{
    spSetReal((char *)Matrix);
    LoadGmin((char *)Matrix, Gmin);
    return spFactor((char *)Matrix);
}

int SMPcReorder(SMPmatrix *Matrix, double PivTol, double PivRel, int *NumSwaps)
{
    *NumSwaps = 1;
    spSetComplex((char *)Matrix);
    return spOrderAndFactor((char *)Matrix, (spREAL*)NULL, (spREAL)PivRel, (spREAL)PivTol, YES);
}

int SMPreorder(SMPmatrix *Matrix, double PivTol, double PivRel, double Gmin)
{
    spSetReal((char *)Matrix);
    LoadGmin((char *)Matrix, Gmin);
    return spOrderAndFactor((char *)Matrix, (spREAL*)NULL, (spREAL)PivRel, (spREAL)PivTol, YES);
}

void SMPcaSolve(SMPmatrix *Matrix, double RHS[], double iRHS[], double Spare[], double iSpare[])
{
    spSolveTransposed((char *)Matrix, RHS, RHS, iRHS, iRHS);
}

void SMPcSolve(SMPmatrix *Matrix, double RHS[], double iRHS[], double Spare[], double iSpare[])
{
    spSolve((char *)Matrix, RHS, RHS, iRHS, iRHS);
}

void SMPsolve(SMPmatrix *Matrix, double RHS[], double Spare[])
{
    spSolve((char *)Matrix, RHS, RHS, (spREAL*)NULL, (spREAL*)NULL);
}

int SMPmatSize(SMPmatrix *Matrix)
{
    return spGetSize((char *)Matrix, 1);
}

int SMPnewMatrix(SMPmatrix **pMatrix)
{
    int Error;
    *pMatrix = (SMPmatrix *)spCreate(0, 1, &Error);
    return Error;
}

void SMPdestroy(SMPmatrix *Matrix)
{
    spDestroy((char *)Matrix);
}

int SMPpreOrder(SMPmatrix *Matrix)
{
    spMNA_Preorder((char *)Matrix);
    return spError((char *)Matrix);
}

void SMPprint(SMPmatrix *Matrix, FILE *File)
{
    spPrint((char *)Matrix, 0, 1, 1);
}

void SMPgetError(SMPmatrix *Matrix, int *Col, int *Row)
{
    spWhereSingular((char *)Matrix, Row, Col);
}

int SMPcDProd(SMPmatrix *Matrix, SPcomplex *pMantissa, int *pExponent)
{
    double	re, im, y, z;
    int		p, ny, nz;

    spDeterminant((char *)Matrix, &p, &re, &im);

#ifndef M_LN2
#define M_LN2   0.69314718055994530942
#endif
#ifndef M_LN10
#define M_LN10  2.30258509299404568402
#endif

#ifdef debug_print
    printf("Determinant 10: (%20g,%20g)^%d\n", re, im, p);
#endif

    /* Convert base 10 numbers to base 2 numbers, for comparison */
    y = p * M_LN10 / M_LN2;
    p = (int) y;
    y -= p;

    /* ASSERT
     *	p = integral part of exponent, y = fraction part of exponent
     */

    /* Fold in the fractional part */
#ifdef debug_print
    printf(" ** base10 -> base2 int =  %d, frac = %20g\n", p, y);
#endif
    z = pow(2.0, y);
    re *= z;
    im *= z;
#ifdef debug_print
    printf(" ** multiplier = %20g\n", z);
#endif

    /* Re-normalize (re or im may be > 2.0 or both < 1.0 */
    if (re != 0.0) {
	(void) frexp(re, &ny);
	ny -= 1;
	if (im != 0.0) {
	    (void) frexp(im, &nz);
	    nz -= 1;
	} else
	    nz = 0;
    } else if (im != 0.0) {
	(void) frexp(im, &nz);
	nz -= 1;
	ny = 0;
    } else {
	/* Singular */
	/*printf("10 -> singular\n");*/
	ny = 0;
	nz = 0;
    }

#ifdef debug_print
    printf(" ** renormalize changes = %d,%d\n", ny, nz);
#endif
    if (ny < nz) ny = nz;

    *pExponent = p + ny;
    y = ldexp(re, -ny);
    z = ldexp(im, -ny);
#ifdef debug_print
    printf(" ** values are: re %g, im %g, y %d, re' %g, im' %g\n",
	    re, im, ny, y, z);
#endif
    pMantissa->real = ldexp(re, -ny);
    pMantissa->imag = ldexp(im, -ny);

#ifdef debug_print
    printf("Determinant 10->2: (%20g,%20g)^%d\n", pMantissa->real,
	pMantissa->imag, *pExponent);
#endif
    return spError((char *)Matrix);
}

/*
 *  The following routines need internal knowledge of the Sparse data
 *  structures.
 */

/*
 *  LOAD GMIN
 *
 *  This routine adds Gmin to each diagonal element.  Because Gmin is
 *  added to the current diagonal, which may bear little relation to
 *  what the outside world thinks is a diagonal, and because the
 *  elements that are diagonals may change after calling spOrderAndFactor,
 *  use of this routine is not recommended.  It is included here simply
 *  for compatibility with Spice3.
 */
static void LoadGmin(char *eMatrix, register double Gmin)
{
    MatrixPtr Matrix = (MatrixPtr)eMatrix;
    register int I;
    register ArrayOfElementPtrs Diag;
    register ElementPtr diag;

    ASSERT(IS_SPARSE(Matrix));

    if (Gmin != 0.0) {
	Diag = Matrix->Diag;
	for (I = Matrix->Size; I > 0; I--) {
	    if ((diag = Diag[I]))
		diag->Real += Gmin;
	}
    }
}

/*
 *  FIND ELEMENT
 *
 *  This routine finds an element in the matrix by row and column number.
 *  If the element exists, a pointer to it is returned.  If not, then NULL
 *  is returned unless the CreateIfMissing flag is true, in which case a
 *  pointer to the new element is returned.
 */
SMPelement * SMPfindElt(SMPmatrix *eMatrix, int Row, int Col, int CreateIfMissing)
{
    MatrixPtr Matrix = (MatrixPtr)eMatrix;
    ElementPtr Element;

    ASSERT(IS_SPARSE(Matrix));
    Row = Matrix->ExtToIntRowMap[Row];
    Col = Matrix->ExtToIntColMap[Col];
    Element = Matrix->FirstInCol[Col];
    Element = spcFindElementInCol(Matrix, &Element, Row, Col, CreateIfMissing);
    return (SMPelement *)Element;
}

/* XXX The following should probably be implemented in spUtils */

int SMPcZeroCol(MatrixPtr Matrix, int Col)
{
    ElementPtr	Element;

    Col = Matrix->ExtToIntColMap[Col];

    for (Element = Matrix->FirstInCol[Col];
	Element != NULL;
	Element = Element->NextInCol)
    {
	Element->Real = 0.0;
	Element->Imag = 0.0;
    }

    return spError((char *)Matrix);
}

int SMPcAddCol(MatrixPtr Matrix, int Accum_Col, int Addend_Col)
{
    ElementPtr	Accum, Addend, *Prev;

    Accum_Col = Matrix->ExtToIntColMap[Accum_Col];
    Addend_Col = Matrix->ExtToIntColMap[Addend_Col];

    Addend = Matrix->FirstInCol[Addend_Col];
    Prev = &Matrix->FirstInCol[Accum_Col];
    Accum = *Prev;;

    while (Addend != NULL) {
	while (Accum && Accum->Row < Addend->Row) {
	    Prev = &Accum->NextInCol;
	    Accum = *Prev;
	}
	if (!Accum || Accum->Row > Addend->Row) {
	    Accum = spcCreateElement(Matrix, Addend->Row, Accum_Col, Prev, 0);
	}
	Accum->Real += Addend->Real;
	Accum->Imag += Addend->Imag;
	Addend = Addend->NextInCol;
    }

    return spError((char *)Matrix);
}

int SMPzeroRow(MatrixPtr Matrix, int Row)
{
    ElementPtr	Element;

    Row = Matrix->ExtToIntColMap[Row];

    if (Matrix->RowsLinked == NO)
	spcLinkRows(Matrix);

#if spCOMPLEX
    if (Matrix->PreviousMatrixWasComplex OR Matrix->Complex) {
	for (Element = Matrix->FirstInRow[Row];
	    Element != NULL;
	    Element = Element->NextInRow)
	{
	    Element->Real = 0.0;
	    Element->Imag = 0.0;
	}
    } else
#endif
    {
	for (Element = Matrix->FirstInRow[Row];
	    Element != NULL;
	    Element = Element->NextInRow)
	{
	    Element->Real = 0.0;
	}
    }

    return spError((char *)Matrix);
}
