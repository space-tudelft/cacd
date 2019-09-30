/**********
Copyright 1990 Regents of the University of California. All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*
 * NIinit
 *  Initialize the Numerical iteration package to perform Newton-Raphson
 *  iterations on a sparse matrix filled by the specified load package
 */

#include "spice.h"
#include "cktdefs.h"
#include <stdio.h>
#include "util.h"
#include "sperror.h"
#include "smpdefs.h"

int NIinit(CKTcircuit *ckt)
{
#ifdef SPARSE
/* a concession to Ken Kundert's sparse matrix package - SMP doesn't need this*/
    int Error;
#endif /* SPARSE */
    ckt->CKTniState = NIUNINITIALIZED;
    return(SMPnewMatrix(&ckt->CKTmatrix));
}
