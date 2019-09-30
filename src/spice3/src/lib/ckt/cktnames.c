/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Thomas L. Quarles
**********/

/*
 * CKTnames(ckt)
 *  output information on all circuit nodes/equations
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "ifsim.h"
#include "iferrmsg.h"
#include "util.h"

int CKTnames(CKTcircuit *ckt, int *numNames, IFuid **nameList)
{
    register CKTnode *here;
    register int i;
    register IFuid *nList;

    *numNames = ckt->CKTmaxEqNum - 1;
    *nameList = (IFuid *)MALLOC(*numNames * sizeof(IFuid));
    if (!*nameList) return(E_NOMEM);
    i = 0;
    nList = *nameList;
    for (here = ckt->CKTnodes->next; here; here = here->next) {
	nList[i++] = here->name;
    }
    return(OK);
}

int CKTdnames(CKTcircuit *ckt)
{
    CKTnode *here;

    for (here = ckt->CKTnodes->next; here; here = here->next) {
        printf("%03d: %s\n", here->number, (char*)here->name);
    }
    return(OK);
}
