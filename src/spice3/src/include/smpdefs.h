#ifndef SMP
#define SMP

typedef char SMPmatrix;
typedef struct MatrixElement *SMPelement;

/**********
Copyright 1990 Regents of the University of California. All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "complex.h"
#include <stdio.h>

int SMPaddElt(SMPmatrix *, int, int, double);
void SMPcClear(SMPmatrix *);
int SMPcLUfac(SMPmatrix *, double);
int SMPcReorder(SMPmatrix *, double, double, int *);
void SMPcSolve(SMPmatrix *, double [], double [], double [], double []);
void SMPclear(SMPmatrix *);
void SMPcolSwap(SMPmatrix *, int, int);
void SMPdestroy(SMPmatrix *);
int SMPfillup(SMPmatrix *);
SMPelement * SMPfindElt(SMPmatrix *, int, int, int);
void SMPgetError(SMPmatrix *, int *, int *);
int SMPluFac(SMPmatrix *, double, double);
double * SMPmakeElt(SMPmatrix *, int, int);
int SMPmatSize(SMPmatrix *);
int SMPnewMatrix(SMPmatrix **);
int SMPnewNode(int, SMPmatrix *);
int SMPpreOrder(SMPmatrix *);
void SMPprint(SMPmatrix *, FILE *);
int SMPreorder(SMPmatrix *, double, double, double);
void SMProwSwap(SMPmatrix *, int, int);
void SMPsolve(SMPmatrix *, double [], double []);

#endif /*SMP*/
